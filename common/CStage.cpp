#include "pch.h"

void CStage::OnMouseEnter(int bEnter) {
    ((VOID(_fastcall * )(CStage * , PVOID, int))
    0x007C4241)(this, NULL, bEnter);
}

void CStage::OnPacket(int nType, CInPacket * iPacket) {
    ((VOID(_fastcall * )(CStage * , PVOID, int, CInPacket *))
    0x007C4260)(this, NULL, nType, iPacket);
}
