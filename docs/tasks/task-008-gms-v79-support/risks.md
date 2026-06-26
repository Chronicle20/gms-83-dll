# GMS v79 Support — Risks

## R1 — v79 is below the floor; enumerated gates have no v79 branch (highest)
`common/CFuncKeyMappedMan.h:38` and `common/CWvsApp.h:97` (and any other
enumerated `==`-gate with no `#else`) leave the gated member **undefined** for
v79 → silent layout shift or build break. Unlike the v84 boundary risk (wrong
branch), this is "no branch at all." **Mitigation:** FR-12a amends every such gate
*first*, deciding v79's branch by disassembly, before relying on any build.

## R2 — `>= 83` / `>= 84` floor gates wrongly exclude v79
A lower-bound gate that used to include every supported version now puts v79 on
the excluded side. If v79 actually *has* the field (the feature predates v83), the
gate silently drops it and shifts the layout. **Mitigation:** FR-12b verifies each
floor gate against v79; lower the floor (`>= 79`) where v79 has the field, confirm
exclusion where it doesn't — with disassembly evidence.

## R3 — Silently-wrong base branch (no anchor below v79)
Most gates resolve v79 onto the "v83 layout" base branch. If v79's base layout
differs from v83 (size or field order), every downstream offset is wrong — and
there is **no labeled IDB below v79** to catch it by triangulation. **Mitigation:**
spot-check struct *size* of every base-branch header against v79 (Category E),
never assume "same as v83" without a confirming probe ([[feedback-prefer-confirmation]]).

## R4 — Backward-direction sentinels (v83-present, v79-absent)
Going from v83 *down* to v79, a feature v83 has may not exist yet in v79, turning
a v83 real-address key into a v79 sentinel; the consuming gate/edit must tolerate
`0`. This is the inverse of the v84 "surprise present" case (e.g. RESET_LSP).
**Mitigation:** FR-7 evaluates each key's v79 disposition in *both* directions;
new v79-only sentinels are flagged for the gate/edit owner, not silently zeroed.

## R5 — Wrong-address relocation from over-trusting v83 proximity
A function may move, split, or inline differently in v79; picking the
address-adjacent v83 analog without a signature yields a plausible-but-wrong
patch. v79's larger version gap from v83 makes raw byte signatures less reliable
than they were for v84. **Mitigation:** every absolute key requires a documented
signature (`signature-catalog.md`) — prefer string/import/call-graph anchors over
byte sigs; label back into the IDB so the choice is inspectable.

## R6 — Stale instruction-relative offsets
`*_OFFSET` keys are byte offsets into host functions; codegen drift between v83
and v79 silently invalidates them even when the host function is correctly
located. **Mitigation:** FR-5 mandates re-deriving every offset from v79
disassembly.

## R7 — Protocol-constant drift in the older build
v79 predates v83; `VERSION_HEADER`, `PLAYER_LOGGED_IN`, `CLIENT_START_ERROR`, and
the opcode/handler table may differ. Copying v83 constants could misalign
`redirect`/`bypass`. **Mitigation:** FR-6 confirms each constant against v79
disassembly. Cross-check against atlas-ms's version-aware registries where
applicable ([[reference_atlas_ms]]).

## R8 — Probing/labeling the wrong IDB
Acting on the wrong connected IDB corrupts both the address findings and the
labels written back. **Mitigation:** `get_metadata` before every probe
([[feedback-verify-ida-target]]); `idb_save` checkpoints; never run two
IDA-probing tasks concurrently (`select_instance` is global state).

## R9 — Decompiler leak during struct verification
Applying inferred struct types into the IDB makes Hex-Rays echo the inferred names
back, masking the very deltas being verified. **Mitigation:** struct verification
stays read-only over raw disassembly; function/global *labeling* for the memory
map is fine and separate.

## R10 — Gate amendment regresses another version
Adding `79` to an enumerated list or lowering a `>= 83` floor could flip
v83/v84/v87/v95/v111/JMS behavior. **Mitigation:** FR-13 validates every rewrite's
truth table across the full matrix; CI builds all versions on PR.

## R11 — DR_init / SetUp init-sequence regression
[[project_v84_movement_anticheat_freeze]] showed a reimplemented `CWvsApp::SetUp`
that dropped a required init step caused an in-field freeze on v84. v79's SetUp
init sequence must be verified, not assumed identical to v83. **Mitigation:**
confirm `C_WVS_APP_SET_UP` and the DR/anti-debug sentinels against v79
disassembly; v79 likely lacks the DR subsystem (like v83), but confirm.

## R12 — Live client smoke test environment
The "done" bar includes launching a real v79 client with proxy + edits; Themida /
VC++ redist / OS specifics can mask a correct build as broken (or vice versa).
**Mitigation:** treat the smoke test as acceptance evidence, record exact results;
distinguish build-correctness (CI) from environment issues per README compatibility
notes.
</content>
