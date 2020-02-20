#include <ctime>
#include "video_frame_decorator.h"
#include "image_drawer.h"

NAMESPACE_START

using namespace std;

VideoFrameDecorator::VideoFrameDecorator( ) :
    cameraTitle( ),
    addTimestampOverlay( false ),
    addCameraTitleOverlay( false ),
    overlayTextColor( { 0xFF000000 } ),
    overlayBackgroundColor( { 0xFFFFFFFF } )
{

}

// 处理image图片对象
void VideoFrameDecorator::OnNewImage( const shared_ptr<const Image>& image )
{
    string  overlay;

    if ( addTimestampOverlay )
    {
        std::time_t time = std::time( 0 );
        std::tm*    now = std::localtime( &time );
        char        buffer[32];

        sprintf( buffer, "%02d/%02d/%02d %02d:%02d:%02d", now->tm_year - 100, now->tm_mon + 1, now->tm_mday,
                                                          now->tm_hour, now->tm_min, now->tm_sec );

        overlay = buffer;
    }

    if ( ( addCameraTitleOverlay ) && ( !cameraTitle.empty( ) ) )
    {
        if ( !overlay.empty( ) )
        {
            overlay += " :: ";
        }

        overlay += cameraTitle;
    }

    if ( !overlay.empty( ) )
    {
        ImageDrawer::PutText( image, overlay, 0, 0, overlayTextColor, overlayBackgroundColor );
    }
}

// Get/Set camera title
string VideoFrameDecorator::CameraTitle( ) const
{
    return cameraTitle;
}
void VideoFrameDecorator::SetCameraTitle( const string& title )
{
    cameraTitle = title;
}

// Get/Set if timestamp should be overlayed on camera images
bool VideoFrameDecorator::TimestampOverlay( ) const
{
    return addTimestampOverlay;
}
void VideoFrameDecorator::SetTimestampOverlay( bool enabled )
{
    addTimestampOverlay = enabled;
}

// Get/Set if camera's title should be overlayed on its images
bool VideoFrameDecorator::CameraTitleOverlay( ) const
{
    return addCameraTitleOverlay;
}
void VideoFrameDecorator::SetCameraTitleOverlay( bool enabled )
{
    addCameraTitleOverlay = enabled;
}

// Get/Set overlay text color
Argb VideoFrameDecorator::OverlayTextColor( ) const
{
    return overlayTextColor;
}
void VideoFrameDecorator::SetOverlayTextColor( Argb color )
{
    overlayTextColor = color;
}

// Get/Set overlay background color
Argb VideoFrameDecorator::OverlayBackgroundColor( ) const
{
    return overlayBackgroundColor;
}
void VideoFrameDecorator::SetOverlayBackgroundColor( Argb color )
{
    overlayBackgroundColor = color;
}

NAMESPACE_END
