#include "net_tcp_connection.h"
#include "net_tcp_server.h"
#include "net_acceptor.h"
#include "net_event_loop.h"
#include "net_event_loop_threadpool.h"
#include "net_sockets_ops.h"



#include <stdio.h>  // snprintf

using namespace MY_NAME_SPACE;
using namespace MY_NAME_SPACE::net;

TcpServer::TcpServer(EventLoop* loop,
                     const InetAddress& listenAddr,
                     const string& nameArg,
                     Option option)
    : loop_(CHECK_NOTNULL(loop)),
        ipPort_(listenAddr.toIpPort()),
        name_(nameArg),
        acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),/* 使用loop初始化accpter */
        threadPool_(new EventLoopThreadPool(loop, name_)),
        connectionCallback_(defaultConnectionCallback),
        messageCallback_(defaultMessageCallback),
        nextConnId_(1)
{
     /* 
    * 设置回调函数，当有客户端请求时，Acceptor接收客户端请求，然后调用这里设置的回调函数
    * 回调函数用于创建TcpConnection连接
    */
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer()
{
    loop_->assertInLoopThread();
    LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";

    for (auto& item : connections_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        /* 绑定可读写时的执行函数，注意这里是在loop中循环执行；因此 */
        conn->getLoop()->runInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadNum(int numThreads)
{
    assert(0 <= numThreads);
    threadPool_->setThreadNum(numThreads);
}
/* 开始服务器 */
void TcpServer::start()
{
    /* 开启 */
    if (started_.getAndSet(1) == 0)
    {
        /* 一般这里是一个空函数，注意这里已经有baseloop了*/
        threadPool_->start(threadInitCallback_);
        /* 将acceptor设置为监听状态 */
        assert(!acceptor_->listenning());
        /* 运行监听函数 */
          /* 
        * Acceptor和TcpServer在同一个线程，通常会直接调用 
        * std::bind只能值绑定，如果传入智能指针会增加引用计数，这里传递普通指针
        * 因为TcpServer没有销毁，所以不用担心Acceptor会销毁；绑定主要监听函数
        */
        loop_->runInLoop(
            std::bind(&Acceptor::listen, get_pointer(acceptor_)));
    }
}
/* 
 * Acceptor接收客户端请求后调用的回调函数
 * @param sockfd: 已经接收完成（三次握手完成）后的客户端套接字
 * @param peerAddr: 客户端地址
 * 
 * Acceptor只负责接收客户端请求
 * TcpServer需要生成一个TcpConnection用于管理tcp连接
 * 
 * 1.TcpServer内有一个EventLoopThreadPool，即事件循环线程池，池子中每个线程都是一个EventLoop
 * 2.每个EventLoop包含一个Poller用于监听注册到这个EventLoop上的所有Channel
 * 3.当建立起一个新的TcpConnection时，这个连接会放到线程池中的某个EventLoop中
 * 4.TcpServer中的baseLoop只用来检测客户端的连接
 * 
 * 从libevent的角度看就是
 * 1.EventLoopThreadPool是一个struct event_base的池子，池子中全是struct event_base
 * 2.TcpServer独占一个event_base，这个event_base不在池子中
 * 3.TcpConnection会扔到这个池子中的某个event_base中
 */
void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
    loop_->assertInLoopThread();
    /* 从事件驱动线程池中取出一个线程给TcpConnection，如果没有就是baseloop */
    EventLoop* ioLoop = threadPool_->getNextLoop();
    /* 为TcpConnection生成独一无二的名字 */
    char buf[64];
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    string connName = name_ + buf;
     /* 
    * 根据sockfd获取tcp连接在本地的<地址，端口>
    * getsockname(int fd, struct sockaddr*, int *size);
    */
    LOG_INFO << "TcpServer::newConnection [" << name_
            << "] - new connection [" << connName
            << "] from " << peerAddr.toIpPort();
    /*  */
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    // FIXME poll with zero timeout to double confirm the new connection
    // FIXME use make_shared if necessary

    /* 创建一个新的TcpConnection代表一个Tcp连接 */
    TcpConnectionPtr conn(new TcpConnection(ioLoop,
                                            connName,
                                            sockfd,
                                            localAddr,
                                            peerAddr));
    /* 添加到所有tcp 连接的map中，键是tcp连接独特的名字（服务器名+客户端<地址，端口>） */
    connections_[connName] = conn;
    /* 为tcp连接设置回调函数（由用户提供） */
    conn->setConnectionCallback(connectionCallback_);
    /* 设置消息回调函数 */
    conn->setMessageCallback(messageCallback_);
    /* 设置写入回调函数 */
    conn->setWriteCompleteCallback(writeCompleteCallback_);
     /* 
    * 关闭回调函数，由TcpServer设置，作用是将这个关闭的TcpConnection从map中删除
    * 当poll返回后，发现被激活的原因是EPOLLHUP，此时需要关闭tcp连接
    * 调用Channel的CloseCallback，进而调用TcpConnection的handleClose，进而调用removeConnection
    */
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection, this, _1)); // FIXME: unsafe
    /* 
    * 连接建立后，调用TcpConnection连接建立成功的函数
    * 1.新建的TcpConnection所在事件循环是在事件循环线程池中的某个线程
    * 2.所以TcpConnection也就属于它所在的事件驱动循环所在的那个线程
    * 3.调用TcpConnection的函数时也就应该在自己所在线程调用
    * 4.所以需要调用runInLoop在自己的那个事件驱动循环所在线程调用这个函数
    * 5.当前线程是TcpServer的主线程，不是TcpConnection的线程，如果在这个线程直接调用会阻塞监听客户端请求
    * 6.其实这里不是因为线程不安全，即使在这个线程调用也不会出现线程不安全，因为TcpConnection本就是由这个线程创建的
    */
    /* 注意:
        这里的 newConnection 一般由TCPsever 主线程，也就是Acceptorz执行，
        所以注入的函数，不会马上执行
        而是调用event_loop thread的queueInLoop将函数添加到执行队列中，在下一次循环中运行；保证线程的独立性
        
    */
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    // FIXME: unsafe
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}
/* 移除连接 */
void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    loop_->assertInLoopThread();
    LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
            << "] - connection " << conn->name();
    size_t n = connections_.erase(conn->name());
    (void)n;
    assert(n == 1);
    //获取链接所在线程
    EventLoop* ioLoop = conn->getLoop();
    //将销毁线程添加到循环队列中
    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn));
}

