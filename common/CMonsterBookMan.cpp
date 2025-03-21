#include "pch.h"
#include "memory_map.h"

void CMonsterBookMan::CreateInstance() {
    ((VOID * *(_fastcall * )())
    C_MONSTER_BOOK_MAN_CREATE_INSTANCE)();
}

CMonsterBookMan *CMonsterBookMan::GetInstance() {
    return reinterpret_cast<CMonsterBookMan *>(*(void **) C_MONSTER_BOOK_MAN_GET_INSTANCE);
}

bool CMonsterBookMan::LoadBook() {
    return ((bool(_fastcall * )(CMonsterBookMan * , PVOID))
    C_MONSTER_BOOK_MAN_LOAD_BOOK)(this, NULL);
}