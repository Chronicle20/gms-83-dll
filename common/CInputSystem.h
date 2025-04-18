#pragma once
#include "memory_map.h"

class CInputSystem {

public:
    static void CreateInstance();

    static CInputSystem *GetInstance();


    CInputSystem() {
        ((VOID(_fastcall * )(CInputSystem * , PVOID))
        C_INPUT_SYSTEM)(this, NULL);
    }

    void Init(HWND__ *, void **);

    void UpdateDevice(int nDeviceIndex);

    int GetISMessage(ISMSG *pISMsg);

    int GenerateAutoKeyDown(ISMSG *pISMsg);

    void ShowCursor(int bShow);
};