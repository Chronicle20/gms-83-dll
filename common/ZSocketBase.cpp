#include "pch.h"

void ZSocketBase::CloseSocket() {
    reinterpret_cast<void(__fastcall*)(ZSocketBase*, void*)>(
            Z_SOCKET_BASE_CLOSE_SOCKET)(this, nullptr);
}