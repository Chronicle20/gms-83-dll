#include <Windows.h>
#include <memedit.h>

const DWORD dwSendMigrateToITCRequest = 0x00A12522;

// main thread
VOID __stdcall MainProc()
{
    BYTE* bArr = new BYTE[1];
    bArr[0] = 0xEB;
    MemEdit::WriteBytes(dwSendMigrateToITCRequest+0xE9, bArr, 1);
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