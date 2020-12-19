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
class VideoSourceToWeb : private Uncopyable
{
public:
    VideoSourceToWeb( uint16_t jpegQuality = 85 );
    ~VideoSourceToWeb( );
    
    // Get video source listener, which could be fed to some video source
    VideoSourceListenerInterface* VideoSourceListener( ) const;

    // 创建image句柄
    std::shared_ptr<WebRequestHandlerInterface> CreateJpegHandler( const std::string& uri ) const;

    /** 创建控制句柄 */ 
    std::shared_ptr<WebRequestHandlerInterface> CreateMjpegHandler( const std::string& uri, uint32_t frameRate ) const;

    /** 设置图片的压缩质量 */
    uint16_t JpegQuality( ) const;
    void SetJpegQuality( uint16_t quality );

private:
    VideoSourceToWebData* mData;            /** 视频转向web的关键数据结构 */
};

NAMESPACE_END




#endif