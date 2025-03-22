#include "pch.h"

ZSocketBuffer *ZSocketBuffer::Alloc(unsigned int u) {
    return ((ZSocketBuffer *(_cdecl * )(unsigned int u))
    Z_SOCKET_BUFFER_ALLOC)(u);
}