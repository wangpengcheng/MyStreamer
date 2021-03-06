/*
* 基本缓冲队列 
*/

#ifndef BASE_BLOCKINGQUEUE_H
#define BASE_BLOCKINGQUEUE_H


#include "base_mutex.h"
#include "base_condition.h"
#include <deque>
#include <assert.h>

NAMESPACE_START

template<typename T>
class BlockingQueue : Uncopyable
{
public:
    BlockingQueue()
        : mutex_(),
        notEmpty_(mutex_),
        queue_()
    {
    }

    void put(const T& x)
    {
        MutexLockGuard lock(mutex_);
        queue_.push_back(x);
        notEmpty_.notify(); // wait morphing saves us
        // http://www.domaigne.com/blog/computing/condvars-signal-with-mutex-locked-or-not/
    }

    void put(T&& x)
    {
        MutexLockGuard lock(mutex_);
        queue_.push_back(std::move(x));
        notEmpty_.notify();
    }

    T take()
    {
        MutexLockGuard lock(mutex_);
        // always use a while-loop, due to spurious wakeup
        while (queue_.empty())
        {
        notEmpty_.wait();
        }
        assert(!queue_.empty());
        T front(std::move(queue_.front()));
        queue_.pop_front();
        return std::move(front);
    }

    size_t size() const
    {
        MutexLockGuard lock(mutex_);
        return queue_.size();
    }

private:
    mutable MutexLock mutex_;
    Condition         notEmpty_ GUARDED_BY(mutex_);
    std::deque<T>     queue_ GUARDED_BY(mutex_);
};

NAMESPACE_END

#endif  // MUDUO_BASE_BLOCKINGQUEUE_H
