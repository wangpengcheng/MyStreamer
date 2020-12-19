/**
 * @file web_camera_server.h
 * @brief 摄像头服务关键封装类 
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2020-12-18 23:11:45
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
 *    <td> 2020-12-18 23:11:45 </td>
 *    <td> 1.0 </td>
 *    <td> wangpengcheng </td>
 *    <td>内容</td>
 * </tr>
 * </table>
 */
#ifndef WEB_CAMERA_SERVER_H
#define WEB_CAMERA_SERVER_H
#include "uncopyable.h"
#include "video_source_to_web.h"
#include "video_listener.h"
#include "web_request_handler.h"
#include "net_http_server.h"
#include "net_event_loop.h"
#include "file_request_handler.h"
#include "net_tcp_connection.h"
#include <memory>

NAMESPACE_START

using net::TcpConnectionPtr;
/**
 * @brief 摄像头服务关键封装类
 */
class WebCameraServer : public Uncopyable
{

public:
    typedef std::unordered_map<std::string, std::shared_ptr<WebRequestHandlerInterface>> RequestMap;
    /**
     * @brief Construct a new Web Camera Server object
     * @param  path             根路径
     * @param  port             服务器端口
     * @param  server_name      服务器名称
     * @param  thread_num_      设置线程池数量
     */
    WebCameraServer(const std::string &path,
                    const uint16_t port,
                    const std::string &server_name,
                    const uint32_t thread_num_);
    /**
     * @brief Destroy the Web Camera Server object
     */
    ~WebCameraServer();
    /**
     * @brief 服务器开始
     */
    void Start();
    /**
     * @brief Get the Root Path object
     * @return std::string 根目录位置
     */
    inline std::string getRootPath() { return root_path_; }
    /**
     * @brief Set the Root Path object
     * @param  new_path        新的根部路径
     */
    inline void setRootPath(const string &new_path) { root_path_ = new_path; }
    /**
     * @brief Get the Http Server object
     * @return std::shared_ptr<net::HttpServer> 获取数据指针
     */
    inline std::shared_ptr<net::HttpServer> getHttpServer() { return http_sever_; }
    /**
     * @brief  添加处理句柄
     * @param  hander_name      句柄名称
     * @param  handler          句柄处理函数
     */
    void AddHandler(const string &hander_name, const std::shared_ptr<WebRequestHandlerInterface> handler);

private:
    /**
     * @brief  内部处理请求函数
     * @param  conn             TCP连接对象
     * @param  req              请求
     * @param  resp             响应
     */
    void onRequest(
        const TcpConnectionPtr &conn,
        const WebRequest &req,
        WebResponse *resp);

    RequestMap function_map_;                     ///< 函数句柄映射
    std::string root_path_;                       ///< 根目录地址
    FileRequestHandler file_hander;               ///< 文件查找函数;这里是自带的
    net::EventLoop loop_;                         ///< 循环事件
    std::shared_ptr<net::HttpServer> http_sever_; ///< http 服务器
};
NAMESPACE_END
#endif