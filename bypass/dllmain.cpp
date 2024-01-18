/*
 This file is part of GMS-83-DLL.

 GMS-83-DLL is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

 GMS-83-DLL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
 */
#include "pch.h"

#include <memedit.h>
#include <timeapi.h>
#include "logger.h"
#include "hooker.h"

const DWORD dwCClientSocketProcessPacket = 0x004965F1;
const DWORD dwCSecurityClientOnPacketCall = dwCClientSocketProcessPacket + 0x7F;

const DWORD dwCSecurityClientCreateInstance = 0x009F9F42;
const DWORD dwCSecurityClientInitModule = 0x00A4BB2B;
const DWORD dwCSecurityClientStartModule = 0x00A4BD91;

const DWORD dwCWvsAppInitializeGr2D = 0x009F7A3B;

const DWORD dwFixFullScreen = dwCWvsAppInitializeGr2D + 0x60; // 0x009F7A9B
const DWORD dwFixFullScreenReturn = dwCWvsAppInitializeGr2D + 0x65;

const DWORD dwCWvsAppSetUp = 0x009F5239;
const DWORD dwNukedCWvsAppSetupReturn = dwCWvsAppSetUp + 0xA08;

const DWORD dwCWvsAppInitializeInput = 0x009F7CE1;
const DWORD dwNukedCWvsAppInitializeInputReturn = dwCWvsAppInitializeInput + 0x52F;

const DWORD dwCWvsAppRun = 0x009F5C50;
const DWORD dwNukedCWvsAppRunReturn = dwCWvsAppRun + 0xD2F; //D2F

const DWORD dwCClientSocketConnect = 0x00494CA3;
const DWORD dwNukedCClientSocketConnectReturn = dwCClientSocketConnect + 0x5F; //D2F

const DWORD dwCWvsAppCallUpdate = 0x009F84D0;
const DWORD dwNukedCWvsAppCallUpdateReturn = dwCWvsAppCallUpdate + 0x680;

const DWORD dwEH_prolog = 0x00A60B98;
const DWORD dwCWvsAppInitializeAuth = 0x009F7097;
const DWORD dwSrand = 0x00A61C60;
const DWORD dwGetSEPrivilege = 0x0044E824;
const DWORD dwCWvsAppInitializePCOM = 0x009F6D77;
const DWORD dwCWvsAppCreateMainWindow = 0x009F6D97;
const DWORD dwCClientSocketCreateInstance = 0x009F9E53;
const DWORD dwCWvsAppConnectLogin = 0x009F6F27;
const DWORD dwCFuncKeyMappedManCreateInstance = 0x009F9E98;
const DWORD dwCQuickslotKeyMappedManCreateInstance = 0x009FA0CB;
const DWORD dwCMacroSysManCreateInstance = 0x009F9EEE;
const DWORD dwCWvsAppInitializeResMan = 0x009F7159;
const DWORD dwCWvsAppInitializeSound = 0x009F82BC;
const DWORD dwCWvsAppInitializeGameData = 0x009F8B61;
const DWORD dwCWvsAppCreateWndManager = 0x009F7034;
const DWORD dwCConfigApplySysOpt = 0x0049EA33;
const DWORD dwCActionManCreateInstance = 0x009F9DA6;
const DWORD dwCMapleTVManInit = 0x00636F4E;
const DWORD dwCQuestManCreateInstance = 0x009F9AC2;
const DWORD dwCQuestManLoadDemand = 0x0071D8DF;
const DWORD dwCxxThrowException = 0x00A60BB7;
const DWORD dwCQuestManLoadPartyQuestInfo = 0x00723341;
const DWORD dwCQuestManLoadExclusive = 0x007247A1;
const DWORD dwCMonsterBookManCreateInstance = 0x009F9B73;
const DWORD dwCMonsterBookManLoadBook = 0x0068487C;
const DWORD dwCRadioManagerCreateInstance = 0x009FA078;
const DWORD dwCWvsAppDir_BackSlashToSlash = 0x009F95FE;
const DWORD dwCWvsAppDir_upDir = 0x009F9644;
const DWORD dwCWvsAppDir_SlashToBackSlash = 0x009F9621;
const DWORD dwZXStringCharGetBuffer = 0x00414617;
const DWORD dwCConfigGetInstance = 0x00538C98;
const DWORD dwCConfigCheckExecPathReg = 0x0049CCF3;
const DWORD dwCLogoCLogo = 0x0062ECE2;
const DWORD dwset_stage = 0x00777347;

const DWORD dwCInputSystemCInputSystem = 0x009F821F;
const DWORD dwCInputSystemCreateInstance = 0x009F9A6A;
const DWORD dwCInputSystemInit = 0x00599EBF;
const DWORD dwZAllocAnonSelectorAlloc = 0x00403065;

const DWORD dwCActionManInit = 0x00406ABD;
const DWORD dwCAnimationDisplayerCreateInstance = 0x009F9DFC;
const DWORD dwCMapleTVManCreateInstance = 0x009F9F87;

const DWORD dwZRefCStage = 0x00496B68;
const DWORD dwCWndManSUpdate = 0x009E47C3;
const DWORD dwComIssueError = 0x00A5FDE4;
const DWORD dwComIssueErrorEx = 0x00A5FDF2;
const DWORD dwZRefCStageDestructor = 0x00496B85;
const DWORD dwCActionManSweepCache = 0x00411BBB;

const DWORD dwCClientSocketManipulatePacket = 0x0049651D;
const DWORD dwCInputSystemUpdateDevice = 0x0059A2E9;
const DWORD dwCInputSystemGetISMessage = 0x0059A306;
const DWORD dwCWvsAppISMsgProc = 0x009F97B7;
const DWORD dwCPatchExceptionCPatchException = 0x0051E834;
const DWORD dwCInputSystemGenerateAutoKeyDown = 0x0059B2D2;
const DWORD dwIWzGr2DGetnextRenderTime = 0x009F6990;
const DWORD dwCWndManRedrawInvalidatedWindows = 0x009E4547;
const DWORD dwIWzGr2DRenderFrame = 0x00777326;
const DWORD dwA61DF2 = 0x00A61DF2;

__declspec(naked) void FixFullScreen() {
    __asm {
            mov eax, 0
            jmp dword ptr[dwFixFullScreenReturn]
    }
}

__declspec(naked) void NukedCWvsAppSetup() {
    __asm {
            mov eax, dword ptr ds :[0x00AE7DF2]
            call dwEH_prolog
            sub esp, 1012
            push ebx
            push esi
            push edi
            mov[ebp-1008], ecx
            mov ecx,[ebp-1008]
            call dwCWvsAppInitializeAuth
            call dword ptr ds :[0x00BF060C] //timeGetTime
            push eax
            call dwSrand
            pop ecx
            call dwGetSEPrivilege
            jmp label21
            label21:
        // CSecurityClient crap
            xor eax, eax
            cmp dword ptr ds :[0x00BEC3A8], 0
            setnz al
            test eax, eax
            jnz short label20
            call dwCSecurityClientCreateInstance
            label20:
            xor eax, eax
            cmp dword ptr ds :[0x00BEC3A8], 0
            setnz al
            test eax, eax
            jmp short label22
            mov ecx, dword ptr ds :[0x00BEC3A8]
            call dwCSecurityClientInitModule
            label22:
            xor eax, eax
            cmp dword ptr ds :[0x00BEC3A8], 0
            setnz al
            test eax, eax
            jmp short label23
            mov ecx, dword ptr ds :[0x00BEC3A8]
            call dwCSecurityClientStartModule
            label23:
            mov dword ptr ds:[0x00BF1AC8], 16
            mov ecx,[ebp-1008]
            call dwCWvsAppInitializePCOM
            mov ecx,[ebp-1008]
            call dwCWvsAppCreateMainWindow
            call dwCClientSocketCreateInstance
            mov ecx,[ebp-1008]
            call dwCWvsAppConnectLogin
            call dwCFuncKeyMappedManCreateInstance
            call dwCQuickslotKeyMappedManCreateInstance
            call dwCMacroSysManCreateInstance
            mov ecx,[ebp-1008]
            call dwCWvsAppInitializeResMan
            lea eax,[ebp-400]
            push eax
            call dword ptr ds :[0x00BF0448]
            push eax
            call dword ptr ds :[0x00BF0444]
            mov eax, 0x00BE7918
            mov eax,[eax+14320]
            mov[ebp-944], eax
            cmp dword ptr[ebp-944], 0
            jz short label1
            push 32
        //mov ecx, dword ptr ds : [0x00BF0B00]
            mov eax, dword ptr[0x00BF0B00]
            mov ecx, eax
            call dwZAllocAnonSelectorAlloc
            mov[ebp-872], eax
            and dword ptr[ebp-4], 0
            cmp dword ptr[ebp-872], 0
            jz short label2
            sub esp, 16
            lea esi,[ebp-400]
            mov edi, esp
            movsd
            movsd
            movsd
            movsd
            mov ecx,[ebp-872]
            call dword ptr ds :[0x0042C3DE]
            mov[ebp-1012], eax
            jmp short label3
            label2:
            and dword ptr[ebp-1012], 0
            label3:
            mov eax,[ebp-1012]
            mov[ebp-868], eax
            or dword ptr[ebp-4], 4294967295
            label1:
            mov ecx,[ebp-1008]
            call dwCWvsAppInitializeGr2D
            mov ecx,[ebp - 1008]
            call dwCWvsAppInitializeInput
            push 300
            call dword ptr ds :[0x00BF02F4]
            mov ecx,[ebp-1008]
            call dwCWvsAppInitializeSound
            push 300
            call dword ptr ds :[0x00BF02F4]
            mov ecx,[ebp - 1008]
            call dwCWvsAppInitializeGameData
            mov ecx,[ebp - 1008]
            call dwCWvsAppCreateWndManager
            push 0
            push 0
            mov ecx, dword ptr ds :[0x00BEBF9C]
            call dwCConfigApplySysOpt
            call dwCActionManCreateInstance
            mov ecx, eax
            call dwCActionManInit
            call dwCAnimationDisplayerCreateInstance
            call dwCMapleTVManCreateInstance
            mov ecx, eax
            call dwCMapleTVManInit
            call dwCQuestManCreateInstance
            mov ecx, eax
            call dwCQuestManLoadDemand
            test eax, eax
            jnz short label4
            mov dword ptr[ebp-880], 570425350
            lea eax,[ebp-880]
            mov[ebp-1016], eax
            mov dword ptr[ebp-4], 1
            mov eax,[ebp-1016]
            mov eax,[eax]
            mov[ebp-876], eax
            push 11814752
            lea eax,[ebp-876]
            push eax
            call dwCxxThrowException
            label4:
            mov ecx, dword ptr ds :[0x00BED614]
            call dwCQuestManLoadPartyQuestInfo
            mov ecx, dword ptr ds :[0x00BED614]
            call dwCQuestManLoadExclusive
            call dwCMonsterBookManCreateInstance
            mov ecx, eax
            call dwCMonsterBookManLoadBook
            test eax, eax
            jnz short label5
            mov dword ptr[ebp - 888], 570425350
            lea eax,[ebp - 888]
            mov[ebp - 1020], eax
            mov dword ptr[ebp - 4], 2
            mov eax,[ebp - 1020]
            mov eax,[eax]
            mov[ebp - 884], eax
            push 11814752
            lea eax,[ebp - 884]
            push eax
            call dwCxxThrowException
            label5:
            call dwCRadioManagerCreateInstance
            push 260
            lea eax,[ebp-292]
            push eax
            push 0
            call dword ptr ds :[0x00BF028C]
            lea eax,[ebp-292]
            push eax
            call dwCWvsAppDir_BackSlashToSlash
            pop ecx
            lea eax,[ebp-292]
            push eax
            call dwCWvsAppDir_upDir
            pop ecx
            lea eax,[ebp - 292]
            push eax
            call dwCWvsAppDir_SlashToBackSlash
            pop ecx
            push ecx
            mov eax, esp
            mov[ebp-892], esp
            mov[ebp-1004], eax
            mov eax,[ebp-1004]
            and dword ptr[eax], 0
            push 4294967295
            lea eax,[ebp-292]
            push eax
            mov ecx,[ebp-1004]
            call dwZXStringCharGetBuffer
            mov dword ptr[ebp-4], 3
            call dwCConfigGetInstance
            mov ecx, eax
            or dword ptr[ebp-4], 4294967295
            call dwCConfigCheckExecPathReg
            push 56
        //mov ecx, dword ptr ds : [0x00BF0B00]
            mov eax, dword ptr[0x00BF0B00]
            mov ecx, eax
            call dwZAllocAnonSelectorAlloc
            mov[ebp-900], eax
            mov dword ptr[ebp-4], 4
            cmp dword ptr[ebp-900], 0
            jz short label6
            mov ecx,[ebp-900]
            call dwCLogoCLogo
            mov[ebp-1024], eax
            jmp short label7
            label6:
            and dword ptr[ebp-1024], 0
            label7:
            mov eax,[ebp-1024]
            mov[ebp-896], eax
            or dword ptr[ebp-4], 4294967295
            push 0
            push dword ptr[ebp-896]
            call dwset_stage
            pop ecx
            pop ecx
            mov dword ptr[ebp-808], 3708088046
            and dword ptr[ebp-312], 0
            jmp short label8
            label15:
            mov eax,[ebp-312]
            inc eax
            mov[ebp-312], eax
            label8:
            cmp dword ptr[ebp-312], 256
            jge label9
            mov eax,[ebp-312]
            mov[ebp-860], eax
            mov dword ptr[ebp-864], 8
            jmp short label13
            label14:
            mov eax,[ebp-864]
            dec eax
            mov[ebp-864], eax
            label13:
            cmp dword ptr[ebp-864], 0
            jle short label10
            mov eax,[ebp-860]
            and eax, 1
            test eax, eax
            jz short label12
            mov eax,[ebp-860]
            shr eax, 1
            mov ecx,[ebp-808]
            sub ecx, 5421
            xor eax, ecx
            mov[ebp-860], eax
            jmp short label11
            label12:
            mov eax,[ebp-860]
            shr eax, 1
            mov[ebp-860], eax
            label11:
            jmp short label14
            label10:
            mov eax,[ebp-312]
            mov ecx,[ebp-860]
            mov dword ptr ds :[0x00BF167C][eax*4], ecx
            jmp label15
            label9:
            jmp short label16
            label16:
            jmp dword ptr[dwNukedCWvsAppSetupReturn]
    }
}

__declspec(naked) void NukedCWvsAppInitializeInput() {
    __asm {
            mov eax, dword ptr ds :[0x00AE812D]
            call dwEH_prolog
            sub esp, 198h
            push ebx
            push esi
            push edi
            mov[ebp-1A0h], ecx
            jmp short label3
            label3:
            push 9D0h
        //mov ecx, dword ptr ds : [0x00BF0B00]
            mov eax, dword ptr[0x00BF0B00]
            mov ecx, eax
            call dwZAllocAnonSelectorAlloc
            mov[ebp-190h], eax
            and dword ptr[ebp-4], 0
            cmp dword ptr[ebp-190h], 0
            jz label1
            mov ecx,[ebp-190h]
            call dwCInputSystemCInputSystem
            mov[ebp-1A4h], eax
            jmp label2
            label1:
            and dword ptr[ebp-1A4h], 0
            label2:
            mov eax,[ebp- 1A4h]
            mov[ebp-18Ch], eax
            or dword ptr[ebp-4], 0FFFFFFFFh
            mov eax,[ebp-1A0h]
            mov eax,[eax + 4]
            mov[ebp-194h], eax
            mov eax,[ebp-1A0h]
            add eax, 54h
            push eax
            push dword ptr[ebp-194h]
            call dwCInputSystemCreateInstance
            mov ecx, eax
            call dwCInputSystemInit
            jmp label4
            label4:
            jmp dword ptr[dwNukedCWvsAppInitializeInputReturn]
    }
}

__declspec(naked) void NukedCWvsAppRun() {
    __asm {
            mov eax, dword ptr[0x00AE7E2C]
            call dwEH_prolog

            sub esp, 0D3Ch
            push ebx
            push esi
            push edi
            mov[ebp-0D30h], ecx
            and dword ptr[ebp-9Ch], 0

            push 6
            pop ecx
            xor eax, eax
            lea edi,[ebp-98h]
            rep stosd
            and dword ptr[ebp-28h], 0
            xor eax, eax
            lea edi,[ebp-24h]
            stosd
            stosd
            xor eax, eax
            cmp dword ptr ds :[0x00BE7914], 0
            setnz al
            test eax, eax
            jz short label9F5CA3
            mov ecx, dword ptr ds :[0x00BE7914]
            call dwCClientSocketManipulatePacket

            label9F5CA3:
            jmp label9F5FDB

            label9F5FDB:
            and dword ptr[ebp-2Ch], 0

            label9F5FDF:
            push 0FFh
            push 0
            push 0
            mov eax,[ebp-0D30h]
            add eax, 54h
            push eax
            push 3
            call dword ptr ds :[0x00BF04EC]

            mov[ebp-0A8h], eax
            mov eax,[ebp-0A8h]
            mov[ebp-0D34h], eax
            cmp dword ptr[ebp-0D34h], 0
            jb label9F62BB

            cmp dword ptr[ebp-0D34h], 2
            jbe label9F6030

            cmp dword ptr[ebp-0D34h], 3
            jz short label9F6079

            jmp label9F62BB

            label9F6030:
            push dword ptr[ebp-0A8h]
            mov ecx, dword ptr ds :[0x00BEC33C]
            call dwCInputSystemUpdateDevice

            label9F6041 :
            lea eax,[ebp-28h]
            push eax
            mov ecx, dword ptr ds :[0x00BEC33C]
            call dwCInputSystemGetISMessage
            test eax, eax
            jz short label9F6074

            push dword ptr[ebp-20h]
            push dword ptr[ebp-24h]
            push dword ptr[ebp-28h]
            mov ecx,[ebp-0D30h]
            call dwCWvsAppISMsgProc

            mov eax,[ebp+8]
            cmp dword ptr[eax], 0
            jz short label9F6072
            jmp label9F6074

            label9F6072 :
            jmp short label9F6041

            label9F6074 :
            jmp label9F694D

            label9F6079:
            push 1
            push 0
            push 0
            push 0
            lea eax,[ebp-9Ch]
            push eax
            call dword ptr ds :[0x00BF04E8]
            test eax, eax
            jz label9F62B6
            lea eax,[ebp-9Ch]
            push eax
            call dword ptr ds :[0x00BF0430]
            lea eax,[ebp-9Ch]
            push eax
            call dword ptr ds :[0x00BF042C]
            mov eax,[ebp-0D30h]
            cmp dword ptr[eax+38h], 0
            jnz short label9F60C5
            and dword ptr[ebp-0D04h], 0
            jmp short label9F60F2

            label9F60C5:
            mov eax,[ebp-0D30h]
            mov eax,[eax+38h]
            mov[ebp-0ACh], eax
            mov eax,[ebp-0D30h]
            and dword ptr[eax+38h], 0
            mov eax,[ebp-0D30h]
            and dword ptr[eax+34h], 0
            mov dword ptr[ebp-0D04h], 1

            label9F60F2:
            cmp dword ptr[ebp-0D04h], 0
            jz short label9F6108
            push 0
            push dword ptr[ebp-0ACh]
            call dwComIssueError

            label9F6108:
            mov eax,[ebp-0D30h]
            cmp dword ptr[eax+34h], 0
            jnz short label9F611D
            and dword ptr[ebp-0D08h], 0
            jmp short label9F614A

            label9F611D:
            mov eax,[ebp-0D30h]
            mov eax,[eax+34h]
            mov[ebp-0ACh], eax
            mov eax,[ebp-0D30h]
            and dword ptr[eax+38h], 0
            mov eax,[ebp-0D30h]
            and dword ptr[eax+34h], 0
            mov dword ptr[ebp-0D08h], 1

            label9F614A:
            cmp dword ptr[ebp-0D08h], 0
            jz label9F629E

            cmp dword ptr[ebp-0ACh], 0x20000000

            jnz short label9F61B1

            mov eax,[ebp-0D30h]
            push dword ptr[eax+40h]
            lea ecx,[ebp-0CE8h]
            call dwCPatchExceptionCPatchException

            mov[ebp-0D38h], eax
            mov eax,[ebp-0D38h]
            mov[ebp-0D3Ch], eax

            and dword ptr[ebp-4], 0
            mov esi,[ebp-0D3Ch]
            mov ecx, 142h
            lea edi,[ebp-7E0h]
            rep movsd
            push 0B52FC8h
            lea eax,[ebp-7E0h]
            push eax
            call dwCxxThrowException

            label9F61B1:
            cmp dword ptr[ebp-0ACh], 0x21000000

            jl short label9F6213

            cmp dword ptr[ebp-0ACh], 0x21000006

            jg short label9F6213

            mov eax,[ebp-0ACh]
            mov[ebp-0D0Ch], eax
            mov eax,[ebp-0D0Ch]
            mov[ebp-0CF0h], eax
            lea eax,[ebp-0CF0h]
            mov[ebp-0D40h], eax
            mov dword ptr[ebp-4], 1
            mov eax,[ebp-0D40h]
            mov eax,[eax]
            mov[ebp-0CECh], eax
            push 11831384 //AVCDisconnectException
            lea eax,[ebp-0CECh]
            push eax
            call dwCxxThrowException

            label9F6213:
            cmp dword ptr[ebp-0ACh], 0x22000000
            jl short label9F6275

            cmp dword ptr[ebp-0ACh], 0x2200000D

            jg short label9F6275

            mov eax,[ebp - 0ACh]
            mov[ebp - 0D10h], eax
            mov eax,[ebp - 0D10h]
            mov[ebp - 0CF8h], eax
            lea eax,[ebp - 0CF8h]
            mov[ebp - 0D44h], eax
            mov dword ptr[ebp - 4], 2
            mov eax,[ebp - 0D44h]
            mov eax,[eax]
            mov[ebp - 0CF4h], eax
            push 0B44760h
            lea eax,[ebp - 0CF4h]
            push eax
            call dwCxxThrowException

            label9F6275:
            mov eax,[ebp - 0ACh]
            mov[ebp - 0D00h], eax
            mov eax,[ebp - 0D00h]
            mov[ebp - 0CFCh], eax
        //push offset AVCDisconnectException
            push    0B44EE0h
            lea eax,[ebp - 0CFCh]
            push eax
            call dwCxxThrowException

            label9F629E:
            mov eax,[ebp+8]
            cmp dword ptr[eax], 0
            jnz short label9F62AF
            cmp dword ptr[ebp-98h], 12h
            jnz short label9F62B1

            label9F62AF:
            jmp short label9F62B6

            label9F62B1:
            jmp label9F6079

            label9F62B6:
            jmp label9F694D

            label9F62BB:
            lea eax,[ebp-28h]
            push eax
            mov ecx, dword ptr ds :[0x00BEC33C]
            call dwCInputSystemGenerateAutoKeyDown
            test eax, eax
            jz short label9F62E2

            push dword ptr[ebp-20h]
            push dword ptr[ebp-24h]
            push dword ptr[ebp-28h]
            mov ecx,[ebp-0D30h]
            call dwCWvsAppISMsgProc

            label9F62E2:
            mov ecx, dword ptr ds :[0x00BEC3A8]
            nop
            nop
            nop
            nop
            nop
            xor eax, eax
            test eax, eax
            jz short label9F62FD
            push 80004003h
            call dwComIssueError

            label9F62FD:
            xor eax, eax
            cmp dword ptr ds :[0x00BF14EC], 0
            setz al
            movzx eax, al
            neg eax
            sbb eax, eax
            inc eax
            movzx eax, al
            test eax, eax
            jz label9F6945
            jmp short label9F632E

            label9F632E:
            cmp dword ptr ds :[0x00BF14EC], 0
            jnz short label9F6341
            push 80004003h
            call dwComIssueError

            label9F6341:
            mov ecx, dword ptr ds :[0x00BF14EC]
            call dwIWzGr2DGetnextRenderTime
            mov[ebp-0B0h], eax
            push dword ptr[ebp-0B0h]
            mov ecx,[ebp-0D30h]
            call dwCWvsAppCallUpdate
            call dwCWndManRedrawInvalidatedWindows
            cmp dword ptr ds :[0x00BF14EC], 0
            jnz short label9F637B
            push 80004003h
            call dwComIssueError

            label9F637B:
            mov ecx, dword ptr ds :[0x00BF14EC]
            call dwIWzGr2DRenderFrame
            jmp label9F6945

            label9F6945:
            push 1
            call dword ptr ds :[0x00BF02F4]

            label9F694D:
            mov eax,[ebp+8]
            cmp dword ptr[eax], 0
            jnz short label9F695E

            cmp dword ptr[ebp-98h], 12h
            jnz short label9F6960

            label9F695E:
            jmp short label9F6965

            label9F6960:
            jmp label9F5FDF

            label9F6965:
            push dword ptr[ebp-34h]
            call dwA61DF2
            pop ecx
            cmp dword ptr[ebp-98h], 12h
            jnz short label9F697
            push 0
            call dword ptr ds :[0x00BF041C]

            label9F697:
            jmp dword ptr[dwNukedCWvsAppRunReturn]
    }
}

// void __thiscall CClientSocket::Connect(CClientSocket *this, const sockaddr_in *pAddr)
typedef VOID(__fastcall *_CClientSocket__Connect_addr_t)(CClientSocket *pThis, PVOID edx, const sockaddr_in *pAddr);

_CClientSocket__Connect_addr_t _CClient__Connect_addr;

// void __thiscall CClientSocket::Connect(CClientSocket *this, const CClientSocket::CONNECTCONTEXT *ctx)
typedef VOID(__fastcall *_CClientSocket__Connect_ctx_t)(CClientSocket *pThis, PVOID edx,
                                                        CClientSocket::CONNECTCONTEXT *ctx);

_CClientSocket__Connect_ctx_t _CClient__Connect_ctx;

// void __thiscall CClientSocket::ClearSendReceiveCtx(CClientSocket *this)
typedef VOID(__fastcall *_CClientSocket__ClearSendReceiveCtx_t)(CClientSocket *pThis, PVOID edx);

_CClientSocket__ClearSendReceiveCtx_t _CClientSocket__ClearSendReceiveCtx = reinterpret_cast<_CClientSocket__ClearSendReceiveCtx_t>(0x004969EE);

// void __thiscall ZSocketBase::CloseSocket(ZSocketBase *this)
typedef VOID(__fastcall *_ZSocketBase__CloseSocket_t)(ZSocketBase *pThis, PVOID edx);

_ZSocketBase__CloseSocket_t _ZSocketBase__CloseSocket = reinterpret_cast<_ZSocketBase__CloseSocket_t>(0x00494857);

// int __thiscall CClientSocket::OnConnect(CClientSocket *this, int bSuccess)
typedef INT(__fastcall *_CClientSocket__OnConnect_t)(CClientSocket *pThis, PVOID edx, INT bSuccess);

_CClientSocket__OnConnect_t _CClientSocket__OnConnect = reinterpret_cast<_CClientSocket__OnConnect_t>(0x00494ED1);

VOID __fastcall CClient__Connect_Addr_Hook(CClientSocket *pThis, PVOID edx, const sockaddr_in *pAddr) {
    Log("CClientSocket::Connect(CClientSocket *this, const sockaddr_in *pAddr)");
    _CClientSocket__ClearSendReceiveCtx(pThis, edx);
    _ZSocketBase__CloseSocket(&(pThis->m_sock), edx);

    pThis->m_sock._m_hSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (pThis->m_sock._m_hSocket == -1) {
        Log("CClientSocket::Connect -> Should throw an exception here.");
        return;
    }
    pThis->m_tTimeout = timeGetTime() + 5000;

    if (WSAAsyncSelect(pThis->m_sock._m_hSocket, pThis->m_hWnd, WM_USER + 1, 0x33) == -1 ||
        connect(pThis->m_sock._m_hSocket, reinterpret_cast<const sockaddr *>(pAddr), sizeof(*pAddr)) != -1 ||
        WSAGetLastError() != WSAEWOULDBLOCK) {
        _CClientSocket__OnConnect(pThis, edx, 0);
    }
}

VOID __fastcall CClient__Connect_Ctx_Hook(CClientSocket *pThis, PVOID edx, CClientSocket::CONNECTCONTEXT *ctx) {
    Log("CClientSocket::Connect(CClientSocket *this, const CClientSocket::CONNECTCONTEXT *ctx)");
    pThis->m_ctxConnect.lAddr.RemoveAll();
    pThis->m_ctxConnect.lAddr.AddTail(&ctx->lAddr);
    pThis->m_ctxConnect.posList = ctx->posList;
    pThis->m_ctxConnect.bLogin = ctx->bLogin;
    pThis->m_ctxConnect.posList = reinterpret_cast<__POSITION *>(pThis->m_ctxConnect.lAddr.GetHeadPosition());
    pThis->m_addr = *pThis->m_ctxConnect.lAddr.GetHeadPosition();
    CClient__Connect_Addr_Hook(pThis, edx, &pThis->m_addr);
}

// int __thiscall CLogin::SendCheckPasswordPacket(CLogin *this, char *sID, char *sPasswd)
typedef INT(__fastcall *_CLogin__SendCheckPasswordPacket_t)(CLogin *pThis, PVOID edx, char *sID, char *sPasswd);

_CLogin__SendCheckPasswordPacket_t _CLogin__SendCheckPasswordPacket;

ZXString<char> *GetCUITitleInstance() {
    return reinterpret_cast<ZXString<char> *>(*(void **) 0x00BEDA60);
}

INT __fastcall CLogin__SendCheckPasswordPacket_Hook(CLogin *pThis, PVOID edx, char *sID, char *sPasswd) {
    if (pThis->m_bRequestSent) {
        return 0;
    }
    pThis->m_bRequestSent = 1;
    pThis->m_WorldItem.RemoveAll();
    pThis->m_aBalloon.RemoveAll();

    auto systemInfo = CSystemInfo();
    systemInfo.Init();
    auto cOutPacket = COutPacket(1);

    ZXString<char> tempString = ZXString<char>(sID, 0xFFFFFFFF);
    cOutPacket.EncodeStr(tempString);

    ZXString<char> tempString2 = ZXString<char>(sPasswd, 0xFFFFFFFF);
    cOutPacket.EncodeStr(tempString2);

    cOutPacket.EncodeBuffer(systemInfo.GetMachineId(), 16);
    int gameRoomClient = systemInfo.GetGameRoomClient();
    Log("GRC %d, GRC PTR %d", gameRoomClient, &gameRoomClient);
    cOutPacket.Encode4(gameRoomClient);
    cOutPacket.Encode1(CWvsApp::GetInstance()->m_nGameStartMode);
    cOutPacket.Encode1(0);
    cOutPacket.Encode1(0);
    cOutPacket.Encode4(CConfig::GetInstance()->GetPartnerCode());
    CClientSocket::GetInstance()->SendPacket(&cOutPacket);
    //((ZXString<char>) reinterpret_cast<ZXString<char> *>(CWvsContext::GetInstance()->m_bFirstUserLoad)) = sID;
//    //something CUITitle
//
    cOutPacket.m_aSendBuff.RemoveAll();
    return 1;
}

// void __thiscall CWvsApp::CallUpdate(CWvsApp *this, int tCurTime)
typedef VOID(__fastcall *_CWvsApp__CallUpdate_t)(CWvsApp *pThis, PVOID edx, int tCurTime);

_CWvsApp__CallUpdate_t _CWvsApp__CallUpdate;

CStage *get_stage() {
    return reinterpret_cast<CStage *>(*(void **) 0x00BEDED4);
}

IWzGr2D* get_gr() {
    return reinterpret_cast<IWzGr2D *>(*(uint32_t **) 0x00BF14EC);
}

VOID __fastcall CWvsApp__CallUpdate_Hook(CWvsApp *pThis, PVOID edx, int tCurTime) {
    if (pThis->m_bFirstUpdate) {
        pThis->m_tUpdateTime = tCurTime;
        pThis->m_tLastServerIPCheck = tCurTime;
        pThis->m_tLastServerIPCheck2 = tCurTime;
        pThis->m_tLastGGHookingAPICheck = tCurTime;
        pThis->m_tLastSecurityCheck = tCurTime;
        pThis->m_bFirstUpdate = 0;
    }

    while (tCurTime - pThis->m_tUpdateTime > 0) {
        CStage* stage = get_stage();
        if (stage) {
            stage->Update();
        }

        CWndMan::s_Update();
        pThis->m_tUpdateTime += 30;
        if (tCurTime - pThis->m_tUpdateTime > 0) {
            auto gr = get_gr();
            auto hr = gr->UpdateCurrentTime(pThis->m_tUpdateTime);
            if (FAILED(hr)) {
                Log("Some sort of com error");
                return;
            }
        }
    }
    auto gr = get_gr();
    auto hr = gr->UpdateCurrentTime(tCurTime);
    if (FAILED(hr)) {
        Log("Some sort of com error");
        return;
    }
    CActionMan::GetInstance()->SweepCache();
}

// main thread
VOID __stdcall MainProc() {
    // Window Mode Magic
    MemEdit::CodeCave(FixFullScreen, dwFixFullScreen, 5);

    // Noop Call to CSecurityClient::OnPacket
    MemEdit::PatchNop(dwCSecurityClientOnPacketCall, 12);

    // Nuke CWvsApp::Setup
    MemEdit::CodeCave(NukedCWvsAppSetup, dwCWvsAppSetUp, 5);
    MemEdit::CodeCave(NukedCWvsAppInitializeInput, dwCWvsAppInitializeInput, 5);
    MemEdit::CodeCave(NukedCWvsAppRun, dwCWvsAppRun, 5);

    INITMAPLEHOOK(_CClient__Connect_ctx, _CClientSocket__Connect_ctx_t, CClient__Connect_Ctx_Hook, 0x00494CA3);
    INITMAPLEHOOK(_CClient__Connect_addr, _CClientSocket__Connect_addr_t, CClient__Connect_Addr_Hook, 0x00494D2F);
    INITMAPLEHOOK(_CLogin__SendCheckPasswordPacket, _CLogin__SendCheckPasswordPacket_t,
                  CLogin__SendCheckPasswordPacket_Hook, 0x005F6952);
    INITMAPLEHOOK(_CWvsApp__CallUpdate, _CWvsApp__CallUpdate_t, CWvsApp__CallUpdate_Hook, 0x009F84D0);
}

// dll entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            DisableThreadLibraryCalls(hModule);
            CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE) &MainProc, nullptr, 0, nullptr);
            break;
        }
    }
    return TRUE;
}