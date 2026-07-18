# GMS v72.1 — Memory Map Porting Plan

This is the working plan for producing `memory_maps/GMS/v72_1.cmake`. It defines the
per-key resolution strategy, the IDB-labeling protocol, and the tracking approach for
every key parsed from `include/memory_map.h.in` (159 keys; re-pin at task start). The
**closest labeled anchor is GMS v79** (task-008-labeled, 7 versions above v72); the
**canonical-name anchor is GMS v83** (most complete symbol set). v87 and the v95 PDB are
secondary references for the upper-gate cross-checks only. There is **no labeled IDB below
v72** — confirm every value by signature, never by proximity.

## Resolution method (per key)

1. Look up the symbol in the **v79 reference first** (closest; task-008 labeled it and
   documented the heuristic in its `signature-catalog.md`). Confirm the canonical name and
   prototype against **v83**. Note the function's distinctive anchors: referenced strings,
   called imports, constants, vtable slot, call-graph neighbors.
2. In the **v72 IDB** (confirm with `get_metadata` first — it is sparse/unverified),
   locate the equivalent via a signature in priority order:
   - **String xref** — most robust across versions; a unique format/literal the function
     references. (Older builds may carry a *different* literal — note it.)
   - **Import/API call anchor** — e.g. the only function calling `socket` + `connect` in a
     given module region.
   - **Call-graph anchor** — child/parent of an already-resolved function.
   - **Constant / opcode** — a magic number, a `push <opcode>` immediate.
   - **Byte/structure signature** (`make_signature`) — last resort; least version-stable
     but precise when codegen matches. Reuse task-008's v79 signatures as starting probes.
3. Record the v72 address; **label it in the v72 IDB** (`rename` with the v83 canonical
   name, `set_type` if prototype known).
4. Add the heuristic to `signature-catalog.md`, noting whether task-008's v79 heuristic
   ported directly or drifted.
5. Write the `set(KEY 0x…)` line to `v72_1.cmake`.

For **offset** keys, disassemble the v72 host function and re-measure the offset to the
target instruction/branch — never copy the v79 or v83 offset.

## Seeding

Seed `v72_1.cmake` from **`v79_1.cmake`**, not `v83_1.cmake`: v79 already carries the
below-floor relocations (the reduced-struct clusters) and the finalized sentinel
dispositions (e.g. `RESET_LSP`, CFileStream relay). Every seeded value is **UNVERIFIED**
until its tracking row is marked ✔ with a signature-catalog entry. A value still equal to
v79 means UNVERIFIED unless a v72 signature confirms it.

## IDB labeling protocol

- Always `get_metadata` before a probe; do not infer the connected IDB from conversation
  ([[feedback-verify-ida-target]]). The v72 IDB is sparse/unverified — confirm its identity
  early and re-confirm after any `select_instance`.
- Apply the v83 canonical name verbatim so cross-version greps line up.
- `idb_save` at checkpoints (e.g. every subsystem group) so labels survive a swap/restart.
- Labeling functions/globals is encouraged. **Do not** apply speculative struct *types*
  into the IDB during the struct-verification pass (decompiler leak — see
  `struct-verification.md`).

## Backward-direction sentinel handling

Going from v79 *down* to v72, the sentinel logic continues the backward direction: a
feature v79 *has* may not exist yet in v72. For each v79 sentinel, confirm v72 is also
absent; and stay alert for v79 *real-address* keys whose backing feature is missing in the
older build (those become **new v72 sentinels**, and the consuming gate/edit must tolerate
`0`).

| Key | v79 disposition (start here) | v72 action |
|---|---|---|
| `C_BATTLE_RECORD_MAN_CREATE_INSTANCE` | sentinel (post-dates v83) | Confirm absent — carry `0x00000000` |
| `DR_CHECK` | sentinel ("does not exist") | Confirm absent — carry `0x00000000` |
| `DR_INIT` | sentinel (DR subsystem absent) | Confirm absent — carry `0x00000000` |
| `CE_TRACER_RUN` | sentinel ("does not exist") | Confirm absent — carry `0x00000000` |
| `RESET_LSP` | (use task-008's v79 verdict, not v83's stale comment) | **Verify in v72** — present or absent? |
| `C_SECURITY_CLIENT_ON_PACKET_*` | JMS sentinel | Carry `0x00000000` (GMS build) |
| `C_WVS_APP_INITIALIZE_GR2D_WINDOWED_OFFSET` | JMS sentinel | Carry `0x00000000` |
| `WIN_MAIN_LAUNCHER_STUB` | JMS sentinel | Carry `0x00000000` |
| `C_FILE_STREAM_*` (relay keys) | (use task-008's v79 verdict) | Decide for v72: recoverable or carry `0`. Document. |
| *(any v79 real key absent in v72)* | n/a | If a v79 feature does not exist in v72, the key becomes a new v72 sentinel — flag for the gate/edit owner |

## Key tracking

Maintain a full tracking table (seeded from `v79_1.cmake`, group order following
`include/memory_map.h.in`) with columns: **Key · v79 value · v72 value · status · signature
ref**. Status legend: ☐ todo · ◐ located, IDB labeled · ✔ written to cmake + catalogued.
Fill during the port; the table is the completeness ledger backing the FR-3 / FR-4 / FR-5
acceptance criteria. (This scaffold intentionally omits the 159-row table; generate it from
`v79_1.cmake` at task start so the v79 seed values are exact.)

## Resolution order (highest value first)

1. **WinMain → CWvsApp** — anchor the image, gain call-graph reach.
2. **CClientSocket / ZSocket → COutPacket** — highest value for `redirect` / `bypass`;
   confirm protocol constants (FR-6) here.
3. **CLogin / CStage / CLogo / CUITitle** — login & stage flow.
4. **Manager singletons** (`*_CREATE_INSTANCE` / `*_INSTANCE_ADDR`).
5. **Utilities** (ZArray/ZXString/fatal-section/CSystemInfo/CIGCipher).
6. **Exception-dispatch keys** (`C_TI_*EXCEPTION`, `C_FILE_STREAM_*`).
7. **Sentinels** last (confirm backward-direction absence).
</content>
