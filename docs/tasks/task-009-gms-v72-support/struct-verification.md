# GMS v72.1 — Struct Size/Layout Verification & Below-Floor Gate Audit

Scope (per confirmed PRD): verify **all 24 version-gated `common/*.h` headers** against
the v72 binary, **amend every enumerated gate that has no v72 branch**, and — for every
two-way `>= 83 || JMS` gate task-008 created — **confirm whether v72 shares v79's reduced
branch or requires a distinct v72/`< 79` branch**.

Follow the read-only verification discipline from `docs/version-porting-workflow.md`:
anchor every verdict to v72 disassembly, and do **not** apply speculative struct types
into the IDB during verification (decompiler leak). Confirm the connected IDB with
`get_metadata` first ([[feedback-verify-ida-target]]) — the v72 IDB is sparse/unverified.
Do not defer a settleable question with "out of scope" / "unlikely to matter"
([[feedback-prefer-confirmation]]) — doubly load-bearing for v72 because **there is no
labeled IDB below v72**, and the branch it lands on is sometimes the *v79-reduced* branch,
which already diverged from the v83 base.

## The two-tier below-floor problem in one paragraph

Every supported version is now ≥ 79 (task-008 made v79 the floor). v72 falls below it, so
for every gate the question is: which branch does v72 land on, and is that branch's layout
actually v72's layout? Four failure modes, in priority order:

1. **No branch at all** — enumerated `==` branches task-008 added *for v79*
   (`CWvsApp.h:97`, `CFuncKeyMappedMan.h:50`) and the `==83` member gate
   (`CLogin.h:235`). v72 selects nothing → lost size guard / silent member drop. **Fix
   first.**
2. **v79-reduced branch, v72 maybe further-reduced** — the two-way `>= 83 || JMS` gates
   (`CWnd.h:25`, `CMob.h:239`, `MobStat.h:128`, `CFuncKeyMappedMan.h:18`,
   `CUIToolTip.h:92`). v72 inherits v79's reduced layout. Confirm v72 == v79; split
   three-way if v72 diverges. **The novel risk.**
3. **Base/excluded branch** — `>= 84/87/95/111`, `== 95/87/83`, `> 83`, `< 95`. v72 takes
   the same side as v79. Spot-check size; a silent v72≠v79 base delta is the quietest
   failure.
4. **CWnd cascade** — a v72≠v79 CWnd shift propagates to 5 derived UI classes; treat as one
   linked verdict.

## Category A — enumerated gates with NO v72 branch (fix first; guard-restoring)

These select by explicit version equality and have no catch-all `#else`, so v72 selects
**nothing** (size guard silently lost) or drops a member. Amend each to give v72 a
defined, layout-correct, size-asserted branch (decide which by v72 disassembly).

| Site | Current structure | v72 result | Required action |
|---|---|---|---|
| `common/CWvsApp.h:97` | `(== 79 \|\| == 83 \|\| == 84)` / `:99 == 87` / `>= 95` | no branch | Verify v72 `CWvsApp` layout; add `72` to the `0x60` branch (v79 joined it). |
| `common/CFuncKeyMappedMan.h:50` | `== 79` (0x388 assert) / `:52 (==83\|\|84\|\|87)` / `==95` / `==111` | no branch | Member gate `:18` already excludes v72; confirm v72 size, add `72` to the matching size-assert branch (`== 72` or `== 72 \|\| == 79`). |
| `common/CLogin.h:235` | `== 83` (member-declaring `unk3[5]`) | false → excluded | Confirm v72 genuinely lacks `unk3[5]` (like v79). If v72 has it, gate becomes `== 83 \|\| == 72` (or `<= 83`). |

> Re-grep at task start — other enumerated-no-`else` gates may exist or line numbers may
> have shifted: `grep -rn "BUILD_MAJOR_VERSION ==" common/*.h`.

## Category B — two-way `>= 83 || JMS` gates (v72 inherits v79's reduced branch; CONFIRM or SPLIT)

task-008 created these splits to exclude v79; they now exclude v72 too. For each, confirm
**v72 == v79** (gate already correct, record size) or **v72 diverges further** (split
three-way: keep v79's branch, add a distinct v72/`< 79` branch). This is the highest-value,
highest-novelty work in the task.

| Site | Field gated out for v79 | v79 reduced size | Must confirm for v72 | If v72 diverges |
|---|---|---|---|---|
| `common/CWnd.h:25` | `m_pAnimationLayer` + `m_pOverlabLayer` | CWnd 0x64 (−8 vs v83 0x6C) | v72 `sizeof(CWnd)` via Destroy/ctor landmarks | Split; re-derive all 5 CWnd-derived classes (cascade) |
| `common/CMob.h:239` | doom tail (`m_bDoomReserved`/SN/`m_lpStatChangeReserved`) | CMob 0x518 (vs v83 0x548) | v72 `?CreateMob@@` Alloc immediate + ctor highest write | Split; re-gate `doom-fix` (see Cat C) |
| `common/MobStat.h:128` | Weakness group (`nWeakness_/rWeakness_/tWeakness_`) | MobStat 0x1F8 (vs v83 0x208) | v72 MobStat tail via CMob ctor `lea` to last member | Split |
| `common/CFuncKeyMappedMan.h:18` | quickslot pair (`m_aQuickslotKeyMapped[8]`×2) | 0x388 (vs v83 0x3C8) | v72 CreateInstance Alloc immediate + ctor extent | Split + matching size-assert branch (Cat A) |
| `common/CUIToolTip.h:92` | `m_pLayerAdditional` | 0x514 (== v83) | v72 ctor `m_pLayer` → `m_aLineInfo` adjacency | Lower floor / split as evidence dictates |

## Category C — base/excluded branch (v72 sides with v79; spot-check size)

Every `>= 84`, `>= 87`, `>= 95`, `>= 111`, `== 95`, `== 87`, `== 83`, `> 83`, `< 95`,
`< 84` gate resolves v72 onto the base/excluded branch (same side as v79). Individually
low-risk, but a silent v72≠v79 base-size delta corrupts every downstream offset.
**Spot-check the struct *size* of each gated header against v72.** Headers in this class
(non-exhaustive — the full gated set is 24): `CLogin.h`, `CLogo.h`, `CConfig.h`,
`ConfigSysOpt.h`, `COutPacket.h`, `CClientSocket.h`, `CFadeWnd.h`, `CCtrlButton.h`,
`CCtrlCheckBox.h`, `CWnd.h` (base), `CUIWnd.h`, `CUITitle.h`, `CUILoginStart.h`,
`CWvsContext.h`, `SecondaryStat.h`, `PartyData.h`, `PartyMember.h`, `GuildData.h`,
`MobStat.h`, `CMapLoadable.h`.

Special attention:
- `common/CWvsContext.h` (`> 83` `m_aClientKey[8]`) — v72 excluded → key treated absent
  (v79 confirmed absent). Confirm v72 `CClientSocket::OnConnect` connect-hello does **not**
  encode an 8-byte client key. Drives `bypass/socket_hooks.cpp` (`> 83`).
- `doom-fix/dllmain.cpp` (`== 83` write gate, post-task-008) — **false for v72** → write
  not applied. Confirm `m_bDoomReserved` is genuinely absent in v72 (so exclusion is
  correct and the write would have been OOB), as task-008 found for v79.
- `common/SecondaryStat.h` embedded size (v79/v83 base = 0xB88) — confirm v72 computes the
  same embedded size; a wrong embedded size shifts every field after `m_forcedStat` in
  `CWvsContext`.

## Per-header verdict log (fill during the task)

For each of the 24 gated headers, record: v72 size, the deciding v72 disasm line(s), every
gated field's present/absent verdict, the gate disposition (unchanged / branch-added /
two-way-confirmed-shares-v79 / split-three-way / rewritten), and — for the v79-reduced
branch headers — an explicit **v72-vs-v79** comparison. One row per header minimum; expand
to per-field where a gate boundary moves.

| Header | v72 size | v72 vs v79 | Gate verdict | Deciding v72 evidence |
|---|---|---|---|---|
| CWvsApp.h | 0x60 | == v79 (both 0x60) | branch-added (72 → 0x60 branch :97) | WinMain stack-ctor @0x8EF809 → this=ebp-0xF4; highest field @+0x5C (m_ahInput[2]), next local @+0x6C → 0x60; ctor @0x8F26C7 field-init through +0x38 matches v79 |
| CFuncKeyMappedMan.h | 0x388 | == v79 (both 0x388) | branch-added (72 → ==79 0x388 branch :50) | task-2: CreateInstance Alloc(0x388) + ctor extent; member gate :18 (>=83\|\|JMS) excludes v72 → quickslot pair absent → header computes 0x388 |
| CUIToolTip.h | **0x50C** (full) | == v79 (ctor byte-identical) | two-way-confirmed-shares-v79 (gate :92 correct) + all upper gates absent | v72 ctor 0x7F9C33 vs v79 0x842317: both `mov [esi+10h]`=m_pLayer then eh-vector-ctor m_aLineInfo[32]@esi+0x20 (elem 0x20 → CLineInfo lacks `>=95` m_bUseDotImage), then identical post-array writes @0x424+. No m_pLayerAdditional (`>=83`, would push array to 0x24); no m_aOptionLineInfo (`>=95\|\|JMS`); no Gen_Unknown(`>=84`)/H_White(`>=87`)/Stan_Prp(`>=87`)/Stan_Dsc..Skill_Dsc(`>=95`); no Durability(`>=84`). **Full size from CCtrlButton embed: alloc 0x59C − m_uiToolTip@0x8C − m_bSelfDisable(4) = 0x50C** (Task 14); == v79 (byte-identical ctor). ⚠ Header field-list bottom-up computes 0x514 — an 8-byte / 2-field overcount in the shared `<87` font/canvas region (no assert_size; shared v72==v79; not gate-relevant — flag for header owner). Plan-table "v79 0x514==v83" was a planning error: v83 has m_pLayerAdditional (0x518) while v72/v79 = 0x50C. |
| CMob.h | 0x4C0 | **DIVERGES**: v72 0x4C0 vs v79 0x518 (−0x58) | **split-three-way** (size diverges; doom-tail field itself absent in both) | CreateMob 0x611C9F `push 4C0h`; ctor 0x611CDB highest write `[esi+4B8h]`. v79 CreateMob 0x630BF0 `push 518h`. Doom tail ABSENT both. **Task 15 PINS the −0x58:** FRONT −0x28 = CLife base −4 (m_nMobChargeCount v72@0x84 vs v79@0x88) + **REGION_GMS block `CMob.h:97–105` (m_bAttackReady…m_effectAttack, 0x24) ABSENT in v72** (v72 1st ZList@0x88 directly after m_nMobChargeCount@0x84; v79 @0xB0 after the 0x24 block); MobStat embed −0x20; TAIL −0x10 (ungated, not byte-pinned). Task-17: gate `:97` block `>=79`; CLife −4 + tail −0x10 not CMob.h-gateable (flag). |
| CMapLoadable.h | 0x114 (276) | == v79 (both <84/<95; front == v95 dump) | base/excluded — `>=84` m_lVisibleByQuest, `>=95` m_bField/m_lpLayerLetterBox/m_bPlayHoldedBGM+m_tPlayHoldedBGM ABSENT | Front anchor: PrepareNextBGM 0x5F3496 `mov [esi+1Ch]`=m_tNextMusic (CStage base 0x18 == v95 dump). Size = v95 0x148 − m_bField(4) − m_lVisibleByQuest(0x14) − m_lpLayerLetterBox(0x14) − m_bPlayHolded pair(8) = 0x114; corroborated by task-006 v84=0x128 (= 0x114 + m_lVisibleByQuest 0x14). No assert_size; tail not independently anchored (Task 15). |
| CLogin.h | 0x23C (572) | v72 0x23C < v79 0x258 < v83 0x2C8; v72 has 1 ZList block, v79/v83 have 2 | unchanged (==83 correctly excludes v72; no edit) | Size: LogoEnd `Alloc(0x23C)` → CLogin::CLogin 0x5AECED → set_stage (sig-cat:389). Member: v72 ctor builds ONE ZList (off_9D317C @this+0x174 = m_lNewEquip) then m_aCmd[5] @+0x1C8. v79 @0x5C94AD/0x5C94C3 and v83 @0x5F3D32/0x5F3D42 build TWO ZLists (unk3@+0x1A0 v83). v72 lacks the unk3 ZList → member absent |
| CWvsContext.h | ≥0x3520 (lower bound) | == v79 base shape (no client key) | base/excluded — `>83` m_aClientKey[8] **ABSENT**; all `>=87`/`>=95` upper fields absent; 52-slot equip arrays (`>=87` #else) | m_aClientKey: OnConnect 0x48528f PLAYER_LOGGED_IN send = Encode4/Encode1/Encode1, **NO EncodeBuffer(key,8)** (Task 4/sig-cat §8-byte-client-key). Size: high-offset field accesses — WinMain `[g_pWvsContext+3510h]`, ApplySysOpt writes [+0x3504]/[+0x3508], SetUp 0x8f2c99 reads [+0x351C] → sizeof≥0x3520. m_dwCharacterId @+0x20A0 base-consistent (SecondaryStat embed not shifted). No assert_size; exact alloc not gate-relevant (singleton creator not pinned in budget) |
| CClientSocket.h | 0x94 (148) | == v79 (both 0x94) | base/excluded — `>=111` dummy1 ABSENT | TSingleton<CClientSocket>::CreateInstance 0x8f621f → `push 94h`/ZAllocEx::Alloc → ctor 0x484c95. v72<111 → no +0x14 dummy1. JMS dummy1 N/A (REGION_GMS) |
| CLogo.h | 0x38 | == v83/84/87 (all 0x38) | base/excluded — `>=95` LayerBackground/VideoMode/videoState, `>=95\|\|JMS` NXFadeOut, `>=111` dummy1 ABSENT; assert `>=0x38` holds | Two anchors: CWvsApp::SetUp 0x8f2f2f `push 38h`→Alloc→CLogo::CLogo 0x5e11f9; ctor highest field write `[esi+34h]` (m_bNXFadeIn) → 0x38. (Task 14) |
| CFadeWnd.h | ≈0xC4 (CDialog→CWnd base) | == v79 (CWnd shares) | base/excluded — `>=87\|\|JMS` m_nLevel/m_nJobCode/m_nExpQuestID ABSENT; CWnd cascade NOT triggered | INDEP v72 anchor: ctor 0x4FFD72 calls CWnd::CWnd then writes own fields @0xA4/0xB0/0xBC(=-1)/0xC0 → highest 0xC0 (m_dwFriendID) → 0xC4. `>=87` block absent (writes jump straight to m_dwSN/m_dwFriendID). REGION_GMS m_bUserAlarm present (GMS). (Task 14) |
| CCtrlButton.h | 0x59C | == v79 base shape | base/excluded — `>=95` m_sToolTipFromData ABSENT | Two anchors: CUIFadeYesNo::OnCreate 0x500921 `push 59Ch`→Alloc→CCtrlButton::CCtrlButton 0x422954; ctor embeds m_uiToolTip(CUIToolTip)@+0x8C, dtor funclets show only m_sToolTipTitle@0x84 + m_sToolTipDesc@0x88 ZXStrings (no m_sToolTipFromData) then m_bSelfDisable@0x598 → 0x59C. (Task 14) |
| CCtrlCheckBox.h | CCtrlWnd base + 0xB ints/ptrs (exact size not pinned — stripped) | base/excluded (arithmetic) | base/excluded — `>=95` m_nTextOffsetX/m_nTextOffsetY ABSENT (v72<95) | Ctor stripped (no `??0CCtrlCheckBox@@` symbol in v72 IDB). Verdict by version arithmetic: `>=95` gate FALSE for v72 → 2 text-offset ints absent. Shares CCtrlWnd base with CCtrlButton (anchored 0x59C). No assert_size → exact size lower-bound acceptable. (Task 14, DONE_WITH_CONCERNS) |
| CWnd.h | 0x64 (base subobject; ≡ v79) | == v79 (ctor + base helper byte-identical) | **two-way-confirmed-shares-v79** — CASCADE NOT TRIGGERED | v72 ctor 0x4D9043 + base helper sub_8DD47E vs v79 ctor 0x4E168D + sub_92D5ED: identical field set. Base helpers jump m_pLayer@0x18→m_nBackgrndX@0x38 (anim layers @0x1C/0x20 ABSENT both, `>=83\|\|JMS`); `>=87\|\|JMS` SECPOINT == `#else` 2 ints (same 8 bytes); `>=95` m_origin + UIOrigin enum absent. **Task 14 resolves the 0x64-vs-0x74 nit: CUIWnd's first own member m_pBtClose lands at +0x64 (ctor sub_83C0EC), proving the CWnd base subobject = 0x64; the ctor's 0x68/0x70 writes are inside CWnd's m_pBackgrnd/ZArray region, not extra members. == v79.** |
| CUIWnd.h | ≈0x5A4 (CWnd base) | == v79 (CWnd shares) | base/excluded — `>=95` m_nSmallScreenX/Y/m_nLargeScreenX/Y/m_bIsLargeMode ABSENT; cascade NOT triggered | INDEP v72 anchor: ctor sub_83C0EC calls CWnd base helper sub_8DD47E, m_pBtClose ZRef@0x64, embeds m_uiToolTip(CUIToolTip 0x50C)@0x6C→ends 0x578, then 7 dense ctor-arg fields 0x57C–0x594 + [0x59C]/[0x5A0]=0 → ~0x5A4. NO `>=95` +0x14 gap (fields packed). (Task 14) |
| CUITitle.h | ≈0x5E4 (base = **CDialog**) | == v79 (CWnd shares) | base/excluded — base-class swap `>=95\|\|JMS`→CFadeWnd is FALSE (v72 base = CDialog); `>=95\|\|JMS` m_rcRMA + JMS m_unk ABSENT; cascade NOT triggered | INDEP v72 anchor: ctor sub_5D3BFB (singleton UITitleInstanceAddr@0xAA5114) calls **CWnd::CWnd** (CDialog inlined; NOT CFadeWnd) → confirms `#else` CDialog base. m_pCanvasRMA[2]@0x80 immediately followed by button ZRefs @0x88 — NO 16-byte m_rcRMA gap (`>=95\|\|JMS` absent). m_uiToolTipTitle(CUIToolTip 0x50C)@0xD8 → ≈0x5E4. (Task 14) |
| CUILoginStart.h | CDialog base (exact size not pinned — stripped) | == v79 (CWnd shares, arithmetic) | base/excluded — `>=95\|\|JMS` m_pFont + m_pCanvasChannelName ABSENT (v72<95); REGION_GMS 5-element m_aBtParam/m_apButton arrays (v72=GMS); cascade NOT triggered | Ctor stripped (no symbol; CDialog inlined so no CDialog::CDialog xref). Verdict by version arithmetic + cascade: `>=95\|\|JMS` gate FALSE → font/canvas pair absent; REGION_GMS → 5-element arrays. CDialog-derived sibling of CUITitle (anchored). No assert_size → lower-bound acceptable. (Task 14, DONE_WITH_CONCERNS) |
| CConfig.h | real 0x3FC (1020); our struct v95-shaped (≥1592) | v72 real 1020 < v83 1072 < our floor | base/excluded — `>=111` m_v111Pad ABSENT, `==95` assert branch false → base `>=1072` assert | WinMain `push 3FCh` @0x8ef7ce → ZAllocEx::Alloc → CConfig::CConfig 0x48c0d3. Our oversize struct (≥1592) >> real 1020 → no heap overflow; compile-time assert `>=1072` holds. (CConfig instance 0xAA3AC0 + 0x3FC = end 0xAA3EBC) |
| ConfigSysOpt.h | base (ends at bSysOpt_Minimap_Normal) | == v83 base | base/excluded — `>=95` bSysOpt_LargeScreen + bSysOpt_WindowedMode ABSENT | v72 windowed-mode is a **standalone global** g_CConfig_SysOpt_WindowedMode @0xAA87AC (SetUp 0x8f2c49 sets =0x10; readers CreateMainWindow/InitializeGr2D — sig-cat:490), OUTSIDE the CConfig instance (0xAA87AC > instance end 0xAA3EBC) → NOT a CONFIG_SYSOPT field. `>=95` gate correctly excludes it from v72's embedded CONFIG_SYSOPT |
| COutPacket.h | 0x10 (base) | == v83/v79 (both 0x10) | base/excluded — `>=111` dummy1 ABSENT | ctor 0x656fa1: `and [esi+4],0` + ZArray _Alloc(0x100) on [esi+4]=m_aSendBuff, then Init(seq) (m_uOffset@8 / m_bIsEncryptedByShanda@0xC / m_bLoopback@0); unwind funclet RemoveAll on [esi+4]. Single ZArray member, no +0x10 field → 0x10. static_assert `>=0x10` holds |
| SecondaryStat.h | **0xAB0** (2736) | **DIVERGES**: v72 0xAB0 vs v79/v83 0xB88 (−0xD8) | base/excluded — `>=87` DojangShield/AssistCharge/Enrage, `==87` byte-variants (v72→#else int), `>=84` Flying/Frozen, `>=95` blocks ALL ABSENT; **+ NEW ungated base shrink −0xD8 (~18 SecureTear slots)** | Embedded @CWvsContext+0x212C in BOTH (OnTemporaryStatReset: v72 0x918F3C / v79 0x96AB32 `add edi,212Ch`). aTemporaryStat[7] (last member, 0x1C): [0]/RideVehicle v72@SecondaryStat+0xA94 vs v79@0xB6C; [4]/GuidedBullet v72@0xAA4 vs v79@0xB7C — both Δ=−0xD8. v72 sizeof = 0xA94+0x1C = **0xAB0**; v79 = 0xB6C+0x1C = 0xB88 (independently re-confirms 0xB88). Reset 0x6CA91A confirms field layout matches header (nPAD_ @+0xC). **SIZE-CRITICAL:** shifts m_forcedStat + all CWvsContext members after m_secondaryStat by −0xD8 (pre-stat region @≤0x212C unaffected). Exact 18 missing base stats not pinned (Task 15 concern). |
| PartyData.h | ☐ | ☐ | ☐ (packed) | |
| PartyMember.h | ☐ | ☐ | ☐ (packed) | |
| GuildData.h | ☐ | ☐ | ☐ (packed) | |
| MobStat.h | **0x1D8** (472) | **DIVERGES**: v72 0x1D8 vs v79 0x1F8 (−0x20) | **split-three-way** (size diverges; Weakness field itself absent in both) | **Firm anchors:** SetFrom v72 0x6D0896 `push 1D8h` memset (=sizeof); CMob ctor `lea eax,[ebx+1C4h]`=lBurnedInfo@0x1C4 (+0x14=0x1D8). v79 SetFrom 0x702675 `push 1F8h`; lBurnedInfo@0x1E0. **Task 15 PINS the −0x1C (sizeof −0x20):** nFs (long double) v72 `fstp[edi+1B8h]`@0x1B8 vs v79 `fstp[edi+1D0h]`@0x1D0 → status region `[0x94,nFs)` is **0x18 (6 ungated status ints) shorter in v72**; PLUS **`int bDisable` (MobStat.h:152) ABSENT in v72** (v72 copies 1 post-nFs field@0x1C0; v79 copies 2 @0x1D8+0x1DC). +0x4 v79 align-pad → sizeof −0x20. (Task 12 "−0x1C" = lBurnedInfo position delta.) Task-17: gate bDisable + 6 status ints `<79`; 6 ints named-candidate. |

## Cross-version safety (FR-13)

Every gate amendment must keep v79/v83/v84/v87/v95/v111/JMS185 selecting their current
branch. Adding `72` to an enumerated list, or splitting a two-way gate into three-way, must
not change any other version's truth value — verify each rewrite's truth table across the
full matrix before committing. Keep v72's exclusion **disjoint** (a `== 72` term, or a
`< 79` term that no supported version satisfies) so it cannot flip another version. CI builds
all versions on PR; a green matrix is the final confirmation.

### Three-way split template (fill per split)

When a Category B gate splits, record the truth table. Example skeleton for a struct where
v72 reduces further than v79:

| Version | Branch selected | size | Changed? |
|---|---|---|---|
| GMS 72 | `== 72` (or `< 79`) | (v72 size) | NEW |
| GMS 79 | `== 79` (or `>= 79 && < 83`) | (v79 size) | unchanged |
| GMS 83/84/87 | `>= 83` | (v83 size) | unchanged |
| GMS 95/111 | `>= 95` tail | … | unchanged |
| JMS 185 | `REGION_JMS` | … | unchanged |

**Empirical verification (fill):** `scripts/wsl-build.sh GMS 72 1` → `>> OK`;
`scripts/wsl-build.sh GMS 79 1` (neighbor sanity) → `>> OK`;
`cmake -DREGION=GMS -DMAJOR=72 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake` → `OK`;
preprocess (`gcc -E`, `{72,79,83}`) confirming each split field's present/absent at each
version.

## Task 3 (Category A) amendment record — early gate restoration

Live gate sites re-grepped (`grep -rn "BUILD_MAJOR_VERSION ==" common/*.h`): `CWvsApp.h:97`,
`CFuncKeyMappedMan.h:50`, `CLogin.h:235` (the three Category-A sites, line numbers unchanged
from plan time). Other `== N` hits — `CConfig.h:84` (==95), `CLogo.h:93` (==95),
`SecondaryStat.h:405/419` (==87||JMS) — are upper-version member gates false for v72 that
resolve onto the base branch (Category C, Tasks 13–15), not amended here.

**Edits applied** (house `#if/#elif/#endif` form, one-line `// v72 size verified task-009` comment each):
- `common/CWvsApp.h:97` — added `BUILD_MAJOR_VERSION == 72` to the `0x60` branch → `(72 || 79 || 83 || 84)`.
- `common/CFuncKeyMappedMan.h:50` — `== 79` → `(72 || 79)`, asserting `0x388`. Stale `:16` comment refreshed ("excludes v79 AND v72").
- `common/CLogin.h:235` — **no edit** (==83 gate correctly excludes v72; `unk3[5]` absent in v72).

### FR-13 truth table — `gcc -E` selected `assert_size`, all versions (verbatim)

| Version | CWvsApp.h | Changed? | CFuncKeyMappedMan.h | Changed? | CLogin.h unk3[5] | Changed? |
|---|---|---|---|---|---|---|
| GMS 72 | 0x60 | **NEW (was none)** | 0x388 | **NEW (was none)** | absent | unchanged |
| GMS 79 | 0x60 | unchanged | 0x388 | unchanged | absent | unchanged |
| GMS 83 | 0x60 | unchanged | 0x3C8 | unchanged | **present** | unchanged |
| GMS 84 | 0x60 | unchanged | 0x3C8 | unchanged | absent | unchanged |
| GMS 87 | 0x6C | unchanged | 0x3C8 | unchanged | absent | unchanged |
| GMS 95 | 0x8C | unchanged | 0x3CC | unchanged | absent | unchanged |
| GMS 111 | 0x8C | unchanged | 0x3D0 | unchanged | absent | unchanged |
| JMS 185 | 0x64 | unchanged | 0x400 | unchanged | absent | unchanged |

Adding `72` to each branch is disjoint (no other supported GMS version equals 72), so no
other version's selected branch changes. v72 moves from "no branch selected (silent guard
loss)" to a defined, measured assert.

### Empirical verification (exactly what was run)

- `gcc -E -DREGION_GMS -DBUILD_MAJOR_VERSION={72,79} -DBUILD_MINOR_VERSION=1 -I common -I include` on each amended header: v72 → CWvsApp `static_assert(sizeof(CWvsApp) == 0x60)`, CFuncKeyMappedMan `== 0x388`; v79 identical (unchanged). Full matrix above.
- `scripts/wsl-build.sh GMS 72 1` → `>> OK` (EXIT=0) — v72 compiles **and links**, so the now-selected asserts (0x60 / 0x388) fire and hold against the real header layout.
- `scripts/wsl-build.sh GMS 79 1` → `>> OK` (EXIT=0) — neighbor, no regression.

**Finding:** v72 was **not** genuinely blocked from compiling before — the gates had no
`#else`, so v72 silently selected no `assert_size` (the size guard was absent, not failing).
The amendment restores the guard. Verified by the build firing the assert post-edit.

### Concern flagged for the gate owner (CLogin)

The v79 `CLogin::CLogin` (0x5C94AD / 0x5C94C3) builds **two** consecutive ZList blocks at
`this+0x178` and `this+0x18C` — the same two-ZList shape as v83 (`unk3`@+0x1A0 + m_lNewEquip).
This suggests v79 **also possesses** the member that `CLogin.h:235` gates to `==83` only.
`CLogin.h` has no `assert_size`, so this would not break the v79 build — it would silently
shift offsets after `m_lNewEquip`. Out of scope for this Category-A v72 task (v72 verdict is
unaffected: v72 genuinely has only one ZList → `unk3[5]` absent → exclusion correct), but the
v79 `==83` exclusion may be a task-008 error worth re-checking.

## Task 12 (Category B) audit record — the five `>= 83 || JMS` below-floor gates

Re-grepped live (`grep -rn "BUILD_MAJOR_VERSION >= 83" common/*.h`): exactly the five sites
`CWnd.h:25`, `CFuncKeyMappedMan.h:19`, `CMob.h:239`, `CUIToolTip.h:92`, `MobStat.h:128`
(CUIToolTip uses the parenthesised `(>=83)||JMS` form; line numbers otherwise as planned).
Lane discipline: v72 = port 13343 `GMS_v72.1_U_DEVM.exe` (active confirmed via `list_instances`);
v79 = port 13339 `GMS_v79_1_DEVM.exe`. Read-only `disasm` only; no struct types applied (R10).
Both IDBs retain full mangled symbols.

**Result: 3 confirmed-shares-v79, 2 split-three-way. The CWnd cascade is NOT triggered.**

### 1. CWnd.h:25 — CASCADE ROOT → `two-way-confirmed-shares-v79` (no cascade)
v72 `CWnd::CWnd` 0x4D9043 → calls base helper sub_8DD47E, then `[esi+64h]=[esi+68h]=[esi+70h]=0`
and sets 3 base vtables. v79 `CWnd::CWnd` 0x4E168D is structurally identical (same three writes,
calls sub_92D5ED). The base helpers are field-for-field identical:
- both `memset(this+8, 0, 0xC)`, then zero 0x14,0x18,0x38,0x3C,0x50,0x54,0x58,0x5C,0x60 and set
  the `m_lpChildren` ZList sentinel @0x48;
- both **skip 0x1C/0x20** (jump m_pLayer@0x18 → m_nBackgrndX@0x38) ⇒ the gated `m_pAnimationLayer`
  + `m_pOverlabLayer` are ABSENT in v72 exactly as in v79;
- identical unwind funclets at +8/+0x18/+0x48/+0x60 (incl. a `ZArray<_com_ptr_t<IWzCanvas>>`@0x60).

**Three landmarks, all == v79:** (a) ctor field-init extent (highest write 0x70 in BOTH);
(b) `CWnd::Destroy` 0x8DF182 touches the same fields (m_pLayer@0x18, m_lpChildren@0x48,
m_pFocusChild@0x5C); (c) the base-helper write set is byte-identical to v79's sub_92D5ED.
v72 CWnd ≡ v79 CWnd ⇒ the `>=83||JMS` gate already excludes v72 correctly. The five derived
classes (CDialog, CUIWnd, CFadeWnd, CUITitle, CUILoginStart) inherit v79's verdict — **no
re-derivation forced**; each is still independently size-confirmed in Task 14 (D5).
> Side note (does not change the verdict): the ctors write through 0x70 in BOTH v79 and v72, so
> the true `sizeof(CWnd)` is ≈0x74, not the `0x64` stated in the CWnd.h task-008 comment. The
> figure is equal across v79/v72, and CWnd.h carries no `assert_size`, so it is harmless here —
> flagged only as a v79-comment accuracy nit for whoever owns CWnd.h.

### 2. CFuncKeyMappedMan.h:19 — `two-way-confirmed-shares-v79`
v72 `TSingleton<CFuncKeyMappedMan>::CreateInstance` 0x8F6264 → `push 388h` / `Alloc` ⇒
`sizeof = 0x388` == v79. The quickslot pair (`m_aQuickslotKeyMapped[8]`×2, +0x40) is gated out
at :19 and absent in v72. Re-derived independently of Task 2 (the size-assert branch
`(==72||==79) → 0x388` added in Task 3 is consistent). Gate correct for v72.

### 3. CUIToolTip.h:92 — `two-way-confirmed-shares-v79`
v72 ctor 0x7F9C33 and v79 ctor 0x842317 are byte-identical in the layer region: both
`mov [esi+10h],edi` (m_pLayer) then `eh vector constructor` over `[esi+20h]`, count 0x20,
element 0x20 (`m_aLineInfo[32]`), then identical post-array zeroing from 0x424. m_pLayer@0x10 is
immediately followed (0x14/0x18/0x1C = m_nLastX/Y/m_nLineNo) by `m_aLineInfo`@0x20 — **no slot
for `m_pLayerAdditional`** (its presence would shift the array to 0x24). It is ABSENT in v72
**and in v79**. ⚠️ The plan-table annotation "v79 0x514 (== v83)" is a planning error: v79 also
lacks `m_pLayerAdditional` (its ctor proves it), so the `>=83||JMS` gate correctly excludes v79
too. Gate correct for v72.

### 4. CMob.h:239 — `split-three-way` (size diverges; doom-tail field shared-absent)
v72 `CreateMob` 0x611C9F → `push 4C0h` ⇒ `sizeof(CMob) = 0x4C0`; ctor 0x611CDB highest member
write `[esi+4B8h]` (last member @0x4B8, +pad → 0x4C0). v79 `CreateMob` 0x630BF0 → `push 518h`.
**v72 0x4C0 vs v79 0x518 = −0x58.** The gated doom tail (`m_bDoomReserved`/SN/
`m_lpStatChangeReserved`, which in v83 begins ~0x528) is ABSENT in both v72 and v79 — so the
`:239` field gate itself stays `#else` for v72. But CMob has **no `assert_size`**, and the
struct size genuinely diverges, so the header silently mis-lays-out v72. The −0x58 decomposes
as: front/base region −0x28 (first sub-object group v72@0x88 vs v79@0xB0; MobStat embed
v72@0x178 vs v79@0x1A0), MobStat-internal −0x1C (see #5), remainder ~−0x14 in the tail.
**Disposition: split-three-way** — v72 needs its own below-floor member gates + a new size guard
distinct from v79's reduced branch (Task 14/16/17). Drives the doom-fix gate (Task 15 / Cat C):
exclusion remains correct (doom write would be OOB on a 0x4C0 object).
Re-anchor (boundary): two independent v72 anchors agree — `CreateMob` Alloc immediate (0x4C0)
and the ctor's highest field write (0x4B8).

### 5. MobStat.h:128 — `split-three-way` (size diverges; Weakness field shared-absent)
MobStat is embedded in CMob (initialised via `MobStat::SetFrom`, this = MobStat base):
- v72: base = CMob+0x178; `lBurnedInfo` ZList (vtable off_9D4020) at `[ebx+1C4h]` = MobStat+**0x1C4**
  (ctor 0x611DA3; confirmed by unwind funclet `add ecx,1C4h`).
- v79: base = CMob+0x1A0; `lBurnedInfo` (vtable off_A30C58) at `[ebx+1E0h]` = MobStat+**0x1E0**
  (ctor 0x630D1E; unwind funclet `add ecx,1E0h`). Matches header comment "lBurnedInfo v79@0x1E0".

**v72 MobStat is 0x1C smaller than v79** (≈0x1D8 vs 0x1F8). The gated Weakness group
(`nWeakness_/rWeakness_/tWeakness_` + pad = 0x10) is ABSENT in both — but v72 reclaims an
ADDITIONAL ~0xC block beyond it (0x1C − 0x10), so v72 diverges further than v79's reduced branch.
MobStat carries no `assert_size`, so this too is a silent mis-layout. **Disposition:
split-three-way** — v72 needs a distinct gate for the extra missing ~0xC (pin the exact member
in Task 14/16). Re-anchor (boundary): two independent v72 paths agree on MobStat+0x1C4 — the
ctor `lea` and the destructor unwind funclet.

### Cross-version safety note (FR-13)
The two confirmed-shares gates (CWnd, CFuncKeyMappedMan, CUIToolTip) need **no edit** — v79's
`#else` branch already serves v72 unchanged; v79/v83/v84/v87/v95/v111/JMS185 untouched. The two
splits (CMob, MobStat) must, per D8, add a GMS-guarded `BUILD_MAJOR_VERSION < 79` arm ABOVE the
existing `#else`, leaving the v79 branch byte-identical in effect (Task 17). Because neither
header has an `assert_size`, the split is about removing the extra v72-absent members (and adding
a v72 size guard), NOT about the doom-tail/Weakness field gates at :239/:128 themselves (those
stay `#else` for v72, shared-absent with v79).

## Task 13 (Category C + special cases) audit record — core/net layout headers (7)

Read-only `disasm` only; no struct types applied (R10). Lane: v72 = port 13343
`GMS_v72.1_U_DEVM.exe` (active confirmed via `list_instances`; md5 05a62ca7…, base 0x400000
confirmed via `survey_binary`). The v72 IDB retains full mangled symbols. **All 7 headers
have a recorded v72 size + per-gate verdict + disasm anchor (D5).** No source edits (evidence
task). Result: **all gated fields ABSENT in v72 → every header takes the base/excluded
branch** (v72 < 87/95/111, and `==83`/`>83` false). No size diverges in a gate-breaking way
(none of these 7 has a member-shifting `assert_size` mismatch).

### Step 1 — `BUILD_MAJOR_VERSION` gates per header, v72 truth value

Every threshold below is **FALSE for v72** (v72 < every constant; `==`/`>` all false) → v72
selects the base/`#else` branch in each case:

| Header:line | Gate | v72 truth | Gated field(s) | v72 verdict |
|---|---|---|---|---|
| CWvsApp.h:14 | `>= 95` | FALSE | OS-version block (m_nOSVersion…m_b64BitInfo) | absent |
| CWvsApp.h:43 | `>= 87` | FALSE | m_tNextSecurityCheck | absent |
| CWvsApp.h:46 | `>= 95` | FALSE | m_bEnabledDX9 | absent |
| CWvsApp.h:49 | `>= 87` | FALSE | m_pBackupBuffer + m_dwBackupBufferSize | absent |
| CWvsApp.h:53 | `>= 95` | FALSE | m_dwClearStackLog + m_bWindowActive | absent |
| CWvsContext.h:46 | `>= 95` | FALSE | m_bFirstUserLoad | absent |
| CWvsContext.h:56 | `>= 87` | FALSE | m_bPetHelpPopUpShown | absent |
| CWvsContext.h:98 | `> 83` | FALSE | **m_aClientKey[8]** (special) | **absent (confirmed)** |
| CWvsContext.h:101 | `>= 87` | FALSE | m_bTesterAccount | absent |
| CWvsContext.h:148 | `>= 87` | FALSE | m_aRealEquip[60]×2 | absent → `#else` 52-slot |
| CWvsContext.h:160/172/181/232/237/… | `>= 95` | FALSE | Dragon/Mechanic equip, item-option rests, ItemMsg etc | absent |
| CClientSocket.h:22 | `>= 111` | FALSE | int dummy1 | absent |
| CLogin.h:235 | `== 83` | FALSE | unk3[5] | absent (Task 3) |
| COutPacket.h:9 | `>= 111` | FALSE | int dummy1 | absent |
| CConfig.h:52 | `>= 111` | FALSE | m_v111Pad | absent |
| CConfig.h:80/82 | `>= 111` / `== 95` | FALSE | (assert-branch selectors) | base `>=1072` assert |
| ConfigSysOpt.h:15 | `>= 95` | FALSE | bSysOpt_LargeScreen + bSysOpt_WindowedMode | absent |

### Step 2 — CWvsContext m_aClientKey (`>83`, special) — ABSENT, confirmed
v72 `CClientSocket::OnConnect` (0x48528f) post-handshake PLAYER_LOGGED_IN send is exactly
`COutPacket(0x14); Encode4(charId=*(g_pWvsContext+0x20A0)); Encode1(TSecType bit
*(g_pWvsContext+0x203C)&0x80); Encode1(0); SendPacket` — **no `EncodeBuffer(m_aClientKey,8)`
anywhere** (verified against the v72 binary, not a server round-trip; Task 4 / sig-cat §8-byte
client-key). v72 has neither the v84+ 16-byte machineId nor any 8-byte client key. The `>83`
gate correctly excludes v72; `bypass/socket_hooks.cpp` (`>83`) is correct for v72.
**SecondaryStat embed observation:** the post-key field `m_dwCharacterId` lands at
`g_pWvsContext+0x20A0` — i.e. the layout *before* it (BasicStat + the embedded SecondaryStat
base, v79/v83 = 0xB88) is consistent with the base shape; nothing shifts it. (Task 15
finalizes SecondaryStat.)
**Total size:** no `assert_size` on CWvsContext.h. Lower bound `sizeof(CWvsContext) ≥ 0x3520`
from three independent high-offset accesses — WinMain `[g_pWvsContext+0x3510]`, ApplySysOpt
writes `[+0x3504]`/`[+0x3508]`, CWvsApp::SetUp 0x8f2c99 reads `[+0x351C]`. The exact alloc
immediate was not pinned in budget (the CWvsContext singleton creator is not reachable by the
`?CreateInstance@?$TSingleton@VCWvsContext@@…` symbol and `search_text` timed out repeatedly on
this IDB); it is **not gate-relevant** — every CWvsContext gate verdict rests on the OnConnect
finding + version arithmetic, not the total size.

### Step 3 — CLogin `==83` unk3[5] — ABSENT (verdict `unchanged`)
Confirmed Task 3: v72 ctor 0x5AECED builds ONE ZList (m_lNewEquip @this+0x174); v79/v83 build
two (the extra `unk3` ZList). Independent size anchor: the CLogin allocation in LogoEnd is
`Alloc(0x23C)` → CLogin::CLogin → set_stage (sig-cat:389) → **sizeof(CLogin) v72 = 0x23C
(572)**, vs v79 0x258 (600) and v83 0x2C8 (712). The `==83` gate correctly excludes v72 — no
edit. (CLogin.h carries no `assert_size`.)

### Step 4 — CWvsApp / CClientSocket / COutPacket / CConfig / ConfigSysOpt
- **CWvsApp 0x60** (Task 3): WinMain stack-ctor; ctor 0x8f26c7 installs vtable off_9DB2E0, fields
  through +0x38, sizeof 0x60. All `>=87`/`>=95` blocks absent (v72<87). Branch-added in Task 3.
- **CClientSocket 0x94 (148):** CreateInstance 0x8f621f `push 94h`/Alloc → ctor 0x484c95. `>=111`
  dummy1 absent. == v79.
- **COutPacket 0x10 (base):** ctor 0x656fa1 — single ZArray member m_aSendBuff @+4 (`_Alloc(0x100)`),
  Init writes m_uOffset@8 / m_bIsEncryptedByShanda@0xC; unwind funclet RemoveAll on [esi+4]. No
  +0x10 field → `>=111` dummy1 absent. `static_assert >=0x10` holds.
- **CConfig real 0x3FC (1020):** WinMain `push 3FCh` @0x8ef7ce → Alloc → CConfig::CConfig 0x48c0d3.
  `>=111` m_v111Pad and `==95` assert-branch both excluded → v72 takes base `>=1072` assert. Our
  GMS struct is v95-shaped (≥1592) and never read/memcpy'd, so it safely covers the real 1020 (no
  heap overflow). Note v72's real size (1020) is *smaller* than the conservative `>=1072` floor and
  smaller than v83 (1072) — harmless because the floor is a `>=` on our oversize struct.
- **ConfigSysOpt base:** `>=95` LargeScreen+WindowedMode absent. **Windowed-mode cross-check
  (Task 8 C_CONFIG_SYS_OPT_WINDOWED_MODE = 0xAA87AC):** in v72 the windowed flag is a STANDALONE
  global (SetUp 0x8f2c49 `mov g_CConfig_SysOpt_WindowedMode, 10h`; readers CreateMainWindow /
  InitializeGr2D), sitting at 0xAA87AC — **outside** the CConfig instance (0xAA3AC0 + real 0x3FC =
  end 0xAA3EBC < 0xAA87AC). So `bSysOpt_WindowedMode` is genuinely NOT a field of the embedded
  CONFIG_SYSOPT in v72; the `>=95` gate is correct.

### Step 5 — boundary re-anchors (independent second probe)
- **CConfig 0x3FC:** anchor 1 = WinMain alloc immediate (`push 3FCh`); anchor 2 = ctor 0x48c0d3
  `memset(this+0x234, 0, 0x1BD)` reaches 0x234+0x1BD = 0x3F1 < 0x3FC (in-bounds, consistent with
  sig-cat:463). Two independent paths agree the object is 0x3FC.
- **CClientSocket 0x94:** anchor 1 = CreateInstance `push 94h`/Alloc; anchor 2 = ctor 0x484c95 is
  the placement ctor over that block (sole Alloc(0x94) → ctor pair in the cluster).
- **CLogin 0x23C:** anchor 1 = LogoEnd `Alloc(0x23C)`; anchor 2 = the alloc+ctor+set_stage triple
  is the sole such caller of CLogin::CLogin (sig-cat:389).

### Cross-version safety (FR-13)
No edits in this evidence task, so no truth-table changes. All gate verdicts are "base/excluded
branch, field absent" — identical disposition to v79 for every one of the 7 headers; v79/v83/v84/
v87/v95/v111/JMS185 selections are unaffected. CWvsApp's `72`-in-`0x60`-branch and CLogin's
`==83` exclusion were already settled in Task 3 (truth table there).

## Task 14 (UI/control family) audit record — 9 headers; CWnd cascade confirmed not triggered

Read-only `disasm` only; no struct types applied (R10). Lane: v72 = port 13343
`GMS_v72.1_U_DEVM.exe` (active confirmed via `list_instances`; md5 05a62ca7…, base 0x400000,
confirmed via `survey_binary`). Full mangled symbols present (except CCtrlCheckBox/CUILoginStart,
stripped to `sub_`). No source edits (evidence task). **Result: all 9 headers take the
base/excluded branch; every `>=87`/`>=95`/`>=111` gated field ABSENT in v72; CWnd cascade NOT
triggered → the 5 CWnd-derived classes INHERIT v79's verdict (no three-way re-split).**

### Step 1 — `BUILD_MAJOR_VERSION` gates per header, v72 truth value
All thresholds below are **FALSE for v72** (v72 < 83/84/87/95/111; not JMS) → base/`#else` branch:

| Header:line | Gate | v72 truth | Gated field(s) | v72 verdict |
|---|---|---|---|---|
| CWnd.h:4 | `>=95\|\|JMS` | FALSE | UIOrigin enum (no size) | n/a |
| CWnd.h:25 | `>=83\|\|JMS` | FALSE | m_pAnimationLayer + m_pOverlabLayer | absent (Task 12) |
| CWnd.h:35 | `>=87\|\|JMS` | FALSE | SECPOINT m_ptCursorRel (`#else` = 2 ints, same 8 B) | base 2-int form |
| CWnd.h:44 | `>=95` | FALSE | UIOrigin m_origin | absent |
| CUIWnd.h:11 | `>=95` | FALSE | m_nSmallScreenX/Y, m_nLargeScreenX/Y, m_bIsLargeMode | absent |
| CUITitle.h:3 | `>=95\|\|JMS` | FALSE | base class CFadeWnd (`#else` = CDialog) | base = CDialog |
| CUITitle.h:12 | `>=95\|\|JMS` | FALSE | tagRECT m_rcRMA | absent |
| CUITitle.h:22 | `REGION_JMS` | FALSE | ZRef m_unk (extra button) | absent |
| CUILoginStart.h:8 | `>=95\|\|JMS` | FALSE | m_pFont + m_pCanvasChannelName | absent |
| CUILoginStart.h:12 | `REGION_GMS` | **TRUE** | m_aBtParam[5]/m_apButton[5] (vs `#else` [8]) | 5-element (GMS) |
| CFadeWnd.h:18 | `REGION_GMS` | **TRUE** | m_bUserAlarm | present (GMS) |
| CFadeWnd.h:24 | `>=87\|\|JMS` | FALSE | m_nLevel, m_nJobCode, m_nExpQuestID | absent |
| CCtrlButton.h:32 | `>=95` | FALSE | ZXString m_sToolTipFromData | absent |
| CCtrlCheckBox.h:16 | `>=95` | FALSE | m_nTextOffsetX, m_nTextOffsetY | absent |
| CLogo.h:5/16/30 | `>=95` | FALSE | VIDEO_STATE enum, m_pLayerBackground, m_bVideoMode, m_videoState | absent |
| CLogo.h:27 | `>=95\|\|JMS` | FALSE | m_bNXFadeOut | absent |
| CLogo.h:34 | `>=111` | FALSE | dummy1 | absent |
| CUIToolTip.h:82 | `>=95` | FALSE | CLineInfo::m_bUseDotImage (elem 0x24→0x20) | absent (Task 12) |
| CUIToolTip.h:92 | `>=83\|\|JMS` | FALSE | m_pLayerAdditional | absent (Task 12) |
| CUIToolTip.h:100 | `>=95\|\|JMS` | FALSE | m_nOptionLineNo + m_aOptionLineInfo[32] | absent |
| CUIToolTip.h:125/128/131/134 | `>=84`/`>=87`/`>=95` | FALSE | Gen_Unknown / H_White / Stan_Prp / Stan_Dsc..Skill_Dsc | absent |
| CUIToolTip.h:152 | `>=84\|\|JMS` | FALSE | m_pCanvasEquip_Durability[2][2] | absent |

### Step 2 — CWnd cascade verdict (linked, not five independent)
v72 `sizeof(CWnd)` base subobject = **0x64** == v79 (anim layers `>=83` absent in both;
Task 12). Cascade NOT triggered → CDialog/CUIWnd/CFadeWnd/CUITitle/CUILoginStart **inherit v79's
verdict**, NOT re-split. Per D5 each derived class was still confirmed against an independent v72
anchor (Step 3). The Task 12 "0x64-vs-0x74 nit" is resolved in v72's favour of **0x64**: CUIWnd's
first own member (m_pBtClose, ctor sub_83C0EC) lands at this+0x64, so the CWnd base subobject is
exactly 0x64; the CWnd ctor's 0x68/0x70 writes are inside CWnd's own m_pBackgrnd/ZArray region.

### Step 3 — per-header v72 size + anchor (all 9)
1. **CWnd 0x64** — ctor 0x4D9043 + helper sub_8DD47E ≡ v79 (Task 12); base-subobject extent
   re-confirmed via CUIWnd m_pBtClose@0x64 (Task 14).
2. **CUIToolTip 0x50C** — embedded in CCtrlButton @0x8C; CCtrlButton alloc 0x59C − 0x8C −
   m_bSelfDisable(4) = 0x50C. == v79 (byte-identical ctor, Task 12). Header bottom-up 0x514 is an
   8-byte/2-field overcount in the shared `<87` font/canvas region (flag; no assert_size).
3. **CLogo 0x38** — two anchors: SetUp 0x8f2f2f `push 38h`→Alloc; ctor 0x5e11f9 highest write
   `[esi+34h]`. assert `>=0x38` holds. == v83/84/87.
4. **CCtrlButton 0x59C** — CUIFadeYesNo::OnCreate 0x500921 `push 59Ch`; ctor 0x422954 embeds
   m_uiToolTip@0x8C; dtor funclets show only 2 trailing ZXStrings (no m_sToolTipFromData).
5. **CCtrlCheckBox** — ctor stripped (no symbol); `>=95` text-offset pair absent by arithmetic;
   CCtrlWnd base shared with CCtrlButton. No assert_size → lower-bound. (concern)
6. **CDialog ≥0x7C** — ctor 0x4a5d59 calls CWnd::CWnd then own field `[esi+78h]`.
7. **CUIWnd ≈0x5A4** — ctor sub_83C0EC; m_pBtClose@0x64, m_uiToolTip(0x50C)@0x6C→0x578, dense
   arg-fields 0x57C–0x594, no `>=95` +0x14 gap.
8. **CFadeWnd ≈0xC4** — ctor 0x4FFD72 calls CWnd::CWnd then writes through `[esi+0C0h]`
   (m_dwFriendID); `>=87` level/job/exp block absent (writes skip to m_dwSN/m_dwFriendID).
9. **CUITitle ≈0x5E4** — ctor sub_5D3BFB (singleton UITitleInstanceAddr@0xAA5114) calls
   **CWnd::CWnd** ⇒ base = CDialog (NOT CFadeWnd; `>=95||JMS` base-swap excluded); m_pCanvasRMA[2]@0x80
   → buttons @0x88 with no m_rcRMA gap; m_uiToolTipTitle(0x50C)@0xD8.
10. **CUILoginStart** — ctor stripped (no symbol; CDialog inlined → no CDialog::CDialog xref);
    `>=95||JMS` font/canvas pair absent by arithmetic; REGION_GMS 5-element arrays; CDialog-derived
    sibling of CUITitle. No assert_size → lower-bound. (concern)

### Step 4 — boundary re-anchors (two independent v72 paths)
- **CLogo 0x38:** SetUp `push 38h` (alloc immediate) AND ctor highest write `[esi+34h]`.
- **CUIToolTip 0x50C:** CCtrlButton embed math (alloc 0x59C, m_uiToolTip@0x8C) AND Task 12's
  byte-identical-to-v79 ctor (m_aLineInfo elem 0x20, no m_pLayerAdditional).
- **CCtrlButton 0x59C:** alloc immediate `push 59Ch` AND m_uiToolTip embed offset + trailing
  m_bSelfDisable, which sum back to 0x59C.

### Cross-version safety (FR-13)
No edits in this evidence task → no truth-table changes. All 9 verdicts are "base/excluded branch,
gated fields absent" — identical disposition to v79; v79/v83/v84/v87/v95/v111/JMS185 selections
unaffected. CWnd cascade confirmed NOT triggered (Task 12 + Task 14 independent anchors), so no
three-way split is needed for any of the 5 derived classes.

### Concerns (DONE_WITH_CONCERNS)
1. **CUIToolTip header overcounts the shared `<87` branch by 8 bytes** (header 0x514 vs binary
   0x50C). Two unattributed 4-byte fields in the font/canvas region. Shared v72==v79 (byte-identical
   ctor), no assert_size, not gate-relevant — flag for the CUIToolTip.h owner; pinning the exact 2
   fields is beyond the read-only budget.
2. **CCtrlCheckBox & CUILoginStart ctors are stripped** (no mangled symbol in the v72 IDB) →
   their exact sizeof anchors are version-arithmetic/cascade-based, not dedicated-ctor disasm. Gate
   verdicts are firm (`>=95`-family FALSE for v72); neither header has an assert_size, so a
   lower-bound is acceptable per the task brief.

## Task 15 (Mob/stat family) audit record — 4 headers; CMob/MobStat split regions PINNED

Lane: v72 = port 13343 `GMS_v72.1_U_DEVM.exe` (active via `list_instances`; `server_health` idb/base 0x400000
confirmed); v79 = port 13339 `GMS_v79_1_DEVM.exe`. Read-only `disasm`; no struct types (R10). `get_metadata` not
exposed → `server_health` substituted. No source edits (evidence task). **All 4 headers: v72 size + per-gate verdict
+ disasm anchor.** Full report: `.superpowers/sdd/task-15-report.md`. **Result: 3 sizes diverge from v79/v83 via
*ungated* member regions (CMob front block, MobStat status ints + bDisable, SecondaryStat ~18 base stats); CMapLoadable
clean (base/excluded).** Status DONE_WITH_CONCERNS (sizes + gate dispositions firm; some ungated members named-not-byte-pinned).

### 1. CMob.h — sizeof v72 = 0x4C0; −0x58 vs v79 0x518, decomposed (two parallel ctor walks)
v72 ctor 0x611CDB vs v79 ctor 0x630C2C, aligned by corresponding members (1st ZList m_lAffectedSkillEntry: v72@0x88 /
v79@0xB0; MobStat embed: v72@0x178 / v79@0x1A0; m_aAction: v72@0x488 / v79@0x4D4; highest write v72 0x4B8 / v79 0x514):
- **FRONT −0x28** = CLife base **−4** (m_nMobChargeCount v72@0x84 vs v79@0x88) + **`REGION_GMS` block CMob.h:97–105
  ABSENT in v72 (−0x24)**: in v72 the 1st ZList sits at 0x88 immediately after m_nMobChargeCount@0x84; in v79 the
  block (6 ints + ATTACKEFFECT 0xC) occupies 0x8C–0xAF and the 1st ZList is at 0xB0. **This is the gateable divergence
  → Task 17 changes `:97` from `REGION_GMS` to `REGION_GMS && BUILD_MAJOR_VERSION >= 79`.**
- **MobStat embed −0x20** (§2). **TAIL −0x10** = ungated CMob tail members absent in v72 (not individually pinned —
  ctor inits via non-sequential SecureTear funclets). doom tail (`:239`) absent both → field gate stays `#else` for v72.
- **CONCERN:** CLife −4 + tail −0x10 are NOT CMob.h-own members; member-exact v72 CMob is not gate-achievable via
  CMob.h alone. CMob has no assert_size; doom-fix exclusion already correct → acceptable, but flagged for Task 17.

### 2. MobStat.h — sizeof v72 = 0x1D8 (memset anchor); −0x20 vs v79 0x1F8
SetFrom v72 0x6D0896 `push 1D8h` / v79 0x702675 `push 1F8h` (both memset the whole struct → firm sizeof). Field-copy
destinations identical through MobStat+0x84 (nLevel, aDamagedElemAttr[8], nPAD…nSpeed 4-int groups). Divergence:
- **nFs (long double)** v72@0x1B8 (`fstp[edi+1B8h]`) vs v79@0x1D0 → the memset-0 status region `[0x94, nFs)` is
  **0x18 (6 ungated status ints) shorter in v72** (exact 6 ints not byte-pinned — region isn't field-copied; candidate).
- **`int bDisable` (MobStat.h:152) ABSENT in v72 (PINNED):** v72 copies ONE post-nFs field → bInvincible@0x1C0,
  lBurnedInfo@0x1C4; v79 copies TWO → bInvincible@0x1D8, bDisable@0x1DC, lBurnedInfo@0x1E0.
- −0x18 (status) + −0x4 (bDisable) = −0x1C at lBurnedInfo; + 0x4 v79 trailing align-pad = −0x20 sizeof. **Task 17:
  gate bDisable + the 6 status ints with `< 79`.**

### 3. SecondaryStat.h — sizeof v72 = 0xAB0 (INDEPENDENT); −0xD8 vs v79/v83 0xB88 — SIZE-CRITICAL
Embedded @CWvsContext+0x212C in BOTH (OnTemporaryStatReset v72 0x918F3C / v79 0x96AB32 `add edi,212Ch` → `SecondaryStat::
Reset(this=edi)`). aTemporaryStat[7] (last member): RideVehicle [0] v72@SecondaryStat+0xA94 / v79@0xB6C; GuidedBullet [4]
v72@0xAA4 / v79@0xB7C — both Δ=−0xD8. sizeof: v72 0xA94+0x1C=**0xAB0**; v79 0xB6C+0x1C=0xB88 (re-confirms 0xB88). Reset
0x6CA91A confirms field layout == header (nPAD_ @+0xC). Per-gate: `>=87` DojangShield/AssistCharge/Enrage, `==87`
byte-variants (→#else int), `>=84` Flying/Frozen, `>=95` blocks — **ALL ABSENT** (v72<87). **CONCERN (size-critical):**
the −0xD8 (=18 × 0xC SecureTear slots) is an UNGATED base shrink → header over-counts v72 by 0xD8, shifting m_forcedStat
+ all CWvsContext members after m_secondaryStat by −0xD8 (pre-stat region ≤0x212C unaffected; Task 13 front verdicts stand).
Exact 18 missing base stats not pinned. No assert_size.

### 4. CMapLoadable.h (README-critical) — sizeof v72 = 0x114; base/excluded, clean
Front == v95 reference: PrepareNextBGM 0x5F3496 `mov [esi+1Ch]`=m_tNextMusic (CStage base 0x18; matches v95 dump → no
front shrink). 0x148 − m_bField(4,`>=95`) − m_lVisibleByQuest(0x14,`>=84`) − m_lpLayerLetterBox(0x14,`>=95`) − m_bPlayHolded
pair(8,`>=95`) = **0x114**; corroborated by task-006 v84=0x128 (= 0x114 + m_lVisibleByQuest). All gated fields absent.
Minor residual: tail not independently disasm-anchored (no assert_size; verdict firm regardless).

### Cross-version safety (FR-13)
No edits in this evidence task → no truth-table changes. CMob/MobStat split dispositions unchanged from Task 12 (add a
GMS `< 79` arm ABOVE the `#else`, v79 byte-identical). SecondaryStat/CMapLoadable take the base/excluded branch
identically to v79 for every gate; v79/v83/v84/v87/v95/v111/JMS185 selections unaffected.
