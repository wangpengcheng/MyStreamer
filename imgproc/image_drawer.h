
/*
image处理，静态类，方便快速进行处理;实现字符ASCII的快速转换和填充；base64图片字符编码
*/

#ifndef IMAGE_DREWER_H
#define IMAGE_DREWER_H

#include "image.h"

NAMESPACE_START

class ImageDrawer
{
public:
    ImageDrawer()=delete;

public:
    static Error HLine( const std::shared_ptr<const Image>& image, int32_t x1, int32_t x2, int32_t y, Argb color );

    // Draw vertical line on the specified image
    static Error VLine( const std::shared_ptr<const Image>& image, int32_t y1, int32_t y2, int32_t x, Argb color );

    // Draw rectangle on the specified image with the specfied color (all coordinates are inclusive)
    static Error Rectangle( const std::shared_ptr<const Image>& image, int32_t x1, int32_t y1, int32_t x2, int32_t y2, Argb color );

    // Draw ASCII text on the image at the specified location
    static Error PutText( const std::shared_ptr<const Image>& image, const std::string& text, int32_t x, int32_t y, Argb color, Argb background, bool addBorder = true );

};


NAMESPACE_END

#endif // IMAGE_DREWER_H

