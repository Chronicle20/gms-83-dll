#include "pch.h"

#include "hooks/process_key_hook.h"

#include "abi/abi_globals.h"
#include "abi/custom_ui_abi.h"
#include "runtime/custom_ui_wnd.h"

#include "hooker.h"
#include "logger.h"
#include "memory_map.h"

namespace custom_ui_host {

namespace {

typedef long(__thiscall* ProcessKey_t)(CWndMan*, unsigned int, unsigned int, long);
ProcessKey_t _ProcessKey = nullptr;

unsigned int SnapshotModifiers() {
    unsigned int m = 0;
    if (GetKeyState(VK_SHIFT) & 0x8000)
        m |= CUSTOM_UI_MOD_SHIFT;
    if (GetKeyState(VK_CONTROL) & 0x8000)
        m |= CUSTOM_UI_MOD_CTRL;
    if (GetKeyState(VK_MENU) & 0x8000)
        m |= CUSTOM_UI_MOD_ALT;
    return m;
}

long __fastcall ProcessKey_Hook(CWndMan* self, void* /*edx*/, unsigned int msg, unsigned int vk, long lParam) {
    static bool s_logged = false;
    if (!s_logged) {
        s_logged = true;
        Log("custom-ui-host: ProcessKey hook first fire");
    }
    // Only rising-edge WM_KEYDOWN / WM_SYSKEYDOWN. lParam bit 30 == 1
    // means the previous key state was down (auto-repeat).
    if ((msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) && (lParam & (1L << 30)) == 0) {
        if (g_hotkeys) {
            unsigned int mods = SnapshotModifiers();
            const auto* binding = g_hotkeys->Lookup(vk, mods);
            if (binding && g_windows) {
                auto* wnd = g_windows->Lookup(binding->target);
                if (wnd) {
                    // CWndMan::ProcessKey is invoked for the SAME WM_KEYDOWN
                    // from two call sites (CWvsApp::ISMsgProc and
                    // CWndMan::TranslateMessage), and ProcessKey never sees
                    // key-up. Without deduping, one physical press toggles the
                    // window twice (show then immediately hide). The duplicate
                    // dispatch lands in the same pump turn (sub-millisecond
                    // apart); debounce per-vk over a window far shorter than any
                    // intentional re-press.
                    static DWORD s_lastToggleTick[256] = {0};
                    bool duplicate = false;
                    if (vk < 256) {
                        DWORD now = GetTickCount();
                        if (now - s_lastToggleTick[vk] < 50)
                            duplicate = true;
                        else
                            s_lastToggleTick[vk] = now;
                    }
                    if (!duplicate) {
                        if (wnd->IsVisible())
                            wnd->Hide();
                        else
                            wnd->Show();
                    }
                    return 1L; // consumed -- only when a target was acted on
                }
                // Binding matched but its target window is gone (stale
                // handle): fall through to the game rather than silently
                // swallowing the key.
            }
        }
    }
    return _ProcessKey(self, msg, vk, lParam);
}

} // namespace

BOOL InstallProcessKeyHook() {
    INITMAPLEHOOK_OR_RETURN(_ProcessKey, ProcessKey_t, &ProcessKey_Hook, C_WND_MAN_PROCESS_KEY);
    return TRUE;
}

} // namespace custom_ui_host
