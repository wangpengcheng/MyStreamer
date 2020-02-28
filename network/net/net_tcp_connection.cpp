#include "net_tcp_connection.h"
#include "logging.h"
#include "weak_callback.h"
#include "net_event_loop.h"
#include "net_socket.h"
#include "net_sockets_ops.h"
#include "net_channel.h"

#include <errno.h>
using namespace MY_NAME_SPACE;
using namespace MY_NAME_SPACE::net;

/* 设置默认连接回调函数 */
void MY_NAME_SPACE::net::defaultConnectionCallback(const TcpConnectionPtr& conn)
{
    LOG_TRACE << conn->localAddress().toIpPort() << " -> "
                << conn->peerAddress().toIpPort() << " is "
                << (conn->connected() ? "UP" : "DOWN");
    // do not call conn->forceClose(), because some users want to register message callback only.
}
/* 设置默认的消息回调 */
void MY_NAME_SPACE::net::defaultMessageCallback(const TcpConnectionPtr&,
                                        Buffer* buf,
                                        Timestamp)
{
    /* 清空buffer */
    buf->retrieveAll();
}
/* 
 * 构造函数，设置当fd就绪时调用的回调函数
 * Channel代表一个对fd事件的监听
 */
TcpConnection::TcpConnection(EventLoop* loop,
                             const string& nameArg,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
  : loop_(CHECK_NOTNULL(loop)),
    name_(nameArg),
    state_(kConnecting),
    reading_(true),
    socket_(new Socket(sockfd)),
    channel_(new Channel(loop, sockfd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr),
    highWaterMark_(64*1024*1024)
{
    /* 设置各种回调函数 */
    channel_->setReadCallback(
        std::bind(&TcpConnection::handleRead, this, _1));
    channel_->setWriteCallback(
        std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(
        std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(
        std::bind(&TcpConnection::handleError, this));
    LOG_DEBUG << "TcpConnection::ctor[" <<  name_ << "] at " << this
                << " fd=" << sockfd;
    /*
    * 设置KEEP-ALIVE属性，如果客户端很久没有和服务器通讯，tcp会自动判断客户端是否还处于连接（类似心跳包）
    * 
    * int setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &sockopt, static_cast<socklen_t>(sizeof(sockopt)));
    */
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG << "TcpConnection::dtor[" <<  name_ << "] at " << this
                << " fd=" << channel_->fd()
                << " state=" << stateToString();
    assert(state_ == kDisconnected);
}

bool TcpConnection::getTcpInfo(struct tcp_info* tcpi) const
{
    return socket_->getTcpInfo(tcpi);
}

string TcpConnection::getTcpInfoString() const
{
    char buf[1024];
    buf[0] = '\0';
    socket_->getTcpInfoString(buf, sizeof buf);
    return buf;
}
/* 发送指定长度的data */
void TcpConnection::send(const void* data, int len)
{
    send(StringPiece(static_cast<const char*>(data), len));
}

void TcpConnection::send(const StringPiece& message)
{
    if (state_ == kConnected)
    {   
        /* 检查事件线程是否正在运行 */
        if (loop_->isInLoopThread())
        {
            sendInLoop(message);/* 直接发送消息 */
        }
        else
        {
            void (TcpConnection::*fp)(const StringPiece& message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(
                std::bind(fp,
                            this,     // FIXME
                            message.as_string()));
                            //std::forward<string>(message)));
        }
    }
}

// FIXME efficiency!!!
void TcpConnection::send(Buffer* buf)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll();
        }
        else
        {
            void (TcpConnection::*fp)(const StringPiece& message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(
                std::bind(fp,
                            this,     // FIXME
                            buf->retrieveAllAsString()));
                            //std::forward<string>(message)));
        }
    }
}
/* 发送消息 */
void TcpConnection::sendInLoop(const StringPiece& message)
{
    sendInLoop(message.data(), message.size());
}
/* 发送消息 */

/* 
 * 写入数据
 * 1.如果Channel没有监听可写事件且输出缓冲区为空，说明之前没有出现内核缓冲区满的情况，直接写进内核
 * 2.如果写入内核出错，且出错信息(errno)是EWOULDBLOCK，说明内核缓冲区满，将剩余部分添加到应用层输出缓冲区
 * 3.如果之前输出缓冲区为空，那么就没有监听内核缓冲区(fd)可写事件，开始监听
 */
void TcpConnection::sendInLoop(const void* data, size_t len)
{
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if (state_ == kDisconnected)
    {
        LOG_WARN << "disconnected, give up writing";
        return;
    }
    // if no thing in output queue, try writing directly
    /* 如果输出缓冲区有数据，就不能尝试发送数据了，否则数据会乱，应该直接写到缓冲区中 */
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = sockets::write(channel_->fd(), data, len);
        if (nwrote >= 0)
        {
            remaining = len - nwrote;
            /* 存在写入回调函数，就执行 */
            if (remaining == 0 && writeCompleteCallback_)
            {
                /* 将写入回调添加到队列中 */
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else // nwrote < 0
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                LOG_SYSERR << "TcpConnection::sendInLoop";
                if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
                {
                    faultError = true;
                }
            }
        }
    }

    assert(remaining <= len);
    /* 检查是否还有没有写完 */
    if (!faultError && remaining > 0)
    {
        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= highWaterMark_
            && oldLen < highWaterMark_
            && highWaterMarkCallback_)
        {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }
            outputBuffer_.append(static_cast<const char*>(data)+nwrote, remaining);
        if (!channel_->isWriting())
        {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdown()
{
    // FIXME: use compare and swap
    if (state_ == kConnected)
    {
        setState(kDisconnecting);
        // FIXME: shared_from_this()?
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if (!channel_->isWriting())
    {
        // we are not writing
        /* 关闭写入 */
        socket_->shutdownWrite();
    }
}

// void TcpConnection::shutdownAndForceCloseAfter(double seconds)
// {
//   // FIXME: use compare and swap
//   if (state_ == kConnected)
//   {
//     setState(kDisconnecting);
//     loop_->runInLoop(std::bind(&TcpConnection::shutdownAndForceCloseInLoop, this, seconds));
//   }
// }

// void TcpConnection::shutdownAndForceCloseInLoop(double seconds)
// {
//   loop_->assertInLoopThread();
//   if (!channel_->isWriting())
//   {
//     // we are not writing
//     socket_->shutdownWrite();
//   }
//   loop_->runAfter(
//       seconds,
//       makeWeakCallback(shared_from_this(),
//                        &TcpConnection::forceCloseInLoop));
// }

void TcpConnection::forceClose()
{
    // FIXME: use compare and swap
    if (state_ == kConnected || state_ == kDisconnecting)
    {
        setState(kDisconnecting);
        loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }
}

void TcpConnection::forceCloseWithDelay(double seconds)
{
    if (state_ == kConnected || state_ == kDisconnecting)
    {
        setState(kDisconnecting);
        loop_->runAfter(
            seconds,
            makeWeakCallback(shared_from_this(),
                            &TcpConnection::forceClose));  // not forceCloseInLoop to avoid race condition
    }
}

void TcpConnection::forceCloseInLoop()
{
    loop_->assertInLoopThread();
    if (state_ == kConnected || state_ == kDisconnecting)
    {
        // as if we received 0 byte in handleRead();
        handleClose();
    }
}

const char* TcpConnection::stateToString() const
{
    switch (state_)
    {
        case kDisconnected:
            return "kDisconnected";
        case kConnecting:
            return "kConnecting";
        case kConnected:
            return "kConnected";
        case kDisconnecting:
            return "kDisconnecting";
        default:
            return "unknown state";
    }
}

void TcpConnection::setTcpNoDelay(bool on)
{
    socket_->setTcpNoDelay(on);
}

void TcpConnection::startRead()
{
    loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::startReadInLoop()
{
    loop_->assertInLoopThread();
    if (!reading_ || !channel_->isReading())
    {
        channel_->enableReading();
        reading_ = true;
    }
}

void TcpConnection::stopRead()
{
    loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

void TcpConnection::stopReadInLoop()
{
    loop_->assertInLoopThread();
    if (reading_ || channel_->isReading())
    {
        channel_->disableReading();
        reading_ = false;
    }
}
/* 
 * 1.创建服务器(TcpServer)时，创建Acceptor，设置接收到客户端请求后执行的回调函数
 * 2.Acceptor创建监听套接字，将监听套接字绑定到一个Channel中，设置可读回调函数为Acceptor的handleRead
 * 3.服务器启动，调用Acceptor的listen函数创建监听套接字，同时将Channel添加到Poller中
 * 4.有客户端请求连接，监听套接字可读，Channel被激活，调用可读回调函数(handleRead)
 * 5.回调函数接收客户端请求，获得客户端套接字和地址，调用TcpServer提供的回调函数(newConnection)
 * 6.TcpServer的回调函数中创建TcpConnection代表这个tcp连接，设置tcp连接各种回调函数(由用户提供给TcpServer)
 * 7.TcpServer让tcp连接所属线程调用TcpConnection的connectEstablished
 * 8.connectEstablished开启对客户端套接字的Channel的可读监听，然后调用用户提供的回调函数
 */
/* 连接剑姬之后 */
void TcpConnection::connectEstablished()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    /* Channel中对TcpConnection的弱引用在这里设置 */
    channel_->tie(shared_from_this());
    /* 设置对可读事件的监听，同时将Channel添加到Poller中 */
    channel_->enableReading();
    /* 用户提供的回调函数，在连接建立成功后调用 */
    connectionCallback_(shared_from_this());
}
/* 销毁连接 */
void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    if (state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();

        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}
/* 读取回调 */
void TcpConnection::handleRead(Timestamp receiveTime)
{
    loop_->assertInLoopThread();
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0)
    {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if (n == 0)
    {
        handleClose();
    }
    else
    {
        errno = savedErrno;
        LOG_SYSERR << "TcpConnection::handleRead";
        handleError();
    }
}
/* 写入回调 */
/* 当tcp缓冲区可写时调用 */
void TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();
    if (channel_->isWriting())
    {
        /* 尝试写入写缓冲区的所有数据，返回实际写入的字节数（tcp缓冲区很有可能仍然不能容纳所有数据） */
        ssize_t n = sockets::write(channel_->fd(),
                                outputBuffer_.peek(),
                                outputBuffer_.readableBytes());
        if (n > 0)
        {
            outputBuffer_.retrieve(n);
            
            if (outputBuffer_.readableBytes() == 0)
            {
                /* 全部写到tcp缓冲区中，关闭对可写事件的监听 */
                channel_->disableWriting();
                /* 如果有写入完成时的回调函数（用户提供，则等待函数结束后调用 */
                if (writeCompleteCallback_)
                {
                    /* 写入回调 */
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                /* 
                * 如果连接正在关闭（通常关闭读端），那么关闭写端，但是是在已经写完的前提下
                * 如果还有数据没有写完，不能关闭，要在写完再关 
                */
                if (state_ == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {
        LOG_SYSERR << "TcpConnection::handleWrite";
        // if (state_ == kDisconnecting)
        // {
        //   shutdownInLoop();
        // }
        }
    }
    else
    {
        LOG_TRACE << "Connection fd = " << channel_->fd()
                << " is down, no more writing";
    }
}
/* 客户提供的关闭回调函数 */
void TcpConnection::handleClose()
{
    loop_->assertInLoopThread();
    LOG_TRACE << "fd = " << channel_->fd() << " state = " << stateToString();
    assert(state_ == kConnected || state_ == kDisconnecting);
    // we don't close fd, leave it to dtor, so we can find leaks easily.
    setState(kDisconnected);
    channel_->disableAll();
    /* 此时当前的TcpConnection的引用计数为2，一个是guardThis，另一个在TcpServer的connections_中 */
    TcpConnectionPtr guardThis(shared_from_this());
    /*  连接回调 */
    connectionCallback_(guardThis);
      /* 
   * closeCallback返回后，TcpServer的connections_(tcp连接map）已经将TcpConnection删除，引用计数变为1
   * 此时如果函数返回，guardThis也会被销毁，引用计数变为0，这个TcpConnection就会被销毁
   * 所以在TcpServer::removeConnectionInLoop使用bind将TcpConnection生命期延长，引用计数加一，变为2
   * 就算guardThis销毁，引用计数仍然有1个
   * 等到调用完connectDestroyed后，bind绑定的TcpConnection也会被销毁，引用计数为0，TcpConnection析构
   */
    closeCallback_(guardThis);
}

void TcpConnection::handleError()
{
    int err = sockets::getSocketError(channel_->fd());
    LOG_ERROR << "TcpConnection::handleError [" << name_
                << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}