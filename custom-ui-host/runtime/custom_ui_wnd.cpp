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

// CWnd::m_pLayer.m_pInterface byte offset (confirmed: CWnd::CreateWnd reads
// [this+0x18]; CWndMan::RegisterUIWindow reads *(wnd+24)).
constexpr std::size_t kLayerInterfaceOffset = 24;
// IWzGr2DLayer vtable byte offset 0x11C (slot 71) = put_visible(BOOL).
// Evidence: CMapLoadable::SetLayerListVisible @0x644ac5 calls
// (*(*layer + 284))(layer, bVisible) on each layer. RemoveWindow only
// delists from CWndMan and never hides the layer, so a hidden window keeps
// compositing its last-blitted content unless we toggle this explicitly.
constexpr std::size_t kLayerPutVisibleSlot = 71;

// Toggle the window layer's COM visibility. COM methods here are __stdcall
// with `this` as the first stack arg (cf. CreateWnd's push this; call [vtbl+N]).
void SetLayerVisible(void* buf, int visible) {
    void* layer = *reinterpret_cast<void**>(static_cast<unsigned char*>(buf) + kLayerInterfaceOffset);
    if (!layer)
        return;
    void** vtbl = *reinterpret_cast<void***>(layer);
    reinterpret_cast<long(__stdcall*)(void*, int)>(vtbl[kLayerPutVisibleSlot])(layer, visible);
}

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
    // string ctor 0x0092C17F dereferences its UOL arg and would crash on null).
    // FREE-FORM dialog: nBackgrnd=0 so OnCreate's background block is skipped
    // (no WZ panel) -- we paint our own frame in the Draw override. closeType=3
    // still builds the stock UI/Basic.img/BtClose button; closeX/closeY place it
    // at the dialog's top-right (inside our border). nUIType=0 is benign (with
    // nBackgrnd=0 it drives no WZ lookup; geometry is set explicitly via
    // CreateWnd). The ctor writes the game vftable at buf[0..3].
    const int closeX = w > 22 ? w - 18 : 4;
    reinterpret_cast<void(__fastcall*)(void*, void*, int, int, int, int, int, int, int)>(C_UI_WND_CTOR_INT)(
        buf, nullptr, /*nUIType*/ 0, /*closeType*/ 3, /*closeX*/ closeX,
        /*closeY*/ 5, /*nBackgrnd*/ 0, /*nBackgrndX*/ 0, /*nBackgrndY*/ 0);

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
    // Ensure the layer is visible. Hide() sets put_visible(FALSE), and
    // delist/relist alone does not re-show it, so a re-Show must flip it back.
    SafeDispatch("CustomUI show layer", [buf] { SetLayerVisible(buf, 1); });
    fe.is_visible = true;
}

void CustomUIWnd::Hide() {
    auto& fe = Extras();
    if (!fe.is_visible)
        return;
    // Hide the COM layer FIRST: CWndMan::RemoveWindow only delists the window
    // for input/draw dispatch -- it never hides the layer, so the layer keeps
    // compositing its last-blitted content. put_visible(FALSE) removes it.
    SafeDispatch("CustomUI hide layer", [this] { SetLayerVisible(GameWnd(), 0); });
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
