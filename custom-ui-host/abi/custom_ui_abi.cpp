#include "pch.h"

#include "abi/custom_ui_abi.h"
#include "abi/abi_globals.h"

#include "host_globals.h"
#include "runtime/host_config.h"
#include "runtime/custom_ui_wnd.h"

#include "logger.h"

#include <string>

namespace custom_ui_host {

std::unique_ptr<WindowRegistry> g_windows;
std::unique_ptr<HotkeyRegistry> g_hotkeys;
std::unique_ptr<PacketRegistry> g_packets;

void InitAbiGlobals() {
    g_windows = std::make_unique<WindowRegistry>();
    g_hotkeys = std::make_unique<HotkeyRegistry>();
    g_packets = std::make_unique<PacketRegistry>(g_config.inbound_op_min,
                                                  g_config.inbound_op_max);
}

namespace {

bool ReadyOrLog(const char *site) {
    if (custom_ui_host::g_double_load.load()) {
        Log("custom-ui-host: %s called on double-load instance -- ignoring", site);
        return false;
    }
    if (!custom_ui_host::g_ready.load()) {
        Log("custom-ui-host: %s called before ready -- ignoring", site);
        return false;
    }
    return true;
}

} // namespace
} // namespace custom_ui_host

extern "C" {

__declspec(dllexport) unsigned int __cdecl CustomUI_GetAbiVersion(void) {
    return 0x00010000u;
}

__declspec(dllexport) int __cdecl CustomUI_IsReady(void) {
    return custom_ui_host::g_ready.load() ? 1 : 0;
}

__declspec(dllexport) CustomUI_WindowHandle __cdecl
CustomUI_CreateWindow(const char *title, int x, int y, int w, int h,
                      void *user) {
    if (!custom_ui_host::ReadyOrLog("CreateWindow")) return 0;
    auto *wnd = custom_ui_host::CustomUIWnd::Create(x, y, w, h, title, user);
    if (!wnd) return 0;
    auto handle = custom_ui_host::g_windows->Register(wnd);
    wnd->Extras().handle = handle;
    return static_cast<CustomUI_WindowHandle>(handle);
}

__declspec(dllexport) int __cdecl
CustomUI_ShowWindow(CustomUI_WindowHandle h) {
    if (!custom_ui_host::ReadyOrLog("ShowWindow")) return 0;
    auto *wnd = custom_ui_host::g_windows->Lookup(static_cast<unsigned int>(h));
    if (!wnd) return 0;
    wnd->Show();
    return 1;
}

__declspec(dllexport) int __cdecl
CustomUI_HideWindow(CustomUI_WindowHandle h) {
    if (!custom_ui_host::ReadyOrLog("HideWindow")) return 0;
    auto *wnd = custom_ui_host::g_windows->Lookup(static_cast<unsigned int>(h));
    if (!wnd) return 0;
    wnd->Hide();
    return 1;
}

__declspec(dllexport) int __cdecl
CustomUI_DestroyWindow(CustomUI_WindowHandle h) {
    if (!custom_ui_host::ReadyOrLog("DestroyWindow")) return 0;
    auto *wnd = custom_ui_host::g_windows->Lookup(static_cast<unsigned int>(h));
    if (!wnd) return 0;
    custom_ui_host::g_windows->Unregister(static_cast<unsigned int>(h));
    custom_ui_host::CustomUIWnd::Destroy(wnd);
    return 1;
}

/* Stubs for controls/hotkeys/packets — implemented in 6.3+. */
__declspec(dllexport) CustomUI_CtrlId __cdecl
CustomUI_AddLabel(CustomUI_WindowHandle, int, int, const char *) { return 0; }
__declspec(dllexport) CustomUI_CtrlId __cdecl
CustomUI_AddButton(CustomUI_WindowHandle, int, int, int, int, const char *,
                   CustomUI_OnClickFn) { return 0; }
__declspec(dllexport) CustomUI_CtrlId __cdecl
CustomUI_AddEdit(CustomUI_WindowHandle, int, int, int, int, const char *) {
    return 0;
}
__declspec(dllexport) int __cdecl
CustomUI_SetLabelText(CustomUI_WindowHandle, CustomUI_CtrlId, const char *) {
    return 0;
}
__declspec(dllexport) int __cdecl
CustomUI_GetEditText(CustomUI_WindowHandle, CustomUI_CtrlId, char *, int) {
    return 0;
}
__declspec(dllexport) CustomUI_HotkeyId __cdecl
CustomUI_BindHotkey(unsigned int, unsigned int, CustomUI_WindowHandle) {
    return 0;
}
__declspec(dllexport) int __cdecl CustomUI_UnbindHotkey(CustomUI_HotkeyId) {
    return 0;
}
__declspec(dllexport) int __cdecl
CustomUI_SendPacket(unsigned short, const void *, unsigned int) { return 0; }
__declspec(dllexport) CustomUI_HandlerId __cdecl
CustomUI_RegisterPacketHandler(unsigned short, CustomUI_PacketHandlerFn,
                               void *) { return 0; }
__declspec(dllexport) int __cdecl
CustomUI_UnregisterPacketHandler(CustomUI_HandlerId) { return 0; }
__declspec(dllexport) void __cdecl CustomUI_DumpRegistries(void) {}

} // extern "C"
