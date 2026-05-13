#include "pch.h"

COutPacket::COutPacket(INT nType) {
    reinterpret_cast<void (__fastcall *)(COutPacket *, void *, INT)>(
            C_OUT_PACKET)(this, nullptr, nType);
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

void COutPacket::MakeBufferList(ZList<ZRef<ZSocketBuffer>>* l, unsigned short uSeqBase,
                                unsigned int* puSeqKey, int bEnc, unsigned int dwKey) const {
    reinterpret_cast<void(__fastcall*)(const COutPacket*, void*,
                                       ZList<ZRef<ZSocketBuffer>>*,
                                       unsigned short, unsigned int*, int, unsigned int)>(
            C_OUT_PACKET_MAKE_BUFFER_LIST)(this, nullptr, l, uSeqBase, puSeqKey, bEnc, dwKey);
}

COutPacket::~COutPacket() {
    this->m_aSendBuff.RemoveAll();
}
