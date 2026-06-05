# Custom UI Framework — Executing-Agent Context

Quick reference for subagents executing `plan.md`. Pairs with `prd.md` /
`design.md` / `hook-design.md` / `risks.md`.

---

## Region / version

Single target build: `BUILD_REGION=GMS BUILD_MAJOR_VERSION=83 BUILD_MINOR_VERSION=1`.
Both new edits (`custom-ui-host`, `custom-ui-demo`) compile-gate on this
combo and `return()` early from their CMakeLists for any other combo. The
existing CI matrix (`.github/workflows/_build.yml`) iterates GMS 83/87/95/111
and JMS 185 — the new edits must build clean (compile-out, no source error)
under every matrix entry.

---

## Key existing files

| Area | Path | Purpose |
|---|---|---|
| Hook macros | `common/hooker.h` | `INITMAPLEHOOK_OR_RETURN`, `SetHook`, Detours wrapper. **The project uses Microsoft Detours, not MinHook** (PRD/hook-design.md note MinHook in error — design §1.1 corrects). |
| Logging | `common/logger.h` | `Log(...)` free function (no level), `LOG_TRACE/DEBUG/INFO/WARN/ERROR` macros gated by `LOG_MIN_LEVEL`. |
| INI parsing | `common/parse_ini.h` + `parse_ini.cpp` | `ms::ini::Parse(path, out, sink)`. Keys keyed `"Section.key"`. |
| PCH | `common/pch.h` | Force-included into every TU via `target_precompile_headers(... REUSE_FROM common_lib)`. Contains every existing `common/` header. |
| Edit DLL helper | `cmake/AddEditDll.cmake` | `add_edit_dll(name SOURCES ...)`. Links `common_lib` + `build_config`, reuses PCH. |
| Memory map gen | `cmake/GenerateMemoryMap.cmake` | Validates every `@KEY@` in `include/memory_map.h.in` has a non-empty CMake var. Build fails fast on a missing key. |
| Memory map template | `include/memory_map.h.in` | All `@KEY@` symbols must be added here when adding to `v83_1.cmake`. |
| v83 memory map | `memory_maps/GMS/v83_1.cmake` | All v83.1 GMS addresses. |
| Reference pattern (DllMain) | `bypass/dllmain.cpp` | `DLL_PROCESS_ATTACH` -> `CreateThread(MainProc)`. |
| Reference pattern (MainProc) | `bypass/bypass_main.cpp` | Chain of `Install*Hooks()` calls. |
| Reference pattern (thunk impl) | `common/CFuncKeyMappedMan.cpp` | `reinterpret_cast<void(__fastcall*)(...)>(ADDR)(this, nullptr, ...)`. The canonical pattern for "method that's actually a thunk to an address". |
| Tests | `tests/CMakeLists.txt`, `tests/test_parse_ini.cpp` | GoogleTest 1.14, fetched via `FetchContent`. Add new test executables here. |

---

## Existing v83.1 addresses (reused as-is)

These are in `memory_maps/GMS/v83_1.cmake` already:

| Symbol | Address | Used by |
|---|---|---|
| `C_CLIENT_SOCKET_PROCESS_PACKET` | `0x004965F1` | H1 hook target |
| `C_CLIENT_SOCKET_SEND_PACKET` | `0x0049637B` | `CustomUI_SendPacket` path |
| `C_OUT_PACKET` | `0x006EC9CE` | `COutPacket(int)` ctor (already wrapped in `common/COutPacket.cpp`) |
| `C_OUT_PACKET_ENCODE_BUFFER` | `0x0046C00C` | already wrapped |
| `C_FUNC_KEY_MAPPED_MAN` | `0x0058DD0D` | ctor thunk (already wrapped) |
| `C_FUNC_KEY_MAPPED_MAN_INSTANCE_ADDR` | `0x00BED5A0` | singleton pointer storage |
| `C_FUNC_KEY_MAPPED_MAN_CREATE_INSTANCE` | `0x009F9E98` | `CreateInstance()` thunk |
| `C_WND_MAN_S_UPDATE` | `0x009E47C3` | H3 restore-latch hook target (already wrapped as `CWndMan::s_Update`) |

---

## New v83.1 addresses required (TBD — IDA work in early plan tasks)

Resolve via IDA before any downstream task that depends on them. Each entry
below must end up in both `memory_maps/GMS/v83_1.cmake` AND
`include/memory_map.h.in` (the build fails fast if a `@KEY@` in `.h.in`
lacks a CMake variable).

| Symbol | What | Hook? | Where used |
|---|---|---|---|
| `C_WND_MAN_PROCESS_KEY` | `CWndMan::ProcessKey(uint, uint, long)` | yes (H2) | `hooks/process_key_hook.cpp` |
| `C_STAGE_DTOR` | `CStage::~CStage()` | yes (H3) | `hooks/stage_dtor_hook.cpp` |
| `C_WND_MAN_REGISTER_UI_WINDOW` | `CWndMan::RegisterUIWindow(CUIWnd*)` | no | `common/CWndMan.cpp` extension |
| `C_WND_MAN_UNREGISTER_UI_WINDOW` | `CWndMan::UnregisterUIWindow(CUIWnd*)` | no | `common/CWndMan.cpp` extension |
| `C_UI_WND_CTOR` | `CUIWnd::CUIWnd(x,y,w,h,name,...)` | no | placement-new in `runtime/custom_ui_wnd.cpp` |
| `C_UI_WND_VFTABLE` | `CUIWnd` vftable address | no | vtable clone source in `runtime/vtable_patch.cpp` |
| `C_UI_WND_DTOR` | `CUIWnd::~CUIWnd()` | no | called from cloned vtable's dtor slot pass-through |
| `C_CTRL_BUTTON_CTOR` | `CCtrlButton::CCtrlButton(...)` | no | `common/CCtrlButton.cpp` (new) |
| `C_CTRL_BUTTON_VFTABLE` | `CCtrlButton` vftable | no | per-instance vtable clone in `runtime/custom_ui_wnd.cpp` |
| `C_CTRL_EDIT_CTOR` | `CCtrlEdit::CCtrlEdit(...)` | no | `common/CCtrlEdit.cpp` (new) |
| `C_CTRL_EDIT_VFTABLE` | `CCtrlEdit` vftable | no | not directly patched, sanity check only |
| `C_IN_PACKET_DECODE2` | `CInPacket::Decode2()` | no | `common/CInPacket.cpp` (new) |
| `C_FUNC_KEY_MAPPED_MAN_FUNC_KEY_MAPPED` | `CFuncKeyMappedMan::FuncKeyMapped(int)` | no | `common/CFuncKeyMappedMan.cpp` extension |
| `SIZEOF_C_UI_WND_V83_1` | `sizeof(CUIWnd)` at v83.1 (decimal or hex) | no | `runtime/custom_ui_wnd.cpp` placement-new buffer sizing |
| `SIZEOF_C_CTRL_BUTTON_V83_1` | `sizeof(CCtrlButton)` | no | same |
| `SIZEOF_C_CTRL_EDIT_V83_1` | `sizeof(CCtrlEdit)` | no | same |
| `C_UI_WND_VTABLE_SLOT_COUNT` | number of slots in `CUIWnd` vftable (for clone array size) | no | `runtime/vtable_patch.cpp` |

The v95 reference address (`0x009b4590` for `ProcessKey`, `0x004b00f0` for
`ProcessPacket`) is documented in `hook-design.md` for triangulation.

---

## Hooks at a glance

| ID | Target | Address | Signature | Behavior |
|---|---|---|---|---|
| H1 | `CClientSocket::ProcessPacket` | `0x004965F1` | `void __thiscall (CClientSocket*, CInPacket*)` | Peek opcode; if in `0x2000..0x20FF` dispatch to `PacketRegistry` and return. Else restore cursor + call original. |
| H2 | `CWndMan::ProcessKey` | TBD | `long __thiscall (CWndMan*, uint msg, uint vk, long lParam)` | On `WM_KEYDOWN` / `WM_SYSKEYDOWN`, rising-edge only: `HotkeyRegistry::Lookup(vk, mods)`. Match → toggle, return 1. Miss → call original. |
| H3a | `CStage::~CStage` | TBD | `void __thiscall (CStage*)` | Before tear-down: `WindowRegistry::SnapshotAndSuspendVisible()`, set `g_pendingRestore`. Then call original. |
| H3b | `CWndMan::s_Update` | `0x009E47C3` | `CWnd** __cdecl ()` | Call original first. If `g_pendingRestore` set: clear flag, `RestoreSnapshotedVisibility()`. |

All hooks installed via `INITMAPLEHOOK_OR_RETURN(...)` from `common/hooker.h`.

---

## ABI / opcode contract (lock these in)

- Outbound opcode range: `0x0F00..0x0FFF` (client→server)
- Inbound opcode range: `0x2000..0x20FF` (server→client)
- ABI version constant: `0x00010000` (1.0.0)
- All exports `__cdecl extern "C" __declspec(dllexport)`. Resolve by name (never ordinal).
- ABI strings are UTF-8 (`MultiByteToWideChar(CP_UTF8, ...)` conversion at boundary).
- All "0 means error" returns. No exceptions across ABI boundary.

---

## Mutex name

`Local\custom-ui-host-singleton` (created by `CreateMutexW`).

---

## Threading

- All host hooks run on the game UI thread (the WM_SOCKET pump).
- Consumer callbacks therefore run on the game UI thread. No marshaling. Document this contract in the public header.
- Registries take `std::mutex` for cross-thread registration (consumer DLL `MainProc` runs on a separate thread). Dispatch already runs on the UI thread but the lock is acquired anyway (small N, no contention cost).

---

## SEH wrapping

Every dispatch into consumer code is wrapped in `SafeDispatch(siteName,
[&]{...})` (defined in `runtime/seh_dispatch.h`). Filter: only
`EXCEPTION_ACCESS_VIOLATION`; other exceptions propagate. SEH cannot cross
C++ object boundaries so the lambda body must only manipulate POD pointers
and primitive arguments — construction/destruction of stack objects happens
*outside* the lambda. R3 mitigation.

---

## Dependency order (read this before sequencing tasks)

```
PRD/doc corrections (independent — can land any time)
        |
        v
IDA: resolve all 'TBD' v83.1 addresses + sizeof constants
        |
        v
common/ extensions (CUIWnd, CWndMan, CCtrlButton, CCtrlEdit, CInPacket,
                    CFuncKeyMappedMan, CStage typedef)  — depends on memory map
        |
        v
host scaffolding (CMake, dllmain, MainProc, mutex, INI parse)
        |
        +-- registries (pure C++ -- can land in parallel with subsequent rows)
        |       |
        |       v
        |   unit tests
        |
        +-- vtable clone runtime  (depends on host scaffolding + common/ extensions)
        |       |
        |       v
        |   CustomUIWnd
        |
        +-- ABI shims (depends on registries + CustomUIWnd)
        |
        +-- H1, H2, H3 hooks   (depend on registries)
        |
        v
custom-ui-demo (depends on everything above + final ABI)
        |
        v
CI matrix extension + README + manual acceptance
```

---

## Build commands (Linux host — for tests only)

The full DLL build is Windows-only (Visual Studio 17, Win32). The
`tests/` target builds host-architecture-native (Linux is fine for
registries/SEH-mocked tests; SEH test is Windows-only and skipped).

```bash
# Configure with tests enabled (any region/version — tests don't link common_lib)
cmake -S . -B build-tests -DBUILD_REGION=GMS -DBUILD_MAJOR_VERSION=83 -DBUILD_MINOR_VERSION=1 -DBUILD_TESTS=ON
cmake --build build-tests
ctest --test-dir build-tests --output-on-failure
```

The actual Windows DLL build verification will happen via CI on each PR.
Local Windows verification (if available) is:

```pwsh
cmake -B build -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 17" -A Win32 `
  -DBUILD_REGION=GMS -DBUILD_MAJOR_VERSION=83 -DBUILD_MINOR_VERSION=1
cmake --build build --config Debug
```

---

## Where to add memory-map entries

When adding a new symbol:

1. Append `set(SYMBOL_NAME 0xADDR)` (or `0` for unsupported builds) to
   `memory_maps/GMS/v83_1.cmake`, AND every other matrix entry under
   `memory_maps/` (`v87_1.cmake`, `v95_1.cmake`, `v111_1.cmake`,
   `JMS/v185_1.cmake`) — use `0` for any version where the symbol isn't
   resolved yet. The memory-map generator validates `@KEY@` presence per
   build; an absent CMake var crashes the configure step for every region.
2. Append `#define SYMBOL_NAME @SYMBOL_NAME@` to `include/memory_map.h.in`.
   Group by class for readability.

`INITMAPLEHOOK_OR_RETURN` already treats `address == 0` as a soft skip
("unsupported on this build"). The non-v83.1 builds therefore compile and
log a "skipping hook" message; the framework's CMake guard prevents the
host's MainProc from ever running on non-v83.1, so the soft-skip is
defense in depth only.

---

## Glossary

- "edit DLL" — a project under the repo root (e.g., `bypass/`) that builds via `add_edit_dll(...)` and ships to `edits/`.
- "thunk" — a C++ method whose body is `reinterpret_cast<…>(ADDR)(this, …)`. See `common/CFuncKeyMappedMan.cpp` for the canonical pattern.
- "host" — the framework DLL (`custom-ui-host`). Installs hooks, owns registries, exports the C ABI.
- "consumer" — an edit DLL that `LoadLibrary`s the host and consumes its C ABI. The `custom-ui-demo` is the canonical example.
