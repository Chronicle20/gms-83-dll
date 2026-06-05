#include "pch.h"

#include "runtime/vtable_patch.h"

#include "logger.h"
#include "memory_map.h"
#include "runtime/custom_ui_wnd.h"
#include "runtime/seh_dispatch.h"
#include "runtime/text_draw.h"
#include "runtime/ui_dispatch.h"

#include <cstring>
#include <new>

namespace custom_ui_host {

// Cloned CUIWnd vtable pointer. Null until InitCustomUIWndVtable() initialises it.
void* g_cloned_cuiwnd_vtable = nullptr;

namespace {

// Slot 8: CUIWnd::OnButtonClicked -- void __thiscall(this, UINT nControlId).
// The argument is the control id we assigned in CreateCtrl, not the control
// pointer. Dispatch by id to the owning ControlEntry's callback under SEH.
void __fastcall OnButtonClicked_Override(void* self, void* /*edx*/, unsigned int nControlId) {
    FrameworkExtras* fe = CustomUIWnd::ExtrasOf(self);
    if (!fe)
        return;
    for (ControlEntry& entry : fe->controls) {
        if (entry.id != nControlId)
            continue;
        if (entry.on_click) {
            SafeDispatch("CustomUI button click", [&] { entry.on_click(fe->handle, entry.id, entry.user); });
        }
        return;
    }
    // Unknown id: this is the stock close button (id 1000) built by
    // CUIWnd::OnCreate. We must NOT forward to stock CUIWnd::OnButtonClicked --
    // it closes by type id (CWvsContext::UI_Close(m_nUIType)), which targets
    // the real Equip window registered under nUIType=1, not our `this`.
    //
    // Hide our own window, but DEFER it to the next UI tick: we're re-entrant
    // inside the game's button-click dispatch, which is iterating the CWndMan
    // window list, so calling RemoveWindow synchronously here doesn't take
    // (the window keeps drawing). s_Update drains this queue outside that
    // iteration. (F8 works synchronously because it runs from ProcessKey, not
    // mid window-list iteration.)
    if (nControlId == 1000) {
        EnqueueUIThreadTask([](void* w) { reinterpret_cast<CustomUIWnd*>(w)->Hide(); }, self);
    }
}

// Slot 11: CWnd::Draw -- void __thiscall(this, const tagRECT* pClip). Paint the
// window's own background via the original Draw first (no-op if none), then draw
// our label/caption text on top.
void __fastcall Draw_Override(void* self, void* /*edx*/, const void* pClip) {
    reinterpret_cast<void(__fastcall*)(void*, void*, const void*)>(C_WND_DRAW)(self, nullptr, pClip);

    FrameworkExtras* fe = CustomUIWnd::ExtrasOf(self);
    if (!fe)
        return;
    // Text drawing creates a font + COM canvas and can throw (_com_error) or
    // AV if a graphics resource is unavailable. This runs inside the game's
    // render loop, so an unguarded fault would crash the client. Contain it.
    SafeDispatch("CustomUI draw labels", [self, fe] {
        for (const ControlEntry& entry : fe->controls) {
            if (!entry.text.empty()) {
                DrawLabel(self, entry.draw_x, entry.draw_y, entry.text.c_str());
            }
        }
    });
}

} // namespace

bool InitCustomUIWndVtable() {
    const std::size_t slots = C_UI_WND_VTABLE_SLOT_COUNT;
    if (slots == 0) {
        Log("custom-ui-host: CUIWnd vtable slot count is 0 -- cannot clone");
        return false;
    }

    void** clone = static_cast<void**>(::operator new(sizeof(void*) * slots, std::nothrow));
    if (!clone) {
        Log("custom-ui-host: failed to allocate cloned CUIWnd vtable");
        return false;
    }

    auto* base = reinterpret_cast<void**>(C_UI_WND_VFTABLE);
    std::memcpy(clone, base, sizeof(void*) * slots);

    clone[8] = reinterpret_cast<void*>(&OnButtonClicked_Override);
    clone[11] = reinterpret_cast<void*>(&Draw_Override);
    // Slot 13 (OnCreate) is left as the stock CUIWnd::OnCreate: we *call* it
    // (from Show, after CreateWnd) to load the WZ background + close button
    // rather than suppressing it. Overriding it would defeat that.
    Log("custom-ui-host: CUIWnd vtable cloned + overrides set");

    g_cloned_cuiwnd_vtable = clone;
    Log("custom-ui-host: cloned CUIWnd vtable ready (%zu slots; overrode 8/11)", slots);
    return true;
}

} // namespace custom_ui_host
