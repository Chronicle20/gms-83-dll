#include "pch.h"

DWORD WINAPI MainProc(LPVOID lpParam);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);
            CreateThread(nullptr, 0, &MainProc, nullptr, 0, nullptr);
            break;
        default:
            break;
    }
    return TRUE;
}
