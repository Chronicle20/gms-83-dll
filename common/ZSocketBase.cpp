#include "pch.h"

void ZSocketBase::CloseSocket() {
    ((VOID(_fastcall * )(ZSocketBase * , PVOID))
    0x004AFC92)(this, nullptr);
}