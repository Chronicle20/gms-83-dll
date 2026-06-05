# Code Hygiene — Quick-Reference Context

Companion to `prd.md`, `design.md`, and `plan.md`. This file is the single
"what does an executor need to know to start" cheat sheet for task-004.

---

## Region / version matrix

All 5 must `cmake` configure and build clean with zero new warnings:

| Region | Major | Minor | Memory map file |
|---|---|---|---|
| GMS | 83 | 1 | `memory_maps/GMS/v83_1.cmake` |
| GMS | 87 | 1 | `memory_maps/GMS/v87_1.cmake` |
| GMS | 95 | 1 | `memory_maps/GMS/v95_1.cmake` |
| GMS | 111 | 1 | `memory_maps/GMS/v111_1.cmake` |
| JMS | 185 | 1 | `memory_maps/JMS/v185_1.cmake` |

Plus a 6th host-arch build:

- `cmake -DBUILD_TESTS=ON ...` configures, builds, `ctest` passes green.

Standard configure (Windows CMake + Ninja, MSVC v143):

```
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM="ninja.exe" \
      -DBUILD_REGION=GMS -DBUILD_MAJOR_VERSION=83 -DBUILD_MINOR_VERSION=1 \
      -G Ninja -S . -B build/gms-83-1
cmake --build build/gms-83-1
```

For the test target, drop the `BUILD_REGION`/`BUILD_MAJOR_VERSION`/
`BUILD_MINOR_VERSION` flags? No — the root `CMakeLists.txt` requires them.
Use a sentinel set (e.g., `GMS 83 1`) plus `-DBUILD_TESTS=ON` in a separate
build directory (host x64, not the x86 matrix).

---

## Hook surface (frozen)

No new hooks. No address changes. No memory-map keys added. The
`INITMAPLEHOOK_OR_RETURN` set is identical before vs. after. See
`bypass/dllmain.cpp:798–897` (pre-split `MainProc`) for the authoritative
pre-refactor list.

| Category | File (post-split) | Hooks installed |
|---|---|---|
| App | `bypass/app_hooks.cpp` | `CWvsApp__CWvsApp`, `__SetUp`, `__InitializeInput`, `__Run`, `__CallUpdate`, `__ConnectLogin` |
| Security | `bypass/security_hooks.cpp` | `CSecurityClient__OnPacket`, `DR__check` (GMS≥87 \|\| JMS), JMS `OnPacket_RET_STUB` + `OnPacket_CHECK` byte patches, `CeTracer__Run` (GMS≥95), `SendHSLog` (GMS) |
| Login | `bypass/login_hooks.cpp` | `CLogin__SendCheckPasswordPacket` |
| Socket | `bypass/socket_hooks.cpp` | `CClientSocket__Connect_addr`, `__Connect_ctx`, `__OnConnect`, `__SendPacket` (GMS≥95) |
| Key-mapped | `bypass/key_mapped_hooks.cpp` | `CFuncKeyMappedMan__CFuncKeyMappedMan` |

Cross-TU call: `app_hooks.cpp::CWvsApp__ConnectLogin_Hook` calls
`CClientSocket__OnConnect_Hook` (defined in `socket_hooks.cpp`). Resolved via
`bypass/socket_hooks_internal.h` — included only by `socket_hooks.cpp` and
`app_hooks.cpp`.

---

## Key files

| Path | Role |
|---|---|
| `CMakeLists.txt` | root build; needs `BUILD_TESTS` option, `GenerateMemoryMap.cmake` include, `LOG_MIN_LEVEL` compile-def |
| `cmake/AddEditDll.cmake` | `add_edit_dll()` helper; needs `target_precompile_headers(... REUSE_FROM common_lib)` |
| `cmake/CommonLib.cmake` | `common_lib` OBJECT lib; needs `target_precompile_headers(common_lib PRIVATE common/pch.h)` |
| `cmake/GenerateMemoryMap.cmake` | **NEW** — validate every `@KEY@` in `memory_map.h.in` is defined; emit single `FATAL_ERROR` listing missing keys |
| `include/memory_map.h.in` | strip `#define MAJOR_VERSION` / `MINOR_VERSION` lines |
| `bypass/dllmain.cpp` | shrinks to `DllMain` only; pre-split 909 lines |
| `bypass/{bypass_main,socket_hooks,login_hooks,security_hooks,app_hooks,key_mapped_hooks}.cpp` | **NEW** TUs |
| `bypass/{socket_hooks,login_hooks,security_hooks,app_hooks,key_mapped_hooks}.h` | **NEW** public headers — each exports a single `BOOL Install<Category>Hooks()` |
| `bypass/socket_hooks_internal.h` | **NEW** — forward decl of `CClientSocket__OnConnect_Hook` for `app_hooks.cpp` |
| `bypass/CMakeLists.txt` | list all new `.cpp` sources |
| `common/logger.{h,cpp}` | severity-gated rewrite |
| `common/pch.h` | delete commented-out `#ifndef PCH_H` block (lines 7–8) |
| `common/COutPacket.cpp` | unchanged (dtor already does `RemoveAll`) |
| `common/parse_ini.{h,cpp}` | **NEW** — host-buildable extraction from `redirect/dllmain.cpp:34–95` |
| `common/byte_ops.{h,cpp}` | **NEW** — host-buildable NOP fill / byte copy helpers |
| `common/memedit.cpp` | `PatchNop` calls `byte_ops::fill_nop` instead of inline loop |
| `redirect/dllmain.cpp` | consume `common/parse_ini.h`; delete the anon-namespace block |
| `tests/CMakeLists.txt` + `tests/test_parse_ini.cpp` + `tests/test_byte_ops.cpp` | **NEW** GoogleTest target |
| `scripts/strip_license_header.py` | **NEW** one-shot license strip; commits with the cleanup |
| `README.md` | remove license badge line + `## License` section |
| `LICENSE` | **DELETED** |

---

## Macros & symbols

- `HOOKTYPEDEF_C(name)` → expands to a `_name_t _name;` variable declaration.
  Inside an installer function it becomes a stack local. Defined in
  `common/hooker.h:92`.
- `INITMAPLEHOOK_OR_RETURN(...)` → `return -1` on hook-install failure. Inside
  installer functions, this means the installer returns `-1` (which we coerce
  to `FALSE` via the installer's `BOOL` return type and the caller
  short-circuits). Defined in `common/hooker.h:38`.
- `BUILD_MAJOR_VERSION` / `BUILD_MINOR_VERSION` — compile defs from
  `build_config` in root `CMakeLists.txt:45–48`. Canonical going forward.
- `MAJOR_VERSION` / `MINOR_VERSION` — currently `#define`d in
  `include/memory_map.h.in:4–5`. **TO BE DELETED**.
- `LOG_MIN_LEVEL` — new compile def, int-valued; Debug ≤ 0 (Trace), Release
  ≥ 3 (Warn).

## Non-`BUILD_`-prefixed version macro occurrences (rename targets)

14 sites total — `MAJOR_VERSION` / `MINOR_VERSION` → `BUILD_*`:

```
bypass/dllmain.cpp:163       version > MINOR_VERSION         → BUILD_MINOR_VERSION
bypass/dllmain.cpp:205       MAJOR_VERSION > 83              → BUILD_MAJOR_VERSION > 83
bypass/dllmain.cpp:592       MAJOR_VERSION >= 95             → BUILD_MAJOR_VERSION >= 95
bypass/dllmain.cpp:600       MAJOR_VERSION >= 95             → BUILD_MAJOR_VERSION >= 95
bypass/dllmain.cpp:777       MAJOR_VERSION >= 111            → BUILD_MAJOR_VERSION >= 111
bypass/dllmain.cpp:827       MAJOR_VERSION >= 87             → BUILD_MAJOR_VERSION >= 87
bypass/dllmain.cpp:886       MAJOR_VERSION >= 95             → BUILD_MAJOR_VERSION >= 95
common/CClientSocket.h:22    MAJOR_VERSION >= 111            → BUILD_MAJOR_VERSION >= 111
common/CWvsContext.h:21      MAJOR_VERSION >= 95             → BUILD_MAJOR_VERSION >= 95
common/CWvsContext.h:31      MAJOR_VERSION >= 87             → BUILD_MAJOR_VERSION >= 87
common/CWvsContext.h:73      MAJOR_VERSION > 83              → BUILD_MAJOR_VERSION > 83
common/CWvsContext.h:76      MAJOR_VERSION >= 87             → BUILD_MAJOR_VERSION >= 87
common/CFuncKeyMappedMan.h:18 MAJOR_VERSION >= 111           → BUILD_MAJOR_VERSION >= 111
common/CFuncKeyMappedMan.h:24 MAJOR_VERSION >= 95            → BUILD_MAJOR_VERSION >= 95
```

`VERSION_HEADER` (protocol-level, set per-version in
`memory_maps/<region>/v*_*.cmake`) is intentionally untouched.

---

## Design decisions worth re-reading

- §2.1 — `Install*Hooks()` returns `BOOL`; `MainProc` checks each. Preserves
  pre-refactor failure short-circuit semantics.
- §2.2 — `read_packet_header` / `read_packet_body` take `int& retries` (deviation
  from PRD verbatim signature) to preserve the shared retry budget.
- §2.4 — `REUSE_FROM common_lib` is applied unconditionally to every edit DLL.
  Force-include accepted; small symbol-flow cost dropped at link via /OPT:REF.
- §2.7 — `memory_maps/<region>/v*_*.cmake` files do NOT `set(MAJOR_VERSION ...)`
  today. PRD §4.7's call to remove those `set()`s is a no-op against current
  repo state.
- §2.8 — Legacy `Log()` forwards directly to `LogImpl(LogLevel::Info, ...)`,
  bypassing `LOG_MIN_LEVEL`. Preserves "Log() always fires" semantics; macro
  migration is the path to silence.
- §2.9 — `parse_ini` carries an optional `std::function<void(const char*)>`
  log sink (default `nullptr`). Host tests pass `nullptr`. `redirect` callsite
  passes a lambda wrapping `Log`.
- §2.9 — `byte_ops` uses `unsigned char`, not Win32 `BYTE`. `MemEdit::PatchNop`
  casts its buffer pointer at the callsite.

---

## Dependencies & ordering hazards

- License strip must run BEFORE the logger rewrite — otherwise the logger
  rewrite leaves the GPL block in place on the new file. (Strip script
  matches by the boilerplate's `"This file is part of GMS-83-DLL"` opener.)
- Version macro rename (§4.7) must land BEFORE the bypass TU split (§4.1)
  OR happen as part of the split — the split copies the affected lines into
  new files. Renaming first is simpler.
- `RemoveAll()` deletion (§4.3) targets lines in pre-split `bypass/dllmain.cpp`
  (`:210`, `:330`). Easier to delete pre-split.
- `pch.h` PCH wiring should land AFTER the logger rewrite (the new `logger.h`
  is included by `pch.h:18`; we want the new header pre-baked into the PCH).
- `OnConnect` refactor (§4.2) lands as `socket_hooks.cpp` is created — single
  task.

## CI / verification

- GitHub Actions matrix in `.github/workflows/` builds the 5 region+version
  configs. Verify all 5 stay green.
- `BUILD_TESTS=ON` CI wiring is **deferred** for this task — TODO entry goes
  into `docs/TODO.md`.
- Symbol-level audit: `dumpbin /symbols` on at least one matrix combo
  (GMS 83.1) before vs. after the bypass split. Every prior hook function
  must still exist by name; only file metadata may differ.

---

End of context.
