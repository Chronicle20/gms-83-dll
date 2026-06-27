# GMS v61 Support — Execution Context (quick reference)

A one-page orientation for an agent picking up this task. Authoritative detail lives
in `prd.md`, `design.md`, `memory-map.md`, and (created during execution)
`struct-verification.md` / `signature-catalog.md`; this is the cheat-sheet.

## Pinned baseline (fill in at Task 1 Step 1)

- **Branch:** `task-010-gms-v61-support`, created from `task-009-gms-v72-support` HEAD,
  in a worktree under `.claude/worktrees/` (D0/§0). `main` untouched.
- **task-009 tip SHA:** `247dd822f6194aa0d8ba33d7fde95f82ba9e785d` (record here +
  in the PR — this is "the baseline" as an exact commit, R13).
- **Packing verdict (D9) — FINALIZED (Task 2):** `pre-Themida` — standard PE with named sections (.text/.idata/.rdata/.data/.export/.import), full import table (kernel32/user32/ws2_32/npkcrypt/etc.), named entry points (ZtlTaskMemAllocImp, ZtlTaskMemFreeImp, ZtlTaskMemReallocImp, start@0x87921f). No Themida/Oreans sections; no obfuscated stub. Determined Task 1 Step 5 via survey_binary on port 13344; **CONFIRMED Task 2**: the PE entry `start`(0x87921F) is a normal C-runtime stub that walks cleanly to `_WinMain@16`(0x8205EF) — fully decompilable, no virtualized/CFG-obfuscated bodies in the WinMain/CWvsApp cluster (contrast v83 SetUp which is CFG-virtualized). Smoke-test fault expectations adjust per design §6 (no Themida integrity faults).
- **DR_init / anti-debug finding (R11, Task 2) — FINALIZED:** v61 **LACKS the DR subsystem**, confirmed from disasm (not assumed). `CWvsApp::SetUp`(0x823175) init sequence has NO DR_init / NtGetContextThread / anti-debug step — anti-tamper is ONLY `CSecurityClient::CreateInstance`+`InitModule`. v61 is **even cleaner than v72**: it also lacks v72's `Global\meteora`+`ehsvc`+IAT-clone+HideDll+WSAStartup-integrity machinery. Also NO `InitializeAuth` (NMCO auth post-dates v61). `DR_INIT`/`DR_CHECK` stay sentinels (feeds Task 10). The v84 in-field freeze class ([[project_v84_movement_anticheat_freeze]]) cannot occur in v61.

## What this task is

Make **GMS v61.1** a first-class build target: a complete **159-key** memory map
located in the v61 binary, a labeled v61 IDB, all 24 version-gated `common/*.h`
headers verified against v61, the below-floor gates audited (amend where v61 has no
branch / split where v61 diverges from v72), and the CI matrix wired. It is a
**porting/relocation** task — no new client-edit features. Direct follow-on to
**task-009 (v72)**; **consume** its signature catalog, labeled v72/v79 IDBs, and v72
memory map.

> **Key count is 159** (160 `@…@` placeholders in `include/memory_map.h.in` minus
> `BUILD_REGION`). Re-pin at task start; stop if it isn't 159.

## Why v61 is uniquely risky: two tiers below the original floor

task-008 made **v79** the floor; task-009 lowered it to **v72**. v61 sits below v72
and re-triggers the below-floor problem one notch lower again, against a gate
landscape **two** prior tasks reshaped:

1. **Enumerated gates that include v72/v79 don't include v61** → lost size guard /
   silent layout shift. `CWvsApp.h:98` (`==72||==79||==83||==84`) and
   `CFuncKeyMappedMan.h:52` (`==72||==79`) are false for v61. Amend early (Task 3).
2. **`>= 83` member gates exclude v61 too** → v61 inherits the v72/v79-reduced `#else`.
3. **The novel tier:** the branch v61 lands on is the *v72-reduced* `#else`, which
   already diverged twice from the v83 base. v61 might match v72 — or diverge further
   and need a **split**. **No labeled IDB below v61**; a **v48 IDB is loaded but is a
   distractor, never an anchor.**

## Arity note (live-pinned — important)

task-009 left **all five Category-B gates two-way** (`>= 83 || JMS` / `#else`) — there
are **zero `< 79` arms** — and folded v72 into the v79 enumerated branches. So a v61
divergence is **two-way → three-way** (add a GMS-guarded `< 72` arm above the unchanged
`#else`), **not** the four-way the design's general language describes. Re-grep
`BUILD_MAJOR_VERSION < 79` at task start; only if task-009's final state added a `< 79`
arm to a gate is that gate's v61 split four-way (then `< 72` goes above `< 79`).

## Anchor strategy

- **Primary/canonical anchor: GMS v83** (gap 22) — canonical names/prototypes from here.
- **Closest anchor: GMS v72** (gap 11) — task-009-labeled; below-floor template + the
  relocation source. Seed `v61_1.cmake` from `v72_1.cmake`. ABOVE v61.
- **Secondary below-floor x-check: GMS v79** (gap 18) — task-008-labeled; chain-trace
  intermediate. ABOVE v61.
- **v84 / v87 / v95 PDB** — upper anchors / upper-gate cross-checks.
- **GMS v48** — loaded **distractor only; NEVER an anchor** (PRD non-goal).

## Gate-direction cheat-sheet (truth value at v61)

| Gate form | v61 value | v61 lands on | Action |
|---|---|---|---|
| `>= 95`, `>= 111`, `>= 87`, `>= 84`, `> 83`, `== 95`, `== 87`, `== 83` | false | base/excluded (like v72) | Confirm base branch matches v61 size/layout (FR-12c) |
| `>= 83 \|\| JMS` (two-way; v61 + v72 + v79 excluded) | **false** | **v72/v79-reduced `#else`** | **Confirm v61 == v72; else split (add `< 72` arm) (FR-12b)** |
| `< 95`, `< 84` | true | base/included | Confirm v61 base layout == v72/v83 |
| `== 72 \|\| == 79 \|\| == 83 \|\| == 84` / `== 72 \|\| == 79` (no else) | **false** | **NO BRANCH** | **Amend — add v61 (restores size guard)** |

## Live gate sites (re-grep at task start — lines may shift)

- **Enumerated, no v61 branch (Cat A → Task 3):** `CWvsApp.h:98`,
  `CFuncKeyMappedMan.h:52`, `CLogin.h:235` (`==83` member). Upper `==95`/`==87` gates
  (`CConfig.h:84`, `CLogo.h:93`, `SecondaryStat.h:405/419`) are Cat C, not Cat A.
- **Two-way `>= 83 || JMS` (exclude 79/72/61) — confirm or split (Cat B → Task 12):**
  `CWnd.h:25`, `CUIToolTip.h:92`, `CFuncKeyMappedMan.h:19`, `MobStat.h:128`,
  `CMob.h:241`. Plus the doom-fix `== 83` write gate (`doom-fix/dllmain.cpp:27`) and
  the client-key `> 83` gate (`common/CWvsContext.h:98`, `bypass/socket_hooks.cpp:310`).

## IDB instances (always `get_metadata` before concluding; `select_instance` is global)

| Version | Binary | Role |
|---|---|---|
| GMS v61.1 | `GMS_v61.1_U_DEVM.exe` | **Target** — locate, label, `idb_save`. **Sparse/unverified** — confirm identity every probe; **distinguish from v48.** |
| GMS v72.1 | `GMS_v72.1_U_DEVM.exe` | **Closest anchor (gap 11)** — task-009-labeled; seed + template. ABOVE v61. |
| GMS v79.1 | `GMS_v79_1_DEVM.exe` | Secondary below-floor x-check (gap 18) — task-008-labeled. ABOVE v61. |
| GMS v83 | `MapleStory_dump.exe` | Canonical anchor (gap 22) — names/prototypes. ABOVE v61. |
| GMS v84/v87 | `GMS_v84.1_U_DEVM.exe` / `GMSv87_4GB.exe` | Upper anchors / upper-gate cross-val |
| GMS v95 (PDB) | `GMS_v95.0_U_DEVM.exe` | PDB reference / upper-gate cross-val |
| JMS v185 | (JMS185) | JMS-only sentinel confirm |
| **GMS v48.1** | `GMS_v48_1_DEVM.exe` | **DISTRACTOR — unlabeled, NEVER an anchor.** Reason `get_metadata` is mandatory. |

`select_instance` is **global shared state** — never run two IDA-probing tasks
concurrently. One serialized lane. [[feedback-verify-ida-target]]

## Key files

| File | Role |
|---|---|
| `memory_maps/GMS/v61_1.cmake` | **output** — 159 keys; seed from `v72_1.cmake`, relocate per cluster |
| `memory_maps/GMS/v72_1.cmake` | seed source + closest anchor values (task-009) |
| `memory_maps/GMS/v79_1.cmake` | secondary below-floor anchor values (task-008) |
| `memory_maps/GMS/v83_1.cmake` | canonical-name anchor |
| `include/memory_map.h.in` | the 159-key contract (CI fails on any missing key) |
| `cmake/CheckMemoryMapKeys.cmake` | completeness check: `cmake -DREGION=GMS -DMAJOR=61 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake` |
| `.github/workflows/_build.yml` | CI matrix — add `{ GMS, 61, 1 }` first |
| `common/*.h` (24 gated) | struct layouts — audit + amend per `struct-verification.md` |
| `scripts/wsl-build.sh` | local clang-cl pre-flight: `scripts/wsl-build.sh GMS 61 1` |
| `docs/tasks/task-009-gms-v72-support/` | **direct precedent + consumed artifact** — catalog, struct verdicts, labeled IDBs, maps |

## The 24 gated headers (partition across Tasks 12–16)

CWvsApp, CWvsContext, CClientSocket, CLogin, COutPacket, CConfig, ConfigSysOpt (T13);
CUITitle, CUILoginStart, CUIToolTip, CUIWnd, CWnd, CCtrlButton, CCtrlCheckBox,
CFadeWnd, CLogo (T14); CMob, MobStat, SecondaryStat, CMapLoadable (T15); PartyData,
PartyMember, GuildData, CFuncKeyMappedMan (T16). Five Cat-B headers (CWnd, CMob,
MobStat, CFuncKeyMappedMan, CUIToolTip) get their confirm-or-split verdict in T12 first.

## Suggested resolution order

Per README §"Adding a new version": **WinMain → CWvsApp** first (anchor the image,
gain call-graph reach, determine packing), then **CClientSocket/ZSocket** and
**COutPacket** (highest value for redirect/bypass), then login/stage/logo, then manager
singletons, then utilities, then party/migrate senders, then exception-dispatch keys,
then sentinels last. Run the **enumerated-gate amendment (FR-12a) early** (Task 3) so
v61 keeps its size guards, then the two-way gate confirmation (FR-12b, Task 12).

## Discipline reminders

- Evidence before assertion; no "same as v72" without a confirming signature
  ([[feedback-prefer-confirmation]]) — triply important: no anchor below v61, base
  branch already twice-diverged, closest anchor 11 versions away.
- Confirm the IDB with `get_metadata` before **every** probe and **distinguish v61 from
  the v48 distractor** ([[feedback-verify-ida-target]]).
- Struct verification is **read-only** over raw disassembly; don't apply speculative
  struct types into the IDB (decompiler leak).
- Feature search goes *backward*: v72-present features may be absent in v61 (new
  v61-only sentinels — ad balloon, patcher, MapleTV, MonsterBook, MTS, CFileStream
  relay). Flag them; the consuming edit/gate must tolerate `0`.
- DR_init is expected era-absent in v61 (the v84 freeze class); confirm, don't assume.
- Work on the feature branch (off task-009), never commit to `main`
  ([[feedback-no-main-commits]]); rebase onto final task-009 + re-confirm inherited
  gates before merge (R13).
