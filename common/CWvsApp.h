#pragma once

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
#if defined(REGION_JMS)
    int dummy18;
    int dummy15;
    ZXString<char> dummy16;
    ZXString<char> dummy17;
#endif
    int m_bShowAdBalloon;
    int m_bExitByTitleEscape;
    HRESULT m_hrZExceptionCode;
    HRESULT m_hrComErrorCode;
#if defined(REGION_GMS)
    unsigned int m_dwSecurityErrorCode;
    int m_nTargetVersion;
    int m_tLastServerIPCheck;
    int m_tLastServerIPCheck2;
    int m_tLastGGHookingAPICheck;
    int m_tLastSecurityCheck;
#elif defined(REGION_JMS)
    int dummy19;
    int dummy20;
    int dummy21;
#endif
    void *m_ahInput[3];
    int m_tNextSecurityCheck;
    ZArray<unsigned char> m_pBackupBuffer;
    unsigned int m_dwBackupBufferSize;

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
