#include <Windows.h>
#include "Client.h"

// main thread
VOID __stdcall MainProc()
{
    Client::m_nGameWidth=1920;
    Client::m_nGameHeight=1080;
    //Client::UpdateLogin();
    Client::UpdateResolution();
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