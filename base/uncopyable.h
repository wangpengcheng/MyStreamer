#ifndef UNCOPYABLE_H
#define UNCOPYABLE_H
#include "base_define.h"
NAMESPACE_START

class Uncopyable
{
protected:
	Uncopyable() = default;
	~Uncopyable() = default;/* 只能在堆上建立对象 */

private:
    Uncopyable( const Uncopyable& ) = delete;
    Uncopyable& operator= ( const Uncopyable& ) = delete;
};
typedef Uncopyable noncopyable;

class copyable
{
protected:
    copyable() = default;
    ~copyable() = default;
};

NAMESPACE_END

#endif  //UNCOPYABLE_H
