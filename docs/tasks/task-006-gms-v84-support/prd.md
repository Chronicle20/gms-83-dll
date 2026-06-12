# GMS v84 Support — Product Requirements Document

Version: v1
Status: Draft
Created: 2026-06-05
---

## 1. Overview

This DLL collection is built per region+version: the build matrix selects a
`memory_maps/<REGION>/v<MAJOR>_<MINOR>.cmake` file, which supplies every
absolute address, opcode, and instruction-relative offset the edits patch into
the client. Source headers in `common/` describe client struct layouts and gate
version-specific deltas behind `#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION …)`
blocks. To support a new client build, three things must come together: the CI
matrix must include it, a complete memory map must exist for it, and the gated
struct layouts must actually match that client.

This task adds **GMS v84.1** as a first-class supported target. A GMS v84 IDB is
already loaded in the IDA MCP, but it has only sparse RE labels. The work is
therefore primarily *signature/pattern-driven relocation*: for each of the 145
memory-map keys, identify the corresponding function/global in the v84 binary by
matching byte/structure signatures derived from the already-labeled reference
IDBs (v83 is the closest anchor; v87 and the v95 PDB are secondary references),
record the v84 address, and **label the function back into the v84 IDB** via the
MCP so the IDB accrues a usable symbol set as we go.

A second, equally important deliverable is a **reusable signature/pattern
catalog**: for each non-trivial function we locate, we document the heuristic
(string xref, call-graph anchor, byte pattern, constant, vtable slot) used to
find it. v84 is explicitly not the last version we will port this way, so the
catalog is a durable artifact that makes the next version port faster.

Finally, because v84 sits *exactly* on an existing source gate boundary
(`< 84` / `> 83` / `== 83` gates already exist), every one of those gates is an
unverified hypothesis for v84. This task verifies the layout/size of all 22
version-gated `common/` headers against the v84 binary and corrects any gate
that v84 disproves.

## 2. Goals

Primary goals:
- Add `{ region: GMS, major: 84, minor: 1 }` to the CI build matrix
  (`.github/workflows/_build.yml`, the single source of truth) so it is
  exercised by PR, snapshot, and release workflows.
- Produce `memory_maps/GMS/v84_1.cmake` populating **all 145 keys** required by
  `include/memory_map.h.in`, with each real address located in the v84 binary
  and each feature-absent / region-specific / offset key handled deliberately
  (see §5, §7).
- Label the v84 IDB in place (via the IDA MCP `rename`/`set_type`) for every
  function/global resolved, so the IDB ends the task substantially more usable
  than it started.
- Verify struct size/layout for **all 22 version-gated `common/*.h` headers**
  against v84 disassembly; empirically confirm or correct every 83/84 boundary
  gate.
- Produce a reusable signature/pattern catalog documenting how each function was
  identified.
- Build GMS 84.1 green in both Debug and Release; smoke-test a live GMS v84
  client with the proxy + core edits loaded.

Non-goals:
- No new client-edit features. This is a porting task only.
- No re-porting of struct *internals* beyond what is needed to confirm size and
  the version-gated field boundaries.
- No changes to non-GMS regions, and no changes to other GMS versions except
  where the 83/84 boundary-gate audit forces a (region-correct) gate rewrite
  that also touches another version's compiled branch.
- Not a goal to reach 100% symbol coverage of the v84 IDB — only the symbols
  backing the 145 keys and the 22 verified structs.

## 3. User Stories

- As a **server operator** running a GMS v84 client, I want prebuilt edit DLLs
  for my exact version so that the proxy and edits load without me reverse
  engineering addresses myself.
- As a **developer** adding the next GMS version, I want a documented signature
  catalog so that I can relocate the same 145 functions quickly instead of
  re-discovering each one.
- As a **maintainer**, I want the v84 IDB labeled and the boundary gates
  verified so that future struct ports start from trustworthy ground truth
  rather than inherited assumptions.
- As a **CI consumer**, I want GMS 84.1 in the matrix so that a regression that
  breaks the v84 build is caught on every PR.

## 4. Functional Requirements

### 4.1 Build matrix
- FR-1: `_build.yml` includes `{ region: GMS, major: 84, minor: 1 }` in the
  `strategy.matrix.config` list. No other workflow file needs editing (PR,
  snapshot, and release all consume `_build.yml`).
- FR-2: The new matrix entry builds both Debug and Release (inherited from the
  reusable workflow's two callers) without modification to other entries.

### 4.2 Memory map
- FR-3: `memory_maps/GMS/v84_1.cmake` defines every one of the 145 keys parsed
  from `include/memory_map.h.in`. The build's `GenerateMemoryMap.cmake` fails
  hard if any key is undefined or empty, so completeness is non-negotiable and
  CI-enforced.
- FR-4: Each **absolute-address** key points to the v84 function/global that is
  the semantic equivalent of the v83 key, confirmed by a documented signature
  (§5).
- FR-5: Each **instruction-relative offset** key (e.g. `*_OFFSET`,
  `WIN_MAIN_AD_BALLOON_CONDITIONAL`, `WIN_MAIN_PATCHER_OFFSET`,
  `C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET`) is re-derived from the v84 instruction
  layout, not copied from v83 (instruction-relative offsets shift when codegen
  changes).
- FR-6: Each **protocol constant** key (`VERSION_HEADER`, `PLAYER_LOGGED_IN`,
  `CLIENT_START_ERROR`) is confirmed against v84; if it differs from v83 the v84
  value is used and the difference is noted.
- FR-7: Each **feature-absent / region-specific sentinel** (`0x00000000` keys
  marked "does not exist" or "JMS only" in v83: `C_BATTLE_RECORD_MAN_CREATE_INSTANCE`,
  `DR_CHECK`, `CE_TRACER_RUN`, the five JMS-only security/launcher keys, and
  `RESET_LSP`) is evaluated for v84: confirm the feature is genuinely absent
  before carrying the sentinel forward, and document the determination.
- FR-8: Where a v83 key carries a clarifying comment (e.g. `# does not exist`,
  `# JMS only`, `# TODO do we still need these`), the v84 map carries an
  accurate equivalent comment reflecting the v84 finding.

### 4.3 IDB labeling
- FR-9: For every function/global resolved for a memory-map key, apply the
  canonical name to the v84 IDB via the IDA MCP (`rename`, and `set_type` where
  a prototype is known). Before treating the IDB as the active target for any
  probe, confirm it via `get_metadata` (per [[feedback-verify-ida-target]]).
- FR-10: Labeling is incremental and committed to the IDB (`idb_save`) at
  reasonable checkpoints so progress survives an MCP swap or restart.

### 4.4 Struct verification & gate audit
- FR-11: For each of the 22 version-gated `common/*.h` headers, determine the
  v84 size and the presence/absence of every version-gated field, anchored to
  v84 disassembly evidence (see `struct-verification.md`).
- FR-12: Each existing 83/84 boundary gate
  (`doom-fix/dllmain.cpp:25` `< 84`, `common/CWvsContext.h:98` `> 83`,
  `bypass/socket_hooks.cpp:233` `> 83`, `common/CLogin.h:235` `== 83`, and any
  `>= 83` / `> 83` gate that changes truth value at 84) is verified against v84
  behavior. Where v84 diverges from the branch it currently selects, the gate is
  rewritten to a correct boundary (e.g. introduce explicit `>= 84` / `== 84`)
  and the change documented with evidence.
- FR-13: A gate rewrite must remain correct for all *other* currently-supported
  versions (v83, v87, v95, v111, JMS 185). Rewrites are evaluated against the
  full matrix, not just v84.

### 4.5 Signature catalog
- FR-14: A signature/pattern catalog (`signature-catalog.md`, see that file for
  schema) records, per resolved function, the heuristic used to find it in v84
  and how robust that heuristic is expected to be across versions.

### 4.6 Build & runtime acceptance
- FR-15: GMS 84.1 configures and builds clean (no errors) for Debug and Release
  locally and in CI.
- FR-16: A live GMS v84 client launches with the proxy `ijl15.dll` and the core
  edits deployed, reaches the title/login screen, and the targeted edits behave
  (no crash, no Themida fault) — manual smoke test, results recorded.

## 5. Hook / Patch Surface

This task does not introduce new hooks; it relocates the existing ones for v84.
The patch surface is the full set of 145 keys in `include/memory_map.h.in`,
grouped by subsystem:

- **CWvsApp lifecycle** (~22 keys, `C_WVS_APP_*`): app init/run/setup, subsystem
  initializers (auth, pcom, res-man, gr2d, input, sound, game-data), window
  creation, directory helpers.
- **CClientSocket / ZSocket** (~14 keys): send/flush/process/connect paths used
  by `redirect` and `bypass`.
- **COutPacket** (~7 keys): encode primitives used to craft packets.
- **CLogin / CLogo / CStage / CUITitle** (login & stage flow), **CConfig**
  (windowed mode / sys-opt), **CInputSystem**, **CFuncKeyMappedMan** /
  quickslot, and the manager singletons (`*_CREATE_INSTANCE` / `*_INSTANCE_ADDR`
  for MapleTV, MonsterBook, Quest, Radio, Security, ActionMan, etc.).
- **WinMain** entry + offsets (`WIN_MAIN`, `WIN_MAIN_AD_BALLOON_CONDITIONAL`,
  `WIN_MAIN_PATCHER_OFFSET`, `SEND_HS_LOG`).
- **Party / migrate message senders** with call-site offsets.
- **Misc utilities**: `ZArray::RemoveAll`, `ZXString` trim/get-buffer, fatal
  section ctor/dtor, `CSystemInfo`, `CIGCipher`.
- **Sentinels** (GMS-absent or JMS-only): see FR-7.

Region/version applicability: GMS v84.1 only. The per-key resolution method and
anchor references are detailed in `memory-map.md`; the identification heuristics
are catalogued in `signature-catalog.md`.

## 6. Configuration

No new INI configuration keys. Existing edits keep their current INI surfaces
(e.g. `redirect.ini`). The only "config" change is the build-time
`BUILD_MAJOR_VERSION=84` selection, wired through the matrix and CMake exactly as
existing versions.

## 7. Memory Map Impact

A new file `memory_maps/GMS/v84_1.cmake` is required, with all 145 keys. Key
classes and how each is handled:

| Class | Examples | v84 handling |
|---|---|---|
| Absolute address | `C_WVS_APP_RUN`, `C_CLIENT_SOCKET_SEND_PACKET` | Relocate via signature; label in IDB |
| Instruction-relative offset | `WIN_MAIN_PATCHER_OFFSET`, `*_MSG_OFFSET` | Re-derive from v84 codegen |
| Protocol constant | `VERSION_HEADER`, `PLAYER_LOGGED_IN` | Confirm against v84 |
| GMS-absent sentinel | `DR_CHECK`, `CE_TRACER_RUN`, `RESET_LSP`, `C_BATTLE_RECORD_MAN_CREATE_INSTANCE` | Confirm absence, carry `0x00000000` + comment |
| JMS-only sentinel | `C_SECURITY_CLIENT_ON_PACKET_CHECK*`, `WIN_MAIN_LAUNCHER_STUB`, `C_WVS_APP_INITIALIZE_GR2D_WINDOWED_OFFSET` | Carry `0x00000000` (GMS build) |

No changes to other regions' maps. The 22 `common/*.h` gate audit (§4.4) may
modify shared headers and the four boundary-gate call sites; those changes are
source, not memory-map.

## 8. Non-Functional Requirements

- **CI completeness enforcement.** The build must fail loudly (it already does
  via `GenerateMemoryMap.cmake`) if any v84 key is missing — no silently-empty
  addresses.
- **No regressions to existing versions.** v83/v87/v95/v111/JMS185 builds must
  remain green; any boundary-gate rewrite is validated against all of them.
- **Evidence discipline.** Every address and every gate verdict is anchored to a
  concrete disassembly observation, per the project's porting discipline
  ([[feedback-prefer-confirmation]]). "Same as v83" is acceptable only when a
  signature confirms it, not assumed.
- **IDB target hygiene.** `get_metadata` confirms the connected IDB before any
  version-specific probe ([[feedback-verify-ida-target]]).
- **Read-only verification.** During struct verification, do not apply inferred
  struct definitions back into the IDB before reading method bodies (decompiler
  leak), per the version-porting workflow. (IDB *function/global labeling* for
  the memory map is fine and desired; applying speculative *struct types* during
  layout verification is what's prohibited.)
- **AV/Themida compatibility.** Patched addresses must be valid for the v84
  image so the client passes Themida integrity expectations the same way other
  versions do.

## 9. Open Questions

- Are the protocol constants (`VERSION_HEADER`=8, `PLAYER_LOGGED_IN`=0x14,
  `CLIENT_START_ERROR`=0x19) identical in v84, or did the opcode table shift
  between v83 and v84? (Resolve during port; affects `redirect`/`bypass`.)
- Does any GMS-absent sentinel feature (`CBattleRecordMan`, `DRCheck`,
  `CETracer`) actually *appear* in v84? If a v83-absent feature exists in v84,
  the corresponding edit gains a real address rather than a sentinel.
- Do all four boundary gates' current assumptions ("v84 behaves like v87+")
  hold, or does v84 sit closer to v83 in some of them? (Empirical; FR-12.)

## 10. Acceptance Criteria

- [ ] `_build.yml` matrix includes `{ GMS, 84, 1 }`; PR/snapshot/release inherit
      it with no other workflow edits.
- [ ] `memory_maps/GMS/v84_1.cmake` exists with all 145 keys defined and
      non-empty (CMake `GenerateMemoryMap` passes).
- [ ] Every absolute-address key is backed by a documented signature in
      `signature-catalog.md`; every offset key is re-derived from v84 codegen;
      every sentinel is justified.
- [ ] The v84 IDB has labels applied for all resolved functions/globals and is
      saved.
- [ ] All 22 version-gated `common/*.h` headers have a recorded v84 size/layout
      verdict in `struct-verification.md`.
- [ ] Each 83/84 boundary gate is verified against v84; divergent gates are
      rewritten with evidence and validated to keep v83/v87/v95/v111/JMS185
      correct.
- [ ] GMS 84.1 builds clean Debug + Release locally and in CI.
- [ ] Live GMS v84 client launches to title/login with proxy + core edits, no
      crash/Themida fault; smoke-test result recorded.
