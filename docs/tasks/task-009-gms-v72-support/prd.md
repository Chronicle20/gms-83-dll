# GMS v72 Support — Product Requirements Document

Version: v1
Status: Draft
Created: 2026-06-26
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

This task adds **GMS v72.1** as a first-class supported target. It is a direct
follow-on to **task-008 (GMS v79)** and uses the same methodology: signature- and
pattern-driven relocation of every memory-map key against the v72 binary, with each
resolved function/global **labeled back into the v72 IDB** via the IDA MCP so the
IDB accrues a usable symbol set as the port proceeds. A v72 IDB is available but
**sparse/unverified** — confirm its identity with `get_metadata` before treating it
as the active probe target ([[feedback-verify-ida-target]]), and treat its existing
label coverage as minimal.

The reusable **signature/pattern catalog** is extended with v72 entries, and — for
the first time — it is *consumed* as well as produced: task-008 left a worked
catalog and a labeled v79 IDB one notch above v72, so many heuristics port directly.

### Anchor strategy

The **primary anchor is GMS v83** (per direction): it carries the most complete,
canonical symbol set, so canonical names and prototypes come from v83. The
**closest anchor is GMS v79** — newly labeled by task-008 and only 7 versions above
v72 — which serves as the immediately-adjacent cross-check and, critically, as the
**below-floor template**: every structural divergence task-008 already discovered
between v79 and the v83 "base" (see §1.1) is the first place to look for v72.
v87 and the v95 PDB remain secondary references for the upper-gate cross-checks.

### 1.1 Why v72 is uniquely risky: it is below the *new* floor, and the floor already moved

task-008 made **v79** the lowest supported version and, in doing so, rewrote the
`common/` gates so that v79 selects defined, layout-correct branches. v72 sits
**below v79**, so it re-triggers every below-floor failure mode one notch lower —
but against a gate landscape that task-008 already reshaped. Three consequences,
and the third is new relative to the v79 port:

1. **Enumerated gates have no v72 branch → lost size guard / silent layout shift.**
   The branches task-008 *added for v79* do not include v72:
   - `common/CWvsApp.h:97` — `(== 79 || == 83 || == 84)` is **false for v72** → no
     branch → the `assert_size(…,0x60)` guard silently does not fire for v72.
   - `common/CFuncKeyMappedMan.h:50` — the `== 79` size-assert branch is **false for
     v72** → no branch.
   - `common/CLogin.h:235` — `== 83` (member-declaring) is false for v72; confirm v72
     lacks the gated member (like v79) rather than silently dropping a member it has.
   These must be amended so v72 selects a defined, **measured** branch (see §4.4 /
   FR-12a).

2. **`>= 83` lower-bound member gates now exclude v72 too.** The two-way splits
   task-008 introduced — `>= 83 || JMS` at `CFuncKeyMappedMan.h:18` (quickslot pair),
   `CMob.h:239` (doom/reserved tail), `CUIToolTip.h:92` (`m_pLayerAdditional`),
   `CWnd.h:25` (secondary-layer com_ptrs), `MobStat.h:128` (Weakness group) — put
   **v72 on the same excluded side as v79**. v72 therefore *inherits v79's reduced
   below-floor branch automatically*. Whether that is *correct* is the central new
   question (point 3).

3. **The "base branch" v72 lands on is now sometimes the v79-reduced branch, not the
   v83 branch — and v72 may diverge from v79.** This is categorically new versus the
   v79 port. For the five structs above, task-008 proved v79 ≠ v83 base (CWnd is −8;
   CMob is 0x518 vs 0x548; MobStat is 0x1F8 vs 0x208). v72 currently rides v79's
   reduced branch. Two outcomes per struct:
   - **v72 == v79** (likely for most): the existing two-way `>= 83 || JMS` gate is
     already correct for v72; record the confirming size and move on.
   - **v72 diverges further** (an even older layout): the two-way gate must become a
     **three-way** split (a v79 branch *and* a distinct v72/`< 79` branch) — a more
     involved rewrite than any task-008 performed. There is **no labeled IDB below
     v72** to triangulate against; v79 is the only nearby anchor and it sits *above*.

This task verifies the layout/size of all 24 version-gated `common/` headers against
the v72 binary, amends every enumerated gate that lacks a v72 branch, and — for every
two-way below-floor gate task-008 created — empirically confirms whether v72 shares
v79's branch or needs its own.

## 2. Goals

Primary goals:
- Add `{ region: GMS, major: 72, minor: 1 }` to the CI build matrix
  (`.github/workflows/_build.yml`, the single source of truth) so it is exercised by
  PR, snapshot, and release workflows. Placed **first** (matrix is version-ascending).
- Produce `memory_maps/GMS/v72_1.cmake` populating **all 159 keys** required by
  `include/memory_map.h.in` (160 `@…@` placeholders minus `BUILD_REGION`; re-pin the
  live count at task start), with each real address located in the v72 binary and
  each feature-absent / region-specific / offset key handled deliberately (see §5, §7).
- Label the v72 IDB in place (via the IDA MCP `rename`/`set_type`) for every
  function/global resolved, so the IDB ends the task substantially more usable than
  it started.
- Verify struct size/layout for **all 24 version-gated `common/*.h` headers** against
  v72; **amend every enumerated gate with no v72 branch**; and for every two-way
  below-floor gate task-008 created, **confirm whether v72 shares v79's branch or
  requires a distinct v72/`< 79` branch**.
- Contribute v72 entries to the reusable signature/pattern catalog, noting which
  task-008 v79 heuristics ported directly and which drifted in the older build.
- Build GMS 72.1 green in both Debug and Release; smoke-test a live GMS v72 client
  with the proxy + core edits loaded, reaching the title/login screen.

Non-goals:
- No new client-edit features. This is a porting task only.
- No re-porting of struct *internals* beyond what is needed to confirm size and the
  version-gated field boundaries.
- No changes to non-GMS regions, and no changes to other GMS versions except where
  the below-floor gate audit forces a (region-correct) gate rewrite — e.g. splitting
  a two-way `>= 83 || JMS` gate into a three-way form that also touches v79's
  compiled branch. Any such change is validated against the full matrix (FR-13).
- Not a goal to reach 100% symbol coverage of the v72 IDB — only the symbols backing
  the 159 keys and the 24 verified structs.

## 3. User Stories

- As a **server operator** running a GMS v72 client, I want prebuilt edit DLLs for my
  exact version so that the proxy and edits load without me reverse engineering
  addresses myself.
- As a **developer** adding the next (possibly even older) GMS version, I want the
  signature catalog and the two-tier below-floor gate audit extended so that I can
  relocate the same functions and lower the floor again quickly.
- As a **maintainer**, I want the v72 IDB labeled and the floor gates verified so that
  the now-two-deep below-floor branch structure (`>= 83` excludes 79 *and* 72) is
  grounded in v72 evidence rather than the assumption "v72 == v79."
- As a **CI consumer**, I want GMS 72.1 in the matrix so that a regression that breaks
  the v72 build is caught on every PR.

## 4. Functional Requirements

### 4.1 Build matrix
- FR-1: `_build.yml` includes `{ region: GMS, major: 72, minor: 1 }` in the
  `strategy.matrix.config` list (placed first to keep the list version-ascending,
  ahead of the existing `{ GMS, 79, 1 }`). No other workflow file needs editing (PR,
  snapshot, and release all consume `_build.yml`).
- FR-2: The new matrix entry builds both Debug and Release (inherited from the
  reusable workflow's two callers) without modification to other entries.

### 4.2 Memory map
- FR-3: `memory_maps/GMS/v72_1.cmake` defines every one of the 159 keys parsed from
  `include/memory_map.h.in`. The build's key-completeness check
  (`cmake/CheckMemoryMapKeys.cmake`) fails hard if any key is undefined or empty, so
  completeness is non-negotiable and CI-enforced. Re-pin the live key count at task
  start (`grep -oE '@[A-Z0-9_]+@' include/memory_map.h.in | sort -u | wc -l`, minus
  `BUILD_REGION`); stop and reconcile if it is not 159.
- FR-4: Each **absolute-address** key points to the v72 function/global that is the
  semantic equivalent of the v83 key, confirmed by a documented signature (§5).
  Prefer relocating from the **v79** label set first (closest), confirming the
  canonical name against **v83**.
- FR-5: Each **instruction-relative offset** key (e.g. `*_OFFSET`,
  `WIN_MAIN_AD_BALLOON_CONDITIONAL`, `WIN_MAIN_PATCHER_OFFSET`,
  `C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET`) is re-derived from the v72 instruction
  layout, not copied from v79 or v83 (instruction-relative offsets shift when codegen
  changes).
- FR-6: Each **protocol constant** key (`VERSION_HEADER`, `PLAYER_LOGGED_IN`,
  `CLIENT_START_ERROR`) is confirmed against v72; if it differs from v79/v83 the v72
  value is used and the difference is noted. v72 is an older protocol revision than
  v79 (whose `VERSION_HEADER` was confirmed 8) — opcode/handler-table drift is *more*
  plausible here, not less. Verify, do not copy. Cross-check against atlas-ms's
  version-aware registries where applicable ([[reference_atlas_ms]]).
- FR-7: Each **feature-absent / region-specific sentinel** (`0x00000000` keys marked
  "does not exist" or "JMS only": `C_BATTLE_RECORD_MAN_CREATE_INSTANCE`, `DR_CHECK`,
  `DR_INIT`, `CE_TRACER_RUN`, the JMS-only security/launcher keys, and any sentinel
  task-008 finalized for v79) is evaluated for v72. **Direction matters:** going
  *backward* from v79, a feature that exists in v79 may be *absent* in v72. Confirm
  each sentinel's v72 disposition, and watch for v79 *non*-sentinel keys whose backing
  feature does not exist in v72 — those become new v72 sentinels and the relevant
  gate/edit must tolerate `0`. Start from the **v79** disposition (task-008 resolved
  several v83 ambiguities, e.g. `RESET_LSP`), not the stale v83 comments.
- FR-8: Where the v79 (or v83) key carries a clarifying comment, the v72 map carries
  an accurate equivalent comment reflecting the v72 finding.
- FR-8a: The exception-dispatch keys (`C_TI_*EXCEPTION`, `C_PATCH_EXCEPTION_BUILDER`,
  `C_COM_RAISE_ERROR_EX`, the `C_FILE_STREAM_*` relay keys) are present in the 159-key
  set and must be resolved or sentinel-justified for v72. Carry forward task-008's v79
  disposition of the CFileStream relay as the starting hypothesis, then confirm.

### 4.3 IDB labeling
- FR-9: For every function/global resolved for a memory-map key, apply the canonical
  name (from v83) to the v72 IDB via the IDA MCP (`rename`, and `set_type` where a
  prototype is known). Before treating the IDB as the active target for any probe,
  confirm it via `get_metadata` ([[feedback-verify-ida-target]]) — the v72 IDB is
  sparse/unverified, so confirming its identity is doubly important.
- FR-10: Labeling is incremental and committed to the IDB (`idb_save`) at reasonable
  checkpoints so progress survives an MCP swap or restart.

### 4.4 Struct verification & below-floor gate audit (two-tier)
- FR-11: For each of the 24 version-gated `common/*.h` headers, determine the v72 size
  and the presence/absence of every version-gated field, anchored to v72 disassembly
  evidence (see `struct-verification.md`).
- FR-12a (**enumerated-gate amendment — highest priority, guard-restoring**): Every
  gate that selects by explicit version equality with **no catch-all `#else`** must be
  amended so v72 selects a defined, measured branch. Known sites (verify the live set
  during the task):
  - `common/CWvsApp.h:97` — `(== 79 || == 83 || == 84)` → v72 has no branch. Add v72
    to the layout-matching branch (decided by v72 disassembly, almost certainly the
    `0x60` branch v79 joined).
  - `common/CFuncKeyMappedMan.h:50` — `== 79` size-assert branch → v72 has no branch.
    Add v72 to the matching size-assert branch (member gate at `:18` already excludes
    v72; confirm the v72 size, then assert it — `== 72` or `== 72 || == 79`).
  - `common/CLogin.h:235` — `== 83` member gate → false for v72 (excluded, like v79);
    confirm v72 genuinely lacks `unk3[5]` rather than silently dropping a member.
- FR-12b (**two-way below-floor gate confirmation — the new tier**): For every
  `>= 83 || JMS` gate task-008 created, v72 inherits v79's reduced branch. Confirm per
  struct whether **v72 == v79** (gate already correct) or **v72 diverges further**
  (gate must become a three-way split with a distinct v72/`< 79` branch). Known sites:
  - `common/CWnd.h:25` — secondary-layer com_ptrs absent for v79 (CWnd 0x64). Confirm
    v72 CWnd size; this base shift propagates to CDialog/CUIWnd/CFadeWnd/CUITitle/
    CUILoginStart, so a v72≠v79 delta here cascades.
  - `common/CMob.h:239` — doom/reserved tail absent for v79 (CMob 0x518). Confirm v72
    CMob size and doom-region disposition (drives `doom-fix`, see FR-12c).
  - `common/MobStat.h:128` — Weakness group absent for v79 (MobStat 0x1F8). Confirm v72.
  - `common/CFuncKeyMappedMan.h:18` — quickslot pair absent for v79 (0x388). Confirm v72.
  - `common/CUIToolTip.h:92` — `m_pLayerAdditional` absent for v79 (0x514). Confirm v72.
- FR-12c (**upper-bound / base-branch spot-check**): Every `>= 84`, `>= 87`, `>= 95`,
  `>= 111`, `== 95`, `== 87`, `== 83`, `> 83`, `< 95`, `< 84` gate resolves v72 onto a
  base/excluded branch (same side as v79). Spot-check the struct *size* of each such
  header against v72; a silent v72≠v79 (or v72≠v83) base delta is the quietest failure
  mode. Includes the `doom-fix` `== 83` write gate (false for v72 → write not applied;
  confirm the field is genuinely absent in v72 so exclusion is correct).
- FR-13: A gate amendment/rewrite must remain correct for all *other* currently-
  supported versions (v79, v83, v84, v87, v95, v111, JMS 185). Three-way splits in
  particular must keep v79's selected branch unchanged. Rewrites are evaluated against
  the full matrix via cross-version truth tables, not just v72.

### 4.5 Signature catalog
- FR-14: The signature/pattern catalog (`signature-catalog.md`) records, per resolved
  function, the heuristic used to find it in v72 and how robust it was across the
  v83→v79→v72 chain — explicitly noting which task-008 v79 heuristics ported directly
  and which string/constant/codegen anchors drifted in the older build.

### 4.6 Build & runtime acceptance
- FR-15: GMS 72.1 configures and builds clean (no errors) for Debug and Release
  locally (`scripts/wsl-build.sh GMS 72 1`) and in CI.
- FR-16: A live GMS v72 client launches with the proxy `ijl15.dll` and the core edits
  deployed, reaches the title/login screen, and the targeted edits behave (no crash,
  no Themida fault) — manual smoke test, results recorded.

## 5. Hook / Patch Surface

This task does not introduce new hooks; it relocates the existing ones for v72. The
patch surface is the full set of 159 keys in `include/memory_map.h.in`, grouped by
subsystem:

- **CWvsApp lifecycle** (`C_WVS_APP_*`): app init/run/setup, subsystem initializers
  (auth, pcom, res-man, gr2d, input, sound, game-data), window creation, directory
  helpers. (Verify `C_WVS_APP_SET_UP` init sequence — see R11.)
- **CClientSocket / ZSocket**: send/flush/process/connect paths used by `redirect` and
  `bypass`.
- **COutPacket**: encode primitives used to craft packets.
- **CLogin / CLogo / CStage / CUITitle** (login & stage flow), **CConfig** (windowed
  mode / sys-opt), **CInputSystem**, **CFuncKeyMappedMan** / quickslot, and the
  manager singletons (`*_CREATE_INSTANCE` / `*_INSTANCE_ADDR`).
- **WinMain** entry + offsets (`WIN_MAIN`, `WIN_MAIN_AD_BALLOON_CONDITIONAL`,
  `WIN_MAIN_PATCHER_OFFSET`, `SEND_HS_LOG`).
- **Party / migrate message senders** with call-site offsets.
- **Misc utilities**: `ZArray::RemoveAll`, `ZXString` trim/get-buffer, fatal section
  ctor/dtor, `CSystemInfo`, `CIGCipher`.
- **Faithful exception-dispatch keys**: `C_TI_*EXCEPTION`, `C_PATCH_EXCEPTION_BUILDER`,
  `C_COM_RAISE_ERROR_EX`, `C_FILE_STREAM_*`.
- **Sentinels** (GMS-absent or JMS-only): see FR-7.

Region/version applicability: GMS v72.1 only. The per-key resolution method and anchor
references are detailed in `memory-map.md`; the identification heuristics are
catalogued in `signature-catalog.md`.

## 6. Configuration

No new INI configuration keys. Existing edits keep their current INI surfaces. The
only "config" change is the build-time `BUILD_MAJOR_VERSION=72` selection, wired
through the matrix and CMake exactly as existing versions.

## 7. Memory Map Impact

A new file `memory_maps/GMS/v72_1.cmake` is required, with all 159 keys. Seed from
`v79_1.cmake` (closest) rather than `v83_1.cmake`, since v79 already carries the
below-floor relocations and sentinel dispositions. Key classes and handling:

| Class | Examples | v72 handling |
|---|---|---|
| Absolute address | `C_WVS_APP_RUN`, `C_CLIENT_SOCKET_SEND_PACKET` | Relocate via signature (from v79 labels); label in v72 IDB |
| Instruction-relative offset | `WIN_MAIN_PATCHER_OFFSET`, `*_MSG_OFFSET` | Re-derive from v72 codegen |
| Protocol constant | `VERSION_HEADER`, `PLAYER_LOGGED_IN` | Confirm against v72 (older protocol — verify, don't copy) |
| GMS-absent sentinel | `DR_CHECK`, `DR_INIT`, `CE_TRACER_RUN`, `C_BATTLE_RECORD_MAN_CREATE_INSTANCE` | Confirm absence (likely also absent older); carry `0x00000000` + comment |
| JMS-only sentinel | `C_SECURITY_CLIENT_ON_PACKET_CHECK*`, `WIN_MAIN_LAUNCHER_STUB`, `C_WVS_APP_INITIALIZE_GR2D_WINDOWED_OFFSET` | Carry `0x00000000` (GMS build) |
| v79-present / v72-absent (new) | any v79 real address whose feature does not exist in v72 | Becomes a v72 sentinel; gate/edit must tolerate `0` |

No changes to other regions' maps. The 24 `common/*.h` gate audit (§4.4) may modify
shared headers and the floor-gate call sites; those changes are source, not
memory-map. Source changes that add v72 to an enumerated gate (FR-12a) or split a
two-way gate into three-way (FR-12b) touch shared headers but must leave every other
version's selected branch unchanged (FR-13).

## 8. Non-Functional Requirements

- **CI completeness enforcement.** The build must fail loudly if any v72 key is
  missing — no silently-empty addresses.
- **No regressions to existing versions.** v79/v83/v84/v87/v95/v111/JMS185 builds must
  remain green; any gate amendment is validated against all of them (FR-13). Three-way
  gate splits must preserve v79's branch exactly.
- **Evidence discipline.** Every address and every gate verdict is anchored to a
  concrete v72 disassembly observation ([[feedback-prefer-confirmation]]). "Same as
  v79" is acceptable only when a signature confirms it — load-bearing here because v72
  has no anchor below it and v79 itself already diverged from the v83 base.
- **IDB target hygiene.** `get_metadata` confirms the connected IDB before any
  version-specific probe ([[feedback-verify-ida-target]]); the v72 IDB is
  sparse/unverified, so do not infer its identity from conversation.
- **Read-only verification.** During struct verification, do not apply inferred struct
  definitions back into the IDB before reading method bodies (decompiler leak), per
  `docs/version-porting-workflow.md`. IDB function/global labeling for the memory map
  is fine and desired; applying speculative *struct types* during layout verification
  is what's prohibited.
- **AV/Themida compatibility.** Patched addresses must be valid for the v72 image so
  the client passes Themida integrity expectations the same way other versions do.

## 9. Open Questions

- Are the protocol constants identical in v72, or did the opcode/handler table differ
  in this older build? v79 confirmed `VERSION_HEADER`=8; v72 is older still — verify
  all three constants. (Resolve during port; affects `redirect`/`bypass`.)
- For each two-way `>= 83 || JMS` gate task-008 created: does v72 share v79's reduced
  layout (gate already correct), or diverge further and require a three-way split?
  (Empirical; FR-12b. Highest-novelty risk — see R3.)
- Does the CWnd −8 base shift hold identically at v72, or does v72's CWnd shrink
  further? A v72≠v79 CWnd cascades through every CWnd-derived UI class. (FR-12b.)
- Which existing branch do v72's `CWvsApp` and `CFuncKeyMappedMan` layouts match — the
  v79 below-floor branch, or do they need their own? (FR-12a.)
- Does any v79-present feature (CFileStream relay helpers, etc.) *not exist* in v72,
  turning a v79 real-address key into a v72 sentinel? (FR-7.)
- Is the v72 IDB a clean retail `MapleStory.exe`, and does the smoke-test client match
  the exact binary the IDB was taken from? (Acceptance; R8/R12.)

## 10. Acceptance Criteria

- [ ] `_build.yml` matrix includes `{ GMS, 72, 1 }` (first entry); PR/snapshot/release
      inherit it with no other workflow edits.
- [ ] `memory_maps/GMS/v72_1.cmake` exists with all 159 keys defined and non-empty
      (CMake key-completeness check passes).
- [ ] Every absolute-address key is backed by a documented signature in
      `signature-catalog.md`; every offset key is re-derived from v72 codegen; every
      sentinel is justified (including any new v72-only sentinels).
- [ ] The v72 IDB has labels applied for all resolved functions/globals and is saved.
- [ ] All 24 version-gated `common/*.h` headers have a recorded v72 size/layout verdict
      in `struct-verification.md`.
- [ ] Every enumerated gate with no v72 branch (≥ `CWvsApp.h:97`,
      `CFuncKeyMappedMan.h:50`, `CLogin.h:235`) is amended so v72 selects a defined,
      layout-correct, size-asserted branch.
- [ ] Every two-way `>= 83 || JMS` below-floor gate is confirmed against v72: either
      v72 shares v79's branch (recorded with evidence) or a three-way split is made and
      validated to keep v79/v83/v84/v87/v95/v111/JMS185 correct.
- [ ] GMS 72.1 builds clean Debug + Release locally and in CI.
- [ ] Live GMS v72 client launches to title/login with proxy + core edits, no
      crash/Themida fault; smoke-test result recorded.
</content>
</invoke>
