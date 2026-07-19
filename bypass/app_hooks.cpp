#include "pch.h"

#include "app_hooks.h"
#include "socket_hooks_internal.h" // CClientSocket__OnConnect_Hook fwd-decl

#include "client_exception.h"
#include "hooker.h"
#include "logger.h"

#include <timeapi.h>

// ---- typedefs -----------------------------------------------------------
typedef VOID(__thiscall* _CWvsApp__CWvsApp_t)(CWvsApp* pThis, const char* sCmdLine);
typedef VOID(__thiscall* _CWvsApp__SetUp_t)(CWvsApp* pThis);
typedef VOID(__thiscall* _CWvsApp__InitializeInput_t)(CWvsApp* pThis);
typedef VOID(__thiscall* _CWvsApp__Run_t)(CWvsApp* pThis, int* pbTerminate);
typedef VOID(__thiscall* _CWvsApp__CallUpdate_t)(CWvsApp* pThis, int tCurTime);
typedef VOID(__thiscall* _CWvsApp__ConnectLogin_t)(CWvsApp* pThis);

typedef VOID(__cdecl* _set_stage_t)(CStage* pStage, void* pParam);
static _set_stage_t _set_stage = reinterpret_cast<_set_stage_t>(SET_STAGE);

typedef BOOL(WINAPI* LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);

// ---- helpers ------------------------------------------------------------
static CStage* get_stage() {
    return reinterpret_cast<CStage*>(*(void**)STAGE_INSTANCE_ADDR);
}

static IWzGr2D* get_gr() {
    return reinterpret_cast<IWzGr2D*>(*(uint32_t**)GR_INSTANCE_ADDR);
}

static void GetSEPrivilege() {
    ((VOID * *(_fastcall*)()) GET_SE_PRIVILEGE)();
}

static DWORD ResetLSP() {
    return reinterpret_cast<DWORD>(*(void**)RESET_LSP);
}

// ---- hook bodies --------------------------------------------------------

VOID __fastcall CWvsApp__CallUpdate_Hook(CWvsApp* pThis, PVOID edx, int tCurTime) {
    if (pThis->m_bFirstUpdate) {
        pThis->m_tUpdateTime = tCurTime;
#if defined(REGION_GMS)
        pThis->m_tLastServerIPCheck = tCurTime;
        pThis->m_tLastServerIPCheck2 = tCurTime;
        pThis->m_tLastGGHookingAPICheck = tCurTime;
#endif
        pThis->m_tLastSecurityCheck = tCurTime;
        pThis->m_bFirstUpdate = 0;
    }

    while (tCurTime - pThis->m_tUpdateTime > 0) {
        CStage* stage = get_stage();
        if (stage) {
            stage->Update();
        }

        CWndMan::s_Update();
        pThis->m_tUpdateTime += 30;
        if (tCurTime - pThis->m_tUpdateTime > 0) {
            auto gr = get_gr();
            auto hr = gr->UpdateCurrentTime(pThis->m_tUpdateTime);
            if (FAILED(hr)) {
                Log("Some sort of com error");
                return;
            }
        }
    }
    auto gr = get_gr();
    auto hr = gr->UpdateCurrentTime(tCurTime);
    if (FAILED(hr)) {
        Log("Some sort of com error");
        return;
    }
    CActionMan::GetInstance()->SweepCache();
}

VOID __fastcall CWvsApp__ConnectLogin_Hook(CWvsApp* pThis, PVOID edx) {
    Log("CWvsApp::ConnectLogin");
    CClientSocket* pSock = CClientSocket::GetInstance();
    pSock->Close();
    Log("CWvsApp::ConnectLogin Call CClientSocket::ConnectLogin");
    pSock->ConnectLogin();

    tagMSG msg{};
    while (true) {
        if (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_USER + 1) {
                unsigned int handle = pSock->m_sock._m_hSocket;
                if (msg.wParam != handle)
                    continue;

                int errorCode = HIWORD(msg.lParam);
                if (errorCode) {
                    if (errorCode != 10038) {
                        Log("CWvsApp::ConnectLogin Call CClientSocket::OnConnect 1. ErrorCode [%d].", errorCode);
                        CClientSocket__OnConnect_Hook(pSock, edx, 0);
                    }
                } else {
                    WORD low = LOWORD(msg.lParam);
                    if (low != 16 && low != 1) {
                        Log("CWvsApp::ConnectLogin Call CClientSocket::OnConnect 2. low [%d].", low);
                        CClientSocket__OnConnect_Hook(pSock, edx, 0);
                        continue;
                    }

                    Log("CWvsApp::ConnectLogin Call CClientSocket::OnConnect 3");
                    if (CClientSocket__OnConnect_Hook(pSock, edx, 1)) {
                        break;
                    }
                }
            } else {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);

                if ((LONG)(timeGetTime() - pSock->m_tTimeout) > 0) {
                    Log("CWvsApp::ConnectLogin Call CClientSocket::OnConnect 4. timeGetTime [%d], timeOut [%d].",
                        timeGetTime(), pSock->m_tTimeout);
                    CClientSocket__OnConnect_Hook(pSock, edx, 0);
                }
            }
        } else {
            if ((LONG)(timeGetTime() - pSock->m_tTimeout) > 0) {
                Log("CWvsApp::ConnectLogin Call CClientSocket::OnConnect 5");
                CClientSocket__OnConnect_Hook(pSock, edx, 0);
            }
        }

        if (msg.message == WM_QUIT) {
            break;
        }
    }

    auto handle = pSock->m_sock._m_hSocket;
    if (handle == 0 || handle == -1) {
        Log("CWvsApp::ConnectLogin Should issue exception here.");
    }
}

VOID __fastcall CWvsApp__InitializeInput_Hook(CWvsApp* pThis, PVOID edx) {
    Log("CWvsApp::InitializeInput");
    CInputSystem::CreateInstance();
    CInputSystem::GetInstance()->Init(pThis->m_hWnd, pThis->m_ahInput);
}

VOID __fastcall CWvsApp__Run_Hook(CWvsApp* pThis, PVOID edx, int* pbTerminate) {
    Log("CWvsApp::Run");
    tagMSG msg{};
    ISMSG isMsg{};
    memset(&msg, 0, sizeof(msg));
    memset(&isMsg, 0, sizeof(isMsg));
    if (CClientSocket::GetInstance()) {
        CClientSocket::GetInstance()->ManipulatePacket();
    }
    do {
        auto dwRet = MsgWaitForMultipleObjects(3u, pThis->m_ahInput, 0, 0, 0xFFu);
        if (dwRet <= 2) {
            CInputSystem::GetInstance()->UpdateDevice(dwRet);
            do {
                if (!CInputSystem::GetInstance()->GetISMessage(&isMsg)) {
                    break;
                }
                pThis->ISMsgProc(isMsg.message, isMsg.wParam, isMsg.lParam);
            } while (!*pbTerminate);
        } else if (dwRet == 3) {
            do {
                if (!PeekMessageA(&msg, nullptr, 0, 0, 1u)) {
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
                if (pThis->m_hrComErrorCode) {
                    HRESULT hr = pThis->m_hrComErrorCode;
                    pThis->m_hrComErrorCode = 0;
                    pThis->m_hrZExceptionCode = 0;
                    RaiseComError(hr);
                }
                if (pThis->m_hrZExceptionCode) {
                    int code = pThis->m_hrZExceptionCode;
                    pThis->m_hrComErrorCode = 0;
                    pThis->m_hrZExceptionCode = 0;
                    RaiseClientException(code);
                }
            } while (!*pbTerminate && msg.message != 18);
        } else {
            if (CInputSystem::GetInstance()->GenerateAutoKeyDown(&isMsg)) {
                pThis->ISMsgProc(isMsg.message, isMsg.wParam, isMsg.lParam);
            }
            auto tCurTime = 0;
            auto hr = get_gr()->GetnextRenderTime(&tCurTime);
            if (FAILED(hr)) {
                RaiseComErrorEx(hr);
            }
            CWvsApp__CallUpdate_Hook(pThis, edx, tCurTime);
            CWndMan::RedrawInvalidatedWindows();
            hr = get_gr()->RenderFrame();
            if (FAILED(hr)) {
                RaiseComErrorEx(hr);
            }
            Sleep(1u);
        }
    } while (!*pbTerminate && msg.message != 18);
    if (msg.message == 18) {
        PostQuitMessage(0);
    }
}

VOID __fastcall CWvsApp__SetUp_Hook(CWvsApp* pThis, PVOID edx) {
    Log("CWvsApp::SetUp");
#if defined(REGION_GMS) && C_WVS_APP_INITIALIZE_AUTH != 0
    // InitializeAuth calls through C_WVS_APP_INITIALIZE_AUTH. The NMCO/Nexon-Passport
    // auth subsystem post-dates v79, so on v79 (and older: v72/v61) that address is the
    // 0x0 absent-sentinel; calling through it AVs at 0 right after the log line. Gate on
    // a non-zero address, same as DR_INIT below. task-008.
    pThis->InitializeAuth();
#endif

    auto time = timeGetTime();
    srand(time);

#if defined(REGION_GMS) && DR_INIT != 0
    // Clean clients call DR_init here (between srand and GetSEPrivilege). It resolves
    // NtGetContextThread into a global that the in-field DR anti-debug check calls through
    // (reached from the CVecCtrlUser movement path). Omitting it on v84 — which does NOT hook
    // DR_check — leaves that pointer NULL, so the check does call(0) => AV at 0x0 ~2-3s into the
    // field. v87+ also hook DR_check, but restoring this call matches the clean client and is
    // harmless. Best-effort: no while(1)/throw/network inside. task-006.
    reinterpret_cast<void(__cdecl*)()>(DR_INIT)();
#endif

#if defined(REGION_GMS)
    GetSEPrivilege();
#endif

    CSecurityClient::CreateInstance();

    PVOID cfgAlloc = ZAllocEx<ZAllocAnonSelector>::GetInstance()->Alloc(sizeof(CConfig));
    CConfig* cConfig;
    if (cfgAlloc) {
        cConfig = new (cfgAlloc) CConfig();
    }

    pThis->InitializePCOM();
    pThis->CreateMainWindow();

    CClientSocket::CreateInstance();
    pThis->ConnectLogin();

    Log("CWvsApp::SetUp after CClientSocket::ConnectLogin");

    CSecurityClient::GetInstance()->m_hMainWnd = pThis->m_hWnd;

    CFuncKeyMappedMan::CreateInstance();
    CQuickslotKeyMappedMan::CreateInstance();
#if C_MACRO_SYS_MAN_CREATE_INSTANCE != 0
    // CreateInstance calls through C_MACRO_SYS_MAN_CREATE_INSTANCE. v72 has no separate
    // CMacroSysMan singleton (the macro-sys-data-init role is folded into
    // CQuickslotKeyMappedMan, created just above), so that address is the 0x0 absent-
    // sentinel; calling through it AVs at 0. Gate on a non-zero address, same as the
    // CRadioManager guard below. task-009.
    CMacroSysMan::CreateInstance();
#endif

#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
    CBattleRecordMan::CreateInstance();
#endif

    pThis->InitializeResMan();
    pThis->InitializeGr2D();
    pThis->InitializeInput();

#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95) || defined(REGION_JMS)
    ShowWindow(pThis->m_hWnd, 5);
    UpdateWindow(pThis->m_hWnd);
    SetForegroundWindow(pThis->m_hWnd);
    auto hr = get_gr()->RenderFrame();
    if (FAILED(hr)) {
        RaiseComErrorEx(hr);
    }
#endif

    Sleep(300);
    pThis->InitializeSound();
    Sleep(300);
    pThis->InitializeGameData();
    pThis->CreateWndManager();

    CConfig::GetInstance()->ApplySysOpt(nullptr, 0);

    CActionMan::CreateInstance();
    CActionMan::GetInstance()->Init();

    CAnimationDisplayer::CreateInstance();

    CMapleTVMan::CreateInstance();
#if defined(REGION_GMS)
    CMapleTVMan::GetInstance()->Init();
#elif defined(REGION_JMS)
    CMapleTVMan::GetInstance()->Init(pThis->unk1[1], pThis->unk1[0]);
#endif

    CQuestMan::CreateInstance();
    if (!CQuestMan::GetInstance()->LoadDemand()) {
        Log("Throw error regarding CQuestMan::LoadDemand");
        return;
    }
    CQuestMan::GetInstance()->LoadPartyQuestInfo();
    CQuestMan::GetInstance()->LoadExclusive();

    CMonsterBookMan::CreateInstance();
    if (!CMonsterBookMan::GetInstance()->LoadBook()) {
        Log("Throw error regarding CMonsterBookMan::LoadBook");
        return;
    }

#if C_RADIO_MANAGER_CREATE_INSTANCE != 0
    // CreateInstance calls through C_RADIO_MANAGER_CREATE_INSTANCE. v79 has no
    // CRadioManager singleton (the scheduled-message/radio role is folded into
    // CMapleTVMan), so that address is the 0x0 absent-sentinel; calling through it
    // AVs at 0 right after the log line, leaving a popped-but-dead window. Gate on a
    // non-zero address, same as the InitializeAuth guard above. task-008.
    CRadioManager::CreateInstance();
#endif

    char sModulePath[260];
    GetModuleFileNameA(nullptr, sModulePath, 260);
    CWvsApp::Dir_BackSlashToSlash(sModulePath);
    CWvsApp::Dir_upDir(sModulePath);
    CWvsApp::Dir_SlashToBackSlash(sModulePath);

    ZXString<char> tempString = ZXString<char>(sModulePath, 0xFFFFFFFF);
    CConfig::GetInstance()->CheckExecPathReg(tempString);

    PVOID ret = ZAllocEx<ZAllocAnonSelector>::GetInstance()->Alloc(sizeof(CLogo));
    CStage* cLogo;
    if (ret) {
        cLogo = new (ret) CLogo();
    } else {
        cLogo = nullptr;
    }
    _set_stage(cLogo, nullptr);
}

VOID __fastcall CWvsApp__CWvsApp_Hook(CWvsApp* pThis, PVOID edx, const char* sCmdLine) {
    Log("CWvsApp::CWvsApp");
    void** instance = reinterpret_cast<void**>(C_WVS_APP_INSTANCE_ADDR);
    *instance = pThis;

    //    void* globalThis = *(void**)0x01002884;
    //    Log("global singleton = %p, hook pThis = %p", globalThis, pThis);

    pThis->m_hWnd = nullptr;
    pThis->m_bPCOMInitialized = 0;
    pThis->m_hHook = nullptr;
    pThis->m_tUpdateTime = 0;
    pThis->m_bFirstUpdate = 1;
    pThis->m_sCmdLine = ZXString<char>();
    pThis->m_nGameStartMode = 0;
    pThis->m_bAutoConnect = 1;
#if defined(REGION_JMS)
    pThis->unk1[0] = 0;
    pThis->unk1[1] = 0;
    pThis->unk2[0] = ZXString<char>();
    pThis->unk2[1] = ZXString<char>();
#endif
    pThis->m_bShowAdBalloon = 0;
    pThis->m_bExitByTitleEscape = 0;
    pThis->m_hrZExceptionCode = 0;
    pThis->m_hrComErrorCode = 0;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 87)
    pThis->m_tNextSecurityCheck = 0;
    pThis->m_pBackupBuffer = ZArray<unsigned char>();
    pThis->m_dwBackupBufferSize = 0;
#endif

#if defined(REGION_JMS)
    pThis->unk2[0] = ZXString<char>("", 0xFFFFFFFF);
    pThis->unk2[0] = ZXString<char>("", 0xFFFFFFFF);
#endif
    pThis->m_sCmdLine = ZXString<char>(sCmdLine, 0xFFFFFFFF);

    pThis->m_sCmdLine = *pThis->m_sCmdLine.TrimRight("\" ")->TrimLeft("\" ");
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 87)
    pThis->m_pBackupBuffer.Alloc(0x1000);
#endif
    ZXString<char> sToken = ZXString<char>();
    pThis->GetCmdLine(&sToken, 0);

    pThis->m_nGameStartMode = 2;
    pThis->m_dwMainThreadId = GetCurrentThreadId();

    OSVERSIONINFO ovi;
    ZeroMemory(&ovi, sizeof(OSVERSIONINFO));
    ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionExA(&ovi);
    pThis->m_bWin9x = ovi.dwPlatformId == 1;
    if (ovi.dwMajorVersion >= 6 && !pThis->m_nGameStartMode) {
        pThis->m_nGameStartMode = 2;
    }

#if defined(REGION_GMS)
    int* g_dwTargetOS = reinterpret_cast<int*>(G_DW_TARGET_OS);

    if (ovi.dwMajorVersion < 5) {
        *g_dwTargetOS = 1996;
    }

    BOOL bIsWow64 = FALSE;
    auto fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
    if (fnIsWow64Process) {
        fnIsWow64Process(GetCurrentProcess(), &bIsWow64);
    }

    if (bIsWow64) {
        *g_dwTargetOS = 1996;
    }
    if (ovi.dwMajorVersion >= 6 && !bIsWow64) {
        ResetLSP();
    }
    sToken.Empty();
#endif
}

// ---- installer ----------------------------------------------------------
BOOL InstallAppHooks() {
    HOOKTYPEDEF_C(CWvsApp__CWvsApp);
    INITMAPLEHOOK_OR_RETURN(_CWvsApp__CWvsApp, _CWvsApp__CWvsApp_t, CWvsApp__CWvsApp_Hook, C_WVS_APP);

    HOOKTYPEDEF_C(CWvsApp__SetUp);
    INITMAPLEHOOK_OR_RETURN(_CWvsApp__SetUp, _CWvsApp__SetUp_t, CWvsApp__SetUp_Hook, C_WVS_APP_SET_UP);

    HOOKTYPEDEF_C(CWvsApp__InitializeInput);
    INITMAPLEHOOK_OR_RETURN(_CWvsApp__InitializeInput, _CWvsApp__InitializeInput_t, CWvsApp__InitializeInput_Hook,
                            C_WVS_APP_INITIALIZE_INPUT);

    HOOKTYPEDEF_C(CWvsApp__Run);
    INITMAPLEHOOK_OR_RETURN(_CWvsApp__Run, _CWvsApp__Run_t, CWvsApp__Run_Hook, C_WVS_APP_RUN);

    HOOKTYPEDEF_C(CWvsApp__CallUpdate);
    INITMAPLEHOOK_OR_RETURN(_CWvsApp__CallUpdate, _CWvsApp__CallUpdate_t, CWvsApp__CallUpdate_Hook,
                            C_WVS_APP_CALL_UPDATE);

    HOOKTYPEDEF_C(CWvsApp__ConnectLogin);
    INITMAPLEHOOK_OR_RETURN(_CWvsApp__ConnectLogin, _CWvsApp__ConnectLogin_t, CWvsApp__ConnectLogin_Hook,
                            C_WVS_APP_CONNECT_LOGIN);

    return TRUE;
}
