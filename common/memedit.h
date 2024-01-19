/*
 This file is part of GMS-83-DLL.

 GMS-83-DLL is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

 GMS-83-DLL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once
#include <Windows.h>
#include <cstdlib>

#include "asserts.h"

// thanks raj for some of these

#define x86CMPEAX 0x3D
#define x86XOR 0x33
#define x86EAXEAX 0xC0
#define x86RET 0xC3
#define x86JMP 0xE9
#define x86CALL 0xE8
#define x86NOP 0x90


class MemEdit {
private:

	// need to pack this so it doesnt auto-align to 8 bytes
#pragma pack(1)
	typedef struct patch_call
	{
		BYTE nPatchType;
		DWORD dwAddress;
	} patch_far_jmp;
#pragma pack()
	assert_size(sizeof(patch_call), 0x5);

public:
	static BOOL PatchRetZero(DWORD dwAddress);
	static BOOL PatchJmp(DWORD dwAddress, PVOID pDestination);
	static BOOL PatchCall(DWORD dwAddress, PVOID pDestination);
	static BOOL PatchNop(DWORD dwAddress, UINT nCount);
	static BOOL WriteBytes(DWORD dwAddress, LPCVOID pData, UINT nCount);

	static void FillBytes(DWORD dwOriginAddress, unsigned char ucValue, int nCount);
	static void WriteByte(DWORD dwOriginAddress, unsigned char ucValue);
	static void WriteInt(DWORD dwOriginAddress, unsigned int dwValue);
	static void CodeCave(void* ptrCodeCave, DWORD dwOriginAddress, int nNOPCount);

	/// <summary>
	/// Attempts to (over)write a value to the specified location in memory.
	/// </summary>
	/// <typeparam name="TType">Type that will be written</typeparam>
	/// <param name="dwAddress">Address to write to</param>
	/// <param name="pValue">Pointer to the value to be written</param>
	/// <returns>True if write operation was successful, otherwise false.</returns>
	template <typename TType>
	static BOOL WriteValue(DWORD dwAddress, TType* pValue)
	{
		// https://stackoverflow.com/a/13026295/14784253
		DWORD dwOldValue, dwTemp;

		VirtualProtect((LPVOID)dwAddress, sizeof(TType), PAGE_EXECUTE_READWRITE, &dwOldValue);
		BOOL bSuccess = WriteProcessMemory(GetCurrentProcess(), (LPVOID)dwAddress, (void*)pValue, sizeof(TType), NULL);
		VirtualProtect((LPVOID)dwAddress, sizeof(TType), dwOldValue, &dwTemp);
		return bSuccess;
	}

	/// <summary>
	/// Reads the value at the given memory location and returns a pointer to it.
	/// </summary>
	/// <typeparam name="TType">Type that will be read</typeparam>
	/// <param name="dwAddr"></param>
	/// <returns>Pointer to the value at the given location</returns>
	template <typename TType>
	static TType* ReadValue(DWORD dwAddr)
	{
		return reinterpret_cast<TType*>(dwAddr);
	}
};