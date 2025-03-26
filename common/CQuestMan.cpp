#include "pch.h"

void CQuestMan::CreateInstance() {
    Log("CQuestMan::CreateInstance");
    reinterpret_cast<void (__fastcall *)()>(C_QUEST_MAN_CREATE_INSTANCE)();
}

CQuestMan *CQuestMan::GetInstance() {
    return reinterpret_cast<CQuestMan *>(*reinterpret_cast<void **>(C_QUEST_MAN_GET_INSTANCE));
}

int CQuestMan::LoadDemand() {
    Log("CQuestMan::LoadDemand");
    return reinterpret_cast<int (__fastcall *)(CQuestMan *, void *)>(
            C_QUEST_MAN_LOAD_DEMAND)(this, nullptr);
}

void CQuestMan::LoadPartyQuestInfo() {
    Log("CQuestMan::LoadPartyQuestInfo");
    reinterpret_cast<void (__fastcall *)(CQuestMan *, void *)>(
            C_QUEST_MAN_LOAD_PARTY_QUEST_INFO)(this, nullptr);
}

void CQuestMan::LoadExclusive() {
    Log("CQuestMan::LoadExclusive");
    reinterpret_cast<void (__fastcall *)(CQuestMan *, void *)>(
            C_QUEST_MAN_LOAD_EXCLUSIVE)(this, nullptr);
}