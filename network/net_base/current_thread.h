#ifndef BASE_CURRENTTHREAD_H
#define BASE_CURRENTTHREAD_H

#include "base_types.h"
#include <string>
NAMESPACE_START
namespace CurrentThread
{
 // internal
  // internal
  extern __thread int t_cachedTid;
  extern __thread char t_tidString[32];
  extern __thread int t_tidStringLength;
  extern __thread const char* t_threadName;
  void cacheTid();

  inline int tid()
  {
    if (__builtin_expect(t_cachedTid == 0, 0))
    {
      cacheTid();
    }
    return t_cachedTid;
  }

  inline const char* tidString() // for logging
  {
    return t_tidString;
  }

  inline int tidStringLength() // for logging
  {
    return t_tidStringLength;
  }

  inline const char* name()
  {
    return t_threadName;
  }
/* 检查主线程 */
bool isMainThread();
/* 睡眠时间 */
void sleepUsec(int64_t usec);  // for testing

std::string stackTrace(bool demangle);
} //end current thread
NAMESPACE_END
#endif