/*
 This file is part of GMS-83-DLL.

 GMS-83-DLL is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

 GMS-83-DLL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
 */

// Exclude rarely-used stuff from Windows headers
// Important to define this before Windows.h is included in a project because of linker issues with the WinSock2 lib
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include "logger.h"
#include "hooker.h"

/// <summary>
/// https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexa
/// </summary>
typedef HWND(WINAPI* CreateWindowExA_t)(
        DWORD     dwExStyle,
        LPCSTR    lpClassName,
        LPCSTR    lpWindowName,
        DWORD     dwStyle,
        int       X,
        int       Y,
        int       nWidth,
        int       nHeight,
        HWND      hWndParent,
        HMENU     hMenu,
        HINSTANCE hInstance,
        LPVOID    lpParam
);
extern CreateWindowExA_t CreateWindowExA_Original;

CreateWindowExA_t CreateWindowExA_Original;

/// <summary>
/// Blocks the startup patcher "Play!" window and forces the login screen to be minimized
/// </summary>
HWND WINAPI CreateWindowExA_Hook(
        DWORD     dwExStyle,
        LPCSTR    lpClassName,
        LPCSTR    lpWindowName,
        DWORD     dwStyle,
        int       X,
        int       Y,
        int       nWidth,
        int       nHeight,
        HWND      hWndParent,
        HMENU     hMenu,
        HINSTANCE hInstance,
        LPVOID    lpParam
)
{
    Log("[CreateWindowExA] => %s - %s", lpClassName, lpWindowName);
    dwExStyle = 0;
    dwStyle |= WS_MINIMIZEBOX; // enable minimize button
    return CreateWindowExA_Original(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

// main thread
VOID __stdcall MainProc()
{
    INITWINHOOK("USER32", "CreateWindowExA", CreateWindowExA_Original, CreateWindowExA_t, CreateWindowExA_Hook);
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