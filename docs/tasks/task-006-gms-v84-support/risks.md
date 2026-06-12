# GMS v84 Support — Risks

## R1 — v84 is on a live gate boundary (highest)
Four+ source gates branch at 83/84 and currently assume v84 == v87+ behavior.
If any assumption is wrong, the v84 build compiles but patches wrong
addresses/layouts → crash or silent misbehavior. **Mitigation:** FR-12 verifies
every boundary gate against v84 disassembly before trusting it; rewrites are
validated across all versions (FR-13).

## R2 — Wrong-address relocation from over-trusting v83 proximity
A function may move, split, or inline differently in v84; picking the
address-adjacent v83 analog without a signature yields a plausible-but-wrong
patch. **Mitigation:** every absolute key requires a documented signature
(`signature-catalog.md`), not address proximity; label back into the IDB so the
choice is inspectable.

## R3 — Sentinel features that actually exist in v84
A v83 `0x00000000` "does not exist" key (e.g. CBattleRecordMan, DRCheck,
CETracer) might be present in v84, in which case carrying the sentinel forward
disables a real edit. **Mitigation:** FR-7 requires confirming absence, not
assuming it.

## R4 — Stale instruction-relative offsets
`*_OFFSET` keys are byte offsets into host functions; codegen drift between v83
and v84 silently invalidates them even when the host function is correctly
located. **Mitigation:** FR-5 mandates re-deriving every offset from v84
disassembly.

## R5 — Probing/labeling the wrong IDB
Acting on the wrong connected IDB corrupts both the address findings and the
labels written back. **Mitigation:** `get_metadata` before every probe
([[feedback-verify-ida-target]]); `idb_save` checkpoints.

## R6 — Decompiler leak during struct verification
Applying inferred struct types into the IDB makes Hex-Rays echo the inferred
names back, masking the very deltas being verified. **Mitigation:** struct
verification stays read-only over raw disassembly (the workflow's anti-pattern
list); function/global *labeling* for the memory map is fine and separate.

## R7 — Gate rewrite regresses another version
Changing a `> 83` gate to accommodate v84 could flip v87/v95/v111/JMS behavior.
**Mitigation:** FR-13 validates every rewrite against the full matrix; CI builds
all versions on PR.

## R8 — Live client smoke test environment
The "done" bar includes launching a real v84 client with proxy + edits; Themida
/ VC++ redist / OS specifics can mask a correct build as broken (or vice versa).
**Mitigation:** treat the smoke test as acceptance evidence, record exact
results; distinguish build-correctness from environment issues per README
compatibility notes.
