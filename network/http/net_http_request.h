#ifndef NET_HTTP_HTTPREQUEST_H
#define NET_HTTP_HTTPREQUEST_H



#include "uncopyable.h"
#include "time_stamp.h"
#include "base_types.h"



#include <map>
#include <assert.h>
#include <stdio.h>
#include <unordered_map>
NAMESPACE_START
/**
 * 对http请求结构体的优化
*/
namespace net
{

class HttpRequest : public copyable
{
public:
    enum Method
    {
        kInvalid, 
        kGet, 
        kPost, 
        kHead, 
        kPut, 
        kDelete
    };
    enum Version
    {
        kUnknown, 
        kHttp10, 
        kHttp11,

    };
    struct HttpDataType
    {
        static std::unordered_map<std::string,Method> request_methods;
    };

    HttpRequest()
        : method_(kInvalid),
        version_(kUnknown)
    {
    }

    void setVersion(Version v)
    {
        version_ = v;
    }

    inline Version getVersion() const
    { return version_; }
    /* 设置http的请求 */
    bool setMethod(const char* start, const char* end)
    {
        assert(method_ == kInvalid);
        string m(start, end);
        
        if (m == "GET")
        {
            method_ = kGet;
        }
        else if (m == "POST")
        {
            method_ = kPost;
        }
        else if (m == "HEAD")
        {
            method_ = kHead;
        }
        else if (m == "PUT")
        {
            method_ = kPut;
        }
        else if (m == "DELETE")
        {
            method_ = kDelete;
        }
        else
        {
            method_ = kInvalid;
        }
        return method_ != kInvalid;
    }

    Method method() const
    { return method_; }

    const char* methodString() const
    {
        const char* result = "UNKNOWN";
        switch(method_)
        {
        case kGet:
            result = "GET";
            break;
        case kPost:
            result = "POST";
            break;
        case kHead:
            result = "HEAD";
            break;
        case kPut:
            result = "PUT";
            break;
        case kDelete:
            result = "DELETE";
            break;
        default:
            break;
        }
        return result;
    }
    /* 设置路径 */
    void setPath(const char* start, const char* end)
    {
        path_.assign(start, end);
    }

    const string& path() const
    { return path_; }
    /* 设置查询参数 */
    void setQuery(const char* start, const char* end)
    {
        query_.assign(start, end);
    }

    const string& query() const
    { return query_; }

    void setReceiveTime(Timestamp t)
    { receiveTime_ = t; }

    Timestamp receiveTime() const
    { return receiveTime_; }

    void addHeader(const char* start, const char* colon, const char* end)
    {
        string field(start, colon);
        ++colon;
        /* 解析参数 */
        while (colon < end && isspace(*colon))
        {
            ++colon;
        }
        string value(colon, end);
        while (!value.empty() && isspace(value[value.size()-1]))
        {
            value.resize(value.size()-1);
        }
        headers_[field] = value;
    }

    string getHeader(const string& field) const
    {
        string result;
        std::map<string, string>::const_iterator it = headers_.find(field);
        if (it != headers_.end())
        {
            result = it->second;
        }
        return result;
    }

    const std::map<string, string>& headers() const
    { return headers_; }

    void swap(HttpRequest& that)
    {
        std::swap(method_, that.method_);
        std::swap(version_, that.version_);
        path_.swap(that.path_);
        query_.swap(that.query_);
        receiveTime_.swap(that.receiveTime_);
        headers_.swap(that.headers_);
    }

private:
    Method method_;                         /* 使用方法 */
    Version version_;                       /* http版本信息 */
    string path_;                           /* 路径 */
    string query_;                          /* 查询参数 */
    Timestamp receiveTime_;                 /* 接收时间 */
    std::map<string, string> headers_;      /* header相关参数 */
};

}  // namespace net
typedef net::HttpRequest WebRequest;
NAMESPACE_END

#endif  // NET_HTTP_HTTPREQUEST_H