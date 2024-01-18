#pragma once

#include "ZXString.h"
#include "ZArray.h"

class CWvsApp {
public:
    virtual ~CWvsApp() = default;

    HWND__ *m_hWnd;
    int m_bPCOMInitialized;
    unsigned int m_dwMainThreadId;
    HHOOK__ *m_hHook;
    int m_bWin9x;
    int m_tUpdateTime;
    int m_bFirstUpdate;
    ZXString<char> m_sCmdLine;
    int m_nGameStartMode;
    int m_bAutoConnect;
    int dummy11;
    int dummy12;
    int dummy13;
    int dummy14;
    int dummy15;
    int dummy16;
    int m_tLastServerIPCheck;
    int m_tLastServerIPCheck2;
    int m_tLastGGHookingAPICheck;
    int m_tLastSecurityCheck;

    static CWvsApp* GetInstance();
};
