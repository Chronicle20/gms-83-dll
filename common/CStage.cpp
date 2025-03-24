#include "pch.h"

void CStage::OnMouseEnter(int bEnter) {
    ((VOID(_fastcall * )(CStage * , PVOID, int))
    C_STAGE_ON_MOUSE_ENTER)(this, nullptr, bEnter);
}

void CStage::OnPacket(int nType, CInPacket * iPacket) {
    ((VOID(_fastcall * )(CStage * , PVOID, int, CInPacket *))
    C_STAGE_ON_PACKET)(this, nullptr, nType, iPacket);
}
