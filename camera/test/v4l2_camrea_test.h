/**
 * @file v4l2_camrea_test.h
 * @brief v4l2的基础类
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2020-12-18 22:55:43
 * @copyright Copyright (c) 2020  IRLSCU
 * 
 * @par 修改日志:
 * <table>
 * <tr>
 *    <th> Commit date</th>
 *    <th> Version </th> 
 *    <th> Author </th>  
 *    <th> Description </th>
 * <tr>
 *    <td> 2020-12-18 22:55:43 </td>
 *    <td> 1.0 </td>
 *    <td> wangpengcheng </td>
 *    <td> 添加文件内容 </td>
 * </tr>
 * </table>
 */
#ifndef V4L2_CAMERA_TEST_H
#define V4L2_CAMERA_TEST_H


/* 创建一个opencv的listener测试类 */

#include "video_source_listener_interface.h"
#include "image.h"
#include <iostream>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#define IMAGEWIDTH 3264
#define IMAGEHEIGHT 2448


NAMESPACE_START


/**
 * @brief opencv监听类，监听处理显示图像
 */
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
    /* 输出宽，高大小以及数据 指针地址 */
    //std::cout<<"width: "<<image->Width()<<"height："<<image->Height()<<"；size:"<<image->Size()<<"data_addr"<<&image<<"; data_addr:"<<(void*)(image->Data())<<";datasize:"<<sizeof(image->Data())<<std::endl;
    std::cout<<image->TimeStamp().tv_sec<<":"<<image->TimeStamp().tv_usec<<std::endl;
    auto temp_img=Image::Allocate(image->Width(),image->Height(),image->Format(),false);
    /* 将数据写入文件 */
    //WriteImageToFile(image);
    /* 在这里进行数据的输出 */
    /* 在这里进行一次数据复制 */
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