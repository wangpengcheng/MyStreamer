#ifndef NET_TCPCONNECTION_H
#define NET_TCPCONNECTION_H
#include "uncopyable.h"
#include "string_piece.h"
#include "base_types.h"
#include "net_buffer.h"
#include "net_inet_address.h"
#include "net_callbacks.h"

#include <memory>
#include <boost/any.hpp>

// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;


NAMESPACE_START

namespace net
{

class Channel;
class EventLoop;
class Socket;

///
/// TCP connection, for both client and server usage.
///
/// This is an interface class, so don't expose too much details.
class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection>
{
public:
    /// Constructs a TcpConnection with a connected sockfd
    ///
    /// User should not create this object.
    TcpConnection(EventLoop* loop,
                    const string& name,
                    int sockfd,
                    const InetAddress& localAddr,
                    const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const string& name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }
    bool connected() const { return state_ == kConnected; }
    bool disconnected() const { return state_ == kDisconnected; }
    // return true if success.
    bool getTcpInfo(struct tcp_info*) const;
    string getTcpInfoString() const;

    // void send(string&& message); // C++11
    void send(const void* message, int len);
    void send(const StringPiece& message);
    // void send(Buffer&& message); // C++11
    void send(Buffer* message);  // this one will swap data
    void shutdown(); // NOT thread safe, no simultaneous calling
    // void shutdownAndForceCloseAfter(double seconds); // NOT thread safe, no simultaneous calling
    void forceClose();
    void forceCloseWithDelay(double seconds);
    void setTcpNoDelay(bool on);
    // reading or not
    void startRead();
    void stopRead();
    bool isReading() const { return reading_; }; // NOT thread safe, may race with start/stopReadInLoop

    void setContext(const boost::any& context)
    { context_ = context; }

    const boost::any& getContext() const
    { return context_; }

    boost::any* getMutableContext()
    { return &context_; }

    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = cb; }

    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb; }

    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
    { highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }

    /// Advanced interface
    Buffer* inputBuffer()
    { return &inputBuffer_; }

    Buffer* outputBuffer()
    { return &outputBuffer_; }

    /// Internal use only.
    void setCloseCallback(const CloseCallback& cb)
    { closeCallback_ = cb; }

    // called when TcpServer accepts a new connection
    void connectEstablished();   // should be called only once
    // called when TcpServer has removed me from its map
    void connectDestroyed();  // should be called only once

private:
    enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();
    // void sendInLoop(string&& message);
    void sendInLoop(const StringPiece& message);
    void sendInLoop(const void* message, size_t len);
    void shutdownInLoop();
    // void shutdownAndForceCloseInLoop(double seconds);
    void forceCloseInLoop();
    void setState(StateE s) { state_ = s; }
    const char* stateToString() const;
    void startReadInLoop();
    void stopReadInLoop();
    /* 主要事件监听指针 */
    EventLoop* loop_;
    /* 连接名称 */
    const string name_;
    /* 状态 */
    StateE state_;  // FIXME: use atomic variable
    bool reading_;
    // we don't expose those classes to client.
    /* 用于tcp连接的套接字，以及用于监听套接字的Channel */
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    /* 本地<地址，端口>，客户端<地址，端口>，由TcpServer传入 */
    const InetAddress localAddr_;
    const InetAddress peerAddr_;
    /* 连接建立后/关闭后的回调函数，通常是由用户提供给TcpServer，然后TcpServer提供给TcpConnection */
    ConnectionCallback connectionCallback_;
    /* 当tcp连接有消息通信时执行的回调函数，也是由用户提供 */
    MessageCallback messageCallback_;
    /* 
   * 写入tcp缓冲区之后的回调函数
   * 通常是tcp缓冲区满然后添加到应用层缓冲区后，由应用层缓冲区写入内核tcp缓冲区
   * 后执行，一般用户不关系这部分
   */
    WriteCompleteCallback writeCompleteCallback_;
    /* 高水位回调，设定缓冲区接收大小，如果应用层缓冲区堆积的数据大于某个给定值时调用 */
    HighWaterMarkCallback highWaterMarkCallback_;
    /* 
   * tcp连接关闭时调用的回调函数，由TcpServer设置，用于TcpServer将这个要关闭的TcpConnection从
   * 保存着所有TcpConnection的map中删除
   * 这个回调函数和TcpConnection自己的handleClose不同，后者是提供给Channel的，函数中会使用到
   * closeCallback_
   */
    CloseCallback closeCallback_;
    /* 高水位值 */
    size_t highWaterMark_;
    Buffer inputBuffer_;
    Buffer outputBuffer_; // FIXME: use list<Buffer> as output buffer.
    boost::any context_;        /* 处理上下文消息的任意指针 */
    // FIXME: creationTime_, lastReceiveTime_
    //        bytesReceived_, bytesSent_
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

}  // namespace net

NAMESPACE_END
#endif