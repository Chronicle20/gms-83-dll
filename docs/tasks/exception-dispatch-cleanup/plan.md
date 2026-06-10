# Faithful Client Exception Dispatch — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the stubbed `CWvsApp::Run` exception dispatch and the wrong-RTTI `socket_hooks` throws with one shared module that raises the client's own typed exceptions, so the stock `WinMain` handler catches them (fixing world-select migration and clean termination).

**Architecture:** A new `bypass/client_exception.{h,cpp}` module calls our CRT `_CxxThrowException` with the **client's** `_ThrowInfo` pointers (new memory-map keys) for `CDisconnect`/`CTerminate`/`ZException`, calls the client's object builder for `CPatch`, and calls the client's `_com_raise_error`/`_com_raise_errorex` helpers for COM failures. All hook call sites route through it.

**Tech Stack:** C++ (MSVC/Win32, `__thiscall` hooks), CMake memory-map codegen, IDA Pro MCP for per-version address discovery.

**Branch base:** This branch is based on `worktree-task-006-gms-v84-support` (v84 build target present). Rebase onto `main` once task-006 merges.

**Build/verify reality:** This is Linux/WSL — the MSVC client-DLL **cannot compile here**. Local checks are (1) the memory-map completeness gate `cmake -DREGION=<R> -DMAJOR=<M> -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake`, and (2) IDA address cross-checks. Full Debug+Release compile of all six targets is **CI**. Live behavior is the **user smoke test**.

**IDB lane discipline:** `select_instance` is global shared state — never run two IDA tasks concurrently; always `get_metadata` after selecting to confirm the loaded IDB before drawing version conclusions. Ports: GMS v83 `13337`, v87 `13338`, v95 `13339`, JMS185 `13340`, v84 `13341`; GMS v111 has no live IDB (open its `.i64` with `open_file`).

**The 8 keys (per build target):** `C_TI_DISCONNECT_EXCEPTION`, `C_TI_TERMINATE_EXCEPTION`, `C_TI_PATCH_EXCEPTION`, `C_TI_ZEXCEPTION`, `C_PATCH_EXCEPTION_BUILDER`, `C_PATCH_EXCEPTION_BUILDER_KIND` (0 = free fn `(version)->obj*`; 1 = `__thiscall` ctor `(buf,version)`), `C_COM_RAISE_ERROR`, `C_COM_RAISE_ERROR_EX`. (The KIND key was added mid-execution after discovery showed the CPatch builder ABI diverges across versions — v84 is a free fn returning the object pointer, v83/v87 are ctors building into a caller buffer.)

**Per-version discovery procedure (referenced by Tasks 2–6):** In that version's IDB, decompile `CWvsApp::Run` (find via WinMain's call to it, or by the `CPatch/CDisconnect/CTerminate/ZException` `_ThrowInfo` trio). Read directly from its disassembly: the `_com_raise_error(hr,0)` callee → `C_COM_RAISE_ERROR`; the four `_CxxThrowException(..., &_TI…)` `_ThrowInfo` operands → the four `C_TI_*`; the patch-object builder called just before the `CPatch` throw → `C_PATCH_EXCEPTION_BUILDER`, **and record its `C_PATCH_EXCEPTION_BUILDER_KIND`** (0 if it is a free function taking `version` and returning the object pointer; 1 if it is a `__thiscall` constructor called as `ctor(buffer, version)`); the render-failure `_com_raise_errorex(hr)` callee → `C_COM_RAISE_ERROR_EX`. Cross-check each `_ThrowInfo` against the IDA RTTI symbol name (`__TI3?AVCDisconnectException@@`, etc.). **Re-derive every value from that binary — never copy v84's.** Confirm the `m_hrZExceptionCode` code ranges in that version's `Run` match v84's (`0x20000000` / `0x21000000–06` / `0x22000000–0D`); if they differ, record the version's ranges in the signature catalog (Task 9 handles them generically, so divergence only needs documenting).

---

## Task 1: Scaffold the 7 keys + v84 values + signature catalog

**Files:**
- Modify: `include/memory_map.h.in` (append placeholder block)
- Modify: `memory_maps/GMS/v84_1.cmake` (append values)
- Create: `docs/tasks/exception-dispatch-cleanup/signature-catalog.md`

- [ ] **Step 1: Add the 7 placeholders to `include/memory_map.h.in`**

Append after the last `#define` line (`WIN_MAIN_LAUNCHER_STUB`):

```c

// --- Faithful client exception dispatch (docs/tasks/exception-dispatch-cleanup) ---
#define C_TI_DISCONNECT_EXCEPTION @C_TI_DISCONNECT_EXCEPTION@
#define C_TI_TERMINATE_EXCEPTION @C_TI_TERMINATE_EXCEPTION@
#define C_TI_PATCH_EXCEPTION @C_TI_PATCH_EXCEPTION@
#define C_TI_ZEXCEPTION @C_TI_ZEXCEPTION@
#define C_PATCH_EXCEPTION_BUILDER @C_PATCH_EXCEPTION_BUILDER@
#define C_COM_RAISE_ERROR @C_COM_RAISE_ERROR@
#define C_COM_RAISE_ERROR_EX @C_COM_RAISE_ERROR_EX@
```

- [ ] **Step 2: Add the v84 values to `memory_maps/GMS/v84_1.cmake`**

Append at end of file (addresses confirmed from the v84 IDB `CWvsApp::Run` @ `0xA3E7E8` during root-cause analysis):

```cmake

# --- Faithful client exception dispatch (docs/tasks/exception-dispatch-cleanup) ---
set(C_TI_DISCONNECT_EXCEPTION 0x00B9C7B8) # __TI3?AVCDisconnectException@@
set(C_TI_TERMINATE_EXCEPTION  0x00B986C0) # __TI3?AVCTerminateException@@
set(C_TI_PATCH_EXCEPTION       0x00BA72F0) # __TI3?AVCPatchException@@
set(C_TI_ZEXCEPTION            0x00B98E40) # __TI1?AVZException@@
set(C_PATCH_EXCEPTION_BUILDER  0x00527978) # builds CPatchException obj from m_nTargetVersion (Run: v3=sub_527978(this[16]))
set(C_COM_RAISE_ERROR          0x00AAC743) # _com_raise_error(hr,0)  (Run com path: sub_AAC743(v37,0))
set(C_COM_RAISE_ERROR_EX       0x00AABF64) # _com_raise_errorex(hr)  (Run render-fail: sub_AABF64(hr))
```

- [ ] **Step 3: Create the signature catalog with the v84 row**

`docs/tasks/exception-dispatch-cleanup/signature-catalog.md`:

```markdown
# Exception-dispatch keys — per-version address catalog

Anchor: each version's `CWvsApp::Run` exception-dispatch block. Re-derived per
binary, never copied. Verify the IDB with `get_metadata` before recording.

## GMS v84.1 (port 13341) — Run @ 0xA3E7E8
| Key | Address | Anchor |
|---|---|---|
| C_TI_DISCONNECT_EXCEPTION | 0x00B9C7B8 | __TI3?AVCDisconnectException@@; throw @0x21000000-range |
| C_TI_TERMINATE_EXCEPTION  | 0x00B986C0 | __TI3?AVCTerminateException@@; throw @0x22000000-range |
| C_TI_PATCH_EXCEPTION       | 0x00BA72F0 | __TI3?AVCPatchException@@; throw @==0x20000000 |
| C_TI_ZEXCEPTION            | 0x00B98E40 | __TI1?AVZException@@; default throw |
| C_PATCH_EXCEPTION_BUILDER  | 0x00527978 | v3=sub_527978(this[16]); qmemcpy 1288 then throw CPatch |
| C_COM_RAISE_ERROR          | 0x00AAC743 | sub_AAC743(v37,0) on m_hrComErrorCode |
| C_COM_RAISE_ERROR_EX       | 0x00AABF64 | sub_AABF64(hr) on FAILED(render hr) |

Ranges (v84): Patch ==0x20000000 · Disconnect 0x21000000–0x21000006 · Terminate 0x22000000–0x2200000D · else ZException.
```

- [ ] **Step 4: Verify the GMS v84 completeness gate passes**

Run: `cmake -DREGION=GMS -DMAJOR=84 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake`
Expected: PASS (all keys defined). The other five targets will now report these 7 as missing until Tasks 2–6 fill them — that is expected mid-branch.

- [ ] **Step 5: Commit**

```bash
git add include/memory_map.h.in memory_maps/GMS/v84_1.cmake docs/tasks/exception-dispatch-cleanup/signature-catalog.md
git commit -m "feat(exceptions): scaffold 7 exception-dispatch keys + GMS v84 values"
```

---

## Task 2: GMS v83 key values

**Files:** Modify `memory_maps/GMS/v83_1.cmake`; Modify `docs/tasks/exception-dispatch-cleanup/signature-catalog.md`

- [ ] **Step 1: Select + confirm the v83 IDB**

`select_instance(port=13337)` then `get_metadata` — confirm it is GMS v83 before proceeding.

- [ ] **Step 2: Discover the 7 addresses**

Apply the **Per-version discovery procedure** (header). Record each address and the anchor it came from.

- [ ] **Step 3: Append the values to `memory_maps/GMS/v83_1.cmake`**

Use the same block format as Task 1 Step 2 (`set(C_TI_DISCONNECT_EXCEPTION 0x........) # ...`), with the v83 addresses discovered in Step 2.

- [ ] **Step 4: Add the v83 catalog section**

Append a `## GMS v83.1 (port 13337) — Run @ 0x….` table to the signature catalog mirroring Task 1 Step 3, with v83 addresses + anchors + the confirmed ranges.

- [ ] **Step 5: Verify the v83 completeness gate**

Run: `cmake -DREGION=GMS -DMAJOR=83 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake`
Expected: PASS.

- [ ] **Step 6: Commit**

```bash
git add memory_maps/GMS/v83_1.cmake docs/tasks/exception-dispatch-cleanup/signature-catalog.md
git commit -m "feat(exceptions): GMS v83 exception-dispatch key values"
```

---

## Task 3: GMS v87 key values

**Files:** Modify `memory_maps/GMS/v87_1.cmake`; Modify the signature catalog

- [ ] **Step 1:** `select_instance(port=13338)` then `get_metadata` — confirm GMS v87.
- [ ] **Step 2:** Discover the 7 addresses via the Per-version discovery procedure.
- [ ] **Step 3:** Append the 7 `set(...)` lines (Task 1 Step 2 format) with v87 addresses to `memory_maps/GMS/v87_1.cmake`.
- [ ] **Step 4:** Append a `## GMS v87.1` catalog section with addresses + anchors + ranges.
- [ ] **Step 5:** Run `cmake -DREGION=GMS -DMAJOR=87 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake` → Expected: PASS.
- [ ] **Step 6:** Commit:
```bash
git add memory_maps/GMS/v87_1.cmake docs/tasks/exception-dispatch-cleanup/signature-catalog.md
git commit -m "feat(exceptions): GMS v87 exception-dispatch key values"
```

---

## Task 4: GMS v95 key values

**Files:** Modify `memory_maps/GMS/v95_1.cmake`; Modify the signature catalog

- [ ] **Step 1:** `select_instance(port=13339)` then `get_metadata` — confirm GMS v95.
- [ ] **Step 2:** Discover the 7 addresses via the Per-version discovery procedure.
- [ ] **Step 3:** Append the 7 `set(...)` lines with v95 addresses to `memory_maps/GMS/v95_1.cmake`.
- [ ] **Step 4:** Append a `## GMS v95.1` catalog section.
- [ ] **Step 5:** Run `cmake -DREGION=GMS -DMAJOR=95 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake` → Expected: PASS.
- [ ] **Step 6:** Commit:
```bash
git add memory_maps/GMS/v95_1.cmake docs/tasks/exception-dispatch-cleanup/signature-catalog.md
git commit -m "feat(exceptions): GMS v95 exception-dispatch key values"
```

---

## Task 5: JMS v185 key values

**Files:** Modify `memory_maps/JMS/v185_1.cmake`; Modify the signature catalog

- [ ] **Step 1:** `select_instance(port=13340)` then `get_metadata` — confirm JMS v185.
- [ ] **Step 2:** Discover the 7 addresses. Note: JMS exception type names may differ (`ZException` is shared; confirm `CDisconnect/CTerminate/CPatch` exist — JMS `Run` is the authority). Record any JMS-specific code ranges in the catalog.
- [ ] **Step 3:** Append the 7 `set(...)` lines with JMS185 addresses to `memory_maps/JMS/v185_1.cmake`.
- [ ] **Step 4:** Append a `## JMS v185.1` catalog section, flagging any range/type differences.
- [ ] **Step 5:** Run `cmake -DREGION=JMS -DMAJOR=185 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake` → Expected: PASS.
- [ ] **Step 6:** Commit:
```bash
git add memory_maps/JMS/v185_1.cmake docs/tasks/exception-dispatch-cleanup/signature-catalog.md
git commit -m "feat(exceptions): JMS v185 exception-dispatch key values"
```

---

## Task 6: GMS v111 key values (no live IDB)

**Files:** Modify `memory_maps/GMS/v111_1.cmake`; Modify the signature catalog

- [ ] **Step 1: Open the v111 IDB**

GMS v111 has no pre-loaded instance. Open its database: `open_file` with the v111 `.i64` (same `IDBs_v9\GMS\v111…` location family as the other GMS IDBs — confirm the exact path from `list_instances` host config or the on-disk IDB folder). Then `get_metadata` to confirm GMS v111. **If the v111 IDB/binary is unavailable, STOP and escalate** — these 7 keys cannot be derived for v111 without it, and the v111 target will fail the completeness gate.

- [ ] **Step 2:** Discover the 7 addresses via the Per-version discovery procedure.
- [ ] **Step 3:** Append the 7 `set(...)` lines with v111 addresses to `memory_maps/GMS/v111_1.cmake`.
- [ ] **Step 4:** Append a `## GMS v111.1` catalog section.
- [ ] **Step 5:** Run `cmake -DREGION=GMS -DMAJOR=111 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake` → Expected: PASS.
- [ ] **Step 6:** Commit:
```bash
git add memory_maps/GMS/v111_1.cmake docs/tasks/exception-dispatch-cleanup/signature-catalog.md
git commit -m "feat(exceptions): GMS v111 exception-dispatch key values"
```

---

## Task 7: Create the `client_exception` header

**Files:** Create `bypass/client_exception.h`

- [ ] **Step 1: Write the header**

```cpp
#pragma once
#include <windows.h>

// Faithful client-RTTI exception raises. Each unwinds into the stock client
// WinMain handler via the client's own _ThrowInfo / COM-raise helpers. None return.

[[noreturn]] void RaiseClientException(int code); // maps m_hrZExceptionCode -> typed throw
[[noreturn]] void RaiseDisconnect(int code);      // 0x21000000-range (server migrate)
[[noreturn]] void RaiseTerminate(int code);       // 0x22000000-range
[[noreturn]] void RaisePatch();                   // ==0x20000000
[[noreturn]] void RaiseZException(int code);      // default
[[noreturn]] void RaiseComError(HRESULT hr);      // _com_raise_error   (C_COM_RAISE_ERROR)
[[noreturn]] void RaiseComErrorEx(HRESULT hr);    // _com_raise_errorex (C_COM_RAISE_ERROR_EX)
```

- [ ] **Step 2: Commit**

```bash
git add bypass/client_exception.h
git commit -m "feat(exceptions): client_exception raise API"
```

---

## Task 8: Implement the `client_exception` module + wire into the build

**Files:** Create `bypass/client_exception.cpp`; Modify `bypass/CMakeLists.txt`

- [ ] **Step 1: Write the implementation**

`bypass/client_exception.cpp`:

```cpp
#include "client_exception.h"
#include "memory_map.h"
#include "CWvsApp.h"

// MSVC CRT EH entry; we pass the CLIENT's _ThrowInfo so the client's
// __CxxFrameHandler (installed by WinMain) matches the catch by its own RTTI.
extern "C" void __stdcall _CxxThrowException(void* pObject, void* pThrowInfo);

namespace {
// Calling conventions confirmed from the v84 Run disasm in Task 1; re-confirm if a
// later version's Run uses a different cc and gate this typedef accordingly.
using ComRaiseFn    = void(__cdecl*)(HRESULT, int);          // _com_raise_error(hr, 0)
using ComRaiseExFn  = void(__cdecl*)(HRESULT);               // _com_raise_errorex(hr)
using PatchFreeFn   = void*(__cdecl*)(int);                  // KIND 0: builder(version) -> obj*
using PatchCtorFn   = void*(__thiscall*)(void*, int);        // KIND 1: ctor(buf, version) -> buf
} // namespace

[[noreturn]] void RaiseDisconnect(int code) {
    _CxxThrowException(&code, reinterpret_cast<void*>(C_TI_DISCONNECT_EXCEPTION));
    __assume(0);
}

[[noreturn]] void RaiseTerminate(int code) {
    _CxxThrowException(&code, reinterpret_cast<void*>(C_TI_TERMINATE_EXCEPTION));
    __assume(0);
}

[[noreturn]] void RaiseZException(int code) {
    _CxxThrowException(&code, reinterpret_cast<void*>(C_TI_ZEXCEPTION));
    __assume(0);
}

[[noreturn]] void RaisePatch() {
    // The client's CPatchException builder has a per-version ABI, captured by
    // C_PATCH_EXCEPTION_BUILDER_KIND: KIND 0 = free fn returning the object
    // pointer (v84); KIND 1 = __thiscall ctor constructing into a caller buffer
    // (v83/v87). _CxxThrowException copies the object (size per the client
    // _ThrowInfo) synchronously before any unwinding, so a stack object is safe.
    int version = CWvsApp::GetInstance()->m_nTargetVersion;
#if (C_PATCH_EXCEPTION_BUILDER_KIND == 1)
    unsigned char buf[2048]; // >= any version's CPatchException (v84 = 1288)
    auto ctor = reinterpret_cast<PatchCtorFn>(C_PATCH_EXCEPTION_BUILDER);
    void* obj = ctor(buf, version);
#else
    auto build = reinterpret_cast<PatchFreeFn>(C_PATCH_EXCEPTION_BUILDER);
    void* obj = build(version);
#endif
    _CxxThrowException(obj, reinterpret_cast<void*>(C_TI_PATCH_EXCEPTION));
    __assume(0);
}

[[noreturn]] void RaiseClientException(int code) {
    if (code == 0x20000000) {
        RaisePatch();
    }
    if (code >= 0x21000000 && code <= 0x21000006) {
        RaiseDisconnect(code);
    }
    // Terminate upper bound is the UNION of per-version maxima: v83/v84/v87 = 0x2200000D,
    // v95 = 0x2200000E (re-confirm/extend after JMS185 + v111 discovery). Safe because a
    // terminate code only occurs in the version that defines it, so widening never
    // misclassifies a code another version would route elsewhere.
    if (code >= 0x22000000 && code <= 0x2200000E) {
        RaiseTerminate(code);
    }
    RaiseZException(code);
}

[[noreturn]] void RaiseComError(HRESULT hr) {
    reinterpret_cast<ComRaiseFn>(C_COM_RAISE_ERROR)(hr, 0);
    __assume(0);
}

[[noreturn]] void RaiseComErrorEx(HRESULT hr) {
    reinterpret_cast<ComRaiseExFn>(C_COM_RAISE_ERROR_EX)(hr);
    __assume(0);
}
```

- [ ] **Step 2: Add the source to `bypass/CMakeLists.txt`**

Insert `client_exception.cpp` into the `add_edit_dll(bypass SOURCES ...)` list (after `app_hooks.cpp`):

```cmake
add_edit_dll(bypass SOURCES
    dllmain.cpp
    bypass_main.cpp
    socket_hooks.cpp
    login_hooks.cpp
    security_hooks.cpp
    app_hooks.cpp
    client_exception.cpp
    key_mapped_hooks.cpp
)
```

- [ ] **Step 3: Confirm the v84 helper calling conventions**

In the v84 IDB, inspect `sub_AAC743`, `sub_AABF64`, `sub_527978` prologues/`retn N` to confirm `__cdecl` (caller-cleaned, `retn`) vs `__stdcall`/`__thiscall`. Adjust the three `using` typedefs in Step 1 if needed and note the cc in the signature catalog. (Pure verification; full compile is CI.)

- [ ] **Step 4: Commit**

```bash
git add bypass/client_exception.cpp bypass/CMakeLists.txt
git commit -m "feat(exceptions): implement client_exception raises + build wiring"
```

---

## Task 9: Migrate `CWvsApp::Run` + `SetUp` hooks

**Files:** Modify `bypass/app_hooks.cpp`

- [ ] **Step 1: Add the include**

At the top of `bypass/app_hooks.cpp`, add `#include "client_exception.h"` alongside the existing includes.

- [ ] **Step 2: Replace the Run hook error-code dispatch**

In `CWvsApp__Run_Hook`, replace the current diagnostic block (the `int isError`/`int m_hrComErrorCode` logic through both `Log("Do proper _com_raise_error …"); return;` branches) with the faithful dispatch, preserving the client's order (COM first, then Z-code):

```cpp
                if (pThis->m_hrComErrorCode) {
                    HRESULT hr = pThis->m_hrComErrorCode;
                    pThis->m_hrComErrorCode = 0;
                    pThis->m_hrZExceptionCode = 0;
                    RaiseComError(hr);
                }
                if (pThis->m_hrZExceptionCode) {
                    int code = pThis->m_hrZExceptionCode;
                    pThis->m_hrComErrorCode = 0;
                    pThis->m_hrZExceptionCode = 0;
                    RaiseClientException(code);
                }
```

- [ ] **Step 3: Replace the two Run render-failure stubs**

Replace each `if (FAILED(hr)) { Log("Do proper _com_raise_errorex"); return; }` (the `GetnextRenderTime` and `RenderFrame` paths) with:

```cpp
            if (FAILED(hr)) {
                RaiseComErrorEx(hr);
            }
```

- [ ] **Step 4: Replace the SetUp render-failure stub**

In `CWvsApp__SetUp_Hook` (the `>=95`/JMS `RenderFrame` block), replace its `Log("Do proper _com_raise_errorex"); return;` with `RaiseComErrorEx(hr);` (drop the `return`).

- [ ] **Step 5: Remove the dead commented stub**

Delete the commented-out `CTerminateException ex(...)` / `_CxxThrowException(exceptionObject, &_TI3_AVCTerminateException__)` lines (~140–142).

- [ ] **Step 6: Verify the edits (grep — full compile is CI; `windows.h` is absent on this Linux box, so no local compile/preprocess of these `.cpp`s)**

Run: `grep -n "RaiseComError\|RaiseClientException\|RaiseComErrorEx\|client_exception.h" bypass/app_hooks.cpp` → Expected: the include + the three call sites present.
Run: `grep -n "Do proper _com_raise\|_TI3_AVCTerminateException" bypass/app_hooks.cpp` → Expected: no matches (all stubs and the dead commented throw removed).

- [ ] **Step 7: Commit**

```bash
git add bypass/app_hooks.cpp
git commit -m "feat(exceptions): faithful dispatch in CWvsApp::Run + SetUp hooks"
```

---

## Task 10: Migrate `socket_hooks` + retire the std exception classes

**Files:** Modify `bypass/socket_hooks.cpp`; Delete `common/CTerminateException.h`

- [ ] **Step 1: Swap the includes in `socket_hooks.cpp`**

Replace `#include "CTerminateException.h"` with `#include "client_exception.h"`.

- [ ] **Step 2: Migrate the explicit throws**

Replace every `throw CTerminateException(570425351);` with `RaiseTerminate(0x22000007);` (`570425351 == 0x22000007`) and every `throw CPatchException();` with `RaisePatch();`.

- [ ] **Step 3: Migrate the remaining logged exception-code stubs**

`OnConnect`'s failure branch currently logs codes and `return 0;`. Confirm against the v84 `CClientSocket::OnConnect` disasm whether the real path raises, then replace:
- `Log("CClientSocket::OnConnect 570425345"); return 0;` → `RaiseTerminate(570425345);` (`0x22000001`)
- `Log("CClientSocket::OnConnect 553648129"); return 0;` → `RaiseDisconnect(553648129);` (`0x21000001`)
- the ADR `Connect` `Log("...Should throw an exception here.")` stub → the raise the disasm dictates (record the chosen code in the signature catalog).

If the disasm shows the hook intentionally returns `0` (does not throw) at any of these, leave it and note why in the catalog rather than forcing a raise.

- [ ] **Step 4: Delete the retired header**

```bash
git rm common/CTerminateException.h
```
Then grep for any other includer: `grep -rn "CTerminateException.h" .` — Expected: no remaining references. Remove any that exist.

- [ ] **Step 5: Verify the edits (grep — full compile is CI)**

Run: `grep -rn "CTerminateException" bypass/ common/` → Expected: no matches (header deleted, all throws migrated).
Run: `grep -n "RaiseTerminate\|RaisePatch\|RaiseDisconnect\|client_exception.h" bypass/socket_hooks.cpp` → Expected: the include + migrated raises present.

- [ ] **Step 6: Commit**

```bash
git add bypass/socket_hooks.cpp common/CTerminateException.h
git commit -m "feat(exceptions): route socket_hooks through client_exception; retire std classes"
```

---

## Task 11: Final verification + docs

**Files:** Create `docs/tasks/exception-dispatch-cleanup/report.md`

- [ ] **Step 1: Run all six completeness gates**

```bash
cmake -DREGION=GMS -DMAJOR=83  -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake
cmake -DREGION=GMS -DMAJOR=84  -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake
cmake -DREGION=GMS -DMAJOR=87  -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake
cmake -DREGION=GMS -DMAJOR=95  -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake
cmake -DREGION=GMS -DMAJOR=111 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake
cmake -DREGION=JMS -DMAJOR=185 -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake
```
Expected: all six PASS (the 7 new keys defined for every target).

- [ ] **Step 2: Confirm no stub strings remain**

Run: `grep -rn "Do proper _com_raise" bypass/`
Expected: no matches (every stub replaced).

- [ ] **Step 3: Write the report**

`docs/tasks/exception-dispatch-cleanup/report.md`: summarize what changed (module, call sites, retired header), the 7×6 key table (link the signature catalog), the build/verify status (CI is authoritative for compile; this box ran only completeness gates + IDA cross-checks), and the pending live smoke test (depends on the atlas-ms client-key fix landing).

- [ ] **Step 4: Commit**

```bash
git add docs/tasks/exception-dispatch-cleanup/report.md
git commit -m "docs(exceptions): completion report + verification status"
```

- [ ] **Step 5: Push + open PR for CI**

```bash
git push -u origin worktree-exception-dispatch-cleanup
```
Open a PR (base: `worktree-task-006-gms-v84-support` until task-006 merges, then retarget `main`). CI compiling all six Debug+Release is the authoritative build check. Then hand off the live v84 smoke test (login → select world → channel migrate succeeds; clean exit works) to the user.

---

## Notes

- **No runtime unit tests:** the DLL only runs inside the packed Win32 client; there is no host harness on this box. Verification is the completeness gate + IDA address cross-checks locally, CI for the full compile, and the user smoke test for behavior. This is the established pattern for this repo (task-006).
- **Diagnostic logging (`5d750b5`):** Task 9 Step 2 replaces those `Log(... m_hr…)` lines, so the diagnostic is superseded automatically.
- **Cross-module EH assumption:** if any version's client uses a non-standard frame handler, its raise paths won't be caught — caught by the per-version IDA cross-check + smoke test; v84 is proven first.
