# GMS v61 Support Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add GMS v61.1 as a first-class supported build target — a complete 159-key memory map located in the v61 binary, a labeled v61 IDB, all 24 version-gated `common/*.h` headers verified against v61 with the **three-tier below-floor gate audit** resolved (amend enumerated gates that lack a v61 branch; confirm-or-split every two-way `>= 83 || JMS` gate task-009 left), and the CI matrix wired so every PR exercises v61.

**Architecture:** A `discovery → label → emit` pipeline produces `memory_maps/GMS/v61_1.cmake`, **seeded from `v72_1.cmake`** (the closest labeled anchor, 11 versions above — it already carries the below-floor relocations and sentinel dispositions), then each key relocated and re-verified against the v61 IDB. Canonical names/prototypes come from **v83** (the most complete symbol set). Because v61 sits **two tiers below the original floor** — below v72 (task-009), which is below v79 (task-008), which is below the v83 base — a separate read-only struct/gate audit treats every `common/` gate as a multi-tier question: "which branch does v61 land on, is that branch's layout actually v61's, and where it rides the v72-reduced `#else` does v61 match v72 or diverge further (needing a split)?" The enumerated no-`#else` gates are amended **early** (design D7) so the v61 build is provably compilable before the full audit runs. All IDA probing runs on a single serialized lane — the IDA MCP `select_instance` is global mutable state; concurrent multi-instance probing is unsafe and **doubly dangerous here because two indistinguishable below-floor IDBs (v61 target, v48 distractor) are both loaded** (design §D3-note).

**Tech Stack:** IDA Pro MCP (target v61 IDB + v72/v79/v83/v84/v87/v95/JMS185 reference IDBs, plus an unlabeled v48 distractor), CMake (`cmake/CheckMemoryMapKeys.cmake` key-completeness validator + the build's `GenerateMemoryMap` gate), GitHub Actions matrix build (MSVC/Win32), C++17 client-edit DLLs, `scripts/wsl-build.sh` (clang-cl + xwin local pre-flight).

## Global Constraints

- **Region/version under construction:** `{ region: GMS, major: 61, minor: 1 }`. `BUILD_REGION=GMS`, `BUILD_MAJOR_VERSION=61`, `BUILD_MINOR_VERSION=1`.
- **Stacked baseline (D0/§0).** This task **stacks on the task-009 (v72) tip**, not `main`. Create `task-010-gms-v61-support` from `task-009-gms-v72-support` HEAD in a dedicated worktree under `.claude/worktrees/`. **Pin the task-009 tip SHA in `context.md` and the PR** so "the baseline" is an exact commit. The PR targets `task-009-gms-v72-support` (or `main` once 008/009 land). **Rebase onto the final task-009 state before merge and re-confirm every inherited gate branch** (§7, R13) — if task-009's below-floor gates change after this task starts, v61's inherited branch assignments shift with them.
- **Key set is 159** (the live count parsed from `include/memory_map.h.in`, 160 `@…@` placeholders minus `BUILD_REGION`). **Phase 0 re-greps and pins the live number** (Task 1 Step 3); if it is not 159, stop and reconcile before proceeding. Every value below that names "159" means "the live count."
- **Seed from v72, names from v83.** `v61_1.cmake` is seeded from `memory_maps/GMS/v72_1.cmake` (closest; carries below-floor relocations + sentinel dispositions). Canonical function/global *names* applied to the IDB come from **v83** verbatim so cross-version greps align. **A value still equal to the v72 seed means UNVERIFIED** unless a v61 signature confirms it.
- **Evidence before assertion.** Every address, every offset, every gate verdict is anchored to a concrete v61 disassembly observation. "Same as v72" is acceptable **only** when a signature confirms the v61 size/address — never by proximity. This is triply load-bearing for v61: there is **no labeled IDB below v61** to triangulate against, the base branch is sometimes the already-twice-diverged v72-reduced branch, and the closest anchor (v72) is **11 versions away** ([[feedback-prefer-confirmation]]).
- **IDB target hygiene.** `get_metadata` confirms the connected IDB before *every* version-specific probe ([[feedback-verify-ida-target]]). The v61 IDB is **sparse/unverified** — confirm it is the expected `GMS_v61.1_U_DEVM` binary and **explicitly distinguish it from the loaded v48 distractor**; never infer the active IDB from "what I selected last."
- **Read-only struct audit.** During struct verification use `disasm`, never `decompile`; **never apply speculative struct types** into any IDB (decompiler leak, R10/R12). Function/global *labeling* of the v61 IDB for the memory map is encouraged and separate.
- **No commits to `main`.** Work on branch `task-010-gms-v61-support` throughout ([[feedback-no-main-commits]]).
- **Cross-version safety (FR-13).** Every gate amendment must keep v72/v79/v83/v84/v87/v95/v111/JMS185 selecting their current branch. Splits in particular must keep **v72's and v79's** selected branches unchanged. v111 is **not loaded** — validate it from build constants + `memory_maps/GMS/v111_1.cmake`; surface any unsettleable v111 case rather than assuming it (A2).
- **Split convention (D8).** When a two-way `>= 83 || JMS` gate must split because v61 diverges further than v72, v61's new branch uses a **GMS-guarded `BUILD_MAJOR_VERSION < 72`** range term (not `== 61`) — future-proofs the next older port and stays disjoint (no supported version is `< 72` except v61). The existing `#else` (v72/v79 branch, GMS 72..82) is left byte-identical in effect; the split adds an arm *above* it. **At the pinned baseline task-009 left every Category-B gate two-way (no `< 79` arm exists), so a v61 split is two-way → three-way.** Re-grep `BUILD_MAJOR_VERSION < 79`/`< 72` at task start; if task-009's final state added any `< 79` arm to a given gate, that gate's v61 split is four-way and the `< 72` arm goes **above** the `< 79` arm (design §5.5).
- **Backward sentinel direction.** Going from v72 *down* to v61, a feature v72 has may be *absent* in v61. Confirm each sentinel's v61 disposition starting from task-009's **v72** verdict (not v83's stale comments); a v72 real-address key whose feature is absent in v61 becomes a **new v61-only sentinel**, flagged — not silently zeroed (R7).
- **Packing determination (D9).** Determine **early** (during Cluster-1 discovery) whether the v61 image is Themida-packed or pre-Themida, and record it in `context.md`/`acceptance.md` — it changes patch-site-validity assumptions and the smoke-test fault expectations (a pre-Themida client will not throw the Themida integrity faults the README documents).

---

## Reading order before starting

Read these task-folder docs — they hold data this plan references rather than duplicates:

- `prd.md` — requirements (FR-1…FR-16), §7 key-class table, acceptance criteria §10.
- `design.md` — the stacked-branch baseline (§0, D0), orchestration (§3), the two-anchor evidence rule (§4, D1/D6), the **three-tier below-floor gate audit** (§5, Categories A–D), the split convention (§5.5, D8), the gate-rewrite strategy (§5.9), the IDB topology incl. the **v48 distractor** (§2), packing determination (§6, D9).
- `memory-map.md` — the per-key resolution method, the seeding rule (from v72), the backward-sentinel table, the anchor-IDB table.
- `risks.md` — R1…R13.
- `context.md` — the one-page orientation / gate-direction cheat-sheet + the pinned task-009 baseline SHA.
- `docs/tasks/task-009-gms-v72-support/` (in the `.claude/worktrees/task-009-gms-v72-support` worktree, or merged) — the **direct methodological precedent and consumed artifact**: its `plan.md` (this plan mirrors its structure), its `signature-catalog.md` (the v72 catalog this port consumes — record only the v61 delta/drift), its `struct-verification.md` (the v72 sizes to re-confirm against v61), its labeled v72/v79 IDBs, and its `v72_1.cmake`/`v79_1.cmake` maps.

---

## IDB lane discipline (applies to EVERY IDA-touching step)

Reference IDBs are loaded (design §2). **Ports are NOT hardcoded — they are reassigned between sessions.** Resolve the port for a version by binary name, every session, via `mcp__ida-pro__list_instances`, then confirm with `get_metadata`:

| Version | Binary | Role |
|---|---|---|
| GMS v61.1 | `GMS_v61.1_U_DEVM.exe` | **Target** — locate, label, `idb_save`. **Sparse/unverified** — confirm identity every probe; existing labels minimal/untrusted. |
| GMS v72.1 | `GMS_v72.1_U_DEVM.exe` | **Closest anchor (gap 11)** — task-009-labeled; seed source + below-floor template. **ABOVE v61.** |
| GMS v79.1 | `GMS_v79_1_DEVM.exe` | **Secondary below-floor x-check (gap 18)** — task-008-labeled; intermediate in the chain trace. **ABOVE v61.** |
| GMS v83 | `MapleStory_dump.exe` | **Primary/canonical anchor (gap 22)** — canonical names/prototypes come from here. **ABOVE v61.** |
| GMS v84/v87 | `GMS_v84.1_U_DEVM.exe` / `GMSv87_4GB.exe` | Upper anchors / upper-gate cross-validation (FR-12c, FR-13). |
| GMS v95 (PDB) | `GMS_v95.0_U_DEVM.exe` | PDB-derived reference / upper-gate cross-validation. |
| JMS v185 | (JMS185 binary) | JMS-only sentinel confirmation / JMS-branch FR-13 check. |
| **GMS v48.1** | `GMS_v48_1_DEVM.exe` | **NOT an anchor — unlabeled distractor.** Loaded only; never triangulate against it. Its presence is why `get_metadata` is mandatory before every probe. |

Every probe follows this sequence (R8/R9, [[feedback-verify-ida-target]]):
1. `mcp__ida-pro__list_instances` → find the port whose binary name matches the version you intend.
2. `mcp__ida-pro__select_instance(port=<resolved>)`.
3. `mcp__ida-pro__get_metadata` (or `server_health`) — confirm the routed IDB is the one you intend **before** drawing any conclusion. For v61, confirm it is the expected `GMS_v61.1_U_DEVM` binary and **explicitly distinguish it from the v48 distractor** (do not trust a sparse IDB's identity).
4. Probe.
5. After **any** version switch, repeat 1–3. Never infer the active IDB from "what I selected last."

There is **no labeled IDB below v61** — every v61 verdict rests on a v61 signature, never on proximity (R1). Never run two IDA-probing tasks concurrently — `select_instance` routing is shared global state (design §D3-note). subagent-driven-development runs one task at a time, which satisfies this; do not parallelize IDA tasks.

---

## Standard procedures (referenced by name from the cluster tasks)

Defined once here, invoked by name from Tasks 2 and 4–10. Follow them literally per key.

### SP-1 — Resolve one absolute-address key

For key `K` whose v72 seed address is `A72` (from `v72_1.cmake`) and v83 canonical name is `N83`:

1. **Characterize in v72** (lane discipline → v72 binary). Probe `A72`:
   - `mcp__ida-pro__decompile(A72)` and `mcp__ida-pro__disasm(A72)` to read the function (it is already labeled by task-009).
   - Record its distinctive anchors: referenced string literals, called imports (`connect`, `socket`, registry/file APIs), magic constants / opcode immediates, vtable slot, call-graph neighbors. Consult task-009's catalog row for `K`.
2. **Confirm the canonical name in v83** (lane discipline → v83 binary) — read the v83 function at its `v83_1.cmake` value to lock the exact prototype/name `N83`.
3. **Locate in v61** (lane discipline → v61 binary; confirm identity first — sparse/unverified, **not v48**) using anchors in this preference order (design §4):
   1. **String xref** — `search_text` / `find_regex` for the exact literal, then `xrefs_to` the string to reach the referencing function. *Older build: the v72/v83 literal may have drifted — see step 7.*
   2. **Import/API call anchor** — `imports` / `imports_query` for the API, then `xrefs_to` it.
   3. **Call-graph anchor** — child/parent of an already-resolved v61 function (`callees` / `xrefs_to`).
   4. **Constant / opcode immediate** — `find_bytes` / `find_regex` / `insn_query` for the immediate.
   5. **Vtable slot** — find the class vtable via its RTTI/type string, index the slot.
   6. **Byte/structure signature** (`make_signature_for_function(A72)` then `find_bytes` in v61) — **tiebreaker only for v61; never counts as an anchor** (D6).
4. **Apply the anchor rule (design §4):**
   - **Baseline (ordinary keys):** at least **two independent anchors** both resolve to the same v61 address. "Independent" = two different kinds, or two clearly distinct instances of the same kind. **At least one must be kind 1–5** — a pair of byte-sigs is invalid.
   - **High-value / high-blast tier (D6)** — socket send/flush/process, the COutPacket encoders, WinMain, `CWvsApp::Run`/`SetUp`, the packet senders, and any function adjacent to a below-floor gate (§5): require **two anchors of kinds 1–5** (two *structural* anchors of two different kinds) **PLUS a confirmed v72→v83 chain trace** — the same function identified in v72 *and* v83 (using v79 as the intermediate where it disambiguates) with both anchors holding, so the v61 relocation is corroborated at the neighbor versions rather than pattern-matched into v61 in isolation (the ~22-version v61↔v83 gap makes a single-version byte match unsafe). A byte-sig may only *disambiguate*; it does not count toward the pair. Flag the key `needs-main-review`.
5. **Disambiguate** if more than one v61 candidate matches: choose by a unique callee or a surrounding string the real one references — **never** by address proximity to v72 alone. Record the ambiguity and the reason for the choice.
6. **Label the v61 IDB**: `mcp__ida-pro__rename(addr=<v61>, name=<canonical v83 name N83 verbatim>)`, and `mcp__ida-pro__set_type(...)` where the prototype is known. (Labeling functions/globals only — do NOT apply struct types.)
7. **Capture v61-vs-v72 drift (FR-14).** If the working anchor differs from the v72 reference (a literal, a constant, a vtable slot index that shifted in the older build), record the **v61-specific form** in the catalog row, plus an explicit "ported directly / drifted" note, so the next (older) backward port benefits and the catalog captures which task-009 heuristics survived one more tier down.
8. **Write the cmake line** (SP-3) and **the catalog row** (SP-4), and flip the key's status in `memory-map.md` from `☐` to `✔` (`◐` if located+labeled but catalog/cmake not yet written).

### SP-2 — Resolve one instruction-relative offset key

Offset keys (`*_OFFSET`, `WIN_MAIN_AD_BALLOON_CONDITIONAL`, `WIN_MAIN_PATCHER_OFFSET`, `C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET`) are **not** addresses and are **never copied from v72 or v83** (R2, FR-5):

1. Identify the offset's host function in v61 (usually already resolved as an absolute key in the same cluster).
2. `mcp__ida-pro__disasm` the host function in v61.
3. Find the target instruction/branch the offset points to. Read the **v72** host at the corresponding v72 offset first, to know *what* you are measuring (the call site, conditional, or patch point the edit patches — cross-reference the consuming edit's source).
4. Compute the byte delta from the host function's base (or the documented reference instruction) to that target instruction in v61. That delta is the v61 offset value.
5. Record in the catalog (SP-4) the identified target instruction plus the measured delta. Write the cmake line (SP-3) and flip status. **Where the host *feature* is era-absent in v61 (ad balloon, patcher), the offset key becomes a sentinel — resolve per SP-5 backward.**

### SP-3 — cmake line format

Edit `memory_maps/GMS/v61_1.cmake` in place (seeded from v72 in Task 1, so the line already exists with the v72 value). Replace the value, preserving key name and any still-accurate trailing comment:

```cmake
set(C_CLIENT_SOCKET_SEND_PACKET 0x00XXXXXX)
```

- Use uppercase `0x` hex matching the file's existing style.
- Where a v72 key carried a clarifying comment (`# does not exist`, `# JMS only`), update it to reflect the **v61** finding (FR-8). A carried-forward sentinel keeps an accurate `# …` note.

### SP-4 — signature-catalog row

Append/update one section per resolved function in `signature-catalog.md`, using the file's existing schema (the entry template at the top of that file — create the file from task-009's template if it does not yet exist):

```
### <CanonicalName> (key: <KEY>)
- v61 address: 0x________
- v72 address (task-009): 0x________
- Heuristic: <string xref "…" / import anchor / call-graph child of … / push <imm> / byte sig>
- Drift v72→v61: <direct | string changed to "…" | inlined differently | split | not found, see note>
- Chain trace (high-value only): <v72 0xA → v83 0xB confirmed same fn (both anchors hold) | n/a>
- Label applied: <yes/no> (rename + set_type if proto known)
- Notes: <ambiguity, disambiguation reason, anything that helps the next older port>
```

Phrase entries about the *function*, not a specific address (the catalog is version-agnostic and reused for the next, older port — FR-14). Where a v72 anchor drifted in v61, record the v61-specific form (SP-1 step 7) and an explicit "ported directly / drifted" note.

### SP-5 — Sentinel confirmation, both directions (FR-7, R7)

For a sentinel key (v72 value `0x00000000`, or a "does not exist" key), absence is **confirmed, not assumed**:

1. Identify the feature's own anchor (its strings, its `CreateInstance` call site, its vtable) from a version where it exists (v83 for GMS-absent features; JMS185 for JMS-only features — confirm the positive case there first).
2. Search v61 for that anchor.
3. If **absent** → carry `0x00000000` forward with an accurate `# …` comment and record "confirmed absent + how" in the catalog.
4. If **present** → resolve as a real address per SP-1, note the surprise in the catalog, and (if it backs a sentinel-gated edit) flag it for the user.

**Backward direction (new v61-only sentinels).** Additionally, for each v72 *real-address* key, stay alert: if the backing feature does not exist in v61, that key becomes a **new v61-only sentinel**. Carry `0x00000000` with a `# absent in v61` comment, and **flag it for the gate/edit owner** in the task report (the consuming gate/edit must tolerate `0`). Candidates to watch (design §4): **ad balloon, patcher window, MTS map restriction, beginner-party block, the CFileStream relay helpers, MTS**. Start each disposition from task-009's **v72** verdict — not the stale v83 comments — then confirm in v61. Do not silently zero a previously-real key without flagging.

### SP-6 — `idb_save` checkpoint

After each cluster task's labeling is complete, `mcp__ida-pro__idb_save` on the v61 IDB (after confirming target via `get_metadata` — **not v48**) so labels survive an MCP swap/restart (FR-10).

---

## Task 1: Setup — worktree off task-009, seed cmake from v72, reconcile the live key set, baseline IDB

**Files:**
- Create: `memory_maps/GMS/v61_1.cmake`
- Modify: `docs/tasks/task-010-gms-v61-support/memory-map.md`, `context.md` (record the pinned baseline SHA)

- [ ] **Step 1: Create the feature branch + worktree off the task-009 tip (D0/§0)**

```bash
# from the main checkout
git worktree add -b task-010-gms-v61-support .claude/worktrees/task-010-gms-v61-support task-009-gms-v72-support
cd .claude/worktrees/task-010-gms-v61-support
git rev-parse task-009-gms-v72-support   # record this SHA
```

Record the printed task-009 tip SHA in `context.md` and (later) the PR body so "the baseline" is an exact commit (R13). Never work on `main` ([[feedback-no-main-commits]]). All subsequent steps run inside this worktree.

- [ ] **Step 2: Seed the v61 memory map from v72**

Copy `memory_maps/GMS/v72_1.cmake` to `memory_maps/GMS/v61_1.cmake` verbatim. v72 (not v83) is the seed because it already carries the below-floor relocations (the reduced-struct clusters) and the finalized sentinel dispositions (e.g. the CFileStream relay disposition). This guarantees all keys are present and non-empty from the start. Prepend a header comment to the new file:

```cmake
# GMS v61.1 memory map. Seeded from v72_1.cmake (closest labeled anchor, +11 versions);
# every value is relocated and re-verified against the v61 binary per
# docs/tasks/task-010-gms-v61-support/. v61 sits TWO tiers below the original floor —
# below v72 (task-009), below v79 (task-008), below the v83 base. A value still equal
# to v72 means UNVERIFIED unless its tracking-table row in memory-map.md is marked ✔
# with a signature-catalog entry. There is NO labeled IDB below v61 to triangulate
# against, and the base branch is sometimes the already-twice-diverged v72-reduced
# branch — confirm every value by a v61 signature, never by proximity. (A v48 IDB is
# also loaded; it is NOT an anchor.)
```

- [ ] **Step 3: Reconcile the live key set (pin the count)**

The validator `cmake/CheckMemoryMapKeys.cmake` already exists. Confirm the live key count and that the seed satisfies it:

```bash
grep -oE '@[A-Z_0-9]+@' include/memory_map.h.in | sed 's/@//g' | sort -u | grep -v '^BUILD_REGION$' | wc -l
cmake -DREGION=GMS -DMAJOR=61 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake
```

Expected: the `wc -l` prints `159`, and the validator prints `OK: all 159 keys defined and non-empty for GMS v61.1`.
**If the count is not 159**, stop — reconcile `memory-map.md`'s tracking table against the live header before proceeding (a key was added/removed since this plan was written). Record the confirmed count in the task report.

- [ ] **Step 4: Generate the 159-row tracking table from the v72 seed**

In `memory-map.md`, generate the per-key table now from `v61_1.cmake` (== v72 values at this point): one row per key with columns **Key · v72 value · v61 value · status · signature ref**, status `☐` for all. Group rows in the order they appear in `include/memory_map.h.in`. This table is the completeness ledger backing FR-3/FR-4/FR-5. Legend: `☐ todo · ◐ located+labeled · ✔ written+catalogued`.

- [ ] **Step 5: Baseline the v61 IDB (and determine packing — D9 kickoff)**

Confirm the target IDB is reachable and is actually v61 (lane discipline):
- `mcp__ida-pro__list_instances` — find the port whose binary is `GMS_v61.1_U_DEVM.exe`. **Note the v48 port too, so you never confuse the two.**
- `mcp__ida-pro__select_instance(port=<resolved>)` then `mcp__ida-pro__get_metadata` — confirm identity (sparse/unverified IDB — verify it is the expected retail v61 binary, **not v48**); record the image base + module name in `signature-catalog.md` notes (anchors WinMain offset math later).
- **Packing (D9):** note entry-point section names / import-table state / Themida-stub presence and record a preliminary packing verdict (pre-Themida vs packed) in `context.md`; the full determination completes in Task 2 (Cluster 1).
- `mcp__ida-pro__idb_save` to take a clean baseline.

- [ ] **Step 6: Commit**

```bash
git add memory_maps/GMS/v61_1.cmake docs/tasks/task-010-gms-v61-support/
git commit -m "feat(v61): scaffold GMS v61.1 memory map seeded from v72"
```

---

## Task 2: Memory map — WinMain + CWvsApp + window-manager cluster (~26 keys)

Resolve first: the entry point anchors the whole image and gives call-graph reach into the rest (README "Adding a new version"; design §3.3 cluster 1). This cluster also produces the disassembly the **early Category-A amendment** (Task 3) consumes, so it additionally locates the `CFuncKeyMappedMan` class/vtable far enough to size it, surfaces the `C_WVS_APP_SET_UP` / DR_init init-sequence question (R11), and **completes the packing determination (D9)**.

**Files:** `memory_maps/GMS/v61_1.cmake`, `docs/tasks/task-010-gms-v61-support/signature-catalog.md`, `memory-map.md`, `context.md` (packing finding).

**Keys (absolute unless noted):** `WIN_MAIN`, `SEND_HS_LOG`, `C_WVS_APP`, `C_WVS_APP_INSTANCE_ADDR`, `C_WVS_APP_IS_MSG_PROC`, `C_WVS_APP_INITIALIZE_AUTH`, `C_WVS_APP_INITIALIZE_PCOM`, `C_WVS_APP_CREATE_MAIN_WINDOW`, `C_WVS_APP_CONNECT_LOGIN`, `C_WVS_APP_INITIALIZE_RES_MAN`, `C_WVS_APP_INITIALIZE_GR2D`, `C_WVS_APP_INITIALIZE_INPUT`, `C_WVS_APP_INITIALIZE_SOUND`, `C_WVS_APP_INITIALIZE_GAME_DATA`, `C_WVS_APP_CREATE_WND_MANAGER`, `C_WVS_APP_GET_CMD_LINE`, `C_WVS_APP_DIR_BACK_SLASH_TO_SLASH`, `C_WVS_APP_DIR_UP_DIR`, `C_WVS_APP_DIR_SLASH_TO_BACK_SLASH`, `C_WVS_APP_GET_EXCEPTION_FILE_NAME`, `C_WVS_APP_CALL_UPDATE`, `C_WVS_APP_RUN`, `C_WVS_APP_SET_UP`, `C_WND_MAN_S_UPDATE`, `C_WND_MAN_REDRAW_INVALIDATED_WINDOWS`, `G_DW_TARGET_OS` (global).
**Offset keys (SP-2):** `WIN_MAIN_AD_BALLOON_CONDITIONAL`, `WIN_MAIN_PATCHER_OFFSET` — both measured inside v61 `WIN_MAIN` (read the v72 seed values only to know *what* to measure; never copy them). **Era-watch (R7):** if v61 predates the ad-balloon and/or patcher features, these become v61 sentinels (SP-5 backward) and the consuming `no-ad-balloon`/`no-patcher` edits must tolerate `0`.

> Confirm the exact key membership of this cluster against the generated tracking table (Task 1 Step 4) — the list above mirrors task-009's cluster 1. Any key the table assigns here that is missing above is still this task's responsibility.

- [ ] **Step 1: Anchor WIN_MAIN from the PE entry point + finish packing determination (D9)**

`WIN_MAIN` is reachable from the PE entry. In v61 (lane discipline) get the entry point (`get_metadata` reports it; or `list_funcs`/`survey_binary`). **Record the final packing verdict here** (D9): is the entry point a normal compiler stub reaching `WinMain`, or a Themida stub? A clean, walkable entry → pre-Themida (record as a first-class finding; smoke-test fault expectations adjust per §6). Confirm `WIN_MAIN` via SP-1 using two structural anchors — e.g. the unique startup string literals it references (string xref) **and** its call to `CWvsApp::SetUp`/`Run` (call-graph) — plus the v72→v83 chain trace (high-value tier, D6). Flag `needs-main-review`. The task-009 v72 catalog's WinMain entry should transfer; if the literal drifted in v61, record the v61 form (SP-1 step 7).

- [ ] **Step 2: Resolve C_WVS_APP, C_WVS_APP_RUN, C_WVS_APP_SET_UP (high-value)**

Per SP-1. `C_WVS_APP_RUN` is the main message-pump loop; `C_WVS_APP_SET_UP` is the init driver that calls the `INITIALIZE_*` subsystem functions in a fixed order. Resolve `SET_UP` first, then walk its `callees` to reach the initializers (call-graph anchor for the rest of the cluster). All three are high-value → two structural anchor kinds + v72→v83 chain trace + `needs-main-review`.

**R11 — DR_init / SetUp init sequence:** while reading `SET_UP`'s `callees`, record the init-call order and note whether any DR/anti-debug init step is present. v61 (older than v72/v79/v83) is expected to **lack** the DR subsystem (the v84 freeze, [[project_v84_movement_anticheat_freeze]], came from a DR_init that v72/v79/v83 do not have) — but confirm absence from this disasm; do not assume identical to v72. This feeds the `DR_INIT`/`DR_CHECK` sentinel confirmation in Task 10.

- [ ] **Step 3: Resolve the remaining CWvsApp keys via call-graph from SET_UP/Run**

For each `C_WVS_APP_INITIALIZE_*`, `CREATE_*`, `GET_CMD_LINE`, the three `DIR_*` helpers, `GET_EXCEPTION_FILE_NAME`, `CALL_UPDATE`, `IS_MSG_PROC`, `CONNECT_LOGIN`: apply SP-1, using the call-graph from `SET_UP`/`Run` as anchor 1 and each function's own string/import/constant as anchor 2. The `DIR_*` helpers reference path-separator chars; `INITIALIZE_AUTH/PCOM/RES_MAN/SOUND/GAME_DATA` each reference distinctive subsystem strings.

- [ ] **Step 4: Resolve the global + window-manager keys + SEND_HS_LOG**

- `C_WVS_APP_INSTANCE_ADDR`, `G_DW_TARGET_OS`: globals — find the `mov [g_xxx], eax` store after the relevant ctor/getter returns. Anchor via the function that writes/reads them.
- `C_WND_MAN_S_UPDATE`, `C_WND_MAN_REDRAW_INVALIDATED_WINDOWS`: reachable from the app update loop; SP-1 with call-graph + a distinctive string/constant.
- `SEND_HS_LOG`: anchor via the `%s\HShield` literal (task-009 catalog) — a string xref; confirm the literal is present/identical in v61 or record drift. **If HShield/anti-cheat logging does not exist this far back, treat as a v61 sentinel (SP-5 backward).**

- [ ] **Step 5: Locate CFuncKeyMappedMan class + size it (feeds Task 3)**

So Task 3 can amend `CFuncKeyMappedMan.h:52` early, locate the `CFuncKeyMappedMan` class in v61 now: find its RTTI/type string (`search_text` for `CFuncKeyMappedMan`/`.?AVCFuncKeyMappedMan@@`) → its vtable (`C_FUNC_KEY_MAPPED_MAN_VFTABLE` candidate). Determine the **v61 size** of `CFuncKeyMappedMan` from a layout anchor (the allocation immediate at its `CreateInstance` call, a ctor field-init extent, or a `RemoveAll`-style `imul stride,N`). The v72/v79 reduced size is `0x388` (quickslot pair gated out at `:19`) — record the **v61** measured size and compare to v72 explicitly (this is a Category-B header — feeds Task 3 *and* Task 16). Do **not** apply a struct type to the IDB (read-only). Full resolution of the FuncKeyMapped key set happens in Task 7.

- [ ] **Step 6: Re-derive the two WinMain offsets (SP-2)**

`WIN_MAIN_AD_BALLOON_CONDITIONAL` and `WIN_MAIN_PATCHER_OFFSET` are byte offsets inside `WIN_MAIN`. Disassemble v61 `WIN_MAIN`, find the ad-balloon conditional branch and the patcher-window call site (read v72 `WIN_MAIN` at its offsets first; cross-reference the `no-ad-balloon` and `no-patcher` edits for the exact patch semantics), and measure the v61 deltas. **Never copy the v72 offsets.** If either feature is absent in v61 (era-watch, R7), carry the offset key as a sentinel per SP-5 backward and flag the consuming edit.

- [ ] **Step 7: Spot-check the high-value keys independently**

For every key flagged `needs-main-review` in this cluster, re-probe the v61 address from scratch with a *different* anchor than the one originally used, and confirm the v72→v83 chain trace independently (R1, design §3.2).

- [ ] **Step 8: Label, checkpoint, validate, commit**

- Label all resolved functions/globals in the v61 IDB (SP-1 step 6).
- `idb_save` (SP-6).
- Run: `cmake -DREGION=GMS -DMAJOR=61 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake` → expect `OK: all 159 keys…`.
- Flip this cluster's rows to ✔ in `memory-map.md`.

```bash
git add memory_maps/GMS/v61_1.cmake docs/tasks/task-010-gms-v61-support/
git commit -m "feat(v61): relocate WinMain + CWvsApp cluster with signatures"
```

---

## Task 3: Early Category-A gate amendment — restore size guards for v61 (FR-12a, D7, R4)

The enumerated gates with **no catch-all `#else`** are false for v61, so v61 selects no branch (its `assert_size` guard silently does not fire) or drops a member. Amend each so v61 selects a defined, measured branch — **early**, while the CWvsApp/CFuncKeyMappedMan disasm from Task 2 is fresh (D7), so the v61 build becomes compilable before the full audit.

**Live sites (verified against the task-009 baseline at plan time — re-grep, line numbers may shift):**
- `common/CWvsApp.h:98` — `(== 72 || == 79 || == 83 || == 84)` asserts `0x60`; `:100 == 87`; then `>= 95`. **No v61 branch.**
- `common/CFuncKeyMappedMan.h:52` — `(== 72 || == 79)` (the `0x388` reduced assert); `:54 (== 83 || == 84 || == 87)`; `:56 == 95`; `:58 == 111`. **No v61 branch.** (Member gate `:19` `>= 83 || JMS` already excludes v61 — quickslot pair absent for v61 unless Task 16/Step 5 proves otherwise.)
- `common/CLogin.h:235` — `== 83` member-declaring gate (`unk3[5]`). **False for v61** (excluded, like v72/v79) — confirm v61 genuinely lacks the member rather than silently dropping one it has.

**Files:** `common/CFuncKeyMappedMan.h`, `common/CWvsApp.h`, `common/CLogin.h`, `docs/tasks/task-010-gms-v61-support/struct-verification.md`.

- [ ] **Step 1: Re-grep the live enumerated-no-`#else` gate set**

```bash
grep -rn "BUILD_MAJOR_VERSION ==" common/*.h
```

Confirm the sites above and inspect any other hit. Classify each: an `assert_size` chain (silent guard loss for v61 — amend here) vs a member-declaring `#if … #endif` (layout shift — `CLogin.h:235` is one; confirm in Step 3). The upper `== 95`/`== 87` hits (`CConfig.h:84`, `CLogo.h:93`, `SecondaryStat.h:405/419`) are false for v61 and resolve onto the base branch — those are Category C (Tasks 13–15), not amended here.

- [ ] **Step 2: Decide v61's CWvsApp + CFuncKeyMappedMan branch from measured size (not "oldest ⇒ v83")**

- `CWvsApp` — Task 2 Step 2 resolved `C_WVS_APP`; measure v61 `sizeof(CWvsApp)` from a layout anchor (the SetUp/ctor field-init extent, or the stack-frame size where WinMain stack-constructs it — v72/v79/v83/v84 = `0x60`). Record the v61 size.
- `CFuncKeyMappedMan` — use the v61 size measured in Task 2 Step 5 (v72/v79 = `0x388`).

For each, the v61 branch is the one whose asserted size **equals the measured v61 size**. If v61 matches the existing branch (e.g. v61 `CWvsApp` == `0x60`, or v61 `CFuncKeyMappedMan` == `0x388`), add `61` to that branch (`== 61 || == 72 || == 79 …`). If v61's size differs from every existing branch, add a **new** `#elif … == 61` branch with the measured size. Record the deciding evidence (disasm line + measured size) in `struct-verification.md` rows `CWvsApp.h` / `CFuncKeyMappedMan.h` with verdict `branch-added` and the v61-vs-v72 comparison. (A5: this branch assignment is an evidence verdict, not "oldest ⇒ v83".)

- [ ] **Step 3: Confirm CLogin `unk3[5]` disposition for v61**

`CLogin.h:235` (`== 83`) declares `int unk3[5]` only for v83; false for v61. Disassemble v61 `CLogin::CLogin` (or a sizeof-bearing method) and determine whether v61 has the member at the gated offset:
- If **v61 lacks it** (expected, like v72/v79) → exclusion is correct; record verdict `unchanged` with the deciding disasm line. **No edit.**
- If **v61 has it** → the gate becomes `(== 83 || == 61)` (or `<= 83`); record `rewritten` and apply in this task (it is a no-`#else` member gate, so leaving v61 out silently drops a real member — fix early).

- [ ] **Step 4: Apply the amendments (keep the house guard form)**

Example — if v61 `CWvsApp` measures `0x60` (matches v72/79/83/84):

```c
#if defined(REGION_GMS) && (BUILD_MAJOR_VERSION == 61 || BUILD_MAJOR_VERSION == 72 || BUILD_MAJOR_VERSION == 79 || BUILD_MAJOR_VERSION == 83 || BUILD_MAJOR_VERSION == 84)
    assert_size(sizeof(CWvsApp), 0x60)
```

Or — if v61 `CWvsApp` measures a distinct size `0xNN`:

```c
#if defined(REGION_GMS) && (BUILD_MAJOR_VERSION == 72 || BUILD_MAJOR_VERSION == 79 || BUILD_MAJOR_VERSION == 83 || BUILD_MAJOR_VERSION == 84)
    assert_size(sizeof(CWvsApp), 0x60)
#elif defined(REGION_GMS) && BUILD_MAJOR_VERSION == 61
    assert_size(sizeof(CWvsApp), 0xNN)
```

Apply the analogous change at `CFuncKeyMappedMan.h:52` (add `61` to the matching size-assert branch, or a new `== 61` branch with the measured size). Add a one-line comment at each amended site, e.g. `// v61 size verified task-010`. Use the existing `#if/#elif/#endif` block form; no ternaries.

- [ ] **Step 5: Cross-version truth-table + prove v61 compiles**

- **Truth table (FR-13):** adding `61` to a branch (or adding a new `== 61` branch) must not change any other version's selected branch. Confirm `72/79/83/84/87/95/111/JMS185` each still hit their original `assert_size`. Record the table in `struct-verification.md`.
- **Empirical build/preprocess check:** confirm the intended `assert_size` now fires for v61. Prefer `scripts/wsl-build.sh GMS 61 1` ([[project_wsl_cross_compile]]); if a full build is too slow at this stage, at minimum syntax-check each amended header for `{61, 72}` and confirm the intended assert is selected:

```bash
echo '#include "CWvsApp.h"' > "$CLAUDE_JOB_DIR/tmp/_gatecheck.cpp"
gcc -fsyntax-only -DREGION_GMS -DBUILD_MAJOR_VERSION=61 -DBUILD_MINOR_VERSION=1 -I common -I include "$CLAUDE_JOB_DIR/tmp/_gatecheck.cpp" 2>&1 | head
```

Record in the task report whether the v61 build was genuinely blocked or merely missing size guards (the expected finding). State what was actually run ([[verification-before-completion]]).

- [ ] **Step 6: Commit**

```bash
git add common/CWvsApp.h common/CFuncKeyMappedMan.h common/CLogin.h docs/tasks/task-010-gms-v61-support/struct-verification.md
git commit -m "fix(v61): add v61 branch to CWvsApp/CFuncKeyMappedMan size gates + confirm CLogin member (Cat A)"
```

---

## Task 4: Memory map — CClientSocket / ZSocket cluster (~14 keys)

Highest-value for the `redirect`/`bypass` edits (design §3.3 cluster 2).

**Files:** `memory_maps/GMS/v61_1.cmake`, `signature-catalog.md`, `memory-map.md`.

**Keys:** `C_CLIENT_SOCKET_INSTANCE_ADDR`, `C_CLIENT_SOCKET_CREATE_INSTANCE`, `C_CLIENT_SOCKET_SEND_PACKET`, `C_CLIENT_SOCKET_FLUSH`, `C_CLIENT_SOCKET_MANIPULATE_PACKET`, `C_CLIENT_SOCKET_PROCESS_PACKET`, `C_CLIENT_SOCKET_CLOSE`, `C_CLIENT_SOCKET_CLEAR_SEND_RECEIVE_CTX`, `C_CLIENT_SOCKET_ON_CONNECT`, `C_CLIENT_SOCKET_CONNECT_LOGIN`, `C_CLIENT_SOCKET_CONNECT_CTX`, `C_CLIENT_SOCKET_CONNECT_ADR`, `Z_SOCKET_BASE_CLOSE_SOCKET`, `Z_SOCKET_BUFFER_ALLOC`.

- [ ] **Step 1: Anchor the connect path via the `connect`/`socket` imports**

In v61 (lane discipline), `imports_query` for `connect` / `socket` (WS2_32). `xrefs_to` them reach `C_CLIENT_SOCKET_ON_CONNECT` / `CONNECT_ADR` / `CONNECT_CTX` / `CONNECT_LOGIN`. Import anchor (kind 2). Disambiguate the four connect variants by their distinct callers/strings.

- [ ] **Step 2: Resolve send/flush/process/manipulate (high-value)**

`SEND_PACKET`, `FLUSH`, `PROCESS_PACKET`, `MANIPULATE_PACKET` are high-value → SP-1 with two structural anchor kinds each + v72→v83 chain trace → `needs-main-review`. Anchor via call-graph from `CClientSocket` getInstance / the connect path + each function's distinctive structure (e.g. flush references the send buffer). Use call-graph adjacency in v61, **not** v72/v83 address arithmetic.

- [ ] **Step 3: Note for the §5.6 audit — does ON_CONNECT encode an 8-byte client key?**

While reading `C_CLIENT_SOCKET_ON_CONNECT` / the connect-hello send path, record whether v61 encodes an **8-byte client key** (`EncodeBuffer(m_aClientKey, 8)`). This is the same fact as the `CWvsContext.h:98` `> 83` gate (Task 13 special case) and `bypass/socket_hooks.cpp:310` (`> 83`). The gate predicts v61 (< 83) has the **no-key** form (v72 confirmed absent — [[project_v84_clientkey_gate_trap]]); capture the disasm evidence here so Task 13 can confirm it without re-deriving. Verify decode order against the v61 binary, not a server round-trip.

- [ ] **Step 4: Resolve the remaining socket keys**

`CLOSE`, `CLEAR_SEND_RECEIVE_CTX`, `CREATE_INSTANCE`, `INSTANCE_ADDR` (global store after CreateInstance), `Z_SOCKET_BASE_CLOSE_SOCKET`, `Z_SOCKET_BUFFER_ALLOC` — SP-1, two anchors each.

- [ ] **Step 5: Spot-check `needs-main-review`, label, checkpoint, validate, commit**

Same as Task 2 Steps 7–8.

```bash
git commit -am "feat(v61): relocate CClientSocket/ZSocket cluster with signatures"
```

---

## Task 5: Memory map — COutPacket cluster (~7 keys)

Encode primitives used to craft packets; high-value (design §3.3 cluster 3).

**Files:** `memory_maps/GMS/v61_1.cmake`, `signature-catalog.md`, `memory-map.md`.

**Keys:** `C_OUT_PACKET`, `C_OUT_PACKET_ENCODE_1`, `C_OUT_PACKET_ENCODE_2`, `C_OUT_PACKET_ENCODE_4`, `C_OUT_PACKET_ENCODE_STR`, `C_OUT_PACKET_ENCODE_BUFFER`, `C_OUT_PACKET_MAKE_BUFFER_LIST`.

- [ ] **Step 1: Resolve the encoders by width signature + call-graph**

`ENCODE_1/2/4` differ by the byte-width they append (1/2/4-byte store/grow). SP-1: anchor 1 = the width-specific store/grow pattern (constant/structure), anchor 2 = called-by from a known packet sender or the shared buffer-grow callee. All high-value → two structural anchors + v72→v83 chain trace + `needs-main-review`.

- [ ] **Step 2: Resolve ENCODE_STR, ENCODE_BUFFER, MAKE_BUFFER_LIST, C_OUT_PACKET**

`ENCODE_STR` references string-length handling; `ENCODE_BUFFER` takes a length arg and memcpy-grows; `MAKE_BUFFER_LIST` and `C_OUT_PACKET` (ctor/base) anchor via call-graph among the COutPacket methods + a distinctive constant/string. SP-1, two anchors each.

- [ ] **Step 3: Spot-check, label, checkpoint, validate, commit**

Same as Task 2 Steps 7–8.

```bash
git commit -am "feat(v61): relocate COutPacket cluster with signatures"
```

---

## Task 6: Memory map — Login / Stage / Logo / Title cluster (~19 keys)

Login & stage flow (design §3.3 cluster 4).

**Files:** `memory_maps/GMS/v61_1.cmake`, `signature-catalog.md`, `memory-map.md`.

**Keys:** `C_LOGIN_UPDATE`, `C_LOGIN_SEND_CHECK_PASSWORD_PACKET`, `C_LOGO`, `C_LOGO_GET_RTTI`, `C_LOGO_IS_KIND_OF`, `C_LOGO_UPDATE`, `C_LOGO_ON_MOUSE_BUTTON`, `C_LOGO_ON_SET_FOCUS`, `C_LOGO_ON_KEY`, `C_LOGO_LOGO_END`, `C_LOGO_FORCED_END`, `C_LOGO_INIT`, `C_LOGO_INIT_NX_LOGO`, `STAGE_INSTANCE_ADDR`, `SET_STAGE`, `C_STAGE_ON_MOUSE_ENTER`, `C_STAGE_ON_PACKET`, `GR_INSTANCE_ADDR`, `C_UI_TITLE_INSTANCE_ADDR`.

- [ ] **Step 1: Resolve CLogo via its RTTI/vtable**

`C_LOGO_GET_RTTI`, `C_LOGO_IS_KIND_OF`, and the `C_LOGO` vtable cluster: find the `CLogo` RTTI/type string in v61 (`search_text` for `CLogo`/`.?AVCLogo@@`), reach its vtable, index the slots for `GetRTTI`/`IsKindOf`/`OnSetFocus`/`OnKey`/`OnMouseButton`/`Update`. Vtable slot is kind 5; confirm each with a second anchor (the method body's distinctive content). Match **slot order** against v72's vtable, not v72 addresses — and note any slot-index drift in v61 (SP-1 step 7).

- [ ] **Step 2: Resolve CLogo lifecycle + NX logo**

`C_LOGO_INIT`, `C_LOGO_INIT_NX_LOGO`, `C_LOGO_LOGO_END`, `C_LOGO_FORCED_END`: `INIT_NX_LOGO` references the NX-logo resource string (string xref). SP-1 two anchors each.

- [ ] **Step 2 note:** In v83 `C_LOGO_UPDATE` and `C_LOGIN_UPDATE` shared one address; task-009 recorded the v72 determination. Confirm whether they are still the same function in v61 (one shared Update) or diverged; record the determination in the catalog.

- [ ] **Step 3: Resolve Login keys**

`C_LOGIN_SEND_CHECK_PASSWORD_PACKET` references the password-check send path (opcode immediate + COutPacket use — anchor via the opcode constant and a call to a resolved encoder). `C_LOGIN_UPDATE` per Step 2 note. SP-1.

- [ ] **Step 4: Resolve Stage + globals + title**

`SET_STAGE`, `C_STAGE_ON_MOUSE_ENTER`, `C_STAGE_ON_PACKET` via CStage vtable/strings; `STAGE_INSTANCE_ADDR`, `GR_INSTANCE_ADDR`, `C_UI_TITLE_INSTANCE_ADDR` are globals (store-after-CreateInstance pattern). SP-1 two anchors each.

- [ ] **Step 5: Spot-check, label, checkpoint, validate, commit**

Same as Task 2 Steps 7–8.

```bash
git commit -am "feat(v61): relocate Login/Stage/Logo/Title cluster with signatures"
```

---

## Task 7: Memory map — Manager singletons cluster (~37 keys)

`*_CREATE_INSTANCE` / `*_INSTANCE_ADDR` pairs + their methods. These cluster in the static-init region and are called in a fixed order from one initializer — find one, the neighbors are adjacent calls (design §3.3 cluster 5). `idb_save` partway through this large cluster.

**Files:** `memory_maps/GMS/v61_1.cmake`, `signature-catalog.md`, `memory-map.md`.

**Keys:**
- ActionMan: `C_ACTION_MAN_CREATE_INSTANCE_ADDR`, `C_ACTION_MAN_INSTANCE_ADDR`, `C_ACTION_MAN_INIT`, `C_ACTION_MAN_SWEEP_CACHE`.
- `C_ANIMATION_DISPLAYER_CREATE_INSTANCE`.
- FuncKeyMapped/quickslot: `C_FUNC_KEY_MAPPED_MAN`, `C_FUNC_KEY_MAPPED_MAN_VFTABLE`, `C_FUNC_KEY_MAPPED_MAN_INSTANCE_ADDR`, `C_FUNC_KEY_MAPPED_MAN_CREATE_INSTANCE`, `DEFAULT_FKM_INSTANCE_ADDR`, `DEFAULT_QKM_INSTANCE_ADDR`, `C_QUICKSLOT_KEY_MAPPED_MAN`.
- InputSystem: `C_INPUT_SYSTEM`, `C_INPUT_SYSTEM_CREATE_INSTANCE`, `C_INPUT_SYSTEM_INSTANCE_ADDR`, `C_INPUT_SYSTEM_INIT`, `C_INPUT_SYSTEM_UPDATE_DEVICE`, `C_INPUT_SYSTEM_GET_IS_MESSAGE`, `C_INPUT_SYSTEM_GENERATE_AUTO_KEY_DOWN`, `C_INPUT_SYSTEM_SHOW_CURSOR`.
- `C_MACRO_SYS_MAN_CREATE_INSTANCE`.
- MapleTV: `C_MAPLE_TV_MAN_CREATE_INSTANCE`, `C_MAPLE_TV_MAN_INSTANCE_ADDR`, `C_MAPLE_TV_MAN_INIT`.
- MonsterBook: `C_MONSTER_BOOK_MAN_CREATE_INSTANCE`, `C_MONSTER_BOOK_MAN_INSTANCE_ADDR`, `C_MONSTER_BOOK_MAN_LOAD_BOOK`.
- Quest: `C_QUEST_MAN_CREATE_INSTANCE`, `C_QUEST_MAN_INSTANCE_ADDR`, `C_QUEST_MAN_LOAD_DEMAND`, `C_QUEST_MAN_LOAD_PARTY_QUEST_INFO`, `C_QUEST_MAN_LOAD_EXCLUSIVE`.
- Radio: `C_RADIO_MANAGER_CREATE_INSTANCE`, `C_RADIO_MANAGER_INSTANCE_ADDR`.
- Security: `C_SECURITY_CLIENT_CREATE_INSTANCE`, `C_SECURITY_CLIENT_INSTANCE_ADDR`, `C_SECURITY_CLIENT_ON_PACKET`.

- [ ] **Step 1: Find the static-init CreateInstance run and walk it**

In v61 (lane discipline), locate the static-init function that calls the `*::CreateInstance` chain (anchor from an already-resolved `CreateInstance` in Task 2's reach, or a distinctive manager string). The `CREATE_INSTANCE` keys are adjacent `call` sites; the matching `*_INSTANCE_ADDR` globals are the `mov [g_xxx], eax` stores right after each call returns. Resolve each pair together with SP-1.

> **Backward-direction watch (R7):** any manager whose feature post-dates v61 (a feature v72 *has* but v61 lacks — e.g. MapleTV, MonsterBook, MTS) will be absent here. If a `CreateInstance` chain slot is missing in v61, that key becomes a **new v61-only sentinel** — resolve per SP-5 backward (carry `0x00000000` + `# absent in v61` + flag), do not force-fit a wrong address.

- [ ] **Step 2: Resolve FuncKeyMapped/quickslot using Task 2's class anchor**

`C_FUNC_KEY_MAPPED_MAN_VFTABLE` was located in Task 2 Step 5 — finish `C_FUNC_KEY_MAPPED_MAN` (ctor/base), `_INSTANCE_ADDR`, `_CREATE_INSTANCE`, `DEFAULT_FKM_INSTANCE_ADDR`/`DEFAULT_QKM_INSTANCE_ADDR` (default-instance globals), `C_QUICKSLOT_KEY_MAPPED_MAN`. SP-1 two anchors each.

- [ ] **Step 3: Resolve manager method keys via the manager class**

For each manager's methods (`_INIT`, `_SWEEP_CACHE`, `_LOAD_BOOK`, `_LOAD_DEMAND`, `_LOAD_PARTY_QUEST_INFO`, `_LOAD_EXCLUSIVE`, `_UPDATE_DEVICE`, `_GET_IS_MESSAGE`, `_GENERATE_AUTO_KEY_DOWN`, `_SHOW_CURSOR`, `_ON_PACKET`): anchor via the resolved class instance/vtable + a distinctive string/constant (e.g. `LOAD_BOOK` references the monster-book data path string). SP-1 two anchors each.

- [ ] **Step 4: Resolve the Radio quirk carefully**

`C_RADIO_MANAGER_INSTANCE_ADDR` — prior ports found the anchor version's value pointed at the **allocator selector**, not the real instance global. Do **not** inherit the v72 address: verify the real v61 instance global (the `mov [g_xxx], eax` after `CRadioManager::CreateInstance` returns). Record the finding (and flag if the seed value is wrong) in the catalog and the task report.

- [ ] **Step 5: idb_save checkpoint mid-cluster**

After the singletons + globals are labeled (before the long method tail), `idb_save` (confirm target via `get_metadata` — not v48).

- [ ] **Step 6: Spot-check boundary-adjacent keys, label, checkpoint, validate, commit**

`C_SECURITY_CLIENT_ON_PACKET` is boundary-adjacent (security_hooks gating) → `needs-main-review`. Then Task 2 Steps 7–8.

```bash
git commit -am "feat(v61): relocate manager-singleton cluster with signatures"
```

---

## Task 8: Memory map — Config / SystemInfo / IGCipher / utilities cluster (~19 keys)

(design §3.3 cluster 6).

**Files:** `memory_maps/GMS/v61_1.cmake`, `signature-catalog.md`, `memory-map.md`.

**Keys:** `GET_SE_PRIVILEGE`, `C_CONFIG`, `C_CONFIG_INSTANCE_ADDR`, `C_CONFIG_GET_PARTNER_CODE`, `C_CONFIG_APPLY_SYS_OPT`, `C_CONFIG_CHECK_EXEC_PATH_REG`, `C_CONFIG_SYS_OPT_WINDOWED_MODE`, `C_IG_CIPHER_INNO_HASH`, `Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_CTOR`, `Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_DTOR`, `C_SYSTEM_INFO`, `C_SYSTEM_INFO_INIT`, `C_SYSTEM_INFO_GET_GAME_ROOM_CLIENT`, `C_SYSTEM_INFO_GET_MACHINE_ID`, `Z_ARRAY_REMOVE_ALL`, `Z_X_STRING_GET_BUFFER`, `Z_X_STRING_TRIM_RIGHT`, `Z_X_STRING_TRIM_LEFT`, `C_MOB_C_MOB`.

- [ ] **Step 1: Resolve CConfig keys (registry/path strings = strong anchors)**

`C_CONFIG_CHECK_EXEC_PATH_REG` references registry-key / exec-path strings; `C_CONFIG_SYS_OPT_WINDOWED_MODE` is a global within the sys-opt struct; `APPLY_SYS_OPT`, `GET_PARTNER_CODE`, `C_CONFIG`, `C_CONFIG_INSTANCE_ADDR` anchor via CConfig strings + call-graph. SP-1 two anchors each.

- [ ] **Step 2: Resolve utilities**

- `GET_SE_PRIVILEGE` references `SeDebugPrivilege` / token APIs (import + string anchors).
- `C_IG_CIPHER_INNO_HASH` — InnoHash; anchor via its constant table / call sites.
- `Z_ARRAY_REMOVE_ALL` — generic; anchor via call-graph + the `imul stride,N` shape (also a struct-audit tool, see Tasks 12–16).
- `Z_X_STRING_GET_BUFFER` / `TRIM_RIGHT` / `TRIM_LEFT` — ZXString helpers; anchor via whitespace/char constants + call-graph.
- `Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_CTOR`/`DTOR` — the fatal-section ctor/dtor pair (adjacent, small; anchor via the critical-section API calls).
- `C_SYSTEM_INFO`, `_INIT`, `_GET_GAME_ROOM_CLIENT`, `_GET_MACHINE_ID` — machine-id/system-info; anchor via the machine-id query strings/APIs.
- `C_MOB_C_MOB` — the `CMob::CMob` constructor (doom-fix hook target; boundary-adjacent, see Task 15) → `needs-main-review`. Anchor via the CMob vtable assignment + CMobTemplate use. **Record whether the ctor leaves `m_bDoomReserved` uninitialized** *and whether the field exists at all in v61* (feeds the doom-fix gate audit — v61 may lack the doom tail entirely, like v72/v79).

SP-1 two anchors each.

- [ ] **Step 3: Spot-check, label, checkpoint, validate, commit**

Same as Task 2 Steps 7–8.

```bash
git commit -am "feat(v61): relocate Config/SystemInfo/utilities cluster with signatures"
```

---

## Task 9: Memory map — Party / migrate / context senders + offsets (~9 keys)

(design §3.3 cluster 7). Packet senders reference their opcode as a `push <imm>` into a COutPacket — the opcode immediate disambiguates similar senders.

**Files:** `memory_maps/GMS/v61_1.cmake`, `signature-catalog.md`, `memory-map.md`.

**Keys:** `C_FIELD_SEND_JOIN_PARTY_MSG`, `C_FIELD_SEND_CREATE_NEW_PARTY_MSG`, `C_WVS_CONTEXT_INSTANCE_ADDR`, `C_WVS_CONTEXT_ON_ENTER_GAME`, `C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST`.
**Offset keys (SP-2):** `C_FIELD_SEND_JOIN_PARTY_MSG_OFFSET`, `C_FIELD_SEND_CREATE_NEW_PARTY_MSG_OFFSET`, `C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET`, `C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST_OFFSET` (read the v72 seed values only to know *what* to measure; never copy them).

- [ ] **Step 1: Resolve the senders (high-value) via opcode immediate**

`C_FIELD_SEND_JOIN_PARTY_MSG`, `C_FIELD_SEND_CREATE_NEW_PARTY_MSG`, `C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST`: each builds a COutPacket with a specific opcode immediate (anchor 1 = the opcode `push`/`mov`), and calls a resolved encoder + the socket send (anchor 2 = call-graph to Tasks 4/5 functions). High-value → v72→v83 chain trace + `needs-main-review`. Disambiguate the two party senders by their distinct opcodes. **Older-build note (R6):** the opcodes may have drifted in v61 — read the immediate, don't assume the v72 value; record any drift. Cross-check against atlas-ms's version-aware registries where applicable ([[reference_atlas_ms]]).

- [ ] **Step 2: Resolve C_WVS_CONTEXT keys**

`C_WVS_CONTEXT_ON_ENTER_GAME` (enter-game handler) and `C_WVS_CONTEXT_INSTANCE_ADDR` (global). SP-1 two anchors each.

- [ ] **Step 3: Re-derive all four offsets (SP-2)**

Each `*_OFFSET` is a call-site/branch offset inside its host sender/handler. Disassemble each v61 host, find the patch-point instruction (read the v72 host at the v72 offset to know which instruction — the offsets correspond to the call-site the party/migrate/redirect edits patch), and measure the v61 delta. **Never copy the v72 offsets.**

- [ ] **Step 4: Spot-check, label, checkpoint, validate, commit**

Same as Task 2 Steps 7–8.

```bash
git commit -am "feat(v61): relocate party/migrate/context senders + offsets"
```

---

## Task 10: Memory map — protocol constants + sentinels + exception-dispatch (~23 keys)

Resolved last (FR-7; design §3.3 cluster 8). Includes the exception-dispatch + CFileStream keys.

**Files:** `memory_maps/GMS/v61_1.cmake`, `signature-catalog.md`, `memory-map.md`.

**Constant keys:** `VERSION_HEADER`, `PLAYER_LOGGED_IN`, `CLIENT_START_ERROR` (v72/v79 confirmed `VERSION_HEADER`=8; v61 is older still — **verify, do not copy**).
**Exception-dispatch keys (absolute):** `C_TI_DISCONNECT_EXCEPTION`, `C_TI_TERMINATE_EXCEPTION`, `C_TI_PATCH_EXCEPTION`, `C_TI_ZEXCEPTION`, `C_PATCH_EXCEPTION_BUILDER`, `C_COM_RAISE_ERROR_EX`.
**CFileStream keys:** `C_FILE_STREAM_RESOLVED` (gate), `C_FILE_STREAM_OPEN_INLINE` (gate), `C_FILE_STREAM_OPEN`, `C_FILE_STREAM_GET_LENGTH`, `C_FILE_STREAM_READ`, `C_FILE_STREAM_CLOSE`, `C_FILE_STREAM_VFTABLE` (start from task-009's v72 disposition — FR-8a).
**GMS-absent sentinels:** `C_BATTLE_RECORD_MAN_CREATE_INSTANCE`, `DR_CHECK`, `DR_INIT`, `CE_TRACER_RUN`, `RESET_LSP` (use task-009's v72 verdict, not v83's stale comment).
**JMS-only sentinels:** `C_SECURITY_CLIENT_ON_PACKET_RET_STUB`, `C_SECURITY_CLIENT_ON_PACKET_CHECK`, `C_SECURITY_CLIENT_ON_PACKET_CHECK_OFFSET`, `C_WVS_APP_INITIALIZE_GR2D_WINDOWED_OFFSET`, `WIN_MAIN_LAUNCHER_STUB`.

> Confirm the exact sentinel/key membership against the generated tracking table (Task 1 Step 4) and task-009's `v72_1.cmake` comments — the lists above mirror task-009's cluster 8.

- [ ] **Step 1: Confirm the three protocol constants against v61 (FR-6, R6, PRD §9)**

Do **not** copy v72 values — v61 predates v72, so opcode/handler-table drift is *more* plausible. In v61 (lane discipline):
- `VERSION_HEADER` — the version word the client sends in its hello/login handshake. Find it in the connect/login send path (resolved Tasks 4/6). Confirm `8` or record the v61 value.
- `PLAYER_LOGGED_IN` / `CLIENT_START_ERROR` — send opcodes. Locate the handler/sender that uses each (login flow), read the immediate, confirm or record the v61 value.
- If any differ, use the v61 value and note the delta in the catalog (affects `redirect`/`bypass`). Cross-check against atlas-ms's version-aware registries ([[reference_atlas_ms]]).

- [ ] **Step 2: Resolve the exception-dispatch keys (SP-1)**

`C_TI_DISCONNECT_EXCEPTION`, `C_TI_TERMINATE_EXCEPTION`, `C_TI_PATCH_EXCEPTION`, `C_TI_ZEXCEPTION` are the TException-family vtables/handlers; `C_PATCH_EXCEPTION_BUILDER` and `C_COM_RAISE_ERROR_EX` are the builder/raise helpers. Anchor via their RTTI/type strings + call-graph (the CLIENT_START_ERROR relay path). SP-1 two anchors each. These exist in v72/v79/v83 — expect them present in v61; confirm by anchor, don't assume.

- [ ] **Step 3: Decide the v61 CFileStream relay disposition (FR-8a)**

Start from task-009's **v72** disposition of the CFileStream relay (its `signature-catalog.md`). Determine for v61 whether the stream helpers (`CFileStream::Open`/`GetLength`/`Read`/`Close` + vftable) are **recoverable** (locatable by anchor) or should carry `0` like the v72 verdict:
- Search v61 for the `CFileStream` RTTI/vtable + the `CreateFileA` call inside its `Open`.
- If recoverable → resolve each as a real address (SP-1), set `C_FILE_STREAM_RESOLVED 1`, and set `C_FILE_STREAM_OPEN_INLINE` per whether v61 inlines `CreateFileA` (1) or has an out-of-line `Open` (0).
- If not cleanly recoverable → carry `0` / `0x0`, with a `# unrecoverable in v61, relay gated off` comment. Document the decision in the catalog either way. **Note the CFileStream relay is a backward-direction sentinel candidate (R7) — if the helpers are a v72-era feature absent in v61, flag it for the relay edit owner.**

- [ ] **Step 4: Confirm GMS-absent sentinels, both directions (SP-5)**

- `C_BATTLE_RECORD_MAN_CREATE_INSTANCE`, `DR_CHECK`, `DR_INIT`, `CE_TRACER_RUN`: search v61 for each feature's anchor. Expect **absent** (these post-date v61 or are GMS-absent). Carry `0x00000000` + comment, record "confirmed absent + how". Cross-reference the R11 finding from Task 2 Step 2 for `DR_INIT`/`DR_CHECK`.
- `RESET_LSP`: use task-009's v72 disposition as the starting hypothesis (not v83's stale "does not exist" comment). Determine the v61 disposition **explicitly** — locate the LSP-reset function in v61 (SP-1) if it exists, else carry a sentinel — and document which.
- **Backward direction (R7):** if any v72 *real-address* key resolved in Tasks 2–9 turned out **absent** in v61, it is a new v61-only sentinel — confirm it is recorded with a `# absent in v61` comment and flagged for the gate/edit owner (SP-5 backward). Collect the full list of new v61-only sentinels here for the task report.

- [ ] **Step 5: Confirm JMS-only sentinels (SP-5)**

The five JMS-only keys: confirm the positive case in JMS185 (lane discipline) so the anchor is real, then show it absent in v61. Carry `0x00000000` with `# JMS only` for the GMS v61 build.

- [ ] **Step 6: Label (where applicable), checkpoint, validate, commit**

Constants need no IDB label; any sentinel/relay found present gets labeled. `idb_save`. Run the validator (expect `OK: all 159 keys…`). Flip rows to ✔.

```bash
git commit -am "feat(v61): confirm protocol constants, exception-dispatch, CFileStream + sentinels"
```

---

## Task 11: Memory map completeness gate

**Files:** `docs/tasks/task-010-gms-v61-support/memory-map.md`, `signature-catalog.md`.

- [ ] **Step 1: Tracking-table audit — zero ☐/◐ remaining**

Open `memory-map.md`. Confirm **every** row is marked `✔`. Any remaining `☐`/`◐` is unfinished — return to the owning cluster task. Count the ✔ rows and assert it equals the live key count (159) minus any keys the table tracks as a single grouped row; reconcile against the validator's key list if the numbers differ.

- [ ] **Step 2: Catalog coverage — every absolute key has a row**

Confirm `signature-catalog.md` has one entry per absolute-address key with two anchors recorded (two **structural** anchors + v72→v83 chain trace for the high-value tier). Every offset key has its measured target instruction + delta. Every sentinel has its absence/presence determination (both directions). Every v61-vs-v72 drift is captured (FR-14), with the explicit "ported directly / drifted" note per heuristic, and the cross-version drift summary table is filled.

- [ ] **Step 3: Final IDB save**

Lane discipline → v61 (confirm `get_metadata`, not v48) → `idb_save`. The v61 IDB now carries labels for all resolved functions/globals (FR-9/FR-10).

- [ ] **Step 4: Final completeness validation + commit**

Run: `cmake -DREGION=GMS -DMAJOR=61 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake`
Expected: `OK: all 159 keys defined and non-empty for GMS v61.1`

```bash
git add docs/tasks/task-010-gms-v61-support/
git commit -m "docs(v61): memory map complete — 159/159 keys verified + catalogued"
```

---

## Task 12: Struct audit — Category-B below-floor gates: confirm-or-split (the novel tier — verify FIRST, R3)

The five two-way `>= 83 || JMS` gates task-009 left now exclude v61 too, so v61 inherits the `#else` (v72/v79-reduced) branch. This is the highest-value, highest-novelty work in the task: for each, **confirm v61 == v72** (gate already correct — record the size) or **v61 diverges further** (the gate must split per design §5.5). **Pinned-baseline note:** task-009 left every Category-B gate **two-way** (no `< 79` arm exists), so a v61 split is **two-way → three-way** (add a GMS-guarded `< 72` arm above the unchanged `#else`). Read-only over raw disassembly: use `mcp__ida-pro__disasm`, **not** `decompile`; **never** apply a struct type (R12). Confirm the connected IDB with `get_metadata` first (not v48).

**Files (evidence only — gate rewrites are applied in Task 17):** `docs/tasks/task-010-gms-v61-support/struct-verification.md`.

**Live sites (verified against the task-009 baseline at plan time — re-grep):**

| Site | Field gated out below floor | v72 reduced size (re-confirm) | v61 must confirm |
|---|---|---|---|
| `common/CWnd.h:25` | `m_pAnimationLayer` + `m_pOverlabLayer` | (task-009 v72 CWnd size, `0x64` if == v79) | v61 `sizeof(CWnd)` via Destroy/ctor landmarks — **cascade root, do FIRST (Step 2)** |
| `common/CMob.h:241` | doom tail (`m_bDoomReserved`/SN/`m_lpStatChangeReserved`) | (task-009 v72 CMob size, `0x518` if == v79) | v61 CMob size via `?CreateMob@@` Alloc immediate + ctor highest write |
| `common/MobStat.h:128` | Weakness group (`nWeakness_/rWeakness_/tWeakness_`) | (task-009 v72 MobStat size, `0x1F8` if == v79) | v61 MobStat tail via CMob ctor `lea` to last member |
| `common/CFuncKeyMappedMan.h:19` | quickslot pair (`m_aQuickslotKeyMapped[8]`×2) | (task-009 v72 size, `0x388` if == v79) | v61 CreateInstance Alloc immediate + ctor extent (size already measured Task 2 Step 5) |
| `common/CUIToolTip.h:92` | `m_pLayerAdditional` | (task-009 v72 size, `0x514` if == v83) | v61 ctor `m_pLayer` → `m_aLineInfo` adjacency |

> Re-confirm the v72 reduced sizes against task-009's `struct-verification.md` at task start (the parenthesised values are task-008's v79 sizes, expected == v72 but pin from task-009's actual verdicts).

- [ ] **Step 1: Re-grep the live Category-B gate set + confirm arity**

```bash
grep -rn "BUILD_MAJOR_VERSION >= 83" common/*.h
grep -rn "BUILD_MAJOR_VERSION < 79" common/*.h    # expect ZERO at the pinned baseline
```

Reconcile against the table above; update `struct-verification.md` if a line moved. Confirm exactly these five sites carry the `>= 83 || JMS` two-way form and that **no `< 79` arm exists** (so a v61 split is three-way). If task-009's final state *did* add a `< 79` arm to any gate, record that gate's split as **four-way** with the `< 72` arm placed above the `< 79` arm (design §5.5).

- [ ] **Step 2: CWnd cascade root — pin v61 `sizeof(CWnd)` FIRST (R5, design §5.7)**

CWnd is the cascade root: its base shift propagates by inheritance to **CDialog, CUIWnd, CFadeWnd, CUITitle, CUILoginStart**. Pin v61 `sizeof(CWnd)` via **three independent landmarks** (per task-008/009's method): the `CWnd::~CWnd`/`Destroy` unwind extent, a ctor field-init extent, and a `RemoveAll`/Alloc immediate where a `CWnd*` array strides. Compare explicitly to v72's size:
- If **v61 CWnd == v72** → the `>= 83 || JMS` gate is already correct for v61; record `confirmed-shares-v72`. The five derived classes inherit v72's verdict (still size-confirmed each in Task 14, D5).
- If **v61 CWnd ≠ v72** → the gate must split (Task 17), and **every derived class is re-measured** (the cascade). Record `split` + the measured v61 size, and flag all five derived headers for re-derivation in Task 14.

- [ ] **Step 3: CMob / MobStat — doom tail + Weakness group**

- `common/CMob.h:241`: measure v61 `sizeof(CMob)` via the `?CreateMob@@` Alloc immediate and the ctor's highest member write. Compare to v72's size. Determine whether the doom tail (`m_bDoomReserved`/SN/`m_lpStatChangeReserved`) is present. Record `confirmed-shares-v72` (v61 == v72) or `split` (v61 diverges). This drives the doom-fix gate (Task 15 / Cat C).
- `common/MobStat.h:128`: measure the v61 MobStat tail via the CMob ctor `lea` to its last member. Compare to v72's size. Determine whether the Weakness group is present. Record verdict.

- [ ] **Step 4: CFuncKeyMappedMan / CUIToolTip — quickslot pair + m_pLayerAdditional**

- `common/CFuncKeyMappedMan.h:19`: the v61 size was measured in Task 2 Step 5. Confirm here whether the quickslot pair (`m_aQuickslotKeyMapped[8]`×2) is present by re-deriving the CreateInstance Alloc immediate + ctor extent independently. Compare to v72's `0x388`. Record verdict. (If v61 diverges, the Task 3 Cat-A size-assert branch must match — reconcile.)
- `common/CUIToolTip.h:92`: in v61, disassemble a `CUIToolTip` method accessing the layer members and determine whether `m_pLayerAdditional` is present (ctor `m_pLayer` → `m_aLineInfo` adjacency). Compare to v72's size. Record verdict.

- [ ] **Step 5: Record verdicts + main-session cross-check (mandatory v61-vs-v72 comparison)**

For each of the five headers, write into `struct-verification.md`: the v61 size, the explicit **v61-vs-v72 comparison** (mandatory for Category B), every gated field's present/absent verdict, the gate disposition (`confirmed-shares-v72` / `split`), and the deciding v61 disasm line(s). Before treating any `split` verdict as final, re-anchor at least one boundary case per such header with one independent probe (design §5.2). **Do not edit source yet** — gate rewrites are applied in Task 17.

```bash
git add docs/tasks/task-010-gms-v61-support/struct-verification.md
git commit -m "docs(v61): Category-B below-floor gate audit (confirm-or-split) + CWnd cascade root"
```

---

## Task 13: Struct audit — core/net layout headers, exhaustive size (Cat C + special cases)

Read-only `disasm` only; no struct-type application (R12). Per **D5**, record the **v61 size** of every header even when it "should" match the base — there is no IDB below v61 to catch a silent base-size delta (R3), and the base is sometimes the already-twice-diverged v72-reduced branch. Use the size-finding techniques in `struct-verification.md`: a `Decode`/`DecodeBuffer(this, N)` literal = struct size; `ZArray<T>::RemoveAll` `imul stride,N` = `sizeof(T)`; destructor unwind extent bounds the layout.

**Files:** `docs/tasks/task-010-gms-v61-support/struct-verification.md`.
**Headers (7):** `common/CWvsApp.h`, `common/CWvsContext.h`, `common/CClientSocket.h`, `common/CLogin.h`, `common/COutPacket.h`, `common/CConfig.h`, `common/ConfigSysOpt.h`.

- [ ] **Step 1: Grep each header's thresholds**

`grep -n BUILD_MAJOR_VERSION` each. Note which change truth value at 61: every `> 83`/`>= 87`/`>= 95`/`>= 111`/`== 83`/`== 87`/`== 95` is **false** for v61 → v61 takes the base ("v83 layout") branch; every `< 95`/`< 84` is **true**. List each gate's v61 truth value.

- [ ] **Step 2: CWvsContext — `m_aClientKey[8]` (`> 83`, special)**

The `> 83` gate (`common/CWvsContext.h:98` and `bypass/socket_hooks.cpp:310`) is **false for v61**, so the 8-byte `m_aClientKey` block is predicted **absent**. Confirm using the Task 4 Step 3 finding: v61's `CClientSocket::OnConnect` / connect-hello does **not** encode an 8-byte client key (v72 confirmed absent — [[project_v84_clientkey_gate_trap]]; verify decode order against the v61 binary, not a server round-trip). Record v61 `sizeof(CWvsContext)` + the verdict. Verify the embedded `SecondaryStat` size independently (cross-ref Task 15) — a wrong embedded size shifts every field after it.

- [ ] **Step 3: CLogin — confirm the `== 83` member verdict (`unk3[5]`)**

`CLogin.h:235` (`== 83`) was confirmed in Task 3 Step 3 (excluded for v61). Record the v61 `sizeof(CLogin)` here and fold in that verdict (`unchanged` if v61 lacks `unk3[5]`, `rewritten` if Task 3 found v61 has it). Anchor the size with an independent probe.

- [ ] **Step 4: CWvsApp / CClientSocket / COutPacket / CConfig / ConfigSysOpt — size + gated fields**

For each: confirm the `>= 87`/`>= 95`/`>= 111` gated fields are **absent** in v61 (v61 < 87 → base layout), record v61 size + per-gate verdict + disasm anchor. `CWvsApp` size was measured in Task 3 — record it here in the verdict table. `ConfigSysOpt` ties to `C_CONFIG_SYS_OPT_WINDOWED_MODE` (Task 8) — cross-check the windowed-mode field offset. `CConfig.h:84` (`== 95`) is false for v61 → base.

- [ ] **Step 5: Main-session cross-check + commit**

Re-anchor one boundary case per header whose v61 size differs from v72/v83. Commit verdicts.

```bash
git commit -am "docs(v61): struct audit — core/net headers (CWvsApp/CWvsContext/CClientSocket/CLogin/COutPacket/CConfig/ConfigSysOpt)"
```

---

## Task 14: Struct audit — UI/control family, exhaustive size (9 headers; CWnd cascade)

Read-only `disasm`; no struct-type application. Record v61 size for each (D5). **This family carries the CWnd cascade (R5):** the verdict from Task 12 Step 2 governs whether the five CWnd-derived classes inherit v72's layout or are re-measured.

**Files:** `docs/tasks/task-010-gms-v61-support/struct-verification.md`.
**Headers (9):** `common/CUITitle.h`, `common/CUILoginStart.h`, `common/CUIToolTip.h` (Cat-B gate done in Task 12 — record full size + remaining gates here), `common/CUIWnd.h`, `common/CWnd.h` (cascade root, size pinned in Task 12 — record here), `common/CCtrlButton.h`, `common/CCtrlCheckBox.h`, `common/CFadeWnd.h`, `common/CLogo.h`.

- [ ] **Step 1: Grep thresholds for all 9 headers**

`grep -n BUILD_MAJOR_VERSION` each. These are predominantly `>= 95`/`>= 87` gates (all false for v61 → v61 sides with the base layout), plus the CUIToolTip `>= 83` gate audited in Task 12 and `CLogo.h:93` (`== 95`, false for v61). List each gate's v61 truth value.

- [ ] **Step 2: Resolve the CWnd cascade verdict (linked, not five independent)**

Per Task 12 Step 2:
- If **v61 CWnd == v72** → CDialog/CUIWnd/CFadeWnd/CUITitle/CUILoginStart inherit v72's verdict; **still confirm each derived size against an independent v61 anchor** (D5 — a derived class can add its own gated field even when the base matches).
- If **v61 CWnd ≠ v72** → re-derive each of the five derived classes from the measured v61 base; the split (design §5.5) cascades to each (flag for Task 17).

- [ ] **Step 3: Verify each header's gated fields + size**

For each of the 9: pick a method that accesses the gated region (disasm), determine v61 size, and confirm each `>= 87`/`>= 95` field is **absent** in v61. `CWnd`/`CUIWnd`/`CCtrlButton`/`CCtrlCheckBox` are control base classes — anchor their sizes carefully (a wrong base size shifts every derived layout). Record verdict + anchor per header. For `CUIToolTip`, fold in the Task 12 verdict and record the full v61 size.

- [ ] **Step 4: Main-session cross-check + commit**

```bash
git commit -am "docs(v61): struct audit — UI/control family (9 headers; CWnd cascade resolved)"
```

---

## Task 15: Struct audit — Mob/stat family, exhaustive size (4 headers)

Read-only `disasm`; no struct-type application. **SecondaryStat is embedded UDT + size-critical** — verify its size independently, do NOT assume v61 == v72 == v83 (`struct-verification.md`; embedded UDTs often shrink in older builds).

**Files:** `docs/tasks/task-010-gms-v61-support/struct-verification.md`.
**Headers (4):** `common/CMob.h` (Cat-B/doom done in Task 12 — record full size here), `common/MobStat.h` (Cat-B done in Task 12 — record full size here), `common/SecondaryStat.h`, `common/CMapLoadable.h` (README-critical).

- [ ] **Step 1: Grep thresholds**

`grep -n BUILD_MAJOR_VERSION` each. SecondaryStat has `== 87`/`>= 87` gates (`:405`/`:419`) — both **absent** for v61 (v61 < 87 → base layout). CMob `>= 83 || JMS` (Cat B, Task 12), plus `>= 84`/`>= 87`/`>= 95`/`< 95` gates. MobStat `>= 83 || JMS` (Cat B) + `>= 95`. CMapLoadable `>= 95`/`>= 84`.

- [ ] **Step 2: Verify SecondaryStat size + gated fields independently**

Find the v61 `SecondaryStat` size from a `Decode`/`RemoveAll` `imul stride,N` anchor where it is embedded (inside `CWvsContext::m_secondaryStat`, cross-ref Task 13 Step 2, or a CUser stat decode). The v72/v79/v83 base embedded size is `0xB88` — **confirm v61 computes the same, do not assume** (a wrong embedded size shifts every field after `m_forcedStat` in `CWvsContext`). Confirm every `>= 87`/`== 87` gated field is **absent** in v61. Record the exact v61 size + per-gate verdict.
**Optional cross-validation:** if any SecondaryStat bit-indexed disagreement is suspected, dispatch the `stat-registry-cross-validator` agent against atlas-ms ([[reference_atlas_ms]]) with the v61 mapping — but only if a disagreement surfaces; the default expectation is "v61 sides with v72/v79/v83."

- [ ] **Step 3: Verify CMob, MobStat, CMapLoadable**

CMob: fold in the Task 12 Cat-B verdict (doom tail present/absent, base size); confirm `< 95` fields present and `>= 84`/`>= 87`/`>= 95` fields absent; record full v61 size. MobStat: fold in the Task 12 Cat-B Weakness-group verdict; confirm `>= 95` fields absent; record full v61 size. CMapLoadable: confirm `>= 95`/`>= 84` fields absent, record v61 size + anchor (README-flagged — anchor with extra care).

- [ ] **Step 4: Main-session cross-check + commit**

```bash
git commit -am "docs(v61): struct audit — Mob/stat family (4 headers)"
```

---

## Task 16: Struct audit — Party/Guild/misc, exhaustive size; complete 24/24 (4 headers)

Read-only `disasm`; no struct-type application. Record v61 size for each (D5).

**Files:** `docs/tasks/task-010-gms-v61-support/struct-verification.md`.
**Headers (4):** `common/PartyData.h`, `common/PartyMember.h`, `common/GuildData.h`, `common/CFuncKeyMappedMan.h` (Cat-A size gate amended in Task 3 + Cat-B quickslot verdict in Task 12 — record the full size + remaining-gate verdict here).

- [ ] **Step 1: Grep thresholds**

`grep -n BUILD_MAJOR_VERSION` each. PartyMember may have `< 95` (true for v61); PartyData/GuildData `>= 95` (false); CFuncKeyMappedMan `>= 83 || JMS` (Cat B, Task 12) + the `assert_size` chain amended in Task 3. List each gate's v61 truth value.

- [ ] **Step 2: Verify each header**

For each: disasm a method accessing the gated region, determine v61 size, confirm the `< 95` field present / `>= 95`/`>= 111` fields absent for v61, record verdict + anchor. These are packed structs — anchor sizes carefully. `CFuncKeyMappedMan` — reconcile the v61 size recorded in Task 2/Task 12 against an independent anchor here, and verify the quickslot + `>= 111`/`>= 95` member gates resolve consistently with the Task 3 size-assert branch.

- [ ] **Step 3: Complete the 24-header verdict table + main-session cross-check + commit**

After this task, all **24** rows in `struct-verification.md` must have a v61 size + verdict + evidence (and the five Category-B rows must carry an explicit v61-vs-v72 comparison). Assert no `☐` rows remain. List the count and confirm it is 24.

```bash
git commit -am "docs(v61): struct audit — Party/Guild/misc (4 headers); 24/24 complete"
```

---

## Task 17: Apply gate rewrites (splits + confirms) + cross-version validation (FR-12b/FR-13, R12)

Only now — with all 24 headers + the Category-B confirm-or-split verdicts in hand — apply source changes for any gate the audit (Tasks 12–16) marked `split` or `rewritten`. The Cat-A size gates were already amended in Task 3. A `confirmed-shares-v72` verdict requires **no** source edit (design §5.9 confirm-in-place is a first-class outcome and the expected common case).

**Files (only those the audit flagged):** any of `common/CWnd.h`, `common/CMob.h`, `common/MobStat.h`, `common/CFuncKeyMappedMan.h`, `common/CUIToolTip.h`, the five CWnd-derived headers (if the cascade split), `common/CLogin.h`, `common/CWvsContext.h`, `bypass/socket_hooks.cpp`, `doom-fix/dllmain.cpp`, and any other `common/*.h` the audit corrected.

- [ ] **Step 1: For each flagged gate, design the minimal correct boundary (design §5.5/§5.9)**

- Keep the house guard form: `#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION <cmp>) ...`.
- **Category-B split (the novel case):** when v61 diverges further than v72, add a GMS-guarded `BUILD_MAJOR_VERSION < 72` arm **above** the unchanged `#else` (v72/v79 branch). At the pinned baseline (gate is two-way), this makes the gate **three-way**. Canonical shape (CWnd example):

```c
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 83) || defined(REGION_JMS)
    // full v83 layout (m_pAnimationLayer + m_pOverlabLayer present)
#elif (defined(REGION_GMS) && BUILD_MAJOR_VERSION < 72)
    // NEW: v61 further-reduced layout
#else
    // v72/v79-reduced layout (GMS 72..82) — UNCHANGED
#endif
```

  Use `< 72` (not `== 61`) per D8: future-proofs the next older port and stays disjoint (no supported version satisfies `< 72` except v61). The `#else` (v72/v79 branch) must remain **byte-identical in effect** — the split adds an arm above it, never edits it. **If task-009's final state added a `< 79` arm to this gate (re-grep), the gate is already three-way; place the new `< 72` arm above the `< 79` arm, making it four-way** (design §5.5; `< 72` must precede `< 79` since every `< 72` version also satisfies `< 79`).
- **Cat-A enumerated-extend (if Task 3 Step 3 found v61 has `unk3[5]`):** `== 83` → `(== 83 || == 61)`. (Already applied in Task 3 if found; otherwise no edit.)
- Gate the **minimum contiguous region** — a single divergent field gets a one-field gate, not a section gate. **No ternary in array dimensions** — use `#if/#else/#endif`.

- [ ] **Step 2: Cross-version truth-table the rewrite (FR-13, design §5.7/§5.9)**

For each rewritten gate, build the truth table across **GMS 61, 72, 79, 83/84/87, 95/111, JMS185** and confirm every version still selects its correct branch (use the §5.9 template in `struct-verification.md`):
- GMS 61 → the new `< 72` arm (NEW).
- GMS 72 → the `#else` (or the `< 79` arm if one exists) — UNCHANGED; verify byte-identical effect.
- GMS 79 → the `#else` — UNCHANGED.
- GMS 83/84/87/95/111 → the `>= 83` arm (unchanged; their IDBs except v111 are live — re-anchor with a probe if the rewrite touches a layout claim).
- JMS 185 → the first arm (`>= 83 || REGION_JMS`, since 185 ≥ 83), unchanged.
- **v111** (not loaded) — evaluate from build constants + `memory_maps/GMS/v111_1.cmake`. If correctness for v111 cannot be settled from source alone, **say so explicitly** in `struct-verification.md` ([[feedback-prefer-confirmation]], A2).
- Record the truth table in `struct-verification.md` next to each rewrite.

- [ ] **Step 3: Apply the source edits**

Edit only the flagged sites. For each, add a brief comment referencing the audit (e.g. `// v61: CWnd further-reduced — split, verified task-010`). Match house style (clang-format the changed lines). For a CWnd cascade split, apply the `< 72` arm to CWnd **and** each of the five derived headers the cascade re-measured.

- [ ] **Step 4: Local sanity — preprocess each changed header for v61 and neighbors**

For each changed file, for `{61, 72, 79, 83}`, confirm each split field resolves as intended:

```bash
echo '#include "CWnd.h"' > "$CLAUDE_JOB_DIR/tmp/_gatecheck.cpp"
gcc -E -DREGION_GMS -DBUILD_MAJOR_VERSION=61 -DBUILD_MINOR_VERSION=1 -I common -I include "$CLAUDE_JOB_DIR/tmp/_gatecheck.cpp" 2>/dev/null | grep -n "m_pAnimationLayer"
```

Expected: the field appears for the versions the audit says should have it, absent otherwise — **and the v72/v79 branch's effect is unchanged at `-DBUILD_MAJOR_VERSION=72` and `=79`**. (This validates the comparator, not the full build — that is Task 18/CI.)

- [ ] **Step 5: Commit**

```bash
git add common/ bypass/ doom-fix/ docs/tasks/task-010-gms-v61-support/
git commit -m "fix(v61): below-floor gate splits per v61 audit; cross-version validated"
```

(If the audit found **every** Category-B gate is `confirmed-shares-v72` — the expected common case — this task's only artifact is the recorded "confirm-in-place" rationale already in `struct-verification.md`: skip Steps 3–5 and note "no further gate rewrites required beyond Task 3's Cat-A amendments" in the task report.)

---

## Task 18: Build wiring + build verification (FR-1/FR-2/FR-15, R12)

**Files:** `.github/workflows/_build.yml`

- [ ] **Step 1: Add the v61 matrix entry (placed FIRST — version-ascending)**

In `.github/workflows/_build.yml`, add to `strategy.matrix.config` as the **first** entry (the list is version-ascending, currently starting at `GMS 72` on the task-009 baseline), aligning columns with the existing rows:

```yaml
          - { region: GMS, major: 61,  minor: 1 }
          - { region: GMS, major: 72,  minor: 1 }
          - { region: GMS, major: 79,  minor: 1 }
```

No other workflow file changes — PR/snapshot/release all consume `_build.yml` (FR-1/FR-2).

- [ ] **Step 2: Local completeness gate (final)**

Run: `cmake -DREGION=GMS -DMAJOR=61 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake`
Expected: `OK: all 159 keys defined and non-empty for GMS v61.1`

- [ ] **Step 3: Local pre-flight build (Debug + Release) via WSL**

Build GMS 61.1 on the WSL/clang-cl path before the Windows/CI handoff ([[project_wsl_cross_compile]]):

```bash
scripts/wsl-build.sh GMS 61 1
```

Confirm it compiles + links clean for Debug and Release. If any gate was rewritten (Tasks 3/17), also rebuild the other matrix versions affected to prove no regression (R12) — **especially v72 and v79**, whose branches every split must preserve:

```bash
scripts/wsl-build.sh GMS 72 1
scripts/wsl-build.sh GMS 79 1
```

Record exactly what was run and its result. **Claim only what was actually run** ([[verification-before-completion]]); the authoritative MSVC build is CI.

- [ ] **Step 4: Commit + open PR (stacked on task-009)**

```bash
git add .github/workflows/_build.yml docs/tasks/task-010-gms-v61-support/
git commit -m "ci(v61): add GMS 61.1 to the build matrix"
git push -u origin task-010-gms-v61-support
gh pr create --base task-009-gms-v72-support --title "feat: GMS v61.1 support (task-010)" --body "<summary + pinned task-009 baseline SHA + PRD §10 acceptance checklist>"
```

(Per D0/R13: the PR base is `task-009-gms-v72-support` while it is unmerged; retarget to `main` once 008/009 land, and rebase onto the final task-009 state — see Task 19 Step 0.)

- [ ] **Step 5: Confirm CI is green for GMS 61.1 (and unchanged versions)**

Watch the PR's build workflow. Confirm GMS 61.1 builds Debug + Release green and **no other version regressed** (the gate-rewrite backstop, R12 — v72/v79 in particular). Paste the result into the task report. If CI fails, that is build-correctness evidence — fix and re-push; do not proceed to acceptance with a red build.

---

## Task 19: Acceptance — inherited-gate re-confirmation + live smoke test (user-run, gates completion — D4/FR-16/R13)

**Files:** `docs/tasks/task-010-gms-v61-support/acceptance.md` (record the result).

- [ ] **Step 0: Rebase onto the final task-009 state + re-confirm inherited gates (R13, design §7)**

Before merge, rebase `task-010-gms-v61-support` onto the final task-009 tip. Then **re-confirm every gate branch v61 *inherited*** (rather than amended) still selects the intended layout — if task-009's below-floor gates changed after this task started, v61's inherited branch assignments silently moved. Concretely: re-grep `BUILD_MAJOR_VERSION` across the five Category-B headers + the Cat-A sites; confirm each still resolves v61 to the branch the audit recorded. Re-run `scripts/wsl-build.sh GMS 61 1` after the rebase. Note any task-009 change that forced a re-audit. (If task-009 is already merged to `main`, retarget the PR base to `main` here.)

- [ ] **Step 1: Prepare the smoke-test instructions for the user**

Create/confirm `acceptance.md` with the deploy + launch + observe checklist (model it on task-009's `acceptance.md`). Confirm the GMS 61.1 artifacts are available — note the **PR matrix build runs `upload: false`** and does not publish DLLs; the user must use the post-merge `Build Snapshot` artifact (`snapshot-GMS-61.1-Release`) or a **local VS2022 x86 build** (`cmake -B build -G "Visual Studio 17" -A Win32 -DBUILD_REGION=GMS -DBUILD_MAJOR_VERSION=61 -DBUILD_MINOR_VERSION=1`, then `--target package_dlls`). Hand the user the checklist. **Interpret the launch against the packing finding (D9/§6): a pre-Themida v61 client will not throw the Themida integrity faults the README documents, so a clean launch is read differently.**

- [ ] **Step 2: User runs the live client**

The user launches the GMS v61 client with the proxy `ijl15.dll` + the core edits (`redirect`, `no-patcher`, `no-ad-balloon`, `bypass`, …) deployed per README "Usage", reaches the title/login screen, exercises the targeted edits, and confirms no crash / no Themida fault (FR-16). **Edits whose backing feature is a v61 sentinel (e.g. `no-ad-balloon`/`no-patcher` if the feature is era-absent) are expected to be no-ops — note this so a "did nothing" result is not misread as a failure.**

- [ ] **Step 3: Record the exact result**

Paste the verbatim outcome (pass/fail per checkbox, plus any crash address / Themida message) into `acceptance.md` and the PR. Distinguish build-correctness from environment issues (Themida / VC++ redist / OS) per README compatibility notes (R8). The task is **not done** until this is run and the outcome recorded — a correct build with a failed/blocked smoke test is reported as such, not as success ([[verification-before-completion]]).

- [ ] **Step 4: Final task report + finish the branch**

Summarize against the PRD §10 acceptance checklist (every box), including any findings flagged for the user: the packing/Themida determination (D9), the Radio quirk, RESET_LSP disposition, protocol-constant drift, **every new v61-only sentinel** (ad balloon / patcher / MapleTV / MonsterBook / MTS / CFileStream relay — and which edit each makes a no-op), the CFileStream decision, every Category-B `split` outcome + its cascade, the DR_init era-absence confirmation, and any v111 unsettleable cases. Then use `superpowers:finishing-a-development-branch` to merge/close out per the user's preference.

---

## Self-review notes (author)

- **Spec coverage:** FR-1/2 → Task 18. FR-3 → Task 1 (seed from v72) + `CheckMemoryMapKeys` + Task 11. FR-4 → Tasks 2,4–10 (SP-1 two-anchor, seed from v72 / name from v83). FR-5 → SP-2 (Tasks 2,9). FR-6 → Task 10 Step 1. FR-7 → SP-5 + Task 10 Steps 4–5 (both directions) + the era-absent watch in Tasks 2/7. FR-8 → SP-3. FR-8a → Task 10 Steps 2–3 (exception-dispatch + CFileStream, from v72 disposition). FR-9/10 → SP-1 step 6 + SP-6 + Task 11 Step 3. FR-11 → Tasks 12–16 (24 headers, exhaustive size D5). FR-12a → Task 3 (early Cat-A: CWvsApp/CFuncKeyMappedMan/CLogin). FR-12b → Tasks 12,17 (Category-B confirm-or-split, `< 72` split). FR-12c → Tasks 13–16 (base/excluded exhaustive size). FR-13 → Task 3 Step 5, Task 17 Step 2 (full-matrix truth tables, v72/v79 branches preserved). FR-14 → SP-4 + SP-1 step 7 + Task 11 Step 2 (drift summary). FR-15 → Task 18. FR-16 → Task 19. Risks: R1→SP-1 D6 two-structural + v72→v83 chain trace + spot-checks; R2→SP-2; R3→T12 (confirm-or-split) + D5 exhaustive size T13–16; R4→T3 (Cat-A early); R5→T12 Step 2 + T14 (CWnd cascade, linked verdict); R6→T9 Step 1 + T10 Step 1; R7→SP-5 backward + T2/T7 era-watch + T10 Step 4; R8→T10 Step 1 + T19 Step 3; R9→lane discipline header + v48-distractor guard + sparse-IDB confirmation; R10→T1 Step 5 + T2 Step 1 (packing) + T12–16 read-only; R11→T2 Step 2 (DR_init/SetUp) + T18; R12→T17 Step 2 + T18; R13→§0 baseline pin + T19 Step 0 (rebase + inherited-gate re-confirm).
- **Anchor-direction (vs task-009):** task-009 seeded from v79, named from v83. task-010 **seeds from v72** (closest, +11; carries below-floor relocations) and **names from v83** (canonical). SP-1 reflects this: characterize in v72, confirm name in v83, locate in v61. The v48 IDB is a loaded **distractor**, never an anchor — every probe distinguishes it via `get_metadata`.
- **Arity correction (live-pinned):** the design anticipated task-009 producing three-way splits (→ v61 four-way). The **actual pinned baseline** is that task-009 left all five Category-B gates **two-way** (`>= 83 || JMS` / `#else`, zero `< 79` arms) and folded v72 into the v79 enumerated branches (`== 72 || == 79`). So a v61 divergence is **two-way → three-way** using the `< 72` arm above `#else` (Task 17 Step 1). Each task re-greps and handles the four-way case if task-009's final state added a `< 79` arm.
- **Live-gate verification:** the five Category-B sites (`CWnd.h:25`, `CUIToolTip.h:92`, `CFuncKeyMappedMan.h:19`, `MobStat.h:128`, `CMob.h:241`), the three Cat-A sites (`CWvsApp.h:98`, `CFuncKeyMappedMan.h:52`, `CLogin.h:235`), the doom-fix `== 83` (`doom-fix/dllmain.cpp:27`), and the socket/context `> 83` (`bypass/socket_hooks.cpp:310`, `common/CWvsContext.h:98`) were all confirmed present in the task-009 baseline tree at plan time. Each task re-greps because line numbers shift.
- **Key-count:** the live header parses to **159** keys (160 `@…@` minus `BUILD_REGION`). Task 1 Step 3 pins it and stops if ≠159. All validator-expected output strings say "159".
- **Validator reused:** `cmake/CheckMemoryMapKeys.cmake` already exists — this plan reuses it (Tasks 1,2,11,18), does not recreate it.
- **Placeholder scan:** unknown v61 addresses are discovered outputs, not placeholders — every task specifies the exact MCP tool + anchor recipe. No "TBD"/"handle edge cases" steps. `0xNN`/`0xXXXXXX` in code samples are illustrative templates for *discovered* values, with the discovery method given.
- **Type/name consistency:** the validator invocation `cmake -DREGION=GMS -DMAJOR=61 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake` is identical in Tasks 1,2,11,18. SP-1…SP-6 referenced by exact name throughout. Cluster→Task mapping mirrors design §3.3 (clusters 1–8 → Tasks 2,4,5,6,7,8,9,10). The 24 audited headers partition disjointly across Tasks 12–16 (Category-B subset of 5 in Task 12; full size in 13[7]+14[9]+15[4]+16[4] = 24, with the 5 Cat-B headers also size-finalized in their family task). Verdict vocabulary (`unchanged`/`branch-added`/`confirmed-shares-v72`/`split`/`rewritten`) matches design §5.2 / `struct-verification.md`.
- **Stacked baseline:** Task 1 branches off the task-009 tip in a worktree and pins the SHA; Task 18 opens the PR with base `task-009-gms-v72-support`; Task 19 Step 0 rebases onto the final task-009 state and re-confirms inherited gates before merge (D0/R13).
- **Temp-file hygiene:** gate-check scratch files use `$CLAUDE_JOB_DIR/tmp` (background-session shared-`/tmp` safety), not `/tmp`.
