#include "pch.h"

#include "host_globals.h"
#include "logger.h"

namespace custom_ui_host {
std::atomic<bool> g_ready{false};
std::atomic<bool> g_double_load{false};
} // namespace custom_ui_host

namespace {

bool AcquireSingletonMutex() {
    HANDLE h = CreateMutexW(nullptr, FALSE, L"Local\\custom-ui-host-singleton");
    if (!h) {
        Log("custom-ui-host: CreateMutexW failed err=%lu", GetLastError());
        return false;  // treat as double-load to be safe
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        Log("custom-ui-host: another host instance is already running -- "
            "this instance will be inert");
        return false;
    }
    return true;
}

} // namespace

DWORD WINAPI MainProc(LPVOID /*lpParam*/) {
    Log("custom-ui-host: MainProc start");

    if (!AcquireSingletonMutex()) {
        custom_ui_host::g_double_load.store(true);
        return 0;
    }

    // Hook installation lands in Phase 7. INI parsing lands in Task 3.3.
    // Vtable cloning lands in Phase 5. For now, just signal ready so the
    // demo's IsReady-poll smoke test passes once Phase 3 lands.
    custom_ui_host::g_ready.store(true);
    Log("custom-ui-host: ready");
    return 0;
}
