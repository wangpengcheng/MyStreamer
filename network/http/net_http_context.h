#ifndef NET_HTTP_HTTPCONTEXT_H
#define NET_HTTP_HTTPCONTEXT_H

#include "uncopyable.h"
#include "net_http_request.h"

NAMESPACE_START

namespace net
{
    
    class Buffer;
    /**
     * @brief http上下文信息，用来解析请求状态参数 
     */
    class HttpContext : public copyable
    {
    public:
        /**
         * @brief HTTP 请求参数枚举类
         */
        enum HttpRequestParseState
        {
            kExpectRequestLine,     /** 处理请求行 */
            kExpectHeaders,         /** 处理HTTP头部 */
            kExpectBody,            /** 处理Body信息 */
            kGotAll,                /** 处理完成 */
        };

        HttpContext()
            : state_(kExpectRequestLine)
        {
        }

        // default copy-ctor, dtor and assignment are fine

        // 解析请求
        bool parseRequest(Buffer *buf, Timestamp receiveTime);

        bool gotAll() const
        {
            return state_ == kGotAll;
        }

        void reset()
        {
            state_ = kExpectRequestLine;
            HttpRequest dummy;
            request_.swap(dummy);
        }

        const HttpRequest &request() const
        {
            return request_;
        }

        HttpRequest &request()
        {
            return request_;
        }

    private:
        bool processRequestLine(const char *begin, const char *end);

        HttpRequestParseState state_; /** 请求状态 */
        HttpRequest request_;         /** 请求解析 */
    };

} // namespace net
NAMESPACE_END

#endif // NET_HTTP_HTTPCONTEXT_H
