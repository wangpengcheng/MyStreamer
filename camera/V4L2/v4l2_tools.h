
#ifndef V4L2_TOOLS_H
#define V4L2_TOOLS_H


#include <stdint.h>
/*===== Linux header start ======*/
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
/*===== Linux header end ======*/
#include "base_tool.h"
#include "img_tools.h"

NAMESPACE_START

// YUYV转化为RGB函数
inline void DecodeYuyvToRgb( const uint8_t* yuyvPtr, uint8_t* rgbPtr, int32_t width, int32_t height, int32_t rgbStride )
{
    /* 
        The code below does YUYV to RGB conversion using the next coefficients.
        However those are multiplied by 256 to get integer calculations.
     
        r = y + (1.4065 * (cr - 128));
        g = y - (0.3455 * (cb - 128)) - (0.7169 * (cr - 128));
        b = y + (1.7790 * (cb - 128));
    */

    int r, g, b;
    int y, u, v;
    int z = 0;

    for ( int32_t iy = 0; iy < height; iy++ )
    {
        uint8_t* rgbRow = rgbPtr + iy * rgbStride;

        for ( int32_t ix = 0; ix < width; ix++ )
        {
            y = ( ( z == 0 ) ? yuyvPtr[0] : yuyvPtr[2] ) << 8;
            u = yuyvPtr[1] - 128;
            v = yuyvPtr[3] - 128;

            r = ( y + ( 360 * v ) ) >> 8;
            g = ( y - ( 88  * u ) - ( 184 * v ) ) >> 8;
            b = ( y + ( 455 * u ) ) >> 8;

            rgbRow[RedIndex]   = (uint8_t) ( r > 255 ) ? 255 : ( ( r < 0 ) ? 0 : r );
            rgbRow[GreenIndex] = (uint8_t) ( g > 255 ) ? 255 : ( ( g < 0 ) ? 0 : g );
            rgbRow[BlueIndex]  = (uint8_t) ( b > 255 ) ? 255 : ( ( b < 0 ) ? 0 : b );

            if ( z++ )
            {
                z = 0;
                yuyvPtr += 4;
            }

            rgbRow += 3;
        }
    }
};
inline std::string V4L2pixelFormatToStr(__u32 fromat){
        std::string result="";
        int count=4;
        while(count){
            char temp=(char)(fromat&0xFF);
            result+=temp;
            fromat>>=8;
            --count;

        }
    return result;

}
inline int enum_frame_intervals(int dev, __u32 pixfmt, __u32 width, __u32 height)
{
    int ret;
    struct v4l2_frmivalenum fival;

    memset(&fival, 0, sizeof(fival));
    fival.index = 0;
    fival.pixel_format = pixfmt;
    fival.width = width;
    fival.height = height;
    printf("\tTime interval between frame: ");
    while ((ret = ioctl(dev, VIDIOC_ENUM_FRAMEINTERVALS, &fival)) == 0) {
        if (fival.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
                printf("%u/%u, ",
                        fival.discrete.numerator, fival.discrete.denominator); //输出分数
        } else if (fival.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) {
                printf("{min { %u/%u } .. max { %u/%u } }, ",
                        fival.stepwise.min.numerator, fival.stepwise.min.numerator,
                        fival.stepwise.max.denominator, fival.stepwise.max.denominator);
                break;
        } else if (fival.type == V4L2_FRMIVAL_TYPE_STEPWISE) {
                printf("{min { %u/%u } .. max { %u/%u } / "
                        "stepsize { %u/%u } }, ",
                        fival.stepwise.min.numerator, fival.stepwise.min.denominator,
                        fival.stepwise.max.numerator, fival.stepwise.max.denominator,
                        fival.stepwise.step.numerator, fival.stepwise.step.denominator);
                break;
        }
        fival.index++;
    }
    printf("\n");
    if (ret != 0 && errno != EINVAL) {
        printf("ERROR enumerating frame intervals: %d\n", errno);
        return errno;
    }

    return 0;
};
inline int enum_frame_sizes(int dev, __u32 pixfmt)
{
    int ret;
    struct v4l2_frmsizeenum fsize;

    memset(&fsize, 0, sizeof(fsize));
    fsize.index = 0;
    fsize.pixel_format = pixfmt;
    while ((ret = ioctl(dev, VIDIOC_ENUM_FRAMESIZES, &fsize)) == 0) {
        if (fsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
            printf("{ discrete: width = %u, height = %u }\n",
                    fsize.discrete.width, fsize.discrete.height);
            ret = enum_frame_intervals(dev, pixfmt,
                    fsize.discrete.width, fsize.discrete.height);  //查找设备支持的 帧的间隔时间
            if (ret != 0)
                printf("  Unable to enumerate frame sizes.\n");
        } else if (fsize.type == V4L2_FRMSIZE_TYPE_CONTINUOUS) {
            printf("{ continuous: min { width = %u, height = %u } .. "
                    "max { width = %u, height = %u } }\n",
                    fsize.stepwise.min_width, fsize.stepwise.min_height,
                    fsize.stepwise.max_width, fsize.stepwise.max_height);
            printf("  Refusing to enumerate frame intervals.\n");
            break;
        } else if (fsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
            printf("{ stepwise: min { width = %u, height = %u } .. "
                    "max { width = %u, height = %u } / "
                    "stepsize { width = %u, height = %u } }\n",
                    fsize.stepwise.min_width, fsize.stepwise.min_height,
                    fsize.stepwise.max_width, fsize.stepwise.max_height,
                    fsize.stepwise.step_width, fsize.stepwise.step_height);
            printf("  Refusing to enumerate frame intervals.\n");
            break;
        }
        fsize.index++;
    }
    if (ret != 0 && errno != EINVAL) {
        printf("ERROR enumerating frame sizes: %d\n", errno);
        return errno;
    }

    return 0;
};

inline int V4L2GetSuportFormat(int fd,std::string& result){
    int ret=0;
    /* 查询摄像头可捕捉的图片类型，VIDIOC_ENUM_FMT: 枚举摄像头帧格式 */ 
    struct v4l2_fmtdesc fmt; 
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.index =0;
    // 指定需要枚举的类型 
    while((ret = ioctl(fd, VIDIOC_ENUM_FMT, &fmt)) == 0)// 有可能摄像头支持的图片格式不止一种 
    { 
        fmt.index++;
        /* 打印摄像头图片格式 */ 
        printf("pixelformat = '%c%c%c%c', description = '%s' \n",
                fmt.pixelformat & 0xFF, (fmt.pixelformat >> 8) & 0xFF,
                (fmt.pixelformat >> 16) & 0xFF, (fmt.pixelformat >> 24) & 0xFF,
                fmt.description);
        /* 查询支持的分辨率 */
        ret = enum_frame_sizes(fd, fmt.pixelformat); // 列举该格式下的帧大小
        if (ret != 0){
            printf("  Unable to enumerate frame sizes.\n");
        }
    }
    if (errno != EINVAL) {
        printf("ERROR enumerating frame formats: %d\n", errno);
        return errno;
    }
    return 0;
};

NAMESPACE_END
#endif // V4L2_TOOLS_H