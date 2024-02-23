#include "pch.h"

ZSocketBuffer *ZSocketBuffer::Alloc(unsigned int u) {
    return ((ZSocketBuffer *(_cdecl * )(unsigned int u))
    0x004A7FDE)(u);
}