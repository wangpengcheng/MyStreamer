/**
 * v4l2封装的基础类，不包含统一的接口
*/
#ifndef V4L2_CAMERA_DATA_H
#define V4L2_CAMERA_DATA_H


/*===== C++  header start ======*/
#include <map>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>
/*===== C++  header end ======*/

/*===== project  header start ======*/
#include "video_source_interface.h"
#include "base_manual_reset_event.h"
#include "uncopyable.h"
#include "v4l2_tools.h"

/*===== project  header end ======*/


NAMESPACE_START


/* 定义摄像头相关参数 */
enum class BaseVideoProperty
{
    Brightness = 0,                 /* 亮度 */
    Contrast,                       /* 对比度 */
    Saturation,                     /* 饱和度 */
    Hue,                            /* 色调 */
    Sharpness,                      /* 锐利 */
    Gain,                           /* 增益 */
    BacklightCompensation,          /* 背光补偿 */
    RedBalance,                     /* 红平衡 */
    BlueBalance,                    /* 蓝平衡 */
    AutoWhiteBalance,               /* 自动白平衡 */
    HorizontalFlip,                 /* 水平翻转 */
    VerticalFlip                    /* 垂直翻转 */
};

typedef BaseVideoProperty VideoProperty;

/* v4l2 用户缓冲buffer大小 */
#define BUFFER_COUNT        (4)

class V4L2CameraData
{
public:
    V4L2CameraData( ) :
        Sync( ), ConfigSync( ), ControlThread( ), NeedToStop( ), Listener( nullptr ), Running( false ),
        VideoFd( -1 ), VideoStreamingActive( false ), MappedBuffers( ), MappedBufferLength( ), PropertiesToSet( ),
        VideoDeviceIndex( 0 ),
        FramesReceived( 0 ), FrameWidth( 640 ), FrameHeight( 480 ), FrameRate( 30 ), JpegEncoding( true )
    {
    }
    /* ===== 信号管理函数 ===== */
    bool Start( );
    void SignalToStop( );
    void WaitForStop( );
    bool IsRunning( );
    /* ===== 接收者管理 ===== */
    VideoSourceListenerInterface* SetListener( VideoSourceListenerInterface* listener );
    
    /* ===== 发送图片，方便响应 ===== */
    void NotifyNewImage( const std::shared_ptr<const Image>& image );
    void NotifyError( const std::string& errorMessage, bool fatal = false );
    /* 获取数据句柄 */
    static void ControlThreadHanlder( V4L2CameraData* me );
    /* 设置摄像头编号 */
    void SetVideoDeviceIndex( uint32_t VideoDeviceIndex );
    /* 设置新的摄像头名称 */
    void SetVideoDeviceName(std::string NewDeviceName);
    inline std::string GetVideoDeviceName(){ return VideoDeviceName;}
    /* 设置图像尺寸 */
    void SetVideoSize( uint32_t width, uint32_t height );
    /* 设置帧率 */
    void SetFrameRate( uint32_t frameRate );
    /* 是否使用JPEG */
    void EnableJpegEncoding( bool enable );
    /* 设置摄像头属性函数 */
    Error SetVideoProperty( VideoProperty property, int32_t value );
    Error GetVideoProperty( VideoProperty property, int32_t* value ) const;
    Error GetVideoPropertyRange( VideoProperty property, int32_t* min, int32_t* max, int32_t* step, int32_t* def ) const;

    private:
    bool Init( );
    void VideoCaptureLoop( );
    void Cleanup( );
private:
    mutable std::recursive_mutex    Sync;                                   /* 同步递归锁 */
    std::recursive_mutex            ConfigSync;                             /* 配置锁 */
    std::thread                     ControlThread;                          /* 控制线程 */
    ManualResetEvent                NeedToStop;                             /* 事件控制器 */
    VideoSourceListenerInterface*   Listener;                               /* 监听接口指针 */

    bool                    Running;                                        /* 是否正在运行 */

    int                     VideoFd;                                        /* 摄像头文件句柄 */
    bool                    VideoStreamingActive;
    uint8_t*                MappedBuffers[BUFFER_COUNT];                    /* 8bit映射缓冲区--灰度 */
    uint32_t                MappedBufferLength[BUFFER_COUNT];               /* 32bit映射缓冲区--rgba */

    std::map<VideoProperty, int32_t> PropertiesToSet;                            /* 属性值设置 */

public:
    uint32_t                    VideoDeviceIndex;                           /* 摄像头index,方便快速查找摄像头 */
    std::string                 VideoDeviceName="";                         /* 摄像头名称；如：/dev/video0 */
    uint32_t                    FramesReceived;                             /* 摄像头接收到的frame数量；如：/dev/video0 */
    uint32_t                    FrameWidth=0;                                 /* 图片宽度 */
    uint32_t                    FrameHeight=0;                                /* 图片高度 */
    uint32_t                    FrameRate;                                  /* 帧率 */
    bool                        JpegEncoding;                               /* 是否为Jpeg编码 */
    std::vector<std::string>    SupportVideoFramte;                         /* 支持的视频格式 */
    //v4l2_buffer                 MyVideoBuffer;                                /* 视频阵缓冲指针，永远指向最新的值，使用拷贝与内存同步 */
};


NAMESPACE_END

#endif