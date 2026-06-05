#include "pch.h"

CWnd **CWndMan::s_Update() {
    return reinterpret_cast<CWnd **(__fastcall *)()>(C_WND_MAN_S_UPDATE)();
}

void CWndMan::RedrawInvalidatedWindows() {
    reinterpret_cast<void (__fastcall *)()>(C_WND_MAN_REDRAW_INVALIDATED_WINDOWS)();
}

void CWndMan::RegisterUIWindow(CUIWnd *pWnd) {
    reinterpret_cast<void(__fastcall *)(CWndMan *, void *, CUIWnd *)>(
        C_WND_MAN_REGISTER_UI_WINDOW)(this, nullptr, pWnd);
}

void CWndMan::UnregisterUIWindow(CUIWnd *pWnd) {
    reinterpret_cast<void(__fastcall *)(CWndMan *, void *, CUIWnd *)>(
        C_WND_MAN_UNREGISTER_UI_WINDOW)(this, nullptr, pWnd);
}

long CWndMan::ProcessKey(unsigned int msg, unsigned int vk, long lParam) {
    return reinterpret_cast<long(__fastcall *)(CWndMan *, void *, unsigned int,
                                               unsigned int, long)>(
        C_WND_MAN_PROCESS_KEY)(this, nullptr, msg, vk, lParam);
}
