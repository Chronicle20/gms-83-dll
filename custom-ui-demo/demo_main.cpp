#include "pch.h"

#include "demo_main.h"

#include "logger.h"
#include "parse_ini.h"

#include <string>

namespace custom_ui_demo {

ResolvedAbi g_abi{};
CustomUI_WindowHandle g_window = 0;
CustomUI_CtrlId g_label = 0;
int g_ping_count = 0;

namespace {

struct DemoConfig {
    int vk = 0x77; // VK_F8
    unsigned int mods = 0;
    int x = 100, y = 100;
};

DemoConfig LoadConfig() {
    DemoConfig c;
    HMODULE self = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       reinterpret_cast<LPCWSTR>(&LoadConfig), &self);
    wchar_t path[MAX_PATH] = {};
    if (!self || !GetModuleFileNameW(self, path, MAX_PATH))
        return c;
    std::wstring wpath = path;
    auto slash = wpath.find_last_of(L"\\/");
    if (slash == std::wstring::npos)
        return c;
    wpath.erase(slash + 1);
    wpath += L"custom-ui-demo.ini";
    std::string apath(wpath.begin(), wpath.end());
    ms::ini::Parsed p;
    if (!ms::ini::Parse(apath, p))
        return c;
    auto find = [&](const char* k, const std::string& def) -> std::string {
        auto it = p.entries.find(k);
        if (it == p.entries.end() || it->second.empty())
            return def;
        return it->second.front();
    };
    try {
        c.vk = std::stoi(find("demo.toggle_hotkey_vk", "119"));
    } catch (...) {
    }
    if (find("demo.toggle_hotkey_shift", "false") == "true")
        c.mods |= CUSTOM_UI_MOD_SHIFT;
    if (find("demo.toggle_hotkey_ctrl", "false") == "true")
        c.mods |= CUSTOM_UI_MOD_CTRL;
    if (find("demo.toggle_hotkey_alt", "false") == "true")
        c.mods |= CUSTOM_UI_MOD_ALT;
    return c;
}

template <class Fn> bool Resolve(HMODULE host, const char* name, Fn& out) {
    auto p = GetProcAddress(host, name);
    if (!p) {
        Log("custom-ui-demo: GetProcAddress(%s) failed", name);
        return false;
    }
    out = reinterpret_cast<Fn>(p);
    return true;
}

bool ResolveAbi(HMODULE host) {
    return Resolve(host, "CustomUI_GetAbiVersion", g_abi.GetAbiVersion) &&
           Resolve(host, "CustomUI_IsReady", g_abi.IsReady) &&
           Resolve(host, "CustomUI_RunOnUIThread", g_abi.RunOnUIThread) &&
           Resolve(host, "CustomUI_CreateWindow", g_abi.CreateWindow) &&
           Resolve(host, "CustomUI_ShowWindow", g_abi.ShowWindow) &&
           Resolve(host, "CustomUI_HideWindow", g_abi.HideWindow) &&
           Resolve(host, "CustomUI_DestroyWindow", g_abi.DestroyWindow) &&
           Resolve(host, "CustomUI_AddLabel", g_abi.AddLabel) && Resolve(host, "CustomUI_AddButton", g_abi.AddButton) &&
           Resolve(host, "CustomUI_SetLabelText", g_abi.SetLabelText) &&
           Resolve(host, "CustomUI_BindHotkey", g_abi.BindHotkey) &&
           Resolve(host, "CustomUI_SendPacket", g_abi.SendPacket) &&
           Resolve(host, "CustomUI_RegisterPacketHandler", g_abi.RegisterPacketHandler);
}

} // namespace

void __cdecl BuildUI(void* /*user*/) {
    Log("custom-ui-demo: BuildUI on UI thread");

    auto cfg = LoadConfig();

    Log("custom-ui-demo: CreateWindow...");
    g_window = g_abi.CreateWindow("Demo", cfg.x, cfg.y, 240, 80, nullptr);
    if (!g_window) {
        Log("custom-ui-demo: CreateWindow failed");
        return;
    }
    Log("custom-ui-demo: CreateWindow -> handle=%d", (int)g_window);
    Log("custom-ui-demo: AddLabel...");
    g_label = g_abi.AddLabel(g_window, 10, 10, "Server says: ?");
    Log("custom-ui-demo: AddLabel -> %d", (int)g_label);
    Log("custom-ui-demo: BindHotkey...");
    auto hk = g_abi.BindHotkey(cfg.vk, cfg.mods, g_window);
    Log("custom-ui-demo: BindHotkey -> %d", (int)hk);
    if (!hk)
        Log("custom-ui-demo: BindHotkey rejected (vk=0x%02X)", cfg.vk);
    Log("custom-ui-demo: RegisterPacketHandler...");
    g_abi.RegisterPacketHandler(0x2000, &OnPong, nullptr);
    Log("custom-ui-demo: RegisterPacketHandler done");
    Log("custom-ui-demo: window built, ready");
    // AddButton LAST: it loads a fragile WZ image and may AV inside the game's
    // CCtrlButton::CreateCtrl. The outer UI-thread SafeDispatch contains the
    // fault, and by this point the window/label/hotkey/packet handler are all
    // already set up, so F8/window/label/packets keep working even if it fails.
    Log("custom-ui-demo: AddButton...");
    g_abi.AddButton(g_window, 10, 40, 60, 20, "Ping", &OnPing);
    Log("custom-ui-demo: AddButton done");
}

} // namespace custom_ui_demo

DWORD WINAPI MainProc(LPVOID /*lpParam*/) {
    Log("custom-ui-demo: MainProc start");

    HMODULE host = LoadLibraryW(L"custom-ui-host.dll");
    if (!host) {
        Log("custom-ui-demo: host DLL not loaded -- demo inert");
        return 0;
    }
    if (!custom_ui_demo::ResolveAbi(host)) {
        Log("custom-ui-demo: ABI resolve failed -- demo inert");
        return 0;
    }
    unsigned int abi = custom_ui_demo::g_abi.GetAbiVersion();
    if ((abi >> 16) != 1) {
        Log("custom-ui-demo: ABI major mismatch (got 0x%08X) -- demo inert", abi);
        return 0;
    }
    while (!custom_ui_demo::g_abi.IsReady())
        Sleep(50);

    Log("custom-ui-demo: deferring UI build to game UI thread");
    custom_ui_demo::g_abi.RunOnUIThread(&custom_ui_demo::BuildUI, nullptr);
    Log("custom-ui-demo: MainProc done (UI build queued)");
    return 0;
}
