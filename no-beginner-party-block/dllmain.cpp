#include <Windows.h>
#include <memedit.h>

const DWORD dwSendJoinPartyMsg = 0x0052FECF;
const DWORD dwSendCreateNewPartyMsg = 0x52FCE1;

// main thread
VOID __stdcall MainProc()
{
    BYTE* bArr = new BYTE[1];
    bArr[0] = 0xEB;
    MemEdit::WriteBytes(dwSendJoinPartyMsg+0x65, bArr, 1);
    MemEdit::WriteBytes(dwSendCreateNewPartyMsg+0xA4, bArr, 1);
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