#include "pch.h"

#include "runtime/vtable_patch.h"

#include "logger.h"
#include "memory_map.h"
#include "runtime/custom_ui_wnd.h"
#include "runtime/seh_dispatch.h"
#include "runtime/text_draw.h"

#include <cstring>
#include <new>

namespace custom_ui_host {

// Cloned CUIWnd vtable pointer. Null until InitCustomUIWndVtable() initialises it.
void* g_cloned_cuiwnd_vtable = nullptr;

namespace {

// Slot 13: CUIWnd::OnCreate -- int __thiscall(this, int unused, const wchar_t*).
// A no-op return-0 suppresses the stock WZ background load + close-button
// creation; our custom windows draw themselves and own their controls.
int __fastcall OnCreate_Override(void* /*self*/, void* /*edx*/, int /*unused*/, const unsigned short* /*arg*/) {
    return 0;
}

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
    // Unknown id: it's our window with no base button behaviour -- do nothing.
}

// Slot 11: CWnd::Draw -- void __thiscall(this, const tagRECT* pClip). Paint the
// window's own background via the original Draw first (no-op if none), then draw
// our label/caption text on top.
void __fastcall Draw_Override(void* self, void* /*edx*/, const void* pClip) {
    reinterpret_cast<void(__fastcall*)(void*, void*, const void*)>(C_WND_DRAW)(self, nullptr, pClip);

    FrameworkExtras* fe = CustomUIWnd::ExtrasOf(self);
    if (!fe)
        return;
    for (const ControlEntry& entry : fe->controls) {
        if (!entry.text.empty()) {
            DrawLabel(self, entry.draw_x, entry.draw_y, entry.text.c_str());
        }
    }
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
    clone[13] = reinterpret_cast<void*>(&OnCreate_Override);
    Log("custom-ui-host: CUIWnd vtable cloned + overrides set");

    g_cloned_cuiwnd_vtable = clone;
    Log("custom-ui-host: cloned CUIWnd vtable ready (%zu slots; overrode 8/11/13)", slots);
    return true;
}

} // namespace custom_ui_host
