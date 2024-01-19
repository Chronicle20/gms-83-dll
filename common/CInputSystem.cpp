#include "pch.h"

CInputSystem *CInputSystem::GetInstance() {
    return reinterpret_cast<CInputSystem *>(*(void **) 0x00BEC33C);
}

//void __thiscall CInputSystem::Init(CInputSystem *this, HWND__ *hWnd, void **ahEvent)
void CInputSystem::Init(HWND__ *hWnd, void **ahEvent) {
    ((VOID(_fastcall * )(CInputSystem * , PVOID, HWND__
                                                 * hWnd, void * *ahEvent))
    0x00599EBF)(this, NULL, hWnd, ahEvent);
}

void CInputSystem::UpdateDevice(int nDeviceIndex) {
    ((VOID(_fastcall * )(CInputSystem * , PVOID, int nDeviceIndex))
    0x0059A2E9)(this, NULL, nDeviceIndex);
}

int CInputSystem::GetISMessage(ISMSG *pISMsg) {
    return ((int(_fastcall * )(CInputSystem * , PVOID, ISMSG *pISMsg))
    0x0059A306)(this, NULL, pISMsg);
}

int CInputSystem::GenerateAutoKeyDown(ISMSG *pISMsg) {
    return ((int(_fastcall * )(CInputSystem * , PVOID, ISMSG *pISMsg))
    0x0059B2D2)(this, NULL, pISMsg);
}