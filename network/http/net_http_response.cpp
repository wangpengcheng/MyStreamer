#include "net_buffer.h"
#include "net_http_response.h"
#include "net_event_loop.h"

#include <stdio.h>

using namespace MY_NAME_SPACE;
using namespace MY_NAME_SPACE::net;

/* map初始化 */
HttpResponse::HttpStateMap HttpResponse::state_map = {
    {HttpResponse::HttpStatusCode::k200Ok, "OK"},
    {HttpResponse::HttpStatusCode::k400BadRequest, "400 Bad Request"},
    {HttpResponse::HttpStatusCode::k404NotFound, "404 Not Found"},
    {HttpResponse::HttpStatusCode::k405MethodNotAllowed, "405 Method Not Allowed"},
    {HttpResponse::HttpStatusCode::k301MovedPermanently, "301 Moved Permanently"},
    {HttpResponse::HttpStatusCode::k500ServerError, "500 Server Error"},
    {HttpResponse::HttpStatusCode::kUnknown, "Unkown error"},

};
/* 请求文件类型初始化 */
std::unordered_map<std::string, std::string> HttpResponse::file_type = {
    {".html", "text/html"},
    {".css", "text/css"},
    {".js", "application/x-javascript"},
    {".avi", "video/x-msvideo"},
    {".bmp", "image/bmp"},
    {".c", "text/plain"},
    {".doc", "application/msword"},
    {".gif", "image/gif"},
    {".gz", "application/x-gzip"},
    {".htm", "text/html"},
    {".ico", "image/x-icon"},
    {".jpg", "image/jpeg"},
    {".png", "image/png"},
    {".txt", "text/plain"},
    {".mp3", "audio/mp3"},
    {"default", "text/html"}
};
/* 将头部信息写入到buffer中，在server中调用，使用send进行发送 */
void HttpResponse::appendToBuffer(Buffer *output)
{
    char buf[32];
    /* 添加头部信息,默认使用http1.1 */
    snprintf(buf, sizeof buf, "HTTP/1.1 %d ", statusCode_);
    output->append(buf);
    /* 添加状态码 */
    output->append(statusMessage_);
    output->append("\r\n");

    if (closeConnection_)
    {
        addHeader("Connection","close");
    }
    else
    {
        /* 添加连接状态 */
        addHeader("Connection","Keep-Alive");
    }
    /* 逐步添加header */
    for (const auto &header : headers_)
    {
        output->append(header.first);
        output->append(": ");
        output->append(header.second);
        output->append("\r\n");
    }
    //output->append("\r\n");
    output->append(externalHeader_);
    /* 添加相关信息 */
    output->append("\r\n");
    /* 添加主体 */
    output->append(body_);
}
/* 设置快速发送 */
void HttpResponse::SendFast(HttpStatusCode send_code, const string &body)
{
    setStatusCode(send_code);
    setStatusMessage(state_map[send_code]);
    setBody(body);
    this->addHeader("Content-Length",std::to_string(body.size()));
}
