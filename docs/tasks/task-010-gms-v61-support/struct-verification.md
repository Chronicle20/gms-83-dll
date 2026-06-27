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

## Task 14 (Category C / cascade) — UI/control family, exhaustive size (9 headers; CWnd cascade)

Read-only `disasm` only; no struct types applied (R10/R12). Lane re-confirmed via `list_instances`:
active IDB = port **13344 `GMS_v61.1_U_DEVM.exe`** (v48@13345 distractor never probed; v72@13343).
Per **D5** every v61 size below is independently measured against a v61 anchor even where it
"should" match the base. v61 retains full mangled C++ symbols.

### Step 1 — per-header gate truth table (v61); every upper gate is FALSE → base/excluded branch

| Header:line | Gate | v61 | Effect on v61 layout |
|---|---|---|---|
| CWnd.h:4 | `>=95 \|\| JMS` | F | `UIOrigin` enum (no member) |
| **CWnd.h:25** | **`>=83 \|\| JMS`** | **F** | `m_pAnimationLayer`/`m_pOverlabLayer` ABSENT (cascade root; Task 12) |
| CWnd.h:35 | `>=87 \|\| JMS` | F | `m_ptCursorRel` → 2-int `#else` (`m_ptCursorRel_x/_y`) |
| CWnd.h:44 | `>=95` | F | `m_origin` ABSENT |
| CUIWnd.h:11 | `>=95` | F | `m_nSmallScreenX..m_bIsLargeMode` (5 fields) ABSENT |
| CUIToolTip.h:82 | `>=95` | F | `CLineInfo::m_bUseDotImage` ABSENT (CLineInfo=0x24 here w/o it; member sits past v61 use) |
| CUIToolTip.h:92 | `>=83 \|\| JMS` | F | `m_pLayerAdditional` ABSENT (Task 12) |
| CUIToolTip.h:100 | `>=95 \|\| JMS` | F | `m_nOptionLineNo`+`m_aOptionLineInfo[32]` ABSENT |
| CUIToolTip.h:125 | `>=84 \|\| JMS` | F | `m_pFontGen_Unknown` ABSENT |
| CUIToolTip.h:128 | `>=87 \|\| JMS` | F | `m_pFontH_White` ABSENT |
| CUIToolTip.h:131 | `>=87` | F | `m_pFontStan_Prp` ABSENT |
| CUIToolTip.h:134 | `>=95` | F | `m_pFontStan_Dsc..m_pFontSkill_Dsc` (4) ABSENT |
| CUIToolTip.h:152 | `>=84 \|\| JMS` | F | `m_pCanvasEquip_Durability[2][2]` ABSENT |
| CCtrlButton.h:32 | `>=95` | F | `m_sToolTipFromData` ABSENT |
| CCtrlCheckBox.h:16 | `>=95` | F | `m_nTextOffsetX`+`m_nTextOffsetY` ABSENT |
| CFadeWnd.h:18 | `REGION_GMS` | **T** | `m_bUserAlarm` PRESENT (region gate, not version) |
| **CFadeWnd.h:24** | **`>=87 \|\| JMS`** | **F** | `m_nLevel`/`m_nJobCode`/`m_nExpQuestID` ABSENT (proven by size arithmetic, below) |
| CUITitle.h:3 | `>=95 \|\| JMS` | F | base = **CDialog** (not CFadeWnd) — correct for v61 |
| CUITitle.h:12 | `>=95 \|\| JMS` | F | `m_rcRMA` ABSENT |
| CUILoginStart.h:8 | `>=95 \|\| JMS` | F | `m_pFont`+`m_pCanvasChannelName` ABSENT |
| CUILoginStart.h:12 | `REGION_GMS` | **T** | 5-element `m_aBtParam[5]`/`m_apButton[5]` (GMS branch) — correct for v61 |
| CLogo.h:5/16/30 | `>=95` | F | `VIDEO_STATE`, `m_pLayerBackground`, `m_bVideoMode`/`m_videoState` ABSENT |
| CLogo.h:27 | `>=95 \|\| JMS` | F | `m_bNXFadeOut` ABSENT |
| CLogo.h:34/91 | `>=111` | F | `dummy1` ABSENT |
| CLogo.h:93 | `==95` | F | falls to `#elif REGION_GMS` → `assert_size(sizeof(CLogo) >= 0x38)` |

### Step 2 — CWnd cascade resolution (R5): NOT triggered; all 5 derived sizes independently confirmed

v61 `sizeof(CWnd)` base subobject = **0x64 == v72** (Task 12; re-anchored here: ctor `CWnd::CWnd`
@`0x4BB456` writes `[esi+64h]=[esi+68h]=[esi+70h]=0` + 3 vtables, highest write 0x70 — anim-layer
pair `>=83||JMS` absent, byte-identical to v72). **Cascade NOT triggered** → the five CWnd-derived
classes inherit v72's verdict, each independently size-confirmed below (D5). No derived class
surfaced a v61-specific size divergence.

| Derived class | v61 size | Independent v61 anchor | Cascade verdict |
|---|---|---|---|
| **CDialog** (intermediate) | **0x74** | `CFadeWnd::SetOption` @`0x4DD232` stores first own member `m_a0`→`[ecx+74h]` (and `CFadeWnd::CFadeWnd` @`0x4DD186` calls `CWnd::CWnd` directly then writes `m_nPhase`@`0xA4`; the 11 anim fields back-fill to `m_a0`@0x74) ⇒ CDialog ends @0x74 (CWnd 0x64 + 0x10) | confirmed-shares-v72 |
| **CUIWnd** | **0x59C** | `CUIWnd::OnDestroy` @`0x707C24` reads `[this+574h]` (`m_nUIType`) ⇒ embedded `m_uiToolTip`(0x50C)@0x68 ends @0x574; then 6 ints + 2 bools + `m_nOption` + ZArray + ZXString ⇒ 0x59C. `>=95` 5-field block absent | confirmed-shares-v72 |
| **CFadeWnd** | **0xD0** | `CUIFadeYesNo::CUIFadeYesNo` @`0x4DDC49` calls `CFadeWnd::CFadeWnd` then writes its FIRST own member `[esi+0D0h]` ⇒ sizeof(CFadeWnd)=0xD0. `CreateGuildInvite` @`0x4E04EB`: `m_nType`@0xBC(=7), `m_sInviter` ZXString @0xC0 (via sub_414F76). **`>=87` block ABSENT — proven:** m_dwSN/m_dwFriendID must occupy 0xC4..0xD0; the 0xC-byte level/job/quest block cannot also fit ⇒ if present, size would be ≥0xD8 (it is 0xD0) | confirmed-shares-v72 |
| **CUITitle** | **≈0x5E4** | `CUITitle::EnableLoginCtrl` @`0x58966D` touches button/edit ZRefs up to `[esi+0D4h]` (`m_pEditPasswd`); trailing embedded `m_uiToolTipTitle`(CUIToolTip 0x50C)@≈0xD8 ⇒ ≈0x5E4. base=CDialog (line 3 `>=95` false), `m_rcRMA` absent. (Size is observed-lower-bound + embedded-toolTip; gate verdict solid.) | confirmed-shares-v72 |
| **CUILoginStart** | **≈0xBC** | Fully inlined in v61 (no standalone symbol). Computed from proven CDialog base (0x74) + `m_pLogin` + GMS `m_aBtParam[5]`(0x28) + `m_apButton[5]`(0x14) + `m_nViewWorldButtonType` + `m_bRequestSent`. `>=95` pair absent; GMS 5-element arrays selected (region gate, correct) | confirmed-shares-v72 |

### Step 3 — per-header size + gated-field present/absent verdict (9 headers)

| Header | v61 size | Per-gate present/absent (v61) | Deciding v61 anchor | Verdict |
|---|---|---|---|---|
| CWnd.h | **0x64** (base subobject; ctor writes through 0x70) | anim-layer pair `>=83` ABSENT; `m_ptCursorRel` 2-int `#else`; `m_origin` `>=95` ABSENT | ctor `CWnd::CWnd` @0x4BB456 (highest write 0x70); == v72 (Task 12, 3 landmarks) | **confirmed-shares-v72** (cascade root, not triggered) |
| CUIWnd.h | **0x59C** (1436) | `>=95` 5-field block ABSENT | `OnDestroy` @0x707C24 `[this+574h]`=m_nUIType ⇒ toolTip 0x68..0x574 | **unchanged** (confirmed-shares-v72) |
| CUIToolTip.h | **0x50C** (1292) | `m_pLayerAdditional`(>=83), opt-line block(>=95), `m_pFontGen_Unknown`(>=84), `m_pFontH_White`/`m_pFontStan_Prp`(>=87), stan/skill fonts(>=95), Durability(>=84), CLineInfo dot(>=95) ALL ABSENT | Task 12 (ctor @0x748FFE byte-identical v72) + re-anchored: embedded in CUIWnd spans 0x68→0x574 = 0x50C | **confirmed-shares-v72** |
| CWnd.h derived → CFadeWnd.h | **0xD0** (208) | `m_bUserAlarm` PRESENT (GMS); `m_nLevel`/`m_nJobCode`/`m_nExpQuestID` `>=87` ABSENT | `CUIFadeYesNo` ctor @0x4DDC49 first own member @0xD0; `CreateGuildInvite` m_nType@0xBC/m_sInviter@0xC0; >=87 block can't fit (arithmetic) | **confirmed-shares-v72** (unchanged) |
| CCtrlButton.h | **0x5A0** (1440) | `m_sToolTipFromData` `>=95` ABSENT | ctor `CCtrlButton::CCtrlButton` @0x44E442: CCtrlWnd base@0x68, embedded `m_uiToolTip`(0x50C)@0x90 ⇒ ends 0x59C, `m_bSelfDisable`@0x59C ⇒ 0x5A0 | **unchanged** |
| CCtrlCheckBox.h | **≈0x9C** (156) | `m_nTextOffsetX`/`m_nTextOffsetY` `>=95` ABSENT | Fully inlined in v61 (no symbol). Computed from CCtrlWnd base 0x68 (proven via CCtrlButton ctor) + 4 ints + ZXString + 5 fields + canvas[4]; only gate is `>=95` (absent). No `assert_size` ⇒ size non-load-bearing | **unchanged** |
| CUITitle.h | **≈0x5E4** (1508) | base=CDialog (`>=95` false); `m_rcRMA` `>=95` ABSENT; JMS `m_unk` absent (GMS) | `EnableLoginCtrl` @0x58966D members to m_pEditPasswd@0xD4 + embedded CUIToolTip 0x50C | **unchanged** |
| CUILoginStart.h | **≈0xBC** (188) | `m_pFont`/`m_pCanvasChannelName` `>=95` ABSENT; 5-element GMS arrays selected | Computed from proven CDialog base 0x74; inlined (no symbol). No `assert_size` | **unchanged** |
| CLogo.h | **0x38** (56) | `m_pLayerBackground`/`m_bVideoMode`/`m_videoState`/`VIDEO_STATE`(>=95), `m_bNXFadeOut`(>=95\|JMS), `dummy1`(>=111) ALL ABSENT | ctor `CLogo::CLogo` @0x5950F4 highest field write `[esi+34h]`=`m_bNXFadeIn` (+vtables to 0xC) ⇒ 0x38; CStage base via sub_45BE8F. == v83/v84/v87 (header note) | **unchanged** (`#elif REGION_GMS` `assert_size >= 0x38` holds) |

### Findings / divergences
- **No split surfaced.** All 9 headers + the CDialog cascade intermediate take the v72/base branch
  for v61; every enumerated `>=83`/`>=84`/`>=87`/`>=95`/`>=111`/`==95` gate is FALSE for v61, and
  the two region gates (`CFadeWnd::m_bUserAlarm`, `CUILoginStart` 5-arrays) select the GMS branch
  correctly. The cascade is confirmed NOT triggered (CWnd 0x64 == v72).
- **Investigated false-alarm (recorded for audit trail):** CUIFadeYesNo's first member at 0xD0 and
  CreateGuildInvite's `m_nType`@0xBC initially read as if CFadeWnd carried the `>=87` quest/level
  block. Probed to resolution — m_dwSN/m_dwFriendID provably occupy 0xC4..0xD0, so the 0xC-byte
  `m_nLevel/m_nJobCode/m_nExpQuestID` block is **absent** (its presence would force size ≥0xD8).
  Gate `CFadeWnd.h:24 >=87` is correct for v61. **Not flagged to Task 17.**
- **Size-precision caveats (gate verdicts unaffected):** CUITitle (≈0x5E4), CUILoginStart (≈0xBC),
  CCtrlCheckBox (≈0x9C) are leaf classes fully/partly inlined in v61 — their byte sizes are
  observed-lower-bound (CUITitle) or computed from a proven base (CUILoginStart/CCtrlCheckBox)
  rather than a single Alloc anchor. None carries an `assert_size`, and none is a layout-shifting
  base, so the exact byte count is non-load-bearing; the `unchanged` gate verdict is firm.

### Cross-version safety (FR-13)
No source edits in this evidence task. All 9 headers' selected branches for v72/v79/v83/v84/v87/
v95/v111/JMS185 are untouched; v61 simply joins the existing base/`#else` branch that v72 already
occupies for every gate. No `assert_size` change, no struct-type application.

## Task 15 (Category B/C struct audit) — Mob/stat family, exhaustive size (4 headers)

Read-only `disasm` only; no struct types applied (R10/R12). Lane re-confirmed via `list_instances`:
active = port **13344 `GMS_v61.1_U_DEVM.exe`**. For the SecondaryStat base re-confirmation I also
measured v72 (13343), v79 (13339), v83 (13340) — single-lane switches, each re-confirmed by binary
name. v61 retains full mangled symbols. **This is the highest-stakes struct task** — the three split
rows must pin which members v61 lacks so Task 17 writes a correct member-level `< 72` gate.

### Step 1 — per-gate truth table (v61); every enumerated upper gate is FALSE → base/excluded branch

| Header:line | Gate | v61 | Effect on v61 layout |
|---|---|---|---|
| CMob.h:99 | `>=79` | F | attack-ready/charge block absent (== v72; both <79) |
| CMob.h:112 | `<95` | **T** | `unknown1` ZList PRESENT (== v72) |
| CMob.h:146/168/188/249 | `>=95`(±JMS) | F | m_nPhase, multi-body arcs, m_tLastHitDazzledMob, v95 tail absent |
| CMob.h:219 | `>=87\|\|JMS` | F | SECPOINT m_ptPos/Prev → 4-int `#else` (== v72) |
| CMob.h:231/235 | `>=84`(±JMS) | F | m_aMultiTargetForBall, m_aRandTimeforAreaAttack/m_delaySkill absent |
| **CMob.h:241** | **`>=83\|\|JMS`** | **F** | doom tail (m_bDoomReserved/SN/m_lpStatChangeReserved) ABSENT (== v72) |
| CMob.h:246 | `>=87\|\|JMS` | F | m_bChasing absent |
| MobStat.h:128 | `>=83\|\|JMS` | F | Weakness group (n/r/tWeakness_) ABSENT (== v72) |
| MobStat.h:133/139/157 | `>=95`(±JMS) | F | TimeBomb, MagicCrash/elem/HealByDamage, bCannotEvade absent |
| **MobStat.h:154** | **`>=79\|\|JMS`** | **F** | `bDisable` ABSENT (== v72; both <79) |
| SecondaryStat.h:393 | `>=87\|\|JMS` | F | DojangShield (3 tears) absent |
| SecondaryStat.h:405/419 | `==87\|\|JMS` | F | nReverseInput/rDojangBerserk → 12B int `#else` (== base) |
| SecondaryStat.h:626 | `>=84\|\|JMS` | F | Flying/Frozen (6 tears) absent |
| SecondaryStat.h:640 | `>=87\|\|JMS` | F | AssistCharge/Enrage (6 tears) absent |
| SecondaryStat.h:655/732 | `>=95`(±JMS) | F | SuddenDeath… + v95 tail absent |
| CMapLoadable.h:143/160/171 | `>=95` | F | m_bField, m_lpLayerLetterBox, m_bPlayHoldedBGM/m_tPlayHoldedBGM absent |
| CMapLoadable.h:154 | `>=84\|\|JMS` | F | m_lVisibleByQuest ABSENT |

### Step 2 — verdict table (4 headers)

| Header | v61 size | v61 vs adjacent | Per-gate (v61) | Deciding v61 anchor | Verdict |
|---|---|---|---|---|---|
| CMob.h | **0x490** (1168) | **DIVERGES** −0x30 vs v72 0x4C0 | all gates == v72 (doom `>=83` absent) | ctor @0x5C2128 highest write 0x47C / Alloc 490h; MobStat embed @+0x170 | **split** (size; via MobStat embed + base — see below) |
| MobStat.h | **0x1B8** (440) | **DIVERGES** −0x20 vs v72 0x1D8 | all gates == v72 (Weakness/bDisable absent) | `SetFrom` @0x6685A7 memset 1B8h; nFs `fstp [edi+198h]`, lBurnedInfo@0x1A4 | **split** (size; 8 trailing status ints) |
| SecondaryStat.h | **0x970** (2416) | **DIVERGES** −0x140 vs v72 0xAB0 | all `>=84/87/95`/`==87` ABSENT | ctor `sub_82C1A8` Δ(forcedStat 0x2A84 − secStat 0x2114) | **split** (size; members UNRESOLVED — flagged) |
| CMapLoadable.h | **0xEC** (236) | smaller than v95 0x148 (gated members) | all `>=84/95` ABSENT | `CField::CField` @0x4E4830 base ctor `sub_59D8CD` highest write +0xE8; own members @+0xEC | **unchanged** |

### Step 3 — CMob: the −0x30 is INHERITED, not an own-member gate
**Every CMob.h version gate evaluates IDENTICALLY for v61 and v72** (proven from source — see Step 1:
all `>=79/83/84/87/95`±JMS FALSE for both, `<95` TRUE for both, both `REGION_GMS` gates TRUE). ⇒
**CMob's own direct members do NOT differ between v61 and v72.** The −0x30 decomposes (measured both
lanes — v72 ctor `??0CMob@@` @0x611CDB):
- **embedded MobStat −0x20**: embed v61 @CMob+0x170 vs v72 @CMob+0x178; MobStat 0x1B8 vs 0x1D8.
  → realized by the **MobStat.h `<72` split**.
- **base/sub-struct −0x10**: CLife base subobject ends @0x80 (v61, `m_nMobChargeCount`@0x80) vs @0x84
  (v72, `m_nMobChargeCount`@0x84) → pre-MobStat −0x8; post-MobStat region −0x8. Lives in the **CLife
  base + embedded sub-structs**, NOT any CMob.h gate.

**Task 17 disposition (CMob.h):** add the `< 72` size guard but **NO new own-member gate** — CMob
shrinks to 0x490 once MobStat.h is gated (−0x20) and CLife is correctly sized (−0x10). CMob.h has no
`assert_size`, so the residual −0x10 (CLife/sub-structs) does not break compilation. Doom write
(`==83`) stays correctly OOB-excluded for v61. **FLAG:** the −0x10 is in **CLife.h** (currently
ungated — JMS-only) + embedded sub-structs, OUTSIDE the 4 audited headers; recorded for whoever
models CLife. (Without it, the CMob model computes ~0x4A0 not 0x490 — precision gap, non-breaking.)

### Step 3 — MobStat: the −0x20 = 8 trailing status ints (candidate flagged)
MobStat.h gates are IDENTICAL for v61/v72 (`bDisable` `>=79` FALSE both; `Weakness` `>=83` FALSE
both). So the −0x20 is **8 status ints (0x20) the header models as always-present but v61 lacks** —
NOT the gated Weakness/bDisable. `SetFrom` (v61 @0x6685A7 / v72 @0x6D0896) copies the early stats
(nLevel@0, aDamagedElemAttr[8]@4, nPAD@0x24, nPDR@0x34, nMAD@0x44, nMDR@0x54, nACC@0x64, nEVA@0x74,
nSpeed@0x84) to **identical offsets** in both; only nFs/bInvincible/lBurnedInfo shift by exactly 0x20
(v61 nFs@0x198, v72 nFs@0x1B8). ⇒ the missing 8 ints sit in the ungated status region **between 0x84
(after nSpeed) and 0x198 (before nFs)**, by trailing append.
- **Strong candidate (FLAGGED, not field-proven):** the trailing ungated group MobStat.h **lines
  117-124** — `nMCounter_`,`rMCounter_`,`tMCounter_`,`wMCounter_`,`nCounterProb_`,`nBodyPressure_`,
  `rBodyPressure_`,`tBodyPressure_` (exactly 8 ints = 0x20). SetFrom does not enumerate the status
  region (it is memset), so confirm the boundary before Task 17 gates it. Base v72 0x1D8 re-confirmed.

### Step 3 — SecondaryStat: base re-confirmed (0xB88 = v79); members UNRESOLVED → FLAGGED
**Full size sequence measured this task** (CWvsContext m_secondaryStat..m_forcedStat adjacency):

| Ver | m_secondaryStat | m_forcedStat | sizeof(SecondaryStat) |
|---|---|---|---|
| v61 | ctx+0x2114 | ctx+0x2A84 | **0x970** |
| v72 | ctx+0x212C | ctx+0x2BDC | **0xAB0** |
| v79 | ctx+0x212C | ctx+0x2CB4 | **0xB88** |
| v83 | ctx+0x2134 | ctx+0x2E0C | **0xCD8** |

**Base re-confirmation:** the premised "base 0xB88" is **v79**, NOT v72 (0xAB0) or v83 (0xCD8). v61's
−0x218 was the v61-vs-**v79** delta. The `< 72` split anchor is **v72 = 0xAB0 ⇒ v61 lacks 0x140 (320
bytes) vs v72.** Every `>=84/87/95`/`==87` gate is ABSENT for v61 (confirmed Step 1) — the `>87`
m_aClientKey and `==87`/`>=87` blocks all correctly excluded.

**MEMBER IDENTIFICATION — UNRESOLVED, FLAGGED FOR THE USER (do NOT let Task 17 guess):**
1. **atlas-ms cannot adjudicate below v87** (cross-validator `stat-registry-cross-validator`): every
   TemporaryStat WeaponAttack..SoulStone is registered unconditionally for all GMS versions; first
   presence gate is `post87`→Flying. atlas carries the same floor the header does.
2. **The header over-models even v83** — real v83 = 0xCD8 but the header (all `>=84/87/95` false for
   v83) computes ~0xE00; it contains UNGATED v95-only slots (`nMechanic_`@0x2A0 + Aura family
   `nMaxLevelBuff_`/`nAura_`/`tUpdateAura_`/`nSuperBody_`/`nDarkAura_`/`nBlueAura_`/`nYellowAura_`,
   ≈0x108-0x12C). So the header field inventory is not a faithful base for any sub-v95 build.
3. **Growth is interspersed, not trailing** — the ride-vehicle stat read in OnTemporaryStatReset is
   @SecondaryStat+0x964 (v61) vs +0xA94 (v72): it moved +0x130 of the +0x140 total, so most new slots
   sit BEFORE that late stat.
- **Consequence:** a guessed `<72` member gate would mis-remove slots and corrupt every CWvsContext
  field after m_secondaryStat (and m_forcedStat@+0x2A84, …). **Recommended:** dispatch
  `version-port-verifier` on the **v61 SecondaryStat ctor `sub_65F66F`** (its per-stat
  `_ZtlSecureTear` inits enumerate every slot in layout order) and diff vs the v72 ctor to enumerate
  the exact removed groups by byte range, BEFORE Task 17 writes the gate. A faithful older-version
  model likely needs multi-way (`<72`/`<79`/`<83`) gates since SecondaryStat grew at every step; the
  ungated v95-only Mechanic/Aura slots should separately be gated `>=95` (affects v83/v87 too).

### Step 3 — CMapLoadable (README-critical): unchanged, all gated members absent
v61 = **0xEC**. Anchor: `CField::CField` @0x4E4830 → base ctor `sub_59D8CD` (calls `CStage::CStage`
@0x45BE8F), highest field write `[esi+0E8h]`; CField's first own member (ZList) @+0xEC. Gated members
confirmed ABSENT at the anchor: `m_bField` (`>=95`) — the slot @+0x34 is the `m_pSpace2D` ZRef vtable
(`off_8E60F4`), no int there; `m_lVisibleByQuest` (`>=84\|\|JMS`); `m_lpLayerLetterBox` (`>=95`);
`m_bPlayHoldedBGM`/`m_tPlayHoldedBGM` (`>=95`) — struct ends cleanly @0xEC (last writes
`[esi+0E4h]`/`[esi+0E8h]` = m_tRestoreBgmVolume/m_nRestoreBgmVolume). No `assert_size` in the header.
**Verdict: `unchanged`** — every gate correctly excludes v61 → base/#else; no edit. (v95 PDB ref
0x148; the delta beyond gated members is the version-variant CStage base.)

### Split dispositions for Task 17 (summary)
- **CMob.h:** `< 72` size guard only; NO own-member gate (members == v72); shrink inherited from
  MobStat embed + CLife base. Doom `>=83` already correct.
- **MobStat.h:** `< 72` arm removing 8 trailing status ints (candidate lines 117-124 — CONFIRM first).
- **SecondaryStat.h:** `< 72` arm needed but **member set UNRESOLVED — must be enumerated from
  `sub_65F66F` before gating** (highest risk; wrong gate corrupts CWvsContext). Base = v72 0xAB0.
- **CMapLoadable.h:** no edit (`unchanged`).

### Cross-version safety (FR-13)
No source edits in this evidence task. The 3 split rows feed Task 17; CMapLoadable needs no change
(v72/v79/v83/v84/v87/v95/v111/JMS185 all keep their existing branches). A `< 72` term is disjoint
from every supported version except v61.

## Task 15b (member-ID resolution) — pin the three flagged splits + edit-impact

Read-only `disasm`; no struct types; no edits. Lanes re-confirmed per switch: v61=13344, v72=13343
(diff partner). Method: diff the per-field enumerators `MobStat::DecodeTemporary` and
`SecondaryStat::Reset`, plus the CMob ctor and `SetFrom`, between v61 and v72.

**Active-vs-latent (grep bypass/ doom-fix/ redirect/ + CWvsContext.cpp): ALL THREE splits are LATENT
for v61.** No v61 edit reads a CMob/MobStat/SecondaryStat/CLife field, nor any CWvsContext field after
`m_secondaryStat`. `doom-fix` hooks `CMob::CMob` but its only write (`m_bDoomReserved`) is gated
`==83` → for v61 the hook is a pass-through (no shifted read). All CWvsContext reads in
`bypass/socket_hooks.cpp` (`m_dwAccountId`/`m_nSubGradeCode`/`m_nWorldID`/`m_nChannelID`/
`m_dwCharacterId`, header lines 63–104) precede `m_secondaryStat` (line 110). So these are
OFFSET-FIDELITY (model-only) issues; none is load-bearing for a live v61 hook, and none header has a
`static_assert`. Gate them for correctness, but priority = fidelity, not crash-avoidance.

| Header | Split | RESOLVED member-IDs (offsets, v61 vs v72) | Task 17 `<72` gate shape | Status |
|---|---|---|---|---|
| **MobStat.h** | −0x20 | Enumerators byte-identical through off 0x194 (`nRiseByToss_`, hdr line 110; `SetFrom` nFs v61@0x198 / v72@0x1B8). **v61 lacks hdr lines 111–118** (8 ints @ v72 0x198–0x1B4): `rRiseByToss_`,`tRiseByToss_`,`nPCounter_`,`rPCounter_`,`tPCounter_`,`wPCounter_`,`nMCounter_`,`rMCounter_`. (Corrects Task 15's "lines 117–124" candidate.) | Gate **lines 111–124** behind the `<72` exclusion → nFs@0x198, size 0x1B8. (Lines 119–124 are absent in real v72 too — see flag.) | **RESOLVED** |
| **CLife.h** (NEW row) | −0x4 | `sizeof(CLife)` = **0x80 (v61) vs 0x84 (v72)** = ONE 4-byte base member v61 lacks (m_nMobChargeCount, first CMob own member, @0x80 vs @0x84). Member name NOT pinned (CLife inlined, no symbol). | CLife.h needs a `<72` gate for one 4-byte member — **name not pinned**; latent + no assert → either focused ctor diff to name it, or leave v72-shaped. | **byte-RESOLVED; name-flagged** |
| **CMob.h** | −0x30 | Full ctor diff (v61 @0x5C2128 / v72 @0x611cdb) closes EXACTLY: CLife base −0x4 + **CMob-own pre-MobStat −0x4** (m_pTemplate@0x15C/0x160 → MobStat@0x170/0x178; one v72-only int in the m_pTemplateByDoom/m_nMP area) + MobStat −0x20 + **CMob-own post-MobStat −0x8**. | **CORRECTION to Task 15:** CMob's OWN members DO diverge (−0xC), not just CLife. Two undocumented v72-only 4-byte members in CMob-own region (1 pre-, ~1 post-MobStat) + 1 CLife base. Gates are identical v61/v72 so these are ungated/embedded-substruct deltas. | **decomp RESOLVED; names flagged** |
| **SecondaryStat.h** | −0x140 | `Reset` (v61 @0x662704 / v72 @0x6ca91a, size 0xf61 both) is byte-offset-**IDENTICAL on [0, ~0x790)** (sampled instr 0–90/600/830 — same offset/index). Divergence is **contiguous-late** in [~0x790, trailing array) — NOT interspersed (overturns Task 15). `aTemporaryStat[7]` (final member) @**0x938 (v61)** / @**0xA7C (v72)**, Δ=0x140. | **CANNOT write a member gate from the header** — the header over-models v72 itself by ~0x350 (computes array@~0xDE4 vs real 0xA7C): its field inventory/offsets match NEITHER binary. Member NAMES of the v61-absent late block NOT pinned. | **structure RESOLVED; names BLOCKED — FLAGGED** |

### Flags for the user (Task 15b)
1. **SecondaryStat (highest risk):** member set still not name-pinned. Root cause now PROVEN: the
   header is a v95+ over-model unfaithful to v72 (array@~0xDE4 vs real v72 0xA7C). A faithful sub-v83
   layout must be **rebuilt from the binary** (Reset/DecodeForLocal enumeration), not trimmed from the
   header; likely needs multi-way (`<72`/`<79`/`<83`) gates + an independent `>= 95` gate for the
   ungated Aura/Mechanic family. Latent for v61 → safest to defer rather than guess. What IS proven:
   contiguous-late −0x140, identical through ~0x790, array @0x938 v61 / 0xA7C v72.
2. **MobStat:** the header over-models v72 by lines 119–124 (`tMCounter_`,`wMCounter_`,`nCounterProb_`,
   `nBodyPressure_`,`rBodyPressure_`,`tBodyPressure_`) — absent in real v72 (nFs@0x1B8 proven). For full
   fidelity split: lines 111–118 `>= 72`, lines 119–124 `>= 79`.
3. **CMob/CLife:** CLife is only −0x4 (not −0x10); CMob.h itself diverges by −0xC own (contradicts the
   Task 15 "CMob own == v72" claim). Three v72-only 4-byte members (1 CLife base + 2 CMob-own) are
   byte-located but not name-pinned. Latent + no assert.

## Task 15c (SecondaryStat full rebuild) — RESOLVES the −0x140; supersedes Task 15/15b row

Read-only `disasm`; no struct types; lanes re-confirmed per switch (v61=13344, v72=13343). Full
reconstruction in **`v61_secondarystat_layout.md`** (this folder). Method: diff the three
enumerators `Reset` (v61 @0x662704 / v72 @0x6ca91a — explicit `lea [esi+OFF]` offset source),
`DecodeForLocal` (v61 @0x663665 — proves the 59 masks are the complete per-bit stat set), and the
flag-mask table whose ADDRESS encodes the atlas wire-bit (low table `unk_977BD8`↓ = bits 0–49;
extension table `unk_977C98`↓ = bits 50–58).

**Overturns Task 15/15b.** v61 and v72 carry the **identical 59 masked stats (atlas bits 0–58) in
identical order** + a trailing two-state array (identical table-jump fingerprint block-for-block).
The struct is **byte-identical v61↔v72 from 0x0 through 0x78C** (bits 0–39). The −0x140 is
**concentrated in 4 late sites**, NOT missing whole stats and NOT interspersed:

| site | stat (atlas bit) | v61 | v72 | v72-extra |
|---|---|---|---|---|
| A | SpiritJavelin / ShadowClaw (40) @0x78C | 3 tears (0x24) | 6 tears (0x48) | **+0x24** (mSpiritJavelin_ +2; tears not individually named) |
| B | Infinity (41) @0x7B0/0x7D4 | 4 tears (0x30) | 10 tears (0x78) | **+0x48** (6 tears not individually named) |
| C | GhostMorph / Ghost (49) trailing | 0x24 @0x934 | 0xDC @0x9A0 | **+0xB8** trailing pre-array block (not individually named) |
| D | two-state array | 6 × 4B = 0x18 @0x958 | 7 × 8B = 0x38 @0xA7C | **+0x20** (one extra entry — likely Undead; entry width 4B→8B) |

Σ = 0x144 (4-byte v72-side array reconciliation; exact v61 = **0x970**, array @0x958, 6×4-byte
entries, ends 0x970 — disasm-exact). Names: atlas bits 0–49 **anchored** (mask arithmetic +
atlas-canonical); bits 50–58 **high-confidence inferred** (extension-table order). The individual
v72-extra tears in sites A/B/C are localized by offset+byte-count and anchored to a named neighbour
but NOT individually named (reached via per-stat setter helpers, not direct `lea`); the v61 map
itself is complete and exact at 0x970.

**Task-17 gate (SecondaryStat.h, `< 72` arm → sizeof 0x970):** the header is unfaithful to v61 AND
v72 (15b), so a member-trim is not derivable from it. The split is **member-level only at sites
A/B/C/D**; bits 0–39 (0x0–0x78C) are shared with v72 and MUST NOT be gated. Preferred: a GMS
`BUILD_MAJOR_VERSION < 72` arm modelling SecondaryStat from `v61_secondarystat_layout.md` (59
groups at the listed offsets + a 6×4-byte two-state array) → `sizeof == 0x970`. Acceptable
fallback: leave model v95-shaped (LATENT — 15b proved no v61 hook reads CWvsContext past
m_secondaryStat; no `static_assert`), documenting the 4 sites. Either way, gate ONLY the 4 late
sites, never the 0x0–0x78C prefix.
