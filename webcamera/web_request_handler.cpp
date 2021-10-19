#include "web_request_handler.h"
#include "logging.h"
#include "time_stamp.h"
#include "net_buffer.h"
#include <functional>
#include <mutex>

NAMESPACE_START

WebRequestHandlerInterface::WebRequestHandlerInterface(
    const std::string &uri,
    bool canHandleSubContent) : mUri(uri), mCanHandleSubContent(canHandleSubContent)
{
    // make sure all URIs start with /
    if (mUri[0] != '/')
    {
        mUri = '/' + mUri;
    }

    // make sure nothing finishes with /
    while ((mUri.length() > 1) && (mUri.back() == '/'))
    {
        mUri.pop_back();
    }
}

WebRequestHandlerInterface::~WebRequestHandlerInterface()
{
}
//
void JpegRequestHandler::HandleHttpRequest(const net::TcpConnectionPtr &conn, const WebRequest &request, WebResponse &response)
{
    if (!Owner->IsError())
    {
        Owner->EncodeCameraImage();
    }
    if (Owner->IsError())
    {
        Owner->ReportError(response);
    }
    else
    {
        /* 这里对图像数据加锁，避免竞争访问 */
        std::lock_guard<std::mutex> lock(Owner->BufferGuard);
        if (Owner == nullptr)
        {
            response.SendFast(WebResponse::k500ServerError, "No image from video source");
        }
        else
        {
            response.setStatusCode(WebResponse::k200Ok);
            response.setStatusMessage("OK");
            response.setContentType("image/png");
            /* 注意这里取消缓存 */
            response.addHeader("Cache-Control", "no-store, must-revalidate");
            response.addHeader("Pragma", "no-cache");
            response.addHeader("Expires", "0");
            response.setBody(std::string((char *)Owner->JpegBuffer, Owner->JpegSize));
            /* 输入主体长度 */
            response.addHeader("Content-Length", std::to_string(Owner->JpegSize));
        }
    }
}

void MjpegRequestHandler::HandleHttpRequest(const net::TcpConnectionPtr &conn, const WebRequest &request, WebResponse &response)
{
    // 统计处理时间
    uint32_t handlingTime = 0;
    if (!Owner->IsError())
    {
        Timestamp startTime = Timestamp::now();
        Owner->EncodeCameraImage();
        Timestamp endTime = Timestamp::now();
        handlingTime = static_cast<uint32_t>(endTime.microSecondsSinceEpoch() - startTime.microSecondsSinceEpoch());
    }
    if (Owner == nullptr || Owner->IsError())
    {
        Owner->ReportError(response);
    }
    else
    {
        std::lock_guard<std::mutex> lock(Owner->BufferGuard);
        response.setStatusCode(WebResponse::k200Ok);
        response.setStatusMessage("OK");
        /* 注意这里取消缓存 */
        response.addHeader("Cache-Control", "no-store, must-revalidate");
        response.addHeader("Pragma", "no-cache");
        response.addHeader("Expires", "0");
        response.addHeader("Content-Type", "multipart/x-mixed-replace; boundary=--myboundary");

        string extenHeader = "\r\n--myboundary\r\nContent-Type: image/jpeg\r\nContent-Length: " + std::to_string(Owner->JpegSize) + "\r\n\r\n";
        extenHeader += std::string((char *)Owner->JpegBuffer, Owner->JpegSize);
        response.setBody(extenHeader);
        //response.setExternalHeader(extenHeader);
        // 设置connect 主动时间回调用
        conn->setTimerCallback(std::bind(&MjpegRequestHandler::HandleTimer, this, conn));
        Timestamp nextTime = addTime(Timestamp::now(), FrameInterval * 1000);
        conn->setTimer(nextTime);
        LOG_DEBUG << "Mjpeg Stream connect name is " << conn->name() << "handle next time:" << nextTime.toFormattedString();
    }
}

void MjpegRequestHandler::HandleTimer(const net::TcpConnectionPtr &conn)
{
    uint32_t handlingTime = 0;
    Timestamp startTime = Timestamp::now();

    // 创建数据
    WebResponse response(false);
    if (!Owner->IsError())
    {

        Owner->EncodeCameraImage();
    }

    if ((Owner == nullptr) || (Owner->IsError()) || (Owner->JpegSize == 0))
    {
        response.setCloseConnection(true);
        // 注意这里是直接执行函数，需要主动关闭连接
        conn->shutdown();
        LOG_INFO << conn->name() << "is closed";
    }
    else
    {
        std::lock_guard<std::mutex> lock(Owner->BufferGuard);

        // don't try sending too much on slow connections - it will only create video lag
        if (conn->outputBuffer()->readableBytes() < 2 * Owner->JpegSize)
        {
            net::Buffer buf;
            // 注意这里的开头和结尾界定符号
            string extenHeader = "\r\n--myboundary\r\nContent-Type: image/jpeg\r\nContent-Length: " + std::to_string(Owner->JpegSize) + "\r\n\r\n";
            buf.append(extenHeader);
            buf.append(std::string((char *)Owner->JpegBuffer, Owner->JpegSize));
            conn->send(&buf);
        }
        else
        {
            LOG_INFO << conn->name() << "buffer is full";
        }

        // 检查是否需要设置下一个
        if (conn->connected() && (!response.closeConnection()))
        {
            // 获取现在时间
            Timestamp endTime = Timestamp::now();
            // get final request handling time
            handlingTime = static_cast<uint32_t>(endTime.microSecondsSinceEpoch() - startTime.microSecondsSinceEpoch());
            uint32_t nextTimespace = ((handlingTime >= FrameInterval * 1000) ? 1000 : FrameInterval * 1000 - handlingTime);
            // set new timer for further images
            Timestamp next_time = addTime(Timestamp::now(), nextTimespace);
            conn->setTimer(next_time);
        }
        else
        {
            LOG_INFO << conn->name() << "is closed,No Next Frame";
        }
    }
    if (response.closeConnection())
    {
        conn->shutdown();
    }
}

NAMESPACE_END