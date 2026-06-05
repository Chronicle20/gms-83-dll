#include "pch.h"

#include "abi/abi_globals.h"
#include "host_globals.h"
#include "hooks/process_packet_hook.h"
#include "logger.h"
#include "runtime/host_config.h"

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

    custom_ui_host::LoadHostConfig();

    // Registries must exist before the host signals ready; InitAbiGlobals
    // reads g_config (packet range) so it must run after LoadHostConfig.
    custom_ui_host::InitAbiGlobals();

    // Detours hook installs. Each installer is fail-fast: on a Detours
    // rejection we log and stay inert (return 0) rather than run partially
    // hooked. Grouped together so the future H2 (CWndMan::ProcessKey, Task
    // 7.1) install can be inserted ahead of the others.
    // H2 (CWndMan::ProcessKey) install is added in Task 7.1 (pending) -- here.
    if (!custom_ui_host::InstallProcessPacketHook()) {
        Log("custom-ui-host: ProcessPacket hook install failed -- staying inert");
        return 0;
    }

    // Vtable cloning lands in Phase 5.
    custom_ui_host::g_ready.store(true);
    Log("custom-ui-host: ready");
    return 0;
}
