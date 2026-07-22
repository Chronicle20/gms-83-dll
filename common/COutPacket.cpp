#include "pch.h"

COutPacket::COutPacket(INT nType) {
#if defined(REGION_GMS) && BUILD_MAJOR_VERSION < 72
    // v61 ONLY: the COutPacket ctor takes a SECOND stack arg (bLoopback, written to
    // [this+0]) and does `retn 8`. v72+ dropped it -> `COutPacket(nType)` / `retn 4`.
    // Passing one arg through the v72-shaped cast leaves the callee cleaning 8 bytes
    // while we push 4 -> ESP mismatch -> Run-Time Check Failure #0 the moment the login
    // packet is built (CLogin::SendCheckPasswordPacket). Pass bLoopback=0 explicitly so
    // the pushed/cleaned byte counts agree. Verified against GMS_v61.1_U_DEVM (ctor
    // @0x5FFC4F reads arg_0=nType + arg_4=loopback, retn 8; call site @0x564471 pushes
    // both). task-010.
    reinterpret_cast<void(__fastcall*)(COutPacket*, void*, INT, INT)>(C_OUT_PACKET)(this, nullptr, nType, 0);
#else
    reinterpret_cast<void(__fastcall*)(COutPacket*, void*, INT)>(C_OUT_PACKET)(this, nullptr, nType);
#endif
}

void COutPacket::Encode1(unsigned char n) {
    reinterpret_cast<void (__fastcall *)(COutPacket *, void *, unsigned char)>(
            C_OUT_PACKET_ENCODE_1)(this, nullptr, n);
}

void COutPacket::Encode2(unsigned short n) {
    reinterpret_cast<void (__fastcall *)(COutPacket *, void *, unsigned int)>(
            C_OUT_PACKET_ENCODE_2)(this, nullptr, n);
}

void COutPacket::Encode4(UINT n) {
    reinterpret_cast<void (__fastcall *)(COutPacket *, void *, unsigned int)>(
            C_OUT_PACKET_ENCODE_4)(this, nullptr, n);
}

void COutPacket::EncodeStr(ZXString<char> s) {
    reinterpret_cast<void (__fastcall *)(COutPacket *, void *, ZXString<char>)>(
            C_OUT_PACKET_ENCODE_STR)(this, nullptr, s);
}

void COutPacket::EncodeBuffer(const void *p, unsigned int uSize) {
    reinterpret_cast<void (__fastcall *)(COutPacket *, void *, const void *, unsigned int)>(
            C_OUT_PACKET_ENCODE_BUFFER)(this, nullptr, p, uSize);
}

void COutPacket::MakeBufferList(ZList<ZRef<ZSocketBuffer>>* l, unsigned short uSeqBase, unsigned int* puSeqKey,
                                int bEnc, unsigned int dwKey) const {
    reinterpret_cast<void(__fastcall*)(const COutPacket*, void*, ZList<ZRef<ZSocketBuffer>>*, unsigned short,
                                       unsigned int*, int, unsigned int)>(C_OUT_PACKET_MAKE_BUFFER_LIST)(
        this, nullptr, l, uSeqBase, puSeqKey, bEnc, dwKey);
}

COutPacket::~COutPacket() {
    this->m_aSendBuff.RemoveAll();
}
