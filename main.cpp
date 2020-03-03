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
    /* 创建数据转换器 */
    MyStreamer::VideoSourceToWeb video_web;
    /* 创建摄像头 */
    auto my_camera=MyStreamer::V4L2Camera::Create();
<<<<<<< HEAD
    my_camera->SetVideoDeviceName("/dev/video1");
===
>>>>>>> acb5c2488120ad408518fd237bdd3c194c025503
    my_camera->SetFrameRate(20);
    my_camera->SetListener(video_web.VideoSourceListener());
    auto jpeg_handler=video_web.CreateJpegHandler("jpeg");
    MyStreamer::WebCameraServer camera_server(string("web"),8000,"mystreamer",1);
    my_camera->Start();
    camera_server.AddHandler("/camera/jpeg",jpeg_handler);
<<<<<<< HEAD
   // camera_server.AddHandler("/camera/info",std::make_shared<MyStreamer::CameraInfoHandler>(my_camera,"/camera/info"));
=======
>>>>>>> acb5c2488120ad408518fd237bdd3c194c025503
    camera_server.Start();
    return 0;
}
