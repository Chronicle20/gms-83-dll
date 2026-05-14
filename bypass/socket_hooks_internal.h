#pragma once
#include <Windows.h>

class CClientSocket;

// Cross-TU forward declaration. CWvsApp::ConnectLogin_Hook (in app_hooks.cpp)
// calls this directly to mirror the pre-refactor inline call-graph.
// Defined in socket_hooks.cpp.
INT __fastcall CClientSocket__OnConnect_Hook(CClientSocket* pThis, PVOID edx, int bSuccess);
