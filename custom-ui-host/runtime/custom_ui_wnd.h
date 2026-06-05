#pragma once

#include "registries/window_registry.h"

#include <cstdint>
#include <vector>

namespace custom_ui_host {

// Distinguishes control kinds for downstream button-click dispatch.
enum class ControlKind {
    Label,
    Button,
    Edit,
};

using CtrlId = std::uint32_t;

struct ControlEntry {
    CtrlId id;
    ControlKind kind;
    void *raw;  // pointer into the per-control byte buffer (CCtrlButton*, etc.)
    // Per-button vtable clone slot (per-instance approach). Unused for
    // Label/Edit.
    void *btn_vtable_clone = nullptr;
    void(__cdecl *on_click)(WindowHandle, CtrlId, void *) = nullptr;
    void *user = nullptr;
};

struct FrameworkExtras {
    WindowHandle handle = 0;
    void *user = nullptr;
    std::vector<ControlEntry> controls;
    // Desired screen rectangle, applied by CWnd::CreateWnd on first Show
    // (Task 5.4). Stored here at Create time.
    int x = 0, y = 0, w = 0, h = 0;
    bool layer_created = false;  // CreateWnd has run at least once
    bool was_visible = false;
    bool is_visible = false;
};

// CustomUIWnd is *not* a normal C++ class — it's a raw game-side CUIWnd
// constructed in place in a host-owned byte buffer, followed by a
// FrameworkExtras block. The game vftable pointer lives at buf[0..3];
// Task 5.3 may replace it with a cloned vtable. Create returns nullptr on
// allocation failure.
struct CustomUIWnd {
    static CustomUIWnd *Create(int x, int y, int w, int h, const char *titleUtf8,
                               void *user);
    static void Destroy(CustomUIWnd *self);

    // Visibility — wired in Task 5.4 (CreateWnd / CWndMan register/remove).
    void Show();
    void Hide();
    bool IsVisible() const;

    // Recover the FrameworkExtras for a given `this` pointer of a placed
    // CUIWnd. Used inside cloned-vtable trampolines (Task 5.3).
    static FrameworkExtras *ExtrasOf(void *cuiwnd_self);

    FrameworkExtras &Extras();
    void *GameWnd();  // returns pointer to the placed CUIWnd (== this buffer)
};

} // namespace custom_ui_host
