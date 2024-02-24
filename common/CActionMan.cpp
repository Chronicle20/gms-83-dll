#include "pch.h"

void CActionMan::CreateInstance() {
    ((VOID * *(_fastcall * )())
    0x00A8E412)();
}

CActionMan *CActionMan::GetInstance() {
    return reinterpret_cast<CActionMan *>(*(void **) 0x00C99FC4);
}

void CActionMan::Init() {
    ((VOID(_fastcall * )(CActionMan * , PVOID))
    0x00406D03)(this, nullptr);
}

// void __thiscall CActionMan::SweepCache(CActionMan *this)
void CActionMan::SweepCache() {
    ((VOID(_fastcall * )(CActionMan * , PVOID))
    0x00411EEF)(this, nullptr);
}