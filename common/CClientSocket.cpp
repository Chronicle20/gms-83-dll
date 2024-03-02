#include "pch.h"

CClientSocket *CClientSocket::GetInstance() {
    return reinterpret_cast<CClientSocket *>(*(void **) 0x00CD11F8);
}

typedef VOID(__cdecl *_CClientSocket__CreateInstance_t)();
_CClientSocket__CreateInstance_t _CClientSocket__CreateInstance = reinterpret_cast<_CClientSocket__CreateInstance_t>(0x00ADCD78);

void CClientSocket::CreateInstance() {
    _CClientSocket__CreateInstance();
}

// void __fastcall CClientSocket::SendPacket(CClientSocket *this, int a2, COutPacket *oPacket)
// typedef VOID(__fastcall *_CClientSocket__SendPacket_t)(CClientSocket *pThis, PVOID edx, COutPacket *oPacket);
// _CClientSocket__SendPacket_t _CClientSocket__SendPacket = reinterpret_cast<_CClientSocket__SendPacket_t>(0x0049637B);
void CClientSocket::SendPacket(COutPacket *oPacket) {
    ((VOID(_fastcall * )(CClientSocket * , PVOID, COutPacket*))
    0x004B14F7)(this, nullptr, oPacket);
}

void CClientSocket::ManipulatePacket() {
    ((VOID(_fastcall * )(CClientSocket * , PVOID))
    0x004B1717)(this, nullptr);
}

void CClientSocket::Close() {
    ((VOID(_fastcall * )(CClientSocket * , PVOID))
    0x004B14E5)(this, nullptr);
}

void CClientSocket::ClearSendReceiveCtx() {
    ((VOID(_fastcall * )(CClientSocket * , PVOID))
    0x004B1CD5)(this, nullptr);
}