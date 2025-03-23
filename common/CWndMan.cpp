#include "pch.h"

CWnd ** CWndMan::s_Update() {
    return ((CWnd **(_fastcall * )())
    C_WND_MAN_S_UPDATE)();
}

void CWndMan::RedrawInvalidatedWindows() {
    ((VOID **(_fastcall * )())
    C_WND_MAN_REDRAW_INVALIDATED_WINDOWS)();
}