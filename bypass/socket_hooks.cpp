#include "pch.h"

#include "socket_hooks.h"
#include "socket_hooks_internal.h"

#include "CTerminateException.h"
#include "hooker.h"
#include "logger.h"

#include <WS2tcpip.h>
#include <timeapi.h>

#include "parse_ini.h"
#include <cstdlib>

// ---- forward declarations (same-TU) ------------------------------------
VOID __fastcall CClientSocket__Connect_Addr_Hook(CClientSocket* pThis, PVOID edx, const sockaddr_in* pAddr);

// ---- typedefs -----------------------------------------------------------
typedef VOID(__thiscall* _CClientSocket__Connect_addr_t)(CClientSocket* pThis, const sockaddr_in* pAddr);
typedef VOID(__thiscall* _CClientSocket__Connect_ctx_t)(CClientSocket* pThis, CClientSocket::CONNECTCONTEXT* ctx);
typedef INT(__thiscall* _CClientSocket__OnConnect_t)(CClientSocket* pThis, INT bSuccess);
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
typedef VOID(__thiscall* _CClientSocket__SendPacket_t)(CClientSocket* pThis, COutPacket* oPacket);
#endif

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
        // Buffer underrun — caller throws CTerminateException(570425351).
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

#if defined(REGION_GMS) && BUILD_MAJOR_VERSION == 84
// ---- v84 login reroute (maplestory-level, not winsock SPI) --------------
// v84's in-game anti-tamper scans the winsock SPI proc-table for hooks and traps
// the bypass when redirect.dll is loaded (it swaps WSPConnect/WSPGetPeerName).
// The login IP+port reroute doesn't have to live there: we do it here, in the
// CClientSocket::Connect detour -- a normal function hook the anti-tamper does
// not inspect -- reading the same edits/redirect.ini. For the v84 build, do NOT
// load redirect.dll (this replaces it).
namespace {
struct RedirectCfg {
    bool loaded = false;
    std::vector<std::string> originalIps;
    std::string redirectIp;
    unsigned short redirectPort = 0;
};

const RedirectCfg& GetRedirectCfg() {
    static const RedirectCfg cfg = [] {
        RedirectCfg c;
        ms::ini::Parsed ini;
        if (!ms::ini::Parse("edits/redirect.ini", ini, [](const char* m) { Log("%s", m); })) {
            Log("REDIR> v84 reroute: edits/redirect.ini not found -- reroute disabled");
            return c;
        }
        auto last = [&](const char* key) -> std::string {
            auto it = ini.entries.find(std::string("Main.") + key);
            return (it == ini.entries.end() || it->second.empty()) ? std::string() : it->second.back();
        };
        c.redirectIp = last("RedirectIP");
        c.redirectPort = static_cast<unsigned short>(std::strtoul(last("RedirectPort").c_str(), nullptr, 10));
        // Collect Main.OriginalIP<N> keys (the format edits/redirect.ini uses).
        const std::string prefix = "Main.OriginalIP";
        for (const auto& kv : ini.entries) {
            if (kv.first.size() <= prefix.size() || kv.first.compare(0, prefix.size(), prefix) != 0)
                continue;
            for (const auto& v : kv.second)
                if (!v.empty())
                    c.originalIps.push_back(v);
        }
        c.loaded = !c.redirectIp.empty() && c.redirectPort != 0 && !c.originalIps.empty();
        Log("REDIR> v84 reroute %s: %zu originalIPs -> %s:%u", c.loaded ? "active" : "INCOMPLETE",
            c.originalIps.size(), c.redirectIp.c_str(), c.redirectPort);
        return c;
    }();
    return cfg;
}

void RerouteGameAddr(sockaddr_in* addr) {
    const RedirectCfg& cfg = GetRedirectCfg();
    if (!cfg.loaded)
        return;
    for (const auto& oip : cfg.originalIps) {
        if (addr->sin_addr.S_un.S_addr == inet_addr(oip.c_str())) {
            addr->sin_addr.S_un.S_addr = inet_addr(cfg.redirectIp.c_str());
            break;
        }
    }
    if (ntohs(addr->sin_port) == 8484)
        addr->sin_port = htons(cfg.redirectPort);
}
} // namespace
#endif

// ---- hook bodies --------------------------------------------------------

INT __fastcall CClientSocket__OnConnect_Hook(CClientSocket* pThis, PVOID edx, int bSuccess) {
    Log("CClientSocket::OnConnect(CClientSocket *this, int bSuccess). bSuccess [%d]", bSuccess);
    if (!pThis->m_ctxConnect.lAddr.GetCount()) {
        return 0;
    }
    if (!bSuccess) {
        if (!pThis->m_ctxConnect.posList) {
            pThis->Close();
            if (pThis->m_ctxConnect.bLogin) {
                Log("CClientSocket::OnConnect 570425345");
                return 0;
            }
            Log("CClientSocket::OnConnect 553648129");
            return 0;
        }
        // TODO do i really care to do the loadbalancing logic?
        CClientSocket__Connect_Addr_Hook(pThis, edx, pThis->m_ctxConnect.lAddr.GetHeadPosition());
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
        throw CTerminateException(570425351);
    }
    if (majorVersion > BUILD_MAJOR_VERSION) {
        throw CPatchException();
    }
    if (majorVersion != BUILD_MAJOR_VERSION) {
        throw CTerminateException(570425351);
    }
    if (minorVersionValue > BUILD_MINOR_VERSION) {
        throw CPatchException();
    }
    if (!minorVersionValue) {
        throw CTerminateException(570425351);
    }
    pThis->ClearSendReceiveCtx();
    pThis->m_ctxConnect.lAddr.RemoveAll();
    pThis->m_ctxConnect.posList = nullptr;
    socklen_t peerAddrLen = sizeof(pThis->m_addr);
    if (getpeername(pThis->m_sock._m_hSocket, reinterpret_cast<struct sockaddr*>(&pThis->m_addr), &peerAddrLen) == -1) {
        int lastError = WSAGetLastError();
        throw CTerminateException(570425351);
    }

    if (pThis->m_ctxConnect.bLogin) {
        Log("CClientSocket::OnConnect should be sending [%d]", CLIENT_START_ERROR);
        // TODO relay CLIENT_START_ERROR
        char* fileName = CWvsApp::GetExceptionFileName();
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
    sockaddr_in connectAddr = *pAddr;
#if defined(REGION_GMS) && BUILD_MAJOR_VERSION == 84
    // Reroute login IP+port here (maplestory layer) instead of via redirect.dll's
    // winsock SPI hook, which v84's anti-tamper detects. Channel hops (non-8484,
    // non-original-IP) pass through unchanged. Do NOT load redirect.dll on v84.
    RerouteGameAddr(&connectAddr);
#endif
    int connectResult = connect(socketHandle, reinterpret_cast<const sockaddr*>(&connectAddr), sizeof(sockaddr_in));
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
