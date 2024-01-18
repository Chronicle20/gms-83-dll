#pragma once

#include "CWnd.h"

class CWndMan {
public:
    static CWnd **s_Update() {
        return ((CWnd **(_fastcall * )())
        0x009E47C3)();
    }
};