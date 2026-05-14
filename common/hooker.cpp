/*
 This file is part of GMS-83-DLL.

 GMS-83-DLL is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

 GMS-83-DLL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
 */

#include "hooker.h"
#include <mutex>
#include <unordered_set>

namespace {
std::mutex g_loadedMutex;
std::unordered_set<HMODULE> g_loadedByUs;
} // namespace

BOOL SetHook(bool bInstall, void** ppvTarget, void* pvDetour)
{
	if (DetourTransactionBegin() != NO_ERROR)
	{
		return FALSE;
	}

	HANDLE pCurThread = GetCurrentThread();

	if (DetourUpdateThread(pCurThread) == NO_ERROR)
	{
		auto pDetourFunc = bInstall ? DetourAttach : DetourDetach;

		if (pDetourFunc(ppvTarget, pvDetour) == NO_ERROR)
		{
			if (DetourTransactionCommit() == NO_ERROR)
			{
				return TRUE;
			}
		}
	}

	DetourTransactionAbort();
	return FALSE;
}

DWORD GetFuncAddress(const char* lpModule, const char* lpFunc)
{
    HMODULE hMod = GetModuleHandleA(lpModule);
    if (!hMod) {
        hMod = LoadLibraryA(lpModule);
        if (!hMod)
            return 0;

        std::lock_guard<std::mutex> lk(g_loadedMutex);
        g_loadedByUs.insert(hMod);
    }
    return (DWORD)GetProcAddress(hMod, lpFunc);
}

// Not called from DllMain: FreeLibrary inside DllMain is documented as deadlock-prone
// (loader lock recursion). Provided for an explicit shutdown path; process exit reclaims
// otherwise.
void FreeLoadedModules() {
    std::unordered_set<HMODULE> toFree;
    {
        std::lock_guard<std::mutex> lk(g_loadedMutex);
        toFree.swap(g_loadedByUs);
    }
    for (HMODULE hMod : toFree) {
        FreeLibrary(hMod);
    }
}

// Credits: https://guidedhacking.com/threads/hook-vtable.13096/post-76763
PVOID HookVTableFunction(void* pVTable, void* fnHookFunc, int nOffset) {
	intptr_t ptrVtable = *((intptr_t*)pVTable); // Pointer to our chosen vtable
	intptr_t ptrFunction = ptrVtable + sizeof(intptr_t) * nOffset; // The offset to the function (remember it's a zero indexed array with a size of four bytes)
	intptr_t ptrOriginal = *((intptr_t*)ptrFunction); // Save original address

	// Edit the memory protection so we can modify it
	MEMORY_BASIC_INFORMATION mbi;
	VirtualQuery((LPCVOID)ptrFunction, &mbi, sizeof(mbi));
	VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &mbi.Protect);

	// Overwrite the old function with our new one
	*((intptr_t*)ptrFunction) = (intptr_t)fnHookFunc;

	// Restore the protection
	VirtualProtect(mbi.BaseAddress, mbi.RegionSize, mbi.Protect, &mbi.Protect);

	// Return the original function address incase we want to call it
	return (void*)ptrOriginal;
}
