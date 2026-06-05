#include "pch.h"

#include "hooks/debug_hooks.h"

#include "hooker.h"
#include "logger.h"
#include "memory_map.h"

namespace custom_ui_host {

namespace {

typedef void(__cdecl* PcCreateIWzFont_t)(const wchar_t*, void**, int);
PcCreateIWzFont_t _PcCreateIWzFont = nullptr;

void __cdecl PcCreateIWzFont_Hook(const wchar_t* a1, void** a2, int a3) {
    Log("DBG PcCreateObject::IWzFont a1=[%ls] a3=%d", a1 ? a1 : L"(null)", a3);
    _PcCreateIWzFont(a1, a2, a3);
}

typedef void(__thiscall* CreateCtrl_t)(void*, void*, unsigned int, long, long, long, void*);
CreateCtrl_t _CreateCtrl = nullptr;

void __fastcall CreateCtrl_Hook(void* self, void* /*edx*/, void* parent, unsigned int id, long x, long y,
                                long nDecClickArea, void* param) {
    const unsigned char* p = static_cast<const unsigned char*>(param);
    const wchar_t* uol = nullptr;
    if (p) {
        // sUOL (ZXString<unsigned short>) is at packed byte offset 3; its
        // m_pStr points at the UOL text.
        uol = *reinterpret_cast<const wchar_t* const*>(p + 3);
    }
    Log("DBG CCtrlButton::CreateCtrl id=%u x=%ld y=%ld decClick=%ld bools=%d,%d,%d uol=[%ls]", id, x, y, nDecClickArea,
        p ? p[0] : -1, p ? p[1] : -1, p ? p[2] : -1, uol ? uol : L"(null)");
    _CreateCtrl(self, parent, id, x, y, nDecClickArea, param);
}

} // namespace

BOOL InstallDebugHooks() {
    INITMAPLEHOOK_OR_RETURN(_PcCreateIWzFont, PcCreateIWzFont_t, &PcCreateIWzFont_Hook, C_PC_CREATE_IWZFONT);
    INITMAPLEHOOK_OR_RETURN(_CreateCtrl, CreateCtrl_t, &CreateCtrl_Hook, C_CTRL_BUTTON_CREATE_CTRL);
    return TRUE;
}

} // namespace custom_ui_host
