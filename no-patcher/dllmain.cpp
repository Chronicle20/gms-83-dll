#include <Windows.h>
#include <memedit.h>

const DWORD dwWinMain = 0x009F19F2;
const DWORD dwWinMainShowStartUpWndModalCall = dwWinMain + 0x212; // 0x009F1C04

// main thread
VOID __stdcall MainProc()
{
    // Noop Patcher
    MemEdit::PatchNop(dwWinMainShowStartUpWndModalCall, 5);
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