#include "pch.h"

ZSocketBuffer *ZSocketBuffer::Alloc(unsigned int u) {
    return reinterpret_cast<ZSocketBuffer *(__cdecl *)(unsigned int)>(
            Z_SOCKET_BUFFER_ALLOC)(u);
}