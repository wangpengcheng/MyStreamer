/**
 * @file web_camera_control_handler.h
 * @brief 摄像头属性查询和参数控制api类
 * @note 注意一定要摄像头支持的属性
 * @todo 摄像头控制的可视化
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2022-01-02 01:14:47
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
 *    <td> 2022-01-02 01:14:47 </td>
 *    <td> 1.0 </td>
 *    <td> wangpengcheng </td>
 *    <td> 增加文档注释 </td>
 * </tr>
 * </table>
 */
#ifndef WEB_CMAERA_CONTROL_HANDLER_H
#define WEB_CMAERA_CONTROL_HANDLER_H
#include "web_request_handler.h"
#include "v4l2_camera_config.h"

NAMESPACE_START
/**
 * @brief 摄像头信息查询类
 */
class CameraInfoHandler : public WebRequestHandlerInterface
{
public:
    /**
     * @brief Construct a new Camera Info Handler object
     * @param  camera           摄像头
     * @param  url              对应url描述符号
     */
    CameraInfoHandler(const std::shared_ptr<V4L2Camera> &camera, const std::string &url);
    /**
     * @brief 响应函数
     * @param  request          请求头部
     * @param  response         响应
     */
    void HandleHttpRequest(const WebRequest &request, WebResponse &response);

private:
    std::shared_ptr<V4L2CameraConfig> camera_data_; ///< data
};

/**
 * @brief 摄像头属性控制类
 */
class CameraControlHandler : public WebRequestHandlerInterface
{
public:
    /**
     * @brief Construct a new Camera Info Handler object
     * @param  camera           摄像头
     * @param  url              对应url描述符号
     */
    CameraControlHandler(const std::shared_ptr<V4L2Camera> &camera, const std::string &url);
    /**
     * @brief 响应函数
     * @param  request          请求头部
     * @param  response         响应
     */
    void HandleHttpRequest(const WebRequest &request, WebResponse &response);

private:
    std::shared_ptr<V4L2CameraConfig> camera_data_; ///< data
};

NAMESPACE_END
#endif