#include "pch.h"

typedef VOID(__cdecl *_CMapleTVMan__CreateInstance_t)();
_CMapleTVMan__CreateInstance_t _CMapleTVMan__CreateInstance = reinterpret_cast<_CMapleTVMan__CreateInstance_t>(0x00ADCF23);

void CMapleTVMan::CreateInstance() {
    _CMapleTVMan__CreateInstance();
}

CMapleTVMan *CMapleTVMan::GetInstance() {
    return reinterpret_cast<CMapleTVMan *>(*(void **) 0x00CD7578);
}

typedef VOID(__thiscall *_CMapleTVMan__Init_t)(CMapleTVMan *pThis, int edx, int another);
_CMapleTVMan__Init_t _CMapleTVMan__Init = reinterpret_cast<_CMapleTVMan__Init_t>(0x006AAE8F);

void CMapleTVMan::Init(int something, int somethingElse) {
    _CMapleTVMan__Init(this, something, somethingElse);
}
