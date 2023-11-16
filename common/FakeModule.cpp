/*
 This file is part of GMS-83-DLL.

 GMS-83-DLL is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

 GMS-83-DLL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
 */

#include "FakeModule.h"

BOOL FakeModule::CreateModule(const char* szModuleName)
{
	this->szModuleName = szModuleName;

	HANDLE hWrite = CreateFile(
		szModuleName,
		(GENERIC_READ | GENERIC_WRITE),
		NULL,
		NULL,
		CREATE_ALWAYS,
		NULL,
		NULL
	);

	if (hWrite != INVALID_HANDLE_VALUE)
	{
		if (!WriteFile(hWrite, this->s_abFakeModuleBinary, sizeof(this->s_abFakeModuleBinary), NULL, NULL))
		{
			CloseHandle(hWrite);
			return FALSE;
		}

		CloseHandle(hWrite);
	}
	else
	{
		return FALSE;
	}

	this->hFakeModule = LoadLibrary(szModuleName);

	if (!this->hFakeModule)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL FakeModule::DeleteModule()
{
	if (!szModuleName || !hFakeModule) return FALSE;

	BOOL bRet = TRUE;

	bRet &= FreeLibrary(hFakeModule);
	bRet &= DeleteFile(TEXT(szModuleName));

	return bRet;
}
