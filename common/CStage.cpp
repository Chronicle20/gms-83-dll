#include "pch.h"
#include "memory_map.h"

void CStage::OnMouseEnter(int bEnter) {
    ((VOID(_fastcall * )(CStage * , PVOID, int))
    C_STAGE_ON_MOUSE_ENTER)(this, NULL, bEnter);
}

void CStage::OnPacket(int nType, CInPacket * iPacket) {
    ((VOID(_fastcall * )(CStage * , PVOID, int, CInPacket *))
    C_STAGE_ON_PACKET)(this, NULL, nType, iPacket);
}
