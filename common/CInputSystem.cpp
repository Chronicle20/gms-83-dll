#include "pch.h"

//void __thiscall CInputSystem::Init(CInputSystem *this, HWND__ *hWnd, void **ahEvent)
void CInputSystem::Init(HWND__ *hWnd, void **ahEvent) {
    ((VOID(_fastcall * )(CInputSystem * , PVOID, HWND__
                                                 * hWnd, void * *ahEvent))
    0x00599EBF)(this, NULL, hWnd, ahEvent);
}

CInputSystem *CInputSystem::GetInstance() {
    return reinterpret_cast<CInputSystem *>(*(void **) 0x00BEC33C);
}