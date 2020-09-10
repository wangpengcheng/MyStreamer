#ifndef NET_EVENTLOOPTHREADPOOL_H
#define NET_EVENTLOOPTHREADPOOL_H

#include "uncopyable.h"
#include "base_types.h"

#include <functional>
#include <memory>
#include <vector>

NAMESPACE_START

namespace net
{

    class EventLoop;
    class EventLoopThread;
    /**
 * Eventloop线程池；主要还是对多个eventloop进行调度
*/
    class EventLoopThreadPool : noncopyable
    {
    public:
        typedef std::function<void(EventLoop *)> ThreadInitCallback;

        EventLoopThreadPool(EventLoop *baseLoop, const string &nameArg);
        ~EventLoopThreadPool();
        void setThreadNum(int numThreads) { numThreads_ = numThreads; }
        void start(const ThreadInitCallback &cb = ThreadInitCallback());

        // valid after calling start()
        /// round-robin
        EventLoop *getNextLoop(); /* 获取空闲的线程 */

        /// with the same hash code, it will always return the same EventLoop
        EventLoop *getLoopForHash(size_t hashCode);

        std::vector<EventLoop *> getAllLoops();

        bool started() const
        {
            return started_;
        }

        const string &name() const
        {
            return name_;
        }

    private:
        EventLoop *baseLoop_; /* 主要事件驱动loop */
        string name_;
        bool started_;
        int numThreads_;
        int next_;                                              /* 记录线程池中已经使用的下标 */
        std::vector<std::unique_ptr<EventLoopThread>> threads_; /* 线程集合 */
        std::vector<EventLoop *> loops_;                        /* 循环事件集合 */
    };

} // namespace net

NAMESPACE_END
#endif