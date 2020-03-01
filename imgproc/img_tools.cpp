#include "img_tools.h"


// 记录各种数据格式需要对应的每个数据的长度
uint32_t ImageBitsPerPixel( PixelFormat format )
{
    static int sizes[]     = { 0, 8, 24, 32, 8 };
    int        formatIndex = static_cast<int>( format );
    //检查越界并输出
    return ( formatIndex >= ( sizeof( sizes ) / sizeof( sizes[0] ) ) ) ? 0 : sizes[formatIndex];
};

// R当每行的位数已知时，返回每个stride的字节数（stride总是32位对齐）
uint32_t ImageBytesPerStride( uint32_t bitsPerLine )
{
    return ( ( bitsPerLine + 31 ) & ~31 ) >> 3;
};

//当每行的位数已知时，返回每行的字节数（行总是8位对齐）
uint32_t ImageBytesPerLine( uint32_t bitsPerLine )
{
    return ( bitsPerLine + 7 ) >> 3;
};