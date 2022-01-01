/**
 * @file img_tools.h
 * @brief 图像处理的基本类和工具函数
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2022-01-02 00:40:48
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
 *    <td> 2022-01-02 00:40:48 </td>
 *    <td> 1.0 </td>
 *    <td> wangpengcheng </td>
 *    <td> 添加文档注释 </td>
 * </tr>
 * </table>
 */

#ifndef IMG_TOOLS_H
#define IMG_TOOLS_H
#include <memory>

#include "base_error.h"
#include "uncopyable.h"

/**
 * @brief 定义支持的图片格式，方便确定数据长度和单位
 * @details
 * - [图像数据格式](https://blog.csdn.net/airk000/article/details/23875759?utm_source=blogxgwz8#commentBox)
 */
enum class PixelFormat
{
    Unknown = 0, ///< 不支持格式
    Grayscale8,  ///< 灰度
    RGB24,       ///< RGB
    RGBA32,      ///< RGBA
    JPEG,        ///< JPEG
    // Enough for this project
};

/**
 * @brief 定义基本的RGB模式index
 */
enum
{
    RedIndex = 0,   ///< R
    GreenIndex = 1, ///< G
    BlueIndex = 2   ///< B
};

/**
 * @brief 定义ARGB色彩空间类型
 * @details 使用union 提高性能
 */
typedef union
{
    uint32_t argb; ///<   argb
    /**
     * @brief bgra
     */
    struct
    {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;
    } components;
} Argb;

/**
 * @brief BT709灰度系数：（0.2125，0.7154，0.0721）预乘以0x10000，因此可以进行整数灰度缩放。转换灰度图像
 * @details
 * - [BT709灰度系数](https://blog.csdn.net/SoaringLee_fighting/article/details/88790371?utm_source=distribute.pc_relevant.none-task)
 **/
#define GRAY_COEF_RED (0x3666)
#define GRAY_COEF_GREEN (0xB724)
#define GRAY_COEF_BLUE (0x1276)

/**
 * @brief rgb 转换为 gray
 */
#define RGB_TO_GRAY(r, g, b) \ 
       ((uint32_t)(GRAY_COEF_RED * (r) + GRAY_COEF_GREEN * (g) + GRAY_COEF_BLUE * (b)) >> 16)

/* 数据格式工具函数 */

/**
 * @brief 记录各种数据格式需要对应的每个数据的长度
 * @param  format           格式
 * @return uint32_t         对应数据长度
 */
uint32_t ImageBitsPerPixel(PixelFormat format);

/**
 * @brief R当每行的位数已知时，返回每个stride的字节数（stride总是32位对齐）
 * @param  bitsPerLine      每行的bit大小
 * @return uint32_t         数据长度
 */
uint32_t ImageBytesPerStride(uint32_t bitsPerLine);

/**
 * @brief  当每行的位数已知时，返回每行的字节数（行总是8位对齐）
 * @param  bitsPerLine     行数据大小
 * @return uint32_t 对应字节数
 */
uint32_t ImageBytesPerLine(uint32_t bitsPerLine);

#endif // IMG_TOOLS_H