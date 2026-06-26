# GMS v79 Support — Execution Context (quick reference)

A one-page orientation for an agent picking up this task. Authoritative detail
lives in `prd.md`, `memory-map.md`, `struct-verification.md`, and
`signature-catalog.md`; this is the cheat-sheet.

## What this task is

Make **GMS v79.1** a first-class build target: a complete **159-key** memory map
located in the v79 binary, a labeled v79 IDB, all 24 version-gated `common/*.h`
headers verified against v79, the below-floor gates audited (and amended where
v79 has no branch / is wrongly excluded), and the CI matrix wired. It is a
**porting/relocation** task — no new client-edit features.

> **Key count is 159, not 155.** The PRD/design say "155" — that figure is stale;
> the live `include/memory_map.h.in` parses to **159** keys (excluding
> `BUILD_REGION`), having grown with the exception-dispatch + CFileStream keys.
> `plan.md` Task 1 Step 3 re-greps and pins the live count; stop if it isn't 159.

## Why v79 is uniquely risky: it is *below the floor*

Every supported version today is ≥ 83; the `common/` gates were authored treating
**v83 as the minimum**. v79 breaks that assumption in two ways:

1. **Enumerated gates with no `#else` leave v79 unhandled.** `== 83 || == 84 ||
   == 87` is **false for v79**. Known sites `CFuncKeyMappedMan.h:38` and
   `CWvsApp.h:97` are `assert_size` chains — with no v79 branch they **silently
   fire no size assertion** (v79 builds, but loses its size guard), not a hard
   build break. Amend early (Task 3) to restore the guard + lock the measured
   v79 size. The genuine *layout-shift* risk is the **member-declaring** gates:
   Cat B `CUIToolTip.h:92` (`>= 83`, drops `m_pLayerAdditional`) and Cat E
   `CLogin.h:235` (`== 83`, drops `unk3[5]`). Task 3 also empirically tests
   whether the v79 build was ever truly blocked rather than assuming it.
2. **`>= 83` / `>= 84` gates newly put v79 on the EXCLUDED side.** A `>= 83` gate
   that used to include *everything* now excludes v79. Is that correct? No anchor
   exists below v79 — v83 is the only neighbor and it's *above*.

Contrast v84 (between 83 and 87, anchors on both sides). v79 has **no lower
anchor at all**: triangulate from v83 upward only.

## Gate-direction cheat-sheet (truth value at v79)

| Gate form | v79 value | v79 lands on | Action |
|---|---|---|---|
| `>= 95`, `>= 111`, `>= 87`, `> 83`, `== 95`, `== 83` | false | base branch | Confirm base branch (= "v83 layout") matches v79 size/layout |
| `>= 84` | false | excluded (like v83) | Confirm gated field genuinely absent in v79 |
| `>= 83` | **false** | **excluded (NEW)** | Confirm v79 lacks field; else lower floor to `>= 79` |
| `< 95`, `< 84` | true | base/included | Confirm v79 base layout == v83 |
| `== 83 \|\| == 84 \|\| == 87` (no else) | **false** | **NO BRANCH** | **Amend — add v79 (build-blocking)** |

## IDB instances (always `get_metadata` before concluding; `select_instance` is global state)

| Version | Role |
|---|---|
| GMS v79.1 | **Target** — locate, label, `idb_save`. Confirm it's the active instance every probe. |
| GMS v83 | Primary anchor (closest + best-labeled). The *only* nearby anchor; it is ABOVE v79. |
| GMS v87 | Secondary anchor / upper-gate cross-val |
| GMS v95 (PDB) | PDB reference / upper-gate cross-val |
| JMS v185 | JMS-only sentinel confirm |

`select_instance` is **global shared state** — never run two IDA-probing tasks
concurrently. One serialized lane. [[feedback-verify-ida-target]]

## Key files

| File | Role |
|---|---|
| `memory_maps/GMS/v79_1.cmake` | **output** — 155 keys; seed from `v83_1.cmake`, relocate per cluster |
| `memory_maps/GMS/v83_1.cmake` | seed source + primary anchor values |
| `include/memory_map.h.in` | the 159-key contract (CI fails on any missing key) |
| `cmake/CheckMemoryMapKeys.cmake` | **already exists** (task-006) — standalone, MSVC-free completeness check: `cmake -DREGION=GMS -DMAJOR=79 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake` |
| `.github/workflows/_build.yml` | CI matrix — add `{ GMS, 79, 1 }` first |
| `common/*.h` (24 gated) | struct layouts — audit + amend per `struct-verification.md` |
| `scripts/wsl-build.sh` | local clang-cl pre-flight: `scripts/wsl-build.sh GMS 79 1` |
| `docs/tasks/task-006-gms-v84-support/` | the direct methodological precedent (plan + signature catalog to port forward) |

## Plan task index (`plan.md`)

| Task | Deliverable |
|---|---|
| 1 | Branch + seed cmake from v83 + reconcile live key count (159) + baseline IDB |
| 2 | Memory map: WinMain + CWvsApp + window-manager cluster (+ locate/size CFuncKeyMappedMan for Task 3) |
| 3 | **Early Cat-A** size-gate amendment (CWvsApp.h:97, CFuncKeyMappedMan.h:38) + prove v79 compiles |
| 4–10 | Memory map clusters: socket · COutPacket · login/stage/logo · managers · config/utils · party/migrate · constants+sentinels+exception-dispatch |
| 11 | Memory-map completeness gate (159/159, catalogued, IDB saved) |
| 12 | Struct audit: below-floor boundary gates first (Cat B/C/D + doom) |
| 13–16 | Struct audit: exhaustive v79 size of all 24 headers (core/net · UI/control · mob/stat · party/guild/misc) |
| 17 | Apply remaining gate rewrites (Cat B/E) + cross-version truth tables |
| 18 | Build wiring (matrix entry first) + WSL pre-flight + CI green |
| 19 | User-run live smoke test + final report |

## Suggested resolution order

Per README §"Adding a new version": **WinMain → CWvsApp** first (anchor the image,
gain call-graph reach), then **CClientSocket/ZSocket** and **COutPacket** (highest
value for redirect/bypass), then login/stage/logo, then manager singletons, then
utilities, then exception-dispatch keys, then sentinels last. Run the **enumerated-
gate amendment (FR-12a) early** — the build won't compile for v79 until those two
headers have a v79 branch.

## Discipline reminders

- Evidence before assertion; no "same as v83" without a confirming signature
  ([[feedback-prefer-confirmation]]) — doubly important with no anchor below v79.
- Struct verification is **read-only** over raw disassembly; don't apply
  speculative struct types into the IDB (decompiler leak).
- Feature search goes *backward* here: v83-present features may be absent in v79
  (inverse of the v84 sentinel direction).
- Work on a feature branch, never commit to `main` ([[feedback-no-main-commits]]).
</content>
