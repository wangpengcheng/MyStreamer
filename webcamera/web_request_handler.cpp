#include "web_request_handler.h"
#include <mutex>


NAMESPACE_START
WebRequestHandlerInterface::WebRequestHandlerInterface(
    const std::string& uri, 
    bool canHandleSubContent
    ):mUri(uri),mCanHandleSubContent(canHandleSubContent)
{
     // make sure all URIs start with /
    if ( mUri[0] != '/' )
    {
        mUri = '/' + mUri;
    }

    // make sure nothing finishes with /
    while ( ( mUri.length( ) > 1 ) && ( mUri.back( ) == '/' ) )
    {
        mUri.pop_back( );
    }
}

WebRequestHandlerInterface::~WebRequestHandlerInterface()
{

}
void JpegRequestHandler::HandleHttpRequest(const WebRequest& request, WebResponse& response)
{
    if(!Owner->IsError())
    {
        Owner->EncodeCameraImage();
    }
    if(Owner->IsError())
    {
        Owner->ReportError(response);
    }else{
        std::lock_guard<std::mutex> lock(Owner->BufferGuard);
        if(Owner==0)
        {
            response.SendFast(WebResponse::k500ServerError,"No image from video source");
        }else{
            response.setStatusCode(WebResponse::k200Ok);
            response.setStatusMessage("OK");
            response.setContentType("image/png");
            response.addHeader("Cache-Control","no-store, must-revalidate");
            response.addHeader("Pragma","no-cache");
            response.addHeader("Expires","0");
            response.setBody(std::string((char*)Owner->JpegBuffer,Owner->JpegBufferSize));
        }
    }

}


void MjpegRequestHandler::HandleHttpRequest( const WebRequest& request, WebResponse& response )
{
    //TODO:实现流的连续传输。tcp连续写入
}
void MjpegRequestHandler::HandleTimer( WebResponse& response )
{
    //TODO;设置TCP间隔的连续写入
}


NAMESPACE_END