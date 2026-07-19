#pragma once
//
// Canonical compile-time size guards for the GMS v72 struct layout.
//
// Motivation (task-009): the v72 struct-verification audit — like task-008's for
// v79 — recorded several headers as "verified/unchanged" from disassembly evidence
// WITHOUT a compile-time assert, and the v79 layout corrections that landed on main
// were gated `== 79`, silently EXCLUDING v72 (v72 fell through to the v95-derived
// `#else` branches). A `static_assert` sweep against GMS_v72.1_U_DEVM.exe found the
// same silent-drift trap for SecondaryStat, CLogin, the CMapLoadable base chain, and
// the CUIToolTip/CCtrlButton/CUIWnd UI family. This file locks every v72 size that IS
// verified so drift can never silently recur — a failing build here means the header
// no longer matches the v72 binary.
//
// Each value is anchored to the v72 IDB (GMS_v72.1_U_DEVM, session eb2a156e): an
// allocator immediate (`push <size>` -> Alloc) or a ctor field-init extent.
//
// Included at the end of pch.h; all referenced types are defined by then.

#include "asserts.h"

#if defined(REGION_GMS) && BUILD_MAJOR_VERSION == 72
// --- base chain (task-009) ---
assert_size(sizeof(CStage), 0x18); // CMapLoadable ctor calls CStage ctor sub_468D66; 4 base vtables
assert_size(sizeof(CMapLoadable),
            0xF8); // base ctor sub_5EADAB; CLogin::m_pConnectionDlg @0xF8 (RemoveNoticeConnecting 0x5B3ED9)
assert_size(sizeof(CLogin), 0x23C); // ctor @0x5AECED; m_aFemaleItem[9] eh-vector @0x218 -> 0x23C; LogoEnd Alloc(0x23C)

// --- context / stat family (task-009) ---
assert_size(sizeof(SecondaryStat), 0xAB0); // ctor sub_6C70E9: aTemporaryStat[7] @0xA78 -> 0xAB0 (-0xD8 vs v79)

// --- UI control family (task-009; v72 shares v79 where noted, else -4 from one fewer CUIToolTip font) ---
assert_size(sizeof(CCtrlWnd), 0x34);      // ctor @0x4cc645 == v79 (3 trailing flags 4-byte)
assert_size(sizeof(CUIToolTip), 0x510);   // ctor @0x7f9c33; one fewer base font than v79 (0x514)
assert_size(sizeof(CCtrlButton), 0x59C);  // alloc @0x500921 push 59Ch; 0x8C + CUIToolTip 0x510 (m_uiToolTip last)
assert_size(sizeof(CCtrlCheckBox), 0x6C); // CCtrlWnd 0x34 base (== v79)
assert_size(sizeof(CUIWnd), 0x5A4); // CWnd 0x64 + CUIToolTip 0x510 @0x6C; ReloadBackgrnd @0x83c71e m_sBackgrndUOL@0x5A0
assert_size(sizeof(CFadeWnd), 0xCC); // CDialog 0x74 base; ctor @0x4ffd72 == v79
#endif
