#include "ZAllocEx.h"

void* operator new(size_t uSize)
{
	return ZAllocEx<ZAllocAnonSelector>::GetInstance()->Alloc(uSize);
}

void* operator new[](size_t uSize)
{
	return ZAllocEx<ZAllocAnonSelector>::GetInstance()->Alloc(uSize);
}

void operator delete(void* p)
{
	ZAllocEx<ZAllocAnonSelector>::GetInstance()->Free((void**)p);
}

void operator delete[](void* p)
{
	ZAllocEx<ZAllocAnonSelector>::GetInstance()->Free((void**)p);
}
