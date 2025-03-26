#include "pch.h"

COutPacket::COutPacket(INT nType) {
    ((VOID(_fastcall * )(COutPacket * , PVOID, INT))
    C_OUT_PACKET)(this, nullptr, nType);
}

void COutPacket::Encode1(unsigned char n) {
    ((VOID(_fastcall * )(COutPacket * , PVOID, unsigned __int8))
    C_OUT_PACKET_ENCODE_1)(this, nullptr, n);
}

void COutPacket::Encode2(unsigned short n) {
    ((VOID(_fastcall * )(COutPacket * , PVOID, unsigned int))
    C_OUT_PACKET_ENCODE_2)(this, nullptr, n);
}

void COutPacket::Encode4(UINT n) {
    ((VOID(_fastcall * )(COutPacket * , PVOID, unsigned int))
    C_OUT_PACKET_ENCODE_4)(this, nullptr, n);
}

void COutPacket::EncodeStr(ZXString<char> s) {
    ((VOID(_fastcall * )(COutPacket * , PVOID, ZXString<char>
    s))
    C_OUT_PACKET_ENCODE_STR)(this, nullptr, s);
}

void COutPacket::EncodeBuffer(const void *p, unsigned int uSize) {
    ((VOID(_fastcall * )(COutPacket * , PVOID,
    const void *p,
    unsigned int uSize))
    C_OUT_PACKET_ENCODE_BUFFER)(this, nullptr, p, uSize);
}

COutPacket::~COutPacket() {
    this->m_aSendBuff.RemoveAll();
}
