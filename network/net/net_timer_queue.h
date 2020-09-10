#ifndef NET_TIMERQUEUE_H
#define NET_TIMERQUEUE_H

#include <set>
#include <vector>
#include "base_mutex.h"
#include "time_stamp.h"
#include "net_callbacks.h"
#include "net_channel.h"
NAMESPACE_START

namespace net
{
class EventLoop;
class Timer;
class TimerId;
/**
 * 利用Linux的timerfd设置epoll的定时激活启动函数
*/
class TimerQueue : noncopyable
{
public:
    explicit TimerQueue(EventLoop *loop);
    ~TimerQueue();

    ///
    /// Schedules the callback to be run at given time,
    /// repeats if @c interval > 0.0.
    ///
    /// Must be thread safe. Usually be called from other threads.
    /* 添加指定时间的回调 */
    TimerId addTimer(TimerCallback cb,/* 回调函数 */
                     Timestamp when,   /* 函数开始时间 */
                     double interval /* 时间间隔 */
                     );

    void cancel(TimerId timerId);

private:
    // FIXME: use unique_ptr<Timer> instead of raw pointers.
    // This requires heterogeneous comparison lookup (N3465) from C++14
    // so that we can find an T* in a set<unique_ptr<T>>.
    typedef std::pair<Timestamp, Timer *> Entry; /* 时间戳和计时器指针 */
    typedef std::set<Entry> TimerList;           /* 设置事件集合 */
    /* 
    * 主要用于删除操作，通过TimerId找到Timer*，再通过Timer*找到在timers_中的位置，将期删除
    * 觉得可以省略
    */
    typedef std::pair<Timer *, int64_t> ActiveTimer; /*  被取消的定时器 */
    typedef std::set<ActiveTimer> ActiveTimerSet;
    /* 循环时间添加函数， 以下成员函数只可能在其所属的I/O线程中调用，因而不必加锁。*
    * 用于注册定时任务
    * @param cb, 超时调用的回调函数
    * @param when，超时时间(绝对时间)
    * @interval，是否是周期性超时任务
    */
    void addTimerInLoop(Timer *timer);
    /* 取消定时任务，每个定时任务都有对应的TimerId，这是addTimer返回给调用者的 */
    void cancelInLoop(TimerId timerId);
    /* 当timerfd被激活时调用的回调函数，表示超时 */
    void handleRead();
    // 移除所有的超时定时器列表
    std::vector<Entry> getExpired(Timestamp now);
    /* 将超时任务中周期性的任务重新添加到timers_中 */
    void reset(const std::vector<Entry> &expired, Timestamp now);
    /* 添加定时器 */
    bool insert(Timer *timer);

    EventLoop *loop_;        /* 所属事件循环指针 */
    const int timerfd_;      /* 由timerfd_create创建的文件描述符 */
    Channel timerfdChannel_; /* 用于监听timerfd的Channel */
    // Timer list sorted by expiration
    TimerList timers_; /* 保存所有的定时任务 */

    // 取消时钟集合
    ActiveTimerSet activeTimers_;           /* 处于激活状态的时钟 */ /* 和timers_对应，主要是方便timer指针的快速查找 */
    bool callingExpiredTimers_; /* atomic */ /* 是否存在超时人物 */
    ActiveTimerSet cancelingTimers_;         /* 保存的是被取消的定时器，列表 */
};
} // namespace net
NAMESPACE_END

#endif