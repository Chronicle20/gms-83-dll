# GMS v79 Support — Design

Status: Draft for review
Created: 2026-06-26
Inputs: `prd.md`, `memory-map.md`, `struct-verification.md`, `signature-catalog.md`,
`risks.md`, `context.md`, `acceptance.md`

This document defines *how* the v79 port is executed. It does not restate the
*what/why* (see `prd.md`) or the per-key tracking table and anchor playbook (see
`memory-map.md` / `signature-catalog.md`). It records the methodology and
architecture decisions, the subagent contracts, the sequencing that keeps the
struct-verification pass clean, the **below-floor gate-audit strategy** that makes
v79 categorically harder than v84, and the acceptance strategy.

The v84 port (`docs/tasks/task-006-gms-v84-support/design.md`) is the direct
methodological precedent. Where a decision is identical, this document says so and
moves on; the new material is the below-floor dimension (§5) and the three
v79-specific knobs settled in brainstorming (§1, D5–D7).

---

## 1. Resolved design decisions

These were settled during brainstorming; the rest of the document elaborates them.

| # | Decision | Choice |
|---|---|---|
| D1 | Signature-robustness baseline | **Two independent anchors for every absolute-address key**, ≥1 structural; a byte-signature is never sole evidence. Inherited from the v84 port. (§4) |
| D2 | IDB instance topology | **All six GMS/JMS IDBs stay loaded and are pinnable** — v79 target + v83/v84/v87/v95/JMS185 references. Ports are resolved at runtime, never hardcoded. (§2) |
| D3 | Orchestration | **Sequential discovery subagents + serial main-session labeling**, on one serialized IDA lane (`select_instance` is global mutable state). Inherited from the v84 port; see D3-note. (§3) |
| D4 | Live smoke test | **User-run, gates completion.** CI-green is the automated bar; FR-16 is a manual step the user performs and records before the task is "done." (§7) |
| D5 | Base-branch verification depth (R3) | **Exhaustive v79 size probe of all 24 gated headers** — including the ~20 that "should" just match the v83 base branch. There is no labeled IDB below v79 to catch a silent v79≠v83 delta, so every base-branch size is *measured*, not assumed. (§5.5) |
| D6 | Anchor rule for the high-value tier | **Two *structural* anchors (kinds 1–5) for the high-blast tier; byte-sig demoted to tiebreaker only** — it may disambiguate two candidates but never counts toward the anchor pair. The v79↔v83 gap is larger than v83↔v84, so byte sigs are less version-stable here. The baseline two-anchor rule (D1) still governs ordinary keys. (§4) |
| D7 | Category-A enumerated-gate timing | **Amend the no-`#else` gates early, from CWvsApp/CFuncKeyMappedMan discovery disasm, before the full 24-header struct pass** — so the v79 build becomes compilable as soon as possible and the rest of the audit can iterate against a building target. (§5.3) |

### D3-note — why discovery is sequential, not parallel

Identical to the v84 finding and still binding: `select_instance` routes **all**
subsequent MCP calls to the chosen instance and is **global mutable state shared
across agents** (Claude Code subagents reuse the parent's MCP connections). Two
concurrent subagents that each `select_instance` to different IDBs clobber each
other's routing — a silent wrong-IDB probe, exactly risk **R8**. All six IDBs
staying loaded removes the *swap cost* (switching is an instant `select_instance`
rather than a close/reopen) but does **not** make concurrent multi-instance
probing safe.

Resolution: keep all IDA probing on a single serialized lane. Discovery subagents
run **one at a time**, each owning a subsystem cluster and holding exclusive use of
the IDA lane for its turn (free to switch among all six IDBs within that turn).
Only non-IDA work (source-gate truth-table reasoning, catalog drafting, cmake
emission) may fan out concurrently. *If the user later stands up a second MCP
server bound to a different port, discovery can be re-parallelized by pinning each
agent to a distinct server; the §3.2 contract is written to make that a drop-in
change.*

---

## 2. IDB instance topology

Confirmed live via `list_instances` on 2026-06-26. **Ports are deliberately not
recorded here — they change between sessions.** Subagents resolve the live port for
a version by calling `list_instances` and matching the binary name, then confirm
with `get_metadata` before drawing any conclusion.

| Version | Binary (match on this) | Role |
|---|---|---|
| GMS v79.1 | `GMS_v79_1_DEVM.exe` | **Target** — locate, label, `idb_save` |
| GMS v83 | `MapleStory_dump.exe` | **Primary anchor** — closest version (gap 4) and most completely labeled; the layout every gate is framed against |
| GMS v84.1 | `GMS_v84.1_U_DEVM.exe` | Secondary anchor — next-closest (gap 5), freshly labeled by task-006; cross-check for discovery |
| GMS v87 | `GMSv87_4GB.exe` | Upper-gate cross-validation (FR-13) |
| GMS v95 | `GMS_v95.0_U_DEVM.exe` | PDB-derived reference / upper-gate cross-validation |
| JMS v185 | `MapleStory_dump_SCY.exe` | JMS-only sentinel confirmation / JMS-branch FR-13 check |

v111 is **not** loaded; its gate validation is source-level only (existing
`v111_1.cmake` + build constants), and any v111 case that cannot be settled from
source is surfaced, not assumed (A2).

**Lane discipline (every IDA-touching step):**
1. `list_instances` → resolve the target version's current port by binary name.
2. `select_instance(port=<resolved>)`.
3. `get_metadata` (or `server_health`) to confirm the routed IDB is the intended
   one *before* concluding anything ([[feedback-verify-ida-target]], R8).
4. Probe.
5. On every version switch, repeat 1–3. Never infer the active IDB from "what I
   selected last" — re-confirm after each switch.

The binary-name → port indirection (rather than a hardcoded port) is what keeps the
design correct across session restarts where ports are reassigned.

---

## 3. Orchestration architecture

The work decomposes into a **discovery → label → emit** pipeline for the memory
map, a separate **read-only struct/gate audit**, and the **build wiring**. Stages
are ordered to keep the audit free of decompiler leak (§5.6) and to unblock the
v79 build early (D7).

### 3.1 Pipeline overview

```
Phase 0  Reference prep         (main)    — re-grep the live 155-key set from
                                            include/memory_map.h.in; extract v83
                                            anchors per key; reconcile the
                                            memory-map.md tracking table
Phase 1  Discovery              (subagent×N, sequential) — locate v79 addr + anchors,
                                            read-only on every IDB
Phase 1a Early Cat-A amendment  (main)    — once CWvsApp + CFuncKeyMappedMan are
                                            located/sized, decide v79's branch and
                                            amend the two no-#else gates (D7)
Phase 2  Spot-check + label     (main)    — re-probe needs-main-review keys; apply
                                            rename/set_type; idb_save at checkpoints
Phase 3  Offsets & sentinels    (main)    — re-derive *_OFFSET from v79 codegen;
                                            confirm sentinel absence (both directions)
Phase 4  Emit cmake + catalog   (main + fan-out) — write v79_1.cmake (all keys) +
                                            signature-catalog rows
Phase 5  Struct/gate audit      (subagent×N, sequential, READ-ONLY) — all 24 headers
                                            (exhaustive size, D5) + the below-floor
                                            gate categories A–E
Phase 6  Build wiring           (main)    — add matrix entry; configure+build
                                            Debug/Release; rebuild affected versions
Phase 7  Acceptance             (user)    — live v79 smoke test; record result
```

Phases 0–4 produce the memory map; Phase 1a is the early build-unblock (D7);
Phase 5 is independent and may begin once function/global labels exist (it reads
them but must not write struct types). Phase 6 depends on 4 (cmake complete) and 5
(gate rewrites landed). Phase 7 depends on 6 (green artifacts).

### 3.2 Discovery subagent contract (Phase 1)

One subagent per subsystem cluster (clusters in §3.3), dispatched **sequentially**
(D3-note). Each is given: its key list with v83 *and* v84 seed addresses, the
binary-name → IDB resolution rule (§2), the anchor playbook
(`signature-catalog.md` + the v84 catalog it points to), and the anchor rules (§4).

**The subagent is read-only on every IDB. It does NOT `rename`, `set_type`,
`idb_save`, or apply any struct type.** Its job is to *find and justify*, not
mutate (the "main session labels" decision, D3).

Per key it returns a structured row:

```
key:            C_CLIENT_SOCKET_SEND_PACKET
v79_addr:       0x00XXXXXX
proposed_name:  CClientSocket::SendPacket
proto:          int __thiscall CClientSocket::SendPacket(CClientSocket*, COutPacket*)   # or null
anchor_1:       { kind: string_xref, detail: "<exact literal>",            ida_evidence: "<addr/line>" }
anchor_2:       { kind: call_graph,  detail: "called by CClientSocket::OnConnect", ida_evidence: "<addr/line>" }
tiebreaker:     <null | "byte-sig at 0xA matched, 0xB did not — chose 0xA"> # NOT counted as an anchor
v79_vs_v83:     <"string present in v79 identical to v83" | "v83 literal '%s' drifted to '...' in v79">
confidence:     high | needs-main-review
ambiguity:      <null | "two candidates 0xA/0xB; chose 0xA because …">
```

`confidence: needs-main-review` is mandatory when (a) the key is high-value or
boundary-adjacent (§4 tier list), (b) anchors disagree across reference versions,
(c) more than one v79 candidate matched, or (d) **the v83 anchor literal/constant
drifted in v79** (a below-floor-specific trigger — the older build is likelier to
have shifted a string or magic number; record the v79 form for the catalog). The
main session **re-probes every `needs-main-review` row independently** before
accepting it.

### 3.3 Subsystem clusters (disjoint key sets)

Following `memory-map.md`'s resolution order so call-graph reach accumulates:

1. **WinMain + CWvsApp** (incl. all `C_WVS_APP_*` initializers) — entry point;
   anchors the image and yields call-graph reach. **Resolve first** — also gates
   the early Cat-A amendment (D7) and surfaces the `C_WVS_APP_SET_UP` /
   DR_init init-sequence question (R11, [[project_v84_movement_anticheat_freeze]]).
2. **CClientSocket / ZSocket** — send/flush/process/connect + buffer/close.
3. **COutPacket** — encode primitives + make-buffer-list.
4. **Login / Stage / Logo / CUITitle** — login & stage flow.
5. **Manager singletons** — `*_CREATE_INSTANCE` / `*_INSTANCE_ADDR` pairs
   (ActionMan, MapleTV, MonsterBook, Quest, Radio, Security, FuncKeyMapped,
   InputSystem, MacroSysMan, AnimationDisplayer, …). Includes the
   `C_RADIO_MANAGER_INSTANCE_ADDR` quirk the v84 port flagged — verify the real
   v79 instance global, don't inherit the v83 allocator-selector address.
6. **Config / SystemInfo / IGCipher / utilities** — CConfig, CSystemInfo,
   ZArray/ZXString helpers, ZFatalSection ctor/dtor, GET_SE_PRIVILEGE.
7. **Party / migrate senders + offsets** — `C_FIELD_SEND_*`,
   `C_WVS_CONTEXT_SEND_MIGRATE_*` and their `*_OFFSET` keys.
8. **Protocol constants + sentinels + exception-dispatch** — `VERSION_HEADER`,
   `PLAYER_LOGGED_IN`, `CLIENT_START_ERROR` (confirm against v79, older protocol —
   R7); the `C_TI_*EXCEPTION` / `C_PATCH_EXCEPTION_BUILDER` / `C_COM_RAISE_ERROR_EX`
   / `C_FILE_STREAM_*` keys; the GMS-absent and JMS-only sentinels; and any **new
   v79-only sentinel** discovered when a v83 real-address feature is absent in v79
   (R4, backward direction). Resolved last (FR-7).

Clusters are address-disjoint, so even if labeling were later parallelized no two
writers touch the same address.

---

## 4. Evidence rule: anchors per address (D1, D6)

Every absolute-address key requires **two independent, version-stable anchors**
that both resolve to the same v79 address before it is written to cmake. "Two
independent" means two *different kinds* of evidence, or two clearly distinct
instances of the same kind — not one fact stated twice.

**Anchor kinds, in preference order** (from `signature-catalog.md`):
1. String xref — a unique literal the function references.
2. Import/API call anchor — e.g. the function calling `connect`/`socket`.
3. Call-graph anchor — child/parent of an already-resolved function.
4. Constant / opcode immediate — a magic number or `push <opcode>`.
5. Vtable slot — via the class RTTI/type string, then slot index.
6. Byte/structure signature (`make_signature`) — **tiebreaker only for v79; never
   an anchor** (see D6 below).

Rules:
- **Baseline (all keys):** at least **one** anchor must be from kinds 1–5. A pair
  of two byte-sigs never satisfies the rule.
- **High-value / high-blast tier (D6) — socket send/flush/process, the COutPacket
  encoders, WinMain, CWvsApp::Run/SetUp, the packet senders, and any function
  adjacent to a below-floor gate (§5):** require **two anchors of kinds 1–5**
  (two *structural* anchors, of two different kinds). A byte-signature may be used
  only to *disambiguate* between two structurally-anchored candidates; it does not
  count toward the pair. These keys are flagged `needs-main-review` (§3.2). This
  tightening reflects the larger v79↔v83 codegen gap, which makes raw byte sigs
  less reliable than they were for v84 (R5).
- **v79-vs-v83 drift capture:** because v79 is older, a v83 anchor string/constant
  may have a different form in v79. When the working anchor differs from the v83
  reference, record the **v79-specific form** in `signature-catalog.md` so the next
  (older) backward port benefits (FR-14).
- **Offset keys** (`*_OFFSET`, `WIN_MAIN_AD_BALLOON_CONDITIONAL`,
  `WIN_MAIN_PATCHER_OFFSET`, `C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET`) are not
  addresses; they are re-measured directly from the v79 host function's
  disassembly (FR-5, R6). The "anchor" for an offset is the identified target
  instruction/branch plus the byte delta to it, shown in the catalog.
- **Protocol constants** (`VERSION_HEADER`, `PLAYER_LOGGED_IN`,
  `CLIENT_START_ERROR`) are confirmed against the v79 opcode table / handler
  dispatch, not copied from v83 (FR-6, R7). v79 predates v83, so drift is
  *plausible* — if a value differs, the v79 value is used and the delta noted
  (PRD §9). Cross-check against atlas-ms's version-aware registries where
  applicable ([[reference-atlas-ms]]).
- **Sentinels (FR-7) — both directions.** Absence is *confirmed*, not assumed. For
  each v83 sentinel, search v79 for the feature's identifying anchor (strings,
  CreateInstance call site, vtable); only after the anchor is shown absent does the
  `0x00000000` + comment carry forward. **Additionally** (the backward-direction
  case, R4): watch for v83 *real-address* keys whose backing feature does not exist
  in v79 — those become **new v79-only sentinels**, flagged for the gate/edit owner
  rather than silently zeroed. `RESET_LSP` (stale v83 comment; v84 found it present)
  is resolved present/absent in v79, not inherited. JMS-only sentinels are confirmed
  present in JMS185 and shown absent in v79.

Every accepted address lands one row in `signature-catalog.md` recording its
anchors and their cross-version stability — the durable artifact for the next
version port (FR-14).

---

## 5. Below-floor struct verification & gate audit (Phase 5)

This is the dimension that makes v79 categorically harder than v84. v84 sat
*between* two supported versions and had anchors on both sides; **v79 sits below the
lowest supported version (83), with no labeled IDB beneath it.** Every `common/`
gate was authored treating v83 as the minimum, so for each gate the question is:
*which branch does v79 land on, and is that branch's layout actually v79's layout?*

The pass is **read-only over raw disassembly** (§5.6). It runs *after* function
labeling (Phases 2–3) so method-body disasm shows resolved call targets, but writes
no struct types to the v79 IDB.

### 5.1 The five failure modes (priority order)

| # | Failure mode | Gate forms | Where v79 lands | Action |
|---|---|---|---|---|
| A | **No branch at all** (build-blocking) | `== 83 \|\| == 84 \|\| == 87` with no `#else` | nothing — member undefined | Amend first (§5.3) |
| B | **Newly excluded** | `>= 83` | excluded side (NEW) | Verify v79 lacks field; else lower floor to `>= 79` (§5.4) |
| C | **Excluded, sides with v83** | `>= 84` | excluded (like v83) | Confirm field genuinely absent in v79 (§5.4) |
| D | **Inverse, included on base** | `< 95`, `< 84` | base/included | Confirm v79 base layout == v83 base layout (§5.5) |
| E | **Silently-wrong base** (quietest) | `>= 87/95/111`, `> 83`, `== 95/83` | base ("v83 layout") | **Exhaustive size probe** — D5 (§5.5) |

### 5.2 Subagent contract (struct verifier)

The 24 headers in `struct-verification.md` are split across sequential read-only
subagents (one IDA lane, D3-note). The existing `version-port-verifier` subagent is
the vehicle where a header is non-trivial; trivial size-only confirmations may be
done inline in the main session.

Per header the subagent returns: **v79 size** (always — D5), present/absent verdict
for each version-gated field, the disassembly anchor for each verdict (a
`Decode`/`DecodeBuffer(this,N)` literal, an `imul stride,N` in `RemoveAll`, a ctor
member-init or destructor unwind extent), and a gate verdict (`unchanged` /
`floor-lowered` / `v79-branch-added` / `rewritten`) with the deciding v79 disasm
line(s). Embedded UDT sizes (e.g. `SecondaryStat`) are verified independently — not
assumed equal to v83/v87/v95.

**Main-session cross-check:** before any gate is rewritten, the main session
re-anchors at least one boundary case per changed header with an independent probe.
Subagent headline verdicts are treated as drafts; the per-offset evidence is
trusted, the verdict is verified.

### 5.3 Category A — amend FIRST, early, from disasm (D7, R1)

`common/CFuncKeyMappedMan.h:38` (`== 83 || == 84 || == 87`) and
`common/CWvsApp.h:97` (`== 83 || == 84`) have no catch-all `#else`, so v79 selects
**nothing** and the gated member is undefined → silent layout shift or build break.
Re-grep at task start (`grep -rn "BUILD_MAJOR_VERSION ==" common/*.h`) — other
enumerated-no-`#else` gates may exist and line numbers may have shifted.

Per D7, as soon as Cluster 1 (WinMain + CWvsApp) and the CFuncKeyMappedMan key are
located and sized in discovery, **decide which branch v79 joins from that
disassembly** (not "oldest ⇒ v83 branch") and amend the two gates so the v79 build
compiles. The remaining 22 headers are audited afterward against a building target.
Which branch v79 joins is itself an evidence verdict recorded in
`struct-verification.md`.

### 5.4 Categories B & C — floor gates that exclude v79

- **Category B (`>= 83`):** `common/CUIToolTip.h:92` (`>= 83 || JMS`). v79 is on the
  excluded side for the first time. Confirm whether v79 `CUIToolTip` carries
  `m_pLayerAdditional` (this+0x14, after `m_pLayer` this+0x10). v84 confirmed it
  present 83→111; if v79 also has it, **lower the floor to `>= 79`** (keep JMS) and
  re-validate all versions. If absent, confirm the exclusion is correct.
- **Category C (`>= 84`):** `common/CMob.h:229`, `:233`, `common/CMapLoadable.h:154`,
  `common/CUIToolTip.h:125`, `:152`. v79 sides with v83 (excluded). Confirm each
  gated field is genuinely absent in v79 (very likely, since v83 also lacks it —
  but verify, don't assume).

### 5.5 Categories D & E — base-branch layout (D5, R3 — the quietest failure)

Most gates resolve v79 onto the "v83 layout" base branch. With **no labeled IDB
below v79**, a silent v79≠v83 base-size delta cannot be caught by triangulation and
would corrupt every downstream offset. Per **D5**, the verification is
**exhaustive: probe the v79 struct size of every gated header (all 24), including
the ~20 expected to match v83.** A size match is recorded as evidence; a mismatch
escalates that header to field-level verification.

Special attention (named in `struct-verification.md`):
- `common/CMob.h:110` (`< 95`, Cat D) — v79 included on base; confirm v79 CMob base
  layout == v83 (size + the doom-field region). The `doom-fix` gate (`< 84` form)
  puts v79 on the needs-fix side — confirm v79's `CMob::CMob` ctor also skips
  `m_bDoomReserved` (so `doom-fix` is genuinely needed for v79).
- `common/CWvsContext.h:98` (`> 83`, Cat E) — **false for v79**, so the 8-byte
  `m_aClientKey` block is treated as **absent**. Confirm v79's
  `CClientSocket::OnConnect` / connect-hello does **not** encode an 8-byte client
  key (v84 found `>83` true → key present; the gate predicts the v83 *no-key* form
  for v79). This also drives `bypass/socket_hooks.cpp:233` (`> 83`).
- `common/CLogin.h:235` (`== 83`, Cat E) — false for v79, so the v83-only 20-byte
  member (a ZList between `m_abOnFamily` and `m_lNewEquip`) is excluded for v79. But
  v79 < 83: does v79 have that member, a different one, or none? Verify against v79
  `CLogin::CLogin`; if v79 has it, the gate becomes `== 83 || == 79` (or `<= 83`).

### 5.6 Read-only discipline & decompiler-leak avoidance (R9)

Applying inferred struct types into the v79 IDB makes Hex-Rays echo the inferred
field names back, masking the very deltas being verified. So: use `disasm`, not
`decompile`, for layout evidence; **do not apply speculative struct types** during
Phase 5. Function/global *labels* from Phases 2–3 are fine and present (they label
symbols, not layout). No conflict: labeling writes symbols, the audit reads layout.

### 5.7 Gate-rewrite expression strategy & cross-version safety (FR-13, R10)

- **Confirm-in-place when possible.** If v79 behaves like the branch the current
  gate already selects, leave it unchanged and record the evidence. "No change,
  here's why" is a first-class outcome.
- **Minimal correct boundary when v79 diverges.** Prefer adjusting the existing
  comparator (lower a `>= 83` floor to `>= 79` when v79 has the field; extend an
  enumerated list with `79`) over inventing a new gate shape. Keep the house
  `#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION …)` guard form. Gate the minimum
  contiguous region; no ternary in array dimensions (use `#if/#else/#endif`).
- **Validate every rewrite across the full matrix.** A rewrite must keep
  **v83/v84/v87/v95/v111/JMS185** selecting their current branch. For the loaded
  IDBs (v83/v84/v87/v95/JMS185) confirm the comparator's truth value and re-anchor
  any layout claim the rewrite touches. For **v111** (not loaded) evaluate from
  build constants + existing `v111_1.cmake`; surface any unsettleable v111 case
  rather than assuming it (A2). CI builds all matrix versions on PR as the final
  backstop, but source-level reasoning precedes it — a green build proves the branch
  *compiled*, not that it is semantically right.

---

## 6. Build-matrix wiring (Phase 6)

- Add `{ region: GMS, major: 79, minor: 1 }` to `strategy.matrix.config` in
  `.github/workflows/_build.yml`, **placed first** to keep the list
  version-ascending (single source of truth; PR/snapshot/release inherit it —
  FR-1/FR-2). No other workflow file is edited.
- `memory_maps/GMS/v79_1.cmake` must define **every** key parsed from
  `include/memory_map.h.in` (re-grepped live in Phase 0 — the set grew between
  ports), non-empty; the CMake key-completeness check fails the build otherwise
  (FR-3). No key may be silently omitted.
- Local pre-flight: `scripts/wsl-build.sh` can compile+link the v79 build on the
  WSL/clang-cl path before the Windows/CI handoff
  ([[project-wsl-cross-compile]]). Build **Debug and Release** for GMS 79.1 green
  (FR-15), and rebuild the other matrix versions affected by any gate rewrite to
  prove no regression (R10).

---

## 7. Verification & acceptance strategy

Two distinct evidence classes, kept separate (R12):

**Build-correctness (automated gate).**
- GMS 79.1 configures + builds clean, Debug and Release, locally and in CI.
- All other supported versions remain green (gate-rewrite regression guard).
- The CMake key-completeness check passes (every required key present, non-empty).
- Every absolute key has a catalog row meeting §4 (two structural anchors for the
  high-value tier); every offset is re-derived from v79 codegen; every sentinel is
  justified in both directions (incl. any new v79-only sentinels); all 24 headers
  have a recorded size + gate verdict in `struct-verification.md`.
- The v79 IDB is labeled for all resolved functions/globals and `idb_save`d.

**Runtime-correctness (user-run, gates completion — D4).**
- The user launches a live GMS v79 client with the proxy `ijl15.dll` and the core
  edits deployed (per `acceptance.md`), reaches title/login, exercises the targeted
  edits, and confirms no crash / no Themida fault (FR-16).
- The exact result is recorded in `acceptance.md` / the PR. The task is **not done**
  until this is run and the outcome pasted. A correct build with a failed/blocked
  smoke test is reported as such, distinguishing build issues from environment
  issues (Themida / VC++ redist / OS) per README compatibility notes
  ([[verification-before-completion]]).

---

## 8. Risk control mapping

| Risk | Control in this design |
|---|---|
| R1 enumerated gate has no v79 branch (build-blocking) | §5.3 amends Category A first/early from disasm (D7); branch chosen by evidence |
| R2 `>=83`/`>=84` floor wrongly excludes v79 | §5.4 verifies each; floor lowered where v79 has the field, exclusion confirmed where it doesn't |
| R3 silently-wrong base branch (no anchor below v79) | §5.5 **exhaustive** v79 size probe of all 24 headers (D5); mismatch escalates to field-level |
| R4 backward-direction sentinel (v83-present, v79-absent) | §4 sentinel rule evaluates both directions; new v79-only sentinels flagged, not zeroed |
| R5 wrong-address relocation (larger v79↔v83 gap) | §4/D6 two *structural* anchors for high-value tier, byte-sig demoted to tiebreaker; §3.2 needs-main-review re-probe |
| R6 stale offsets | §4 offsets re-measured from v79 disasm, never copied |
| R7 protocol-constant drift (older build) | §4 constants confirmed against v79 opcode/handler table; atlas-ms cross-check |
| R8 wrong-IDB probe | §2 lane discipline — binary-name→port resolution, `get_metadata` after every `select_instance`; D3-note (no concurrent selection) |
| R9 decompiler leak | §5.6 read-only audit, `disasm` not `decompile`, no struct-type application |
| R10 gate rewrite regresses a version | §5.7 validate across v83/v84/v87/v95/v111/JMS185 + CI matrix |
| R11 DR_init / SetUp init-sequence regression | §3.3 Cluster 1 surfaces `C_WVS_APP_SET_UP` init sequence; confirm DR subsystem absence for v79, don't assume identical to v83 |
| R12 smoke-test environment | §7 build- vs runtime-correctness kept separate; exact results recorded in `acceptance.md` |

---

## 9. Out of scope (mirrors PRD §2 non-goals)

- No new client-edit features; relocation only.
- No struct-internals re-port beyond size + the version-gated field boundaries.
- No changes to non-GMS regions, or to other GMS versions except a region-correct
  gate rewrite that also touches another version's compiled branch.
- Not aiming for 100% v79 IDB symbol coverage — only the keys backing the memory
  map and the 24 verified structs.

---

## 10. Assumptions & open items

- **A1 — topology must hold for the run.** All six IDBs (v79 target +
  v83/v84/v87/v95/JMS185) were confirmed loaded on 2026-06-26. If a session
  restarts and an anchor is missing — **especially v83, the primary anchor and the
  layout every gate is framed against** — discovery/audit must pause until it is
  reloaded rather than silently falling back to a further anchor. Ports are resolved
  per-session by binary name (§2), never hardcoded.
- **A2 — v111 is source-validated only.** Its IDB is not loaded; gate rewrites are
  validated for v111 from build constants + existing `v111_1.cmake`. Any
  unsettleable v111 case is surfaced, not assumed ([[feedback-prefer-confirmation]]).
- **A3 — protocol-constant equality v83↔v79 is unverified until Phase 1** (PRD §9).
  The design handles both same and different outcomes; v79 is an older protocol
  revision, so drift is treated as plausible (R7).
- **A4 — sentinel dispositions unknown until §4 runs.** Whether GMS-absent
  sentinels (CBattleRecordMan, DRCheck, DRInit, CETracer) are also absent in v79
  (expected), whether `RESET_LSP` is present, whether the `C_FILE_STREAM_*` relay
  helpers are recoverable in v79, and whether any v83 real-address feature is absent
  in v79 (a new v79-only sentinel, R4) are all resolved during discovery, not
  pre-judged.
- **A5 — Category-A branch assignment is an evidence verdict.** Whether v79's
  `CWvsApp` and `CFuncKeyMappedMan` layouts join the `83/84/87` branch or need their
  own is decided by v79 disassembly in Phase 1a (D7/§5.3), not by "oldest ⇒ v83."
- **A6 — serial IDA lane.** Discovery is serialized because `select_instance` is
  global state (D3-note). If the user stands up a second MCP server, discovery can
  be parallelized per §3.2 — flagged for the user's review.
