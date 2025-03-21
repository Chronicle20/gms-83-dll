#include "pch.h"
#include "memory_map.h"

void CQuestMan::CreateInstance() {
    ((VOID * *(_fastcall * )())
    C_QUEST_MAN_CREATE_INSTANCE)();
}

CQuestMan *CQuestMan::GetInstance() {
    return reinterpret_cast<CQuestMan *>(*(void **) C_QUEST_MAN_GET_INSTANCE);
}

int CQuestMan::LoadDemand() {
    return ((int(_fastcall * )(CQuestMan * , PVOID))
    C_QUEST_MAN_LOAD_DEMAND)(this, NULL);
}

void CQuestMan::LoadPartyQuestInfo() {
    ((VOID(_fastcall * )(CQuestMan * , PVOID))
    C_QUEST_MAN_LOAD_PARTY_QUEST_INFO)(this, NULL);
}

void CQuestMan::LoadExclusive() {
    ((VOID(_fastcall * )(CQuestMan * , PVOID))
    C_QUEST_MAN_LOAD_EXCLUSIVE)(this, NULL);
}