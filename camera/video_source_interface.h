
/*
视频源，统一抽象接口类

*/
#ifndef VIDEO_SOURCE_INTERFACE_H
#define VIDEO_SOURCE_INTERFACE_H

#include "video_source_listener_interface.h"
#include "base_tool.h"

NAMESPACE_START

class VideoSourceInterface
{
public:
    virtual ~VideoSourceInterface( ) { }

    // 开始获取视频，并初始化缓冲诊队列
    virtual bool Start( ) = 0;
    // 发送停止信号
    virtual void SignalToStop( ) = 0;
    // 等待所有线程停止
    virtual void WaitForStop( ) = 0;
    // 检查视频源是否正常运行
    virtual bool IsRunning( ) = 0;

    // 检查获取到的视频帧数目
    virtual uint32_t FramesReceived( ) = 0;

    // 设置视频监听
    virtual VideoSourceListenerInterface* SetListener( VideoSourceListenerInterface* listener ) = 0;
};

NAMESPACE_END


#endif //VIDEO_SOURCE_INTERFACE_H