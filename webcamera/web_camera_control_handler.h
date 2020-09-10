/**
 * 摄像头属性查询和参数控制api;
 * 注意一定要摄像头支持的属性
 * TODO 摄像头控制的可视化
 */
/* 摄像头属性查询控制类 */

#ifndef WEB_CMAERA_CONTROL_HANDLER_H
#define WEB_CMAERA_CONTROL_HANDLER_H
#include "web_request_handler.h"
#include "v4l2_camera_config.h"

NAMESPACE_START
/* 摄像头信息查询类 */
class CameraInfoHandler:public WebRequestHandlerInterface
{
public :
    CameraInfoHandler(const std::shared_ptr<V4L2Camera>& camera,const std::string& url);
    /* 响应函数 */
    void HandleHttpRequest( const WebRequest& request, WebResponse&  response  );
private:
    /* data */
    std::shared_ptr<V4L2CameraConfig> camera_data_;
};



class CameraControlHandler:public WebRequestHandlerInterface
{
public :
    CameraControlHandler(const std::shared_ptr<V4L2Camera>& camera,const std::string& url);
    /* 响应函数 */
    void HandleHttpRequest( const WebRequest& request, WebResponse&  response  );
private:
    /* data */
    std::shared_ptr<V4L2CameraConfig> camera_data_;
};

NAMESPACE_END
#endif