#include "pch.h"

void CQuestMan::CreateInstance() {
    ((VOID * *(_fastcall * )())
    0x00ADC9DC)();
}

CQuestMan *CQuestMan::GetInstance() {
    return reinterpret_cast<CQuestMan *>(*(void **) 0x00CD73A0);
}

int CQuestMan::LoadDemand() {
    return ((int(_fastcall * )(CQuestMan * , PVOID))
    0x00784129)(this, nullptr);
}

void CQuestMan::LoadPartyQuestInfo() {
    ((VOID(_fastcall * )(CQuestMan * , PVOID))
    0x00789D21)(this, nullptr);
}

typedef VOID(__thiscall *_CQuestMan__LoadExclusive_t)(CQuestMan *pThis);
_CQuestMan__LoadExclusive_t _CQuestMan__LoadExclusive = reinterpret_cast<_CQuestMan__LoadExclusive_t>(0x0078B181);

void CQuestMan::LoadExclusive() {
    _CQuestMan__LoadExclusive(this);
}