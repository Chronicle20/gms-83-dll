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
| CWvsApp.h | ☐ | ☐ | ☐ (expect: add 72 to 0x60 branch) | |
| CFuncKeyMappedMan.h | ☐ | ☐ | ☐ (expect: add 72 size-assert; confirm quickslot still absent) | |
| CUIToolTip.h | ☐ | ☐ | ☐ | |
| CMob.h | ☐ | ☐ | ☐ (confirm v72 doom tail absent + base size) | |
| CMapLoadable.h | ☐ | ☐ | ☐ | |
| CLogin.h | ☐ | ☐ | ☐ (confirm unk3[5] absent for v72) | |
| CWvsContext.h | ☐ | ☐ | ☐ (m_aClientKey absent; SecondaryStat embed size) | |
| CClientSocket.h | ☐ | ☐ | ☐ | |
| CLogo.h | ☐ | ☐ | ☐ | |
| CFadeWnd.h | ☐ | ☐ | ☐ (CWnd cascade) | |
| CCtrlButton.h | ☐ | ☐ | ☐ | |
| CCtrlCheckBox.h | ☐ | ☐ | ☐ | |
| CWnd.h | ☐ | ☐ | ☐ (pin sizeof first — cascade root) | |
| CUIWnd.h | ☐ | ☐ | ☐ (CWnd cascade) | |
| CUITitle.h | ☐ | ☐ | ☐ (CWnd cascade) | |
| CUILoginStart.h | ☐ | ☐ | ☐ (CWnd cascade) | |
| CConfig.h | ☐ | ☐ | ☐ | |
| ConfigSysOpt.h | ☐ | ☐ | ☐ | |
| COutPacket.h | ☐ | ☐ | ☐ | |
| SecondaryStat.h | ☐ | ☐ | ☐ | |
| PartyData.h | ☐ | ☐ | ☐ (packed) | |
| PartyMember.h | ☐ | ☐ | ☐ (packed) | |
| GuildData.h | ☐ | ☐ | ☐ (packed) | |
| MobStat.h | ☐ | ☐ | ☐ (Weakness group: confirm absent for v72) | |

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
</content>
