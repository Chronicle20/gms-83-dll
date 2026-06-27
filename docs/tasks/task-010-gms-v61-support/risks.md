# GMS v61 Support — Risk Register

Companion to `prd.md`. Risks are rated **L**ikelihood × **I**mpact. The defining
theme: v61 is the **deepest below-floor port yet** (below v72, which is below v79,
which is below the original v83 floor), with **no labeled IDB beneath it**.

| ID | Risk | L | I | Mitigation |
|---|---|---|---|---|
| R1 | **Memory-map key resolved to wrong v61 site** (signature drifted in older codegen). | M | H | Re-derive every key from v61, confirm canonical name against v83, document the heuristic. Don't offset-arithmetic from v72. (FR-4, FR-14) |
| R2 | **Instruction-relative offset copied instead of re-derived** → patch lands mid-instruction → crash. | M | H | All `*_OFFSET` / `WIN_MAIN_*` keys re-derived from v61 disassembly. (FR-5) |
| R3 | **Below-floor gate divergence: v61 ≠ v72.** A struct task-009 reduced for v72 shrinks further at v61 → need a distinct v61/`< 72` branch (four-way split). Silent layout shift / heap overrun if missed. | M | H | Per-struct size confirmation against v61 (FR-12b); assert-net catches the overrun; truth-table validate four-way splits across all versions (FR-13). **Highest-novelty risk.** |
| R4 | **Enumerated gate has no v61 branch** → size guard doesn't fire / member silently dropped. | H | H | Re-pin live enumerated-gate set vs task-009 baseline; amend CWvsApp/CFuncKeyMappedMan/CLogin etc. so v61 selects a measured branch. (FR-12a) |
| R5 | **CWnd base shift differs at v61** → cascades through every CWnd-derived UI class (CDialog/CUIWnd/CFadeWnd/CUITitle/CUILoginStart). | M | H | Confirm v61 CWnd size *first*; treat as the cascade root before auditing derived classes. (FR-12b) |
| R6 | **Protocol constant drift** (`VERSION_HEADER`/opcodes differ in older build) → redirect/bypass craft wrong packets. | M | M | Confirm all three constants in v61; cross-check atlas-ms version-aware registries ([[reference_atlas_ms]]). (FR-6) |
| R7 | **Era-absent feature treated as present** → memory map points at a non-existent function, or an edit hooks a no-op. (ad balloon, patcher, MTS restriction, beginner-party, DR_* anti-debug.) | M | M | Investigate each candidate; sentinel `0x00000000` with reason; ensure consuming edit/gate tolerates `0`. (FR-7) — *chosen policy: investigate, don't assume.* |
| R8 | **IDB ≠ smoke-test binary** → addresses correct for IDB but not the running client. | M | H | `get_metadata` to confirm IDB identity; record IDB build string; obtain the matching client for the smoke test. (FR-9, FR-16) |
| R9 | **Probed the wrong IDB** (v48 distractor also loaded below the floor). | M | M | `get_metadata` before every version-specific probe ([[feedback_verify_ida_target]]); never infer the active target from context. |
| R10 | **Themida/packing differs at v61** (possibly pre-Themida) → patch-site validity / integrity-check assumptions wrong. | M | M | Determine packing status early; record it; adjust patch-validity + smoke-test expectations accordingly. (Open Question) |
| R11 | **`C_WVS_APP_SET_UP` init-sequence reimpl drops a load-bearing init** → in-field freeze class. | L | H | Follow `docs/playbooks/version-port-and-freeze-debug.md` ([[reference_version_port_playbook]]); diff the v61 SetUp against our reimpl. DR_init is era-absent here, so the specific v84 freeze is unlikely, but the *class* recurs. |
| R12 | **Regression to an existing version** from a shared-header gate rewrite. | M | H | Validate every gate change against the full matrix (v72/v79/v83/v84/v87/v95/v111/JMS185); four-way splits must leave v72/v79 branches unchanged. (FR-13) |
| R13 | **Branch drift from task-009 baseline.** This task branches on v72 (task-009); if task-009 changes before it lands, v61's inherited gates shift. | M | M | Track task-009; rebase on its final state before merge; re-confirm inherited gate branches if task-009's below-floor gates change. |

## Out-of-scope reminders (so they aren't silently treated as risks here)

- Porting/labeling the v48 IDB — explicitly out of scope (PRD non-goals).
- New edit features or struct-internal re-ports beyond size/boundary confirmation.
- Non-GMS regions.
