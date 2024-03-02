#include "pch.h"

typedef VOID(__cdecl *_CRadioManager__CreateInstance_t)();
_CRadioManager__CreateInstance_t _CRadioManager__CreateInstance = reinterpret_cast<_CRadioManager__CreateInstance_t>(0x00ADCF8C);

void CRadioManager::CreateInstance() {
    _CRadioManager__CreateInstance();
}

CRadioManager *CRadioManager::GetInstance() {
    return reinterpret_cast<CRadioManager *>(*(void **) 0x00CD5C58);
}