#include "pch.h"

typedef VOID(__cdecl *_CQuestMan__CreateInstance_t)();
_CQuestMan__CreateInstance_t _CQuestMan__CreateInstance = reinterpret_cast<_CQuestMan__CreateInstance_t>(0x00ADC9DC);

void CQuestMan::CreateInstance() {
    _CQuestMan__CreateInstance();
}

CQuestMan *CQuestMan::GetInstance() {
    return reinterpret_cast<CQuestMan *>(*(void **) 0x00CD73A0);
}

typedef int(__thiscall *_CQuestMan__LoadDemand_t)(CQuestMan *pThis);
_CQuestMan__LoadDemand_t _CQuestMan__LoadDemand = reinterpret_cast<_CQuestMan__LoadDemand_t>(0x00784129);

int CQuestMan::LoadDemand() {
    return _CQuestMan__LoadDemand(this);
}

typedef VOID(__thiscall *_CQuestMan__LoadPartyQuestInfo_t)(CQuestMan *pThis);
_CQuestMan__LoadPartyQuestInfo_t _CQuestMan__LoadPartyQuestInfo = reinterpret_cast<_CQuestMan__LoadPartyQuestInfo_t>(0x00789D21);

void CQuestMan::LoadPartyQuestInfo() {
    _CQuestMan__LoadPartyQuestInfo(this);
}

typedef VOID(__thiscall *_CQuestMan__LoadExclusive_t)(CQuestMan *pThis);
_CQuestMan__LoadExclusive_t _CQuestMan__LoadExclusive = reinterpret_cast<_CQuestMan__LoadExclusive_t>(0x0078B181);

void CQuestMan::LoadExclusive() {
    _CQuestMan__LoadExclusive(this);
}