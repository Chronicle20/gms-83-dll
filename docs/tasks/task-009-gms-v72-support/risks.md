# GMS v72 Support — Risks

## R1 — v72 is below the *new* floor; enumerated v79 branches have no v72 entry (highest, build-guard)
task-008 added v79-specific branches (`CWvsApp.h:97` `==79||==83||==84`,
`CFuncKeyMappedMan.h:50` `==79`) that are **false for v72** → v72 selects no branch and
**silently fires no `assert_size`** (builds, but loses its size guard). **Mitigation:**
FR-12a amends every such gate *early*, deciding v72's branch by disassembly and locking
the measured v72 size, before relying on any build.

## R2 — `>= 83` floor gates now wrongly exclude v72 as well as v79
The two-way `>= 83 || JMS` gates task-008 created put v72 on the excluded (v79-reduced)
side automatically. If v72 actually *has* a field v79 lacks (feature predates v79), the
gate silently drops it. **Mitigation:** FR-12b confirms each two-way gate against v72;
lower/split where v72 has the field, confirm exclusion where it doesn't.

## R3 — v72 diverges from v79's reduced base → two-way gate must become three-way (NEW, quietest)
This is the categorically new risk versus the v79 port. For CWnd/CMob/MobStat/
CFuncKeyMappedMan/CUIToolTip, v72 rides the *v79-reduced* branch — but v79 itself
already differs from v83 (CWnd −8, CMob 0x518, MobStat 0x1F8). If v72 is older still and
shrinks further, every downstream offset on that branch is wrong, and there is **no
labeled IDB below v72** to catch it. **Mitigation:** spot-check struct *size* of every
v79-reduced-branch header against v72 (FR-12b/c); where v72≠v79, split the gate
three-way (distinct v72/`< 79` branch) and validate the truth table across the matrix.
Never assume "same as v79" without a confirming probe ([[feedback-prefer-confirmation]]).

## R4 — CWnd cascade: a v72≠v79 base shift propagates to every UI class
The CWnd −8 shift task-008 found propagates by inheritance to CDialog, CUIWnd, CFadeWnd,
CUITitle, CUILoginStart. If v72's CWnd differs from v79's 0x64, all five derived headers
shift again. **Mitigation:** pin v72 `sizeof(CWnd)` first (3 independent landmarks, per
task-008's method), then re-derive each derived class; treat the cascade as one linked
verdict, not five independent ones.

## R5 — Backward-direction sentinels (v79-present, v72-absent)
Going from v79 *down* to v72, a feature v79 has may not exist yet in v72, turning a v79
real-address key into a v72 sentinel; the consuming gate/edit must tolerate `0`.
**Mitigation:** FR-7 evaluates each key's v72 disposition starting from v79's (not v83's
stale comments); new v72-only sentinels are flagged for the gate/edit owner, not silently
zeroed.

## R6 — Wrong-address relocation from over-trusting v79 proximity
A function may move, split, or inline differently in v72; picking the v79 analog without
a signature yields a plausible-but-wrong patch. The wider gap from v83 (12 versions) makes
raw byte signatures less reliable. **Mitigation:** every absolute key requires a documented
signature (`signature-catalog.md`) — prefer string/import/call-graph anchors over byte
sigs; relocate from the closest (v79) labels but confirm; label back into the v72 IDB so
the choice is inspectable.

## R7 — Stale instruction-relative offsets
`*_OFFSET` keys are byte offsets into host functions; codegen drift silently invalidates
them even when the host function is correctly located. **Mitigation:** FR-5 mandates
re-deriving every offset from v72 disassembly — never copy v79's.

## R8 — Protocol-constant drift in the older build
v72 predates v79 (which confirmed `VERSION_HEADER`=8); `PLAYER_LOGGED_IN`,
`CLIENT_START_ERROR`, and the opcode/handler table may differ more here than they did at
the 79 boundary. **Mitigation:** FR-6 confirms each constant against v72 disassembly;
cross-check atlas-ms version-aware registries ([[reference_atlas_ms]]).

## R9 — Probing/labeling the wrong (or unverified) IDB
The v72 IDB is **sparse/unverified** — acting on the wrong connected IDB, or assuming an
unverified IDB is the retail v72 binary, corrupts both findings and labels.
**Mitigation:** `get_metadata` before every probe ([[feedback-verify-ida-target]]);
confirm the IDB identity early; `idb_save` checkpoints; never run two IDA-probing tasks
concurrently (`select_instance` is global state).

## R10 — Decompiler leak during struct verification
Applying inferred struct types into the IDB makes Hex-Rays echo the inferred names back,
masking the deltas being verified. **Mitigation:** struct verification stays read-only over
raw disassembly; function/global *labeling* for the memory map is fine and separate.

## R11 — Three-way gate split regresses v79 or another version
Splitting a two-way `>= 83 || JMS` gate into a three-way form (v72 / v79 / `>= 83`) is more
error-prone than task-008's two-way splits and could flip v79's selected branch.
**Mitigation:** FR-13 validates every rewrite's truth table across the full matrix
(v79/v83/v84/v87/v95/v111/JMS); CI builds all versions on PR; keep v72's exclusion
*disjoint* so it cannot flip another version.

## R12 — DR_init / SetUp init-sequence regression
[[project_v84_movement_anticheat_freeze]] showed a reimplemented `CWvsApp::SetUp` that
dropped a required init step caused an in-field freeze. v72's SetUp init sequence must be
verified, not assumed identical to v79/v83. **Mitigation:** confirm `C_WVS_APP_SET_UP` and
the DR/anti-debug sentinels against v72 disassembly; v72 likely lacks the DR subsystem
(like v79/v83), but confirm.

## R13 — Live client smoke test environment
The "done" bar includes launching a real v72 client with proxy + edits; Themida / VC++
redist / OS specifics can mask a correct build as broken (or vice versa). **Mitigation:**
treat the smoke test as acceptance evidence, record exact results; distinguish
build-correctness (CI) from environment issues per README compatibility notes.
</content>
