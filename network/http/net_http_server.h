#ifndef NET_HTTP_HTTPSERVER_H
#define NET_HTTP_HTTPSERVER_H

#include "net_tcp_server.h"

NAMESPACE_START

namespace net
{

    class HttpRequest;
    class HttpResponse;

    /// A simple embeddable HTTP server designed for report status of a program.
    /// It is not a fully HTTP 1.1 compliant server, but provides minimum features
    /// that can communicate with HttpClient and Web browser.
    /// It is synchronous, just like Java Servlet.
    class HttpServer : noncopyable
    {
    public:
        typedef std::function<void(const TcpConnectionPtr &conn,
                                   const HttpRequest &,
                                   HttpResponse *)>
            HttpCallback;

        HttpServer(EventLoop *loop,
                   const InetAddress &listenAddr,
                   const string &name,
                   TcpServer::Option option = TcpServer::kNoReusePort);

        EventLoop *getLoop() const { return server_.getLoop(); }

        /// Not thread safe, callback be registered before calling start().
        void setHttpCallback(const HttpCallback &cb)
        {
            httpCallback_ = cb;
        }

        void setThreadNum(int numThreads)
        {
            server_.setThreadNum(numThreads);
        }

        void start();
        void stop();

    private:
        /* 连接处理函数 */
        void onConnection(const TcpConnectionPtr &conn);
        /* 消息处理函数 */
        void onMessage(const TcpConnectionPtr &conn,
                       Buffer *buf,
                       Timestamp receiveTime);

        /* 注意谨慎使用 */
        void onRequest(const TcpConnectionPtr &, const HttpRequest &);

        TcpServer server_;          /* tcp server */
        HttpCallback httpCallback_; /* 响应回调函数 */
                                    /* server主动回调函数 */
        
    };

} // namespace net

NAMESPACE_END

#endif