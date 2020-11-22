#include "video_source_to_webdata.h"
#include <mutex>
#include <thread>

using namespace MY_NAME_SPACE;
using namespace MY_NAME_SPACE::net;

// Check if any errors happened
bool VideoSourceToWebData::IsError( )
{
    return ( ( InternalError != Error::Success ) || ( VideoSourceError ) );
}

// 报告HTTP错误
void VideoSourceToWebData::ReportError( net::HttpResponse& response )
{
    if ( InternalError != Error::Success )
    {
        response.SendFast(HttpResponse::k500ServerError,InternalError.ToString( ).c_str( ));
    }
    else if ( VideoSourceError )
    {
        std::lock_guard<std::mutex> imageLock( ImageGuard );

        response.SendFast(HttpResponse::k500ServerError, VideoSourceErrorMessage.c_str( ) );
    }
}

// Encode current camera image as JPEG
void VideoSourceToWebData::EncodeCameraImage( )
{
    if ( NewImageAvailable )
    {
        // 图片加锁
        std::lock_guard<std::mutex> imageLock( ImageGuard );
        // 缓冲区加锁
        std::lock_guard<std::mutex> bufferLock( BufferGuard );

        if ( JpegBuffer == nullptr )
        {
            InternalError = Error::OutOfMemory;
        }
        else
        {
            //jpeg格式直接复制
            if ( CameraImage->Format( ) == PixelFormat::JPEG )
            {
                // check allocated buffer size
                if ( JpegBufferSize < static_cast<uint32_t>( CameraImage->Width( ) ) )
                {
                    // make new size 10% bigger than needed
                    uint32_t newSize = CameraImage->Width( ) + CameraImage->Width( ) / 10;

                    JpegBuffer = (uint8_t*) realloc( JpegBuffer, newSize );
                    if ( JpegBuffer != nullptr )
                    {
                        JpegBufferSize = newSize;
                    }
                    else
                    {
                        InternalError = Error::OutOfMemory;
                    }
                }

                if ( JpegBuffer != nullptr )
                {
                    // just copy JPEG data if we got already encoded image
                    memcpy( JpegBuffer, CameraImage->Data( ), CameraImage->Width( ) );
                    JpegSize = CameraImage->Width( );
                }
            }
            else
            {
                // 获取旧数据指针，因为libjpeg会主动分配内存，需要更新buffer
                uint8_t* oldJpegBuffer = JpegBuffer;
                // encode image as JPEG (buffer is re-allocated if too small by encoder)
                JpegSize      = JpegBufferSize;
                // 将图片压缩之后拷贝到JpegBuffer中
                InternalError = jpeg_encoder.EncodeToMemory( CameraImage, &JpegBuffer, &JpegSize );
                // 检查是否需要扩展内存
                if ( JpegSize > JpegBufferSize )
                {
                    JpegBufferSize = JpegSize;
                    free( oldJpegBuffer );
                }
            }
        }

        NewImageAvailable = false;
    }
}