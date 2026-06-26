# GMS v79 Support Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add GMS v79.1 as a first-class supported build target — a complete 159-key memory map located in the v79 binary, a labeled v79 IDB, all 24 version-gated `common/*.h` headers verified against v79 with the **below-floor gate audit** resolved, and the CI matrix wired so every PR exercises v79.

**Architecture:** A `discovery → label → emit` pipeline produces `memory_maps/GMS/v79_1.cmake` (seeded from v83 so the 159-key set is always complete, then each key relocated and re-verified against the v79 IDB with two independent anchors). Because v79 sits **below the lowest currently-supported version (83)**, a separate read-only struct/gate audit treats every `common/` gate as a "which branch does v79 land on, and is that branch's layout actually v79's?" question (Categories A–E, design §5). The enumerated no-`#else` gates are amended **early** (design D7) so the v79 build is provably compilable before the full audit runs. All IDA probing runs on a single serialized lane — the IDA MCP `select_instance` is global mutable state; concurrent multi-instance probing is unsafe (design §D3-note).

**Tech Stack:** IDA Pro MCP (6 live IDBs: v79 target + v83/v84/v87/v95/JMS185 references), CMake (`GenerateMemoryMap.cmake` key-completeness gate + the existing standalone `cmake/CheckMemoryMapKeys.cmake` validator), GitHub Actions matrix build (MSVC/Win32), C++17 client-edit DLLs, `scripts/wsl-build.sh` (clang-cl + xwin local pre-flight).

## Global Constraints

- **Region/version under construction:** `{ region: GMS, major: 79, minor: 1 }`. `BUILD_REGION=GMS`, `BUILD_MAJOR_VERSION=79`, `BUILD_MINOR_VERSION=1`.
- **Key set is 159** (the live count parsed from `include/memory_map.h.in`, excluding `BUILD_REGION`). The PRD/design say "155" — that figure is stale; the set grew with the exception-dispatch + CFileStream keys. **Phase 0 re-greps and pins the live number** (Task 1 Step 3); if it is not 159, stop and reconcile before proceeding. Every value below that names "159" means "the live count."
- **Evidence before assertion.** Every address, every offset, every gate verdict is anchored to a concrete v79 disassembly observation. "Same as v83" is acceptable **only** when a signature confirms it — never assumed. This is doubly load-bearing for v79: there is **no labeled IDB below v79** to triangulate against ([[feedback-prefer-confirmation]]).
- **IDB target hygiene.** `get_metadata` confirms the connected IDB before *every* version-specific probe ([[feedback-verify-ida-target]]). Never infer the active IDB from "what I selected last."
- **Read-only struct audit.** During struct verification use `disasm`, never `decompile`; **never apply speculative struct types** into any IDB (decompiler leak, R9). Function/global *labeling* of the v79 IDB for the memory map is encouraged and separate.
- **No commits to `main`.** Work on branch `task-008-gms-v79-support` throughout ([[feedback-no-main-commits]]).
- **Cross-version safety.** Every gate amendment must keep v83/v84/v87/v95/v111/JMS185 selecting their current branch (FR-13). v111 is **not loaded** — validate it from build constants + `memory_maps/GMS/v111_1.cmake`; surface any unsettleable v111 case rather than assuming it (A2).
- **Backward sentinel direction.** Going from v83 *down* to v79, a feature v83 has may be *absent* in v79 (the inverse of the v84 port). Confirm each sentinel's v79 disposition in **both** directions; a v83 real-address key whose feature is absent in v79 becomes a **new v79-only sentinel**, flagged — not silently zeroed (R4).

---

## Reading order before starting

Read these task-folder docs — they hold data this plan references rather than duplicates:

- `prd.md` — requirements (FR-1…FR-16), §7 key-class table, acceptance criteria §10.
- `design.md` — orchestration (§3), the two-anchor evidence rule (§4, D1/D6), the below-floor gate audit strategy (§5, Categories A–E), the gate-rewrite expression strategy (§5.7), the IDB topology (§2).
- `memory-map.md` — **the authoritative work-list**: per-key v83 seed address, class (addr/offset/constant/sentinel), backward-sentinel table, and the live status legend (`☐ todo · ◐ located+labeled · ✔ written+catalogued`). The tracking table is updated in place as the source of truth for "what is done."
- `struct-verification.md` — the 24-header verdict table + the Category A–E gate breakdown + the named special-attention gates (CMob/CWvsContext/CLogin).
- `signature-catalog.md` — the durable anchor catalog to populate (one row per resolved function); **consult the v84 catalog at `docs/tasks/task-006-gms-v84-support/signature-catalog.md` first** — most anchors transfer; record only v79 deltas here.
- `risks.md` — R1…R12.
- `context.md` — the one-page orientation / gate-direction cheat-sheet.

---

## IDB lane discipline (applies to EVERY IDA-touching step)

Six reference IDBs are loaded (design §2). **Ports are NOT hardcoded — they are reassigned between sessions.** Resolve the port for a version by binary name, every session:

| Version | Binary (match on this) | Role |
|---|---|---|
| GMS v79.1 | `GMS_v79_1_DEVM.exe` | **Target** — locate, label, `idb_save` |
| GMS v83 | `MapleStory_dump.exe` | **Primary anchor** (closest, best-labeled; sits *above* v79) |
| GMS v84.1 | `GMS_v84.1_U_DEVM.exe` | Secondary anchor (next-closest; freshly labeled by task-006) |
| GMS v87 | `GMSv87_4GB.exe` | Upper-gate cross-validation |
| GMS v95 | `GMS_v95.0_U_DEVM.exe` | PDB reference / upper-gate cross-validation |
| JMS v185 | `MapleStory_dump_SCY.exe` | JMS-only sentinel confirmation / JMS-branch FR-13 check |

Every probe follows this sequence (R8, [[feedback-verify-ida-target]]):
1. `mcp__ida-pro__list_instances` → find the port whose binary name matches the version you intend.
2. `mcp__ida-pro__select_instance(port=<resolved>)`.
3. `mcp__ida-pro__get_metadata` (or `server_health`) — confirm the routed IDB is the one you intend **before** drawing any conclusion.
4. Probe.
5. After **any** version switch, repeat 1–3. Never infer the active IDB from "what I selected last."

Never run two IDA-probing tasks concurrently — `select_instance` routing is shared global state. subagent-driven-development runs one task at a time, which satisfies this; do not parallelize IDA tasks. (If the user later stands up a second MCP server on a distinct port, discovery may be re-parallelized by pinning each agent to a server — design §A6; until then, serial.)

---

## Standard procedures (referenced by name from the cluster tasks)

Defined once here, invoked by name from Tasks 2 and 4–10. Follow them literally per key.

### SP-1 — Resolve one absolute-address key

For key `K` whose v83 seed address is `A83` (from `memory-map.md`):

1. **Characterize in v83** (lane discipline → v83 binary). Probe `A83`:
   - `mcp__ida-pro__decompile(A83)` and `mcp__ida-pro__disasm(A83)` to read the function.
   - Record its distinctive anchors: referenced string literals (`xrefs_to` / strings in the body), called imports (`connect`, `socket`, registry/file APIs), magic constants / opcode immediates, vtable slot, and call-graph neighbors (`callees`, `xrefs_to`).
2. **Locate in v79** (lane discipline → v79 binary) using anchors in this preference order (design §4):
   1. **String xref** — `search_text` / `find_regex` for the exact literal, then `xrefs_to` the string to reach the referencing function. *Older build: the v83 literal may have drifted — see step 6.*
   2. **Import/API call anchor** — `imports` / `imports_query` for the API, then `xrefs_to` it.
   3. **Call-graph anchor** — child/parent of an already-resolved v79 function (`callees` / `xrefs_to`).
   4. **Constant / opcode immediate** — `find_bytes` / `find_regex` / `insn_query` for the immediate.
   5. **Vtable slot** — find the class vtable via its RTTI/type string, index the slot.
   6. **Byte/structure signature** (`make_signature_for_function(A83)` then `find_bytes` in v79) — **tiebreaker only for v79; never counts as an anchor** (D6).
3. **Apply the anchor rule (design §4):**
   - **Baseline (ordinary keys):** at least **two independent anchors** both resolve to the same v79 address. "Independent" = two different kinds, or two clearly distinct instances of the same kind. **At least one must be kind 1–5** — a pair of byte-sigs is invalid.
   - **High-value / high-blast tier (D6)** — socket send/flush/process, the COutPacket encoders, WinMain, `CWvsApp::Run`/`SetUp`, the packet senders, and any function adjacent to a below-floor gate (§5): require **two anchors of kinds 1–5** (two *structural* anchors of two different kinds). A byte-sig may only *disambiguate* between two structurally-anchored candidates; it does not count toward the pair. Flag the key `needs-main-review`.
4. **Disambiguate** if more than one v79 candidate matches: choose by a unique callee or a surrounding string the real one references — **never** by address proximity to v83 alone (v79↔v83 codegen gap is larger than v83↔v84). Record the ambiguity and the reason for the choice.
5. **Label the v79 IDB**: `mcp__ida-pro__rename(addr=<v79>, name=<canonical v83 name verbatim>)`, and `mcp__ida-pro__set_type(...)` where the prototype is known. Apply the v83 canonical name verbatim so cross-version greps align. (Labeling functions/globals only — do NOT apply struct types.)
6. **Capture v79-vs-v83 drift (FR-14).** If the working anchor differs from the v83 reference (a literal, a constant, a vtable slot index that shifted in the older build), record the **v79-specific form** in the catalog row so the next (older) backward port benefits.
7. **Write the cmake line** (SP-3) and **the catalog row** (SP-4), and flip the key's status in `memory-map.md` from `☐` to `✔` (`◐` if located+labeled but catalog/cmake not yet written).

### SP-2 — Resolve one instruction-relative offset key

Offset keys (`*_OFFSET`, `WIN_MAIN_AD_BALLOON_CONDITIONAL`, `WIN_MAIN_PATCHER_OFFSET`, `C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET`) are **not** addresses and are **never copied from v83** (R6, FR-5):

1. Identify the offset's host function in v79 (usually already resolved as an absolute key in the same cluster).
2. `mcp__ida-pro__disasm` the host function in v79.
3. Find the target instruction/branch the offset points to. Read the **v83** host at the corresponding v83 offset first, to know *what* you are measuring (the call site, conditional, or patch point the edit patches).
4. Compute the byte delta from the host function's base (or the documented reference instruction) to that target instruction in v79. That delta is the v79 offset value.
5. Record in the catalog (SP-4) the identified target instruction plus the measured delta. Write the cmake line (SP-3) and flip status.

### SP-3 — cmake line format

Edit `memory_maps/GMS/v79_1.cmake` in place (seeded from v83 in Task 1, so the line already exists with the v83 value). Replace the value, preserving key name and any still-accurate trailing comment:

```cmake
set(C_CLIENT_SOCKET_SEND_PACKET 0x00XXXXXX)
```

- Use uppercase `0x` hex matching the file's existing style.
- Where a v83 key carried a clarifying comment (`# does not exist`, `# JMS only`), update it to reflect the **v79** finding (FR-8). A carried-forward sentinel keeps an accurate `# …` note.

### SP-4 — signature-catalog row

Append/update one section per resolved function in `signature-catalog.md`, using the file's existing schema:

```
### <CANONICAL_FUNCTION_NAME>   (memory-map key: <KEY>)
- Primary anchor: <string xref | import call | call-graph | constant | vtable slot>
- Detail: <the exact literal / API / parent / constant / slot index used>
- Fallback anchor: <secondary method>
- Cross-version stability: <which versions confirmed; v79-vs-v83 drift if any>
- v79 address: <0x… (named in IDB)>
- Notes: <ambiguity, near-duplicates, disambiguation reason>
```

Phrase entries about the *function*, not a specific address (the catalog is version-agnostic and reused for the next, older port — FR-14). Where a v83 anchor drifted in v79, record the v79-specific form (SP-1 step 6).

### SP-5 — Sentinel confirmation, both directions (FR-7, R4)

For a sentinel key (v83 value `0x00000000`, or a "does not exist" key), absence is **confirmed, not assumed**:

1. Identify the feature's own anchor (its strings, its `CreateInstance` call site, its vtable) from a version where it exists (v83 for GMS-absent features; JMS185 for JMS-only features — confirm the positive case there first).
2. Search v79 for that anchor.
3. If **absent** → carry `0x00000000` forward with an accurate `# …` comment and record "confirmed absent + how" in the catalog.
4. If **present** → resolve as a real address per SP-1, note the surprise in the catalog, and (if it backs a sentinel-gated edit) flag it for the user.

**Backward direction (new v79-only sentinels).** Additionally, for each v83 *real-address* key, stay alert: if the backing feature does not exist in v79, that key becomes a **new v79-only sentinel**. Carry `0x00000000` with a `# absent in v79` comment, and **flag it for the gate/edit owner** in the task report (the consuming gate/edit must tolerate `0`). Do not silently zero a previously-real key without flagging.

### SP-6 — `idb_save` checkpoint

After each cluster task's labeling is complete, `mcp__ida-pro__idb_save` on the v79 IDB (after confirming target via `get_metadata`) so labels survive an MCP swap/restart (FR-10).

---

## Task 1: Setup — branch, seed cmake, reconcile the live key set, baseline IDB

**Files:**
- Create: `memory_maps/GMS/v79_1.cmake`
- Modify: `docs/tasks/task-008-gms-v79-support/memory-map.md` (tracking table stays the live source of truth)

- [ ] **Step 1: Create the feature branch**

```bash
git checkout -b task-008-gms-v79-support
```

- [ ] **Step 2: Seed the v79 memory map from v83**

Copy `memory_maps/GMS/v83_1.cmake` to `memory_maps/GMS/v79_1.cmake` verbatim. This guarantees all keys are present and non-empty from the start (so `GenerateMemoryMap` never fails mid-port) and gives each key a documented v83 value to relocate from. Prepend a header comment to the new file:

```cmake
# GMS v79.1 memory map. Seeded from v83_1.cmake; every value is relocated and
# re-verified against the v79 binary per docs/tasks/task-008-gms-v79-support/.
# v79 sits BELOW the lowest previously-supported version (83): a value still equal
# to v83 means UNVERIFIED unless its tracking-table row in memory-map.md is marked
# ✔ with a signature-catalog entry. There is no labeled IDB below v79 to
# triangulate against — confirm every value by signature, never by proximity.
```

- [ ] **Step 3: Reconcile the live key set (pin the count)**

The validator `cmake/CheckMemoryMapKeys.cmake` already exists (created by the v84 task). Confirm the live key count and that the seed satisfies it:

```bash
grep -oE '@[A-Z_0-9]+@' include/memory_map.h.in | sed 's/@//g' | sort -u | grep -v '^BUILD_REGION$' | wc -l
cmake -DREGION=GMS -DMAJOR=79 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake
```

Expected: the `wc -l` prints `159`, and the validator prints `OK: all 159 keys defined and non-empty for GMS v79.1`.
**If the count is not 159**, stop — reconcile `memory-map.md`'s tracking table against the live header before proceeding (a key was added/removed since this plan was written). Any new key not in the table must be added to the table (and resolved in the matching cluster task). Record the confirmed count in the task report.

- [ ] **Step 4: Baseline the v79 IDB**

Confirm the target IDB is reachable and is actually v79 (lane discipline):
- `mcp__ida-pro__list_instances` — find the port whose binary is `GMS_v79_1_DEVM.exe`.
- `mcp__ida-pro__select_instance(port=<resolved>)` then `mcp__ida-pro__get_metadata` — record the image base and module name in `signature-catalog.md` notes (anchors WinMain offset math later).
- `mcp__ida-pro__idb_save` to take a clean baseline.

- [ ] **Step 5: Commit**

```bash
git add memory_maps/GMS/v79_1.cmake docs/tasks/task-008-gms-v79-support/
git commit -m "feat(v79): scaffold GMS v79.1 memory map seeded from v83"
```

---

## Task 2: Memory map — WinMain + CWvsApp + window-manager cluster (~26 keys)

Resolve first: the entry point anchors the whole image and gives call-graph reach into the rest (README "Adding a new version"; design §3.3 cluster 1). This cluster also produces the disassembly the **early Category-A amendment** (Task 3) consumes, so it additionally locates the `CFuncKeyMappedMan` class/vtable far enough to size it.

**Files:** `memory_maps/GMS/v79_1.cmake`, `docs/tasks/task-008-gms-v79-support/signature-catalog.md`, `memory-map.md`.

**Keys (absolute unless noted):** `WIN_MAIN`, `SEND_HS_LOG`, `C_WVS_APP`, `C_WVS_APP_INSTANCE_ADDR`, `C_WVS_APP_IS_MSG_PROC`, `C_WVS_APP_INITIALIZE_AUTH`, `C_WVS_APP_INITIALIZE_PCOM`, `C_WVS_APP_CREATE_MAIN_WINDOW`, `C_WVS_APP_CONNECT_LOGIN`, `C_WVS_APP_INITIALIZE_RES_MAN`, `C_WVS_APP_INITIALIZE_GR2D`, `C_WVS_APP_INITIALIZE_INPUT`, `C_WVS_APP_INITIALIZE_SOUND`, `C_WVS_APP_INITIALIZE_GAME_DATA`, `C_WVS_APP_CREATE_WND_MANAGER`, `C_WVS_APP_GET_CMD_LINE`, `C_WVS_APP_DIR_BACK_SLASH_TO_SLASH`, `C_WVS_APP_DIR_UP_DIR`, `C_WVS_APP_DIR_SLASH_TO_BACK_SLASH`, `C_WVS_APP_GET_EXCEPTION_FILE_NAME`, `C_WVS_APP_CALL_UPDATE`, `C_WVS_APP_RUN`, `C_WVS_APP_SET_UP`, `C_WND_MAN_S_UPDATE`, `C_WND_MAN_REDRAW_INVALIDATED_WINDOWS`, `G_DW_TARGET_OS` (global).
**Offset keys (SP-2):** `WIN_MAIN_AD_BALLOON_CONDITIONAL` (v83 `0xA3D`), `WIN_MAIN_PATCHER_OFFSET` (v83 `0x212`) — both measured inside `WIN_MAIN`.

- [ ] **Step 1: Anchor WIN_MAIN from the PE entry point**

`WIN_MAIN` is reachable from the PE entry. In v79 (lane discipline) get the entry point (`get_metadata` reports it; or `list_funcs`/`survey_binary`). Confirm via SP-1 using two structural anchors — e.g. the unique startup string literals it references (string xref) **and** its call to `CWvsApp::SetUp`/`Run` (call-graph). High-value → two kinds + `needs-main-review`. The v84 catalog's WinMain entry (IExplorer-title string) should transfer; if the literal drifted in v79, record the v79 form (SP-1 step 6).

- [ ] **Step 2: Resolve C_WVS_APP, C_WVS_APP_RUN, C_WVS_APP_SET_UP (high-value)**

Per SP-1. `C_WVS_APP_RUN` is the main message-pump loop; `C_WVS_APP_SET_UP` is the init driver that calls the `INITIALIZE_*` subsystem functions in a fixed order. Resolve `SET_UP` first, then walk its `callees` to reach the initializers (call-graph anchor for the rest of the cluster). All three are high-value → two structural anchor kinds + `needs-main-review`.

**R11 — DR_init / SetUp init sequence:** while reading `SET_UP`'s `callees`, record the init-call order and note whether any DR/anti-debug init step is present. v79 (like v83) is expected to **lack** the DR subsystem (the v84 freeze, [[project_v84_movement_anticheat_freeze]], came from a DR_init that v79/v83 do not have) — but confirm absence from this disasm; do not assume. This feeds the `DR_INIT`/`DR_CHECK` sentinel confirmation in Task 10.

- [ ] **Step 3: Resolve the remaining CWvsApp keys via call-graph from SET_UP/Run**

For each `C_WVS_APP_INITIALIZE_*`, `CREATE_*`, `GET_CMD_LINE`, the three `DIR_*` helpers, `GET_EXCEPTION_FILE_NAME`, `CALL_UPDATE`, `IS_MSG_PROC`, `CONNECT_LOGIN`: apply SP-1, using the call-graph from `SET_UP`/`Run` as anchor 1 and each function's own string/import/constant as anchor 2. The `DIR_*` helpers reference path-separator chars; `INITIALIZE_AUTH/PCOM/RES_MAN/SOUND/GAME_DATA` each reference distinctive subsystem strings.

- [ ] **Step 4: Resolve the global + window-manager keys + SEND_HS_LOG**

- `C_WVS_APP_INSTANCE_ADDR`, `G_DW_TARGET_OS`: globals — find the `mov [g_xxx], eax` store after the relevant ctor/getter returns. Anchor via the function that writes/reads them.
- `C_WND_MAN_S_UPDATE`, `C_WND_MAN_REDRAW_INVALIDATED_WINDOWS`: reachable from the app update loop; SP-1 with call-graph + a distinctive string/constant.
- `SEND_HS_LOG`: anchor via the `%s\HShield` literal (v84 catalog) — a string xref; confirm the literal is present/identical in v79 or record drift.

- [ ] **Step 5: Locate CFuncKeyMappedMan class + size it (feeds Task 3)**

So Task 3 can amend `CFuncKeyMappedMan.h:38` early, locate the `CFuncKeyMappedMan` class in v79 now: find its RTTI/type string (`search_text` for `CFuncKeyMappedMan`/`.?AVCFuncKeyMappedMan@@`) → its vtable (`C_FUNC_KEY_MAPPED_MAN_VFTABLE` candidate). Determine the **v79 size** of `CFuncKeyMappedMan` from a layout anchor (a `Decode`/ctor field-init extent, a `RemoveAll`-style `imul stride,N`, or the allocation size at its `CreateInstance` call). Record the measured v79 size in `struct-verification.md` (row `CFuncKeyMappedMan.h`). Do **not** apply a struct type to the IDB (read-only). Full resolution of the FuncKeyMapped key set happens in Task 7; this step only needs the class anchor + size.

- [ ] **Step 6: Re-derive the two WinMain offsets (SP-2)**

`WIN_MAIN_AD_BALLOON_CONDITIONAL` (v83 `0xA3D`) and `WIN_MAIN_PATCHER_OFFSET` (v83 `0x212`) are byte offsets inside `WIN_MAIN`. Disassemble v79 `WIN_MAIN`, find the ad-balloon conditional branch and the patcher-window call site (read v83 at the v83 offsets first; cross-reference the `no-ad-balloon` and `no-patcher` edits for the exact patch semantics), and measure the v79 deltas. **Never copy 0xA3D / 0x212.**

- [ ] **Step 7: Spot-check the high-value keys independently**

For every key flagged `needs-main-review` in this cluster, re-probe the v79 address from scratch with a *different* anchor than the one originally used and confirm it lands on the same function (R5, design §3.2).

- [ ] **Step 8: Label, checkpoint, validate, commit**

- Label all resolved functions/globals in the v79 IDB (SP-1 step 5).
- `idb_save` (SP-6).
- Run: `cmake -DREGION=GMS -DMAJOR=79 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake` → expect `OK: all 159 keys…`.
- Flip this cluster's rows to ✔ in `memory-map.md`.

```bash
git add memory_maps/GMS/v79_1.cmake docs/tasks/task-008-gms-v79-support/
git commit -m "feat(v79): relocate WinMain + CWvsApp cluster with signatures"
```

---

## Task 3: Early Category-A gate amendment — restore size guards for v79 (FR-12a, D7, R1)

The two known enumerated gates with **no catch-all `#else`** are **`assert_size` chains**, not member declarations:
- `common/CFuncKeyMappedMan.h:38` — `(== 83 || == 84 || == 87)` → `0x3C8` / `:40 == 95` → `0x3CC` / `:42 == 111` → `0x3D0` / `:44 REGION_JMS` → `0x400`. No v79 branch.
- `common/CWvsApp.h:97` — `(== 83 || == 84)` → `0x60` / `:99 == 87` → `0x6C` / `:101 >= 95` → `0x8C` / `:103 REGION_JMS` → `0x64`. No v79 branch.

**Accurate failure mode (refine the PRD's "build break"):** because these are `assert_size` (compile-time `static_assert`) chains, an unmatched v79 simply **fires no assertion** — the struct compiles with its base member layout but loses its size guard. The amendment restores the guard *and* locks v79's measured size. The member layout itself is decided by the `>= 111` / `>= 95` member gates earlier in each header (false for v79 → v79 takes the base members). So the build is **not** hard-blocked by these two sites; the genuine layout-shift risk lives in the *member-declaring* gates audited later (Cat B, Task 12). Amend these two early anyway (D7) — while the CWvsApp/CFuncKeyMappedMan disasm from Task 2 is fresh — and prove the v79 build is actually compilable (Step 4).

**Files:** `common/CFuncKeyMappedMan.h`, `common/CWvsApp.h`, `docs/tasks/task-008-gms-v79-support/struct-verification.md`.

- [ ] **Step 1: Re-grep the live enumerated-no-`#else` gate set**

Line numbers may have shifted, and other no-`#else` `==` gates may exist:

```bash
grep -rn "BUILD_MAJOR_VERSION ==" common/*.h
```

Confirm the two known sites and inspect any other hit. For each hit, classify it: an `assert_size` chain (silent guard loss for v79) vs a member-declaring `#if … #endif` (layout shift — those are Cat B/E, handled in Tasks 12–13, not here). Only the `assert_size` no-`#else` chains are amended in this task.

- [ ] **Step 2: Decide v79's branch for each from measured size (not "oldest ⇒ v83")**

- `CWvsApp` — Task 2 Step 2 resolved `C_WVS_APP`; measure v79 `sizeof(CWvsApp)` from a layout anchor (the SetUp/ctor field-init extent, or the stack-frame size where WinMain stack-constructs it — the header comment notes v83/v84 = `0x60`). Record the v79 size.
- `CFuncKeyMappedMan` — use the v79 size measured in Task 2 Step 5.

For each, the v79 branch is the one whose asserted size **equals the measured v79 size**. If v79 matches the `83/84/87` size (`0x60` / `0x3C8` respectively), add `79` to that branch. If v79's size differs from every existing branch, add a **new** `#elif … == 79` branch with the measured size. Record the deciding evidence (the disasm line + measured size) in `struct-verification.md` rows `CWvsApp.h` / `CFuncKeyMappedMan.h` with verdict `v79-branch-added`.

- [ ] **Step 3: Apply the amendments (keep the house guard form)**

Example — if v79 `CWvsApp` measures `0x60` (matches 83/84):

```c
#if defined(REGION_GMS) && (BUILD_MAJOR_VERSION == 79 || BUILD_MAJOR_VERSION == 83 || BUILD_MAJOR_VERSION == 84)
assert_size(sizeof(CWvsApp), 0x60)
```

Or — if v79 `CWvsApp` measures a distinct size `0xNN`:

```c
#if defined(REGION_GMS) && (BUILD_MAJOR_VERSION == 83 || BUILD_MAJOR_VERSION == 84)
assert_size(sizeof(CWvsApp), 0x60)
#elif defined(REGION_GMS) && BUILD_MAJOR_VERSION == 79
assert_size(sizeof(CWvsApp), 0xNN)
```

Apply the analogous change at `CFuncKeyMappedMan.h:38`. Add a one-line comment at each amended site, e.g. `// v79 size verified task-008`. Use the existing `#if/#elif/#endif` block form; no ternaries.

- [ ] **Step 4: Cross-version truth-table + prove v79 compiles**

- **Truth table (FR-13):** adding `79` to a branch (or adding a new `== 79` branch) must not change any other version's selected branch. Confirm `83/84/87/95/111/JMS185` each still hit their original `assert_size`. (Adding a disjoint `|| == 79` or a new `#elif == 79` cannot flip another version — record the table in `struct-verification.md`.)
- **Empirical build check (refute the "build break" assumption per [[feedback-prefer-confirmation]]):** attempt the v79 build and confirm whether anything was *actually* build-breaking before/after. Prefer `scripts/wsl-build.sh` for GMS 79.1 ([[project_wsl_cross_compile]]). If a full build is too slow at this stage, at minimum preprocess the two headers for v79 and a neighbor and confirm the intended `assert_size` now fires:

```bash
echo '#include "CWvsApp.h"' > /tmp/_gatecheck.cpp
gcc -fsyntax-only -DREGION_GMS -DBUILD_MAJOR_VERSION=79 -DBUILD_MINOR_VERSION=1 -I common -I include /tmp/_gatecheck.cpp 2>&1 | head
```

Record in the task report whether the v79 build was genuinely blocked, or merely missing size guards (the expected finding). State what was actually run.

- [ ] **Step 5: Commit**

```bash
git add common/CFuncKeyMappedMan.h common/CWvsApp.h docs/tasks/task-008-gms-v79-support/struct-verification.md
git commit -m "fix(v79): add v79 branch to CWvsApp/CFuncKeyMappedMan size gates (Cat A)"
```

---

## Task 4: Memory map — CClientSocket / ZSocket cluster (~14 keys)

Highest-value for the `redirect`/`bypass` edits (design §3.3 cluster 2).

**Files:** `memory_maps/GMS/v79_1.cmake`, `signature-catalog.md`, `memory-map.md`.

**Keys:** `C_CLIENT_SOCKET_INSTANCE_ADDR`, `C_CLIENT_SOCKET_CREATE_INSTANCE`, `C_CLIENT_SOCKET_SEND_PACKET`, `C_CLIENT_SOCKET_FLUSH`, `C_CLIENT_SOCKET_MANIPULATE_PACKET`, `C_CLIENT_SOCKET_PROCESS_PACKET`, `C_CLIENT_SOCKET_CLOSE`, `C_CLIENT_SOCKET_CLEAR_SEND_RECEIVE_CTX`, `C_CLIENT_SOCKET_ON_CONNECT`, `C_CLIENT_SOCKET_CONNECT_LOGIN`, `C_CLIENT_SOCKET_CONNECT_CTX`, `C_CLIENT_SOCKET_CONNECT_ADR`, `Z_SOCKET_BASE_CLOSE_SOCKET`, `Z_SOCKET_BUFFER_ALLOC`.

- [ ] **Step 1: Anchor the connect path via the `connect`/`socket` imports**

In v79 (lane discipline), `imports_query` for `connect` / `socket` (WS2_32). `xrefs_to` them reach `C_CLIENT_SOCKET_ON_CONNECT` / `CONNECT_ADR` / `CONNECT_CTX` / `CONNECT_LOGIN`. Import anchor (kind 2). Disambiguate the four connect variants by their distinct callers/strings.

- [ ] **Step 2: Resolve send/flush/process/manipulate (high-value)**

`SEND_PACKET`, `FLUSH`, `PROCESS_PACKET`, `MANIPULATE_PACKET` are high-value → SP-1 with two structural anchor kinds each → `needs-main-review`. Anchor via call-graph from `CClientSocket` getInstance / the connect path + each function's distinctive structure (e.g. flush references the send buffer). The v83 functions are clustered (0x49637B…0x4965F1) — use call-graph adjacency in v79, **not** v83 address arithmetic.

- [ ] **Step 3: Note for the §5.5 audit — does ON_CONNECT encode an 8-byte client key?**

While reading `C_CLIENT_SOCKET_ON_CONNECT` / the connect-hello send path, record whether v79 encodes an **8-byte client key** (`EncodeBuffer(m_aClientKey, 8)`). This is the same fact as the `CWvsContext.h:98` `> 83` gate (Task 13 / §5.5 special case) and `bypass/socket_hooks.cpp:233`. The gate predicts v79 (< 83) has the **no-key** form; capture the disasm evidence here so Task 13 can confirm it without re-deriving.

- [ ] **Step 4: Resolve the remaining socket keys**

`CLOSE`, `CLEAR_SEND_RECEIVE_CTX`, `CREATE_INSTANCE`, `INSTANCE_ADDR` (global store after CreateInstance), `Z_SOCKET_BASE_CLOSE_SOCKET`, `Z_SOCKET_BUFFER_ALLOC` — SP-1, two anchors each.

- [ ] **Step 5: Spot-check `needs-main-review`, label, checkpoint, validate, commit**

Same as Task 2 Steps 7–8.

```bash
git commit -am "feat(v79): relocate CClientSocket/ZSocket cluster with signatures"
```

---

## Task 5: Memory map — COutPacket cluster (~7 keys)

Encode primitives used to craft packets; high-value (design §3.3 cluster 3).

**Files:** `memory_maps/GMS/v79_1.cmake`, `signature-catalog.md`, `memory-map.md`.

**Keys:** `C_OUT_PACKET`, `C_OUT_PACKET_ENCODE_1`, `C_OUT_PACKET_ENCODE_2`, `C_OUT_PACKET_ENCODE_4`, `C_OUT_PACKET_ENCODE_STR`, `C_OUT_PACKET_ENCODE_BUFFER`, `C_OUT_PACKET_MAKE_BUFFER_LIST`.

- [ ] **Step 1: Resolve the encoders by width signature + call-graph**

`ENCODE_1/2/4` differ by the byte-width they append (1/2/4-byte store/grow). SP-1: anchor 1 = the width-specific store/grow pattern (constant/structure), anchor 2 = called-by from a known packet sender or the shared buffer-grow callee. All high-value → two structural anchors + `needs-main-review`.

- [ ] **Step 2: Resolve ENCODE_STR, ENCODE_BUFFER, MAKE_BUFFER_LIST, C_OUT_PACKET**

`ENCODE_STR` references string-length handling; `ENCODE_BUFFER` takes a length arg and memcpy-grows; `MAKE_BUFFER_LIST` and `C_OUT_PACKET` (ctor/base) anchor via call-graph among the COutPacket methods + a distinctive constant/string. SP-1, two anchors each.

- [ ] **Step 3: Spot-check, label, checkpoint, validate, commit**

Same as Task 2 Steps 7–8.

```bash
git commit -am "feat(v79): relocate COutPacket cluster with signatures"
```

---

## Task 6: Memory map — Login / Stage / Logo / Title cluster (~22 keys)

Login & stage flow (design §3.3 cluster 4).

**Files:** `memory_maps/GMS/v79_1.cmake`, `signature-catalog.md`, `memory-map.md`.

**Keys:** `C_LOGIN_UPDATE`, `C_LOGIN_SEND_CHECK_PASSWORD_PACKET`, `C_LOGO`, `C_LOGO_GET_RTTI`, `C_LOGO_IS_KIND_OF`, `C_LOGO_UPDATE`, `C_LOGO_ON_MOUSE_BUTTON`, `C_LOGO_ON_SET_FOCUS`, `C_LOGO_ON_KEY`, `C_LOGO_LOGO_END`, `C_LOGO_FORCED_END`, `C_LOGO_INIT`, `C_LOGO_INIT_NX_LOGO`, `STAGE_INSTANCE_ADDR`, `SET_STAGE`, `C_STAGE_ON_MOUSE_ENTER`, `C_STAGE_ON_PACKET`, `GR_INSTANCE_ADDR`, `C_UI_TITLE_INSTANCE_ADDR`.

- [ ] **Step 1: Resolve CLogo via its RTTI/vtable**

`C_LOGO_GET_RTTI`, `C_LOGO_IS_KIND_OF`, and the `C_LOGO` vtable cluster: find the `CLogo` RTTI/type string in v79 (`search_text` for `CLogo`/`.?AVCLogo@@`), reach its vtable, index the slots for `GetRTTI`/`IsKindOf`/`OnSetFocus`/`OnKey`/`OnMouseButton`/`Update`. Vtable slot is kind 5; confirm each with a second anchor (the method body's distinctive content). Match **slot order** against v83's vtable, not v83 addresses — and note any slot-index drift in v79 (SP-1 step 6).

- [ ] **Step 2: Resolve CLogo lifecycle + NX logo**

`C_LOGO_INIT`, `C_LOGO_INIT_NX_LOGO`, `C_LOGO_LOGO_END`, `C_LOGO_FORCED_END`: `INIT_NX_LOGO` references the NX-logo resource string (string xref). SP-1 two anchors each.

- [ ] **Step 2 note:** `C_LOGO_UPDATE` and `C_LOGIN_UPDATE` share the v83 address `0x005F4C16`. Confirm whether they are still the same function in v79 (one shared Update) or diverged; record the determination in the catalog.

- [ ] **Step 3: Resolve Login keys**

`C_LOGIN_SEND_CHECK_PASSWORD_PACKET` references the password-check send path (opcode immediate + COutPacket use — anchor via the opcode constant and a call to a resolved encoder). `C_LOGIN_UPDATE` per Step 2 note. SP-1.

- [ ] **Step 4: Resolve Stage + globals + title**

`SET_STAGE`, `C_STAGE_ON_MOUSE_ENTER`, `C_STAGE_ON_PACKET` via CStage vtable/strings; `STAGE_INSTANCE_ADDR`, `GR_INSTANCE_ADDR`, `C_UI_TITLE_INSTANCE_ADDR` are globals (store-after-CreateInstance pattern). SP-1 two anchors each.

- [ ] **Step 5: Spot-check, label, checkpoint, validate, commit**

Same as Task 2 Steps 7–8.

```bash
git commit -am "feat(v79): relocate Login/Stage/Logo/Title cluster with signatures"
```

---

## Task 7: Memory map — Manager singletons cluster (~37 keys)

`*_CREATE_INSTANCE` / `*_INSTANCE_ADDR` pairs + their methods. These cluster in the static-init region and are called in a fixed order from one initializer — find one, the neighbors are adjacent calls (design §3.3 cluster 5). `idb_save` partway through this large cluster.

**Files:** `memory_maps/GMS/v79_1.cmake`, `signature-catalog.md`, `memory-map.md`.

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

In v79 (lane discipline), locate the static-init function that calls the `*::CreateInstance` chain (anchor from an already-resolved `CreateInstance` in Task 2's reach, or a distinctive manager string). The `CREATE_INSTANCE` keys are adjacent `call` sites; the matching `*_INSTANCE_ADDR` globals are the `mov [g_xxx], eax` stores right after each call returns. Resolve each pair together with SP-1.

- [ ] **Step 2: Resolve FuncKeyMapped/quickslot using Task 2's class anchor**

`C_FUNC_KEY_MAPPED_MAN_VFTABLE` was located in Task 2 Step 5 — finish `C_FUNC_KEY_MAPPED_MAN` (ctor/base), `_INSTANCE_ADDR`, `_CREATE_INSTANCE`, `DEFAULT_FKM_INSTANCE_ADDR`/`DEFAULT_QKM_INSTANCE_ADDR` (default-instance globals), `C_QUICKSLOT_KEY_MAPPED_MAN`. SP-1 two anchors each.

- [ ] **Step 3: Resolve manager method keys via the manager class**

For each manager's methods (`_INIT`, `_SWEEP_CACHE`, `_LOAD_BOOK`, `_LOAD_DEMAND`, `_LOAD_PARTY_QUEST_INFO`, `_LOAD_EXCLUSIVE`, `_UPDATE_DEVICE`, `_GET_IS_MESSAGE`, `_GENERATE_AUTO_KEY_DOWN`, `_SHOW_CURSOR`, `_ON_PACKET`): anchor via the resolved class instance/vtable + a distinctive string/constant (e.g. `LOAD_BOOK` references the monster-book data path string). SP-1 two anchors each.

- [ ] **Step 4: Resolve the Radio quirk carefully**

`C_RADIO_MANAGER_INSTANCE_ADDR` — the v84 port found v83's value pointed at the **allocator selector**, not the real instance global. Do **not** inherit the v83 address: verify the real v79 instance global (the `mov [g_xxx], eax` after `CRadioManager::CreateInstance` returns). Record the finding (and flag if the v83 map's value is wrong) in the catalog and the task report.

- [ ] **Step 5: idb_save checkpoint mid-cluster**

After the singletons + globals are labeled (before the long method tail), `idb_save` (confirm target via `get_metadata`).

- [ ] **Step 6: Spot-check boundary-adjacent keys, label, checkpoint, validate, commit**

`C_SECURITY_CLIENT_ON_PACKET` is boundary-adjacent (security_hooks gating) → `needs-main-review`. Then Task 2 Steps 7–8.

```bash
git commit -am "feat(v79): relocate manager-singleton cluster with signatures"
```

---

## Task 8: Memory map — Config / SystemInfo / IGCipher / utilities cluster (~19 keys)

(design §3.3 cluster 6).

**Files:** `memory_maps/GMS/v79_1.cmake`, `signature-catalog.md`, `memory-map.md`.

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
- `C_MOB_C_MOB` — the `CMob::CMob` constructor (doom-fix hook target; boundary-adjacent, see Task 15) → `needs-main-review`. Anchor via the CMob vtable assignment + CMobTemplate use. **Record whether the ctor leaves `m_bDoomReserved` uninitialized** (feeds the doom-fix gate audit).

SP-1 two anchors each.

- [ ] **Step 3: Spot-check, label, checkpoint, validate, commit**

Same as Task 2 Steps 7–8.

```bash
git commit -am "feat(v79): relocate Config/SystemInfo/utilities cluster with signatures"
```

---

## Task 9: Memory map — Party / migrate / context senders + offsets (~9 keys)

(design §3.3 cluster 7). Packet senders reference their opcode as a `push <imm>` into a COutPacket — the opcode immediate disambiguates similar senders.

**Files:** `memory_maps/GMS/v79_1.cmake`, `signature-catalog.md`, `memory-map.md`.

**Keys:** `C_FIELD_SEND_JOIN_PARTY_MSG`, `C_FIELD_SEND_CREATE_NEW_PARTY_MSG`, `C_WVS_CONTEXT_INSTANCE_ADDR`, `C_WVS_CONTEXT_ON_ENTER_GAME`, `C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST`.
**Offset keys (SP-2):** `C_FIELD_SEND_JOIN_PARTY_MSG_OFFSET` (v83 `0x65`), `C_FIELD_SEND_CREATE_NEW_PARTY_MSG_OFFSET` (v83 `0xA4`), `C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET` (v83 `0x10`), `C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST_OFFSET` (v83 `0xE9`).

- [ ] **Step 1: Resolve the senders (high-value) via opcode immediate**

`C_FIELD_SEND_JOIN_PARTY_MSG`, `C_FIELD_SEND_CREATE_NEW_PARTY_MSG`, `C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST`: each builds a COutPacket with a specific opcode immediate (anchor 1 = the opcode `push`/`mov`), and calls a resolved encoder + the socket send (anchor 2 = call-graph to Tasks 4/5 functions). High-value → `needs-main-review`. Disambiguate the two party senders by their distinct opcodes. **Older-build note:** the opcodes may have drifted in v79 (R7) — read the immediate, don't assume the v83 value; record any drift.

- [ ] **Step 2: Resolve C_WVS_CONTEXT keys**

`C_WVS_CONTEXT_ON_ENTER_GAME` (enter-game handler) and `C_WVS_CONTEXT_INSTANCE_ADDR` (global). SP-1 two anchors each.

- [ ] **Step 3: Re-derive all four offsets (SP-2)**

Each `*_OFFSET` is a call-site/branch offset inside its host sender/handler. Disassemble each v79 host, find the patch-point instruction (read the v83 host at the v83 offset to know which instruction — the offsets correspond to the call-site the party/migrate/redirect edits patch), and measure the v79 delta. **Never copy 0x65 / 0xA4 / 0x10 / 0xE9.**

- [ ] **Step 4: Spot-check, label, checkpoint, validate, commit**

Same as Task 2 Steps 7–8.

```bash
git commit -am "feat(v79): relocate party/migrate/context senders + offsets"
```

---

## Task 10: Memory map — protocol constants + sentinels + exception-dispatch (~23 keys)

Resolved last (FR-7; design §3.3 cluster 8). Includes the exception-dispatch + CFileStream keys added after the v84 port.

**Files:** `memory_maps/GMS/v79_1.cmake`, `signature-catalog.md`, `memory-map.md`.

**Constant keys:** `VERSION_HEADER` (v83 `8`), `PLAYER_LOGGED_IN` (v83 `0x14`), `CLIENT_START_ERROR` (v83 `0x19`).
**Exception-dispatch keys (absolute):** `C_TI_DISCONNECT_EXCEPTION`, `C_TI_TERMINATE_EXCEPTION`, `C_TI_PATCH_EXCEPTION`, `C_TI_ZEXCEPTION`, `C_PATCH_EXCEPTION_BUILDER`, `C_COM_RAISE_ERROR_EX`.
**CFileStream keys:** `C_FILE_STREAM_RESOLVED` (gate, v83 `0`), `C_FILE_STREAM_OPEN_INLINE` (gate, v83 `0`), `C_FILE_STREAM_OPEN`, `C_FILE_STREAM_GET_LENGTH`, `C_FILE_STREAM_READ`, `C_FILE_STREAM_CLOSE`, `C_FILE_STREAM_VFTABLE` (all v83 `0x0`).
**GMS-absent sentinels:** `C_BATTLE_RECORD_MAN_CREATE_INSTANCE`, `DR_CHECK`, `DR_INIT`, `CE_TRACER_RUN`, `RESET_LSP` (v83 carried `0x0044ED47` with a stale "does not exist" comment).
**JMS-only sentinels:** `C_SECURITY_CLIENT_ON_PACKET_RET_STUB`, `C_SECURITY_CLIENT_ON_PACKET_CHECK`, `C_SECURITY_CLIENT_ON_PACKET_CHECK_OFFSET`, `C_WVS_APP_INITIALIZE_GR2D_WINDOWED_OFFSET`, `WIN_MAIN_LAUNCHER_STUB`.

- [ ] **Step 1: Confirm the three protocol constants against v79 (FR-6, R7, PRD §9)**

Do **not** copy v83 values — v79 predates v83, so opcode/handler-table drift is plausible. In v79 (lane discipline):
- `VERSION_HEADER` — the version word the client sends in its hello/login handshake. Find it in the connect/login send path (resolved Tasks 4/6). Confirm `8` or record the v79 value.
- `PLAYER_LOGGED_IN` / `CLIENT_START_ERROR` — send opcodes. Locate the handler/sender that uses each (login flow), read the immediate, confirm `0x14` / `0x19` or record the v79 value.
- If any differ, use the v79 value and note the delta in the catalog (affects `redirect`/`bypass`). Cross-check against atlas-ms's version-aware registries where applicable ([[reference_atlas_ms]]).

- [ ] **Step 2: Resolve the exception-dispatch keys (SP-1)**

`C_TI_DISCONNECT_EXCEPTION`, `C_TI_TERMINATE_EXCEPTION`, `C_TI_PATCH_EXCEPTION`, `C_TI_ZEXCEPTION` are the TException-family vtables/handlers; `C_PATCH_EXCEPTION_BUILDER` and `C_COM_RAISE_ERROR_EX` are the builder/raise helpers. Anchor via their RTTI/type strings + call-graph (the CLIENT_START_ERROR relay path, task-007). SP-1 two anchors each. These exist in v83 — expect them present in v79; confirm by anchor, don't assume.

- [ ] **Step 3: Decide the v79 CFileStream relay disposition (FR-8a)**

v83 gates the CFileStream relay **off** (`C_FILE_STREAM_RESOLVED 0`, helpers `0x0`). Determine for v79 whether the stream helpers (`CFileStream::Open`/`GetLength`/`Read`/`Close` + vftable) are **recoverable** (locatable by anchor) or should carry `0` like v83:
- Search v79 for the `CFileStream` RTTI/vtable + the `CreateFileA` call inside its `Open`.
- If recoverable → resolve each as a real address (SP-1), set `C_FILE_STREAM_RESOLVED 1`, and set `C_FILE_STREAM_OPEN_INLINE` per whether v79 inlines `CreateFileA` (1) or has an out-of-line `Open` (0).
- If not cleanly recoverable → carry `0` / `0x0` like v83, with a `# unrecoverable in v79, relay gated off` comment. Document the decision in the catalog either way.

- [ ] **Step 4: Confirm GMS-absent sentinels, both directions (SP-5)**

- `C_BATTLE_RECORD_MAN_CREATE_INSTANCE` (CBattleRecordMan — post-v83 feature), `DR_CHECK`, `DR_INIT` (DR anti-debug subsystem), `CE_TRACER_RUN` (CETracer): search v79 for each feature's anchor. Expect **absent** (these post-date v83 or are GMS-absent in v83). Carry `0x00000000` + comment, record "confirmed absent + how". Cross-reference the R11 finding from Task 2 Step 2 for `DR_INIT`/`DR_CHECK`.
- `RESET_LSP`: v83 had a real address (`0x0044ED47`) under a stale "does not exist" comment; the v84 port found it present. Determine the v79 disposition **explicitly** — locate the LSP-reset function in v79 (SP-1) if it exists, else carry a sentinel — and document which. Do not inherit the stale comment.
- **Backward direction:** if any v83 *real-address* key resolved in Tasks 2–9 turned out **absent** in v79, it is a new v79-only sentinel — confirm it is recorded with a `# absent in v79` comment and flagged for the gate/edit owner (SP-5 backward).

- [ ] **Step 5: Confirm JMS-only sentinels (SP-5)**

The five JMS-only keys: confirm the positive case in JMS185 (lane discipline) so the anchor is real, then show it absent in v79. Carry `0x00000000` with `# JMS only` for the GMS v79 build.

- [ ] **Step 6: Label (where applicable), checkpoint, validate, commit**

Constants need no IDB label; any sentinel/relay found present gets labeled. `idb_save`. Run the validator (expect `OK: all 159 keys…`). Flip rows to ✔.

```bash
git commit -am "feat(v79): confirm protocol constants, exception-dispatch, CFileStream + sentinels"
```

---

## Task 11: Memory map completeness gate

**Files:** `docs/tasks/task-008-gms-v79-support/memory-map.md`, `signature-catalog.md`.

- [ ] **Step 1: Tracking-table audit — zero ☐/◐ remaining**

Open `memory-map.md`. Confirm **every** row is marked `✔`. Any remaining `☐`/`◐` is unfinished — return to the owning cluster task. Count the ✔ rows and assert it equals the live key count (159) minus any keys the table tracks as a single grouped row; reconcile against the validator's key list if the numbers differ.

- [ ] **Step 2: Catalog coverage — every absolute key has a row**

Confirm `signature-catalog.md` has one section per absolute-address key with two anchors recorded (two **structural** anchors for the high-value tier). Every offset key has its measured target instruction + delta. Every sentinel has its absence/presence determination (both directions). Every v79-vs-v83 drift is captured (FR-14).

- [ ] **Step 3: Final IDB save**

Lane discipline → v79 → `get_metadata` → `idb_save`. The v79 IDB now carries labels for all resolved functions/globals (FR-9/FR-10).

- [ ] **Step 4: Final completeness validation + commit**

Run: `cmake -DREGION=GMS -DMAJOR=79 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake`
Expected: `OK: all 159 keys defined and non-empty for GMS v79.1`

```bash
git add docs/tasks/task-008-gms-v79-support/
git commit -m "docs(v79): memory map complete — 159/159 keys verified + catalogued"
```

---

## Task 12: Struct audit — below-floor boundary gates (verify FIRST — R1/R2)

The gates whose **truth value changes at the 79 boundary** — the highest-risk subset. Read-only over raw disassembly: use `mcp__ida-pro__disasm`, **not** `decompile`; **never** apply a struct type (R9). Confirm the connected IDB with `get_metadata` first. The Category-A *size* gates were already amended in Task 3; this task handles the **member-declaring** floor gates (Cat B/C) and the doom-field, which are the genuine layout-shift risks.

**Files (evidence only):** `docs/tasks/task-008-gms-v79-support/struct-verification.md`.

The boundary gates (from `struct-verification.md`):

| Site | Gate | Category | v79 lands | Must confirm |
|---|---|---|---|---|
| `common/CUIToolTip.h:92` | `>= 83 \|\| JMS` | B | excluded (NEW) | Does v79 carry `m_pLayerAdditional` (this+0x14, after `m_pLayer` this+0x10)? |
| `common/CMob.h:229` | `>= 84` | C | excluded (like v83) | v84+ field genuinely absent in v79? |
| `common/CMob.h:233` | `>= 84 \|\| JMS` | C | excluded | v84+ field genuinely absent in v79? |
| `common/CMapLoadable.h:154` | `>= 84 \|\| JMS` | C | excluded | v84+ field genuinely absent in v79? |
| `common/CUIToolTip.h:125` | `>= 84 \|\| JMS` | C | excluded | v84+ field genuinely absent in v79? |
| `common/CUIToolTip.h:152` | `>= 84 \|\| JMS` | C | excluded | v84+ field genuinely absent in v79? |
| `common/CMob.h:110` | `< 95` | D | included (base) | v79 CMob base layout == v83 (size + doom region) |
| `doom-fix/dllmain.cpp` | `< 84` form | D | needs-fix side | Does v79 `CMob::CMob` also skip `m_bDoomReserved` (so doom-fix is genuinely needed)? |

- [ ] **Step 1: Re-grep the live gate set (line numbers may have shifted)**

```bash
grep -n "BUILD_MAJOR_VERSION" common/CUIToolTip.h common/CMob.h common/CMapLoadable.h
grep -rn "BUILD_MAJOR_VERSION" doom-fix/
```

Reconcile against the table above; update `struct-verification.md` if a line moved.

- [ ] **Step 2: Category B — CUIToolTip `m_pLayerAdditional` (`>= 83`, v79 newly excluded)**

In v79, disassemble a `CUIToolTip` method that accesses the layer members (after `m_pLayer` at this+0x10). Determine whether v79 carries `m_pLayerAdditional` at this+0x14. v84 confirmed it present 83→111.
- If **v79 has it** → the `>= 83` gate wrongly excludes v79: the fix is to lower the floor to `>= 79` (keep `|| JMS`), applied in Task 17. Record verdict `floor-lowered`.
- If **v79 lacks it** → the exclusion is correct; record verdict `unchanged` with evidence.

- [ ] **Step 3: Category C — the five `>= 84` fields (v79 sides with v83, excluded)**

For `CMob.h:229`, `:233`, `CMapLoadable.h:154`, `CUIToolTip.h:125`, `:152`: confirm each gated v84+ field is genuinely **absent** in v79 (very likely — v83 also lacks it, but verify). For each, disasm a method accessing the gated region and show the field is absent at the expected offset (the struct size matches the no-field layout). Record verdict `unchanged` + the deciding disasm line per field.

- [ ] **Step 4: Category D — CMob base layout + doom-field**

- `CMob.h:110` (`< 95`, true for v79 → base/included): confirm v79 `CMob` base layout matches v83 (size + the doom-field region). Anchor v79 `sizeof(CMob)` (a `Decode`/`RemoveAll` `imul stride,N` / dtor extent).
- doom-fix: from `C_MOB_C_MOB` (Task 8 Step 2), confirm whether v79's `CMob::CMob` ctor leaves `m_bDoomReserved` uninitialized (the bug doom-fix patches). The doom-fix gate (`< 84` form) puts v79 on the **needs-fix** side — confirm that is correct (v79 ctor skips the field). Record verdict.

- [ ] **Step 5: Record verdicts + main-session cross-check**

Write each verdict (gate correct / needs change) + the deciding disasm line into `struct-verification.md`. Before treating any "needs change" verdict as final, re-anchor it with one independent probe (design §5.2). **Do not edit source yet** — gate rewrites are applied in Task 17 against the full picture.

```bash
git add docs/tasks/task-008-gms-v79-support/struct-verification.md
git commit -m "docs(v79): below-floor boundary-gate audit (Cat B/C/D)"
```

---

## Task 13: Struct audit — core/net layout headers, exhaustive size (Cat D/E + special cases)

Read-only `disasm` only; no struct-type application (R9). Per **D5**, record the **v79 size** of every header even when it "should" match v83 — there is no IDB below v79 to catch a silent base-size delta (R3). Use the size-finding techniques in `struct-verification.md`: a `Decode`/`DecodeBuffer(this, N)` literal = struct size; `ZArray<T>::RemoveAll` `imul stride,N` = `sizeof(T)`; destructor unwind extent bounds the layout.

**Files:** `docs/tasks/task-008-gms-v79-support/struct-verification.md`.
**Headers (7):** `common/CWvsApp.h`, `common/CWvsContext.h`, `common/CClientSocket.h`, `common/CLogin.h`, `common/COutPacket.h`, `common/CConfig.h`, `common/ConfigSysOpt.h`.

- [ ] **Step 1: Grep each header's thresholds**

`grep -n BUILD_MAJOR_VERSION` each. Note which change truth value at 79: every `> 83`/`>= 87`/`>= 95`/`>= 111`/`== 83`/`== 95` is **false** for v79 → v79 takes the base ("v83 layout") branch. List each gate's v79 truth value.

- [ ] **Step 2: CWvsContext — `m_aClientKey[8]` (`> 83`, Cat E special)**

`common/CWvsContext.h:98` (`> 83`) is **false for v79**, so the 8-byte `m_aClientKey` block is predicted **absent**. Confirm using the Task 4 Step 3 finding: v79's `CClientSocket::OnConnect` / connect-hello does **not** encode an 8-byte client key. (v84 found `> 83` true → key present; v79 < 83 → the gate predicts the v83 *no-key* form.) This also drives `bypass/socket_hooks.cpp:233` (`> 83`) — the same fact. Record v79 `sizeof(CWvsContext)` + the verdict. Verify the embedded `SecondaryStat` size independently (cross-ref Task 15) — a wrong embedded size shifts every field after it.

- [ ] **Step 3: CLogin — the `== 83` member (`unk3[5]`, Cat E special)**

`common/CLogin.h:235` (`== 83`) declares `int unk3[5]` (20 bytes) between `m_abOnFamily` and `m_lNewEquip`, **only for v83**. For v79 (`== 83` false) it is excluded. But v79 < 83 — does v79 have that member, a different one, or none? Disassemble v79 `CLogin::CLogin` (or a Decode/sizeof-bearing method) and determine the field's presence at the gated offset.
- If **v79 has it** → the gate must become `(== 83 || == 79)` (or `<= 83`), applied in Task 17. Record `rewritten`.
- If **v79 lacks it** → exclusion correct; record `unchanged`.

- [ ] **Step 4: CWvsApp / CClientSocket / COutPacket / CConfig / ConfigSysOpt — size + gated fields**

For each: confirm the `>= 87`/`>= 95`/`>= 111` gated fields are **absent** in v79 (v79 < 87 → base layout), record v79 size + per-gate verdict + disasm anchor. `CWvsApp` size was measured in Task 3 — record it here in the verdict table. `ConfigSysOpt` ties to `C_CONFIG_SYS_OPT_WINDOWED_MODE` (Task 8) — cross-check the windowed-mode field offset.

- [ ] **Step 5: Main-session cross-check + commit**

Re-anchor one boundary case per "needs change" header independently. Commit verdicts.

```bash
git commit -am "docs(v79): struct audit — core/net headers (CWvsApp/CWvsContext/CClientSocket/CLogin/COutPacket/CConfig/ConfigSysOpt)"
```

---

## Task 14: Struct audit — UI/control family, exhaustive size (9 headers)

Read-only `disasm`; no struct-type application. Record v79 size for each (D5).

**Files:** `docs/tasks/task-008-gms-v79-support/struct-verification.md`.
**Headers (9):** `common/CUITitle.h`, `common/CUILoginStart.h`, `common/CUIToolTip.h` (boundary gates done in Task 12 — finish remaining gates + record full size here), `common/CUIWnd.h`, `common/CWnd.h`, `common/CCtrlButton.h`, `common/CCtrlCheckBox.h`, `common/CFadeWnd.h`, `common/CLogo.h`.

- [ ] **Step 1: Grep thresholds for all 9 headers**

`grep -n BUILD_MAJOR_VERSION` each. These are predominantly `>= 95`/`>= 87` gates (all false for v79 → v79 sides with v83 layout), plus the CUIToolTip `>= 83`/`>= 84` gates audited in Task 12. List each gate's v79 truth value.

- [ ] **Step 2: Verify each header's gated fields + size**

For each of the 9: pick a method that accesses the gated region (disasm), determine v79 size, and confirm each `>= 87`/`>= 95` field is **absent** in v79. `CWnd`/`CUIWnd`/`CCtrlButton`/`CCtrlCheckBox` are control base classes — their sizes propagate into derived UI structs, so anchor their sizes carefully (a wrong base size shifts every derived layout). Record verdict + anchor per header. For `CUIToolTip`, fold in the Task 12 verdict and record the full v79 size.

- [ ] **Step 3: Main-session cross-check + commit**

```bash
git commit -am "docs(v79): struct audit — UI/control family (9 headers)"
```

---

## Task 15: Struct audit — Mob/stat family, exhaustive size (4 headers)

Read-only `disasm`; no struct-type application. **SecondaryStat is embedded UDT + size-critical** — verify its size independently, do NOT assume v79 == v83 == v87 (`struct-verification.md`; embedded UDTs often shrink in older builds).

**Files:** `docs/tasks/task-008-gms-v79-support/struct-verification.md`.
**Headers (4):** `common/CMob.h` (boundary/doom done in Task 12 — record full size here), `common/MobStat.h`, `common/SecondaryStat.h`, `common/CMapLoadable.h` (README-critical).

- [ ] **Step 1: Grep thresholds**

`grep -n BUILD_MAJOR_VERSION` each. SecondaryStat has `== 87`/`>= 87` gates — both **absent** for v79 (v79 < 87 → base layout). CMob `< 95` (true), `>= 84`/`>= 87`/`>= 95` (false). MobStat `>= 95` (false). CMapLoadable `>= 95`/`>= 84` (false).

- [ ] **Step 2: Verify SecondaryStat size + gated fields independently**

Find the v79 `SecondaryStat` size from a `Decode`/`RemoveAll` `imul stride,N` anchor where it is embedded (inside `CWvsContext::m_secondaryStat`, cross-ref Task 13 Step 2, or a CUser stat decode). Confirm every `>= 87`/`== 87` gated field is **absent** in v79. Size-critical: a wrong size shifts every field after the embed. Record the exact v79 size + per-gate verdict.
**Optional cross-validation:** if any SecondaryStat bit-indexed disagreement is suspected, dispatch the `stat-registry-cross-validator` agent against atlas-ms ([[reference_atlas_ms]]) with the v79 mapping — but only if a disagreement surfaces; the default expectation is "v79 sides with v83."

- [ ] **Step 3: Verify CMob, MobStat, CMapLoadable**

CMob: confirm `< 95` fields present (v79 < 95 → true) and `>= 84`/`>= 87`/`>= 95` fields absent (fold in Task 12 doom/base verdict); record full v79 size. MobStat / CMapLoadable: confirm `>= 95`/`>= 84` fields absent, record v79 size + anchor. CMapLoadable is README-flagged — anchor its size with extra care.

- [ ] **Step 4: Main-session cross-check + commit**

```bash
git commit -am "docs(v79): struct audit — Mob/stat family (4 headers)"
```

---

## Task 16: Struct audit — Party/Guild/misc, exhaustive size; complete 24/24 (4 headers)

Read-only `disasm`; no struct-type application. Record v79 size for each (D5).

**Files:** `docs/tasks/task-008-gms-v79-support/struct-verification.md`.
**Headers (4):** `common/PartyData.h`, `common/PartyMember.h`, `common/GuildData.h`, `common/CFuncKeyMappedMan.h` (Cat-A size gate amended in Task 3 — record the full size + remaining-gate verdict here).

- [ ] **Step 1: Grep thresholds**

`grep -n BUILD_MAJOR_VERSION` each. PartyMember may have `< 95` (true for v79); PartyData/GuildData `>= 95` (false); CFuncKeyMappedMan `>= 111`/`>= 95` member gates (false → v79 base members) + the `assert_size` chain amended in Task 3. List each gate's v79 truth value.

- [ ] **Step 2: Verify each header**

For each: disasm a method accessing the gated region, determine v79 size, confirm the `< 95` field present / `>= 95`/`>= 111` fields absent for v79, record verdict + anchor. `CFuncKeyMappedMan` — confirm the v79 size recorded in Task 3 against an independent anchor here, and verify the `>= 111`/`>= 95` member gates resolve to the base layout for v79.

- [ ] **Step 3: Complete the 24-header verdict table + main-session cross-check + commit**

After this task, all **24** rows in `struct-verification.md` must have a v79 size + verdict + evidence. Assert no `☐` rows remain. List the count and confirm it is 24.

```bash
git commit -am "docs(v79): struct audit — Party/Guild/misc (4 headers); 24/24 complete"
```

---

## Task 17: Apply remaining gate rewrites + cross-version validation (FR-12b/FR-13, R10)

Only now — with all 24 headers + the Cat B/C/D boundary gates audited — apply source changes for any gate the audit (Tasks 12–16) marked "needs change." The Cat-A size gates were already amended in Task 3. A "no change, here's why" verdict requires **no** source edit (design §5.7 confirm-in-place is a first-class outcome).

**Files (only those the audit flagged):** any of `common/CUIToolTip.h`, `common/CLogin.h`, `common/CWvsContext.h`, `bypass/socket_hooks.cpp`, `common/CMob.h`, `doom-fix/dllmain.cpp`, and any other `common/*.h` the audit corrected.

- [ ] **Step 1: For each flagged gate, design the minimal correct boundary (design §5.7)**

- Keep the house guard form: `#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION <cmp>) ...`.
- Prefer adjusting the existing comparator to the smallest correct boundary:
  - **Cat B floor-lower:** if v79 has the field, `>= 83` → `>= 79` (keep `|| JMS`). E.g. `CUIToolTip.h:92` `m_pLayerAdditional`.
  - **Cat E enumerated-extend:** if v79 has a v83-only member, `== 83` → `(== 83 || == 79)` (or `<= 83`). E.g. `CLogin.h:235` `unk3[5]`.
  - For a v79-only divergence with no analog: introduce explicit `== 79`.
- Gate the **minimum contiguous region** — a single divergent field gets a one-field gate, not a section gate. **No ternary in array dimensions** — use `#if/#else/#endif`.

- [ ] **Step 2: Cross-version truth-table the rewrite (FR-13, design §5.7)**

For each rewritten comparator, build the truth table across **v83, v84, v87, v95, v111, JMS185** and confirm every version still selects its correct branch:
- v83/v84/v87/v95/JMS185 — confirm the new comparator selects the same branch as before (their IDBs are live; re-anchor with a probe if the rewrite touches a layout claim, not just a threshold).
- **v111** (not loaded) — evaluate from build constants + `memory_maps/GMS/v111_1.cmake`. If correctness for v111 cannot be settled from source alone, **say so explicitly** in `struct-verification.md` rather than assuming ([[feedback-prefer-confirmation]], A2).
- Record the truth table in `struct-verification.md` next to each rewrite.

- [ ] **Step 3: Apply the source edits**

Edit only the flagged sites. For each, add a brief comment referencing the audit (e.g. `// v79: m_pLayerAdditional present — floor lowered, verified task-008`). Match house style (clang-format the changed lines).

- [ ] **Step 4: Local sanity — preprocess each changed header for v79 and neighbors**

For each changed file, for `{79, 83, 84}`, confirm the gated field resolves as intended:

```bash
echo '#include "CUIToolTip.h"' > /tmp/_gatecheck.cpp
gcc -E -DREGION_GMS -DBUILD_MAJOR_VERSION=79 -DBUILD_MINOR_VERSION=1 -I common -I include /tmp/_gatecheck.cpp 2>/dev/null | grep -n "m_pLayerAdditional"
```

Expected: the field appears for the versions the audit says should have it, absent otherwise. (This validates the comparator, not the full build — that is Task 18/CI.) If a header can't be preprocessed standalone, the 3-line `_gatecheck.cpp` includer above handles it.

- [ ] **Step 5: Commit**

```bash
git add common/ bypass/ doom-fix/ docs/tasks/task-008-gms-v79-support/
git commit -m "fix(v79): rewrite below-floor gates per v79 audit; cross-version validated"
```

(If the audit found **no** gate needs changing beyond Task 3's Cat-A amendments, this task's only artifact is the recorded "confirm-in-place" rationale already in `struct-verification.md` — skip Steps 3–5 and note "no further gate rewrites required" in the task report.)

---

## Task 18: Build wiring + build verification (FR-1/FR-2/FR-15, R10)

**Files:** `.github/workflows/_build.yml`

- [ ] **Step 1: Add the v79 matrix entry (placed FIRST — version-ascending)**

In `.github/workflows/_build.yml`, add to `strategy.matrix.config` as the **first** entry (the list is version-ascending), aligning columns with the existing rows:

```yaml
          - { region: GMS, major: 79,  minor: 1 }
          - { region: GMS, major: 83,  minor: 1 }
          - { region: GMS, major: 84,  minor: 1 }
```

No other workflow file changes — PR/snapshot/release all consume `_build.yml` (FR-1/FR-2).

- [ ] **Step 2: Local completeness gate (final)**

Run: `cmake -DREGION=GMS -DMAJOR=79 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake`
Expected: `OK: all 159 keys defined and non-empty for GMS v79.1`

- [ ] **Step 3: Local pre-flight build (Debug + Release) via WSL**

Build GMS 79.1 on the WSL/clang-cl path before the Windows/CI handoff ([[project_wsl_cross_compile]]):

```bash
scripts/wsl-build.sh GMS 79 1
```

Confirm it compiles + links clean for Debug and Release. If any gate was rewritten (Tasks 3/17), also rebuild the other matrix versions affected to prove no regression (R10) — or rely on CI for the full matrix (Step 5). Record exactly what was run and its result. **Claim only what was actually run** ([[verification-before-completion]]); the authoritative MSVC build is CI.

- [ ] **Step 4: Commit + open PR**

```bash
git add .github/workflows/_build.yml docs/tasks/task-008-gms-v79-support/
git commit -m "ci(v79): add GMS 79.1 to the build matrix"
git push -u origin task-008-gms-v79-support
gh pr create --title "feat: GMS v79.1 support (task-008)" --body "<summary + PRD §10 acceptance checklist>"
```

- [ ] **Step 5: Confirm CI is green for GMS 79.1 (and unchanged versions)**

Watch the PR's build workflow. Confirm GMS 79.1 builds Debug + Release green and **no other version regressed** (the gate-rewrite backstop, R10). Paste the result into the task report. If CI fails, that is build-correctness evidence — fix and re-push; do not proceed to acceptance with a red build.

---

## Task 19: Acceptance — live smoke test (user-run, gates completion — D4/FR-16/R12)

**Files:** `docs/tasks/task-008-gms-v79-support/acceptance.md` (record the result).

This step is **user-run**; the agent prepares and records, it does not perform the launch.

- [ ] **Step 1: Prepare the smoke-test instructions for the user**

`acceptance.md` already contains the deploy + launch + observe checklist. Confirm the GMS 79.1 artifacts are available (CI artifact `artifacts-GMS-79.1-Release`/`-Debug`, or a local VS2022 x86 build per `acceptance.md` Step 1). Hand the user the checklist.

- [ ] **Step 2: User runs the live client**

The user launches the GMS v79 client with the proxy `ijl15.dll` + the core edits (`redirect`, `no-patcher`, `no-ad-balloon`, `bypass`, …) deployed per README "Usage", reaches the title/login screen, exercises the targeted edits, and confirms no crash / no Themida fault (FR-16).

- [ ] **Step 3: Record the exact result**

Paste the verbatim outcome (pass/fail per checkbox, plus any crash address / Themida message) into `acceptance.md` and the PR. Distinguish build-correctness from environment issues (Themida / VC++ redist / OS) per README compatibility notes (R12). The task is **not done** until this is run and the outcome recorded — a correct build with a failed/blocked smoke test is reported as such, not as success ([[verification-before-completion]]).

- [ ] **Step 4: Final task report + finish the branch**

Summarize against the PRD §10 acceptance checklist (every box), including any findings flagged for the user (Radio quirk, RESET_LSP disposition, protocol-constant drift, new v79-only sentinels, CFileStream decision, v111 unsettleable cases). Then use `superpowers:finishing-a-development-branch` to merge/close out per the user's preference.

---

## Self-review notes (author)

- **Spec coverage:** FR-1/2 → Task 18. FR-3 → Task 1 (seed) + `CheckMemoryMapKeys` (already exists) + Task 11. FR-4 → Tasks 2,4–10 (SP-1 two-anchor). FR-5 → SP-2 (Tasks 2,9). FR-6 → Task 10 Step 1. FR-7 → SP-5 + Task 10 Steps 4–5 (both directions). FR-8 → SP-3. FR-8a → Task 10 Steps 2–3 (exception-dispatch + CFileStream). FR-9/10 → SP-1 step 5 + SP-6 + Task 11 Step 3. FR-11 → Tasks 12–16 (24 headers). FR-12a → Task 3 (early Cat-A). FR-12b → Tasks 12,17. FR-13 → Tasks 3 Step 4, 17 Step 2. FR-14 → SP-4 + SP-1 step 6 + Task 11 Step 2. FR-15 → Task 18. FR-16 → Task 19. Risks: R1→T3/T12; R2→T12; R3→D5 exhaustive size Tasks 13–16; R4→SP-5 backward + T10 Step 4; R5→SP-1 D6 two-structural + spot-checks; R6→SP-2; R7→T10 Step 1; R8→lane discipline header; R9→T12–16 read-only; R10→T17 Step 2 + T18 Step 5; R11→T2 Step 2 + T10 Step 4; R12→T19 Step 3.
- **Key-count correction:** the live header parses to **159** keys (not the PRD's "155" — stale; the set grew with exception-dispatch + CFileStream). Task 1 Step 3 pins it and stops if ≠159. All validator-expected output strings say "159".
- **Failure-mode correction:** the two Cat-A sites are `assert_size` chains (silent guard loss for v79), not hard build breaks; the genuine layout-shift risk is the member-declaring gates (Cat B `CUIToolTip.h:92`, Cat E `CLogin.h:235`). Task 3 amends the size gates early AND empirically tests whether the v79 build was ever truly blocked (refuting the "build break" assumption per [[feedback-prefer-confirmation]]).
- **Validator already exists:** `cmake/CheckMemoryMapKeys.cmake` was created by task-006 — this plan reuses it (Task 1 Step 3), does not recreate it.
- **Placeholder scan:** unknown v79 addresses are discovered outputs, not placeholders — every task specifies the exact MCP tool + anchor recipe to find them. No "TBD"/"handle edge cases" steps.
- **Type/name consistency:** the validator invocation `cmake -DREGION=GMS -DMAJOR=79 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake` is identical in Tasks 1,2,11,18. Standard procedures SP-1…SP-6 are referenced by exact name throughout. Cluster→Task mapping mirrors design §3.3 (clusters 1–8 → Tasks 2,4,5,6,7,8,9,10). The 24 audited headers are partitioned disjointly across Tasks 12–16 (boundary subset in 12; full size in 13[7]+14[9]+15[4]+16[4] = 24).
