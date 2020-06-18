#include "v4l2_camera_data.h"
#include <iostream>
NAMESPACE_START

using namespace std;
/* 定义属性参数 */
static const uint32_t nativeVideoProperties[] =
{
    V4L2_CID_BRIGHTNESS,
    V4L2_CID_CONTRAST,
    V4L2_CID_SATURATION,
    V4L2_CID_HUE,
    V4L2_CID_SHARPNESS,
    V4L2_CID_GAIN,
    V4L2_CID_BACKLIGHT_COMPENSATION,
    V4L2_CID_RED_BALANCE,
    V4L2_CID_BLUE_BALANCE,
    V4L2_CID_AUTO_WHITE_BALANCE,
    V4L2_CID_HFLIP,
    V4L2_CID_VFLIP
};


// 初始化并且开始工作
bool V4L2CameraData::Start( )
{
    lock_guard<recursive_mutex> lock( Sync );

    if ( !IsRunning( ) )
    {
        NeedToStop.Reset( );
        Running = true;
        FramesReceived = 0;
        /* 在线程中创建，是的device与主进程分离 */
        ControlThread = thread( ControlThreadHanlder, this );
    }

    return true;
}

// 发射停止信号
void V4L2CameraData::SignalToStop( )
{
    lock_guard<recursive_mutex> lock( Sync );

    if ( IsRunning( ) )
    {
        NeedToStop.Signal( );
    }
}

// 等待线程结束并退出
void V4L2CameraData::WaitForStop( )
{
    SignalToStop( );

    if ( ( IsRunning( ) ) || ( ControlThread.joinable( ) ) )
    {
        ControlThread.join( );
    }
}

// 返回线程是否正在工作
bool V4L2CameraData::IsRunning( )
{
    lock_guard<recursive_mutex> lock( Sync );
    
    if ( ( !Running ) && ( ControlThread.joinable( ) ) )
    {
        ControlThread.join( );
    }
    
    return Running;
};

// Set video source listener
VideoSourceListenerInterface* V4L2CameraData::SetListener( VideoSourceListenerInterface* listener )
{
    lock_guard<recursive_mutex> lock( Sync );
    VideoSourceListenerInterface* oldListener = listener;

    Listener = listener;

    return oldListener;
};

// 将一张新图片放入监听结果中
void V4L2CameraData::NotifyNewImage( const std::shared_ptr<const Image>& image )
{
    VideoSourceListenerInterface* myListener;
    
    {
        lock_guard<recursive_mutex> lock( Sync );
        myListener = Listener;
    }
    
    if ( myListener != nullptr )
    {
        myListener->OnNewImage( image );
    }
}

// 发射错误信息
void V4L2CameraData::NotifyError( const string& errorMessage, bool fatal )
{
    VideoSourceListenerInterface* myListener;
    
    {
        lock_guard<recursive_mutex> lock( Sync );
        myListener = Listener;
    }
    
    if ( myListener != nullptr )
    {
        myListener->OnError( errorMessage, fatal );
    }
}

// 初始化摄像头
bool V4L2CameraData::Init( )
{
    lock_guard<recursive_mutex> lock( ConfigSync );
    char                        strVideoDevice[64];
    bool                        ret = true;
    int                         ecode;
    /* 设置device编号和名字 */
    if(VideoDeviceName.size()>0){
        if(VideoDeviceName.size()>64){
            NotifyError( "Failed opening video device", true );
            return false;
        }else{
            strcpy(strVideoDevice,VideoDeviceName.c_str());
        }
    }else{
        sprintf( strVideoDevice, "/dev/video%d", VideoDeviceIndex );
    }
    

    // 打开设备
    VideoFd = open( strVideoDevice, O_RDWR );
    if ( VideoFd == -1 )
    {
        NotifyError( "Failed opening video device", true );
        ret = false;
    }
    else
    {
        v4l2_capability videoCapability = { 0 };

        // 查询摄像头能力
        ecode = ioctl( VideoFd, VIDIOC_QUERYCAP, &videoCapability );
        if ( ecode < 0 )
        {
            NotifyError( "Failed getting video capabilities of the device", true );
            ret = false;
        }
        // 是否支持摄像相机
        else if ( ( videoCapability.capabilities & V4L2_CAP_VIDEO_CAPTURE ) == 0 )
        {
            NotifyError( "Device does not support video capture", true );
            ret = false;
        }
        else if ( ( videoCapability.capabilities & V4L2_CAP_STREAMING ) == 0 )
        {
            NotifyError( "Device does not support streaming", true );
            ret = false;
        }
    }

    // 设置视频格式
    if ( ret )
    {
        v4l2_format videoFormat = { 0 },queryFormat={0};
        uint32_t    pixelFormat = ( JpegEncoding ) ? V4L2_PIX_FMT_MJPEG : V4L2_PIX_FMT_YUYV;

        videoFormat.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        videoFormat.fmt.pix.width       = FrameWidth;
        videoFormat.fmt.pix.height      = FrameHeight;
        videoFormat.fmt.pix.pixelformat = pixelFormat;
        videoFormat.fmt.pix.field       = V4L2_FIELD_ANY;
        /* 设置视频格式 */
        ecode = ioctl( VideoFd, VIDIOC_S_FMT, &videoFormat );
        if ( ecode < 0 )
        {
            NotifyError( "Failed setting video format", true );
            ret = false;
        }
        else if ( videoFormat.fmt.pix.pixelformat != pixelFormat )
        {
            NotifyError(string( "The camera does not support requested format: " ) + ( ( JpegEncoding ) ? "MJPEG" : "YUYV" )+("\n")+("Please Use:")+(V4L2pixelFormatToStr(videoFormat.fmt.pix.pixelformat)), true );
            std::string result="";
            V4L2GetSuportFormat(VideoFd,result);
            ret = false;
        }
        FrameWidth  = videoFormat.fmt.pix.width;
        FrameHeight = videoFormat.fmt.pix.height;
        std::cout<<FrameWidth<<";"<<FrameHeight<<std::endl;
    }

    // 初始化 Memory Mapping 
    if ( ret )
    {
        v4l2_requestbuffers requestBuffers = { 0 };

        requestBuffers.count  = BUFFER_COUNT;
        requestBuffers.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        /* 设置内存格式为内存映射 */
        requestBuffers.memory = V4L2_MEMORY_MMAP;
        /* 修改buffer参数 */
        ecode = ioctl( VideoFd, VIDIOC_REQBUFS, &requestBuffers );
        if ( ecode < 0 )
        {
            NotifyError( "Unable to allocate capture buffers", true );
            ret = false;
        }
        else if ( requestBuffers.count < BUFFER_COUNT )
        {
            NotifyError( "Not enough memory to allocate capture buffers", true );
            ret = false;
        }
    }

    // 询问Buffer状态
    if ( ret )
    {
        v4l2_buffer videoBuffer;

        for ( int i = 0; i < BUFFER_COUNT; i++ )
        {
            //将buffer重新设置为0
            memset( &videoBuffer, 0, sizeof( videoBuffer ) );

            videoBuffer.index  = i;
            videoBuffer.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            videoBuffer.memory = V4L2_MEMORY_MMAP;

            ecode = ioctl( VideoFd, VIDIOC_QUERYBUF, &videoBuffer );
            if ( ecode < 0 )
            {
                NotifyError( "Unable to query capture buffer", true );
                ret = false;
                break;
            }
            //将MappedBuffers映射到内核的图片缓冲区域
            MappedBuffers[i]      = (uint8_t*) mmap( 0, videoBuffer.length, PROT_READ, MAP_SHARED, VideoFd, videoBuffer.m.offset );
            //设置缓冲区长度
            MappedBufferLength[i] = videoBuffer.length;

            if ( MappedBuffers[i] == nullptr )
            {
                NotifyError( "Unable to map capture buffer", true );
                ret = false;
                break;
            }
        }
    }

    // 和Driver交换buffer；获取摄像头图像缓冲--将图像写入缓冲区
    if ( ret )
    {
        v4l2_buffer videoBuffer;

        for ( int i = 0; i < BUFFER_COUNT; i++ )
        {
            memset( &videoBuffer, 0, sizeof( videoBuffer ) );
        
            videoBuffer.index  = i;
            videoBuffer.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            videoBuffer.memory = V4L2_MEMORY_MMAP;
        
            ecode = ioctl( VideoFd, VIDIOC_QBUF, &videoBuffer );
        
            if ( ecode < 0 )
            {
                NotifyError( "Unable to enqueue capture buffer", true );
                ret = false;
            }
        }
    }

    // 开始以流发方式，发送数据
    if ( ret )
    {
        int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
        ecode = ioctl( VideoFd, VIDIOC_STREAMON, &type );
        if ( ecode < 0 )
        {
            NotifyError( "Failed starting video streaming", true );
            ret = false;
        }
        else
        {
            VideoStreamingActive = true;
        }
    }

    // 确认所有设置的内能力值
    if ( ret )
    {
        bool configOK = true;

        for ( auto property : PropertiesToSet )
        {
            configOK &= static_cast<bool>( SetVideoProperty( property.first, property.second ) );
        }
        PropertiesToSet.clear( );
    
        if ( !configOK )
        {
            NotifyError( "Failed applying video configuration" );
        }
    }

    return ret;
}

// 停止摄像头并，清除队列
void V4L2CameraData::Cleanup( )
{
    lock_guard<recursive_mutex> lock( ConfigSync );

    // disable vide streaming
    if ( VideoStreamingActive )
    {
        int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        ioctl( VideoFd, VIDIOC_STREAMOFF, &type );
        VideoStreamingActive = false;
    }

    // unmap capture buffers
    for ( int i = 0; i < BUFFER_COUNT; i++ )
    {
        if ( MappedBuffers[i] != nullptr )
        {
            munmap( MappedBuffers[i], MappedBufferLength[i] );
            MappedBuffers[i]      = nullptr;
            MappedBufferLength[i] = 0;
        }
    }

    // close the video device
    if ( VideoFd != -1 )
    {
        close( VideoFd );
        VideoFd = -1;
    }
}
// 开启设备线程，直到收到停止信号
void V4L2CameraData::VideoCaptureLoop( )
{
    v4l2_buffer videoBuffer;
    uint32_t    sleepTime = 1000 / FrameRate;
    uint32_t    frameTime = 1000 / FrameRate;
    uint32_t    handlingTime ;
    int         ecode;

    // If JPEG encoding is used, client is notified with an image wrapping a mapped buffer.
    // If not used howver, we decode YUYV data into RGB.
    shared_ptr<Image> rgbImage;
    //非jpeg编码，就直接进行拷贝。
    if ( !JpegEncoding )
    {
        rgbImage = Image::Allocate( FrameWidth, FrameHeight, PixelFormat::RGB24);

        if ( !rgbImage )
        {
            NotifyError( "Failed allocating an image", true );
            return;
        }
    }

    // 等待一段时间再进行数据的读取
    while ( !NeedToStop.Wait( sleepTime ) )
    {
        std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now( );

        // dequeue buffer
        memset( &videoBuffer, 0, sizeof( videoBuffer ) );

        videoBuffer.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        videoBuffer.memory = V4L2_MEMORY_MMAP;

        ecode = ioctl( VideoFd, VIDIOC_DQBUF, &videoBuffer );
        if ( ecode < 0 )
        {
            NotifyError( "Failed to dequeue capture buffer" );
        }
        else
        {
            shared_ptr<Image> image;

            FramesReceived++;

            if ( JpegEncoding )
            {
                //注意这里创建的时候，指针指向的是v4l2_buffer 结构体,直接使用buffer大小和1计算它的总长度
                image = Image::Create( MappedBuffers[videoBuffer.index], videoBuffer.bytesused, 1, videoBuffer.bytesused, PixelFormat::JPEG );
            }
            else
            {
                DecodeYuyvToRgb( MappedBuffers[videoBuffer.index], rgbImage->Data( ), FrameWidth, FrameHeight, rgbImage->Stride( ) );
                image = rgbImage;
            }
            image->UpdateTimeStamp(videoBuffer.timestamp);
            if ( image )
            {
                //分发全部的image指针,使用
                NotifyNewImage( image );
            }
            else
            {
                NotifyError( "Failed allocating an image" );
            }

            // 再次查询buffer
            ecode = ioctl( VideoFd, VIDIOC_QBUF, &videoBuffer );
            if ( ecode < 0 )
            {
                NotifyError( "Failed to requeue capture buffer" );
            }
        }
        /* 获取总的处理时间 */
        handlingTime = static_cast<uint32_t>( std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now( ) - startTime ).count( ) );
        /* 设置睡眠时间 */
        sleepTime    = ( handlingTime > frameTime ) ? 0 : ( frameTime - handlingTime );
    }
}

// Background control thread - performs camera init/clean-up and runs video loop
void V4L2CameraData::ControlThreadHanlder( V4L2CameraData* me )
{    
    if ( me->Init() )
    {
        me->VideoCaptureLoop();
    }
    me->Cleanup();
    {
        lock_guard<recursive_mutex> lock( me->Sync );
        me->Running = false;
    }
}

// 设置设备编号
void V4L2CameraData::SetVideoDeviceIndex( uint32_t videoDeviceIndex )
{
    lock_guard<recursive_mutex> lock( Sync );

    if ( !IsRunning( ) )
    {
        VideoDeviceIndex = videoDeviceIndex;
    }
}
void V4L2CameraData::SetVideoDeviceName( std::string videoDeviceName )
{
    lock_guard<recursive_mutex> lock( Sync );

    if ( !IsRunning( ) )
    {
        VideoDeviceName = videoDeviceName;
    }
}
// Set size of video frames to request
void V4L2CameraData::SetVideoSize( uint32_t width, uint32_t height )
{
    lock_guard<recursive_mutex> lock( Sync );

    if ( !IsRunning( ) )
    {
        FrameWidth  = width;
        FrameHeight = height;
    }
}

// Set rate to query images at
void V4L2CameraData::SetFrameRate( uint32_t frameRate )
{
    lock_guard<recursive_mutex> lock( Sync );

    if ( !IsRunning( ) )
    {
        FrameRate = frameRate;
    }
}

// 开启jpeg编码
void V4L2CameraData::EnableJpegEncoding( bool enable )
{
    lock_guard<recursive_mutex> lock( Sync );

    if ( !IsRunning( ) )
    {
        JpegEncoding = enable;
    }
}

// 设置属性
Error V4L2CameraData::SetVideoProperty( VideoProperty property, int32_t value )
{
    lock_guard<recursive_mutex> lock( Sync );
    Error                      ret = Error::Success;

    if ( ( property < VideoProperty::Brightness ) || ( property > VideoProperty::Gain ) )
    {
        ret = Error::UnknownProperty;
    }
    else if ( ( !Running ) || ( VideoFd == -1 ) )
    {
        // save property value and try setting it when device gets runnings
        PropertiesToSet[property] = value;
    }
    else
    {
        v4l2_control control;

        control.id    = nativeVideoProperties[static_cast<int>( property )];
        control.value = value;

        if ( ioctl( VideoFd, VIDIOC_S_CTRL, &control ) < 0 )
        {
            ret = Error::Failed;
        }
    }

    return ret;
}

// 获取属性
Error V4L2CameraData::GetVideoProperty( VideoProperty property, int32_t* value ) const
{
    lock_guard<recursive_mutex> lock( Sync );
    Error                      ret = Error::Success;

    if ( value == nullptr )
    {
        ret = Error::NullPointer;
    }
    else if ( ( property < VideoProperty::Brightness ) || ( property > VideoProperty::Gain ) )
    {
        ret = Error::UnknownProperty;
    }
    else if ( ( !Running ) || ( VideoFd == -1 ) )
    {
        ret = Error::DeivceNotReady;
    }
    else
    {
        v4l2_control control;

        control.id = nativeVideoProperties[static_cast<int>( property )];

        if ( ioctl( VideoFd, VIDIOC_G_CTRL, &control ) < 0 )
        {
            ret = Error::Failed;
        }
        else
        {
            *value = control.value;
        }
    }

    return ret;
}

// 设置属性值范围
Error V4L2CameraData::GetVideoPropertyRange( VideoProperty property, int32_t* min, int32_t* max, int32_t* step, int32_t* def ) const
{
    lock_guard<recursive_mutex> lock( Sync );
    Error                      ret = Error::Success;

    if ( ( min == nullptr ) || ( max == nullptr ) || ( step == nullptr ) || ( def == nullptr ) )
    {
        ret = Error::NullPointer;
    }
    else if ( ( property < VideoProperty::Brightness ) || ( property > VideoProperty::Gain ) )
    {
        ret = Error::UnknownProperty;
    }
    else if ( ( !Running ) || ( VideoFd == -1 ) )
    {
        ret = Error::DeivceNotReady;
    }
    else
    {
        v4l2_queryctrl queryControl;

        queryControl.id = nativeVideoProperties[static_cast<int>( property )];

        if ( ioctl( VideoFd, VIDIOC_QUERYCTRL, &queryControl ) < 0 )
        {
            ret = Error::Failed;
        }
        else if ( ( queryControl.flags & V4L2_CTRL_FLAG_DISABLED ) != 0 )
        {
            ret = Error::ConfigurationNotSupported;
        }
        else if ( ( queryControl.type & ( V4L2_CTRL_TYPE_BOOLEAN | V4L2_CTRL_TYPE_INTEGER ) ) != 0 )
        {
            /*
            printf( "property: %d, min: %d, max: %d, step: %d, def: %d, type: %s \n ", static_cast<int>( property ),
                    queryControl.minimum, queryControl.maximum, queryControl.step, queryControl.default_value,
                    ( queryControl.type & V4L2_CTRL_TYPE_BOOLEAN ) ? "bool" : "int" );
            */

            *min  = queryControl.minimum;
            *max  = queryControl.maximum;
            *step = queryControl.step;
            *def  = queryControl.default_value;
        }
        else
        {
            ret = Error::ConfigurationNotSupported;
        }
    }

    return ret;
}

NAMESPACE_END