// Exclude rarely-used stuff from Windows headers
// Important to define this before Windows.h is included in a project because of linker issues with the WinSock2 lib
#define WIN32_LEAN_AND_MEAN

#include "hooker.h"
#include "logger.h"
#include "parse_ini.h"
#include "winhook_types.h"
#include "winhooks.h"
#include <WS2tcpip.h>
#include <Windows.h>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

SOCKET m_GameSock = INVALID_SOCKET;
WSPPROC_TABLE m_ProcTable = {nullptr};

namespace {
std::string Trim(std::string s) {
    auto left = std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isspace(c); });
    s.erase(s.begin(), left);
    auto right = std::find_if(s.rbegin(), s.rend(), [](unsigned char c) { return !std::isspace(c); });
    s.erase(right.base(), s.end());
    return s;
}
} // namespace

struct Config {
    std::vector<std::string> originalIps;
    std::string redirectIp;
    uint16_t redirectPort = 0;

    bool Load(const std::string& path);
};

static Config& GetConfig() {
    static Config cfg;
    return cfg;
}

bool Config::Load(const std::string& path) {
    ms::ini::Parsed ini;
    if (!ms::ini::Parse(path, ini, [](const char* msg) { Log("%s", msg); }))
        return false;

    auto TakeLast = [&](const char* key) -> std::optional<std::string> {
        auto it = ini.entries.find(std::string("Main.") + key);
        if (it == ini.entries.end() || it->second.empty())
            return std::nullopt;
        if (it->second.size() > 1) {
            Log("INI: duplicate key Main.%s, using last value", key);
        }
        return it->second.back();
    };

    auto rip = TakeLast("RedirectIP");
    if (!rip) {
        Log("INI: missing required key Main.RedirectIP");
        return false;
    }
    if (inet_addr(rip->c_str()) == INADDR_NONE) {
        Log("INI: Main.RedirectIP is not a valid IPv4 address: %s", rip->c_str());
        return false;
    }
    redirectIp = *rip;

    auto rport = TakeLast("RedirectPort");
    if (!rport) {
        Log("INI: missing required key Main.RedirectPort");
        return false;
    }
    char* endp = nullptr;
    unsigned long n = std::strtoul(rport->c_str(), &endp, 10);
    if (!endp || *endp != '\0' || n == 0 || n > 65535) {
        Log("INI: Main.RedirectPort out of range or non-numeric: %s", rport->c_str());
        return false;
    }
    redirectPort = static_cast<uint16_t>(n);

    originalIps.clear();
    auto ips = ini.entries.find("Main.OriginalIPs");
    if (ips != ini.entries.end()) {
        for (auto& csv : ips->second) {
            std::stringstream ss(csv);
            std::string item;
            while (std::getline(ss, item, ',')) {
                std::string t = Trim(std::move(item));
                if (!t.empty())
                    originalIps.push_back(std::move(t));
            }
        }
    }
    // Pick up any Main.OriginalIPN keys (N >= 1, no upper bound).
    const std::string prefix = "Main.OriginalIP";
    for (auto& kv : ini.entries) {
        if (kv.first.size() <= prefix.size())
            continue;
        if (kv.first.compare(0, prefix.size(), prefix) != 0)
            continue;
        const std::string tail = kv.first.substr(prefix.size());
        if (tail.empty() || !std::all_of(tail.begin(), tail.end(), [](unsigned char c) { return std::isdigit(c); }))
            continue;
        for (auto& v : kv.second) {
            std::string t = Trim(v);
            if (!t.empty())
                originalIps.push_back(std::move(t));
        }
    }
    if (originalIps.empty()) {
        Log("INI: at least one OriginalIP (Main.OriginalIPs= or Main.OriginalIPN=) is required");
        return false;
    }

    return true;
}

INT WSPAPI WSPConnect_Hook(SOCKET s, const struct sockaddr *name, int namelen, LPWSABUF lpCallerData,
                           LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS, LPINT lpErrno) {
    char szAddr[50];
    DWORD dwLen = 50;
    WSAAddressToString((sockaddr *) name, namelen, nullptr, szAddr, &dwLen);

    auto *service = (sockaddr_in *) name;
    Config& cfg = GetConfig();

    Log("Detected socket connection to IP: %s", szAddr);
    for (auto& i : cfg.originalIps) {
        if (std::strncmp(szAddr, i.c_str(), i.size()) == 0) {
            Log("Detected and rerouting socket connection to IP: %s", cfg.redirectIp.c_str());
            service->sin_addr.S_un.S_addr = inet_addr(cfg.redirectIp.c_str());
            m_GameSock = s;
            break;
        }
    }

    u_short nPort = ntohs(service->sin_port);
    u_short defaultPort = 8484;
    if (nPort == defaultPort) {
        Log("Port Replaced: %d -> %d", defaultPort, cfg.redirectPort);
        service->sin_port = htons(cfg.redirectPort);
        m_GameSock = s;
    }

    return m_ProcTable.lpWSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS,
                                    lpGQOS, lpErrno);
}

INT WSPAPI WSPGetPeerName_Hook(SOCKET s, struct sockaddr *name, LPINT namelen, LPINT lpErrno) {
    int nRet = m_ProcTable.lpWSPGetPeerName(s, name, namelen, lpErrno);

    if (nRet == SOCKET_ERROR) {
        Log("WSPGetPeerName Socket Error: %d", nRet);
        return nRet;
    }

    char szAddr[50];
    DWORD dwLen = 50;
    WSAAddressToString((sockaddr *) name, *namelen, nullptr, szAddr, &dwLen);

    auto *service = (sockaddr_in *) name;
    Config& cfg = GetConfig();

    u_short nPort = ntohs(service->sin_port);

    if (s != m_GameSock) {
        Log("WSPGetPeerName => IP Ignored: %s:%d", szAddr, nPort);
        return nRet;
    }

    service->sin_addr.S_un.S_addr = inet_addr(cfg.redirectIp.c_str());
    Log("WSPGetPeerName => IP Replaced: %s -> %s", cfg.redirectIp.c_str(), szAddr);

    u_short defaultPort = 8484;
    if (nPort == defaultPort) {
        Log("WSPGetPeerName => Port Replaced: %d -> %d", defaultPort, cfg.redirectPort);
        service->sin_port = htons(cfg.redirectPort);
    }
    return nRet;
}

INT WSPAPI WSPCloseSocket_Hook(SOCKET s, LPINT lpErrno) {
    int nRet = m_ProcTable.lpWSPCloseSocket(s, lpErrno);

    if (s == m_GameSock) {
        Log("Socket closed by application.. (%d). CallAddr: %02x", nRet, _ReturnAddress());
        m_GameSock = INVALID_SOCKET;
    }

    return nRet;
}

INT WSPAPI WSPStartup_Hook(WORD wVersionRequested, LPWSPDATA lpWSPData, LPWSAPROTOCOL_INFOW lpProtocolInfo,
                           WSPUPCALLTABLE UpcallTable, LPWSPPROC_TABLE lpProcTable) {
    int nRet = WSPStartup_Original(wVersionRequested, lpWSPData, lpProtocolInfo, UpcallTable, lpProcTable);

    if (nRet != NO_ERROR) {
        Log("WSPStartup Error Code: %d", nRet);
        return nRet;
    }

    Log("Overriding socket routines..");
    m_GameSock = INVALID_SOCKET;
    m_ProcTable = *lpProcTable;
    lpProcTable->lpWSPConnect = WSPConnect_Hook;
    lpProcTable->lpWSPGetPeerName = WSPGetPeerName_Hook;
    lpProcTable->lpWSPCloseSocket = WSPCloseSocket_Hook;
    return nRet;
}

// main thread
DWORD WINAPI MainProc(LPVOID lpParam) {
    if (!GetConfig().Load("edits/redirect.ini")) {
        return -1;
    }

    INITWINHOOK_OR_RETURN("MSWSOCK", "WSPStartup", WSPStartup_Original, WSPStartup_t, WSPStartup_Hook);
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
        case DLL_PROCESS_DETACH: {
            if (m_GameSock != INVALID_SOCKET) {
                Log("Closing socket..");
                m_ProcTable.lpWSPCloseSocket(m_GameSock, nullptr);
                m_GameSock = INVALID_SOCKET;
            }
        }
    }
    return TRUE;
}