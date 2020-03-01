#include <iostream>
#include "base_error.h"
#include "web_camera_server.h"
using namespace std;

int main(int argc,char* argv[])
{
    MyStreamer::WebCameraServer camera_server(string("./web"),8000,"mystreamer",1);
    camera_server.Start();
    return 0;
}