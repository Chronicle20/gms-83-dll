#include "pch.h"

CActionMan *CActionMan::GetInstance() {
    return reinterpret_cast<CActionMan *>(*(void **) 0x00BE78D4);
}

// void __thiscall CActionMan::SweepCache(CActionMan *this)
void CActionMan::SweepCache() {
    ((VOID(_fastcall * )(CActionMan * , PVOID))
    0x00411BBB)(this, NULL);
}