#include "web_request_handler.h"
#include "logging.h"
#include "time_stamp.h"
#include <functional>
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
//
void JpegRequestHandler::HandleHttpRequest(const TcpConnectionPtr &conn,const WebRequest& request, WebResponse& response)
{
    if(!Owner->IsError())
    {
        Owner->EncodeCameraImage();
    }
    if(Owner->IsError())
    {
        Owner->ReportError(response);
    }else{
        /* 这里对图像数据加锁，避免竞争访问 */
        std::lock_guard<std::mutex> lock(Owner->BufferGuard);
        if(Owner==nullptr)
        {
            response.SendFast(WebResponse::k500ServerError,"No image from video source");
        }else{
            response.setStatusCode(WebResponse::k200Ok);
            response.setStatusMessage("OK");
            response.setContentType("image/png");
            /* 注意这里取消缓存 */
            response.addHeader("Cache-Control","no-store, must-revalidate");
            response.addHeader("Pragma","no-cache");
            response.addHeader("Expires","0");
            response.setBody(std::string((char*)Owner->JpegBuffer,Owner->JpegSize));
        }
    }

}


void MjpegRequestHandler::HandleHttpRequest(const TcpConnectionPtr &conn, const WebRequest& request, WebResponse& response )
{
    // 丛集处理时间
    uint32_t handlingTime = 0;
    if(!Owner->IsError()) {
        Timestamp startTime = Timestamp::now();
        Owner->EncodeCameraImage();
        Timestamp endTime  = Timestamp::now();
        handlingTime = static_cast<uint32_t>(endTime.microSecondsSinceEpoch()-startTime.microSecondsSinceEpoch());
    }
    if(Owner==nullptr||Owner->IsError()) {
        Owner->ReportError(response);
    }else {
        std::lock_guard<std::mutex> lock(Owner->BufferGuard);
        response.setStatusCode(WebResponse::k200Ok);
        response.setStatusMessage("OK");
        response.setContentType("image/png");
        /* 注意这里取消缓存 */
        response.addHeader("Cache-Control","no-store, must-revalidate");
        response.addHeader("Pragma","no-cache");
        response.addHeader("Expires","0");
        response.addHeader("Content-Type","multipart/x-mixed-replace; boundary=--myboundary");
        string extenHeader = "--myboundary\r\nContent-Type: image/jpeg\r\nContent-Length:"+std::to_string(Owner->JpegSize)+"\r\n";
        response.setExternalHeader(extenHeader);
        response.setBody(std::string((char*)Owner->JpegBuffer,Owner->JpegSize));
        // 设置
        conn->setTimerCallback(std::bind(&MjpegRequestHandler::HandleTimer,this,_1));
        Timestamp nextTime = Timestamp(Timestamp::now().microSecondsSinceEpoch()+FrameInterval);
        conn->setTimer(nextTime);
    }
}
void MjpegRequestHandler::HandleTimer(const TcpConnectionPtr &conn)
{
    uint32_t handlingTime = 0;
    // 创建数据
    WebResponse response(false);
    if(!Owner->IsError()) {
        Timestamp startTime = Timestamp::now();
        Owner->EncodeCameraImage();
        Timestamp endTime  = Timestamp::now();
        handlingTime = static_cast<uint32_t>(endTime.microSecondsSinceEpoch()-startTime.microSecondsSinceEpoch());
    }

    if ((Owner==nullptr)|| ( Owner->IsError( ) ) || ( Owner->JpegSize == 0 ) )
    {
        conn->shutdown();
        LOG_INFO<<conn->name()<<"is closed";
    }
    else
    {
        Timestamp startTime = Timestamp::now();
        std::lock_guard<std::mutex> lock(Owner->BufferGuard);
    
        // don't try sending too much on slow connections - it will only create video lag
        if ( conn->outputBuffer()->readableBytes() < 2 * Owner->JpegSize )
        {
            // provide subsequent images of the MJPEG stream
            string extenHeader = "--myboundary\r\nContent-Type: image/jpeg\r\nContent-Length:"+std::to_string(Owner->JpegSize)+"\r\n";
            response.setExternalHeader(extenHeader);
            response.setBody(std::string((char*)Owner->JpegBuffer,Owner->JpegSize));
        }
        Timestamp endTime = Timestamp::now();
        // get final request handling time
        handlingTime = static_cast<uint32_t>(endTime.microSecondsSinceEpoch()-startTime.microSecondsSinceEpoch());
        uint32_t nextTimespace = (( handlingTime >= FrameInterval ) ? 1 : FrameInterval - handlingTime );
        // set new timer for further images
        if(conn->connected()) {
            Timestamp next_time = addTime(Timestamp::now(),nextTimespace);
            conn->setTimer(next_time);
        }else {
            LOG_INFO<<conn->name()<<"is closed,No Next Frame";
        }
    }
    
}


NAMESPACE_END