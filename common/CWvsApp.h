#pragma once

#include "asserts.h"

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

// Stack-constructed in WinMain; our ctor/SetUp hooks write its fields and Run/CallUpdate read
// them every frame, so a wrong size = wrong field offsets = corruption. Real sizes (size sweep):
// v79/v83/v84 = 0x60, v87 = 0x6C, v111 = 0x8C, JMS = 0x64 (v95 TBD). These pass with the current
// version gates — locking them here guards against a gate regression.
// v72 size verified task-009 (WinMain stack-ctor @0x8EF809 -> this=ebp-0xF4; highest field @+0x5C (m_ahInput[2]), next
// local @+0x6C; ctor @0x8F26C7 field-init through +0x38 == v79 -> 0x60)
#if defined(REGION_GMS) &&                                                                                             \
    (BUILD_MAJOR_VERSION == 72 || BUILD_MAJOR_VERSION == 79 || BUILD_MAJOR_VERSION == 83 || BUILD_MAJOR_VERSION == 84)
assert_size(sizeof(CWvsApp), 0x60) // v79 size verified task-008 (ctor @0x942D3B base layout == v83/84)
#elif defined(REGION_GMS) && BUILD_MAJOR_VERSION == 87
assert_size(sizeof(CWvsApp), 0x6C)
#elif defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95
assert_size(sizeof(CWvsApp), 0x8C) // v95 and v111 both 0x8C
#elif defined(REGION_JMS)
assert_size(sizeof(CWvsApp), 0x64)
#endif
