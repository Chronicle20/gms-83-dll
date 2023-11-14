
// Exclude rarely-used stuff from Windows headers
// Important to define this before Windows.h is included in a project because of linker issues with the WinSock2 lib
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include "logger.h"
#include "hooker.h"
#include "winhooks.h"
#include "winhook_types.h"
#include <fstream>
#include <string>
#include <map>
#include <vector>

SOCKET m_GameSock = INVALID_SOCKET;
WSPPROC_TABLE m_ProcTable = {nullptr};

std::vector<std::string> originalIps;
std::string redirectIp;

std::map<std::string, std::string> parseINI(const std::string &filePath) {
    std::map<std::string, std::string> iniData;

    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
        Log("Failed to open INI file: %s", filePath.c_str());
        return iniData;
    }

    std::string line;
    std::string currentSection;
    while (std::getline(inputFile, line)) {
        if (line.empty()) continue;

        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.size() - 2);
        } else {
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                iniData[currentSection + "." + key] = value;
            }
        }
    }

    inputFile.close();
    return iniData;
}

INT WSPAPI WSPConnect_Hook(SOCKET s, const struct sockaddr *name, int namelen, LPWSABUF lpCallerData,
                           LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS, LPINT lpErrno) {
    char szAddr[50];
    DWORD dwLen = 50;
    WSAAddressToString((sockaddr *) name, namelen, nullptr, szAddr, &dwLen);

    auto *service = (sockaddr_in *) name;

    Log("Detected socket connection to IP: %s", szAddr);
    for (auto &i: originalIps) {
        if (std::strncmp(szAddr, i.c_str(), i.size()) == 0) {
            Log("Detected and rerouting socket connection to IP: %s", redirectIp.c_str());
            service->sin_addr.S_un.S_addr = inet_addr(redirectIp.c_str());
            m_GameSock = s;
            break;
        }
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

    u_short nPort = ntohs(service->sin_port);

    if (s != m_GameSock) {
        Log("WSPGetPeerName => IP Ignored: %s:%d", szAddr, nPort);
        return nRet;
    }

    service->sin_addr.S_un.S_addr = inet_addr(redirectIp.c_str());

    Log("WSPGetPeerName => IP Replaced: %s -> %s on port %d", redirectIp.c_str(), szAddr, nPort);
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
VOID __stdcall MainProc() {
    std::map<std::string, std::string> iniData = parseINI("edits/redirect.ini");
    if (iniData.empty()) {
        return;
    }

    originalIps.push_back(iniData["Main.OriginalIP1"]);
    originalIps.push_back(iniData["Main.OriginalIP2"]);
    originalIps.push_back(iniData["Main.OriginalIP3"]);
    redirectIp = iniData["Main.RedirectIP"];

    INITWINHOOK("MSWSOCK", "WSPStartup", WSPStartup_Original, WSPStartup_t, WSPStartup_Hook);
}

// dll entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            DisableThreadLibraryCalls(hModule);
            CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE) &MainProc, nullptr, 0, nullptr);
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