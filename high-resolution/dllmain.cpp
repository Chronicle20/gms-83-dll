#include <Windows.h>
#include <cmath>
#include "Client.h"

// main thread
VOID __stdcall MainProc() {

    //Client::UpdateLogin();
    Client::SetResolution(1920, 1080);
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