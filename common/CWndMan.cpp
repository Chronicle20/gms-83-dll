#include "pch.h"

typedef CWnd **(__thiscall *_CWndMan__s_Update_t)();

_CWndMan__s_Update_t _CWndMan__s_Update = reinterpret_cast<_CWndMan__s_Update_t>(0x00AC5202);

CWnd ** CWndMan::s_Update() {
    return _CWndMan__s_Update();
}

typedef VOID(__stdcall *_CWndMan__RedrawInvalidatedWindows_t)();

_CWndMan__RedrawInvalidatedWindows_t _CWndMan__RedrawInvalidatedWindows = reinterpret_cast<_CWndMan__RedrawInvalidatedWindows_t>(0x00AC4F86);

void CWndMan::RedrawInvalidatedWindows() {
    _CWndMan__RedrawInvalidatedWindows();
}