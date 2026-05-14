/*
 This file is part of GMS-83-DLL.

 GMS-83-DLL is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

 GMS-83-DLL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once
#include <Windows.h>
#include "detours.h"

#pragma comment(lib, "detours.lib")

#pragma region Macros

/// Best-effort maple hook: logs success/fail. Skips with INFO when dwAddress == 0
/// (legitimate "not present on this build" state in the memory map).
#define INITMAPLEHOOK(pOrigFunc, Func_t, pNewFunc, dwAddress)                                                          \
    do {                                                                                                               \
        if ((dwAddress) == 0) {                                                                                        \
            Log("Skipping hook (unsupported on this build): %s", #pOrigFunc);                                          \
            break;                                                                                                     \
        }                                                                                                              \
        pOrigFunc = reinterpret_cast<Func_t>(dwAddress);                                                               \
        if (!SetHook(TRUE, reinterpret_cast<void**>(&pOrigFunc), pNewFunc)) {                                          \
            Log("Failed to hook %s at 0x%08X", #pOrigFunc, (DWORD)(dwAddress));                                        \
        } else {                                                                                                       \
            Log("Hooked %s at 0x%08X", #pOrigFunc, (DWORD)(dwAddress));                                                \
        }                                                                                                              \
    } while (0)

/// Fail-fast maple hook: like INITMAPLEHOOK, but returns -1 from the enclosing
/// function (intended for use in MainProc) when Detours rejects the hook.
/// A dwAddress of 0 is still a soft skip, not a failure.
#define INITMAPLEHOOK_OR_RETURN(pOrigFunc, Func_t, pNewFunc, dwAddress)                                                \
    do {                                                                                                               \
        if ((dwAddress) == 0) {                                                                                        \
            Log("Skipping hook (unsupported on this build): %s", #pOrigFunc);                                          \
            break;                                                                                                     \
        }                                                                                                              \
        pOrigFunc = reinterpret_cast<Func_t>(dwAddress);                                                               \
        if (!SetHook(TRUE, reinterpret_cast<void**>(&pOrigFunc), pNewFunc)) {                                          \
            Log("Failed to hook %s at 0x%08X -- aborting MainProc", #pOrigFunc, (DWORD)(dwAddress));                   \
            return -1;                                                                                                 \
        }                                                                                                              \
        Log("Hooked %s at 0x%08X", #pOrigFunc, (DWORD)(dwAddress));                                                    \
    } while (0)

/// Best-effort Win32 hook: logs success/fail, never returns.
#define INITWINHOOK(sModName, sFuncName, pOrigFunc, Func_t, pNewFunc)                                                  \
    do {                                                                                                               \
        pOrigFunc = (Func_t)GetFuncAddress(sModName, sFuncName);                                                       \
        if (!pOrigFunc) {                                                                                              \
            Log("GetFuncAddress failed for %s!%s", sModName, sFuncName);                                               \
            break;                                                                                                     \
        }                                                                                                              \
        if (SetHook(TRUE, reinterpret_cast<void**>(&pOrigFunc), pNewFunc)) {                                           \
            Log("Hooked %s!%s", sModName, sFuncName);                                                                  \
        } else {                                                                                                       \
            Log("Failed to hook %s!%s", sModName, sFuncName);                                                          \
        }                                                                                                              \
    } while (0)

/// Fail-fast Win32 hook: returns -1 from the enclosing function on resolution or
/// install failure.
#define INITWINHOOK_OR_RETURN(sModName, sFuncName, pOrigFunc, Func_t, pNewFunc)                                        \
    do {                                                                                                               \
        pOrigFunc = (Func_t)GetFuncAddress(sModName, sFuncName);                                                       \
        if (!pOrigFunc) {                                                                                              \
            Log("GetFuncAddress failed for %s!%s -- aborting MainProc", sModName, sFuncName);                          \
            return -1;                                                                                                 \
        }                                                                                                              \
        if (!SetHook(TRUE, reinterpret_cast<void**>(&pOrigFunc), pNewFunc)) {                                          \
            Log("Failed to hook %s!%s -- aborting MainProc", sModName, sFuncName);                                     \
            return -1;                                                                                                 \
        }                                                                                                              \
        Log("Hooked %s!%s", sModName, sFuncName);                                                                      \
    } while (0)

#define HOOKTYPEDEF_FUNCTYPE(functionName) _##functionName##_t

// Use this macro in MapleAPI.h to define function types to use for hooks
// functionName expands to _functionName_t for the type name and _functionName for the reference name.
#define HOOKTYPEDEF_H(returnType, callingConvention, functionName, ...)						\
typedef returnType (callingConvention* HOOKTYPEDEF_FUNCTYPE(functionName))(__VA_ARGS__);	\
extern HOOKTYPEDEF_FUNCTYPE(functionName) _##functionName

// Use this macro in MapleAPI.cpp to complete the function types for hooks
#define HOOKTYPEDEF_C(functionName) HOOKTYPEDEF_FUNCTYPE(functionName) _##functionName

#pragma endregion

extern BOOL SetHook(bool bInstall, void** ppvTarget, void* pvDetour);
extern DWORD GetFuncAddress(const char* lpModule, const char* lpFunc);
extern PVOID HookVTableFunction(void* pVTable, void* fnHookFunc, int nOffset);
extern void FreeLoadedModules();