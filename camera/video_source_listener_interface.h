/**
 * @file video_source_listener_interface.h
 * @brief 监听者模式下的，视频监听数据相关类
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2020-12-16 00:07:54
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
 *    <td> 2020-12-16 00:07:54 </td>
 *    <td> 1.0 </td>
 *    <td> wangpengcheng </td>
 *    <td> 添加文档注释 </td>
 * </tr>
 * </table>
 */

#ifndef VIDEO_SOURCE_LISTENER_INTERFACE_H
#define VIDEO_SOURCE_LISTENER_INTERFACE_H

#include <string>
#include <list>

#include "base_tool.h"
#include "image.h"

NAMESPACE_START

/**
 * @brief 接收视频数据接口类
 */
class VideoSourceListenerInterface
{
public:
    /**
     * @brief Destroy the Video Source Listener Interface object
     */
    virtual ~VideoSourceListenerInterface() {}
    /**
     * @brief  接受图片的处理信号函数
     * @param  image            接受图片数据指针
     */
    virtual void OnNewImage(const std::shared_ptr<const Image> &image) = 0;
    /**
     * @brief  错误信息接受处理接口
     * @param  errorMessage     错误消息
     * @param  fatal            是否扩散
     */
    virtual void OnError(const std::string &errorMessage, bool fatal) = 0;
};

/**
 * @brief 监听者链类，组合执行多个线程的监听
 */
class VideoSourceListenerChain : public VideoSourceListenerInterface
{
public:
    /**
     * @brief  接受图片的处理信号函数
     * @param  image            接受图片数据指针
     */
    virtual void OnNewImage(const std::shared_ptr<const Image> &image)
    {
        for (auto listener : chain)
        {
            listener->OnNewImage(image);
        }
    }

    /**
     * @brief  错误信息接受处理接口
     * @param  errorMessage     错误消息
     * @param  fatal            是否扩散
     */
    virtual void OnError(const std::string &errorMessage, bool fatal)
    {
        for (auto listener : chain)
        {
            listener->OnError(errorMessage, fatal);
        }
    }
    /**
     * @brief  添加监听者到新的监听队列中 
     * @param  listener         监听者指针
     */
    void Add(VideoSourceListenerInterface *listener)
    {
        if (listener != nullptr)
        {
            chain.push_back(listener);
        }
    }
    /**
     * @brief 清除监听者链
     */
    void Clear()
    {
        chain.clear();
    }

private:
    std::list<VideoSourceListenerInterface *> chain; ///< 监听者私有链表
};

NAMESPACE_END // namespace end

#endif //VIDEO_SOURCE_LISTENER_INTERFACE_H