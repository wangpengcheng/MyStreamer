/**
 * @file web_request_handler.h
 * @brief web请求处理handler
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2020-12-18 23:06:30
 * @copyright Copyright (c) 2020  IRLSCU
 *
 * @par 修改日志:
 * <table>
 * <tr>
 *    <th> Commit date</th>
 *    <th> Version </th>
 *    <th> Author </th>
 *    <th> Description </th>
 * <tr>
 *    <td> 2020-12-18 23:06:30 </td>
 *    <td> 1.0 </td>
 *    <td> wangpengcheng </td>
 *    <td> 添加注释 </td>
 * </tr>
 * </table>
 */
#ifndef WEB_REQUEST_HANDLER_H
#define WEB_REQUEST_HANDLER_H

#include "net_http_response.h"
#include "net_http_request.h"
#include "video_source_to_webdata.h"
#include "net_tcp_connection.h"
NAMESPACE_START

/**
 * @brief 请求响应控制句柄函数，主要是方便添加reposens函数;对于不同的reques和Response进行处理
 */
class WebRequestHandlerInterface
{
public:
    /**
     * @brief Construct a new Web Request Handler Interface object
     * @param  uri              HTTP请求的路径
     * @param  canHandleSubContentMy 是否使用子上下文解析
     */
    WebRequestHandlerInterface(const std::string &uri, bool canHandleSubContent);
    /**
     * @brief Destroy the Web Request Handler Interface object
     */
    virtual ~WebRequestHandlerInterface();
    /**
     * @brief 处理的关键函数，继承类必须实现
     * @param  conn             TCP 连接
     * @param  request          解析的请求
     * @param  response         请求的响应
     */
    virtual void HandleHttpRequest(const net::TcpConnectionPtr &conn, const WebRequest &request, WebResponse &response) = 0;
    /**
     * @brief  获取方法对应的URL
     * @return std::string
     */
    inline std::string Uri() const { return mUri; }
    /**
     * @brief  是否使用子处理函数请求
     * @return true             是
     * @return false            否
     */
    inline bool CanHandleSubContent() const { return mCanHandleSubContent; }

private:
    std::string mUri;          ///< 对应处理的url参数
    bool mCanHandleSubContent; ///< 是否使用子链接
};

/**
 * @brief jpeg图片请求，对网络请求进行再次封装,每次请求输出单张图片
 */
class JpegRequestHandler : public WebRequestHandlerInterface
{
public:
    JpegRequestHandler(
        const string &uri,
        VideoSourceToWebData *owner) : WebRequestHandlerInterface(uri, false),
                                       Owner(owner)
    {
    }
    void HandleHttpRequest(
        const net::TcpConnectionPtr &conn,
        const WebRequest &request,
        WebResponse &response);

private:
    VideoSourceToWebData *Owner; ///< 关键操作处理函数
};

/**
 * @brief MJPEG stream 流发送
 */
class MjpegRequestHandler : public WebRequestHandlerInterface
{
public:
    /**
     * @brief Construct a new Mjpeg Request Handler object
     * @param  uri              请求url
     * @param  frameRate        设置请求的帧率
     * @param  owner            拥有者
     */
    MjpegRequestHandler(
        const string &uri,
        uint32_t frameRate,
        VideoSourceToWebData *owner) : WebRequestHandlerInterface(uri, false),
                                       Owner(owner),
                                       FrameInterval(1000 / frameRate)
    {
    }
    /**
     * @brief  请求处理函数
     * @param  conn             TCP连接
     * @param  request          请求对象
     * @param  response         处理对象
     */
    void HandleHttpRequest(
        const net::TcpConnectionPtr &conn,
        const WebRequest &request,
        WebResponse &response);
    /**
     * @brief  定义唤醒处理函数，用来定时主动请求
     * @param  conn             TCP连接对象
     */
    void HandleTimer(const net::TcpConnectionPtr &conn);

private:
    VideoSourceToWebData *Owner; ///< 数据函数封装类
    uint32_t FrameInterval;      ///< 图像的帧率
};

NAMESPACE_END
#endif