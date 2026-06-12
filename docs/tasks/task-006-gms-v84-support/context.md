# GMS v84 Support — Execution Context (quick reference)

A one-page orientation for an agent picking up `plan.md`. Authoritative detail
lives in `plan.md`, `design.md`, and `memory-map.md`; this is the cheat-sheet.

## What this task is

Make **GMS v84.1** a first-class build target: a complete 145-key memory map
located in the v84 binary, a labeled v84 IDB, all 22 version-gated `common/*.h`
headers verified against v84, the five 83/84 boundary gates audited (and rewritten
if wrong), and the CI matrix wired. It is a **porting/relocation** task — no new
client-edit features.

## Why v84 is uniquely risky

v84 is the **only supported version strictly between 83 and 87**. So:
- A `> 83` gate is **true** for v84 → puts v84 on the newer (v87+) side.
- A `>= 87` gate is **false** for v84 → puts v84 on the older (v83) side.
Either may be wrong for a given field — only the v84 layout decides. Every `> 83`
and `>= 87` gate is therefore a v84 trap, not just the five pre-flagged ones.

## IDB ports (always `select_instance` then `get_metadata` before concluding)

| Version | Port | Role |
|---|---|---|
| GMS v84.1 | **13341** | Target — locate, label, `idb_save` |
| GMS v83 | 13337 | Primary anchor (best-labeled) |
| GMS v87 | 13338 | Secondary anchor / gate cross-val |
| GMS v95 | 13339 | PDB reference / gate cross-val |
| JMS v185 | 13340 | JMS-only sentinel confirm / gate cross-val |
| GMS v111 | — | not loaded; validate from source + `v111_1.cmake` |

`select_instance` is **global shared state** — never run two IDA-probing tasks
concurrently. One serialized lane (subagent-driven-development already enforces
one-task-at-a-time). [[feedback-verify-ida-target]]

## Key files

| File | Role |
|---|---|
| `memory_maps/GMS/v84_1.cmake` | **output** — 145 keys; seeded from v83 in Task 1, relocated per cluster |
| `memory_maps/GMS/v83_1.cmake` | seed source + primary v83 anchor values |
| `include/memory_map.h.in` | the 145 `@KEY@` placeholders (the canonical key set) |
| `cmake/GenerateMemoryMap.cmake` | CI completeness gate (fails on any missing/empty key) |
| `cmake/CheckMemoryMapKeys.cmake` | **created Task 1** — MSVC-free local completeness check (`cmake -P`) |
| `.github/workflows/_build.yml` | single source of truth for the build matrix (FR-1) |
| `CMakeLists.txt` | selects the memory map by `BUILD_REGION`/`MAJOR`/`MINOR`; includes GenerateMemoryMap |
| `common/*.h` (22 gated) | structs verified read-only in Tasks 12–15 |

## The five boundary gates (Task 11 verifies first)

| Site | Gate | v84 must confirm |
|---|---|---|
| `doom-fix/dllmain.cpp:25` | `< 84` | Is `m_bDoomReserved=0` fixup still needed in v84? (comment says fixed in v84) |
| `common/CWvsContext.h:98` | `> 83` | Does v84 CWvsContext carry `m_aClientKey[8]`? |
| `bypass/socket_hooks.cpp:233` | `> 83` | Does v84 hello path `EncodeBuffer(m_aClientKey,8)`? (same fact as above) |
| `common/CLogin.h:235` | `== 83` | Is `int unk3[5]` v83-only (absent in v84)? |
| `common/CUIToolTip.h:92` | `>= 83` | Confirm v84 still has `m_pLayerAdditional` |

## Memory-map cluster → plan task map

| Task | Cluster | ~keys |
|---|---|---|
| 2 | WinMain + CWvsApp + WndMan + globals (+ 2 WinMain offsets) | 24 |
| 3 | CClientSocket / ZSocket | 14 |
| 4 | COutPacket encoders | 7 |
| 5 | Login / Stage / Logo / Title | 19 |
| 6 | Manager singletons (Action/Anim/FKM/Input/Macro/MapleTV/MonsterBook/Quest/Radio/Security) | 37 |
| 7 | Config / SystemInfo / IGCipher / ZArray / ZXString / FatalSection / CMob ctor | 21 |
| 8 | Party / migrate / CWvsContext senders + 4 offsets | 9 |
| 9 | Protocol constants + GMS-absent + JMS-only sentinels | 13 |

(Counts are approximate; the authoritative per-key list is `memory-map.md`'s
145-row tracking table — every row must reach `✔` by Task 10.)

## Evidence rules (non-negotiable)

- **Two independent anchors** per absolute key before it is written; ≥1 must be
  structural (string/import/call-graph/constant/vtable). A pair of byte-sigs is
  invalid. High-value keys (socket send/flush/process, COutPacket encoders,
  WinMain, CWvsApp::Run/SetUp, packet senders, boundary-adjacent fns) need two
  *different* kinds + `needs-main-review` re-probe. (design §4)
- **Offsets re-measured** from v84 disasm, never copied from v83. (SP-2, R4)
- **Sentinels confirmed absent** via the feature's own anchor before carrying
  `0x00000000`. (SP-5, R3)
- **Struct audit is read-only** — `disasm` not `decompile`, never apply a struct
  type into the IDB (decompiler leak, R6). Function/global *labeling* is fine.
- Anchor every claim to a concrete disasm line; don't defer settleable questions.
  [[feedback-prefer-confirmation]]

## Build/verify reality on this box

- This is **Linux/WSL** — the MSVC/Win32 client-DLL compile **cannot** run here.
- Local gate available: `cmake -DREGION=GMS -DMAJOR=84 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake`
  (asserts all 145 keys defined+non-empty). Run it after every cluster.
- Gate-logic sanity: `gcc -E` preprocessor checks on changed headers (Task 16).
- Full Debug+Release build + cross-version regression = **CI** (Task 17). Live
  client smoke test = **user** (Task 18). Report only what was actually run.

## Cross-validation oracle

`~/source/atlas-ms/atlas/` (sibling Go server) has version-aware registries —
especially `libs/atlas-packet/model/character_temporary_stat.go` — useful when
verifying bitmask-indexed structs like `SecondaryStat`. [[reference-atlas-ms]]

## Definition of done (PRD §10)

Matrix has `{GMS,84,1}` · 145 keys defined+non-empty · every absolute key has a
2-anchor catalog row · every offset re-derived · every sentinel justified · v84
IDB labeled + saved · 22 headers have a recorded verdict · each boundary gate
verified (rewrites cross-version validated) · GMS 84.1 green Debug+Release in CI ·
live v84 smoke test recorded.
