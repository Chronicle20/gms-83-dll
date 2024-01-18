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

const DWORD dwCWvsAppInitializeGr2D = 0x009F7A3B;
const DWORD dwFixFullScreen = dwCWvsAppInitializeGr2D + 0x60; // 0x009F7A9B
const DWORD dwFixFullScreenReturn = dwCWvsAppInitializeGr2D + 0x65;

__declspec(naked) void FixFullScreen() {
    __asm {
            mov eax, 0
            jmp dword ptr[dwFixFullScreenReturn]
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

// void __cdecl set_stage(CStage *pStage, void *pParam)
typedef VOID(__cdecl *_set_stage_t)(CStage *pStage, void *pParam);
_set_stage_t _set_stage = reinterpret_cast<_set_stage_t>(0x00777347);

IWzGr2D *get_gr() {
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
        CStage *stage = get_stage();
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

// void __stdcall TSingleton<CInputSystem>::CreateInstance()
typedef VOID(__stdcall *_TSingleton_CInputSystem__CreateInstance_t)();

_TSingleton_CInputSystem__CreateInstance_t _TSingleton_CInputSystem__CreateInstance = reinterpret_cast<_TSingleton_CInputSystem__CreateInstance_t>(0x009F9A6A);

// void __thiscall CWvsApp::InitializeInput(CWvsApp *this)
typedef VOID(__fastcall *_CWvsApp__InitializeInput_t)(CWvsApp *pThis, PVOID edx);

_CWvsApp__InitializeInput_t _CWvsApp__InitializeInput;

VOID __fastcall CWvsApp__InitializeInput_Hook(CWvsApp *pThis, PVOID edx) {
    _TSingleton_CInputSystem__CreateInstance();
    CInputSystem::GetInstance()->Init(pThis->m_hWnd, pThis->m_ahInput);
}

// void __thiscall CWvsApp::Run(CWvsApp *this, int *pbTerminate)
typedef VOID(__stdcall *_CWvsApp__Run_t)(CWvsApp *pThis, int *pbTerminate);

_CWvsApp__Run_t _CWvsApp__Run;

VOID __fastcall CWvsApp__Run_Hook(CWvsApp *pThis, PVOID edx, int *pbTerminate) {
    tagMSG msg{};
    ISMSG isMsg{};
    memset(&msg, 0, sizeof(msg));
    memset(&isMsg, 0, sizeof(isMsg));
    if (CClientSocket::GetInstance()) {
        CClientSocket::GetInstance()->ManipulatePacket();
    }
    do {
        auto dwRet = MsgWaitForMultipleObjects(3u, pThis->m_ahInput, 0, 0, 0xFFu);
        if (dwRet <= 2) {
            CInputSystem::GetInstance()->UpdateDevice(dwRet);
            do {
                if (!CInputSystem::GetInstance()->GetISMessage(&isMsg)) {
                    break;
                }
                pThis->ISMsgProc(isMsg.message, isMsg.wParam, isMsg.lParam);
            } while (!*pbTerminate);
        } else if (dwRet == 3) {
            do {
                if ( !PeekMessageA(&msg, 0, 0, 0, 1u) ) {
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
                int isError = 0;
                int m_hrComErrorCode = 0;
                if (pThis->m_hrComErrorCode) {
                    m_hrComErrorCode = pThis->m_hrComErrorCode;
                    pThis->m_hrComErrorCode = 0;
                    pThis->m_hrZExceptionCode = 0;
                    isError = 1;
                }
                if (isError) {
                    Log("Do proper _com_raise_error");
                    return;
                }
                if (pThis->m_hrZExceptionCode) {
                    m_hrComErrorCode = pThis->m_hrZExceptionCode;
                    pThis->m_hrComErrorCode = 0;
                    pThis->m_hrZExceptionCode = 0;
                    isError = 1;
                }
                if (isError) {
                    Log("Do proper _com_raise_error");
                    return;
                }
            } while ( !*pbTerminate && msg.message != 18 );
        } else {
            if (CInputSystem::GetInstance()->GenerateAutoKeyDown(&isMsg)) {
                pThis->ISMsgProc(isMsg.message, isMsg.wParam, isMsg.lParam);
            }
            auto tCurTime = 0;
            auto hr = get_gr()->GetnextRenderTime(&tCurTime);
            if (FAILED(hr)) {
                Log("Do proper _com_raise_errorex");
                return;
            }
            CWvsApp__CallUpdate_Hook(pThis, edx, tCurTime);
            CWndMan::RedrawInvalidatedWindows();
            hr = get_gr()->RenderFrame();
            if (FAILED(hr)) {
                Log("Do proper _com_raise_errorex");
                return;
            }
            Sleep(1u);
        }
    } while (!*pbTerminate && msg.message != 18);
    if (msg.message == 18) {
        PostQuitMessage(0);
    }
}

void GetSEPrivilege() {
    ((VOID **(_fastcall * )())
    0x0044E824)();
}

// void __thiscall CWvsApp::SetUp(CWvsApp *this)
typedef VOID(__stdcall *_CWvsApp__SetUp_t)(CWvsApp *pThis);
_CWvsApp__SetUp_t _CWvsApp__SetUp;

void newCLogo(CLogo * logo) {
    ((VOID(_fastcall * )(CLogo * , PVOID))
    0x0062ECE2)(logo, NULL);
}

VOID __fastcall CWvsApp__SetUp_Hook(CWvsApp *pThis) {
    pThis->InitializeAuth();
    auto time = timeGetTime();
    srand(time);
    GetSEPrivilege();
    CSecurityClient::CreateInstance();
    // dword_BF1AC8 = 0x10;
    pThis->InitializePCOM();
    pThis->CreateMainWindow();
    CClientSocket::CreateInstance();
    pThis->ConnectLogin();
    CFuncKeyMappedMan::CreateInstance();
    CQuickslotKeyMappedMan::CreateInstance();
    CMacroSysMan::CreateInstance();
    pThis->InitializeResMan();

//    dword_BF0444(v2);
//    if ( *((_DWORD *)dword_BE7918 + 3580) )
//    {
//        v23 = ZAllocEx<ZAllocAnonSelector>::Alloc(dword_BF0B00, 0x20u);
//        v31 = 0;
//        if ( v23 )
//            v12 = sub_42C3DE(v23, v3, v28[0], v28[1], v28[2], v28[3]);
//        else
//            v12 = 0;
//        v24 = v12;
//        v31 = -1;
//    }

    pThis->InitializeGr2D();
    pThis->InitializeInput();
    Sleep(300);
    pThis->InitializeSound();
    Sleep(300);
    pThis->InitializeGameData();
    pThis->CreateWndManager();
    CConfig::GetInstance()->ApplySysOpt(0, 0);
    CActionMan::CreateInstance();
    CActionMan::GetInstance()->Init();
    CAnimationDisplayer::CreateInstance();
    CMapleTVMan::CreateInstance();
    CMapleTVMan::GetInstance()->Init();
    CQuestMan::CreateInstance();
    if (!CQuestMan::GetInstance()->LoadDemand()) {
        Log("Throw error regarding CQuestMan::LoadDemand");
        return;
    }
    CQuestMan::GetInstance()->LoadPartyQuestInfo();
    CQuestMan::GetInstance()->LoadExclusive();
    CMonsterBookMan::CreateInstance();
    if (!CMonsterBookMan::GetInstance()->LoadBook()) {
        Log("Throw error regarding CMonsterBookMan::LoadBook");
        return;
    }
    CRadioManager::CreateInstance();
    char sModulePath[260];
    GetModuleFileNameA(0, sModulePath, 260);
    pThis->Dir_BackSlashToSlash(sModulePath);
    pThis->Dir_upDir(sModulePath);
    pThis->Dir_SlashToBackSlash(sModulePath);

    ZXString<char> tempString = ZXString<char>(sModulePath, 0xFFFFFFFF);
    CConfig::GetInstance()->CheckExecPathReg(tempString);

    PVOID ret = ZAllocEx<ZAllocAnonSelector>::GetInstance()->Alloc(56u);
    CStage * cLogo;
    if (ret) {
        cLogo = new (ret) CLogo();
    } else {
        cLogo = nullptr;
    }
    _set_stage(cLogo, nullptr);
//    poly = -586093038;
//    for ( ii = 0; ii < 256; ++ii )
//    {
//        crc32 = ii;
//        for ( i = 8; i > 0; --i )
//        {
//            if ( (crc32 & 1) != 0 )
//                crc32 = (poly - 401) ^ (crc32 >> 1);
//            else
//                crc32 >>= 1;
//        }
//        g_crc32Table[ii] = crc32;
}

// main thread
VOID __stdcall MainProc() {
    // Window Mode Magic
    MemEdit::CodeCave(FixFullScreen, dwFixFullScreen, 5);

    // Noop Call to CSecurityClient::OnPacket
    MemEdit::PatchNop(dwCSecurityClientOnPacketCall, 12);

    INITMAPLEHOOK(_CClient__Connect_ctx, _CClientSocket__Connect_ctx_t, CClient__Connect_Ctx_Hook, 0x00494CA3);
    INITMAPLEHOOK(_CClient__Connect_addr, _CClientSocket__Connect_addr_t, CClient__Connect_Addr_Hook, 0x00494D2F);
    INITMAPLEHOOK(_CLogin__SendCheckPasswordPacket, _CLogin__SendCheckPasswordPacket_t,
                  CLogin__SendCheckPasswordPacket_Hook, 0x005F6952);
    INITMAPLEHOOK(_CWvsApp__CallUpdate, _CWvsApp__CallUpdate_t, CWvsApp__CallUpdate_Hook, 0x009F84D0);
    INITMAPLEHOOK(_CWvsApp__InitializeInput, _CWvsApp__InitializeInput_t, CWvsApp__InitializeInput_Hook, 0x009F7CE1);
    INITMAPLEHOOK(_CWvsApp__Run, _CWvsApp__Run_t, CWvsApp__Run_Hook, 0x009F5C50);
    INITMAPLEHOOK(_CWvsApp__SetUp, _CWvsApp__SetUp_t, CWvsApp__SetUp_Hook, 0x009F5239);
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