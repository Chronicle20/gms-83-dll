#include "pch.h"

CClientSocket *CClientSocket::GetInstance() {
    return reinterpret_cast<CClientSocket *>(*(void **) 0x00BE7914);
}

void CClientSocket::CreateInstance() {
    ((VOID * *(_fastcall * )())
    0x009F9E53)();
}

// void __fastcall CClientSocket::SendPacket(CClientSocket *this, int a2, COutPacket *oPacket)
// typedef VOID(__fastcall *_CClientSocket__SendPacket_t)(CClientSocket *pThis, PVOID edx, COutPacket *oPacket);
// _CClientSocket__SendPacket_t _CClientSocket__SendPacket = reinterpret_cast<_CClientSocket__SendPacket_t>(0x0049637B);
void CClientSocket::SendPacket(COutPacket *oPacket) {
    ((VOID(_fastcall * )(CClientSocket * , PVOID, COutPacket*))
    0x0049637B)(this, NULL, oPacket);
}

void CClientSocket::ManipulatePacket() {
    ((VOID(_fastcall * )(CClientSocket * , PVOID))
    0x0049651D)(this, NULL);
}