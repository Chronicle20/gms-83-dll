#include <Windows.h>
#include <memedit.h>
#include "memory_map.h"

// main thread
DWORD WINAPI MainProc(LPVOID lpParam) {
    // Noop Patcher
    if (strcmp(BUILD_REGION, "GMS") == 0) {
        MemEdit::PatchNop(WIN_MAIN + WIN_MAIN_PATCHER_OFFSET, 5);
    } else if (strcmp(BUILD_REGION, "JMS") == 0) {
        if (WIN_MAIN_LAUNCHER_STUB != 0) {
            constexpr BYTE retOne[] = {0xB8, 0x01, 0x00, 0x00, 0x00, 0xC3};
            MemEdit::WriteBytes(WIN_MAIN_LAUNCHER_STUB, retOne);
        }
    }
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