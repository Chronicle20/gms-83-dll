# GMS v84 Support — Design

Status: Draft for review
Created: 2026-06-05
Inputs: `prd.md`, `memory-map.md`, `signature-catalog.md`, `struct-verification.md`, `risks.md`

This document defines *how* the v84 port is executed. It does not restate the
*what/why* (see `prd.md`) or the per-key tracking table and anchor playbook (see
`memory-map.md` / `signature-catalog.md`). It records the methodology and
architecture decisions, the subagent contracts, the sequencing that keeps the
struct-verification pass clean, and the acceptance strategy.

---

## 1. Resolved design decisions

These were settled during brainstorming; the rest of the document elaborates them.

| # | Decision | Choice |
|---|---|---|
| D1 | Signature-robustness threshold | **Two independent anchors for every absolute-address key.** Byte-signature is never sole evidence. (§4) |
| D2 | IDB instance topology | **All five GMS/JMS IDBs stay loaded and are pinnable.** v84 target + v83/v87/v95/JMS185 references. v111 is not loaded (validated from source, see §6). (§2) |
| D3 | Orchestration | **Sequential discovery subagents + serial main-session labeling.** IDA probing runs on one serialized lane; see D3-note. (§3) |
| D4 | Live smoke test | **User-run, gates completion.** CI-green is the automated bar; FR-16 is a manual step the user performs and records before the task is "done." (§7) |

### D3-note — why discovery is sequential, not parallel

The brainstorming answer favored parallel discovery. Verification of the MCP
tooling (`select_instance` schema: *"All subsequent tool calls will be routed to
the selected instance"*; `list_instances` shows a single global `active` flag)
shows instance selection is **global mutable state shared across all agents**
(Claude Code subagents reuse the parent's MCP connections). Two concurrent
subagents that each `select_instance` to different IDBs clobber each other's
routing — a silent wrong-IDB probe, which is exactly risk **R5**.

The plugin update therefore removed the *swap cost* (all IDBs stay open;
switching is an instant `select_instance` call rather than a manual close/reopen)
but did **not** make concurrent multi-instance probing safe. True parallel IDA
probing would require multiple MCP servers, which is not configured.

Resolution: keep all IDA probing on a single serialized lane. Discovery
subagents run **one at a time**, each owning a subsystem cluster and holding
exclusive use of the IDA lane for its turn (free to switch among all five IDBs
within that turn). The benefits the parallel choice was reaching for —
context isolation per subsystem, and "main session does the labeling" — are
both preserved. Only non-IDA work (source-gate logic, catalog drafting, cmake
emission) may fan out concurrently.

*If the user later runs a second MCP server bound to a different port, the
discovery phase can be re-parallelized by pinning each agent to a distinct
server; the subagent contract in §3.2 is written to make that a drop-in change.*

---

## 2. IDB instance topology

Confirmed live via `list_instances` on 2026-06-05:

| Version | Binary | Port | Role |
|---|---|---|---|
| GMS v84.1 | `GMS_v84.1_U_DEVM.exe` | 13341 | **Target** — locate, label, save |
| GMS v83 | `MapleStory_dump.exe` | 13337 | Primary anchor (most completely labeled) |
| GMS v87 | `GMSv87_4GB.exe` | 13338 | Secondary anchor / gate cross-validation |
| GMS v95 | `GMS_v95.0_U_DEVM.exe` | 13339 | PDB-derived reference / gate cross-validation |
| JMS v185 | `MapleStory_dump_SCY.exe` | 13340 | JMS-only sentinel confirmation / gate cross-validation |
| GMS v111 | — | not loaded | Gate validation from source + existing `v111_1.cmake` (§6) |

**Lane discipline (every IDA-touching step):**
1. `select_instance(port=<target>)`.
2. `get_metadata` (or `server_health`) to confirm the routed IDB is the intended
   one *before* drawing any conclusion ([[feedback-verify-ida-target]], R5).
3. Probe.
4. When switching versions mid-flow, repeat 1–2. Never infer the active IDB
   from "what I selected last" — re-confirm after every switch.

Ports are recorded here so subagents receive them as literal inputs rather than
discovering them (and risk pinning the wrong one).

---

## 3. Orchestration architecture

The work decomposes into a **discovery → label → emit** pipeline for the memory
map, a separate **read-only struct/gate audit**, and the **build wiring**. The
pipeline stages are ordered to keep the audit pass free of decompiler leak (§5).

### 3.1 Pipeline overview

```
Phase 0  Reference prep        (main)    — extract v83 anchors per key, batch
Phase 1  Discovery             (subagent×N, sequential) — locate v84 addr + 2 anchors, read-only
Phase 2  Spot-check + label    (main)    — verify boundary/high-value keys, apply rename/set_type, idb_save
Phase 3  Offsets & sentinels   (main)    — re-derive *_OFFSET; confirm sentinel absence
Phase 4  Emit cmake + catalog  (main + fan-out) — write v84_1.cmake (145 keys) + signature-catalog rows
Phase 5  Struct/gate audit     (subagent×N, sequential, READ-ONLY) — 22 headers + 5 boundary gates
Phase 6  Build wiring          (main)    — add matrix entry; configure+build Debug/Release
Phase 7  Acceptance            (user)    — live v84 smoke test; record result
```

Phases 0–4 produce the memory map; Phase 5 is independent and may begin once the
function/global labels exist (it reads them but must not write struct types).
Phase 6 depends on 4 (cmake complete) and 5 (any gate rewrites landed). Phase 7
depends on 6 (green build artifacts).

### 3.2 Discovery subagent contract (Phase 1)

One subagent per subsystem cluster (clusters in §3.3). Dispatched **sequentially**
(D3-note). Each is given: its key list with v83 addresses, the IDB port table
(§2), the anchor playbook (`signature-catalog.md`), and the 2-anchor rule (§4).

**The subagent is read-only on every IDB. It does NOT `rename`, `set_type`,
`idb_save`, or apply any struct type.** Its job is to *find and justify*, not to
mutate. (This is the "main session labels" decision, D3.)

Per key it returns a structured row:

```
key:            C_CLIENT_SOCKET_SEND_PACKET
v84_addr:       0x00XXXXXX
proposed_name:  CClientSocket::SendPacket
proto:          int __thiscall CClientSocket::SendPacket(CClientSocket*, COutPacket*)   # or null
anchor_1:       { kind: string_xref,  detail: "<exact literal>",        ida_evidence: "<addr/line>" }
anchor_2:       { kind: call_graph,   detail: "called by CClientSocket::OnConnect", ida_evidence: "<addr/line>" }
confidence:     high | needs-main-review
ambiguity:      <null | "two candidates at 0xA and 0xB; chose 0xA because …">
```

`confidence: needs-main-review` is mandatory when (a) the key is high-value or
boundary-adjacent (§4 tier list), (b) the two anchors disagree across reference
versions, or (c) more than one v84 candidate matched. The main session **re-probes
every `needs-main-review` row independently** before accepting it (R2, and the
"spot-check subagent structural claims" discipline from the workflow runbook).

### 3.3 Subsystem clusters (disjoint key sets)

Following `memory-map.md`'s resolution order so call-graph reach accumulates:

1. **WinMain + CWvsApp** (~24 keys) — entry point and all `C_WVS_APP_*`
   initializers; anchors the image and yields call-graph reach into the rest.
   Resolve first.
2. **CClientSocket / ZSocket** (~14) — send/flush/process/connect + buffer/close.
3. **COutPacket** (~7) — encode primitives + make-buffer-list.
4. **Login / Stage / Logo / CUITitle** (~22) — login & stage flow.
5. **Manager singletons** (~30) — `*_CREATE_INSTANCE` / `*_INSTANCE_ADDR` pairs
   (ActionMan, MapleTV, MonsterBook, Quest, Radio, Security, FuncKeyMapped,
   InputSystem, MacroSysMan, AnimationDisplayer, …). Clustered in static-init.
6. **Config / SystemInfo / IGCipher / utilities** (~20) — CConfig, CSystemInfo,
   ZArray/ZXString helpers, ZFatalSection ctor/dtor, GET_SE_PRIVILEGE.
7. **Party / migrate senders + offsets** (~6) — `C_FIELD_SEND_*`,
   `C_WVS_CONTEXT_SEND_MIGRATE_*` and their `*_OFFSET` keys.
8. **Protocol constants + sentinels** (~13) — `VERSION_HEADER`,
   `PLAYER_LOGGED_IN`, `CLIENT_START_ERROR`; the GMS-absent and JMS-only
   sentinels. Resolved last (FR-7).

Clusters are address-disjoint, so even if labeling were later parallelized no two
writers touch the same address.

---

## 4. Evidence rule: two anchors for every address (D1)

Every absolute-address key requires **two independent, version-stable anchors**
that both resolve to the same v84 address before it is written to cmake. "Two
independent" means two *different kinds* of evidence, or two clearly distinct
instances of the same kind — not one fact stated twice.

**Anchor kinds, in preference order** (from `signature-catalog.md`):
1. String xref — a unique literal the function references.
2. Import/API call anchor — e.g. the function calling `connect`/`socket`.
3. Call-graph anchor — child/parent of an already-resolved function.
4. Constant / opcode immediate — a magic number or `push <opcode>`.
5. Vtable slot — via the class RTTI/type string, then slot index.
6. Byte/structure signature (`make_signature`) — **fallback only, never sole
   evidence** (R2; least version-stable).

Rules:
- At least **one** anchor must be from kinds 1–5 (a structural/semantic anchor).
  A pair of two byte-sigs does not satisfy D1.
- For the **high-value / high-blast-radius tier** — socket send/flush/process,
  the COutPacket encoders, WinMain, CWvsApp::Run/SetUp, the packet senders, and
  any function adjacent to a boundary gate (§5) — the two anchors must be of two
  *different kinds*, and the key is flagged `needs-main-review` (§3.2).
- **Offset keys** (`*_OFFSET`, `WIN_MAIN_AD_BALLOON_CONDITIONAL`,
  `WIN_MAIN_PATCHER_OFFSET`, `C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET`) are not
  addresses; they are re-measured directly from the v84 host function's
  disassembly (FR-5, R4). The "anchor" for an offset is the identified target
  instruction/branch plus the byte delta to it, shown in the catalog.
- **Protocol constants** (`VERSION_HEADER`, `PLAYER_LOGGED_IN`,
  `CLIENT_START_ERROR`) are confirmed against the v84 opcode table / handler
  dispatch, not copied from v83 (FR-6). If a value differs, the v84 value is used
  and the delta noted (open question in PRD §9).
- **Sentinels** (FR-7): absence is *confirmed*, not assumed. For each, search
  v84 for the feature's identifying anchor (its strings, its CreateInstance call
  site, its vtable). Only after the anchor is shown absent does the `0x00000000`
  + comment carry forward. If an anchor *is* present, the key gets a real address
  and the catalog records the surprise (R3). JMS-only sentinels are confirmed
  against JMS185 (port 13340) for the positive case and shown absent in v84.

Every accepted address lands one row in `signature-catalog.md` recording the two
anchors and their cross-version stability — that file is the durable artifact for
the next version port (FR-14).

---

## 5. Struct verification & boundary-gate audit (Phase 5)

This pass is **read-only over raw disassembly**. Function/global *labels* from
Phases 2–3 are fine and present (they do not cause field-name leak); applying a
**struct type** into the v84 IDB is prohibited (R6 / decompiler leak — the audit
would then read its own hypothesis back). Use `disasm`, not `decompile`, for
layout evidence.

### 5.1 Sequencing rationale

The audit runs *after* function labeling (Phases 2–3) so that disassembly of
method bodies shows resolved call targets, but the audit itself writes nothing to
the IDB. There is no conflict: labeling writes *symbols*, the audit reads
*layout*; the prohibited action (struct-type application) is never taken in
either phase.

### 5.2 Struct verifier subagent contract

The 22 headers in `struct-verification.md` are split across sequential
read-only subagents (one IDA lane, D3-note). The existing
`version-port-verifier` subagent is the vehicle where a header is non-trivial;
trivial size-only confirmations may be done inline in the main session.

Per header the subagent returns: v84 size, present/absent verdict for each
version-gated field, the disassembly anchor for each verdict (a `Decode`/
`DecodeBuffer(this,N)` literal, an `imul stride,N` in `RemoveAll`, or a
destructor unwind extent), and a gate verdict (correct / needs-change). Embedded
UDT sizes (e.g. `SecondaryStat`) are verified independently — not assumed equal
to v87/v95.

**Main-session cross-check:** before any gate is rewritten, the main session
re-anchors at least one boundary case per changed header with an independent
probe (workflow "subagent cross-validation discipline"). Subagent summary numbers
are treated as drafts; the per-offset evidence is trusted, the headline verdict
is verified.

### 5.3 The five boundary gates (verify first — R1)

| Site | Current gate | v84 must confirm |
|---|---|---|
| `doom-fix/dllmain.cpp:25` | `< 84` | Does v84 belong on the `>= 84` side? |
| `common/CWvsContext.h:98` | `> 83` | Does v84 CWvsContext have the `> 83` field(s)? |
| `bypass/socket_hooks.cpp:233` | `> 83` | Does v84's socket path match the `> 83` form? |
| `common/CLogin.h:235` | `== 83` | Is the v83-only member truly absent in v84? |
| `common/CUIToolTip.h:92` | `>= 83` | Confirm v84 still on this side. |

### 5.4 Gate-rewrite expression strategy

- **Confirm-in-place when possible.** If v84 genuinely behaves like the branch
  the current gate already selects, leave the gate unchanged and record the
  evidence. A "no change, here's why" verdict is a first-class outcome.
- **Minimal explicit boundary when v84 diverges.** When v84 needs to split from
  the branch it currently selects, introduce the *smallest* correct boundary:
  prefer adjusting the existing comparator (`> 83` → `>= 85` if v84 sides with
  v83; `< 84` → `< 85` if v84 sides with v83; etc.) or an explicit `== 84` for a
  v84-only case. Keep the existing `defined(REGION_GMS)` guard form
  (`#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION …)`) to match house style.
- **Gate the minimum contiguous region** (workflow Phase 3 convention) — a single
  divergent field gets a one-field gate, not a section gate.
- **No ternary in array dimensions** — use `#if/#else/#endif` block form.

### 5.5 Cross-version validation of every rewrite (FR-13, R7)

A rewritten gate must keep the correct branch for **v83, v87, v95, v111, JMS185**.
Validation:
- For v83/v87/v95/JMS185 — confirm the new comparator selects the same branch
  those versions selected before (their IDBs are live; re-anchor if the rewrite
  touches a layout claim, not just a numeric threshold).
- For **v111** (not loaded) — evaluate the comparator's truth value from the
  build constants and confirm the selected branch matches `v111_1.cmake` /
  existing v111 source expectations. If a rewrite's correctness for v111 cannot
  be settled from source alone, that is called out explicitly rather than assumed
  ([[feedback-prefer-confirmation]]).
- CI builds all matrix versions on PR (the ultimate backstop), but source-level
  reasoning must precede the CI check — a green build does not prove the *branch*
  is semantically right, only that it compiled.

---

## 6. Build-matrix wiring (Phase 6)

- Add `{ region: GMS, major: 84, minor: 1 }` to `strategy.matrix.config` in
  `.github/workflows/_build.yml` (single source of truth; PR/snapshot/release
  inherit it — FR-1/FR-2). No other workflow file is edited.
- `memory_maps/GMS/v84_1.cmake` must define all 145 keys non-empty;
  `GenerateMemoryMap.cmake` fails the build otherwise (FR-3) — this is the
  completeness enforcement, so no key may be silently omitted.
- Local acceptance: configure + build **Debug and Release** for GMS 84.1 green
  (FR-15), and re-build the other matrix versions affected by any gate rewrite to
  prove no regression (R7).

---

## 7. Verification & acceptance strategy

Two distinct evidence classes, kept separate (R8):

**Build-correctness (automated gate).**
- GMS 84.1 configures + builds clean, Debug and Release, locally and in CI.
- All other supported versions remain green (gate-rewrite regression guard).
- `GenerateMemoryMap` passes (all 145 keys present).
- Every absolute key has a 2-anchor catalog row; every offset is re-derived;
  every sentinel is justified; the 22 headers each have a recorded verdict.
- The v84 IDB is labeled for all resolved functions/globals and `idb_save`d.

**Runtime-correctness (user-run, gates completion — D4).**
- The user launches a live GMS v84 client with the proxy `ijl15.dll` and core
  edits deployed, reaches title/login, exercises the targeted edits, and confirms
  no crash / no Themida fault (FR-16).
- The exact result is recorded in the task folder. The task is **not done** until
  this is run and the outcome pasted. A correct build with a failed/blocked smoke
  test is reported as such, distinguishing build issues from environment issues
  (Themida / VC++ redist / OS) per README compatibility notes.

---

## 8. Risk control mapping

| Risk | Control in this design |
|---|---|
| R1 boundary gate wrong | §5.3 verifies all five first; §5.4 minimal-boundary rewrite; §5.5 cross-version validation |
| R2 wrong-address relocation | §4 two-anchor rule, byte-sig never sole; §3.2 `needs-main-review` re-probe |
| R3 sentinel feature actually present | §4 sentinel rule — confirm absence via the feature's own anchor before carrying `0x0` |
| R4 stale offsets | §4 offsets re-measured from v84 disasm, never copied |
| R5 wrong-IDB probe | §2 lane discipline — `get_metadata` after every `select_instance`; D3-note (no concurrent selection) |
| R6 decompiler leak | §5 read-only audit, `disasm` not `decompile`, no struct-type application |
| R7 gate rewrite regresses a version | §5.5 validate across v83/v87/v95/v111/JMS185 + CI matrix |
| R8 smoke-test environment | §7 build- vs runtime-correctness kept separate; exact results recorded |

---

## 9. Out of scope (mirrors PRD §2 non-goals)

- No new client-edit features; relocation only.
- No struct-internals re-port beyond size + version-gated field boundaries.
- No changes to non-GMS regions, or to other GMS versions except a region-correct
  boundary-gate rewrite that also touches another version's compiled branch.
- Not aiming for 100% v84 IDB symbol coverage — only the 145 keys and 22 structs.

## 10. Assumptions & open items

- **A1 (overrides a brainstorming answer):** IDA probing is serialized because
  `select_instance` is global state (D3-note). If the user stands up a second MCP
  server, discovery can be parallelized per §3.2 — flagged for the user's review.
- **A2:** v111 gate validation is source-level only (IDB not loaded, §6). Any
  unsettleable v111 case is surfaced, not assumed.
- **A3:** Protocol-constant equality v83↔v84 is unverified until Phase 1
  (PRD §9 open question); the design handles both same and different outcomes.
- **A4:** Whether any GMS-absent sentinel feature (CBattleRecordMan, DRCheck,
  CETracer) appears in v84 is unknown until §4's sentinel confirmation runs.
