#include "pch.h"
#include "memory_map.h"

CClientSocket *CClientSocket::GetInstance() {
    return reinterpret_cast<CClientSocket *>(*(void **) C_CLIENT_SOCKET_GET_INSTANCE);
}

void CClientSocket::CreateInstance() {
    ((VOID * *(_fastcall * )())
    C_CLIENT_SOCKET_CREATE_INSTANCE)();
}

void CClientSocket::SendPacket(COutPacket *oPacket) {
    ((VOID(_fastcall * )(CClientSocket * , PVOID, COutPacket * ))
    C_CLIENT_SOCKET_SEND_PACKET)(this, NULL, oPacket);
}

void CClientSocket::ManipulatePacket() {
    ((VOID(_fastcall * )(CClientSocket * , PVOID))
    C_CLIENT_SOCKET_MANIPULATE_PACKET)(this, NULL);
}