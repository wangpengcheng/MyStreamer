#ifndef WEB_CAMERA_SERVER_H
#define WEB_CAMERA_SERVER_H
#include "uncopyable.h"
#include "video_source_to_web.h"
#include "video_listener.h"
#include "web_request_handler.h"
#include "net_http_server.h"
#include "net_event_loop.h"
#include "file_request_handler.h"
#include <memory>
NAMESPACE_START

class WebCameraServer:public Uncopyable
{

public:
    WebCameraServer(const std::string& path,
                    const uint16_t port,
                    const std::string& server_name,
                    const uint32_t thread_num_);
    ~WebCameraServer();
    typedef std::unordered_map<std::string,std::shared_ptr<WebRequestHandlerInterface> > RequestMap;
    void Start();
    /* 设置根目录 */
    inline std::string getRootPath(){return root_path_;}
    inline void setRootPath(const string & new_path){root_path_=new_path;}
    inline std::shared_ptr<net::HttpServer> getHttpServer(){return http_sever_;}
    void  AddHandler(const string & hander_name,const std::shared_ptr<WebRequestHandlerInterface> handler);
private:
    /* 执行函数 */
    void onRequest(const WebRequest& req, WebResponse* resp);
    /* data */
    
    /* 函数句柄映射 */
    RequestMap function_map_;
    /* 跟文件目录 */
    std::string root_path_;
     /* 文件查找函数;这里是自带的 */
    FileRequestHandler file_hander; 
    /* 循环事件 */
    net::EventLoop   loop_;
    /* http 服务器 */
    std::shared_ptr<net::HttpServer>  http_sever_;

};
/* 初始化file_type */
NAMESPACE_END
#endif