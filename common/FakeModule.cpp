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
