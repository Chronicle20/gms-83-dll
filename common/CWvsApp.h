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
    int m_nOSVersion;
    int m_bFirstUpdate;
    ZXString<char> m_sCmdLine;
    int m_nGameStartMode;
    int m_bAutoConnect;
    int m_bShowAdBalloon;
    int m_bExitByTitleEscape;
    HRESULT m_hrZExceptionCode;
    HRESULT m_hrComErrorCode;

    static CWvsApp* GetInstance();
};
