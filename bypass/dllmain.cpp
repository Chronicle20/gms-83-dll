/*
 This file is part of GMS-83-DLL.

 GMS-83-DLL is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

 GMS-83-DLL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
 */
#include "pch.h"

#include <memedit.h>
#include <timeapi.h>
#include <WS2tcpip.h>
#include "logger.h"
#include "hooker.h"

const DWORD dwCClientSocketProcessPacket = 0x004A8622;
const DWORD dwCSecurityClientOnPacketCall = dwCClientSocketProcessPacket + 0x93;

// void __thiscall CClientSocket::Connect(CClientSocket *this, const sockaddr_in *pAddr)
typedef VOID(__fastcall *_CClientSocket__Connect_addr_t)(CClientSocket *pThis, PVOID edx, const sockaddr_in *pAddr);

_CClientSocket__Connect_addr_t _CClientSocket__Connect_addr;

// void __thiscall CClientSocket::Connect(CClientSocket *this, const CClientSocket::CONNECTCONTEXT *ctx)
typedef VOID(__fastcall *_CClientSocket__Connect_ctx_t)(CClientSocket *pThis, PVOID edx,
                                                        CClientSocket::CONNECTCONTEXT *ctx);

_CClientSocket__Connect_ctx_t _CClientSocket__Connect_ctx;

// int __thiscall CClientSocket::OnConnect(CClientSocket *this, int bSuccess)
typedef INT(__fastcall *_CClientSocket__OnConnect_t)(CClientSocket *pThis, PVOID edx, INT bSuccess);

_CClientSocket__OnConnect_t _CClientSocket__OnConnect;

VOID __fastcall CClientSocket__Connect_Addr_Hook(CClientSocket *pThis, PVOID edx, const sockaddr_in *pAddr);

INT __fastcall CClientSocket__OnConnect_Hook(CClientSocket *pThis, PVOID edx, int bSuccess) {
    Log("CClientSocket::OnConnect(CClientSocket *this, int bSuccess)");
    if (!pThis->m_ctxConnect.lAddr.GetCount()) {
        Log("CClientSocket::GetCount() 0");
        return 0;
    }
    if (!bSuccess) {
        if (!pThis->m_ctxConnect.posList) {
            pThis->Close();
            if (pThis->m_ctxConnect.bLogin) {
                // some login specific exception.
                Log("CClientSocket::OnConnect 570425345");
                return 0;
            }
            // some other specific exception.
            Log("CClientSocket::OnConnect 553648129");
            return 0;
        }
        Log("CClientSocket::OnConnect CClientSocket__Connect_Addr_Hook");
        //TODO do i really care to do the loadbalancing logic?
        CClientSocket__Connect_Addr_Hook(pThis, edx, pThis->m_ctxConnect.lAddr.GetHeadPosition());
        return 0;
    }

    Log("CClientSocket::OnConnect ZSocketBuffer::Alloc");
    int BUFFER_SIZE = 1460;
    ZSocketBuffer *buf = ZSocketBuffer::Alloc(BUFFER_SIZE);
    ZRef<ZSocketBuffer> pBuff = ZRef<ZSocketBuffer>();
    pBuff.p = buf;
    if (buf->m_nRef) {
        InterlockedIncrement(&buf->m_nRef);
    }
    char *buffer = buf->buf;
    char *accumulatedBuf = buffer;
    int bLenRead = 0;
    Log("CClientSocket::OnConnect Start Recv Loop");
    int src = 0;
    int something = 40;
    int bytesReceived;
    while (true) {
        do {
            while (true) {
                while (true) {
                    int lenToRead = 0;
                    if (bLenRead) {
                        lenToRead = src;
                    } else {
                        lenToRead = 2;
                    }
                    bytesReceived = recv(pThis->m_sock._m_hSocket, accumulatedBuf, lenToRead, 0);
                    if (bytesReceived != -1) {
                        Log("CClientSocket::OnConnect breaking 1");
                        break;
                    }
                    int wsaLastError = WSAGetLastError();
                    Log("CClientSocket::OnConnect wsaLastError=[%d]", wsaLastError);
                    if (wsaLastError == WSAEWOULDBLOCK) {
                        Sleep(500);
                        if (--something >= 0) {
                            continue;
                        }
                    }
                    bytesReceived = 0;
                    Log("CClientSocket::OnConnect breaking 2");
                    break;
                }
                accumulatedBuf += bytesReceived;
                Log("CClientSocket::OnConnect bytesReceived=[%d] totalBytesReceived=[%d]", bytesReceived,
                    accumulatedBuf - buffer);
                if (!bytesReceived) {
                    Log("CClientSocket::OnConnect dipping 1");
                    CClientSocket__OnConnect_Hook(pThis, edx, 0);
                    return 0;
                }
                if (!bLenRead) {
                    break;
                }
                if (accumulatedBuf - buffer == src) {
                    goto label_26;
                }
            }
        } while (accumulatedBuf - buffer != 2);
        src = *buffer;
        if (src > buf->len) {
            break;
        }
        bLenRead = 1;
        accumulatedBuf = buffer;
    }
    bytesReceived = 0;
    label_26:
    if (!bytesReceived) {
        Log("CClientSocket::OnConnect dipping 2");
        CClientSocket__OnConnect_Hook(pThis, edx, 0);
        return 0;
    }

    Log("CClientSocket::OnConnect Recv Decoding");
    unsigned short majorVersion;
    char *result = &buffer[CIOBufferManipulator::Decode2(&majorVersion, buffer, accumulatedBuf - buffer)];
    Log("CClientSocket::OnConnect majorVersion=[%d]", majorVersion);
    ZXString<char> minorVersion = ZXString<char>();
    result = &result[CIOBufferManipulator::DecodeStr(&minorVersion, result, accumulatedBuf - result)];
    Log("CClientSocket::OnConnect minorVersion=[%s]", minorVersion.m_pStr);
    int version = atoi(minorVersion.m_pStr);
    minorVersion.Empty();
    Log("CClientSocket::OnConnect version=[%d]", version);
    unsigned int uSeqSnd;
    result = &result[CIOBufferManipulator::Decode4(&uSeqSnd, result, accumulatedBuf - result)];
    unsigned int uSeqRcv;
    result = &result[CIOBufferManipulator::Decode4(&uSeqRcv, result, accumulatedBuf - result)];
    unsigned char nVersionHeader;
    result = &result[CIOBufferManipulator::Decode1(&nVersionHeader, result, accumulatedBuf - result)];
    if (result < accumulatedBuf) {
        // throw an exception.
        return 0;
    }
    Log("CClientSocket::OnConnect nVersionHeader=[%d]", nVersionHeader);
    Log("CClientSocket::OnConnect m_uSeqSnd=[%d] m_uSeqRcv=[%d]", uSeqSnd, uSeqRcv);
    pThis->m_uSeqSnd = uSeqSnd;
    pThis->m_uSeqRcv = uSeqRcv;

    int nGameStartMode = CWvsApp::GetInstance()->m_nGameStartMode;
    Log("CClientSocket::OnConnect m_nGameStartMode=[%d]", nGameStartMode);
    if (nGameStartMode != 1) {
        if (nGameStartMode == 2) {
            nGameStartMode = 0;
        } else {
            Log("CClientSocket::OnConnect dipping 3");
            return 0;
        }
    }
    if (nVersionHeader != 3) {
        throw std::invalid_argument("570425351");
    }
    if (majorVersion > 185) {
        throw std::invalid_argument("CPatchException");
    }
    if (majorVersion != 185) {
        throw std::invalid_argument("570425351");
    }
    if (version > 1) {
        throw std::invalid_argument("CPatchException");
    }
    if (!version) {
        throw std::invalid_argument("570425351");
    }
    pThis->ClearSendReceiveCtx();
    pThis->m_ctxConnect.lAddr.RemoveAll();
    pThis->m_ctxConnect.posList = 0;
    socklen_t peerAddrLen = sizeof(pThis->m_addr);
    if (getpeername(pThis->m_sock._m_hSocket, reinterpret_cast<struct sockaddr*>(&pThis->m_addr), &peerAddrLen) == -1) {
        int lastError = WSAGetLastError();
        throw std::invalid_argument("570425351");
    }

    //TODO check version stuff
    if (pThis->m_ctxConnect.bLogin) {
        Log("CClientSocket::OnConnect should be sending 0xF");
        char * fileName = CWvsApp::GetExceptionFileName();

    } else {
        Log("CClientSocket::OnConnect accountId=[%d], worldId=[%d], channelId=[%d], characterId=[%d]",CWvsContext::GetInstance()->m_dwAccountId, CWvsContext::GetInstance()->m_nWorldID, CWvsContext::GetInstance()->m_nChannelID, CWvsContext::GetInstance()->m_dwCharacterId);
        auto systemInfo = CSystemInfo();
        systemInfo.Init();
        auto cOutPacket = COutPacket(7);
        cOutPacket.Encode4(CWvsContext::GetInstance()->m_dwCharacterId);
        cOutPacket.EncodeBuffer(systemInfo.GetMachineId(), 16);
        if (CWvsContext::GetInstance()->m_nSubGradeCode.GetData() >= 0) {
            cOutPacket.Encode1(0);
        } else {
            cOutPacket.Encode1(1);
        }
        cOutPacket.Encode1(0);
        cOutPacket.EncodeBuffer(CWvsContext::GetInstance()->m_aClientKey, 8);

        CClientSocket::GetInstance()->SendPacket(&cOutPacket);
        cOutPacket.m_aSendBuff.RemoveAll();
    }

    //_ZRef_ZSocketBuffer__Destructor(&pBuff, edx, 0);
    return 1;
}


VOID __fastcall CClientSocket__Connect_Addr_Hook(CClientSocket *pThis, PVOID edx, const sockaddr_in *pAddr) {
    Log("CClientSocket::Connect(CClientSocket *this, const sockaddr_in *pAddr)");
    pThis->ClearSendReceiveCtx();
    pThis->m_sock.CloseSocket();

    pThis->m_sock._m_hSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (pThis->m_sock._m_hSocket == -1) {
        Log("CClientSocket::Connect -> Should throw an exception here.");
        return;
    }
    pThis->m_tTimeout = timeGetTime() + 5000;

    if (WSAAsyncSelect(pThis->m_sock._m_hSocket, pThis->m_hWnd, WM_USER + 1, 0x33) == -1 ||
        connect(pThis->m_sock._m_hSocket, reinterpret_cast<const sockaddr *>(pAddr), sizeof(*pAddr)) != -1 ||
        WSAGetLastError() != WSAEWOULDBLOCK) {
        int ret = WSAGetLastError();
        Log("CClientSocket::Connect -> WSAGetLastError=[%d]", ret);
        CClientSocket__OnConnect_Hook(pThis, edx, 0);
    }
}

VOID __fastcall CClientSocket__Connect_Ctx_Hook(CClientSocket *pThis, PVOID edx, CClientSocket::CONNECTCONTEXT *ctx) {
    Log("CClientSocket::Connect(CClientSocket *this, const CClientSocket::CONNECTCONTEXT *ctx)");
    pThis->m_ctxConnect.lAddr.RemoveAll();
    pThis->m_ctxConnect.lAddr.AddTail(&ctx->lAddr);
    pThis->m_ctxConnect.posList = ctx->posList;
    pThis->m_ctxConnect.bLogin = ctx->bLogin;
    pThis->m_ctxConnect.posList = reinterpret_cast<__POSITION *>(pThis->m_ctxConnect.lAddr.GetHeadPosition());
    pThis->m_addr = *pThis->m_ctxConnect.lAddr.GetHeadPosition();
    CClientSocket__Connect_Addr_Hook(pThis, edx, &pThis->m_addr);
}

// int __thiscall CLogin::SendCheckPasswordPacket(CLogin *this, char *sID, char *sPasswd)
typedef INT(__fastcall *_CLogin__SendCheckPasswordPacket_t)(CLogin *pThis, PVOID edx, char *sID, char *sPasswd);

_CLogin__SendCheckPasswordPacket_t _CLogin__SendCheckPasswordPacket;

INT __fastcall CLogin__SendCheckPasswordPacket_Hook(CLogin *pThis, PVOID edx, char *sID, char *sPasswd) {
    Log("CLogin::SendCheckPasswordPacket(CLogin *pThis, PVOID edx, char *sID, char *sPasswd)");
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
    // ZXString<char>::GetBuffer(CWvsContext::GetInstance() + 8264, -1, sID, 0xFFFFFFFF);
    CUITitle *cuiTitle = CUITitle::GetInstance();
    if (cuiTitle) {
        cuiTitle->ClearToolTip();
    }

    cOutPacket.m_aSendBuff.RemoveAll();
    return 1;
}

// void __thiscall CWvsApp::CallUpdate(CWvsApp *this, int tCurTime)
typedef VOID(__fastcall *_CWvsApp__CallUpdate_t)(CWvsApp *pThis, PVOID edx, int tCurTime);

_CWvsApp__CallUpdate_t _CWvsApp__CallUpdate;

CStage *get_stage() {
    return reinterpret_cast<CStage *>(*(void **) 0x00CD7F04);
}

// void __cdecl set_stage(CStage *pStage, void *pParam)
typedef VOID(__cdecl *_set_stage_t)(CStage *pStage, void *pParam);

_set_stage_t _set_stage = reinterpret_cast<_set_stage_t>(0x007EFFC0);

IWzGr2D *get_gr() {
    return reinterpret_cast<IWzGr2D *>(*(void **) 0x00CDB7E0);
}

VOID __fastcall CWvsApp__CallUpdate_Hook(CWvsApp *pThis, PVOID edx, int tCurTime) {
//    Log("CWvsApp::CallUpdate(CWvsApp *pThis, PVOID edx, int tCurTime=[%d])", tCurTime);
    if (pThis->m_bFirstUpdate) {
        pThis->m_tUpdateTime = tCurTime;
//        pThis->m_tLastServerIPCheck = tCurTime;
//        pThis->m_tLastServerIPCheck2 = tCurTime;
//        pThis->m_tLastGGHookingAPICheck = tCurTime;
//        pThis->m_tLastSecurityCheck = tCurTime;
        pThis->m_bFirstUpdate = 0;
        pThis->dummy21 = tCurTime;
    }

//    Log("CWvsApp::CallUpdate => m_tUpdateTime=[%d], tCurTime=[%d])", pThis->m_tUpdateTime, tCurTime);
    while (tCurTime - pThis->m_tUpdateTime > 0) {
        CStage *stage = get_stage();
        if (stage) {
            stage->Update();
        }

//        Log("CWvsApp::CallUpdate => CWndMan::s_Update()");
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
//    Log("CWvsApp::CallUpdate => get_gr()->UpdateCurrentTime(tCurTime)");
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

_TSingleton_CInputSystem__CreateInstance_t _TSingleton_CInputSystem__CreateInstance = reinterpret_cast<_TSingleton_CInputSystem__CreateInstance_t>(0x00ADC984);

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
    Log("CWvsApp::Run(CWvsApp *pThis, PVOID edx, int *pbTerminate)");
    tagMSG msg{};
    ISMSG isMsg{};
    memset(&msg, 0, sizeof(msg));
    memset(&isMsg, 0, sizeof(isMsg));
    if (CClientSocket::GetInstance()) {
        CClientSocket::GetInstance()->ManipulatePacket();
    }
    do {
        auto dwRet = MsgWaitForMultipleObjects(3u, pThis->m_ahInput, 0, 0, 0xFFu);
        //Log("CWvsApp::Run dwRet=[%d]", dwRet);
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
                if (!PeekMessageA(&msg, 0, 0, 0, 1u)) {
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
                    Log("m_hrComErrorCode = [%d]", m_hrComErrorCode);
                    _com_raise_error(m_hrComErrorCode);
                }
                if (pThis->m_hrZExceptionCode) {
                    m_hrComErrorCode = pThis->m_hrZExceptionCode;
                    pThis->m_hrComErrorCode = 0;
                    pThis->m_hrZExceptionCode = 0;
                    isError = 1;
                }
                if (isError) {
                    Log("m_hrComErrorCode = [%d]", m_hrComErrorCode);
                    if (m_hrComErrorCode == 0x20000000) {
                        Log("AVCPatchException");
                        _com_raise_error(m_hrComErrorCode);
                    }
                    if ( m_hrComErrorCode >= 553648128 && m_hrComErrorCode <= 553648134) {
                        Log("AVCDisconnectException");
                        _com_raise_error(m_hrComErrorCode);
                    }
                    if ( m_hrComErrorCode >= 570425344 && m_hrComErrorCode <= 570425357 ) {
                        Log("AVCTerminateException");
                        _com_raise_error(m_hrComErrorCode);
                    }
                    Log("AVZException");
                    _com_raise_error(m_hrComErrorCode);
                }
            } while (!*pbTerminate && msg.message != 18);
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

// void __thiscall CWvsApp::SetUp(CWvsApp *this)
typedef VOID(__fastcall *_CWvsApp__SetUp_t)(CWvsApp *pThis, PVOID edx);

_CWvsApp__SetUp_t _CWvsApp__SetUp;

VOID __fastcall CWvsApp__SetUp_Hook(CWvsApp *pThis, PVOID edx) {
    Log("CWvsApp::SetUp(CWvsApp *this)");

    auto time = timeGetTime();
    srand(time);

    CSecurityClient::CreateInstance();

    PVOID ret = ZAllocEx<ZAllocAnonSelector>::GetInstance()->Alloc(0x4ACu);
    CConfig *cConfig;
    if (ret) {
        cConfig = new(ret) CConfig();
    }

    pThis->InitializePCOM();
    pThis->CreateMainWindow();

    Log("CWvsApp::SetUp => m_hWnd=[%p]", pThis->m_hWnd);

    CClientSocket::CreateInstance();
    pThis->ConnectLogin();
    CSecurityClient::GetInstance()->m_hMainWnd = pThis->m_hWnd;

    CFuncKeyMappedMan::CreateInstance();
    CQuickslotKeyMappedMan::CreateInstance();
    CMacroSysMan::CreateInstance();
    pThis->InitializeResMan();
    pThis->InitializeGr2D();
    pThis->InitializeInput();
    ShowWindow(pThis->m_hWnd, 5);
    UpdateWindow(pThis->m_hWnd);
    SetForegroundWindow(pThis->m_hWnd);
    auto hr = get_gr()->RenderFrame();
    if (FAILED(hr)) {
        Log("Do proper _com_raise_errorex");
        return;
    }
    Sleep(100);
    pThis->InitializeSound();
    Sleep(100);

    pThis->InitializeGameData();
    pThis->CreateWndManager();

    CConfig::GetInstance()->ApplySysOpt(nullptr, 0);

    CActionMan::CreateInstance();
    CActionMan::GetInstance()->Init();

    CAnimationDisplayer::CreateInstance();

    CMapleTVMan::CreateInstance();
    //TODO look up what this is in CWvsApp
    CMapleTVMan::GetInstance()->Init(pThis->dummy15, pThis->dummy18);

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
    GetModuleFileNameA(nullptr, sModulePath, 260);
    CWvsApp::Dir_BackSlashToSlash(sModulePath);
    CWvsApp::Dir_upDir(sModulePath);
    CWvsApp::Dir_SlashToBackSlash(sModulePath);

    ZXString<char> tempString = ZXString<char>(sModulePath, 0xFFFFFFFF);
    CConfig::GetInstance()->CheckExecPathReg(tempString);

    ret = ZAllocEx<ZAllocAnonSelector>::GetInstance()->Alloc(0x2Cu);
    CStage *cLogo;
    if (ret) {
        cLogo = new(ret) CLogo();
    } else {
        cLogo = nullptr;
    }
    _set_stage(cLogo, nullptr);
}

// main thread
VOID __stdcall MainProc() {
    // Window Mode Magic
    MemEdit::WriteBytes(0x00ADA8D7+0x94, new BYTE[7]{0xC7, 0x45, 0xDC, 0x00, 0x00, 0x00, 0x00}, 7);

    // CWvsApp::SetUp
    INITMAPLEHOOK(_CWvsApp__SetUp, _CWvsApp__SetUp_t, CWvsApp__SetUp_Hook, 0x00AD7D58);

    // CWvsApp::Run
    INITMAPLEHOOK(_CWvsApp__Run, _CWvsApp__Run_t, CWvsApp__Run_Hook, 0x00AD8328);

    // CActionMan::SweepCache - ???

    // dunno, but need to noop
    MemEdit::WriteBytes(0x00B3B96B, new BYTE[1]{0xC3}, 1);

    // DR_check
    MemEdit::WriteBytes(0x004A9617, new BYTE[3]{0x33, 0xC0, 0xC3}, 3);

    // CClientSocket::OnAliveReq - think we're good here.
    // CWvsContext::OnEnterField - think we're good here

    // CLogin::SendCheckPasswordPacket
//    INITMAPLEHOOK(_CLogin__SendCheckPasswordPacket, _CLogin__SendCheckPasswordPacket_t,
//                  CLogin__SendCheckPasswordPacket_Hook, 0x0066DA6A);

    // Noop Call to CSecurityClient::OnPacket
//    MemEdit::PatchNop(dwCSecurityClientOnPacketCall, 16);

    INITMAPLEHOOK(_CClientSocket__Connect_ctx, _CClientSocket__Connect_ctx_t, CClientSocket__Connect_Ctx_Hook,
                  0x004AFF6C);
    INITMAPLEHOOK(_CClientSocket__Connect_addr, _CClientSocket__Connect_addr_t, CClientSocket__Connect_Addr_Hook,
                  0x004AFFD1);
    INITMAPLEHOOK(_CClientSocket__OnConnect, _CClientSocket__OnConnect_t, CClientSocket__OnConnect_Hook, 0x004B0066);
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