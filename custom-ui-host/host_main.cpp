#include "pch.h"

#include "abi/abi_globals.h"
#include "hooks/debug_hooks.h"
#include "hooks/process_key_hook.h"
#include "hooks/process_packet_hook.h"
#include "hooks/s_update_hook.h"
#include "hooks/stage_dtor_hook.h"
#include "host_globals.h"
#include "logger.h"
#include "runtime/host_config.h"
#include "runtime/vtable_patch.h"

namespace custom_ui_host {
std::atomic<bool> g_ready{false};
std::atomic<bool> g_double_load{false};
} // namespace custom_ui_host

namespace {

bool AcquireSingletonMutex() {
    HANDLE h = CreateMutexW(nullptr, FALSE, L"Local\\custom-ui-host-singleton");
    if (!h) {
        Log("custom-ui-host: CreateMutexW failed err=%lu", GetLastError());
        return false; // treat as double-load to be safe
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
    Log("custom-ui-host: config loaded");

    // The cloned CUIWnd vtable must exist before any custom window is created
    // (CustomUIWnd::Create patches the vptr from g_cloned_cuiwnd_vtable).
    if (!custom_ui_host::InitCustomUIWndVtable()) {
        Log("custom-ui-host: vtable init failed -- staying inert");
        return 0;
    }
    Log("custom-ui-host: vtable ready");

    // Registries must exist before the host signals ready; InitAbiGlobals
    // reads g_config (packet range) so it must run after LoadHostConfig.
    custom_ui_host::InitAbiGlobals();
    Log("custom-ui-host: registries ready");

    // Detours hook installs. Each installer is fail-fast: on a Detours
    // rejection we log and stay inert (return 0) rather than run partially
    // hooked. Grouped together so the future H2 (CWndMan::ProcessKey, Task
    // 7.1) install can be inserted ahead of the others.
    if (!custom_ui_host::InstallProcessKeyHook()) {
        Log("custom-ui-host: ProcessKey hook install failed -- staying inert");
        return 0;
    }
    Log("custom-ui-host: ProcessKey hook installed");
    if (!custom_ui_host::InstallProcessPacketHook()) {
        Log("custom-ui-host: ProcessPacket hook install failed -- staying inert");
        return 0;
    }
    Log("custom-ui-host: ProcessPacket hook installed");
    if (!custom_ui_host::InstallStageDtorHook()) {
        Log("custom-ui-host: StageDtor hook install failed -- staying inert");
        return 0;
    }
    Log("custom-ui-host: StageDtor hook installed");
    if (!custom_ui_host::InstallSUpdateHook()) {
        Log("custom-ui-host: s_Update hook install failed -- staying inert");
        return 0;
    }
    Log("custom-ui-host: sUpdate hook installed");

    if (!custom_ui_host::InstallDebugHooks()) {
        Log("custom-ui-host: debug hooks install failed");
    }

    // Vtable cloning lands in Phase 5.
    custom_ui_host::g_ready.store(true);
    Log("custom-ui-host: ready");
    return 0;
}
