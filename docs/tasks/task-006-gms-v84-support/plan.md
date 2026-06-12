# GMS v84 Support Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add GMS v84.1 as a first-class supported build target — a complete 145-key memory map located in the v84 binary, a labeled v84 IDB, all 22 version-gated headers verified against v84, and the build matrix wired so CI exercises it.

**Architecture:** A `discovery → label → emit` pipeline produces `memory_maps/GMS/v84_1.cmake` (seeded from v83 so the 145-key set is always complete, then each key relocated and re-verified against the v84 IDB with two independent anchors). A separate read-only struct/gate audit verifies the 22 gated headers and the five 83/84 boundary gates against v84 disassembly. Build wiring and a user-run live smoke test close the task. All IDA probing runs on a single serialized lane (the IDA MCP `select_instance` is global mutable state — concurrent multi-instance probing is unsafe; see design §D3-note).

**Tech Stack:** IDA Pro MCP (5 live IDBs: v84 target + v83/v87/v95/JMS185 references), CMake (`GenerateMemoryMap.cmake` key-completeness gate), GitHub Actions matrix build (MSVC/Win32), C++17 client-edit DLLs.

---

## Reading order before starting

Read these task-folder docs — they hold data this plan references rather than duplicates:

- `prd.md` — requirements (FR-1…FR-16), acceptance criteria.
- `design.md` — orchestration, the two-anchor evidence rule (§4), gate-rewrite strategy (§5.4), IDB port table (§2).
- `memory-map.md` — **the authoritative work-list**: per-key v83 anchor address, class (addr/offset/constant/sentinel), and the live status legend (`☐ todo · ◐ located+labeled · ✔ written+catalogued`). The tracking table is updated in place as the source of truth for "what is done."
- `signature-catalog.md` — the durable anchor catalog to populate (one row per resolved function) + the version-agnostic heuristic playbook.
- `struct-verification.md` — the 22-header verdict table + the five pre-flagged boundary gates.
- `risks.md` — R1…R8.

---

## IDB lane discipline (applies to EVERY IDA-touching step)

The five reference IDBs and their ports (from `design.md` §2):

| Version | Port | Role |
|---|---|---|
| GMS v84.1 | **13341** | Target — locate, label, save |
| GMS v83 | 13337 | Primary anchor (most completely labeled) |
| GMS v87 | 13338 | Secondary anchor / gate cross-validation |
| GMS v95 | 13339 | PDB reference / gate cross-validation |
| JMS v185 | 13340 | JMS-only sentinel confirmation / gate cross-validation |
| GMS v111 | — | not loaded; gate-validated from source + `v111_1.cmake` |

Every probe follows this sequence (R5, [[feedback-verify-ida-target]]):
1. `mcp__ida-pro__select_instance(port=<intended>)`.
2. `mcp__ida-pro__get_metadata` (or `server_health`) — confirm the routed IDB is the one you intend **before** drawing any conclusion. Never infer the active IDB from "what I selected last."
3. Probe.
4. After any version switch, repeat 1–2.

Never run two IDA-probing tasks concurrently — `select_instance` routing is shared global state. subagent-driven-development already runs one task at a time, which satisfies this; do not parallelize IDA tasks.

---

## Standard procedures (referenced by every memory-map cluster task)

These are defined once here and invoked by name from Tasks 2–9. They are the concrete recipe — follow them literally per key.

### SP-1 — Resolve one absolute-address key

For key `K` whose v83 anchor address is `A83` (from `memory-map.md`):

1. **Characterize in v83** (port 13337, lane discipline). Probe `A83`:
   - `mcp__ida-pro__decompile(A83)` and `mcp__ida-pro__disasm(A83)` to read the function.
   - Record its distinctive anchors: referenced string literals (`mcp__ida-pro__xrefs_to` / strings in the body), called imports (`connect`, `socket`, registry/file APIs), magic constants / opcode immediates, vtable slot, and call-graph neighbors (`mcp__ida-pro__callees`, `mcp__ida-pro__xrefs_to`).
2. **Locate in v84** (port 13341, lane discipline) using anchors in this preference order (design §4):
   1. String xref — `mcp__ida-pro__search_text` / `find_regex` for the exact literal, then `xrefs_to` the string to reach the referencing function.
   2. Import/API call anchor — `mcp__ida-pro__imports` / `imports_query` for the API, then `xrefs_to` it.
   3. Call-graph anchor — child/parent of an already-resolved v84 function (`callees` / `xrefs_to`).
   4. Constant / opcode immediate — `mcp__ida-pro__find_bytes` / `find_regex` / `insn_query` for the immediate.
   5. Vtable slot — find the class vtable via its RTTI/type string, index the slot.
   6. Byte/structure signature — `mcp__ida-pro__make_signature_for_function(A83)` then `find_bytes` in v84. **Fallback only, never sole evidence.**
3. **Apply the two-anchor rule (design §4 / D1).** Do not accept the v84 address until **two independent anchors** both resolve to it. "Independent" = two different kinds, or two clearly distinct instances of the same kind. At least one anchor must be kind 1–5 (a pair of byte-sigs is invalid). For **high-value / high-blast-radius keys** (socket send/flush/process, the COutPacket encoders, WinMain, `CWvsApp::Run`/`SetUp`, the packet senders, and any function adjacent to a boundary gate) the two anchors must be of **two different kinds**, and the key is flagged `needs-main-review`.
4. **Disambiguate** if more than one v84 candidate matches: choose by a unique callee or a surrounding string the real one references — never by address proximity to v83 alone. Record the ambiguity and the reason for the choice.
5. **Label the v84 IDB** (port 13341): `mcp__ida-pro__rename(addr=<v84>, name=<canonical v83 name verbatim>)`, and `mcp__ida-pro__set_type(...)` where the prototype is known. Apply the v83 canonical name verbatim so cross-version greps align. (Labeling functions/globals is encouraged; do NOT apply struct types — that is the audit phase's prohibition, SP-5.)
6. **Write the cmake line** (see SP-3) and **the catalog row** (see SP-4), and flip the key's status in `memory-map.md` from `☐` to `✔` (use `◐` if located+labeled but catalog/cmake not yet written).

### SP-2 — Resolve one instruction-relative offset key

Offset keys (`*_OFFSET`, `WIN_MAIN_AD_BALLOON_CONDITIONAL`, `WIN_MAIN_PATCHER_OFFSET`, `C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET`) are **not** addresses and are **never copied from v83** (R4, FR-5):

1. Identify the offset's host function in v84 (usually already resolved as an absolute key in the same cluster).
2. `mcp__ida-pro__disasm` the host function in v84 (port 13341, lane discipline).
3. Find the target instruction/branch the offset points to (the call site, conditional, or patch point that the v83 offset described — read the v83 host at the corresponding spot to know *what* you are measuring).
4. Compute the byte delta from the host function's base (or the documented reference instruction) to that target instruction. That delta is the v84 offset value.
5. Record in the catalog (SP-4) the identified target instruction plus the measured delta. Write the cmake line (SP-3) and flip status.

### SP-3 — cmake line format

Edit `memory_maps/GMS/v84_1.cmake` in place (it was seeded from v83 in Task 1, so the line already exists with the v83 value). Replace the value, preserving key name and any trailing comment that is still accurate:

```cmake
set(C_CLIENT_SOCKET_SEND_PACKET 0x00XXXXXX)
```

- Use uppercase `0x` hex, full width matching the file's existing style.
- Where a v83 key carried a clarifying comment (`# does not exist`, `# JMS only`, `# TODO …`), update it to reflect the **v84** finding (FR-8). A carried-forward sentinel keeps an accurate `# …` note.

### SP-4 — signature-catalog row

Append/update one section per resolved function in `signature-catalog.md`, using the file's existing schema:

```
### <CANONICAL_FUNCTION_NAME>   (memory-map key: <KEY>)
- Primary anchor: <string xref | import call | call-graph | constant | vtable slot | byte sig>
- Detail: <the exact literal / API / parent / constant / slot index used>
- Fallback anchor: <secondary method>
- Cross-version stability: <which versions confirmed; known breakages>
- Notes: <ambiguity, near-duplicates, the v84 address>
```

Phrase entries about the *function*, not a specific address (the catalog is version-agnostic and reused for the next port, FR-14).

### SP-5 — Sentinel confirmation (FR-7, R3)

For a sentinel key (v83 value `0x00000000`, or a "does not exist" key), absence is **confirmed, not assumed**:

1. Identify the feature's own anchor (its strings, its `CreateInstance` call site, its vtable) from a version where it exists (v83 for GMS-absent features; JMS185 port 13340 for JMS-only features — confirm the positive case there first).
2. Search v84 (port 13341) for that anchor.
3. If the anchor is **absent** → carry `0x00000000` forward with an accurate `# …` comment and record "confirmed absent + how" in the catalog.
4. If the anchor is **present** → the feature exists in v84; resolve it as a real address per SP-1, note the surprise in the catalog, and (if it backs an edit that was previously sentinel-gated) flag it for the user in the task report.

### SP-6 — `idb_save` checkpoint

After each cluster task's labeling is complete, `mcp__ida-pro__idb_save` on the v84 IDB (port 13341, after confirming target via `get_metadata`) so labels survive an MCP swap/restart (FR-10).

---

## Task 1: Setup — scaffold cmake, baseline IDB, local completeness check

**Files:**
- Create: `memory_maps/GMS/v84_1.cmake`
- Create: `cmake/CheckMemoryMapKeys.cmake` (standalone local validator, MSVC-free)
- Modify: `docs/tasks/task-006-gms-v84-support/memory-map.md` (tracking table stays the live source of truth)

- [ ] **Step 1: Seed the v84 memory map from v83**

Copy `memory_maps/GMS/v83_1.cmake` to `memory_maps/GMS/v84_1.cmake` verbatim. This guarantees all 145 keys are present and non-empty from the start (so `GenerateMemoryMap` never fails mid-port), and gives each key a documented v83 value to relocate from. Add a header comment to the new file:

```cmake
# GMS v84.1 memory map. Seeded from v83_1.cmake; every value is relocated and
# re-verified against the v84 binary per docs/tasks/task-006-gms-v84-support/.
# A value still equal to v83 means UNVERIFIED unless its tracking-table row in
# memory-map.md is marked ✔ with a signature-catalog entry.
```

- [ ] **Step 2: Write the standalone completeness validator**

Create `cmake/CheckMemoryMapKeys.cmake` — a script-mode (`cmake -P`) validator that reuses the same key-extraction logic as `GenerateMemoryMap.cmake` but does NOT require a compiler (so it runs on this Linux/WSL box). It takes the region/version via `-D` and asserts every `@KEY@` in `include/memory_map.h.in` is `set()` non-empty by the memory map:

```cmake
# CheckMemoryMapKeys.cmake — MSVC-free completeness check.
# Usage: cmake -DREGION=GMS -DMAJOR=84 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake
# (run from the repo root)
if (NOT DEFINED REGION OR NOT DEFINED MAJOR OR NOT DEFINED MINOR)
    message(FATAL_ERROR "Set -DREGION -DMAJOR -DMINOR")
endif()
set(_INFILE  "${CMAKE_CURRENT_LIST_DIR}/../include/memory_map.h.in")
set(_MAPFILE "${CMAKE_CURRENT_LIST_DIR}/../memory_maps/${REGION}/v${MAJOR}_${MINOR}.cmake")
if (NOT EXISTS "${_INFILE}")  ; message(FATAL_ERROR "missing ${_INFILE}")  ; endif()
if (NOT EXISTS "${_MAPFILE}") ; message(FATAL_ERROR "missing ${_MAPFILE}") ; endif()
include("${_MAPFILE}")
file(STRINGS "${_INFILE}" _LINES)
set(_KEYS "")
foreach(_LINE IN LISTS _LINES)
    string(REGEX MATCHALL "@([A-Z0-9_]+)@" _MATCHES "${_LINE}")
    foreach(_M IN LISTS _MATCHES)
        string(REGEX REPLACE "^@(.*)@$" "\\1" _K "${_M}")
        list(APPEND _KEYS "${_K}")
    endforeach()
endforeach()
list(REMOVE_DUPLICATES _KEYS)
# BUILD_REGION is supplied by the build invocation (-DBUILD_REGION), not the
# memory map file, so it is not one of the 145 memory-map keys. Exclude it here
# so this standalone check reflects exactly the memory-map key set.
list(REMOVE_ITEM _KEYS BUILD_REGION)
set(_MISSING "")
foreach(_K IN LISTS _KEYS)
    if (NOT DEFINED ${_K})
        list(APPEND _MISSING "${_K}")
    elseif ("${${_K}}" STREQUAL "")
        list(APPEND _MISSING "${_K}")
    endif()
endforeach()
list(LENGTH _KEYS _N)
if (_MISSING)
    string(REPLACE ";" "\n  " _J "${_MISSING}")
    message(FATAL_ERROR "v${MAJOR}.${MINOR} missing/empty keys:\n  ${_J}")
endif()
message(STATUS "OK: all ${_N} keys defined and non-empty for ${REGION} v${MAJOR}.${MINOR}")
```

- [ ] **Step 3: Run the validator — expect PASS for the seeded file**

Run: `cmake -DREGION=GMS -DMAJOR=84 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake`
Expected: `OK: all 145 keys defined and non-empty for GMS v84.1`
(If the count is not 145, stop — the seed copy or the `.h.in` parse is wrong.)

- [ ] **Step 4: Baseline the v84 IDB**

Confirm the target IDB is reachable and is actually v84:
- `mcp__ida-pro__list_instances` — confirm port 13341 is GMS_v84.1.
- `mcp__ida-pro__select_instance(port=13341)` then `mcp__ida-pro__get_metadata` — record the image base and module name in `signature-catalog.md` notes (anchors WinMain offset math later).
- `mcp__ida-pro__idb_save` to take a clean baseline.

- [ ] **Step 5: Commit**

```bash
git checkout -b task-006-gms-v84-support
git add memory_maps/GMS/v84_1.cmake cmake/CheckMemoryMapKeys.cmake docs/tasks/task-006-gms-v84-support/
git commit -m "feat(v84): scaffold GMS v84.1 memory map seeded from v83 + key validator"
```

---

## Task 2: Memory map — WinMain + CWvsApp + window manager cluster (~24 keys)

Resolve first: the entry point anchors the whole image and gives call-graph reach into the rest (README "Adding a new version"; design §3.3 cluster 1).

**Files:**
- Modify: `memory_maps/GMS/v84_1.cmake`
- Modify: `docs/tasks/task-006-gms-v84-support/signature-catalog.md`, `memory-map.md`

**Keys (absolute unless noted):** `WIN_MAIN`, `SEND_HS_LOG`, `C_WVS_APP`, `C_WVS_APP_INSTANCE_ADDR`, `C_WVS_APP_IS_MSG_PROC`, `C_WVS_APP_INITIALIZE_AUTH`, `C_WVS_APP_INITIALIZE_PCOM`, `C_WVS_APP_CREATE_MAIN_WINDOW`, `C_WVS_APP_CONNECT_LOGIN`, `C_WVS_APP_INITIALIZE_RES_MAN`, `C_WVS_APP_INITIALIZE_GR2D`, `C_WVS_APP_INITIALIZE_INPUT`, `C_WVS_APP_INITIALIZE_SOUND`, `C_WVS_APP_INITIALIZE_GAME_DATA`, `C_WVS_APP_CREATE_WND_MANAGER`, `C_WVS_APP_GET_CMD_LINE`, `C_WVS_APP_DIR_BACK_SLASH_TO_SLASH`, `C_WVS_APP_DIR_UP_DIR`, `C_WVS_APP_DIR_SLASH_TO_BACK_SLASH`, `C_WVS_APP_GET_EXCEPTION_FILE_NAME`, `C_WVS_APP_CALL_UPDATE`, `C_WVS_APP_RUN`, `C_WVS_APP_SET_UP`, `C_WND_MAN_S_UPDATE`, `C_WND_MAN_REDRAW_INVALIDATED_WINDOWS`, `G_DW_TARGET_OS` (global).
**Offset keys:** `WIN_MAIN_AD_BALLOON_CONDITIONAL`, `WIN_MAIN_PATCHER_OFFSET` (both via SP-2, measured inside `WIN_MAIN`).

- [ ] **Step 1: Anchor WIN_MAIN from the PE entry point**

`WIN_MAIN` is reachable from the PE entry. In v84 (port 13341, lane discipline) get the entry point (`get_metadata` reports it; or `list_funcs`/`survey_binary`). Confirm via SP-1 using two anchors — e.g. the unique startup string literals it references (string xref) **and** its call to `CWvsApp::SetUp`/`Run` (call-graph). High-value key → two different kinds + `needs-main-review`. Record both in `signature-catalog.md` (the catalog already has a `WinMain` stub to fill).

- [ ] **Step 2: Resolve C_WVS_APP, C_WVS_APP_RUN, C_WVS_APP_SET_UP (high-value)**

Per SP-1. `C_WVS_APP_RUN` is the main message-pump loop (catalog stub exists); `C_WVS_APP_SET_UP` is the init driver that calls the `INITIALIZE_*` subsystem functions in a fixed order. Resolve `SET_UP` first, then walk its `callees` to reach the initializers (call-graph anchor for the rest of the cluster). All three are high-value → two different anchor kinds + `needs-main-review`.

- [ ] **Step 3: Resolve the remaining CWvsApp keys via call-graph from SET_UP/Run**

For each `C_WVS_APP_INITIALIZE_*`, `CREATE_*`, `GET_CMD_LINE`, the three `DIR_*` helpers, `GET_EXCEPTION_FILE_NAME`, `CALL_UPDATE`, `IS_MSG_PROC`: apply SP-1, using the call-graph from `SET_UP`/`Run` as anchor 1 and each function's own string/import/constant as anchor 2. The `DIR_*` helpers reference path-separator characters; `INITIALIZE_AUTH/PCOM/RES_MAN/SOUND/GAME_DATA` each reference distinctive subsystem strings — use those as the second anchor.

- [ ] **Step 4: Resolve the global + window-manager keys**

- `C_WVS_APP_INSTANCE_ADDR`, `G_DW_TARGET_OS`, `C_WVS_APP_INSTANCE_ADDR`: globals — find the `mov [g_xxx], eax` store after the relevant constructor/getter returns (catalog playbook: `*_INSTANCE_ADDR` globals). Anchor via the function that writes/reads them.
- `C_WND_MAN_S_UPDATE`, `C_WND_MAN_REDRAW_INVALIDATED_WINDOWS`: reachable from the app update loop; SP-1 with call-graph + a distinctive string/constant.

- [ ] **Step 5: Re-derive the two WinMain offsets (SP-2)**

`WIN_MAIN_AD_BALLOON_CONDITIONAL` (v83 `0xA3D`) and `WIN_MAIN_PATCHER_OFFSET` (v83 `0x212`) are byte offsets inside `WIN_MAIN`. Disassemble v84 `WIN_MAIN`, find the ad-balloon conditional branch and the patcher-window call site (read v83 at the v83 offsets to know what each points at — cross-reference `no-ad-balloon` and `no-patcher` edits for the exact patch semantics), and measure the v84 deltas. **Never copy 0xA3D / 0x212.**

- [ ] **Step 6: Spot-check the high-value keys independently (main-session discipline)**

For every key flagged `needs-main-review` in this cluster, re-probe the v84 address from scratch with a *different* anchor than the one originally used and confirm it lands on the same function (R2, design §3.2).

- [ ] **Step 7: Label, checkpoint, validate, commit**

- Label all resolved functions/globals in the v84 IDB (SP-1 step 5).
- `idb_save` (SP-6).
- Run: `cmake -DREGION=GMS -DMAJOR=84 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake` → expect `OK: all 145 keys…` (still 145; values updated).
- Flip this cluster's rows to ✔ in `memory-map.md`.

```bash
git add memory_maps/GMS/v84_1.cmake docs/tasks/task-006-gms-v84-support/
git commit -m "feat(v84): relocate WinMain + CWvsApp cluster with signatures"
```

---

## Task 3: Memory map — CClientSocket / ZSocket cluster (~14 keys)

Highest-value for the `redirect`/`bypass` edits (design §3.3 cluster 2).

**Files:** `memory_maps/GMS/v84_1.cmake`, `signature-catalog.md`, `memory-map.md`.

**Keys:** `C_CLIENT_SOCKET_INSTANCE_ADDR`, `C_CLIENT_SOCKET_CREATE_INSTANCE`, `C_CLIENT_SOCKET_SEND_PACKET`, `C_CLIENT_SOCKET_FLUSH`, `C_CLIENT_SOCKET_MANIPULATE_PACKET`, `C_CLIENT_SOCKET_PROCESS_PACKET`, `C_CLIENT_SOCKET_CLOSE`, `C_CLIENT_SOCKET_CLEAR_SEND_RECEIVE_CTX`, `C_CLIENT_SOCKET_ON_CONNECT`, `C_CLIENT_SOCKET_CONNECT_LOGIN`, `C_CLIENT_SOCKET_CONNECT_CTX`, `C_CLIENT_SOCKET_CONNECT_ADR`, `Z_SOCKET_BASE_CLOSE_SOCKET`, `Z_SOCKET_BUFFER_ALLOC`.

- [ ] **Step 1: Anchor the connect path via the `connect`/`socket` imports**

In v84 (lane discipline), `imports_query` for `connect` / `socket` (WS2_32). `xrefs_to` them reach `C_CLIENT_SOCKET_ON_CONNECT` / `CONNECT_ADR` / `CONNECT_CTX` / `CONNECT_LOGIN`. This is the import anchor (kind 2). Disambiguate the four connect variants by their distinct callers/strings.

- [ ] **Step 2: Resolve send/flush/process/manipulate (high-value)**

`SEND_PACKET`, `FLUSH`, `PROCESS_PACKET`, `MANIPULATE_PACKET` are high-value (blast radius into bypass/redirect). Apply SP-1 with two different anchor kinds each → `needs-main-review`. Anchor via call-graph from `CClientSocket` getInstance/the connect path + each function's distinctive structure (e.g. flush references the send buffer). The v83 functions are clustered (0x49637B…0x4965F1) — use call-graph adjacency in v84, not v83 address arithmetic.

- [ ] **Step 3: Resolve the remaining socket keys**

`CLOSE`, `CLEAR_SEND_RECEIVE_CTX`, `CREATE_INSTANCE`, `INSTANCE_ADDR` (global store after CreateInstance), `Z_SOCKET_BASE_CLOSE_SOCKET`, `Z_SOCKET_BUFFER_ALLOC` — SP-1, two anchors each.

- [ ] **Step 4: Spot-check `needs-main-review`, label, checkpoint, validate, commit**

Same as Task 2 Steps 6–7.

```bash
git commit -am "feat(v84): relocate CClientSocket/ZSocket cluster with signatures"
```

---

## Task 4: Memory map — COutPacket cluster (~7 keys)

Encode primitives used to craft packets; high-value (design §3.3 cluster 3).

**Files:** `memory_maps/GMS/v84_1.cmake`, `signature-catalog.md`, `memory-map.md`.

**Keys:** `C_OUT_PACKET`, `C_OUT_PACKET_ENCODE_1`, `C_OUT_PACKET_ENCODE_2`, `C_OUT_PACKET_ENCODE_4`, `C_OUT_PACKET_ENCODE_STR`, `C_OUT_PACKET_ENCODE_BUFFER`, `C_OUT_PACKET_MAKE_BUFFER_LIST`.

- [ ] **Step 1: Resolve the encoders by size signature + call-graph**

The `ENCODE_1/2/4` encoders differ by the byte-width they append (1/2/4-byte store/grow). Apply SP-1: anchor 1 = the width-specific store/grow pattern (constant/structure), anchor 2 = called-by relationship from a known packet sender (e.g. the socket send path or a resolved `C_FIELD_SEND_*` once Task 8 runs — if Task 8 is later, anchor instead via `ENCODE_*`'s shared buffer-grow callee). All are high-value → `needs-main-review`.

- [ ] **Step 2: Resolve ENCODE_STR, ENCODE_BUFFER, MAKE_BUFFER_LIST, C_OUT_PACKET**

`ENCODE_STR` references string-length handling; `ENCODE_BUFFER` takes a length arg and memcpy-grows; `MAKE_BUFFER_LIST` and the `C_OUT_PACKET` (ctor/base) anchor via call-graph among the COutPacket methods + a distinctive constant/string. SP-1, two anchors each.

- [ ] **Step 3: Spot-check, label, checkpoint, validate, commit**

Same as Task 2 Steps 6–7.

```bash
git commit -am "feat(v84): relocate COutPacket cluster with signatures"
```

---

## Task 5: Memory map — Login / Stage / Logo / Title cluster (~22 keys)

Login & stage flow (design §3.3 cluster 4).

**Files:** `memory_maps/GMS/v84_1.cmake`, `signature-catalog.md`, `memory-map.md`.

**Keys:** `C_LOGIN_UPDATE`, `C_LOGIN_SEND_CHECK_PASSWORD_PACKET`, `C_LOGO`, `C_LOGO_GET_RTTI`, `C_LOGO_IS_KIND_OF`, `C_LOGO_UPDATE`, `C_LOGO_ON_MOUSE_BUTTON`, `C_LOGO_ON_SET_FOCUS`, `C_LOGO_ON_KEY`, `C_LOGO_LOGO_END`, `C_LOGO_FORCED_END`, `C_LOGO_INIT`, `C_LOGO_INIT_NX_LOGO`, `STAGE_INSTANCE_ADDR`, `SET_STAGE`, `C_STAGE_ON_MOUSE_ENTER`, `C_STAGE_ON_PACKET`, `GR_INSTANCE_ADDR`, `C_UI_TITLE_INSTANCE_ADDR`.

- [ ] **Step 1: Resolve CLogo via its RTTI/vtable**

`C_LOGO_GET_RTTI`, `C_LOGO_IS_KIND_OF`, and the `C_LOGO` vtable cluster: find the `CLogo` RTTI/type string in v84 (`search_text` for `CLogo`/`.?AVCLogo@@`), reach its vtable, and index the slots for `GetRTTI`/`IsKindOf`/`OnSetFocus`/`OnKey`/`OnMouseButton`/`Update`. Vtable slot is anchor kind 5; confirm each with a second anchor (the method body's distinctive content). The CLogo methods are sequential vtable slots — match slot order against v83's vtable, not v83 addresses.

- [ ] **Step 2: Resolve CLogo lifecycle + NX logo**

`C_LOGO_INIT`, `C_LOGO_INIT_NX_LOGO`, `C_LOGO_LOGO_END`, `C_LOGO_FORCED_END`: `INIT_NX_LOGO` references the NX logo resource string (string xref, kind 1). SP-1 two anchors each.

- [ ] **Step 2 note:** `C_LOGO_UPDATE` and `C_LOGIN_UPDATE` share the v83 address `0x005F4C16` — confirm whether they are still the same function in v84 (one shared Update) or have diverged; record the determination in the catalog.

- [ ] **Step 3: Resolve Login keys**

`C_LOGIN_SEND_CHECK_PASSWORD_PACKET` references the password-check send path (opcode immediate + COutPacket use — anchor via the opcode constant and the call to a resolved encoder). `C_LOGIN_UPDATE` per Step 2 note. SP-1.

- [ ] **Step 4: Resolve Stage + globals + title**

`SET_STAGE`, `C_STAGE_ON_MOUSE_ENTER`, `C_STAGE_ON_PACKET` via CStage vtable/strings; `STAGE_INSTANCE_ADDR`, `GR_INSTANCE_ADDR`, `C_UI_TITLE_INSTANCE_ADDR` are globals (store-after-CreateInstance pattern). SP-1 two anchors each.

- [ ] **Step 5: Spot-check, label, checkpoint, validate, commit**

Same as Task 2 Steps 6–7.

```bash
git commit -am "feat(v84): relocate Login/Stage/Logo/Title cluster with signatures"
```

---

## Task 6: Memory map — Manager singletons cluster (~37 keys)

`*_CREATE_INSTANCE` / `*_INSTANCE_ADDR` pairs + their methods. These cluster in the static-init region and are called in a fixed order from one initializer — find one, the neighbors are adjacent calls (catalog playbook; design §3.3 cluster 5). `idb_save` partway through this large cluster.

**Files:** `memory_maps/GMS/v84_1.cmake`, `signature-catalog.md`, `memory-map.md`.

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

In v84 (lane discipline), locate the static-init function that calls the `*::CreateInstance` chain (anchor from an already-resolved `CreateInstance` in Task 2's reach, or from a distinctive manager string). The `CREATE_INSTANCE` keys are adjacent `call` sites in this chain; the matching `*_INSTANCE_ADDR` globals are the `mov [g_xxx], eax` stores right after each call returns (catalog playbook). Resolve each pair together with SP-1.

- [ ] **Step 2: Resolve manager method keys via the manager class**

For each manager's methods (`_INIT`, `_SWEEP_CACHE`, `_LOAD_BOOK`, `_LOAD_DEMAND`, `_LOAD_PARTY_QUEST_INFO`, `_LOAD_EXCLUSIVE`, `_UPDATE_DEVICE`, `_GET_IS_MESSAGE`, `_GENERATE_AUTO_KEY_DOWN`, `_SHOW_CURSOR`, `_ON_PACKET`): anchor via the resolved class instance/vtable + a distinctive string/constant the method references (e.g. `LOAD_BOOK` references the monster-book data path string). SP-1 two anchors each. `C_FUNC_KEY_MAPPED_MAN_VFTABLE` is the vtable address (via RTTI). `DEFAULT_FKM_INSTANCE_ADDR`/`DEFAULT_QKM_INSTANCE_ADDR` are default-instance globals.

- [ ] **Step 3: idb_save checkpoint mid-cluster**

After the singletons + globals are labeled (before the long method tail), `mcp__ida-pro__idb_save` (port 13341, confirm via `get_metadata`).

- [ ] **Step 4: Spot-check any high-value/boundary-adjacent keys, label, checkpoint, validate, commit**

`C_SECURITY_CLIENT_ON_PACKET` is boundary-adjacent (security_hooks gating) → `needs-main-review`. Then Task 2 Steps 6–7.

```bash
git commit -am "feat(v84): relocate manager-singleton cluster with signatures"
```

---

## Task 7: Memory map — Config / SystemInfo / IGCipher / utilities cluster (~21 keys)

(design §3.3 cluster 6).

**Files:** `memory_maps/GMS/v84_1.cmake`, `signature-catalog.md`, `memory-map.md`.

**Keys:** `GET_SE_PRIVILEGE`, `C_CONFIG`, `C_CONFIG_INSTANCE_ADDR`, `C_CONFIG_GET_PARTNER_CODE`, `C_CONFIG_APPLY_SYS_OPT`, `C_CONFIG_CHECK_EXEC_PATH_REG`, `C_CONFIG_SYS_OPT_WINDOWED_MODE`, `C_IG_CIPHER_INNO_HASH`, `Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_CTOR`, `Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_DTOR`, `C_SYSTEM_INFO`, `C_SYSTEM_INFO_INIT`, `C_SYSTEM_INFO_GET_GAME_ROOM_CLIENT`, `C_SYSTEM_INFO_GET_MACHINE_ID`, `Z_ARRAY_REMOVE_ALL`, `Z_X_STRING_GET_BUFFER`, `Z_X_STRING_TRIM_RIGHT`, `Z_X_STRING_TRIM_LEFT`, `C_MOB_C_MOB`.

- [ ] **Step 1: Resolve CConfig keys (registry/path strings = strong anchors)**

`C_CONFIG_CHECK_EXEC_PATH_REG` references registry key / exec-path strings; `C_CONFIG_SYS_OPT_WINDOWED_MODE` is a global within the sys-opt struct; `APPLY_SYS_OPT`, `GET_PARTNER_CODE`, `C_CONFIG`, `C_CONFIG_INSTANCE_ADDR` anchor via CConfig strings + call-graph. SP-1 two anchors each.

- [ ] **Step 2: Resolve utilities**

- `GET_SE_PRIVILEGE` references `SeDebugPrivilege`/token APIs (import + string anchors).
- `C_IG_CIPHER_INNO_HASH` — InnoHash; anchor via its constant table / call sites.
- `Z_ARRAY_REMOVE_ALL` — generic; anchor via call-graph and the `imul stride,N` shape (note: this is also a struct-audit tool, see SP for struct sizes).
- `Z_X_STRING_GET_BUFFER` / `TRIM_RIGHT` / `TRIM_LEFT` — ZXString helpers; anchor via whitespace/char constants + call-graph.
- `Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_CTOR`/`DTOR` — the fatal-section ctor/dtor pair (adjacent, small; anchor via the critical-section API calls).
- `C_SYSTEM_INFO`, `_INIT`, `_GET_GAME_ROOM_CLIENT`, `_GET_MACHINE_ID` — machine-id/system-info; anchor via the machine-id query strings/APIs.
- `C_MOB_C_MOB` — the `CMob::CMob` constructor (doom-fix hook target; boundary-adjacent, see Task 11) → `needs-main-review`. Anchor via the CMob vtable assignment + CMobTemplate use.

SP-1 two anchors each.

- [ ] **Step 3: Spot-check, label, checkpoint, validate, commit**

Same as Task 2 Steps 6–7.

```bash
git commit -am "feat(v84): relocate Config/SystemInfo/utilities cluster with signatures"
```

---

## Task 8: Memory map — Party / migrate / context senders + offsets (~9 keys)

(design §3.3 cluster 7). Packet senders reference their opcode as a `push <imm>` into a COutPacket — the opcode constant disambiguates similar senders (catalog playbook).

**Files:** `memory_maps/GMS/v84_1.cmake`, `signature-catalog.md`, `memory-map.md`.

**Keys:** `C_FIELD_SEND_JOIN_PARTY_MSG`, `C_FIELD_SEND_CREATE_NEW_PARTY_MSG`, `C_WVS_CONTEXT_INSTANCE_ADDR`, `C_WVS_CONTEXT_ON_ENTER_GAME`, `C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST`.
**Offset keys (SP-2):** `C_FIELD_SEND_JOIN_PARTY_MSG_OFFSET` (v83 `0x65`), `C_FIELD_SEND_CREATE_NEW_PARTY_MSG_OFFSET` (v83 `0xA4`), `C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET` (v83 `0x10`), `C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST_OFFSET` (v83 `0xE9`).

- [ ] **Step 1: Resolve the senders (high-value) via opcode immediate**

`C_FIELD_SEND_JOIN_PARTY_MSG`, `C_FIELD_SEND_CREATE_NEW_PARTY_MSG`, `C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST`: each builds a COutPacket with a specific opcode immediate (anchor 1 = the opcode `push`/`mov`), and calls a resolved encoder + the socket send (anchor 2 = call-graph to Task 3/4 functions). High-value → `needs-main-review`. Disambiguate the two party senders by their distinct opcodes.

- [ ] **Step 2: Resolve C_WVS_CONTEXT keys**

`C_WVS_CONTEXT_ON_ENTER_GAME` (enter-game handler) and `C_WVS_CONTEXT_INSTANCE_ADDR` (global). SP-1 two anchors each.

- [ ] **Step 3: Re-derive all four offsets (SP-2)**

Each `*_OFFSET` is a call-site/branch offset inside its host sender/handler. Disassemble each v84 host, find the patch-point instruction (read the v83 host at the v83 offset to know which instruction — the offsets correspond to the call-site the party/migrate/redirect edits patch), and measure the v84 delta. **Never copy 0x65 / 0xA4 / 0x10 / 0xE9.**

- [ ] **Step 4: Spot-check, label, checkpoint, validate, commit**

Same as Task 2 Steps 6–7.

```bash
git commit -am "feat(v84): relocate party/migrate/context senders + offsets"
```

---

## Task 9: Memory map — Protocol constants + sentinels (~13 keys)

Resolved last (FR-7; design §3.3 cluster 8).

**Files:** `memory_maps/GMS/v84_1.cmake`, `signature-catalog.md`, `memory-map.md`.

**Constant keys:** `VERSION_HEADER` (v83 `8`), `PLAYER_LOGGED_IN` (v83 `0x14`), `CLIENT_START_ERROR` (v83 `0x19`).
**GMS-absent sentinels:** `C_BATTLE_RECORD_MAN_CREATE_INSTANCE`, `DR_CHECK`, `CE_TRACER_RUN`, `RESET_LSP` (note: v83 carried a non-zero `0x0044ED47` but commented "does not exist").
**JMS-only sentinels:** `C_SECURITY_CLIENT_ON_PACKET_RET_STUB`, `C_SECURITY_CLIENT_ON_PACKET_CHECK`, `C_SECURITY_CLIENT_ON_PACKET_CHECK_OFFSET`, `C_WVS_APP_INITIALIZE_GR2D_WINDOWED_OFFSET`, `WIN_MAIN_LAUNCHER_STUB`.

- [ ] **Step 1: Confirm the three protocol constants against v84 (FR-6, PRD §9)**

Do NOT copy v83 values. In v84 (lane discipline):
- `VERSION_HEADER` — the version word the client sends in its hello/login handshake. Find it in the connect/login send path (resolved Task 3/Task 5). Confirm `8` or record the v84 value.
- `PLAYER_LOGGED_IN` / `CLIENT_START_ERROR` — send opcodes. Locate the handler/sender that uses each (the login flow), read the immediate, confirm `0x14` / `0x19` or record the v84 value.
- If any differ, use the v84 value and note the delta in the catalog (affects `redirect`/`bypass`).

- [ ] **Step 2: Confirm GMS-absent sentinels (SP-5)**

For `C_BATTLE_RECORD_MAN_CREATE_INSTANCE` (CBattleRecordMan), `DR_CHECK` (DRCheck), `CE_TRACER_RUN` (CETracer): search v84 for each feature's own anchor (its strings / CreateInstance / vtable). If absent → carry `0x00000000` + `# does not exist` comment. If present → resolve as a real address (SP-1) and flag the surprise (R3, A4).
For `RESET_LSP`: v83 had a real address but a "does not exist" comment — determine the v84 disposition explicitly (locate the LSP-reset function if it exists in v84; otherwise carry a sentinel) and document which.

- [ ] **Step 3: Confirm JMS-only sentinels (SP-5)**

These five are JMS-only. Confirm the positive case in JMS185 (port 13340, lane discipline) so the anchor is real, then show it absent in v84 (port 13341). Carry `0x00000000` with `# JMS only` for the GMS v84 build. (For `*_OFFSET`/`*_WINDOWED_OFFSET` sentinels the carried value is `0x00000000` per the v83 map.)

- [ ] **Step 4: Label (where applicable), checkpoint, validate, commit**

Constants need no IDB label; any sentinel found present (Step 2) gets labeled. `idb_save`. Run the validator. Flip rows to ✔.

```bash
git commit -am "feat(v84): confirm protocol constants + sentinels for v84"
```

---

## Task 10: Memory map completeness gate

**Files:** `docs/tasks/task-006-gms-v84-support/memory-map.md`, `signature-catalog.md`.

- [ ] **Step 1: Tracking-table audit — zero ☐ remaining**

Open `memory-map.md`. Confirm **every** one of the 145 rows is marked `✔` (written + catalogued). Any remaining `☐`/`◐` is unfinished work — return to the owning cluster task. List the count of ✔ rows and assert it equals 145.

- [ ] **Step 2: Catalog coverage — every absolute key has a row**

Confirm `signature-catalog.md` has one row per absolute-address key with two anchors recorded. Every offset key has its measured target instruction + delta. Every sentinel has its absence/presence determination. (FR-14, design §7.)

- [ ] **Step 3: Final IDB save**

`mcp__ida-pro__select_instance(port=13341)` → `get_metadata` → `idb_save`. The v84 IDB now carries labels for all resolved functions/globals (FR-9/FR-10).

- [ ] **Step 4: Final completeness validation + commit**

Run: `cmake -DREGION=GMS -DMAJOR=84 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake`
Expected: `OK: all 145 keys defined and non-empty for GMS v84.1`

```bash
git add docs/tasks/task-006-gms-v84-support/
git commit -m "docs(v84): memory map complete — 145/145 keys verified + catalogued"
```

---

## Task 11: Struct audit — the five 83/84 boundary gates (verify FIRST — R1)

Read-only over raw disassembly (SP-5-struct discipline below). Use `mcp__ida-pro__disasm`, **not** `decompile`, for layout evidence, and **never** apply a struct type into the IDB (R6 decompiler leak). Confirm the connected IDB with `get_metadata` first.

**Files (evidence only this task):** `docs/tasks/task-006-gms-v84-support/struct-verification.md`.

The five gates (from `struct-verification.md`):

| Site | Current gate | v84 truth (current) | Must confirm |
|---|---|---|---|
| `doom-fix/dllmain.cpp:25` | `< 84` | false → v87+ branch | Does v84 belong on the `>= 84` side (no `m_bDoomReserved=0` needed)? |
| `common/CWvsContext.h:98` | `> 83` | true → has `m_aClientKey[8]` | Does v84 CWvsContext carry `m_aClientKey[8]`? |
| `bypass/socket_hooks.cpp:233` | `> 83` | true → encodes client key | Does v84's socket hello path encode the 8-byte client key? |
| `common/CLogin.h:235` | `== 83` | false → no `unk3[5]` | Is the v83-only `int unk3[5]` truly absent in v84? |
| `common/CUIToolTip.h:92` | `>= 83` | true → has `m_pLayerAdditional` | Confirm v84 still has `m_pLayerAdditional`. |

- [ ] **Step 1: doom-fix `m_bDoomReserved` — was it fixed in v84?**

The comment says "fixed in v84". Disassemble v84 `CMob::CMob` (`C_MOB_C_MOB`, resolved Task 7) and inspect whether the `m_bDoomReserved` field is left dirty (requiring the manual `=0`) or initialized by the constructor. Cross-check v83 (where the bug exists) and v87 (where it is presumably fixed). Verdict: does v84 belong on the `< 84` (apply fix) or `>= 84` (no fix) side? Record the disasm line that decides it.

- [ ] **Step 2: CWvsContext `m_aClientKey[8]` presence in v84**

Disassemble a v84 function that touches `m_aClientKey` (the socket hello path / `CWvsContext` access). Determine whether v84 CWvsContext carries the 8-byte client key at the gated offset. This gate (`> 83`) and the socket_hooks gate (`> 83`, Step 3) are the **same fact** — the client key — so resolve them together. Anchor against v83 (no key) and v87 (has key).

- [ ] **Step 3: socket_hooks client-key encode in v84**

Confirm the v84 socket hello/connect-context send path (`bypass/socket_hooks.cpp` patches it) does/does not `EncodeBuffer(m_aClientKey, 8)`. Must match the Step 2 verdict (same field). Record the v84 send-path disasm line.

- [ ] **Step 4: CLogin `unk3[5]` — v83-only?**

Disassemble v84 CLogin layout evidence (a `Decode`/sizeof-bearing method, or the constructor's field initialization extent) and determine whether the `int unk3[5]` (20 bytes) between `m_abOnFamily` and `m_lNewEquip` exists in v84. v83 has it (`== 83`); v87+ does not. Verdict for v84.

- [ ] **Step 5: CUIToolTip `m_pLayerAdditional` in v84**

Confirm v84 CUIToolTip carries `m_pLayerAdditional` (the `>= 83` field). Disassemble a CUIToolTip method that accesses the layer members and confirm the field is present at the gated offset.

- [ ] **Step 6: Record verdicts + main-session cross-check**

For each of the five, write the verdict (gate correct / gate needs change) + the deciding disasm line into `struct-verification.md`. Before treating any "needs change" verdict as final, re-anchor it with one independent probe in the main session (design §5.2 cross-check). **Do not edit source yet** — gate rewrites are applied in Task 16 after all 22 headers are audited, so the rewrites are designed against the full picture.

```bash
git add docs/tasks/task-006-gms-v84-support/struct-verification.md
git commit -m "docs(v84): boundary-gate audit verdicts (5 gates)"
```

---

## Task 12: Struct audit — core layout headers (CWvsApp, CWvsContext, CClientSocket, CLogin)

Read-only `disasm` only; no struct-type application (R6). Each header: record v84 size, per-gated-field present/absent verdict, the disasm anchor, and the gate verdict, into `struct-verification.md`. Use the size-finding techniques in `struct-verification.md` ("How to find a v84 struct size"): a `Decode`/`DecodeBuffer(this, N)` literal = struct size; `ZArray<T>::RemoveAll` `imul stride,N` = `sizeof(T)`; destructor unwind extent bounds the layout.

**Files:** `docs/tasks/task-006-gms-v84-support/struct-verification.md`.

- [ ] **Step 1: Grep each header's thresholds**

For `common/CWvsApp.h`, `common/CWvsContext.h`, `common/CClientSocket.h`, `common/CLogin.h`: `grep -n BUILD_MAJOR_VERSION <file>` and list every gate. (CWvsContext: `>= 95`, `>= 87`, `> 83`. CWvsApp: `>= 95`, `>= 87`. CClientSocket: `>= 111`. CLogin: `>= 95`, `== 83`.) Note which change truth value at 84: `> 83` (true), `== 83` (false), `>= 87`/`>= 95`/`>= 111` (all false → v84 sides with v83). **Every `> 83` and `>= 87` gate is a v84 trap** — v84 is the only supported version strictly between 83 and 87, so a `> 83` gate puts v84 on the newer side while a `>= 87` gate puts it on the older side; the layout decides which is correct.

- [ ] **Step 2: Verify CWvsContext (already partly done in Task 11)**

Beyond `m_aClientKey` (Task 11), verify the `>= 87` field `m_bTesterAccount` (line 101) and the `>= 87`/`>= 95` blocks are correctly absent in v84. Determine v84 CWvsContext size from a Decode/dtor anchor. Record verdict per gate.

- [ ] **Step 3: Verify CWvsApp, CClientSocket, CLogin**

For each: confirm the `>= 87`/`>= 95`/`>= 111` gated fields are absent in v84 (v84 < 87, so all should resolve to the v83-side layout) and the `== 83`/`> 83` boundary fields match Task 11. Record v84 size + per-gate verdict + disasm anchor.

- [ ] **Step 4: Main-session cross-check + commit**

Re-anchor one boundary case per "needs change" header independently. Commit verdicts.

```bash
git commit -am "docs(v84): struct audit — CWvsApp/CWvsContext/CClientSocket/CLogin"
```

---

## Task 13: Struct audit — UI/control family (8 headers)

Read-only `disasm`; no struct-type application.

**Files:** `docs/tasks/task-006-gms-v84-support/struct-verification.md`.
**Headers:** `common/CUITitle.h`, `common/CUILoginStart.h`, `common/CUIToolTip.h` (boundary gate done in Task 11; finish remaining gates here), `common/CUIWnd.h`, `common/CWnd.h`, `common/CCtrlButton.h`, `common/CCtrlCheckBox.h`, `common/CFadeWnd.h`.

- [ ] **Step 1: Grep thresholds for all 8 headers**

`grep -n BUILD_MAJOR_VERSION` each. Note these are predominantly `>= 95`/`>= 87` gates (all false for v84 → v84 sides with v83 layout), plus the CUIToolTip `>= 83` (true) confirmed in Task 11. List each gate's v84 truth value.

- [ ] **Step 2: Verify each header's gated fields are absent/present per the v84-sides-with-v83 expectation**

For each of the 8: pick a method that accesses the gated region (disasm), determine v84 size, and confirm each `>= 87`/`>= 95` field is absent in v84. CWnd/CUIWnd/CCtrlButton/CCtrlCheckBox are the control base classes — their sizes propagate into derived UI structs, so anchor their sizes carefully (a wrong base size shifts every derived layout). Record verdict + anchor per header.

- [ ] **Step 3: Main-session cross-check + commit**

```bash
git commit -am "docs(v84): struct audit — UI/control family (8 headers)"
```

---

## Task 14: Struct audit — Mob/stat family (CMob, MobStat, SecondaryStat, CMapLoadable)

Read-only `disasm`; no struct-type application. **SecondaryStat is embedded UDT + size-critical** — verify its size independently, do NOT assume v84 == v87 == v95 (`struct-verification.md`; embedded UDTs often shrink pre-v95).

**Files:** `docs/tasks/task-006-gms-v84-support/struct-verification.md`.
**Headers:** `common/CMob.h`, `common/MobStat.h`, `common/SecondaryStat.h`, `common/CMapLoadable.h` (README-critical).

- [ ] **Step 1: Grep thresholds**

`grep -n BUILD_MAJOR_VERSION` each. SecondaryStat has both `>= 87` and `== 87` gates (line 405/419) — the `== 87` blocks are **v87-exclusive**, so they must be absent for v84; the `>= 87` blocks are also absent for v84 (84 < 87). CMob has `< 95` (true), `>= 87` (false), `>= 95` (false). MobStat `>= 95` (false). CMapLoadable `>= 95`/`>= 87` (false).

- [ ] **Step 2: Verify SecondaryStat size + gated fields independently**

Find the v84 `SecondaryStat` size from a `Decode`/`RemoveAll` `imul stride,N` anchor where it is embedded (e.g. inside CWvsContext's `m_secondaryStat`, or a CUser stat decode). Confirm every `>= 87` and `== 87` gated field is absent in v84 (v84 sides with the v83 layout). This is size-critical because `SecondaryStat` is embedded in `CWvsContext` — a wrong size shifts every field after it. Record the exact v84 size + per-gate verdict.

- [ ] **Step 3: Verify CMob, MobStat, CMapLoadable**

CMob: confirm `< 95` fields present (v84 < 95 → true) and `>= 87`/`>= 95` fields absent. Note CMob's `m_bDoomReserved` relates to the doom-fix gate (Task 11). MobStat / CMapLoadable: confirm `>= 95`/`>= 87` fields absent, record v84 size + anchor. CMapLoadable is README-flagged as minimally-important-to-get-right — anchor its size with extra care.

- [ ] **Step 4: Main-session cross-check + commit**

```bash
git commit -am "docs(v84): struct audit — Mob/stat family (4 headers)"
```

---

## Task 15: Struct audit — Party/Guild/Config/misc (5 headers)

Read-only `disasm`; no struct-type application.

**Files:** `docs/tasks/task-006-gms-v84-support/struct-verification.md`.
**Headers:** `common/PartyData.h`, `common/PartyMember.h`, `common/GuildData.h`, `common/ConfigSysOpt.h`, `common/CFuncKeyMappedMan.h`, `common/CLogo.h`.

- [ ] **Step 1: Grep thresholds**

`grep -n BUILD_MAJOR_VERSION` each. PartyMember has `< 95` (true for v84); PartyData/GuildData `>= 95` (false); ConfigSysOpt `>= 95` (false); CFuncKeyMappedMan `>= 111`/`>= 95` (false); CLogo `>= 95`/`>= 111` (false). So all side with the v83-layout except PartyMember's `< 95` block (present in v84).

- [ ] **Step 2: Verify each header**

For each: disasm a method accessing the gated region, determine v84 size, confirm the `< 95` field present / `>= 95`/`>= 111` fields absent for v84, record verdict + anchor. `ConfigSysOpt` ties to `C_CONFIG_SYS_OPT_WINDOWED_MODE` (Task 7) — cross-check the windowed-mode field offset. `CFuncKeyMappedMan`/`CLogo` cross-check against their resolved functions (Tasks 5/6).

- [ ] **Step 3: Complete the 22-header verdict table + main-session cross-check + commit**

After this task, all 22 rows in `struct-verification.md` must have a v84 size + verdict + evidence. Assert no `☐` rows remain.

```bash
git commit -am "docs(v84): struct audit — Party/Guild/Config/misc (6 headers); 22/22 complete"
```

---

## Task 16: Apply gate rewrites + cross-version validation (FR-12/FR-13, R7)

Only now — with all 22 headers + 5 boundary gates audited — apply source changes for any gate the audit marked "needs change." A "no change, here's why" verdict requires **no** source edit (design §5.4 confirm-in-place).

**Files (only those the audit flagged):** any of `doom-fix/dllmain.cpp`, `common/CWvsContext.h`, `bypass/socket_hooks.cpp`, `common/CLogin.h`, `common/CUIToolTip.h`, and any other `common/*.h` the audit corrected.

- [ ] **Step 1: For each flagged gate, design the minimal correct boundary (design §5.4)**

- Keep the house guard form: `#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION <cmp>) ...`.
- Prefer adjusting the existing comparator to the smallest correct boundary. Examples (apply only the ones the audit actually requires):
  - If v84 sides with v83 on a `> 83` field: change `> 83` → `>= 85` (v84 now excluded with v83) — **but** verify this keeps v87/v95/v111/JMS correct.
  - If v84 sides with v87+ on a `< 84` gate: change `< 84` → `< 84` stays, or to `< 85` if v84 should take the v83 branch.
  - For a v84-only divergence: introduce explicit `== 84`.
- Gate the **minimum contiguous region** — a single divergent field gets a one-field gate, not a section gate.
- **No ternary in array dimensions** — use `#if/#else/#endif` block form.

- [ ] **Step 2: Cross-version truth-table the rewrite (FR-13, design §5.5)**

For each rewritten comparator, build the truth table across **v83, v84, v87, v95, v111, JMS185** and confirm every version still selects its correct branch:
- v83/v87/v95/JMS185 — confirm the new comparator selects the same branch as before (their IDBs are live, port 13337/13338/13339/13340; re-anchor with a probe if the rewrite touches a layout claim, not just a threshold).
- **v111** (not loaded) — evaluate the comparator from build constants and confirm the selected branch matches `memory_maps/GMS/v111_1.cmake` / existing v111 source expectations. If correctness for v111 cannot be settled from source alone, **say so explicitly** in `struct-verification.md` rather than assuming ([[feedback-prefer-confirmation]], A2).
- Record the truth table in `struct-verification.md` next to each rewrite.

- [ ] **Step 3: Apply the source edits**

Edit only the flagged sites. For each, add a brief comment referencing the audit (e.g. `// v84: m_aClientKey present — gate verified task-006`). Run `clang-format` on changed lines to match house style (per the repo's existing style commits).

- [ ] **Step 4: Local sanity — preprocess each changed header for v84 and a neighbor**

Since MSVC compile is CI-only, sanity-check the gates resolve as intended with the local compiler's preprocessor (gates are region/version macros, region-portable). For each changed file, for `{84, 83, 87}`:

Run (example for one file/version):
```bash
gcc -E -DREGION_GMS -DBUILD_MAJOR_VERSION=84 -DBUILD_MINOR_VERSION=1 -I common -I include <changed-file-or-a-tiny-includer>.cpp 2>/dev/null | grep -n "<the gated field name>"
```
Expected: the field appears for the versions the audit says should have it, and is absent otherwise. (This validates the comparator logic, not the full build — that is Task 17/CI.) If a header can't be preprocessed standalone, write a 3-line `_gatecheck.cpp` that only `#include`s it.

- [ ] **Step 5: Commit**

```bash
git add doom-fix/ common/ bypass/ docs/tasks/task-006-gms-v84-support/
git commit -m "fix(v84): rewrite boundary gates per v84 audit; cross-version validated"
```

(If the audit found **no** gate needs changing, this task's only artifact is the recorded "confirm-in-place" rationale already in `struct-verification.md` from Tasks 11–15 — skip Steps 3–5 and note "no gate rewrites required" in the task report.)

---

## Task 17: Build wiring + build verification (FR-1/FR-2/FR-15)

**Files:**
- Modify: `.github/workflows/_build.yml`

- [ ] **Step 1: Add the v84 matrix entry**

In `.github/workflows/_build.yml`, add to `strategy.matrix.config` (after the GMS 83 line, keeping alignment):

```yaml
          - { region: GMS, major: 83,  minor: 1 }
          - { region: GMS, major: 84,  minor: 1 }
          - { region: GMS, major: 87,  minor: 1 }
```

No other workflow file changes — PR/snapshot/release all consume `_build.yml` (FR-1/FR-2).

- [ ] **Step 2: Local completeness gate (final)**

Run: `cmake -DREGION=GMS -DMAJOR=84 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake`
Expected: `OK: all 145 keys defined and non-empty for GMS v84.1`

- [ ] **Step 3: Note the MSVC build is CI-verified**

The full Debug+Release MSVC/Win32 compile (FR-15) runs on Windows in CI; this Linux/WSL box cannot produce the client DLLs. Record in the task report that build-correctness is asserted via the CI matrix (and the user's local CLion build if they run one). If any gate was rewritten (Task 16), CI also rebuilds v83/v87/v95/v111/JMS185 — that is the regression backstop (R7). Do **not** claim a green build locally; claim only what was actually run (the `-P` completeness check) per [[feedback-prefer-confirmation]] and verification-before-completion.

- [ ] **Step 4: Commit + open PR**

```bash
git add .github/workflows/_build.yml docs/tasks/task-006-gms-v84-support/
git commit -m "ci(v84): add GMS 84.1 to the build matrix"
git push -u origin task-006-gms-v84-support
gh pr create --title "feat: GMS v84.1 support" --body "<summary + acceptance checklist>"
```

- [ ] **Step 5: Confirm CI is green for GMS 84.1 (and unchanged versions)**

Watch the PR's build workflow. Confirm GMS 84.1 builds Debug + Release green and no other version's build regressed. Paste the result into the task report. If CI fails, this is build-correctness evidence — fix and re-push; do not proceed to acceptance with a red build.

---

## Task 18: Acceptance — live smoke test (user-run, gates completion — D4/FR-16/R8)

**Files:** `docs/tasks/task-006-gms-v84-support/` (record the result).

This step is **user-run**; the agent prepares and records, it does not perform the launch.

- [ ] **Step 1: Prepare the smoke-test instructions for the user**

Produce a short checklist for the user: deploy the proxy `ijl15.dll` + the core edits (built from the green CI artifacts for GMS 84.1) into a real GMS v84 client per README "Usage", launch, and observe.

- [ ] **Step 2: User runs the live client**

The user launches the GMS v84 client with proxy + core edits, reaches the title/login screen, exercises the targeted edits (e.g. `redirect` localhost, `no-patcher`, `no-ad-balloon`), and confirms no crash / no Themida fault (FR-16).

- [ ] **Step 3: Record the exact result**

Paste the verbatim outcome into the task folder (a new `acceptance.md` or appended to the report). Distinguish build-correctness from environment issues (Themida / VC++ redist / OS) per README compatibility notes (R8). The task is **not done** until this is run and the outcome recorded — a correct build with a failed/blocked smoke test is reported as such, not as success.

- [ ] **Step 4: Final task report + finish the branch**

Summarize against the PRD §10 acceptance checklist (every box). Then use `superpowers:finishing-a-development-branch` to merge/close out per the user's preference.

---

## Self-review notes (author)

- **Spec coverage:** FR-1/2 → Task 17. FR-3 → Task 1 (seed) + Task 10 (gate) + `CheckMemoryMapKeys`. FR-4 → Tasks 2–8 (SP-1 two-anchor). FR-5 → SP-2 (Tasks 2,8). FR-6 → Task 9 Step 1. FR-7 → Task 9 Steps 2–3 (SP-5). FR-8 → SP-3. FR-9/10 → SP-1 step 5 + SP-6 + Task 10 Step 3. FR-11 → Tasks 12–15. FR-12 → Task 11 + Task 16. FR-13 → Task 16 Step 2. FR-14 → SP-4 + Task 10 Step 2. FR-15 → Task 17. FR-16 → Task 18. R1→T11; R2→SP-1 two-anchor + spot-checks; R3→SP-5; R4→SP-2; R5→lane discipline header; R6→T11–15 read-only; R7→T16 Step 2; R8→T18 Step 3.
- **Placeholder scan:** unknown v84 addresses are discovered outputs, not placeholders — every task specifies the exact MCP tool + anchor recipe to find them. No "TBD"/"handle edge cases" steps.
- **Type/name consistency:** the validator file is `cmake/CheckMemoryMapKeys.cmake` and the same `cmake -DREGION=GMS -DMAJOR=84 -DMINOR=1 -P …` invocation is used in Tasks 1,2,10,17. Standard procedures SP-1…SP-6 are referenced by exact name throughout.
