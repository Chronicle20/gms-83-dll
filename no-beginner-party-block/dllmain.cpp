#include <Windows.h>
#include <memedit.h>
#include "memory_map.h"

// main thread
DWORD WINAPI MainProc(LPVOID lpParam) {
    constexpr BYTE jmpShort[] = {0xEB};
    MemEdit::WriteBytes(C_FIELD_SEND_JOIN_PARTY_MSG + C_FIELD_SEND_JOIN_PARTY_MSG_OFFSET, jmpShort);
    MemEdit::WriteBytes(C_FIELD_SEND_CREATE_NEW_PARTY_MSG + C_FIELD_SEND_CREATE_NEW_PARTY_MSG_OFFSET, jmpShort);
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