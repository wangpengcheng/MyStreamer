/**
 * @file image_drawer.h
 * @brief  image 处理相关类
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2022-01-02 00:55:36
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
 *    <td> 2022-01-02 00:55:36 </td>
 *    <td> 1.0 </td>
 *    <td> wangpengcheng </td>
 *    <td> 增加文档注释 </td>
 * </tr>
 * </table>
 */

#ifndef IMAGE_DREWER_H
#define IMAGE_DREWER_H

#include "image.h"

NAMESPACE_START

/**
 * @brief image处理，静态类，方便快速进行处理;实现字符ASCII的快速转换和填充；base64图片字符编码
 */
class ImageDrawer
{
public:
    ImageDrawer() = delete;

public:
    /**
     * @brief Draw H line on the specified image
     * @param  image            原始图像
     * @param  x1               起始点水平坐标
     * @param  x2               结束点水平坐标
     * @param  y                高度
     * @param  color            颜色
     * @return Error            错误信息
     */
    static Error HLine(const std::shared_ptr<const Image> &image, int32_t x1, int32_t x2, int32_t y, Argb color);

    /**
     * @brief Draw vertical line on the specified image
     * @param  image            原始图像
     * @param  y1               起始点垂直坐标
     * @param  y2               结束点垂直坐标
     * @param  x                水平坐标
     * @param  color            颜色
     * @return Error            错误信息
     */
    static Error VLine(const std::shared_ptr<const Image> &image, int32_t y1, int32_t y2, int32_t x, Argb color);

    /**
     * @brief  Draw rectangle on the specified image with the specfied color (all coordinates are inclusive)
     * @param  image            原始图像
     * @param  x1               起始点水平坐标
     * @param  y1               起始点垂直坐标
     * @param  x2               结束点水平坐标
     * @param  y2               结束点垂直坐标
     * @param  color            颜色
     * @return Error            错误信息
     */
    static Error Rectangle(const std::shared_ptr<const Image> &image, int32_t x1, int32_t y1, int32_t x2, int32_t y2, Argb color);

    /**
     * @brief  Draw ASCII text on the image at the specified location
     * @param  image            原始图像
     * @param  text             目标字符串
     * @param  x                水平坐标
     * @param  y                垂直坐标
     * @param  color            颜色
     * @param  background       背景
     * @param  addBorder        是否增加边框
     * @return Error            错误信息
     */
    static Error PutText(const std::shared_ptr<const Image> &image, const std::string &text, int32_t x, int32_t y, Argb color, Argb background, bool addBorder = true);
};

NAMESPACE_END

#endif // IMAGE_DREWER_H
