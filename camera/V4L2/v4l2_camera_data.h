/**
 * @file v4l2_camera_data.h
 * @brief v4l2封装的基础类，不包含统一的接口
 * @author wangpengcheng (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2021-12-06 11:02:30
 * @copyright Copyright (c) 2021  IRLSCU
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
 *    <td> 2021-12-06 11:02:30 </td>
 *    <td> 1.0 </td>
 *    <td> wangpengcheng </td>
 *    <td> 添加文档注释 </td>
 * </tr>
 * </table>
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
    Brightness = 0,        /* 亮度 */
    Contrast,              /* 对比度 */
    Saturation,            /* 饱和度 */
    Hue,                   /* 色调 */
    Sharpness,             /* 锐利 */
    Gain,                  /* 增益 */
    BacklightCompensation, /* 背光补偿 */
    RedBalance,            /* 红平衡 */
    BlueBalance,           /* 蓝平衡 */
    AutoWhiteBalance,      /* 自动白平衡 */
    HorizontalFlip,        /* 水平翻转 */
    VerticalFlip           /* 垂直翻转 */
};

typedef BaseVideoProperty VideoProperty;

/* v4l2 用户缓冲buffer大小 默认为4  */
#define BUFFER_COUNT (4)
/**
 * @brief V4l2数据对象封装类
 */
class V4L2CameraData
{
public:
    /**
     * @brief Construct a new V4L2CameraData object
     */
    V4L2CameraData() : Sync(), ConfigSync(), ControlThread(), NeedToStop(), Listener(nullptr), Running(false),
                       VideoFd(-1), VideoStreamingActive(false), MappedBuffers(), MappedBufferLength(), PropertiesToSet(),
                       VideoDeviceIndex(0),
                       FramesReceived(0), FrameWidth(640), FrameHeight(480), FrameRate(30), JpegEncoding(true)
    {
    }
    /* ===== 信号管理函数 ===== */

    /**
     * @brief  开始运行数据监听线程
     * @return true
     * @return false
     */
    bool Start();
    /**
     * @brief 发送信号进行停止
     */
    void SignalToStop();
    /**
     * @brief 等待信号进行停止
     */
    void WaitForStop();
    /**
     * @brief 检查是否正在运行
     * @return true  正在运行
     * @return false 没有运行
     */
    bool IsRunning();
    /* ===== 接收者管理 ===== */
    /**
     * @brief Set the Listener object
     * @param  listener         数据监听者列表
     * @return VideoSourceListenerInterface*
     */
    VideoSourceListenerInterface *SetListener(VideoSourceListenerInterface *listener);

    /* ===== 发送图片，方便响应 ===== */
    /**
     * @brief  发送图片帧函数
     * @param  image     图片数据共享指针
     */
    void NotifyNewImage(const std::shared_ptr<const Image> &image);
    /**
     * @brief  异常抛出信号函数
     * @param  errorMessage     异常信息
     * @param  fatal            是否为最终异常
     */
    void NotifyError(const std::string &errorMessage, bool fatal = false);
    /**
     * @brief 获取数据句柄
     * @param  me    相机原属数据指针
     */
    static void ControlThreadHandler(V4L2CameraData *me);
    /**
     * @brief 设置摄像头编号
     * @param  VideoDeviceIndex 摄像头逻辑编号
     */
    void SetVideoDeviceIndex(uint32_t VideoDeviceIndex);

    /**
     * @brief 设置新的摄像头名称
     * @param  NewDeviceName    摄像头名称
     */
    void SetVideoDeviceName(std::string NewDeviceName);
    /**
     * @brief Get the Video Device Name
     * @return std::string  摄像头名称
     */
    inline std::string GetVideoDeviceName() { return VideoDeviceName; }
    /**
     * @brief 设置图像尺寸
     * @param  width           图像宽度
     * @param  height          图像高度
     */
    void SetVideoSize(uint32_t width, uint32_t height);
    /**
     * @brief 设置帧率
     * @param  frameRate        帧率
     */
    void SetFrameRate(uint32_t frameRate);
    /**
     * @brief 是否使用JPEG，默认为true,否则直接输出YUV格式的数据
     * @param  enable
     */
    void EnableJpegEncoding(bool enable);
    /**
     * @brief 设置摄像头属性
     * @param  property         属性名称
     * @param  value            值
     * @return Error            错误信息
     */
    Error SetVideoProperty(VideoProperty property, int32_t value);
    /**
     * @brief 获取摄像头属性
     * @param  property         属性名称
     * @param  value            值
     * @return Error            错误信息
     */
    Error GetVideoProperty(VideoProperty property, int32_t *value) const;
    /**
     * @brief  获取属性范围
     * @param  property        属性名称
     * @param  min              最小值
     * @param  max              最大值
     * @param  step             增长步长
     * @param  def              差异
     * @return Error            错误信息
     */
    Error GetVideoPropertyRange(VideoProperty property, int32_t *min, int32_t *max, int32_t *step, int32_t *def) const;
    inline std::map<VideoProperty, int32_t> GetVideoProperties() { return PropertiesToSet; }

private:
    /**
     * @brief 相机相关参数初始化
     * @return true  初始化成功
     * @return false  初始化失败
     */
    bool Init();
    /**
     * @brief 开启摄像头数据读取线程循环
     */
    void VideoCaptureLoop();
    /**
     * @brief  清除相关数据
     */
    void Cleanup();

private:
    mutable std::recursive_mutex Sync;                ///< 同步递归锁
    std::recursive_mutex ConfigSync;                  ///< 配置锁
    std::thread ControlThread;                        ///< 控制线程
    ManualResetEvent NeedToStop;                      ///< 事件控制器
    VideoSourceListenerInterface *Listener;           ///< 监听接口指针
    bool Running;                                     ///< 是否正在运行
    int VideoFd;                                      ///< 摄像头文件句柄
    bool VideoStreamingActive;                        ///< 是否使用stream流的方式读取数据
    uint8_t *MappedBuffers[BUFFER_COUNT];             ///<  8bit映射缓冲区--灰度
    uint32_t MappedBufferLength[BUFFER_COUNT];        ///< 32bit映射缓冲区--rgba
    std::map<VideoProperty, int32_t> PropertiesToSet; ///< 属性值

public:
    uint32_t VideoDeviceIndex;                   /** 摄像头index,方便快速查找摄像头 */
    std::string VideoDeviceName = "";            /** 摄像头名称；如：/dev/video0 */
    uint32_t FramesReceived;                     /** 摄像头接收到的frame数量；如：/dev/video0 */
    uint32_t FrameWidth = 0;                     /** 图片宽度 */
    uint32_t FrameHeight = 0;                    /** 图片高度 */
    uint32_t FrameRate;                          /** 帧率 */
    bool JpegEncoding;                           /** 是否为Jpeg编码 */
    std::vector<std::string> SupportVideoFormat; /** 支持的视频格式 */
    // v4l2_buffer MyVideoBuffer;                   /** 视频阵缓冲指针，永远指向最新的值，使用拷贝与内存同步 */
};

NAMESPACE_END

#endif