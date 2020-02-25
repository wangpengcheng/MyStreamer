
#include <iostream>
#include "v4l2_camrea_test.h"

#include "v4l2_camera.h"


using namespace MY_NAME_SPACE;

int main(int argc,char* argv[])
{
    std::cout<<"hello word"<<std::endl;
    /* 创建摄像头 */
    auto my_camera=V4L2Camera::Create();
    /* 创建监听者 */
    OpenCVListener testlistener;
    my_camera->SetVideoDeviceName("/dev/video1");
    my_camera->SetFrameRate(20);
    my_camera->SetListener(&testlistener);
    my_camera->Start();
    char temp;
    while (1)
    {
        temp=getchar();
        if(temp=='k'){
            my_camera->SignalToStop();
            my_camera->WaitForStop();
            break;
        }
        
    }
    

    return 0;
}