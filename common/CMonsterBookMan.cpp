#include "pch.h"

void CMonsterBookMan::CreateInstance() {
    ((VOID * *(_fastcall * )())
    0x00A8E1DF)();
}

CMonsterBookMan *CMonsterBookMan::GetInstance() {
    return reinterpret_cast<CMonsterBookMan *>(*(void **) 0x00CA012C);
}

bool CMonsterBookMan::LoadBook() {
    return ((bool (_fastcall * )(CMonsterBookMan * , PVOID))
    0x006C118C)(this, nullptr);
}