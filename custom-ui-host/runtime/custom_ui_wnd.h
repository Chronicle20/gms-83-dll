#pragma once

#include "registries/window_registry.h"

#include <cstdint>
#include <string>
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
    void* raw; // pointer into the per-control byte buffer (CCtrlButton*, etc.)
    // Per-button vtable clone slot (per-instance approach). Unused now that
    // click dispatch is id-based via the window's cloned slot-8 override; left
    // in place so existing Destroy() teardown stays valid.
    void* btn_vtable_clone = nullptr;
    void(__cdecl* on_click)(WindowHandle, CtrlId, void*) = nullptr;
    void* user = nullptr;
    // Drawn text. For a Label this is the label caption; for a Button it is an
    // optional caption painted on top of the (image-only) button. Empty means
    // nothing is drawn. Coordinates are window-relative (window origin = 0,0).
    std::string text; // UTF-8
    int draw_x = 0;
    int draw_y = 0;
};

struct FrameworkExtras {
    WindowHandle handle = 0;
    void* user = nullptr;
    std::vector<ControlEntry> controls;
    // Per-window monotonic control-id allocator. ids are echoed back to the
    // window's slot-8 OnButtonClicked override for dispatch; start at 1 so 0
    // remains the "invalid id" sentinel returned on failure.
    CtrlId next_ctrl_id = 1;
    // Desired screen rectangle, applied by CWnd::CreateWnd on first Show
    // (Task 5.4). Stored here at Create time.
    int x = 0, y = 0, w = 0, h = 0;
    bool layer_created = false; // CreateWnd has run at least once
    bool was_visible = false;
    bool is_visible = false;
};

// CustomUIWnd is *not* a normal C++ class — it's a raw game-side CUIWnd
// constructed in place in a host-owned byte buffer, followed by a
// FrameworkExtras block. The game vftable pointer lives at buf[0..3];
// Task 5.3 may replace it with a cloned vtable. Create returns nullptr on
// allocation failure.
struct CustomUIWnd {
    static CustomUIWnd* Create(int x, int y, int w, int h, const char* titleUtf8, void* user);
    static void Destroy(CustomUIWnd* self);

    // Visibility — wired in Task 5.4 (CreateWnd / CWndMan register/remove).
    void Show();
    void Hide();
    bool IsVisible() const;

    // Recover the FrameworkExtras for a given `this` pointer of a placed
    // CUIWnd. Used inside cloned-vtable trampolines (Task 5.3).
    static FrameworkExtras* ExtrasOf(void* cuiwnd_self);

    FrameworkExtras& Extras();
    void* GameWnd(); // returns pointer to the placed CUIWnd (== this buffer)
};

// Walks all registered windows; for each visible one, records the
// visibility in FrameworkExtras::was_visible and calls Hide().
void SnapshotAndSuspendVisibleWindows();

// For each window where was_visible is true, calls Show() and clears the
// flag.
void RestoreSuspendedWindows();

} // namespace custom_ui_host
