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

/// Fail-fast maple hook: like INITMAPLEHOOK, but returns FALSE from the
/// enclosing function when Detours rejects the hook. Intended for use inside
/// the per-category `BOOL Install<Category>Hooks()` installers; bypass_main's
/// MainProc checks each and short-circuits the chain on the first failure.
/// A dwAddress of 0 is still a soft skip, not a failure.
#define INITMAPLEHOOK_OR_RETURN(pOrigFunc, Func_t, pNewFunc, dwAddress)                                                \
    do {                                                                                                               \
        if ((dwAddress) == 0) {                                                                                        \
            Log("Skipping hook (unsupported on this build): %s", #pOrigFunc);                                          \
            break;                                                                                                     \
        }                                                                                                              \
        pOrigFunc = reinterpret_cast<Func_t>(dwAddress);                                                               \
        if (!SetHook(TRUE, reinterpret_cast<void**>(&pOrigFunc), pNewFunc)) {                                          \
            Log("Failed to hook %s at 0x%08X -- aborting installer", #pOrigFunc, (DWORD)(dwAddress));                  \
            return FALSE;                                                                                              \
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