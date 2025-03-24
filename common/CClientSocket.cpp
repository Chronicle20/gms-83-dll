#include "pch.h"

CClientSocket *CClientSocket::GetInstance() {
    Log("CClientSocket::GetInstance");
    return reinterpret_cast<CClientSocket *>(*(void **) C_CLIENT_SOCKET_GET_INSTANCE);
}

void CClientSocket::CreateInstance() {
    Log("CClientSocket::CreateInstance");
    ((VOID * *(_fastcall * )())
    C_CLIENT_SOCKET_CREATE_INSTANCE)();
}

void CClientSocket::SendPacket(COutPacket *oPacket) {
    Log("CClientSocket::SendPacket");
    ((VOID(_fastcall * )(CClientSocket * , PVOID, COutPacket * ))
    C_CLIENT_SOCKET_SEND_PACKET)(this, nullptr, oPacket);
}

void CClientSocket::ManipulatePacket() {
    Log("CClientSocket::ManipulatePacket");
    ((VOID(_fastcall * )(CClientSocket * , PVOID))
    C_CLIENT_SOCKET_MANIPULATE_PACKET)(this, nullptr);
}

void CClientSocket::Close() {
    Log("CClientSocket::Close");
    ((VOID(_fastcall * )(CClientSocket * , PVOID))
    C_CLIENT_SOCKET_CLOSE)(this, nullptr);
}

void CClientSocket::ClearSendReceiveCtx() {
    Log("CClientSocket::ClearSendReceiveCtx");
    ((VOID(_fastcall * )(CClientSocket * , PVOID))
    C_CLIENT_SOCKET_CLEAR_SEND_RECEIVE_CTX)(this, nullptr);
}

void CClientSocket::ConnectLogin() {
    Log("CClientSocket::ConnectLogin");
    ((VOID(_fastcall * )(CClientSocket * , PVOID))
    C_CLIENT_SOCKET_CONNECT_LOGIN)(this, nullptr);
}