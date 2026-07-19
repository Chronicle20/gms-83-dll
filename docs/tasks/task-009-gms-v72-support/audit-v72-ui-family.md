# GMS v72 UI-family layout audit (6 structs) — vs corrected v79 guards

**Target IDB (verified FIRST):** `GMS_v72.1_U_DEVM.exe`, session `eb2a156e`, MD5
`05a62ca755b1d3719223426b4eee41a9`, base 0x400000 (confirmed via `survey_binary`; matches dispatch).
**Cross-ref IDB:** `GMS_v79_1_DEVM.exe`, session `88dfa464`.
**Method:** read-only `disasm` of allocators (`push <size>`→`ZAllocEx::Alloc`) and ctors in both
IDBs; no struct types applied (no decompiler leak). Offsets from raw `[reg+OFFh]`.

## TL;DR — the crux is resolved against the binary

- **CUIToolTip: v72 = 0x510, v79 = 0x514 (Δ −4).** The pre-existing v72 claim "both v72 and v79
  are 0x50C" is **WRONG on both counts**. The v79 guard's **0x514 is CORRECT**. v72 is **0x510**,
  not 0x50C — the old 0x50C came from subtracting a *nonexistent* `m_bSelfDisable` from the
  CCtrlButton allocation.
- The v72/v79 ctors are **NOT byte-identical** (old claim): v79's zero-init font run is one dword
  longer (20 vs 19 fonts) — v72 lacks exactly one always-present Gen font (binary offset 0x470).
- **CCtrlButton: v72 = 0x59C, v79 = 0x5A0.** The v79 guard **0x5A4 is WRONG (+4)** — a phantom
  `m_bSelfDisable`. Three v79 allocation sites push 0x5A0.
- **CFadeWnd: v72 = 0xCC (== v79), NOT ≈0xC4.** Old audit misread `m_sInviter@0xC0` as
  `m_dwFriendID`; the real `m_dwFriendID@0xC8` is written by `CreateFriendReg`.
- CCtrlWnd 0x34, CCtrlCheckBox 0x6C, CUIWnd 0x5A4 — pinned.

## Per-struct table

| Struct | v72 size | v79 size (real) | v79 guard | v72 vs v79 | Binary anchor (v72) | Confidence |
|---|---|---|---|---|---|---|
| CCtrlWnd (base) | **0x34** | 0x34 | 0x34 ✓ | same | ctor `0x4CC645` == v79 `0x4D4378` byte-identical; flags int @0x28/2C/30 | FIRM |
| CUIToolTip | **0x510** | 0x514 | 0x514 ✓ | **−4** | ctor `0x7F9C33` last member @0x50C; +3 embed cross-checks | FIRM (×4) |
| CCtrlButton | **0x59C** | 0x5A0 | 0x5A4 ✗ | **−4** | `CUIFadeYesNo::OnCreate 0x500921` `push 59Ch`→ctor `0x422954` | FIRM |
| CCtrlCheckBox | **0x6C** | 0x6C | 0x6C ✓ | same | base 0x34 (confirmed) + members all `<95`; ctor stripped | FIRM (arith) |
| CUIWnd | **0x5A4** | 0x5A8 | 0x5A8 ✓ | **−4** | ctor `sub_83C0EC`: m_uiToolTip@0x6C, m_nUIType@0x57C, m_sBackgrndUOL@0x5A0 | FIRM |
| CFadeWnd | **0xCC** | 0xCC | 0xCC ✓ | same | ctor `0x4FFD72` == v79 `0x50B6D6`; `CreateFriendReg` m_dwFriendID@0xC8 | FIRM |

## Evidence detail

### CUIToolTip = 0x510 (v72) vs 0x514 (v79) — THE crux
Both ctors: `mov [esi+10h]` (m_pLayer), then `eh vector ctor` over `[esi+20h]` count 0x20 elem
0x20 (m_aLineInfo[32], ends 0x420). Then a run of zeroed font dwords:
- **v72** `0x7F9C33`: zeroes 0x424..0x46C = **19 dwords**, first canvas ZArray @0x470; final member
  writes `[esi+504h]`,`[esi+508h]`,`[esi+50Ch]` (m_bIngoreWeddingInfo@0x50C) → **sizeof 0x510**.
- **v79** `0x842317`: zeroes 0x424..**0x470** = **20 dwords** (one extra), first canvas ZArray
  @0x474; final member `[esi+510h]` → **sizeof 0x514**.

Every offset ≥ 0x470 is uniformly +4 in v79. The extra member is one always-present font at binary
0x470 (the last of the HL/Gen block, i.e. `m_pFontGen_Blue` — identity inferred from the +4 boundary
position + the historical append pattern; not byte-pinned to a specific `PcCreateObject`, but the
**−4 size effect is firm**).

v72 = 0x510 is quadruple-anchored:
1. own ctor last member @0x50C.
2. CUIWnd embed: `sub_83C0EC` m_uiToolTip@0x6C, next member (m_nUIType) @0x57C → 0x57C−0x6C = 0x510.
3. CCtrlButton embed: alloc 0x59C, m_uiToolTip@0x8C → 0x59C−0x8C = 0x510.
4. `sub_46284F` embed: m_uiToolTip@0x25C, next member @0x76C → 0x76C−0x25C = 0x510.

Note the `vfptr` offset-0 slot: `m_pLayer@0x10` in **both** v72 and v79 (4 dwords precede it). The
header only reserves that slot under `== 79`, so **v72 needs it too** (else m_pLayer lands at 0xC).

### CCtrlButton = 0x59C (v72) / 0x5A0 (v79) — v79 guard 0x5A4 is wrong
Both ctors (`0x422954` / `0x422D59`) are byte-identical in own layout: base ctor, m_pPropFocusFrame
@0x64, m_apPropButton eh-vector @0x6C, ZXStrings @0x84/0x88, **m_uiToolTip@0x8C** in both. So the only
size driver is the CUIToolTip embed. Allocations:
- v72 `CUIFadeYesNo::OnCreate 0x500921` `push 59Ch` → `0x8C + 0x510 = 0x59C`. No room for m_bSelfDisable.
- v79 `CUIFadeYesNo::OnCreate 0x50C293` `push 5A0h` AND `sub_46AD5C 0x46AEA7` `push 5A0h`
  → `0x8C + 0x514 = 0x5A0`. Also no room for m_bSelfDisable.

⇒ `m_bSelfDisable` is **absent in both v72 and v79**. The header carries it ungated, so the v79 header
computes 0x5A4 and the v79 guard asserts 0x5A4 (self-consistent, so the v79 build passes) but the
**binary is 0x5A0**. The member is a v95-PDB artifact; its true introduction version between 80–95 is
**not pinned by this audit** (only ≤79 proven absent).

### CFadeWnd = 0xCC (v72 == v79) — not 0xC4
Ctors `0x4FFD72` (v72) / `0x50B6D6` (v79) are byte-identical: `CWnd::CWnd`, `or [esi+0BCh],-1`
(m_nType@0xBC = -1), zero [esi+0A4h]/[esi+0B0h]/**[esi+0C0h]** (m_sInviter — the old audit misread
this as m_dwFriendID), vtables. The `>=87||JMS` level/job/exp block is absent in both. Tail confirmed
present in v72: `CreateFriendReg` (`0x502FAB`) does `mov [esi+0C8h], eax` (friendID) ⇒ **m_dwFriendID@0xC8**;
with m_dwSN@0xC4 the struct ends **0xCC**.

### CCtrlWnd base = 0x34 (v72 == v79)
Ctors `0x4CC645` (v72) / `0x4D4378` (v79) byte-identical: 3 vtables, `or [esi+14h],-1`, and the three
trailing flags written as **4-byte ints** `mov [esi+28h]=1`, `[esi+2Ch]=1`, `[esi+30h]=1` → 0x34. The
v79 "flags are int not bool" modeling applies to v72 verbatim.

### CCtrlCheckBox = 0x6C (v72 == v79)
v72 ctor is stripped (no mangled symbol). By arithmetic: base CCtrlWnd 0x34 (binary-confirmed identical
above) + members `m_nCheckBoxState@0x34 … m_apCanvasCheckBox[4]@0x5C` (all ungated / `<95`) → 0x6C. The
`>=95` m_nTextOffsetX/Y are absent (v72<95). No divergence from v79.

### CUIWnd = 0x5A4 (v72) / 0x5A8 (v79)
Ctor `sub_83C0EC`: `CWnd` base helper `sub_8DD47E`, m_pBtClose ZRef@0x64 (unwind funclet `add ecx,64h`),
**m_uiToolTip@0x6C**, then args at [esi+57Ch..594h] (m_nUIType@0x57C, m_nBtCloseType@0x580 …), last
member zeroed `and [esi+5A0h],0` (m_sBackgrndUOL@0x5A0) → **0x5A4**. Uniformly −4 vs v79 (m_nBtCloseType
v79@0x584, m_sBackgrndUOL v79@0x5A4, size 0x5A8), driven entirely by the CUIToolTip −4.

## Gate decisions + exact header edits

Dependency/assert order (base → embedder): **CCtrlWnd → CUIToolTip → CCtrlButton, CCtrlCheckBox,
CUIWnd → CFadeWnd** (CUIWnd embeds CUIToolTip; CCtrlButton embeds CUIToolTip + derives CCtrlWnd;
CCtrlCheckBox derives CCtrlWnd; CFadeWnd derives CDialog/CWnd).

### 1. `common/CCtrlWnd.h` — v72 SHARES v79 (widen `==79`)
- Line 13 gate `... BUILD_MAJOR_VERSION == 79)` →
  `... (BUILD_MAJOR_VERSION == 72 || BUILD_MAJOR_VERSION == 79))` (int-flag modeling for v72).
- Line 29 assert gate: same widen. `assert_size(sizeof(CCtrlWnd), 0x34);`

### 2. `common/CUIToolTip.h` — v72 DIVERGES −4 (widen vfptr + gate one font + v72 assert)
- Line 91 vfptr gate `... == 79` → `... (BUILD_MAJOR_VERSION == 72 || BUILD_MAJOR_VERSION == 79)`
  (v72 also has the offset-0 slot; m_pLayer@0x10).
- Line 136 `m_pFontGen_Blue` — wrap so v72 drops it:
  ```cpp
  #if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 79) || defined(REGION_JMS)
      _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontGen_Blue;
  #endif
  ```
  (disjoint: v83/84/87/95/111 ≥79 keep it, JMS keeps it, only v72 drops it.)
- Assert block (line 171) — add a v72 arm ABOVE the ==79 arm:
  ```cpp
  #if defined(REGION_GMS) && BUILD_MAJOR_VERSION == 72
  assert_size(sizeof(CUIToolTip), 0x510); // ctor @0x7F9C33 (GMS_v72.1); 19 HL/Gen fonts (v79 has 20)
  static_assert(offsetof(CUIToolTip, m_pLayer) == 0x10, "v72 m_pLayer @0x10");
  static_assert(offsetof(CUIToolTip, m_pNumberCan) == 0x4A0, "v72 m_pNumberCan @0x4A0 (-4 vs v79)");
  #elif defined(REGION_GMS) && BUILD_MAJOR_VERSION == 79
  assert_size(sizeof(CUIToolTip), 0x514); // (unchanged — v79 guard is CORRECT)
  ...existing v79 asserts...
  #endif
  ```

### 3. `common/CCtrlButton.h` — v72 = 0x59C; also CORRECTS v79
- Gate `m_bSelfDisable` (line 35) so v72 **and** v79 drop it (both proven absent):
  ```cpp
  #if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 83) || defined(REGION_JMS)
      int m_bSelfDisable;
  #endif
  ```
  **PROVISIONAL boundary** — only ≤79 is proven absent; verify v83/84/87 before trusting `>=83`
  (member originates from the v95 PDB; true intro version 80–95 unpinned).
- Replace the ==79 assert (line 38-41) with:
  ```cpp
  #if defined(REGION_GMS) && BUILD_MAJOR_VERSION == 72
  assert_size(sizeof(CCtrlButton), 0x59C); // alloc 0x500921 push 59Ch; CUIToolTip 0x510 @0x8C, no m_bSelfDisable
  #elif defined(REGION_GMS) && BUILD_MAJOR_VERSION == 79
  assert_size(sizeof(CCtrlButton), 0x5A0); // CORRECTED from 0x5A4: allocs 0x50C293/0x46AEA7 push 5A0h
  #endif
  ```
  (If the `>=83` boundary for m_bSelfDisable is wrong, these two asserts still hold — they don't
  include it.)

### 4. `common/CCtrlCheckBox.h` — v72 SHARES v79 (widen `==79`)
- Line 22 assert gate `... == 79` → `... (BUILD_MAJOR_VERSION == 72 || BUILD_MAJOR_VERSION == 79)`.
  `assert_size(sizeof(CCtrlCheckBox), 0x6C);`

### 5. `common/CUIWnd.h` — v72 = 0x5A4 (auto-follows CUIToolTip fix; add v72 assert)
- Add a v72 arm ABOVE the ==79 arm (line 29):
  ```cpp
  #if defined(REGION_GMS) && BUILD_MAJOR_VERSION == 72
  assert_size(sizeof(CUIWnd), 0x5A4); // CWnd 0x64 + CUIToolTip 0x510 @0x6C (ctor sub_83C0EC)
  static_assert(offsetof(CUIWnd, m_nBtCloseType) == 0x580, "v72 m_nBtCloseType @0x580 (-4 vs v79)");
  static_assert(offsetof(CUIWnd, m_sBackgrndUOL) == 0x5A0, "v72 m_sBackgrndUOL @0x5A0 (-4 vs v79)");
  #elif defined(REGION_GMS) && BUILD_MAJOR_VERSION == 79
  assert_size(sizeof(CUIWnd), 0x5A8); // (unchanged — CORRECT)
  ...existing...
  #endif
  ```

### 6. `common/CFadeWnd.h` — v72 SHARES v79 (widen `==79`)
- Line 21 int-flag block gate `... == 79` → `... (BUILD_MAJOR_VERSION == 72 || BUILD_MAJOR_VERSION == 79)`.
- Line 50 assert gate: same widen. `assert_size(sizeof(CFadeWnd), 0xCC);` (m_nType@0xBC, m_dwSN@0xC4).

## Conflict resolution ledger

| Claim source | Struct | Claimed | Verdict | Correct value |
|---|---|---|---|---|
| pre-existing v72 audit | CUIToolTip | 0x50C (v72 == v79) | **WRONG** (both parts) | v72 0x510, v79 0x514 |
| pre-existing v72 audit | CUIToolTip ctor v72==v79 byte-identical | **WRONG** | v79 has +1 font dword @0x470 |
| pre-existing v72 audit | CFadeWnd | ≈0xC4 | **WRONG** (misread m_sInviter) | 0xCC (== v79) |
| pre-existing v72 audit | CCtrlButton | 0x59C (m_uiToolTip@0x8C, m_bSelfDisable@0x598, CUIToolTip 0x50C) | size right, **decomposition wrong** | 0x59C; CUIToolTip 0x510, NO m_bSelfDisable |
| pre-existing v72 audit | CUIWnd | ≈0x5A4 | **CORRECT** | 0x5A4 |
| pre-existing v72 audit | CCtrlCheckBox | base + ~0xB ints/ptrs | resolves to | 0x6C |
| pre-existing v72 audit | CCtrlWnd base | not pinned | pinned | 0x34 |
| **v79 guard** | CUIToolTip | 0x514 | **CORRECT** | 0x514 |
| **v79 guard** | CCtrlButton | 0x5A4 | **WRONG (+4)** — phantom m_bSelfDisable | 0x5A0 |
| **v79 guard** | CFadeWnd/CUIWnd/CCtrlWnd/CCtrlCheckBox | 0xCC/0x5A8/0x34/0x6C | CORRECT | unchanged |

## Open questions / caveats
- **`m_bSelfDisable` introduction version** — proven absent ≤79; present in the v95 PDB. The `>=83`
  gate above is provisional. Disassemble a v83/v84/v87 `CUIFadeYesNo::OnCreate` allocation (`push`
  immediate feeding `??0CCtrlButton@@`) to pin whether it is `0x5A0` (still absent) or larger.
- **Exact v72-absent font** — the −4 is one 4-byte pointer in the always-present HL/Gen block
  (binary 0x470 = boundary → `m_pFontGen_Blue`). Size-correct regardless; no consumer reads these
  fonts by offset. If exact identity is required, diff the two ctors' `PcCreateObject`/StringPool-ID
  sequences past instruction 140.
- **v79 guard corrections needed** (flagged, not applied): CCtrlButton 0x5A4→0x5A0; and once
  `m_bSelfDisable` is gated `>=83`, the v79 header recomputes 0x5A0 so the v79 assert MUST move in
  lockstep or the v79 build breaks.
