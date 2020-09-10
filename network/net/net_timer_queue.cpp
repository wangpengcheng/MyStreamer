#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include "net_timer_queue.h"
#include "net_event_loop.h"
#include "net_timer.h"
#include "net_timer_id.h"
#include "net_event_loop.h"
#include "logging.h"
#include <sys/timerfd.h>
#include <unistd.h>

NAMESPACE_START

namespace net
{
namespace detail
{
int createTimerfd()
{
    /**
     * 创建计时器
     */
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                    TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0)
    {
        LOG_SYSFATAL << "Failed in timerfd_create";
    }
    return timerfd;
}
/* 计算某个时间点到当前的时间 */
struct timespec howMuchTimeFromNow(Timestamp when)
{
    /* 毫秒统计 */
    int64_t microseconds = when.microSecondsSinceEpoch()
                            - Timestamp::now().microSecondsSinceEpoch();
    /* 设置最小时间间隔为100毫秒 */
    if (microseconds < 100)
    {
        microseconds = 100;
    }
    struct timespec ts;
    // 获取秒
    ts.tv_sec = static_cast<time_t>(
        microseconds / Timestamp::kMicroSecondsPerSecond);
    // 获取纳秒
    ts.tv_nsec = static_cast<long>(
        (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}
/* 读取timerfd中的相关这只，主要是获取超时的次数 */
void readTimerfd(int timerfd, Timestamp now)
{
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
    LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
    if (n != sizeof howmany)
    {
        LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
    }
}
/* 重现设置timerfd;将器超时时间设置为expiration*/
void resetTimerfd(int timerfd, Timestamp expiration)
{
    // wake up loop by timerfd_settime()
    /* 设置启动描述符的定时器 */
    struct itimerspec newValue;
    struct itimerspec oldValue;
    /* 将结构体设置为0 */
    memZero(&newValue, sizeof newValue);
    memZero(&oldValue, sizeof oldValue);
    newValue.it_value = howMuchTimeFromNow(expiration);/* 设置第一次到时时间 */
    /* 设置定时器 */
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret)
    {
        LOG_SYSERR << "timerfd_settime()";
    }
}

} //namespace net
} //namespace detail

NAMESPACE_END

using namespace MY_NAME_SPACE;
using namespace MY_NAME_SPACE::net;
using namespace MY_NAME_SPACE::net::detail;

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop),
        timerfd_(createTimerfd()),
        timerfdChannel_(loop, timerfd_),
        timers_(),
        callingExpiredTimers_(false)
{
    /* 添加计时器文件，读取监听函数 */
    timerfdChannel_.setReadCallback(
        std::bind(&TimerQueue::handleRead, this));
    // we are always reading the timerfd, we disarm it with timerfd_settime.
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    ::close(timerfd_);
    // 并不移除channel
    for (const Entry& timer : timers_)
    {
        delete timer.second;
    }
}

TimerId TimerQueue::addTimer(TimerCallback cb,
                             Timestamp when,
                             double interval)
{
    /* 创建新的定时器 */
    Timer* timer = new Timer(std::move(cb), when, interval);
    /* 
    * 在自己所属线程调用addTimerInLoop函数 
    * 用户只能通过初始创建的EventLoop调用addTimer，为什么还会考虑线程问题 why?
    * 这个线程和TcpServer的线程应该是同一个
    */
    loop_->runInLoop(
        std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer, timer->sequence());
}

void TimerQueue::cancel(TimerId timerId)
{
  loop_->runInLoop(
      std::bind(&TimerQueue::cancelInLoop, this, timerId));
}
/* 向计时器队列中添加超时事件 */
void TimerQueue::addTimerInLoop(Timer* timer)
{
    /* 检查是否是主线程 */
    loop_->assertInLoopThread();
    bool earliestChanged = insert(timer);

    if (earliestChanged)
    {
        /* 重新设置过期时间 */
        resetTimerfd(timerfd_, timer->expiration());
    }
}
/* 取消计时器 */
void TimerQueue::cancelInLoop(TimerId timerId)
{
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    ActiveTimer timer(timerId.timer_, timerId.sequence_);
    /* 查找定时器 */
    ActiveTimerSet::iterator it = activeTimers_.find(timer);
    if (it != activeTimers_.end())
    {
        size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
        assert(n == 1); (void)n;
        delete it->first; // FIXME: no delete please
        activeTimers_.erase(it);
    }
    else if (callingExpiredTimers_)
    {
        cancelingTimers_.insert(timer);
    }
    assert(timers_.size() == activeTimers_.size());
}
/* 读取句柄 */
void TimerQueue::handleRead()
{
    /* 检查当前线程是否正在执行 */
    loop_->assertInLoopThread();
    /* 获取当前时间 */
    Timestamp now(Timestamp::now());
    /* 当前输出超时次数 */
    readTimerfd(timerfd_, now);
    /* 获取所有的超时定时器 */
    std::vector<Entry> expired = getExpired(now);
    /* 清楚超时定时器 */
    callingExpiredTimers_ = true;
    cancelingTimers_.clear();
    /* 执行所有的超时定时器函数 */
    for (const Entry& it : expired)
    {
        it.second->run();
    }
    callingExpiredTimers_ = false;
    /* 重新设置定时器；将周期性任务，重新添加到计时器队列中 */
    reset(expired, now);
} 
/* 将超时的任务拿出来 */
std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    /* 所有任务都超时，直接跳出 */
    assert(timers_.size() == activeTimers_.size());
    /* 超时任务集合 */
    std::vector<Entry> expired;
    /* 定义一个最大时间的超时器 */
    Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    /* lower_bound：找到第一个大于等于参数的位置，返回迭代器(此处如果超时时间恰好是now，应该不算作超时) */
    TimerList::iterator end = timers_.lower_bound(sentry);
    /* 如果没有，则直接中断 */
    assert(end == timers_.end() || now < end->first);
    /* 将超时函数，插入集合的尾部 */
    std::copy(timers_.begin(), end, back_inserter(expired));
    /* 定时器中删除超时的集合 */
    timers_.erase(timers_.begin(), end);

    /* 从激活的计时器队列中移除选中的定时器 */
    for (const Entry& it : expired)
    {
        ActiveTimer timer(it.second, it.second->sequence());
        size_t n = activeTimers_.erase(timer);
        assert(n == 1); (void)n;
    }
    /* 如果计时器总大小和真实的剩余大小不同；直接返回错误 */
    assert(timers_.size() == activeTimers_.size());
    return expired;
}
/* 
 * 调用完所有超时的回调函数后，需要对这些超时任务进行整理
 * 将周期性的定时任务重新添加到set中
 */
void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
    Timestamp nextExpire;

    for (const Entry& it : expired)
    {
        ActiveTimer timer(it.second, it.second->sequence());
        if (it.second->repeat()  /* 是否是周期性的定时任务 */
            && cancelingTimers_.find(timer) == cancelingTimers_.end())/* 如果用户手动删除了这个定时任务，就不添加了 */
        {
            /* 重新计算超时时间 */
            it.second->restart(now);
            /* 重新添加到set中 */
            insert(it.second);
        }
        else /* 否则删除定时器 */
        {
            // FIXME move to a free list
            delete it.second; // FIXME: no delete please
        }
    }
    /* 计算下次timerfd被激活的时间 */
    if (!timers_.empty())
    {
        nextExpire = timers_.begin()->second->expiration();
    }

    if (nextExpire.valid())
    {
        resetTimerfd(timerfd_, nextExpire);
    }
}
/* 将定时器，插入到定时任务set中 */
bool TimerQueue::insert(Timer* timer)
{
    loop_->assertInLoopThread();
    /* 如果计时器队列和激活的队列相同，即所有都被激活，直接退出 */
    assert(timers_.size() == activeTimers_.size());
    /* 获取timer的UTC时间戳，和timer组成std::pair<Timestamp, Timer*> */
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    /* timers_begin()是set顶层元素（红黑树根节点），是超时时间最近的Timer* */
    TimerList::iterator it = timers_.begin();
    /* 如果要添加的timer的超时时间,比timers_中的超时时间近，更改新的超时时间 */
    if (it == timers_.end() || when < it->first)
    {
        earliestChanged = true;
    }
    {
        /* 添加到定时任务的set中 */
        std::pair<TimerList::iterator, bool> result
        = timers_.insert(Entry(when, timer));
        assert(result.second); (void)result;
    }
    {
        /* 同时也添加到activeTimers_中，用于删除时查找操作 */
        std::pair<ActiveTimerSet::iterator, bool> result
        = activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
        assert(result.second); (void)result;
    }

    assert(timers_.size() == activeTimers_.size());
    return earliestChanged;
}
