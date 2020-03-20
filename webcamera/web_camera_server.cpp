#include "web_camera_server.h"
#include "net_inet_address.h"
#include <memory>
#include "base_tool.h"
using namespace  MY_NAME_SPACE;

WebCameraServer::WebCameraServer(
                    const std::string& path,
                    const uint16_t port,
                    const std::string& server_name,
                    const uint32_t thread_num_
                    ):file_hander(path)
{
    root_path_=path;
    /* 创建http sever */
    http_sever_=std::make_shared<net::HttpServer>(&loop_,net::InetAddress(port),server_name);
    http_sever_->setThreadNum(thread_num_);
    /* 绑定关键函数 */
    http_sever_->setHttpCallback(
        std::bind(&WebCameraServer::onRequest,this,_1,_2)
        );
    file_hander.setRootPath(root_path_);
}
WebCameraServer::~WebCameraServer()
{

}
void WebCameraServer::Start()
{
    http_sever_->start();
    loop_.loop();

}
void WebCameraServer::AddHandler(const string & hander_name,const std::shared_ptr<WebRequestHandlerInterface> handler)
{
    function_map_[hander_name]=handler;
}
/* 主要的请求处理函数 */
void WebCameraServer::onRequest(const WebRequest& req, WebResponse* resp)
{


 //   auto M=req.headers();
// std::map<std::string,std::string>::iterator iter;//定义一个迭代指针iter
//    for(iter=M.begin(); iter!=M.end(); iter++){
 //       std::cout<<iter->first <<"->"<<iter->second<<std::endl;   
//	}
    string req_path=req.path();
    std::string full_name=root_path_+req_path;
    /* 首先进行文件定位查找查找 */
    if(FileExiting(full_name)){
       file_hander.HandleHttpRequest(req,(*resp));
    }else{
        /* 查询其它服务 */
        auto search=function_map_.find(req_path);
        /* 执行函数 */
        if(search!=function_map_.end()){
            auto func=search->second;
            func->HandleHttpRequest(req,(*resp));
        }else{
            resp->SendFast(WebResponse::k404NotFound,"Not found request service ");
            resp->setCloseConnection(true);
        }
    }

}
