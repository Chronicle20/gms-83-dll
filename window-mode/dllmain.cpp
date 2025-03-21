/*
 This file is part of GMS-83-DLL.

 GMS-83-DLL is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

 GMS-83-DLL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
 */

#include <Windows.h>
#include <memedit.h>
#include "memory_map.h"

__declspec(naked) void FixFullScreen() {
    __asm {
            mov eax, 0
            jmp dword ptr[C_WVS_APP_INITIALIZE_GR2D + C_WVS_APP_INITIALIZE_GR2D_FULL_SCREEN_RETURN]
    }
}

// main thread
VOID __stdcall MainProc() {
    // Window Mode Magic
    MemEdit::CodeCave(FixFullScreen, C_WVS_APP_INITIALIZE_GR2D + C_WVS_APP_INITIALIZE_GR2D_FULL_SCREEN_OFFSET, 5);
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