#ifndef WEAK_CALLBACK_H
#define WEAK_CALLBACK_H

#include <functional>
#include <memory>
#include "base_tool.h"
NAMESPACE_START

// A barely usable WeakCallback
/*
wpc: 2019-11-27 23:23 
这个主要是使用C++的functional实现回调函数类抽象，
主要是，实现回调函数的晚绑定。
参考连接：
https://blog.csdn.net/fengbingchun/article/details/78006735
https://www.cnblogs.com/jerry-fuyi/p/11248665.html
https://zh.cppreference.com/w/cpp/header/functional
*/
template<typename CLASS, typename... ARGS>
class WeakCallback
{
 public:

  WeakCallback(const std::weak_ptr<CLASS>& object,
               const std::function<void (CLASS*, ARGS...)>& function)
    : object_(object), function_(function)
  {
  }

  // 设置默认操作函数；将对象和回调函数输入function_中

  void operator()(ARGS&&... args) const
  {
    std::shared_ptr<CLASS> ptr(object_.lock());
    if (ptr)
    {
        /* 这里将左值转发为右值 ；注意引用折叠；实现完美转发
            https://blog.csdn.net/theonegis/article/details/86568427
        */
      function_(ptr.get(), std::forward<ARGS>(args)...);
    }
    // else
    // {
    //   LOG_TRACE << "expired";
    // }
  }

 private:

  std::weak_ptr<CLASS> object_;                         /* 独占对象指针 */
  std::function<void (CLASS*, ARGS...)> function_;      /* 回调函数 */
};
/* 创建函数对象 */
template<typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> makeWeakCallback(const std::shared_ptr<CLASS>& object,
                                              void (CLASS::*function)(ARGS...))
{
  return WeakCallback<CLASS, ARGS...>(object, function);
}

template<typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> makeWeakCallback(const std::shared_ptr<CLASS>& object,
                                              void (CLASS::*function)(ARGS...) const)
{
  return WeakCallback<CLASS, ARGS...>(object, function);
}

NAMESPACE_END
#endif