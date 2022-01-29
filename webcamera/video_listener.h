/**
 * @file video_listener.h
 * @brief 视频监听基础类
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2020-12-18 22:58:52
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
 *    <td> 2020-12-18 22:58:52 </td>
 *    <td> 1.0 </td>
 *    <td> wangpengcheng </td>
 *    <td> 添加文档 </td>
 * </tr>
 * </table>
 */
#ifndef VIDEO_LISTENER_H
#define VIDEO_LISTENER_H
#include "video_source_listener_interface.h"

NAMESPACE_START
class VideoSourceToWebData;
/**
 * @brief 继承监听接口,方便执行函数获取img数据
 */
class VideoListener : public VideoSourceListenerInterface
{
public:
    /**
     * @brief Construct a new Video Listener object
     * @param  owner            数据封装函数类
     */
    VideoListener(VideoSourceToWebData *owner);
    /**
     * @brief Destroy the Video Listener object
     */
    ~VideoListener();
    /**
     * @brief 重载接收函数
     * @param  image           图像共享指针
     */
    void OnNewImage(const std::shared_ptr<const Image> &image);
    /**
     * @brief  错误类处理接口类
     * @param  errorMessage     错误信息
     * @param  fatal            是否抛出
     */
    void OnError(const std::string &errorMessage, bool fatal);

private:
    VideoSourceToWebData *owner_; ///< 数据转换的关键数据结构
};

NAMESPACE_END
#endif