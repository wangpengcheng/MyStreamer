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

void defaultHttpCallback(const HttpRequest&, HttpResponse* resp)
{
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
}

}  // namespace detail
}  // namespace net

NAMESPACE_END

HttpServer::HttpServer(EventLoop* loop,
                       const InetAddress& listenAddr,
                       const string& name,
                       TcpServer::Option option)
  : server_(loop, listenAddr, name, option),
    httpCallback_(detail::defaultHttpCallback)
{
    /* 设置连接的函数 */
  server_.setConnectionCallback(
      std::bind(&HttpServer::onConnection, this, _1));
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
void HttpServer::onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        conn->setContext(HttpContext());
    }
}
/* 设置连接响应函数 */
void HttpServer::onMessage(const TcpConnectionPtr& conn,
                           Buffer* buf,
                           Timestamp receiveTime)
{
    HttpContext* context = boost::any_cast<HttpContext>(conn->getMutableContext());
    /* 解析连接 */
    if (!context->parseRequest(buf, receiveTime))
    {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }
    /* 解析所有参数 */
    if (context->gotAll())
    {
        onRequest(conn, context->request());
        context->reset();
    }
}
/* 执行请求的相关函数 */
void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req)
{
    const string& connection = req.getHeader("Connection");
    bool close = connection == "close" ||
        (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
    /* 创建响应结构体 */
    HttpResponse response(close);
    /* 执行相关函数 */
    httpCallback_(req, &response);
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