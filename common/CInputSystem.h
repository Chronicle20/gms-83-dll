#pragma once

class CInputSystem {

public:
    static CInputSystem *GetInstance();

    typedef VOID(__thiscall *_CInputSystem__CInputSystem_t)(CInputSystem *pThis);
    _CInputSystem__CInputSystem_t _CInputSystem__CInputSystem = reinterpret_cast<_CInputSystem__CInputSystem_t>(0x00ADAD17);

    CInputSystem() {
        _CInputSystem__CInputSystem(this);
    }

    void Init(HWND__ *, void **);

    void UpdateDevice(int nDeviceIndex);

    int GetISMessage(ISMSG *pISMsg);

    int GenerateAutoKeyDown(ISMSG *pISMsg);
};