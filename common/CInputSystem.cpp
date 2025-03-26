#include "pch.h"

CInputSystem *CInputSystem::GetInstance() {
    // High volume call
    return reinterpret_cast<CInputSystem *>(*(void **) C_INPUT_SYSTEM_GET_INSTANCE);
}

void CInputSystem::Init(HWND__ *hWnd, void **ahEvent) {
    Log("CInputSystem::Init");
    ((VOID(_fastcall * )(CInputSystem * , PVOID, HWND__
                                                 * hWnd, void * *ahEvent))
    C_INPUT_SYSTEM_INIT)(this, nullptr, hWnd, ahEvent);
}

void CInputSystem::UpdateDevice(int nDeviceIndex) {
    Log("CInputSystem::UpdateDevice");
    ((VOID(_fastcall * )(CInputSystem * , PVOID, int
    nDeviceIndex))
    C_INPUT_SYSTEM_UPDATE_DEVICE)(this, nullptr, nDeviceIndex);
}

int CInputSystem::GetISMessage(ISMSG *pISMsg) {
    Log("CInputSystem::GetISMessage");
    return ((int(_fastcall * )(CInputSystem * , PVOID, ISMSG * pISMsg))
    C_INPUT_SYSTEM_GET_IS_MESSAGE)(this, nullptr, pISMsg);
}

int CInputSystem::GenerateAutoKeyDown(ISMSG *pISMsg) {
    // High volume call
    return ((int(_fastcall * )(CInputSystem * , PVOID, ISMSG * pISMsg))
    C_INPUT_SYSTEM_GENERATE_AUTO_KEY_DOWN)(this, nullptr, pISMsg);
}

void CInputSystem::ShowCursor(int bShow) {
    Log("CInputSystem::ShowCursor");
    ((VOID(_fastcall * )(CInputSystem * , PVOID, int
    nDeviceIndex))
    C_INPUT_SYSTEM_SHOW_CURSOR)(this, nullptr, bShow);
}