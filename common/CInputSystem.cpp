#include "pch.h"

CInputSystem *CInputSystem::GetInstance() {
    return reinterpret_cast<CInputSystem *>(*(void **) 0x00CD5A80);
}

typedef VOID(__thiscall *_CInputSystem__Init_t)(CInputSystem *pThis, HWND__ *hWnd, void **ahEvent);
_CInputSystem__Init_t _CInputSystem__Init = reinterpret_cast<_CInputSystem__Init_t>(0x005F98BA);

//void __thiscall CInputSystem::Init(CInputSystem *this, HWND__ *hWnd, void **ahEvent)
void CInputSystem::Init(HWND__ *hWnd, void **ahEvent) {
    _CInputSystem__Init(this, hWnd, ahEvent);
}

typedef VOID(__thiscall *_CInputSystem__UpdateDevice_t)(CInputSystem *pThis, int nDeviceIndex);
_CInputSystem__UpdateDevice_t _CInputSystem__UpdateDevice = reinterpret_cast<_CInputSystem__UpdateDevice_t>(0x005F9E49);

void CInputSystem::UpdateDevice(int nDeviceIndex) {
    _CInputSystem__UpdateDevice(this, nDeviceIndex);
}

typedef INT(__thiscall *_CInputSystem__GetISMessage_t)(CInputSystem *pThis, ISMSG *pISMsg);
_CInputSystem__GetISMessage_t _CInputSystem__GetISMessage = reinterpret_cast<_CInputSystem__GetISMessage_t>(0x005F9E70);

int CInputSystem::GetISMessage(ISMSG *pISMsg) {
    return _CInputSystem__GetISMessage(this, pISMsg);
}

typedef INT(__thiscall *_CInputSystem__GenerateAutoKeyDown_t)(CInputSystem *pThis, ISMSG *pISMsg);
_CInputSystem__GenerateAutoKeyDown_t _CInputSystem__GenerateAutoKeyDown = reinterpret_cast<_CInputSystem__GenerateAutoKeyDown_t>(0x005FAE83);

int CInputSystem::GenerateAutoKeyDown(ISMSG *pISMsg) {
    return _CInputSystem__GenerateAutoKeyDown(this, pISMsg);
}