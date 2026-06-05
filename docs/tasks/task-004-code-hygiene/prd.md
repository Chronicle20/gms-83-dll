# Code Hygiene Pass — Product Requirements Document

Version: v1
Status: Draft
Created: 2026-05-14
---

## 1. Overview

This task is a cross-cutting code-hygiene pass against the items raised in
the "structure / readability" section of the v0 review (items 19–27). It is
purely a refactor and infrastructure cleanup — no edit DLL gains or loses
behavior, no hook is repositioned, no memory map address changes. The goal is
to make the codebase easier to read, easier to extend, and easier to verify
before subsequent feature work lands on top of it.

The work touches five distinct surfaces: the `bypass/` edit (which has
accumulated 12 unrelated hooks in a single 870-line file), the shared
`common/` library (logger, redundant explicit cleanups, the `pch.h` story),
the build system (`memory_map.h.in` validation, version-macro deduplication,
PCH wiring), repo-wide cosmetic issues (license headers), and a brand-new
unit test target for the small handful of host-testable utilities.

Because the work is bundled into a single task, the central guarantee is
*no observable behavior change*. The acceptance bar is "every matrix
configuration in `.github/workflows` continues to build cleanly, each edit
DLL still loads its hooks under `DllMain`, and the new `tests/` target
passes for at least one host build."

## 2. Goals

Primary goals:
- Split `bypass/dllmain.cpp` into per-concern translation units so the file
  named `dllmain.cpp` contains only `DllMain` plus the `MainProc` hook
  installation orchestrator.
- Refactor the `CClientSocket::OnConnect` recv loop into named helpers that
  read like protocol code, not decompiler output.
- Eliminate the redundant `COutPacket::m_aSendBuff.RemoveAll()` calls that
  duplicate what the destructor already does.
- Pick one PCH strategy and follow it through: wire `pch.h` as a real
  precompiled header via `target_precompile_headers` on `common_lib` and
  every edit DLL.
- Strip the placeholder GNU GPL "Foobar" license header from every source
  and header file in the repository; remove the AGPL claim/badge from the
  README. The project is unlicensed going forward.
- Replace the monolithic `memory_map.h.in` generation flow with a CMake
  script that fails loudly when a referenced key is undefined.
- Pick `BUILD_MAJOR_VERSION` / `BUILD_MINOR_VERSION` as the canonical
  build-time version macros and delete the duplicate `MAJOR_VERSION` /
  `MINOR_VERSION` shadows.
- Replace the unconditional `Log()` function with a compile-time
  severity-gated API (`LOG_TRACE` / `LOG_DEBUG` / `LOG_INFO` / `LOG_WARN` /
  `LOG_ERROR`) with a truncation marker on overflow.
- Introduce a `tests/` target driven by GoogleTest (fetched via
  `FetchContent`) and gated behind `BUILD_TESTS=ON`, covering the easily
  isolable utility surfaces.

Non-goals:
- Lifting the JMS `MemEdit::WriteBytes` byte patches on
  `CSecurityClient::OnPacket` into proper hooks (already a documented TODO
  in `bypass/dllmain.cpp` and tracked separately).
- Behavioral fixes from the other items in the v0 review (1–18).
- New edit DLLs, new region/version support, or any change to the existing
  hook surface or address tables.
- Cosmetic-only formatting passes outside the files touched by the items
  above (e.g., no repo-wide `clang-format` run).
- A runtime-configurable logger level. The level is compile-time only for
  this iteration.

## 3. User Stories

- As a developer adding a new hook to `bypass`, I want each hook category in
  its own file so I can find the existing `CClientSocket` hooks without
  scrolling past unrelated `CFuncKeyMappedMan` and `CWvsApp::Run` code.
- As a developer reviewing the `OnConnect` recv loop, I want the protocol
  semantics (read header → read body → decode handshake) named explicitly
  so I do not have to mentally re-derive them from a `goto label_26`.
- As a developer porting an edit to a new version, I want a missing
  memory-map key to fail at the `cmake` configure step with a clear "key
  `FOO` not defined for region `BAR` version `BAZ`" message, not with a
  C++ compile error on `@FOO@`.
- As a developer building in Release, I want `OutputDebugString` per packet
  to be compiled out so the DLL is not chatty when it ships.
- As a developer changing the INI parser or `MemEdit::PatchNop`, I want a
  fast host-build test target that catches regressions before I have to
  attach to the Maple client.
- As a maintainer, I want one canonical `BUILD_MAJOR_VERSION` symbol so
  `#if MAJOR_VERSION > 83` and `#if BUILD_MAJOR_VERSION > 83` cannot drift
  to different values.

## 4. Functional Requirements

The nine items below correspond 1:1 to items 19–27 of the v0 review.

### 4.1 Split `bypass/dllmain.cpp` (item 19)

The current `bypass/dllmain.cpp` (`909` lines per `wc -l`) MUST be split into
the following translation units, all under `bypass/`:

| File | Contents (hooks defined + helpers) |
|---|---|
| `dllmain.cpp` | `DllMain` only. Spawns the `MainProc` thread. |
| `bypass_main.cpp` | `MainProc` (hook installation orchestrator). Includes the per-category headers below. |
| `socket_hooks.{h,cpp}` | `CClientSocket__Connect_Addr_Hook`, `CClientSocket__Connect_Ctx_Hook`, `CClientSocket__OnConnect_Hook`, `CClientSocket__SendPacket_Hook` (v95+), plus the recv-loop helpers from §4.2. |
| `login_hooks.{h,cpp}` | `CLogin__SendCheckPasswordPacket_Hook`. |
| `security_hooks.{h,cpp}` | `CSecurityClient__OnPacket_Hook`, `CeTracer__Run_Hook`, `SendHSLog_Hook`, `DR__check_Hook`, JMS `MemEdit::WriteBytes` byte-patch installation block. |
| `app_hooks.{h,cpp}` | `CWvsApp__CWvsApp_Hook`, `CWvsApp__SetUp_Hook`, `CWvsApp__Run_Hook`, `CWvsApp__CallUpdate_Hook`, `CWvsApp__ConnectLogin_Hook`, `CWvsApp__InitializeInput_Hook`, plus `get_stage` / `get_gr` / `GetSEPrivilege` / `ResetLSP` helpers. |
| `key_mapped_hooks.{h,cpp}` | `CFuncKeyMappedMan__CFuncKeyMappedMan_Hook`. |

Requirements:
- Each `*_hooks.h` exports a single `Install*Hooks()` function called from
  `bypass_main.cpp`. The `HOOKTYPEDEF_C` / `INITMAPLEHOOK_OR_RETURN` macro
  invocations MUST live inside that `Install*Hooks()` function, not at
  global scope.
- Conditional region/version blocks (e.g., `#if (defined(REGION_GMS) &&
  BUILD_MAJOR_VERSION >= 95)`) MUST remain inside the same translation unit
  as the hook they guard; no `#ifdef` block may be split across files.
- `bypass/CMakeLists.txt` MUST be updated to list every new `.cpp` in the
  `SOURCES` argument to `add_edit_dll`.

Acceptance: `bypass-*.dll` MUST build for every region+version combo in the
matrix, and the produced DLL MUST install the same set of hooks as before
this task (verified by `MainProc` installing exactly the same sequence of
`INITMAPLEHOOK_OR_RETURN` calls, in the same order, gated by the same
`#if`s).

### 4.2 Refactor the `OnConnect` recv loop (item 20)

`CClientSocket__OnConnect_Hook` lines 67–113 (the quad-nested `while(true)`
with `goto label_26`) MUST be replaced with three named helpers, declared
`static` inside `socket_hooks.cpp`:

```cpp
// Returns: number of bytes read into `out` (always 2 on success), or 0 on
// short read / disconnect. Returns -1 if the caller should retry the entire
// OnConnect path.
static int read_packet_header(CClientSocket* pSock, char* out);

// Returns: number of bytes read into `out` (always `expectedLen` on
// success), or 0 on short read / disconnect.
static int read_packet_body(CClientSocket* pSock, char* out, int expectedLen);

// Decodes the handshake payload at `[buf, buf+len)`. On success writes
// `outMajorVersion`, `outMinorVersion`, `outSeqSnd`, `outSeqRcv`,
// `outVersionHeader`. Returns true on success, false if the buffer
// underruns mid-decode.
static bool decode_handshake(const char* buf, int len,
                              unsigned short& outMajorVersion,
                              int& outMinorVersion,
                              unsigned int& outSeqSnd,
                              unsigned int& outSeqRcv,
                              unsigned char& outVersionHeader);
```

Requirements:
- The retry count (`something = 40` in the current code) and the
  `Sleep(500)` on `WSAEWOULDBLOCK` MUST be preserved verbatim inside
  `read_packet_header` / `read_packet_body`.
- No `goto` MUST remain in the refactored `OnConnect`.
- The `pBuff` `ZRef<ZSocketBuffer>` lifetime MUST remain identical (allocated
  with `BUFFER_SIZE = 1460`, ref incremented if non-null, never explicitly
  released — the trailing comment about `_ZRef_ZSocketBuffer__Destructor`
  stays as the current `//` comment to preserve the existing intentional
  leak/lifetime semantics).

### 4.3 Remove redundant `m_aSendBuff.RemoveAll()` calls (item 21)

Confirmed by inspection: `common/COutPacket.cpp:64` already calls
`m_aSendBuff.RemoveAll()` from `~COutPacket()`. The premise of v0 review
item 21 (the dtor doesn't release) is false; the explicit clears at
`bypass/dllmain.cpp:210` and `bypass/dllmain.cpp:330` are redundant.

Requirement:
- Delete both `cOutPacket.m_aSendBuff.RemoveAll();` callsites. The dtor
  fires at end-of-scope and handles cleanup. No dtor change is needed.
- Do not touch the `pThis->m_ctxConnect.lAddr.RemoveAll()` / `m_WorldItem.RemoveAll()` /
  `m_aBalloon.RemoveAll()` calls — those operate on long-lived heap-owned
  objects, not stack-locals, and are not redundant.

### 4.4 Wire `pch.h` as a real precompiled header (item 22)

Today, `bypass/dllmain.cpp` and `skip-logo/dllmain.cpp` `#include "pch.h"`,
but the file is not registered via `target_precompile_headers`, so the ~100
transitive includes are paid in full on every compile.

Requirements:
- Add `target_precompile_headers(common_lib PRIVATE pch.h)` in
  `cmake/CommonLib.cmake`, and propagate to consumers by adding the same
  call inside `add_edit_dll` in `cmake/AddEditDll.cmake` (using `REUSE_FROM
  common_lib` to share the PCH artifact).
- Edit DLLs that currently do not `#include "pch.h"` (every edit except
  `bypass` and `skip-logo`) MUST continue to build — `REUSE_FROM` does not
  force-include; the include in source files is what activates PCH for
  that TU. No changes to those edits' sources are required.
- The two existing `#include "pch.h"` directives in `bypass/dllmain.cpp`
  and `skip-logo/dllmain.cpp` stay.
- The commented-out `//#ifndef PCH_H` block at the top of `common/pch.h`
  may be deleted; it is dead.
- No new headers added to `pch.h` in this task.

Slimming/splitting `pch.h` into per-domain groupings is out of scope for
this task — to be revisited if/when build profiling shows the unified PCH
is still the bottleneck.

### 4.5 Strip license headers (item 23)

The repository's source files carry the GNU GPL boilerplate with the
unreplaced placeholder `"along with Foobar. If not, see..."`; the README
advertises AGPL 3.0. The user has chosen to remove the license entirely.

Requirements:
- Strip the leading `/* ... */` comment block matching the GNU GPL
  boilerplate from every `.cpp`, `.h`, `.cmake`, and `.cmake.in` file under
  the repository root.
- Implement via a one-shot Python or shell script committed under
  `scripts/strip_license_header.py`. The script MUST match the boilerplate
  by the `"This file is part of GMS-83-DLL."` opener and the trailing
  `"<https://www.gnu.org/licenses/>"` line so it cannot accidentally match
  unrelated comment blocks. Files with no such header are left alone.
- Update `README.md`: remove the `![GitHub license]` badge line and the
  trailing "## License" section.
- Do not add a `LICENSE` file; the existing one MAY be removed in the same
  commit. (Confirm with maintainer at PR review time.)
- The script MAY be deleted after the cleanup commit lands; it does not
  need to stay as ongoing tooling.

### 4.6 Memory map validation (item 24)

The current flow uses CMake's `configure_file` on a monolithic
`include/memory_map.h.in`. If a `memory_maps/<REGION>/v<MAJ>_<MIN>.cmake`
file forgets to `set()` a key, the resulting header contains
`#define FOO @FOO@`, and the error surfaces as a C++ compile error far from
the cause.

Requirements:
- Replace the existing `configure_file` invocation in the root
  `CMakeLists.txt` with a CMake script (e.g., `cmake/GenerateMemoryMap.cmake`)
  that:
  1. Parses `include/memory_map.h.in` to extract every `@KEY@` placeholder.
  2. For each extracted key, checks that the corresponding CMake variable
     is defined and non-empty.
  3. On any missing key, emits `message(FATAL_ERROR "Memory map for
     ${BUILD_REGION} v${BUILD_MAJOR_VERSION}.${BUILD_MINOR_VERSION} is
     missing key: ${KEY}")` and aborts. Multiple missing keys MUST all be
     reported in a single `FATAL_ERROR` (collected and joined), not one at
     a time.
  4. Only after validation, calls `configure_file` to write the final
     `${CMAKE_BINARY_DIR}/generated/memory_map.h`.
- The script MUST be invoked from the root `CMakeLists.txt` after the
  `include(${MEMORY_MAP_FILE})` line.
- The monolithic structure of `memory_map.h.in` stays — splitting per
  domain is deferred. The script is the entire deliverable here.

### 4.7 Unify version macros (item 25)

The repo mixes `MAJOR_VERSION` / `MINOR_VERSION` (defined by
`memory_map.h.in`) with `BUILD_MAJOR_VERSION` / `BUILD_MINOR_VERSION`
(defined as compile defs on `build_config` in the root `CMakeLists.txt`).
`bypass/dllmain.cpp` uses both, on adjacent lines, interchangeably. Note
that `MAJOR_VERSION` also collides with a Windows SDK macro in some headers.

Requirements:
- Pick `BUILD_MAJOR_VERSION` and `BUILD_MINOR_VERSION` as canonical.
- Delete the `MAJOR_VERSION` and `MINOR_VERSION` `#define`s from
  `include/memory_map.h.in` and the corresponding `@MAJOR_VERSION@` /
  `@MINOR_VERSION@` placeholders.
- Delete the `VERSION_HEADER` is unaffected (it is a per-version protocol
  constant, not a build identity).
- `grep -rn "MAJOR_VERSION\|MINOR_VERSION"` across the repo MUST yield
  only matches that are either (a) `BUILD_MAJOR_VERSION` /
  `BUILD_MINOR_VERSION`, or (b) the protocol-level `VERSION_HEADER`. All
  other occurrences MUST be rewritten or deleted.
- Every `memory_maps/<region>/v*_*.cmake` file MUST have its `set(MAJOR_VERSION ...)`
  / `set(MINOR_VERSION ...)` lines removed (those values are already
  supplied by the `BUILD_MAJOR_VERSION` / `BUILD_MINOR_VERSION` CMake
  cache vars passed in from the build command).

### 4.8 Severity-gated logger (item 26)

Today `Log()` is unconditional, the buffer is 1024 bytes with no
truncation indicator, and Release builds spam `OutputDebugString` per
packet.

Requirements:
- Introduce a `LogLevel` enum in `common/logger.h`:

  ```cpp
  enum class LogLevel { Trace = 0, Debug = 1, Info = 2, Warn = 3, Error = 4 };
  ```

- Introduce `LOG_TRACE(fmt, ...)`, `LOG_DEBUG(...)`, `LOG_INFO(...)`,
  `LOG_WARN(...)`, `LOG_ERROR(...)` macros. Each expands to either a call
  into `LogImpl(level, fmt, ...)` or to nothing, depending on a
  compile-time threshold `LOG_MIN_LEVEL`.
- `LOG_MIN_LEVEL` defaults are baked into `build_config` as compile defs:
  - Debug build: `LogLevel::Trace` (everything).
  - Release build: `LogLevel::Warn`.
  Use `$<CONFIG:...>` generator expressions to select.
- `LogImpl` MUST format into a `1024`-byte stack buffer and append a
  three-character ellipsis `...` truncation marker if `vsnprintf` returns
  a size `>=` the buffer length. The output line MUST be prefixed with the
  level tag, e.g. `[INFO] CClientSocket::OnConnect ...`.
- The existing free function `Log(const char*, ...)` is kept as a thin
  wrapper that forwards to `LOG_INFO(fmt, ...)` so callers can land
  without an all-at-once rename. New code MUST use the `LOG_*` macros.
- No INI/runtime override in this iteration.

### 4.9 Unit test target (item 27)

A `tests/` subdirectory MUST be added at repo root, driven by GoogleTest.

Requirements:
- Top-level CMake option `BUILD_TESTS` (default `OFF`). When `OFF`, the
  `tests/` subdir is not added — matrix builds remain unaffected.
- When `BUILD_TESTS=ON`:
  - Use `FetchContent` to pull GoogleTest pinned to a specific release tag
    (e.g., `v1.14.0`). The fetched source MUST NOT be committed.
  - Add `tests/` as a subdirectory containing one or more
    `add_executable` GoogleTest targets, linked against `gtest_main`.
  - Tests build for the **host** architecture (x64); they do not link
    against the Maple binary, do not load Maple-side memory addresses,
    and do not link `common_lib` as it currently stands (which is x86-
    only). Anything the tests cover MUST first be extractable into a
    small, host-buildable static library (e.g., `tests/support/parse_ini.cpp`)
    by copying or moving the relevant source.
- Initial coverage (at least one test case per item):
  - `ParseINI` from `redirect/dllmain.cpp` — extract into
    `common/parse_ini.{h,cpp}` (host-buildable, no Win32 deps) and have
    `redirect` consume the new header. Tests cover: valid k=v pair,
    leading/trailing whitespace, `;` and `#` comments, duplicate keys,
    malformed lines, missing file.
  - `MemEdit::WriteBytes` and `MemEdit::PatchNop` — pure-byte-manipulation
    pieces (those that do not call `VirtualProtect`) extracted into a
    host-buildable helper. Tests cover: byte copy correctness, NOP fill,
    boundary on zero-length write.
- A new GitHub Actions job (or a new step on the existing matrix) MAY run
  the tests; CI wiring is OPTIONAL for this task — if not in this PR, a
  TODO note in `docs/TODO.md` is sufficient.

## 5. Hook / Patch Surface

No new hooks. No address changes. No memory map entries added. The set of
`INITMAPLEHOOK_OR_RETURN` calls in the post-refactor `bypass` edit MUST be
identical to the pre-refactor set (same hook names, same address macros,
same `#if` guards, same install order).

## 6. Configuration

No runtime config. One new compile-time knob:

| CMake variable | Default | Effect |
|---|---|---|
| `LOG_MIN_LEVEL` | `Trace` (Debug) / `Warn` (Release) | Compile-time threshold; any `LOG_*` macro below this expands to no-op. |
| `BUILD_TESTS` | `OFF` | Enables the `tests/` subdirectory. |

No INI keys are added or removed by this task.

## 7. Memory Map Impact

- No new keys defined.
- Keys `MAJOR_VERSION`, `MINOR_VERSION` REMOVED from `memory_map.h.in`
  (replaced by `BUILD_MAJOR_VERSION`, `BUILD_MINOR_VERSION` from
  `build_config`).
- `set(MAJOR_VERSION ...)` / `set(MINOR_VERSION ...)` lines removed from
  every `memory_maps/<region>/v*_*.cmake` file.
- No other key values change.
- New CMake script `cmake/GenerateMemoryMap.cmake` validates that every
  `@KEY@` in `memory_map.h.in` is set before `configure_file` runs.

## 8. Non-Functional Requirements

- **Behavior preservation.** The compiled `bypass-*.dll` and every other
  edit DLL MUST behave identically to its pre-refactor build for every
  region+version in the matrix. "Behave identically" is defined as
  installing the same hooks at the same addresses, gated by the same
  conditional compilation, in the same order.
- **Build cleanliness.** No new MSVC warnings introduced. Existing warnings
  in untouched files may persist.
- **Build time.** PCH wiring MUST NOT increase clean build time. (No hard
  numerical target; the user can spot-check before/after.)
- **No Themida/AV impact.** This task does not change the on-disk layout
  of the DLLs' code in a way that would affect Themida signature behavior
  beyond what reordering object files in the link line already does.
- **No runtime perf regression.** The `LOG_*` macros below threshold MUST
  compile away to a single empty statement (no varargs evaluation, no
  call). Verify by inspecting the disassembly of a Release-build TU that
  uses `LOG_TRACE`.

## 9. Open Questions

- Should `LICENSE` (the file) be deleted in the same commit as the header
  strip, or kept around until a project license decision is revisited?
  Default for this task: delete it. Confirm at PR review.
- Should `BUILD_TESTS=ON` be wired into CI matrix as a separate job, or
  left as a developer-local switch? Default for this task: developer-local
  only, with a TODO note for CI follow-up.

## 10. Acceptance Criteria

Build matrix (must all configure + build clean, zero new warnings):

- `cmake -DBUILD_REGION=GMS -DBUILD_MAJOR_VERSION=83 -DBUILD_MINOR_VERSION=1 ...`
- `cmake -DBUILD_REGION=GMS -DBUILD_MAJOR_VERSION=87 -DBUILD_MINOR_VERSION=1 ...`
- `cmake -DBUILD_REGION=GMS -DBUILD_MAJOR_VERSION=95 -DBUILD_MINOR_VERSION=1 ...`
- `cmake -DBUILD_REGION=GMS -DBUILD_MAJOR_VERSION=111 -DBUILD_MINOR_VERSION=1 ...`
- `cmake -DBUILD_REGION=JMS -DBUILD_MAJOR_VERSION=185 -DBUILD_MINOR_VERSION=1 ...`

Plus the host test build:

- `cmake -DBUILD_TESTS=ON ...` configures, builds, and `ctest` passes
  green on the host architecture.

Per-item completion checklist:

- [x] **19** — `bypass/dllmain.cpp` contains only `DllMain`; six new files
  exist under `bypass/`; every hook lives in the file the table in §4.1
  assigns it to; `bypass/CMakeLists.txt` lists every new source.
- [x] **20** — `CClientSocket__OnConnect_Hook` calls
  `read_packet_header` / `read_packet_body` / `decode_handshake` instead of
  the nested-while + `goto label_26`; no `goto` remains in the hook;
  retry/sleep behavior preserved.
- [x] **21** — `cOutPacket.m_aSendBuff.RemoveAll();` deleted from both
  callsites; `~COutPacket()` unchanged.
- [x] **22** — `target_precompile_headers` registered on `common_lib`;
  every edit DLL reuses it via `REUSE_FROM`; both existing `#include
  "pch.h"` directives still present; commented-out `#ifndef PCH_H` block
  removed.
- [x] **23** — No source/header file in the repo contains the
  `"This file is part of GMS-83-DLL"` boilerplate; README's license badge
  and "## License" section removed; one-shot
  `scripts/strip_license_header.py` committed (or run-and-deleted, per
  reviewer preference).
- [x] **24** — `cmake/GenerateMemoryMap.cmake` exists; root `CMakeLists.txt`
  calls it instead of bare `configure_file`; deliberately undefining a key
  in a `.cmake` file produces `FATAL_ERROR` naming the key (verify by
  manual test).
- [x] **25** — `grep -rn "\bMAJOR_VERSION\b\|\bMINOR_VERSION\b" .` returns
  only `BUILD_MAJOR_VERSION` / `BUILD_MINOR_VERSION` matches (and any
  protocol-level `VERSION_HEADER` matches); `memory_map.h.in` no longer
  defines `MAJOR_VERSION` / `MINOR_VERSION`; every
  `memory_maps/<region>/v*.cmake` no longer `set()`s them.
- [x] **26** — `LOG_TRACE` / `LOG_DEBUG` / `LOG_INFO` / `LOG_WARN` /
  `LOG_ERROR` macros exist in `common/logger.h`; truncation marker
  appended on overflow; `LOG_MIN_LEVEL` defaults to `Trace` in Debug and
  `Warn` in Release; legacy free `Log()` forwards to `LOG_INFO`.
- [x] **27** — `tests/` subdirectory builds with `BUILD_TESTS=ON`;
  GoogleTest fetched via `FetchContent` (pinned tag); `ParseINI` and
  `MemEdit` test cases exist and pass on host; `redirect` consumes the
  new `common/parse_ini.{h,cpp}`.

Functional verification (each must hold post-refactor):

- [x] `MainProc` in the new `bypass_main.cpp` installs hooks in the same
  order, with the same `#if` guards, as the pre-refactor `MainProc`.
- [x] `nm` / `dumpbin` symbol diff of `bypass-*.dll` before vs after
  shows only naming / ordering churn — every prior hook function is still
  present under the same name (modulo file relocation), every prior
  exported symbol is still exported.
- [x] No edit DLL fails its `DllMain` `CreateThread(&MainProc, ...)` step
  when loaded by the proxy.
