/**
 * @file file_request_handler.h
 * @brief 文件请求处理具柄
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2022-01-02 01:02:22
 * @copyright Copyright (c) 2022  IRLSCU
 *
 * @par 修改日志:
 * <table>
 * <tr>
 *    <th> Commit date</th>
 *    <th> Version </th>
 *    <th> Author </th>
 *    <th> Description </th>
 * </tr>
 * <tr>
 *    <td> 2022-01-02 01:02:22 </td>
 *    <td> 1.0 </td>
 *    <td> wangpengcheng </td>
 *    <td> 增加文档注释 </td>
 * </tr>
 * </table>
 */
#ifndef FILE_REQUEST_HANDLER_H
#define FILE_REQUEST_HANDLER_H
#include "web_request_handler.h"

NAMESPACE_START
/**
 * @brief 文件请求处理具柄
 */
class FileRequestHandler //: public WebRequestHandlerInterface
{
public:
    /**
     * @brief Construct a new File Request Handler object
     */
    explicit FileRequestHandler();
    /**
     * @brief Construct a new File Request Handler object
     * @param  new_path_        新的文件路径
     */
    FileRequestHandler(const std::string &new_path_);
    /**
     * @brief Set the Root Path
     * @param  new_path         根路径
     */
    inline void setRootPath(const std::string &new_path) { root_path_ = new_path; }
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

private:
    std::string root_path_;
};

NAMESPACE_END

#endif
