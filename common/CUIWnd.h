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
#elif defined(REGION_GMS) && BUILD_MAJOR_VERSION == 72
// v72: CWnd(0x64) + m_uiToolTip(CUIToolTip 0x510)@0x6C -> trailing starts 4 below
// v79. ctor sub_83C0EC: m_pBtClose@0x64, m_uiToolTip@0x6C, dense args @0x57C..0x594,
// [esi+59Ch]/[esi+5A0h] zeroed; ReloadBackgrnd @0x83c71e: lea ecx, [esi+5A0h].
assert_size(sizeof(CUIWnd), 0x5A4);
static_assert(offsetof(CUIWnd, m_nBtCloseType) == 0x580,
              "v72 CUIWnd::m_nBtCloseType @0x580 (ctor sub_83C0EC: mov [esi+580h], arg) -- 4 below v79");
static_assert(offsetof(CUIWnd, m_sBackgrndUOL) == 0x5A0,
              "v72 CUIWnd::m_sBackgrndUOL @0x5A0 (ReloadBackgrnd @0x83c71e: lea ecx, [esi+5A0h])");
#endif
