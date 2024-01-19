/*
 This file is part of GMS-83-DLL.

 GMS-83-DLL is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

 GMS-83-DLL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
 */

#include "winhook_types.h"

CreateFileA_t CreateFileA_Original;
WinExec_t WinExec_Original;
GetProcAddress_t GetProcAddress_Original;
CreateMutexA_t CreateMutexA_Original;
OpenMutexA_t OpenMutexA_Original;
WSPStartup_t WSPStartup_Original;
RegisterClassExA_t RegisterClassExA_Original;
CreateProcessW_t CreateProcessW_Original;
CreateProcessA_t CreateProcessA_Original;
OpenProcess_t OpenProcess_Original;
CreateThread_t CreateThread_Original;
GetACP_t GetACP_Original;
CreateWindowExA_t CreateWindowExA_Original;
NtTerminateProcess_t NtTerminateProcess_Original;
RegCreateKeyExA_t RegCreateKeyExA_Original;