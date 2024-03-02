#include "pch.h"

ZSocketBuffer *ZSocketBuffer::Alloc(unsigned int u) {
    return ((ZSocketBuffer *(_cdecl * )(unsigned int u))
    0x004B1132)(u);
}