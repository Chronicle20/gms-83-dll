# Plan Audit — task-009-gms-v72-support

**Plan Path:** docs/tasks/task-009-gms-v72-support/plan.md
**Audit Date:** 2026-06-27
**Branch:** task-009-gms-v72-support
**Base Branch:** main (task-009 delta audited as `0cdc922..98d3108`, the 19 commits after the task-008 merge)

## Executive Summary

All 18 build/implementation tasks (Tasks 1–18) are faithfully implemented and corroborated by the
git delta, the source tree, and live build/validator runs. The 159-key memory map validates clean,
both the GMS 72.1 primary and GMS 79.1 regression WSL builds link successfully, all 24 struct headers
carry a v72 verdict (0 unresolved rows), and the CI matrix has `{ GMS, 72, 1 }` as its first entry.
Task 19 (live in-client smoke test) is correctly PENDING — it is user-run by design and a green build
is not acceptance. One material deviation from the plan's predictions is noted (the Category-B gate
*split sites* moved, evidence-driven, and are correctly surfaced), plus a documentation-hygiene note
(plan checkboxes were never flipped; the live ledgers in `memory-map.md`/`struct-verification.md` are
the source of truth). All documented residuals are surfaced findings, not silent gaps.

## Task Completion

| # | Task | Status | Evidence / Notes |
|---|------|--------|------------------|
| 1 | Setup — branch, seed cmake from v79, reconcile key set, baseline IDB | DONE | commit `e788e37`; `memory_maps/GMS/v72_1.cmake` created (216 lines) with the v72 seed-from-v79 header; live key count = 159 (re-greped); validator `OK: all 159…` |
| 2 | Memory map — WinMain + CWvsApp + window-manager cluster | DONE | commit `eb0dd50`; cluster rows ✔ in `memory-map.md`; catalog rows present |
| 3 | Early Category-A gate amendment (CWvsApp/CFuncKeyMappedMan/CLogin) | DONE | commit `aac7d58`; `common/CWvsApp.h:97` adds `BUILD_MAJOR_VERSION == 72` to the `0x60` branch; `common/CFuncKeyMappedMan.h:51` adds `== 72` to the `0x388` branch; `CLogin.h` confirmed unchanged (verdict `unchanged`, `struct-verification.md:105`) |
| 4 | Memory map — CClientSocket / ZSocket cluster | DONE | commit `304f0ec`; client-key absence captured (`struct-verification.md:106`, OnConnect 0x48528f has NO EncodeBuffer(key,8)) |
| 5 | Memory map — COutPacket cluster | DONE | commit `84523d7` |
| 6 | Memory map — Login / Stage / Logo / Title cluster | DONE | commits `19e480a` + structural-anchor fixup `6dbfb3b` (CStage::OnMouseEnter + CLogo::IsKindOf) |
| 7 | Memory map — Manager singletons cluster | DONE | commit `d6e92a1` (Radio quirk handled per cluster) |
| 8 | Memory map — Config / SystemInfo / IGCipher / utilities | DONE | commit `66afe6e` |
| 9 | Memory map — Party / migrate / context senders + offsets | DONE | commit `5592de7` |
| 10 | Memory map — protocol constants + sentinels + exception-dispatch | DONE | commit `1dbd843` (CFileStream + DR/sentinels confirmed) |
| 11 | Memory map completeness gate | DONE | commit `588fef3`; validator `OK: all 159 keys…`; `memory-map.md` 0 real `☐`/`◐` rows (the 2 `☐`/`◐` grep hits are legend text at lines 80/100) |
| 12 | Cat-B confirm-or-split audit (the novel tier) | DONE | commit `0349b34`; `struct-verification.md:212` — "3 confirmed-shares-v79, 2 split-three-way; CWnd cascade NOT triggered" |
| 13 | Struct audit — core/net headers (7) | DONE | commit `38cdf37`; verdict rows 105–106 etc. |
| 14 | Struct audit — UI/control family (9; CWnd cascade) | DONE | commit `23bb620`; cascade resolved not-triggered (`struct-verification.md:423`) |
| 15 | Struct audit — Mob/stat family (4) | DONE | commit `0dd8834`; SecondaryStat 0xAB0 (−0xD8) pinned (line 119) |
| 16 | Struct audit — Party/Guild/misc; complete 24/24 | DONE | commit `0cdefae`; `struct-verification.md:570` "Zero ☐ rows remain. Count = 24/24" |
| 17 | Apply gate rewrites + cross-version validation | DONE (deviation) | commit `225b793`; splits applied at `common/CMob.h:97` (`>= 79`) and `common/MobStat.h:152` (`>= 79 || JMS`) — different members than the plan predicted (see below) |
| 18 | Build wiring + build verification | DONE | commit `98d3108`; `.github/workflows/_build.yml` adds `{ GMS, 72, 1 }` as first matrix entry; WSL builds 72.1 + 79.1 green (this audit) |
| 19 | Acceptance — live smoke test (user-run) | PENDING (expected) | `acceptance.md` present; all 6 acceptance checkboxes unchecked (lines 45–52). Correctly NOT marked complete — user-gated, blocks "done" by design (FR-16/R13). |

**Completion Rate:** 18/19 implementation tasks DONE; Task 19 is a user-run acceptance gate, correctly PENDING.
**Skipped without approval:** 0
**Partial implementations:** 0 (Task 17 is DONE with a justified, surfaced deviation in split location)

## Skipped / Deferred Tasks

None silently skipped or deferred. The only open task is Task 19 (live smoke test), which the plan
explicitly defines as user-run and completion-gating; it is correctly recorded as pending, not as
success.

### Deviation — Task 17 gate-split sites (justified, surfaced, evidence-driven)

The plan's Category-B table predicted v72 would diverge at the **CMob doom tail**
(`m_bDoomReserved`…) and the **MobStat Weakness group**, requiring three-way splits of the existing
`>= 83 || JMS` gates at `CMob.h:239` / `MobStat.h:128`. The audit instead found (read-only disasm,
`struct-verification.md:251`, `:266`):

- Those doom-tail / Weakness fields are **absent in BOTH v72 and v79** — the existing `>= 83 || JMS`
  gates are already correct and were left unchanged.
- The real v72 divergence is at previously **ungated** members: the CMob attack-ready block
  (`m_bAttackReady`…`m_effectAttack`, 0x24, `CMob.h:97`) and `MobStat::bDisable` (+6 ungated status
  ints). These got new GMS-guarded gates: `CMob.h:97` → `BUILD_MAJOR_VERSION >= 79`; `MobStat.h:152`
  → `BUILD_MAJOR_VERSION >= 79 || REGION_JMS`.

This is consistent with the plan's "verify, do not copy" mandate and [[feedback-prefer-confirmation]].
The fix is functionally a v72 / v79–82 / v83+ three-way distinction (v72 is the only GMS `< 79`), so
the `>= 79` form is correct and disjoint, even though it is not the literal D8 `< 79`-arm-above-`#else`
shape (that shape applies to splitting an existing `>= 83 || JMS` gate; here the divergent members
carried no prior gate). Empirically verified below. Minor note: commit `225b793`'s message says
"three-way below-floor gate splits," which is slightly loose phrasing for "new `>= 79` member gates."

### Documented residuals (surfaced, NOT silent gaps)

All flagged in `struct-verification.md` (Documented-residuals section ~line 643, and inline):

1. **SecondaryStat −0xD8 ungated base shrink** (v72 0xAB0 vs v79/v83 0xB88, ~18 SecureTear slots),
   SIZE-CRITICAL but carries **no `assert_size`** and **no v72 consumer dereferences** the shifted
   fields; exact 18 missing base stats named-not-byte-pinned (`struct-verification.md:119`, `:509`).
2. **CMob** CLife-base −4 + ungated −0x10 tail — not `CMob.h`-gateable, flagged (`:103`, `:251`).
3. **MobStat** 6 ungated status ints shorter in v72 — named-candidate, not byte-pinned (`:123`, `:500`).

These are explicitly surfaced verdicts with disasm anchors, not silent zeros.

### Documentation-hygiene note

The plan's task checkboxes are uniformly unchecked (0 checked / 92 `- [ ]`). Completion is tracked in
the live ledgers (`memory-map.md` tracking table → all ✔; `struct-verification.md` → 24/24) per the
plan's own "tracking table stays the live source of truth" design, plus the commit history. Not a
functional gap, but the plan checkboxes were never reconciled to the completed state.

## Build Results

| Region | Version | Configure / Validator | Build (WSL clang-cl) | Notes |
|--------|---------|-----------------------|----------------------|-------|
| GMS | 72.1 | PASS — `OK: all 159 keys defined and non-empty for GMS v72.1` | PASS — `[68/69] Linking … redirect-1.0.0.dll  >> OK` | only `-Wdeprecated-declarations`/`-Wmicrosoft-cast` warnings in `redirect/dllmain.cpp` (pre-existing, not v72-specific) |
| GMS | 79.1 | (regression target) | PASS — `>> OK` | regression check: v79 branch preserved across the v72 splits |

**Live key count:** `grep … include/memory_map.h.in … | wc -l` → `159` (matches plan's pinned count).

**FR-13 preprocess gate verification** (this audit, `gcc -E`):

| Field | v72 | v79 | v83 | Verdict |
|---|---|---|---|---|
| `CMob::m_bAttackReady` | absent (0) | present (1) | present (1) | correct — v72 excluded, v79 preserved |
| `MobStat::bDisable` | absent (0) | present (1) | present (1) | correct — v72 excluded, v79 preserved |

**Cat-A size guards** (build-confirmed, since the WSL build links): `CWvsApp` 0x60 branch now includes
`== 72` (`common/CWvsApp.h:97`); `CFuncKeyMappedMan` 0x388 branch now includes `== 72`
(`common/CFuncKeyMappedMan.h:51`). The successful link implies both `assert_size` chains fire for v72.

**CI matrix:** `.github/workflows/_build.yml:48` adds `- { region: GMS, major: 72, minor: 1 }` as the
first (version-ascending) entry. Authoritative MSVC/Win32 CI is not runnable on this WSL box; not exercised here.

## Manual Verification Required

Task 19 (user-run, gates completion — `acceptance.md`):

1. Build/obtain GMS 72.1 x86 DLLs (post-merge `snapshot-GMS-72.1-Release` artifact or local VS2022
   Win32 `--target package_dlls`; the PR matrix build runs `upload: false` and publishes nothing).
2. Deploy proxy `ijl15.dll` + core edits (`redirect`, `no-patcher`, `no-ad-balloon`, `bypass`) per README.
3. Launch the live v72 client; confirm: no crash / no Themida fault; patcher suppressed; reaches
   title/login; `redirect` targets configured IP; no `bypass` startup fault; no ad balloon on exit.
4. Record verbatim outcome into `acceptance.md` and the PR; distinguish build-correctness from
   environment (Themida / VC++ redist / OS) per R13.

## Overall Assessment

- **Plan Adherence:** MOSTLY_COMPLETE — every implementation task (1–18) faithfully executed and
  build-verified; the single deviation (Task 17 split sites) is evidence-driven and correctly
  surfaced; the only incomplete item is the by-design user-gated acceptance test (Task 19).
- **Recommendation:** NEEDS_REVIEW — code is faithful and builds green (72.1 + 79.1); merge is gated
  on the user-run live smoke test (Task 19) per FR-16/R13. No code fixes are required to proceed to
  that acceptance run.

## Action Items

1. **User:** run Task 19 live smoke test against a real GMS v72.1 client and record the result in
   `acceptance.md` + the PR. This is the only blocker to "done."
2. (Optional, hygiene) Reconcile the plan's task checkboxes to the completed state, or note in the
   PR that the live ledgers (`memory-map.md` / `struct-verification.md`) are the completion source.
3. (Optional, accuracy) Adjust commit `225b793`'s framing in the PR summary: the CMob/MobStat fixes
   are new `>= 79` member gates at the *actual* v72-divergent members, not three-way splits of the
   predicted `>= 83 || JMS` doom-tail/Weakness gates (which were already correct).
4. (Optional, follow-up) Track the surfaced SecondaryStat −0xD8 ungated base shrink as a known
   residual for the next older port; it is currently harmless (no `assert_size`, no v72 deref) but is
   SIZE-CRITICAL if a future edit dereferences `CWvsContext` fields after `m_secondaryStat` on v72.
