
#include <string.h>
#include <new>
#include "image.h"

NAMESPACE_START

Image::Image(uint8_t* data, int32_t width, int32_t height, int32_t stride, PixelFormat format,bool ownMemory ) :
    mData( data ), mWidth( width ), mHeight( height ), mStride( stride ), mFormat( format ),mOwnMemory( ownMemory )
{
    mSize=height*stride;
    mTimeStamp.tv_sec=0;
    mTimeStamp.tv_usec=0;  
}

// Destroy image
Image::~Image( )
{
    if ( ( mOwnMemory ) && ( mData != nullptr ) )
    {
        free( mData );
        mData=nullptr;
    }
}

// 根据格式来进行数据分配
std::shared_ptr<Image> Image::Allocate( int32_t width, int32_t height, PixelFormat format, bool zeroInitialize )
{
    int32_t  stride = (int32_t) ImageBytesPerStride( width * ImageBitsPerPixel( format ) );
    Image*  image  = nullptr;
    uint8_t* data   = nullptr;
    /**
     * 假设需要初始化，调用calloc
     * https://blog.csdn.net/weibo1230123/article/details/81503135
     * */
    if ( zeroInitialize ){
        data = (uint8_t*) calloc( 1, height * stride );
    }else{
        data = (uint8_t*) malloc( height * stride );
    }

    if(data!=nullptr )
    {
        image = new (std::nothrow) Image( data, width, height, stride, format, true );
        
    }

    return std::shared_ptr<Image>( image );
}

//直接在已有的内存上直接进程创建
std::shared_ptr<Image> Image::Create( uint8_t* data, int32_t width, int32_t height, int32_t stride, PixelFormat format )
{
    return std::shared_ptr<Image>( new (std::nothrow) Image( data, width, height, stride, format, false ) );
};
void Image::UpdateTimeStamp(const struct timeval& new_time) 
{
    if((new_time.tv_sec>mTimeStamp.tv_sec)||(new_time.tv_usec>mTimeStamp.tv_usec)){
        mTimeStamp.tv_sec=new_time.tv_sec;
        mTimeStamp.tv_usec=new_time.tv_usec;
    }
};
/**
 * 深度拷贝数据，慎重使用
*/
std::shared_ptr<Image> Image::Clone( ) const
{
    std::shared_ptr<Image> clone;

    if ( mData != nullptr )
    {
        clone = Allocate( mWidth, mHeight, mFormat );

        if ( clone )
        {
            if ( CopyData( clone ) != Error::Success )
            {
                clone.reset( );
            }
        }
    }

    return clone;
}

//直接进行数据的拷贝和复制
Error Image::CopyData( const std::shared_ptr<Image>& copyTo ) const
{
    Error ret = Error::Success;

    if ( ( mData == nullptr ) || ( !copyTo ) || ( copyTo->mData == nullptr ) )
    {
        ret = Error::NullPointer;
    }
    //先检查高度是否对应，对于JPEG格式来说，一般只进行行内压缩，因此，只检查高度就可以了
    else if ( ( mHeight != copyTo->mHeight ) || ( mFormat != copyTo->mFormat ) )
    {
        ret = Error::ImageParametersMismatch;
    }
    else if  ( ( ( mFormat != PixelFormat::JPEG ) && ( mWidth != copyTo->mWidth ) ) ||
               ( ( mFormat == PixelFormat::JPEG ) && ( mStride > copyTo->mStride ) ) )
    {
        ret = Error::ImageParametersMismatch;
    }
    else
    {
        //计算每行大小
        uint32_t lineSize  = ImageBytesPerLine( mWidth * ImageBitsPerPixel( mFormat ) );
        uint8_t* srcPtr    = mData;
        uint8_t* dstPtr    = copyTo->mData;
        int32_t  dstStride = copyTo->mStride;
        //对行进行拷贝
        for ( int y = 0; y < mHeight; y++ )
        {
            memcpy( dstPtr, srcPtr, lineSize );
            srcPtr += mStride;
            dstPtr += dstStride;
        }
    }

    return ret;
}

//对于相同大小的数据，直接进行拷贝
Error Image::CopyDataOrClone(std::shared_ptr<Image>& copyTo ) const
{
    Error ret = Error::Success;

    if ( ( !copyTo ) ||
         ( copyTo->Height( ) != mHeight ) ||
         ( copyTo->Format( ) != mFormat ) ||
         ( ( mFormat != PixelFormat::JPEG ) && ( copyTo->Width( ) != mWidth ) ) ||
         ( ( mFormat == PixelFormat::JPEG ) && ( copyTo->Stride( ) < mStride ) )
       )
    {
        copyTo = Clone( );
        if ( !copyTo )
        {
            ret = Error::OutOfMemory;
        }
    }
    else
    {
        ret = CopyData( copyTo );
    }

    return ret;
}

Error Image::CopyDataStrict(std::shared_ptr<Image>& copyTo) const
{
    Error ret = Error::Success;
    if(
        (!copyTo)||
        (copyTo->Format()!=mFormat)||
        (copyTo->Width()!=mWidth)||
        (copyTo->Height()!=mHeight)
    )
    {
            ret = Error::OutOfMemory;
    }else{
        uint32_t totle_size  = mHeight*ImageBytesPerLine( mWidth * ImageBitsPerPixel( mFormat ) );
        uint8_t* srcPtr    = mData;
        uint8_t* dstPtr    = copyTo->mData;
        memcpy(dstPtr,srcPtr,totle_size);
    }
    return ret;
};
Error Image::CopyDataFast( uint8_t* dst_buffer,uint32_t buffer_size) const{
    buffer_size=buffer_size?((buffer_size<mSize)?buffer_size:mSize):mSize;
    memcpy(dst_buffer,mData,buffer_size);
}
NAMESPACE_END  //namespace