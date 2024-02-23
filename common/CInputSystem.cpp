#include "pch.h"

CInputSystem *CInputSystem::GetInstance() {
    return reinterpret_cast<CInputSystem *>(*(void **) 0x00C9EA64);
}

//void __thiscall CInputSystem::Init(CInputSystem *this, HWND__ *hWnd, void **ahEvent)
void CInputSystem::Init(HWND__ *hWnd, void **ahEvent) {
    ((VOID(_fastcall * )(CInputSystem * , PVOID, HWND__
                                                 * hWnd, void * *ahEvent))
    0x005C9961)(this, NULL, hWnd, ahEvent);
}

void CInputSystem::UpdateDevice(int nDeviceIndex) {
    ((VOID(_fastcall * )(CInputSystem * , PVOID, int nDeviceIndex))
    0x005C9D8B)(this, NULL, nDeviceIndex);
}

int CInputSystem::GetISMessage(ISMSG *pISMsg) {
    return ((int(_fastcall * )(CInputSystem * , PVOID, ISMSG *pISMsg))
    0x005C9DA8)(this, NULL, pISMsg);
}

int CInputSystem::GenerateAutoKeyDown(ISMSG *pISMsg) {
    return ((int(_fastcall * )(CInputSystem * , PVOID, ISMSG *pISMsg))
    0x005CAD74)(this, NULL, pISMsg);
}