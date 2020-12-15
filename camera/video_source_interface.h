/**
 * @file video_source_interface.h
 * @brief 视频源，统一抽象接口类 定义公共函数接口
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2020-12-16 00:17:32
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
 *    <td> 2020-12-16 00:17:32 </td>
 *    <td> 1.0 </td>
 *    <td> wangpengcheng </td>
 *    <td> 添加文档注释 </td>
 * </tr>
 * </table>
 */

#ifndef VIDEO_SOURCE_INTERFACE_H
#define VIDEO_SOURCE_INTERFACE_H

#include "video_source_listener_interface.h"
#include "base_tool.h"

NAMESPACE_START
/**
 * @brief 视频源接口类
 */
class VideoSourceInterface
{
public:
    /**
     * @brief Destroy the Video Source Interface object
     */
    virtual ~VideoSourceInterface() {}
    /**
     * @brief  开始获取视频，并初始化缓冲诊队列
     * @return true 执行成功
     * @return false 执行失败
     */
    virtual bool Start() = 0;
    /**
     * @brief 发送停止信号,安全停止数据采集线程
     */
    virtual void SignalToStop() = 0;
    /**
     * @brief 等待所有线程停止
     */
    virtual void WaitForStop() = 0;
    /**
     * @brief 检查视频源是否正常运行
     * @return true 正常运行
     * @return false 不正常运行
     */
    virtual bool IsRunning() = 0;
    /**
     * @brief 检查获取到的视频帧数目
     * @return uint32_t 获取帧数的结果
     */
    virtual uint32_t FramesReceived() = 0;
    /**
     * @brief 设置视频监听者
     * @param  listener         监听者
     * @return VideoSourceListenerInterface* 监听结果
     */
    //
    virtual VideoSourceListenerInterface *SetListener(VideoSourceListenerInterface *listener) = 0;
};

NAMESPACE_END

#endif //VIDEO_SOURCE_INTERFACE_H