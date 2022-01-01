#include "jpeg_encoder.h"

NAMESPACE_START

/**
 * jpeg错误信息类
 */
class JpegException : public std::exception
{
public:
    virtual const char *what() const throw()
    {
        return "JPEG coding failure";
    }
};
static void my_error_exit(j_common_ptr /* cinfo */)
{
    throw JpegException();
}

static void my_output_message(j_common_ptr /* cinfo */)
{
    // do nothing - kill the message
}

JpegEncoderData::JpegEncoderData(uint16_t quality, bool fasterCompression) : Quality(quality), FasterCompression(fasterCompression)
{
    if (Quality > 100)
    {
        Quality = 100;
    }

    //初始化jpeg结构体信息
    cinfo.err = jpeg_std_error(&jerr);
    jerr.error_exit = my_error_exit;
    jerr.output_message = my_output_message;
    /*  创建压缩信息 */
    jpeg_create_compress(&cinfo);
}
JpegEncoderData::~JpegEncoderData()
{
    jpeg_destroy_compress(&cinfo);
}

/* 关键压缩函数 */
Error JpegEncoderData::EncodeToMemory(
    const std::shared_ptr<const Image> &image, /* 源图片地址 */
    uint8_t **buffer,                          /* 压缩后的目标地址 */
    uint32_t *bufferSize                       /* buffer的长度 */
)
{
    /* 编码的行指针 */
    JSAMPROW row_pointer[1];
    Error ret = Error::Success;
    /* 检查数据 */
    if ((!image) || (image->Data() == nullptr) || (buffer == nullptr) || (*buffer == nullptr) || (bufferSize == nullptr))
    {
        ret = Error::NullPointer;
    }
    else if ((image->Format() != PixelFormat::RGB24) && (image->Format() != PixelFormat::Grayscale8))
    {
        ret = Error::UnsupportedPixelFormat;
    }
    else
    {
        try
        {
            //获取buffer的长度
            unsigned long mem_buffer_size = *bufferSize;
            /* 分配内存，将cinfo中的指针指向buffer */
            jpeg_mem_dest(&cinfo, buffer, &mem_buffer_size);

            // 获取压缩的参数信息
            cinfo.image_width = image->Width();
            cinfo.image_height = image->Height();

            if (image->Format() == PixelFormat::RGB24)
            {
                cinfo.input_components = 3;
                cinfo.in_color_space = JCS_RGB;
            }
            else
            {
                cinfo.input_components = 1;
                cinfo.in_color_space = JCS_GRAYSCALE;
            }

            //设置默认的压缩参数
            jpeg_set_defaults(&cinfo);
            // 设置压缩质量
            jpeg_set_quality(&cinfo, (int)Quality, TRUE /* limit to baseline-JPEG values */);

            // 是否使用快速压缩算法
            cinfo.dct_method = (FasterCompression) ? JDCT_FASTEST : JDCT_DEFAULT;

            //开始压缩
            jpeg_start_compress(&cinfo, TRUE);

            // 开始进行压缩
            while (cinfo.next_scanline < cinfo.image_height)
            {
                /* 获取偏移指针 */
                row_pointer[0] = image->Data() + image->Stride() * cinfo.next_scanline;
                /* 写入压缩数据 */
                jpeg_write_scanlines(&cinfo, row_pointer, 1);
            }

            // 完成压缩，添加尾部数据
            jpeg_finish_compress(&cinfo);

            *bufferSize = (uint32_t)mem_buffer_size;
        }
        catch (const JpegException &)
        {
            ret = Error::FailedImageEncoding;
        }
    }

    return ret;
}
/**
 * jpegdecoder实现
 */
JpegEncoder::JpegEncoder(uint16_t quality, bool fasterCompression) : mData(new JpegEncoderData(quality, fasterCompression))
{
}
JpegEncoder::~JpegEncoder()
{
    delete mData;
}
uint16_t JpegEncoder::Quality() const
{
    return mData->Quality;
}
void JpegEncoder::SetQuality(uint16_t quality)
{
    mData->Quality = quality;
    if (mData->Quality > 100)
        mData->Quality = 100;
    if (mData->Quality < 1)
        mData->Quality = 1;
}

// Set/get faster compression (but less accurate) flag
bool JpegEncoder::FasterCompression() const
{
    return mData->FasterCompression;
}
void JpegEncoder::SetFasterCompression(bool faster)
{
    mData->FasterCompression = faster;
}

// Compress the specified image into provided buffer
Error JpegEncoder::EncodeToMemory(const std::shared_ptr<const Image> &image, uint8_t **buffer, uint32_t *bufferSize)
{
    return mData->EncodeToMemory(image, buffer, bufferSize);
}

NAMESPACE_END