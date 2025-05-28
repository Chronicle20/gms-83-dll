#include "pch.h"

void CRadioManager::CreateInstance() {
    Log("CRadioManager::CreateInstance");
    reinterpret_cast<void (__cdecl *)()>(C_RADIO_MANAGER_CREATE_INSTANCE)();
}

CRadioManager *CRadioManager::GetInstance() {
    return reinterpret_cast<CRadioManager *>(*reinterpret_cast<void **>(C_RADIO_MANAGER_INSTANCE_ADDR));
}