#ifndef WEB_CAMERA_SERVER_H
#define WEB_CAMERA_SERVER_H
#include "uncopyable.h"
#include "video_source_to_web.h"
#include "video_listener.h"
#include "web_request_handler.h"
#include "net_http_server.h"
#include "net_event_loop.h"
#include <memory>
NAMESPACE_START

class WebCameraServer:public Uncopyable
{

public:
    explicit WebCameraServer();
    WebCameraServer(const std::string& root,
                    uint16_t port,
                    const std::string& server_name,
                    uint32_t thread_nums);
    ~WebCameraServer();
    static std::unordered_map<std::string,std::string> file_type;
    typedef std::unordered_map<std::string,WebRequestHandlerInterface > RequestMap;
    void Start();
    /* 设置根目录 */
    inline std::string getRootPath(){return root_path_;}
    inline void setRootPath(const string & new_path){root_path_=new_path;}
    inline std::shared_ptr<net::HttpServer> getHttpServer(){return http_sever_;}
    void  AddHandler(const string & hander_name,const WebRequestHandlerInterface& handler);
private:
    /* 执行函数 */
    void onRequest(const WebRequest& req, WebResponse* resp);
    /* data */
    
    /* 函数句柄映射 */
    RequestMap function_map_;
    /* 跟文件目录 */
    std::string root_path_;
    /* 循环事件 */
    net::EventLoop   loop_;
    /* http 服务器 */
    std::shared_ptr<net::HttpServer>  http_sever_;
};
/* 初始化file_type */
std::unordered_map<std::string,std::string> WebCameraServer::file_type={
    {".html", "text/html"},
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
NAMESPACE_END
#endif