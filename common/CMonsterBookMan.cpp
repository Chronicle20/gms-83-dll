#include "pch.h"

typedef VOID(__cdecl *_CMonsterBookMan__CreateInstance_t)();
_CMonsterBookMan__CreateInstance_t _CMonsterBookMan__CreateInstance = reinterpret_cast<_CMonsterBookMan__CreateInstance_t>(0x00ADCA98);

void CMonsterBookMan::CreateInstance() {
    _CMonsterBookMan__CreateInstance();
}

CMonsterBookMan *CMonsterBookMan::GetInstance() {
    return reinterpret_cast<CMonsterBookMan *>(*(void **) 0x00CD739C);
}

typedef BOOL(__thiscall *_CMonsterBookMan__LoadBook_t)(CMonsterBookMan *pThis);
_CMonsterBookMan__LoadBook_t _CMonsterBookMan__LoadBook = reinterpret_cast<_CMonsterBookMan__LoadBook_t>(0x0070530E);

bool CMonsterBookMan::LoadBook() {
    return _CMonsterBookMan__LoadBook(this);
}