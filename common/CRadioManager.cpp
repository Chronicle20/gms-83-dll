#include "pch.h"

void CRadioManager::CreateInstance() {
    ((VOID * *(_fastcall * )())
    C_RADIO_MANAGER_CREATE_INSTANCE)();
}

CRadioManager *CRadioManager::GetInstance() {
    return reinterpret_cast<CRadioManager *>(*(void **) C_RADIO_MANAGER_GET_INSTANCE);
}