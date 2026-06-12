# Playbook: Porting a Client Version & Debugging the In-Field Freeze

Two exercises recur in this repo and are likely to repeat:

1. **Port the bypass to a new client version** (a new `BUILD_MAJOR_VERSION` in `GMS`/`JMS`).
2. **Diagnose the "freezes/crashes a few seconds after entering a field" class of bug** — the
   hardest part, because it usually means our reimplementation of a fully-replaced client
   function dropped something load-bearing.

This is the consolidated method + the pain points, distilled from the GMS v84.1 port
(task-006). For the per-task narrative see `docs/tasks/<task>/`.

---

## Part 1 — Version-port workflow

1. **Scaffold the memory map.** Seed `memory_maps/<REGION>/v<N>_<m>.cmake` from the nearest
   existing version, then relocate each cluster against the target IDB using signatures
   (string/import/opcode/call-graph anchors), not blind offset arithmetic.
2. **Completeness gate.** Every `@KEY@` in `include/memory_map.h.in` must be `set(...)` in
   **all** version maps (currently GMS v83/v84/v87/v95/v111 + JMS v185).
   `cmake/CheckMemoryMapKeys.cmake` enforces it. Adding one key ⇒ edit the template **and**
   every map. Where a function genuinely doesn't exist on a version, set `0x00000000` with a
   `# does not exist` / reason comment.
3. **Struct audit + the assert net** (`common/*.h`). Determine each struct's **real
   per-version size** from the IDB (a "size sweep": read the `Alloc(sizeof(X))` immediate or
   the ctor's writes), then gate our layout:
   - **Allocated structs** (`Alloc(sizeof(X))` then the real ctor runs into the block):
     assert `sizeof(X) >= real_size` to prevent a heap overrun. Exact layout only matters for
     members **we** read/write; otherwise a version-gated trailing pad to reach the size is
     correct and sufficient (e.g. `CConfig` v111 → `char m_v111Pad[...]`).
   - **memcpy'd / exact-layout structs** (e.g. `CFuncKeyMappedMan`, where hooks `memcpy` into
     member arrays): `assert_size(..., ==)`. Member offsets must match exactly.
   - Prefer adding a real field where you can name it; use `dummyN` only when the field's
     purpose is unknown but its size/position is confirmed.
4. **CI matrix.** Add the version to `.github/workflows/_build.yml` `matrix.config`. Builds run
   on **`windows-2022`**, `Visual Studio 17` generator, `-A Win32`, via the reusable workflow
   (`build-snapshot.yml` / `pull-request.yml` call `_build.yml`).

A version can be *map-scaffolded* (configures, has a CI entry) yet **not actually ported**
(structs wrong). The assert net is what surfaces that — don't weaken an assert to make a build
green; that re-hides a real heap-overflow.

---

## Part 2 — Debugging the in-field freeze (the `DR_init` class)

**Symptom shape:** works on some versions, freezes or crashes ~2-3s after entering *any* field
(even a map with no monsters), frequently a call-through-null → `EIP=0, EAX=0, EDX=0`.

**Method (evidence first — do not guess fixes):**

1. **Capture the fault in x32dbg.** Pause at the freeze. A `call`-through-null pushes the
   return address *before* faulting, so **`[ESP]` (top of the Stack pane) is the return
   address of the faulting call** — the instruction right after the bad `call`. Read it from
   the Stack pane (x32dbg auto-annotates `return to module.addr`). Command-box equivalent is
   `dump esp` — **not** `dd esp` (that's WinDbg syntax and does nothing in x32dbg).
2. **Resolve that address in the matching IDB.** If it lands above the normal `.text` range and
   disassembles to garbage (`out`/`iret`/`les`/`arpl`/random `add [eax],al`), it's a
   **Themida-encrypted/virtualized** region — the binary is packed. Switch to the **depacked
   repack** (e.g. a localhost `..._L.exe`) where the same code is readable, or read the
   address in a `_dump`/`DEVM` IDB.
3. **Bisect the hooks.** Comment out hook groups one at a time (best on the depacked client)
   until the freeze stops. This localizes which installer/function is responsible. (v84 pinned
   to `CWvsApp::SetUp` this way.)
4. **Diff our reimplementation against the *real* per-version disassembly.** When a hook
   **fully replaces** a client function (`SetUp`, `Run`, `CallUpdate`), it must account for
   **every** call the original makes. Some calls that look like skippable anti-tamper actually
   **initialize a pointer the game later calls through.** Canonical case: `DR_init` resolves
   `NtGetContextThread` into a global; the in-field DR anti-debug check (reached from
   `CVecCtrlUser::EndUpdateActive` — *player* movement) executes `call (k ^ that_global)`. Drop
   `DR_init` → the global is 0 → `call(0)` → `EIP=0`. That single omission produced every
   symptom (in-field, ~2-3s, player-not-mob, packed-vs-readable PC).
5. **Reason across versions.** A version that *works* tells you why. v87+ survive the same
   omission only because we **hook `DR_check` → return 0**, short-circuiting before the null
   call; v84 neither hooked `DR_check` nor ran `DR_init`.

**Replicate vs. skip — the rule of thumb for replaced init functions:**

- **DO replicate** benign state-setup the clean client does: e.g. `DR_init` (resolves
  `NtGetContextThread`; no `while(1)`, no throw, no network). Wire it through the memory map and
  call it where the clean client does.
- **DON'T call** anti-cheat bring-up: e.g. `CSecurityClient_Init` / its sibling — they start
  AhnLab HackShield/EHSvc and **throw** when it's absent. The clean `Run`/`CallUpdate` are also
  walls of `while(1)` anti-tamper (`getpeername` IP-table validation, PE-section checksum, code
  re-verify); our rewrites correctly omit those.

**Rule out the server first.** A field "freeze" can be server-side: atlas-channel emitting an
unmapped movement code that the client can't dispatch → client crash. Confirm whether a *stock*
client (no bypass) reproduces it before blaming our code.

---

## Part 3 — Recurring gotchas / pain points

- **IDA MCP `select_instance` is global shared state.** Serialize switches and restore to your
  target instance when done. `list_instances` to map ports → versions (during task-006:
  13337=v83, 13338=v87, 13339=v95, 13340=JMS185, 13341=v84, 13342=v111).
- **Sibling IDBs are often *bypassed dumps*.** Their `SetUp` may no longer call `DR_init` (it
  was stripped), but the **function still exists** — populate it for every version; clean
  clients (what we inject into) *do* call it.
- **clang-format CI checks only PR-introduced lines.** Reproduce exactly with
  `git clang-format <merge-base>` — *not* a whole-file format, which churns pre-existing style
  the repo deliberately leaves alone (`style: apply clang-format to PR-introduced lines`).
- **`runs-on: windows-latest` is a trap.** Its migration to `windows-2025` breaks CMake's
  Visual Studio detection (`could not find any instance of Visual Studio`) at the *configure*
  step, failing every matrix build before a line compiles. Pin **`windows-2022`**.
- **Squash-merge ⇒ `git branch -d` refuses** (branch SHAs aren't ancestors of main). Verify the
  work landed (`git log origin/main`), then `git branch -D`. The remote branch usually
  auto-deletes on merge.
- **Leftover untracked files block `git pull --ff-only`** (e.g. stale `docs/tasks/<task>/`
  copies in the main checkout). Compare them against the merged versions first, back up any
  local-only/divergent file, then `git clean -fd <path>` and pull.
- **Never commit to `main`.** Branch + PR. Worktrees live under `.claude/worktrees/`; remove
  with `git worktree remove` + `git worktree prune` from the main repo.

---

## Part 4 — Cross-validation oracles

- **atlas-ms** (sibling Go server, `~/source/atlas-ms/atlas/`) — version-aware registries and
  the movement/attack handlers. Consult it when porting bitmask-indexed structs and when
  deciding whether a "freeze" is actually server-side.
- **v87 / v95 reference IDBs + the v95 PDB** — best-symbolized layout references; v84 tracks
  v87/v95 structurally (off-by-one pattern).

---

## Appendix — v84 case study (worked example)

The v84 in-field freeze took a long path because three unrelated things were conflated:

1. **A real server bug** (atlas-channel sent movement code 255 → client crash) — fixed
   server-side; reproduced on a stock client, so it masked the client-side issue during early
   bisection.
2. **A misread as Themida self-destruct** — the fault PC sat in Themida-encrypted code on the
   packed client, which *looked* like an anti-tamper trap.
3. **The actual root cause:** our `CWvsApp::SetUp` replacement dropped `DR_init`, leaving the
   DR anti-debug check's `NtGetContextThread` pointer null → `call(0)` from the player-movement
   path → `EIP=0`.

What finally cracked it: bisecting hooks on the **depacked** L.exe (pinned to `SetUp`), then
**diffing our `SetUp` against the real v84 `SetUp` disassembly** call-by-call. The fix was to
restore `DR_init`, wired through the memory map (`DR_INIT`) and gated to the GMS versions that
have it. v87+ had survived the same bug only because we hook `DR_check` there.

Lesson, generalized: **a full-replacement hook is a liability the moment the client changes; the
most reliable way to find what it dropped is to diff it against the real per-version function,
not to reason about what "should" be skippable.**
