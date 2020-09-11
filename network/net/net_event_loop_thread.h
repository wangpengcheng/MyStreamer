#ifndef NET_EVENTLOOPTHREAD_H
#define NET_EVENTLOOPTHREAD_H

#include "base_condition.h"
#include "base_mutex.h"
#include "base_thread.h"

NAMESPACE_START

namespace net
{

    class EventLoop;

    /*
事件循环线程;
主要是执行eventloop中的函数
*/
    class EventLoopThread : noncopyable
    {
    public:
        /* 线程初始化回调函数，注意输入参数是evenloop事件循环 */
        typedef std::function<void(EventLoop *)> ThreadInitCallback;

        EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),
                        const string &name = string());
        ~EventLoopThread();
        EventLoop *startLoop();

    private:
        void threadFunc();

        EventLoop *loop_ GUARDED_BY(mutex_);
        bool exiting_;
        Thread thread_;
        MutexLock mutex_;
        Condition cond_ GUARDED_BY(mutex_);
        ThreadInitCallback callback_;
    };

} // namespace net
NAMESPACE_END

#endif // NET_EVENTLOOPTHREAD_H