#include "pch.h"

void CStage::OnMouseEnter(int bEnter) {
    ((VOID(_fastcall * )(CStage * , PVOID, int))
    0x007EEA10)(this, nullptr, bEnter);
}

void CStage::OnPacket(int nType, CInPacket * iPacket) {
    ((VOID(_fastcall * )(CStage * , PVOID, int, CInPacket *))
    0x007EEA2F)(this, nullptr, nType, iPacket);
}
