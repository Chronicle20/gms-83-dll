#include "pch.h"

CClientSocket *CClientSocket::GetInstance() {
    Log("CClientSocket::GetInstance");
    return reinterpret_cast<CClientSocket *>(*reinterpret_cast<void **>(
            C_CLIENT_SOCKET_GET_INSTANCE));
}

void CClientSocket::CreateInstance() {
    Log("CClientSocket::CreateInstance");
    reinterpret_cast<void (__fastcall *)()>(
            C_CLIENT_SOCKET_CREATE_INSTANCE)();
}

void CClientSocket::SendPacket(COutPacket *oPacket) {
    Log("CClientSocket::SendPacket");
    reinterpret_cast<void (__fastcall *)(CClientSocket *, void *, COutPacket *)>(
            C_CLIENT_SOCKET_SEND_PACKET)(this, nullptr, oPacket);
}

void CClientSocket::ManipulatePacket() {
    Log("CClientSocket::ManipulatePacket");
    reinterpret_cast<void (__fastcall *)(CClientSocket *, void *)>(
            C_CLIENT_SOCKET_MANIPULATE_PACKET)(this, nullptr);
}

void CClientSocket::Close() {
    Log("CClientSocket::Close");
    reinterpret_cast<void (__fastcall *)(CClientSocket *, void *)>(
            C_CLIENT_SOCKET_CLOSE)(this, nullptr);
    Log("CClientSocket::After Close");
}

void CClientSocket::ClearSendReceiveCtx() {
    Log("CClientSocket::ClearSendReceiveCtx");
    reinterpret_cast<void (__fastcall *)(CClientSocket *, void *)>(
            C_CLIENT_SOCKET_CLEAR_SEND_RECEIVE_CTX)(this, nullptr);
}

void CClientSocket::ConnectLogin() {
    Log("CClientSocket::ConnectLogin");
    reinterpret_cast<void (__fastcall *)(CClientSocket *, void *)>(
            C_CLIENT_SOCKET_CONNECT_LOGIN)(this, nullptr);
}