#include "pch.h"

typedef VOID(__thiscall *_CStage__OnMouseEnter_t)(CStage *pThis, int bEnter);
_CStage__OnMouseEnter_t _CStage__OnMouseEnter = reinterpret_cast<_CStage__OnMouseEnter_t>(0x007EEA10);

void CStage::OnMouseEnter(int bEnter) {
    _CStage__OnMouseEnter(this, bEnter);
}

typedef VOID(__thiscall *_CStage__OnPacket_t)(CStage *pThis, int nType, CInPacket * iPacket);
_CStage__OnPacket_t _CStage__OnPacket = reinterpret_cast<_CStage__OnPacket_t>(0x007EEA2F);

void CStage::OnPacket(int nType, CInPacket * iPacket) {
    _CStage__OnPacket(this, nType, iPacket);
}
