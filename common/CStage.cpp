#include "pch.h"

void CStage::OnMouseEnter(int bEnter) {
    reinterpret_cast<void (__fastcall *)(CStage *, void *, int)>(
            C_STAGE_ON_MOUSE_ENTER)(this, nullptr, bEnter);
}

void CStage::OnPacket(int nType, CInPacket *iPacket) {
    reinterpret_cast<void (__fastcall *)(CStage *, void *, int, CInPacket *)>(
            C_STAGE_ON_PACKET)(this, nullptr, nType, iPacket);
}
