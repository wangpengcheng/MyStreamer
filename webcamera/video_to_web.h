/**
 * VideoToWeb类，主要是进行，camera到web的基本转换，含有http server
 * 主要的videolistener以及jpeg缓冲区
*/

#ifndef VIDEO_TO_WEB_H
#define VIDEO_TO_WEB_H
#include "uncopyable.h"
#include "video_source_listener_interface.h"

NAMESPACE_START

class VideoSourceToWebData;
class WebRequestHandler;
class VideoSourceToWeb : private Uncopyable
{
public:
    VideoSourceToWeb( uint16_t jpegQuality = 85 );
    ~VideoSourceToWeb( );
    
    // Get video source listener, which could be fed to some video source
    VideoSourceListenerInterface* VideoSourceListener( ) const;

    // Create web request handler to provide camera images as JPEGs
    std::shared_ptr<WebRequestHandler> CreateJpegHandler( const std::string& uri ) const;

    // Create web request handler to provide camera images as MJPEG stream
    std::shared_ptr<WebRequestHandler> CreateMjpegHandler( const std::string& uri, uint32_t frameRate ) const;

    // Get/Set JPEG quality (valid only if camera provides uncompressed images)
    uint16_t JpegQuality( ) const;
    void SetJpegQuality( uint16_t quality );

private:
    VideoSourceToWebData* mData;
};

NAMESPACE_END




#endif