#include "net_acceptor.h"
#include "logging.h"
#include "net_event_loop.h"
#include "net_inet_address.h"
#include "net_sockets_ops.h"

#include <errno.h>
#include <fcntl.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <unistd.h>

using namespace MY_NAME_SPACE;
using namespace MY_NAME_SPACE::net;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
    : loop_(loop),
        acceptSocket_(sockets::createNonblockingOrDie(listenAddr.family())),
        acceptChannel_(loop, acceptSocket_.fd()),   /*  */
        listenning_(false),
        idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
    assert(idleFd_ >= 0);
     /* 
   * setsockopt设置套接字选项SO_REUSEADDR；地址重用，对于端口bind，如果这个地址/端口处于TIME_WAIT，也可bind成功
   * int flag = 1;
   * setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
   */
    acceptSocket_.setReuseAddr(true);
    /*
   * setsockopt设置套接字选项SO_REUSEPORT；端口重用，作用是对于多核cpu，允许在同一个<ip, port>对上运行多个相同服务器
   * 内核会采用负载均衡的的方式分配客户端的连接请求给某一个服务器
   */
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);
    /* Channel设置读事件的回调函数，此时还没有开始监听这个Channel，需要调用Channel::enableReading() */
    acceptChannel_.setReadCallback(
        std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
    ::close(idleFd_);
}

void Acceptor::listen()
{
    loop_->assertInLoopThread();
    listenning_ = true;
    acceptSocket_.listen();
    /* 
   * 开始监听Channel，也就是设置fd关心的事件(EPOLLIN/EPOLLOUT等)，然后添加到Poller中 
   * Poller中保存着所有注册到EventLoop中的Channel
   */
    acceptChannel_.enableReading();
}
/*
 * 当有客户端尝试连接服务器时，监听套接字变为可读，epoll_wait/poll返回
 * EventLoop处理激活队列中的Channel，调用对应的回调函数
 * 监听套接字的Channel的回调函数是handleRead()，用于接收客户端请求
 */
void Acceptor::handleRead()
{
    loop_->assertInLoopThread();
    InetAddress peerAddr;
    /* 执行accept监听文件描述符 */
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0)
    {
        // string hostport = peerAddr.toIpPort();
        // LOG_TRACE << "Accepts of " << hostport;
        /* 检查是否存在回调函数 */
         /* 
     * 如果设置了回调函数，那么就调用，参数是客户端套接字和地址/端口
     * 否则就关闭连接，因为并没有要处理客户端的意思
     * 
     * 这个回调函数是TcpServer中的newConnection，用于创建一个TcpConnection连接
     */
        if (newConnectionCallback_)
        {
            /* 主要用于，构造tcpconnecttion,注意这里的connfd和peerAddr*/
            newConnectionCallback_(connfd, peerAddr);
        }
        else
        {
            sockets::close(connfd);
        }
    }
    else /* 发生错误 ，文件描述符号耗尽时*/
    {
        LOG_SYSERR << "in Acceptor::handleRead";
        // Read the section named "The special problem of
        // accept()ing when you can't" in libev's doc.
        // By Marc Lehmann, author of libev.
        if (errno == EMFILE)
        {
            /* 关闭空闲文件描述符 */
            ::close(idleFd_);
            /* 接收新的描述符 */
            idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
            /* 关闭新的文件描述符 */
            ::close(idleFd_);
            idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}