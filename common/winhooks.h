/*
 This file is part of GMS-83-DLL.

 GMS-83-DLL is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

 GMS-83-DLL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <WinSock2.h>
#include <windows.h>
#include <WS2spi.h>

namespace WinHooks {
    /// <summary>
    /// CreateMutexA is the first Windows library call after the executable unpacks itself.
    /// We hook this function to do all our memory edits and hooks when it's called.
    /// </summary>
    HANDLE WINAPI CreateMutexA_Hook(
            LPSECURITY_ATTRIBUTES lpMutexAttributes,
            BOOL bInitialOwner,
            LPCSTR lpName
    );

    VOID WINAPI GetStartupInfoA_Hook(
            LPSTARTUPINFOA lpStartupInfo
    );

    HMODULE WINAPI GetModuleHandleA_Hook(
            LPCSTR lpModuleName
    );

    /// <summary>
    /// This library call is used by nexon to determine the locale of the connecting clients PC. We spoof it.
    /// </summary>
    /// <returns></returns>
    UINT WINAPI GetACP_Hook(); // AOB: FF 15 ?? ?? ?? ?? 3D ?? ?? ?? 00 00 74 <- library call inside winmain func
}