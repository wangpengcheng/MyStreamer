#ifndef NET_HTTP_HTTPRESPONSE_H
#define NET_HTTP_HTTPRESPONSE_H

#include "uncopyable.h"
#include "base_types.h"
#include "net_tcp_connection.h"
#include "logging.h"
#include <map>
#include <unordered_map>
NAMESPACE_START
/* 主要是组成http respone的关键文字部分
 * 方便设置参数；并将参数写入到buffer中
 */
namespace net
{

    class Buffer;

    class HttpResponse : public copyable
    {
    public:
        /* http响应状态参数 */
        enum HttpStatusCode
        {
            kUnknown = 0,
            k200Ok = 200,
            k301MovedPermanently = 301,
            k400BadRequest = 400,
            k404NotFound = 404,
            k405MethodNotAllowed = 405,
            k500ServerError = 500
        };
        typedef std::unordered_map<int, string> HttpStateMap;
        /* 状态映射图 */
        static HttpStateMap state_map;
        /* 文件请求图 */
        static std::unordered_map<std::string, std::string> file_type;
        explicit HttpResponse(bool close)
            : statusCode_(kUnknown),
              closeConnection_(close)
        {
        }

        void setStatusCode(HttpStatusCode code)
        {
            statusCode_ = code;
        }
        void setExternalHeader(const string &externalHeader) {
            externalHeader_ = externalHeader;
        }
        void setStatusMessage(const string &message)
        {
            statusMessage_ = message;
        }
        /* 是否关闭连接 */
        void setCloseConnection(bool on)
        {
            closeConnection_ = on;
        }

        bool closeConnection() const
        {
            return closeConnection_;
        }
        /* 设置上下文 */
        void setContentType(const string &contentType)
        {
            addHeader("Content-Type", contentType);
        }

        // FIXME: replace string with StringPiece
        void addHeader(const string &key, const string &value)
        {
            headers_[key] = value;
        }
        // 移除多余的header 
        void removeHeader(const string &key) {
            if(headers_.count(key)>0) {
                headers_.erase(key);
            }else {
                LOG_TRACE<<"Header Map no this key";
            }
        };
        inline std::map<std::string, std::string> getHeaders() const
        {
            return headers_;
        }
        /* 设置主体信息 */
        void setBody(const string &body)
        {
            body_ = body;
        }
        inline string& getBody() 
        {
            return body_;
        };
        /* 添加到buffer中 */
        void appendToBuffer(Buffer *output);
        /* 添加快速发送函数 */
        void SendFast(HttpStatusCode send_code, const string &body);

    private:
        std::map<string, string> headers_; /* 结构体的主要信息 */
        HttpStatusCode statusCode_;        /* http状态 */
        // FIXME: add http version
        string statusMessage_; /* 对应的状态回应信息 */
        bool closeConnection_; /* 关闭连接 */
        string body_;          /* http主体信息 */
        string externalHeader_;/* 额外的header 信息，主要是为了mjpeg信息 */
    };
} // namespace net
typedef net::HttpResponse WebResponse;
NAMESPACE_END

#endif // NET_HTTP_HTTPRESPONSE_H