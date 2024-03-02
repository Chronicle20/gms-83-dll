#include "pch.h"

void CRadioManager::CreateInstance() {
    ((VOID * *(_fastcall * )())
    0x00ADCF8C)();
}

CRadioManager *CRadioManager::GetInstance() {
    return reinterpret_cast<CRadioManager *>(*(void **) 0x00CD5C58);
}