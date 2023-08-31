#include "Common.h"

Common* Common::_s_pInstance;
Common::Config* Common::_s_pConfig;

Common::Common(BOOL bHookWinLibs, std::function<void()> pPostMutexFunc)
{
	this->m_bThemidaUnpacked = FALSE;
	this->m_dwGetProcRetAddr = 0;

	if (!pPostMutexFunc)
	{
#if _DEBUG
		Log("Invalid function pointer passed to Common constructor.");
#endif
		return;
	}

	if (this->GetConfig()->InjectImmediately) // call post-unpack function right away
	{
		pPostMutexFunc();
	}
	else // set pointer to function that is executed after client unpacks itself
	{
		this->m_PostMutexFunc = pPostMutexFunc;
	}

	// required for proper injection
	INITWINHOOK("KERNEL32", "CreateMutexA", CreateMutexA_Original, CreateMutexA_t, WinHooks::CreateMutexA_Hook);

	if (Common::GetConfig()->LocaleSpoofValue)
	{
		INITWINHOOK("KERNEL32", "GetACP", GetACP_Original, GetACP_t, WinHooks::GetACP_Hook);
	}

	if (Common::GetConfig()->HookToggleInfo.OpenProcess_Logging)
	{
		INITWINHOOK("KERNEL32", "OpenProcess", OpenProcess_Original, OpenProcess_t, WinHooks::OpenProcess_Hook);
	}

	if (Common::GetConfig()->HookToggleInfo.OpenMutexA_Logging || Common::GetConfig()->HookToggleInfo.OpenMutexA_Spoof)
	{
		INITWINHOOK("KERNEL32", "OpenMutexA", OpenMutexA_Original, OpenMutexA_t, WinHooks::OpenMutexA_Hook);
	}

	if (Common::GetConfig()->HookToggleInfo.NtTerminateProc_Logging)
	{
		INITWINHOOK("NTDLL", "NtTerminateProcess", NtTerminateProcess_Original, NtTerminateProcess_t, WinHooks::NtTerminateProcess_Hook);
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
	if (Common::GetConfig()->InjectImmediately) return;

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
