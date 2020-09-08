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
/**
 * 摄像头服务关键封装类
 * 
 */
class WebCameraServer:public Uncopyable
{

public:
    WebCameraServer(const std::string& path,/* 根路径 */
                    const uint16_t port, /* 服务器端口 */
                    const std::string& server_name,/* 服务器名称 */
                    const uint32_t thread_num_ /* 设置线程池数量 */
                    );
    ~WebCameraServer();
    typedef std::unordered_map<std::string,std::shared_ptr<WebRequestHandlerInterface> > RequestMap;
    void Start();
    /* 设置根目录 */
    inline std::string getRootPath(){return root_path_;}
    inline void setRootPath(const string & new_path){root_path_=new_path;}
    /* 添加HTTP服务器 */
    inline std::shared_ptr<net::HttpServer> getHttpServer(){return http_sever_;}
    /* 添加处理句柄 */
    void  AddHandler(const string & hander_name,const std::shared_ptr<WebRequestHandlerInterface> handler);
private:
    /* 执行函数 */
    void onRequest(const WebRequest& req, WebResponse* resp);
    /* data */
    
    /* 函数句柄映射 */
    RequestMap function_map_;
    /* 根目录地址 */
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