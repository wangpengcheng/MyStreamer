#include  "net_timer.h"

NAMESPACE_START
namespace net
{
AtomicInt64 Timer::s_numCreated_;

void Timer::restart(Timestamp now)
{

    if (repeat_)
    {
        /* 更新过期时间 */
        expiration_ = addTime(now, interval_);
    }
    else
    {
        expiration_ = Timestamp::invalid();
    }
}

}//namespace net
NAMESPACE_END