#include "pch.h"

void CStage::OnMouseEnter(int bEnter) {
    ((VOID(_fastcall * )(CStage * , PVOID, int))
    0x00775FC7)(this, NULL, bEnter);
}

void CStage::OnPacket(int nType, CInPacket * iPacket) {
    ((VOID(_fastcall * )(CStage * , PVOID, int, CInPacket *))
    0x00775FE6)(this, NULL, nType, iPacket);
}
