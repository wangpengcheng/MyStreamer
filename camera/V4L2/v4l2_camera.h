/**
 * @file v4l2_camera.h
 * @brief  相机抽象接口类
 * @todo 设置单例表，每个保证每个设备单例稳定
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2020-12-16 20:26:56
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
 *    <td> 2020-12-16 20:26:56 </td>
 *    <td> 1.0 </td>
 *    <td> wangpengcheng </td>
 *    <td>内容</td>
 * </tr>
 * </table>
 */
#ifndef V4L2_CAMERA_H
#define V4L2_CAMERA_H
#include "v4l2_tools.h"
#include "v4l2_camera_data.h"
#include "video_source_interface.h"
#include "uncopyable.h"
NAMESPACE_START

/**
 * @brief V4L2摄像头控制封装;使用单例模式创建摄像头
 */
class V4L2Camera : public VideoSourceInterface, private Uncopyable
{
protected:
    /**
     * @brief 基础构造函数
     */
    V4L2Camera();

public:
    /**
     * @brief Destroy the V4L2Camera object
     */
    ~V4L2Camera();
    /**
     * @brief  单例构造函数类
     * @return const std::shared_ptr<V4L2Camera>
     */
    static const std::shared_ptr<V4L2Camera> Create();

    /**
     * @brief  相机开始工作
     * @return true  开启相机成功
     * @return false 开启相机失败
     */
    bool Start();
    /**
     * @brief 发射信号，相机停止，并清空数据
     */
    void SignalToStop();

    /**
     * @brief 等待直到收到停止信号
     */
    void WaitForStop();
    /**
     * @brief  检查相机是否正在正常运行
     * @return true
     * @return false
     */
    bool IsRunning();

    /**
     * @brief  从开始接收到的帧数
     * @return uint32_t  接收到的帧数
     */
    uint32_t FramesReceived();

    /**
     * @brief Set the Listener object
     * @param  listener         监听者接收类
     * @return VideoSourceListenerInterface*
     */
    VideoSourceListenerInterface *SetListener(VideoSourceListenerInterface *listener);

public:
    ///< 设置相机的各种属性，只能在未运行前使用，开始之后，无法进行操作

    /**
     * @brief  get video device
     * @return uint32_t 当前对应的索引
     */
    uint32_t VideoDeviceIndex() const;
    /**
     * @brief Get the Video Device Name object
     * @return std::string 设备名称
     */
    std::string GetVideoDeviceName() const;
    /**
     * @brief Set the Video Device Index object
     * @param  videoDeviceIndex 设备编号和索引
     */
    void SetVideoDeviceIndex(uint32_t videoDeviceIndex);
    /**
     * @brief Set the Video Device Name object
     * @param  videoDeviceName  设备名称
     */
    void SetVideoDeviceName(std::string videoDeviceName);
    /**
     * @brief 获取长度
     * @return uint32_t 对应长度
     */
    uint32_t Width() const;
    /**
     * @brief   对应高度
     * @return uint32_t 高度大小
     */
    uint32_t Height() const;
    /**
     * @brief Set the Video Size object
     * @param  width            宽度
     * @param  height           高度
     */
    void SetVideoSize(uint32_t width, uint32_t height);

    /**
     * @brief 查询帧率
     * @return uint32_t
     */
    uint32_t FrameRate() const;
    /**
     * @brief  设置帧率
     * @param  frameRate        My Param doc
     */
    void SetFrameRate(uint32_t frameRate);
    /**
     * @brief  开启jpeg编码
     * @return true
     * @return false
     */
    bool IsJpegEncodingEnabled() const;
    /**
     * @brief 关闭jpeg编码
     * @param  enable           My Param doc
     */
    void EnableJpegEncoding(bool enable);

public:
    /**
     * @brief 特殊属性设置，主要是在运行时进行写入
     * @param  property         属性名称
     * @param  value            属性值
     * @return Error            错误信息
     */
    Error SetVideoProperty(VideoProperty property, int32_t value);
    /**
     * @brief 获取运行时的属性
     * @param  property         属性名称
     * @param  value            属性值
     * @return Error            错误信息
     */
    Error GetVideoProperty(VideoProperty property, int32_t *value) const;
    /**
     * @brief 获取带有范围值的属性
     * @param  property         属性名称
     * @param  value            属性值
     * @return Error            错误信息
     */
    Error GetVideoPropertyRange(VideoProperty property, int32_t *min, int32_t *max, int32_t *step, int32_t *def) const;

private:
    V4L2CameraData *mData; ///< 封装的数据接口类
};

NAMESPACE_END
#endif