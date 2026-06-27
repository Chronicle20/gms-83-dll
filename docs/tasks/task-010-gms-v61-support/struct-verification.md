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

## Task 13 (Category C + special) — core/net layout headers, exhaustive size

Read-only `disasm` only; no struct types applied (R10/R12). Lane re-confirmed: active IDB = port
13344 `GMS_v61.1_U_DEVM.exe` (`list_instances`; v48@13345 distractor never probed). Per **D5**
every v61 size is independently measured (no IDB below v61). v61 retains full mangled symbols.

### Step 1 — per-header gate truth table (v61)

Every enumerated upper gate is **false** for v61 (→ base/excluded branch). Confirmed truth values:

| Header:line | Gate | v61 | Effect on v61 layout |
|---|---|---|---|
| CWvsApp.h:14 | `>= 95` | F | OS-version block (`m_nOSVersion`…`m_b64BitInfo`) absent |
| CWvsApp.h:38 | `>= 72` | F | `m_tLastServerIPCheck2`+`m_tLastGGHookingAPICheck` absent (the −0x8, Task 3) |
| CWvsApp.h:45/51 | `>= 87` | F | `m_tNextSecurityCheck`, `m_pBackupBuffer`+`m_dwBackupBufferSize` absent |
| CWvsApp.h:48/55 | `>= 95` | F | `m_bEnabledDX9`, `m_dwClearStackLog`+`m_bWindowActive` absent |
| CWvsApp.h:102 | `== 61` | **T** | selected branch → `assert_size 0x58` |
| CWvsContext.h:46 | `>= 95` | F | `m_bFirstUserLoad` absent |
| CWvsContext.h:56/101/148/241/253 | `>= 87` | F | `m_bPetHelpPopUpShown`, `m_bTesterAccount`, 60-slot equip→**52-slot #else**, `m_pUIAccountMoreInfo`/`m_pUIFindFriend`, `m_pClock` absent |
| **CWvsContext.h:98** | **`> 83`** | **F** | **`m_aClientKey[8]` ABSENT** (special — see Step 2) |
| CWvsContext.h:160/172/181/201/215/232/237/339 | `>= 95` (±JMS) | F | v95 pools/UI/itemmsg members absent |
| CClientSocket.h:22 | `>= 111` | F | `dummy1` (+4 pad) absent |
| CLogin.h:205/211/221/260/278/282 | `>= 95` (±JMS) | F | `m_pLayerBook`, `m_nFadeOutLoginStep`, `m_nBuyCharCount`, `m_nCurSelectedSubJob`, `m_bCharSale`/`m_nCharSaleJob`, `m_bCanHaveExtraChar` absent |
| **CLogin.h:235** | **`== 83`** | **F** | **`unk3[5]` ABSENT** (special — Task 3, re-anchored Step 3) |
| COutPacket.h:9/32 | `>= 111` | F | `dummy1` (+4 pad) absent |
| CConfig.h:52/82 | `>= 111` | F | `m_v111Pad` absent |
| CConfig.h:84 | `== 95` | F | → base GMS `#else` branch (`assert_size >= 1072`) |
| ConfigSysOpt.h:15 | `>= 95` | F | `bSysOpt_LargeScreen`+`bSysOpt_WindowedMode` ABSENT (windowed-mode cross-check, Step 4) |

### Step 2 — CWvsContext `m_aClientKey[8]` (`> 83`, special) + embedded SecondaryStat

**`m_aClientKey[8]` ABSENT — confirmed against the v61 binary.** `CClientSocket::OnConnect`
login-hello (tail @`0x4731bb`) builds `COutPacket`, then `Encode4([g_pWvsContext+0x2088])` →
`Encode1` → `Encode1` → `SendPacket`. There is **NO** `EncodeBuffer(m_aClientKey, 8)` anywhere in
the hello — matching Task 4/10 (`COutPacket(20);Encode4;Encode1;Encode1`, no 8-byte key). So the
`> 83` block is correctly excluded for v61. [[project_v84_clientkey_gate_trap]] confirmed at source.

**⚠️ Embedded SecondaryStat = 0x970 — DIVERGES from base 0xB88 by −0x218 (Cat-C surprise).**
Two independent v61 anchors, neither assuming 0xB88:
- `CWvsContext::OnTemporaryStatReset` @`0x84353a`: `mov edi,ecx; add edi,2114h; … mov ecx,edi;
  call SecondaryStat::Reset` ⇒ **m_secondaryStat @ ctx+0x2114**.
- `CWvsContext::OnForcedStatSet` @`0x843718`: `lea ecx,[esi+2A84h]; call ForcedStat::Decode`
  ⇒ **m_forcedStat @ ctx+0x2A84** (ctor `sub_82C1A8` corroborates: `lea ecx,[esi+2114h]` SecondaryStat init then `lea ecx,[esi+2A84h]` `ForcedStat::Clear`).
- Δ = 0x2A84 − 0x2114 = **0x970** = `sizeof(SecondaryStat)` (adjacent members). Corroborated a 3rd
  way: the highest SecondaryStat field touched in `OnTemporaryStatReset` is `[edi+0x96C]`
  (+4 = 0x970 — last member, a `TemporaryStat_GuidedBullet` ZRef). **This shifts every CWvsContext
  field after m_secondaryStat; Task 15 MUST gate SecondaryStat to 0x970 for v61 (split), not 0xB88.**

**`sizeof(CWvsContext) ≈ 0x3340`.** v61 CWvsContext is a **static/BSS object @`0x978F70`** (NOT
heap-allocated): `sub_82C11E` = `mov ecx, offset unk_978F70; jmp sub_82C1A8(ctor)`; ctor writes
`g_pWvsContext`(=`0x974ef8`) = aligned base. Ctor `sub_82C1A8` highest field write = `[esi+0x333C]`
(`0x82c5d1`) ⇒ end ≈ **0x3340**. Embedded landmarks: vtable@0, m_secondaryStat@0x2114,
m_forcedStat@0x2A84, m_temporaryStatView@0x2B14, GUILDDATA@0x2CAC, ALLIANCEDATA@0x2CE0.
**Gate verdict: `unchanged`** (the `> 83` m_aClientKey block is correctly absent; no edit to
CWvsContext.h needed) — **BUT flag the embedded-SecondaryStat split to Task 15** (size delta, not a
CWvsContext.h gate).

### Step 3 — CLogin `unk3[5]` (`== 83`) re-anchored

**`sizeof(CLogin) = 0x1DC` (476).** Independent anchor: stage-creation in `sub_8460D8`
(`CWvsApp::ConnectLogin` path) — `push 1DCh; Alloc(ZAllocEx); CLogin::CLogin(@0x5620d4); set_stage`
(`0x8460f9`–`0x846115`). `unk3[5]` (`== 83`, 0x14) is **ABSENT** in v61 (Task 3: ctor builds exactly
one ZList = `m_lNewEquip`). **Verdict: `unchanged`** (`== 83` correctly excludes v61; no edit).
Note: 0x1DC < the v95-PDB ref 0x2C8 (expected: v61 lacks all `>= 95` members + `unk3[5]`); any v61-vs-v72
base delta would trace to the `CMapLoadable` base class, deferred to its own audit (Task 14/15), not a
CLogin.h gate.

### Step 4 — CWvsApp / CClientSocket / COutPacket / CConfig / ConfigSysOpt

- **CWvsApp = 0x58** (recorded here per D5; full derivation in Task 3 above). All `>= 87`/`>= 95`
  blocks absent (gate table). `== 61` branch selected → `assert_size 0x58`. **Verdict:
  `branch-added`** (Task 3), upper-gate members confirmed absent.
- **CClientSocket = 0x94** (148). Anchor: `TSingleton<CClientSocket>::CreateInstance` @`0x825ff3`
  → `push 94h; Alloc; CClientSocket::CClientSocket(@0x4727fb)` (`0x826007`–`0x826023`). `>= 111`
  `dummy1` absent → base layout. **Verdict: `unchanged`.**
- **COutPacket = 0x10** (16). Anchor: ctor `??0COutPacket@@QAE@J@Z` @`0x5ffc4f` → init `sub_5FFC98`
  writes `[esi]`(m_bLoopback), `[esi+8]=0`(m_uOffset), `[esi+0Ch]=0`(m_bIsEncryptedByShanda);
  `m_aSendBuff` ZArray@+4 (RemoveAll unwind). Highest write +0xC ⇒ 0x10. `>= 111` `dummy1` absent.
  Byte-identical to documented v83/v84/v87 (0x10). **Verdict: `unchanged`.**
- **CConfig = 0x380** (896). Anchor: `CWvsApp::SetUp` @`0x8231a8` → `push 380h; Alloc;
  CConfig::CConfig(@0x47921d)`. `>= 111`/`== 95` false → base GMS `#else` (assert `>= 1072`).
  **Verdict: `unchanged`** (gates correct; our v95-shaped struct is oversized-but-safe, no member read).
  **⚠️ Note:** real v61 size **896 < the assert comment's claimed "1072 = smallest GMS real size"**
  (header CConfig.h:91 comment is now stale — v61 is the new GMS floor at 896). Functionally safe:
  the real client allocs its own 896 immediate; the static_assert checks OUR struct (≥1592) ≥ 1072,
  which holds; we read no CConfig data member.
- **ConfigSysOpt (CONFIG_SYSOPT) = 0x30** (12 ints). Anchor: `CConfig::ApplySysOpt` @`0x47b28e`
  → `push 0Ch; lea edi,[ebx+58h]; rep movsd` copies **12 dwords** from the source `CONFIG_SYSOPT*`
  into `CConfig.m_sysOpt` (@CConfig+0x58). `>= 95` `bSysOpt_LargeScreen`+`bSysOpt_WindowedMode`
  **ABSENT** ⇒ 12-int base (with them it would be 14 = 0x38). Windowed-mode cross-check: SetUp sets
  the **global** `g_CConfig_SysOpt_WindowedMode` (`0x978e24`) = 0x10 (`0x8231cd`) — a default flag,
  NOT a struct field; ApplySysOpt's last copied field is `[CConfig+0x84]` = `bSysOpt_Minimap_Normal`
  (index 11), confirming no windowed-mode slot in-struct. **Verdict: `unchanged`.**

### Step 5 — verdict table (7 headers)

| Header | v61 size | Per-gate present/absent (v61) | Deciding v61 anchor | Verdict |
|---|---|---|---|---|
| CWvsApp.h | **0x58** (88) | `>=72`/`>=87`/`>=95` blocks ABSENT; `==61` selected | WinMain stack-ctor Δ=0x58 (Task 3) | `branch-added` (Task 3) |
| CWvsContext.h | **≈0x3340** (static @0x978F70) | `>83` m_aClientKey ABSENT; `>=87` 60-slot→52-slot #else; `>=87`/`>=95` members ABSENT | ctor `sub_82C1A8` highest write +0x333C; OnConnect hello no key-buffer | `unchanged` (gate) **+ flag SecondaryStat=0x970 split → Task 15** |
| CClientSocket.h | **0x94** (148) | `>=111` dummy1 ABSENT | CreateInstance `push 94h` @0x826007 | `unchanged` |
| CLogin.h | **0x1DC** (476) | `==83` unk3[5] ABSENT; `>=95` members ABSENT | stage Alloc `push 1DCh` @0x8460f9 | `unchanged` |
| COutPacket.h | **0x10** (16) | `>=111` dummy1 ABSENT | ctor init sub_5FFC98 highest write +0xC | `unchanged` |
| CConfig.h | **0x380** (896) | `>=111`/`==95` ABSENT → base `#else` | SetUp Alloc `push 380h` @0x8231a8 | `unchanged` (⚠ real 896 < comment's 1072 floor) |
| ConfigSysOpt.h | **0x30** (48) | `>=95` LargeScreen+WindowedMode ABSENT | ApplySysOpt `rep movsd` 0xC @0x47b29d | `unchanged` |

### Cat-C divergences surfaced (do NOT force `unchanged`)
1. **Embedded SecondaryStat = 0x970 (v61) vs base 0xB88 (−0x218)** — measured 3 ways
   (m_forcedStat−m_secondaryStat delta; highest field [+0x96C]). Shifts every CWvsContext field
   after m_secondaryStat. **Feeds Task 15 as a SecondaryStat SPLIT; CWvsContext.h itself needs no
   gate edit** (it embeds whatever SecondaryStat.h computes).
2. **CConfig real = 896 (0x380)** is below the CConfig.h:91 comment's stated "1072 = smallest GMS
   real size." Comment is stale (v61 is the new GMS floor); the `>= 1072` assert still holds against
   our oversized v95-shaped struct, and no CConfig member is read, so functionally safe — flag for a
   comment refresh, no gate change.

All seven gate verdicts are `unchanged`/`branch-added` (no NEW source edit required for the 7
headers in this task); the two divergences above are size facts that propagate to Task 15
(SecondaryStat) and a stale-comment note, not new gates on these 7 headers.
