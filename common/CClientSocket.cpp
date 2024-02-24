#include "pch.h"

CClientSocket *CClientSocket::GetInstance() {
    return reinterpret_cast<CClientSocket *>(*(void **) 0x00C9A004);
}

void CClientSocket::CreateInstance() {
    ((VOID * *(_fastcall * )())
    0x00A8E4BF)();
}

// void __fastcall CClientSocket::SendPacket(CClientSocket *this, int a2, COutPacket *oPacket)
// typedef VOID(__fastcall *_CClientSocket__SendPacket_t)(CClientSocket *pThis, PVOID edx, COutPacket *oPacket);
// _CClientSocket__SendPacket_t _CClientSocket__SendPacket = reinterpret_cast<_CClientSocket__SendPacket_t>(0x0049637B);
void CClientSocket::SendPacket(COutPacket *oPacket) {
    ((VOID(_fastcall * )(CClientSocket * , PVOID, COutPacket*))
    0x004A83AC)(this, nullptr, oPacket);
}

void CClientSocket::ManipulatePacket() {
    ((VOID(_fastcall * )(CClientSocket * , PVOID))
    0x004A854E)(this, nullptr);
}

void CClientSocket::Close() {
    ((VOID(_fastcall * )(CClientSocket * , PVOID))
    0x004A839A)(this, nullptr);
}

void CClientSocket::ClearSendReceiveCtx() {
    ((VOID(_fastcall * )(CClientSocket * , PVOID))
    0x004A8A51)(this, nullptr);
}