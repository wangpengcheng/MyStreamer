#include "web_camera_server.h"
#include "net_inet_address.h"
#include <memory>
#include "base_tool.h"
using namespace  MY_NAME_SPACE;

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

WebCameraServer::WebCameraServer(
                    const std::string& root="",
                    uint16_t port=8000,
                    const std::string& server_name="webcamera",
                    uint32_t thread_nums=1):root_path_(root)
{
    http_sever_=std::make_shared<net::HttpServer>(&loop_,net::InetAddress(port),server_name);
    http_sever_->setThreadNum(thread_nums);
    /* 绑定关键函数 */
    http_sever_->setHttpCallback(
        std::bind(&WebCameraServer::onRequest,this,_1,_2)
        );
}
void WebCameraServer::Start()
{
    http_sever_->start();
    loop_.loop();

}
void WebCameraServer::AddHandler(const string & hander_name,const WebRequestHandlerInterface& handler)
{
    /* 注意这里会覆盖同名函数 */
    function_map_[hander_name]=handler;
}

void WebCameraServer::onRequest(const WebRequest& req, WebResponse* resp)
{
    string req_path=req.path();
    std::string full_name=root_path_+req_path;
    /* 首先进行文件查找 */
    if(FileExiting(full_name)){
        auto func=function_map_["file"];
        func.HandleHttpRequest(req,(*resp));
    }else{
        /* 查询其它服务 */
        auto search=function_map_.find(req_path);
        /* 执行函数 */
        if(search!=function_map_.end()){
            auto func=search->second;
            func.HandleHttpRequest(req,(*resp));
        }else{
            resp->SendFast(WebResponse::k404NotFound,"Not found This file");
        }
    }
    
}