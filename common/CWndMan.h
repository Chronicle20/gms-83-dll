#pragma once

class CWndMan {
public:
    static CWnd **s_Update() {
        return ((CWnd **(_fastcall * )())
        0x00AC5202)();
    }

    static void RedrawInvalidatedWindows() {
        ((VOID **(_fastcall * )())
        0x00AC4F86)();
    }
};