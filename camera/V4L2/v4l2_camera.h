#ifndef V4L2_CAMERA_H
#define V4L2_CAMERA_H
#include "v4l2_tools.h"
#include "v4l2_camera_data.h"
#include "video_source_interface.h"
#include "uncopyable.h"
NAMESPACE_START

// V4L2摄像头控制封装;使用单例模式创建摄像头

class V4L2Camera : public VideoSourceInterface, private Uncopyable
{
protected:
    V4L2Camera( );

public:
    ~V4L2Camera( );

    static const std::shared_ptr<V4L2Camera> Create( );

    // 开始
    bool Start( );
    // Signal source video to stop, so it could finalize and clean-up
    void SignalToStop( );
    // Wait till video source (its thread) stops
    void WaitForStop( );
    // Check if video source is still running
    bool IsRunning( );

    // Get number of frames received since the start of the video source
    uint32_t FramesReceived( );

    // 设置监听者，并返回取代的后面一个
    VideoSourceListenerInterface* SetListener( VideoSourceListenerInterface* listener );

public: //设置相机的各种属性，只能在未运行前使用，开始之后，无法进行操作

    // Set/get video device
    uint32_t VideoDeviceIndex( ) const;
    std::string GetVideoDeviceName() const;
    void SetVideoDeviceIndex( uint32_t videoDeviceIndex );
    void SetVideoDeviceName( std::string videoDeviceName );
    // Get/Set video size
    uint32_t Width( ) const;
    uint32_t Height( ) const;
    void SetVideoSize( uint32_t width, uint32_t height );

    // Get/Set frame rate
    uint32_t FrameRate( ) const;
    void SetFrameRate( uint32_t frameRate );

    // Enable/Disable JPEG encoding
    bool IsJpegEncodingEnabled( ) const;
    void EnableJpegEncoding( bool enable );

public:

    // 特殊属性设置，主要是在运行时进行写入
    Error SetVideoProperty( VideoProperty property, int32_t value );
    // 获取运行时的属性
    Error GetVideoProperty( VideoProperty property, int32_t* value ) const;
    // 获取带有范围值的属性
    Error GetVideoPropertyRange( VideoProperty property, int32_t* min, int32_t* max, int32_t* step, int32_t* def ) const;

private:
    V4L2CameraData* mData;
};

NAMESPACE_END
#endif