#ifndef NET_TIMERID_H
#define NET_TIMERID_H
#include "base_tool.h"
#include "uncopyable.h"
NAMESPACE_START
namespace net
{
class Timer;

/**
 * 时间id对象，帮助取消时钟
*/
class TimerId : public copyable
{
public:
    TimerId()
        : timer_(NULL),
        sequence_(0)
    {
    }

    TimerId(Timer* timer, int64_t seq)
        : timer_(timer),
        sequence_(seq)
    {
    }

friend class TimerQueue;

private:
    Timer* timer_;              /* 计时器指针 */
    int64_t sequence_;          /* 相关队列 */
};


}//namespace net

NAMESPACE_END

#endif