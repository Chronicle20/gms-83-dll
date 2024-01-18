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
};