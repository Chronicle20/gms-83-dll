#pragma once

// Exclude rarely-used stuff from Windows headers
// Important to define this before Windows.h is included in a project because of linker issues with the WinSock2 lib
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <functional>
#include "hooker.h"
#include "logger.h"
#include "winhooks.h"
#include "winhook_types.h"
#include "FakeModule.h"

#define MAPLE_INJECT_USE_IJL TRUE
#define MAPLE_MULTICLIENT FALSE
#define MAPLE_INSTAJECT FALSE

/// <summary>
/// 
/// </summary>
class Common
{
private:
	struct Config
	{
	private:
		struct WinHooks
		{
			/* define toggles for logging and other behavior separately */
		public:
			BOOL OpenMutexA_Spoof;

			BOOL WSPConnect_Logging;
			BOOL NtTerminateProc_Logging;
			BOOL OpenProcess_Logging;
			BOOL CreateProcess_Logging;
			BOOL OpenMutexA_Logging;

			WinHooks()
			{
				OpenMutexA_Spoof = TRUE;
				WSPConnect_Logging = TRUE;
				NtTerminateProc_Logging = TRUE;
				OpenProcess_Logging = FALSE;
				CreateProcess_Logging = TRUE;
				OpenMutexA_Logging = TRUE;
			}
		};
	public:
		const char* DllName = "ijl15.dll";
		const char* MapleExeName = "MapleStory.exe";
		const char* MapleStartupArgs = " GameLaunching 127.0.0.1 8484";

		const char* MapleExitWindowWebUrl = "http";
		const char* MapleWindowClass = "MapleStoryClass";
		const char* MaplePatcherClass = "StartUpDlgClass";
		const char* MapleMutex = "WvsClientMtx";

		DWORD LocaleSpoofValue;
		DWORD SleepAfterUnpackDuration;

		BOOL  ForceWindowedOnStart;
		BOOL  InjectImmediately;
		BOOL  AllowMulticlient;

		Common::Config::WinHooks HookToggleInfo;

		Config()
		{
			HookToggleInfo = WinHooks();

			LocaleSpoofValue = 0;
			SleepAfterUnpackDuration = 0;
			ForceWindowedOnStart = TRUE;
			InjectImmediately = FALSE;
			AllowMulticlient = TRUE;
		}
	};

private:
	static Common* _s_pInstance;
	static Common::Config* _s_pConfig;

public: // public because all the C-style hooks have to access these members
	/* TODO throw all the winsock stuff into its own class */
	DWORD			m_dwGetProcRetAddr;
	BOOL			m_bThemidaUnpacked;
	FakeModule* m_pFakeHsModule;

	/// <summary>
	/// Gets called when mutex hook is triggered.
	/// </summary>
	std::function<void()> m_PostMutexFunc;

private: // forcing the class to only have one instance, created through CreateInstance
	Common(BOOL bHookWinLibs, std::function<void()> pPostMutexFunc);
	Common() = delete;
	Common(const Common&) = delete;
	Common& operator =(const Common&) = delete;

public:
	~Common();

	/// <summary>
	/// Function called from library hooks.
	/// Most of the time this should be triggered by the Mutex hook, however, in the case that
	/// the Mutex hook does not get triggered then this will be executed by CreateWindowExA
	/// for redundancy. The contents of this function will only be executed once, even if both 
	/// Mutex and CreateWindow hooks are called properly.
	/// </summary>
	void OnThemidaUnpack();

public:
	static void CreateInstance(BOOL bHookWinLibs, std::function<void()> pMutexFunc)
	{
		if (_s_pInstance) return;

		_s_pInstance = new Common(bHookWinLibs, pMutexFunc);
	}

	static Common* GetInstance()
	{
		return _s_pInstance;
	}

	static Config* GetConfig()
	{
		if (!_s_pConfig) _s_pConfig = new Config();

		return _s_pConfig;
	}
};

