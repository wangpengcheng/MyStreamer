#ifndef NET_EVENTLOOP_H
#define NET_EVENTLOOP_H
#include <atomic>
#include <functional>
#include <vector>

#include <boost/any.hpp>

#include "base_mutex.h"
#include "current_thread.h"
#include "time_stamp.h"
#include "net_callbacks.h"
#include "net_timer_id.h"

NAMESPACE_START

namespace net
{
    class Channel;
    class Poller;
    class TimerQueue;

///
/// Reactor, at most one per thread.
///
/// This is an interface class, so don't expose too much details.
/**
 * 反应分发类，基本每一个线程一个
 * 每个线程的响应类，这个是一个接口类，没有太多细节
 * 1.首先我们应该调用updateChannel来添加一些事件(内部调用poller->updateChannel()来添加注册事件)
 * 2.接着调用loop函数来执行事件循环，在执行事件循环的过程中，会阻塞在poller->poll调用处，Poller类会把活跃的事件放在activeChannel集合中
 * 3.然后调用Channel中的handleEvent来处理事件发生时对应的回调函数，处理完事件函数后还会处理必须由I/O线程来完成的doPendingFunctors(额外操作函数)函数
*/
class EventLoop : noncopyable
{
public:
    typedef std::function<void()> Functor;

    EventLoop();
    ~EventLoop();  // force out-line dtor, for std::unique_ptr members.

    ///
    /// Loops forever.
    ///
    /// Must be called in the same thread as creation of the object.
    ///
    void loop();

    /// Quits loop.
    ///
    /// This is not 100% thread safe, if you call through a raw pointer,
    /// better to call through shared_ptr<EventLoop> for 100% safety.
    void quit();

    ///
    /// poll延迟的时间
    ///
    Timestamp pollReturnTime() const { return pollReturnTime_; }
    // 迭代次数
    int64_t iteration() const { return iteration_; }

    /// Runs callback immediately in the loop thread.
    /// It wakes up the loop, and run the cb.
    /// If in the same loop thread, cb is run within the function.
    /// Safe to call from other threads.
    void runInLoop(Functor cb);
    /// Queues callback in the loop thread.
    /// Runs after finish pooling.
    /// Safe to call from other threads.
    void queueInLoop(Functor cb);

    size_t queueSize() const;

    // timers

    ///
    /// Runs callback at 'time'.
    /// Safe to call from other threads.
    /// 某个时间点执行回调
    TimerId runAt(Timestamp time, TimerCallback cb);
    ///
    /// Runs callback after @c delay seconds.
    /// Safe to call from other threads.
    /// 某个时间点之后执行回调
    TimerId runAfter(double delay, TimerCallback cb);
    ///
    /// Runs callback every @c interval seconds.
    /// Safe to call from other threads.
    /// 在每个时间间隔处理某个回调函数
    TimerId runEvery(double interval, TimerCallback cb);
    ///
    /// Cancels the timer.
    /// Safe to call from other threads.
    /// 删除某个定时器
    void cancel(TimerId timerId);

    // internal usage
 
  void wakeup();		                //唤醒事件通知描述符
  void updateChannel(Channel* channel);		//添加某个事件分发器
  void removeChannel(Channel* channel);		//移除某个事件分发器
  bool hasChannel(Channel* channel);        //是否拥有某个分发器

    // 如果不在I/O线程中则退出程序
    void assertInLoopThread()
    {
        if (!isInLoopThread())
        {
            abortNotInLoopThread();
        }
    }
    /* 是否在I/O线程中 */
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
    // 是否正在处理事件
    bool eventHandling() const { return eventHandling_; }
    /* 设置上下文 */
    void setContext(const boost::any& context)
    { context_ = context; }

    const boost::any& getContext() const
    { return context_; }

    boost::any* getMutableContext()
    { return &context_; }

    static EventLoop* getEventLoopOfCurrentThread(); //判断当前线程是否为I/O线程

private:
    void abortNotInLoopThread();                    //不在主I/O线程
    void handleRead();                              //将事件通知描述符里的内容读走,以便让其继续检测事件通知
    void doPendingFunctors();                       //执行转交给I/O的任务

    void printActiveChannels() const;               //将发生的事件写入日志

    typedef std::vector<Channel*> ChannelList;      //事件分发器列表

    bool looping_;                  /* 是否正在执行 */
    std::atomic<bool> quit_;        /* 是否结束 */
    bool eventHandling_;            /* 是否绑定响应事件 */
    bool callingPendingFunctors_;   /* atomic */
    int64_t iteration_;             //事件循环的次数
    const pid_t threadId_;          //运行loop的线程ID，一个线程只能有一个eventloop
    Timestamp pollReturnTime_;          /* poll阻塞的时间 */
    std::unique_ptr<Poller> poller_;    /* 指向的poller列表,表示只有一个poller_ */
    std::unique_ptr<TimerQueue> timerQueue_;    /* 指向的时间队列 */
    int wakeupFd_;                              /* 唤醒当前线程的定时器描述符 */
    /* 
    * 用于唤醒当前线程，因为当前线程主要阻塞在poll函数上
    * 所以唤醒的方法就是手动激活这个wakeupChannel_，即写入几个字节让Channel变为可读
    * 注: 这个Channel也注册到Poller中
    */
    std::unique_ptr<Channel> wakeupChannel_;        //封装事件描述符
    boost::any context_;
    /* 
    * 激活队列，poll函数在返回前将所有激活的Channel添加到激活队列中
    * 在当前事件循环中的所有Channel在Poller中
    */
    ChannelList activeChannels_;                    //活跃的事件集
    Channel* currentActiveChannel_;                 //当前处理的事件集
    /* 
    * queueInLoop添加函数时给pendingFunctors_上锁，防止多个线程同时添加
    * 
    * mutable,突破const限制，在被const声明的函数仍然可以更改这个变量
    */
    mutable MutexLock mutex_;                       //互斥锁
    /* 
    * 等待在当前线程调用的回调函数，
    * 原因是本来属于当前线程的回调函数会被其他线程调用时，应该把这个回调函数添加到它属于的线程中
    * 等待它属于的线程被唤醒后调用，以满足线程安全性
    * 
    * TcpServer::removeConnection是个例子
    * 当关闭一个TcpConnection时，需要调用TcpServer::removeConnection，但是这个函数属于TcpServer，
    * 然而TcpServer和TcpConnection不属于同一个线程，这就容易将TcpServer暴露给其他线程，
    * 万一其他线程析构了TcpServer怎么办（线程不安全）
    * 所以会调用EventLoop::runInLoop，如果要调用的函数属于当前线程，直接调用
    * 否则，就添加到这个队列中，等待当前线程被唤醒
    * https://blog.csdn.net/sinat_35261315/article/details/78329657
    */
    std::vector<Functor> pendingFunctors_ GUARDED_BY(mutex_);//需要在主I/O线程执行的任务
};
}//namespace net

NAMESPACE_END

#endif