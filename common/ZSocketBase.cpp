#include "pch.h"

#include <winsock2.h>

void ZSocketBase::CloseSocket() {
#if Z_SOCKET_BASE_CLOSE_SOCKET != 0
    reinterpret_cast<void(__fastcall*)(ZSocketBase*, void*)>(
            Z_SOCKET_BASE_CLOSE_SOCKET)(this, nullptr);
#else
    // v72 (and older): there is NO standalone ZSocketBase::CloseSocket — the binary
    // inlines the teardown at every call site (Close/Connect(sockaddr_in)/~CClientSocket):
    // closesocket(_m_hSocket) then set the handle to -1, with no shutdown() call.
    // Z_SOCKET_BASE_CLOSE_SOCKET is the 0x0 absent-sentinel here, so calling through it
    // jumps to address 0 and AVs — which crashes CClientSocket::Connect right after
    // ClearSendReceiveCtx (before the socket is ever created), so the client never reaches
    // the login window. Replicate the inline teardown instead. task-009; flagged in
    // memory_maps/GMS/v72_1.cmake (Z_SOCKET_BASE_CLOSE_SOCKET).
    if (_m_hSocket != INVALID_SOCKET) {
        closesocket(_m_hSocket);
        _m_hSocket = INVALID_SOCKET;
    }
#endif
}
