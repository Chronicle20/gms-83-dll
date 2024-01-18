#include "pch.h"

void CRadioManager::CreateInstance() {
    ((VOID * *(_fastcall * )())
    0x009FA078)();
}

CRadioManager *CRadioManager::GetInstance() {
    return reinterpret_cast<CRadioManager *>(*(void **) 0x00BF0B00);
}