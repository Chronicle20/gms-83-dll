#include "pch.h"

void CMonsterBookMan::CreateInstance() {
    ((VOID * *(_fastcall * )())
    0x009F9B73)();
}

CMonsterBookMan *CMonsterBookMan::GetInstance() {
    return reinterpret_cast<CMonsterBookMan *>(*(void **) 0x00BED610);
}

bool CMonsterBookMan::LoadBook() {
    return ((bool (_fastcall * )(CMonsterBookMan * , PVOID))
    0x0068487C)(this, NULL);
}