#include "base_date.h"
#include <stdio.h>
NAMESPACE_START
namespace detail
{

/* 为32位整数分配内存数组 */
char require_32_bit_integer_at_least[sizeof(int) >= sizeof(int32_t) ? 1 : -1];

// algorithm and explanation see:
// http://www.faqs.org/faqs/calendars/faq/part2/
// http://blog.csdn.net/Solstice

int getJulianDayNumber(int year, int month, int day)
{
    (void) require_32_bit_integer_at_least; // no warning please
    // 计算月份年，2月份之前算上一年
    int a = (14 - month) / 12;
    // 计算年
    int y = year + 4800 - a;
    // 计算对应的月份
    int m = month + 12 * a - 3;
    // 计算天数
    return day + (153*m + 2) / 5 + y*365 + y/4 - y/100 + y/400 - 32045;
}
struct Date::YearMonthDay getYearMonthDay(int julianDayNumber)
{
    int a = julianDayNumber + 32044;
    int b = (4 * a + 3) / 146097;
    int c = a - ((b * 146097) / 4);
    int d = (4 * c + 3) / 1461;
    int e = c - ((1461 * d) / 4);
    int m = (5 * e + 2) / 153;
    Date::YearMonthDay ymd;
    ymd.day = e - ((153 * m + 2) / 5) + 1;
    ymd.month = m + 3 - 12 * (m / 10);
    ymd.year = b * 100 + d - 4800 + (m / 10);
    return ymd;
}
}  // namespace detail
/*  对date值进行初始化，返回天数 */
const int Date::kJulianDayOf1970_01_01 = detail::getJulianDayNumber(1970, 1, 1);

/* 结构体的相关初始化 */
Date::Date(int y, int m, int d)
  : julianDayNumber_(detail::getJulianDayNumber(y, m, d))
{
}

Date::Date(const struct tm& t)
  : julianDayNumber_(detail::getJulianDayNumber(
        t.tm_year+1900,
        t.tm_mon+1,
        t.tm_mday))
{
}

string Date::toIsoString() const
{
  char buf[32];
  YearMonthDay ymd(yearMonthDay());
  snprintf(buf, sizeof buf, "%4d-%02d-%02d", ymd.year, ymd.month, ymd.day);
  return buf;
}

Date::YearMonthDay Date::yearMonthDay() const
{
  return detail::getYearMonthDay(julianDayNumber_);
}
NAMESPACE_END