#include <Windows.h>
#include <memedit.h>
#include "memory_map.h"

// main thread
DWORD WINAPI MainProc(LPVOID lpParam) {
    if (strcmp(BUILD_REGION, "GMS") == 0) {
        auto *instance = reinterpret_cast<unsigned int *>(C_CONFIG_SYS_OPT_WINDOWED_MODE);
        *instance = 0;
    } else if (strcmp(BUILD_REGION, "JMS") == 0) {
        if (C_WVS_APP_INITIALIZE_GR2D != 0 && C_WVS_APP_INITIALIZE_GR2D_WINDOWED_OFFSET != 0) {
            constexpr BYTE windowedPatch[] = {0xC7, 0x45, 0xDC, 0x00, 0x00, 0x00, 0x00};
            MemEdit::WriteBytes(C_WVS_APP_INITIALIZE_GR2D + C_WVS_APP_INITIALIZE_GR2D_WINDOWED_OFFSET, windowedPatch);
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