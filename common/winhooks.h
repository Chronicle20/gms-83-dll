#pragma once
#include <WinSock2.h>
#include <windows.h>
#include <WS2spi.h>

namespace WinHooks
{
	/// <summary>
	/// CreateMutexA is the first Windows library call after the executable unpacks itself.
	/// We hook this function to do all our memory edits and hooks when it's called.
	/// </summary>
	HANDLE WINAPI CreateMutexA_Hook(
		LPSECURITY_ATTRIBUTES lpMutexAttributes,
		BOOL				  bInitialOwner,
		LPCSTR				  lpName
	);

	/// <summary>
	/// This library call is used by nexon to determine the locale of the connecting clients PC. We spoof it.
	/// </summary>
	/// <returns></returns>
	UINT WINAPI GetACP_Hook(); // AOB: FF 15 ?? ?? ?? ?? 3D ?? ?? ?? 00 00 74 <- library call inside winmain func
}