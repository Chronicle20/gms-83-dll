#include "pch.h"

DWORD WINAPI MainProc(LPVOID lpParam);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID /*reserved*/) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, &MainProc, nullptr, 0, nullptr);
    }
    return TRUE;
}
