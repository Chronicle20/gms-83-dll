#include "pch.h"

#include "runtime/custom_ui_wnd.h"

#include "abi/abi_globals.h"
#include "logger.h"
#include "memory_map.h"
#include "runtime/seh_dispatch.h"

#include <cstring>
#include <new>

namespace custom_ui_host {

// Provided by runtime/vtable_patch.cpp (Task 5.3). Null until that lands.
extern void* g_cloned_cuiwnd_vtable;

namespace {

constexpr std::size_t kCUIWndSize = SIZEOF_C_UI_WND_V83_1;
constexpr std::size_t kBufSize = kCUIWndSize + sizeof(FrameworkExtras);

} // namespace

FrameworkExtras* CustomUIWnd::ExtrasOf(void* cuiwnd_self) {
    auto* p = static_cast<unsigned char*>(cuiwnd_self);
    return reinterpret_cast<FrameworkExtras*>(p + kCUIWndSize);
}

CustomUIWnd* CustomUIWnd::Create(int x, int y, int w, int h, const char* /*titleUtf8*/, void* user) {
    auto* buf = static_cast<unsigned char*>(::operator new(kBufSize, std::nothrow));
    if (!buf)
        return nullptr;
    std::memset(buf, 0, kBufSize);

    // Construct the game-side CUIWnd in place via the INT-ONLY ctor (the
    // string ctor 0x0092C17F dereferences its UOL arg and would crash on
    // null). We mirror CUIEquip's exact ctor args so the stock
    // CUIWnd::OnCreate path (driven later from Show) loads a real, always-
    // present WZ panel: nUIType=1 -> "Equip" (sub_92C61C), closeType=3 ->
    // UI/Basic.img/BtClose close button, closeX/closeY = Equip's close-button
    // offsets, nBackgrnd=1 -> OnCreate's background block is enabled
    // (gated on [this+0x598] != 0). The ctor writes the game vftable at buf[0..3].
    reinterpret_cast<void(__fastcall*)(void*, void*, int, int, int, int, int, int, int)>(C_UI_WND_CTOR_INT)(
        buf, nullptr, /*nUIType*/ 1, /*closeType*/ 3, /*closeX*/ 155,
        /*closeY*/ 6, /*nBackgrnd*/ 1, /*nBackgrndX*/ 0, /*nBackgrndY*/ 0);

    // If the cloned CUIWnd vtable is available (Task 5.3), patch the vptr to
    // it. If not yet initialised, leave the stock game vftable in place —
    // the window still works, just without our slot overrides.
    if (g_cloned_cuiwnd_vtable) {
        *reinterpret_cast<void**>(buf) = g_cloned_cuiwnd_vtable;
    }

    // Construct FrameworkExtras after the CUIWnd block; stash geometry + user.
    auto* fe = new (buf + kCUIWndSize) FrameworkExtras();
    fe->user = user;
    fe->x = x;
    fe->y = y;
    fe->w = w;
    fe->h = h;

    return reinterpret_cast<CustomUIWnd*>(buf);
}

void CustomUIWnd::Destroy(CustomUIWnd* self) {
    if (!self)
        return;
    auto* buf = reinterpret_cast<unsigned char*>(self);
    auto* fe = reinterpret_cast<FrameworkExtras*>(buf + kCUIWndSize);

    if (fe->is_visible)
        self->Hide(); // RemoveWindow (Task 5.4)

    // Free per-control byte buffers + any per-button vtable clones.
    for (auto& ctrl : fe->controls) {
        if (ctrl.btn_vtable_clone) {
            ::operator delete(ctrl.btn_vtable_clone);
            ctrl.btn_vtable_clone = nullptr;
        }
        if (ctrl.raw) {
            ::operator delete(ctrl.raw);
            ctrl.raw = nullptr;
        }
    }

    // Run the plain (non-deleting) game-side dtor on the buffer. We own the
    // buffer via ::operator new, so we must NOT use the scalar-deleting dtor
    // (0x0092C15B) which would try to free via the game allocator.
    reinterpret_cast<void(__fastcall*)(void*, void*)>(C_UI_WND_DTOR)(buf, nullptr);

    fe->~FrameworkExtras();
    ::operator delete(buf);
}

FrameworkExtras& CustomUIWnd::Extras() {
    auto* buf = reinterpret_cast<unsigned char*>(this);
    return *reinterpret_cast<FrameworkExtras*>(buf + kCUIWndSize);
}

void* CustomUIWnd::GameWnd() {
    return reinterpret_cast<unsigned char*>(this);
}

void CustomUIWnd::Show() {
    auto& fe = Extras();
    if (fe.is_visible)
        return;
    void* buf = GameWnd();
    if (!fe.layer_created) {
        // First show: CreateWnd builds the sized layer at (x,y) of (w,h),
        // stores geometry, and registers the window for display.
        reinterpret_cast<void(__fastcall*)(void*, void*, int, int, int, int, int, int, int, int)>(C_WND_CREATE_WND)(
            buf, nullptr, fe.x, fe.y, fe.w, fe.h,
            /*z*/ 10, /*bScreenCoord*/ 1, /*pData*/ 0,
            /*bSetFocus*/ 1);
        fe.layer_created = true;

        // Now that the layer exists, drive the stock CUIWnd::OnCreate exactly
        // as a real window's OnCreate does (cf. sub_809CCC: it calls the base
        // OnCreate with an empty UOL). With nUIType=1/nBackgrnd=1 this loads
        // "UI/UIWindow.img/Equip/backgrnd" into m_pBackgrnd (blitted by the
        // stock CWnd::Draw our Draw override calls first) and builds the
        // close button via the game's own CreateCtrl. CreateCtrl can throw a
        // _com_error if a WZ node is unexpectedly missing; the background is
        // loaded *before* the close button, so SafeDispatch keeps a button
        // failure from losing the already-loaded background. Run once.
        if (!fe.on_create_done) {
            SafeDispatch("CustomUI OnCreate", [buf] {
                reinterpret_cast<void(__fastcall*)(void*, void*, void*, const unsigned short*)>(C_UI_WND_ON_CREATE)(
                    buf, nullptr, /*pData*/ nullptr, /*sBackgrndUOL*/ nullptr);
            });
            fe.on_create_done = true;
        }
    } else {
        // Re-show: layer already exists; re-add to the display z-list.
        reinterpret_cast<void(__cdecl*)(void*)>(C_WND_MAN_REGISTER_UI_WINDOW)(buf);
    }
    fe.is_visible = true;
}

void CustomUIWnd::Hide() {
    auto& fe = Extras();
    if (!fe.is_visible)
        return;
    // Remove from the display z-list; the layer/object persists for re-show.
    reinterpret_cast<void(__cdecl*)(void*)>(C_WND_MAN_UNREGISTER_UI_WINDOW)(GameWnd());
    fe.is_visible = false;
}

bool CustomUIWnd::IsVisible() const {
    return const_cast<CustomUIWnd*>(this)->Extras().is_visible;
}

void SnapshotAndSuspendVisibleWindows() {
    if (!g_windows)
        return;
    g_windows->ForEach([](CustomUIWnd* wnd) {
        auto& fe = wnd->Extras();
        fe.was_visible = fe.is_visible;
        if (fe.is_visible)
            wnd->Hide();
    });
}

void RestoreSuspendedWindows() {
    if (!g_windows)
        return;
    g_windows->ForEach([](CustomUIWnd* wnd) {
        auto& fe = wnd->Extras();
        if (fe.was_visible) {
            wnd->Show();
            fe.was_visible = false;
        }
    });
}

} // namespace custom_ui_host
