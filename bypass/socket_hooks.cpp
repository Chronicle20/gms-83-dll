#include "pch.h"

#include "socket_hooks.h"
#include "socket_hooks_internal.h"

#include "client_exception.h"
#include "hooker.h"
#include "logger.h"

#include <WS2tcpip.h>
#include <timeapi.h>

// ---- forward declarations (same-TU) ------------------------------------
VOID __fastcall CClientSocket__Connect_Addr_Hook(CClientSocket* pThis, PVOID edx, const sockaddr_in* pAddr);

// ---- typedefs -----------------------------------------------------------
typedef VOID(__thiscall* _CClientSocket__Connect_addr_t)(CClientSocket* pThis, const sockaddr_in* pAddr);
typedef VOID(__thiscall* _CClientSocket__Connect_ctx_t)(CClientSocket* pThis, CClientSocket::CONNECTCONTEXT* ctx);
typedef INT(__thiscall* _CClientSocket__OnConnect_t)(CClientSocket* pThis, INT bSuccess);
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
typedef VOID(__thiscall* _CClientSocket__SendPacket_t)(CClientSocket* pThis, COutPacket* oPacket);
#endif

// ---- CLIENT_START_ERROR report-read mirror (§3) -------------------------
// Thin stack mirror of the stock CFileStream object. Only vftable@[0] and
// handle@[+0x10] are semantically meaningful to us, but the buffer MUST be at
// least the full stock object size so Open/Read/Close do not write OOB.
// `pad` is sized from the v84.1 stack frame (Task 3 / OQ-1): Open writes a dword
// at [+0x34], so total object >= 0x38; pad[0x3C] (total 0x40) is the conservative
// cover.
struct ClientFileStream {
    void* vftable;  // [+0x00] -> C_FILE_STREAM_VFTABLE
    char pad[0x3C]; // [+0x04] conservative; covers the [+0x34] flag write
};

using FileStreamOpenFn = int(__thiscall*)(void* self, const char* name, unsigned access, unsigned share, int a3,
                                          unsigned disp, int a5, int a6);
using FileStreamLenFn = unsigned(__thiscall*)(void* self);
using FileStreamReadFn = int(__thiscall*)(void* self, void* dst, unsigned len);
using FileStreamCloseFn = void(__thiscall*)(void* self);

// ---- OnConnect helpers (§4.2 refactor) ---------------------------------
namespace {

// Read up to `lenToRead` bytes into `out`. Decrements `retries` once per
// WSAEWOULDBLOCK observed and sleeps 500 ms before retrying — preserving
// the pre-refactor budget that was shared across the header+body reads.
// Returns the bytes actually received, or 0 on disconnect / retry budget
// exhausted (same sentinel the inlined loop used).
int read_chunk(CClientSocket* pSock, char* out, int lenToRead, int& retries) {
    while (true) {
        int bytesReceived = recv(pSock->m_sock._m_hSocket, out, lenToRead, 0);
        if (bytesReceived != -1) {
            return bytesReceived;
        }
        int wsaLastError = WSAGetLastError();
        Log("CClientSocket::OnConnect wsaLastError=[%d]", wsaLastError);
        if (wsaLastError == WSAEWOULDBLOCK) {
            Sleep(500);
            if (--retries >= 0) {
                continue;
            }
        }
        return 0;
    }
}

// PRD §4.2 signature. Header is always 2 bytes; we keep the literal here
// rather than parameterizing it because the protocol constant doesn't
// vary across the call sites.
int read_packet_header(CClientSocket* pSock, char* out, int& retries) {
    return read_chunk(pSock, out, 2, retries);
}

int read_packet_body(CClientSocket* pSock, char* out, int expectedLen, int& retries) {
    int total = 0;
    while (total < expectedLen) {
        int got = read_chunk(pSock, out + total, expectedLen - total, retries);
        if (got == 0) {
            return 0;
        }
        total += got;
    }
    return total;
}

bool decode_handshake(const char* buf, int len, unsigned short& outMajorVersion, int& outMinorVersion,
                      unsigned int& outSeqSnd, unsigned int& outSeqRcv, unsigned char& outVersionHeader) {
    const char* end = buf + len;
    const char* result = buf;

    unsigned short majorVersion;
    result += CIOBufferManipulator::Decode2(&majorVersion, const_cast<char*>(result), end - result);

    ZXString<char> minorVersionStr;
    result += CIOBufferManipulator::DecodeStr(&minorVersionStr, const_cast<char*>(result), end - result);
    int minor = atoi(minorVersionStr.m_pStr);
    minorVersionStr.Empty();

    unsigned int seqSnd;
    result += CIOBufferManipulator::Decode4(&seqSnd, const_cast<char*>(result), end - result);

    unsigned int seqRcv;
    result += CIOBufferManipulator::Decode4(&seqRcv, const_cast<char*>(result), end - result);

    unsigned char versionHeader;
    result += CIOBufferManipulator::Decode1(&versionHeader, const_cast<char*>(result), end - result);

    if (result < end) {
        // Buffer underrun — caller calls RaiseTerminate(0x22000007).
        return false;
    }

    outMajorVersion = majorVersion;
    outMinorVersion = minor;
    outSeqSnd = seqSnd;
    outSeqRcv = seqRcv;
    outVersionHeader = versionHeader;
    return true;
}

} // anonymous namespace

// ---- hook bodies --------------------------------------------------------

INT __fastcall CClientSocket__OnConnect_Hook(CClientSocket* pThis, PVOID edx, int bSuccess) {
    Log("CClientSocket::OnConnect(CClientSocket *this, int bSuccess). bSuccess [%d]", bSuccess);
    if (!pThis->m_ctxConnect.lAddr.GetCount()) {
        return 0;
    }
    if (!bSuccess) {
        if (!pThis->m_ctxConnect.posList) {
            // Address list exhausted: close and raise the stock typed exception,
            // which unwinds into the client WinMain handler (both [[noreturn]]).
            pThis->Close();
            if (pThis->m_ctxConnect.bLogin) {
                Log("CClientSocket::OnConnect connect failed (login) -> RaiseTerminate(0x22000001)");
                RaiseTerminate(0x22000001); // CTerminateException, magic 570425345
            }
            Log("CClientSocket::OnConnect connect failed -> RaiseDisconnect(0x21000001)");
            RaiseDisconnect(0x21000001); // CDisconnectException, magic 553648129
        }

        // Advance the load-balancing cursor (GetNext returns the current node and
        // moves posList to node->m_pNext) and retry the *current* node's address.
        auto* pos = reinterpret_cast<ZInetAddr*>(pThis->m_ctxConnect.posList);
        ZInetAddr* current = pThis->m_ctxConnect.lAddr.GetNext(&pos);
        pThis->m_ctxConnect.posList = reinterpret_cast<__POSITION*>(pos);
        CClientSocket__Connect_Addr_Hook(pThis, edx, current); // ZInetAddr : sockaddr_in
        return 0;
    }

    const int BUFFER_SIZE = 1460;
    ZRef<ZSocketBuffer> pBuff = ZRef<ZSocketBuffer>();
    pBuff.p = ZSocketBuffer::Alloc(BUFFER_SIZE);
    if (pBuff.p && pBuff.p->m_nRef) {
        InterlockedIncrement(&pBuff.p->m_nRef);
    }
    char* buffer = pBuff.p->buf;
    char* accumulatedBuf = buffer;
    int retries = 40;

    int hdrBytes = read_packet_header(pThis, buffer, retries);
    if (hdrBytes == 0) {
        CClientSocket__OnConnect_Hook(pThis, edx, 0);
        return 0;
    }
    accumulatedBuf = buffer + hdrBytes;

    int expectedLen = static_cast<unsigned char>(buffer[0]);
    if (expectedLen > pBuff.p->len) {
        CClientSocket__OnConnect_Hook(pThis, edx, 0);
        return 0;
    }

    int bodyBytes = read_packet_body(pThis, buffer, expectedLen, retries);
    if (bodyBytes == 0) {
        CClientSocket__OnConnect_Hook(pThis, edx, 0);
        return 0;
    }
    accumulatedBuf = buffer + bodyBytes;

    Log("CClientSocket::OnConnect Recv Decoding");

    unsigned short majorVersion;
    int minorVersionValue;
    unsigned int uSeqSnd;
    unsigned int uSeqRcv;
    unsigned char nVersionHeader;
    if (!decode_handshake(buffer, accumulatedBuf - buffer, majorVersion, minorVersionValue, uSeqSnd, uSeqRcv,
                          nVersionHeader)) {
        // buffer underrun mid-decode — original code returned 0 here
        return 0;
    }
    Log("CClientSocket::OnConnect majorVersion=[%d]", majorVersion);
    Log("CClientSocket::OnConnect minorVersion=[%d]", minorVersionValue);
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
        RaiseTerminate(0x22000007);
    }
    if (majorVersion > BUILD_MAJOR_VERSION) {
        RaisePatch();
    }
    if (majorVersion != BUILD_MAJOR_VERSION) {
        RaiseTerminate(0x22000007);
    }
    if (minorVersionValue > BUILD_MINOR_VERSION) {
        RaisePatch();
    }
    if (!minorVersionValue) {
        RaiseTerminate(0x22000007);
    }
    pThis->ClearSendReceiveCtx();
    pThis->m_ctxConnect.lAddr.RemoveAll();
    pThis->m_ctxConnect.posList = nullptr;
    socklen_t peerAddrLen = sizeof(pThis->m_addr);
    if (getpeername(pThis->m_sock._m_hSocket, reinterpret_cast<struct sockaddr*>(&pThis->m_addr), &peerAddrLen) == -1) {
        int lastError = WSAGetLastError();
        RaiseTerminate(0x22000007);
    }

    if (pThis->m_ctxConnect.bLogin) {
        Log("CClientSocket::OnConnect relaying CLIENT_START_ERROR [%d]", CLIENT_START_ERROR);
#if C_FILE_STREAM_RESOLVED
        char* fileName = CWvsApp::GetExceptionFileName();

        // OQ-throw (b): the stock Open() throws ZException on a missing file and
        // the throw escapes OnConnect. Pre-check existence so a routine missing
        // report is a silent no-op (no packet, no exception) per FR-11. Only the
        // (0 < len < 0x2000) guard below is insufficient — the throw is earlier.
        if (fileName && GetFileAttributesA(fileName) != INVALID_FILE_ATTRIBUTES) {
            ClientFileStream fs{};
            fs.vftable = reinterpret_cast<void*>(C_FILE_STREAM_VFTABLE);

#if C_FILE_STREAM_OPEN_INLINE
            // v95 has no standalone Open(): the stock OnConnect inlines CreateFileA and
            // stores the handle at [+0x10], then ORs the open flag into the state dword
            // at [+0x34]. Replicate that exact inline open, then drive the resolved
            // GetLength/Read/Close by address (the existence guard above already made a
            // routine missing file a no-op, mirroring the stock INVALID_HANDLE_VALUE throw).
            *reinterpret_cast<HANDLE*>(reinterpret_cast<char*>(&fs) + 0x10) = CreateFileA(
                fileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            *reinterpret_cast<unsigned*>(reinterpret_cast<char*>(&fs) + 0x34) |= 1u;
#else
            // Open(name, GENERIC_READ=0x80000000, share=128, 1, disp=0, 0, 0)
            reinterpret_cast<FileStreamOpenFn>(C_FILE_STREAM_OPEN)(&fs, fileName, 0x80000000u, 128, 1, 0, 0, 0);
#endif
            unsigned len = reinterpret_cast<FileStreamLenFn>(C_FILE_STREAM_GET_LENGTH)(&fs);

            ZArray<char> report;
            if (len && len < 0x2000) {
                char* dst = report.SetSize(len);
                reinterpret_cast<FileStreamReadFn>(C_FILE_STREAM_READ)(&fs, dst, len);
            }
            reinterpret_cast<FileStreamCloseFn>(C_FILE_STREAM_CLOSE)(&fs);

            if (report.GetCount()) {
                COutPacket pkt(CLIENT_START_ERROR);
                pkt.Encode2(static_cast<unsigned short>(report.GetCount()));
                pkt.EncodeBuffer(report.GetData(), report.GetCount());
                CClientSocket::GetInstance()->SendPacket(&pkt);
            }
            // ZArray<char> report frees itself at scope exit; fs is a stack object,
            // its handle released by Close().
        }
#else
        Log("CClientSocket::OnConnect CLIENT_START_ERROR relay gated off for this version");
#endif
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
        if (CWvsContext::GetInstance()->m_nSubGradeCode.GetData() >= 0) {
            cOutPacket.Encode1(0);
        } else {
            cOutPacket.Encode1(1);
        }
#elif defined(REGION_JMS)
        cOutPacket.Encode2(CConfig::GetInstance()->dummy1);
#endif
        cOutPacket.Encode1(0);
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION > 83) || (defined(REGION_JMS))
        cOutPacket.EncodeBuffer(CWvsContext::GetInstance()->m_aClientKey, 8);
#endif
        CClientSocket::GetInstance()->SendPacket(&cOutPacket);
        // (m_aSendBuff.RemoveAll() removed per §4.3 — dtor handles it.)
    }

    //_ZRef_ZSocketBuffer__Destructor(&pBuff, edx, 0);
    return 1;
}

VOID __fastcall CClientSocket__Connect_Addr_Hook(CClientSocket* pThis, PVOID edx, const sockaddr_in* pAddr) {
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
    const UINT WM_SOCKET = WM_USER + 1;
    const long eventMask = FD_READ | FD_WRITE | FD_CONNECT | FD_CLOSE;

    int asyncResult = WSAAsyncSelect(socketHandle, hwnd, WM_SOCKET, eventMask);
    int connectResult = connect(socketHandle, reinterpret_cast<const sockaddr*>(pAddr), sizeof(sockaddr_in));
    int lastError = WSAGetLastError();

    Log("CClientSocket::Connect ADR asyncResult [%d], connectResult [%d], lastError [%d].", asyncResult, connectResult,
        lastError);

    if (asyncResult == SOCKET_ERROR || connectResult != SOCKET_ERROR || lastError != WSAEWOULDBLOCK) {
        Log("CClientSocket::Connect ADR Try CClientSocket::OnConnect");
        CClientSocket__OnConnect_Hook(pThis, edx, 0);
    }
    Log("CClientSocket::Connect ADR Happy Path");
}

VOID __fastcall CClientSocket__Connect_Ctx_Hook(CClientSocket* pThis, PVOID edx, CClientSocket::CONNECTCONTEXT* ctx) {
    Log("CClientSocket::Connect(CClientSocket *this, const CClientSocket::CONNECTCONTEXT *ctx)");
    pThis->m_ctxConnect.lAddr.RemoveAll();
    pThis->m_ctxConnect.lAddr.AddTail(&ctx->lAddr);
    pThis->m_ctxConnect.posList = ctx->posList;
    pThis->m_ctxConnect.bLogin = ctx->bLogin;
    pThis->m_ctxConnect.posList = reinterpret_cast<__POSITION*>(pThis->m_ctxConnect.lAddr.GetHeadPosition());
    pThis->m_addr = *pThis->m_ctxConnect.lAddr.GetHeadPosition();
    CClientSocket__Connect_Addr_Hook(pThis, edx, &pThis->m_addr);
    Log("CClientSocket::Connect CTX Happy Path");
}

#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
VOID __fastcall CClientSocket__SendPacket_Hook(CClientSocket* pThis, PVOID edx, COutPacket* oPacket) {
    Log("CClientSocket::SendPacket (rewritten)");

    ZSynchronizedHelper<ZFatalSection> sync(&pThis->m_lockSend);

    unsigned int hSocket = pThis->m_sock._m_hSocket;
    if (hSocket && hSocket != INVALID_SOCKET && !pThis->m_ctxConnect.lAddr.GetCount()) {
        oPacket->MakeBufferList(&pThis->m_lpSendBuff, 0x5F, &pThis->m_uSeqSnd, 1, pThis->m_uSeqSnd);
        pThis->m_uSeqSnd = CIGCipher::innoHash(reinterpret_cast<unsigned char*>(&pThis->m_uSeqSnd), 4, nullptr);
        pThis->Flush();
    }
}
#endif

// ---- installer ----------------------------------------------------------
BOOL InstallSocketHooks() {
    HOOKTYPEDEF_C(CClientSocket__Connect_ctx);
    INITMAPLEHOOK_OR_RETURN(_CClientSocket__Connect_ctx, _CClientSocket__Connect_ctx_t, CClientSocket__Connect_Ctx_Hook,
                            C_CLIENT_SOCKET_CONNECT_CTX);

    HOOKTYPEDEF_C(CClientSocket__Connect_addr);
    INITMAPLEHOOK_OR_RETURN(_CClientSocket__Connect_addr, _CClientSocket__Connect_addr_t,
                            CClientSocket__Connect_Addr_Hook, C_CLIENT_SOCKET_CONNECT_ADR);

    HOOKTYPEDEF_C(CClientSocket__OnConnect);
    INITMAPLEHOOK_OR_RETURN(_CClientSocket__OnConnect, _CClientSocket__OnConnect_t, CClientSocket__OnConnect_Hook,
                            C_CLIENT_SOCKET_ON_CONNECT);

#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
    HOOKTYPEDEF_C(CClientSocket__SendPacket);
    INITMAPLEHOOK_OR_RETURN(_CClientSocket__SendPacket, _CClientSocket__SendPacket_t, CClientSocket__SendPacket_Hook,
                            C_CLIENT_SOCKET_SEND_PACKET);
#endif

    return TRUE;
}
