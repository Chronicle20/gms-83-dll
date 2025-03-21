#include "pch.h"
#include "memory_map.h"

COutPacket::COutPacket(INT nType) {
    ((VOID(_fastcall * )(COutPacket * , PVOID, INT))
    C_OUT_PACKET)(this, NULL, nType);
}

void COutPacket::Encode1(unsigned char n) {
    ((VOID(_fastcall * )(COutPacket * , PVOID, unsigned __int8))
    C_OUT_PACKET_ENCODE_1)(this, NULL, n);
}

void COutPacket::Encode4(UINT n) {
    ((VOID(_fastcall * )(COutPacket * , PVOID, unsigned int))
    C_OUT_PACKET_ENCODE_4)(this, NULL, n);
}

void COutPacket::EncodeStr(ZXString<char> s) {
    ((VOID(_fastcall * )(COutPacket * , PVOID, ZXString<char>
    s))
    C_OUT_PACKET_ENCODE_STR)(this, NULL, s);
}

void COutPacket::EncodeBuffer(const void *p, unsigned int uSize) {
    ((VOID(_fastcall * )(COutPacket * , PVOID,
    const void *p,
    unsigned int uSize))
    C_OUT_PACKET_BUFFER)(this, NULL, p, uSize);
}

COutPacket::~COutPacket() {
    this->m_aSendBuff.RemoveAll();
}
