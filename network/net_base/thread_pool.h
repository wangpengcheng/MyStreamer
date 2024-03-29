// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_THREADPOOL_H
#define MUDUO_BASE_THREADPOOL_H

#include "base_condition.h"
#include "base_mutex.h"
#include "base_thread.h"
#include "base_types.h"

#include <deque>
#include <vector>

NAMESPACE_START

class ThreadPool : noncopyable
{
public:
    typedef std::function<void()> Task;

    explicit ThreadPool(const string &nameArg = string("ThreadPool"));
    ~ThreadPool();

    // Must be called before start().
    void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }
    void setThreadInitCallback(const Task &cb)
    {
        threadInitCallback_ = cb;
    }

    void start(int numThreads);
    void stop();

    const string &name() const
    {
        return name_;
    }

    size_t queueSize() const;

    // Could block if maxQueueSize > 0
    // There is no move-only version of std::function in C++ as of C++14.
    // So we don't need to overload a const& and an && versions
    // as we do in (Bounded)BlockingQueue.
    // https://stackoverflow.com/a/25408989
    void run(Task f);

private:
    bool isFull() const REQUIRES(mutex_);
    void runInThread();
    Task take();

    mutable MutexLock mutex_;
    Condition notEmpty_ GUARDED_BY(mutex_);         /* 非空 */
    Condition notFull_ GUARDED_BY(mutex_);          /* 非满 */
    string name_;                                   ///< 线程池名称
    Task threadInitCallback_;                       /* 线程池初始化任务，主要用于任务分发 */
    std::vector<std::unique_ptr<Thread> > threads_; /* 线程队列 */
    std::deque<Task> queue_ GUARDED_BY(mutex_);     /* 任务队列访问指针 */
    size_t maxQueueSize_;                           ///< 缓冲队列大小
    bool running_;                                  /* 是否存在线程正在运行 */
};

NAMESPACE_END

#endif // MUDUO_BASE_THREADPOOL_H
