/*
frame修饰器，主要添加水印信息
*/
#ifndef VIDEO_FRAME_DECORATOR_H
#define VIDEO_FRAME_DECORATOR_H

#include "base_tool.h"
#include "video_source_listener_interface.h"

NAMESPACE_START
/**
 *  图片帧装饰器，主要用来进行图片添加水印
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
    std::string cameraTitle;                    /* 摄像机标题 */
    bool        addTimestampOverlay;            /* 是否添加时间戳 */
    bool        addCameraTitleOverlay;          /* 是否添加标题 */
    Argb       overlayTextColor;                /* 设置色彩颜色 */
    Argb       overlayBackgroundColor;          /* 设置背景颜色 */
};

NAMESPACE_END


#endif