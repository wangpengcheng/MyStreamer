
/* 定义日期相关函数 */
#ifndef BASE_DATE_H
#define BASE_DATE_H

#include <ctime>
#include "base_tool.h"
#include "uncopyable.h"
#include "base_types.h"

NAMESPACE_START

class Date : public Uncopyable
{
public:
    struct YearMonthDay
    {
        int year;  // [1900..2500]
        int month; // [1..12]
        int day;   // [1..31]
    };
    /* 定义每周的天数 */
    static const int kDaysPerWeek = 7;
    /*定义 1970到现在所有的时间 */
    static const int kJulianDayOf1970_01_01;
    ///
    /// Constucts an invalid Date.
    ///
    Date()
        : julianDayNumber_(0)
    {
    }

    ///
    /// Constucts a yyyy-mm-dd Date.
    ///
    /// 1 <= month <= 12
    Date(int year, int month, int day);

    ///
    /// Constucts a Date from Julian Day Number.
    ///
    explicit Date(int julianDayNum)
        : julianDayNumber_(julianDayNum)
    {
    }

    ///
    /// Constucts a Date from struct tm
    ///
    explicit Date(const struct tm &);

    // default copy/assignment/dtor are Okay

    void swap(Date &that)
    {
        std::swap(julianDayNumber_, that.julianDayNumber_);
    }

    bool valid() const { return julianDayNumber_ > 0; }

    ///
    /// Converts to yyyy-mm-dd format.
    ///
    string toIsoString() const;

    struct YearMonthDay yearMonthDay() const;

    int year() const
    {
        return yearMonthDay().year;
    }

    int month() const
    {
        return yearMonthDay().month;
    }

    int day() const
    {
        return yearMonthDay().day;
    }

    // [0, 1, ..., 6] => [Sunday, Monday, ..., Saturday ]
    int weekDay() const
    {
        return (julianDayNumber_ + 1) % kDaysPerWeek;
    }

    int julianDayNumber() const { return julianDayNumber_; }

private:
    int julianDayNumber_;
};

inline bool operator<(Date x, Date y)
{
    return x.julianDayNumber() < y.julianDayNumber();
}

inline bool operator==(Date x, Date y)
{
    return x.julianDayNumber() == y.julianDayNumber();
}

NAMESPACE_END

#endif