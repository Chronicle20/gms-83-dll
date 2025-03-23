#pragma once

class CWndMan {
public:
    static CWnd **s_Update();

    static void RedrawInvalidatedWindows();
};