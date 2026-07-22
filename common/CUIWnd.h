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
    // v61: the trailing m_sBackgrndUOL is absent — the ctor (sub_787356) stops initializing
    // at [esi+56C] (= m_abOption), giving sizeof 0x570 (vs the 0x574 the shared tail would
    // yield). Size binary-proven by 3 direct subclasses that begin own members @0x570; the
    // dropped-member identity is best-effort. task-010.
#if !(defined(REGION_GMS) && BUILD_MAJOR_VERSION == 61)
    ZXString<unsigned short> m_sBackgrndUOL;
#endif
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
#elif defined(REGION_GMS) && BUILD_MAJOR_VERSION == 61
// v61: CWnd(0x64) + m_uiToolTip(CUIToolTip 0x4E0)@0x6C -> 0x30 below v72; trailing
// m_sBackgrndUOL absent -> sizeof 0x570 (3 direct subclasses begin own members @0x570;
// ctor sub_787356). task-010.
assert_size(sizeof(CUIWnd), 0x570);
static_assert(offsetof(CUIWnd, m_nBtCloseType) == 0x550,
              "v61 CUIWnd::m_nBtCloseType @0x550 (m_uiToolTip 0x4E0 -> trailing 0x30 below v72)");
#endif
