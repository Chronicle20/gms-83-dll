#pragma once

class CWndMan {
public:
    static CWnd **s_Update() {
        return ((CWnd **(_fastcall * )())
        0x00A769D8)();
    }

    static void RedrawInvalidatedWindows() {
        ((VOID **(_fastcall * )())
        0x00A7675C)();
    }
};