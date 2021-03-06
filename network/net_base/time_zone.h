/* 
声明时间管理器函数
*/

#ifndef BASE_TIMEZONE_H
#define BASE_TIMEZONE_H
#include "base_tool.h"
#include "uncopyable.h"
#include <memory>
#include <time.h>

NAMESPACE_START
/* 定义时钟封装,主要将Linux时间转换为自定义时间 */
class TimeZone
{
public:
    explicit TimeZone(const char *zonefile);
    TimeZone(int eastOfUtc, const char *tzname); // a fixed timezone
    TimeZone() = default;                        // an invalid timezone

    // default copy ctor/assignment/dtor are Okay.
    /* 检查数是否存在 */
    bool valid() const
    {
        // 'explicit operator bool() const' in C++11
        return static_cast<bool>(data_);
    }

    struct tm toLocalTime(time_t secondsSinceEpoch) const;
    time_t fromLocalTime(const struct tm &) const;

    // gmtime(3)
    static struct tm toUtcTime(time_t secondsSinceEpoch, bool yday = false);
    // timegm(3)
    static time_t fromUtcTime(const struct tm &);
    // year in [1900..2500], month in [1..12], day in [1..31]
    static time_t fromUtcTime(int year, int month, int day,
                              int hour, int minute, int seconds);

    struct Data;

private:
    std::shared_ptr<Data> data_;
};
NAMESPACE_END
#endif