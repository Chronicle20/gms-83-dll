#include "pch.h"

#include "runtime/custom_ui_wnd.h"

#include "abi/abi_globals.h"
#include "logger.h"
#include "memory_map.h"

#include <cstring>
#include <new>

namespace custom_ui_host {

// Provided by runtime/vtable_patch.cpp (Task 5.3). Null until that lands.
extern void *g_cloned_cuiwnd_vtable;

namespace {

constexpr std::size_t kCUIWndSize = SIZEOF_C_UI_WND_V83_1;
constexpr std::size_t kBufSize = kCUIWndSize + sizeof(FrameworkExtras);

}  // namespace

FrameworkExtras *CustomUIWnd::ExtrasOf(void *cuiwnd_self) {
    auto *p = static_cast<unsigned char *>(cuiwnd_self);
    return reinterpret_cast<FrameworkExtras *>(p + kCUIWndSize);
}

CustomUIWnd *CustomUIWnd::Create(int x, int y, int w, int h,
                                 const char * /*titleUtf8*/, void *user) {
    auto *buf = static_cast<unsigned char *>(::operator new(kBufSize, std::nothrow));
    if (!buf) return nullptr;
    std::memset(buf, 0, kBufSize);

    // Construct the game-side CUIWnd in place via the INT-ONLY ctor (no WZ
    // background, no null-deref). nUIType=1 (benign existing slot),
    // closeType=0 (no close button), remaining args 0. The ctor writes the
    // game's vftable pointer at buf[0..3].
    reinterpret_cast<void(__fastcall *)(void *, void *, int, int, int, int, int,
                                        int, int)>(C_UI_WND_CTOR_INT)(
        buf, nullptr, /*nUIType*/ 1, /*closeType*/ 0, /*closeX*/ 0,
        /*closeY*/ 0, /*nBackgrnd*/ 0, /*nBackgrndX*/ 0, /*nBackgrndY*/ 0);

    // If the cloned CUIWnd vtable is available (Task 5.3), patch the vptr to
    // it. If not yet initialised, leave the stock game vftable in place —
    // the window still works, just without our slot overrides.
    if (g_cloned_cuiwnd_vtable) {
        *reinterpret_cast<void **>(buf) = g_cloned_cuiwnd_vtable;
    }

    // Construct FrameworkExtras after the CUIWnd block; stash geometry + user.
    auto *fe = new (buf + kCUIWndSize) FrameworkExtras();
    fe->user = user;
    fe->x = x;
    fe->y = y;
    fe->w = w;
    fe->h = h;

    return reinterpret_cast<CustomUIWnd *>(buf);
}

void CustomUIWnd::Destroy(CustomUIWnd *self) {
    if (!self) return;
    auto *buf = reinterpret_cast<unsigned char *>(self);
    auto *fe = reinterpret_cast<FrameworkExtras *>(buf + kCUIWndSize);

    if (fe->is_visible) self->Hide();  // RemoveWindow (Task 5.4)

    // Free per-control byte buffers + any per-button vtable clones.
    for (auto &ctrl : fe->controls) {
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
    reinterpret_cast<void(__fastcall *)(void *, void *)>(C_UI_WND_DTOR)(buf,
                                                                        nullptr);

    fe->~FrameworkExtras();
    ::operator delete(buf);
}

FrameworkExtras &CustomUIWnd::Extras() {
    auto *buf = reinterpret_cast<unsigned char *>(this);
    return *reinterpret_cast<FrameworkExtras *>(buf + kCUIWndSize);
}

void *CustomUIWnd::GameWnd() { return reinterpret_cast<unsigned char *>(this); }

void CustomUIWnd::Show() {
    auto &fe = Extras();
    if (fe.is_visible) return;
    void *buf = GameWnd();
    if (!fe.layer_created) {
        // First show: CreateWnd builds the sized layer at (x,y) of (w,h),
        // stores geometry, and registers the window for display.
        reinterpret_cast<void(__fastcall *)(void *, void *, int, int, int, int,
                                            int, int, int, int)>(
            C_WND_CREATE_WND)(buf, nullptr, fe.x, fe.y, fe.w, fe.h,
                              /*z*/ 10, /*bScreenCoord*/ 1, /*pData*/ 0,
                              /*bSetFocus*/ 1);
        fe.layer_created = true;
    } else {
        // Re-show: layer already exists; re-add to the display z-list.
        reinterpret_cast<void(__cdecl *)(void *)>(C_WND_MAN_REGISTER_UI_WINDOW)(
            buf);
    }
    fe.is_visible = true;
}

void CustomUIWnd::Hide() {
    auto &fe = Extras();
    if (!fe.is_visible) return;
    // Remove from the display z-list; the layer/object persists for re-show.
    reinterpret_cast<void(__cdecl *)(void *)>(C_WND_MAN_UNREGISTER_UI_WINDOW)(
        GameWnd());
    fe.is_visible = false;
}

bool CustomUIWnd::IsVisible() const {
    return const_cast<CustomUIWnd *>(this)->Extras().is_visible;
}

void SnapshotAndSuspendVisibleWindows() {
    if (!g_windows) return;
    g_windows->ForEach([](CustomUIWnd *wnd) {
        auto &fe = wnd->Extras();
        fe.was_visible = fe.is_visible;
        if (fe.is_visible) wnd->Hide();
    });
}

void RestoreSuspendedWindows() {
    if (!g_windows) return;
    g_windows->ForEach([](CustomUIWnd *wnd) {
        auto &fe = wnd->Extras();
        if (fe.was_visible) {
            wnd->Show();
            fe.was_visible = false;
        }
    });
}

}  // namespace custom_ui_host
