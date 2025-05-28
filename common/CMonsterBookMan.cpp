#include "pch.h"

void CMonsterBookMan::CreateInstance() {
    Log("CMonsterBookMan::CreateInstance");
    reinterpret_cast<void (__fastcall *)()>(C_MONSTER_BOOK_MAN_CREATE_INSTANCE)();
}

CMonsterBookMan *CMonsterBookMan::GetInstance() {
    return reinterpret_cast<CMonsterBookMan *>(*reinterpret_cast<void **>(C_MONSTER_BOOK_MAN_INSTANCE_ADDR));
}

bool CMonsterBookMan::LoadBook() {
    Log("CMonsterBookMan::LoadBook");
    return reinterpret_cast<bool (__fastcall *)(CMonsterBookMan *, void *)>(
            C_MONSTER_BOOK_MAN_LOAD_BOOK)(this, nullptr);
}