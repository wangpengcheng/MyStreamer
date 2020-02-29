
#ifndef WEB_REQUEST_HANDLER_H
#define WEB_REQUEST_HANDLER_H
#include "net_http_response.h"
#include "net_http_request.h"
NAMESPACE_START

/**
 * 强求响应控制句柄函数，主要是方便添加reposens函数;对于不同的reques和Response进行处理
*/

class  WebRequestHandlerInterface
{
typedef net::HttpRequest WebRequest;
typedef net::HttpResponse WebResponse;
public:
    inline std::string Uri( ) const { return mUri;  }
    inline bool CanHandleSubContent( ) const { return mCanHandleSubContent; }
    WebRequestHandlerInterface(const std::string& uri, bool canHandleSubContent);
    virtual ~WebRequestHandlerInterface();
    // 处理的关键函数
    virtual void HandleHttpRequest( const WebRequest& request, WebRequest& response ) = 0;
private:
    std::string mUri;                   /* 对应处理的url参数 */
    bool        mCanHandleSubContent;
};

NAMESPACE_END
#endif