#pragma once

class CWvsApp {
public:
    virtual ~CWvsApp() = default;

    HWND__ *m_hWnd;
    int m_bPCOMInitialized;
    unsigned int m_dwMainThreadId;
    HHOOK__ *m_hHook;
    int m_bWin9x;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
    int m_nOSVersion;
    int m_nOSMinorVersion;
    int m_nOSBuildNumber;
    ZXString<char> m_sCSDVersion;
    int m_b64BitInfo;
#endif
    int m_tUpdateTime;
    int m_bFirstUpdate;
    ZXString<char> m_sCmdLine;
    int m_nGameStartMode;
    int m_bAutoConnect;
#if defined(REGION_JMS)
    int unk1[2];
    ZXString<char> unk2[2];
#endif
    int m_bShowAdBalloon;
    int m_bExitByTitleEscape;
    HRESULT m_hrZExceptionCode;
    HRESULT m_hrComErrorCode;
    unsigned int m_dwSecurityErrorCode;
    int m_nTargetVersion;
#if defined(REGION_GMS)
    int m_tLastServerIPCheck;
    int m_tLastServerIPCheck2;
    int m_tLastGGHookingAPICheck;
#endif
    int m_tLastSecurityCheck;
    void *m_ahInput[3];
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 87)
    int m_tNextSecurityCheck;
#endif
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
    bool m_bEnabledDX9;
#endif
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 87)
    ZArray<unsigned char> m_pBackupBuffer;
    unsigned int m_dwBackupBufferSize;
#endif
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
    unsigned int m_dwClearStackLog;
    int m_bWindowActive;
#endif

    static CWvsApp* GetInstance();

    void ISMsgProc(unsigned int message, unsigned int wParam, int lParam);

    void InitializeAuth();

    void InitializePCOM();

    void CreateMainWindow();

    void ConnectLogin();

    void InitializeResMan();

    void InitializeGr2D();

    void InitializeInput();

    void InitializeSound();

    void InitializeGameData();

    void CreateWndManager();

    ZXString<char> * GetCmdLine(ZXString<char> *result, int nArg);

    static void Dir_BackSlashToSlash(char string[260]);

    static void Dir_upDir(char string[260]);

    static void Dir_SlashToBackSlash(char string[260]);

    static char * GetExceptionFileName();
};
