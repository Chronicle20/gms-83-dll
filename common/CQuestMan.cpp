#include "pch.h"

void CQuestMan::CreateInstance() {
    ((VOID * *(_fastcall * )())
    0x00A8E12E)();
}

CQuestMan *CQuestMan::GetInstance() {
    return reinterpret_cast<CQuestMan *>(*(void **) 0x00CA0130);
}

int CQuestMan::LoadDemand() {
    return ((int(_fastcall * )(CQuestMan * , PVOID))
    0x00762DD4)(this, nullptr);
}

void CQuestMan::LoadPartyQuestInfo() {
    ((VOID(_fastcall * )(CQuestMan * , PVOID))
    0x00768B2E)(this, nullptr);
}

void CQuestMan::LoadExclusive() {
    ((VOID(_fastcall * )(CQuestMan * , PVOID))
    0x00769F8E)(this, nullptr);
}