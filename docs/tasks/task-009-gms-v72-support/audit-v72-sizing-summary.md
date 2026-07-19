# GMS v72 sizing/gating audit — summary & applied fixes (task-009)

Triggered by play-testing issues on the task-008 (v79) work. Root cause class: the
task-008 correction fixed 8 structs whose C++ headers had silently drifted from the
binary because "verified" rows were never enforced by a compile-time `assert_size`.
Those v79 fixes landed gated `BUILD_MAJOR_VERSION == 79`, and **v72 had no compile-guard
file at all** — so v72 silently fell through to the v95-derived `#else` branches for every
one of those structs, reproducing the exact same latent-corruption trap.

All findings below are anchored to **GMS_v72.1_U_DEVM.exe** (IDA session `eb2a156e`),
cross-checked against **GMS_v79_1_DEVM.exe** (`88dfa464`). Full per-struct evidence in
`audit-v72-secondarystat.md`, `audit-v72-clogin.md`, `audit-v72-cmaploadable.md`,
`audit-v72-ui-family.md`. Every size below is now locked by a `static_assert`
(`common/v72_layout_guards.h`, included from `pch.h`) so the drift cannot silently recur.

## Applied v72 fixes

| Struct | v72 size | vs v79 | Gate change | Key anchor |
|---|---|---|---|---|
| `SecondaryStat` | **0xAB0** | −0xD8 | `GMS_V79_ABSENT` widened to `(72\|\|79)`; new `GMS_V72_ABSENT` fences 6 stat triples (EventRate…SmartKnockback) | ctor sub_6C70E9 `aTemporaryStat[7]@0xA78` |
| `CMapLoadable` (base) | **0xF8** | same binary; v72-gated only | 0x1C tail block gated `!= 72` | base ctor sub_5EADAB; `CLogin::m_pConnectionDlg@0xF8` |
| `CStage` (base) | 0x18 | same | — (locked) | CMapLoadable ctor → CStage ctor sub_468D66 |
| `CLogin` | **0x23C** | −0x1C | new `== 72` branch; `m_pConnectionDlg` 4B (not ZRef); `m_bRecommandWorld` absent | ctor @0x5AECED; ResetVAC/MakeVACDlg VAC offsets |
| `CCtrlWnd` | 0x34 | same | `== 79` → `(72\|\|79)` | ctor @0x4cc645 (== v79) |
| `CFadeWnd` | 0xCC | same | `== 79` → `(72\|\|79)` | ctor @0x4ffd72 (== v79) |
| `CCtrlCheckBox` | 0x6C | same | `== 79` → `(72\|\|79)` | CCtrlWnd base |
| `CUIToolTip` | **0x510** | −4 | vfptr gate `(72\|\|79)`; `m_pFontGen_Blue` absent for v72 | ctor @0x7f9c33 (19 fonts vs v79's 20) |
| `CCtrlButton` | **0x59C** | −4 | v72 assert; `m_bSelfDisable` gated `>= 83` | alloc @0x500921 `push 59Ch` |
| `CUIWnd` | **0x5A4** | −4 | v72 branch | ctor sub_83C0EC; ReloadBackgrnd @0x83c71e |

The three UI `−4` deltas share one cause: v72's `CUIToolTip` has one fewer base font,
which propagates through everything that embeds it.

### Two subtleties the per-struct audits under-specified (verified here against the binary)

- **`CLogin::m_pConnectionDlg` is 4 bytes in v72** (raw pointer), not the 8-byte `ZRef`
  of v79+. Confirmed: `RemoveNoticeConnecting@0x5B3ED9` reads `[this+0xF8]` and
  `MakeVACDlg@0x5B4394` writes `m_bIsWaitingVAC` at `0xFC` — 4 bytes above. Without this,
  gating out `m_bRecommandWorld` alone still lands `m_aAvatarDataVAC` at 0x118, not 0x114.
- **`CMapLoadable` 0x1C tail**: the *size* (0xF8) is binary-firm via `m_pConnectionDlg@0xF8`;
  the exact 4 absent tail members are a best-effort split (their individual identities are
  not byte-pinned — the base ctor leaves that region uninitialized). The chosen split
  (`m_rcViewRange`/`m_bSysOptTremble`/`m_bMagLevelModifying`/`m_aObstacleInfo`) is consistent
  with the ctor init reaching `[esi+0ECh]`/`[esi+0F0h]`. Flagged for a follow-up tail-accessor pass.

## Cross-version findings (NOT v72 — flagged, mostly not fixed here)

1. **v79 `CCtrlButton` was 0x5A4 in the merged task-008 guard; true size is 0x5A0.**
   The v79 alloc immediate is `push 5A0h` (@0x50c293) → `m_bSelfDisable` is a phantom
   trailing field on v79 too. **FIXED on this branch** (leaf struct, low-risk, and the
   shared `m_bSelfDisable` gate necessarily covers both v72 and v79): `v79_layout_guards.h`
   corrected 0x5A4 → 0x5A0.
2. **v79 `CMapLoadable` is also 0xF8, not the guard's 0x114.** v72 and v79 base ctors are
   byte-identical. **NOT fixed here** — v79's `CLogin` (0x258) compensates internally and
   its members are pinned to correct *absolute* offsets, so the 0x114 is a harmless
   over-attribution, not a live bug. Correcting it would cascade through v79's whole base
   chain and re-touch merged, play-tested code. Left as-is; flagged for a dedicated v79 task.
3. **`m_bSelfDisable` / `m_pFontGen_Blue` introduction versions unpinned.** Both are gated
   present for `>= 83` / `>= 79` respectively (v83+/v84+ unchanged from before), but only
   `<= 79` absence is proven. A full-matrix `assert_size` audit with alloc-immediate anchors
   is warranted for v83/84/87/95 (task-008's arithmetic-derived sizes proved unreliable
   exactly where a `push <size>` immediate exists to settle them).
4. **atlas-ms `temporary_stat.go` over-includes 6 stats for v72.** Bits 73–78 (EventRate,
   AranCombo/ComboDrain group, ComboBarrier, BodyPressure, SmartKnockback) are treated as
   all-version but the v72 client lacks them (Aran/combo-era). Recommend a `MajorVersion >=`
   gate on the atlas side (same class as `Chronicle20/atlas#564`).

## Verification

Full WSL cross-compile matrix, all `>> OK`, all v72 `assert_size`/`offsetof` guards live:
GMS 72, 79, 83, 84, 87, 95, 111, JMS 185. clang-format clean vs origin/main.
