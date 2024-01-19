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

/// Sets hook and outputs result to debug window (pass/fail)
#define INITWINHOOK(sModName, sFuncName, pOrigFunc, Func_t, pNewFunc)		\
pOrigFunc = (Func_t)GetFuncAddress(sModName, sFuncName);					\
if (SetHook(TRUE, reinterpret_cast<void**>(&pOrigFunc), pNewFunc))			\
{ Log("Hooked %s", sFuncName); }											\
else																		\
{ Log("Failed to hook %s", sFuncName); } // end macro

#define INITMAPLEHOOK(pOrigFunc, Func_t, pNewFunc, dwAddress)				\
pOrigFunc = reinterpret_cast<Func_t>(dwAddress);							\
if (!SetHook(TRUE, reinterpret_cast<void**>(&pOrigFunc), pNewFunc))			\
{ Log("Failed to hook maple func at address %d", dwAddress); } // end macro

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