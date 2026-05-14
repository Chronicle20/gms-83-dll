#include "pch.h"

#include "logger.h"
#include "hooker.h"

// void __thiscall CLogo::Init(CLogo *this)
typedef VOID(__thiscall *_CLogo_Init_t)(CLogo *pThis, void *unused);

VOID __fastcall CLogo_Init_Hook(CLogo *pThis, PVOID edx, void *unused) {
    Log("CLogo::Init");
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 111)
    pThis->InitNXLogo();
#endif
    CInputSystem::GetInstance()->ShowCursor(0);
    auto m_nGameStartMode = CWvsApp::GetInstance()->m_nGameStartMode;

    ZXString<char> cmdLine = ZXString<char>();
    if (m_nGameStartMode == 1) {
        CWvsApp::GetInstance()->GetCmdLine(&cmdLine, 5);
    } else {
        CWvsApp::GetInstance()->GetCmdLine(&cmdLine, 3);
    }
    if (cmdLine && *cmdLine && CWvsApp::GetInstance()->m_bAutoConnect) {
        pThis->LogoEnd();
    }
    cmdLine.Empty();
    pThis->LogoEnd();
}

DWORD WINAPI MainProc(LPVOID lpParam) {
    HOOKTYPEDEF_C(CLogo_Init);
    INITMAPLEHOOK(_CLogo_Init, _CLogo_Init_t, CLogo_Init_Hook, C_LOGO_INIT);
    return 0;
}

// dll entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            DisableThreadLibraryCalls(hModule);
            CreateThread(nullptr, 0, &MainProc, nullptr, 0, nullptr);
            break;
        }
    }
    return TRUE;
}