/**
 * @file video_source_to_web.h
 * @brief VideoToWeb类，基本处理工具的转换 \n VideoSourceToWebData 的简单适配封装
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2020-12-19 15:00:52
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
 *    <td> 2020-12-19 15:00:52 </td>
 *    <td> 1.0 </td>
 *    <td> wangpengcheng </td>
 *    <td> 文档注释 </td>
 * </tr>
 * </table>
 */

#ifndef VIDEO_SOURCE_TO_WEB_H
#define VIDEO_SOURCE_TO_WEB_H
#include "uncopyable.h"
#include "video_source_listener_interface.h"

NAMESPACE_START

class VideoSourceToWebData;
class WebRequestHandlerInterface;
/**
 * @brief 数据连接转换类
 */
class VideoSourceToWeb : private Uncopyable
{
public:
    VideoSourceToWeb(uint16_t jpegQuality = 85);
    ~VideoSourceToWeb();

    // Get video source listener, which could be fed to some video source
    VideoSourceListenerInterface *VideoSourceListener() const;

    /**
     * @brief 创建image句柄
     * @param  uri              句柄对应url
     * @return std::shared_ptr<WebRequestHandlerInterface> 处理句柄函数对象
     */
    std::shared_ptr<WebRequestHandlerInterface> CreateJpegHandler(const std::string &uri) const;

    /**
     * @brief 创建MJPEG控制句柄
     * @param  uri              句柄对应url
     * @param  frameRate        帧率
     * @return std::shared_ptr<WebRequestHandlerInterface> 处理句柄函数对象
     */
    std::shared_ptr<WebRequestHandlerInterface> CreateMjpegHandler(const std::string &uri, uint32_t frameRate) const;

    /**
     * @brief  获取JPEG编码质量
     * @return uint16_t  质量参数
     */
    uint16_t JpegQuality() const;
    /**
     * @brief 设置图片的压缩质量
     * @param  quality          目标质量
     */
    void SetJpegQuality(uint16_t quality);

private:
    VideoSourceToWebData *mData; ///< 视频转向web的关键数据结构
};

NAMESPACE_END

#endif