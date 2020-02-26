#ifndef TIME_STAMP_H
#define TIME_STAMP_H

#include "uncopyable.h"
#include "base_types.h"

NAMESPACE_START

///
/// Time stamp in UTC, in microseconds resolution.
///
/// This class is immutable.
/// It's recommended to pass it by value, since it's passed in register on x64.
///
class Timestamp 
{
public:
    ///
    /// Constucts an invalid Timestamp.
    ///
    Timestamp()
        : microSecondsSinceEpoch_(0)
    {
    }

    ///
    /// Constucts a Timestamp at specific time
    ///
    /// @param microSecondsSinceEpoch
    explicit Timestamp(int64_t microSecondsSinceEpochArg)
        : microSecondsSinceEpoch_(microSecondsSinceEpochArg)
    {
    }

    void swap(Timestamp& that)
    {
        std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
    }

    // default copy/assignment/dtor are Okay

    string toString() const;
    string toFormattedString(bool showMicroseconds = true) const;

    bool valid() const { return microSecondsSinceEpoch_ > 0; }

    // for internal usage.
    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
    time_t secondsSinceEpoch() const
    { 
        return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond); 
    }

    ///
    /// Get time of now.
    ///
    static Timestamp now();
    static Timestamp invalid()
    {
        return Timestamp();
    }

    static Timestamp fromUnixTime(time_t t)
    {
        return fromUnixTime(t, 0);
    }

    static Timestamp fromUnixTime(time_t t, int microseconds)
    {
        return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds);
    }
    /* 定义每毫秒的时间 */

    static const int kMicroSecondsPerSecond = 1000 * 1000;
    /* 重载比较操作符号 */
    inline bool operator<(const Timestamp& rhs)
    {
        return this->microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
    }
    inline bool operator<=(const Timestamp& rhs)
    {
        return this->microSecondsSinceEpoch() <= rhs.microSecondsSinceEpoch();
    }
    inline bool operator>(const Timestamp& rhs)
    {
        return this->microSecondsSinceEpoch()> rhs.microSecondsSinceEpoch();
    }
    inline bool operator>=(const Timestamp& rhs)
    {
        return this->microSecondsSinceEpoch()>=rhs.microSecondsSinceEpoch();
    }
    inline bool operator==(const Timestamp& rhs)
    {
        return this->microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
    }
    inline bool operator!=(const Timestamp& rhs)
    {
        return this->microSecondsSinceEpoch() != rhs.microSecondsSinceEpoch();
    }


private:
    int64_t microSecondsSinceEpoch_;                        /* 格林威志时间的总的秒数 */
};


///
/// Gets time difference of two timestamps, result in seconds.
///
/// @param high, low
/// @return (high-low) in seconds
/// @c double has 52-bit precision, enough for one-microsecond
/// resolution for next 100 years.
inline double timeDifference(Timestamp high, Timestamp low)
{
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

///
/// Add @c seconds to given timestamp.
///
/// @return timestamp+seconds as Timestamp
///
inline Timestamp addTime(Timestamp timestamp, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}


NAMESPACE_END

#endif