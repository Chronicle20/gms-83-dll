# GMS v61.1 — Struct Size/Layout Verification & Below-Floor Gate Audit

Scope: verify the version-gated `common/*.h` headers against the v61 binary and amend every
enumerated gate that has no catch-all `#else` so v61 selects a defined, measured branch.

IDB lane discipline (R9): v61 = port 13344 `GMS_v61.1_U_DEVM.exe` (active confirmed via
`list_instances`); v48 = port 13345 DISTRACTOR (never confuse); v72 = 13343. Read-only `disasm`
only; no struct types applied to any IDB (R10). v61 retains full mangled C++ symbols.

## Task 3 (Category A) — early gate restoration record

Live Category-A sites re-grepped (`grep -rn "BUILD_MAJOR_VERSION ==" common/*.h`):
`CWvsApp.h:98`, `CFuncKeyMappedMan.h:52`, `CLogin.h:235`. Other `== N` hits — `CLogo.h:93`
(==95), `CConfig.h:84` (==95), `SecondaryStat.h:405/419` (==87||JMS) — are upper-version member
gates **false for v61** that resolve onto the base/excluded branch (Category C, later tasks),
NOT amended here.

### Per-header verdict log

| Header | v61 size | v61 vs v72 | Gate verdict | Deciding v61 evidence |
|---|---|---|---|---|
| CWvsApp.h | **0x58** (88) | **DIVERGES**: v61 0x58 vs v72 0x60 (−0x8) | **branch-added + member-gate** (new `#elif ==61` 0x58 :103; gated `m_tLastServerIPCheck2`+`m_tLastGGHookingAPICheck` behind `>= 72`) | Two size anchors: (a) WinMain stack-ctor `lea ecx,[ebp+var_CC]` @0x8207B3 → object base var_CC(frame 0x330); first post-object local var_74(0x388) ⇒ Δ=**0x58**. (b) `m_ahInput` @**+0x4C** via TWO independent reads — `InitializeInput` 0x8247F9 `add esi,4Ch; push esi` → `CInputSystem::Init(HWND,void**)`, and `Run` 0x823411 `lea eax,[esi+4Ch]` → input poll. Field pins: `m_nTargetVersion`@+0x40 (Run 0x82355A `push [esi+40h]`); `m_tLastServerIPCheck`@+0x44 (CallUpdate 0x82493B/0x8249D3 — getpeername + 30000ms server-IP recheck timer). Only ONE int slot at +0x48 between +0x44 and m_ahInput@+0x4C ⇒ it is the unconditional `m_tLastSecurityCheck` (forced — it cannot be dropped); the two GMS GameGuard-era timers `m_tLastServerIPCheck2`/`m_tLastGGHookingAPICheck` are ABSENT in v61 (added v72). ctor 0x822E44 field-init extent reaches +0x38 (== v72/v79). |
| CFuncKeyMappedMan.h | **0x388** (904) | == v72 (both 0x388) | **branch-added** (`==61` joined the `(==72\|\|==79)` 0x388 assert :52) | Independent re-derivation (not just Task 2 trust): `TSingleton<CFuncKeyMappedMan>::CreateInstance` @0x826038 `push 388h` → `ZAllocEx::Alloc` → ctor 0x51AA0E ⇒ sizeof = **0x388**. Member gate :19 (`>= 83 \|\| JMS`) excludes v61 → quickslot pair absent; :25 (`>= 111`)/:31 (`>= 95`) excluded → header computes 0x388 for v61. Assert compiles & fires (build OK). |
| CLogin.h | 0x1D4+ (one-ZList shape) | == v72 (one ZList; v72 0x23C) | **unchanged** (no edit; `==83` correctly excludes v61) | `CLogin::CLogin` @0x5620D4 builds exactly ONE ZList-vtable object (`off_8E8C54` @this+0x16C = `m_lNewEquip`), then `m_aCmd[5]` ZXString vector-ctor @+0x1C0. NO second ZList ⇒ the v83-only `unk3` ZList (header `int unk3[5]`, 0x14) is ABSENT. Matches v72/v79 (one ZList). `==83` gate excludes v61 correctly. |

(A5: each branch assignment is an **evidence verdict** from measured size, not "oldest ⇒ v83".)

### Edits applied (house `#if/#elif/#endif` form; one-line `task-010` comment each)

- `common/CWvsApp.h` — (1) member-gated `m_tLastServerIPCheck2` + `m_tLastGGHookingAPICheck`
  behind `#if BUILD_MAJOR_VERSION >= 72` (absent in v61; the 0x8 shrink); (2) added
  `#elif … == 61 → assert_size(sizeof(CWvsApp), 0x58)`. The member gate is **required for hook
  correctness** (see below), not just to satisfy the assert.
- `common/CFuncKeyMappedMan.h` — `(==72||==79)` → `(==61||==72||==79)`, asserting `0x388`;
  refreshed the `:16` member-gate comment.
- `common/CLogin.h` — **no edit** (`==83` correctly excludes v61; `unk3[5]` absent).
- `bypass/app_hooks.cpp` — downstream consumer: gated the two `m_tLast…` writes in
  `CWvsApp__CallUpdate_Hook` behind `>= 72` to mirror the header (else v61 fails to compile, and
  more importantly would write to wrong offsets on the real 0x58 object).

### Why the CWvsApp member gate is mandatory (not deferrable to the audit)

CWvsApp is **stack-constructed by the real v61 client's WinMain** (0x58 bytes); our hooks
(`CallUpdate`, `SetUp`, …) receive that `pThis` and access fields **by header offset**. If the
header modelled 0x60 (the two extra ints present), `m_tLastSecurityCheck` and `m_ahInput` would
resolve to +0x50/+0x54 instead of the real v61 +0x48/+0x4C → our hooks would corrupt the object.
So modelling v61's true layout (member gate → 0x58) is a correctness requirement; the matching
size assert just locks it. (Contrast the v79 CFuncKeyMappedMan deferral, where no hook indexed the
shifted region.)

### FR-13 cross-version truth table — preprocessor-selected branch (verbatim from `gcc -E`)

| Version | CWvsApp.h | Changed? | CFuncKeyMappedMan.h | Changed? | CLogin.h unk3[5] | Changed? |
|---|---|---|---|---|---|---|
| GMS 61 | 0x58 | **NEW (was none)** | 0x388 | **NEW (was none)** | absent | unchanged |
| GMS 72 | 0x60 | unchanged | 0x388 | unchanged | absent | unchanged |
| GMS 79 | 0x60 | unchanged | 0x388 | unchanged | absent | unchanged |
| GMS 83 | 0x60 | unchanged | 0x3C8 | unchanged | **present** | unchanged |
| GMS 84 | 0x60 | unchanged | 0x3C8 | unchanged | absent | unchanged |
| GMS 87 | 0x6C | unchanged | 0x3C8 | unchanged | absent | unchanged |
| GMS 95 | 0x8C | unchanged | 0x3CC | unchanged | absent | unchanged |
| GMS 111 | 0x8C | unchanged | 0x3D0 | unchanged | absent | unchanged |
| JMS 185 | 0x64 | unchanged | 0x400 | unchanged | absent | unchanged |

`== 61` and `>= 72` are disjoint from every other supported version's selector (no supported GMS
version equals 61, and 72/79/83/84/87/95/111 all satisfy `>= 72`; JMS is `REGION_JMS`-gated, the
GMS-only ints never apply). v61 moves from "no branch selected (silent guard loss / silent member
shift)" to a defined, measured layout + assert. **v111 IDB not loaded** — validated from build
constants (`>= 95` → 0x8C, `== 111` → 0x3D0); unaffected because the v61 edits are strictly
additive/disjoint and never touch a `>= 95`/`== 111` selector.

### Empirical verification (exactly what was run)

- `scripts/wsl-build.sh GMS 61 1` → `>> OK` (compiles **and links** all edit DLLs) — the now-selected
  `assert_size(...,0x58)` and `assert_size(...,0x388)` **fire and hold** against the real header
  layout (the member gate makes the header compute 0x58).
- `scripts/wsl-build.sh GMS 72 1` → `>> OK` — neighbor, no regression.
- `gcc -E -DREGION_{GMS,JMS} -DBUILD_MAJOR_VERSION={61,72,79,83,84,87,95,111,185}` on each amended
  header → the full matrix above (selected `static_assert` size + `unk3[5]` presence per version).

**Finding:** v61 was not previously *blocked* from compiling CWvsApp.h/CFuncKeyMappedMan.h (the
gates had no `#else`, so v61 silently selected no `assert_size`). The amendment restores the guard.
The genuinely load-bearing discovery is the CWvsApp **0x58 below-floor member shift** (−2 GMS
security ints vs v72), which both sizes the object and fixes the offsets our hooks use on v61.

## Task 12 (Category B) audit record — the five `>= 83 || JMS` below-floor gates (confirm-or-split)

Re-grepped live (`grep -rn "BUILD_MAJOR_VERSION >= 83" common/*.h`): exactly the five sites
`CWnd.h:25`, `CFuncKeyMappedMan.h:19`, `CMob.h:241`, `CUIToolTip.h:92` (parenthesised
`(>=83)||JMS` form), `MobStat.h:128`. `grep -rn "BUILD_MAJOR_VERSION < 79"` and `< 72` →
**ZERO** arms ⇒ a v61 split is **two-way → three-way** (add a GMS-guarded `< 72` arm ABOVE the
unchanged `#else`; Task 17 applies). Lane: v61 = port 13344 `GMS_v61.1_U_DEVM.exe` (active
confirmed via `list_instances`; v48 distractor = 13345, never probed). Read-only `disasm` only;
no struct types applied (R10/R12). v61 retains full mangled symbols. v72 reduced sizes
re-confirmed from task-009 `struct-verification.md`: CWnd 0x64 (base subobj), CMob 0x4C0,
MobStat 0x1D8, CFuncKeyMappedMan 0x388, CUIToolTip 0x50C.

**Result: 3 confirmed-shares-v72, 2 split. The CWnd cascade is NOT triggered** (v61 CWnd ≡ v72
⇒ CDialog/CUIWnd/CFadeWnd/CUITitle/CUILoginStart inherit v72's verdict; no re-measurement forced
in Task 14).

| Header | v61 size | v61 vs v72 | Gated field present? | Gate verdict | Deciding v61 evidence |
|---|---|---|---|---|---|
| CWnd.h:25 | **0x64** (base subobject; ctor writes through 0x70) | **== v72** (0x64) | `m_pAnimationLayer`/`m_pOverlabLayer` **ABSENT** | **confirmed-shares-v72** (CASCADE ROOT — not triggered) | 3 landmarks all == v72: (a) ctor `CWnd::CWnd` @0x4BB456 writes `[esi+64h]=[esi+68h]=[esi+70h]=0` + 3 vtables (highest write 0x70); (b) base helper sub_811225 `memset(this+8,0,0xC)` then zeros 0x14,0x18,[**skips 0x1C/0x20**],0x38,0x3C,ZList sentinel@0x48,0x50–0x60, ZArray<IWzCanvas>@0x60 unwind — field-for-field identical to v72 sub_8DD47E, anim layers absent; (c) `CWnd::Destroy` @0x812F29 touches m_pLayer@0x18 / m_lpChildren@0x48 / m_pFocusChild@0x5C (== v72). |
| CMob.h:241 | **0x490** (1168) | **DIVERGES**: 0x490 vs v72 0x4C0 (**−0x30**); v83 0x548 | doom tail (`m_bDoomReserved`/SN/`m_lpStatChangeReserved`, begins ~0x528 in v83) **ABSENT** | **split** (three-way; size diverges, doom-tail field shared-absent with v72) | `CreateMob` @0x5C20EC `push 490h` → ZAllocEx::Alloc ⇒ sizeof=0x490; ctor `CMob::CMob` @0x5C2128 highest field write `[esi+47Ch]` (+ unwind sub-object @+0x478) — within 0x490. Doom tail past end of 0x490 object ⇒ absent (as v72/v79). MobStat embedded @CMob+0x170 (`lea ebx,[esi+170h]` → `MobStat::SetFrom`; v72 embed @0x178, −0x8). Boundary re-anchor: Alloc(0x490) and ctor highest write 0x47C agree. |
| MobStat.h:128 | **0x1B8** (440) | **DIVERGES**: 0x1B8 vs v72 0x1D8 (**−0x20**); v79 0x1F8, v83 0x208 | Weakness group (`nWeakness_/rWeakness_/tWeakness_`) **ABSENT** | **split** (three-way; size diverges, Weakness field shared-absent with v72) | `MobStat::SetFrom` @0x6685A7 `push 1B8h` memset (=sizeof); CMob ctor lBurnedInfo ZList @MobStat+0x1A4 (`lea eax,[ebx+1A4h]`, ebx=MobStat base; unwind funclet `add ecx,1A4h`) +0x14 (ZList) = 0x1B8. Two independent anchors agree. v72 lBurnedInfo @MobStat+0x1C4 (+0x14=0x1D8). Weakness (`>=83`) past end of both ⇒ shared-absent. |
| CFuncKeyMappedMan.h:19 | **0x388** (904) | **== v72** (0x388) | quickslot pair (`m_aQuickslotKeyMapped[8]`×2) **ABSENT** | **confirmed-shares-v72** (Cat-B finalized; Task 3 size-assert branch `==61` consistent) | `TSingleton<CFuncKeyMappedMan>::CreateInstance` @0x826038 `push 388h`→Alloc→ctor 0x51AA0E ⇒ sizeof=0x388. ctor: vtable@0, memcpy m_aFuncKeyMapped[89]@+4 (0x1BD), memcpy m_aFuncKeyMapped_Old[89]@+0x1C1 (0x1BD, ends 0x37E), pet ints `and [esi+380h],0`/`and [esi+384h],0` (highest write 0x384) ⇒ 0x388. Pet ints sit **immediately after** the two FK arrays with **no 0x40 quickslot gap** ⇒ quickslot pair absent (== v72 layout exactly). |
| CUIToolTip.h:92 | **0x50C** (1292) | **== v72** (0x50C; ctor layer region byte-identical) | `m_pLayerAdditional` **ABSENT** | **confirmed-shares-v72** | ctor `CUIToolTip::CUIToolTip` @0x748FFE: `mov [edi+10h],ebx` (m_pLayer@0x10) then eh-vector-ctor over `[edi+20h]` count 0x20 element 0x20 (m_aLineInfo[32]@0x20), then post-array zeroing from `[edi+424h]`. m_pLayer@0x10 → 0x14/0x18/0x1C (m_nLastX/Y/m_nLineNo) → m_aLineInfo@0x20 with **no slot at 0x14-region for m_pLayerAdditional** (its presence would shift the array to 0x24). Byte-identical to v72 ctor 0x7F9C33 ⇒ == v72 (0x50C); m_pLayerAdditional absent. |

### CWnd cascade decision (R5, design §5.7)
v61 `sizeof(CWnd)` base subobject = **0x64 == v72** (anim-layer pair `>=83||JMS` absent in both;
three independent landmarks above). **Cascade NOT triggered** → the five CWnd-derived classes
(CDialog, CUIWnd, CFadeWnd, CUITitle, CUILoginStart) inherit v72's verdict and are **not** flagged
for re-derivation in Task 14 (each still independently size-confirmed there per D5). The CWnd.h
`>=83||JMS` gate already excludes v61 correctly — no split needed.

### Split dispositions (Task 17 applies; D8 — two-way → three-way)
Both splits diverge by **size**, not by the gated field itself (doom tail / Weakness are
shared-absent with v72, stay `#else`). Neither CMob.h nor MobStat.h carries an `assert_size`, so
the split is about modelling v61's smaller true layout (the extra v61-absent members below the
gated tail) + adding a v61 size guard — NOT about the `:241`/`:128` field gates. Per D8, Task 17
adds a GMS-guarded `BUILD_MAJOR_VERSION < 72` arm ABOVE the unchanged `#else`, leaving the v72/v79
branch byte-identical in effect. The exact v61-absent members (the −0x30 CMob front/base+tail and
the −0x20 MobStat status region) are pinned in the Cat-C/struct tasks (Tasks 14/15), as in
task-009's CMob/MobStat split records. Drives the doom-fix gate: the `==83` doom write stays
correctly excluded for v61 (the write would be OOB on a 0x490 object).

### Cross-version safety (FR-13)
No source edits in this evidence task. The 3 confirmed-shares-v72 gates need no change — v72's
`#else` already serves v61; v72/v79/v83/v84/v87/v95/v111/JMS185 untouched. The 2 splits, when
applied in Task 17, add a `< 72` term that no supported version except v61 satisfies (disjoint),
keeping every other version's selected branch unchanged.
