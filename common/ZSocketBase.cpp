#include "pch.h"

void ZSocketBase::CloseSocket() {
    ((VOID(_fastcall * )(ZSocketBase * , PVOID))
    0x004A67C5)(this, nullptr);
}