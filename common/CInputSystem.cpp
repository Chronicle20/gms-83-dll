#include "pch.h"

void CInputSystem::CreateInstance() {
    reinterpret_cast<void (__fastcall *)()>(C_INPUT_SYSTEM_CREATE_INSTANCE)();
}

CInputSystem *CInputSystem::GetInstance() {
    // High volume call
    return reinterpret_cast<CInputSystem *>(*reinterpret_cast<void **>(C_INPUT_SYSTEM_GET_INSTANCE));
}

void CInputSystem::Init(HWND__ *hWnd, void **ahEvent) {
    Log("CInputSystem::Init");
    reinterpret_cast<void (__fastcall *)(CInputSystem *, void *, HWND__ *, void **)>(
            C_INPUT_SYSTEM_INIT)(this, nullptr, hWnd, ahEvent);
}

void CInputSystem::UpdateDevice(int nDeviceIndex) {
    Log("CInputSystem::UpdateDevice");
    reinterpret_cast<void (__fastcall *)(CInputSystem *, void *, int)>(
            C_INPUT_SYSTEM_UPDATE_DEVICE)(this, nullptr, nDeviceIndex);
}

int CInputSystem::GetISMessage(ISMSG *pISMsg) {
    Log("CInputSystem::GetISMessage");
    return reinterpret_cast<int (__fastcall *)(CInputSystem *, void *, ISMSG *)>(
            C_INPUT_SYSTEM_GET_IS_MESSAGE)(this, nullptr, pISMsg);
}

int CInputSystem::GenerateAutoKeyDown(ISMSG *pISMsg) {
    // High volume call
    return reinterpret_cast<int (__fastcall *)(CInputSystem *, void *, ISMSG *)>(
            C_INPUT_SYSTEM_GENERATE_AUTO_KEY_DOWN)(this, nullptr, pISMsg);
}

void CInputSystem::ShowCursor(int bShow) {
    Log("CInputSystem::ShowCursor");
    reinterpret_cast<void (__fastcall *)(CInputSystem *, void *, int)>(
            C_INPUT_SYSTEM_SHOW_CURSOR)(this, nullptr, bShow);
}