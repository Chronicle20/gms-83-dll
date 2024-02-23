#pragma once

struct ZSocketBuffer : ZRefCounted, _WSABUF, ZRefCountedAccessorBase {
    ZRef<ZSocketBuffer> _m_pParent;

    static ZSocketBuffer* Alloc(unsigned int u);
};