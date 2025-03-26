/*
 This file is part of GMS-83-DLL.

 GMS-83-DLL is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

 GMS-83-DLL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
 */
#include "pch.h"

#include "logger.h"
#include "hooker.h"

// void __thiscall CLogo::Init(CLogo *this)
typedef VOID(__thiscall *_CLogo_Init_t)(CLogo *pThis, void *unused);

HOOKTYPEDEF_C(CLogo_Init);

VOID __fastcall CLogo_Init_Hook(CLogo *pThis, PVOID edx, void *unused) {
    Log("CLogo::Init");
    pThis->InitNXLogo();
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

VOID __stdcall MainProc() {
    INITMAPLEHOOK(_CLogo_Init, _CLogo_Init_t, CLogo_Init_Hook, C_LOGO_INIT);
}

// dll entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            DisableThreadLibraryCalls(hModule);
            CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE) &MainProc, nullptr, 0, nullptr);
            break;
        }
    }
    return TRUE;
}