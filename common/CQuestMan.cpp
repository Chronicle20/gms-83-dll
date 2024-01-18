#include "pch.h"

void CQuestMan::CreateInstance() {
    ((VOID * *(_fastcall * )())
    0x009F9AC2)();
}

CQuestMan *CQuestMan::GetInstance() {
    return reinterpret_cast<CQuestMan *>(*(void **) 0x00BED614);
}

int CQuestMan::LoadDemand() {
    return ((int(_fastcall * )(CQuestMan * , PVOID))
    0x0071D8DF)(this, NULL);
}

void CQuestMan::LoadPartyQuestInfo() {
    ((VOID(_fastcall * )(CQuestMan * , PVOID))
    0x00723341)(this, NULL);
}

void CQuestMan::LoadExclusive() {
    ((VOID(_fastcall * )(CQuestMan * , PVOID))
    0x007247A1)(this, NULL);
}