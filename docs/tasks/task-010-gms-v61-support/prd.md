# GMS v61 Support — Product Requirements Document

Version: v1
Status: Draft
Created: 2026-06-27
---

## 1. Overview

This DLL collection is built per region+version: the build matrix selects a
`memory_maps/<REGION>/v<MAJOR>_<MINOR>.cmake` file, which supplies every absolute
address, opcode, and instruction-relative offset the edits patch into the client.
Source headers in `common/` describe client struct layouts and gate
version-specific deltas behind `#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION …)`
blocks. To support a new client build, three things must come together: the CI
matrix must include it, a complete memory map must exist for it, and the gated
struct layouts must actually match that client.

This task adds **GMS v61.1** as a first-class supported target. It is a direct
follow-on to **task-008 (GMS v79)** and **task-009 (GMS v72)** and uses the same
methodology: signature- and pattern-driven relocation of every memory-map key
against the v61 binary, with each resolved function/global **labeled back into the
v61 IDB** via the IDA MCP so the IDB accrues a usable symbol set as the port
proceeds. A v61 IDB is available and reachable (`GMS_v61.1_U_DEVM.exe`) but
**sparse/unverified** — confirm its identity with `get_metadata` before treating it
as the active probe target ([[feedback_verify_ida_target]]), and treat its existing
label coverage as minimal.

The reusable **signature/pattern catalog** is extended with v61 entries. As with
v72, the catalog is *consumed* as well as produced: task-008 and task-009 left a
worked catalog and labeled v79 + v72 IDBs immediately above v61, so many heuristics
port directly down one more notch.

### Baseline / anchor strategy

Per direction, **this task branches on the task-009 (v72) baseline** — it builds
atop v72's floor-lowering gate amendments rather than re-deriving them from `main`.
The anchor chain is therefore:

- **Primary (canonical) anchor: GMS v83** — most complete, canonical symbol set;
  canonical names and prototypes come from v83.
- **Closest anchor: GMS v72** — newly labeled by task-009, only 11 versions above
  v61, and the **immediately-adjacent below-floor template**. Every structural
  divergence task-009 already discovered between v72 and the v79/v83 base is the
  first place to look for v61.
- **Secondary below-floor cross-check: GMS v79** (task-008), one tier up from v72.
- **Upper references: GMS v87 and the v95 PDB**, for the upper-gate cross-checks.

A **GMS v48 IDB is also loaded but unlabeled** — it is *not* a usable anchor below
v61 and must not be relied on for triangulation; it is noted only so it is not
mistaken for a labeled reference.

### 1.1 Why v61 is uniquely risky: it is below the *new* floor, two tiers deep

task-008 made v79 the lowest supported version; task-009 then lowered the floor
again to v72, rewriting the `common/` gates so that v72 selects defined,
layout-correct branches. v61 sits **below v72**, so it re-triggers every
below-floor failure mode one tier lower still — against a gate landscape that two
prior tasks have already reshaped. Three consequences, mirroring the v72 port but
amplified:

1. **Enumerated gates have no v61 branch → lost size guard / silent layout shift.**
   Any gate that selects by explicit version equality with no catch-all `#else`
   (e.g. the `CWvsApp` size-assert, `CFuncKeyMappedMan` size-assert, `CLogin`
   member gate) will, after task-009, enumerate `79`/`72`/`83`/`84`… but **not
   `61`**. Where v61 has no branch the size guard silently does not fire and/or the
   member is dropped. These must be amended so v61 selects a defined, **measured**
   branch (see §4.4 / FR-12a). Re-pin the exact live gate set at task start against
   the task-009 baseline.

2. **`>= 83` / `< 79` lower-bound member gates now exclude v61 too.** The two-way
   (and, where task-009 split them, three-way) below-floor gates put **v61 on the
   same excluded/reduced side as v72**. v61 therefore *inherits v72's reduced
   below-floor branch automatically*. Whether that is *correct* is the central
   question (point 3).

3. **The branch v61 lands on may be the v72-reduced branch — and v61 may diverge
   from v72.** For each struct where task-009 proved v72 ≠ v79 (or created a
   distinct v72/`< 79` branch), v61 currently rides v72's branch. Two outcomes per
   struct:
   - **v61 == v72** (likely for most): the existing gate is already correct for
     v61; record the confirming size and move on.
   - **v61 diverges further** (an even older layout): the gate must gain a distinct
     v61/`< 72` branch — potentially turning a task-009 three-way split into a
     four-way one. There is **no labeled IDB below v61** to triangulate against (v48
     is unlabeled); v72 is the only nearby anchor and it sits *above*.

This task verifies the layout/size of all **24** version-gated `common/` headers
against the v61 binary, amends every enumerated gate that lacks a v61 branch, and —
for every below-floor gate task-008/task-009 created — empirically confirms whether
v61 shares v72's branch or needs its own.

## 2. Goals

Primary goals:
- Add `{ region: GMS, major: 61, minor: 1 }` to the CI build matrix
  (`.github/workflows/_build.yml`, the single source of truth) so it is exercised by
  PR, snapshot, and release workflows. Placed **first** (matrix is version-ascending,
  ahead of the existing `{ GMS, 72, 1 }`).
- Produce `memory_maps/GMS/v61_1.cmake` populating **all 159 keys** required by
  `include/memory_map.h.in` (160 `@…@` placeholders minus `BUILD_REGION`; re-pin the
  live count at task start), with each real address located in the v61 binary and
  each feature-absent / region-specific / offset key handled deliberately (see §5, §7).
- Label the v61 IDB in place (via the IDA MCP `rename`/`set_type`) for every
  function/global resolved, so the IDB ends the task substantially more usable than
  it started.
- Verify struct size/layout for **all 24 version-gated `common/*.h` headers** against
  v61; **amend every enumerated gate with no v61 branch**; and for every below-floor
  gate task-008/task-009 created, **confirm whether v61 shares v72's branch or
  requires a distinct v61/`< 72` branch**.
- Contribute v61 entries to the reusable signature/pattern catalog, noting which
  task-009 v72 heuristics ported directly and which drifted in this older build.
- Build GMS 61.1 green in both Debug and Release; smoke-test a live GMS v61 client
  with the proxy + core edits loaded, reaching the title/login screen.

Non-goals:
- No new client-edit features. This is a porting task only.
- No re-porting of struct *internals* beyond what is needed to confirm size and the
  version-gated field boundaries.
- No changes to non-GMS regions, and no changes to other GMS versions except where
  the below-floor gate audit forces a (region-correct) gate rewrite — e.g. splitting
  a task-009 three-way gate into a four-way form that also touches v72/v79's compiled
  branch. Any such change is validated against the full matrix (FR-13).
- Not a goal to reach 100% symbol coverage of the v61 IDB — only the symbols backing
  the 159 keys and the 24 verified structs.
- Not a goal to label or port the v48 IDB (it is referenced only to avoid mistaking
  it for an anchor).

## 3. User Stories

- As a **server operator** running a GMS v61 client, I want prebuilt edit DLLs for my
  exact version so that the proxy and edits load without me reverse engineering
  addresses myself.
- As a **developer** adding the next (possibly even older) GMS version, I want the
  signature catalog and the multi-tier below-floor gate audit extended so that I can
  relocate the same functions and lower the floor again quickly.
- As a **maintainer**, I want the v61 IDB labeled and the floor gates verified so that
  the now-three-deep below-floor branch structure (`>= 83` excludes 79, 72 *and* 61)
  is grounded in v61 evidence rather than the assumption "v61 == v72."
- As a **CI consumer**, I want GMS 61.1 in the matrix so that a regression that breaks
  the v61 build is caught on every PR.

## 4. Functional Requirements

### 4.1 Build matrix
- FR-1: `_build.yml` includes `{ region: GMS, major: 61, minor: 1 }` in the
  `strategy.matrix.config` list (placed first to keep the list version-ascending,
  ahead of the existing `{ GMS, 72, 1 }`). No other workflow file needs editing (PR,
  snapshot, and release all consume `_build.yml`).
- FR-2: The new matrix entry builds both Debug and Release (inherited from the
  reusable workflow's two callers) without modification to other entries.

### 4.2 Memory map
- FR-3: `memory_maps/GMS/v61_1.cmake` defines every one of the 159 keys parsed from
  `include/memory_map.h.in`. The build's key-completeness check
  (`cmake/CheckMemoryMapKeys.cmake`) fails hard if any key is undefined or empty, so
  completeness is non-negotiable and CI-enforced. Re-pin the live key count at task
  start (`grep -oE '@[A-Z0-9_]+@' include/memory_map.h.in | sort -u | wc -l`, minus
  `BUILD_REGION`); stop and reconcile if it is not 159.
- FR-4: Each **absolute-address** key points to the v61 function/global that is the
  semantic equivalent of the v83 key, confirmed by a documented signature (§5).
  Prefer relocating from the **v72** label set first (closest), confirming the
  canonical name against **v83**.
- FR-5: Each **instruction-relative offset** key (e.g. `*_OFFSET`,
  `WIN_MAIN_AD_BALLOON_CONDITIONAL`, `WIN_MAIN_PATCHER_OFFSET`,
  `C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET`) is re-derived from the v61 instruction
  layout, not copied from v72 or v83 (instruction-relative offsets shift when codegen
  changes).
- FR-6: Each **protocol constant** key (`VERSION_HEADER`, `PLAYER_LOGGED_IN`,
  `CLIENT_START_ERROR`) is confirmed against v61; if it differs from v72/v83 the v61
  value is used and the difference is noted. v61 is an older protocol revision than
  v72 — opcode/handler-table drift is *more* plausible here, not less. Verify, do not
  copy. Cross-check against atlas-ms's version-aware registries where applicable
  ([[reference_atlas_ms]]).
- FR-7: Each **feature-absent / region-specific sentinel** (`0x00000000` keys marked
  "does not exist" or "JMS only": `C_BATTLE_RECORD_MAN_CREATE_INSTANCE`, `DR_CHECK`,
  `DR_INIT`, `CE_TRACER_RUN`, the JMS-only security/launcher keys, and any sentinel
  task-008/task-009 finalized) is evaluated for v61. **Direction matters:** going
  *backward* from v72, a feature that exists in v72 may be *absent* in v61. Confirm
  each sentinel's v61 disposition, and watch for v72 *non*-sentinel keys whose backing
  feature does not exist in v61 — those become new v61 sentinels and the relevant
  gate/edit must tolerate `0`. Start from the **v72** disposition (task-009 resolved
  several below-floor ambiguities), not the stale v83 comments. **This is the chosen
  policy: investigate each candidate and sentinel deliberately — never carry a v72
  address forward to v61 without confirming the feature exists.**
- FR-8: Where the v72 (or v83) key carries a clarifying comment, the v61 map carries
  an accurate equivalent comment reflecting the v61 finding.
- FR-8a: The exception-dispatch keys (`C_TI_*EXCEPTION`, `C_PATCH_EXCEPTION_BUILDER`,
  `C_COM_RAISE_ERROR_EX`, the `C_FILE_STREAM_*` relay keys) are present in the 159-key
  set and must be resolved or sentinel-justified for v61. Carry forward task-009's v72
  disposition of the CFileStream relay as the starting hypothesis, then confirm.

### 4.3 IDB labeling
- FR-9: For every function/global resolved for a memory-map key, apply the canonical
  name (from v83) to the v61 IDB via the IDA MCP (`rename`, and `set_type` where a
  prototype is known). Before treating the IDB as the active target for any probe,
  confirm it via `get_metadata` ([[feedback_verify_ida_target]]) — the v61 IDB is
  sparse/unverified, so confirming its identity is doubly important (and the v48 IDB
  is also loaded, raising the chance of probing the wrong target).
- FR-10: Labeling is incremental and committed to the IDB (`idb_save`) at reasonable
  checkpoints so progress survives an MCP swap or restart.

### 4.4 Struct verification & below-floor gate audit (multi-tier)
- FR-11: For each of the 24 version-gated `common/*.h` headers, determine the v61 size
  and the presence/absence of every version-gated field, anchored to v61 disassembly
  evidence (see `struct-verification.md`).
- FR-12a (**enumerated-gate amendment — highest priority, guard-restoring**): Every
  gate that selects by explicit version equality with **no catch-all `#else`** must be
  amended so v61 selects a defined, measured branch. Re-pin the exact live set against
  the task-009 baseline; expected sites (verify during the task):
  - `common/CWvsApp.h` — the size-assert version-equality branch enumerates the
    supported versions; v61 has no branch. Add v61 to the layout-matching branch
    (decided by v61 disassembly, likely the same branch v72/v79 joined).
  - `common/CFuncKeyMappedMan.h` — the size-assert branch enumerates `… == 72 == 79`;
    v61 has no branch. Confirm the v61 size, then assert it.
  - `common/CLogin.h` — the `== 83` member gate is false for v61 (excluded, like
    v72/v79); confirm v61 genuinely lacks the gated member rather than silently
    dropping a member it has.
- FR-12b (**below-floor gate confirmation — the recurring tier**): For every
  `>= 83 || JMS` (or task-009 three-way) below-floor gate, v61 inherits v72's reduced
  branch. Confirm per struct whether **v61 == v72** (gate already correct) or **v61
  diverges further** (gate must gain a distinct v61/`< 72` branch — potentially a
  four-way split). Expected sites (verify the live set):
  - `common/CWnd.h` — secondary-layer com_ptrs absent below floor; CWnd base shift
    propagates to CDialog/CUIWnd/CFadeWnd/CUITitle/CUILoginStart, so a v61≠v72 delta
    here cascades. Confirm v61 CWnd size first.
  - `common/CMob.h` — doom/reserved tail disposition (drives `doom-fix`, see FR-12c).
    Confirm v61 CMob size and doom-region disposition.
  - `common/MobStat.h` — Weakness group disposition. Confirm v61.
  - `common/CFuncKeyMappedMan.h` — quickslot pair member gate. Confirm v61.
  - `common/CUIToolTip.h` — `m_pLayerAdditional` disposition. Confirm v61.
  - `common/SecondaryStat.h`, `common/PartyData.h`, `common/PartyMember.h`,
    `common/GuildData.h`, `common/MobStat.h` — bitmask/array-indexed structs: confirm
    against v61 and cross-check with atlas-ms ([[reference_atlas_ms]]).
- FR-12c (**upper-bound / base-branch spot-check**): Every `>= 84`, `>= 87`, `>= 95`,
  `>= 111`, `== 95`, `== 87`, `== 83`, `> 83`, `< 95`, `< 84`, `< 79`, `< 72` gate
  resolves v61 onto a base/excluded branch (same side as v72). Spot-check the struct
  *size* of each such header against v61; a silent v61≠v72 (or v61≠v83) base delta is
  the quietest failure mode. Includes the `doom-fix` `== 83` write gate (false for v61
  → write not applied; confirm the field is genuinely absent in v61 so exclusion is
  correct).
- FR-13: A gate amendment/rewrite must remain correct for all *other* currently-
  supported versions (v72, v79, v83, v84, v87, v95, v111, JMS 185). Four-way splits in
  particular must keep v72's and v79's selected branches unchanged. Rewrites are
  evaluated against the full matrix via cross-version truth tables, not just v61.

### 4.5 Signature catalog
- FR-14: The signature/pattern catalog (`signature-catalog.md`) records, per resolved
  function, the heuristic used to find it in v61 and how robust it was across the
  v83→v72→v61 chain — explicitly noting which task-009 v72 heuristics ported directly
  and which string/constant/codegen anchors drifted in the older build.

### 4.6 Build & runtime acceptance
- FR-15: GMS 61.1 configures and builds clean (no errors) for Debug and Release
  locally (`scripts/wsl-build.sh GMS 61 1`) and in CI.
- FR-16: A live GMS v61 client launches with the proxy `ijl15.dll` and the core edits
  deployed, reaches the title/login screen, and the targeted edits behave (no crash,
  no Themida fault) — manual smoke test, results recorded. (Chosen acceptance bar:
  build-green **plus** live smoke test.)

## 5. Hook / Patch Surface

This task does not introduce new hooks; it relocates the existing ones for v61. The
patch surface is the full set of 159 keys in `include/memory_map.h.in`, grouped by
subsystem:

- **CWvsApp lifecycle** (`C_WVS_APP_*`): app init/run/setup, subsystem initializers
  (auth, pcom, res-man, gr2d, input, sound, game-data), window creation, directory
  helpers. (Verify `C_WVS_APP_SET_UP` init sequence — see R11; DR_init is GMS-absent
  for this era, confirm.)
- **CClientSocket / ZSocket**: send/flush/process/connect paths used by `redirect` and
  `bypass`.
- **COutPacket**: encode primitives used to craft packets.
- **CLogin / CLogo / CStage / CUITitle** (login & stage flow), **CConfig** (windowed
  mode / sys-opt), **CInputSystem**, **CFuncKeyMappedMan** / quickslot, and the
  manager singletons (`*_CREATE_INSTANCE` / `*_INSTANCE_ADDR`).
- **WinMain** entry + offsets (`WIN_MAIN`, `WIN_MAIN_AD_BALLOON_CONDITIONAL`,
  `WIN_MAIN_PATCHER_OFFSET`, `SEND_HS_LOG`). Several of these back era-specific
  features (ad balloon, patcher) that may not exist in v61 — investigate and sentinel
  deliberately per FR-7.
- **Party / migrate message senders** with call-site offsets.
- **Misc utilities**: `ZArray::RemoveAll`, `ZXString` trim/get-buffer, fatal section
  ctor/dtor, `CSystemInfo`, `CIGCipher`.
- **Faithful exception-dispatch keys**: `C_TI_*EXCEPTION`, `C_PATCH_EXCEPTION_BUILDER`,
  `C_COM_RAISE_ERROR_EX`, `C_FILE_STREAM_*`.
- **Sentinels** (GMS-absent or JMS-only): see FR-7.

Region/version applicability: GMS v61.1 only. The per-key resolution method and anchor
references are detailed in `memory-map.md`; the identification heuristics are
catalogued in `signature-catalog.md`.

## 6. Configuration

No new INI configuration keys. Existing edits keep their current INI surfaces. The
only "config" change is the build-time `BUILD_MAJOR_VERSION=61` selection, wired
through the matrix and CMake exactly as existing versions.

## 7. Memory Map Impact

A new file `memory_maps/GMS/v61_1.cmake` is required, with all 159 keys. Seed from
`v72_1.cmake` (closest) rather than `v83_1.cmake`, since v72 already carries the
below-floor relocations and sentinel dispositions one tier above v61. Key classes and
handling:

| Class | Examples | v61 handling |
|---|---|---|
| Absolute address | `C_WVS_APP_RUN`, `C_CLIENT_SOCKET_SEND_PACKET` | Relocate via signature (from v72 labels); label in v61 IDB |
| Instruction-relative offset | `WIN_MAIN_PATCHER_OFFSET`, `*_MSG_OFFSET` | Re-derive from v61 codegen |
| Protocol constant | `VERSION_HEADER`, `PLAYER_LOGGED_IN` | Confirm against v61 (older protocol — verify, don't copy) |
| GMS-absent sentinel | `DR_CHECK`, `DR_INIT`, `CE_TRACER_RUN`, `C_BATTLE_RECORD_MAN_CREATE_INSTANCE` | Confirm absence (likely also absent older); carry `0x00000000` + comment |
| JMS-only sentinel | `C_SECURITY_CLIENT_ON_PACKET_CHECK*`, `WIN_MAIN_LAUNCHER_STUB`, `C_WVS_APP_INITIALIZE_GR2D_WINDOWED_OFFSET` | Carry `0x00000000` (GMS build) |
| v72-present / v61-absent (new) | any v72 real address whose feature does not exist in v61 (e.g. ad balloon, patcher, MTS) | Becomes a v61 sentinel; gate/edit must tolerate `0` |

No changes to other regions' maps. The 24 `common/*.h` gate audit (§4.4) may modify
shared headers and the floor-gate call sites; those changes are source, not
memory-map. Source changes that add v61 to an enumerated gate (FR-12a) or split a
below-floor gate to add a distinct v61 branch (FR-12b) touch shared headers but must
leave every other version's selected branch unchanged (FR-13).

## 8. Non-Functional Requirements

- **CI completeness enforcement.** The build must fail loudly if any v61 key is
  missing — no silently-empty addresses.
- **No regressions to existing versions.** v72/v79/v83/v84/v87/v95/v111/JMS185 builds
  must remain green; any gate amendment is validated against all of them (FR-13).
  Four-way gate splits must preserve v72's and v79's branches exactly.
- **Evidence discipline.** Every address and every gate verdict is anchored to a
  concrete v61 disassembly observation ([[feedback_prefer_confirmation]]). "Same as
  v72" is acceptable only when a signature confirms it — load-bearing here because v61
  has no labeled anchor below it and v72 itself already diverged from the v83 base.
- **IDB target hygiene.** `get_metadata` confirms the connected IDB before any
  version-specific probe ([[feedback_verify_ida_target]]); the v61 IDB is
  sparse/unverified and a v48 IDB is also loaded, so do not infer the active target
  from conversation.
- **Read-only verification.** During struct verification, do not apply inferred struct
  definitions back into the IDB before reading method bodies (decompiler leak), per
  `docs/version-porting-workflow.md`. IDB function/global labeling for the memory map
  is fine and desired; applying speculative *struct types* during layout verification
  is what's prohibited.
- **AV/Themida compatibility.** Patched addresses must be valid for the v61 image so
  the client passes Themida integrity expectations the same way other versions do. If
  v61 predates Themida packing on GMS, record that finding (it changes the integrity
  risk profile) — see R10.

## 9. Open Questions

- Are the protocol constants identical in v61, or did the opcode/handler table differ
  in this older build? v72/v79 confirmed `VERSION_HEADER`=8; v61 is older still —
  verify all three constants. (Resolve during port; affects `redirect`/`bypass`.)
- For each below-floor gate task-008/task-009 created: does v61 share v72's reduced
  layout (gate already correct), or diverge further and require a distinct v61/`< 72`
  branch (a four-way split)? (Empirical; FR-12b. Highest-novelty risk — see R3.)
- Does the CWnd base shift hold identically at v61, or does v61's CWnd shrink further?
  A v61≠v72 CWnd cascades through every CWnd-derived UI class. (FR-12b.)
- Which existing branch do v61's `CWvsApp` and `CFuncKeyMappedMan` layouts match — the
  v72 below-floor branch, or do they need their own? (FR-12a.)
- Which v72-present features *do not exist* in v61 (ad balloon, patcher window, MTS map
  restriction, beginner-party block, CFileStream relay helpers, etc.), turning a v72
  real-address key into a v61 sentinel and possibly making an edit a no-op? (FR-7.)
- Does GMS v61 ship Themida-packed, or is this pre-Themida? Changes the integrity/AV
  risk profile and the smoke-test expectations. (R10.)
- Is the v61 IDB a clean retail `MapleStory.exe`, and does the smoke-test client match
  the exact binary the IDB was taken from? (Acceptance; R8/R12.)

## 10. Acceptance Criteria

- [ ] `_build.yml` matrix includes `{ GMS, 61, 1 }` (first entry); PR/snapshot/release
      inherit it with no other workflow edits.
- [ ] `memory_maps/GMS/v61_1.cmake` exists with all 159 keys defined and non-empty
      (CMake key-completeness check passes).
- [ ] Every absolute-address key is backed by a documented signature in
      `signature-catalog.md`; every offset key is re-derived from v61 codegen; every
      sentinel is justified (including any new v61-only sentinels for era-absent
      features).
- [ ] The v61 IDB has labels applied for all resolved functions/globals and is saved.
- [ ] All 24 version-gated `common/*.h` headers have a recorded v61 size/layout verdict
      in `struct-verification.md`.
- [ ] Every enumerated gate with no v61 branch (CWvsApp / CFuncKeyMappedMan / CLogin
      size-assert & member gates) is amended so v61 selects a defined, layout-correct,
      size-asserted branch.
- [ ] Every below-floor gate is confirmed against v61: either v61 shares v72's branch
      (recorded with evidence) or a distinct v61 branch is made and validated to keep
      v72/v79/v83/v84/v87/v95/v111/JMS185 correct.
- [ ] GMS 61.1 builds clean Debug + Release locally and in CI.
- [ ] Live GMS v61 client launches to title/login with proxy + core edits, no
      crash/Themida fault; smoke-test result recorded.
