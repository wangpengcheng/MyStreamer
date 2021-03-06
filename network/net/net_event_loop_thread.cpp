#include "net_event_loop_thread.h"
#include "net_event_loop.h"

using namespace MY_NAME_SPACE;
using namespace MY_NAME_SPACE::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                 const string& name)
    : loop_(NULL),
        exiting_(false),
        thread_(std::bind(&EventLoopThread::threadFunc, this), name),/* 在这里创建线程并绑定对应的函数 */
        mutex_(),
        cond_(mutex_),
        callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != NULL) // not 100% race-free, eg. threadFunc could be running callback_.
    {
        // still a tiny chance to call destructed object, if threadFunc exits just now.
        // but when EventLoopThread destructs, usually programming is exiting anyway.
        loop_->quit();
        thread_.join();
    }
}
/* 开始事件循环，线程一本事件循环是null因此会被阻塞 */
EventLoop* EventLoopThread::startLoop()
{
    assert(!thread_.started());
    /* 主线程调用线程类的start函数，创建线程；线程的主要执行函数是 threadFunc*/
    thread_.start();

    EventLoop* loop = NULL;
    {
         /* 加锁，原因是loop_可能会被子线程更改 */
        MutexLockGuard lock(mutex_);
         /*
        * 如果loop_仍然为null，说明子线程那边还没有进入threadFunc创建EventLoop，wait等待
        * pthread_cond_wait(pthread_cond_t&, pthread_mutex_t&);会自动解锁，然后睡眠
        * 等待某个线程使用pthread_cond_signal(&pthread_cond_t&);或pthread_cond_boardcast(pthread_cond_t&)
        * 唤醒wait的一个/全部线程
        * 当主线程从wait中返回后，子线程已经创建了EventLoop，主线程返回到EventLoopThreadPool中
        * 子线程执行EventLoop::loop函数监听事件
        */
        /* 这里的cond_.notify 由threadFunc创建，在thread初始化时就存在了，一般不会阻塞;
            会自动创建一个loop_
            */
        /* 开启事件循环前，需要保证线程和event已经就绪 */
        while (loop_ == NULL)
        {
            // 在这里阻塞等待event_loop已经准备好
            cond_.wait();
        }
        loop = loop_;
    }
    /* 返回当前线程的loop */
    return loop;
}
/* 执行函数，这里会创建一个eventloop */
void EventLoopThread::threadFunc()
{
    /* 创建loop */
    EventLoop loop;
    /* 存在回调;就继续执行回调函数 */
    if (callback_)
    {
        callback_(&loop);
    }

    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        /* 发送消息，解除startLoop的阻塞 */
        cond_.notify();
    }
    /* 子线程开启事件监听，进入无限循环，监听Accpetor 发送的socket描述符，进行处理，不返回 */
    loop.loop();
    //assert(exiting_);
    /* 循环结束，表明，出现问题，意外终止了，或者主动终止了循环，
        此时销毁loop，并重新设置
        线程执行函数结束
     */
    MutexLockGuard lock(mutex_);
    loop_ = NULL;
}