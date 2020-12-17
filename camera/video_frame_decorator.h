/**
 * @file video_frame_decorator.h
 * @brief frame修饰器，主要添加水印信息
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2020-12-16 00:21:58
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
 *    <td> 2020-12-16 00:21:58 </td>
 *    <td> 1.0 </td>
 *    <td> wangpengcheng </td>
 *    <td>添加文档注释</td>
 * </tr>
 * </table>
 */
#ifndef VIDEO_FRAME_DECORATOR_H
#define VIDEO_FRAME_DECORATOR_H

#include "base_tool.h"
#include "video_source_listener_interface.h"

NAMESPACE_START
/**
 * @brief 图片帧装饰器类，主要用来进行图片添加水印
 */
class VideoFrameDecorator : public VideoSourceListenerInterface
{
public:
    /**
     * @brief Construct a new Video Frame Decorator object
     */
    VideoFrameDecorator();

    /**
     * @brief  新图片处理槽函数
     * @param  image            图片数据共享指针
     */
    void OnNewImage( const std::shared_ptr<const Image>& image ) override;
    /**
     * @brief 发射错误信息
     */
    void OnError( const std::string& /* errorMessage */, bool /* fatal */ ) override { }

    /**
     * @brief  获取摄像头标题
     * @return std::string  摄像头标题名称
     */
    std::string CameraTitle( ) const;
    /**
     * @brief 设置摄像头标题
     * @param  title            摄像头标题名称
     */
    void SetCameraTitle( const std::string& title );

    /**
     * @brief  是否已开启时间戳
     * @return true  
     * @return false 
     */
    bool TimestampOverlay( ) const;
    /**
     * @brief 开启时间戳
     * @param  enabled          是否开启时间戳
     */
    void SetTimestampOverlay( bool enabled );
    /**
     * @brief  是否需要添加标题
     * @return true 
     * @return false 
     */
    bool CameraTitleOverlay( ) const;
    /**
     * @brief Set the Camera Title Overlay object
     * @param  enabled          是否开启
     */
    void SetCameraTitleOverlay( bool enabled );

    /**
     * @brief  设置文字颜色类型
     * @return Argb 
     */
    Argb OverlayTextColor( ) const;
    /**
     * @brief Set the Overlay Text Color object
     * @param  color            颜色
     */
    void SetOverlayTextColor( Argb color );

    /**
     * @brief 查询背景颜色
     * @return Argb 
     */
    Argb OverlayBackgroundColor( ) const;
    /**
     * @brief 设置背景颜色
     * @param  color            对应颜色信息
     */
    void SetOverlayBackgroundColor( Argb color );

private:
    std::string cameraTitle;                    ///< 摄像机标题
    bool        addTimestampOverlay;            ///< 是否添加时间戳
    bool        addCameraTitleOverlay;          ///< 是否添加标题
    Argb       overlayTextColor;                ///< 设置色彩颜色
    Argb       overlayBackgroundColor;          ///< 设置背景颜色
};

NAMESPACE_END


#endif