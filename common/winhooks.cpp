#pragma once

#include "winhooks.h"
#include "winhook_types.h"
#include "hooker.h"
#include "logger.h"
#include "memedit.h"
#include "FakeModule.h"
#include "Common.h"

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