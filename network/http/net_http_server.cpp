#include "net_http_server.h"
#include "net_http_response.h"
#include "net_http_context.h"
#include "net_http_request.h"
#include "logging.h"

using namespace MY_NAME_SPACE;
using namespace MY_NAME_SPACE::net;

NAMESPACE_START

namespace net
{
    namespace detail
    {
        /* 设置默认的回调函数 */
        void defaultHttpCallback(const HttpRequest &, HttpResponse *resp)
        {
            resp->setStatusCode(HttpResponse::k404NotFound);
            resp->setStatusMessage("Not Found");
            resp->setCloseConnection(true);
        }

    } // namespace detail
} // namespace net

NAMESPACE_END

HttpServer::HttpServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &name,
                       TcpServer::Option option)
    : server_(loop, listenAddr, name, option),
      httpCallback_(detail::defaultHttpCallback)
{
    /* 设置连接回调函数 */
    server_.setConnectionCallback(
        std::bind(&HttpServer::onConnection, this, _1));
    /* 设置消息回调函数 */
    server_.setMessageCallback(
        std::bind(&HttpServer::onMessage, this, _1, _2, _3));
}

void HttpServer::start()
{
    LOG_WARN << "HttpServer[" << server_.name()
             << "] starts listenning on " << server_.ipPort();
    server_.start();
}
void HttpServer::stop()
{
    LOG_WARN << "HttpServer[" << server_.name()
             << "] Stop Not complate" << server_.ipPort();
    //TODO 完成安全的销毁
}
/* 设置连接结构体 */
void HttpServer::onConnection(const TcpConnectionPtr &conn)
{
    /* 设置上下文 */
    if (conn->connected())
    {
        conn->setContext(HttpContext());
    }
}
/* 设置连接响应函数 */
void HttpServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buf,
                           Timestamp receiveTime)
{
    /* 获取上下文 */
    HttpContext *context = boost::any_cast<HttpContext>(conn->getMutableContext());
    /* 使用context解析连接 */
    if (!context->parseRequest(buf, receiveTime))
    {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }
    /* 是否解析所有参数 */
    if (context->gotAll())
    {
        /* 调用请求处理函数 */
        onRequest(conn, context->request());
        context->reset();
    }
}
/* 请求回调函数 */
void HttpServer::onRequest(const TcpConnectionPtr &conn, const HttpRequest &req)
{
    const string &connection = req.getHeader("Connection");
    bool close = connection == "close" ||
                 (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
    /* 创建响应结构体 */
    HttpResponse response(close);
    /* 执行绑定的请求处理函数相关函数 */
    httpCallback_(req, &response);
    /* 
        TODO 根据参数的不同，
        绑定不同的Thread
    */
    Buffer buf;
    /* 将结构体，添加到buffer中 */
    response.appendToBuffer(&buf);

    /* 发送buffer */
    conn->send(&buf);
    /* 检查是否需要关闭 */
    if (response.closeConnection())
    {
        conn->shutdown();
    }
}
void HttpServer::onRespone(const TcpConnectionPtr &conn)
{
    
}