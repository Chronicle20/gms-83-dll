# GMS v79.1 — Struct Size/Layout Verification & Below-Floor Gate Audit

Scope (per confirmed PRD): verify **all 24 version-gated `common/*.h` headers**
against the v79 binary, **amend every enumerated gate that has no v79 branch**,
and empirically verify/correct every lower-bound gate whose truth value changes at
the 79 boundary.

Follow the read-only verification discipline from
`docs/version-porting-workflow.md`: anchor every verdict to v79 disassembly, and
do **not** apply speculative struct types into the IDB during verification
(decompiler leak). Confirm the connected IDB with `get_metadata` first
([[feedback-verify-ida-target]]). Do not defer a settleable question with "out of
scope" / "unlikely to matter" ([[feedback-prefer-confirmation]]) — this is doubly
load-bearing for v79 because **there is no labeled IDB below v79** to triangulate
against; v83 is the only nearby anchor and it sits *above* v79.

## The below-floor problem in one paragraph

Every supported version is ≥ 83, so the `common/` gates were authored treating
v83 as the minimum. v79 falls *below* that floor, so for every gate the question
is: which branch does v79 land on, and is that branch's layout actually v79's
layout? Three failure modes, in priority order:

1. **No branch at all** (enumerated `==` gates with no `#else`) → member
   undefined / silent layout shift / build break. **Fix before anything else.**
2. **Newly excluded** (`>= 83`, `>= 84`) → v79 drops a field it might actually
   have. Verify; lower the floor if v79 has it.
3. **Silently wrong base** (`>= 87/95/111`, `> 83`, `< 95`, `== 83/95`) → v79
   takes the "v83 layout" base branch, which may not match v79's real layout.
   Spot-check size; this is the quietest failure.

## Category A — enumerated gates with NO v79 branch (fix first; build-blocking)

These select by explicit version equality and have no catch-all `#else`, so v79
selects **nothing** and the gated member is left undefined. Amend each to give
v79 a defined, layout-correct branch (decide which by v79 disassembly).

| Site | Current structure | v79 result | Required action |
|---|---|---|---|
| `common/CFuncKeyMappedMan.h:38` | `== 83 \|\| == 84 \|\| == 87` / `:40 == 95` / `:42 == 111` | no branch | Verify v79 `CFuncKeyMappedMan` layout; add `79` to the matching branch (likely the `83/84/87` one) |
| `common/CWvsApp.h:97` | `(== 83 \|\| == 84)` / `:99 == 87` / `:101 >= 95` | no branch | Verify v79 `CWvsApp` layout; add `79` to the matching branch |

> Re-grep at task start — other enumerated-no-`else` gates may exist or these line
> numbers may have shifted: `grep -rn "BUILD_MAJOR_VERSION ==" common/*.h`.

## Category B — `>= 83` floor gates (v79 newly EXCLUDED)

A `>= 83` gate previously included every supported version; v79 is the first build
on the excluded side. Determine whether v79 genuinely lacks the field.

| Site | Gate | v79 lands | Must confirm | If v79 HAS it |
|---|---|---|---|---|
| `common/CUIToolTip.h:92` | `>= 83 \|\| JMS` | excluded | Does v79 `CUIToolTip` carry `m_pLayerAdditional` (this+0x14, after `m_pLayer` this+0x10)? v84 confirmed it present 83→111. | Lower floor to `>= 79` (keep JMS), re-validate all versions |

**Task-12 verdict (Cat B): `unchanged` — v79 LACKS `m_pLayerAdditional`; the `>=83` exclusion of v79 is CORRECT.**
Deciding v79 disasm (`??0CUIToolTip@@QAE@XZ` @0x842317): `mov [esi+10h], edi` (`m_pLayer`=0) is immediately
followed by `lea eax, [esi+20h]` → `eh vector constructor` of `m_aLineInfo[32]`. So `m_aLineInfo` starts at
**this+0x20**, leaving exactly 0x14/0x18/0x1C for `m_nLastX`/`m_nLastY`/`m_nLineNo` and **no `m_pLayerAdditional`**
(its presence would push `m_aLineInfo` to 0x24). v79 `CUIToolTip` sizeof = **0x514**, byte-identical to the v83
typed struct (`m_pLayer`@0x10, `m_nLastX`@0x14, `m_aLineInfo`@0x20, `m_bIngoreWeddingInfo`@0x510). No floor change.
NOTE/CONCERN (out of v79 scope): the v83 IDB type ALSO shows `m_nLastX`@0x14 (no additional layer, sizeof 0x514),
which is in tension with the gate comment "v84 confirmed present 83→111" — the `>=83` floor itself may warrant a
separate re-check at the v83/v84 boundary, but that does not affect the v79 verdict (v79 is excluded either way).

## Category C — `>= 84` gates (v79 EXCLUDED, sides with v83)

v79 takes the same excluded side as v83. Confirm the gated field is genuinely
absent in v79 (very likely, since v83 also lacks it — but verify, don't assume).

| Site | Gate | Field/region | Confirm absent in v79 |
|---|---|---|---|
| `common/CMob.h:229` | `>= 84` | `m_aMultiTargetForBall` | ☑ ABSENT |
| `common/CMob.h:233` | `>= 84 \|\| JMS` | `m_aRandTimeforAreaAttack`,`m_delaySkill` | ☑ ABSENT |
| `common/CMapLoadable.h:154` | `>= 84 \|\| JMS` | `m_lVisibleByQuest` | ☑ ABSENT |
| `common/CUIToolTip.h:125` | `>= 84 \|\| JMS` | `m_pFontGen_Unknown` | ☑ ABSENT |
| `common/CUIToolTip.h:152` | `>= 84 \|\| JMS` | `m_pCanvasEquip_Durability` | ☑ ABSENT |

**Task-12 verdict (Cat C): all five `>=84` fields `unchanged` — genuinely ABSENT in v79; exclusions CORRECT.**
Deciding v79 disasm per field:
- `CMob.h:229/:233` (`m_aMultiTargetForBall`, `m_aRandTimeforAreaAttack`, `m_delaySkill`): v79 `CMob::CMob` @0x630C2C
  highest member write is `mov [esi+514h], edi` = `m_bWaitingToBeSetTossed` (the **last** member; sizeof=0x518 ends
  at 0x518). The v83 typed `CMob` shows `m_bDoomReserved`@0x528 directly after `m_bWaitingToBeSetTossed`@0x524 with
  **no** `>=84` array members between them; v79 has neither those nor the doom region → the three `>=84` fields are
  absent (they would sit between `m_bWaitingToBeSetTossed` and the doom region, which is empty in v79).
- `CUIToolTip.h:125` (`m_pFontGen_Unknown`): v79 ctor zero-inits the font block 0x424→**0x470** (20 fonts, last =
  `m_pFontGen_Blue`@0x470) then `lea eax, [esi+474h]` builds `m_pCanvasEquip_ReqItem`@0x474 — no font at the >=84 slot.
- `CUIToolTip.h:152` (`m_pCanvasEquip_Durability`): v79 ctor writes `m_pNumberGrowthDisable`@0x50C then
  `m_bIngoreWeddingInfo`@0x510 directly (sizeof 0x514); the 0x10-byte `[2][2]` durability array is absent.
- `CMapLoadable.h:154` (`m_lVisibleByQuest`): `CMapLoadable::SetObjectState` @0x612abe does `add ecx, 0A0h` →
  `m_mNamedObj` ZMap @**0xA0**, directly after `m_lpRefInfo`@0x8C (ZList 0x14). A present `m_lVisibleByQuest`
  (ZList 0x14) would put `m_mNamedObj` at 0xB4. Matches the v83 typed struct (`m_mNamedObj`@0xA0).

## Category D — `< 95` / `< 84` inverse gates (v79 INCLUDED on base branch)

v79 is true for these and takes the base/included branch. Confirm v79's base
layout matches the v83 layout the branch was written for.

| Site | Gate | v79 lands | Confirm |
|---|---|---|---|
| `common/CMob.h:110` | `< 95` | included (base) | v79 CMob base layout == v83 base layout (size + the doom-field region) |

**Task-12 verdict (Cat D): NEEDS-CHANGE — v79 CMob base ≠ v83 base. `<95` gate self is fine, but the header
overshoots v79 by 0x30, and the doom region is ABSENT in v79.**

- **sizeof:** v79 `sizeof(CMob)` = **0x518 (1304)** — authoritative from the allocator `?CreateMob@@` @0x630BF0:
  `push 518h` → `ZAllocEx::Alloc(0x518)`. v83 `sizeof(CMob)` = **0x548 (1352)** (`?CreateMob@@` @0x66219D `push 548h`,
  and the v83 typed `CMob` reports size 1352). **v79 is 0x30 smaller than v83.**
- **CMob.h:110 (`unknown1` ZList, `<95`):** TRUE for both v79 and v83 → both include the field. That specific gate is
  correct for v79. But the *overall* base layout differs (below), so the header still needs a v79 branch.
- **Where the 0x30 lives (two parts):**
  1. **~0x10 inside `MobStat`** (sub-struct, separately gated by `MobStat.h`): v79 ctor writes `[esi+3A0h],0FFh`
     where v83 writes `[esi+3B0h],0FFh` (the `m_nTeamForMCarnival`/`m_rgHorz` region) — a clean 0x10 shift that
     originates in `m_stat` (`MobStat` @0x1A0 in both). Flag for the `MobStat.h` row (Cat E).
  2. **0x20 doom/reserved tail region — ABSENT in v79** (see doom verdict below).
- **Re-anchor (Step 5):** the v79↔v83 0x10 tail delta is independently confirmed by THREE landmarks across two
  different functions: (a) `m_mDelayedHPIndicator` ZMap v79@0x4D4 / v83@0x4E4; (b) `m_nHPpercentage`=100 default
  written by the ctor at v79 `[esi+510h]` / v83 `[esi+520h]`; (c) `m_bWaitingToBeSetTossed` cleared by
  `CMob::SetTemporaryStat` at v79 `and [esi+514h],0` (@0x63EAC4) / v83 `and [esi+524h],0` (@0x66FB3F).

### Doom-field verdict (doom-fix `dllmain.cpp:25`, `< 84` form) — NEEDS-CHANGE

**`m_bDoomReserved` does NOT exist in v79.** It is appended in v83+, not present in v79.

- v83 typed `CMob` (authoritative): `m_nHPpercentage`@0x520, `m_bWaitingToBeSetTossed`@0x524,
  **`m_bDoomReserved`@0x528**, `m_bDoomReservedSN`@0x52C (u8), `m_lpStatChangeReserved`@0x530 (ZList, ends 0x544).
  v83 ctor @0x6621D9 writes 0x524=0 then SKIPS 0x528 (writes 0x52C byte=0, 0x530 ZList) — i.e. v83 genuinely
  leaves `m_bDoomReserved` uninitialized → this is the real bug the doom-fix patches **on v83**.
- v79: `m_bWaitingToBeSetTossed`@**0x514** is the **last** member; `sizeof=0x518` ends one dword later. Mapping the
  v83 doom offset (0x528) down by the verified 0x10 tail delta gives **0x518 = exactly past the end** → the field,
  its SN byte, and `m_lpStatChangeReserved` are all **absent** in v79. Corroboration: the v79 ctor contains **no**
  trailing ZList-object init (no `m_lpStatChangeReserved`) and **no** tail byte-write (no SN) beyond the
  `m_mDelayedHPIndicator` ZMap @0x4D4 — unlike v83 which inits a ZList at 0x530.
- **Consequence:** the doom-fix gate `< 84` is TRUE for v79, so the hook executes `pThis->m_bDoomReserved = 0` — but
  v79's real `CMob` (game-allocated 0x518) has no such field. With the current header (which declares the doom region
  unconditionally) the write lands past the real object → **out-of-bounds / heap corruption on v79**. The fix is
  correct for v83 (field present-but-uninit) but **must be re-gated to exclude v79** (e.g. `== 83`), and `CMob.h`
  must give v79 a branch that omits `m_bDoomReserved`/`m_bDoomReservedSN`/`m_lpStatChangeReserved` (Task 17).
- **Corrects the Task-8 carry-forward:** Task 8 observed the right ctor facts (Alloc 0x518, highest write 0x514, no
  doom-zero block) but interpreted them as "field present-but-uninitialized." The v83 typed-struct comparison shows
  the field is **absent** in v79. Observable ctor behavior is identical; the distinction is decisive for the fix.

## Category E — upper-bound gates (v79 → base branch; spot-check size)

Every `>= 87`, `>= 95`, `>= 111`, `== 95`, `== 83`, `> 83` gate is **false** for
v79, so v79 takes the base ("v83 layout") branch. Individually low-risk, but a
silent v79≠v83 base-size delta would corrupt every downstream offset. **Spot-check
the struct *size* of each gated header against v79**, even when you expect v79 to
match v83. Headers in this class (non-exhaustive — the full gated set is 24):
`CLogin.h`, `CLogo.h`, `CConfig.h`, `ConfigSysOpt.h`, `COutPacket.h`,
`CClientSocket.h`, `CFadeWnd.h`, `CCtrlButton.h`, `CCtrlCheckBox.h`, `CWnd.h`,
`CUIWnd.h`, `CUITitle.h`, `CUILoginStart.h`, `CWvsContext.h`, `SecondaryStat.h`,
`PartyData.h`, `PartyMember.h`, `GuildData.h`, `MobStat.h`, `CMapLoadable.h`.

Special attention:
- `common/CWvsContext.h:98` (`> 83`) — v79 excluded → the 8-byte `m_aClientKey`
  block is treated as **absent** for v79. Confirm v79's `CClientSocket::OnConnect`
  / connect-hello does **not** encode an 8-byte client key (v84 found `>83` true →
  key present; for v79 the gate predicts the v83 *no-key* form). This also drives
  `bypass/socket_hooks.cpp:233` (`> 83`).
- `common/CLogin.h:235` (`== 83`) — **false for v79**, so the v83-only 20-byte
  member (a ZList between `m_abOnFamily` and `m_lNewEquip`) is **excluded** for
  v79. But v79 < 83: does v79 have that member, a different one, or none? If v79
  has it, the gate must become `== 83 || == 79` (or `<= 83`). Verify against v79
  `CLogin::CLogin`.
- `common/CMob.h` doom-field region — relevant to `doom-fix`. The v83 `CMob::CMob`
  ctor skips `m_bDoomReserved` (the bug the edit fixes). Confirm whether v79's ctor
  also skips it (so `doom-fix` is needed) or not. The `doom-fix` gate
  (`doom-fix/dllmain.cpp`, `< 84` form per the v84 audit) makes v79 take the
  needs-fix side — confirm that's correct for v79.

## Per-header verdict log (fill during the task)

For each of the 24 gated headers, record: v79 size, the deciding v79 disasm
line(s), every gated field's present/absent verdict, and the gate disposition
(unchanged / floor-lowered / v79-branch-added / rewritten). One row per header
minimum; expand to per-field where a gate boundary moves.

| Header | v79 size | Gate verdict | Deciding v79 evidence |
|---|---|---|---|
| CWvsApp.h | 0x60 (96) | **v79-branch-added** (Cat A, World A) @ :97 — `79` added to the `0x60` branch | task-008: ctor `??0CWvsApp@@QAE@PBD@Z` @0x942D3B writes the exact v83/84 base layout (vtable@0; @8,@0xC,@0x10,@0x14,@0x18,@0x1C; ZXString@0x20; ints @0x24..0x38) — NO below-floor surprise member; CWvsApp.h has no `<84`/`==83` member gate so v79 ≡ v83 by construction. Header computes 0x60 for v79; clang-cl static_assert(sizeof==0x60) **PASSED**. task-13 re-confirm: the `>=87` (`m_tNextSecurityCheck`, `m_pBackupBuffer`) and `>=95` (OS-version block, DX9, clearstack) gated fields are all FALSE for v79 → v79 base layout == v83/84 (0x60); the v79 `(==79\|\|==83\|\|==84)` branch already added Task 8 is correct. |
| CFuncKeyMappedMan.h | 0x388 (904) | **v79-member-shift CONFIRMED — DEFER to struct audit (Task 12/16)** (Cat A→B). No v79 assert written (a 0x388 assert FAILS to compile). | task-008 re-measure (2 independent anchors): TSingleton `CreateInstance` @0x946AFB `push 388h`→`Alloc(0x388=904)`; ctor `??0CFuncKeyMappedMan@@QAE@XZ` @0x569DE5 field-init extent = vtable@0 + memcpy 0x1BD@+4 + memcpy 0x1BD@+0x1C1 (arrays end @0x37E) + dwords zeroed @+0x380/+0x384 → ends 0x388. Header computes 0x3C8 (968): clang-cl scratch probe `assert_size(...,0x388)` for v79 FAILED with `expression evaluates to '968 == 904'` — proving a 0x40 below-floor MEMBER shift (the two `m_aQuickslotKeyMapped[8]` int arrays, 0x40 bytes, absent in v79), which a size-assert cannot express. NOTE: v79 cmake map `C_FUNC_KEY_MAPPED_MAN_CREATE_INSTANCE=0x009F9E98` is STALE (points to `sub_753F6A`, an unwind funclet) — flag for Task 7. |
| CUIToolTip.h | 0x514 (1300) | **unchanged** (Cat B `m_pLayerAdditional` absent; Cat C `m_pFontGen_Unknown` + `m_pCanvasEquip_Durability` absent) | ctor `??0CUIToolTip@@QAE@XZ` @0x842317: `m_pLayer`@0x10 then `m_aLineInfo[32]`@0x20 (no additional layer); fonts 0x424→0x470 then `m_pCanvasEquip_ReqItem`@0x474 (no `m_pFontGen_Unknown`); `m_pNumberGrowthDisable`@0x50C → `m_bIngoreWeddingInfo`@0x510 (no durability), sizeof 0x514 == v83. |
| CMob.h | **0x518 (1304)** | **NEEDS-CHANGE — v79 base ≠ v83 (0x548)** (Cat C `>=84` fields absent ✓; Cat D doom region **absent**) | `?CreateMob@@` @0x630BF0 `push 518h`→`Alloc(0x518)`. ctor @0x630C2C highest member write `mov [esi+514h],edi` = `m_bWaitingToBeSetTossed` (last member); NO doom-zero tail, NO trailing ZList init. v83 typed CMob (0x548) has `m_bDoomReserved`@0x528/SN@0x52C/`m_lpStatChangeReserved`@0x530 — **all absent in v79** (would start at 0x518=past end). 0x30 delta = ~0x10 in `MobStat` + 0x20 doom tail. doom-fix `<84` write would corrupt v79 → re-gate + header v79 branch in Task 17. |
| CMapLoadable.h | (base, no v79≠v83 delta found) | **unchanged** (Cat C `m_lVisibleByQuest` absent) | `?SetObjectState@CMapLoadable@@` @0x612aa4: `add ecx, 0A0h` → `m_mNamedObj` ZMap @0xA0 directly after `m_lpRefInfo`@0x8C; `m_lVisibleByQuest` (ZList) absent (else `m_mNamedObj` would be @0xB4). Matches v83 typed struct. |
| CLogin.h | **0x258 (600)** | **unchanged** (`==83` unk3[5] EXCLUSION CORRECT — v79 LACKS it) (task-13) | Alloc: `CLogo::LogoEnd` @0x5ffa57 `push 258h`→`Alloc(0x258)`. ctor `??0CLogin@@QAE@XZ` @0x5c93e7: `m_abOnFamily`(ZArray<int>)@**0x174** [dtor funclet @0x9dc669 `add ecx,174h`→sub_57407A; sub_57407A is `ZArray<int>::RemoveAll` — same fn destroys `CWvsContext+0x3548` (`m_aAdultChannel`, a `ZArray<int>`)] is **immediately** followed by `m_lNewEquip`(ZList)@**0x178** (ctor `lea eax,[esi+178h]; mov [eax],off_A2F9FC` vtable, dtor sub_5D1C40). Only 4 bytes between → NO 20-byte unk3[5] gap (which would push m_lNewEquip to 0x18C). v79<83 has neither the v83 `unk3[5]` nor any replacement → the `==83` exclusion is CORRECT. |
| CWvsContext.h | **static global @unk_B116A0; layout verified through last dtor member @0x36D4, + scalar tail (m_nScreenWidth..m_aPasssiveSkillBuffing[22]) ≈ **0x3740** total — NOT heap-alloc; no single Alloc immediate (UNCERTAIN exact byte)** | **unchanged** (`>83` m_aClientKey[8] ABSENT — gate correctly excludes for v79) (task-13) | ctor sub_94E1DC; `@0x94e1fe mov g_pWvsContext(0xB07848), ecx`; instance is a **static** (sub_94E152 `mov ecx, offset unk_B116A0; jmp ctor`), so no `Alloc(sizeof)`. **m_aClientKey absent** (confirms Task-4 + drives `socket_hooks.cpp:233` `>83`): `CClientSocket::OnConnect` @0x48cb81 connect-hello builds at @0x48d017 (`push 14h`) and encodes Encode4(charId)+Encode1+Encode1 — **no** `EncodeBuffer(m_aClientKey,8)`. **Embedded SecondaryStat = 0xB88 (2952)**: `SecondaryStat::Clear` sub_6F6D0C @[esi+212Ch] (m_secondaryStat) → `ForcedStat::Clear` @0x6f66ea @[esi+2CB4h] (m_forcedStat); BasicStat init sub_4E51DB @[esi+20B4h]. ⚠ CROSS-CHECK Task 15: confirm `SecondaryStat.h` computes **0xB88** for the v79/v83 base branch (a wrong embedded size shifts every field after m_forcedStat). |
| CClientSocket.h | **0x94 (148)** | **unchanged** (`>=111` dummy1 ABSENT; JMS dummy1 ABSENT) (task-13) | `?CreateInstance@?$TSingleton@VCClientSocket@@` @0x946aca `push 94h`→`Alloc(0x94)`. `>=111` gate false → no +0x14 pad dword; v79 base layout. |
| CLogo.h | **0x38 (56)** | **unchanged** (>=95 m_pLayerBackground/m_bVideoMode/m_videoState + >=95\|\|JMS m_bNXFadeOut + >=111 dummy1 all ABSENT) (task-14) | base = `CStage` (NOT CWnd → unaffected by the CWnd −8 base shift). ctor `??0CLogo@@QAE@XZ` @0x5ff8c4: CStage base (sub_468CA3, 4 vtables @0/4/8/0xC, size 0x18) then field-init extent zeroes 0x18(m_pLayerMain),0x1C(m_pLogoProp),0x24,0x28,0x2C,0x30,**0x34**(m_bNXFadeIn, last v79 member; 0x20=m_nLogoCount set in Init). sizeof = 0x38, byte-identical to v83 typed CLogo (0x38, m_bNXFadeIn@0x34). Vtables match Task-6 (primary off_A307BC, IUIMsgHandler off_A30770). |
| CFadeWnd.h | **0xCC (204)** | **NEEDS-CHANGE — inherits the CWnd −8 base shift (CWnd 0x64 vs v83 0x6C)** (own >=87 gate m_nLevel/m_nJobCode/m_nExpQuestID correctly ABSENT; REGION_GMS m_bUserAlarm present) (task-14) | `CFadeWnd : CDialog : CWnd`. v79 CDialog = **0x74** (v83 0x7C, −8). `?SetOption@CFadeWnd@@` @0x50b782 writes m_a0@**0x74** / m_t0@0x80 / m_pt0@0x8C / m_pt1@0x9C(end 0xA4) — every offset is −8 vs the v83 typed CFadeWnd (m_a0@0x7C). Tail: m_nType@0xBC, m_sInviter@0xC0, then **m_dwSN@0xC4 / m_dwFriendID@0xC8** directly (the >=87 3-int block ABSENT — else m_dwSN would be @0xD0). sizeof v79 = 0xCC vs v83 0xD4. Own gates are correct; header is wrong ONLY because of the inherited CWnd −8 overshoot (missing m_pAnimationLayer + m_pOverlabLayer). |
| CCtrlButton.h | **0x5A4 (1444)** | **unchanged** (>=95 m_sToolTipFromData ABSENT) (task-14) | base = `CCtrlWnd` (0x34, NOT CWnd → unaffected). ctor `??0CCtrlButton@@QAE@XZ` @0x422d59: CCtrlWnd::CCtrlWnd, then m_pPropFocusFrame@0x64, m_pLayerFocusFrame@0x68, eh-vector-ctor m_apPropButton[4]@0x6C, m_sToolTipTitle@0x84, m_sToolTipDesc@0x88, **m_uiToolTip CUIToolTip ctor @[esi+8Ch]** → m_uiToolTip@0x8C..0x5A0, m_bSelfDisable@0x5A0 (no ZXString init after m_uiToolTip → >=95 m_sToolTipFromData absent). sizeof 0x5A4 = v83 typed CCtrlButton. |
| CCtrlCheckBox.h | **0x6C (108)** | **unchanged** (>=95 m_nTextOffsetX/m_nTextOffsetY ABSENT) (task-14) | base = `CCtrlWnd` (0x34, anchored by its own ctor @0x4d4378: last member m_bShown@0x30 → 0x34; NOT CWnd → unaffected). Members are pure scalars/ptrs: m_nCheckBoxState@0x34 … m_apCanvasCheckBox[4]@0x5C..0x6C (last). sizeof 0x6C = v83 typed CCtrlCheckBox. No standalone v79 method symbol to spot-check, but the base is unaffected by the CWnd shift and the layout is gate-free below >=95 → high confidence. |
| CWnd.h | **0x64 (100)** | **NEEDS-CHANGE — v79 lacks `m_pAnimationLayer` + `m_pOverlabLayer` (CWnd 0x64 vs v83 0x6C, −8)** (>=95 UIOrigin enum + m_origin ABSENT ✓; >=87 SECPOINT ABSENT ✓; **m_ptCursorRel PRESENT in v79 @0x40/0x44** — the `<87` 2-int else-branch is CORRECT; the WRONG part is the two UNGATED secondary-layer com_ptrs) (task-14, RECONCILED) | CONTROL BASE CLASS. **−8 LOCKED by 3 independent v79↔v83 landmarks** (v83 typed struct = ground truth): (1) m_lpChildren ZList v79@**0x48**/v83@0x50 — `?Destroy@CWnd@@` v79@0x92f2f1 `lea eax,[esi+48h]`→ZList::AddTail vs v83@0x9e00af `lea eax,[esi+50h]`; (2) m_pFocusChild v79@**0x5C**/v83@0x64 — Destroy clears it (v79 `mov [esi+5Ch],ebx` / v83 `mov [esi+64h],ebx`); (3) sizeof(CDialog) v79@**0x74**/v83@0x7C — CFadeWnd::SetOption m_a0 (v79@0x50b782 `[ecx+74h]` / v83@0x51f93d `[ecx+7Ch]`). **Exact missing field PINNED:** v83 CWnd ctor @0x9de383 zeroes the 2 secondary-layer com_ptrs @0x1C (m_pAnimationLayer) / 0x20 (m_pOverlabLayer); v79 base ctor sub_92D5ED zeroes m_pLayer@0x18 then jumps to m_nBackgrndX@**0x38** (v83 @0x40) — NEVER touching 0x1C/0x20 → those 2 layers are ABSENT in v79. v79 map: m_pLayer@0x18, m_width@0x1C, m_height@0x20, m_rcInvalidated@0x24, m_nBackgrndX@0x38, m_nBackgrndY@0x3C, m_ptCursorRel_x/_y@0x40/0x44, m_lpChildren@0x48, m_pFocusChild@0x5C, m_pBackgrnd@0x60; sizeof 0x64. **[esi+0x70] RESOLVED (was a CONCERN):** the named `??0CWnd@@QAE@XZ` @0x4e168d = sub_92D5ED (CWnd base, ends m_pBackgrnd@0x60) + zeroing the **CDialog prefix** it is inlined into — 0x64=CDialog::m_nRet, 0x68=CDialog::m_bTerminate, 0x70=CDialog::m_pChildModal (ZRef@0x6C). These are NOT CWnd members (CDialog::m_nRet is CDialog's FIRST own member @0x64 = sizeof CWnd). Same codegen pattern in CDialog::CDialog zeroing [0x78]=CFadeWnd::m_a. sizeof(CWnd)=0x64 stands. |
| CUIWnd.h | **0x5A8 (1448)** | **NEEDS-CHANGE — inherits the CWnd −8 shift** (>=95 5-field block m_nSmallScreenX/Y,m_nLargeScreenX/Y,m_bIsLargeMode ABSENT ✓) (task-14) | CONTROL BASE CLASS. `CUIWnd : CWnd`(0x64). m_pBtClose(ZRef, 8B)@0x64, m_uiToolTip CUIToolTip(0x514)@**0x6C**..0x580, m_nUIType@0x580, **m_nBtCloseType@0x584** (`?OnCreate@CUIWnd@@` @0x8862c7 switches on [esi+584h] 1..4 = close-btn corner), … **m_sBackgrndUOL(ZXString)@0x5A4** last (`?ReloadBackgrnd@CUIWnd@@` @0x8866dc `lea ecx,[esi+5A4h]`→ZXString op + cond@[esi+590h]). Every offset is −8 vs v83 typed CUIWnd (m_nBtCloseType@0x58C, m_sBackgrndUOL@0x5AC, size 0x5B0). >=95 block absent (else m_sBackgrndUOL@0x5B8). sizeof v79 = 0x5A8. The −8 originates entirely in the CWnd base (missing m_pAnimationLayer + m_pOverlabLayer). |
| CUITitle.h | **~0x5E4 (UNCERTAIN ±0xC)** | **NEEDS-CHANGE — inherits the CWnd/CDialog −8 shift** (>=95 CFadeWnd-vs-CDialog base swap + >=95 m_rcRMA + JMS m_unk all ABSENT for v79 ✓) (task-14) | `CUITitle : CDialog`(v79 0x74) for v79 (the >=95 `: CFadeWnd` form excluded). `?EnableLoginCtrl@CUITitle@@` @0x5f3192 drives ZRef<CCtrlButton> members at [esi+8Ch],[esi+94h],[esi+0CCh],[esi+0D4h] via vtbl+0x1C (SetEnable) — all shifted from the v83 typed CUITitle (m_pBtLogin@0x90 …). Embeds CUIToolTip m_uiToolTipTitle (0x514) as last member (v83 @0xD8 → v79 ~0xD0 → size ~0x5E4 vs v83 0x5EC). ⚠ exact internal offsets don't cleanly match a uniform −8 (TSingleton base padding @0x7C complicates) → size UNCERTAIN; the NEEDS-CHANGE (base-driven) verdict is firm. Task-17 must re-pin offsets. |
| CUILoginStart.h | **~0xD0 (UNCERTAIN)** | **NEEDS-CHANGE — inherits the CWnd/CDialog −8 shift** (>=95 m_pFont/m_pCanvasChannelName ABSENT ✓; REGION_GMS 5-element m_aBtParam/m_apButton arrays selected ✓) (task-14) | `CUILoginStart : CDialog`(v79 0x74). No v79 symbol located (EnableLoginStartCtrl unnamed/inlined). Header (v79): m_pLogin@0x74, m_aBtParam[5] (CREATEPARAM 8B each = 0x28), m_apButton[5] (ZRef 8B each = 0x28), m_nViewWorldButtonType, m_bRequestSent → ~0xD0. Could not anchor a v79 member offset → size UNCERTAIN; NEEDS-CHANGE because its CDialog/CWnd base carries the verified −8 shift (missing m_pAnimationLayer + m_pOverlabLayer). |
| CConfig.h | **0x418 (1048)** | **unchanged** (base GMS branch; `>=111` & `==95` false; no underflow) (task-13) | `_WinMain` @0x93fbd8 `push 418h`→`Alloc(0x418)`, then ctor `??0CConfig@@QAE@XZ` @0x49392c. Our struct is v95-shaped (≥1592) ≥ v79 real 1048 → NO heap overflow; `static_assert(sizeof≥1072)` passes. ⚠ NOTE: v79 real 0x418=1048 is BELOW the header comment's "smallest GMS real size (1072)"; harmless (our struct is oversized, assert is `>=`) but the comment's floor is inaccurate for v79 — flag for a Task-17 doc fix. |
| ConfigSysOpt.h | **0x30 (48)** | **unchanged** (`>=95` LargeScreen+WindowedMode ABSENT) (task-13) | **v79 binary anchor (slab copy):** `CConfig::ApplySysOpt` @0x4960f9 copies the passed `CONFIG_SYSOPT*` into `this->m_sysOpt` via `496108 push 0Ch` / `49610a lea edi,[ebx+64h]` / `49610d pop ecx` / `49610e rep movsd` → 0xC dwords = **0x30 bytes**, so `sizeof(CONFIG_SYSOPT)=0x30` and `m_sysOpt`@CConfig+0x64. (Earlier 12-int field-count agreed but was circular; this `rep movsd` immediate is the concrete anchor.) A present `>=95` `bSysOpt_LargeScreen`/`bSysOpt_WindowedMode` pair would make ApplySysOpt copy 0xE dwords (0x38) — it copies 0xC → pair absent. Windowed-mode CROSS-CHECK (`C_CONFIG_SYS_OPT_WINDOWED_MODE`=0xB11548, Task 8): 0xB11548 is a **standalone global** (xrefs: `CWvsApp::SetUp` @0x9432bd, `CreateMainWindow` @0x9441b6, `InitializeGr2D` @0x944cf1) — NOT a CONFIG_SYSOPT member (CConfig is heap-alloc'd at 0x418, cannot contain a fixed 0xB11548 address). Consistent with the `>=95` gate correctly excluding `bSysOpt_WindowedMode` from v79's CONFIG_SYSOPT. |
| COutPacket.h | **0x10 (16)** | **unchanged** (`>=111` dummy1 ABSENT) (task-13) | ctor `??0COutPacket@@QAE@J@Z` @0x67ad6b + `Init` @0x67ae68: m_bLoopback@0, m_aSendBuff(ZArray)@4 (dtor `add ecx,4; ZArray::RemoveAll` @0x9eacac), m_uOffset@8 (`and [esi+8],0`), m_bIsEncryptedByShanda@0xC (`and [esi+0Ch],0`) → highest member @0xC, sizeof 0x10. No +0x10 v111 dummy. `static_assert(sizeof≥0x10)` holds. |
| SecondaryStat.h | ☐ | ☐ | |
| PartyData.h | ☐ | ☐ | |
| PartyMember.h | ☐ | ☐ | |
| GuildData.h | ☐ | ☐ | |
| MobStat.h | ☐ | ☐ | |

## Cross-version safety (FR-13)

Every gate amendment must keep v83/v84/v87/v95/v111/JMS185 selecting their current
branch. Adding `79` to an enumerated list, or lowering a `>= 83` floor to `>= 79`,
must not change any other version's truth value — verify each rewrite's truth table
across the full matrix before committing. CI builds all versions on PR; a green
matrix is the final confirmation.

### Task-008 (Cat A) truth table — verified

`CWvsApp.h` (only change: `79 ||` prepended to the `0x60` branch; disjoint `==79` cannot flip any other version):

| Version | Branch selected | assert_size | Changed? |
|---|---|---|---|
| GMS 79 | `(==79\|\|==83\|\|==84)` | 0x60 | NEW (was: no branch) |
| GMS 83 | `(==79\|\|==83\|\|==84)` | 0x60 | unchanged |
| GMS 84 | `(==79\|\|==83\|\|==84)` | 0x60 | unchanged |
| GMS 87 | `==87` | 0x6C | unchanged |
| GMS 95 | `>=95` | 0x8C | unchanged |
| GMS 111 | `>=95` | 0x8C | unchanged |
| JMS 185 | `REGION_JMS` | 0x64 | unchanged |

`CFuncKeyMappedMan.h` (World B — gate logic UNCHANGED; only a deferral comment added):

| Version | Branch selected | assert_size | Changed? |
|---|---|---|---|
| GMS 79 | none (deferred to Task 12/16) | (no assert — same as before) | unchanged |
| GMS 83/84/87 | `(==83\|\|==84\|\|==87)` | 0x3C8 | unchanged |
| GMS 95 | `==95` | 0x3CC | unchanged |
| GMS 111 | `==111` | 0x3D0 | unchanged |
| JMS 185 | `REGION_JMS` | 0x400 | unchanged |

**Empirical verification (clang-cl + xwin via `scripts/wsl-build.sh`, target `bypass`):**
- `wsl-build.sh GMS 79 1 bypass` → `>> OK` (links `bypass-1.0.0.dll`). Compiles `app_hooks.cpp` (CWvsApp.h, new v79 0x60 assert PASSES) + `CFuncKeyMappedMan.cpp`/`key_mapped_hooks.cpp` (CFuncKeyMappedMan.h, no v79 assert).
- `wsl-build.sh GMS 83 1 bypass` → `>> OK` (neighbor sanity; v83 still selects its original branches).
- World-B scratch probe (`assert_size(sizeof(CFuncKeyMappedMan),0x388)` for v79) → compile FAILED `static assertion failed ... expression evaluates to '968 == 904'`; probe reverted.
- `cmake -P cmake/CheckMemoryMapKeys.cmake` (GMS 79.1) → `OK: all 159 keys defined and non-empty`.

**Build-blocked status:** v79 was NOT genuinely build-blocked before this change — both headers compiled (the unmatched v79 simply fired no `assert_size`, silently losing its guard). The CWvsApp amendment restores the guard; CFuncKeyMappedMan's guard remains deferred pending the member-gate fix.
</content>
