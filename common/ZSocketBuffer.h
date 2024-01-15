#pragma once

#include <winsock2.h>
#include <Windows.h>
#include "ZRef.h"
#include "ZRefCounted.h"

struct ZSocketBuffer : ZRefCounted, _WSABUF, ZRefCountedAccessorBase {
    ZRef<ZSocketBuffer> _m_pParent;
};