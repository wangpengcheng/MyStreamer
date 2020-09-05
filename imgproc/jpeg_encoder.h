/*
* jpeg格式转化类，主要将rgb图像；进行jpeg编码
*/

#ifndef JPEG_ENCODER_H
#define JPEG_ENCODER_H
#include <stdio.h>
#include <jpeglib.h>
#include "uncopyable.h"
#include "image.h"
#include "base_error.h"
#include "img_tools.h"

NAMESPACE_START

/**
 * 定义对libjpeg函数的统一封装类
 * 主要用于
 * https://www.iteye.com/blog/canlynet-1433259
 * https://blog.csdn.net/qq_34888036/article/details/81058600
*/
class JpegEncoderData
{
public:
    JpegEncoderData( uint16_t quality, bool fasterCompression);

    ~JpegEncoderData();
    Error EncodeToMemory( const std::shared_ptr<const Image>& image, uint8_t** buffer, uint32_t* bufferSize );
public:
    uint16_t                    Quality;                    /* 图片质量参数 */
    bool                        FasterCompression;          /* 是否使用快速压缩 */
private:
    struct jpeg_compress_struct cinfo;                      /* jpeg压缩信息结构体 */
    struct jpeg_error_mgr       jerr;                       /* 错误信息 */
};
/**
 * 定义jpegencoder对外接口
 */

class JpegEncoder : private Uncopyable
{
public:
    JpegEncoder( uint16_t quality = 85, bool fasterCompression = false );
    ~JpegEncoder( );

    // Set/get compression quality, [0, 100]
    uint16_t Quality( ) const;
    void SetQuality( uint16_t quality );

    // Set/get faster compression (but less accurate) flag
    bool FasterCompression( ) const;
    void SetFasterCompression( bool faster );

    /* Compress the specified image into provided buffer

       On input, buffer size must be set to the size of provided buffer.
       On output, it is set to the size of encoded JPEG image. If provided
       buffer is too small, it will be re-allocated (realloc).
    */
    Error EncodeToMemory( const std::shared_ptr<const Image>& image, uint8_t** buffer, uint32_t* bufferSize );

private:
    JpegEncoderData* mData;
};

NAMESPACE_END


#endif