#pragma once
#include "asserts.h"

struct ZRefCountedAccessorBase
{
};


template<class T>
class ZRefCountedAccessor : ZRefCountedAccessorBase
{

};

assert_size(sizeof(ZRefCountedAccessor<int>), 0x01);
assert_size(sizeof(ZRefCountedAccessorBase), 0x01);