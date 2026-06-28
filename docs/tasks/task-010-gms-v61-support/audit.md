# Plan Audit — task-010-gms-v61-support

**Plan Path:** docs/tasks/task-010-gms-v61-support/plan.md
**Audit Date:** 2026-06-28
**Branch:** task-010-gms-v61-support
**Base Branch:** task-009-gms-v72-support (pinned tip `247dd822f6194aa0d8ba33d7fde95f82ba9e785d`), retargets to main once 008/009 land

## Executive Summary

All 18 implementation tasks (T1–T18) are faithfully executed; Task 19 (acceptance) is pending-by-design — a user-run live smoke test plus the rebase onto final task-009. The v61 memory map is complete (validator: `OK: all 159 keys`), the three Category-A size gates and the below-floor member splits are in place, the full v61 `SecondaryStat` body carries `assert_size(... , 0x970)`, and the final-review Critical fix (sentinel consumers guarded `!= 0`) is present. All three targeted builds compile + link clean (GMS 61.1 target; GMS 72.1 / 79.1 regression). No silently skipped or under-delivered task found. Note: the plan's checkboxes were left mostly unticked (only Task 10's 6 steps marked `[x]`); completion was judged from the 24 commits + artifacts, which clearly map 1:1 to the task list.

## Task Completion

| # | Task | Status | Evidence / Notes |
|---|------|--------|------------------|
| 1 | Scaffold: worktree off task-009, seed cmake from v72, 159 keys, tracking table | DONE | commit 9125c74; `memory_maps/GMS/v61_1.cmake` (159 `set(` lines, seeded-from-v72 header line 1); context.md:11 pins baseline SHA 247dd82 |
| 2 | WinMain + CWvsApp + window-manager cluster (~26 keys) | DONE | commit 795bc2a; keys in v61_1.cmake + signature-catalog.md |
| 3 | Cat-A gates (CWvsApp 0x58 branch, CFuncKeyMappedMan, CLogin) | DONE | commit 1dab5bb; CWvsApp.h:100-106 adds `== 61` arm; CFuncKeyMappedMan.h:52 `(== 61 || == 72 || == 79)`; CLogin.h:235 left `== 83` (v61 confirmed to lack `unk3[5]`, verdict unchanged) |
| 4 | CClientSocket / ZSocket cluster (~14 keys) | DONE | commit 01ad7a9 |
| 5 | COutPacket cluster (~7 keys) | DONE | commit da53249 |
| 6 | Login / Stage / Logo / Title cluster (~19 keys) | DONE | commit b98af8a |
| 7 | Manager singletons cluster (~37 keys) | DONE | commit a6b88c2 |
| 8 | Config / SystemInfo / IGCipher / utilities (~19 keys) | DONE | commit a4e68b3 |
| 9 | Party / migrate / context senders + offsets (~9 keys) | DONE | commit e5b7255 |
| 10 | Protocol constants + exception-dispatch + CFileStream + sentinels (~23 keys) | DONE | commit 7abae6c; only task with steps ticked `[x]` (plan.md:570-598) |
| 11 | Memory-map completeness gate | DONE | commit 6872337; validator `OK: all 159 keys`; memory-map.md tracking table all ✔ (the only ☐/◐ are legend text at lines 75,77) |
| 12 | Cat-B below-floor gates: confirm-or-split | DONE | commit 3c561d9; verdicts in struct-verification.md |
| 13 | Struct audit — core/net headers (7) | DONE | commit 816538a |
| 14 | Struct audit — UI/control family (9; CWnd cascade) | DONE | commit 793adcc |
| 15 | Struct audit — Mob/stat family (4) | DONE | commit 742de6f; deeper SecondaryStat rebuild in e7ec563/58d78d0/3332880 |
| 16 | Struct audit — Party/Guild/misc; 24/24 | DONE | commit 35bd4f5; struct-verification.md "24/24 complete" assertion present |
| 17 | Apply gate rewrites (splits + confirms) + cross-version validation | DONE | commit 0cb7b4b (member splits) + 2bc1b2c (full v61 SecondaryStat body, static_assert 0x970) + 7076c92 (truth-table correction). See split inventory below |
| 18 | Build wiring + build verification | DONE (CI-green = Step 5, CI/user domain) | commit 48bc69e; `_build.yml:48` adds `{GMS,61,1}` first; all 3 local builds OK (below) |
| 19 | Acceptance — rebase on final task-009 + live smoke test | PENDING BY DESIGN | acceptance.md not present; user-gated per plan §Task 19; not a defect |
| — | Final-review Critical fix: guard v61-0x0 sentinel consumers `!= 0` | DONE | commit b1c504f; bypass/app_hooks.cpp SetUp/ctor reimpl |

**Completion Rate:** 18/18 implementation tasks (100%); T19 pending-by-design.
**Skipped without approval:** 0
**Partial implementations:** 0

## Gate-Split Inventory (T3 + T17 verification)

Member-level / size-gate edits, all evidence-anchored:

- `common/CWvsApp.h:38` — `#if BUILD_MAJOR_VERSION >= 72` excludes the 2nd IP-check + GG-hook timer for v61; comment records v61 `sizeof 0x58`. Cat-A size assert chain (`:100-106`) adds a dedicated `== 61` branch (v61 measured 0x58, not the v72/79/83/84 0x60 — an evidence-driven divergence from the plan's "if v61 == 0x60" assumption, handled exactly per Task 3 Step 2's "new == 61 branch" path).
- `common/CFuncKeyMappedMan.h:52` — v61 folded into the `0x388` assert branch `(== 61 || == 72 || == 79)`; member gate `:19` `>= 83 || JMS` unchanged (quickslot pair absent for v61).
- `common/CLife.h:11` — new `BUILD_MAJOR_VERSION < 72` v61 split arm.
- `common/CMob.h:144,154` — `#if !(... < 72)` excludes v61 members.
- `common/MobStat.h:115` — `#if !(... < 72)` excludes v61 members.
- `common/SecondaryStat.h:6` — full v61 body under `#if ... < 72`; `:1152-1153` `assert_size(sizeof(SecondaryStat), 0x970)`. **Static-assert 0x970 confirmed present and the v61 build passes it.**
- `common/CLogin.h:235`, `common/CWvsContext.h:98` — left unchanged (`== 83` / `> 83`); v61 confirmed to lack `unk3[5]` and the 8-byte client key respectively.

Note: these use a `< 72` GMS-guarded form (per design D8) rather than `== 61`, future-proofing the next older port. The `>= 83 || JMS` / `#else` v72/79 branches are preserved (confirmed by the green 72.1/79.1 builds).

## Sentinel-Guard Critical Fix (verification)

`bypass/app_hooks.cpp` (commit b1c504f) guards each v61-0x0 memory-map consumer with the established `KEY != 0` idiom so the call compiles out below the floor instead of `call(0)` → AV:
- SetUp: `C_WVS_APP_INITIALIZE_AUTH != 0`, `GET_SE_PRIVILEGE != 0`, `C_MACRO_SYS_MAN_CREATE_INSTANCE != 0`, `C_QUEST_MAN_LOAD_PARTY_QUEST_INFO != 0`, `C_QUEST_MAN_LOAD_EXCLUSIVE != 0`, `C_RADIO_MANAGER_CREATE_INSTANCE != 0`.
- ctor reimpl: `G_DW_TARGET_OS != 0` (both `*g_dwTargetOS` stores), `RESET_LSP != 0`.

All use `!= 0` as required.

## Build Results

All run via `scripts/wsl-build.sh` (single-config Ninja **Debug**, clang-cl + xwin; Release is CI's domain — not claimed here).

| Region | Version | Configure | Build | Notes |
|--------|---------|-----------|-------|-------|
| GMS | 61.1 | PASS | PASS | `>> OK`; 69 targets built+linked; only `-Wdeprecated-declarations`/`-Wmicrosoft-cast` warnings (pre-existing in redirect/). v61 SecondaryStat `assert_size 0x970` passed. |
| GMS | 72.1 | PASS | PASS | `>> OK` (exit 0); regression — splits preserve v72 `#else` branch |
| GMS | 79.1 | PASS | PASS | `>> OK` (exit 0); regression — splits preserve v79 `#else` branch |

Completeness validator:
```
cmake -DREGION=GMS -DMAJOR=61 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake
-- OK: all 159 keys defined and non-empty for GMS v61.1
```

## Manual Verification Required

Task 19 (user-gated, pending-by-design) — deferred to the user per plan:
1. Rebase task-010 onto the final task-009 tip and re-confirm every inherited gate branch (Task 19 Step 0); re-run `scripts/wsl-build.sh GMS 61 1` after rebase.
2. Deploy proxy `ijl15.dll` + core edits (redirect, no-patcher, no-ad-balloon, bypass, …) to a live GMS v61 client; reach title/login, exercise edits, confirm no crash / no Themida fault (FR-16). Interpret against the packing finding (D9) — a pre-Themida v61 client will not throw the README's Themida integrity faults.
3. Edits whose backing feature is a v61 sentinel (no-ad-balloon / no-patcher if era-absent) are expected no-ops — a "did nothing" result is not a failure.
4. Record verbatim outcome in `acceptance.md` (not yet created) and the PR; summarize against PRD §10.
5. CI green confirmation for GMS 61.1 Debug+Release and no regression on other versions (Task 18 Step 5).

## Overall Assessment

- **Plan Adherence:** FULL (T1–T18 complete; T19 pending-by-design as the plan specifies)
- **Recommendation:** READY_TO_MERGE pending the user-gated Task 19 acceptance (rebase + live smoke test). No code fixes required.

## Action Items

1. (User) Execute Task 19: rebase onto final task-009, create/populate `acceptance.md`, run the live v61 smoke test, confirm CI green. None of this blocks code correctness — it is the designed human gate.
2. (Optional housekeeping) The plan's checkboxes were left unticked for T1–T9, T11–T18 despite the work being done; consider ticking them to keep the ledger honest, or rely on this audit as the completion record.
