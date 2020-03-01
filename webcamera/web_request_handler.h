
#ifndef WEB_REQUEST_HANDLER_H
#define WEB_REQUEST_HANDLER_H

#include "net_http_response.h"
#include "net_http_request.h"
#include "video_source_to_webdata.h"

NAMESPACE_START

/**
 * 强求响应控制句柄函数，主要是方便添加reposens函数;对于不同的reques和Response进行处理
*/

class  WebRequestHandlerInterface
{
public:
    WebRequestHandlerInterface(const std::string& uri, bool canHandleSubContent);
    virtual ~WebRequestHandlerInterface();
    // 处理的关键函数
    virtual void HandleHttpRequest( const WebRequest& request, WebResponse&  response  ) = 0;
    inline std::string Uri( ) const { return mUri;  }
    inline bool CanHandleSubContent( ) const { return mCanHandleSubContent; }
private:
    std::string     mUri;                   /* 对应处理的url参数 */
    bool            mCanHandleSubContent;
};

// Web request handler providing camera images as JPEGs
class JpegRequestHandler : public WebRequestHandlerInterface
{
private:
    VideoSourceToWebData* Owner;

public:
    JpegRequestHandler( 
        const string& uri, 
        VideoSourceToWebData* owner ) :
        WebRequestHandlerInterface( uri, false ), 
        Owner( owner )
    {
    }

    void HandleHttpRequest( const WebRequest& request,WebResponse&  response  );
};

// Web request handler providing camera images as MJPEG stream
class MjpegRequestHandler : public WebRequestHandlerInterface
{
private:
    VideoSourceToWebData* Owner;
    uint32_t               FrameInterval;

public:
    MjpegRequestHandler( 
        const string& uri, 
        uint32_t frameRate, 
        VideoSourceToWebData* owner ) :
        WebRequestHandlerInterface( uri, false ), 
        Owner( owner ), 
        FrameInterval( 1000 / frameRate )
    {
    }

    void HandleHttpRequest( const WebRequest& request,WebResponse&  response  );
    void HandleTimer(WebResponse&  response  );
};


NAMESPACE_END
#endif