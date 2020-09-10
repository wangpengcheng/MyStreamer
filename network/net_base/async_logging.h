// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef BASE_ASYNCLOGGING_H
#define BASE_ASYNCLOGGING_H

#include "blocking_queue.h"
#include "bounded_blocking_queue.h"
#include "count_downlatch.h"
#include "base_mutex.h"
#include "base_thread.h"
#include "log_stream.h"
#include "base_tool.h"
#include <atomic>
#include <vector>

NAMESPACE_START
/**
 * 异步日志线程类
 * 主要用于异步的线程日志操作
 * **/
class AsyncLogging : noncopyable
{
public:
    AsyncLogging(const string &basename,
                 off_t rollSize,
                 int flushInterval = 3);

    ~AsyncLogging()
    {
        if (running_)
        {
            stop();
        }
    }

    void append(const char *logline, int len);

    void start()
    {
        running_ = true;
        thread_.start();
        latch_.wait();
    }

    void stop() NO_THREAD_SAFETY_ANALYSIS
    {
        running_ = false;
        cond_.notify();
        thread_.join();
    }

private:
    void threadFunc();

    typedef detail::FixedBuffer<detail::kLargeBuffer> Buffer;       /* 缓冲区 */
    typedef std::vector<std::unique_ptr<Buffer>> BufferVector;      /*  缓冲区队列 */
    typedef BufferVector::value_type BufferPtr;                     /* 缓冲区指针 */

    const int flushInterval_;                                       /* 刷新的时间周期 */
    std::atomic<bool> running_;                                     /* 是否正在运行 */
    const string basename_;                                         /* 名称 */
    const off_t rollSize_;
    Thread thread_;
    CountDownLatch latch_;
    MutexLock mutex_;
    Condition cond_ GUARDED_BY(mutex_);
    BufferPtr currentBuffer_ GUARDED_BY(mutex_);
    BufferPtr nextBuffer_ GUARDED_BY(mutex_);
    BufferVector buffers_ GUARDED_BY(mutex_);
};

NAMESPACE_END

#endif // BASE_ASYNCLOGGING_H
