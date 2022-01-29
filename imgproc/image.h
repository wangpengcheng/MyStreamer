/**
 * @file image.h
 * @brief
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2022-01-02 00:25:32
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
 *    <td> 2022-01-02 00:25:32 </td>
 *    <td> 1.0 </td>
 *    <td> wangpengcheng </td>
 *    <td> 增加文档注释 </td>
 * </tr>
 * </table>
 */
#ifndef IMAGE_H
#define IMAGE_H
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include "uncopyable.h"
#include "img_tools.h"

NAMESPACE_START
/**
 * @brief 定义简单基本图像类，包含图像信息和基本数据
 */
class Image : private Uncopyable
{
private:
    /**
     * @brief  图像构造函数
     * @details 构造函数私有化，禁止其在栈上分配内存
     * @param  data            原始数据指针
     * @param  width           宽
     * @param  height          高
     * @param  stride          边缘长度，方便扩充使用
     * @param  format          格式
     * @param  ownMemory       是否自己管理内存, true--自己管理内存， false 仅仅是引用
     */
    Image(uint8_t *data, int32_t width, int32_t height, int32_t stride, PixelFormat format, bool ownMemory);

public:
    ~Image();

    /**
     * @brief  直接内存分配函数
     * @param  width            图像宽度
     * @param  height           图像高度
     * @param  format           图像格式
     * @param  zeroInitialize   进行零值初始化
     * @return std::shared_ptr<Image> 指向数据的共享指针
     */

    static std::shared_ptr<Image> Allocate(int32_t width, int32_t height, PixelFormat format, bool zeroInitialize = false);
    /**
     * @brief  直接在已有的数据内存上直接进行数据拷贝和复制
     * @param  data            原始数据指针
     * @param  width           宽
     * @param  height          高
     * @param  stride          边缘长度，方便扩充使用
     * @param  format          格式
     * @return std::shared_ptr<Image> 指向数据的共享指针
     */
    static std::shared_ptr<Image> Create(uint8_t *data, int32_t width, int32_t height, int32_t stride, PixelFormat format);
    /**
     * @brief 更新时间，只有比它的时间更大才能更新，保证实时性
     * @param  new_time        新的时间
     */
    void UpdateTimeStamp(const struct timeval &new_time);
    /**
     * @brief 深层拷贝数据
     * @details 重新创建内存，并返回器共享指针
     * @return std::shared_ptr<Image>  新的共享指针
     */
    std::shared_ptr<Image> Clone() const;
    /**
     * @brief 深层拷贝数据
     * @details 将数据拷贝至目标对象
     * @param  copyTo           目标对象指针
     * @return Error            错误信息
     */
    Error CopyData(const std::shared_ptr<Image> &copyTo) const;
    /**
     * @brief  对于相同大小的数据，直接进行拷贝
     * @details 无需进行内存扩充
     * @param  copyTo           目标对象指针
     * @return Error            错误信息
     */
    Error CopyDataOrClone(std::shared_ptr<Image> &copyTo) const;
    /**
     * @brief   数据拷贝
     * @details 严格检查数据类型，方便直接进行拷贝
     * @param  copyTo           目标对象指针
     * @return Error            错误信息
     */
    Error CopyDataStrict(std::shared_ptr<Image> &copyTo) const;
    /**
     * @brief  拷贝数据至目标指针
     * @details 快速拷贝，不做任何检查，直接全部拷贝;谨慎使用
     * @param  dst_buffer       目标数据指针
     * @param  buffer_size      缓冲区大小
     * @return Error            错误信息
     */
    Error CopyDataFast(uint8_t *dst_buffer, uint32_t buffer_size = 0) const;
    // Image properties
    /**
     * @brief   图像宽度
     * @return int32_t 图像宽度
     */
    int32_t Width() const { return mWidth; }
    /**
     * @brief   图像高度
     * @return int32_t 图像高度
     */
    int32_t Height() const { return mHeight; }
    /**
     * @brief   图像步长
     * @return int32_t 图像步长
     */
    int32_t Stride() const { return mStride; }
    /**
     * @brief 图像数据大小
     * @return int32_t
     */
    int32_t Size() const { return mSize; }
    /**
     * @brief 对应时间戳
     * @return struct timeval 时间戳
     */
    struct timeval TimeStamp() const { return mTimeStamp; }
    /**
     * @brief 图像
     * @return PixelFormat 图像格式
     */
    PixelFormat Format() const { return mFormat; }
    /**
     * @brief  原始数据指针
     * @return uint8_t* 数据指针
     */
    uint8_t *Data() const { return mData; }

private:
    /* data */
    int32_t mWidth;            ///< 图像宽度
    int32_t mHeight;           ///< 图像高度
    int32_t mStride;           ///< 记录每个像素点的大小
    int32_t mSize;             ///< 记录数据块的总大小以字节为单位
    PixelFormat mFormat;       ///< 图像格式
    bool mOwnMemory;           ///< 是否自己进行内存管理
    struct timeval mTimeStamp; ///< 记录图片的时间戳;后期可以换掉
    uint8_t *mData;            ///< 原始数据指针
};

/**
 * @brief 独立函数，将图片写入文件
 */
inline void WriteImageToFile(const std::shared_ptr<const Image> &srcImage)
{
    std::string file_name = std::to_string((int)srcImage->TimeStamp().tv_sec) + std::to_string((int)srcImage->TimeStamp().tv_usec);
    WriteFile(file_name + ".jpg", srcImage->Data(), srcImage->Size());
}
NAMESPACE_END

#endif // IMAGE_H
