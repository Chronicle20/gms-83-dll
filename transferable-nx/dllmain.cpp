#include <Windows.h>
#include <memedit.h>

const DWORD dwCDraggableItemThrowItem = 0x004F31A5;
const DWORD dwCashItemCheck = dwCDraggableItemThrowItem + 0x367;
const DWORD dwCashItemCheck2 = dwCDraggableItemThrowItem + 0x379;

// main thread
VOID __stdcall MainProc() {
    // Noop CItemInfo::IsCashItem check
    MemEdit::PatchNop(dwCashItemCheck, 6);
    MemEdit::PatchNop(dwCashItemCheck2, 6);
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