# Version porting workflow

This project maintains hand-crafted C++ headers in `common/` whose struct layouts must match the MapleStory client binary for each supported region/version (e.g. GMS v83, v87, v95; JMS). The canonical source of truth is the **v95 PDB**, which IDA can load directly. Older versions don't have PDBs, so their layouts are reverse-engineered from disassembly.

The workflow below codifies how to port a single struct from a v95 PDB-derived reference back to an older version (here: v83) and emit a single header file with `#if BUILD_MAJOR_VERSION` gates that compiles correctly for both.

This document is the runbook. Two helpers automate parts of it:

- **`scripts/ida_struct_to_md.py`** — converts IDA `analyze_struct_detailed` JSON to a markdown reference table (with optional SecureTear-pair collapsing).
- **`.claude/agents/version-port-verifier.md`** — a subagent that takes a v95 reference + target-version IDB and produces a delta report.

## Constraints to know up front

- **One IDB at a time.** The IDA MCP plugin only services one IDB per port. You'll need to swap manually between v95 (extract) and v83 (verify). Plan to batch all extraction calls before swapping.
- **PDB symbols differ from source style.** PDB types may have IDA-only quirks: `int*` placeholders for unported dependent types, anonymous EBCO base classes (`TSingleton<T>`), flat-named `_ZtlSecureTear_long_` instead of templated pairs. See the **IDA conventions** section below.
- **Decompiler output leaks ground truth.** Once a struct definition is applied in the target IDB, Hex-Rays output will substitute the v95 field names into v83 method bodies, masking the very deltas you're trying to find. Always use raw disassembly (`disassemble_function`), not `decompile_function`, during verification.

## Phase 1: Extract the v95 reference

With the v95 IDB connected:

1. `mcp__ida-pro__analyze_struct_detailed name="<Struct>"` — pulls the full layout (offsets, sizes, types, names).
2. If the struct embeds other UDTs (e.g. `CWvsContext` embeds `SecondaryStat`, `PARTYDATA`, etc.), recurse: pull `analyze_struct_detailed` for each nested UDT.
3. Pipe the JSON results through `scripts/ida_struct_to_md.py` to produce a markdown reference under `docs/tasks/<port-name>/v95_<struct>_reference.md`.

For very-large UDTs (e.g. `SecondaryStat` is 5104 bytes / 830 raw members), use `--collapse-tears` to fold `_ZtlSecureTear_X` payload + `_X_CS` cookie pairs into single logical rows.

**Save the references.** Once you swap IDBs you cannot get them back without swapping again.

## Phase 2: Verify against the target IDB

Swap the MCP target to the v83 IDB. Then:

1. **Confirm metadata.** `get_metadata` shows the IDB you're actually connected to (a swap that failed silently is a real failure mode).
2. **Anchor a few high-confidence offsets first** before going deep. Disassemble 2–3 v83 methods that touch fields at the *start*, *middle*, and *end* of the struct. For each access, compute `v83_offset` and look up the v95 field at that offset. Confirm or refute the running delta Δ = v83_offset − v95_offset.
3. **Find the v83 struct size.** Sometimes a method like `Decode` calls `DecodeBuffer(this, <N>)` with a literal byte count — that's the struct size in v83. `ZArray<T>::RemoveAll` uses an `imul stride, <N>` instruction whose constant is `sizeof(T)`. The destructor unwind table also tells you total layout extent.
4. **Walk the v95 reference and verify each field group.** Track Δ as you go. Each Δ change tells you about a v95 field that v83 lacks (or vice versa). Anchor each verdict to a concrete disassembly line.
5. **Verify embedded UDT sizes** (don't assume same as v95 — they often shrink in earlier versions).

For non-trivial structs, this is where the **version-port-verifier subagent** earns its keep — dispatch it with the v95 reference path and let it produce the per-field verdict table.

## Phase 3: Propose and apply gates

From the verdict table, generate `#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)` blocks around the v95-only fields. Project conventions:

- Use the explicit gate `#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)` (matches existing `common/*.h` style; the `defined(REGION_GMS)` half is important because JMS may need different gates).
- For fields *added* in v83 (rare but happens — see `PARTYMEMBER::adwFieldID`), use the inverse: `#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION < 95)`.
- For array-size differences, branch the whole declaration: `#if … >= 95` / `array[60]` / `#else` / `array[52]` / `#endif`. Don't use ternary in array dimensions — readers and the preprocessor agree better with the if/else block form.
- Prefer to gate the *minimum* contiguous region. A v95-only field surrounded by present-in-both fields gets a 1-field gate, not a section-level gate.

Apply the edits to the existing `common/<Struct>.h`. If the struct references types not yet defined in the project, the choices are:

- Add a `#include` for a newly-created opaque stub (see "Stubs" below).
- Add an inline forward declaration in the header — sufficient for `ZRef<T>`, `ZArray<T>`, `ZList<T>` (all pointer-only storage).

## Stubs for unported dependent types

If `<Struct>` references type `T` that you haven't ported yet:

- **`T` used directly as a member field** (e.g. `T m_foo;`): the compiler needs `T`'s exact size. Create a header `common/<T>.h` with `struct T { unsigned char _opaque[<size>]; };` using the v95 size as the placeholder. Layout will be correct; field-level access in code that touches `m_foo` won't work until the type is properly ported, but layout-shifts in the parent struct are avoided.
- **`T` only via `ZRef<T>` / `ZArray<T>` / `ZList<T>`**: the wrappers all store `T*`, so a forward declaration suffices: `class T;` or `struct T;` at the top of the parent header.

Add `#include "<T>.h"` lines to `common/pch.h` *above* the parent struct's include line — pch.h is the precompiled header that pulls everything in dependency order. Forward declarations stay inline in the parent header.

## IDA conventions (gotchas)

These are intentional shortcuts; do not "fix" them when generating headers:

- **`int*` placeholder for unported pointer types.** When a v95 PDB has a `CFoo*` member but `CFoo` is not yet defined in this project's IDB, the user replaces the type with `int*` (size matches, semantics deferred). The v95 PDB-loaded IDB usually has the real type; the v83 IDB usually has `int*`.
- **Templates flattened to mangled names.** `ZtlSecureTear<T>` may appear as `_ZtlSecureTear_long_` (single typed member) or as two sibling fields (`_ZtlSecureTear_X` + `_ZtlSecureTear_X_CS`). Both are equivalent for layout.
- **EBCO bases land at offset 8 in v95-PDB-loaded structs.** When v95 has a `long double` member (forcing 8-byte class alignment), the vtable pointer is at offset 0 and the first explicit member starts at offset 0x08 — not 0x04. The 4 bytes at 0x04..0x07 are pure alignment padding. Older client versions usually have the same long double members, so the same alignment hole exists there even if the local IDA struct doesn't surface it.

## Common deltas to watch for

These patterns appeared in the CWvsContext port and are worth checking generically:

- **Class added in v95**: `m_pUIDragonBox`, `m_pUIBattleRecord`, `m_pUIAccountMoreInfo`, `m_pUIFindFriend`, `m_pClock`. v95-only UI types are common at the end of UI-pointer blocks. v83 simply lacks the member; gate `>= 95`.
- **Class became a TSingleton**: `m_pUIQuestAlarm`. v83 stores the pointer as a global `dword_XXXXXX` (TSingleton instance); v95 made it an instance member. Gate the member `>= 95` and update calling code to use the global in v83.
- **Mid-struct insertion**: `nDojangShield_`/`tDojangShield_`/`rDojangShield_` in SecondaryStat were added in v95; everything after shifts up by 0x24 bytes in v95 vs v83.
- **Array slot count growth**: `m_aRealEquip[60]` in v95 was `[52]` in v83. `m_aRealMechanicEquip[5]` was `[4]`. `m_aRealDragonEquip` is entirely v95+.
- **Embedded UDT growth**: `SecondaryStat` (3288→5104), `PARTYDATA` (298→378), `GUILDDATA` (42→74). The full body of these classes needs their own porting pass.
- **Feature removal in target**: PQReward (Party Quest reward) infrastructure is present in v95 but absent from v83. If `list_functions_filter "<Feature>"` returns 0 hits in v83, the related struct members can be gated `>= 95` with high confidence.

## A worked example: CWvsContext

See commit `8febec9` and the `docs/tasks/cwvscontext-port/` artifacts. That port:

1. Pulled v95 reference (`CWvsContext` 245 members, 16984 bytes) and nested types (`CFriend`, `Massacre`, `ITEMMSG`, `ITEMMSGINFO`).
2. Anchored on `OnSetAvatarMegaphone` and `SetCharacterData` for the first running-delta map.
3. Dispatched a verifier subagent on v83 that built the field-by-field verdict table.
4. Dispatched a second pass to resolve 4 open questions (pool pointer locations, time-field stride, PARTYDATA size, ItemMsg/WorldMap presence).
5. Generated gates for `common/CWvsContext.h`, `common/SecondaryStat.h`, `common/PartyData.h`, `common/PartyMember.h`, `common/GuildData.h`.
6. Created 4 opaque-but-typed stubs (`Tips.h`, `Privilege.h`, `FamilyInfo.h`, `PartyRaidTeam.h`) with v95-PDB-derived field layouts.
7. Verified the 4 stubs by disassembling `Decode`/`Init` methods in v83 — byte-for-byte identical to v95 in all three cases.

Total source impact: 10 files changed (4 new, 6 modified), 129 insertions / 18 deletions. Total IDA MCP calls across all subagents: ~150.

## Anti-patterns

- **Don't trust `analyze_struct_detailed` on a partially-mapped target IDB.** The user often defines just a few struct members in the target IDB; the rest is implicit. Trust disassembly over the (incomplete) struct definition.
- **Don't apply structures back to the IDB during verification.** Once you `declare_c_type` your inferred struct into the target IDB, Hex-Rays will show those names in subsequent decompiles, which loops back into your evidence. Keep the verification pass read-only.
- **Don't fight the IDA conventions.** `int*` placeholders, flat-named SecureTear pairs, anonymous EBCO bases — these are intentional and consistent. Preserve them in source where the user has used them, even if the v95 PDB shows a richer name.
- **Don't try to read runtime-initialized data with `read_memory_bytes`.** UINT128 mask constants, virtual table pointers, and anything in BSS/uninitialized data sections return all `0xFF` from the static image — the runtime fills them in via static initializers. When you need bit positions or values, derive them from disassembly access patterns (e.g., which `[esi+OFFSET]` writes follow each `push offset unk_X`), not from the data sections themselves.
- **Don't rely on IDA-label conventions across IDBs.** A class's stat-ID globals might be named `CTS_<Stat>` in one IDB (e.g. v83 here has 82 named CTS_ globals from the user's prior RE work) and completely unlabeled in another (v87 has zero CTS_ globals — same binary class, different IDA labeling state). When a labeling probe returns nothing, fall back to structural probes (Reset disassembly, mask-comparison sequences).
- **Don't infer the connected IDB from conversation.** A user message saying "v87 loaded" doesn't mean the swap happened — they might have meant to and forgotten. Call `get_metadata` before drawing any version-specific conclusions. See [[feedback-verify-ida-target]].

## Cross-validating against atlas-ms

The user maintains a sibling Go server codebase at `~/source/atlas-ms/atlas/` (`Chronicle20/atlas` on GitHub) with version-aware registries that are authoritative for the **set and order** of bitmask-indexed fields. Cross-referencing client RE against atlas-ms catches bugs in BOTH directions.

The most useful registry: `libs/atlas-packet/model/character_temporary_stat.go` — the `buildCharacterTemporaryStatRegistry` function lists all 125 character TemporaryStat IDs in canonical declaration order, with `t.MajorVersion()` / `t.Region()` gates marking per-version availability. Each `newAndIncNonDiseased(...)` is one stat; the order is the UINT128 bit order.

**How to use it:**

1. After identifying client-side stat groups (e.g. by counting Reset's UINT128 mask comparisons), look up each one's name in atlas-ms's registry.
2. Check whether atlas-ms's gates match what the client actually has. A gate like `MajorVersion() > 83` opens for v87, v95, etc. — but if the client at one of those versions doesn't have a particular stat, atlas-ms is over-permissive and the wire format will be misaligned for that tenant. (This is exactly the bug found in the 2026-05-22 session: atlas-ms's `> 83` gate over-included 24 stats for v87. Fixed in `Chronicle20/atlas#564`.)
3. Disagreements between atlas-ms and client RE are real findings worth investigating — don't silently pick one as ground truth.

**Limits:** atlas-ms encodes the user's evolving understanding, which can itself be wrong. Treat as one source of evidence, not the oracle. The pattern works best for *registries* (stat lists, packet opcodes) — less so for arbitrary struct layouts where the server doesn't care about field offsets.

## Subagent cross-validation discipline

Multiple subagents in the 2026-05-21/22 sessions returned reports with structural errors that only later, more careful passes caught:

- The original v83 SecondaryStat verifier counted all 269 pairs as 12-byte. The later deep mapping found 2 of them were 8-byte `SecureTear<char>` slots.
- The v87 SecondaryStat deep mapping mis-identified `rMirrorImage_` at v87 0xD24 as `nStopMotion_`, and inferred a 16-byte "mystery padding" that didn't exist.
- Multiple verifiers initially treated "Atlas's `> 83` gate is right" as a load-bearing premise without checking.

**Rule:** before treating a subagent's structural claim as authoritative, anchor it to at least one independent disassembly probe in the main session. The subagent's offset-by-offset table is usually right; the high-level summary numbers ("v87 has 84 pairs" / "v87 stops at nStopMotion") often hide errors. Spot-check the boundary cases.

## Future work

- A dual-IDB MCP setup would eliminate the swap dance entirely. The IDA MCP plugin supports binding to a specific port; running two instances on different ports lets one Claude session query both. Not yet set up in this project.
- Some embedded UDTs need their own porting passes (`SecondaryStat` internals beyond size; `PARTYMEMBER`'s 7th array field; `GUILDDATA`'s pre-v95 members). These are deferred from the CWvsContext port.

## Appendix: mistakes from prior sessions (don't repeat)

These are documented separately so they don't get repeated in future sessions. If you find yourself making any of these, stop and re-read this section.

1. **Deferring known bugs with "out of scope" / "unlikely to matter" reasoning.** Caught the 2026-05-21 session deferring a SecureTear byte-vs-long fix for two fields because "they're dojang fields probably nobody uses." That's not a real reason — the work to fix was trivial. If a probe can settle a question, run the probe. (Memory: [[feedback-prefer-confirmation]].)
2. **Probing the wrong IDB.** Ran `list_globals_filter` against v83 while the conversation context implied v87 was loaded. User had to point out the mistake before the probes were ever directed at v87. (Memory: [[feedback-verify-ida-target]].)
3. **Trusting a subagent's structural summary without cross-checking.** v87 SecondaryStat verifier's "v87 stops at `nStopMotion_`" claim was wrong by 4 stat groups; corrected later. The detailed per-offset table was right; the summary number wasn't.
4. **Building gates from one verifier without consulting the others.** First v83 SecondaryStat report said v83 lacks DojangShield (`nSmartKnockback_..nStopMotion_` "absent in v83"); deep mapping found v83 has all of those. Gates added based on the first report were wrong and had to be reversed.
5. **Adding speculative source-layout placeholders.** The v87 16-byte `_v87_unknown_0xD30` padding I inserted based on the wrong subagent finding became dead code that had to be removed in a later commit. When unsure about a gap, document it in the verification report rather than encoding it in source.
6. **Committing directly to `main`.** Done once on 2026-05-21; user corrected immediately. Always work on a feature branch. (Memory: [[feedback-no-main-commits]].)
