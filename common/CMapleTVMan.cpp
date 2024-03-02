#include "pch.h"

void CMapleTVMan::CreateInstance() {
    ((VOID * *(_fastcall * )())
    0x00ADCF23)();
}

CMapleTVMan *CMapleTVMan::GetInstance() {
    return reinterpret_cast<CMapleTVMan *>(*(void **) 0x00CD7578);
}

typedef VOID(__thiscall *_CMapleTVMan__Init_t)(CMapleTVMan *pThis, PVOID edx, PVOID another);
_CMapleTVMan__Init_t _CMapleTVMan__Init = reinterpret_cast<_CMapleTVMan__Init_t>(0x006AAE8F);

void CMapleTVMan::Init() {
    _CMapleTVMan__Init(this, nullptr, nullptr);
}
