#include "video_source_to_webdata.h"
#include "web_request_handler.h"
#include "video_source_to_web.h"
#include <mutex>
#include <memory>
using namespace MY_NAME_SPACE;

VideoSourceToWeb::VideoSourceToWeb( uint16_t jpegQuality ) :
    mData( new VideoSourceToWebData( jpegQuality ) )
{
}

VideoSourceToWeb::~VideoSourceToWeb( )
{
    delete mData;
}

// Get video source listener, which could be fed to some video source
VideoSourceListenerInterface* VideoSourceToWeb::VideoSourceListener( ) const
{
    return &mData->VideoSourceListener;
}

/* 强制转换句柄指针 */
std::shared_ptr<WebRequestHandlerInterface> VideoSourceToWeb::CreateJpegHandler( const string& uri ) const
{
    return std::make_shared<JpegRequestHandler>( uri, mData );
}

// Create web request handler to provide camera images as MJPEG stream
std::shared_ptr<WebRequestHandlerInterface> VideoSourceToWeb::CreateMjpegHandler( const string& uri, uint32_t frameRate ) const
{
    return std::make_shared<MjpegRequestHandler>( uri, frameRate, mData );
}

// Get/Set JPEG quality (valid only if camera provides uncompressed images)
uint16_t VideoSourceToWeb::JpegQuality( ) const
{
    return mData->jpeg_encoder.Quality( );
}
void VideoSourceToWeb::SetJpegQuality( uint16_t quality )
{
    mData->jpeg_encoder.SetQuality( quality );
}