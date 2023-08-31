#pragma once
#include <WinSock2.h>
#include <Windows.h>
#include <WS2spi.h>

namespace WinHooks
{
	/// <summary>
	/// Used to map out imports used by MapleStory.
	/// The log output can be used to reconstruct the _ZAPIProcAddress struct
	/// ZAPI struct is the dword before the while loop when searching for aob: 68 FE 00 00 00 ?? 8D
	/// </summary>
	FARPROC WINAPI GetProcAddress_Hook(HMODULE hModule, LPCSTR lpProcName);

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
	/// In some versions, Maple calls this library function to check if the anticheat has started.
	/// We can spoof this and return a fake handle for it to close.
	/// </summary>
	HANDLE WINAPI OpenMutexA_Hook(
		DWORD  dwDesiredAccess,
		BOOL   bInitialOwner,
		LPCSTR lpName
	);

	/// <summary>
	/// Used to track what maple is trying to start (mainly for anticheat modules).
	/// </summary>
	BOOL WINAPI CreateProcessW_Hook(
		LPCWSTR               lpApplicationName,
		LPWSTR                lpCommandLine,
		LPSECURITY_ATTRIBUTES lpProcessAttributes,
		LPSECURITY_ATTRIBUTES lpThreadAttributes,
		BOOL                  bInheritHandles,
		DWORD                 dwCreationFlags,
		LPVOID                lpEnvironment,
		LPCWSTR               lpCurrentDirectory,
		LPSTARTUPINFOW        lpStartupInfo,
		LPPROCESS_INFORMATION lpProcessInformation
	);

	/// <summary>
	/// Used same as above and also to kill/redirect some web requests.
	/// </summary>
	BOOL WINAPI CreateProcessA_Hook(
		LPCSTR                lpApplicationName,
		LPSTR                 lpCommandLine,
		LPSECURITY_ATTRIBUTES lpProcessAttributes,
		LPSECURITY_ATTRIBUTES lpThreadAttributes,
		BOOL                  bInheritHandles,
		DWORD                 dwCreationFlags,
		LPVOID                lpEnvironment,
		LPCSTR                lpCurrentDirectory,
		LPSTARTUPINFOA        lpStartupInfo,
		LPPROCESS_INFORMATION lpProcessInformation
	);

	/// <summary>
	/// Same as CreateProcessW
	/// </summary>
	HANDLE WINAPI CreateThread_Hook(
		LPSECURITY_ATTRIBUTES   lpThreadAttributes,
		SIZE_T                  dwStackSize,
		LPTHREAD_START_ROUTINE  lpStartAddress,
		__drv_aliasesMem LPVOID lpParameter,
		DWORD                   dwCreationFlags,
		LPDWORD                 lpThreadId
	);

	/// <summary>
	/// Used to track what processes Maple opens.
	/// </summary>
	HANDLE WINAPI OpenProcess_Hook(
		DWORD dwDesiredAccess,
		BOOL  bInheritHandle,
		DWORD dwProcessId
	);

	/// <summary>
	/// This library call is used by nexon to determine the locale of the connecting clients PC. We spoof it.
	/// </summary>
	/// <returns></returns>
	UINT WINAPI GetACP_Hook(); // AOB: FF 15 ?? ?? ?? ?? 3D ?? ?? ?? 00 00 74 <- library call inside winmain func

	/// <summary>
	/// We use this function to track what memory addresses are killing the process.
	/// There are more ways that Maple kills itself, but this is one of them.
	/// </summary>
	LONG NTAPI NtTerminateProcess_Hook(
		HANDLE hProcHandle,
		LONG   ntExitStatus
	);

	/// <summary>
	/// Maplestory saves registry information (config stuff) for a number of things. This can be used to track that.
	/// </summary>
	LSTATUS WINAPI RegCreateKeyExA_Hook(
		HKEY                        hKey,
		LPCSTR                      lpSubKey,
		DWORD                       Reserved,
		LPSTR                       lpClass,
		DWORD                       dwOptions,
		REGSAM                      samDesired,
		const LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		PHKEY                       phkResult,
		LPDWORD                     lpdwDisposition
	);
}