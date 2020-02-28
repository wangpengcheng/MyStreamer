// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef BASE_THREAD_H
#define BASE_THREAD_H


#include "base_atomic.h"
#include "count_downlatch.h"
#include "base_types.h"

#include <functional>
#include <memory>
#include <pthread.h>

NAMESPACE_START

class Thread : Uncopyable
{
public:
  typedef std::function<void ()> ThreadFunc;

  explicit Thread(ThreadFunc, const string& name = string());
  // FIXME: make it movable in C++11
  ~Thread();

  void start();
  int join(); // return pthread_join()

  bool started() const { return started_; }
  /* 获取线程id */
  pid_t tid() const { return tid_; }
  const string& name() const { return name_; }

  static int numCreated() { return numCreated_.get(); }

 private:
  void setDefaultName();

  bool       started_;
  bool       joined_;
  pthread_t  pthreadId_;
  pid_t      tid_;
  ThreadFunc func_;
  string     name_;
  CountDownLatch latch_;        /* 同步计时器 */

  static AtomicInt32 numCreated_;
};

NAMESPACE_END
#endif  // BASE_THREAD_H
