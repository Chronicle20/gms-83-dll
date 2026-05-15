#pragma once
#include <Windows.h>

// Installs every CClientSocket::* hook in the bypass edit. Returns FALSE on
// any Detours install failure (matching pre-refactor MainProc semantics:
// the first failure short-circuits the whole installation chain).
BOOL InstallSocketHooks();
