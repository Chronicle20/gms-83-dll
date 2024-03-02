#include "pch.h"

void CActionMan::CreateInstance() {
    ((VOID * *(_fastcall * )())
    0x00ADCCCB)();
}

CActionMan *CActionMan::GetInstance() {
    return reinterpret_cast<CActionMan *>(*(void **) 0x00CD11AC);
}

void CActionMan::Init() {
    ((VOID(_fastcall * )(CActionMan * , PVOID))
    0x0040705D)(this, nullptr);
}

// void __thiscall CActionMan::SweepCache(CActionMan *this)
void CActionMan::SweepCache() {
    ((VOID(_fastcall * )(CActionMan * , PVOID))
    0x004123D3)(this, nullptr);
}