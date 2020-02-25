
#ifndef IMAGE_H
#define IMAGE_H
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include "uncopyable.h"
#include "img_tools.h"


NAMESPACE_START
/**
 * 定义简单基本图像类，包含图像信息和基本数据
*/
class Image:private Uncopyable
{
private:
    /*构造函数私有化，禁止其在栈上分配内存 */
    /**
     * args
     * data :原始数据指针
     * width,height 宽和高
     * stride 边缘长度，方便扩充使用
     * format：格式
     * ownMemory：是否自己管理内存
    */
    Image(uint8_t* data, int32_t width, int32_t height, int32_t stride, PixelFormat format, bool ownMemory );
public:
    
    ~Image();
    /**
     * 直接内存分配函数
      */
    static std::shared_ptr<Image> Allocate(int32_t width, int32_t height, PixelFormat format, bool zeroInitialize = false);
    /**
     * 直接在已有的数据内存上直接进行数据拷贝和复制
    */
    static std::shared_ptr<Image> Create( uint8_t* data, int32_t width, int32_t height, int32_t stride, PixelFormat format );
    //更新时间，只有比它的时间更大才能更新，保证实时性
    void UpdateTimeStamp(const struct timeval& new_time);
    // 深层拷贝数据
    std::shared_ptr<Image> Clone( ) const; 

    // 深层拷贝数据
    Error CopyData( const std::shared_ptr<Image>& copyTo ) const;
    // 对于相同大小的数据，直接进行拷贝
    Error CopyDataOrClone( std::shared_ptr<Image>& copyTo ) const;
    // 严格检查数据类型，方便直接进行拷贝
    Error CopyDataStrict( std::shared_ptr<Image>& copyTo ) const;
    //快速拷贝，不做任何检查，直接全部拷贝;谨慎使用
    Error CopyDataFast( uint8_t* dst_buffer,uint32_t buffer_size=0 ) const;
    // Image properties
    int32_t Width( )       const { return mWidth;  }
    int32_t Height( )      const { return mHeight; }
    int32_t Stride( )      const { return mStride; }
    int32_t Size()         const { return mSize;}
    struct timeval TimeStamp()    const { return mTimeStamp;}
    PixelFormat Format( ) const { return mFormat; }
    // Raw data of the image
    uint8_t* Data( )       const { return mData;   }
private:
    /* data */        
    int32_t      mWidth;            
    int32_t      mHeight;
    int32_t      mStride;           /* 记录每个像素点的大小 */
    int32_t      mSize;             /* 记录数据块的总大小以字节为单位 */
    PixelFormat  mFormat;
    bool         mOwnMemory;
    struct timeval      mTimeStamp;          /* 记录图片的时间戳;后期可以换掉 */
    uint8_t*     mData;  
};
/* 独立函数，将图片写入文件 */
inline void WriteImageToFile(const std::shared_ptr<const Image>& srcImage)
{
    std::string file_name=std::to_string((int)srcImage->TimeStamp().tv_sec)+std::to_string((int)srcImage->TimeStamp().tv_usec);
    WriteFile(file_name+".jpg",srcImage->Data(),srcImage->Size());
}

NAMESPACE_END

#endif //IMAGE_H