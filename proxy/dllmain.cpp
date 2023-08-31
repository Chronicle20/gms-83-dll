#include "Common.h"
#include <iostream>
#include <string>

extern "C" __declspec(dllexport)
void LoadDLLsFromDirectory(const std::string &directory) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile((directory + "\\*.dll").c_str(), &findFileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            HMODULE hModule = LoadLibrary((directory + findFileData.cFileName).c_str());
            if (hModule) {
                Log("Loaded: %s", (directory + findFileData.cFileName).c_str());
            } else {
                Log("Failed to load: %s", (directory + findFileData.cFileName).c_str());
            }
        } while (FindNextFile(hFind, &findFileData) != 0);
        FindClose(hFind);
    }
}

// executed after the client is unpacked
VOID MainFunc() {
    LoadDLLsFromDirectory("edits/");
}

VOID __stdcall MainProc() {
    Log(__FUNCTION__);

    Common::CreateInstance
            (
                    TRUE,            // true if you want to hook windows libraries (besides mutex) set this to false if you already edited your IP into the client (eg v83 localhosts)
                    MainFunc        // function to be executed after client is unpacked
            );
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
