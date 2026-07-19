# `CLogin` v72 audit (GMS_v72.1_U_DEVM) — vs v79/v83

**Target IDB (verified first):** `GMS_v72.1_U_DEVM.exe`, session `eb2a156e`
(`server_health` → `idb_path …\GMS\v72\GMS_v72.1_U_DEVM.exe.i64`, imagebase 0x400000). Confirmed **before** any version-specific probe.
**Cross-refs:** v79 `GMS_v79_1_DEVM` session `88dfa464` (CLogin ctor 0x5C93E7); v83 numbers from `common/CLogin.h` comment / prior audits.
**Methodology:** disassembled the v72 ctor `??0CLogin@@QAE@XZ` @0x5AECED (field-init + `__CxxFrameHandler` unwind funclets, which name each member's dtor helper) and `OnSelectWorldResult` @0x5B4ABC. Raw `[esi+OFFh]` / `add ecx,OFFh` extraction only. No decompile, no `analyze_struct_detailed`.

---

## TL;DR / verdicts

- **v72 `sizeof(CLogin)` = `0x23C` (572)** — CONFIRMED from the binary. The pre-existing v72 numbers (0x23C, one ZList, ctor 0x5AECED, `m_lNewEquip@0x174`, `m_aCmd[5]@0x1C8`) are all **correct**.
- The pre-existing **conclusion "no header edit needed / `#else` branch already yields v72"** is **WRONG** (it only checked the `==83` `unk3[5]` exclusion, never the total layout). v72 currently falls into the `#else` (v83/v95-shaped) branch, which cannot reproduce v72's layout. There is no `assert_size`, so the drift is silent — the exact task-008 failure mode. **v72 needs its own dedicated `== 72` branch + a `common/v72_layout_guards.h`.**
- v72 genuinely has **ONE** `ZList<NEWEQUIP>` (`off_9D317C` @ `this+0x174`); v79/v83 build **TWO**. So the `CLogin.h:235` `unk3[5]` (== the *second* ZList, 5 dwords = 0x14 B) is correctly **absent** in v72.
- **v79 concern (flagged) — CONFIRMED:** v79 ctor @0x5C93E7 builds two ZLists (`off_A2F9FC` at `[esi+178h]` AND `[esi+18Ch]`). v79 **does** possess the member `CLogin.h:235` gates to `==83`-only. Today it is masked because the `==79` branch models it as `m_lNewEquip2`; the `==83` gate is therefore *under-inclusive* but harmless for v79. No v72 impact.

---

## Binary anchors (v72)

| Anchor | Address | Evidence |
| --- | --- | --- |
| `CLogin::CLogin` | `0x5AECED` (size 0x19D) | ctor; first own write `mov [esi+0F8h],ebx` @0x5AED0A → CLogin members start at **0xF8** |
| ZList vtable (only one) | `off_9D317C` | `mov [eax],offset off_9D317C`, `eax=esi+174h` @0x5AEDBC |
| `m_aCmd[5]` ctor | eh-vector `[esi+1C8h]`, count 5, size 4, ZXString dtor | @0x5AEE0C-17 |
| `m_aMaleItem[9]` | eh-vector `[esi+1F4h]`, count 9, size 4 | @0x5AEE38 |
| `m_aFemaleItem[9]` | eh-vector `[esi+218h]`, count 9, size 4 → **ends 0x218+0x24 = 0x23C** | @0x5AEE4E |
| size (independent) | audit sig-cat:389: `CLogo::LogoEnd Alloc(0x23C)` → CLogin::CLogin → set_stage | matches ctor extent |
| `m_bRequestSent @0x14C` | `mov [ecx+14Ch],ebx` at entry of `OnSelectWorldResult` @0x5B4ACF | independent of ctor |

## v72 member map (absolute, this-relative; from ctor body + unwind funclets)

| off | member | note / unwind helper |
| --- | --- | --- |
| 0x0-0xF7 | `CMapLoadable` base | base ctor `sub_5EADAB`; **base ends 0xF8** (see base flag below) |
| 0xF8 | `m_pConnectionDlg` (ZRef, 4B here) | 0xF8,0xFC 4 apart → ZRef is 4B in v72 |
| 0xFC | `m_bIsWaitingVAC` | |
| 0x100 | `m_bIsVACDlgOn` (GMS) | |
| 0x104 | `m_tSentTimeVACPacket` | |
| 0x108 | `m_nCountRelatedSvrs` | |
| 0x10C | `m_nCountCharacters` | |
| 0x110 | `m_nCountDataReceivedCharacters` | **only 7 shared VAC dwords (0xF8..0x113); one GMS shared field absent** — see flag |
| 0x114 | `m_aAvatarDataVAC` ZArray<AvatarData> | unwind +0x114 → `ZArray<AvatarData>::RemoveAll` |
| 0x118 | `m_aRankVAC` ZArray<RANK> | +0x118 → `ZArray<RANK>::RemoveAll` |
| 0x11C | `m_adwCharacterID` ZArray<ulong> | +0x11C |
| 0x120 | `m_asCharacterName` ZArray<ZXString> | +0x120 |
| 0x124 | `m_anWorldID` ZArray<long> | +0x124 |
| 0x128 | `m_lock_Avatar` ZFatalSection (8B) | +0x128 nullsub |
| 0x130 | `m_lock_CountSvr` (8B) | +0x130 nullsub |
| 0x138 | `m_lock_Character` (8B) | +0x138 nullsub |
| 0x140 | com_ptr (`sub_412571`) — `m_pLayerBook`-analog | +0x140; **present in v72 though header gates m_pLayerBook `>=95`** |
| 0x144,0x148 | login-step ints | |
| 0x14C | `m_bRequestSent` | ctor + `OnSelectWorldResult` write `[.+14Ch]=0` |
| (0x150) | byte field `m_bLoginOpt` (unwritten dword) | |
| 0x154,0x158,0x15C,0x160 | login-step block | |
| 0x164 | `m_WorldItem` ZArray<WORLDITEM> | +0x164 → `sub_5B63FC` (**the heap-corruption crash field**) |
| 0x168 | `m_nCharSelected` (= -1) | `or [esi+168h],0FFFFFFFFh` |
| 0x16C | `m_aAvatarData` ZArray<AvatarData> | +0x16C |
| 0x170 | `m_aRank` ZArray<RANK> | +0x170 → `ZArray<RANK>::RemoveAll` |
| — | `m_abOnFamily` ZArray<int> | **ABSENT in v72** (v95/`#else` has it between m_aRank and m_lNewEquip) |
| 0x174 | `m_lNewEquip` ZList<NEWEQUIP> (0x14B) | **only ZList** `off_9D317C` |
| 0x188 | `m_nRegStatID` (byte) | unwritten dword slot |
| 0x18C | `m_pLayerLight` com_ptr | +0x18C `sub_412571` |
| 0x190 | `m_pLayerDust` com_ptr | +0x190 |
| 0x194 | `m_pNewAvatar` ZRef<CAvatar> (8B) | +0x194 `sub_425407` |
| 0x19C | `m_pLoginStart` ZRef (8B) | +0x19C |
| 0x1A4 | `m_pLoginDesc0` ZRef (8B) | +0x1A4 `sub_5B6456` |
| 0x1AC | `m_pLoginDesc1` ZRef (8B) | +0x1AC |
| 0x1B4 | `m_pChildModal` ZRef<CDialog> (8B) | +0x1B4 |
| 0x1BC | `m_sEventCharacterID` ZXString | +0x1BC ZXString dtor |
| 0x1C0 | int (`m_nBalloonCount`) | |
| 0x1C4 | ZArray (`m_aBalloon`) | +0x1C4 `sub_5B64A0` — **v72 has almost none of v95's mid-block (no m_nLatestConnectedWorldID / m_bRecommendWorldMsgLoaded / m_aRecommendWorldMsg / m_nCurSelectedRace / m_nCurSelectedSubJob here)** |
| 0x1C8 | `m_aCmd[5]` ZXString eh-vector (0x14B) → ends 0x1DC | |
| 0x1DC..0x1E3 | gap (m_tFadeInRemain / m_bNeedAgreement) | |
| 0x1E4 | byte (`m_nGender`) | `mov [esi+1E4h],bl` |
| 0x1F0 | `m_sCheckedName` ZXString | +0x1F0 ZXString dtor |
| 0x1F4 | `m_aMaleItem[9]` (0x24B) → ends 0x218 | |
| 0x218 | `m_aFemaleItem[9]` (0x24B) → **ends 0x23C** | last member |

## Delta vs v79 / v83

| | v72 | v79 | v83 |
| --- | --- | --- | --- |
| `sizeof(CLogin)` | **0x23C** | 0x258 | 0x2C8 |
| `ZList<NEWEQUIP>` count | **1** (@0x174) | 2 (@0x178,@0x18C) | 2 |
| `m_aCmd[5]` | **0x1C8** | 0x1E4 | — |
| `m_WorldItem` | 0x164 | 0x164 | 0x1CC |
| `m_abOnFamily` | absent | present (0x174) | present |
| CLogin own-fields start | **0xF8** | 0x114(base)+prefix | — |

Δsize(v79−v72)=0x1C: −0x14 (missing 2nd ZList) −0x04 (missing `m_abOnFamily`) −0x04 (shorter shared-VAC prefix: 7 vs 8 dwords) plus v72's compressed EventCharacterID→aCmd block offsetting other small +/−.

---

## Why the current header mis-lays-out v72 (the real bug)

For a `BUILD_MAJOR_VERSION == 72` build, `CLogin.h` selects the **`#else`** branch (72 ≠ 79). That branch is the v83/v95/JMS generic shape and diverges from the v72 binary in at least four independent ways:

1. **Base:** v72 CLogin members start at **0xF8** → `CMapLoadable` compiles to 0xF8 in v72; the `#else` branch inherits whatever `CMapLoadable.h` yields for v72 (its `>=95`/`>=84` gates compute ≈0x114). Base is off before CLogin even begins.
2. **Shared VAC prefix:** header emits **8** GMS dwords (`m_pConnectionDlg…m_bRecommandWorld`); the v72 ctor writes only **7** (0xF8..0x113). One shared field (most likely `m_bRecommandWorld`) is absent in v72 → +4 drift.
3. **`m_abOnFamily`:** present in `#else`, **absent** in v72 → +4 drift.
4. **Mid-block bloat:** `#else` inserts `m_nLatestConnectedWorldID`, `m_bRecommendWorldMsgLoaded`, `m_aRecommendWorldMsg`, `m_nCurSelectedRace` between `m_sEventCharacterID` and `m_aCmd`; v72 has none of them, so `#else` puts `m_aCmd` far past 0x1C8 and the struct far past 0x23C.

Net: the `#else` branch does **not** sum to 0x23C for v72; `m_WorldItem` does not land at 0x164. Since `m_WorldItem.RemoveAll()` in `login_hooks.cpp` is the v79 heap-corruption site, a wrong v72 offset here reproduces the same crash class.

---

## Proposed header gate edits — `common/CLogin.h`

Add a dedicated v72 branch, mirroring the existing `== 79` block's opaque-filler-with-anchors style. Insert as a new `#elif` **before** the current `#else`:

```cpp
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION == 79)
    ... existing v79 block ...
#elif (defined(REGION_GMS) && BUILD_MAJOR_VERSION == 72)
    // =====================================================================
    // GMS v72 layout (task-009). Anchored to GMS_v72.1_U_DEVM.exe:
    //   ctor            @0x5AECED (field-init extent; last member ends 0x23C)
    //   OnSelectWorldResult @0x5B4ABC ([this+14Ch]=m_bRequestSent)
    // v72 has ONE ZList (m_lNewEquip @0x174); v79/v83 build TWO. v72 also
    // LACKS m_abOnFamily and most of the v95 EventCharacterID..aCmd mid-block.
    // Offsets absolute; CLogin own-fields begin at 0xF8 (CMapLoadable base).
    // Opaque gap fillers sized to exact binary extent (all 4B-aligned).
    // =====================================================================
    ZArray<AvatarData> m_aAvatarDataVAC;     // 0x114
    ZArray<CLogin::RANK> m_aRankVAC;         // 0x118
    ZArray<unsigned long> m_adwCharacterID;  // 0x11C
    ZArray<ZXString<char>> m_asCharacterName;// 0x120
    ZArray<long> m_anWorldID;                // 0x124
    ZFatalSection m_lock_Avatar;             // 0x128
    ZFatalSection m_lock_CountSvr;           // 0x130
    ZFatalSection m_lock_Character;          // 0x138
    int m_v72_gap140;                        // 0x140 com_ptr (m_pLayerBook-analog)
    int m_v72_gap144[2];                     // 0x144..0x14B
    int m_bRequestSent;                      // 0x14C  (OnSelectWorldResult [.+14Ch])
    int m_v72_gap150[5];                     // 0x150..0x163
    ZArray<CLogin::WORLDITEM> m_WorldItem;   // 0x164  RemoveAll crash field
    int m_nCharSelected;                     // 0x168  ctor = -1
    ZArray<AvatarData> m_aAvatarData;        // 0x16C
    ZArray<CLogin::RANK> m_aRank;            // 0x170
    ZList<CLogin::NEWEQUIP> m_lNewEquip;     // 0x174  ONLY ZList (off_9D317C)
    int m_v72_gap188[16];                    // 0x188..0x1C7 (m_nRegStatID..m_aBalloon)
    ZXString<char> m_aCmd[5];                // 0x1C8  eh-vector[5]
    int m_v72_gap1DC[6];                     // 0x1DC..0x1F3
    ZArray<CLogin::ASITEM> m_aMaleItem[9];   // 0x1F4  eh-vector[9]
    ZArray<CLogin::ASITEM> m_aFemaleItem[9]; // 0x218  eh-vector[9] -> 0x23C
#else
    ... existing generic branch ...
#endif
```

Also add a `== 72` guard block at the bottom of `CLogin.h` (mirroring the `== 79` block, lines 333-352):

```cpp
#if defined(REGION_GMS) && BUILD_MAJOR_VERSION == 72
assert_size(sizeof(CLogin), 0x23C);          // ctor @0x5AECED ends 0x23C; LogoEnd Alloc(0x23C)
static_assert(offsetof(CLogin, m_aAvatarDataVAC) == 0x114, "v72 m_aAvatarDataVAC @0x114");
static_assert(offsetof(CLogin, m_bRequestSent)   == 0x14C, "v72 m_bRequestSent @0x14C (OnSelectWorldResult)");
static_assert(offsetof(CLogin, m_WorldItem)      == 0x164, "v72 m_WorldItem @0x164 (RemoveAll crash field)");
static_assert(offsetof(CLogin, m_nCharSelected)  == 0x168, "v72 m_nCharSelected @0x168 (ctor = -1)");
static_assert(offsetof(CLogin, m_lNewEquip)      == 0x174, "v72 m_lNewEquip @0x174 (only ZList, off_9D317C)");
static_assert(offsetof(CLogin, m_aCmd)           == 0x1C8, "v72 m_aCmd[5] @0x1C8");
#endif
```

`CLogin.h:235` (`#if ... == 83) int unk3[5];`) — **no change**; v72 legitimately lacks the second ZList so exclusion is correct. (Separately: this gate is under-inclusive for v79, which *does* have it — but v79's dedicated branch already models it as `m_lNewEquip2`, so no build breakage. Out of scope for v72; noted for the gate owner.)

## Proposed `common/v72_layout_guards.h` (new file)

Mirror `common/v79_layout_guards.h`. Minimum CLogin content:

```cpp
#pragma once
#include "asserts.h"
#if defined(REGION_GMS) && BUILD_MAJOR_VERSION == 72
// Anchored to GMS_v72.1_U_DEVM.exe (session eb2a156e).
assert_size(sizeof(CLogin), 0x23C);   // ctor @0x5AECED field-init extent; LogoEnd Alloc(0x23C)
static_assert(offsetof(CLogin, m_aAvatarDataVAC) == 0x114, "CLogin::m_aAvatarDataVAC @0x114 (v72)");
static_assert(offsetof(CLogin, m_bRequestSent)   == 0x14C, "CLogin::m_bRequestSent @0x14C (v72)");
static_assert(offsetof(CLogin, m_WorldItem)      == 0x164, "CLogin::m_WorldItem @0x164 (v72; RemoveAll crash field)");
static_assert(offsetof(CLogin, m_nCharSelected)  == 0x168, "CLogin::m_nCharSelected @0x168 (v72)");
static_assert(offsetof(CLogin, m_lNewEquip)      == 0x174, "CLogin::m_lNewEquip @0x174 (v72; single ZList)");
static_assert(offsetof(CLogin, m_aCmd)           == 0x1C8, "CLogin::m_aCmd[5] @0x1C8 (v72)");
#endif
```
(Include it at the end of `pch.h` alongside/instead of the v79 guards, or `#include` both and let the `BUILD_MAJOR_VERSION` gates select.)

---

## REQUIRED companion fixes (the CLogin assert will not pass without them)

1. **`CMapLoadable` must compile to 0xF8 for v72.** The v72 CLogin ctor's first own write is `[esi+0F8h]`, so `sizeof(CMapLoadable)==0xF8` in v72 (vs 0x114 in v79). `CMapLoadable.h`'s current gates (`>=95`, `>=84`) compute ≈0x114 for v72 — ~0x1C too large. Needs a separate v72 `CMapLoadable` verification; until fixed, `offsetof(CLogin,m_aAvatarDataVAC)==0x114` will fail. **Recommend a follow-up CMapLoadable v72 audit.**
2. **Shared VAC prefix is 7 dwords in v72, not 8.** One of the GMS shared fields above the `#if` (lines 188-199) — most likely `m_bRecommandWorld` — must be gated out for v72, else `m_aAvatarDataVAC` lands at 0x118 not 0x114. Confirm which field via a VAC-touching method (`ResetVAC` @0x5B4408 / `MakeVACDlg` @0x5B4394) before choosing the gate.

## Conflict with pre-existing v72 audit

- **Numbers:** no conflict — 0x23C, one ZList, ctor 0x5AECED, `m_lNewEquip@0x174`, `m_aCmd@0x1C8` all reproduced from the binary.
- **Conclusion:** the pre-existing `struct-verification.md` "unchanged / no edit (==83 excludes v72)" is **incomplete and wrong as a whole-layout statement**. Excluding `unk3[5]` is necessary but the `#else` branch still mis-lays-out v72 (base 0xF8, 7-dword prefix, no `m_abOnFamily`, compressed mid-block). v72 requires a dedicated branch + guards, exactly as task-008 concluded for v79.

## Open questions

- Which specific shared VAC field is absent in v72 (m_bRecommandWorld vs m_bIsVACDlgOn)? Disassemble `ResetVAC` @0x5B4408 / `MakeVACDlg` @0x5B4394.
- `CMapLoadable` v72 exact size/gating (0xF8) — separate audit; blocks the CLogin guard.
- Names for the two v72 com_ptrs at 0x140 and 0x18C/0x190 (light/dust); cosmetic — offsets are pinned.
