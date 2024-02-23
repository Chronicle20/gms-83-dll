#include "pch.h"

void CRadioManager::CreateInstance() {
    ((VOID * *(_fastcall * )())
    0x00A8E708)();
}

CRadioManager *CRadioManager::GetInstance() {
    return reinterpret_cast<CRadioManager *>(*(void **) 0x00C9EC04);
}