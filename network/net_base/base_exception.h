// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef BASE_EXCEPTION_H
#define BASE_EXCEPTION_H

#include "base_types.h"

#include <exception>

NAMESPACE_START

class Exception : public std::exception
{
 public:
  Exception(string what);
  /* 禁止函数重载 */
  ~Exception() noexcept override = default;

  // default copy-ctor and operator= are okay.

  const char* what() const noexcept override
  {
    return message_.c_str();
  }

  const char* stackTrace() const noexcept
  {
    return stack_.c_str();
  }

 private:
  std::string message_;
  std::string stack_;
};


NAMESPACE_END

#endif  // BASE_EXCEPTION_H
