#include "pch.h"

void CMonsterBookMan::CreateInstance() {
    ((VOID * *(_fastcall * )())
    0x00ADCA98)();
}

CMonsterBookMan *CMonsterBookMan::GetInstance() {
    return reinterpret_cast<CMonsterBookMan *>(*(void **) 0x00CD739C);
}

bool CMonsterBookMan::LoadBook() {
    return ((bool (_fastcall * )(CMonsterBookMan * , PVOID))
    0x0070530E)(this, nullptr);
}