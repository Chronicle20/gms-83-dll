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

// void __thiscall CSecurityClient::OnPacket(CSecurityClient *this, CInPacket *iPacket)
typedef VOID(__fastcall *_CSecurityClient__OnPacket_t)(CSecurityClient *pThis, PVOID edx,
                                                       CInPacket *iPacket);

VOID __fastcall CSecurityClient__OnPacket_Hook(CSecurityClient *pThis, PVOID edx,CInPacket *iPacket) {
    Log("CSecurityClient::OnPacket.");
}

// void __thiscall CClientSocket::Connect(CClientSocket *this, const sockaddr_in *pAddr)
typedef VOID(__fastcall *_CClientSocket__Connect_addr_t)(CClientSocket *pThis, PVOID edx, const sockaddr_in *pAddr);

VOID __fastcall CClientSocket__Connect_Addr_Hook(CClientSocket *pThis, PVOID edx, const sockaddr_in *pAddr);

// int __thiscall CClientSocket::OnConnect(CClientSocket *this, int bSuccess)
typedef INT(__fastcall *_CClientSocket__OnConnect_t)(CClientSocket *pThis, PVOID edx, INT bSuccess);

INT __fastcall CClientSocket__OnConnect_Hook(CClientSocket *pThis, PVOID edx, int bSuccess) {
    Log("CClientSocket::OnConnect(CClientSocket *this, int bSuccess). bSuccess [%d]", bSuccess);
    if (!pThis->m_ctxConnect.lAddr.GetCount()) {
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
        //TODO do i really care to do the loadbalancing logic?
        CClientSocket__Connect_Addr_Hook(pThis, edx, pThis->m_ctxConnect.lAddr.GetHeadPosition());
        return 0;
    }

    int BUFFER_SIZE = 1460;
    ZRef<ZSocketBuffer> pBuff = ZRef<ZSocketBuffer>();
    pBuff.p = ZSocketBuffer::Alloc(BUFFER_SIZE);
    if (pBuff.p && pBuff.p->m_nRef) {
        InterlockedIncrement(&pBuff.p->m_nRef);
    }
    char* buffer = pBuff.p->buf;
    char *accumulatedBuf = buffer;
    int bLenRead = 0;
    int src = 0;
    int something = 40;
    int bytesReceived;
    while (true) {
        do {
            while (true) {
                while (true) {
                    int lenToRead;
                    if (bLenRead) {
                        lenToRead = src;
                    } else {
                        lenToRead = 2;
                    }
                    bytesReceived = recv(pThis->m_sock._m_hSocket, accumulatedBuf, lenToRead, 0);
                    if (bytesReceived != -1) {
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
                    break;
                }
                accumulatedBuf += bytesReceived;
                if (!bytesReceived) {
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
        if (src > pBuff.p->len) {
            break;
        }
        bLenRead = 1;
        accumulatedBuf = buffer;
    }
    bytesReceived = 0;
    label_26:
    if (!bytesReceived) {
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
            return 0;
        }
    }
    if (nVersionHeader != VERSION_HEADER) {
        throw std::invalid_argument("570425351");
    }
    if (majorVersion > BUILD_MAJOR_VERSION) {
        throw std::invalid_argument("CPatchException");
    }
    if (majorVersion != BUILD_MAJOR_VERSION) {
        throw std::invalid_argument("570425351");
    }
    if (version > MINOR_VERSION) {
        throw std::invalid_argument("CPatchException");
    }
    if (!version) {
        throw std::invalid_argument("570425351");
    }
    pThis->ClearSendReceiveCtx();
    pThis->m_ctxConnect.lAddr.RemoveAll();
    pThis->m_ctxConnect.posList = nullptr;
    socklen_t peerAddrLen = sizeof(pThis->m_addr);
    if (getpeername(pThis->m_sock._m_hSocket, reinterpret_cast<struct sockaddr *>(&pThis->m_addr), &peerAddrLen) ==
        -1) {
        int lastError = WSAGetLastError();
        throw std::invalid_argument("570425351");
    }

    if (pThis->m_ctxConnect.bLogin) {
        Log("CClientSocket::OnConnect should be sending [%d]", CLIENT_START_ERROR);
        // TODO relay CLIENT_START_ERROR
        char *fileName = CWvsApp::GetExceptionFileName();

    } else {
        Log("CClientSocket::OnConnect accountId=[%d], worldId=[%d], channelId=[%d], characterId=[%d]",
            CWvsContext::GetInstance()->m_dwAccountId, CWvsContext::GetInstance()->m_nWorldID,
            CWvsContext::GetInstance()->m_nChannelID, CWvsContext::GetInstance()->m_dwCharacterId);
        auto systemInfo = CSystemInfo();
        systemInfo.Init();
        auto cOutPacket = COutPacket(PLAYER_LOGGED_IN);
        cOutPacket.Encode4(CWvsContext::GetInstance()->m_dwCharacterId);
        cOutPacket.EncodeBuffer(systemInfo.GetMachineId(), 16);
#if defined(REGION_GMS)
        // Is the user a GM?
        if (CWvsContext::GetInstance()->m_nSubGradeCode.GetData() >= 0) {
            cOutPacket.Encode1(0);
        } else {
            cOutPacket.Encode1(1);
        }
#elif defined(REGION_JMS)
    cOutPacket.Encode2(CConfig::GetInstance()->dummy1);
#endif
        cOutPacket.Encode1(0);
        // TODO not sure if this exists in less than GMS 87 but greater than 83.
#if (defined(REGION_GMS) && MAJOR_VERSION > 83) || (defined(REGION_JMS))
        cOutPacket.EncodeBuffer(CWvsContext::GetInstance()->m_aClientKey, 8);
#endif

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
        Log("CClientSocket::Connect ADR Should throw an exception here.");
        return;
    }

    pThis->m_tTimeout = timeGetTime() + 5000;
    HWND hwnd = pThis->m_hWnd;

    unsigned int socketHandle = pThis->m_sock._m_hSocket;
    const UINT WM_SOCKET = WM_USER + 1; // (1025)
    const long eventMask = FD_READ | FD_WRITE | FD_CONNECT | FD_CLOSE;

    int asyncResult = WSAAsyncSelect(socketHandle, hwnd, WM_SOCKET, eventMask);
    int connectResult = connect(socketHandle, reinterpret_cast<const sockaddr *>(pAddr), sizeof(sockaddr_in));
    int lastError = WSAGetLastError();

    Log("CClientSocket::Connect ADR asyncResult [%d], connectResult [%d], lastError [%d].", asyncResult, connectResult,
        lastError);

    if (asyncResult == SOCKET_ERROR || connectResult != SOCKET_ERROR || lastError != WSAEWOULDBLOCK) {
        Log("CClientSocket::Connect ADR Try CClientSocket::OnConnect");
        CClientSocket__OnConnect_Hook(pThis, edx, 0);
    }
}

// void __thiscall CClientSocket::Connect(CClientSocket *this, const CClientSocket::CONNECTCONTEXT *ctx)
typedef VOID(__fastcall *_CClientSocket__Connect_ctx_t)(CClientSocket *pThis, PVOID edx,
                                                        CClientSocket::CONNECTCONTEXT *ctx);

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

INT __fastcall CLogin__SendCheckPasswordPacket_Hook(CLogin *pThis, PVOID edx, char *sID, char *sPasswd) {
    Log("CLogin::SendCheckPasswordPacket. ID [%s]. bRequestSent [%d].", sID, pThis->m_bRequestSent);
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
#if defined(REGION_GMS)
    cOutPacket.Encode4(CConfig::GetInstance()->GetPartnerCode());
#endif
    CClientSocket::GetInstance()->SendPacket(&cOutPacket);
#if defined(REGION_JMS)
    CWvsContext::GetInstance()->unk1.Assign(sID, 0xFFFFFFFF);
#endif
    // ZXString<char>::GetBuffer(CWvsContext::GetInstance() + 8264, -1, sID, 0xFFFFFFFF);
    CUITitle *cuiTitle = CUITitle::GetInstance();
    if (cuiTitle) {
        cuiTitle->ClearToolTip();
    }

    cOutPacket.m_aSendBuff.RemoveAll();
    return 1;
}

CStage *get_stage() {
    return reinterpret_cast<CStage *>(*(void **) GET_STAGE);
}

// void __cdecl set_stage(CStage *pStage, void *pParam)
typedef VOID(__cdecl *_set_stage_t)(CStage *pStage, void *pParam);

_set_stage_t _set_stage = reinterpret_cast<_set_stage_t>(SET_STAGE);

IWzGr2D *get_gr() {
    return reinterpret_cast<IWzGr2D *>(*(uint32_t **) GET_GR);
}

// int __cdecl DR_check()
typedef INT(__cdecl *_DR__check_t)();

INT __cdecl DR__check_Hook() {
    return 0;
}

// void __thiscall CWvsApp::CallUpdate(CWvsApp *this, int tCurTime)
typedef VOID(__fastcall *_CWvsApp__CallUpdate_t)(CWvsApp *pThis, PVOID edx, int tCurTime);

VOID __fastcall CWvsApp__CallUpdate_Hook(CWvsApp *pThis, PVOID edx, int tCurTime) {
    if (pThis->m_bFirstUpdate) {
        pThis->m_tUpdateTime = tCurTime;
#if defined(REGION_GMS)
        pThis->m_tLastServerIPCheck = tCurTime;
        pThis->m_tLastServerIPCheck2 = tCurTime;
        pThis->m_tLastGGHookingAPICheck = tCurTime;
#endif
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

// void __thiscall CWvsApp::ConnectLogin(CWvsApp *this)
typedef VOID(__fastcall *_CWvsApp__ConnectLogin_t)(CWvsApp *pThis, PVOID edx);

VOID __fastcall CWvsApp__ConnectLogin_Hook(CWvsApp *pThis, PVOID edx) {
    Log("CWvsApp::ConnectLogin_Hook");
    CClientSocket *pSock = CClientSocket::GetInstance();
    pSock->Close();
    pSock->ConnectLogin();

    tagMSG msg{};
    while (true) {
        if (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_USER + 1) {
                unsigned int handle = pSock->m_sock._m_hSocket;
                if (msg.wParam != handle)
                    continue;

                int errorCode = HIWORD(msg.lParam);
                if (errorCode) {
                    if (errorCode != 10038) {
                        Log("CWvsApp::ConnectLogin_Hook Call CClientSocket::OnConnect 1");
                        CClientSocket__OnConnect_Hook(pSock, edx, 0);
                    }
                } else {
                    WORD low = LOWORD(msg.lParam);
                    if (low != 16 && low != 1) {
                        Log("CWvsApp::ConnectLogin_Hook Call CClientSocket::OnConnect 2");
                        CClientSocket__OnConnect_Hook(pSock, edx, 0);
                        continue;
                    }

                    Log("CWvsApp::ConnectLogin_Hook Call CClientSocket::OnConnect 3");
                    if (CClientSocket__OnConnect_Hook(pSock, edx, 1)) {
                        break;
                    }
                }
            } else {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);

                if ((LONG) (timeGetTime() - pSock->m_tTimeout) > 0) {
                    Log("CWvsApp::ConnectLogin_Hook Call CClientSocket::OnConnect 4. timeGetTime [%d], timeOut [%d].",
                        timeGetTime(), pSock->m_tTimeout);
                    CClientSocket__OnConnect_Hook(pSock, edx, 0);
                }
            }
        } else {
            if ((LONG) (timeGetTime() - pSock->m_tTimeout) > 0) {
                Log("CWvsApp::ConnectLogin_Hook Call CClientSocket::OnConnect 5");
                CClientSocket__OnConnect_Hook(pSock, edx, 0);
            }
        }

        if (msg.message == WM_QUIT) {
            break;
        }
    }

    auto handle = pSock->m_sock._m_hSocket;
    if (handle == 0 || handle == -1) {
        Log("CWvsApp::ConnectLogin_Hook Should issue exception here.");
//        CTerminateException ex(570425345);
//        int* exceptionObject = reinterpret_cast<int*>(&ex); // if needed
//        _CxxThrowException(exceptionObject, &_TI3_AVCTerminateException__);
    }
}

// void __thiscall CWvsApp::InitializeInput(CWvsApp *this)
typedef VOID(__fastcall *_CWvsApp__InitializeInput_t)(CWvsApp *pThis, PVOID edx);

VOID __fastcall CWvsApp__InitializeInput_Hook(CWvsApp *pThis, PVOID edx) {
    Log("CWvsApp::InitializeInput");
    CInputSystem::CreateInstance();
    CInputSystem::GetInstance()->Init(pThis->m_hWnd, pThis->m_ahInput);
}

// void __thiscall CWvsApp::Run(CWvsApp *this, int *pbTerminate)
typedef VOID(__stdcall *_CWvsApp__Run_t)(CWvsApp *pThis, int *pbTerminate);

VOID __fastcall CWvsApp__Run_Hook(CWvsApp *pThis, PVOID edx, int *pbTerminate) {
    Log("CWvsApp::Run");
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
                if (!PeekMessageA(&msg, nullptr, 0, 0, 1u)) {
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

void GetSEPrivilege() {
    ((VOID * *(_fastcall * )())
    GET_SE_PRIVILEGE)();
}

// void __thiscall CWvsApp::SetUp(CWvsApp *this)
typedef VOID(__stdcall *_CWvsApp__SetUp_t)(CWvsApp *pThis);

VOID __fastcall CWvsApp__SetUp_Hook(CWvsApp *pThis) {
    Log("CWvsApp::SetUp");
#if defined(REGION_GMS)
    pThis->InitializeAuth();
#endif

    auto time = timeGetTime();
    srand(time);

#if defined(REGION_GMS)
    GetSEPrivilege();
#endif

    CSecurityClient::CreateInstance();

    PVOID cfgAlloc = ZAllocEx<ZAllocAnonSelector>::GetInstance()->Alloc(sizeof(CConfig));
    CConfig *cConfig;
    if (cfgAlloc) {
        cConfig = new(cfgAlloc) CConfig();
    }

    pThis->InitializePCOM();
    pThis->CreateMainWindow();

    CClientSocket::CreateInstance();
    pThis->ConnectLogin();

    CSecurityClient::GetInstance()->m_hMainWnd = pThis->m_hWnd;

    CFuncKeyMappedMan::CreateInstance();
    CQuickslotKeyMappedMan::CreateInstance();
    CMacroSysMan::CreateInstance();

    pThis->InitializeResMan();
    pThis->InitializeGr2D();
    pThis->InitializeInput();

#if defined(REGION_JMS)
    ShowWindow(pThis->m_hWnd, 5);
    UpdateWindow(pThis->m_hWnd);
    SetForegroundWindow(pThis->m_hWnd);
    auto hr = get_gr()->RenderFrame();
    if (FAILED(hr)) {
        Log("Do proper _com_raise_errorex");
        return;
    }
#endif

    Sleep(300);
    pThis->InitializeSound();
    Sleep(300);
    pThis->InitializeGameData();
    pThis->CreateWndManager();

    CConfig::GetInstance()->ApplySysOpt(nullptr, 0);

    CActionMan::CreateInstance();
    CActionMan::GetInstance()->Init();

    CAnimationDisplayer::CreateInstance();

    CMapleTVMan::CreateInstance();
#if defined(REGION_GMS)
    CMapleTVMan::GetInstance()->Init();
#elif defined(REGION_JMS)
    CMapleTVMan::GetInstance()->Init(pThis->unk1[1], pThis->unk1[0]);
#endif

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

    PVOID ret = ZAllocEx<ZAllocAnonSelector>::GetInstance()->Alloc(sizeof(CLogo));
    CStage *cLogo;
    if (ret) {
        cLogo = new(ret) CLogo();
    } else {
        cLogo = nullptr;
    }
    _set_stage(cLogo, nullptr);
}

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);

DWORD ResetLSP() {
    return reinterpret_cast<DWORD>(*(void **) RESET_LSP);
}

typedef VOID(__stdcall *_CWvsApp__CWvsApp_t)(CWvsApp *pThis, const char *sCmdLine);

// CWvsApp::CWvsApp
VOID __fastcall CWvsApp__CWvsApp_Hook(CWvsApp *pThis, const char *sCmdLine) {
    Log("CWvsApp::CWvsApp");
    void **instance = reinterpret_cast<void **>(C_WVS_APP_INSTANCE);
    *instance = &pThis->m_hWnd != 0 ? pThis : 0;

    pThis->m_hWnd = nullptr;
    pThis->m_bPCOMInitialized = 0;
    pThis->m_hHook = nullptr;
    pThis->m_tUpdateTime = 0;
    pThis->m_bFirstUpdate = 1;
    pThis->m_sCmdLine = ZXString<char>();
    pThis->m_nGameStartMode = 0;
    pThis->m_bAutoConnect = 1;
#if defined(REGION_JMS)
    pThis->unk1[0] = 0;
    pThis->unk1[1] = 0;
    pThis->unk2[0] = ZXString<char>();
    pThis->unk2[1] = ZXString<char>();
#endif
    pThis->m_bShowAdBalloon = 0;
    pThis->m_bExitByTitleEscape = 0;
    pThis->m_hrZExceptionCode = 0;
    pThis->m_hrComErrorCode = 0;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 87)
    pThis->m_tNextSecurityCheck = 0;
    pThis->m_pBackupBuffer = ZArray<unsigned char>();
    pThis->m_dwBackupBufferSize = 0;
#endif

#if defined(REGION_JMS)
    pThis->unk2[0] = ZXString<char>("", 0xFFFFFFFF);
    pThis->unk2[0] = ZXString<char>("", 0xFFFFFFFF);
#endif
    pThis->m_sCmdLine = ZXString<char>(sCmdLine, 0xFFFFFFFF);

    pThis->m_sCmdLine = *pThis->m_sCmdLine.TrimRight("\" ")->TrimLeft("\" ");
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 87)
    pThis->m_pBackupBuffer.Alloc(0x1000);
#endif
    ZXString<char> sToken = ZXString<char>();
    pThis->GetCmdLine(&sToken, 0);

    pThis->m_nGameStartMode = 2;
    pThis->m_dwMainThreadId = GetCurrentThreadId();

    OSVERSIONINFO ovi;
    ZeroMemory(&ovi, sizeof(OSVERSIONINFO));
    ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionExA(&ovi);
    pThis->m_bWin9x = ovi.dwPlatformId == 1;
    if (ovi.dwMajorVersion >= 6 && !pThis->m_nGameStartMode) {
        pThis->m_nGameStartMode = 2;
    }

#if defined(REGION_GMS)
    int *g_dwTargetOS = reinterpret_cast<int *>(G_DW_TARGET_OS);

    if (ovi.dwMajorVersion < 5) {
        *g_dwTargetOS = 1996;
    }

    BOOL bIsWow64 = FALSE;
    auto fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
            GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
    if (fnIsWow64Process) {
        fnIsWow64Process(GetCurrentProcess(), &bIsWow64);
    }

    if (bIsWow64) {
        *g_dwTargetOS = 1996;
    }
    if (ovi.dwMajorVersion >= 6 && !bIsWow64) {
        ResetLSP();
    }
    sToken.Empty();
#endif
}

// main thread
DWORD WINAPI MainProc(LPVOID lpParam) {

    // CWvsApp::CWvsApp
    HOOKTYPEDEF_C(CWvsApp__CWvsApp);
    INITMAPLEHOOK(_CWvsApp__CWvsApp, _CWvsApp__CWvsApp_t, CWvsApp__CWvsApp_Hook, C_WVS_APP);

    // CWvsApp::SetUp
    HOOKTYPEDEF_C(CWvsApp__SetUp);
    INITMAPLEHOOK(_CWvsApp__SetUp, _CWvsApp__SetUp_t, CWvsApp__SetUp_Hook, C_WVS_APP_SET_UP);

    // CWvsApp::InitializeInput
    HOOKTYPEDEF_C(CWvsApp__InitializeInput);
    INITMAPLEHOOK(_CWvsApp__InitializeInput, _CWvsApp__InitializeInput_t, CWvsApp__InitializeInput_Hook, C_WVS_APP_INITIALIZE_INPUT);

    // CWvsApp::Run
    HOOKTYPEDEF_C(CWvsApp__Run);
    INITMAPLEHOOK(_CWvsApp__Run, _CWvsApp__Run_t, CWvsApp__Run_Hook, C_WVS_APP_RUN);

#if defined(REGION_JMS)
    // TODO define these functions.
    MemEdit::WriteBytes(0x00B3B96B, new BYTE[1]{0xC3}, 1);

#endif
#if (defined(REGION_GMS) && MAJOR_VERSION >= 87) || defined(REGION_JMS)
    HOOKTYPEDEF_C(DR__check);
    INITMAPLEHOOK(_DR__check, _DR__check_t, DR__check_Hook, DR_CHECK);
#endif

#if defined(REGION_JMS)
    MemEdit::WriteBytes(0x00B3B5F7 + 0x19, new BYTE[2]{0x90, 0x90}, 2);
#endif

    // CWvsApp::CallUpdate
    HOOKTYPEDEF_C(CWvsApp__CallUpdate);
    INITMAPLEHOOK(_CWvsApp__CallUpdate, _CWvsApp__CallUpdate_t, CWvsApp__CallUpdate_Hook, C_WVS_APP_CALL_UPDATE);

    // CWvsApp::ConnectLogin
    HOOKTYPEDEF_C(CWvsApp__ConnectLogin);
    INITMAPLEHOOK(_CWvsApp__ConnectLogin, _CWvsApp__ConnectLogin_t, CWvsApp__ConnectLogin_Hook,
                  C_WVS_APP_CONNECT_LOGIN);

    // CLogin::SendCheckPasswordPacket
    HOOKTYPEDEF_C(CLogin__SendCheckPasswordPacket);
    INITMAPLEHOOK(_CLogin__SendCheckPasswordPacket, _CLogin__SendCheckPasswordPacket_t,
                  CLogin__SendCheckPasswordPacket_Hook, C_LOGIN_SEND_CHECK_PASSWORD_PACKET);

    // CSecurityClient::OnPacket
    HOOKTYPEDEF_C(CSecurityClient__OnPacket);
    INITMAPLEHOOK(_CSecurityClient__OnPacket, _CSecurityClient__OnPacket_t, CSecurityClient__OnPacket_Hook,
                  C_SECURITY_CLIENT_ON_PACKET);

    // CClientSocket::Connect
    HOOKTYPEDEF_C(CClientSocket__Connect_ctx);
    INITMAPLEHOOK(_CClientSocket__Connect_ctx, _CClientSocket__Connect_ctx_t, CClientSocket__Connect_Ctx_Hook,
                  C_CLIENT_SOCKET_CONNECT_CTX);

    // CClientSocket::Connect
    HOOKTYPEDEF_C(CClientSocket__Connect_addr);
    INITMAPLEHOOK(_CClientSocket__Connect_addr, _CClientSocket__Connect_addr_t, CClientSocket__Connect_Addr_Hook,
                  C_CLIENT_SOCKET_CONNECT_ADR);

    // // CClientSocket::OnConnect
    HOOKTYPEDEF_C(CClientSocket__OnConnect);
    INITMAPLEHOOK(_CClientSocket__OnConnect, _CClientSocket__OnConnect_t, CClientSocket__OnConnect_Hook,
                  C_CLIENT_SOCKET_ON_CONNECT);
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