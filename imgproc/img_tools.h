/*

图像处理的基本类和工具函数

*/
#ifndef IMG_TOOLS_H
#define IMG_TOOLS_H
#include <memory>

#include "base_error.h"
#include "uncopyable.h"

NAMESPACE_START

/* 
 * 定义支持的图片格式，方便确定数据长度和单位 
 * https://blog.csdn.net/airk000/article/details/23875759?utm_source=blogxgwz8#commentBox
 * */
enum class PixelFormat
{
    Unknown = 0,            //不支持格式
    Grayscale8,             //灰度
    RGB24,                  //RGB
    RGBA32,                 //RGBA

    JPEG,                   //JPEG
    // Enough for this project
};

/* 定义基本的RGB模式index */
enum
{
    RedIndex   = 0,
    GreenIndex = 1,
    BlueIndex  = 2
};
/*
 * 定义ARGB色彩空间类型
 **/
typedef union
{
    uint32_t argb;
    //bgra
    struct
    {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;
    }components;
}Argb;
/*
 * BT709灰度系数：（0.2125，0.7154，0.0721）预乘以0x10000，因此可以进行整数灰度缩放。转换灰度图像
 * https://blog.csdn.net/SoaringLee_fighting/article/details/88790371?utm_source=distribute.pc_relevant.none-task
**/

#define GRAY_COEF_RED   (0x3666)
#define GRAY_COEF_GREEN (0xB724)
#define GRAY_COEF_BLUE  (0x1276)

/* rgb to gray */
#define RGB_TO_GRAY(r, g, b) ((uint32_t) ( GRAY_COEF_RED * (r) + GRAY_COEF_GREEN * (g) + GRAY_COEF_BLUE * (b) ) >> 16 )

/* 数据格式工具函数 */

// 记录各种数据格式需要对应的每个数据的长度
inline uint32_t ImageBitsPerPixel( PixelFormat format )
{
    static int sizes[]     = { 0, 8, 24, 32, 8 };
    int        formatIndex = static_cast<int>( format );
    //检查越界并输出
    return ( formatIndex >= ( sizeof( sizes ) / sizeof( sizes[0] ) ) ) ? 0 : sizes[formatIndex];
}

// R当每行的位数已知时，返回每个stride的字节数（stride总是32位对齐）
inline uint32_t ImageBytesPerStride( uint32_t bitsPerLine )
{
    return ( ( bitsPerLine + 31 ) & ~31 ) >> 3;
}

//当每行的位数已知时，返回每行的字节数（行总是8位对齐）
inline uint32_t ImageBytesPerLine( uint32_t bitsPerLine )
{
    return ( bitsPerLine + 7 ) >> 3;
}


NAMESPACE_END

#endif //IMG_TOOLS_H