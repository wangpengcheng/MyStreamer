#include "net_connector.h"
#include "net_channel.h"
#include "net_event_loop.h"
#include "net_sockets_ops.h"
#include "logging.h"

#include <errno.h>

using namespace MY_NAME_SPACE;
using namespace MY_NAME_SPACE::net;

/* 初始化最大尝试数量 */
const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop *loop, const InetAddress &serverAddr)
    : loop_(loop),/* 事件循环指针 */
      serverAddr_(serverAddr),/* 服务器地址 */
      connect_(false), /* 是否连接 */
      state_(kDisconnected),/* 没有链接 */
      retryDelayMs_(kInitRetryDelayMs) /* 最大延迟时间 */
{
    LOG_DEBUG << "ctor[" << this << "]";
}

Connector::~Connector()
{
    LOG_DEBUG << "dtor[" << this << "]";
    assert(!channel_);
}

void Connector::start()
{
    connect_ = true;
    /* 开始连接处理事件循环 */
    loop_->runInLoop(std::bind(&Connector::startInLoop, this)); // FIXME: unsafe
}
/* 开始循环 */
void Connector::startInLoop()
{
    loop_->assertInLoopThread();
    assert(state_ == kDisconnected);
    /* 进行连接处理 */
    if (connect_)
    {
        connect();
    }
    else
    {
        LOG_DEBUG << "do not connect";
    }
}

void Connector::stop()
{
    connect_ = false;
    loop_->queueInLoop(std::bind(&Connector::stopInLoop, this)); // FIXME: unsafe
    // FIXME: cancel timer
}

void Connector::stopInLoop()
{
    loop_->assertInLoopThread();
    if (state_ == kConnecting)
    {
        setState(kDisconnected);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}
/* tcp connect 连接处理函数 */
void Connector::connect()
{
    /* 创建非阻塞的socket连接描述符 */
    int sockfd = sockets::createNonblockingOrDie(serverAddr_.family());
    /* 绑定地址连接 */
    int ret = sockets::connect(sockfd, serverAddr_.getSockAddr());
    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno)
    {
        case 0:
        case EINPROGRESS:/* 连接正在处理，网卡忙 */
        case EINTR:/* 陷入系统中断 */
        case EISCONN: /* 已经建立连接 */
            connecting(sockfd); /* 执行连接函数 */
            break;

        case EAGAIN:   /* 需要重试 */
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:  /* 没有到达网络的可用路由 */
            retry(sockfd); /* 重新尝试 */
            break;

        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK: /* 非socket上执行socket操作，不是socket文件描述符 */
            LOG_SYSERR << "connect error in Connector::startInLoop " << savedErrno;
            sockets::close(sockfd);
            break;

        default:
            LOG_SYSERR << "Unexpected error in Connector::startInLoop " << savedErrno;
            sockets::close(sockfd);
            // connectErrorCallback_();
            break;
    }
}
/* 开启事件循环 */
void Connector::restart()
{
    loop_->assertInLoopThread();
    setState(kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_ = true;
    startInLoop();
}
/* 执行连接函数 */
void Connector::connecting(int sockfd)
{
    setState(kConnecting);
    assert(!channel_);
    /* 重新设置channel；注意channel在这里创建*/
    channel_.reset(new Channel(loop_, sockfd));
    /* 设置写回调 */
    channel_->setWriteCallback(
        std::bind(&Connector::handleWrite, this)); // FIXME: unsafe
    /* 设置错误回调 */
    channel_->setErrorCallback(
        std::bind(&Connector::handleError, this)); // FIXME: unsafe

    // channel_->tie(shared_from_this()); is not working,
    // as channel_ is not managed by shared_ptr
    /* 设置监听事件为可写事件 */
    channel_->enableWriting();
}
/* 重新设置监听事件 */
int Connector::removeAndResetChannel()
{
    channel_->disableAll();
    channel_->remove();
    int sockfd = channel_->fd();
    // Can't reset channel_ here, because we are inside Channel::handleEvent
    /* 将重设事件添加到事件循环中 */
    loop_->queueInLoop(std::bind(&Connector::resetChannel, this)); // FIXME: unsafe
    return sockfd;
}

void Connector::resetChannel()
{
    channel_.reset();
}
// 执行写入
void Connector::handleWrite()
{
    LOG_TRACE << "Connector::handleWrite " << state_;

    if (state_ == kConnecting)
    {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        if (err)
        {
            LOG_WARN << "Connector::handleWrite - SO_ERROR = "
                     << err << " " << strerror_tl(err);
            retry(sockfd);
        }
        /* 是否为本地回环连接 */
        else if (sockets::isSelfConnect(sockfd))
        {
            LOG_WARN << "Connector::handleWrite - Self connect";
            retry(sockfd);
        }
        else
        {
            setState(kConnected);
            /* 执行连接建立回调 */
            if (connect_)
            {
                newConnectionCallback_(sockfd);
            }
            else
            {
                sockets::close(sockfd);
            }
        }
    }
    else
    {
        // what happened?
        assert(state_ == kDisconnected);
    }
}

void Connector::handleError()
{
    LOG_ERROR << "Connector::handleError state=" << state_;
    if (state_ == kConnecting)
    {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err);
        retry(sockfd);
    }
}
/* 重新连接 */
void Connector::retry(int sockfd)
{
    /* 关闭原来的连接 */
    sockets::close(sockfd);
    setState(kDisconnected);
    if (connect_)
    {
        LOG_INFO << "Connector::retry - Retry connecting to " << serverAddr_.toIpPort()
                 << " in " << retryDelayMs_ << " milliseconds. ";
        /* 一段时间后重新执行 */
        loop_->runAfter(retryDelayMs_ / 1000.0,
                        std::bind(&Connector::startInLoop, shared_from_this()));
        /* 更新重新连接的时间 */
        retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
    }
    else
    {
        LOG_DEBUG << "do not connect";
    }
}