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
    VideoFrameDecorator();

    // 添加新图片
    void OnNewImage( const std::shared_ptr<const Image>& image ) override;
    // 发射错误信息
    void OnError( const std::string& /* errorMessage */, bool /* fatal */ ) override { }

    // 设置摄像头标题
    std::string CameraTitle( ) const;
    void SetCameraTitle( const std::string& title );

    // 设置时间戳
    bool TimestampOverlay( ) const;
    void SetTimestampOverlay( bool enabled );

    // 是否添加标题
    bool CameraTitleOverlay( ) const;
    void SetCameraTitleOverlay( bool enabled );

    // 设置文字颜色类型
    Argb OverlayTextColor( ) const;
    void SetOverlayTextColor( Argb color );

    //设置背景颜色
    Argb OverlayBackgroundColor( ) const;
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