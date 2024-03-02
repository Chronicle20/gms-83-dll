#include "pch.h"

CInputSystem *CInputSystem::GetInstance() {
    return reinterpret_cast<CInputSystem *>(*(void **) 0x00CD5A80);
}

//void __thiscall CInputSystem::Init(CInputSystem *this, HWND__ *hWnd, void **ahEvent)
void CInputSystem::Init(HWND__ *hWnd, void **ahEvent) {
    ((VOID(_fastcall * )(CInputSystem * , PVOID, HWND__
                                                 * hWnd, void * *ahEvent))
    0x005F98BA)(this, nullptr, hWnd, ahEvent);
}

void CInputSystem::UpdateDevice(int nDeviceIndex) {
    ((VOID(_fastcall * )(CInputSystem * , PVOID, int nDeviceIndex))
    0x005F9E49)(this, nullptr, nDeviceIndex);
}

int CInputSystem::GetISMessage(ISMSG *pISMsg) {
    return ((int(_fastcall * )(CInputSystem * , PVOID, ISMSG *pISMsg))
    0x005F9E70)(this, nullptr, pISMsg);
}

int CInputSystem::GenerateAutoKeyDown(ISMSG *pISMsg) {
    return ((int(_fastcall * )(CInputSystem * , PVOID, ISMSG *pISMsg))
    0x005FAE83)(this, nullptr, pISMsg);
}