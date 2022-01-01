/**
 * @file video_source_to_webdata.h
 * @brief  VideoSourceToWebData 原始转换类封装
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2022-01-02 01:12:19
 * @copyright Copyright (c) 2022  IRLSCU
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
 *    <td> 2022-01-02 01:12:19 </td>
 *    <td> 1.0 </td>
 *    <td> wangpengcheng </td>
 *    <td>内容</td>
 * </tr>
 * </table>
 */

#ifndef VIDEO_SOURCE_TO_WEBDATA_H
#define VIDEO_SOURCE_TO_WEBDATA_H

#include "video_listener.h"
#include "jpeg_encoder.h"
#include "net_http_response.h"

#include <mutex>
NAMESPACE_START

/**
 * @brief 定义结构体的数据类
 */
#define JPEG_BUFFER_SIZE (1024 * 1024)

/**
 * @brief 摄像头camera转换为web的关键函数类
 * @details
 *  主要用于存储数据，连接摄像头和网络服务器
 *  创建buffer进行网络服务器的图片暂存封装
 */
struct VideoSourceToWebData
{
public:
    /**
     * @brief Construct a new Video Source To Web Data object
     * @param  jpegQuality      图片压缩质量
     */
    VideoSourceToWebData(uint16_t jpegQuality) : NewImageAvailable(false),
                                                 VideoSourceError(false),
                                                 InternalError(Error::Success),
                                                 JpegBuffer(nullptr),
                                                 JpegBufferSize(0),
                                                 JpegSize(0),
                                                 VideoSourceListener(this),
                                                 CameraImage(),
                                                 VideoSourceErrorMessage(),
                                                 ImageGuard(),
                                                 BufferGuard(),
                                                 jpeg_encoder(jpegQuality, true)
    {
        /* 为jpeg分配buffer */
        JpegBuffer = (uint8_t *)malloc(JPEG_BUFFER_SIZE);
        if (JpegBuffer != nullptr)
        {
            JpegBufferSize = JPEG_BUFFER_SIZE;
        }
    }

    ~VideoSourceToWebData()
    {
        if (JpegBuffer != nullptr)
        {
            free(JpegBuffer);
            JpegBuffer = nullptr;
        }
    }
    /**
     * @brief  是否错误
     * @return true  错误
     * @return false 正常
     */
    bool IsError();
    /**
     * @brief  异常抛出处理
     * @param  response         异常处理函数信息
     */
    void ReportError(net::HttpResponse &response);
    /**
     * @brief 将图片进行编码
     */
    void EncodeCameraImage();

public:
    volatile bool NewImageAvailable; ///< 是否存在新图片
    volatile bool VideoSourceError;  ///< 视频源错误
    Error InternalError;             ///< 网络错误信息
    /* jpegbuffer相关类;主要是是用来执行jpeg压缩 */
    uint8_t *JpegBuffer;     ///< jpegbuffer缓冲
    uint32_t JpegBufferSize; ///< 压缩图片大小
    /* 图片类 */
    uint32_t JpegSize;                   ///< jpeg 数据大小
    VideoListener VideoSourceListener;   ///< 视频监听者
    std::shared_ptr<Image> CameraImage;  ///< 图片指向source的img
    std::string VideoSourceErrorMessage; ///< 视频源错误信息
    std::mutex ImageGuard;               ///< 图片锁
    std::mutex BufferGuard;              ///< buffer锁
    JpegEncoder jpeg_encoder;            ///< jpeg编码器
};

NAMESPACE_END

#endif