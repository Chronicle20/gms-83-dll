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

## Category C — `>= 84` gates (v79 EXCLUDED, sides with v83)

v79 takes the same excluded side as v83. Confirm the gated field is genuinely
absent in v79 (very likely, since v83 also lacks it — but verify, don't assume).

| Site | Gate | Field/region | Confirm absent in v79 |
|---|---|---|---|
| `common/CMob.h:229` | `>= 84` | v84+ CMob field | ☐ |
| `common/CMob.h:233` | `>= 84 \|\| JMS` | v84+ CMob field | ☐ |
| `common/CMapLoadable.h:154` | `>= 84 \|\| JMS` | v84+ field | ☐ |
| `common/CUIToolTip.h:125` | `>= 84 \|\| JMS` | v84+ field | ☐ |
| `common/CUIToolTip.h:152` | `>= 84 \|\| JMS` | v84+ field | ☐ |

## Category D — `< 95` / `< 84` inverse gates (v79 INCLUDED on base branch)

v79 is true for these and takes the base/included branch. Confirm v79's base
layout matches the v83 layout the branch was written for.

| Site | Gate | v79 lands | Confirm |
|---|---|---|---|
| `common/CMob.h:110` | `< 95` | included (base) | v79 CMob base layout == v83 base layout (size + the doom-field region) |

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
| CWvsApp.h | 0x60 (96) | **v79-branch-added** (Cat A, World A) @ :97 — `79` added to the `0x60` branch | task-008: ctor `??0CWvsApp@@QAE@PBD@Z` @0x942D3B writes the exact v83/84 base layout (vtable@0; @8,@0xC,@0x10,@0x14,@0x18,@0x1C; ZXString@0x20; ints @0x24..0x38) — NO below-floor surprise member; CWvsApp.h has no `<84`/`==83` member gate so v79 ≡ v83 by construction. Header computes 0x60 for v79; clang-cl static_assert(sizeof==0x60) **PASSED**. |
| CFuncKeyMappedMan.h | 0x388 (904) | **v79-member-shift CONFIRMED — DEFER to struct audit (Task 12/16)** (Cat A→B). No v79 assert written (a 0x388 assert FAILS to compile). | task-008 re-measure (2 independent anchors): TSingleton `CreateInstance` @0x946AFB `push 388h`→`Alloc(0x388=904)`; ctor `??0CFuncKeyMappedMan@@QAE@XZ` @0x569DE5 field-init extent = vtable@0 + memcpy 0x1BD@+4 + memcpy 0x1BD@+0x1C1 (arrays end @0x37E) + dwords zeroed @+0x380/+0x384 → ends 0x388. Header computes 0x3C8 (968): clang-cl scratch probe `assert_size(...,0x388)` for v79 FAILED with `expression evaluates to '968 == 904'` — proving a 0x40 below-floor MEMBER shift (the two `m_aQuickslotKeyMapped[8]` int arrays, 0x40 bytes, absent in v79), which a size-assert cannot express. NOTE: v79 cmake map `C_FUNC_KEY_MAPPED_MAN_CREATE_INSTANCE=0x009F9E98` is STALE (points to `sub_753F6A`, an unwind funclet) — flag for Task 7. |
| CUIToolTip.h | ☐ | ☐ (Cat B/C) | |
| CMob.h | ☐ | ☐ (Cat C/D + doom-field) | |
| CMapLoadable.h | ☐ | ☐ (Cat C) | |
| CLogin.h | ☐ | ☐ (`==83` @ :235) | |
| CWvsContext.h | ☐ | ☐ (`>83` clientkey @ :98) | |
| CClientSocket.h | ☐ | ☐ | |
| CLogo.h | ☐ | ☐ | |
| CConfig.h | ☐ | ☐ | |
| ConfigSysOpt.h | ☐ | ☐ | |
| COutPacket.h | ☐ | ☐ | |
| CFadeWnd.h | ☐ | ☐ | |
| CCtrlButton.h | ☐ | ☐ | |
| CCtrlCheckBox.h | ☐ | ☐ | |
| CWnd.h | ☐ | ☐ | |
| CUIWnd.h | ☐ | ☐ | |
| CUITitle.h | ☐ | ☐ | |
| CUILoginStart.h | ☐ | ☐ | |
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
