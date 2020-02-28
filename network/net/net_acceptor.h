#ifndef NET_ACCEPTOR_H
#define NET_ACCEPTOR_H

#include <functional>

#include "net_channel.h"
#include "net_socket.h"


NAMESPACE_START



namespace net
{

class EventLoop;
class InetAddress;

/**
 * 网络连接管理类，供TCP server使用，声明周期由Tcpserver决定
*/
/* 
 * 对TCP socket, bind, listen, accept的封装 
 * 将sockfd以Channel的形式注册到EventLoop的Poller中，检测到sockfd可读时，接收客户端
 */
class Acceptor : noncopyable
{
public:
    /* 定义回调函数雷子那个 */
    typedef std::function<void (int sockfd, const InetAddress&)> NewConnectionCallback;
    /* 构造函数，监听地址 */
    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
    ~Acceptor();
    /* 由服务器TcpServer设置的回调函数，在接收完客户端请求后执行，用于创建TcpConnection */
    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { newConnectionCallback_ = cb; }
    /* 是否正在监听 */
    bool listenning() const { return listenning_; }
    void listen();

private:
    void handleRead();

    EventLoop* loop_;
    Socket acceptSocket_;                           /* 封装了socket文件描述符分生命周期，指向server的监听socket */
    Channel acceptChannel_;                         /* Channel，保存着sockfd，被添加到Poller中，等待被激活 */
    /* 
   * 当有客户端连接时首先内部接收连接，然后调用的用户提供的回调函数
   * 客户端套接字和地址作为参数传入
   */
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;
    int idleFd_;
};
 /* 
   * Tcp连接建立的流程
   *    1.服务器调用socket,bind,listen开启监听套接字监听客户端请求
   *    2.客户端调用socket,connect连接到服务器
   *    3.第一次握手客户端发送SYN请求分节（数据序列号）
   *    4.服务器接收SYN后保存在本地然后发送自己的SYN分节（数据序列号）和ACK确认分节告知客户端已收到
   *      同时开启第二次握手
   *    5.客户端接收到服务器的SYN分节和ACK确认分节后保存在本地然后发送ACK确认分节告知服务器已收到
   *      此时第二次握手完成，客户端connect返回
   *      此时，tcp连接已经建立完成，客户端tcp状态转为ESTABLISHED，而在服务器端，新建的连接保存在内核tcp
   *      连接的队列中，此时服务器端监听套接字变为可读，等待服务器调用accept函数取出这个连接
   *    6.服务器接收到客户端发来的ACK确认分节，服务器端调用accept尝试找到一个空闲的文件描述符，然后
   *      从内核tcp连接队列中取出第一个tcp连接，分配这个文件描述符用于这个tcp连接
   *      此时服务器端tcp转为ESTABLISHED，三次握手完成，tcp连接建立
   *      
   * 服务器启动时占用的一个空闲文件描述符，/dev/null,作用是解决文件描述符耗尽的情况
   * 原理如下：
   *    当服务器端文件描述符耗尽，当客户端再次请求连接，服务器端由于没有可用文件描述符
   *        会返回-1，同时errno为EMFILE，意为描述符到达hard limit，无可用描述符，此时服务器端
   *        accept函数在获取一个空闲文件描述符时就已经失败，还没有从内核tcp连接队列中取出tcp连接
   *        这会导致监听套接字一直可读，因为tcp连接队列中一直有客户端的连接请求
   *        
   *    所以服务器在启动时打开一个空闲描述符/dev/null(文件描述符)，先站着'坑‘，当出现上面
   *        情况accept返回-1时，服务器暂时关闭idleFd_让出'坑'，此时就会多出一个空闲描述符
   *        然后再次调用accept接收客户端请求，然后close接收后的客户端套接字，优雅的告诉
   *        客户端关闭连接，然后再将'坑'占上
   */
}  // namespace net

NAMESPACE_END
#endif