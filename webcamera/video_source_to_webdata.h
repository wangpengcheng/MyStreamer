/**
 * 将摄像头camera转换为web的关键函数;主要用于存储数据
 * 
*/

#ifndef VIDEO_SOURCE_TO_WEBDATA_H
#define VIDEO_SOURCE_TO_WEBDATA_H

#include "video_listener.h"
#include "jpeg_encoder.h"
#include "net_http_response.h"

#include <mutex>
NAMESPACE_START

/* 定义结构体的数据类 */
#define JPEG_BUFFER_SIZE (1024 * 1024)
struct VideoSourceToWebData
{
public:
    VideoSourceToWebData( uint16_t jpegQuality ) :
        NewImageAvailable( false ), 
        VideoSourceError( false ), 
        InternalError( Error::Success ),
        JpegBuffer( nullptr ), 
        JpegBufferSize( 0 ), 
        JpegSize( 0 ), 
        VideoSourceListener( this ),
        CameraImage( ), 
        VideoSourceErrorMessage( ), 
        ImageGuard( ), 
        BufferGuard( ),
        jpeg_encoder(jpegQuality,true)
    {
        /* 为jpeg分配buffer */
        JpegBuffer = (uint8_t*) malloc( JPEG_BUFFER_SIZE );
        if ( JpegBuffer != nullptr )
        {
            JpegBufferSize = JPEG_BUFFER_SIZE;
        }
    }

    ~VideoSourceToWebData( )
    {
        if ( JpegBuffer != nullptr )
        {
            free( JpegBuffer );
            JpegBuffer=nullptr;
        }
    }

    bool IsError( );
    void ReportError(net::HttpResponse& response);
    void EncodeCameraImage( );
public:
    volatile bool      NewImageAvailable;                   /* 是否存在新图片 */
    volatile bool      VideoSourceError;                    /* 视频源错误 */
    Error               InternalError;                      
    /* jpegbuffer相关类 */
    uint8_t*           JpegBuffer;                          /* jpegbuffer缓冲 */
    uint32_t           JpegBufferSize;
    /* 图片类 */
    uint32_t           JpegSize;
    VideoListener      VideoSourceListener;                 /* 视频监听者 */
    /* 图片那指向source的img */
    std::shared_ptr<Image> CameraImage;                 
    std::string             VideoSourceErrorMessage;
    std::mutex              ImageGuard;                     /* 图片锁 */
    std::mutex              BufferGuard;                    /* buffer锁 */
    JpegEncoder             jpeg_encoder;                    /* jpeg编码器 */
};

NAMESPACE_END

#endif