#include "pch.h"

void ZSocketBase::CloseSocket() {
    ((VOID(_fastcall * )(ZSocketBase * , PVOID))
    Z_SOCKET_BASE_CLOSE_SOCKET)(this, nullptr);
}