#pragma once

class CWndMan {
public:
    static CWnd **s_Update() {
        return ((CWnd **(_fastcall * )())
        0x009E47C3)();
    }

    static void RedrawInvalidatedWindows() {
        ((VOID **(_fastcall * )())
        0x009E4547)();
    }
};