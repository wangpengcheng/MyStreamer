/**
 * @file jpeg_encoder.h
 * @brief  jpeg格式转化类，主要将rgb图像；进行jpeg编码
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2022-01-02 00:48:22
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
 *    <td> 2022-01-02 00:48:22 </td>
 *    <td> 1.0 </td>
 *    <td> wangpengcheng </td>
 *    <td> 增加文档注释 </td>
 * </tr>
 * </table>
 */

#ifndef JPEG_ENCODER_H
#define JPEG_ENCODER_H
#include <stdio.h>
#include <jpeglib.h>
#include "uncopyable.h"
#include "image.h"
#include "base_error.h"
#include "img_tools.h"

NAMESPACE_START

/**
 * @brief 定义对libjpeg函数的统一封装类
 * @details 主要用于原始数据转向jpeg
 * - [libjpeg](https://www.iteye.com/blog/canlynet-1433259)
 * - [libjpeg2](https://blog.csdn.net/qq_34888036/article/details/81058600)
 */
class JpegEncoderData
{
public:
    /**
     * @brief Construct a new Jpeg Encoder Data object
     * @param  quality             压缩质量
     * @param  fasterCompressionMy 是否启用快速压缩，默认开启
     */
    JpegEncoderData(uint16_t quality, bool fasterCompression);

    ~JpegEncoderData();
    /**
     * @brief  将数据压缩至指定内存地址
     * @param  image            图像数据指针
     * @param  buffer           缓冲区指针
     * @param  bufferSize       缓冲区大小
     * @return Error            错误信息
     */
    Error EncodeToMemory(const std::shared_ptr<const Image> &image, uint8_t **buffer, uint32_t *bufferSize);

public:
    uint16_t Quality;       /** 图片质量参数 */
    bool FasterCompression; /** 是否使用快速压缩 */
private:
    struct jpeg_compress_struct cinfo; /** jpeg压缩信息结构体 */
    struct jpeg_error_mgr jerr;        /** 错误信息 */
};
/**
 *
 */

/**
 * @brief 定义jpegencoder对外接口
 */
class JpegEncoder : private Uncopyable
{
public:
    /**
     * @brief Construct a new Jpeg Encoder object
     * @param  quality             压缩质量
     * @param  fasterCompressionMy 是否启用快速压缩，默认开启
     */
    JpegEncoder(uint16_t quality = 85, bool fasterCompression = false);
    ~JpegEncoder();
    /**
     * @brief 获取图像质量
     * @details Set/get compression quality, [0, 100]
     * @return uint16_t 图像质量数据
     */
    uint16_t Quality() const;
    /**
     * @brief Set the Quality object
     * @param  quality          目标压缩质量
     */
    void SetQuality(uint16_t quality);

    /**
     * @brief get faster compression (but less accurate) flag
     * @return true  启用
     * @return false 未启用
     */
    bool FasterCompression() const;
    /**
     * @brief Set the Faster Compression object
     * @param  faster           开启快速压缩
     */
    void SetFasterCompression(bool faster);
    /**
     * @brief Compress the specified image into provided buffer
     * @details
     *  On input, buffer size must be set to the size of provided buffer.
     *  On output, it is set to the size of encoded JPEG image. If provided
     *  buffer is too small, it will be re-allocated (realloc).
     * @param  image            图像数据指针
     * @param  buffer           缓冲区指针
     * @param  bufferSize       缓冲区大小
     * @return Error            错误信息
     */
    Error EncodeToMemory(const std::shared_ptr<const Image> &image, uint8_t **buffer, uint32_t *bufferSize);

private:
    JpegEncoderData *mData; ///< jpeg 数据封装类
};

NAMESPACE_END

#endif