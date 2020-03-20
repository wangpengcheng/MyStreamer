#include "video_listener.h"
#include "video_source_to_webdata.h"

#include <string.h>
#include <mutex>
#include <chrono>
#include <iostream>

using namespace MY_NAME_SPACE;
VideoListener::VideoListener(VideoSourceToWebData* owner):owner_(owner)
{

}
VideoListener::~VideoListener()
{

}
/* 将图片写入owner_ */
void VideoListener::OnNewImage( const std::shared_ptr<const Image>& image )
{
    std::lock_guard<std::mutex> lock( owner_->ImageGuard );
    /* 将数据拷贝过来 */
    owner_->InternalError = image->CopyDataOrClone( owner_->CameraImage );
    if ( owner_->InternalError == Error::Success )
    {
        owner_->NewImageAvailable = true;
    }

    // since we got an image from video source, clear any error reported by it
    owner_->VideoSourceErrorMessage.clear( );
    owner_->VideoSourceError = false;
}
// An error coming from video source
void VideoListener::OnError( const string& errorMessage, bool /* fatal */ )
{
    std::lock_guard<std::mutex> imageLock( owner_->ImageGuard );

    owner_->VideoSourceErrorMessage = errorMessage;
    owner_->VideoSourceError = true;
}