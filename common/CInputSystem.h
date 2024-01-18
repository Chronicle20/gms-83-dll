#pragma once

#include "IsMsg.h"

class CInputSystem {

public:
    static CInputSystem *GetInstance();


    CInputSystem() {
        ((VOID(_fastcall * )(CInputSystem * , PVOID))
        0x009F821F)(this, NULL);
    }

    void Init(HWND__ *, void **);

    void UpdateDevice(int nDeviceIndex);

    int GetISMessage(ISMSG *pISMsg);

    int GenerateAutoKeyDown(ISMSG *pISMsg);
};