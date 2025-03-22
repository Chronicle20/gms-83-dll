#include "pch.h"

CInputSystem *CInputSystem::GetInstance() {
    return reinterpret_cast<CInputSystem *>(*(void **) C_INPUT_SYSTEM_GET_INSTANCE);
}

void CInputSystem::Init(HWND__ *hWnd, void **ahEvent) {
    ((VOID(_fastcall * )(CInputSystem * , PVOID, HWND__
                                                 * hWnd, void * *ahEvent))
    C_INPUT_SYSTEM_INIT)(this, NULL, hWnd, ahEvent);
}

void CInputSystem::UpdateDevice(int nDeviceIndex) {
    ((VOID(_fastcall * )(CInputSystem * , PVOID, int
    nDeviceIndex))
    C_INPUT_SYSTEM_UPDATE_DEVICE)(this, NULL, nDeviceIndex);
}

int CInputSystem::GetISMessage(ISMSG *pISMsg) {
    return ((int(_fastcall * )(CInputSystem * , PVOID, ISMSG * pISMsg))
    C_INPUT_SYSTEM_GET_IS_MESSAGE)(this, NULL, pISMsg);
}

int CInputSystem::GenerateAutoKeyDown(ISMSG *pISMsg) {
    return ((int(_fastcall * )(CInputSystem * , PVOID, ISMSG * pISMsg))
    C_INPUT_SYSTEM_GENERATE_AUTO_KEY_DOWN)(this, NULL, pISMsg);
}