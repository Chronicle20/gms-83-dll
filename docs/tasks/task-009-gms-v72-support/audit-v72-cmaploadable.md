# `CMapLoadable` v72 verification (vs v95 reference + v79 cross-check)

**Target IDB:** GMS_v72.1_U_DEVM.exe (session `eb2a156e`), imagebase 0x400000 — confirmed via `server_health` before probing.
**Cross-check IDB:** GMS_v79_1_DEVM.exe (session `88dfa464`).
**v95 reference:** header block in `common/CMapLoadable.h` (sizeof 0x148) + `common/CLogin.h` (sizeof 0x2C8).
**Methodology:** read-only disassembly only (no decompile, no `analyze_struct_detailed`). Anchored the abstract base via the concrete derived `CLogin` (alloc-anchored) + method accessors.

---

## TL;DR — the premise was wrong; here is the corrected picture

1. **v72 `CMapLoadable` = `0xF8` (248).** Anchored by `CLogin::RemoveNoticeConnecting` (`mov ecx,[ecx+0F8h]` → first CLogin member `m_pConnectionDlg` @0xF8) and by the `CMapLoadable` base ctor.
2. **v72 `CMapLoadable` == v79 `CMapLoadable`.** The two base ctors (`sub_5EADAB` v72 / `sub_60975A` v79) are **byte-for-byte identical** and both place the first derived member at 0xF8. There is **no v72-specific base shrink**. The base does not differ between v72 and v79 at all.
3. **The `0xF8` vs `~0x114` discrepancy is real, but it is a header/guard *over-size* bug affecting BOTH v72 and v79 — not a v72 delta.** `common/v79_layout_guards.h` asserts `sizeof(CMapLoadable)==0x114`; `struct-verification.md` lines 104/519 triangulate 0x114. Both are wrong by **0x1C**. True size is **0xF8**. This is exactly the "assert locks a triangulated-but-unanchored value" drift trap that motivated task-008/009 — the 0x114 was never alloc/method-anchored ("tail not independently anchored (Task 15)").
4. **The CLogin `0x23C` (v72) vs `0x258` (v79) size delta (Δ=0x1C) lives entirely inside CLogin's *own* members, not the base.**
5. **`m_bRecommandWorld` is absent in v72 — and also absent in v79.** So it is a real member delta vs the v95 reference, but it is **not** a v72-vs-v79 differentiator, and it is a **`CLogin.h`** field (not `CMapLoadable`).

---

## Base chain (v72), confirmed offsets

| Class | Range | Size | Anchor |
| --- | --- | --- | --- |
| `CStage` (IGObj@0, IUIMsgHandler@4, INetMsgHandler@8, ZRefCounted@0xC) | 0x00–0x18 | 0x18 | `CMapLoadable` ctor calls CStage ctor `sub_468D66` (v72)/`sub_468CA3` (v79); writes 4 vtables at [0],[4],[8],[0xC]. Unchanged vs header. |
| `CMapLoadable` own members | 0x18–0xF8 | **0xF8 total** | base ctor `sub_5EADAB`; last field-init `mov [esi+0F0h]` @0x5eaec2. |
| `CLogin` own members | 0xF8–0x23C | (CLogin=0x23C) | `m_pConnectionDlg`@0xF8; last member `m_aFemaleItem[9]` eh-vector @0x218 → 0x23C. |

CStage(0x18) ⊂ CMapLoadable(0xF8) ⊂ CLogin(0x23C).

---

## Evidence — CMapLoadable size = 0xF8

**Base ctors byte-identical (v72 `sub_5EADAB` @0x5EADAB / v79 `sub_60975A` @0x60975A):** same field-init sequence, same immediates (`push 1Fh`, `push 64h`, `push 18h`), same last field-init `mov [esi+0F0h], edi`, then the four CStage vtables at [0]/[4]/[8]/[0xC]. Identical → base identical between versions.

**First derived member anchors the base end at 0xF8:**
- `CLogin::RemoveNoticeConnecting` v72 @0x5B3ED9 — `mov ecx,[ecx+0F8h]; mov eax,[ecx]; push 3; call [eax+34h]` → virtual call through `m_pConnectionDlg` (ZRef<CConnectionNoticeDlg>) at **0xF8**.
- v79 @0x5CEDD3 — identical `mov ecx,[ecx+0F8h]` → **0xF8** in v79 too.
- Both `CLogin` ctors (`??0CLogin` v72 @0x5AECED / v79 @0x5C93E7) begin their own-member zero-init at `mov [esi+0F8h], ebx` — first write above the base ctor call.

Because the concrete `CLogin` is alloc-anchored (v72 0x23C: `m_aFemaleItem[9]` eh-vector @0x218; v79 0x258: @0x234) and its first own member sits at 0xF8, the base is 0xF8, not 0x114.

---

## Evidence — CLogin VAC prefix; `m_bRecommandWorld` absent

`CLogin::ResetVAC` v72 @0x5B4408 and `CLogin::MakeVACDlg` v72 @0x5B4394 fix the VAC-region offsets exactly:

| v72 offset | field | evidence |
| --- | --- | --- |
| 0xF8 | `m_pConnectionDlg` (ZRef) | RemoveNoticeConnecting `[ecx+0F8h]` virtual call |
| 0xFC | `m_bIsWaitingVAC` | MakeVACDlg `and [esi+0FCh], 0` |
| 0x100 | `m_bIsVACDlgOn` | MakeVACDlg `mov dword [esi+100h], 1` (dialog-on flag) |
| 0x104 | `m_tSentTimeVACPacket` | ResetVAC clears [esi+104h] |
| 0x108 | `m_nCountRelatedSvrs` | ResetVAC clears [esi+108h] |
| 0x10C | `m_nCountCharacters` | ResetVAC clears [esi+10Ch] |
| 0x110 | `m_nCountDataReceivedCharacters` | ResetVAC clears [esi+110h] |
| 0x114 | `m_aAvatarDataVAC` (ZArray<AvatarData>) | ResetVAC `lea ecx,[esi+114h]; call ZArray<AvatarData>::RemoveAll` |
| 0x118 | `m_aRankVAC` (ZArray<RANK>) | ResetVAC `[esi+118h]` ZArray<RANK>::RemoveAll |
| 0x11C/0x120/0x124 | `m_adwCharacterID`/`m_asCharacterName`/`m_anWorldID` | CLogin ctor unwind funclets (ZArray<ulong>/<ZXString>/<long>) |
| 0x128/0x130/0x138 | `m_lock_Avatar`/`m_lock_CountSvr`/`m_lock_Character` (ZFatalSection ×3, 8B each) | ResetVAC ZSynchronizedHelper on 0x128 & 0x130; ctor nullsub funclets |

The scalar VAC prefix is **6 dwords** (0xFC–0x110). The header (`CLogin.h`) declares **7** GMS-conditional VAC scalars (`m_bIsWaitingVAC … m_bRecommandWorld`). `m_aAvatarDataVAC` sits directly at 0x114 with no gap after `m_nCountDataReceivedCharacters`@0x110 → **`m_bRecommandWorld` is absent**. `m_bIsVACDlgOn` is confirmed present (MakeVACDlg sets it =1), so it is specifically `m_bRecommandWorld` that is missing, not `m_bIsVACDlgOn`.

The v72 and v79 CLogin ctors are byte-identical through 0x170 (same 6-scalar VAC prefix, same `m_aAvatarDataVAC`@0x114), so **`m_bRecommandWorld` is absent in v79 as well.** It is present in the v95 reference (@0x168). Introduction version is somewhere in (79, 95]; from the two IDBs I can probe, the provable boundary is `>= 95`. (84/87 not probed — flagged below.)

---

## CLogin internal Δ (v72 lacks vs v79) — for context, NOT the base

Both CLogin ctors identical through 0x170; divergence begins at 0x174 (from ctor field-inits + unwind funclets):

| member | v79 | v72 | note |
| --- | --- | --- | --- |
| `m_abOnFamily` (ZArray<int>, 4B) | @0x174 (funclet `sub_57407A`) | **absent** | v72's 0x174 is the ZList instead |
| `m_lNewEquip` (ZList, 0x14) | @0x178 | @0x174 | present both |
| `m_lNewEquip2` (2nd ZList, 0x14) | @0x18C | **absent** | v79 has TWO sibling ZLists; v72 has one |
| tail dword near `m_aBalloon`/`m_aCmd` (4B) | extra @0x1E0 (`m_aBalloon`@0x1DC → `m_aCmd`@0x1E4) | absent (`m_aBalloon`@0x1C4 → `m_aCmd`@0x1C8) | 1 dword |

4 + 0x14 + 4 = **0x1C** = exactly the CLogin size delta (0x258 − 0x23C). All three are CLogin members; the base is untouched.

---

## Running-delta map (v95 → v72), CMapLoadable

| region | v95 | v72 | Δ | cause |
| --- | --- | --- | --- | --- |
| CStage base | 0x00–0x18 | 0x00–0x18 | 0 | identical |
| front members (JukeBox…PropField) | 0x18–0x34 | 0x18–0x34 | 0 | identical (`m_tNextMusic`@0x1C confirmed, PrepareNextBGM `[esi+1Ch]`) |
| `m_bField` (>=95) | 0x34 | absent | −4 | already gated `>=95` |
| space/layer/list/map block | 0x38–0x100 | ~0x34–0xE8 | drifts | `m_lVisibleByQuest`(>=84, 0x14), `m_lpLayerLetterBox`(>=95, 0x14) gated; ZMaps present but slid up |
| tail (MagLevel/ViewRange/Obstacle/RestoreBGM) | 0x114–0x140 (0x2C) | ~0xE8–0xF8 (0x10) | −0x1C | **~7 tail dwords absent in v72/v79 beyond currently-gated fields** |
| `m_bPlayHoldedBGM`/`m_tPlayHoldedBGM` (>=95) | 0x140 | absent | −8 | already gated `>=95` |
| **total** | **0x148** | **0xF8** | **−0x50** | current header only gates −0x34 → 0x114; missing another −0x1C |

The header currently removes only 0x34 (`m_bField`+`m_lVisibleByQuest`+`m_lpLayerLetterBox`+PlayHolded pair) from 0x148 → 0x114. The binary is 0xF8, so **an additional 0x1C of tail is absent in v72 *and* v79** that the header does not gate.

---

## Per-field verdict (CMapLoadable)

| v95 field | v72 verdict | evidence |
| --- | --- | --- |
| CStage base (0x00–0x18) | same | base ctor writes 4 vtables; CStage ctor sub_468D66 |
| `m_nJukeBoxItemID`…`m_pPropField` (0x18–0x30) | same | base ctor `[esi+18h/20h/24h/28h(ZXString)/2Ch/30h]` |
| `m_bField` (0x34) | absent (already `>=95`) | base ctor skips 0x34 (30h→38h) |
| space/layer/lists/maps (0x38–0xE8) | present, slid | base ctor ZList vtable `off_9D01D8`@0x3C, ZMap vtables @0xA0/0xB8/0xD0 |
| `m_lVisibleByQuest` (>=84) | absent (already `>=84`) | not in base ctor |
| `m_lpLayerLetterBox` (>=95) | absent (already `>=95`) | not in base ctor |
| tail: `m_nMagLevel_*`,`m_rcViewRange`,`m_bSysOptTremble`,`m_bMagLevelModifying`,`m_aObstacleInfo`,`m_tRestoreBgmVolume`,`m_nRestoreBgmVolume` | **partially absent — ~0x1C worth gone** | base ctor init reaches only `[esi+0EСh],[esi+0F0h]`; only ~4 tail dwords (0xE8–0xF8) survive vs v95's 11 |
| `m_bPlayHoldedBGM`/`m_tPlayHoldedBGM` (>=95) | absent (already `>=95`) | not in base ctor |

**Exact identities of the 7 absent tail dwords are NOT individually byte-pinned** — the ctor leaves the tail-int region (0xD8–0xEC) uninitialized, so the constructor alone cannot label them, and no derived-class anchor constrains them. This is a best-lower-bound: total tail shrink = 0x1C (proven by 0xF8 vs 0x114); per-member split is an open question (see below).

---

## Proposed edits

### `common/CMapLoadable.h`
Add an additional `>= 84` tail gate removing 0x1C so GMS `< 84` (both v72 and v79) sums to 0xF8. Candidate block (VERIFY member identities before applying — the *size* is firm, the *which-members* is not):

```cpp
    int m_nMagLevel_Obj;
    int m_nMagLevel_Back;
    tagRECT m_rcViewRange;
    int m_bSysOptTremble;
    int m_bMagLevelModifying;
    ZArray<CMapLoadable::OBSTACLE_INFO> m_aObstacleInfo;
    int m_tRestoreBgmVolume;
    int m_nRestoreBgmVolume;
```
→ wrap the subset totalling 0x1C (7 dwords) in `#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 84) || defined(REGION_JMS)`. Do a focused tail-accessor pass (below) to pick the exact 7 before committing.

### `common/CLogin.h`
`m_bRecommandWorld` (line 197–199) is currently `#if defined(REGION_GMS)` (all GMS). It is absent in v72 AND v79. Re-gate:
```cpp
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
    int m_bRecommandWorld;
#endif
```
(`>= 95` is the provable boundary from the v72/v79/v95 evidence; confirm against v84/v87 IDBs — it may lower to `>= 84`/`>= 87`.) Note: this also corrects v79, whose current header wrongly includes it.

### `common/v72_layout_guards.h` (new file)
```cpp
#if defined(REGION_GMS) && BUILD_MAJOR_VERSION == 72
assert_size(sizeof(CStage), 0x18);        // CMapLoadable ctor calls CStage ctor sub_468D66; 4 base vtables
assert_size(sizeof(CMapLoadable), 0xF8);  // base ctor sub_5EADAB; CLogin::m_pConnectionDlg @0xF8 (RemoveNoticeConnecting 0x5B3ED9)
static_assert(offsetof(CLogin, m_pConnectionDlg) == 0xF8,
              "CLogin base subobject (CMapLoadable) must end at 0xF8 (v72: RemoveNoticeConnecting [ecx+0F8h])");
assert_size(sizeof(CLogin), 0x23C);       // m_aFemaleItem[9] eh-vector @0x218 -> 0x23C (ctor 0x5AECED)
static_assert(offsetof(CLogin, m_aAvatarDataVAC) == 0x114,
              "v72: ResetVAC 0x5B4408 lea [esi+114h] ZArray<AvatarData>::RemoveAll");
#endif
```

### `common/v79_layout_guards.h` (correction — drift)
`assert_size(sizeof(CMapLoadable), 0x114)` is **wrong** — v79 base ctor `sub_60975A` + v79 `RemoveNoticeConnecting` (`[ecx+0F8h]`) prove **0xF8**. Change to `0xF8` (and correspondingly re-gate the CMapLoadable tail + `m_bRecommandWorld` for v79). This is a genuine v79 drift the current assert masks by locking the triangulated value.

---

## Inferred UDT size deltas

| UDT | v95 size | v72/v79 size | evidence |
| --- | --- | --- | --- |
| `CMapLoadable` | 0x148 | **0xF8** | base ctor + `m_pConnectionDlg`@0xF8; identical v72==v79 |
| `CLogin` | 0x2C8 | 0x23C (v72) / 0x258 (v79) | alloc-anchored ctor extents |
| `CStage` | 0x18 | 0x18 | unchanged |

---

## Open questions / recommended follow-up

1. **Exact identity of the 0x1C absent CMapLoadable tail (7 dwords).** Firm that 0x1C is gone; not firm which members. Disassemble a v72 tail accessor — e.g. a SysOpt-tremble / magnification-level / RestoreBGM-volume handler that touches `[esi+0E8h..0F4h]` — and compare to v79 to name them. `RestoreBGM`@0x5F37DE only touched `m_sChangedBgmUOL`@0x28, so it does not help; try `SetObjectState`@0x5F4087, `LoadMap`@0x5EB448, or a weather/tremble handler.
2. **Introduction version of `m_bRecommandWorld`.** Absent ≤79, present @95. Probe v84 (session on port 13345) / v87 (13343) to pick `>= 84` vs `>= 87` vs `>= 95`.
3. **`struct-verification.md` / task-006 cross-version numbers (v79=0x114, v84=0x128) rest on the same unanchored triangulation** now shown wrong for v79. Re-anchor v84 CMapLoadable directly before trusting 0x128.

## Tool calls used
`idb_list`, `server_health` (eb2a156e); `list_funcs` (v72 CMapLoadable/CLogin, v79 CLogin filters); `disasm` v72: CLogin ctor 0x5AECED, base ctor 0x5EADAB, ResetVAC 0x5B4408, MakeVACDlg 0x5B4394, RemoveNoticeConnecting 0x5B3ED9, RestoreBGM 0x5F37DE; `disasm` v79: CLogin ctor 0x5C93E7, base ctor 0x60975A, RemoveNoticeConnecting 0x5CEDD3. No decompile; no `analyze_struct_detailed`.
