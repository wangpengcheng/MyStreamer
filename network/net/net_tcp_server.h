
#ifndef NET_TCPSERVER_H
#define NET_TCPSERVER_H

#include "base_atomic.h"
#include "base_types.h"
#include "logging.h"
#include "net_tcp_connection.h"

#include <map>

NAMESPACE_START

namespace net
{

    class Acceptor;
    class EventLoop;
    class EventLoopThreadPool;

    ///
    /// TCP server, supports single-threaded and thread-pool models.
    ///
    /// This is an interface class, so don't expose too much details.
    class TcpServer : noncopyable
    {
    public:
        typedef std::function<void(EventLoop *)> ThreadInitCallback;
        // 是否执行端口复用
        enum Option
        {
            kNoReusePort,
            kReusePort,
        };

        //TcpServer(EventLoop* loop, const InetAddress& listenAddr);
        TcpServer(EventLoop *loop,
                  const InetAddress &listenAddr,
                  const string &nameArg,
                  Option option = kNoReusePort);
        ~TcpServer(); // force out-line dtor, for std::unique_ptr members.

        const string &ipPort() const { return ipPort_; }
        const string &name() const { return name_; }
        EventLoop *getLoop() const { return loop_; }

        /// Set the number of threads for handling input.
        ///
        /// Always accepts new connection in loop's thread.
        /// Must be called before @c start
        /// @param numThreads
        /// - 0 means all I/O in loop's thread, no thread will created.
        ///   this is the default value.
        /// - 1 means all I/O in another thread.
        /// - N means a thread pool with N threads, new connections
        ///   are assigned on a round-robin basis.
        void setThreadNum(int numThreads);
        /* 设置线程初始化回调函数 */
        void setThreadInitCallback(const ThreadInitCallback &cb)
        {
            threadInitCallback_ = cb;
        }
        /// valid after calling start()
        std::shared_ptr<EventLoopThreadPool> threadPool()
        {
            return threadPool_;
        }

        /// Starts the server if it's not listenning.
        ///
        /// It's harmless to call it multiple times.
        /// Thread safe.
        void start();

        /// Set connection callback.
        /// Not thread safe.
        //连接处理函数；非线程安全
        void setConnectionCallback(const ConnectionCallback &cb)
        {
            connectionCallback_ = cb;
        }

        /// Set message callback.
        /// Not thread safe.
        //消息回调函数，一般为可读的回调函数
        void setMessageCallback(const MessageCallback &cb)
        {
            messageCallback_ = cb;
        }

        /// Set write complete callback.
        /// Not thread safe.
        //写入回调函数
        void setWriteCompleteCallback(const WriteCompleteCallback &cb)
        {
            writeCompleteCallback_ = cb;
        }

    private:
        /// Not thread safe, but in loop
        void newConnection(int sockfd, const InetAddress &peerAddr);
        /// Thread safe.
        void removeConnection(const TcpConnectionPtr &conn);
        /// Not thread safe, but in loop
        void removeConnectionInLoop(const TcpConnectionPtr &conn);

        typedef std::map<string, TcpConnectionPtr> ConnectionMap;

        /* TcpServer所在的主线程下运行的事件驱动循环，负责监听Acceptor的Channel */
        EventLoop *loop_; // the acceptor loop
        /* 服务器负责监听的本地ip和端口 */
        const string ipPort_;
        /* 服务器名字，创建时传入 */
        const string name_;
        /* Acceptor对象，负责监听客户端连接请求，运行在主线程的EventLoop中,只有一个acceptor */
        std::unique_ptr<Acceptor> acceptor_; // avoid revealing Acceptor
        /* 事件驱动线程池，池中每个线程运行一个EventLoop */
        std::shared_ptr<EventLoopThreadPool> threadPool_;
        /* 用户传入，有tcp连接到达或tcp连接关闭时调用，传给TcpConnection */
        ConnectionCallback connectionCallback_;
        /* 用户传入，对端发来消息时调用，传给TcpConnection */
        MessageCallback messageCallback_;
        /* 成功写入内核tcp缓冲区后调用，传给TcpConnection */
        WriteCompleteCallback writeCompleteCallback_;
        /* 线程池初始化完成后调用，传给EventLoopThreadPool，再传给每个EventLoopThread */
        ThreadInitCallback threadInitCallback_;
        AtomicInt32 started_;
        // always in loop thread
        /* TcpConnection特有id，每增加一个TcpConnection，nextConnId_加一 */
        int nextConnId_;
        /* 所有的TcpConnection对象，智能指针 */
        ConnectionMap connections_;
    };

} // namespace net

NAMESPACE_END

#endif