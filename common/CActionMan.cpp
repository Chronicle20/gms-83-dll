#include "pch.h"

void CActionMan::CreateInstance() {
    ((VOID * *(_fastcall * )())
    0x009F9DA6)();
}

CActionMan *CActionMan::GetInstance() {
    return reinterpret_cast<CActionMan *>(*(void **) 0x00BE78D4);
}

void CActionMan::Init() {
    ((VOID(_fastcall * )(CActionMan * , PVOID))
    0x00406ABD)(this, NULL);
}

// void __thiscall CActionMan::SweepCache(CActionMan *this)
void CActionMan::SweepCache() {
    ((VOID(_fastcall * )(CActionMan * , PVOID))
    0x00411BBB)(this, NULL);
}