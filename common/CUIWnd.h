#pragma once

#include "asserts.h"
#include <cstddef>

struct CUIWnd : CWnd {
    ZRef<CCtrlButton> m_pBtClose;
    CUIToolTip m_uiToolTip;
    int m_nUIType;
    int m_nBtCloseType;
    int m_nBtCloseX;
    int m_nBtCloseY;
    int m_nBackgrndX;
    int m_nBackgrndY;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
    int m_nSmallScreenX;
    int m_nSmallScreenY;
    int m_nLargeScreenX;
    int m_nLargeScreenY;
    bool m_bIsLargeMode;
#endif
    bool m_bPosSave;
    bool m_bBackgrnd;
    int m_nOption;
    ZArray<unsigned char> m_abOption;
    ZXString<unsigned short> m_sBackgrndUOL;
};

#if defined(REGION_GMS) && BUILD_MAJOR_VERSION == 79
assert_size(sizeof(CUIWnd), 0x5A8); // CWnd(0x64) + m_uiToolTip(CUIToolTip 0x514)@0x6C
static_assert(offsetof(CUIWnd, m_nBtCloseType) == 0x584,
              "v79 CUIWnd::m_nBtCloseType @0x584 (OnCreate @0x8862c7 switches on [esi+584h])");
static_assert(offsetof(CUIWnd, m_sBackgrndUOL) == 0x5A4,
              "v79 CUIWnd::m_sBackgrndUOL @0x5A4 (ReloadBackgrnd @0x8866dc: lea ecx, [esi+5A4h])");
#endif
