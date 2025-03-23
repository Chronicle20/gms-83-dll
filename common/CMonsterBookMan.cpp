#include "pch.h"

void CMonsterBookMan::CreateInstance() {
    Log("CMonsterBookMan::CreateInstance");
    ((VOID * *(_fastcall * )())
    C_MONSTER_BOOK_MAN_CREATE_INSTANCE)();
}

CMonsterBookMan *CMonsterBookMan::GetInstance() {
    return reinterpret_cast<CMonsterBookMan *>(*(void **) C_MONSTER_BOOK_MAN_GET_INSTANCE);
}

bool CMonsterBookMan::LoadBook() {
    Log("CMonsterBookMan::LoadBook");
    return ((bool(_fastcall * )(CMonsterBookMan * , PVOID))
    C_MONSTER_BOOK_MAN_LOAD_BOOK)(this, NULL);
}