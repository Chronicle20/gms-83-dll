#include "pch.h"

void CQuestMan::CreateInstance() {
    Log("CQuestMan::CreateInstance");
    ((VOID * *(_fastcall * )())
    C_QUEST_MAN_CREATE_INSTANCE)();
}

CQuestMan *CQuestMan::GetInstance() {
    return reinterpret_cast<CQuestMan *>(*(void **) C_QUEST_MAN_GET_INSTANCE);
}

int CQuestMan::LoadDemand() {
    Log("CQuestMan::LoadDemand");
    return ((int(_fastcall * )(CQuestMan * , PVOID))
    C_QUEST_MAN_LOAD_DEMAND)(this, nullptr);
}

void CQuestMan::LoadPartyQuestInfo() {
    Log("CQuestMan::LoadPartyQuestInfo");
    ((VOID(_fastcall * )(CQuestMan * , PVOID))
    C_QUEST_MAN_LOAD_PARTY_QUEST_INFO)(this, nullptr);
}

void CQuestMan::LoadExclusive() {
    Log("CQuestMan::LoadExclusive");
    ((VOID(_fastcall * )(CQuestMan * , PVOID))
    C_QUEST_MAN_LOAD_EXCLUSIVE)(this, nullptr);
}