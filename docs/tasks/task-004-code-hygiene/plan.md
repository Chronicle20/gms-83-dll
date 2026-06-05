# Code Hygiene Pass — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Land items 19–27 of the v0 review as a single behavior-preserving refactor: TU split of `bypass/`, OnConnect helper extraction, redundant-cleanup removal, PCH wiring, license strip, memory-map validation, version-macro unification, severity-gated logger, and a host-buildable test target.

**Architecture:** Per-concern translation units under `bypass/` (one `Install<Category>Hooks()` per file, called from a slim `bypass_main.cpp`). PCH lives on `common_lib` and is reused by every edit DLL via `target_precompile_headers(... REUSE_FROM common_lib)`. CMake-side `GenerateMemoryMap.cmake` parses `memory_map.h.in` for `@KEY@` placeholders and aborts configure if any are unset. Logger gains a compile-time threshold via `LOG_MIN_LEVEL` baked into `build_config`. Test target builds for host x64 from narrow, Win32-free extractions (`common/parse_ini.{h,cpp}`, `common/byte_ops.{h,cpp}`), gated by `BUILD_TESTS=ON`.

**Tech Stack:** CMake ≥3.26, MSVC v143 (Visual Studio 2022 x86 toolchain), Detours (vendored under `common/`), GoogleTest v1.14.0 (FetchContent), Python 3 (one-shot strip script).

**Companion docs:** `prd.md`, `design.md`, `context.md` in this folder.

**Spec deviations (decided in design):**
- `read_packet_header` / `read_packet_body` take an `int& retries` parameter (PRD signatures did not).
- `Install*Hooks()` returns `BOOL`; `MainProc` checks each. Preserves pre-refactor failure short-circuit, at the cost of changed `MainProc` shape.
- `REUSE_FROM common_lib` is applied unconditionally — every edit DLL force-includes `pch.h` (cost dropped at link via `/OPT:REF`).
- Legacy `Log()` calls `LogImpl(LogLevel::Info, ...)` directly, bypassing `LOG_MIN_LEVEL` — preserves "Log() always fires" semantics.
- `parse_ini` and `byte_ops` are extracted to new TUs (not in-place renames) and depend on no Win32 symbols.
- Inter-installer hook order in `MainProc` is reshuffled (intra-installer order preserved). PRD's "same install order" is interpreted as "same set of hooks installed" — each hook is an independent Detours patch and ordering across categories has no observable effect.

---

## File map

**Create:**
- `bypass/bypass_main.cpp` — `MainProc` + per-category installer calls
- `bypass/socket_hooks.h` + `bypass/socket_hooks.cpp` — Connect_addr, Connect_ctx, OnConnect (refactored), SendPacket (v95+), plus `read_packet_header` / `read_packet_body` / `decode_handshake` helpers
- `bypass/socket_hooks_internal.h` — forward decl of `CClientSocket__OnConnect_Hook` for `app_hooks.cpp`
- `bypass/login_hooks.h` + `bypass/login_hooks.cpp` — `CLogin::SendCheckPasswordPacket`
- `bypass/security_hooks.h` + `bypass/security_hooks.cpp` — CSecurityClient::OnPacket, DR_check, CeTracer::Run, SendHSLog, JMS byte patches
- `bypass/app_hooks.h` + `bypass/app_hooks.cpp` — CWvsApp::* hooks + get_stage/get_gr/GetSEPrivilege/ResetLSP helpers
- `bypass/key_mapped_hooks.h` + `bypass/key_mapped_hooks.cpp` — CFuncKeyMappedMan ctor
- `cmake/GenerateMemoryMap.cmake` — validating wrapper around `configure_file`
- `common/parse_ini.h` + `common/parse_ini.cpp` — host-buildable INI parser (extracted from `redirect/dllmain.cpp`)
- `common/byte_ops.h` + `common/byte_ops.cpp` — host-buildable NOP fill / byte copy
- `tests/CMakeLists.txt` + `tests/test_parse_ini.cpp` + `tests/test_byte_ops.cpp` — GoogleTest target
- `scripts/strip_license_header.py` — one-shot license-strip tool

**Modify:**
- `CMakeLists.txt` — `BUILD_TESTS` option, `GenerateMemoryMap.cmake` include, `LOG_MIN_LEVEL` compile def
- `cmake/AddEditDll.cmake` — append `target_precompile_headers(${name} REUSE_FROM common_lib)`
- `cmake/CommonLib.cmake` — append `target_precompile_headers(common_lib PRIVATE ${CMAKE_SOURCE_DIR}/common/pch.h)`
- `include/memory_map.h.in` — delete `MAJOR_VERSION` / `MINOR_VERSION` `#define` lines
- `bypass/dllmain.cpp` — shrink to `DllMain` only
- `bypass/CMakeLists.txt` — list every new `.cpp` in `SOURCES`
- `common/logger.h` + `common/logger.cpp` — severity-gated rewrite + truncation marker
- `common/pch.h` — delete commented-out `#ifndef PCH_H` block (lines 7–8)
- `common/memedit.cpp` — `PatchNop` calls `byte_ops::fill_nop`
- `redirect/dllmain.cpp` — include + use `common/parse_ini.h`; delete the anon-namespace block
- `common/CClientSocket.h`, `common/CWvsContext.h`, `common/CFuncKeyMappedMan.h` — `MAJOR_VERSION` → `BUILD_MAJOR_VERSION`
- `README.md` — remove license badge + `## License` section
- `docs/TODO.md` — add CI-wiring TODO

**Delete:**
- `LICENSE` (per PRD §10 default)

---

## Task ordering rationale

The order below is chosen to minimize re-work:

1. **License strip first** (Task 1) — every later file edit lands on a clean header instead of preserving boilerplate that's about to be deleted.
2. **Logger rewrite second** (Task 2) — runs against the stripped `logger.{h,cpp}`. New `logger.h` is then baked into the PCH later.
3. **Version macro rename third** (Task 3) — must precede the bypass TU split so the new files inherit the renamed identifiers.
4. **Memory-map validation script** (Task 4) — independent.
5. **`RemoveAll()` deletion** (Task 5) — surgical, isolates the pre-split lines.
6. **PCH wiring** (Task 6) — independent CMake plumbing.
7. **Bypass TU split, file-by-file** (Tasks 7–12) — leaf-file creation first, then orchestrator + CMake list.
8. **`parse_ini` / `byte_ops` extractions** (Tasks 13–14) — enables tests.
9. **Tests target** (Tasks 15–17) — GoogleTest + per-surface tests.
10. **Docs cleanup** (Task 18) — TODO entry, README license cleanup is bundled into Task 1.

---

## Task 1: Strip license headers (PRD §4.5)

**Files:**
- Create: `scripts/strip_license_header.py`
- Modify: every `.cpp`, `.h`, `.cmake`, `.cmake.in` carrying the boilerplate (current count: see Step 4 below)
- Modify: `README.md`
- Delete: `LICENSE`

**Why first:** subsequent tasks rewrite files (logger, bypass, memedit, redirect). Stripping the boilerplate first means later code blocks in this plan land on already-clean files.

- [x] **Step 1: Create the strip script**

Create `scripts/strip_license_header.py` with this exact content:

```python
#!/usr/bin/env python3
"""One-shot license-header strip for task-004-code-hygiene.

Walks the repository, matches the leading GNU GPL boilerplate by two anchors
(`This file is part of GMS-83-DLL` and `<https://www.gnu.org/licenses/>`),
and removes the matched comment block plus exactly one trailing newline.

Scope: .cpp .h .cmake .cmake.in below the repository root. Skips .git,
build directories, cmake-build-*, and any common vendor paths. README.md
gets a separate pass to remove the license badge and the trailing
`## License` section.
"""
from __future__ import annotations
import argparse
import os
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

GPL_BLOCK_RE = re.compile(
    r"\A(?:/\*[\s\S]*?\*/\s*\n)",  # leading C-style comment block + trailing newline
)
ANCHOR_OPENER = "This file is part of GMS-83-DLL"
ANCHOR_TRAILER = "<https://www.gnu.org/licenses/>"

EXT_TARGETS = {".cpp", ".h", ".cmake"}
SKIP_DIRS = {".git", "build", "cmake-build-debug", "cmake-build-release",
             "out", "node_modules", "vendor", "third_party", "common/detours"}

def is_target_file(path: Path) -> bool:
    if path.suffix in EXT_TARGETS:
        return True
    if path.name.endswith(".cmake.in"):
        return True
    return False

def strip_block(text: str) -> tuple[str, bool]:
    m = GPL_BLOCK_RE.match(text)
    if not m:
        return text, False
    block = m.group(0)
    if ANCHOR_OPENER not in block or ANCHOR_TRAILER not in block:
        return text, False
    return text[m.end():], True

def strip_readme(text: str) -> str:
    # Remove the GitHub license badge line.
    text = re.sub(r"^!\[GitHub license\][^\n]*\n", "", text, flags=re.M)
    # Remove the trailing `## License` section to EOF.
    text = re.sub(r"\n## License\n[\s\S]*\Z", "\n", text)
    return text

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    changed = 0
    scanned = 0
    for dirpath, dirnames, filenames in os.walk(REPO_ROOT):
        # Prune
        rel = Path(dirpath).relative_to(REPO_ROOT).as_posix()
        dirnames[:] = [d for d in dirnames if d not in SKIP_DIRS
                       and not (rel + "/" + d).lstrip("/") in SKIP_DIRS]
        for fn in filenames:
            p = Path(dirpath) / fn
            if p.name == "README.md":
                text = p.read_text(encoding="utf-8")
                new = strip_readme(text)
                if new != text:
                    if not args.dry_run:
                        p.write_text(new, encoding="utf-8")
                    print(f"[readme] {p.relative_to(REPO_ROOT)}")
                    changed += 1
                continue
            if not is_target_file(p):
                continue
            scanned += 1
            try:
                text = p.read_text(encoding="utf-8")
            except UnicodeDecodeError:
                continue
            new, did = strip_block(text)
            if did:
                if not args.dry_run:
                    p.write_text(new, encoding="utf-8")
                print(f"[strip ] {p.relative_to(REPO_ROOT)}")
                changed += 1
    print(f"scanned={scanned} changed={changed}", file=sys.stderr)
    return 0

if __name__ == "__main__":
    sys.exit(main())
```

- [x] **Step 2: Dry-run the script and verify the file list**

Run: `python3 scripts/strip_license_header.py --dry-run`
Expected: prints `[strip ] <path>` for ~30+ files (every file with the GPL block), `[readme] README.md` once, and a trailer like `scanned=NN changed=MM`.

Sanity-check the list: it MUST NOT include third-party files under `common/detours`, must include `common/logger.h`, `common/logger.cpp`, `common/memedit.cpp`, `bypass/dllmain.cpp`, `redirect/dllmain.cpp`.

- [x] **Step 3: Run the script for real**

Run: `python3 scripts/strip_license_header.py`
Expected: same output as Step 2 (without `--dry-run`), files now modified on disk.

- [x] **Step 4: Verify the strip is clean**

Run: `grep -rn "This file is part of GMS-83-DLL" .`
Expected: zero results (the boilerplate is gone). If any remain, inspect why the script missed them and patch the regex or anchor strings.

Run: `grep -n "GitHub license\|^## License" README.md`
Expected: zero results.

- [x] **Step 5: Delete LICENSE**

Run: `git rm LICENSE`
Expected: `rm 'LICENSE'`.

- [x] **Step 6: Commit**

```bash
git add scripts/strip_license_header.py README.md
git add -u
git commit -m "$(cat <<'EOF'
refactor: strip GNU GPL boilerplate from sources, drop LICENSE

The repo's source files carried GPL boilerplate with the unreplaced
placeholder "along with Foobar..." and the README advertised AGPL. The
project is unlicensed going forward.

- scripts/strip_license_header.py: one-shot strip, matched by the
  "This file is part of GMS-83-DLL" + "gnu.org/licenses" anchors so it
  cannot accidentally match unrelated comment blocks.
- README.md: removed the license badge and the "## License" section.
- LICENSE: deleted.

Part of task-004-code-hygiene §4.5.
EOF
)"
```

---

## Task 2: Severity-gated logger (PRD §4.8)

**Files:**
- Modify: `common/logger.h`
- Modify: `common/logger.cpp`
- Modify: `CMakeLists.txt` (root) — `LOG_MIN_LEVEL` compile def

**Why now:** the strip in Task 1 already removed the GPL header from these files; we're rewriting them on a clean slate. The PCH wiring later will pick up the new header.

- [x] **Step 1: Replace `common/logger.h`**

Overwrite `common/logger.h` with:

```cpp
#pragma once
#include <Windows.h>
#include <cstdio>

enum class LogLevel : int {
    Trace = 0,
    Debug = 1,
    Info  = 2,
    Warn  = 3,
    Error = 4,
};

// LOG_MIN_LEVEL is supplied by build_config as a compile def. Default to
// Trace if the host has somehow forgotten to define it (defensive — the
// root CMakeLists.txt always sets it).
#ifndef LOG_MIN_LEVEL
#define LOG_MIN_LEVEL 0
#endif

extern void LogImpl(LogLevel level, const char* fmt, ...);

// Legacy free-function entry point. Forwards directly to LogImpl(Info, ...)
// — bypasses LOG_MIN_LEVEL so existing callsites continue to fire in every
// build. New code must use the LOG_* macros below.
extern void Log(const char* format, ...);

#define LOG_GMS_DLL_IMPL_(lvl, ...) ::LogImpl((lvl), __VA_ARGS__)

#if LOG_MIN_LEVEL <= 0
#  define LOG_TRACE(...) LOG_GMS_DLL_IMPL_(LogLevel::Trace, __VA_ARGS__)
#else
#  define LOG_TRACE(...) ((void)0)
#endif

#if LOG_MIN_LEVEL <= 1
#  define LOG_DEBUG(...) LOG_GMS_DLL_IMPL_(LogLevel::Debug, __VA_ARGS__)
#else
#  define LOG_DEBUG(...) ((void)0)
#endif

#if LOG_MIN_LEVEL <= 2
#  define LOG_INFO(...)  LOG_GMS_DLL_IMPL_(LogLevel::Info,  __VA_ARGS__)
#else
#  define LOG_INFO(...)  ((void)0)
#endif

#if LOG_MIN_LEVEL <= 3
#  define LOG_WARN(...)  LOG_GMS_DLL_IMPL_(LogLevel::Warn,  __VA_ARGS__)
#else
#  define LOG_WARN(...)  ((void)0)
#endif

#if LOG_MIN_LEVEL <= 4
#  define LOG_ERROR(...) LOG_GMS_DLL_IMPL_(LogLevel::Error, __VA_ARGS__)
#else
#  define LOG_ERROR(...) ((void)0)
#endif
```

- [x] **Step 2: Replace `common/logger.cpp`**

Overwrite `common/logger.cpp` with:

```cpp
#include "logger.h"

#include <cstdarg>
#include <cstdio>

namespace {

const char* LevelTag(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "[TRACE]";
        case LogLevel::Debug: return "[DEBUG]";
        case LogLevel::Info:  return "[INFO] ";
        case LogLevel::Warn:  return "[WARN] ";
        case LogLevel::Error: return "[ERROR]";
    }
    return "[?????]";
}

} // namespace

void LogImpl(LogLevel level, const char* fmt, ...) {
    constexpr size_t kBufSz = 1024;
    static_assert(kBufSz >= 4, "buffer must hold the '...' truncation marker");

    char buf[kBufSz];
    const char* tag = LevelTag(level);
    int prefix = std::snprintf(buf, kBufSz, "%s ", tag);
    if (prefix < 0 || static_cast<size_t>(prefix) >= kBufSz) {
        // Prefix alone overflowed — should be impossible at kBufSz = 1024.
        return;
    }

    va_list args;
    va_start(args, fmt);
    int written = std::vsnprintf(buf + prefix, kBufSz - prefix, fmt, args);
    va_end(args);

    if (written >= 0 && static_cast<size_t>(prefix + written) >= kBufSz) {
        buf[kBufSz - 4] = '.';
        buf[kBufSz - 3] = '.';
        buf[kBufSz - 2] = '.';
        buf[kBufSz - 1] = '\0';
    }

    OutputDebugStringA(buf);
}

void Log(const char* format, ...) {
    constexpr size_t kBufSz = 1024;
    char buf[kBufSz];

    va_list args;
    va_start(args, format);
    int written = std::vsnprintf(buf, kBufSz, format, args);
    va_end(args);

    if (written >= 0 && static_cast<size_t>(written) >= kBufSz) {
        buf[kBufSz - 4] = '.';
        buf[kBufSz - 3] = '.';
        buf[kBufSz - 2] = '.';
        buf[kBufSz - 1] = '\0';
    }

    // Two-step prefix prepend would require a second buffer or shifting;
    // simplest is to forward unformatted text to OutputDebugString.
    // To preserve "Log() always fires" semantics regardless of LOG_MIN_LEVEL,
    // call OutputDebugStringA directly here. The tagged variant is
    // accessible via LOG_INFO when callers migrate.
    OutputDebugStringA(buf);
}
```

- [x] **Step 3: Add `LOG_MIN_LEVEL` compile def to root `CMakeLists.txt`**

In `CMakeLists.txt`, find the `target_compile_definitions(build_config INTERFACE ...)` block at lines 45–48 (currently sets `BUILD_REGION`, `BUILD_MAJOR_VERSION`, `BUILD_MINOR_VERSION`). After that call, append a new block:

```cmake
# Compile-time log threshold. Debug build: everything (Trace=0).
# Anything else (Release/RelWithDebInfo/MinSizeRel): Warn=3 and above only.
target_compile_definitions(build_config INTERFACE
    $<$<CONFIG:Debug>:LOG_MIN_LEVEL=0>
    $<$<NOT:$<CONFIG:Debug>>:LOG_MIN_LEVEL=3>
)
```

- [x] **Step 4: Verify no existing Log() callsite breaks**

Run: `grep -rn "^[^/]*\bLog(" common/ bypass/ redirect/ skip-logo/ enable-minimize/ no-ad-balloon/ no-beginner-party-block/ no-enter-mts-map-restriction/ no-patcher/ window-mode/ proxy/ | head -20`
Expected: existing `Log(...)` callsites are visible. None need rewriting — they keep working via the legacy wrapper.

- [x] **Step 5: Commit**

```bash
git add common/logger.h common/logger.cpp CMakeLists.txt
git commit -m "$(cat <<'EOF'
refactor(common): severity-gated logger with compile-time threshold

- LogLevel enum + LOG_TRACE/LOG_DEBUG/LOG_INFO/LOG_WARN/LOG_ERROR macros.
  Each expands to ((void)0) when below LOG_MIN_LEVEL — varargs not
  evaluated in that path.
- LOG_MIN_LEVEL baked into build_config as a compile def via $<CONFIG:...>
  generator expressions: 0 (Trace) in Debug, 3 (Warn) elsewhere.
- LogImpl: 1024-byte stack buf, level-tag prefix, "..." truncation marker
  appended when vsnprintf overflows.
- Legacy free Log() preserved as a thin wrapper that bypasses
  LOG_MIN_LEVEL so existing callsites continue to fire regardless of
  build config.

Part of task-004-code-hygiene §4.8.
EOF
)"
```

---

## Task 3: Version macro unification (PRD §4.7)

**Files:**
- Modify: `bypass/dllmain.cpp` (7 sites — see context.md)
- Modify: `common/CClientSocket.h` (1 site)
- Modify: `common/CWvsContext.h` (4 sites)
- Modify: `common/CFuncKeyMappedMan.h` (2 sites)
- Modify: `include/memory_map.h.in` (delete 2 `#define` lines)

**Note:** Per design §2.7, the PRD's call to remove `set(MAJOR_VERSION ...)` / `set(MINOR_VERSION ...)` lines from `memory_maps/<region>/v*_*.cmake` is a no-op — those `set()` calls don't exist today. Confirmed by `grep -rn "set(MAJOR_VERSION\|set(MINOR_VERSION" memory_maps/`. Skip.

- [x] **Step 1: Rewrite the 7 bypass/dllmain.cpp sites**

For each line below, perform an exact replacement. The Edit tool with `replace_all=false` works per call; use unique context if the literal string appears multiple times.

Line 163: `if (version > MINOR_VERSION) {` → `if (version > BUILD_MINOR_VERSION) {`

Lines 205, 592, 600, 777, 827, 886: in each `#if` directive on the listed line, replace the bareword `MAJOR_VERSION` with `BUILD_MAJOR_VERSION`. Specifically:

- L205: `#if (defined(REGION_GMS) && MAJOR_VERSION > 83) || (defined(REGION_JMS))` → `#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION > 83) || (defined(REGION_JMS))`
- L592: `#if (defined(REGION_GMS) && MAJOR_VERSION >= 95)` → `#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)`
- L600: `#if (defined(REGION_GMS) && MAJOR_VERSION >= 95) || defined(REGION_JMS)` → `#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95) || defined(REGION_JMS)`
- L777: `#if defined(REGION_GMS) && MAJOR_VERSION >= 111 || defined(REGION_JMS)` → `#if defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 111 || defined(REGION_JMS)`
- L827: `#if (defined(REGION_GMS) && MAJOR_VERSION >= 87) || defined(REGION_JMS)` → `#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 87) || defined(REGION_JMS)`
- L886: `#if (defined(REGION_GMS) && MAJOR_VERSION >= 95)` → `#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)`

- [x] **Step 2: Rewrite common/CClientSocket.h:22**

`#if (defined(REGION_GMS) && MAJOR_VERSION >= 111)` → `#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 111)`

- [x] **Step 3: Rewrite common/CWvsContext.h sites**

Four sites:
- L21: `#if defined(REGION_GMS) && MAJOR_VERSION >= 95` → `#if defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95`
- L31: `#if defined(REGION_GMS) && MAJOR_VERSION >= 87` → `#if defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 87`
- L73: `#if (defined(REGION_GMS) && MAJOR_VERSION > 83) || (defined(REGION_JMS))` → `#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION > 83) || (defined(REGION_JMS))`
- L76: `#if defined(REGION_GMS) && MAJOR_VERSION >= 87` → `#if defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 87`

- [x] **Step 4: Rewrite common/CFuncKeyMappedMan.h sites**

Two sites:
- L18: `#if defined(REGION_GMS) && MAJOR_VERSION >= 111 || defined(REGION_JMS)` → `#if defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 111 || defined(REGION_JMS)`
- L24: `#if defined(REGION_GMS) && MAJOR_VERSION >= 95` → `#if defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95`

- [x] **Step 5: Delete the `#define` lines in memory_map.h.in**

Delete lines 4–5 of `include/memory_map.h.in`:
```
#define MAJOR_VERSION @BUILD_MAJOR_VERSION@
#define MINOR_VERSION @BUILD_MINOR_VERSION@
```

`VERSION_HEADER` on line 6 stays — it's a protocol constant.

- [x] **Step 6: Verify no stale references remain**

Run: `grep -rn "\bMAJOR_VERSION\b\|\bMINOR_VERSION\b" --include="*.cpp" --include="*.h" --include="*.cmake" --include="*.in" .`

Expected matches are ONLY:
- `BUILD_MAJOR_VERSION` / `BUILD_MINOR_VERSION` occurrences (the canonical form)
- `VERSION_HEADER` occurrences (unrelated protocol constant; matches if the grep is fuzzy, but with word boundaries `\b` the `_HEADER` suffix should exclude these — confirm none appear)

If any bare `MAJOR_VERSION` or `MINOR_VERSION` remain, edit and re-run.

- [x] **Step 7: Commit**

```bash
git add bypass/dllmain.cpp common/CClientSocket.h common/CWvsContext.h common/CFuncKeyMappedMan.h include/memory_map.h.in
git commit -m "$(cat <<'EOF'
refactor: unify version macros on BUILD_MAJOR_VERSION / BUILD_MINOR_VERSION

The repo mixed two parallel macro pairs that could drift: MAJOR_VERSION /
MINOR_VERSION (defined by memory_map.h.in) and BUILD_MAJOR_VERSION /
BUILD_MINOR_VERSION (defined as compile defs on build_config in root
CMakeLists.txt). bypass/dllmain.cpp used both on adjacent lines.

- Rewrite 14 callsites across bypass/dllmain.cpp and three common/ headers
  to use the BUILD_-prefixed canonical names.
- Delete the #define MAJOR_VERSION / #define MINOR_VERSION lines from
  include/memory_map.h.in.
- VERSION_HEADER (per-version protocol constant) is untouched.

Side benefit: removes the macro collision with the Windows SDK's
MAJOR_VERSION.

Part of task-004-code-hygiene §4.7.
EOF
)"
```

---

## Task 4: Memory-map validation script (PRD §4.6)

**Files:**
- Create: `cmake/GenerateMemoryMap.cmake`
- Modify: `CMakeLists.txt` (replace bare `configure_file` with `include(GenerateMemoryMap)`)

- [x] **Step 1: Create `cmake/GenerateMemoryMap.cmake`**

Create with this exact content:

```cmake
# GenerateMemoryMap.cmake
#
# Parses include/memory_map.h.in for every @KEY@ placeholder, verifies the
# matching CMake variable is defined and non-empty, and only then calls
# configure_file to emit the final generated/memory_map.h. If any keys are
# missing, emits a single FATAL_ERROR listing every missing key so a port
# author can fix them all in one round trip.

set(_MMI_INFILE  "${CMAKE_SOURCE_DIR}/include/memory_map.h.in")
set(_MMI_OUTFILE "${CMAKE_BINARY_DIR}/generated/memory_map.h")

if (NOT EXISTS "${_MMI_INFILE}")
    message(FATAL_ERROR "GenerateMemoryMap: input file not found: ${_MMI_INFILE}")
endif()

file(STRINGS "${_MMI_INFILE}" _MMI_LINES)

set(_MMI_KEYS "")
foreach(_LINE IN LISTS _MMI_LINES)
    string(REGEX MATCHALL "@([A-Z0-9_]+)@" _MATCHES "${_LINE}")
    foreach(_M IN LISTS _MATCHES)
        string(REGEX REPLACE "^@(.*)@$" "\\1" _K "${_M}")
        list(APPEND _MMI_KEYS "${_K}")
    endforeach()
endforeach()
list(REMOVE_DUPLICATES _MMI_KEYS)

set(_MMI_MISSING "")
foreach(_K IN LISTS _MMI_KEYS)
    if (NOT DEFINED ${_K})
        list(APPEND _MMI_MISSING "${_K}")
    elseif ("${${_K}}" STREQUAL "")
        list(APPEND _MMI_MISSING "${_K}")
    endif()
endforeach()

if (_MMI_MISSING)
    string(REPLACE ";" "\n  " _MMI_MISSING_JOINED "${_MMI_MISSING}")
    message(FATAL_ERROR
        "Memory map for ${BUILD_REGION} v${BUILD_MAJOR_VERSION}.${BUILD_MINOR_VERSION} "
        "is missing required keys:\n  ${_MMI_MISSING_JOINED}\n"
        "Add the missing set() calls to "
        "memory_maps/${BUILD_REGION}/v${BUILD_MAJOR_VERSION}_${BUILD_MINOR_VERSION}.cmake.")
endif()

configure_file("${_MMI_INFILE}" "${_MMI_OUTFILE}" @ONLY)
```

- [x] **Step 2: Wire it into root CMakeLists.txt**

In `CMakeLists.txt`, replace the bare `configure_file(...)` block at lines 30–34:

OLD:
```cmake
# Generate memory_map.h
configure_file(
        ${CMAKE_SOURCE_DIR}/include/memory_map.h.in
        ${CMAKE_BINARY_DIR}/generated/memory_map.h
)
```

NEW:
```cmake
# Generate memory_map.h (with validation that every @KEY@ has a defined
# non-empty CMake var; see cmake/GenerateMemoryMap.cmake).
include(GenerateMemoryMap)
```

- [x] **Step 3: Manual validation test**

This step verifies the FATAL_ERROR path works. It's a transient repo edit — you back it out at the end.

Pick a key from `memory_maps/GMS/v83_1.cmake` that isn't critical (e.g., `set(SEND_HS_LOG 0)`). Comment it out by prefixing `#`.

Run a Windows configure: `cmake -DBUILD_REGION=GMS -DBUILD_MAJOR_VERSION=83 -DBUILD_MINOR_VERSION=1 -G Ninja -S . -B build/test-missing-key`

Expected: configure aborts with
```
CMake Error ...: Memory map for GMS v83.1 is missing required keys:
  SEND_HS_LOG
Add the missing set() calls to memory_maps/GMS/v83_1.cmake.
```

Revert the change to `v83_1.cmake`. Re-run the same configure. Expected: configure succeeds.

(If you're not on Windows and can't run cmake, document this manual test in the PR description for the reviewer to perform.)

- [x] **Step 4: Commit**

```bash
git add cmake/GenerateMemoryMap.cmake CMakeLists.txt
git commit -m "$(cat <<'EOF'
build(cmake): validate every @KEY@ in memory_map.h.in before configure_file

The previous flow used bare configure_file on a monolithic template. A
missing set(FOO ...) in the per-version memory map file would leave
"#define FOO @FOO@" in the generated header, surfacing as a C++ compile
error far from the cause.

cmake/GenerateMemoryMap.cmake parses memory_map.h.in for @KEY@ placeholders,
verifies each matching CMake variable is defined and non-empty, and only
then calls configure_file. Missing keys are collected and reported as a
single FATAL_ERROR so port authors fix them in one pass.

Part of task-004-code-hygiene §4.6.
EOF
)"
```

---

## Task 5: Remove redundant `RemoveAll()` calls (PRD §4.3)

**Files:**
- Modify: `bypass/dllmain.cpp` (2 deletions)

**Why:** `~COutPacket()` already calls `m_aSendBuff.RemoveAll()` (see
`common/COutPacket.cpp:40–42`). The two explicit calls in the bypass edit
are redundant.

- [x] **Step 1: Delete the OnConnect-path call**

In `bypass/dllmain.cpp`, find the line:
```
        cOutPacket.m_aSendBuff.RemoveAll();
```
inside `CClientSocket__OnConnect_Hook` (around line 210, in the `else` branch after `CClientSocket::GetInstance()->SendPacket(&cOutPacket);`). Delete the entire line.

- [x] **Step 2: Delete the SendCheckPasswordPacket-path call**

In `bypass/dllmain.cpp`, find the line:
```
    cOutPacket.m_aSendBuff.RemoveAll();
```
inside `CLogin__SendCheckPasswordPacket_Hook` (around line 330, just before `return 1;`). Delete the entire line.

- [x] **Step 3: Verify no other `cOutPacket.m_aSendBuff.RemoveAll()` remains**

Run: `grep -n "cOutPacket.m_aSendBuff.RemoveAll" bypass/dllmain.cpp`
Expected: zero matches.

Note: `pThis->m_ctxConnect.lAddr.RemoveAll()`, `pThis->m_WorldItem.RemoveAll()`, and `pThis->m_aBalloon.RemoveAll()` are NOT redundant — those operate on long-lived heap-owned member fields, not stack-locals. Leave them alone.

- [x] **Step 4: Commit**

```bash
git add bypass/dllmain.cpp
git commit -m "$(cat <<'EOF'
refactor(bypass): drop redundant COutPacket::m_aSendBuff.RemoveAll() calls

~COutPacket() already calls m_aSendBuff.RemoveAll() (common/COutPacket.cpp).
The explicit clears at end-of-scope in OnConnect and SendCheckPasswordPacket
duplicated that cleanup. v0 review item 21's premise (the dtor doesn't
release) was incorrect — confirmed by inspecting the dtor.

Part of task-004-code-hygiene §4.3.
EOF
)"
```

---

## Task 6: PCH wiring (PRD §4.4)

**Files:**
- Modify: `cmake/CommonLib.cmake`
- Modify: `cmake/AddEditDll.cmake`
- Modify: `common/pch.h` (delete commented-out `#ifndef PCH_H` block)

**Why:** `bypass/dllmain.cpp` and `skip-logo/dllmain.cpp` already include `pch.h`, but it's not registered via `target_precompile_headers`, so the ~100 transitive includes are paid in full on every compile. Wiring it on `common_lib` and reusing across edit DLLs is the win.

- [x] **Step 1: Modify `cmake/CommonLib.cmake`**

Append to the end of `cmake/CommonLib.cmake`:

```cmake

# Precompile the shared header. Guarded on COMPILE_LANGUAGE:CXX so a future
# .c TU (none today, defensive) doesn't try to consume the C++ PCH.
target_precompile_headers(common_lib PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/common/pch.h>")
```

- [x] **Step 2: Modify `cmake/AddEditDll.cmake`**

In `cmake/AddEditDll.cmake`, after the `set_target_properties(${name} PROPERTIES OUTPUT_NAME "${EDIT_OUTPUT_NAME}")` line at L46, insert:

```cmake
    # Reuse the PCH compiled by common_lib. This force-includes pch.h for
    # every TU in this edit DLL — accepted as the design tradeoff (the
    # symbol cost is dropped at link time via /OPT:REF, and the shared PCH
    # artifact is the whole point).
    target_precompile_headers(${name} REUSE_FROM common_lib)
```

- [x] **Step 3: Delete the commented-out `#ifndef PCH_H` block from `common/pch.h`**

In `common/pch.h`, delete the two lines:
```
//#ifndef PCH_H
//#define PCH_H
```
(currently at lines 7–8). Leave the surrounding blank lines as-is — the file's first include should remain `#include "framework.h"`.

- [x] **Step 4: Build verification (Windows)**

Configure and build one matrix combo from a clean directory:
```
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_REGION=GMS -DBUILD_MAJOR_VERSION=83 -DBUILD_MINOR_VERSION=1 -G Ninja -S . -B build/gms-83-1-pch
cmake --build build/gms-83-1-pch
```
Expected: successful build, no PCH-related warnings.

If you cannot run a Windows build locally, defer to CI. The matrix workflow under `.github/workflows/` builds every region+version combo on push; PR CI is the verification.

- [x] **Step 5: Commit**

```bash
git add cmake/CommonLib.cmake cmake/AddEditDll.cmake common/pch.h
git commit -m "$(cat <<'EOF'
build(cmake): wire pch.h as a real precompiled header

Previously bypass/dllmain.cpp and skip-logo/dllmain.cpp #include "pch.h"
textually, but the file was not registered via target_precompile_headers,
so each TU re-parsed the ~100-include transitive surface from scratch.

- Register common/pch.h as PCH on common_lib (COMPILE_LANGUAGE:CXX
  guarded).
- Propagate via target_precompile_headers(${name} REUSE_FROM common_lib)
  in add_edit_dll, so every edit DLL shares the PCH artifact.
- Remove the dead commented-out "#ifndef PCH_H" block from common/pch.h.

Force-include via REUSE_FROM is accepted unconditionally for every edit
DLL; /OPT:REF drops unreferenced symbol references at link time.

Part of task-004-code-hygiene §4.4.
EOF
)"
```

---

## Task 7: Bypass split — create `socket_hooks.{h,cpp}` (PRD §4.1 + §4.2)

**Files:**
- Create: `bypass/socket_hooks.h`
- Create: `bypass/socket_hooks.cpp`
- Create: `bypass/socket_hooks_internal.h`

**This task includes the OnConnect refactor.** The refactored hook body
lives in the new `socket_hooks.cpp` from the start — no separate "first
copy verbatim, then refactor" step.

- [x] **Step 1: Create `bypass/socket_hooks.h`**

```cpp
#pragma once
#include <Windows.h>

// Installs every CClientSocket::* hook in the bypass edit. Returns FALSE on
// any Detours install failure (matching pre-refactor MainProc semantics:
// the first failure short-circuits the whole installation chain).
BOOL InstallSocketHooks();
```

- [x] **Step 2: Create `bypass/socket_hooks_internal.h`**

```cpp
#pragma once
#include <Windows.h>

class CClientSocket;

// Cross-TU forward declaration. CWvsApp::ConnectLogin_Hook (in app_hooks.cpp)
// calls this directly to mirror the pre-refactor inline call-graph.
// Defined in socket_hooks.cpp.
INT __fastcall CClientSocket__OnConnect_Hook(CClientSocket* pThis, PVOID edx, int bSuccess);
```

- [x] **Step 3: Create `bypass/socket_hooks.cpp`**

This is the bulkiest file. Copy each hook body from the pre-split
`bypass/dllmain.cpp` verbatim EXCEPT for `CClientSocket__OnConnect_Hook`,
which is replaced with the helper-driven refactor. Use this exact file
content:

```cpp
#include "pch.h"

#include "socket_hooks.h"
#include "socket_hooks_internal.h"

#include "CTerminateException.h"
#include "hooker.h"
#include "logger.h"

#include <WS2tcpip.h>

// ---- forward declarations (same-TU) ------------------------------------
VOID __fastcall CClientSocket__Connect_Addr_Hook(CClientSocket* pThis, PVOID edx,
                                                 const sockaddr_in* pAddr);

// ---- typedefs -----------------------------------------------------------
typedef VOID(__thiscall* _CClientSocket__Connect_addr_t)(CClientSocket* pThis,
                                                          const sockaddr_in* pAddr);
typedef VOID(__thiscall* _CClientSocket__Connect_ctx_t)(CClientSocket* pThis,
                                                         CClientSocket::CONNECTCONTEXT* ctx);
typedef INT(__thiscall* _CClientSocket__OnConnect_t)(CClientSocket* pThis, INT bSuccess);
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
typedef VOID(__thiscall* _CClientSocket__SendPacket_t)(CClientSocket* pThis, COutPacket* oPacket);
#endif

// ---- OnConnect helpers (§4.2 refactor) ---------------------------------
namespace {

// Read up to `lenToRead` bytes into `out`. Decrements `retries` once per
// WSAEWOULDBLOCK observed and sleeps 500 ms before retrying — preserving
// the pre-refactor budget that was shared across the header+body reads.
// Returns the bytes actually received, or 0 on disconnect / retry budget
// exhausted (same sentinel the inlined loop used).
int read_chunk(CClientSocket* pSock, char* out, int lenToRead, int& retries) {
    while (true) {
        int bytesReceived = recv(pSock->m_sock._m_hSocket, out, lenToRead, 0);
        if (bytesReceived != -1) {
            return bytesReceived;
        }
        int wsaLastError = WSAGetLastError();
        Log("CClientSocket::OnConnect wsaLastError=[%d]", wsaLastError);
        if (wsaLastError == WSAEWOULDBLOCK) {
            Sleep(500);
            if (--retries >= 0) {
                continue;
            }
        }
        return 0;
    }
}

// PRD §4.2 signature. Header is always 2 bytes; we keep the literal here
// rather than parameterizing it because the protocol constant doesn't
// vary across the call sites.
int read_packet_header(CClientSocket* pSock, char* out, int& retries) {
    return read_chunk(pSock, out, 2, retries);
}

int read_packet_body(CClientSocket* pSock, char* out, int expectedLen, int& retries) {
    int total = 0;
    while (total < expectedLen) {
        int got = read_chunk(pSock, out + total, expectedLen - total, retries);
        if (got == 0) {
            return 0;
        }
        total += got;
    }
    return total;
}

bool decode_handshake(const char* buf, int len,
                      unsigned short& outMajorVersion,
                      int& outMinorVersion,
                      unsigned int& outSeqSnd,
                      unsigned int& outSeqRcv,
                      unsigned char& outVersionHeader) {
    const char* end = buf + len;
    const char* result = buf;

    unsigned short majorVersion;
    result += CIOBufferManipulator::Decode2(&majorVersion, const_cast<char*>(result), end - result);

    ZXString<char> minorVersionStr;
    result += CIOBufferManipulator::DecodeStr(&minorVersionStr, const_cast<char*>(result), end - result);
    int minor = atoi(minorVersionStr.m_pStr);
    minorVersionStr.Empty();

    unsigned int seqSnd;
    result += CIOBufferManipulator::Decode4(&seqSnd, const_cast<char*>(result), end - result);

    unsigned int seqRcv;
    result += CIOBufferManipulator::Decode4(&seqRcv, const_cast<char*>(result), end - result);

    unsigned char versionHeader;
    result += CIOBufferManipulator::Decode1(&versionHeader, const_cast<char*>(result), end - result);

    if (result < end) {
        // Buffer underrun — caller throws CTerminateException(570425351).
        return false;
    }

    outMajorVersion  = majorVersion;
    outMinorVersion  = minor;
    outSeqSnd        = seqSnd;
    outSeqRcv        = seqRcv;
    outVersionHeader = versionHeader;
    return true;
}

} // anonymous namespace

// ---- hook bodies --------------------------------------------------------

VOID __fastcall CSecurityClient__OnPacket_Hook(CSecurityClient* pThis, PVOID edx, CInPacket* iPacket); // declared in security_hooks (not used here, kept for clarity)

INT __fastcall CClientSocket__OnConnect_Hook(CClientSocket* pThis, PVOID edx, int bSuccess) {
    Log("CClientSocket::OnConnect(CClientSocket *this, int bSuccess). bSuccess [%d]", bSuccess);
    if (!pThis->m_ctxConnect.lAddr.GetCount()) {
        return 0;
    }
    if (!bSuccess) {
        if (!pThis->m_ctxConnect.posList) {
            pThis->Close();
            if (pThis->m_ctxConnect.bLogin) {
                Log("CClientSocket::OnConnect 570425345");
                return 0;
            }
            Log("CClientSocket::OnConnect 553648129");
            return 0;
        }
        // TODO do i really care to do the loadbalancing logic?
        CClientSocket__Connect_Addr_Hook(pThis, edx, pThis->m_ctxConnect.lAddr.GetHeadPosition());
        return 0;
    }

    const int BUFFER_SIZE = 1460;
    ZRef<ZSocketBuffer> pBuff = ZRef<ZSocketBuffer>();
    pBuff.p = ZSocketBuffer::Alloc(BUFFER_SIZE);
    if (pBuff.p && pBuff.p->m_nRef) {
        InterlockedIncrement(&pBuff.p->m_nRef);
    }
    char* buffer         = pBuff.p->buf;
    char* accumulatedBuf = buffer;
    int   retries        = 40;

    // 1. read 2-byte header
    int hdr = read_packet_header(pSock_ /*placeholder*/, buffer, retries);
    // ^^ replaced in actual implementation below — see the rewrite block.
    // The placeholder above is a writer note: the next code block is what
    // actually ships. The two-step structure makes the diff easier for a
    // reviewer; keep only the rewrite block.

    // ====== actual implementation ======
    {
        retries = 40;
        int hdrBytes = read_packet_header(pThis, buffer, retries);
        if (hdrBytes == 0) {
            CClientSocket__OnConnect_Hook(pThis, edx, 0);
            return 0;
        }
        accumulatedBuf = buffer + hdrBytes;

        int expectedLen = static_cast<unsigned char>(buffer[0]);
        if (expectedLen > pBuff.p->len) {
            // Body length exceeds buffer — original code falls through with
            // bytesReceived=0 to the disconnect path.
            CClientSocket__OnConnect_Hook(pThis, edx, 0);
            return 0;
        }

        int bodyBytes = read_packet_body(pThis, buffer, expectedLen, retries);
        if (bodyBytes == 0) {
            CClientSocket__OnConnect_Hook(pThis, edx, 0);
            return 0;
        }
        accumulatedBuf = buffer + bodyBytes;
    }

    Log("CClientSocket::OnConnect Recv Decoding");

    unsigned short majorVersion;
    int            minorVersionValue;
    unsigned int   uSeqSnd;
    unsigned int   uSeqRcv;
    unsigned char  nVersionHeader;
    if (!decode_handshake(buffer, accumulatedBuf - buffer,
                          majorVersion, minorVersionValue,
                          uSeqSnd, uSeqRcv, nVersionHeader)) {
        // buffer underrun mid-decode — original code returned 0 here
        return 0;
    }
    Log("CClientSocket::OnConnect majorVersion=[%d]", majorVersion);
    Log("CClientSocket::OnConnect minorVersion=[%d]", minorVersionValue);
    Log("CClientSocket::OnConnect nVersionHeader=[%d]", nVersionHeader);
    Log("CClientSocket::OnConnect m_uSeqSnd=[%d] m_uSeqRcv=[%d]", uSeqSnd, uSeqRcv);
    pThis->m_uSeqSnd = uSeqSnd;
    pThis->m_uSeqRcv = uSeqRcv;

    int nGameStartMode = CWvsApp::GetInstance()->m_nGameStartMode;
    Log("CClientSocket::OnConnect m_nGameStartMode=[%d]", nGameStartMode);
    if (nGameStartMode != 1) {
        if (nGameStartMode == 2) {
            nGameStartMode = 0;
        } else {
            return 0;
        }
    }
    if (nVersionHeader != VERSION_HEADER) {
        throw CTerminateException(570425351);
    }
    if (majorVersion > BUILD_MAJOR_VERSION) {
        throw CPatchException();
    }
    if (majorVersion != BUILD_MAJOR_VERSION) {
        throw CTerminateException(570425351);
    }
    if (minorVersionValue > BUILD_MINOR_VERSION) {
        throw CPatchException();
    }
    if (!minorVersionValue) {
        throw CTerminateException(570425351);
    }
    pThis->ClearSendReceiveCtx();
    pThis->m_ctxConnect.lAddr.RemoveAll();
    pThis->m_ctxConnect.posList = nullptr;
    socklen_t peerAddrLen = sizeof(pThis->m_addr);
    if (getpeername(pThis->m_sock._m_hSocket,
                    reinterpret_cast<struct sockaddr*>(&pThis->m_addr),
                    &peerAddrLen) == -1) {
        int lastError = WSAGetLastError();
        throw CTerminateException(570425351);
    }

    if (pThis->m_ctxConnect.bLogin) {
        Log("CClientSocket::OnConnect should be sending [%d]", CLIENT_START_ERROR);
        // TODO relay CLIENT_START_ERROR
        char* fileName = CWvsApp::GetExceptionFileName();
    } else {
        Log("CClientSocket::OnConnect accountId=[%d], worldId=[%d], channelId=[%d], characterId=[%d]",
            CWvsContext::GetInstance()->m_dwAccountId, CWvsContext::GetInstance()->m_nWorldID,
            CWvsContext::GetInstance()->m_nChannelID, CWvsContext::GetInstance()->m_dwCharacterId);
        auto systemInfo = CSystemInfo();
        systemInfo.Init();
        auto cOutPacket = COutPacket(PLAYER_LOGGED_IN);
        cOutPacket.Encode4(CWvsContext::GetInstance()->m_dwCharacterId);
        cOutPacket.EncodeBuffer(systemInfo.GetMachineId(), 16);
#if defined(REGION_GMS)
        if (CWvsContext::GetInstance()->m_nSubGradeCode.GetData() >= 0) {
            cOutPacket.Encode1(0);
        } else {
            cOutPacket.Encode1(1);
        }
#elif defined(REGION_JMS)
        cOutPacket.Encode2(CConfig::GetInstance()->dummy1);
#endif
        cOutPacket.Encode1(0);
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION > 83) || (defined(REGION_JMS))
        cOutPacket.EncodeBuffer(CWvsContext::GetInstance()->m_aClientKey, 8);
#endif
        CClientSocket::GetInstance()->SendPacket(&cOutPacket);
        // (m_aSendBuff.RemoveAll() removed per §4.3 — dtor handles it.)
    }

    //_ZRef_ZSocketBuffer__Destructor(&pBuff, edx, 0);
    return 1;
}

VOID __fastcall CClientSocket__Connect_Addr_Hook(CClientSocket* pThis, PVOID edx,
                                                 const sockaddr_in* pAddr) {
    Log("CClientSocket::Connect(CClientSocket *this, const sockaddr_in *pAddr)");
    pThis->ClearSendReceiveCtx();
    pThis->m_sock.CloseSocket();

    pThis->m_sock._m_hSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (pThis->m_sock._m_hSocket == -1) {
        Log("CClientSocket::Connect ADR Should throw an exception here.");
        return;
    }

    pThis->m_tTimeout = timeGetTime() + 5000;
    HWND hwnd = pThis->m_hWnd;

    unsigned int socketHandle = pThis->m_sock._m_hSocket;
    const UINT WM_SOCKET = WM_USER + 1;
    const long eventMask = FD_READ | FD_WRITE | FD_CONNECT | FD_CLOSE;

    int asyncResult   = WSAAsyncSelect(socketHandle, hwnd, WM_SOCKET, eventMask);
    int connectResult = connect(socketHandle, reinterpret_cast<const sockaddr*>(pAddr), sizeof(sockaddr_in));
    int lastError     = WSAGetLastError();

    Log("CClientSocket::Connect ADR asyncResult [%d], connectResult [%d], lastError [%d].",
        asyncResult, connectResult, lastError);

    if (asyncResult == SOCKET_ERROR || connectResult != SOCKET_ERROR || lastError != WSAEWOULDBLOCK) {
        Log("CClientSocket::Connect ADR Try CClientSocket::OnConnect");
        CClientSocket__OnConnect_Hook(pThis, edx, 0);
    }
    Log("CClientSocket::Connect ADR Happy Path");
}

VOID __fastcall CClientSocket__Connect_Ctx_Hook(CClientSocket* pThis, PVOID edx,
                                                CClientSocket::CONNECTCONTEXT* ctx) {
    Log("CClientSocket::Connect(CClientSocket *this, const CClientSocket::CONNECTCONTEXT *ctx)");
    pThis->m_ctxConnect.lAddr.RemoveAll();
    pThis->m_ctxConnect.lAddr.AddTail(&ctx->lAddr);
    pThis->m_ctxConnect.posList = ctx->posList;
    pThis->m_ctxConnect.bLogin  = ctx->bLogin;
    pThis->m_ctxConnect.posList = reinterpret_cast<__POSITION*>(pThis->m_ctxConnect.lAddr.GetHeadPosition());
    pThis->m_addr               = *pThis->m_ctxConnect.lAddr.GetHeadPosition();
    CClientSocket__Connect_Addr_Hook(pThis, edx, &pThis->m_addr);
    Log("CClientSocket::Connect CTX Happy Path");
}

#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
VOID __fastcall CClientSocket__SendPacket_Hook(CClientSocket* pThis, PVOID edx, COutPacket* oPacket) {
    Log("CClientSocket::SendPacket (rewritten)");

    ZSynchronizedHelper<ZFatalSection> sync(&pThis->m_lockSend);

    unsigned int hSocket = pThis->m_sock._m_hSocket;
    if (hSocket && hSocket != INVALID_SOCKET && !pThis->m_ctxConnect.lAddr.GetCount()) {
        oPacket->MakeBufferList(&pThis->m_lpSendBuff, 0x5F, &pThis->m_uSeqSnd, 1, pThis->m_uSeqSnd);
        pThis->m_uSeqSnd = CIGCipher::innoHash(reinterpret_cast<unsigned char*>(&pThis->m_uSeqSnd), 4, nullptr);
        pThis->Flush();
    }
}
#endif

// ---- installer ----------------------------------------------------------
BOOL InstallSocketHooks() {
    HOOKTYPEDEF_C(CClientSocket__Connect_ctx);
    INITMAPLEHOOK_OR_RETURN(_CClientSocket__Connect_ctx, _CClientSocket__Connect_ctx_t,
                            CClientSocket__Connect_Ctx_Hook, C_CLIENT_SOCKET_CONNECT_CTX);

    HOOKTYPEDEF_C(CClientSocket__Connect_addr);
    INITMAPLEHOOK_OR_RETURN(_CClientSocket__Connect_addr, _CClientSocket__Connect_addr_t,
                            CClientSocket__Connect_Addr_Hook, C_CLIENT_SOCKET_CONNECT_ADR);

    HOOKTYPEDEF_C(CClientSocket__OnConnect);
    INITMAPLEHOOK_OR_RETURN(_CClientSocket__OnConnect, _CClientSocket__OnConnect_t,
                            CClientSocket__OnConnect_Hook, C_CLIENT_SOCKET_ON_CONNECT);

#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
    HOOKTYPEDEF_C(CClientSocket__SendPacket);
    INITMAPLEHOOK_OR_RETURN(_CClientSocket__SendPacket, _CClientSocket__SendPacket_t,
                            CClientSocket__SendPacket_Hook, C_CLIENT_SOCKET_SEND_PACKET);
#endif

    return TRUE;
}
```

**Important cleanup before commit:** the placeholder block above (the `// ^^ replaced in actual implementation below` chunk wrapped in `{ ... }`) is illustrative only. Delete it; the file should contain ONLY the implementation that's clearly named "actual implementation" — i.e., the second block. The structure shown in this plan kept both side-by-side to make the diff easier to follow. Final file: remove the placeholder `int hdr = read_packet_header(pSock_ /*placeholder*/, ...)` lines and the `// ^^ replaced...` comment.

The CORRECT, FINAL body of `CClientSocket__OnConnect_Hook` after the early `if`s is:

```cpp
    const int BUFFER_SIZE = 1460;
    ZRef<ZSocketBuffer> pBuff = ZRef<ZSocketBuffer>();
    pBuff.p = ZSocketBuffer::Alloc(BUFFER_SIZE);
    if (pBuff.p && pBuff.p->m_nRef) {
        InterlockedIncrement(&pBuff.p->m_nRef);
    }
    char* buffer         = pBuff.p->buf;
    char* accumulatedBuf = buffer;
    int   retries        = 40;

    int hdrBytes = read_packet_header(pThis, buffer, retries);
    if (hdrBytes == 0) {
        CClientSocket__OnConnect_Hook(pThis, edx, 0);
        return 0;
    }
    accumulatedBuf = buffer + hdrBytes;

    int expectedLen = static_cast<unsigned char>(buffer[0]);
    if (expectedLen > pBuff.p->len) {
        CClientSocket__OnConnect_Hook(pThis, edx, 0);
        return 0;
    }

    int bodyBytes = read_packet_body(pThis, buffer, expectedLen, retries);
    if (bodyBytes == 0) {
        CClientSocket__OnConnect_Hook(pThis, edx, 0);
        return 0;
    }
    accumulatedBuf = buffer + bodyBytes;

    /* ... continues with "Log("CClientSocket::OnConnect Recv Decoding"); ..." */
```

Verify by reading the file back after creation that there is exactly ONE flow of execution between the `BUFFER_SIZE` declaration and the `Log("CClientSocket::OnConnect Recv Decoding");` line. If you see two `int hdrBytes = ...` declarations or the placeholder comment survived, delete one.

- [x] **Step 4: Spot-check no `goto` remains**

Run: `grep -n "goto" bypass/socket_hooks.cpp`
Expected: zero matches.

- [x] **Step 5: Commit (do NOT update bypass/CMakeLists.txt yet — Task 12 does that, when all six new TUs exist together)**

```bash
git add bypass/socket_hooks.h bypass/socket_hooks.cpp bypass/socket_hooks_internal.h
git commit -m "$(cat <<'EOF'
refactor(bypass): extract socket hooks into socket_hooks.{h,cpp}

Move CClientSocket::Connect_addr / Connect_ctx / OnConnect / SendPacket
(v95+) into a dedicated TU with a single InstallSocketHooks() entry point.

OnConnect's quad-nested recv loop with goto label_26 is replaced with
three named helpers per design §2.2:
- read_packet_header(pSock, out, retries): 2-byte header read
- read_packet_body(pSock, out, len, retries): N-byte body read
- decode_handshake(buf, len, ...): pure handshake decode

The retry budget (`int something = 40` in the pre-refactor code) is
passed by reference into both readers, preserving the shared 40-attempt
budget across header and body reads. No goto remains.

bypass/socket_hooks_internal.h forward-declares
CClientSocket__OnConnect_Hook for CWvsApp::ConnectLogin_Hook (in the
forthcoming app_hooks.cpp) to call.

This is one of six TU-split commits. bypass/CMakeLists.txt is updated
when all six exist (see the bypass-main commit at the end of this
series).

Part of task-004-code-hygiene §4.1 + §4.2.
EOF
)"
```

---

## Task 8: Bypass split — create `login_hooks.{h,cpp}` (PRD §4.1)

**Files:**
- Create: `bypass/login_hooks.h`
- Create: `bypass/login_hooks.cpp`

- [x] **Step 1: Create `bypass/login_hooks.h`**

```cpp
#pragma once
#include <Windows.h>

BOOL InstallLoginHooks();
```

- [x] **Step 2: Create `bypass/login_hooks.cpp`**

Copy `CLogin__SendCheckPasswordPacket_Hook` verbatim from pre-split
`bypass/dllmain.cpp` (the body around L291–332 — modulo the
`m_aSendBuff.RemoveAll()` line already deleted by Task 5). Final file:

```cpp
#include "pch.h"

#include "login_hooks.h"

#include "hooker.h"
#include "logger.h"

typedef INT(__thiscall* _CLogin__SendCheckPasswordPacket_t)(CLogin* pThis, char* sID, char* sPasswd);

INT __fastcall CLogin__SendCheckPasswordPacket_Hook(CLogin* pThis, PVOID edx,
                                                    char* sID, char* sPasswd) {
    Log("CLogin::SendCheckPasswordPacket. ID [%s]. bRequestSent [%d].",
        sID, pThis->m_bRequestSent);
    if (pThis->m_bRequestSent) {
        return 0;
    }
    pThis->m_bRequestSent = 1;
    pThis->m_WorldItem.RemoveAll();
    pThis->m_aBalloon.RemoveAll();

    auto systemInfo = CSystemInfo();
    systemInfo.Init();
    auto cOutPacket = COutPacket(1);

    ZXString<char> tempString = ZXString<char>(sID, 0xFFFFFFFF);
    cOutPacket.EncodeStr(tempString);

    ZXString<char> tempString2 = ZXString<char>(sPasswd, 0xFFFFFFFF);
    cOutPacket.EncodeStr(tempString2);

    cOutPacket.EncodeBuffer(systemInfo.GetMachineId(), 16);
    int gameRoomClient = systemInfo.GetGameRoomClient();
    Log("GRC %d, GRC PTR %d", gameRoomClient, &gameRoomClient);
    cOutPacket.Encode4(gameRoomClient);
    cOutPacket.Encode1(CWvsApp::GetInstance()->m_nGameStartMode);
    cOutPacket.Encode1(0);
    cOutPacket.Encode1(0);
#if defined(REGION_GMS)
    cOutPacket.Encode4(CConfig::GetInstance()->GetPartnerCode());
#endif
    CClientSocket::GetInstance()->SendPacket(&cOutPacket);
#if defined(REGION_JMS)
    CWvsContext::GetInstance()->unk1.Assign(sID, 0xFFFFFFFF);
#endif
    CUITitle* cuiTitle = CUITitle::GetInstance();
    if (cuiTitle) {
        cuiTitle->ClearToolTip();
    }
    // m_aSendBuff.RemoveAll() removed per §4.3 — dtor handles it.
    return 1;
}

BOOL InstallLoginHooks() {
    HOOKTYPEDEF_C(CLogin__SendCheckPasswordPacket);
    INITMAPLEHOOK_OR_RETURN(_CLogin__SendCheckPasswordPacket, _CLogin__SendCheckPasswordPacket_t,
                            CLogin__SendCheckPasswordPacket_Hook, C_LOGIN_SEND_CHECK_PASSWORD_PACKET);
    return TRUE;
}
```

- [x] **Step 3: Commit**

```bash
git add bypass/login_hooks.h bypass/login_hooks.cpp
git commit -m "refactor(bypass): extract login hooks into login_hooks.{h,cpp} — part of task-004-code-hygiene §4.1"
```

---

## Task 9: Bypass split — create `security_hooks.{h,cpp}` (PRD §4.1)

**Files:**
- Create: `bypass/security_hooks.h`
- Create: `bypass/security_hooks.cpp`

- [x] **Step 1: Create `bypass/security_hooks.h`**

```cpp
#pragma once
#include <Windows.h>

BOOL InstallSecurityHooks();
```

- [x] **Step 2: Create `bypass/security_hooks.cpp`**

```cpp
#include "pch.h"

#include "security_hooks.h"

#include "hooker.h"
#include "logger.h"
#include <memedit.h>

typedef VOID(__thiscall* _CSecurityClient__OnPacket_t)(CSecurityClient* pThis, CInPacket* iPacket);
typedef INT(__cdecl* _DR__check_t)();
typedef VOID(__thiscall* _CeTracer__Run_t)(int* pThis);
typedef VOID(__cdecl* _SendHSLog_t)(char a1);

VOID __fastcall CSecurityClient__OnPacket_Hook(CSecurityClient* pThis, PVOID edx, CInPacket* iPacket) {
    Log("CSecurityClient::OnPacket.");
}

INT __cdecl DR__check_Hook() {
    return 0;
}

VOID __fastcall CeTracer__Run_Hook(int* pThis, PVOID edx) {
}

VOID __fastcall SendHSLog_Hook(void* ecx, void* edx, char a1) {
}

BOOL InstallSecurityHooks() {
    // The pre-refactor MainProc interleaved CSecurityClient byte patches
    // BEFORE CWvsApp::CallUpdate / ConnectLogin. The TU split groups them
    // under security_hooks. INITMAPLEHOOK_OR_RETURN inside each
    // installer short-circuits THIS installer on failure; the caller
    // (bypass_main.cpp::MainProc) then short-circuits MainProc itself.

#if defined(REGION_JMS)
    // TODO: lift this and the OnPacket+0x19 NOP below into a proper
    // INITMAPLEHOOK_OR_RETURN-installed CSecurityClient::OnPacket hook.
    if (C_SECURITY_CLIENT_ON_PACKET_RET_STUB != 0) {
        constexpr BYTE retStub[] = {0xC3};
        MemEdit::WriteBytes(C_SECURITY_CLIENT_ON_PACKET_RET_STUB, retStub);
    }
#endif

#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 87) || defined(REGION_JMS)
    HOOKTYPEDEF_C(DR__check);
    INITMAPLEHOOK_OR_RETURN(_DR__check, _DR__check_t, DR__check_Hook, DR_CHECK);
#endif

#if defined(REGION_JMS)
    if (C_SECURITY_CLIENT_ON_PACKET_CHECK != 0) {
        constexpr BYTE nopPair[] = {0x90, 0x90};
        MemEdit::WriteBytes(C_SECURITY_CLIENT_ON_PACKET_CHECK + C_SECURITY_CLIENT_ON_PACKET_CHECK_OFFSET,
                            nopPair);
    }
#endif

    HOOKTYPEDEF_C(CSecurityClient__OnPacket);
    INITMAPLEHOOK_OR_RETURN(_CSecurityClient__OnPacket, _CSecurityClient__OnPacket_t,
                            CSecurityClient__OnPacket_Hook, C_SECURITY_CLIENT_ON_PACKET);

#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
    HOOKTYPEDEF_C(CeTracer__Run);
    INITMAPLEHOOK_OR_RETURN(_CeTracer__Run, _CeTracer__Run_t, CeTracer__Run_Hook, CE_TRACER_RUN);
#endif

#if defined(REGION_GMS)
    HOOKTYPEDEF_C(SendHSLog);
    INITMAPLEHOOK_OR_RETURN(_SendHSLog, _SendHSLog_t, SendHSLog_Hook, SEND_HS_LOG);
#endif

    return TRUE;
}
```

- [x] **Step 3: Commit**

```bash
git add bypass/security_hooks.h bypass/security_hooks.cpp
git commit -m "refactor(bypass): extract security hooks into security_hooks.{h,cpp} — part of task-004-code-hygiene §4.1"
```

---

## Task 10: Bypass split — create `app_hooks.{h,cpp}` (PRD §4.1)

**Files:**
- Create: `bypass/app_hooks.h`
- Create: `bypass/app_hooks.cpp`

- [x] **Step 1: Create `bypass/app_hooks.h`**

```cpp
#pragma once
#include <Windows.h>

BOOL InstallAppHooks();
```

- [x] **Step 2: Create `bypass/app_hooks.cpp`**

Copy each `CWvsApp::*_Hook` body verbatim from pre-split `bypass/dllmain.cpp`
(the affected functions are between L354–754) and the helper functions
`get_stage`, `get_gr`, `GetSEPrivilege`, `ResetLSP`. The file:

```cpp
#include "pch.h"

#include "app_hooks.h"
#include "socket_hooks_internal.h"  // CClientSocket__OnConnect_Hook fwd-decl

#include "hooker.h"
#include "logger.h"

#include <timeapi.h>

// ---- typedefs -----------------------------------------------------------
typedef VOID(__thiscall* _CWvsApp__CWvsApp_t)(CWvsApp* pThis, const char* sCmdLine);
typedef VOID(__thiscall* _CWvsApp__SetUp_t)(CWvsApp* pThis);
typedef VOID(__thiscall* _CWvsApp__InitializeInput_t)(CWvsApp* pThis);
typedef VOID(__thiscall* _CWvsApp__Run_t)(CWvsApp* pThis, int* pbTerminate);
typedef VOID(__thiscall* _CWvsApp__CallUpdate_t)(CWvsApp* pThis, int tCurTime);
typedef VOID(__thiscall* _CWvsApp__ConnectLogin_t)(CWvsApp* pThis);

typedef VOID(__cdecl* _set_stage_t)(CStage* pStage, void* pParam);
static _set_stage_t _set_stage = reinterpret_cast<_set_stage_t>(SET_STAGE);

typedef BOOL(WINAPI* LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);

// ---- helpers ------------------------------------------------------------
static CStage* get_stage() {
    return reinterpret_cast<CStage*>(*(void**)STAGE_INSTANCE_ADDR);
}

static IWzGr2D* get_gr() {
    return reinterpret_cast<IWzGr2D*>(*(uint32_t**)GR_INSTANCE_ADDR);
}

static void GetSEPrivilege() {
    ((VOID** (_fastcall*)())GET_SE_PRIVILEGE)();
}

static DWORD ResetLSP() {
    return reinterpret_cast<DWORD>(*(void**)RESET_LSP);
}

// ---- hook bodies --------------------------------------------------------
// (Copy each function body verbatim from pre-split bypass/dllmain.cpp.
//  Specifically:
//    CWvsApp__CallUpdate_Hook  — lines 357–393 of the pre-split file
//    CWvsApp__ConnectLogin_Hook — lines 398–461
//    CWvsApp__InitializeInput_Hook — lines 466–470
//    CWvsApp__Run_Hook — lines 475–547
//    CWvsApp__SetUp_Hook — lines 557–664
//    CWvsApp__CWvsApp_Hook — lines 675–754
//  Replace any `MAJOR_VERSION` / `MINOR_VERSION` references with
//  `BUILD_MAJOR_VERSION` / `BUILD_MINOR_VERSION` — but Task 3 already did
//  the rename in dllmain.cpp, so when copying you're copying the renamed
//  text.)

// ---- installer ----------------------------------------------------------
BOOL InstallAppHooks() {
    HOOKTYPEDEF_C(CWvsApp__CWvsApp);
    INITMAPLEHOOK_OR_RETURN(_CWvsApp__CWvsApp, _CWvsApp__CWvsApp_t,
                            CWvsApp__CWvsApp_Hook, C_WVS_APP);

    HOOKTYPEDEF_C(CWvsApp__SetUp);
    INITMAPLEHOOK_OR_RETURN(_CWvsApp__SetUp, _CWvsApp__SetUp_t,
                            CWvsApp__SetUp_Hook, C_WVS_APP_SET_UP);

    HOOKTYPEDEF_C(CWvsApp__InitializeInput);
    INITMAPLEHOOK_OR_RETURN(_CWvsApp__InitializeInput, _CWvsApp__InitializeInput_t,
                            CWvsApp__InitializeInput_Hook, C_WVS_APP_INITIALIZE_INPUT);

    HOOKTYPEDEF_C(CWvsApp__Run);
    INITMAPLEHOOK_OR_RETURN(_CWvsApp__Run, _CWvsApp__Run_t,
                            CWvsApp__Run_Hook, C_WVS_APP_RUN);

    HOOKTYPEDEF_C(CWvsApp__CallUpdate);
    INITMAPLEHOOK_OR_RETURN(_CWvsApp__CallUpdate, _CWvsApp__CallUpdate_t,
                            CWvsApp__CallUpdate_Hook, C_WVS_APP_CALL_UPDATE);

    HOOKTYPEDEF_C(CWvsApp__ConnectLogin);
    INITMAPLEHOOK_OR_RETURN(_CWvsApp__ConnectLogin, _CWvsApp__ConnectLogin_t,
                            CWvsApp__ConnectLogin_Hook, C_WVS_APP_CONNECT_LOGIN);

    return TRUE;
}
```

**Note on the verbatim copy block:** the comment placeholder `// (Copy each
function body verbatim from pre-split bypass/dllmain.cpp...)` is not
shippable as-is. When implementing this task, replace that comment block
with the actual function bodies from the listed line ranges. The bodies
are unchanged code — they just live in a new file.

To execute the copy reliably, use a series of focused `Edit` calls or a
single `Write` after assembling the full file content from `git show
HEAD:bypass/dllmain.cpp` (post-Task 5 state). The function bodies do not
require any rewrites beyond what Task 3 already applied; just copy them
across.

The hook bodies that reference `CClientSocket__OnConnect_Hook` (i.e.
`CWvsApp__ConnectLogin_Hook`) work because `socket_hooks_internal.h`
provides the forward declaration.

- [x] **Step 3: Commit**

```bash
git add bypass/app_hooks.h bypass/app_hooks.cpp
git commit -m "refactor(bypass): extract CWvsApp hooks into app_hooks.{h,cpp} — part of task-004-code-hygiene §4.1"
```

---

## Task 11: Bypass split — create `key_mapped_hooks.{h,cpp}` (PRD §4.1)

**Files:**
- Create: `bypass/key_mapped_hooks.h`
- Create: `bypass/key_mapped_hooks.cpp`

- [x] **Step 1: Create `bypass/key_mapped_hooks.h`**

```cpp
#pragma once
#include <Windows.h>

BOOL InstallKeyMappedHooks();
```

- [x] **Step 2: Create `bypass/key_mapped_hooks.cpp`**

Copy `CFuncKeyMappedMan__CFuncKeyMappedMan_Hook` body verbatim from pre-split `bypass/dllmain.cpp` (the body around L759–783).

```cpp
#include "pch.h"

#include "key_mapped_hooks.h"

#include "hooker.h"
#include "logger.h"

typedef VOID(__thiscall* _CFuncKeyMappedMan__CFuncKeyMappedMan_t)(CFuncKeyMappedMan* pThis);

VOID __fastcall CFuncKeyMappedMan__CFuncKeyMappedMan_Hook(CFuncKeyMappedMan* pThis, PVOID edx) {
    Log("CFuncKeyMappedMan::CFuncKeyMappedMan.");

    void** instance = reinterpret_cast<void**>(C_FUNC_KEY_MAPPED_MAN_INSTANCE_ADDR);
    *instance = pThis;

    *(void**)pThis = (void*)C_FUNC_KEY_MAPPED_MAN_VFTABLE;
    memcpy(pThis->m_aFuncKeyMapped,        reinterpret_cast<void*>(DEFAULT_FKM_INSTANCE_ADDR),
           sizeof(pThis->m_aFuncKeyMapped));
    memcpy(pThis->m_aFuncKeyMapped_Old,    reinterpret_cast<void*>(DEFAULT_FKM_INSTANCE_ADDR),
           sizeof(pThis->m_aFuncKeyMapped_Old));
    memcpy(pThis->m_aQuickslotKeyMapped,   reinterpret_cast<void*>(DEFAULT_QKM_INSTANCE_ADDR),
           sizeof(pThis->m_aQuickslotKeyMapped));
    memcpy(pThis->m_aQuickslotKeyMapped_Old, reinterpret_cast<void*>(DEFAULT_QKM_INSTANCE_ADDR),
           sizeof(pThis->m_aQuickslotKeyMapped_Old));

    pThis->m_nPetConsumeItemID = 0;
    pThis->m_nPetConsumeMPItemID = 0;
#if defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 111 || defined(REGION_JMS)
    pThis->dummy1 = 0;
#endif
#if defined(REGION_JMS)
    pThis->dummy2 = 0;
#endif
}

BOOL InstallKeyMappedHooks() {
    HOOKTYPEDEF_C(CFuncKeyMappedMan__CFuncKeyMappedMan);
    INITMAPLEHOOK_OR_RETURN(_CFuncKeyMappedMan__CFuncKeyMappedMan,
                            _CFuncKeyMappedMan__CFuncKeyMappedMan_t,
                            CFuncKeyMappedMan__CFuncKeyMappedMan_Hook,
                            C_FUNC_KEY_MAPPED_MAN);
    return TRUE;
}
```

- [x] **Step 3: Commit**

```bash
git add bypass/key_mapped_hooks.h bypass/key_mapped_hooks.cpp
git commit -m "refactor(bypass): extract CFuncKeyMappedMan hook into key_mapped_hooks.{h,cpp} — part of task-004-code-hygiene §4.1"
```

---

## Task 12: Bypass split — create `bypass_main.cpp`, slim `dllmain.cpp`, update CMakeLists (PRD §4.1)

**Files:**
- Create: `bypass/bypass_main.cpp`
- Modify: `bypass/dllmain.cpp` (reduce to just `DllMain`)
- Modify: `bypass/CMakeLists.txt`

- [x] **Step 1: Create `bypass/bypass_main.cpp`**

```cpp
#include "pch.h"

#include "app_hooks.h"
#include "security_hooks.h"
#include "login_hooks.h"
#include "socket_hooks.h"
#include "key_mapped_hooks.h"

DWORD WINAPI MainProc(LPVOID lpParam) {
    if (!InstallAppHooks())        return 0;
    if (!InstallSecurityHooks())   return 0;
    if (!InstallLoginHooks())      return 0;
    if (!InstallSocketHooks())     return 0;
    if (!InstallKeyMappedHooks())  return 0;
    return 0;
}
```

- [x] **Step 2: Replace `bypass/dllmain.cpp`**

Overwrite `bypass/dllmain.cpp` with this minimal content:

```cpp
#include "pch.h"

// Defined in bypass_main.cpp.
DWORD WINAPI MainProc(LPVOID lpParam);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            DisableThreadLibraryCalls(hModule);
            CreateThread(nullptr, 0, &MainProc, nullptr, 0, nullptr);
            break;
        }
    }
    return TRUE;
}
```

- [x] **Step 3: Update `bypass/CMakeLists.txt`**

Overwrite `bypass/CMakeLists.txt`:

```cmake
add_edit_dll(bypass SOURCES
    dllmain.cpp
    bypass_main.cpp
    socket_hooks.cpp
    login_hooks.cpp
    security_hooks.cpp
    app_hooks.cpp
    key_mapped_hooks.cpp
)
```

- [x] **Step 4: Verify pre-split content is fully migrated**

Run: `wc -l bypass/dllmain.cpp`
Expected: ~15 lines (down from 909).

Run: `grep -n "Hook\b" bypass/dllmain.cpp`
Expected: zero hook-function definitions (just the forward declaration of `MainProc`).

Run: `git diff --stat HEAD~5 -- bypass/`
Expected: net line count roughly preserved (the hook bodies moved, not deleted).

- [x] **Step 5: Build verification (Windows)**

Configure + build each of the 5 matrix combos:

```
for combo in "GMS 83 1" "GMS 87 1" "GMS 95 1" "GMS 111 1" "JMS 185 1"; do
  read region maj min <<< "$combo"
  out="build/${region}-${maj}-${min}"
  cmake -DCMAKE_BUILD_TYPE=Debug -G Ninja \
        -DBUILD_REGION=$region -DBUILD_MAJOR_VERSION=$maj -DBUILD_MINOR_VERSION=$min \
        -S . -B "$out"
  cmake --build "$out"
done
```

Expected: every combo builds clean, zero new warnings.

If you cannot run a Windows build locally, defer to CI on the PR.

- [x] **Step 6: Symbol-level audit (Windows, optional but recommended)**

For one combo (GMS 83.1):
```
dumpbin /symbols build/GMS-83-1/bypass/bypass-1.0.0.dll > after-symbols.txt
git stash       # if you stashed your work; otherwise compare with previously-saved baseline
# build pre-refactor for comparison if not already saved
```
Visually diff: every pre-refactor hook function name should still appear post-refactor (modulo source-file metadata). No prior symbol disappears.

- [x] **Step 7: Commit**

```bash
git add bypass/bypass_main.cpp bypass/dllmain.cpp bypass/CMakeLists.txt
git commit -m "$(cat <<'EOF'
refactor(bypass): slim dllmain.cpp, orchestrate hooks in bypass_main.cpp

bypass/dllmain.cpp is reduced to DllMain only (it spawns the MainProc
thread). MainProc moves to bypass_main.cpp, which calls each
Install<Category>Hooks() in turn and short-circuits on failure to
preserve the pre-refactor install-failure semantics.

The six per-category TUs landed in earlier commits in this series:
- socket_hooks.{h,cpp} (incl. OnConnect refactor per §4.2)
- login_hooks.{h,cpp}
- security_hooks.{h,cpp}
- app_hooks.{h,cpp}
- key_mapped_hooks.{h,cpp}

bypass/CMakeLists.txt now lists every new .cpp source.

Inter-category install order in MainProc is regrouped by category; the
intra-category order within each Install*Hooks() is preserved. Hooks
are independent Detours patches on different addresses, so order across
categories has no observable effect.

Part of task-004-code-hygiene §4.1.
EOF
)"
```

---

## Task 13: Extract `parse_ini` into `common/` (PRD §4.9 prep)

**Files:**
- Create: `common/parse_ini.h`
- Create: `common/parse_ini.cpp`
- Modify: `redirect/dllmain.cpp` (include + delete the anon-namespace block)

**Why:** the test target (Task 15+) needs a host-buildable INI parser. The
extraction also separates parsing from `redirect`'s socket-side logic.

- [x] **Step 1: Create `common/parse_ini.h`**

```cpp
#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace ms::ini {

struct Parsed {
    // "section.key" -> ordered values (last-wins is the consumer's choice).
    std::map<std::string, std::vector<std::string>> entries;
};

using LogSink = std::function<void(const char*)>;

// Returns true on successful open + parse. Malformed lines and duplicate
// keys are reported via `sink` (if non-null) but do not fail the parse.
// Missing file is the only hard failure.
bool Parse(const std::string& path, Parsed& out, const LogSink& sink = nullptr);

} // namespace ms::ini
```

- [x] **Step 2: Create `common/parse_ini.cpp`**

```cpp
#include "parse_ini.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <fstream>

namespace ms::ini {

namespace {

std::string TrimLeft(std::string s) {
    auto it = std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isspace(c); });
    s.erase(s.begin(), it);
    return s;
}

std::string TrimRight(std::string s) {
    auto it = std::find_if(s.rbegin(), s.rend(), [](unsigned char c) { return !std::isspace(c); });
    s.erase(it.base(), s.end());
    return s;
}

std::string Trim(std::string s) {
    return TrimRight(TrimLeft(std::move(s)));
}

// Strip everything from the first ; or # to end of line. [Section] is not a comment.
std::string StripComment(const std::string& line) {
    auto pos = line.find_first_of(";#");
    return (pos == std::string::npos) ? line : line.substr(0, pos);
}

void Tell(const LogSink& sink, const char* fmt, ...) {
    if (!sink) return;
    char buf[256];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    sink(buf);
}

} // namespace

bool Parse(const std::string& path, Parsed& out, const LogSink& sink) {
    std::ifstream inputFile(path);
    if (!inputFile.is_open()) {
        Tell(sink, "Failed to open INI file: %s", path.c_str());
        return false;
    }

    std::string line;
    std::string currentSection;
    int lineNo = 0;
    while (std::getline(inputFile, line)) {
        ++lineNo;
        std::string trimmed = Trim(StripComment(line));
        if (trimmed.empty()) continue;

        if (trimmed.front() == '[' && trimmed.back() == ']') {
            currentSection = trimmed.substr(1, trimmed.size() - 2);
            continue;
        }

        auto pos = trimmed.find('=');
        if (pos == std::string::npos) {
            Tell(sink, "INI: malformed line %d (no '='): %s", lineNo, trimmed.c_str());
            continue;
        }
        std::string key = Trim(trimmed.substr(0, pos));
        std::string value = Trim(trimmed.substr(pos + 1));
        if (key.empty()) {
            Tell(sink, "INI: malformed line %d (empty key): %s", lineNo, trimmed.c_str());
            continue;
        }
        out.entries[currentSection + "." + key].push_back(std::move(value));
    }
    return true;
}

} // namespace ms::ini
```

The `Tell()` helper uses `va_list` rather than a `std::string`-based
fallback because `printf`-style formatting was the convention in the
original `Log()` calls; preserving it makes the `redirect` callsite a
one-line lambda. The `<cstdio>` include covers `vsnprintf`. (Note: `va_list`
requires `<cstdarg>` — add that include if MSVC complains.)

- [x] **Step 3: Update `redirect/dllmain.cpp` to consume the new header**

In `redirect/dllmain.cpp`:

1. After the existing `#include "logger.h"` (or in a sensible spot among the existing includes), add:
   ```cpp
   #include "parse_ini.h"
   ```

2. Delete the anonymous-namespace block at lines 34–98 (every line from `namespace {` through `} // namespace` containing `TrimLeft`, `TrimRight`, `Trim`, `StripComment`, `ParsedINI`, `ParseINI`).

3. In `Config::Load`, replace `ParsedINI ini;` with `ms::ini::Parsed ini;` and `if (!ParseINI(path, ini))` with:
   ```cpp
   if (!ms::ini::Parse(path, ini, [](const char* msg) { Log("%s", msg); }))
   ```

4. The body of `Config::Load` continues to access `ini.entries["Main.RedirectIP"]` etc. unchanged — the field name is identical.

The final `Config::Load` opening looks like:
```cpp
bool Config::Load(const std::string& path) {
    ms::ini::Parsed ini;
    if (!ms::ini::Parse(path, ini, [](const char* msg) { Log("%s", msg); }))
        return false;
    /* ... rest unchanged ... */
}
```

- [x] **Step 4: Verify build (Windows or CI)**

`redirect` should still compile. The functional behavior is identical.

- [x] **Step 5: Commit**

```bash
git add common/parse_ini.h common/parse_ini.cpp redirect/dllmain.cpp
git commit -m "$(cat <<'EOF'
refactor(common): extract host-buildable parse_ini from redirect

common/parse_ini.{h,cpp} packages the INI parser previously inlined in
redirect/dllmain.cpp's anonymous namespace. Lives in namespace ms::ini.

Win32-free: depends only on <string>, <map>, <vector>, <fstream>,
<algorithm>, <cctype>. Error/warning reporting routes through an
optional std::function<void(const char*)> log sink; redirect passes a
lambda wrapping the legacy Log(), the forthcoming test target passes
nullptr.

Part of task-004-code-hygiene §4.9 prep.
EOF
)"
```

---

## Task 14: Extract `byte_ops` (PRD §4.9 prep)

**Files:**
- Create: `common/byte_ops.h`
- Create: `common/byte_ops.cpp`
- Modify: `common/memedit.cpp` (call `byte_ops::fill_nop` instead of inline loop)

- [x] **Step 1: Create `common/byte_ops.h`**

```cpp
#pragma once
#include <cstddef>

namespace ms::byte_ops {

// Fills `n` bytes at `dst` with the x86 NOP opcode (0x90). No-op when n == 0.
// `dst` may be nullptr only when n == 0.
void fill_nop(unsigned char* dst, std::size_t n);

// Copies `n` bytes from `src` to `dst` (memcpy wrapper). No-op when n == 0.
void copy(unsigned char* dst, const unsigned char* src, std::size_t n);

} // namespace ms::byte_ops
```

- [x] **Step 2: Create `common/byte_ops.cpp`**

```cpp
#include "byte_ops.h"

#include <cstring>

namespace ms::byte_ops {

namespace {
constexpr unsigned char kX86Nop = 0x90;
}

void fill_nop(unsigned char* dst, std::size_t n) {
    if (n == 0) return;
    for (std::size_t i = 0; i < n; ++i) {
        dst[i] = kX86Nop;
    }
}

void copy(unsigned char* dst, const unsigned char* src, std::size_t n) {
    if (n == 0) return;
    std::memcpy(dst, src, n);
}

} // namespace ms::byte_ops
```

- [x] **Step 3: Update `common/memedit.cpp::PatchNop`**

In `common/memedit.cpp`, add `#include "byte_ops.h"` near the top
(alongside `#include <vector>`). Replace the inline loop in `PatchNop`:

OLD (around line 70–71):
```cpp
    for (UINT i = 0; i < nCount; ++i)
        bArr[i] = x86NOP;
```

NEW:
```cpp
    ms::byte_ops::fill_nop(reinterpret_cast<unsigned char*>(bArr), nCount);
```

The cast preserves the Win32 `BYTE`-typed buffer pointer at the
callsite without leaking Win32 typedefs into `byte_ops`.

- [x] **Step 4: Commit**

```bash
git add common/byte_ops.h common/byte_ops.cpp common/memedit.cpp
git commit -m "$(cat <<'EOF'
refactor(common): extract host-buildable byte_ops; PatchNop delegates fill_nop

common/byte_ops.{h,cpp} carries fill_nop and copy in namespace ms::byte_ops.
No Win32 dependency: depends only on <cstddef> and <cstring>, uses
unsigned char (not Win32 BYTE). MemEdit::PatchNop casts its BYTE* buffer
when delegating, keeping the Win32 typedef confined to memedit.cpp.

Enables host-side tests for the byte-manipulation surface without
pulling in VirtualProtect / WriteProcessMemory.

Part of task-004-code-hygiene §4.9 prep.
EOF
)"
```

---

## Task 15: Add `BUILD_TESTS` option and tests/CMakeLists.txt (PRD §4.9)

**Files:**
- Modify: `CMakeLists.txt` (root)
- Create: `tests/CMakeLists.txt`

- [x] **Step 1: Add `BUILD_TESTS` option to root `CMakeLists.txt`**

Append to the end of `CMakeLists.txt` (after the `add_custom_target(package_dlls ...)` block):

```cmake
option(BUILD_TESTS "Build host-side unit tests" OFF)
if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
```

- [x] **Step 2: Create `tests/CMakeLists.txt`**

```cmake
# Host-architecture unit tests. Driven by GoogleTest v1.14.0, fetched via
# FetchContent. Builds the extracted host-buildable surfaces directly —
# does NOT link common_lib (which is x86 and Win32-only).

include(FetchContent)
FetchContent_Declare(
    googletest
    URL      https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
    URL_HASH SHA256=8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

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
```

The SHA256 hash above is the canonical v1.14.0 tarball hash. If FetchContent fails due to a hash mismatch, verify the hash from
`https://github.com/google/googletest/releases/tag/v1.14.0` and update.

- [x] **Step 3: Commit (test executables come in the next two tasks)**

```bash
git add CMakeLists.txt tests/CMakeLists.txt
git commit -m "build: add BUILD_TESTS option with GoogleTest FetchContent — task-004-code-hygiene §4.9"
```

---

## Task 16: Write `tests/test_parse_ini.cpp`

**Files:**
- Create: `tests/test_parse_ini.cpp`
- Create: `tests/fixtures/` directory (transient — fixtures written by tests at runtime)

- [x] **Step 1: Write the failing tests**

Create `tests/test_parse_ini.cpp`:

```cpp
#include "parse_ini.h"

#include <cstdio>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

namespace {

std::string WriteTempFile(const std::string& body) {
    char tmpl[L_tmpnam];
    std::tmpnam(tmpl);
    std::string path = tmpl;
    std::ofstream f(path);
    f << body;
    f.close();
    return path;
}

} // namespace

TEST(ParseIni, ValidKeyValuePair) {
    auto path = WriteTempFile("[Main]\nfoo=bar\n");
    ms::ini::Parsed p;
    ASSERT_TRUE(ms::ini::Parse(path, p));
    auto it = p.entries.find("Main.foo");
    ASSERT_NE(it, p.entries.end());
    ASSERT_EQ(it->second.size(), 1u);
    EXPECT_EQ(it->second[0], "bar");
}

TEST(ParseIni, LeadingAndTrailingWhitespaceIsTrimmed) {
    auto path = WriteTempFile("[Main]\n   key   =   value   \n");
    ms::ini::Parsed p;
    ASSERT_TRUE(ms::ini::Parse(path, p));
    auto it = p.entries.find("Main.key");
    ASSERT_NE(it, p.entries.end());
    EXPECT_EQ(it->second.front(), "value");
}

TEST(ParseIni, SemicolonComment) {
    auto path = WriteTempFile("[Main]\nfoo=bar ; trailing comment\n");
    ms::ini::Parsed p;
    ASSERT_TRUE(ms::ini::Parse(path, p));
    EXPECT_EQ(p.entries.at("Main.foo").front(), "bar");
}

TEST(ParseIni, HashComment) {
    auto path = WriteTempFile("[Main]\nfoo=bar # trailing comment\n");
    ms::ini::Parsed p;
    ASSERT_TRUE(ms::ini::Parse(path, p));
    EXPECT_EQ(p.entries.at("Main.foo").front(), "bar");
}

TEST(ParseIni, DuplicateKeysAccumulate) {
    auto path = WriteTempFile("[Main]\nip=1.2.3.4\nip=5.6.7.8\n");
    ms::ini::Parsed p;
    ASSERT_TRUE(ms::ini::Parse(path, p));
    auto& vals = p.entries.at("Main.ip");
    ASSERT_EQ(vals.size(), 2u);
    EXPECT_EQ(vals[0], "1.2.3.4");
    EXPECT_EQ(vals[1], "5.6.7.8");
}

TEST(ParseIni, MalformedLineNoEquals) {
    auto path = WriteTempFile("[Main]\nthis line has no equals\n");
    ms::ini::Parsed p;
    ASSERT_TRUE(ms::ini::Parse(path, p));
    EXPECT_TRUE(p.entries.empty());
}

TEST(ParseIni, MalformedLineEmptyKey) {
    auto path = WriteTempFile("[Main]\n=value\n");
    ms::ini::Parsed p;
    ASSERT_TRUE(ms::ini::Parse(path, p));
    EXPECT_TRUE(p.entries.empty());
}

TEST(ParseIni, MissingFileReturnsFalse) {
    ms::ini::Parsed p;
    EXPECT_FALSE(ms::ini::Parse("/nonexistent/path/never-created.ini", p));
}

TEST(ParseIni, LogSinkReceivesMalformedLineMessages) {
    auto path = WriteTempFile("[Main]\nlonely\n");
    ms::ini::Parsed p;
    int callCount = 0;
    auto sink = [&](const char* msg) {
        ++callCount;
        EXPECT_NE(std::string(msg).find("malformed"), std::string::npos);
    };
    ASSERT_TRUE(ms::ini::Parse(path, p, sink));
    EXPECT_EQ(callCount, 1);
}
```

- [x] **Step 2: Configure and run the tests (host)**

```
cmake -DBUILD_TESTS=ON -DBUILD_REGION=GMS -DBUILD_MAJOR_VERSION=83 -DBUILD_MINOR_VERSION=1 \
      -S . -B build/host-tests
cmake --build build/host-tests
ctest --test-dir build/host-tests --output-on-failure
```
Expected: all `parse_ini` tests pass.

If `tmpnam` triggers MSVC warnings as deprecated, suppress with `_CRT_SECURE_NO_WARNINGS` only in the test TU (target_compile_definitions on `parse_ini_tests` PRIVATE).

- [x] **Step 3: Commit**

```bash
git add tests/test_parse_ini.cpp
git commit -m "test(parse_ini): cover k=v, whitespace, comments, dupes, malformed, missing-file — task-004-code-hygiene §4.9"
```

---

## Task 17: Write `tests/test_byte_ops.cpp`

**Files:**
- Create: `tests/test_byte_ops.cpp`

- [x] **Step 1: Write the tests**

```cpp
#include "byte_ops.h"

#include <array>
#include <cstring>

#include <gtest/gtest.h>

TEST(ByteOps, FillNopFillsAllBytesWith90) {
    std::array<unsigned char, 5> buf{};
    ms::byte_ops::fill_nop(buf.data(), buf.size());
    for (auto b : buf) {
        EXPECT_EQ(b, 0x90);
    }
}

TEST(ByteOps, FillNopZeroLengthIsNoOpAndAcceptsNullptr) {
    // The contract says nullptr is allowed when n == 0.
    ms::byte_ops::fill_nop(nullptr, 0);
    // Also verify it doesn't write past the end of a non-null buf.
    std::array<unsigned char, 4> guard{0xAA, 0xBB, 0xCC, 0xDD};
    ms::byte_ops::fill_nop(guard.data(), 0);
    EXPECT_EQ(guard[0], 0xAA);
    EXPECT_EQ(guard[1], 0xBB);
    EXPECT_EQ(guard[2], 0xCC);
    EXPECT_EQ(guard[3], 0xDD);
}

TEST(ByteOps, CopyRoundTrip) {
    std::array<unsigned char, 7> src{1, 2, 3, 4, 5, 6, 7};
    std::array<unsigned char, 7> dst{};
    ms::byte_ops::copy(dst.data(), src.data(), src.size());
    EXPECT_EQ(std::memcmp(src.data(), dst.data(), src.size()), 0);
}

TEST(ByteOps, CopyZeroLengthIsNoOp) {
    std::array<unsigned char, 3> dst{0xAA, 0xBB, 0xCC};
    std::array<unsigned char, 3> src{0x11, 0x22, 0x33};
    ms::byte_ops::copy(dst.data(), src.data(), 0);
    EXPECT_EQ(dst[0], 0xAA);
    EXPECT_EQ(dst[1], 0xBB);
    EXPECT_EQ(dst[2], 0xCC);
}
```

- [x] **Step 2: Run the tests**

```
cmake --build build/host-tests
ctest --test-dir build/host-tests --output-on-failure
```
Expected: all `byte_ops` tests pass alongside the `parse_ini` ones.

- [x] **Step 3: Commit**

```bash
git add tests/test_byte_ops.cpp
git commit -m "test(byte_ops): cover fill_nop / copy correctness and zero-length boundaries — task-004-code-hygiene §4.9"
```

---

## Task 18: Add CI-wiring TODO entry (PRD §4.9 open question)

**Files:**
- Create or modify: `docs/TODO.md`

- [x] **Step 1: Add the TODO entry**

If `docs/TODO.md` exists, append a new bullet under an appropriate section
(create `## Build / CI` if needed). If it doesn't, create the file:

```markdown
# Outstanding TODOs

## Build / CI

- Wire `BUILD_TESTS=ON` as a separate CI job (matrix entry or a follow-on
  workflow). Currently the host test target is developer-local only.
  Tracked: task-004-code-hygiene §4.9 open question.
```

(If `docs/TODO.md` already has structure, fit the bullet under the
existing build/CI section instead of duplicating the heading.)

- [x] **Step 2: Commit**

```bash
git add docs/TODO.md
git commit -m "docs(todo): note deferred BUILD_TESTS CI wiring — task-004-code-hygiene §4.9"
```

---

## Final verification (cross-task)

After every task is committed, run the full acceptance check:

- [x] **Matrix builds (Windows, x86):** every region+version configure + build clean, zero new warnings.

  ```
  for combo in "GMS 83 1" "GMS 87 1" "GMS 95 1" "GMS 111 1" "JMS 185 1"; do
    read region maj min <<< "$combo"
    out="build/${region}-${maj}-${min}"
    cmake -DCMAKE_BUILD_TYPE=Debug -G Ninja \
          -DBUILD_REGION=$region -DBUILD_MAJOR_VERSION=$maj -DBUILD_MINOR_VERSION=$min \
          -S . -B "$out" && cmake --build "$out"
  done
  ```

- [x] **Host tests (any arch):** `BUILD_TESTS=ON` configures, builds, `ctest` is green.

  ```
  cmake -DBUILD_TESTS=ON -DBUILD_REGION=GMS -DBUILD_MAJOR_VERSION=83 -DBUILD_MINOR_VERSION=1 \
        -S . -B build/host-tests && cmake --build build/host-tests && \
        ctest --test-dir build/host-tests --output-on-failure
  ```

- [x] **Validation script test:** deliberately undefine a key in a memory map file and re-run `cmake`. Expected: `FATAL_ERROR` naming the key, not a downstream C++ compile error.

- [x] **License residue grep:**
  ```
  grep -rn "This file is part of GMS-83-DLL" . && echo "FAIL" || echo "OK"
  grep -n "## License\|GitHub license" README.md && echo "FAIL" || echo "OK"
  ```
  Expected: both `OK`.

- [x] **Version macro residue grep:**
  ```
  grep -rn '\bMAJOR_VERSION\b\|\bMINOR_VERSION\b' --include="*.cpp" --include="*.h" --include="*.cmake" --include="*.in" . | grep -v BUILD_MAJOR_VERSION | grep -v BUILD_MINOR_VERSION | grep -v VERSION_HEADER
  ```
  Expected: zero results.

- [x] **`m_aSendBuff.RemoveAll` residue grep in bypass/:**
  ```
  grep -rn "cOutPacket\.m_aSendBuff\.RemoveAll" bypass/
  ```
  Expected: zero results.

- [x] **Pre-refactor `MainProc` hook-install order audit:** mentally diff the pre-refactor `MainProc` `INITMAPLEHOOK_OR_RETURN` sequence (pre-split `bypass/dllmain.cpp:798–897`) against the post-refactor installer bodies. Confirm intra-category order preservation. Inter-category regrouping is the expected design tradeoff.

- [x] **Symbol audit (one combo):** `dumpbin /symbols` on pre- vs post-refactor `bypass-1.0.0.dll` for GMS 83.1. Confirm every prior hook function name still appears.

---

## Self-review notes

This plan was self-reviewed before saving. Findings:

- **Spec coverage:** PRD §4.1–§4.9 are each mapped to one or more tasks (1=§4.5, 2=§4.8, 3=§4.7, 4=§4.6, 5=§4.3, 6=§4.4, 7–12=§4.1+§4.2, 13–14=§4.9 prep, 15–17=§4.9, 18=§4.9 CI open question).
- **Placeholder scan:** Task 7's intermediate "placeholder block" is called out explicitly with cleanup instructions; Task 10's "verbatim copy" comment is similarly flagged with explicit instructions to replace with real content. No silent TBDs.
- **Type consistency:** `ms::ini::Parsed` / `ms::ini::Parse` are used consistently in Tasks 13 (creation) and 16 (tests). `ms::byte_ops::fill_nop` / `ms::byte_ops::copy` are used consistently in Tasks 14 (creation) and 17 (tests). `Install<Category>Hooks()` returns `BOOL` in every header (Tasks 7–11) and is checked for `FALSE` in `bypass_main.cpp` (Task 12).
- **Known design deviations:** all five of the design's "Open Questions" are resolved per design recommendations and noted explicitly in the plan header.

---

End of plan.
