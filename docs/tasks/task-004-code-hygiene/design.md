# Code Hygiene Pass — Design

Version: v1
Status: Draft
Created: 2026-05-14
Companion to: `prd.md`

---

## 1. Scope & Guiding Principles

This design backs the nine functional requirements in `prd.md` §4. The
guiding principles, in priority order:

1. **No observable behavior change.** Every edit DLL produced by this task
   MUST install the same hooks at the same addresses, gated by the same
   `#if`s, in the same install order. The "before vs after" diff is
   exclusively (a) source-file relocation, (b) added/renamed symbols
   private to a TU, and (c) PCH/CMake plumbing.
2. **Touch only what the PRD lists.** No drive-by formatting. No `clang-format`
   sweep. No header reordering outside the files listed in §4.
3. **Small, host-buildable seams.** The two pieces that go into tests
   (`parse_ini`, the byte-only portion of `MemEdit`) MUST be split such
   that no Win32-only symbol leaks into the host TU. That isolation is
   the design constraint that decides whether item 27 is feasible at
   all; if a clean split can't be drawn, the test target is the part
   that gets cut, not the seams that are stretched to make it fit.

The remainder of this document walks each PRD §4 item top-to-bottom and
records the decisions that the PRD left to design judgment.

---

## 2. Per-Item Design

### 2.1 `bypass/` TU Split (PRD §4.1)

**Decisions:**

- **Hook function definitions live in the `.cpp`, not the header.** Each
  `*_hooks.h` exports exactly one symbol: `void Install<Category>Hooks();`.
  Hook bodies, typedefs, helper functions, and `HOOKTYPEDEF_C`-generated
  function pointers stay file-local (anonymous namespace or `static`).
  This keeps the header surface near-zero and prevents the original-pointer
  globals from being multiply-defined if a header is later included
  twice.
- **`HOOKTYPEDEF_C` / `INITMAPLEHOOK_OR_RETURN` placement.** PRD §4.1
  pins these inside `Install*Hooks()`. The `HOOKTYPEDEF_C` macro
  generates a translation-unit-local pointer variable; placing it
  inside the function (it expands to a `static` local with explicit
  initialization in the existing macro design — confirm against
  `common/hooker.h` during implementation; if it generates a global,
  it stays at namespace scope inside the same TU).
- **`bypass_main.cpp` is the only TU that includes all six `*_hooks.h`
  headers.** No `*_hooks.cpp` includes another `*_hooks.h`. Cross-hook
  call sites (e.g., `CClientSocket__OnConnect_Hook` calling itself or
  `CClientSocket__Connect_Addr_Hook` calling `OnConnect`) are handled
  by giving each hook external linkage *within* `socket_hooks.cpp`
  (they're in the same TU). The single cross-TU caller —
  `CWvsApp::ConnectLogin_Hook` in `app_hooks.cpp` calls
  `CClientSocket__OnConnect_Hook` (defined in `socket_hooks.cpp`) — gets
  a single `extern "C"` (or just a forward declaration with C++ linkage)
  in a tiny header `socket_hooks_internal.h`. We do NOT promote
  `CClientSocket__OnConnect_Hook` to a public symbol via
  `socket_hooks.h`; the public header still exposes only
  `InstallSocketHooks()`. The forward decl in `socket_hooks_internal.h`
  is included only by `app_hooks.cpp` and `socket_hooks.cpp`.
- **`MainProc` install order.** `bypass_main.cpp::MainProc` calls the
  six `Install*Hooks()` functions in this order, matching the
  pre-refactor install order of the affected `INITMAPLEHOOK_OR_RETURN`
  blocks:

      InstallAppHooks();          // CWvsApp::* (CWvsApp, SetUp, InitializeInput, Run, CallUpdate, ConnectLogin)
      InstallSecurityHooks();     // CSecurityClient::OnPacket, JMS byte patches, DR_check, CeTracer, SendHSLog
      InstallLoginHooks();        // CLogin::SendCheckPasswordPacket
      InstallSocketHooks();       // CClientSocket::OnPacket, Connect_ctx, Connect_addr, OnConnect, SendPacket (v95+)
      InstallKeyMappedHooks();    // CFuncKeyMappedMan::CFuncKeyMappedMan

  Each `Install*Hooks()` runs its `INITMAPLEHOOK_OR_RETURN` calls in
  the same relative order they appeared in the pre-refactor `MainProc`,
  with the same `#if` guards. Crucially, `INITMAPLEHOOK_OR_RETURN`
  early-returns from its enclosing function on failure, so a failed
  install inside an `Install*Hooks()` returns to `MainProc` and the
  subsequent installer still runs. That is a deliberate **behavior
  change vs. the pre-refactor `MainProc`**, which short-circuited the
  remaining installs on any single failure.

  To preserve pre-refactor semantics, `Install*Hooks()` MUST return
  `BOOL` and `MainProc` MUST check each return value and itself
  `return 0;` on failure. The `INITMAPLEHOOK_OR_RETURN` macro inside the
  installer returns 0 (the BOOL false) from the installer on failure;
  the installer is declared `BOOL Install<Category>Hooks();` and ends
  with `return TRUE;`. `MainProc` becomes:

      if (!InstallAppHooks())        return 0;
      if (!InstallSecurityHooks())   return 0;
      ...

  This preserves the exact install-failure short-circuit behavior.

- **`bypass/CMakeLists.txt`** is updated to:

      add_edit_dll(bypass SOURCES
          dllmain.cpp
          bypass_main.cpp
          socket_hooks.cpp
          login_hooks.cpp
          security_hooks.cpp
          app_hooks.cpp
          key_mapped_hooks.cpp
      )

- **`#include "pch.h"`** stays at the top of every new `*_hooks.cpp` and
  of `bypass_main.cpp` so each TU activates the precompiled header (see
  §2.4). `dllmain.cpp` already has it; new TUs gain it.

**Alternatives considered:**

- *One header per hook function.* Rejected — explodes file count, and
  the natural grouping is by C++ class (the PRD's choice).
- *Keep `HOOKTYPEDEF_C` at namespace scope, expose the original-function
  pointer via the header.* Rejected — leaks an implementation detail and
  makes the header non-trivially includable.
- *Skip the per-installer return value and accept the looser failure
  semantics.* Rejected — would be the one observable behavior change in
  the refactor.

### 2.2 `OnConnect` Recv-Loop Refactor (PRD §4.2)

**Decisions:**

- **Retry budget is a shared local in `OnConnect`, passed by reference
  into both helpers.** The pre-refactor code declares `int something =
  40;` once outside both loops and decrements it across every
  `WSAEWOULDBLOCK` occurrence in either the header read OR the body
  read. To preserve the byte-for-byte equivalence the PRD requires, the
  refactored signatures are:

      // Returns number of bytes read (lenToRead on full success), or 0
      // on short read / disconnect. `retries` is decremented on every
      // WSAEWOULDBLOCK observed and persists across calls.
      static int read_packet_header(CClientSocket* pSock, char* out, int& retries);
      static int read_packet_body(CClientSocket* pSock, char* out, int expectedLen, int& retries);

  This is a deviation from the PRD's literal signatures (which take no
  `retries` parameter), motivated by behavior preservation. If the user
  prefers the PRD signatures verbatim, the alternative is to reset
  `retries = 40` at the top of each helper, which is a slight behavior
  change (more total retries possible end-to-end). **Recommendation:
  pass by reference.** Open question for user review.

- **`decode_handshake` signature.** Adopted verbatim from PRD §4.2. Its
  body uses `CIOBufferManipulator::Decode2 / DecodeStr / Decode4 /
  Decode1` exactly as the current inline code does, and returns `false`
  if the post-decode `result` pointer is past `accumulatedBuf` (the
  buffer-underrun check at current line 136). The
  `version > MINOR_VERSION` and `majorVersion > BUILD_MAJOR_VERSION`
  termination throws are NOT inside `decode_handshake`; they stay in
  `OnConnect` after the helper returns. `decode_handshake` is pure
  decoding.

- **`OnConnect` post-refactor control flow** (pseudocode, no `goto`):

      int retries = 40;
      while (true) {
          int hdr = read_packet_header(pSock, buffer, retries);
          if (hdr == 0) {
              CClientSocket__OnConnect_Hook(pThis, edx, 0);
              return 0;
          }
          int expectedLen = static_cast<unsigned char>(buffer[0]);
          if (expectedLen > pBuff.p->len) {
              break; // body too large for the buffer — fall through with bytesReceived=0 like original
          }
          int body = read_packet_body(pSock, buffer, expectedLen, retries);
          if (body == 0) {
              CClientSocket__OnConnect_Hook(pThis, edx, 0);
              return 0;
          }
          accumulatedBuf = buffer + body;
          break; // proceed to handshake decode
      }
      // ... rest of OnConnect unchanged ...

  Note the "body too large" exit: the original code falls out of the
  outer while with `bytesReceived = 0`, then hits `label_26`, which
  takes the disconnect path. We replicate that by going through the
  same disconnect call. Subtle but PRD-required.

- **`pBuff` lifetime preserved.** The `ZRef<ZSocketBuffer> pBuff` and
  its `InterlockedIncrement` stay in `OnConnect` exactly as today. The
  trailing `//_ZRef_ZSocketBuffer__Destructor(&pBuff, edx, 0);` comment
  also stays (PRD §4.2 explicit requirement).

- **`Sleep(500)`** stays inside `read_packet_header` / `read_packet_body`
  on the `WSAEWOULDBLOCK` branch (PRD requirement; the helper handles
  blocking, not the caller).

**Alternatives considered:**

- *Recursion to handle short reads.* Rejected — `recv` can short-read in
  practice; recursion masks that and adds stack depth.
- *Replace `recv` loop with a single blocking `recv` of `lenToRead`.*
  Rejected — sockets in this path may be non-blocking and a single
  `recv` can return fewer bytes than requested; the loop is doing real
  work.

### 2.3 Redundant `RemoveAll()` Removal (PRD §4.3)

Straight deletion. No design — the two callsites identified by the PRD
(`bypass/dllmain.cpp:210` and `:330` pre-refactor, in `OnConnect` and
`CLogin::SendCheckPasswordPacket` respectively) become a one-line
deletion each. Post-TU-split, these are in `socket_hooks.cpp` and
`login_hooks.cpp`.

### 2.4 PCH Wiring (PRD §4.4)

**Decisions:**

- **`common_lib` is an OBJECT library.** CMake supports
  `target_precompile_headers` on OBJECT libraries (CMake ≥ 3.16, we're
  on 3.26). `REUSE_FROM common_lib` on a SHARED library consumer also
  works — the consumer reuses the PCH artifact built once for
  `common_lib`. Verified semantics: the consumer must use the same set
  of compile options, and the toolchain must be the same. Our edit
  DLLs all link `build_config` and `common_lib` the same way, so this
  is met.
- **`cmake/CommonLib.cmake`** adds:

      target_precompile_headers(common_lib PRIVATE
          "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/common/pch.h>")

  Genex-guarded on COMPILE_LANGUAGE:CXX so a stray `.c` file (none
  exist today, defensive) doesn't try to consume the C++ PCH.

- **`cmake/AddEditDll.cmake`** appends after `add_library(${name}
  SHARED ...)`:

      target_precompile_headers(${name} REUSE_FROM common_lib)

  Every edit DLL gets this unconditionally. Edits that don't `#include
  "pch.h"` in their TU still pay nothing — the PCH is force-included
  ONLY for TUs that already include it textually, with `REUSE_FROM`.
  Actually: this is a subtle PRD claim worth verifying. `REUSE_FROM`
  by default DOES force-include the PCH header. To get the PRD's
  desired behavior ("only TUs that include pch.h activate it"), we
  need to verify. **Implementation note:** if `REUSE_FROM` is
  unconditional and force-includes `pch.h`, we either (a) accept the
  force-include in every edit DLL (it's just a couple hundred
  transitive includes pre-baked — most of the surface is unused per TU
  but the linker drops it), or (b) skip `REUSE_FROM` for the
  non-`pch.h`-including edits. **Recommendation: (a) accept the
  force-include**. It simplifies the CMake helper, and the build-time
  win from sharing the PCH across all edits more than offsets any cost
  of extra symbols flowing through compile but getting dropped at link.
  This is a deviation from PRD §4.4's "edits that don't include pch.h
  build unchanged" guideline; calling it out explicitly here.

- **`common/pch.h` cleanup.** The commented-out `//#ifndef PCH_H` /
  `//#define PCH_H` block at lines 7–8 is deleted (PRD requirement).
  No new includes are added.

- **`common/pch.cpp`.** Today, `CommonLib.cmake` explicitly removes
  `pch.cpp` from the source glob. With `target_precompile_headers`,
  CMake auto-generates a synthetic `cmake_pch.cxx` translation unit
  to build the PCH; the existing `pch.cpp` is still skipped (it's a
  Visual Studio artifact). Leave the `list(REMOVE_ITEM ... pch.cpp)`
  call alone.

**Alternatives considered:**

- *Forge a separate PCH per edit DLL.* Rejected — defeats the purpose;
  every edit would re-build the same ~100-include PCH.
- *Make `common_lib` a STATIC library so PCH sharing is more natural.*
  Rejected — out of scope; OBJECT-vs-STATIC is a separate decision,
  and OBJECT lets `/OPT:REF` drop unreferenced TUs at link time, which
  STATIC also does but the chain is more straightforward.

### 2.5 License Header Strip (PRD §4.5)

**Decisions:**

- **Language: Python 3.** Cross-platform, multi-line regex is trivial,
  and the script is short enough that "but is Python installed" is a
  non-issue for the target audience (it's the CI image's default and
  every developer machine has it via Git Bash or WSL).
- **Match pattern.** The script reads each file's bytes, finds the
  first `/*` at the top (within the first 100 bytes), and tests
  whether the closing `*/` is preceded by the literal
  `This file is part of GMS-83-DLL.` somewhere inside the block AND
  the literal `<https://www.gnu.org/licenses/>` near the end of the
  block. Both anchors required — neither alone is sufficient. If
  matched, the entire `/* ... */` block plus one trailing newline is
  removed.
- **Scope: `.cpp`, `.h`, `.cmake`, `.cmake.in` under the repository
  root.** Walks the tree; skips `.git`, `build*`, and any third-party
  vendor directory if present.
- **README.md.** Separately handled: a small `sed`-style line removal
  for the badge line and the `## License` section (everything from
  `^## License$` to end-of-file or next `^## ` heading). The Python
  script handles this as a separate code path keyed off filename.
- **`LICENSE` file.** Deleted in the same commit as the cleanup. The
  PRD marks this as confirm-at-PR-review; default is to delete.
- **Script lifetime.** Committed at `scripts/strip_license_header.py`,
  run once during this PR, kept in the repo for auditability (so the
  reviewer can re-run it against the diff). It does NOT become
  ongoing tooling; a follow-up commit may remove it after merge if
  desired.

### 2.6 Memory Map Validation (PRD §4.6)

**Decisions:**

- **New file: `cmake/GenerateMemoryMap.cmake`.** Invoked from root
  `CMakeLists.txt` immediately after `include(${MEMORY_MAP_FILE})`,
  replacing the bare `configure_file()` call.
- **Extraction strategy:** read the .in file via `file(STRINGS
  ${INFILE} LINES)`, then for each line run `string(REGEX MATCHALL
  "@[A-Z0-9_]+@" matches "${LINE}")`. Collect unique keys into a CMake
  list. Use `list(REMOVE_DUPLICATES)`.
- **Validation:** for each key, test `if(NOT DEFINED ${KEY})` AND
  `if("${${KEY}}" STREQUAL "")` (both checks — a `set(FOO "")` should
  still fail validation, because configure_file would substitute an
  empty literal). On failure, append the key to a `MISSING_KEYS`
  list.
- **Reporting:** after the loop, if `MISSING_KEYS` is non-empty, emit a
  single `FATAL_ERROR` like:

      message(FATAL_ERROR
          "Memory map for ${BUILD_REGION} v${BUILD_MAJOR_VERSION}.${BUILD_MINOR_VERSION} "
          "is missing required keys:\n  ${MISSING_KEYS_JOINED}")

  where `MISSING_KEYS_JOINED` is the list joined with `\n  ` so each
  key is on its own indented line.
- **After validation:** call `configure_file()` from inside the script
  (or have the script set a flag and let the root `CMakeLists.txt`
  call it — either works; the script-owns-configure_file variant is
  cleaner and is what we'll do).

**Alternatives considered:**

- *Split `memory_map.h.in` into per-domain files.* Rejected — PRD
  explicitly defers this.
- *Compile-time check via `static_assert` in C++.* Rejected — fails too
  late and per-key duplication is awful.

### 2.7 Version Macro Unification (PRD §4.7)

**Decisions:**

- **Action: pure rename + delete.** All non-`BUILD_`-prefixed
  `MAJOR_VERSION` / `MINOR_VERSION` references become
  `BUILD_MAJOR_VERSION` / `BUILD_MINOR_VERSION`.
- **Pre-audit grep result** (recorded for the plan-writing phase):
  - `include/memory_map.h.in:4` — `#define MAJOR_VERSION
    @BUILD_MAJOR_VERSION@` → delete this line.
  - `include/memory_map.h.in:5` — `#define MINOR_VERSION
    @BUILD_MINOR_VERSION@` → delete this line.
  - `bypass/dllmain.cpp:163` — `version > MINOR_VERSION` →
    `BUILD_MINOR_VERSION`.
  - `bypass/dllmain.cpp:205` — `MAJOR_VERSION > 83` →
    `BUILD_MAJOR_VERSION > 83`.
  - `bypass/dllmain.cpp:592`, `:600`, `:886` — `MAJOR_VERSION >= 95` →
    `BUILD_MAJOR_VERSION >= 95`.
  - `bypass/dllmain.cpp:777` — `MAJOR_VERSION >= 111` →
    `BUILD_MAJOR_VERSION >= 111`.
  - `common/CClientSocket.h:22` — `MAJOR_VERSION >= 111` →
    `BUILD_MAJOR_VERSION >= 111`.
  - `common/CWvsContext.h:21,31,73,76` — four sites, each →
    `BUILD_MAJOR_VERSION`.
  - `common/CFuncKeyMappedMan.h:18,24` — two sites, each →
    `BUILD_MAJOR_VERSION`.
- **PRD inaccuracy: `memory_maps/<region>/v*_*.cmake` files do NOT
  `set(MAJOR_VERSION ...)` or `set(MINOR_VERSION ...)` today.** Only
  `VERSION_HEADER` is set in those files. The PRD §4.7 requirement to
  "remove those `set()` calls" is a no-op against current repo state;
  the design records this and the plan-writing phase will skip those
  edits. `VERSION_HEADER` (the per-version protocol constant) stays
  untouched.
- **`VERSION_HEADER` stays in `memory_map.h.in`.** It's a protocol
  constant, not build identity. PRD §4.7 explicitly preserves it.
- **Windows SDK `MAJOR_VERSION` collision.** Once we delete the
  `#define MAJOR_VERSION` from `memory_map.h.in`, any future include of
  certain Windows headers that define `MAJOR_VERSION` no longer
  collides with ours. Net positive.

### 2.8 Severity-Gated Logger (PRD §4.8)

**Decisions:**

- **`LogLevel` enum in `common/logger.h`:**

      enum class LogLevel : int { Trace = 0, Debug = 1, Info = 2, Warn = 3, Error = 4 };

  Explicit `: int` so the underlying type matches the `int`-typed
  `LOG_MIN_LEVEL` compile def.

- **Compile-time threshold.** `LOG_MIN_LEVEL` is passed as an `int` via
  `target_compile_definitions` on `build_config`, with a `$<CONFIG:...>`
  generator expression in the root `CMakeLists.txt`:

      target_compile_definitions(build_config INTERFACE
          $<$<CONFIG:Debug>:LOG_MIN_LEVEL=0>
          $<$<NOT:$<CONFIG:Debug>>:LOG_MIN_LEVEL=3>
      )

  (Debug = Trace = 0; everything else = Warn = 3.) Multi-config
  generators (MSVC) evaluate per-config; single-config generators get
  the right value at configure time.

- **Macro shape — VA-args.** C++17 has no `__VA_OPT__`. Two
  no-trailing-args options: (a) MSVC traditional preprocessor's
  comma-swallowing of `, __VA_ARGS__` when empty; (b) GCC's `, ##__VA_ARGS__`
  extension. Both work in MSVC ≥ VS2019 16.x with `/Zc:preprocessor`
  default in newer toolchains; without it MSVC's traditional
  preprocessor already does the swallow. **Decision: require at least
  one argument (the format string)** and accept that `LOG_INFO("foo")`
  expands to `LogImpl(LogLevel::Info, "foo")` with no varargs. This
  avoids preprocessor flag dependencies and the existing codebase's
  `Log()` callers all pass either zero or N args; zero-arg becomes
  one-arg `("literal")` which works fine with `vsnprintf`.

  Macro:

      #define LOG_IMPL_(level, fmt, ...) \
          ::LogImpl(level, fmt, ##__VA_ARGS__)
      #if LOG_MIN_LEVEL <= 0
      #define LOG_TRACE(fmt, ...) LOG_IMPL_(LogLevel::Trace, fmt, ##__VA_ARGS__)
      #else
      #define LOG_TRACE(fmt, ...) ((void)0)
      #endif
      // ... and so on for DEBUG/INFO/WARN/ERROR

  We use `, ##__VA_ARGS__` because MSVC has supported it for a decade
  even with traditional preprocessor; if a future toolchain bump breaks
  this, we revisit. The fallback (`/Zc:preprocessor` + `__VA_OPT__`)
  is a one-line CMake addition.

- **`LogImpl` body** in `common/logger.cpp`:

      void LogImpl(LogLevel level, const char* fmt, ...) {
          constexpr size_t kBufSz = 1024;
          char buf[kBufSz];
          const char* tag = LevelTag(level); // "[TRACE]" etc.
          int prefix = snprintf(buf, kBufSz, "%s ", tag);
          if (prefix < 0 || static_cast<size_t>(prefix) >= kBufSz) return;
          va_list args;
          va_start(args, fmt);
          int written = vsnprintf(buf + prefix, kBufSz - prefix, fmt, args);
          va_end(args);
          if (written >= 0 && static_cast<size_t>(prefix + written) >= kBufSz) {
              // truncation marker
              static_assert(kBufSz >= 4, "buffer must hold '...' + NUL");
              buf[kBufSz - 4] = '.';
              buf[kBufSz - 3] = '.';
              buf[kBufSz - 2] = '.';
              buf[kBufSz - 1] = '\0';
          }
          OutputDebugStringA(buf);
      }

  `LevelTag` is a small `static const char*` lookup keyed off the
  enum. Marked `inline` in the header? No — keeps the .cpp self-contained.

- **Legacy `Log()` wrapper.** PRD §4.8 requires keeping `Log()` as a
  thin forwarder to `LOG_INFO`. Because `LOG_INFO` is a macro that may
  expand to `((void)0)` in Release, calling `Log()` from the new wrapper
  needs to bypass the macro gate and go straight to `LogImpl(LogLevel::Info, ...)`
  — otherwise `Log()` becomes a no-op in Release, which IS a behavior
  change vs. the unconditional pre-task `Log()`.

  **Decision:** the legacy `Log()` forwards directly to `LogImpl(LogLevel::Info, ...)`,
  ignoring `LOG_MIN_LEVEL`. This preserves "every existing `Log()` call
  fires in every build" — which is a behavior change vs. having
  LOG_MIN_LEVEL gate everything, but PRD §4.8 explicitly says
  "callers can land without an all-at-once rename" implying the
  forwarder must keep firing. The path forward is to migrate callers
  to `LOG_*` macros over time; the gate is opt-in, not retroactive.

  **Alternative:** make `Log()` route through `LOG_INFO` (and accept
  Release silence). Discuss with user at design review.

- **No-op verification.** PRD requires inspecting Release-build
  disassembly to confirm `LOG_TRACE(...)` compiles to nothing.
  Because `LOG_TRACE` expands to `((void)0)` when the `#if` guard
  excludes it, the varargs are never evaluated. No new code path
  needs to verify this beyond a single spot-check (record in PR
  description).

### 2.9 Unit Test Target (PRD §4.9)

**Decisions:**

- **`BUILD_TESTS` flag** in root `CMakeLists.txt`:

      option(BUILD_TESTS "Build host-side unit tests" OFF)
      if (BUILD_TESTS)
          enable_testing()
          add_subdirectory(tests)
      endif()

  The `tests/` subdir is added AFTER all edit subdirs so target order is
  predictable. `enable_testing()` enables `ctest`.

- **GoogleTest fetch.** `tests/CMakeLists.txt`:

      include(FetchContent)
      FetchContent_Declare(
          googletest
          URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
          URL_HASH SHA256=<known hash>
      )
      set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
      FetchContent_MakeAvailable(googletest)

  Pinned to `v1.14.0`. URL_HASH set for reproducibility.

- **Host vs target architecture.** The matrix builds edit DLLs as
  x86 (32-bit, the Maple ABI). Tests build for the **host**
  architecture (x64 on a modern dev box). Two separate CMake configures
  are required if a developer wants both. The test target uses the
  same compiler/runtime as the matrix builds but does NOT pass
  `-A Win32`. This means `BUILD_TESTS=ON` is intended to be used in a
  separate build directory, not stacked on the matrix configure. The
  PRD acknowledges this ("Tests build for the host architecture").

- **What gets extracted.**
  - **`parse_ini`** → new files `common/parse_ini.h` / `common/parse_ini.cpp`.
    Contents copied (not moved) from `redirect/dllmain.cpp:34–95`
    (the anonymous namespace block containing `TrimLeft`, `TrimRight`,
    `Trim`, `StripComment`, `ParsedINI`, `ParseINI`). The anonymous
    namespace becomes `namespace ms { namespace ini {` (or simply lift
    to the global namespace with a `ParseINI` symbol — TBD by user
    preference; recommend `namespace ms::ini`).
    `redirect/dllmain.cpp` then `#include "parse_ini.h"` and deletes
    its private copy.

    **Log() dependency.** `ParseINI` calls `Log()` four times. The
    host-build test target does NOT link `common/logger.cpp`. Two
    options:
    1. **Inject a `std::function<void(const char*)>` log sink as an
       optional ctor/parameter, defaulting to `nullptr`.** `ParseINI`
       checks `if (sink) sink(msg);` instead of `Log(...)`. The
       `redirect` callsite passes a lambda that wraps `Log`. Tests
       pass `nullptr`. **Recommendation: this option.**
    2. *Compile out the `Log()` calls with `#ifndef GMS_HOST_BUILD`.*
       Rejected — splits the source across two compilation
       behaviors.

    The injectable sink is a small API surface (one extra optional
    parameter on `ParseINI`) and preserves call semantics for
    `redirect`.

  - **`MemEdit` byte-only helpers** → no Win32 dependency. The current
    `MemEdit::PatchNop` and `MemEdit::WriteBytes` both call
    `VirtualProtect` / `WriteProcessMemory`. The pure byte-level
    logic worth testing is:

    - **Building a NOP buffer of length N** — the loop
      `for (i = 0; i < nCount; ++i) bArr[i] = x86NOP;` in `PatchNop`.
    - **Copying source bytes** — trivial wrapping of `memcpy` in
      `WriteBytes`.

    The extracted form is a new TU `common/byte_ops.h` / `common/byte_ops.cpp`
    exposing:

        namespace ms::byte_ops {
            void fill_nop(BYTE* dst, size_t n);
            void copy(BYTE* dst, const BYTE* src, size_t n); // for symmetry; tests assert correctness
        }

    `MemEdit::PatchNop` then calls `ms::byte_ops::fill_nop(bArr, nCount)`
    instead of the inline loop. `MemEdit::WriteBytes` does NOT change
    (the `memcpy` is via `WriteProcessMemory`, not directly). The
    tests cover only `fill_nop` and `copy`, which is sufficient for
    PRD §4.9's "byte copy correctness, NOP fill, boundary on
    zero-length write."

    **Win32 dependency check:** `byte_ops.{h,cpp}` includes only
    `<cstddef>` and `<cstring>`. `BYTE` is a Win32 typedef; for the
    host build it's defined as `using BYTE = unsigned char;` inside a
    small `tests/support/win_types_shim.h`, OR the file uses
    `unsigned char` directly and `MemEdit::PatchNop` does the cast.
    **Recommendation: use `unsigned char` in `byte_ops`** and cast at
    the `MemEdit` callsite. Avoids the Win32 typedef leak.

- **Test target structure.** `tests/CMakeLists.txt` creates:

      add_executable(parse_ini_tests
          test_parse_ini.cpp
          ${CMAKE_SOURCE_DIR}/common/parse_ini.cpp)
      target_include_directories(parse_ini_tests PRIVATE ${CMAKE_SOURCE_DIR}/common)
      target_link_libraries(parse_ini_tests PRIVATE gtest_main)

      add_executable(byte_ops_tests
          test_byte_ops.cpp
          ${CMAKE_SOURCE_DIR}/common/byte_ops.cpp)
      target_include_directories(byte_ops_tests PRIVATE ${CMAKE_SOURCE_DIR}/common)
      target_link_libraries(byte_ops_tests PRIVATE gtest_main)

      include(GoogleTest)
      gtest_discover_tests(parse_ini_tests)
      gtest_discover_tests(byte_ops_tests)

  Two small executables, one per surface. Each compiles the
  extracted .cpp directly (NOT through `common_lib`, which is x86 and
  can't link into the host x64 test). The PRD constraint that "tests
  do not link common_lib as it currently stands" is what motivates
  this duplication.

- **Test cases.** Per PRD §4.9 minimum:
  - `parse_ini`: valid k=v, leading/trailing whitespace, `;` comment,
    `#` comment, duplicate keys (vector-of-values preserved), malformed
    line (no `=`, empty key), missing file (returns `false`).
  - `byte_ops`: `fill_nop(buf, 5)` → all `0x90`, `fill_nop(nullptr,
    0)` → no-op (zero-length boundary), `copy(dst, src, n)` round-trip.

- **CI wiring.** Deferred per PRD §4.9 open question. A `docs/TODO.md`
  entry: *"Wire `BUILD_TESTS=ON` as a separate CI job; today the test
  target is developer-local only."*

**Alternatives considered:**

- *Use Catch2 instead of GoogleTest.* Rejected — PRD specifies
  GoogleTest.
- *Make tests link a host-build variant of `common_lib`.* Rejected —
  would require a `common_lib_host` target with conditional Win32
  shims, doubling the maintenance surface. The narrow extraction
  approach is cheaper.
- *Tests as a separate top-level project (out-of-tree).* Rejected —
  PRD §4.9 puts `tests/` at repo root.

---

## 3. Cross-Cutting Concerns

### 3.1 Build-Order Sensitivity

The TU split (§2.1) changes object-file order on the `bypass-*.dll`
link line. Themida signatures are content-keyed, not order-keyed (the
v0 review confirmed no Themida impact from cosmetic moves), so this
is OK. The `nm` / `dumpbin` audit step in PRD §10 is what we'll use to
verify.

### 3.2 PCH + License Strip Ordering

The license-strip script runs before the PCH wiring takes effect (it
just edits source bytes). No ordering hazard.

### 3.3 Logger + License Strip Ordering

The license-strip script edits `common/logger.h` (deletes the leading
comment block). The logger rewrite (§2.8) also edits `logger.h` /
`logger.cpp`. These should be done in the SAME commit-or-PR — running
strip first cleans up the file, then logger edits land on a clean
header. Plan-writing phase to order accordingly.

### 3.4 `parse_ini` Host-Build Constraints

`common/parse_ini.cpp` uses only:
- `<string>`, `<map>`, `<vector>`, `<fstream>`, `<algorithm>`, `<cctype>`

No Win32 dependency once the `Log()` calls are routed through the
injectable sink. Host-buildable on Linux/macOS too (relevant for any
future cross-platform CI).

### 3.5 `byte_ops` Host-Build Constraints

`common/byte_ops.cpp` uses only `<cstring>` and `<cstddef>`. Pure C++.

### 3.6 Memory Map Changes Affecting `bypass`

Removing `MAJOR_VERSION` / `MINOR_VERSION` from `memory_map.h.in`
(§2.7) requires the bypass TU-split work (§2.1, §2.2) to have its
references rewritten in the SAME commit. The strict ordering is:

1. Land §4.7 rewrite of `bypass/dllmain.cpp` and `common/*.h`
   references (`MAJOR_VERSION` → `BUILD_MAJOR_VERSION`).
2. Land §4.7 deletion of `#define`s in `memory_map.h.in`.
3. Land §4.1 / §4.2 TU split (post-rename, before deletion is fine
   too — the rename is what unblocks).

Or simpler: do §4.7 and §4.1 in the same logical change, since §4.1's
new files inherit the renamed identifiers.

### 3.7 No-Behavior-Change Verification

PRD §10 lists three functional checks. The design records that we
will:

1. Run `dumpbin /symbols` (or `objdump`) on pre- and post-refactor
   `bypass-*.dll` for at least one matrix combo (GMS 83.1) and diff
   the function names. Expected diff: file path metadata may shift; no
   prior function name disappears.
2. Diff the pre- and post-refactor `MainProc` call list (the
   `INITMAPLEHOOK_OR_RETURN` sequence) by hand from source review.
   This is captured in the audit step at the end of the plan.

---

## 4. Risks & Open Questions

### 4.1 Resolved Risks (Recorded for Plan Phase)

- **PCH `REUSE_FROM` force-include**: design accepts the force-include
  in all edit DLLs (§2.4 deviation from PRD §4.4 guideline).
- **Per-installer return-value short-circuit**: design preserves
  pre-refactor failure semantics by having `Install*Hooks()` return
  `BOOL` and `MainProc` check it (§2.1).
- **OnConnect retry budget**: design passes retries by reference
  (§2.2 deviation from PRD §4.2 verbatim signature).
- **Legacy `Log()` always fires**: design forwards directly to
  `LogImpl(Info, ...)`, bypassing `LOG_MIN_LEVEL` (§2.8).
- **`memory_maps/*.cmake` files don't `set(MAJOR_VERSION)` today**:
  PRD §4.7 requirement is a no-op against current state; recorded.

### 4.2 Open Questions for User Review

1. **OnConnect retry budget — pass by ref or reset per helper?**
   Recommendation: pass by ref (preserves behavior).
2. **Legacy `Log()` — bypass `LOG_MIN_LEVEL` or honor it?**
   Recommendation: bypass (preserves "always fires" behavior; macro
   migration is the path to silence).
3. **`namespace ms::ini` for the extracted `ParseINI` — or global?**
   Recommendation: `ms::ini` (small but clear boundary).
4. **`LICENSE` file — delete in this PR or defer?** PRD default is
   delete. Confirm.
5. **PCH `REUSE_FROM` force-include accepted for all edit DLLs?**
   Recommendation: yes (simplifies CMake helper).

---

## 5. Deliverables Summary

When the plan-writing phase consumes this design, the deliverables it
should plan for are:

| # | File / Change | Origin |
|---|---|---|
| 1 | 6 new files under `bypass/` (`bypass_main.cpp`, `socket_hooks.{h,cpp}`, `login_hooks.{h,cpp}`, `security_hooks.{h,cpp}`, `app_hooks.{h,cpp}`, `key_mapped_hooks.{h,cpp}`) + 1 internal header (`socket_hooks_internal.h`) | §2.1 |
| 2 | `bypass/dllmain.cpp` reduced to `DllMain` only | §2.1 |
| 3 | `bypass/CMakeLists.txt` updated `SOURCES` list | §2.1 |
| 4 | `OnConnect` refactor with 3 helpers, no `goto` | §2.2 |
| 5 | Two `RemoveAll()` call deletions | §2.3 |
| 6 | `target_precompile_headers(common_lib ...)` + `REUSE_FROM` in `AddEditDll.cmake` | §2.4 |
| 7 | `pch.h` comment cleanup | §2.4 |
| 8 | `scripts/strip_license_header.py` + repo-wide header removal | §2.5 |
| 9 | `README.md` license badge + section removal | §2.5 |
| 10 | `LICENSE` file deletion | §2.5 |
| 11 | `cmake/GenerateMemoryMap.cmake` + root `CMakeLists.txt` integration | §2.6 |
| 12 | `MAJOR_VERSION` / `MINOR_VERSION` → `BUILD_*` rename (10 call sites) | §2.7 |
| 13 | `memory_map.h.in` deletes `MAJOR_VERSION` / `MINOR_VERSION` `#define`s | §2.7 |
| 14 | `common/logger.h` / `.cpp` rewrite (enum, macros, `LogImpl`, `LevelTag`, truncation marker, legacy `Log()` wrapper) | §2.8 |
| 15 | `build_config` `LOG_MIN_LEVEL` compile def (config-aware) | §2.8 |
| 16 | `common/parse_ini.{h,cpp}` extraction; `redirect/dllmain.cpp` consumes it | §2.9 |
| 17 | `common/byte_ops.{h,cpp}` extraction; `MemEdit::PatchNop` calls `fill_nop` | §2.9 |
| 18 | `tests/CMakeLists.txt` + `tests/test_parse_ini.cpp` + `tests/test_byte_ops.cpp` | §2.9 |
| 19 | `BUILD_TESTS` option + `enable_testing()` in root `CMakeLists.txt` | §2.9 |
| 20 | `docs/TODO.md` entry for CI wiring | §2.9 |

Test acceptance:

- All 5 matrix configures + builds clean (zero new warnings).
- `BUILD_TESTS=ON` host build: `ctest` green.
- Source-level `MainProc` install order diff: zero functional changes.
- `nm` / `dumpbin` symbol audit: no prior hook function disappears.

---

End of design.
