# GMS v72 Support — Execution Context (quick reference)

A one-page orientation for an agent picking up this task. Authoritative detail lives
in `prd.md`, `memory-map.md`, `struct-verification.md`, and `signature-catalog.md`;
this is the cheat-sheet.

## What this task is

Make **GMS v72.1** a first-class build target: a complete **159-key** memory map
located in the v72 binary, a labeled v72 IDB, all 24 version-gated `common/*.h`
headers verified against v72, the below-floor gates audited (amended where v72 has no
branch / split where v72 diverges from v79), and the CI matrix wired. It is a
**porting/relocation** task — no new client-edit features. Direct follow-on to
**task-008 (v79)**; reuse its signature catalog and labeled v79 IDB.

> **Key count is 159** (160 `@…@` placeholders in `include/memory_map.h.in` minus
> `BUILD_REGION`). Re-pin at task start; stop if it isn't 159.

## Why v72 is uniquely risky: below the *new* floor, and the floor already moved

task-008 made **v79** the minimum and rewrote the `common/` gates to give v79 defined
branches. v72 sits below v79 and re-triggers the below-floor problem one notch lower,
against a gate landscape task-008 already reshaped:

1. **Enumerated gates task-008 added *for v79* don't include v72** → lost size guard /
   silent layout shift. `CWvsApp.h:97` (`==79||==83||==84`) and
   `CFuncKeyMappedMan.h:50` (`==79`) are false for v72. Amend early.
2. **`>= 83` member gates now exclude v72 too** → v72 inherits v79's reduced branch.
3. **NEW vs the v79 port:** the branch v72 lands on is sometimes the *v79-reduced*
   branch (CWnd −8, CMob 0x518, MobStat 0x1F8), not the v83 branch. v72 might match
   v79 — or diverge further and need a **three-way** gate split. No anchor below v72.

## Anchor strategy

- **Primary anchor: GMS v83** — most complete canonical symbol set; canonical
  names/prototypes come from here.
- **Closest anchor: GMS v79** — newly labeled by task-008, 7 versions above v72.
  Use as the immediately-adjacent cross-check and the **below-floor template**: every
  v79≠v83 divergence is the first place to look for v72. Seed `v72_1.cmake` from
  `v79_1.cmake`.
- **v87 / v95 PDB** — secondary, upper-gate cross-checks only.

## Gate-direction cheat-sheet (truth value at v72)

| Gate form | v72 value | v72 lands on | Action |
|---|---|---|---|
| `>= 95`, `>= 111`, `>= 87`, `>= 84`, `> 83`, `== 95`, `== 87`, `== 83` | false | base/excluded (like v79) | Confirm base branch matches v72 size/layout (FR-12c) |
| `>= 83 \|\| JMS` (task-008's two-way splits) | **false** | **v79-reduced branch** | **Confirm v72 == v79; else split three-way (FR-12b)** |
| `< 95`, `< 84` | true | base/included | Confirm v72 base layout == v79/v83 |
| `== 79 \|\| == 83 \|\| == 84` / `== 79` (no else) | **false** | **NO BRANCH** | **Amend — add v72 (restores size guard)** |

## Live gate sites (re-grep at task start — lines may shift)

- **Enumerated, no v72 branch (Cat A):** `CWvsApp.h:97`, `CFuncKeyMappedMan.h:50`,
  `CLogin.h:235` (`==83` member), upper `==95`/`==87` gates (`CConfig.h:84`,
  `CLogo.h:93`, `SecondaryStat.h:405/419`).
- **Two-way `>= 83 || JMS` (exclude 79 *and* 72) — confirm or split (Cat B):**
  `CFuncKeyMappedMan.h:18`, `CMob.h:239`, `CUIToolTip.h:92`, `CWnd.h:25`,
  `MobStat.h:128`. Plus `doom-fix` `== 83` write gate.

## IDB instances (always `get_metadata` before concluding; `select_instance` is global)

| Version | Role |
|---|---|
| GMS v72.1 | **Target** — locate, label, `idb_save`. **Sparse/unverified** — confirm identity every probe. |
| GMS v79.1 | **Closest anchor** — task-008-labeled; below-floor template. ABOVE v72. |
| GMS v83 | **Primary anchor** — canonical names/prototypes. ABOVE v72. |
| GMS v87 | Secondary anchor / upper-gate cross-val |
| GMS v95 (PDB) | PDB reference / upper-gate cross-val |
| JMS v185 | JMS-only sentinel confirm |

`select_instance` is **global shared state** — never run two IDA-probing tasks
concurrently. One serialized lane. [[feedback-verify-ida-target]]

## Key files

| File | Role |
|---|---|
| `memory_maps/GMS/v72_1.cmake` | **output** — 159 keys; seed from `v79_1.cmake`, relocate per cluster |
| `memory_maps/GMS/v79_1.cmake` | seed source + closest anchor values |
| `memory_maps/GMS/v83_1.cmake` | canonical-name anchor |
| `include/memory_map.h.in` | the 159-key contract (CI fails on any missing key) |
| `cmake/CheckMemoryMapKeys.cmake` | completeness check: `cmake -DREGION=GMS -DMAJOR=72 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake` |
| `.github/workflows/_build.yml` | CI matrix — add `{ GMS, 72, 1 }` first |
| `common/*.h` (24 gated) | struct layouts — audit + amend per `struct-verification.md` |
| `scripts/wsl-build.sh` | local clang-cl pre-flight: `scripts/wsl-build.sh GMS 72 1` |
| `docs/tasks/task-008-gms-v79-support/` | **direct precedent** — port its catalog + below-floor verdicts forward |

## Suggested resolution order

Per README §"Adding a new version": **WinMain → CWvsApp** first (anchor the image,
gain call-graph reach), then **CClientSocket/ZSocket** and **COutPacket** (highest
value for redirect/bypass), then login/stage/logo, then manager singletons, then
utilities, then exception-dispatch keys, then sentinels last. Run the **enumerated-
gate amendment (FR-12a) early** so v72 keeps its size guards, then the two-way gate
confirmation (FR-12b).

## Discipline reminders

- Evidence before assertion; no "same as v79" without a confirming signature
  ([[feedback-prefer-confirmation]]) — doubly important with no anchor below v72 and a
  base branch that already diverged from v83.
- Struct verification is **read-only** over raw disassembly; don't apply speculative
  struct types into the IDB (decompiler leak).
- Feature search goes *backward*: v79-present features may be absent in v72 (inverse of
  the v84 sentinel direction).
- Work on a feature branch, never commit to `main` ([[feedback-no-main-commits]]).
</content>
