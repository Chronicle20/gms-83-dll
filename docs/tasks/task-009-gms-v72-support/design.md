# GMS v72 Support — Design

Status: Draft for review
Created: 2026-06-26
Inputs: `prd.md`, `memory-map.md`, `struct-verification.md`, `signature-catalog.md`,
`risks.md`, `context.md`, `acceptance.md`

This document defines *how* the v72 port is executed. It does not restate the
*what/why* (see `prd.md`) or the per-key tracking table and anchor playbook (see
`memory-map.md` / `signature-catalog.md`). It records the methodology and
architecture decisions, the subagent contracts, the sequencing that keeps the
struct-verification pass clean, the **two-tier below-floor gate-audit strategy**
that makes v72 categorically harder than v79, and the acceptance strategy.

The v79 port (`docs/tasks/task-008-gms-v79-support/design.md`) is the direct
methodological precedent — and, for the first time, a *consumed* artifact: its
signature catalog, its labeled v79 IDB, and its below-floor gate verdicts are the
closest anchor and template for this port. Where a decision is identical to task-008,
this document says so and moves on. The genuinely new material is the **two-tier
below-floor dimension** (§5): v72 lands below the *new* floor (79) that task-008
established, sometimes on the *v79-reduced* branch that already diverged from the
v83 base — so for five structs the two-way gate task-008 created may have to become a
**three-way split**, work no prior port performed.

---

## 1. Resolved design decisions

These were settled during brainstorming; the rest of the document elaborates them.
D1–D4 and D7 port directly from task-008; D5, D6, and D8 are the v72-specific
tightenings and the new tier.

| # | Decision | Choice |
|---|---|---|
| D1 | Signature-robustness baseline | **Two independent anchors for every absolute-address key**, ≥1 structural; a byte-signature is never sole evidence. Inherited from task-006/008. (§4) |
| D2 | IDB instance topology | **All loaded GMS/JMS IDBs stay pinnable** — v72 target + v79/v83/v87/v95/JMS185 references. v72 is **sparse/unverified**; confirm its identity with `get_metadata` before every probe. Ports resolved at runtime by binary name, never hardcoded. (§2) |
| D3 | Orchestration | **Sequential discovery subagents + serial main-session labeling**, on one serialized IDA lane (`select_instance` is global mutable state). Inherited from task-006/008; see D3-note. (§3) |
| D4 | Live smoke test | **User-run, gates completion.** CI-green is the automated bar; FR-16 is a manual step the user performs and records before the task is "done." (§7) |
| D5 | Base-branch verification depth (R3) | **Exhaustive v72 size probe of all 24 gated headers** — including the ~14–19 expected to match a base branch. There is no labeled IDB below v72, *and* the base is sometimes the already-diverged v79-reduced branch, so every base-branch size is *measured*, not assumed. A match is positive evidence; a mismatch escalates to field-level. (§5.6) |
| D6 | Anchor rule for the high-value tier (R6) | **Two *structural* anchors (kinds 1–5) AND a confirmed v79→v83 chain trace** for the high-blast tier; byte-sig demoted to tiebreaker only. v72 is ~12 versions below the canonical v83 anchor, so a relocation is accepted only when it is anchored in v72 *and* traced through the v79 (gap 7) and v83 (gap 12) neighbors — never pattern-matched into v72 alone. The baseline two-anchor rule (D1) still governs ordinary keys. (§4) |
| D7 | Category-A enumerated-gate timing | **Amend the no-`#else` gates early, from CWvsApp/CFuncKeyMappedMan discovery disasm, before the full 24-header struct pass** — so the v72 build becomes compilable as soon as possible and the rest of the audit iterates against a building target. (§5.3) |
| D8 | Three-way-split convention (R3/R11, **new tier**) | When a Category-B two-way gate must split because v72 diverges further than v79, **v72's branch uses a GMS-guarded `BUILD_MAJOR_VERSION < 79` range term**, not `== 72`. This future-proofs the next (even-older) port — v6x falls into the same branch automatically — and stays disjoint because the supported floor is 72 (no supported version is `< 79` except v72). Every split records its full-matrix truth table before commit. (§5.5, §5.7) |

### D3-note — why discovery is sequential, not parallel

Identical to the task-006/008 finding and still binding: `select_instance` routes
**all** subsequent MCP calls to the chosen instance and is **global mutable state
shared across agents** (Claude Code subagents reuse the parent's MCP connections).
Two concurrent subagents that each `select_instance` to different IDBs clobber each
other's routing — a silent wrong-IDB probe, exactly risk **R9**. Keeping all IDBs
loaded removes the *swap cost* (switching is an instant `select_instance` rather than
a close/reopen) but does **not** make concurrent multi-instance probing safe.

Resolution: keep all IDA probing on a single serialized lane. Discovery subagents
run **one at a time**, each owning a subsystem cluster and holding exclusive use of
the IDA lane for its turn (free to switch among all loaded IDBs within that turn).
Only non-IDA work (source-gate truth-table reasoning, catalog drafting, cmake
emission) may fan out concurrently. *If the user later stands up a second MCP server
bound to a different port, discovery can be re-parallelized by pinning each agent to a
distinct server; the §3.2 contract is written to make that a drop-in change.*

---

## 2. IDB instance topology

Confirm live via `list_instances` at task start. **Ports are deliberately not
recorded here — they change between sessions.** Subagents resolve the live port for a
version by calling `list_instances` and matching the binary name, then confirm with
`get_metadata` before drawing any conclusion. The v72 IDB is **sparse/unverified**,
so its identity confirmation is doubly important and its existing labels are treated
as minimal/untrusted.

| Version | Role |
|---|---|
| GMS v72.1 | **Target** — locate, label, `idb_save`. Sparse/unverified; confirm identity every probe; existing labels minimal. |
| GMS v79.1 | **Closest anchor (gap 7)** — task-008-labeled; the below-floor template and the primary relocation source. **ABOVE v72.** |
| GMS v83 | **Primary/canonical anchor (gap 12)** — most complete symbol set; canonical names/prototypes come from here. **ABOVE v72.** |
| GMS v87 | Secondary anchor / upper-gate cross-validation (FR-13) |
| GMS v95 (PDB) | PDB-derived reference / upper-gate cross-validation |
| JMS v185 | JMS-only sentinel confirmation / JMS-branch FR-13 check |

There is **no labeled IDB below v72.** v79 is the only nearby anchor and it sits
*above* — the structural reason every v72 verdict must rest on a v72 signature, never
on proximity (R6). v111 is **not** loaded; its gate validation is source-level only
(existing `v111_1.cmake` + build constants), and any v111 case that cannot be settled
from source is surfaced, not assumed (A2).

**Lane discipline (every IDA-touching step):**
1. `list_instances` → resolve the target version's current port by binary name.
2. `select_instance(port=<resolved>)`.
3. `get_metadata` (or `server_health`) to confirm the routed IDB is the intended one
   *before* concluding anything ([[feedback-verify-ida-target]], R9). For v72,
   confirm it is the expected retail binary — do not trust a sparse IDB's identity.
4. Probe.
5. On every version switch, repeat 1–3. Never infer the active IDB from "what I
   selected last" — re-confirm after each switch.

The binary-name → port indirection (rather than a hardcoded port) is what keeps the
design correct across session restarts where ports are reassigned.

---

## 3. Orchestration architecture

The work decomposes into a **discovery → label → emit** pipeline for the memory map,
a separate **read-only struct/gate audit**, and the **build wiring**. Stages are
ordered to keep the audit free of decompiler leak (§5.8) and to unblock the v72 build
early (D7).

### 3.1 Pipeline overview

```
Phase 0  Reference prep         (main)    — re-pin the live key count from
                                            include/memory_map.h.in (expect 159; stop
                                            if not); seed v72_1.cmake from v79_1.cmake;
                                            generate the 159-row tracking table with
                                            exact v79 seed values; extract v83 canonical
                                            names per key
Phase 1  Discovery              (subagent×N, sequential) — locate v72 addr + anchors,
                                            read-only on every IDB; trace v79→v83 chain
                                            for high-value tier (D6)
Phase 1a Early Cat-A amendment  (main)    — once CWvsApp + CFuncKeyMappedMan are
                                            located/sized, decide v72's branch and amend
                                            the no-#else gates + CLogin member gate (D7)
Phase 2  Spot-check + label     (main)    — re-probe needs-main-review keys; apply
                                            rename/set_type to v72 IDB; idb_save at
                                            checkpoints
Phase 3  Offsets & sentinels    (main)    — re-derive *_OFFSET from v72 codegen;
                                            confirm sentinel absence (backward direction)
Phase 4  Emit cmake + catalog   (main + fan-out) — finalize v72_1.cmake (all 159 keys) +
                                            signature-catalog rows + drift notes
Phase 5  Struct/gate audit      (subagent×N, sequential, READ-ONLY) — all 24 headers
                                            (exhaustive size, D5) + the two-tier
                                            below-floor categories A–C; CWnd cascade
Phase 6  Build wiring           (main)    — add matrix entry first; configure+build
                                            Debug/Release; rebuild affected versions
Phase 7  Acceptance             (user)    — live v72 smoke test; record result
```

Phases 0–4 produce the memory map; Phase 1a is the early build-unblock (D7); Phase 5
is independent and may begin once function/global labels exist (it reads them but must
not write struct types). Phase 6 depends on 4 (cmake complete) and 5 (gate rewrites
landed). Phase 7 depends on 6 (green artifacts).

### 3.2 Discovery subagent contract (Phase 1)

One subagent per subsystem cluster (clusters in §3.3), dispatched **sequentially**
(D3-note). Each is given: its key list with v79 (closest) *and* v83 (canonical) seed
addresses, the binary-name → IDB resolution rule (§2), the anchor playbook
(`signature-catalog.md` + the task-008 v79 catalog it consumes), and the anchor rules
(§4).

**The subagent is read-only on every IDB. It does NOT `rename`, `set_type`,
`idb_save`, or apply any struct type.** Its job is to *find and justify*, not mutate
(the "main session labels" decision, D3).

Per key it returns a structured row:

```
key:            C_CLIENT_SOCKET_SEND_PACKET
v72_addr:       0x00XXXXXX
v79_addr:       0x00XXXXXX   # task-008 catalog value used as the relocation seed
proposed_name:  CClientSocket::SendPacket
proto:          int __thiscall CClientSocket::SendPacket(CClientSocket*, COutPacket*)   # or null
anchor_1:       { kind: string_xref, detail: "<exact literal>",            ida_evidence: "<addr/line>" }
anchor_2:       { kind: call_graph,  detail: "called by CClientSocket::OnConnect", ida_evidence: "<addr/line>" }
chain_trace:    <"v79 0xA → v83 0xB confirmed same fn (both anchors hold)" | n/a for non-high-value>  # D6
tiebreaker:     <null | "byte-sig at 0xA matched, 0xB did not — chose 0xA"> # NOT counted as an anchor
v72_vs_v79:     <"string identical to v79" | "v79 literal '%s' drifted to '...' in v72" | "fn inlined/split">
confidence:     high | needs-main-review
ambiguity:      <null | "two candidates 0xA/0xB; chose 0xA because …">
```

`confidence: needs-main-review` is mandatory when (a) the key is high-value or
boundary-adjacent (§4 tier list), (b) anchors disagree across reference versions,
(c) more than one v72 candidate matched, or (d) **the v79/v83 anchor literal/constant
drifted in v72** (a below-floor-specific trigger — the older build is likelier to have
shifted a string or magic number; record the v72 form for the catalog). The main
session **re-probes every `needs-main-review` row independently** before accepting it.

### 3.3 Subsystem clusters (disjoint key sets)

Following `memory-map.md`'s resolution order so call-graph reach accumulates:

1. **WinMain + CWvsApp** (incl. all `C_WVS_APP_*` initializers) — entry point;
   anchors the image and yields call-graph reach. **Resolve first** — also gates the
   early Cat-A amendment (D7) and surfaces the `C_WVS_APP_SET_UP` / DR_init
   init-sequence question (R12, [[project_v84_movement_anticheat_freeze]]).
2. **CClientSocket / ZSocket** — send/flush/process/connect + buffer/close.
3. **COutPacket** — encode primitives + make-buffer-list.
4. **Login / Stage / Logo / CUITitle** — login & stage flow.
5. **Manager singletons** — `*_CREATE_INSTANCE` / `*_INSTANCE_ADDR` pairs (ActionMan,
   MapleTV, MonsterBook, Quest, Radio, Security, FuncKeyMapped, InputSystem,
   MacroSysMan, AnimationDisplayer, …). Includes the `C_RADIO_MANAGER_INSTANCE_ADDR`
   quirk prior ports flagged — verify the real v72 instance global, don't inherit the
   anchor's allocator-selector address.
6. **Config / SystemInfo / IGCipher / utilities** — CConfig, CSystemInfo,
   ZArray/ZXString helpers, ZFatalSection ctor/dtor, GET_SE_PRIVILEGE.
7. **Party / migrate senders + offsets** — `C_FIELD_SEND_*`,
   `C_WVS_CONTEXT_SEND_MIGRATE_*` and their `*_OFFSET` keys.
8. **Protocol constants + sentinels + exception-dispatch** — `VERSION_HEADER`,
   `PLAYER_LOGGED_IN`, `CLIENT_START_ERROR` (confirm against v72, older protocol than
   v79 — R8); the `C_TI_*EXCEPTION` / `C_PATCH_EXCEPTION_BUILDER` / `C_COM_RAISE_ERROR_EX`
   / `C_FILE_STREAM_*` keys; the GMS-absent and JMS-only sentinels; and any **new
   v72-only sentinel** discovered when a v79 real-address feature is absent in v72
   (R5, backward direction). Resolved last (FR-7).

Clusters are address-disjoint, so even if labeling were later parallelized no two
writers touch the same address.

---

## 4. Evidence rule: anchors per address (D1, D6)

Every absolute-address key requires **two independent, version-stable anchors** that
both resolve to the same v72 address before it is written to cmake. "Two independent"
means two *different kinds* of evidence, or two clearly distinct instances of the same
kind — not one fact stated twice.

**Anchor kinds, in preference order** (from `signature-catalog.md`):
1. String xref — a unique literal the function references.
2. Import/API call anchor — e.g. the function calling `connect`/`socket`.
3. Call-graph anchor — child/parent of an already-resolved function.
4. Constant / opcode immediate — a magic number or `push <opcode>`.
5. Vtable slot — via the class RTTI/type string, then slot index.
6. Byte/structure signature (`make_signature`) — **tiebreaker only for v72; never an
   anchor** (see below).

Rules:
- **Baseline (all keys):** at least **one** anchor must be from kinds 1–5. A pair of
  two byte-sigs never satisfies the rule.
- **High-value / high-blast tier (D6) — socket send/flush/process, the COutPacket
  encoders, WinMain, CWvsApp::Run/SetUp, the packet senders, and any function adjacent
  to a below-floor gate (§5):** require **two anchors of kinds 1–5** (two *structural*
  anchors, of two different kinds) **plus a confirmed v79→v83 chain trace** — the same
  function must be identified in v79 *and* v83 with both anchors holding, so the v72
  relocation is corroborated at both neighbor versions rather than pattern-matched into
  v72 in isolation. The ~12-version v72↔v83 gap makes a single-version byte match
  unsafe; the chain trace is the mitigation. A byte-signature may be used only to
  *disambiguate* between two structurally-anchored candidates; it does not count toward
  the pair. These keys are flagged `needs-main-review` (§3.2).
- **v72-vs-v79 drift capture:** because v72 is older, a v79/v83 anchor string/constant
  may have a different form in v72. When the working anchor differs from the v79
  reference, record the **v72-specific form** in `signature-catalog.md`, plus an
  explicit "ported directly / drifted" note (FR-14) so the next (older) backward port
  benefits.
- **Offset keys** (`*_OFFSET`, `WIN_MAIN_AD_BALLOON_CONDITIONAL`,
  `WIN_MAIN_PATCHER_OFFSET`, `C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET`) are not addresses;
  they are re-measured directly from the v72 host function's disassembly (FR-5, R7).
  The "anchor" for an offset is the identified target instruction/branch plus the byte
  delta to it, shown in the catalog. **Never copy the v79 or v83 offset.**
- **Protocol constants** (`VERSION_HEADER`, `PLAYER_LOGGED_IN`, `CLIENT_START_ERROR`)
  are confirmed against the v72 opcode table / handler dispatch, not copied from
  v79/v83 (FR-6, R8). v72 predates v79 (which confirmed `VERSION_HEADER`=8), so drift
  is *more* plausible here, not less — if a value differs, the v72 value is used and
  the delta noted (PRD §9). Cross-check against atlas-ms's version-aware registries
  where applicable ([[reference_atlas_ms]]).
- **Sentinels (FR-7) — backward direction.** Absence is *confirmed*, not assumed. For
  each v79 sentinel, search v72 for the feature's identifying anchor (strings,
  CreateInstance call site, vtable); only after the anchor is shown absent does the
  `0x00000000` + comment carry forward. **Additionally** (the backward-direction case,
  R5): watch for v79 *real-address* keys whose backing feature does not exist in v72 —
  those become **new v72-only sentinels**, flagged for the gate/edit owner rather than
  silently zeroed. Start each disposition from task-008's **v79** verdict (e.g.
  `RESET_LSP`, the CFileStream relay) — not the stale v83 comments — then confirm in
  v72. JMS-only sentinels are confirmed present in JMS185 and shown absent in v72.

Every accepted address lands one row in `signature-catalog.md` recording its anchors
and their cross-version stability (v83→v79→v72) — the durable artifact for the next
version port (FR-14).

---

## 5. Two-tier below-floor struct verification & gate audit (Phase 5)

This is the dimension that makes v72 categorically harder than v79. task-008 made
**v79** the lowest supported version and rewrote the `common/` gates so v79 selects
defined branches. v72 sits **below v79**, so it re-triggers every below-floor failure
mode one notch lower — but against a gate landscape task-008 already reshaped. The
central new question: for the five structs where task-008 created a two-way
`>= 83 || JMS` split, **the branch v72 lands on is the v79-reduced branch, which
already diverged from the v83 base** — and v72 may diverge from v79 in turn.

The pass is **read-only over raw disassembly** (§5.8). It runs *after* function
labeling (Phases 2–3) so method-body disasm shows resolved call targets, but writes no
struct types to the v72 IDB.

### 5.1 The four failure modes (priority order)

| # | Failure mode | Gate forms | Where v72 lands | Action |
|---|---|---|---|---|
| A | **No branch at all** (guard silently lost) | `== 79 \|\| == 83 \|\| == 84`, `== 79`, `== 83` member gate — no `#else` | nothing — guard doesn't fire / member dropped | Amend first (§5.3) |
| B | **v79-reduced branch, v72 maybe further-reduced** (the novel risk) | `>= 83 \|\| JMS` two-way splits | v79-reduced side | Confirm `v72 == v79` or split three-way (§5.4–5.5) |
| C | **Base/excluded branch** (quietest) | `>= 84/87/95/111`, `== 95/87/83`, `> 83`, `< 95/84` | base, same side as v79 | **Exhaustive size probe** — D5 (§5.6) |
| D | **CWnd cascade** | base shift in CWnd | propagates to 5 derived classes | one linked verdict (§5.7) |

### 5.2 Subagent contract (struct verifier)

The 24 headers in `struct-verification.md` are split across sequential read-only
subagents (one IDA lane, D3-note). The existing `version-port-verifier` subagent is
the vehicle where a header is non-trivial; trivial size-only confirmations may be done
inline in the main session.

Per header the subagent returns: **v72 size** (always — D5), present/absent verdict
for each version-gated field, the disassembly anchor for each verdict (a
`Decode`/`DecodeBuffer(this,N)` literal, an `imul stride,N` in `RemoveAll`, a ctor
member-init or destructor unwind extent, an Alloc immediate at the CreateMob/
CreateInstance site), an explicit **v72-vs-v79 size comparison** (mandatory for the
five Category-B reduced-branch headers), and a gate verdict (`unchanged` /
`branch-added` / `two-way-confirmed-shares-v79` / `split-three-way` / `rewritten`)
with the deciding v72 disasm line(s). Embedded UDT sizes (e.g. `SecondaryStat`'s
embedded `0xB88`) are verified independently — not assumed equal to v79/v83.

**Main-session cross-check:** before any gate is rewritten, the main session re-anchors
at least one boundary case per changed header with an independent probe. Subagent
headline verdicts are treated as drafts; the per-offset evidence is trusted, the
verdict is verified.

### 5.3 Category A — amend FIRST, early, from disasm (D7, R1)

These select by explicit version equality with no catch-all `#else`, so v72 selects
**nothing** (size guard silently does not fire) or drops a member. Re-grep at task
start (`grep -rn "BUILD_MAJOR_VERSION ==" common/*.h`) — line numbers shift and other
enumerated-no-`#else` gates may exist. Known sites:
- `common/CWvsApp.h:97` — `(== 79 || == 83 || == 84)` is false for v72 → no branch,
  `assert_size(…,0x60)` doesn't fire. Verify v72 `CWvsApp` layout; add `72` to the
  matching branch (almost certainly the `0x60` branch v79 joined).
- `common/CFuncKeyMappedMan.h:50` — `== 79` size-assert branch false for v72 → no
  branch. Member gate `:18` already excludes v72; confirm the v72 size and add `72` to
  the matching size-assert branch (`== 72` or `== 72 || == 79`).
- `common/CLogin.h:235` — `== 83` member-declaring gate false for v72 (excluded, like
  v79). Confirm v72 **genuinely lacks** `unk3[5]` rather than silently dropping a
  member it has; if v72 has it, the gate becomes `== 83 || == 72` (or `<= 83`).

Per D7, as soon as Cluster 1 (WinMain + CWvsApp) and the CFuncKeyMappedMan key are
located and sized in discovery, **decide which branch v72 joins from that
disassembly** (not "oldest ⇒ v83 branch") and amend these gates so the v72 build
compiles with its guards restored. The remaining headers are audited afterward against
a building target. Which branch v72 joins is itself an evidence verdict recorded in
`struct-verification.md`.

### 5.4 Category B — two-way `>= 83 || JMS` gates: confirm or split (the novel tier)

task-008 created these two-way splits to exclude v79; they now exclude v72 too, so v72
inherits v79's *reduced* branch. For each, confirm **v72 == v79** (gate already
correct — record the confirming size and move on) or **v72 diverges further** (the
gate must become a three-way split per §5.5). Known sites and their v79 reduced sizes:

| Site | Field gated out for v79 | v79 reduced size | v72 must confirm |
|---|---|---|---|
| `common/CWnd.h:25` | `m_pAnimationLayer` + `m_pOverlabLayer` | CWnd 0x64 (−8 vs v83 0x6C) | v72 `sizeof(CWnd)` via Destroy/ctor landmarks (cascade root — §5.7) |
| `common/CMob.h:239` | doom tail (`m_bDoomReserved`/SN/`m_lpStatChangeReserved`) | CMob 0x518 (vs v83 0x548) | v72 CMob size via `?CreateMob@@` Alloc immediate + ctor highest write |
| `common/MobStat.h:128` | Weakness group (`nWeakness_/rWeakness_/tWeakness_`) | MobStat 0x1F8 (vs v83 0x208) | v72 MobStat tail via CMob ctor `lea` to last member |
| `common/CFuncKeyMappedMan.h:18` | quickslot pair (`m_aQuickslotKeyMapped[8]`×2) | 0x388 (vs v83 0x3C8) | v72 CreateInstance Alloc immediate + ctor extent |
| `common/CUIToolTip.h:92` | `m_pLayerAdditional` | 0x514 (== v83) | v72 ctor `m_pLayer` → `m_aLineInfo` adjacency |

Because there is no IDB below v72, "v72 == v79" is **only** acceptable with a v72
signature confirming the size — never by proximity ([[feedback-prefer-confirmation]]).

### 5.5 Three-way split when v72 diverges (D8)

When a Category-B size probe shows v72 ≠ v79, the two-way gate splits three ways. Per
**D8**, v72's branch uses a **GMS-guarded `BUILD_MAJOR_VERSION < 79` range term**, not
`== 72`. Canonical shape (CWnd example; the first arm already catches v83+ and JMS185
because 185 ≥ 83):

```c
#if (BUILD_MAJOR_VERSION >= 83 || defined(REGION_JMS))
    // full v83 layout (m_pAnimationLayer + m_pOverlabLayer present)
#elif (defined(REGION_GMS) && BUILD_MAJOR_VERSION < 79)
    // NEW: v72 further-reduced layout
#else
    // v79-reduced layout (GMS 79..82) — UNCHANGED
#endif
```

Why `< 79` over `== 72`:
- **Future-proofs the next older port.** A v6x port inherits the v72 branch with no
  edit (unless v6x diverges again, in which case it splits *that* branch — the same
  pattern one notch lower).
- **Stays disjoint.** The supported floor is 72; no supported version satisfies `< 79`
  except v72, so the term cannot flip another version's truth value (FR-13).
- **GMS-guarded** so JMS/other regions never fall into it (and JMS185 is already caught
  by the first arm).

Every split records its full-matrix truth table (§5.7 template) and is preprocessed
(`gcc -E`) at `{72,79,83}` to prove each field's present/absent at each version before
commit. The `#else` (v79 branch) must remain **byte-identical in effect** — the split
adds an arm above it, never edits it.

### 5.6 Category C — base/excluded branch: exhaustive size probe (D5, R3 — quietest)

Every `>= 84/87/95/111`, `== 95/87/83`, `> 83`, `< 95/84` gate resolves v72 onto the
base/excluded branch (same side as v79). Individually low-risk, but a silent v72≠v79
(or v72≠v83) base-size delta corrupts every downstream offset and **cannot be caught
by triangulation** — there is no anchor below v72. Per **D5**, verification is
**exhaustive: probe the v72 struct size of every gated header (all 24), including the
~14–19 expected to match the base.** A size match is recorded as evidence; a mismatch
escalates that header to field-level verification.

Special attention (named in `struct-verification.md`):
- `common/CWvsContext.h` (`> 83`, `m_aClientKey[8]`) — false for v72 → key treated
  **absent** (v79 confirmed absent). Confirm v72's `CClientSocket::OnConnect`
  connect-hello does **not** encode an 8-byte client key. Drives `bypass/socket_hooks.cpp`
  (`> 83`). [[project_v84_clientkey_gate_trap]] — verify decode order against the v72
  binary, not a server round-trip.
- `common/SecondaryStat.h` embedded size (v79/v83 base `0xB88`) — confirm v72 computes
  the same embedded size; a wrong embedded size shifts every field after `m_forcedStat`
  in `CWvsContext`. Cross-validate bitmask gating against atlas-ms
  ([[reference_atlas_ms]]).
- `doom-fix/dllmain.cpp` (`== 83` write gate) — false for v72 → write not applied.
  Confirm `m_bDoomReserved` is genuinely absent in v72 (so exclusion is correct and the
  write would have been OOB), as task-008 found for v79.

### 5.7 Category D — CWnd cascade (R4)

The CWnd −8 base shift task-008 found propagates by inheritance to **CDialog, CUIWnd,
CFadeWnd, CUITitle, CUILoginStart**. If v72's CWnd differs from v79's 0x64, all five
derived headers shift again. Treat the cascade as **one linked verdict, not five
independent ones**: pin v72 `sizeof(CWnd)` first via three independent landmarks (per
task-008's method), then re-derive each derived class from that base. If v72 CWnd ==
v79, the derived classes inherit v79's verdict (confirm each size anyway, D5); if v72
CWnd diverges, every derived class is re-measured and the split (§5.5) cascades.

### 5.8 Read-only discipline & decompiler-leak avoidance (R10)

Applying inferred struct types into the v72 IDB makes Hex-Rays echo the inferred field
names back, masking the very deltas being verified. So: use `disasm`, not `decompile`,
for layout evidence; **do not apply speculative struct types** during Phase 5.
Function/global *labels* from Phases 2–3 are fine and present (they label symbols, not
layout). No conflict: labeling writes symbols, the audit reads layout.

### 5.9 Gate-rewrite expression strategy & cross-version safety (FR-13, R11)

- **Confirm-in-place when possible.** If v72 behaves like the branch the current gate
  already selects, leave it unchanged and record the evidence. "No change, here's why"
  is a first-class outcome and is expected to be the common case for Category B.
- **Minimal correct boundary when v72 diverges.** For Category A, extend the enumerated
  list with `72` (or `== 72 || == 79`) per the discovered branch. For Category B
  splits, add the `< 79` arm (D8) above the unchanged `#else`. Keep the house
  `#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION …)` guard form. Gate the minimum
  contiguous region; no ternary in array dimensions (use `#if/#else/#endif`).
- **Validate every rewrite across the full matrix.** A rewrite must keep
  **v79/v83/v84/v87/v95/v111/JMS185** selecting their current branch — three-way splits
  in particular must keep v79's selected branch unchanged. For the loaded IDBs
  (v79/v83/v87/v95/JMS185) confirm the comparator's truth value and re-anchor any
  layout claim the rewrite touches. For **v111** (not loaded) evaluate from build
  constants + existing `v111_1.cmake`; surface any unsettleable v111 case rather than
  assuming it (A2). Keep v72's exclusion **disjoint** (`< 79`, which no other supported
  version satisfies) so it cannot flip another version. CI builds all matrix versions
  on PR as the final backstop, but source-level reasoning precedes it — a green build
  proves the branch *compiled*, not that it is semantically right.

#### Three-way split truth-table template (fill per split)

| Version | Branch selected | size | Changed? |
|---|---|---|---|
| GMS 72 | `< 79` (GMS-guarded) | (v72 size) | NEW |
| GMS 79 | `#else` | (v79 size) | unchanged |
| GMS 83/84/87 | `>= 83` | (v83 size) | unchanged |
| GMS 95/111 | `>= 83` (+ any `>= 95` tail) | … | unchanged |
| JMS 185 | `>= 83 \|\| REGION_JMS` (first arm) | … | unchanged |

**Empirical verification (fill):** `scripts/wsl-build.sh GMS 72 1` → `>> OK`;
`scripts/wsl-build.sh GMS 79 1` (neighbor sanity) → `>> OK`;
`cmake -DREGION=GMS -DMAJOR=72 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake` → `OK`;
`gcc -E` at `{72,79,83}` confirming each split field's present/absent at each version.

---

## 6. Build-matrix wiring (Phase 6)

- Add `{ region: GMS, major: 72, minor: 1 }` to `strategy.matrix.config` in
  `.github/workflows/_build.yml`, **placed first** to keep the list version-ascending
  (single source of truth; PR/snapshot/release inherit it — FR-1/FR-2). No other
  workflow file is edited.
- `memory_maps/GMS/v72_1.cmake` is **seeded from `v79_1.cmake`** (closest; carries the
  below-floor relocations and sentinel dispositions) and must define **every** key
  parsed from `include/memory_map.h.in` (re-pinned live in Phase 0 — expect 159; stop
  and reconcile if not), non-empty; the CMake key-completeness check fails the build
  otherwise (FR-3). **Every seeded value is UNVERIFIED until a v72 signature confirms
  it** — a value still equal to v79 means UNVERIFIED, not "done."
- Local pre-flight: `scripts/wsl-build.sh GMS 72 1` compiles+links the v72 build on the
  WSL/clang-cl path before the Windows/CI handoff ([[project_wsl_cross_compile]]). Build
  **Debug and Release** for GMS 72.1 green (FR-15), and rebuild the other matrix
  versions affected by any gate rewrite to prove no regression (R11) — especially v79,
  whose branch every three-way split must preserve.

---

## 7. Verification & acceptance strategy

Two distinct evidence classes, kept separate (R13):

**Build-correctness (automated gate).**
- GMS 72.1 configures + builds clean, Debug and Release, locally and in CI.
- All other supported versions remain green (gate-rewrite regression guard; v79 branch
  preserved through every three-way split).
- The CMake key-completeness check passes (every required key present, non-empty).
- Every absolute key has a catalog row meeting §4 (two structural anchors + v79→v83
  chain trace for the high-value tier); every offset is re-derived from v72 codegen;
  every sentinel is justified in the backward direction (incl. any new v72-only
  sentinels); all 24 headers have a recorded v72 size + gate verdict (and, for the five
  Category-B headers, an explicit v72-vs-v79 comparison) in `struct-verification.md`.
- The v72 IDB is labeled for all resolved functions/globals and `idb_save`d.

**Runtime-correctness (user-run, gates completion — D4).**
- The user launches a live GMS v72 client with the proxy `ijl15.dll` and the core edits
  deployed (per `acceptance.md`), reaches title/login, exercises the targeted edits, and
  confirms no crash / no Themida fault (FR-16).
- The exact result is recorded in `acceptance.md` / the PR. The task is **not done**
  until this is run and the outcome pasted. A correct build with a failed/blocked smoke
  test is reported as such, distinguishing build issues from environment issues
  (Themida / VC++ redist / OS) per README compatibility notes
  ([[verification-before-completion]]).

---

## 8. Risk control mapping

| Risk | Control in this design |
|---|---|
| R1 enumerated v79 gate has no v72 branch (guard lost) | §5.3 amends Category A first/early from disasm (D7); branch chosen by evidence |
| R2 `>= 83` floor now wrongly excludes v72 too | §5.4 confirms each two-way gate against v72; lower/split where v72 has the field |
| R3 v72 diverges from v79's reduced base → three-way split (NEW) | §5.4–5.5 confirm-or-split with `< 79` convention (D8); §5.6 exhaustive size probe (D5); no "same as v79" without a v72 probe |
| R4 CWnd cascade | §5.7 pin v72 `sizeof(CWnd)` first, treat the 5 derived classes as one linked verdict |
| R5 backward-direction sentinel (v79-present, v72-absent) | §4 sentinel rule (backward); new v72-only sentinels flagged, not zeroed |
| R6 wrong-address relocation (12-version v72↔v83 gap) | §4/D6 two structural anchors **+ v79→v83 chain trace** for high-value tier; §3.2 needs-main-review re-probe |
| R7 stale offsets | §4 offsets re-measured from v72 disasm, never copied |
| R8 protocol-constant drift (older build) | §4 constants confirmed against v72 opcode/handler table; atlas-ms cross-check |
| R9 wrong/unverified-IDB probe | §2 lane discipline — binary-name→port resolution, `get_metadata` after every `select_instance`, doubly so for the sparse v72 IDB; D3-note (no concurrent selection) |
| R10 decompiler leak | §5.8 read-only audit, `disasm` not `decompile`, no struct-type application |
| R11 three-way split regresses v79 or another version | §5.5/§5.9 `< 79` disjoint term, `#else` (v79) untouched, full-matrix truth table + `gcc -E` + CI |
| R12 DR_init / SetUp init-sequence regression | §3.3 Cluster 1 surfaces `C_WVS_APP_SET_UP` init sequence; confirm DR subsystem absence for v72 ([[project_v84_movement_anticheat_freeze]]), don't assume identical to v79 |
| R13 smoke-test environment | §7 build- vs runtime-correctness kept separate; exact results recorded in `acceptance.md` |

---

## 9. Out of scope (mirrors PRD §2 non-goals)

- No new client-edit features; relocation only.
- No struct-internals re-port beyond size + the version-gated field boundaries.
- No changes to non-GMS regions, or to other GMS versions except a region-correct gate
  rewrite that also touches another version's compiled branch (a three-way split that
  adds an arm above v79's unchanged branch).
- Not aiming for 100% v72 IDB symbol coverage — only the symbols backing the 159 keys
  and the 24 verified structs.

---

## 10. Assumptions & open items

- **A1 — topology must hold for the run.** The v72 target + v79/v83/v87/v95/JMS185
  references must be loaded. If a session restarts and an anchor is missing —
  **especially v79 (closest/template) or v83 (canonical names)** — discovery/audit must
  pause until it is reloaded rather than silently falling back to a further anchor.
  Ports are resolved per-session by binary name (§2), never hardcoded. The v72 IDB is
  sparse/unverified — its identity is confirmed every probe, not assumed.
- **A2 — v111 is source-validated only.** Its IDB is not loaded; gate rewrites are
  validated for v111 from build constants + existing `v111_1.cmake`. Any unsettleable
  v111 case is surfaced, not assumed ([[feedback-prefer-confirmation]]).
- **A3 — protocol-constant equality v79↔v72 is unverified until Phase 1** (PRD §9). The
  design handles both same and different outcomes; v72 is an older protocol revision
  than v79, so drift is treated as *more* plausible (R8).
- **A4 — sentinel dispositions unknown until §4 runs.** Whether GMS-absent sentinels
  (CBattleRecordMan, DRCheck, DRInit, CETracer) are also absent in v72 (expected),
  whether `RESET_LSP` is present, whether the `C_FILE_STREAM_*` relay helpers are
  recoverable in v72, and whether any v79 real-address feature is absent in v72 (a new
  v72-only sentinel, R5) are all resolved during discovery, not pre-judged — starting
  from task-008's v79 verdicts, not v83's stale comments.
- **A5 — Category-A branch assignment is an evidence verdict.** Whether v72's `CWvsApp`
  and `CFuncKeyMappedMan` layouts join the v79 below-floor branch or need their own is
  decided by v72 disassembly in Phase 1a (D7/§5.3), not by "oldest ⇒ v83."
- **A6 — Category-B confirm-vs-split is empirical per struct.** "v72 == v79" is the
  expected common case but is accepted only with a confirming v72 size signature; the
  three-way split (D8) is the contingency for genuine divergence, and the CWnd split (if
  any) cascades through five derived classes (§5.7).
- **A7 — serial IDA lane.** Discovery is serialized because `select_instance` is global
  state (D3-note). If the user stands up a second MCP server, discovery can be
  parallelized per §3.2 — flagged for the user's review.
