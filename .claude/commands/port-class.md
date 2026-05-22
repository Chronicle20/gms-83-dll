---
description: Run the standard MapleStory client struct port dispatch — v95 PDB reference + per-version IDB verification
argument-hint: Struct name (e.g. "CWvsContext", "SecondaryStat", "PARTYDATA")
---

You are kicking off a port of the **$ARGUMENTS** struct from the v95 PDB-derived reference down to the older client versions the project supports (GMS v83, GMS v87, JMS v185).

Follow `docs/version-porting-workflow.md` exactly — it is the runbook. Key checkpoints before dispatching any subagent:

1. **Confirm which IDB is loaded.** Ask the user, and verify with `mcp__ida-pro__get_metadata` if MCP is connected. Do not assume.
2. **v95 reference first.** If no `docs/tasks/<port-name>/v95_<struct>_reference.md` exists yet:
   - Connect to v95 IDB.
   - Run `mcp__ida-pro__analyze_struct_detailed` on $ARGUMENTS.
   - Pipe through `scripts/ida_struct_to_md.py --collapse-tears` to produce the reference markdown.
3. **Per-target version dispatch.** For each target version the user wants ported (typically v83 → v87 → JMS v185, in that order):
   - Ask the user to swap the IDB and confirm with `get_metadata`.
   - Dispatch the `version-port-verifier` agent with: struct name, target version, v95 reference path, current source header path, prior verification reports if any.
   - Read the verification report it produces.
   - Apply the proposed `#if BUILD_MAJOR_VERSION` gates to `common/<Struct>.h`.
   - Build all four targets (`cmake --build` for v83/v87/v95/JMS-185). Fix any breakage before moving on.
4. **Cross-validate bitmask registries.** If $ARGUMENTS is `SecondaryStat`, `ForcedStat`, or another bitmask-indexed registry, dispatch the `stat-registry-cross-validator` agent against atlas-ms (`~/source/atlas-ms/atlas/libs/atlas-packet/model/character_temporary_stat.go`) after each per-version mapping. Surface disagreements to the user — do not silently pick one side.
5. **Branch hygiene.** Do not commit to `main`. Use a feature branch (`port-<struct-lowercased>`). Do not skip pre-commit hooks.

## Anti-patterns to avoid

- Treating Hex-Rays decompiler output as ground truth (it substitutes already-applied types).
- Calling `read_memory_bytes` on UINT128 mask constants (BSS, runtime-init, returns 0xFF).
- Inferring the target version from the dispatch prompt instead of verifying the connected IDB.
- "Out of scope" deferrals of structural fixes you've already identified — surface them and let the user decide.
- Renaming the user's `int*`-placeholder types or `_ZtlSecureTear_<T>` flat-name encoding (deliberate conventions).

## End-of-port checklist

- [ ] All four build targets compile clean.
- [ ] Each target version has a verification report under `docs/tasks/<port-name>/`.
- [ ] For bitmask registries: a cross-validation report against atlas-ms exists for each version.
- [ ] Source header gates are sorted by version (`>= 87`, `>= 95`, JMS) and grouped, not scattered.
- [ ] If atlas-ms gating bugs were found, a separate PR has been opened against `Chronicle20/atlas`.
- [ ] Commit messages on the port branch reference the affected version(s).

Begin by reading `docs/version-porting-workflow.md` and checking whether `docs/tasks/<port-name>/v95_$ARGUMENTS_reference.md` already exists.
