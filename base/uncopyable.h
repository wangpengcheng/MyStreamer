#ifndef UNCOPYABLE_H
#define UNCOPYABLE_H
#include "base_define.h"
NAMESPACE_START

class Uncopyable
{
protected:
	Uncopyable() { }
	~Uncopyable() { }

private:
    Uncopyable( const Uncopyable& ) = delete;
    Uncopyable& operator= ( const Uncopyable& ) = delete;
};

NAMESPACE_END

#endif  //UNCOPYABLE_H
