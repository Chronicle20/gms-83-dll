#pragma once
//
// Canonical compile-time size guards for the GMS v79 struct layout.
//
// Motivation (task-008): the struct-verification.md audit recorded several
// headers as "verified/unchanged" from disassembly evidence WITHOUT a
// compile-time assert, and the C++ headers silently drifted from the
// binary-anchored sizes. A `static_assert` sweep found 7 mismatches
// (CLogin, SecondaryStat, CUIToolTip, CUIWnd, CCtrlButton, CCtrlCheckBox,
// CFadeWnd). This file locks every v79 size that IS verified so drift can
// never silently recur again — a failing build here means the header no
// longer matches the v79 binary.
//
// Each entry's value is anchored to the v79 IDB (GMS_v79_1_DEVM, port 13340):
// an allocator immediate (`push <size>` -> Alloc) or a ctor field-init extent.
// Add a struct here the moment its header is confirmed against the binary.
//
// Included at the end of pch.h; all referenced types are defined by then.

#include "asserts.h"

#if defined(REGION_GMS) && BUILD_MAJOR_VERSION == 79
// --- confirmed-correct (task-008 audit: header == v79 binary) ---
assert_size(sizeof(CMapLoadable),      0x114); // abstract base; triangulated (task-15)
assert_size(sizeof(CWvsApp),           0x60);  // ctor @0x942D3B base layout == v83/84
assert_size(sizeof(CClientSocket),     0x94);  // CreateInstance @0x946aca push 94h
assert_size(sizeof(CLogo),             0x38);  // ctor @0x5ff8c4; CStage base
assert_size(sizeof(COutPacket),        0x10);  // ctor @0x67ad6b / Init @0x67ae68
assert_size(sizeof(CONFIG_SYSOPT),     0x30);  // ApplySysOpt @0x4960f9 rep movsd 0xC dwords
assert_size(sizeof(CFuncKeyMappedMan), 0x388); // CreateInstance @0x946AFB push 388h
assert_size(sizeof(CWnd),              0x64);  // -8 vs v83 (no m_pAnimationLayer/m_pOverlabLayer)
assert_size(sizeof(CDialog),           0x74);  // CFadeWnd::SetOption m_a0 @[ecx+74h]
assert_size(sizeof(CMob),              0x518); // CreateMob @0x630BF0 push 518h
assert_size(sizeof(MobStat),           0x1F8); // CMob ctor lBurnedInfo @0x1E0 -> 0x1F8

// --- BROKEN (task-008 audit: header != v79 binary; assert added as each is
// fixed. Values are the v79 binary targets the header must be corrected to). ---
// assert_size(sizeof(CUIToolTip),     0x514); // header 0x510 (-4)  -- TODO fix
assert_size(sizeof(CLogin),            0x258); // task-008: header rebuilt to v79 (ctor @0x5c93e7)
// assert_size(sizeof(CCtrlButton),    0x5A4); // header 0x598 (-0xC) -- TODO fix
// assert_size(sizeof(CFadeWnd),       0xCC);  // header 0xC4  (-8)   -- TODO fix
// assert_size(sizeof(CCtrlCheckBox),  0x6C);  // header 0x64  (-8)   -- TODO fix
// assert_size(sizeof(CUIWnd),         0x5A8); // header 0x5A4 (-4)   -- TODO fix (embeds CUIToolTip)
// assert_size(sizeof(SecondaryStat),  0xB88); // header 0xE1C (+0x294) -- TODO fix (shifts CWvsContext)
#endif
