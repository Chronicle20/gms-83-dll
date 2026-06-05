#include "pch.h"

unsigned short CInPacket::Decode2() {
    return reinterpret_cast<unsigned short(__fastcall *)(CInPacket *, void *)>(
        C_IN_PACKET_DECODE2)(this, nullptr);
}
