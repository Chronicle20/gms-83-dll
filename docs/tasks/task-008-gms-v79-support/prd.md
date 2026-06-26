# GMS v79 Support — Product Requirements Document

Version: v1
Status: Draft
Created: 2026-06-26
---

## 1. Overview

This DLL collection is built per region+version: the build matrix selects a
`memory_maps/<REGION>/v<MAJOR>_<MINOR>.cmake` file, which supplies every
absolute address, opcode, and instruction-relative offset the edits patch into
the client. Source headers in `common/` describe client struct layouts and gate
version-specific deltas behind
`#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION …)` blocks. To support a new
client build, three things must come together: the CI matrix must include it, a
complete memory map must exist for it, and the gated struct layouts must
actually match that client.

This task adds **GMS v79.1** as a first-class supported target. A GMS v79 IDB is
already loaded in the IDA MCP, but it has only sparse RE labels. The work is
therefore primarily *signature/pattern-driven relocation*: for each of the 155
memory-map keys, identify the corresponding function/global in the v79 binary by
matching byte/structure signatures derived from the already-labeled reference
IDBs (v83 is the closest anchor; v87 and the v95 PDB are secondary references),
record the v79 address, and **label the function back into the v79 IDB** via the
MCP so the IDB accrues a usable symbol set as we go.

A second deliverable is a contribution to the **reusable signature/pattern
catalog**: for each non-trivial function we locate, we document the heuristic
(string xref, call-graph anchor, byte pattern, constant, vtable slot) used to
find it. v79 is explicitly not the last version we will port this way, so the
catalog is a durable artifact that makes the next version port faster.

### Why v79 is uniquely risky: it is below the floor

v84 sat *between* two supported versions (83 and 87); v79 sits **below the
lowest supported version (83)**. Every other supported build is ≥ 83, so the
entire body of `common/` gates was authored treating **v83 as the minimum**.
Two structural consequences follow, and they make this port categorically
different from the v84 port:

1. **Enumerated gates have no branch for v79 → hard build breaks.** Some headers
   gate by explicit version equality with no catch-all `#else`. `BUILD_MAJOR_VERSION
   == 83 || == 84 || == 87` (and similar) is **false for v79**, so the member is
   left undefined and the struct layout silently shifts or the build fails. These
   *must* be amended for v79 before it compiles at all (see §4.4 / FR-12a).
2. **Lower-bound (`>= 83`, `>= 84`) gates newly exclude v79.** A `>= 83` gate that
   currently includes *every* supported version puts **v79 on the excluded side**
   for the first time. Whether that exclusion is *correct* (does v79 genuinely
   lack the field?) is an unverified hypothesis for each such gate, and there is
   **no labeled IDB below v79** to triangulate against — v83 is the only nearby
   anchor and it is *above* v79.

This task verifies the layout/size of all 24 version-gated `common/` headers
against the v79 binary, amends every enumerated gate that lacks a v79 branch, and
empirically confirms or corrects every lower-bound gate that newly excludes v79.

## 2. Goals

Primary goals:
- Add `{ region: GMS, major: 79, minor: 1 }` to the CI build matrix
  (`.github/workflows/_build.yml`, the single source of truth) so it is
  exercised by PR, snapshot, and release workflows. Placed first (matrix is
  version-ascending).
- Produce `memory_maps/GMS/v79_1.cmake` populating **all 155 keys** required by
  `include/memory_map.h.in`, with each real address located in the v79 binary
  and each feature-absent / region-specific / offset key handled deliberately
  (see §5, §7).
- Label the v79 IDB in place (via the IDA MCP `rename`/`set_type`) for every
  function/global resolved, so the IDB ends the task substantially more usable
  than it started.
- Verify struct size/layout for **all 24 version-gated `common/*.h` headers**
  against v79 disassembly; **amend every enumerated gate with no v79 branch**,
  and empirically confirm or correct every lower-bound (`>= 83` / `>= 84` / `< 95`)
  gate whose truth value changes at the 79 boundary.
- Contribute v79 entries to the reusable signature/pattern catalog.
- Build GMS 79.1 green in both Debug and Release; smoke-test a live GMS v79
  client with the proxy + core edits loaded, reaching the title/login screen.

Non-goals:
- No new client-edit features. This is a porting task only.
- No re-porting of struct *internals* beyond what is needed to confirm size and
  the version-gated field boundaries.
- No changes to non-GMS regions, and no changes to other GMS versions except
  where the below-floor gate audit forces a (region-correct) gate rewrite that
  also touches another version's compiled branch.
- Not a goal to reach 100% symbol coverage of the v79 IDB — only the symbols
  backing the 155 keys and the 24 verified structs.

## 3. User Stories

- As a **server operator** running a GMS v79 client, I want prebuilt edit DLLs
  for my exact version so that the proxy and edits load without me reverse
  engineering addresses myself.
- As a **developer** adding the next (possibly even older) GMS version, I want a
  documented signature catalog and a worked below-floor gate audit so that I can
  relocate the same 155 functions and lower the gate floor quickly.
- As a **maintainer**, I want the v79 IDB labeled and the floor gates verified so
  that future struct ports start from trustworthy ground truth rather than the
  implicit "v83 is the minimum" assumption.
- As a **CI consumer**, I want GMS 79.1 in the matrix so that a regression that
  breaks the v79 build is caught on every PR.

## 4. Functional Requirements

### 4.1 Build matrix
- FR-1: `_build.yml` includes `{ region: GMS, major: 79, minor: 1 }` in the
  `strategy.matrix.config` list (placed first to keep the list version-ascending).
  No other workflow file needs editing (PR, snapshot, and release all consume
  `_build.yml`).
- FR-2: The new matrix entry builds both Debug and Release (inherited from the
  reusable workflow's two callers) without modification to other entries.

### 4.2 Memory map
- FR-3: `memory_maps/GMS/v79_1.cmake` defines every one of the 155 keys parsed
  from `include/memory_map.h.in`. The build's key-completeness check fails hard
  if any key is undefined or empty, so completeness is non-negotiable and
  CI-enforced.
- FR-4: Each **absolute-address** key points to the v79 function/global that is
  the semantic equivalent of the v83 key, confirmed by a documented signature
  (§5).
- FR-5: Each **instruction-relative offset** key (e.g. `*_OFFSET`,
  `WIN_MAIN_AD_BALLOON_CONDITIONAL`, `WIN_MAIN_PATCHER_OFFSET`,
  `C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET`) is re-derived from the v79 instruction
  layout, not copied from v83 (instruction-relative offsets shift when codegen
  changes).
- FR-6: Each **protocol constant** key (`VERSION_HEADER`, `PLAYER_LOGGED_IN`,
  `CLIENT_START_ERROR`) is confirmed against v79; if it differs from v83 the v79
  value is used and the difference is noted. (v79 is an older protocol revision;
  opcode/handler-table drift versus v83 is plausible and must be checked, not
  assumed.)
- FR-7: Each **feature-absent / region-specific sentinel** (`0x00000000` keys
  marked "does not exist" or "JMS only" in v83: `C_BATTLE_RECORD_MAN_CREATE_INSTANCE`,
  `DR_CHECK`, `DR_INIT`, `CE_TRACER_RUN`, the JMS-only security/launcher keys, and
  `RESET_LSP`) is evaluated for v79. **Direction matters:** going *backward* from
  v83, a feature that exists in v83 may be *absent* in v79 (the inverse of the v84
  case). Confirm each sentinel's v79 disposition before carrying it forward, and
  also watch for v83 *non*-sentinel keys whose backing feature does not exist in
  v79 — those become new v79 sentinels and the relevant gate/edit must tolerate it.
- FR-8: Where a v83 key carries a clarifying comment (e.g. `# does not exist`,
  `# JMS only`, `# DR anti-debug subsystem absent in v83`), the v79 map carries an
  accurate equivalent comment reflecting the v79 finding.
- FR-8a: The exception-dispatch keys added after the v84 port
  (`C_TI_*EXCEPTION`, `C_PATCH_EXCEPTION_BUILDER`, `C_COM_RAISE_ERROR_EX`, the
  `C_FILE_STREAM_*` relay keys) are present in the 155-key set and must be
  resolved or sentinel-justified for v79 (v83 gates the CFileStream relay off with
  `0`; confirm whether v79 can or cannot recover those helpers).

### 4.3 IDB labeling
- FR-9: For every function/global resolved for a memory-map key, apply the
  canonical name to the v79 IDB via the IDA MCP (`rename`, and `set_type` where
  a prototype is known). Before treating the IDB as the active target for any
  probe, confirm it via `get_metadata` (per [[feedback-verify-ida-target]]).
- FR-10: Labeling is incremental and committed to the IDB (`idb_save`) at
  reasonable checkpoints so progress survives an MCP swap or restart.

### 4.4 Struct verification & below-floor gate audit
- FR-11: For each of the 24 version-gated `common/*.h` headers, determine the
  v79 size and the presence/absence of every version-gated field, anchored to
  v79 disassembly evidence (see `struct-verification.md`).
- FR-12a (**enumerated-gate amendment — highest priority, build-blocking**): Every
  gate that selects by explicit version equality with **no catch-all `#else`**
  must be amended so v79 selects a defined branch. Known sites (verify the live
  set during the task):
  - `common/CFuncKeyMappedMan.h:38` — `(== 83 || == 84 || == 87)` → v79 has no
    branch. Add v79 to the layout-matching branch.
  - `common/CWvsApp.h:97` — `(== 83 || == 84)` → v79 has no branch. Add v79 to
    the layout-matching branch.
  Which branch v79 joins is decided by v79 disassembly, not by assuming "oldest
  ⇒ v83 branch."
- FR-12b (**lower-bound gate audit**): Every gate whose truth value changes at the
  79 boundary — primarily `>= 83` and `>= 84` gates that newly put v79 on the
  *excluded* side — is verified against v79 behavior. Known sites:
  - `common/CUIToolTip.h:92` — `>= 83`: confirm whether v79 has the field
    (`m_pLayerAdditional`); if v79 has it, lower the floor (e.g. `>= 79`), else
    confirm exclusion is correct.
  - `>= 84` gates (`common/CMob.h:229`, `:233`, `common/CMapLoadable.h:154`,
    `common/CUIToolTip.h:125`, `:152`) — v79 excluded, sides with v83; confirm the
    gated field is genuinely absent in v79.
  - `< 95` / base-branch gates (`common/CMob.h:110`, and every `>= 87`/`>= 95`/
    `>= 111`/`== 95`/`== 83`/`> 83` gate that resolves v79 onto the *base* branch):
    confirm the base branch (authored as "v83 layout") actually matches v79's
    layout/size. Spot-check sizes; a silent v79≠v83 base delta is the quietest
    failure mode.
- FR-13: A gate amendment/rewrite must remain correct for all *other*
  currently-supported versions (v83, v84, v87, v95, v111, JMS 185). Rewrites are
  evaluated against the full matrix, not just v79.

### 4.5 Signature catalog
- FR-14: The signature/pattern catalog (`signature-catalog.md`) records, per
  resolved function, the heuristic used to find it in v79 and how robust that
  heuristic is expected to be across versions (including v79-specific notes where
  the v83 anchor's string/constant drifted in the older build).

### 4.6 Build & runtime acceptance
- FR-15: GMS 79.1 configures and builds clean (no errors) for Debug and Release
  locally and in CI.
- FR-16: A live GMS v79 client launches with the proxy `ijl15.dll` and the core
  edits deployed, reaches the title/login screen, and the targeted edits behave
  (no crash, no Themida fault) — manual smoke test, results recorded.

## 5. Hook / Patch Surface

This task does not introduce new hooks; it relocates the existing ones for v79.
The patch surface is the full set of 155 keys in `include/memory_map.h.in`,
grouped by subsystem:

- **CWvsApp lifecycle** (`C_WVS_APP_*`): app init/run/setup, subsystem
  initializers (auth, pcom, res-man, gr2d, input, sound, game-data), window
  creation, directory helpers.
- **CClientSocket / ZSocket**: send/flush/process/connect paths used by
  `redirect` and `bypass`.
- **COutPacket**: encode primitives used to craft packets.
- **CLogin / CLogo / CStage / CUITitle** (login & stage flow), **CConfig**
  (windowed mode / sys-opt), **CInputSystem**, **CFuncKeyMappedMan** / quickslot,
  and the manager singletons (`*_CREATE_INSTANCE` / `*_INSTANCE_ADDR` for MapleTV,
  MonsterBook, Quest, Radio, Security, ActionMan, etc.).
- **WinMain** entry + offsets (`WIN_MAIN`, `WIN_MAIN_AD_BALLOON_CONDITIONAL`,
  `WIN_MAIN_PATCHER_OFFSET`, `SEND_HS_LOG`).
- **Party / migrate message senders** with call-site offsets.
- **Misc utilities**: `ZArray::RemoveAll`, `ZXString` trim/get-buffer, fatal
  section ctor/dtor, `CSystemInfo`, `CIGCipher`.
- **Faithful exception-dispatch keys**: `C_TI_*EXCEPTION`,
  `C_PATCH_EXCEPTION_BUILDER`, `C_COM_RAISE_ERROR_EX`, `C_FILE_STREAM_*`.
- **Sentinels** (GMS-absent or JMS-only): see FR-7.

Region/version applicability: GMS v79.1 only. The per-key resolution method and
anchor references are detailed in `memory-map.md`; the identification heuristics
are catalogued in `signature-catalog.md`.

## 6. Configuration

No new INI configuration keys. Existing edits keep their current INI surfaces
(e.g. `redirect.ini`). The only "config" change is the build-time
`BUILD_MAJOR_VERSION=79` selection, wired through the matrix and CMake exactly as
existing versions.

## 7. Memory Map Impact

A new file `memory_maps/GMS/v79_1.cmake` is required, with all 155 keys. Key
classes and how each is handled:

| Class | Examples | v79 handling |
|---|---|---|
| Absolute address | `C_WVS_APP_RUN`, `C_CLIENT_SOCKET_SEND_PACKET` | Relocate via signature; label in IDB |
| Instruction-relative offset | `WIN_MAIN_PATCHER_OFFSET`, `*_MSG_OFFSET` | Re-derive from v79 codegen |
| Protocol constant | `VERSION_HEADER`, `PLAYER_LOGGED_IN` | Confirm against v79 (older protocol — verify, don't copy) |
| GMS-absent sentinel | `DR_CHECK`, `DR_INIT`, `CE_TRACER_RUN`, `RESET_LSP`, `C_BATTLE_RECORD_MAN_CREATE_INSTANCE` | Confirm absence (likely also absent older); carry `0x00000000` + comment |
| JMS-only sentinel | `C_SECURITY_CLIENT_ON_PACKET_CHECK*`, `WIN_MAIN_LAUNCHER_STUB`, `C_WVS_APP_INITIALIZE_GR2D_WINDOWED_OFFSET` | Carry `0x00000000` (GMS build) |
| v83-present / v79-absent (new) | any v83 real address whose feature does not exist in v79 | Becomes a v79 sentinel; gate/edit must tolerate `0` |

No changes to other regions' maps. The 24 `common/*.h` gate audit (§4.4) may
modify shared headers and the floor-gate call sites; those changes are source,
not memory-map. Source changes that add v79 to an enumerated gate (FR-12a) touch
shared headers but must leave every other version's selected branch unchanged.

## 8. Non-Functional Requirements

- **CI completeness enforcement.** The build must fail loudly if any v79 key is
  missing — no silently-empty addresses.
- **No regressions to existing versions.** v83/v84/v87/v95/v111/JMS185 builds must
  remain green; any gate amendment is validated against all of them (FR-13).
- **Evidence discipline.** Every address and every gate verdict is anchored to a
  concrete disassembly observation, per the project's porting discipline
  ([[feedback-prefer-confirmation]]). "Same as v83" is acceptable only when a
  signature confirms it, not assumed — especially load-bearing here because v79
  has no anchor below it.
- **IDB target hygiene.** `get_metadata` confirms the connected IDB before any
  version-specific probe ([[feedback-verify-ida-target]]).
- **Read-only verification.** During struct verification, do not apply inferred
  struct definitions back into the IDB before reading method bodies (decompiler
  leak), per `docs/version-porting-workflow.md`. IDB *function/global labeling*
  for the memory map is fine and desired; applying speculative *struct types*
  during layout verification is what's prohibited.
- **AV/Themida compatibility.** Patched addresses must be valid for the v79 image
  so the client passes Themida integrity expectations the same way other versions
  do.

## 9. Open Questions

- Are the protocol constants (`VERSION_HEADER`=8, `PLAYER_LOGGED_IN`=0x14,
  `CLIENT_START_ERROR`=0x19) identical in v79, or did the opcode/handler table
  differ in this older build? (Resolve during port; affects `redirect`/`bypass`.)
- For each `>= 83` and `>= 84` floor gate that newly excludes v79: does v79
  actually lack the gated field, or does the field exist further back than v83 and
  the gate floor should be lowered (e.g. `>= 79`)? (Empirical; FR-12b.)
- Which existing branch do v79's `CFuncKeyMappedMan` and `CWvsApp` layouts match —
  the `83/84/87` branch, or do they need their own? (FR-12a.)
- Does any v83-present feature (CFileStream relay helpers, etc.) *not exist* in
  v79, turning a v83 real-address key into a v79 sentinel? (FR-7.)
- Is the v79 IDB a clean retail `MapleStory.exe`, and does the smoke-test client
  match the exact binary the IDB was taken from? (Acceptance; R8.)

## 10. Acceptance Criteria

- [ ] `_build.yml` matrix includes `{ GMS, 79, 1 }` (first entry); PR/snapshot/
      release inherit it with no other workflow edits.
- [ ] `memory_maps/GMS/v79_1.cmake` exists with all 155 keys defined and
      non-empty (CMake key-completeness check passes).
- [ ] Every absolute-address key is backed by a documented signature in
      `signature-catalog.md`; every offset key is re-derived from v79 codegen;
      every sentinel is justified (including any new v79-only sentinels).
- [ ] The v79 IDB has labels applied for all resolved functions/globals and is
      saved.
- [ ] All 24 version-gated `common/*.h` headers have a recorded v79 size/layout
      verdict in `struct-verification.md`.
- [ ] Every enumerated gate with no v79 branch (≥ `CFuncKeyMappedMan.h:38`,
      `CWvsApp.h:97`) is amended so v79 selects a defined, layout-correct branch.
- [ ] Every lower-bound (`>= 83` / `>= 84`) gate that newly excludes v79 is
      verified against v79; floor lowered where v79 has the field, exclusion
      confirmed where it doesn't — with evidence, and validated to keep
      v83/v84/v87/v95/v111/JMS185 correct.
- [ ] GMS 79.1 builds clean Debug + Release locally and in CI.
- [ ] Live GMS v79 client launches to title/login with proxy + core edits, no
      crash/Themida fault; smoke-test result recorded.
</content>
</invoke>
