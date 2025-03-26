/*
 This file is part of GMS-83-DLL.

 GMS-83-DLL is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

 GMS-83-DLL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
 */

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

DWORD WINAPI MainProc(LPVOID lpParam) {
    Log(__FUNCTION__);

    Common::CreateInstance
            (
                    TRUE,            // true if you want to hook windows libraries (besides mutex) set this to false if you already edited your IP into the client (eg v83 localhosts)
                    MainFunc        // function to be executed after client is unpacked
            );
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
