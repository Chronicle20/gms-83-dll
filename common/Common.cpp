/*
 This file is part of GMS-83-DLL.

 GMS-83-DLL is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

 GMS-83-DLL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
 */

#include "Common.h"

Common* Common::_s_pInstance;
Common::Config* Common::_s_pConfig;

Common::Common(BOOL bHookWinLibs, std::function<void()> pPostMutexFunc)
{
	this->m_bThemidaUnpacked = FALSE;

	if (!pPostMutexFunc)
	{
#if _DEBUG
		Log("Invalid function pointer passed to Common constructor.");
#endif
		return;
	}

    this->m_PostMutexFunc = pPostMutexFunc;

	// required for proper injection
	INITWINHOOK("KERNEL32", "CreateMutexA", CreateMutexA_Original, CreateMutexA_t, WinHooks::CreateMutexA_Hook);

	if (Common::GetConfig()->LocaleSpoofValue)
	{
		INITWINHOOK("KERNEL32", "GetACP", GetACP_Original, GetACP_t, WinHooks::GetACP_Hook);
	}

	if (!bHookWinLibs) return;
}

Common::~Common()
{
#if _DEBUG
	Log("Cleaning up common..");
#endif

	if (this->m_pFakeHsModule)
	{
		// TODO figure out some common library call to put this instead of in dll detach
		// CLogo constructor is pretty good but its not a library call so idk
		this->m_pFakeHsModule->DeleteModule();
	}
}

void Common::OnThemidaUnpack()
{
	if (this->m_bThemidaUnpacked) return;

	this->m_bThemidaUnpacked = TRUE;

	if (Common::GetConfig()->SleepAfterUnpackDuration)
	{
		Log("Themida unpacked => sleeping for %d milliseconds.", Common::GetConfig()->SleepAfterUnpackDuration);
		Sleep(Common::GetConfig()->SleepAfterUnpackDuration);
	}

#if _DEBUG
	Log("Themida unpacked, editing memory..");
#endif

	this->m_PostMutexFunc();
}
