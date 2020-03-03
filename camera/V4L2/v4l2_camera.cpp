#include "v4l2_camera.h"

NAMESPACE_START
const std::shared_ptr<V4L2Camera> V4L2Camera::Create( )
{
    return std::shared_ptr<V4L2Camera>( new V4L2Camera);
}

V4L2Camera::V4L2Camera( ):
    mData( new V4L2CameraData( ) )
{
}

V4L2Camera::~V4L2Camera( )
{
    SignalToStop();
    WaitForStop();
    delete mData;
}

// Start the video source
bool V4L2Camera::Start( )
{
    return mData->Start( );
}

// Signal video source to stop
void V4L2Camera::SignalToStop( )
{
    mData->SignalToStop( );
}

// Wait till video source stops
void V4L2Camera::WaitForStop( )
{
    mData->WaitForStop( );
}

// Check if video source is still running
bool V4L2Camera::IsRunning( )
{
    return mData->IsRunning( );
}

// Get number of frames received since the start of the video source
uint32_t V4L2Camera::FramesReceived( )
{
    return mData->FramesReceived;
}

// Set video source listener
VideoSourceListenerInterface* V4L2Camera::SetListener( VideoSourceListenerInterface* listener )
{
    return mData->SetListener( listener );
}

// Set/get video device
uint32_t V4L2Camera::VideoDeviceIndex( ) const
{
    return mData->VideoDeviceIndex;
}
void V4L2Camera::SetVideoDeviceIndex( uint32_t videoDeviceIndex )
{
    mData->SetVideoDeviceIndex( videoDeviceIndex );
}
std::string V4L2Camera::GetVideoDeviceName( ) const
{
    return mData->VideoDeviceName;
}
void V4L2Camera::SetVideoDeviceName( std::string videoDeviceName )
{
    mData->SetVideoDeviceName( videoDeviceName );
}

// Get/Set video size
uint32_t V4L2Camera::Width( ) const
{
    return mData->FrameWidth;
}
uint32_t V4L2Camera::Height( ) const
{
    return mData->FrameHeight;
}
void V4L2Camera::SetVideoSize( uint32_t width, uint32_t height )
{
    mData->SetVideoSize( width, height );
}

// Get/Set frame rate
uint32_t V4L2Camera::FrameRate( ) const
{
    return mData->FrameRate;
}
void V4L2Camera::SetFrameRate( uint32_t frameRate )
{
    mData->SetFrameRate( frameRate );
}

// Enable/Disable JPEG encoding
bool V4L2Camera::IsJpegEncodingEnabled( ) const
{
    return mData->JpegEncoding;
}
void V4L2Camera::EnableJpegEncoding( bool enable )
{
    mData->EnableJpegEncoding( enable );
}

// Set the specified video property
Error V4L2Camera::SetVideoProperty( VideoProperty property, int32_t value )
{
    return mData->SetVideoProperty( property, value );
}

// Get current value if the specified video property
Error V4L2Camera::GetVideoProperty( VideoProperty property, int32_t* value ) const
{
    return mData->GetVideoProperty( property, value );
}

// Get range of values supported by the specified video property
Error V4L2Camera::GetVideoPropertyRange( VideoProperty property, int32_t* min, int32_t* max, int32_t* step, int32_t* def ) const
{
    return mData->GetVideoPropertyRange( property, min, max, step, def );
}

NAMESPACE_END