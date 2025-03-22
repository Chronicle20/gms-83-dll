#pragma once
#include "memory_map.h"

class CWndMan {
public:
    static CWnd **s_Update() {
        return ((CWnd **(_fastcall * )())
        C_WND_MAN_S_UPDATE)();
    }

    static void RedrawInvalidatedWindows() {
        ((VOID **(_fastcall * )())
                C_WND_MAN_REDRAW_INVALIDATED_WINDOWS)();
    }
};