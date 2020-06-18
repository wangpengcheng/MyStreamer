#include "net_event_loop.h"
#include "logging.h"
#include "net_channel.h"
#include "net_poller.h"
#include "net_sockets_ops.h"
#include "net_timer_queue.h"

#include <algorithm>

#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

using namespace MY_NAME_SPACE;
using namespace MY_NAME_SPACE::net;

namespace
{
__thread EventLoop* t_loopInThisThread = 0; /* 记录当前loop的线程id指针 */
/* 设置超时事件为10s */
const int kPollTimeMs = 10000;
/* 创建事件描述符 */
int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_SYSERR << "Failed in eventfd";
        abort();
    }
    return evtfd;
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe
{
    public:
    IgnoreSigPipe()
    {
        ::signal(SIGPIPE, SIG_IGN);
        // LOG_TRACE << "Ignore SIGPIPE";
    }
};
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe initObj;
}  // namespace

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
  return t_loopInThisThread;
}

EventLoop::EventLoop()
    : looping_(false),
        quit_(false),
        eventHandling_(false),
        callingPendingFunctors_(false),
        iteration_(0),
        threadId_(CurrentThread::tid()),
        poller_(Poller::newDefaultPoller(this)),//初始化poll;虽然每个线程都有poll但是只有accpter回正式调用
        timerQueue_(new TimerQueue(this)),
        wakeupFd_(createEventfd()), //创建唤醒事件描述符
        wakeupChannel_(new Channel(this, wakeupFd_)), /*  每个事件循环会有一个唤醒Channel ；主要还是给Acceptor使用*/
        currentActiveChannel_(NULL)
{
    LOG_DEBUG << "EventLoop created " << this << " in thread " << threadId_;
    /* 检查当前线程是否含有其它线程 */
    if (t_loopInThisThread)
    {
        LOG_FATAL << "Another EventLoop " << t_loopInThisThread
                << " exists in this thread " << threadId_;
    }
    else
    {
        t_loopInThisThread = this;/*  设置当前线程的eventloop */
    }
    /* 添加唤醒事件,读取8个字节的值 */
    wakeupChannel_->setReadCallback(
        std::bind(&EventLoop::handleRead, this));
    // we are always reading the wakeupfd
    /* 设置可读事件 */
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
                << " destructs in thread " << CurrentThread::tid();
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = NULL;
}
/* 主要循环函数 */
/* 
 * 事件驱动主循环
 * 
 * 1.每个TcpServer对应一个事件驱动循环线程池
 * 2.每个事件驱动循环线程池对应多个事件驱动循环线程
 * 3.每个事件驱动循环线程对应一个事件驱动主循环
 * 4.每个事件驱动主循环对应一个io多路复用函数
 * 5.每个io多路复用函数监听多个Channel
 * 6.每个Channel对应一个fd，也就对应一个TcpConnection或者监听套接字
 * 7.在poll返回后处理激活队列中Channel的过程是同步的，也就是一个一个调用回调函数
 * 8.调用回调函数的线程和事件驱动主循环所在线程是同一个，也就是同步执行回调函数
 * 9.线程池用在事件驱动循环上层，也就是事件驱动循环是线程池中的一个线程
 */
void EventLoop::loop()
{
    assert(!looping_);
    /* 进行断言防止递归调用 */
    assertInLoopThread();
    looping_ = true;
    quit_ = false;  // FIXME: what if someone calls quit() before loop() ?
    LOG_TRACE << "EventLoop " << this << " start looping";

    while (!quit_)
    {
        /* 清除时间循环中的活动事件 */
        activeChannels_.clear();
         /* epoll_wait返回后会将所有就绪的Channel添加到激活队列activeChannel中 */
         /* 循环检测是否可读写 */
         /* 获取可操作的事件，并返回事件集合 */
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);/*  注意这里不会阻塞，而是会直接执行 */
        ++iteration_;
        /* 输出激活的事件 */
        if (Logger::logLevel() <= Logger::TRACE)
        {
            printActiveChannels();
        }
        /* 执行所有在激活队列中的Channel的回调函数 */
        eventHandling_ = true;
        for (Channel* channel : activeChannels_)
        {
            currentActiveChannel_ = channel;
            currentActiveChannel_->handleEvent(pollReturnTime_);
        }
        currentActiveChannel_ = NULL;
        eventHandling_ = false;
        /* 执行额外的注册的函数；包括队列中的 */
        doPendingFunctors();
    }

    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}
/* 关闭loop */
void EventLoop::quit()
{
    quit_ = true;
    // There is a chance that loop() just executes while(!quit_) and exits,
    // then EventLoop destructs, then we are accessing an invalid object.
    // Can be fixed using mutex_ in both places.
    if (!isInLoopThread())
    {
        wakeup();
    }
}
/* 运行函数 */
void EventLoop::runInLoop(Functor cb)
{
    /* 事件正在线程中，直接执行 */
  if (isInLoopThread())
  {
    cb();
  }
  else
  {
      /* 否则排序等待 */
    queueInLoop(std::move(cb));
  }
}
/* 将函数插入执行队列 */
/*
 * 由runInLoop调用，也可直接调用，作用
 * 1.将相应的回调函数存在事件驱动循环的队列中，等待回到自己线程再调用它
 * 2.激活自己线程的事件驱动循环
 */
void EventLoop::queueInLoop(Functor cb)
{
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }
    /* 如果没有在运行，就唤醒线程 */
    if(!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}
/* 获取当前的函数队列 */
size_t EventLoop::queueSize() const
{
    MutexLockGuard lock(mutex_);
    return pendingFunctors_.size();
}
/* 某个时间点执行函数回调函数 */
TimerId EventLoop::runAt(Timestamp time, TimerCallback cb)
{
    return timerQueue_->addTimer(std::move(cb), time, 0.0);
}
/* 在某个时间点之后执行 */
TimerId EventLoop::runAfter(double delay, TimerCallback cb)
{
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, std::move(cb));
}
/* 执行函数 */
TimerId EventLoop::runEvery(double interval, TimerCallback cb)
{
    /* 设置计时器 */
    Timestamp time(addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(std::move(cb), time, interval);
}
/* 取消计时器 */
void EventLoop::cancel(TimerId timerId)
{
    return timerQueue_->cancel(timerId);
}
/* 更新channel */
void EventLoop::updateChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}
/* 移除channel */
void EventLoop::removeChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    if (eventHandling_)
    {
        assert(currentActiveChannel_ == channel ||
            std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
    }
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    return poller_->hasChannel(channel);
}
/* 检查当前执行线程非本线程 */
void EventLoop::abortNotInLoopThread()
{
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
                << " was created in threadId_ = " << threadId_
                << ", current thread id = " <<  CurrentThread::tid();
}
/* 执行唤醒函数 */
void EventLoop::wakeup()
{
  uint64_t one = 1;
  ssize_t n = sockets::write(wakeupFd_, &one, sizeof one);
  if (n != sizeof one)
  {
    LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = sockets::read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}
/* 执行扩展的函数 */
void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        MutexLockGuard lock(mutex_);
        functors.swap(pendingFunctors_);
    }
     /* 执行集合中的所有函数 */
    for (const Functor& functor : functors)
    {
       
        functor();
    }
    callingPendingFunctors_ = false;
}
/* 输出激活的事件 */
void EventLoop::printActiveChannels() const
{
    for (const Channel* channel : activeChannels_)
    {
        LOG_TRACE << "{" << channel->reventsToString() << "} ";
    }
}
