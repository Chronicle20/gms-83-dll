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
    // `msg` is NOT a usable down/up discriminator: CWndMan::TranslateMessage
    // normalizes it to 256 (WM_KEYDOWN) for BOTH key-down and key-up before
    // calling ProcessKey (and also masks lParam with 0x1FF0000, dropping the
    // bit-30 repeat flag). The real edge is lParam bit 31 -- 0 = down, 1 = up
    // -- which is exactly what ProcessKey itself tests via (a4 & 0x80000000).
    const bool keyDown = (lParam & 0x80000000L) == 0;

    // Per-vk rising-edge latch. ProcessKey is dispatched for the same event
    // from two sites (CWvsApp::ISMsgProc + CWndMan::TranslateMessage) and
    // repeats key-downs while held, so a raw keydown test would toggle several
    // times per press. Toggle only on the first down; the matching up re-arms.
    static bool s_held[256] = {false};

    if (!keyDown) {
        // Key-up: if we owned this press (latched on its down), clear the latch
        // and consume the up too so the game doesn't see a half event.
        if (vk < 256 && s_held[vk]) {
            s_held[vk] = false;
            return 1L;
        }
        return _ProcessKey(self, msg, vk, lParam);
    }

    if (g_hotkeys) {
        unsigned int mods = SnapshotModifiers();
        const auto* binding = g_hotkeys->Lookup(vk, mods);
        if (binding && g_windows) {
            auto* wnd = g_windows->Lookup(binding->target);
            if (wnd) {
                if (vk < 256 && !s_held[vk]) {
                    s_held[vk] = true; // rising edge -- act once
                    if (wnd->IsVisible())
                        wnd->Hide();
                    else
                        wnd->Show();
                }
                return 1L; // consume the key-down (and repeats) for a bound key
            }
            // Binding matched but its target window is gone (stale handle):
            // fall through to the game rather than silently swallowing the key.
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
