#include "hooker.h"

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
	HMODULE hMod = LoadLibraryA(lpModule);

	return !hMod ? 0 : (DWORD)GetProcAddress(hMod, lpFunc);
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
