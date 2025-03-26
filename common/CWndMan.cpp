#include "pch.h"

CWnd **CWndMan::s_Update() {
    return reinterpret_cast<CWnd **(__fastcall *)()>(C_WND_MAN_S_UPDATE)();
}

void CWndMan::RedrawInvalidatedWindows() {
    reinterpret_cast<void (__fastcall *)()>(C_WND_MAN_REDRAW_INVALIDATED_WINDOWS)();
}