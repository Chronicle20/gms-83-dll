/*
 This file is part of GMS-83-DLL.

 GMS-83-DLL is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

 GMS-83-DLL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <vector>
#include "winhooks.h"
#include "winhook_types.h"
#include "hooker.h"
#include "logger.h"
#include "memedit.h"
#include "FakeModule.h"
#include "Common.h"
#include<intrin.h>
#pragma intrinsic(_ReturnAddress)
#include<tlhelp32.h>

namespace WinHooks {
    // fix returnaddress func
    // https://docs.microsoft.com/en-us/cpp/intrinsics/returnaddress?view=msvc-160
#pragma intrinsic(_ReturnAddress)

// link socket library
#pragma comment(lib, "WS2_32.lib")

// deprecated api call warning
#pragma warning(disable : 4996)

    /// <summary>
    /// CreateMutexA is the first Windows library call after the executable unpacks itself.
    /// We hook this function to do all our memory edits and hooks when it's called.
    /// </summary>
    HANDLE WINAPI CreateMutexA_Hook(
            LPSECURITY_ATTRIBUTES lpMutexAttributes,
            BOOL bInitialOwner,
            LPCSTR lpName
    ) {
        Log("[CreateMutex] Called with name: %s", lpName);
        if (!CreateMutexA_Original) {
            Log("Original CreateMutex pointer corrupted. Failed to return mutex value to calling function.");

            return nullptr;
        } else if (lpName && strstr(lpName, Common::GetConfig()->MapleMutex)) {
#if MAPLE_MULTICLIENT
            // from https://github.com/pokiuwu/AuthHook-v203.4/blob/AuthHook-v203.4/Client176/WinHook.cpp

            char szMutex[128];
            int nPID = GetCurrentProcessId();

            sprintf_s(szMutex, "%s-%d", lpName, nPID);
            lpName = szMutex;
#endif

            Common::GetInstance()->OnThemidaUnpack();
            return CreateMutexA_Original(lpMutexAttributes, bInitialOwner, lpName);
        }

        return CreateMutexA_Original(lpMutexAttributes, bInitialOwner, lpName);
    }

    std::vector<MEMORY_BASIC_INFORMATION> section_list;

    bool InitSectionInformation() {
        DWORD pid = GetCurrentProcessId();

        if (!pid) {
            return false;
        }

        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);

        if (hSnapshot == INVALID_HANDLE_VALUE) {
            return false;
        }

        MODULEENTRY32W me;
        memset(&me, 0, sizeof(me));
        me.dwSize = sizeof(me);
        if (!Module32FirstW(hSnapshot, &me)) {
            return false;
        }

        std::vector<MODULEENTRY32W> module_list;
        do {
            module_list.push_back(me);
        } while (Module32NextW(hSnapshot, &me));

        CloseHandle(hSnapshot);

        // first module may be exe
        if (module_list.size() < 0) {
            return false;
        }

        MEMORY_BASIC_INFORMATION mbi;
        memset(&mbi, 0, sizeof(mbi));

        ULONG_PTR section_base = (ULONG_PTR)module_list[0].modBaseAddr;

        while (section_base < ((ULONG_PTR)module_list[0].modBaseAddr + module_list[0].modBaseSize) && (VirtualQuery((void *)section_base, &mbi, sizeof(mbi)) == sizeof(mbi))) {
            section_list.push_back(mbi);
            section_base += mbi.RegionSize;
        }

        if (!section_list.size()) {
            return false;
        }

        return true;
    }

    int IsCallerEXE(void *vReturnAddress) {
        // init
        if (section_list.size() == 0) {
            InitSectionInformation();
        }

        // check addresss
        for (size_t i = 0; i < section_list.size(); i++) {
            if ((ULONG_PTR)section_list[i].BaseAddress <= (ULONG_PTR)vReturnAddress && (ULONG_PTR)vReturnAddress <= ((ULONG_PTR)section_list[i].BaseAddress + section_list[i].RegionSize)) {
                return (int)i;
            }
        }

        return 0;
    }

    VOID WINAPI GetStartupInfoA_Hook(LPSTARTUPINFOA lpStartupInfo) {
        if (lpStartupInfo && IsCallerEXE(_ReturnAddress())) {
            Common::GetInstance()->OnThemidaUnpack();
        }
        GetStartupInfoA_Original(lpStartupInfo);
    }

    HMODULE WINAPI GetModuleHandleA_Hook(LPCSTR lpModuleName) {
        if (lpModuleName && !strcmp("ehsvc.dll", lpModuleName)) {
            return (HMODULE) 1;
        }

        // Call the original function
        if (GetModuleHandleA_Original)
            return GetModuleHandleA_Original(lpModuleName);

        return nullptr;
    }

    /// <summary>
    /// This library call is used by nexon to determine the locale of the connecting clients PC. We spoof it.
    /// </summary>
    /// <returns></returns>
    UINT WINAPI GetACP_Hook() // AOB: FF 15 ?? ?? ?? ?? 3D ?? ?? ?? 00 00 74 <- library call inside winmain func
    {
        UINT uiNewLocale = Common::GetConfig()->LocaleSpoofValue;

        if (!uiNewLocale) return GetACP_Original(); // should not happen cuz we dont hook if value is zero

        // we dont wanna unhook until after themida is unpacked
        // because if themida isn't unpacked then the call we are intercepting is not from maple
        if (Common::GetInstance()->m_bThemidaUnpacked) {
            DWORD dwRetAddr = reinterpret_cast<DWORD>(_ReturnAddress());

            // return address should be a cmp eax instruction because ret value is stored in eax
            // and nothing else should happen before the cmp
            if (*MemEdit::ReadValue<BYTE>(dwRetAddr) == x86CMPEAX) {
                uiNewLocale = *MemEdit::ReadValue<DWORD>(dwRetAddr + 1); // check value is always 4 bytes

                Log("[GetACP] Found desired locale: %d", uiNewLocale);
            } else {
                Log("[GetACP] Unable to automatically determine locale, using stored locale: %d", uiNewLocale);
            }

            Log("[GetACP] Locale spoofed to %d, unhooking. Calling address: %08X", uiNewLocale, dwRetAddr);

            if (!SetHook(FALSE, reinterpret_cast<void **>(&GetACP_Original), GetACP_Hook)) {
                Log("Failed to unhook GetACP.");
            }
        }

        return uiNewLocale;
    }
}