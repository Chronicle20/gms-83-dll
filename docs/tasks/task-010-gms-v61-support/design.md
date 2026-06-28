# GMS v61 Support — Design

Status: Draft for review
Created: 2026-06-27
Inputs: `prd.md`, `memory-map.md`, `risks.md`
Outputs created during execution: `struct-verification.md`, `signature-catalog.md`,
`acceptance.md` (the per-key, per-struct, and smoke-test working artifacts).

This document defines *how* the v61 port is executed. It does not restate the
*what/why* (see `prd.md`) or the per-key anchor playbook (see `memory-map.md`). It
records the methodology and architecture decisions, the subagent contracts, the
sequencing that keeps the struct-verification pass clean, the **three-tier
below-floor gate-audit strategy** (now capable of producing **four-way** gate
splits) that makes v61 categorically harder than v72, the **stacked-branch
baseline** this task builds on, and the acceptance strategy.

The v72 port (`docs/tasks/task-009-gms-v72-support/design.md`) is the direct
methodological precedent — and a *consumed* artifact: its signature catalog, its
labeled v72 IDB, its v72/v79 memory maps, and its below-floor gate verdicts are the
closest anchor and template for this port. Where a decision is identical to
task-009, this document says so and moves on. The genuinely new material is the
**third below-floor tier** (§5): task-009 lowered the floor to 72 and, for several
structs, turned task-008's two-way gate into a **three-way** split. v61 lands below
72 — sometimes on the *v72-reduced* branch that already diverged twice from the v83
base — so a three-way gate may now have to become a **four-way** split, work no
prior port performed. Two further v61-firsts: the closest labeled anchor (v72) is
**11 versions away** with **no labeled IDB beneath v61** (only an *unlabeled v48
distractor*), and v61 may predate Themida packing (§6, R10).

---

## 0. Baseline & isolation (the stacked-branch decision)

**Decision (settled in brainstorming): task-010 stacks on the task-009 tip.**
task-008 (v79) and task-009 (v72) are functionally complete but **unmerged**; their
branch chain is `main → task-008-gms-v79-support → task-009-gms-v72-support`. The
task-009 tip already carries `memory_maps/GMS/v79_1.cmake`, `v72_1.cmake`, the
floor-lowering gate amendments, and the labeled v72/v79 IDB work this port depends
on. Re-deriving them from `main` would discard exactly the anchors the PRD's
methodology rests on.

- **Branch:** create `task-010-gms-v61-support` from `task-009-gms-v72-support` HEAD,
  in a dedicated git worktree under `.claude/worktrees/` (consistent with task-008/
  task-009). All work happens there; `main` is untouched ([[feedback_no_main_commits]]).
- **Stacked-PR posture (R13).** The PR targets `task-009-gms-v72-support` (or `main`
  once 008/009 have landed). Because the base is unmerged, **rebase onto the final
  task-009 state before merge** and re-confirm every *inherited* gate branch — if
  task-009's below-floor gates change after this task starts, v61's inherited
  branch assignments (§5) shift with them. The inherited-gate re-confirmation is a
  named acceptance step, not an afterthought.
- **Pin the baseline at task start.** Record the task-009 tip SHA in `context.md`/the
  PR so "the baseline" is an exact commit, not "whatever task-009 is today."

---

## 1. Resolved design decisions

These were settled during brainstorming; the rest of the document elaborates them.
D1–D7 port from task-009 one tier lower; **D8 is the v61-specific tightening** (the
four-way split), and D0/D9 are new for this port.

| # | Decision | Choice |
|---|---|---|
| D0 | Git baseline | **Stack on the task-009 tip** in a worktree; rebase + re-confirm inherited gates before merge (§0, R13). |
| D1 | Signature-robustness baseline | **Two independent anchors for every absolute-address key**, ≥1 structural; a byte-signature is never sole evidence. Inherited from task-006/008/009. (§4) |
| D2 | IDB instance topology | **All loaded GMS/JMS IDBs stay pinnable** — v61 target + v72/v79/v83/v84/v87/v95/JMS185 references, **plus an unlabeled v48 distractor that is NOT an anchor**. v61 is **sparse/unverified**; confirm identity with `get_metadata` before every probe, doubly so because two below-floor IDBs (v61, v48) are loaded. Ports resolved at runtime by binary name, never hardcoded. (§2) |
| D3 | Orchestration | **Sequential discovery subagents + serial main-session labeling**, on one serialized IDA lane (`select_instance` is global mutable state). Inherited; see D3-note. (§3) |
| D4 | Live smoke test | **User-run, gates completion. Client confirmed available**, so full FR-16 (build-green + live smoke to title/login) is achievable this task — not deferred. (§7) |
| D5 | Base-branch verification depth (R3) | **Exhaustive v61 size probe of all 24 gated headers** — including the ones expected to match a base branch. There is no labeled IDB below v61, *and* the base is sometimes the already-twice-diverged v72-reduced branch, so every base-branch size is *measured*, not assumed. A match is positive evidence; a mismatch escalates to field-level. (§5.6) |
| D6 | Anchor rule for the high-value tier (R1) | **Two *structural* anchors (kinds 1–5) AND a confirmed v72→v83 chain trace** (via v79 where helpful) for the high-blast tier; byte-sig demoted to tiebreaker only. v61 is **22 versions** below canonical v83 and **11** below the closest labeled anchor v72, so a relocation is accepted only when anchored in v61 *and* corroborated at v72 (gap 11) and v83 (gap 22) — never pattern-matched into v61 alone. The baseline two-anchor rule (D1) still governs ordinary keys. (§4) |
| D7 | Category-A enumerated-gate timing | **Amend the no-`#else` gates early, from CWvsApp/CFuncKeyMappedMan discovery disasm, before the full 24-header struct pass** — so the v61 build becomes compilable as soon as possible and the rest of the audit iterates against a building target. (§5.3) |
| D8 | **Four-way-split convention (R3, NEW tier)** | When a below-floor gate must split because v61 diverges further than v72, **v61's branch uses a GMS-guarded `BUILD_MAJOR_VERSION < 72` range term**, placed **above** the existing `< 79` (v72) arm. This future-proofs the next (even-older) port — a v5x/v4x port falls into the same branch automatically — and stays disjoint because the supported floor is 61 (no supported version is `< 72` except v61). Every split records its full-matrix truth table before commit. (§5.5, §5.7) |
| D9 | **Packing/integrity determination (R10)** | **Determine v61's packing status early** (pre-Themida vs Themida-packed) and record it; it changes patch-site-validity assumptions and the smoke-test fault expectations. A pre-Themida finding is recorded as a first-class result, not a footnote. (§6) |

### D3-note — why discovery is sequential, not parallel

Identical to the task-006/008/009 finding and still binding: `select_instance`
routes **all** subsequent MCP calls to the chosen instance and is **global mutable
state shared across agents** (Claude Code subagents reuse the parent's MCP
connections). Two concurrent subagents that each `select_instance` to different IDBs
clobber each other's routing — a silent wrong-IDB probe, exactly risk **R9**, and
**more dangerous here** because two indistinguishable below-floor IDBs (v61 target,
v48 distractor) are both loaded. Keeping all IDBs loaded removes the *swap cost* but
does **not** make concurrent multi-instance probing safe.

Resolution: keep all IDA probing on a single serialized lane. Discovery subagents run
**one at a time**, each owning a subsystem cluster and holding exclusive use of the
IDA lane for its turn. Only non-IDA work (source-gate truth-table reasoning, catalog
drafting, cmake emission) may fan out concurrently. *If the user later stands up a
second MCP server on a different port, discovery can be re-parallelized by pinning
each agent to a distinct server; the §3.2 contract is written to make that a drop-in
change.*

---

## 2. IDB instance topology

Confirm live via `list_instances` at task start. **Ports are deliberately not
recorded here — they change between sessions.** Subagents resolve the live port for a
version by calling `list_instances` and matching the binary name, then confirm with
`get_metadata` before drawing any conclusion. The v61 IDB is **sparse/unverified**, so
its identity confirmation is doubly important and its existing labels are treated as
minimal/untrusted.

| Version | Role |
|---|---|
| GMS v61.1 (`GMS_v61.1_U_DEVM.exe`) | **Target** — locate, label, `idb_save`. Sparse/unverified; confirm identity every probe; existing labels minimal. |
| GMS v72.1 | **Closest anchor (gap 11)** — task-009-labeled; the below-floor template and the primary relocation source. **ABOVE v61.** |
| GMS v79.1 | **Secondary below-floor x-check (gap 18)** — task-008-labeled; intermediate in the chain trace. **ABOVE v61.** |
| GMS v83 (`MapleStory_dump.exe`) | **Canonical anchor (gap 22)** — most complete symbol set; canonical names/prototypes come from here. **ABOVE v61.** |
| GMS v84 / v87 / v95 (PDB) | Upper anchors / upper-gate cross-validation (FR-12c, FR-13). |
| JMS v185 | JMS-only sentinel confirmation / JMS-branch FR-13 check. |
| **GMS v48.1 (`GMS_v48_1_DEVM.exe`)** | **NOT an anchor — unlabeled distractor.** Loaded only; never triangulate against it (PRD non-goal). Its presence is the reason `get_metadata` is mandatory before every probe. |

There is **no labeled IDB below v61.** v72 is the only nearby anchor and it sits
*above* — the structural reason every v61 verdict must rest on a v61 signature, never
on proximity (R1). v111 is **not** loaded; its gate validation is source-level only
(existing `v111_1.cmake` + build constants), and any v111 case that cannot be settled
from source is surfaced, not assumed (A2).

**Lane discipline (every IDA-touching step):**
1. `list_instances` → resolve the target version's current port by binary name.
2. `select_instance(port=<resolved>)`.
3. `get_metadata` (or `server_health`) to confirm the routed IDB is the intended one
   *before* concluding anything ([[feedback_verify_ida_target]], R9). For v61, confirm
   it is the expected `GMS_v61.1_U_DEVM` binary — **explicitly distinguish it from the
   v48 distractor** — and do not trust a sparse IDB's identity.
4. Probe.
5. On every version switch, repeat 1–3. Never infer the active IDB from "what I
   selected last."

The binary-name → port indirection (rather than a hardcoded port) keeps the design
correct across session restarts where ports are reassigned.

---

## 3. Orchestration architecture

The work decomposes into a **discovery → label → emit** pipeline for the memory map,
a separate **read-only struct/gate audit**, and the **build wiring**. Stages are
ordered to keep the audit free of decompiler leak (§5.8) and to unblock the v61 build
early (D7).

### 3.1 Pipeline overview

```
Phase 0  Reference prep         (main)    — pin the task-009 baseline SHA; re-pin the
                                            live key count from include/memory_map.h.in
                                            (expect 159 = 160 placeholders − BUILD_REGION;
                                            stop if not); seed v61_1.cmake from v72_1.cmake;
                                            generate the 159-row tracking table with exact
                                            v72 seed values; extract v83 canonical names
Phase 1  Discovery              (subagent×N, sequential) — locate v61 addr + anchors,
                                            read-only on every IDB; trace v72→(v79)→v83
                                            chain for the high-value tier (D6); determine
                                            packing status early (D9)
Phase 1a Early Cat-A amendment  (main)    — once CWvsApp + CFuncKeyMappedMan are
                                            located/sized, decide v61's branch and amend
                                            the no-#else gates + CLogin member gate (D7)
Phase 2  Spot-check + label     (main)    — re-probe needs-main-review keys; apply
                                            rename/set_type to v61 IDB; idb_save at
                                            checkpoints
Phase 3  Offsets & sentinels    (main)    — re-derive *_OFFSET from v61 codegen;
                                            confirm sentinel absence (backward direction,
                                            from v72 dispositions)
Phase 4  Emit cmake + catalog   (main + fan-out) — finalize v61_1.cmake (all 159 keys) +
                                            signature-catalog rows + ported/drifted notes
Phase 5  Struct/gate audit      (subagent×N, sequential, READ-ONLY) — all 24 headers
                                            (exhaustive size, D5) + the three-tier
                                            below-floor categories A–D; CWnd cascade
Phase 6  Build wiring           (main)    — add matrix entry first; configure+build
                                            Debug/Release; rebuild affected versions
Phase 7  Acceptance             (user)    — live v61 smoke test; record result
```

Phases 0–4 produce the memory map; Phase 1a is the early build-unblock (D7); Phase 5
is independent and may begin once function/global labels exist (it reads them but must
not write struct types). Phase 6 depends on 4 (cmake complete) and 5 (gate rewrites
landed). Phase 7 depends on 6 (green artifacts).

### 3.2 Discovery subagent contract (Phase 1)

One subagent per subsystem cluster (clusters in §3.3), dispatched **sequentially**
(D3-note). Each is given: its key list with v72 (closest) *and* v83 (canonical) seed
addresses, the binary-name → IDB resolution rule (§2, including the **v48-distractor
guard**), the anchor playbook (the task-009 v72 catalog it consumes), and the anchor
rules (§4).

**The subagent is read-only on every IDB. It does NOT `rename`, `set_type`,
`idb_save`, or apply any struct type.** Its job is to *find and justify*, not mutate
(the "main session labels" decision, D3).

Per key it returns a structured row:

```
key:            C_CLIENT_SOCKET_SEND_PACKET
v61_addr:       0x00XXXXXX
v72_addr:       0x00XXXXXX   # task-009 catalog value used as the relocation seed
proposed_name:  CClientSocket::SendPacket
proto:          int __thiscall CClientSocket::SendPacket(CClientSocket*, COutPacket*)   # or null
anchor_1:       { kind: string_xref, detail: "<exact literal>",            ida_evidence: "<addr/line>" }
anchor_2:       { kind: call_graph,  detail: "called by CClientSocket::OnConnect", ida_evidence: "<addr/line>" }
chain_trace:    <"v72 0xA → v83 0xB confirmed same fn (both anchors hold)" | n/a for non-high-value>  # D6
tiebreaker:     <null | "byte-sig at 0xA matched, 0xB did not — chose 0xA"> # NOT counted as an anchor
v61_vs_v72:     <"string identical to v72" | "v72 literal '%s' drifted to '...' in v61" | "fn inlined/split">
confidence:     high | needs-main-review
ambiguity:      <null | "two candidates 0xA/0xB; chose 0xA because …">
```

`confidence: needs-main-review` is mandatory when (a) the key is high-value or
boundary-adjacent (§4 tier list), (b) anchors disagree across reference versions, (c)
more than one v61 candidate matched, or (d) **the v72/v83 anchor literal/constant
drifted in v61** (a below-floor-specific trigger — the older build is likelier to have
shifted a string or magic number; record the v61 form for the catalog). The main
session **re-probes every `needs-main-review` row independently** before accepting it.

### 3.3 Subsystem clusters (disjoint key sets)

Following `memory-map.md`'s resolution order so call-graph reach accumulates:

1. **WinMain + CWvsApp** (incl. all `C_WVS_APP_*` initializers) — entry point; anchors
   the image and yields call-graph reach. **Resolve first** — gates the early Cat-A
   amendment (D7) and surfaces the `C_WVS_APP_SET_UP` init-sequence question (R11,
   [[project_v84_movement_anticheat_freeze]]; DR_init is expected era-absent for v61 —
   confirm). **WinMain era-feature offsets** (`WIN_MAIN_AD_BALLOON_CONDITIONAL`,
   `WIN_MAIN_PATCHER_OFFSET`) are prime candidates to become **v61 sentinels** if the
   ad-balloon/patcher features don't exist this far back (FR-7, R7).
2. **CClientSocket / ZSocket** — send/flush/process/connect + buffer/close.
3. **COutPacket** — encode primitives + make-buffer-list.
4. **Login / Stage / Logo / CUITitle** — login & stage flow.
5. **Manager singletons** — `*_CREATE_INSTANCE` / `*_INSTANCE_ADDR` pairs. Verify the
   real v61 instance global; don't inherit an allocator-selector address from the
   anchor.
6. **Config / SystemInfo / IGCipher / utilities** — CConfig, CSystemInfo, ZArray/
   ZXString helpers, ZFatalSection ctor/dtor, GET_SE_PRIVILEGE.
7. **Party / migrate senders + offsets** — `C_FIELD_SEND_*`,
   `C_WVS_CONTEXT_SEND_MIGRATE_*` and their `*_OFFSET` keys.
8. **Protocol constants + sentinels + exception-dispatch** — `VERSION_HEADER`,
   `PLAYER_LOGGED_IN`, `CLIENT_START_ERROR` (confirm against v61, **older protocol than
   v72** — R6; cross-check atlas-ms [[reference_atlas_ms]]); the `C_TI_*EXCEPTION` /
   `C_PATCH_EXCEPTION_BUILDER` / `C_COM_RAISE_ERROR_EX` / `C_FILE_STREAM_*` keys
   (carry task-009's v72 CFileStream-relay disposition as the starting hypothesis,
   then confirm — FR-8a); the GMS-absent and JMS-only sentinels; and any **new v61-only
   sentinel** discovered when a v72 real-address feature is absent in v61 (R7, backward
   direction). Resolved last (FR-7).

Clusters are address-disjoint, so even if labeling were later parallelized no two
writers touch the same address.

---

## 4. Evidence rule: anchors per address (D1, D6)

Every absolute-address key requires **two independent, version-stable anchors** that
both resolve to the same v61 address before it is written to cmake. "Two independent"
means two *different kinds* of evidence, or two clearly distinct instances of the same
kind — not one fact stated twice.

**Anchor kinds, in preference order:**
1. String xref — a unique literal the function references.
2. Import/API call anchor — e.g. the function calling `connect`/`socket`.
3. Call-graph anchor — child/parent of an already-resolved function.
4. Constant / opcode immediate — a magic number or `push <opcode>`.
5. Vtable slot — via the class RTTI/type string, then slot index.
6. Byte/structure signature (`make_signature`) — **tiebreaker only for v61; never an
   anchor.**

Rules:
- **Baseline (all keys):** at least **one** anchor must be from kinds 1–5. A pair of
  two byte-sigs never satisfies the rule.
- **High-value / high-blast tier (D6) — socket send/flush/process, the COutPacket
  encoders, WinMain, CWvsApp::Run/SetUp, the packet senders, and any function adjacent
  to a below-floor gate (§5):** require **two anchors of kinds 1–5** (two *structural*
  anchors, of two different kinds) **plus a confirmed v72→v83 chain trace** — the same
  function identified in v72 *and* v83 (using v79 as the intermediate where it
  disambiguates) with both anchors holding, so the v61 relocation is corroborated at
  the neighbor versions rather than pattern-matched into v61 in isolation. The
  22-version v61↔v83 gap makes a single-version byte match unsafe; the chain trace is
  the mitigation. A byte-signature may only *disambiguate* two structurally-anchored
  candidates; it does not count toward the pair. These keys are flagged
  `needs-main-review` (§3.2).
- **v61-vs-v72 drift capture:** because v61 is older, a v72/v83 anchor string/constant
  may have a different form in v61. When the working anchor differs from the v72
  reference, record the **v61-specific form** in `signature-catalog.md`, plus an
  explicit "ported directly / drifted" note (FR-14) so the next (older) backward port
  benefits — and so the catalog captures *which* task-009 heuristics survived one more
  tier down.
- **Offset keys** (`*_OFFSET`, `WIN_MAIN_AD_BALLOON_CONDITIONAL`,
  `WIN_MAIN_PATCHER_OFFSET`, `C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET`) are not addresses;
  they are re-measured directly from the v61 host function's disassembly (FR-5, R2).
  The "anchor" for an offset is the identified target instruction/branch plus the byte
  delta to it, shown in the catalog. **Never copy the v72 or v83 offset.** Where the
  host *feature* is era-absent, the offset key becomes a sentinel (below).
- **Protocol constants** (`VERSION_HEADER`, `PLAYER_LOGGED_IN`, `CLIENT_START_ERROR`)
  are confirmed against the v61 opcode table / handler dispatch, not copied from
  v72/v83 (FR-6, R6). v61 predates v72 (which confirmed `VERSION_HEADER`=8), so drift
  is *more* plausible here — if a value differs, the v61 value is used and the delta
  noted (PRD §9). Cross-check against atlas-ms's version-aware registries where
  applicable ([[reference_atlas_ms]]).
- **Sentinels (FR-7) — backward direction.** Absence is *confirmed*, not assumed. For
  each v72 sentinel, search v61 for the feature's identifying anchor (strings,
  CreateInstance call site, vtable); only after the anchor is shown absent does the
  `0x00000000` + comment carry forward. **Additionally** (the backward-direction case,
  R7): watch for v72 *real-address* keys whose backing feature does not exist in v61 —
  **ad balloon, patcher window, MTS map restriction, beginner-party block, the
  CFileStream relay helpers, MTS** — those become **new v61-only sentinels**, flagged
  for the gate/edit owner rather than silently zeroed, and the consuming edit/gate must
  tolerate `0`. Start each disposition from task-009's **v72** verdict — not the stale
  v83 comments — then confirm in v61. JMS-only sentinels are confirmed present in
  JMS185 and shown absent in v61.

Every accepted address lands one row in `signature-catalog.md` recording its anchors
and their cross-version stability (v83→v79→v72→v61) — the durable artifact for the next
version port (FR-14).

---

## 5. Three-tier below-floor struct verification & gate audit (Phase 5)

This is the dimension that makes v61 categorically harder than v72. task-008 made
**v79** the floor and rewrote the `common/` gates; task-009 lowered the floor to **72**
and, for several structs, turned task-008's two-way `>= 83 || JMS` split into a
**three-way** split (`>= 83 || JMS` / `GMS && < 79` / `#else`). v61 sits **below 72**,
so it re-triggers every below-floor failure mode one notch lower still — against a gate
landscape **two** prior tasks already reshaped. The central new question: for each gate
task-009 split, **the branch v61 lands on is the v72-reduced branch, which already
diverged from the v79-reduced base, which already diverged from v83** — and v61 may
diverge from v72 in turn, forcing a **four-way** split.

The pass is **read-only over raw disassembly** (§5.8). It runs *after* function
labeling (Phases 2–3) so method-body disasm shows resolved call targets, but writes no
struct types to the v61 IDB.

### 5.1 The four failure modes (priority order)

| # | Failure mode | Gate forms | Where v61 lands | Action |
|---|---|---|---|---|
| A | **No branch at all** (guard silently lost) | enumerated `== 72 \|\| == 79 \|\| == 83 …`, `== 83` member gate — no `#else` | nothing — guard doesn't fire / member dropped | Amend first (§5.3) |
| B | **Inherited reduced branch, v61 maybe further-reduced** (the novel risk) | task-009 three-way `>= 83 \|\| JMS` / `< 79` / `#else`, **or** still-two-way `>= 83 \|\| JMS` / `#else` | the `< 79` (v72) arm, or the shared `#else` | Confirm `v61 == v72` or split (→ four-way / three-way) (§5.4–5.5) |
| C | **Base/excluded branch** (quietest) | `>= 84/87/95/111`, `== 95/87/83`, `> 83`, `< 95/84`, `< 79`, `< 72` | base, same side as v72 | **Exhaustive size probe** — D5 (§5.6) |
| D | **CWnd cascade** | base shift in CWnd | propagates to ≥5 derived classes | one linked verdict (§5.7) |

> Note the gate arity v61 inherits is **per-gate**: where task-009 left a gate
> two-way (because v72 == v79), v61 shares the `#else`; where task-009 made it
> three-way, v61 rides the `< 79` arm. Either way, a v61 divergence adds a **`< 72`
> arm** (D8) — turning a two-way into three-way, or a three-way into four-way.

### 5.2 Subagent contract (struct verifier)

The 24 headers in `struct-verification.md` are split across sequential read-only
subagents (one IDA lane, D3-note). The existing `version-port-verifier` subagent is
the vehicle where a header is non-trivial; trivial size-only confirmations may be done
inline in the main session.

Per header the subagent returns: **v61 size** (always — D5), present/absent verdict for
each version-gated field, the disassembly anchor for each verdict (a
`Decode`/`DecodeBuffer(this,N)` literal, an `imul stride,N` in `RemoveAll`, a ctor
member-init or destructor unwind extent, an Alloc immediate at a CreateMob/
CreateInstance site), an explicit **v61-vs-v72 size comparison** (mandatory for the
below-floor reduced-branch headers), and a gate verdict (`unchanged` / `branch-added` /
`confirmed-shares-v72` / `split` (three- or four-way) / `rewritten`) with the deciding
v61 disasm line(s). Embedded UDT sizes (e.g. `SecondaryStat`'s embedded block) are
verified independently — not assumed equal to v72/v83.

**Main-session cross-check:** before any gate is rewritten, the main session re-anchors
at least one boundary case per changed header with an independent probe. Subagent
headline verdicts are treated as drafts; the per-offset evidence is trusted, the
verdict is verified.

### 5.3 Category A — amend FIRST, early, from disasm (D7, R4)

These select by explicit version equality with no catch-all `#else`, so v61 selects
**nothing** (size guard silently does not fire) or drops a member. Re-grep at task
start (`grep -rn "BUILD_MAJOR_VERSION ==" common/*.h`) against the task-009 baseline —
line numbers shifted when task-009 added its `72` branches, and other enumerated-no-
`#else` gates may exist. Expected sites (verify live):
- `common/CWvsApp.h` — the size-assert version-equality branch now enumerates the
  supported set (`… == 72 == 79 == 83 == 84`) and is false for v61 → no branch,
  `assert_size` doesn't fire. Verify v61 `CWvsApp` layout; add `61` to the matching
  branch (decided by v61 disasm, likely the branch v72/v79 joined).
- `common/CFuncKeyMappedMan.h` — the size-assert branch enumerates `… == 72 == 79`
  (task-009 added 72) and is false for v61 → no branch. The member gate already
  excludes v61; confirm the v61 size and add `61` to the matching size-assert branch.
- `common/CLogin.h` — the `== 83` member-declaring gate is false for v61 (excluded,
  like v72/v79). Confirm v61 **genuinely lacks** the gated member rather than silently
  dropping a member it has.

Per D7, as soon as Cluster 1 (WinMain + CWvsApp) and the CFuncKeyMappedMan key are
located and sized in discovery, **decide which branch v61 joins from that disassembly**
(not "oldest ⇒ v83 branch") and amend these gates so the v61 build compiles with its
guards restored. The remaining headers are audited afterward against a building target.
Which branch v61 joins is itself an evidence verdict recorded in
`struct-verification.md`.

### 5.4 Category B — inherited reduced branch: confirm or split (the novel tier)

For each below-floor gate, v61 inherits the branch its comparator selects — the `< 79`
(v72) arm of a task-009 three-way, or the shared `#else` of a still-two-way gate. For
each, confirm **v61 == v72** (gate already correct — record the confirming size and
move on) or **v61 diverges further** (the gate gains a `< 72` arm per §5.5). Expected
sites (verify the live set against the task-009 baseline) and the v72 reduced size to
re-confirm against v61:

| Site | Field gated below floor | v72 reduced size (re-confirm) | v61 must confirm |
|---|---|---|---|
| `common/CWnd.h` | secondary-layer com_ptrs | (task-009 v72 CWnd size) | v61 `sizeof(CWnd)` via Destroy/ctor landmarks (**cascade root — §5.7, do first**) |
| `common/CMob.h` | doom/reserved tail | (task-009 v72 CMob size) | v61 CMob size via `?CreateMob@@` Alloc immediate + ctor highest write |
| `common/MobStat.h` | Weakness group | (task-009 v72 MobStat size) | v61 MobStat tail via CMob ctor `lea` to last member |
| `common/CFuncKeyMappedMan.h` | quickslot pair | (task-009 v72 size) | v61 CreateInstance Alloc immediate + ctor extent |
| `common/CUIToolTip.h` | `m_pLayerAdditional` | (task-009 v72 size) | v61 ctor `m_pLayer` → `m_aLineInfo` adjacency |
| `common/SecondaryStat.h`, `PartyData.h`, `PartyMember.h`, `GuildData.h` | bitmask/array-indexed members | (task-009 v72 sizes) | v61 sizes; **cross-check atlas-ms** ([[reference_atlas_ms]]) |

Because there is no IDB below v61, "v61 == v72" is **only** acceptable with a v61
signature confirming the size — never by proximity ([[feedback_prefer_confirmation]]).

### 5.5 Split when v61 diverges — the four-way convention (D8)

When a Category-B size probe shows v61 ≠ v72, the gate gains an arm. Per **D8**, v61's
branch uses a **GMS-guarded `BUILD_MAJOR_VERSION < 72` range term**, placed **above**
the existing `< 79` (v72) arm. Canonical shape, extending a task-009 three-way into a
four-way (CWnd example; the first arm already catches v83+ and JMS185 because 185 ≥ 83):

```c
#if (BUILD_MAJOR_VERSION >= 83 || defined(REGION_JMS))
    // full v83 layout
#elif (defined(REGION_GMS) && BUILD_MAJOR_VERSION < 72)
    // NEW: v61 further-reduced layout
#elif (defined(REGION_GMS) && BUILD_MAJOR_VERSION < 79)
    // v72-reduced layout (GMS 72..78) — UNCHANGED in effect
#else
    // v79-reduced layout (GMS 79..82) — UNCHANGED in effect
#endif
```

(Where the gate task-009 left **two-way**, the same `< 72` arm turns it three-way
instead — the convention is identical; only the number of pre-existing arms differs.)

Why `< 72` over `== 61`:
- **Future-proofs the next older port.** A v5x/v4x port inherits the v61 branch with no
  edit (unless it diverges again, in which case it splits *that* branch — the same
  pattern one notch lower).
- **Stays disjoint.** The supported floor is 61; no supported version satisfies `< 72`
  except v61, so the term cannot flip another version's truth value (FR-13).
- **GMS-guarded** so JMS/other regions never fall into it (JMS185 is already caught by
  the first arm).
- **Ordering is load-bearing.** `< 72` must precede `< 79` (every `< 72` version also
  satisfies `< 79`); the new arm goes **above** the v72 arm, and the existing `< 79`
  and `#else` arms must remain **byte-identical in effect** — the split adds an arm
  above them, never edits them.

Every split records its full-matrix truth table (§5.9 template) and is preprocessed
(`gcc -E`) at `{61, 72, 79, 83}` to prove each field's present/absent at each version
before commit.

### 5.6 Category C — base/excluded branch: exhaustive size probe (D5, R3 — quietest)

Every `>= 84/87/95/111`, `== 95/87/83`, `> 83`, `< 95/84`, **`< 79`, `< 72`** gate
resolves v61 onto a base/excluded branch (same side as v72). Individually low-risk, but
a silent v61≠v72 (or v61≠v83) base-size delta corrupts every downstream offset and
**cannot be caught by triangulation** — there is no anchor below v61. Per **D5**,
verification is **exhaustive: probe the v61 struct size of every gated header (all 24),
including those expected to match the base.** A size match is recorded as evidence; a
mismatch escalates that header to field-level verification.

Special attention (named in `struct-verification.md`):
- `common/CWvsContext.h` (`> 83`, `m_aClientKey[8]`) — false for v61 → key treated
  **absent** (v72 confirmed absent). Confirm v61's `CClientSocket::OnConnect`
  connect-hello does **not** encode an 8-byte client key. Drives `bypass/socket_hooks.cpp`
  (`> 83`). [[project_v84_clientkey_gate_trap]] — verify decode order against the v61
  binary, not a server round-trip.
- `common/SecondaryStat.h` embedded size — confirm v61 computes the same embedded size
  as the v72 base; a wrong embedded size shifts every field after `m_forcedStat` in
  `CWvsContext`. Cross-validate bitmask gating against atlas-ms ([[reference_atlas_ms]]).
- `doom-fix/dllmain.cpp` (`== 83` write gate) — false for v61 → write not applied.
  Confirm the doom-reserved field is genuinely absent in v61 (so exclusion is correct
  and the write would have been OOB), as task-008/009 found for v79/v72.

### 5.7 Category D — CWnd cascade (R5)

The CWnd base shift propagates by inheritance to **CDialog, CUIWnd, CFadeWnd, CUITitle,
CUILoginStart**. If v61's CWnd differs from v72's size, all derived headers shift again.
Treat the cascade as **one linked verdict, not five independent ones**: pin v61
`sizeof(CWnd)` **first** via three independent landmarks (per task-008/009's method),
then re-derive each derived class from that base. If v61 CWnd == v72, the derived
classes inherit v72's verdict (confirm each size anyway, D5); if v61 CWnd diverges,
every derived class is re-measured and the split (§5.5) **cascades** — potentially a
four-way arm in CWnd *and* each derived header.

### 5.8 Read-only discipline & decompiler-leak avoidance

Applying inferred struct types into the v61 IDB makes Hex-Rays echo the inferred field
names back, masking the very deltas being verified. So: use `disasm`, not `decompile`,
for layout evidence; **do not apply speculative struct types** during Phase 5
(`docs/version-porting-workflow.md`, PRD §8). Function/global *labels* from Phases 2–3
are fine and present (they label symbols, not layout). No conflict: labeling writes
symbols, the audit reads layout.

### 5.9 Gate-rewrite expression strategy & cross-version safety (FR-13, R12)

- **Confirm-in-place when possible.** If v61 behaves like the branch the current gate
  already selects, leave it unchanged and record the evidence. "No change, here's why"
  is a first-class outcome and is expected to be the common case for Category B.
- **Minimal correct boundary when v61 diverges.** For Category A, extend the enumerated
  list with `61` per the discovered branch. For Category B splits, add the `< 72` arm
  (D8) above the unchanged `< 79`/`#else` arms. Keep the house
  `#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION …)` guard form. Gate the minimum
  contiguous region; no ternary in array dimensions (use `#if/#else/#endif`).
- **Validate every rewrite across the full matrix.** A rewrite must keep
  **v72/v79/v83/v84/v87/v95/v111/JMS185** selecting their current branch — four-way
  splits in particular must keep **v72's and v79's** selected branches unchanged. For
  the loaded IDBs (v72/v79/v83/v87/v95/JMS185 — and v84) confirm the comparator's truth
  value and re-anchor any layout claim the rewrite touches. For **v111** (not loaded)
  evaluate from build constants + existing `v111_1.cmake`; surface any unsettleable
  v111 case rather than assuming it (A2). Keep v61's exclusion **disjoint** (`< 72`,
  which no other supported version satisfies). CI builds all matrix versions on PR as
  the final backstop, but source-level reasoning precedes it — a green build proves the
  branch *compiled*, not that it is semantically right.

#### Four-way split truth-table template (fill per split)

| Version | Branch selected | size | Changed? |
|---|---|---|---|
| GMS 61 | `< 72` (GMS-guarded) | (v61 size) | NEW |
| GMS 72 | `< 79` (GMS-guarded) | (v72 size) | unchanged |
| GMS 79 | `#else` | (v79 size) | unchanged |
| GMS 83/84/87 | `>= 83` | (v83 size) | unchanged |
| GMS 95/111 | `>= 83` (+ any `>= 95` tail) | … | unchanged |
| JMS 185 | `>= 83 \|\| REGION_JMS` (first arm) | … | unchanged |

**Empirical verification (fill):** `scripts/wsl-build.sh GMS 61 1` → `>> OK`;
`scripts/wsl-build.sh GMS 72 1` and `… GMS 79 1` (neighbor sanity) → `>> OK`;
the CMake key-completeness check passes for GMS 61.1; `gcc -E` at `{61, 72, 79, 83}`
confirming each split field's present/absent at each version.

---

## 6. Build-matrix wiring & packing determination (Phase 6 / D9)

- Add `{ region: GMS, major: 61, minor: 1 }` to `strategy.matrix.config` in
  `.github/workflows/_build.yml`, **placed first** to keep the list version-ascending
  (single source of truth; PR/snapshot/release inherit it — FR-1/FR-2). No other
  workflow file is edited.
- `memory_maps/GMS/v61_1.cmake` is **seeded from `v72_1.cmake`** (closest; carries the
  below-floor relocations and sentinel dispositions one tier above v61) and must define
  **every** key parsed from `include/memory_map.h.in` (re-pinned live in Phase 0 —
  expect 159 = 160 placeholders − `BUILD_REGION`; stop and reconcile if not), non-empty;
  the CMake key-completeness check (`cmake/CheckMemoryMapKeys.cmake`) fails the build
  otherwise (FR-3). **Every seeded value is UNVERIFIED until a v61 signature confirms
  it** — a value still equal to v72 means UNVERIFIED, not "done."
- Local pre-flight: `scripts/wsl-build.sh GMS 61 1` compiles+links the v61 build on the
  WSL/clang-cl path before the Windows/CI handoff ([[project_wsl_cross_compile]]). Build
  **Debug and Release** for GMS 61.1 green (FR-15), and rebuild the other matrix
  versions affected by any gate rewrite to prove no regression (R12) — especially v72
  and v79, whose branches every four-way split must preserve.
- **Packing determination (D9, R10).** Early in the port (during Cluster-1 discovery),
  determine whether the v61 image is **Themida-packed or pre-Themida** (entry-point
  section names, import-table state, the presence/absence of the Themida stub the newer
  builds carry). Record the finding in `context.md`/`acceptance.md`: it changes (a)
  patch-site validity assumptions and (b) the smoke-test fault expectations — a
  pre-Themida client will not throw the Themida integrity faults the README documents,
  so a clean launch is interpreted differently.

---

## 7. Verification & acceptance strategy

Two distinct evidence classes, kept separate:

**Build-correctness (automated gate).**
- GMS 61.1 configures + builds clean, Debug and Release, locally and in CI.
- All other supported versions remain green (gate-rewrite regression guard; v72 and v79
  branches preserved through every four-way split).
- The CMake key-completeness check passes (every required key present, non-empty).
- Every absolute key has a catalog row meeting §4 (two structural anchors + v72→v83
  chain trace for the high-value tier); every offset is re-derived from v61 codegen;
  every sentinel is justified in the backward direction (incl. any new v61-only
  sentinels for era-absent features); all 24 headers have a recorded v61 size + gate
  verdict (and, for the below-floor headers, an explicit v61-vs-v72 comparison) in
  `struct-verification.md`.
- The v61 IDB is labeled for all resolved functions/globals and `idb_save`d.
- **Inherited-gate re-confirmation (R13).** Before merge, after rebasing onto the final
  task-009 state, re-confirm that every gate branch v61 *inherited* (rather than
  amended) still selects the intended layout — task-009 changing a below-floor gate
  after this task started would silently move v61's branch.

**Runtime-correctness (user-run, gates completion — D4).**
- The user launches a live GMS v61 client (confirmed available) with the proxy
  `ijl15.dll` and the core edits deployed (per `acceptance.md`), reaches title/login,
  exercises the targeted edits, and confirms no crash / no Themida fault (FR-16),
  interpreted against the packing finding (§6/D9).
- The exact result is recorded in `acceptance.md` / the PR. The task is **not done**
  until this is run and the outcome pasted. A correct build with a failed/blocked smoke
  test is reported as such, distinguishing build issues from environment issues
  (Themida / VC++ redist / OS) per README compatibility notes
  ([[reference_version_port_playbook]]).

---

## 8. Risk control mapping

| Risk | Control in this design |
|---|---|
| R1 wrong-address relocation (11-version v61↔v72 gap, no IDB below) | §4/D6 two structural anchors **+ v72→v83 chain trace** for high-value tier; §3.2 needs-main-review re-probe; never offset-arithmetic from v72 |
| R2 stale instruction-relative offset | §4 offsets re-measured from v61 disasm, never copied; era-absent host → sentinel |
| R3 below-floor divergence v61 ≠ v72 → four-way split (NEW) | §5.4–5.5 confirm-or-split with the `< 72` convention (D8); §5.6 exhaustive size probe (D5); no "same as v72" without a v61 probe |
| R4 enumerated gate has no v61 branch (guard lost) | §5.3 amends Category A first/early from disasm (D7); branch chosen by evidence |
| R5 CWnd cascade differs at v61 | §5.7 pin v61 `sizeof(CWnd)` first, treat derived classes as one linked verdict; four-way arm cascades |
| R6 protocol-constant drift (older build) | §4 constants confirmed against v61 opcode/handler table; atlas-ms cross-check |
| R7 era-absent feature treated as present (backward-direction sentinel) | §3.3 Cluster 1/8 + §4 sentinel rule (backward); new v61-only sentinels flagged, edit/gate tolerates `0` |
| R8 IDB ≠ smoke-test binary | §2 record IDB build string; §7 user runs the matching v61 client; result recorded |
| R9 wrong/unverified-IDB probe (v48 distractor loaded) | §2 lane discipline — binary-name→port resolution, `get_metadata` after every `select_instance`, explicit v48 distinction; D3-note (no concurrent selection) |
| R10 packing differs at v61 (possibly pre-Themida) | §6/D9 determine packing status early; adjust patch-validity + smoke-test expectations; record the finding |
| R11 `C_WVS_APP_SET_UP` init-sequence regression | §3.3 Cluster 1 surfaces the SetUp init sequence; DR subsystem expected era-absent for v61 — confirm, don't assume ([[project_v84_movement_anticheat_freeze]], [[reference_version_port_playbook]]) |
| R12 four-way split regresses v72/v79 or another version | §5.5/§5.9 `< 72` disjoint term, existing arms untouched, full-matrix truth table + `gcc -E` + CI |
| R13 branch drift from the task-009 baseline | §0 stack-and-pin the baseline SHA; rebase onto final task-009 before merge; §7 inherited-gate re-confirmation |

---

## 9. Out of scope (mirrors PRD §2 non-goals)

- No new client-edit features; relocation only.
- No struct-internals re-port beyond size + the version-gated field boundaries.
- No changes to non-GMS regions, or to other GMS versions except a region-correct gate
  rewrite that also touches another version's compiled branch (a four-way split that
  adds an arm above v72's/v79's unchanged branches).
- Not aiming for 100% v61 IDB symbol coverage — only the symbols backing the 159 keys
  and the 24 verified structs.
- No labeling or porting of the v48 IDB (referenced only to avoid mistaking it for an
  anchor).

---

## 10. Assumptions & open items

- **A1 — topology must hold for the run.** The v61 target + v72/v79/v83/v84/v87/v95/
  JMS185 references must be loaded; the v48 distractor is loaded but never an anchor.
  If a session restarts and an anchor is missing — **especially v72 (closest/template)
  or v83 (canonical names)** — discovery/audit must pause until it is reloaded rather
  than silently falling back to a further anchor. Ports are resolved per-session by
  binary name (§2), never hardcoded. The v61 IDB is sparse/unverified — identity
  confirmed every probe and distinguished from v48.
- **A2 — v111 is source-validated only.** Its IDB is not loaded; gate rewrites are
  validated for v111 from build constants + existing `v111_1.cmake`. Any unsettleable
  v111 case is surfaced, not assumed ([[feedback_prefer_confirmation]]).
- **A3 — protocol-constant equality v72↔v61 is unverified until Phase 1** (PRD §9). The
  design handles both same and different outcomes; v61 is an older protocol revision
  than v72, so drift is treated as *more* plausible (R6).
- **A4 — sentinel dispositions unknown until §4 runs.** Whether GMS-absent sentinels
  (CBattleRecordMan, DRCheck, DRInit, CETracer) are also absent in v61 (expected),
  whether the `C_FILE_STREAM_*` relay helpers are recoverable in v61, and whether any
  v72 real-address feature (ad balloon, patcher, MTS, beginner-party) is absent in v61
  (a new v61-only sentinel, R7) are all resolved during discovery, starting from
  task-009's v72 verdicts, not v83's stale comments.
- **A5 — Category-A branch assignment is an evidence verdict.** Whether v61's `CWvsApp`
  and `CFuncKeyMappedMan` layouts join the v72 below-floor branch or need their own is
  decided by v61 disassembly in Phase 1a (D7/§5.3), not by "oldest ⇒ v83."
- **A6 — Category-B confirm-vs-split is empirical per struct.** "v61 == v72" is the
  expected common case but is accepted only with a confirming v61 size signature; the
  four-way split (D8) is the contingency for genuine divergence, and the CWnd split (if
  any) cascades through the derived classes (§5.7).
- **A7 — serial IDA lane.** Discovery is serialized because `select_instance` is global
  state (D3-note); the loaded v48 distractor makes a wrong-IDB probe more likely, not
  less. If the user stands up a second MCP server, discovery can be parallelized per
  §3.2 — flagged for the user's review.
- **A8 — baseline is the task-009 tip, pinned by SHA (§0).** This task assumes task-009
  lands substantially as-is; if task-009's below-floor gates change materially before
  merge, the inherited-gate re-confirmation (§7) catches the drift, but a large task-009
  change may warrant re-running affected Category-B confirmations.
