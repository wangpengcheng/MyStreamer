/**
 * 计时器响应回调函数
 * 
*/
#ifndef NET_TIMER_H
#define NET_TIMER_H
#include "base_atomic.h"
#include "time_stamp.h"
#include "net_callbacks.h"
NAMESPACE_START

namespace net
{
/* 超时类事件 */
class Timer : noncopyable
{
public:
    Timer(TimerCallback cb, Timestamp when, double interval)
        : callback_(std::move(cb)),
        expiration_(when),
        interval_(interval),
        repeat_(interval > 0.0),
        sequence_(s_numCreated_.incrementAndGet())
    { }

    void run() const
    {
        callback_();
    }

    Timestamp expiration() const  { return expiration_; }
    bool repeat() const { return repeat_; }
    int64_t sequence() const { return sequence_; }

    void restart(Timestamp now);

    static int64_t numCreated() { return s_numCreated_.get(); }

private:
    const TimerCallback callback_;              /* 回调函数 */
    Timestamp expiration_;                      /* 过期时间 */
    const double interval_;                     /* 设置时间间隔 */
    const bool repeat_;                         /* 时钟是否可以重复 */
    const int64_t sequence_;                    /* 当前定时器的编号（基于s_numCreated_） */

    static AtomicInt64 s_numCreated_;           /* 记录定时器的个数 */
};

}  // namespace net

NAMESPACE_END

#endif