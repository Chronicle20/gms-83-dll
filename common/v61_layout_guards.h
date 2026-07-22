#pragma once
//
// Canonical compile-time size guards for the GMS v61 struct layout.
//
// Motivation (task-010): the v61 struct-verification audit — like task-008 (v79)
// and task-009 (v72) before it — recorded most headers as "confirmed-shares-v72 /
// unchanged" from disassembly evidence WITHOUT a compile-time assert. Worse, the
// UI base-struct corrections that landed on main (CCtrlWnd/CFadeWnd packed-bool ->
// 4-byte flags, CUIToolTip offset-0 vfptr slot) were gated `== 72 || == 79`, which
// silently EXCLUDES v61 — so v61 falls through to the pre-correction `#else` model
// exactly as v72 once did. A `static_assert` sweep against GMS_v61.1_U_DEVM.exe
// (IDA session 9a1bdd7a) is the only thing that turns the audit's prose verdicts
// into enforced facts. This file locks every v61 size that IS measured so drift can
// never silently recur — a failing build here means the header no longer matches the
// v61 binary.
//
// Each value is anchored to the v61 IDB (GMS_v61.1_U_DEVM, session 9a1bdd7a): an
// allocator immediate (`push <size>` -> Alloc) or a ctor field-init extent.
//
// Included at the end of pch.h; all referenced types are defined by then.

#include "asserts.h"
#include <cstddef>

#if defined(REGION_GMS) && BUILD_MAJOR_VERSION == 61
// --- CWvsContext account region (task-010 post-testing; char-select garbage fix) ---
// v61 reorders/resizes the account fields (m_nSubGradeCode is a plain byte, ZXStrings precede
// it) vs v72+. These offsets are the fields our edit DLLs actually read; a wrong value here is
// the reported channelId/characterId garbage. Verified against GMS_v61.1_U_DEVM (IDA 9a1bdd7a):
// SetAccountInfo @0x56613D, sub_82CE54 (world/channel), OnConnect migrate [ctx+0x2088]=charId.
static_assert(offsetof(CWvsContext, m_dwAccountId) == 0x2028, "v61 CWvsContext::m_dwAccountId @0x2028");
static_assert(offsetof(CWvsContext, m_nSubGradeCode) == 0x2044,
              "v61 CWvsContext::m_nSubGradeCode @0x2044 (plain byte)");
static_assert(offsetof(CWvsContext, m_nWorldID) == 0x2048, "v61 CWvsContext::m_nWorldID @0x2048");
static_assert(offsetof(CWvsContext, m_nChannelID) == 0x204C, "v61 CWvsContext::m_nChannelID @0x204C");
static_assert(offsetof(CWvsContext, m_dwCharacterId) == 0x2088, "v61 CWvsContext::m_dwCharacterId @0x2088");

// --- core / net (task-010 Task 13; Alloc-immediate anchored) ---
assert_size(sizeof(CClientSocket), 0x94); // CreateInstance @0x826007 push 94h
assert_size(sizeof(COutPacket), 0x10);    // ctor init sub_5FFC98 highest write +0xC
assert_size(sizeof(CONFIG_SYSOPT), 0x30); // ApplySysOpt @0x47b29d rep movsd 0xC dwords
assert_size(sizeof(CLogin), 0x1DC);       // stage Alloc @0x8460f9 push 1DCh (one ZList; unk3[5] absent)
assert_size(sizeof(CLogo), 0x38);         // ctor @0x5950F4 highest write +0x34; CStage base

// --- base chain (task-010 Task 12/14) ---
assert_size(sizeof(CWnd), 0x64);    // == v72 (anim-layer pair >=83 absent); ctor @0x4BB456
assert_size(sizeof(CDialog), 0x74); // CWnd 0x64 + 0x10; CFadeWnd::SetOption m_a0 @[ecx+74h]

// --- Mob / stat family (task-010 Task 15; splits below the v72 floor) ---
assert_size(sizeof(CMob), 0x490);        // CreateMob @0x5C20EC push 490h (-0x30 vs v72 0x4C0)
assert_size(sizeof(MobStat), 0x1B8);     // SetFrom @0x6685A7 memset 1B8h (-0x20 vs v72 0x1D8)
assert_size(sizeof(CMapLoadable), 0xEC); // CField::CField @0x4E4830 base ctor extent +0xE8

// --- UI control family (task-010 Task 12/14; v61 EXCLUDED from the ==72||==79 fixes) ---
// NOTE: these are added incrementally as the v61 IDB re-verification confirms each
// exact size and whether v61 must join the CCtrlWnd/CFadeWnd/CUIToolTip corrections.
#endif
