#ifndef VIDEO_LISTENER_H
#define VIDEO_LISTENER_H
#include "video_source_listener_interface.h"

NAMESPACE_START
class VideoSourceToWebData;
/* 继承监听接口,方便执行函数获取img数据 */
class VideoListener:public VideoSourceListenerInterface
{
public:

    VideoListener(VideoSourceToWebData* owner);
    ~VideoListener();
    /* 重载接收函数 */
    void OnNewImage( const std::shared_ptr<const Image>& image );
    /* 重载错误信息 */
    void OnError( const std::string& errorMessage, bool fatal );
private:
    VideoSourceToWebData* owner_;           /* 数据转换的关键数据结构 */
};

NAMESPACE_END
#endif