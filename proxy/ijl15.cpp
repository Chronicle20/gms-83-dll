/*
 This file is part of GMS-83-DLL.

 GMS-83-DLL is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

 GMS-83-DLL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
 */

#include "framework.h"
#include "ijl15.h"

#define LIB_NAME	"ijl15.dll.bak"
#define LIB_EXPORT	extern "C" __declspec(dllexport)

DWORD ijlErrorStr_Proc;
DWORD ijlFree_Proc;
DWORD ijlGetLibVersion_Proc;
DWORD ijlInit_Proc;
DWORD ijlRead_Proc;
DWORD ijlWrite_Proc;

BOOL InitializeIjl15()
{
	DWORD dw;
	HANDLE hOrg = CreateFileA(LIB_NAME, (GENERIC_READ | GENERIC_WRITE), NULL, nullptr, CREATE_ALWAYS, NULL, nullptr);

	if (hOrg)
	{
		WriteFile(hOrg, g_Ijl15_Raw, sizeof(g_Ijl15_Raw), &dw, nullptr);
		CloseHandle(hOrg);
	}
	else
	{
		OutputDebugStringA("ijl15: Unable to write " LIB_NAME " to disk");
	}

	HMODULE SeData_Base = LoadLibraryA(LIB_NAME);

	if (SeData_Base)
	{
		ijlErrorStr_Proc = (DWORD)GetProcAddress(SeData_Base, "ijlErrorStr");
		ijlFree_Proc = (DWORD)GetProcAddress(SeData_Base, "ijlFree");
		ijlGetLibVersion_Proc = (DWORD)GetProcAddress(SeData_Base, "ijlGetLibVersion");
		ijlInit_Proc = (DWORD)GetProcAddress(SeData_Base, "ijlInit");
		ijlRead_Proc = (DWORD)GetProcAddress(SeData_Base, "ijlRead");
		ijlWrite_Proc = (DWORD)GetProcAddress(SeData_Base, "ijlWrite");

		return TRUE;
	}

	return FALSE;
}

BOOL g_InitIjl15 = InitializeIjl15(); //This is the static initializer !!!

LIB_EXPORT void ijlGetLibVersion()
{
	__asm	jmp dword ptr[ijlGetLibVersion_Proc] // make sure you're compiling in x86
}

LIB_EXPORT void ijlInit()
{
	__asm	jmp dword ptr[ijlInit_Proc]
}

LIB_EXPORT void ijlFree()
{
	__asm	jmp dword ptr[ijlFree_Proc]
}

LIB_EXPORT void ijlRead()
{
	__asm	jmp dword ptr[ijlRead_Proc]
}

LIB_EXPORT void ijlWrite()
{
	__asm	jmp dword ptr[ijlWrite_Proc]
}

LIB_EXPORT void ijlErrorStr()
{
	__asm	jmp dword ptr[ijlErrorStr_Proc]
}
