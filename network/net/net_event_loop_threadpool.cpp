

#include "net_event_loop.h"
#include "net_event_loop_thread.h"
#include "net_event_loop_threadpool.h"
#include <stdio.h>

using namespace MY_NAME_SPACE;
using namespace MY_NAME_SPACE::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const string& nameArg)
    : baseLoop_(baseLoop),
        name_(nameArg),
        started_(false),
        numThreads_(0),
        next_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
  // Don't delete loop, it's stack variable
}
/* 
 * 线程池所在线程在每创建一个EventLoopThread后会调用相应对象的startLoop函数，注意主线程和子线程之分
 * 主线程是TcpServer所在线程，也是线程池所在线程
 * 子线程是由线程池通过pthread_create创建的线程，每一个子线程运行一个EventLoop::loop 
 * 
 * 1.主线程EventLoopThreadPool创建EventLoopThread对象
 * 2.主线程EventLoopThread构造函数中初始化线程类Thread并传递回调函数EventLoopThread::threadFunc
 * 3.主线程EventLoopThreadPool创建完EventLoopThread后，调用EventLoopThread::startLoop函数；开始阻塞在等待处
 * 4.主线程EventLoopThread::startLoop函数开启线程类Thread，即调用Thread::start
 * 5.主线程Thread::start函数中使用pthread_create创建线程后
 *   子线程调用回调函数EventLoopThread::threadFunc，主线程返回到EventLoopThread::startLoop
 * 6.主线程EventLoopThread::startLoop由于当前事件驱动循环loop_为null（构造时初始化为null）导致wait
 * 7.子线程EventLoopThread::threadFunc创建EventLoop并赋值给loop_，然后唤醒阻塞在cond上的主线程
 * 8.主线程EventLoopThread::startLoop被唤醒后，返回loop_给EventLoopThreadPool
 * 9.主线程EventLoopThreadPool保存返回的loop_，存放在成员变量std::vector<EventLoop*> loops_中
 * 10.子线程仍然在threadFunc中，调用EventLoop::loop函数，无限循环监听
 */
void EventLoopThreadPool::start(const ThreadInitCallback& cb)
{
    assert(!started_);
    baseLoop_->assertInLoopThread();

    started_ = true;
    /* 初始化线程，创建指定个EventLoopThread */
    for(int i = 0; i < numThreads_; ++i)
    { 
        /* 线程名称 */
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        /* 创建一个线程 */
        EventLoopThread* t = new EventLoopThread(cb, buf);
        /* 将线程添加到线程队列中 */
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        /* 创建新线程，返回新线程的事件驱动循环EventLoop */
        /* EventLoopThread主线程返回后，将事件驱动循环保存下来，然后继续创建线程 */
        /* 线程在event loop创建之后，会调用loop陷入无限循环 */
        loops_.push_back(t->startLoop());
    }
    //没有创建线程由，主线程执行函数
    if (numThreads_ == 0 && cb)
    {
        cb(baseLoop_);
    }
    /* 至此线程池的创建工作完成，每一个线程都运行着
        EventLoop::loop，
        进行EventLoop::loop -> Poller::poll -> 
        Channel::handleEvent -> TcpConnection::handle* -> 
        EventLoop::doPendingFunctors -> EventLoop::loop的工作。
        如果提供了回调函数，在创建完成后也会执行，
        但通常用户不会在意线程池的创建工作，
        所以一般都不提供 
    */
    
}
//获取下一条线程
EventLoop* EventLoopThreadPool::getNextLoop()
{
    baseLoop_->assertInLoopThread();
    assert(started_);
    EventLoop* loop = baseLoop_;

    if (!loops_.empty())
    {
        // round-robin
        loop = loops_[next_];
        ++next_;
        /* 超过之后，重新设置为0 */
        if (implicit_cast<size_t>(next_) >= loops_.size())
        {
            next_ = 0;
        }
    }
    return loop;
}

EventLoop* EventLoopThreadPool::getLoopForHash(size_t hashCode)
{
    baseLoop_->assertInLoopThread();
    EventLoop* loop = baseLoop_;

    if (!loops_.empty())
    {
        loop = loops_[hashCode % loops_.size()];
    }
    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
{
    baseLoop_->assertInLoopThread();
    assert(started_);
    if (loops_.empty())
    {
        return std::vector<EventLoop*>(1, baseLoop_);
    }
    else
    {
        return loops_;
    }
}
