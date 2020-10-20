#include <iostream>
#include "base_error.h"
#include "web_camera_server.h"
#include "video_source_to_web.h"
#include "v4l2_camera.h"
#include "web_camera_control_handler.h"
#include <memory>
using namespace std;

int main(int argc,char* argv[])
{
    /* 创建视频数据转换器 */
    MyStreamer::VideoSourceToWeb video_web;
    /* 创建摄像头 */
    auto my_camera=MyStreamer::V4L2Camera::Create();
    my_camera->SetVideoDeviceName("/dev/video0");
    //是否开启jpeg编码，开启的化，只能接收jpeg的摄像头视频源
    my_camera->EnableJpegEncoding(false);
    // 设置帧率
    my_camera->SetFrameRate(20);
    my_camera->SetVideoSize(320,240);
    // 设置监听者
    my_camera->SetListener(video_web.VideoSourceListener());
    // 创建图片handler 
    auto jpeg_handler=video_web.CreateJpegHandler("jpeg");
    // 创建
    MyStreamer::WebCameraServer camera_server(string("web"),8000,"mystreamer",8);
    my_camera->Start();
    /* 添加图像服务 */
    camera_server.AddHandler("/camera/jpeg",jpeg_handler);
    //camera_server.AddHandler("/camera/info",std::make_shared<MyStreamer::CameraInfoHandler>(my_camera,"/camera/info"));
    camera_server.Start();
    return 0;
}
