
#ifndef WEB_REQUEST_HANDLER_H
#define WEB_REQUEST_HANDLER_H
#include "net_http_response.h"
#include "net_http_request.h"
/**
 * 强求响应控制句柄函数，主要是方便添加reposens函数;对于不同的reques和Response进行处理
*/
class  WebRequestHandlerInterface
{
private:
    /* data */
public:
     WebRequestHandlerInterface(const std::string& uri, bool canHandleSubContent);
    virtual ~ WebRequestHandlerInterface();
};

 WebRequestHandler:: WebRequestHandler(/* args */)
{
}

 WebRequestHandler::~ WebRequestHandler()
{
}
#endif