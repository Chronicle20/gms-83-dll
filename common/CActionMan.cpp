#include "pch.h"

typedef VOID(__cdecl *_CActionMan__CreateInstance_t)();
_CActionMan__CreateInstance_t _CActionMan__CreateInstance = reinterpret_cast<_CActionMan__CreateInstance_t>(0x00ADCCCB);

void CActionMan::CreateInstance() {
    _CActionMan__CreateInstance();
}

CActionMan *CActionMan::GetInstance() {
    return reinterpret_cast<CActionMan *>(*(void **) 0x00CD11AC);
}

typedef VOID(__thiscall *_CActionMan__Init_t)(CActionMan *pThis);
_CActionMan__Init_t _CActionMan__Init = reinterpret_cast<_CActionMan__Init_t>(0x0040705D);

void CActionMan::Init() {
    _CActionMan__Init(this);
}

// void __thiscall CActionMan::SweepCache(CActionMan *this)
void CActionMan::SweepCache() {
    ((VOID(_fastcall * )(CActionMan * , PVOID))
    0x004123D3)(this, nullptr);
}