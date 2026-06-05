#include "pch.h"

#include "hooks/debug_hooks.h"

#include "hooker.h"
#include "logger.h"
#include "memory_map.h"
#include "runtime/seh_dispatch.h"

namespace custom_ui_host {

namespace {

typedef void(__cdecl* PcCreateIWzFont_t)(const wchar_t*, void**, int);
PcCreateIWzFont_t _PcCreateIWzFont = nullptr;

void __cdecl PcCreateIWzFont_Hook(const wchar_t* a1, void** a2, int a3) {
    // %ls dereferences a1; SEH-guard in case a1 is not a valid string pointer
    // (an unguarded fault here would crash the client's render/UI thread).
    SafeDispatch("DBG IWzFont",
                 [a1, a3] { Log("DBG PcCreateObject::IWzFont a1=[%ls] a3=%d", a1 ? a1 : L"(null)", a3); });
    _PcCreateIWzFont(a1, a2, a3);
}

typedef void(__thiscall* CreateCtrl_t)(void*, void*, unsigned int, long, long, long, void*);
CreateCtrl_t _CreateCtrl = nullptr;

void __fastcall CreateCtrl_Hook(void* self, void* /*edx*/, void* parent, unsigned int id, long x, long y,
                                long nDecClickArea, void* param) {
    // Dump the raw CREATEPARAM bytes + candidate UOL pointers at offsets 3 and
    // 4 so we can read the true layout. Each pointer deref is SEH-guarded
    // individually because a wrong offset would fault.
    SafeDispatch("DBG CreateCtrl", [id, x, y, nDecClickArea, param] {
        const unsigned char* p = static_cast<const unsigned char*>(param);
        if (!p) {
            Log("DBG CreateCtrl id=%u (param null)", id);
            return;
        }
        Log("DBG CreateCtrl id=%u x=%ld y=%ld dec=%ld bytes=%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X "
            "%02X %02X %02X %02X %02X",
            id, x, y, nDecClickArea, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12],
            p[13], p[14], p[15]);
        const wchar_t* uol3 = *reinterpret_cast<const wchar_t* const*>(p + 3);
        SafeDispatch("DBG uol@3", [uol3] { Log("DBG   uol@3=[%ls]", uol3 ? uol3 : L"(null)"); });
        const wchar_t* uol4 = *reinterpret_cast<const wchar_t* const*>(p + 4);
        SafeDispatch("DBG uol@4", [uol4] { Log("DBG   uol@4=[%ls]", uol4 ? uol4 : L"(null)"); });
    });
    _CreateCtrl(self, parent, id, x, y, nDecClickArea, param);
}

} // namespace

BOOL InstallDebugHooks() {
    INITMAPLEHOOK_OR_RETURN(_PcCreateIWzFont, PcCreateIWzFont_t, &PcCreateIWzFont_Hook, C_PC_CREATE_IWZFONT);
    INITMAPLEHOOK_OR_RETURN(_CreateCtrl, CreateCtrl_t, &CreateCtrl_Hook, C_CTRL_BUTTON_CREATE_CTRL);
    return TRUE;
}

} // namespace custom_ui_host
