#ifndef V4L2_CAMERA_TEST_H
#define V4L2_CAMERA_TEST_H


/* 创建一个opencv的listener测试类 */

#include "video_source_listener_interface.h"
#include "image.h"
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#define IMAGEWIDTH 3264
#define IMAGEHEIGHT 2448


NAMESPACE_START



class OpenCVListener: public VideoSourceListenerInterface{
public :
    OpenCVListener();
    ~ OpenCVListener();
    void OnNewImage( const std::shared_ptr<const Image>& image );
    // 视频错误信号
    void OnError( const std::string& errorMessage, bool fatal );
    IplImage* img;
    CvMat cvmat;
};

OpenCVListener::OpenCVListener()
{
    cvNamedWindow("Capture",CV_WINDOW_AUTOSIZE);
}
OpenCVListener::~OpenCVListener()
{

}

void OpenCVListener::OnNewImage( const std::shared_ptr<const Image>& image ){
    std::cout<<"width: "<<image->Width()<<"height"<<image->Height()<<std::endl;
    std::cout<<image->TimeStamp().tv_sec<<":"<<image->TimeStamp().tv_usec<<std::endl;
    cvmat=cvMat(image->Width(),image->Height(),CV_8UC3,(void*)image->Data());
    img = cvDecodeImage(&cvmat,1);
    std::cout<<img->width<<","<<img->height<<std::endl;
    if(!img){
		printf("DecodeImage error!\n");
    }
	cvShowImage("Capture",img);
    cvReleaseImage(&img);
    if((cvWaitKey(1)&255) == 27){
			exit(0);
    }
}
void OpenCVListener::OnError( const std::string& errorMessage, bool fatal ){
    std::cout<<errorMessage<<std::endl;
}
NAMESPACE_END
#endif