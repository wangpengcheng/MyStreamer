#include "net_buffer.h"
#include "net_http_response.h"
#include "net_event_loop.h"

#include <stdio.h>

using namespace MY_NAME_SPACE;
using namespace MY_NAME_SPACE::net;

/* map出初始化 */
HttpResponse::HttpStateMap HttpResponse::state_map={
    {HttpResponse::HttpStatusCode::k200Ok,"OK"},
    {HttpResponse::HttpStatusCode::k400BadRequest,"400 Bad Request"},
    {HttpResponse::HttpStatusCode::k404NotFound,"404 Not Found"},
    {HttpResponse::HttpStatusCode::k405MethodNotAllowed,"Method Not Allowed"},
    {HttpResponse::HttpStatusCode::k301MovedPermanently,"301 Moved Permanently"},
    {HttpResponse::HttpStatusCode::k500ServerError,"500 Server Error"},
    {HttpResponse::HttpStatusCode::kUnknown,"Unkown error"},

};
std::unordered_map<std::string,std::string > HttpResponse::file_type={
    {".html", "text/html"},
    {".css","text/css"},
    {".js","application/x-javascript"},
    
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
void HttpResponse::appendToBuffer(Buffer* output) const
{
    char buf[32];
    /* 添加头部信息 */
    snprintf(buf, sizeof buf, "HTTP/1.1 %d ", statusCode_);
    output->append(buf);
    /* 添加状态码 */
    output->append(statusMessage_);
    output->append("\r\n");

    if (closeConnection_)
    {
        output->append("Connection: close\r\n");
    }
    else
    {
        /* 输入主体长度 */
        snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", body_.size());
        /* 添加主体 */
        output->append(buf);
        /* 添加连接状态 */
        output->append("Connection: Keep-Alive\r\n");
    }

    for (const auto& header : headers_)
    {
        output->append(header.first);
        output->append(": ");
        output->append(header.second);
        output->append("\r\n");
    }
    /* 添加相关信息 */
    output->append("\r\n");
    /* 添加主体 */
    output->append(body_);
}

void HttpResponse::SendFast(HttpStatusCode send_code,const string& body)
{
    setStatusCode(send_code);
    setStatusMessage(state_map[send_code]);
    setBody(body);
}
