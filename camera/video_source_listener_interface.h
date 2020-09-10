/*
    视频数据接受结构体，作为输出的缓冲数据结构
*/
#ifndef VIDEO_SOURCE_LISTENER_INTERFACE_H
#define VIDEO_SOURCE_LISTENER_INTERFACE_H

#include <string>
#include <list>

#include "base_tool.h"
#include "image.h"

NAMESPACE_START

// 接收视频数据接口,主要作为数据的接收缓冲队列

class VideoSourceListenerInterface
{
public:
    virtual ~VideoSourceListenerInterface( ) { }

    // 添加新图片信号
    virtual void OnNewImage( const std::shared_ptr<const Image>& image ) = 0;

    // 视频错误信号
    virtual void OnError( const std::string& errorMessage, bool fatal ) = 0;
};

// 设置监听者链类，执行多个线程的监听
class VideoSourceListenerChain : public VideoSourceListenerInterface
{
public:
    // New video frame notification
    virtual void OnNewImage( const std::shared_ptr<const Image>& image )
    {
        for ( auto listener : chain )
        {
            listener->OnNewImage( image );
        }
    }

    // 视屏错误信号
    virtual void OnError( const std::string& errorMessage, bool fatal )
    {
        for ( auto listener : chain )
        {
            listener->OnError( errorMessage, fatal );
        }
    }
    
    // 添加数据到新的监听队列中
    void Add( VideoSourceListenerInterface* listener )
    {
        if ( listener != nullptr )
        {
            chain.push_back( listener );
        }
    }
    // Clear all listeners from the chain
    void Clear( )
    {
        chain.clear( );
    }
    
private:
    std::list<VideoSourceListenerInterface*> chain;
};

NAMESPACE_END // namespace end


#endif //VIDEO_SOURCE_LISTENER_INTERFACE_H