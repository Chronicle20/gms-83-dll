/*
 This file is part of GMS-83-DLL.

 GMS-83-DLL is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

 GMS-83-DLL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
 */
#include "pch.h"

#include "logger.h"
#include "hooker.h"

// void __thiscall CMob::CMob(CMob *this, CMobTemplate *pMobTemplate)
typedef VOID (__thiscall *_CMob_CMob_t)(CMob *pThis, CMobTemplate *pMobTemplate);
_CMob_CMob_t _CMob_CMob;

VOID __fastcall CMob_CMob_Hook(CMob *pThis, PVOID edx, CMobTemplate *pMobTemplate) {
    _CMob_CMob(pThis, pMobTemplate);
    // primarily noticed in v83, may be broken earlier. fixed in v84
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION < 84)
    pThis->m_bDoomReserved = 0;
#endif
}

DWORD WINAPI MainProc(LPVOID lpParam) {
    INITMAPLEHOOK(_CMob_CMob, _CMob_CMob_t, CMob_CMob_Hook, C_MOB_C_MOB);
    return 0;
}

// dll entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            DisableThreadLibraryCalls(hModule);
            CreateThread(nullptr, 0, &MainProc, nullptr, 0, nullptr);
            break;
        }
    }
    return TRUE;
}
