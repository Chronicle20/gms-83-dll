# Custom UI Framework Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking. Companion docs: `prd.md`, `design.md`, `hook-design.md`, `risks.md`, `context.md`.

**Goal:** Ship a reusable in-DLL UI framework (`custom-ui-host`) + a working demo edit (`custom-ui-demo`) that proves the entire pipeline (window, hotkey, custom inbound/outbound opcodes) on GMS v83.1.

**Architecture:** A new "host" edit DLL installs three Detours hooks (H1: ProcessPacket inbound dispatch, H2: ProcessKey hotkey toggle, H3: stage-end auto-hide via CStage dtor + s_Update restore latch), owns three thread-safe registries (window/hotkey/packet), and exposes a stable `extern "C" __cdecl` ABI via `GetProcAddress`. Consumer DLLs `LoadLibrary` the host and call `CustomUI_*` to declare UI. Game-class surface is extended in `common/` via the existing address-thunk pattern. Custom windows are byte-buffer-backed `CUIWnd` instances with a shared cloned vtable for OnDraw/OnSetFocus/dtor overrides; `CCtrlButton` uses per-instance vtable clones for click identity.

**Tech Stack:** C++17, MSVC v143, Win32 (x86), Microsoft Detours, GoogleTest 1.14, CMake 3.26, Microsoft SEH.

---

## Phase 0 — PRD/doc corrections (independent, can land any time)

### Task 0.1: Correct MinHook references to Detours in PRD and hook-design

**Files:**
- Modify: `docs/tasks/task-005-custom-ui-framework/prd.md` (search/replace "MinHook")
- Modify: `docs/tasks/task-005-custom-ui-framework/hook-design.md` (search/replace "MinHook")

- [ ] **Step 1: Replace MinHook reference in PRD §5**

In `prd.md` find:
```
The host installs **three** detours via MinHook (the existing `hooker.h` pattern).
```
Replace with:
```
The host installs **three** detours via Microsoft Detours (the existing `hooker.h` pattern around `SetHook` + `INITMAPLEHOOK_OR_RETURN`).
```

- [ ] **Step 2: Replace MinHook reference in PRD §8.4**

In `prd.md` find:
```
- No new IAT patches; all hooks are MinHook detours, matching the existing
  `bypass/` and `enable-minimize/` patterns. Themida tolerance is therefore the
  same as the existing edits.
- No code is written into the game's `.text` section beyond what MinHook itself
  writes.
```
Replace with:
```
- No new IAT patches; all hooks are Microsoft Detours trampolines, matching the existing
  `bypass/` and `enable-minimize/` patterns. Themida tolerance is therefore the
  same as the existing edits.
- No code is written into the game's `.text` section beyond what Detours itself
  writes.
```

- [ ] **Step 3: Replace MinHook reference in hook-design.md preamble**

In `hook-design.md` find:
```
All hooks are MinHook detours using the existing `INITWINHOOK`-style helper
pattern from `common/hooker.h` (adapted for in-process function-address hooks
rather than IAT entries — see `bypass/dllmain.cpp` for an example).
```
Replace with:
```
All hooks are Microsoft Detours trampolines using the existing `INITMAPLEHOOK_OR_RETURN`
helper from `common/hooker.h`. See `bypass/socket_hooks.cpp` for an example.
```

- [ ] **Step 4: Replace MinHook reference in hook-design.md "Uninstall" section**

In `hook-design.md` find:
```
loaded until process exit. This matches every other edit in the project.
If a future use case demands hot-unload, MinHook supports `MH_DisableHook`
and the cleanup ordering would be: unregister all consumer handlers,
disable H1+H2+H3, then unload. Not in milestone scope.
```
Replace with:
```
loaded until process exit. This matches every other edit in the project.
If a future use case demands hot-unload, Detours supports
`DetourTransactionBegin` + `DetourDetach`, and the cleanup ordering would be:
unregister all consumer handlers, detach H1+H2+H3 inside a single transaction,
then unload. Not in milestone scope.
```

- [ ] **Step 5: Commit**

```bash
git add docs/tasks/task-005-custom-ui-framework/prd.md docs/tasks/task-005-custom-ui-framework/hook-design.md
git commit -m "docs(task-005): correct MinHook references to Microsoft Detours"
```

---

## Phase 1 — Memory map: resolve v83.1 addresses via IDA

These tasks require an IDA session loaded against the v83.1 client. Each
task confirms one symbol; the executing agent should call `get_metadata`
first to verify the IDB before recording an address (per the user's
"verify IDA target" memory rule). Addresses come from disassembly evidence,
not guesswork.

For each TBD symbol, the agent must:
1. Confirm v83.1 IDB is loaded.
2. Locate the symbol (typically by xref-walking from a known nearby v95
   address, or by string-search for the mangled name).
3. Record the address in `memory_maps/GMS/v83_1.cmake` (placed grouped by
   class, alphabetically).
4. Add `0` placeholders in `memory_maps/GMS/v87_1.cmake`,
   `memory_maps/GMS/v95_1.cmake`, `memory_maps/GMS/v111_1.cmake`,
   `memory_maps/JMS/v185_1.cmake` for the same symbol (per
   "Where to add memory-map entries" in `context.md`).
5. Add `#define <SYM> @<SYM>@` to `include/memory_map.h.in`.

The non-v83.1 placeholders are required by `GenerateMemoryMap.cmake`: it
fails fast on a missing `@KEY@`. Use `0` (which `INITMAPLEHOOK_OR_RETURN`
treats as a soft skip).

### Task 1.1: Resolve `C_WND_MAN_PROCESS_KEY`

**Files:**
- Modify: `memory_maps/GMS/v83_1.cmake`, `memory_maps/GMS/v87_1.cmake`, `memory_maps/GMS/v95_1.cmake`, `memory_maps/GMS/v111_1.cmake`, `memory_maps/JMS/v185_1.cmake`, `include/memory_map.h.in`

- [ ] **Step 1: Confirm IDB**

Run: MCP `get_metadata` and verify the path matches the v83.1 IDB.

- [ ] **Step 2: Locate `CWndMan::ProcessKey(uint, uint, long)`**

In IDA: search for the mangled name `?ProcessKey@CWndMan@@QAEJIIJ@Z` via
`mcp__ida-pro__get_function_by_name`. If not present, scan
`mcp__ida-pro__list_functions_filter` for `ProcessKey`. The v95 reference
is `0x009b4590` — use it as a triangulation anchor when sanity-checking
the v83 address (xrefs from the WM_KEYDOWN handler in `CWvsApp::WndProc`
should land on it).

- [ ] **Step 3: Record address**

In `memory_maps/GMS/v83_1.cmake`, add a new line grouped with the
existing `C_WND_MAN_*` entries (alphabetical):
```cmake
set(C_WND_MAN_PROCESS_KEY 0xXXXXXXXX)  # CWndMan::ProcessKey(uint, uint, long)
```

In `memory_maps/GMS/v87_1.cmake`, `v95_1.cmake`, `v111_1.cmake`, and
`memory_maps/JMS/v185_1.cmake`, add `set(C_WND_MAN_PROCESS_KEY 0)` in
the same group.

- [ ] **Step 4: Add to memory_map.h.in**

In `include/memory_map.h.in`, add (grouped with other `C_WND_MAN_*`):
```c
#define C_WND_MAN_PROCESS_KEY @C_WND_MAN_PROCESS_KEY@
```

- [ ] **Step 5: Configure-test all five matrix combos**

Run from a Linux/WSL shell — configure-only is enough; we only care that
`GenerateMemoryMap.cmake` accepts the new `@KEY@`.
```bash
for v in 83 87 95 111; do
  cmake -S . -B "build-cfg-gms-$v" -DBUILD_REGION=GMS -DBUILD_MAJOR_VERSION=$v -DBUILD_MINOR_VERSION=1 -DBUILD_TESTS=OFF 2>&1 | tail -5
done
cmake -S . -B build-cfg-jms-185 -DBUILD_REGION=JMS -DBUILD_MAJOR_VERSION=185 -DBUILD_MINOR_VERSION=1 -DBUILD_TESTS=OFF 2>&1 | tail -5
```
Expected: every configure prints `-- Configuring done` (no `Missing keys:`).

- [ ] **Step 6: Commit**

```bash
git add memory_maps/ include/memory_map.h.in
git commit -m "memmap(task-005): resolve CWndMan::ProcessKey v83.1 address"
```

### Task 1.2: Resolve `C_STAGE_DTOR`

**Files:**
- Modify: same five memory maps + `include/memory_map.h.in`

- [ ] **Step 1: Confirm IDB (`get_metadata` shows v83.1)**

- [ ] **Step 2: Locate `CStage::~CStage()`**

Search: mangled `??1CStage@@UAE@XZ` via `mcp__ida-pro__get_function_by_name`.
If absent, walk vftable references from `STAGE_INSTANCE_ADDR` (already
mapped as `0x00BEDED4`) — vftable slot 0 typically references the dtor on
MSVC layouts (scalar deleting destructor wraps the real dtor; either is
acceptable as a hook target so long as it fires exactly once per stage
tear-down).

- [ ] **Step 3: Record address**

Add `set(C_STAGE_DTOR 0xXXXXXXXX)` (v83.1) and `set(C_STAGE_DTOR 0)` in the
other four. Group with `C_STAGE_*` symbols.

- [ ] **Step 4: Add `#define C_STAGE_DTOR @C_STAGE_DTOR@` to memory_map.h.in**

- [ ] **Step 5: Configure-test all five matrix combos (see Task 1.1 Step 5)**

- [ ] **Step 6: Commit**

```bash
git add memory_maps/ include/memory_map.h.in
git commit -m "memmap(task-005): resolve CStage::~CStage v83.1 address"
```

### Task 1.3: Resolve `C_WND_MAN_REGISTER_UI_WINDOW` + `C_WND_MAN_UNREGISTER_UI_WINDOW`

**Files:** same.

- [ ] **Step 1: Confirm IDB (v83.1)**

- [ ] **Step 2: Locate both functions**

Mangled names: `?RegisterUIWindow@CWndMan@@QAEXPAVCUIWnd@@@Z`,
`?UnregisterUIWindow@CWndMan@@QAEXPAVCUIWnd@@@Z`. Both should be in the
same `CWndMan` member-function cluster near `s_Update` (`0x009E47C3`).

- [ ] **Step 3: Record both addresses in all five memory maps**

v83.1: real addresses. Other four: `0` placeholders.

- [ ] **Step 4: Add both `#define`s to memory_map.h.in (grouped with `C_WND_MAN_*`)**

- [ ] **Step 5: Configure-test**

- [ ] **Step 6: Commit**

```bash
git commit -m "memmap(task-005): resolve CWndMan UI window register/unregister v83.1"
```

### Task 1.4: Resolve `C_UI_WND_CTOR`, `C_UI_WND_VFTABLE`, `C_UI_WND_DTOR`, `SIZEOF_C_UI_WND_V83_1`, `C_UI_WND_VTABLE_SLOT_COUNT`

**Files:** same.

- [ ] **Step 1: Confirm IDB (v83.1)**

- [ ] **Step 2: Locate `CUIWnd::CUIWnd` constructor**

Mangled `??0CUIWnd@@QAE@...`. The signature historically is `(x, y, w, h,
name, ...)` — confirm via prototype recovery (`mcp__ida-pro__decompile_function`).
Pick the constructor overload the framework will call (the one accepting
position+size+name); record only that address. Comment the chosen overload
inside `v83_1.cmake`.

- [ ] **Step 3: Locate `CUIWnd` vftable**

Use `mcp__ida-pro__search_structures` or scan rdata xrefs from the ctor:
the first `mov dword ptr [ecx], <addr>` inside the ctor writes the vptr,
and `<addr>` is the vftable.

- [ ] **Step 4: Locate `CUIWnd::~CUIWnd` destructor**

Mangled `??1CUIWnd@@UAE@XZ`. The dtor slot in the vftable above is the
quickest reference.

- [ ] **Step 5: Determine `sizeof(CUIWnd)` at v83.1**

Use `mcp__ida-pro__get_struct_info_simple` on `CUIWnd` or, if absent,
analyze the ctor's stack-frame allocation. Record as decimal — e.g.
`set(SIZEOF_C_UI_WND_V83_1 256)`.

- [ ] **Step 6: Determine vtable slot count**

Walk vftable entries until the first non-function-pointer (typically the
next vftable in rdata starts here). Record as decimal — e.g.
`set(C_UI_WND_VTABLE_SLOT_COUNT 24)`.

- [ ] **Step 7: Record all five symbols in all five memory maps**

v83.1: real values. Other four: `0` placeholders for all five symbols
(including the sizes — `0` is a valid sentinel; the host's static_assert
catches mismatch at compile time, but on non-v83.1 the host's CMake
guard already skips the build).

- [ ] **Step 8: Add all five `#define`s to memory_map.h.in**

- [ ] **Step 9: Configure-test**

- [ ] **Step 10: Commit**

```bash
git commit -m "memmap(task-005): resolve CUIWnd ctor/vftable/dtor/size v83.1"
```

### Task 1.5: Resolve `C_CTRL_BUTTON_CTOR`, `C_CTRL_BUTTON_VFTABLE`, `SIZEOF_C_CTRL_BUTTON_V83_1`

Same pattern as 1.4 but for `CCtrlButton`. Mangled `??0CCtrlButton@@...`.

- [ ] **Step 1: Confirm IDB**
- [ ] **Step 2: Locate ctor, vftable, sizeof**
- [ ] **Step 3: Record in all five memory maps**
- [ ] **Step 4: Add `#define`s to memory_map.h.in**
- [ ] **Step 5: Configure-test**
- [ ] **Step 6: Commit** — `memmap(task-005): resolve CCtrlButton ctor/vftable/size v83.1`

### Task 1.6: Resolve `C_CTRL_EDIT_CTOR`, `C_CTRL_EDIT_VFTABLE`, `SIZEOF_C_CTRL_EDIT_V83_1`

Same pattern. Mangled `??0CCtrlEdit@@...`.

- [ ] **Step 1: Confirm IDB**
- [ ] **Step 2: Locate ctor, vftable, sizeof**
- [ ] **Step 3: Record in all five memory maps**
- [ ] **Step 4: Add `#define`s to memory_map.h.in**
- [ ] **Step 5: Configure-test**
- [ ] **Step 6: Commit** — `memmap(task-005): resolve CCtrlEdit ctor/vftable/size v83.1`

### Task 1.7: Resolve `C_IN_PACKET_DECODE2`

- [ ] **Step 1: Confirm IDB**

- [ ] **Step 2: Locate `CInPacket::Decode2()`**

Mangled `?Decode2@CInPacket@@QAEGXZ`. v95 reference exists inside the
top of `CClientSocket::ProcessPacket` (`0x004b00f0`); for v83 the same
call site is at the top of `0x004965F1`. Use
`mcp__ida-pro__get_callees(0x004965F1)` to enumerate; `Decode2` is one of
the early calls.

- [ ] **Step 3: Record in all five memory maps**
- [ ] **Step 4: Add `#define` to memory_map.h.in**
- [ ] **Step 5: Configure-test**
- [ ] **Step 6: Commit** — `memmap(task-005): resolve CInPacket::Decode2 v83.1`

### Task 1.8: Resolve `C_FUNC_KEY_MAPPED_MAN_FUNC_KEY_MAPPED`

- [ ] **Step 1: Confirm IDB**

- [ ] **Step 2: Locate `CFuncKeyMappedMan::FuncKeyMapped(int)`**

Mangled `?FuncKeyMapped@CFuncKeyMappedMan@@QAE?AUFUNCKEY_MAPPED@@H@Z` (or
similar — confirm by signature recovery). Likely near
`C_FUNC_KEY_MAPPED_MAN` (`0x0058DD0D`).

- [ ] **Step 3: Record + h.in + configure-test**
- [ ] **Step 4: Commit** — `memmap(task-005): resolve CFuncKeyMappedMan::FuncKeyMapped v83.1`

### Task 1.9: Final memory-map sanity sweep

- [ ] **Step 1: List all new symbols added in Phase 1**

Expected new keys (15):
```
C_WND_MAN_PROCESS_KEY, C_STAGE_DTOR, C_WND_MAN_REGISTER_UI_WINDOW,
C_WND_MAN_UNREGISTER_UI_WINDOW, C_UI_WND_CTOR, C_UI_WND_VFTABLE,
C_UI_WND_DTOR, SIZEOF_C_UI_WND_V83_1, C_UI_WND_VTABLE_SLOT_COUNT,
C_CTRL_BUTTON_CTOR, C_CTRL_BUTTON_VFTABLE, SIZEOF_C_CTRL_BUTTON_V83_1,
C_CTRL_EDIT_CTOR, C_CTRL_EDIT_VFTABLE, SIZEOF_C_CTRL_EDIT_V83_1,
C_IN_PACKET_DECODE2, C_FUNC_KEY_MAPPED_MAN_FUNC_KEY_MAPPED
```

- [ ] **Step 2: Grep each in memory_map.h.in and each region cmake**

Run:
```bash
for sym in C_WND_MAN_PROCESS_KEY C_STAGE_DTOR C_WND_MAN_REGISTER_UI_WINDOW \
           C_WND_MAN_UNREGISTER_UI_WINDOW C_UI_WND_CTOR C_UI_WND_VFTABLE \
           C_UI_WND_DTOR SIZEOF_C_UI_WND_V83_1 C_UI_WND_VTABLE_SLOT_COUNT \
           C_CTRL_BUTTON_CTOR C_CTRL_BUTTON_VFTABLE SIZEOF_C_CTRL_BUTTON_V83_1 \
           C_CTRL_EDIT_CTOR C_CTRL_EDIT_VFTABLE SIZEOF_C_CTRL_EDIT_V83_1 \
           C_IN_PACKET_DECODE2 C_FUNC_KEY_MAPPED_MAN_FUNC_KEY_MAPPED; do
  grep -l "$sym" include/memory_map.h.in memory_maps/GMS/v83_1.cmake \
                 memory_maps/GMS/v87_1.cmake memory_maps/GMS/v95_1.cmake \
                 memory_maps/GMS/v111_1.cmake memory_maps/JMS/v185_1.cmake \
   || echo "MISSING in some file: $sym"
done
```
Expected: every symbol appears in all six files. Any "MISSING" line is
a defect — fix before continuing.

- [ ] **Step 3: Configure-test all five matrix combos one more time (see Task 1.1 Step 5)**

If any "Missing keys" or "FATAL_ERROR" appears, fix and re-run.

- [ ] **Step 4: Commit (if any cleanup happened) or skip**

```bash
git status memory_maps/ include/
# if anything's staged:
git commit -m "memmap(task-005): sanity sweep — fix missing symbol entries"
```

---

---

## Phase 2 — `common/` extensions (game-class surface)

Each task adds the method declarations the framework needs to existing
`common/` headers, with `.cpp` thunks following the canonical pattern in
`common/CFuncKeyMappedMan.cpp`. Headers added here become available to
every edit DLL via the PCH (`common/pch.h`).

### Task 2.1: Extend `CInPacket` with `Decode2`

**Files:**
- Modify: `common/CInPacket.h`
- Create: `common/CInPacket.cpp`

- [ ] **Step 1: Add `Decode2()` declaration to `CInPacket.h`**

Replace the existing file contents with:
```cpp
#pragma once

struct CInPacket {
    int m_bLoopback;
    int m_nState;
    ZArray<unsigned char> m_aRecvBuff;
    unsigned __int16 m_uLength;
    unsigned __int16 m_uRawSeq;
    unsigned __int16 m_uDataLen;
    unsigned int m_uOffset;

    unsigned short Decode2();
};
```

- [ ] **Step 2: Create `common/CInPacket.cpp` with the thunk**

```cpp
#include "pch.h"

unsigned short CInPacket::Decode2() {
    return reinterpret_cast<unsigned short(__fastcall *)(CInPacket *, void *)>(
        C_IN_PACKET_DECODE2)(this, nullptr);
}
```

- [ ] **Step 3: Configure-test v83.1 build**

```bash
cmake -S . -B build-cfg-gms-83 -DBUILD_REGION=GMS -DBUILD_MAJOR_VERSION=83 -DBUILD_MINOR_VERSION=1 2>&1 | tail -5
```
Expected: `-- Configuring done` (no errors).

- [ ] **Step 4: Commit**

```bash
git add common/CInPacket.h common/CInPacket.cpp
git commit -m "common: add CInPacket::Decode2 thunk"
```

### Task 2.2: Extend `CWndMan` with `RegisterUIWindow`, `UnregisterUIWindow`, `ProcessKey`

**Files:**
- Modify: `common/CWndMan.h`, `common/CWndMan.cpp`

- [ ] **Step 1: Forward-declare `CUIWnd` in CWndMan.h and add the three methods**

Replace contents:
```cpp
#pragma once

struct CUIWnd;

class CWndMan {
public:
    static CWnd **s_Update();
    static void RedrawInvalidatedWindows();

    void RegisterUIWindow(CUIWnd *pWnd);
    void UnregisterUIWindow(CUIWnd *pWnd);
    long ProcessKey(unsigned int msg, unsigned int vk, long lParam);
};
```

- [ ] **Step 2: Add thunk bodies to CWndMan.cpp**

Append to the existing file (after the existing `RedrawInvalidatedWindows`
definition):
```cpp
void CWndMan::RegisterUIWindow(CUIWnd *pWnd) {
    reinterpret_cast<void(__fastcall *)(CWndMan *, void *, CUIWnd *)>(
        C_WND_MAN_REGISTER_UI_WINDOW)(this, nullptr, pWnd);
}

void CWndMan::UnregisterUIWindow(CUIWnd *pWnd) {
    reinterpret_cast<void(__fastcall *)(CWndMan *, void *, CUIWnd *)>(
        C_WND_MAN_UNREGISTER_UI_WINDOW)(this, nullptr, pWnd);
}

long CWndMan::ProcessKey(unsigned int msg, unsigned int vk, long lParam) {
    return reinterpret_cast<long(__fastcall *)(CWndMan *, void *, unsigned int,
                                               unsigned int, long)>(
        C_WND_MAN_PROCESS_KEY)(this, nullptr, msg, vk, lParam);
}
```

- [ ] **Step 3: Configure-test**

(Same as Task 2.1 Step 3.)

- [ ] **Step 4: Commit**

```bash
git add common/CWndMan.h common/CWndMan.cpp
git commit -m "common: add CWndMan UI registration + ProcessKey thunks"
```

### Task 2.3: Extend `CFuncKeyMappedMan` with `FuncKeyMapped` + singleton accessor

**Files:**
- Modify: `common/CFuncKeyMappedMan.h`, `common/CFuncKeyMappedMan.cpp`

- [ ] **Step 1: Add `FuncKeyMapped` + `GetInstance` to header**

Replace the existing `CFuncKeyMappedMan.h` member-list section so the class
gains the two methods. Final file:
```cpp
#pragma once

class CFuncKeyMappedMan {
public:
    virtual ~CFuncKeyMappedMan() = default;

#if defined(REGION_GMS)
    FUNCKEY_MAPPED m_aFuncKeyMapped[89];
    FUNCKEY_MAPPED m_aFuncKeyMapped_Old[89];
#elif defined(REGION_JMS)
    FUNCKEY_MAPPED m_aFuncKeyMapped[94];
    FUNCKEY_MAPPED m_aFuncKeyMapped_Old[94];
#endif
    int m_aQuickslotKeyMapped[8];
    int m_aQuickslotKeyMapped_Old[8];
    int m_nPetConsumeItemID;
    int m_nPetConsumeMPItemID;
#if defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 111 || defined(REGION_JMS)
    int dummy1;
#endif
#if defined(REGION_JMS)
    int dummy2;
#endif
#if defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95
    int m_nNormalAttackCode;
#endif

    CFuncKeyMappedMan();

    static CFuncKeyMappedMan *GetInstance();
    static void CreateInstance();

    FUNCKEY_MAPPED FuncKeyMapped(int vk);
};
```

- [ ] **Step 2: Add `GetInstance` + `FuncKeyMapped` bodies to .cpp**

Append after the existing `CreateInstance` function:
```cpp
CFuncKeyMappedMan *CFuncKeyMappedMan::GetInstance() {
    return *reinterpret_cast<CFuncKeyMappedMan **>(
        C_FUNC_KEY_MAPPED_MAN_INSTANCE_ADDR);
}

FUNCKEY_MAPPED CFuncKeyMappedMan::FuncKeyMapped(int vk) {
    // Returned struct is 5 bytes (packed); follow MSVC __thiscall calling
    // convention: caller-allocated return-slot pointer passed as hidden
    // first arg in ECX is NOT what the game uses here — small POD returns
    // are returned in EDX:EAX on x86. Confirm via decompile if the
    // generated thunk crashes; fall back to pass-by-reference if needed.
    return reinterpret_cast<FUNCKEY_MAPPED(__fastcall *)(
        CFuncKeyMappedMan *, void *, int)>(
        C_FUNC_KEY_MAPPED_MAN_FUNC_KEY_MAPPED)(this, nullptr, vk);
}
```

- [ ] **Step 3: Configure-test**

- [ ] **Step 4: Commit**

```bash
git add common/CFuncKeyMappedMan.h common/CFuncKeyMappedMan.cpp
git commit -m "common: add CFuncKeyMappedMan::FuncKeyMapped + GetInstance thunks"
```

### Task 2.4: Extend `CCtrlButton` with ctor + `OnClick` placeholder

**Files:**
- Modify: `common/CCtrlButton.h`
- Create: `common/CCtrlButton.cpp`

- [ ] **Step 1: Add ctor declaration to header**

At the end of the existing `CCtrlButton` class body (before the closing
brace), add (and add a default `public:` block if missing):
```cpp
public:
    CCtrlButton(int x, int y, int w, int h, const unsigned short *pwszName,
                const CCtrlButton::CREATEPARAM *pParam);
```

- [ ] **Step 2: Create `common/CCtrlButton.cpp`**

```cpp
#include "pch.h"

CCtrlButton::CCtrlButton(int x, int y, int w, int h,
                         const unsigned short *pwszName,
                         const CCtrlButton::CREATEPARAM *pParam) {
    // The actual game-side signature for CCtrlButton::CCtrlButton is
    // recovered from IDA at port time; the framework calls the ctor via
    // placement-new on a host-owned buffer, so this thunk only needs to
    // forward the args. Confirm parameter order at v83.1 before relying.
    reinterpret_cast<void(__fastcall *)(CCtrlButton *, void *, int, int, int, int,
                                        const unsigned short *,
                                        const CCtrlButton::CREATEPARAM *)>(
        C_CTRL_BUTTON_CTOR)(this, nullptr, x, y, w, h, pwszName, pParam);
}
```

- [ ] **Step 3: Configure-test**

- [ ] **Step 4: Commit**

```bash
git add common/CCtrlButton.h common/CCtrlButton.cpp
git commit -m "common: add CCtrlButton ctor thunk"
```

### Task 2.5: Extend `CCtrlEdit` with ctor

**Files:**
- Modify: `common/CCtrlEdit.h`
- Create: `common/CCtrlEdit.cpp`

- [ ] **Step 1: Add ctor declaration to header**

Append to the struct body (before the closing brace), add a `public:`
block immediately after the opening brace if the struct is `struct CCtrlEdit
: CCtrlWnd { ... };` (which it currently is — default-public is fine, so
no `public:` keyword is needed):
```cpp
CCtrlEdit(int x, int y, int w, int h, const unsigned short *pwszInitText);
```

- [ ] **Step 2: Create `common/CCtrlEdit.cpp`**

```cpp
#include "pch.h"

CCtrlEdit::CCtrlEdit(int x, int y, int w, int h,
                     const unsigned short *pwszInitText) {
    // As with CCtrlButton, parameter order is the recovered v83.1 ctor
    // signature; framework places via placement-new. Adjust if disasm
    // shows additional flags / canvas args.
    reinterpret_cast<void(__fastcall *)(CCtrlEdit *, void *, int, int, int, int,
                                        const unsigned short *)>(
        C_CTRL_EDIT_CTOR)(this, nullptr, x, y, w, h, pwszInitText);
}
```

- [ ] **Step 3: Configure-test**

- [ ] **Step 4: Commit**

```bash
git add common/CCtrlEdit.h common/CCtrlEdit.cpp
git commit -m "common: add CCtrlEdit ctor thunk"
```

### Task 2.6: Extend `CUIWnd` with ctor

**Files:**
- Modify: `common/CUIWnd.h`
- Create: `common/CUIWnd.cpp`

- [ ] **Step 1: Add ctor declaration to header**

Convert the `struct CUIWnd : CWnd { ... };` into one with a public ctor
declaration. Add at the top of the struct body:
```cpp
CUIWnd(int x, int y, int w, int h, const unsigned short *pwszName);
```

- [ ] **Step 2: Create `common/CUIWnd.cpp`**

```cpp
#include "pch.h"

CUIWnd::CUIWnd(int x, int y, int w, int h, const unsigned short *pwszName) {
    // Construct via the game-side ctor at C_UI_WND_CTOR. Parameter order
    // recovered from v83.1 disasm; adjust if it carries additional flags.
    reinterpret_cast<void(__fastcall *)(CUIWnd *, void *, int, int, int, int,
                                        const unsigned short *)>(
        C_UI_WND_CTOR)(this, nullptr, x, y, w, h, pwszName);
}
```

- [ ] **Step 3: Configure-test**

- [ ] **Step 4: Commit**

```bash
git add common/CUIWnd.h common/CUIWnd.cpp
git commit -m "common: add CUIWnd ctor thunk"
```

### Task 2.7: Annotate `CStage` for hookability

**Files:**
- Modify: `common/CStage.h`

- [ ] **Step 1: Document the dtor hook target as a comment**

`CStage` is hooked at the dtor address from the memory map. We don't need
to declare the dtor as a virtual method here (the existing class layout
inherits virtual destruction via `ZRefCounted`); the address itself is the
hook target. Add a comment block above the class clarifying that, so a
future reader doesn't add a duplicate dtor declaration.

In `common/CStage.h`, change:
```cpp
class CStage : public IGObj, public IUIMsgHandler, public INetMsgHandler, public ZRefCounted {
public:
    void OnMouseEnter(int) override;

    void OnPacket(int, CInPacket *) override;
};
```
to:
```cpp
// CStage::~CStage() is hooked by edits via the memory-map address
// C_STAGE_DTOR. We deliberately do not declare ~CStage here — the
// virtual destruction slot is inherited from ZRefCounted and the hook
// is installed against the recovered v83.1 address directly.
class CStage : public IGObj, public IUIMsgHandler, public INetMsgHandler, public ZRefCounted {
public:
    void OnMouseEnter(int) override;

    void OnPacket(int, CInPacket *) override;
};
```

- [ ] **Step 2: Configure-test**

- [ ] **Step 3: Commit**

```bash
git add common/CStage.h
git commit -m "common: document CStage dtor hook target"
```

### Task 2.8: Compile-link verification of all common/ extensions

**Files:** none (verification only).

- [ ] **Step 1: Run a full Windows build of common_lib only, via CMake's per-target build**

If running on Windows:
```pwsh
cmake -B build -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 17" -A Win32 `
  -DBUILD_REGION=GMS -DBUILD_MAJOR_VERSION=83 -DBUILD_MINOR_VERSION=1
cmake --build build --config Debug --target common_lib
```
Expected: `Build succeeded`. Any compile error in the new `common/` `.cpp`
files surfaces here.

If only Linux is available, the configure step (`cmake -S . -B build-cfg…`)
already exercises memory_map.h generation and the test target's
compilation of `common/parse_ini.cpp` and `common/byte_ops.cpp`. The new
`common/*.cpp` files are not part of the test target, so a Linux configure
alone is insufficient — defer this verification to CI on push.

- [ ] **Step 2: Commit nothing (verification step)**

---

## Phase 3 — `custom-ui-host` scaffolding

### Task 3.1: Add `custom-ui-host/CMakeLists.txt` + register subdirectory

**Files:**
- Create: `custom-ui-host/CMakeLists.txt`
- Modify: `CMakeLists.txt` (root)

- [ ] **Step 1: Create `custom-ui-host/CMakeLists.txt`**

```cmake
# custom-ui-host: reusable in-DLL UI framework. Hosts the C ABI, installs
# the three Detours hooks, and owns the window/hotkey/packet registries.
# Compile-gated to GMS v83.1 only for this milestone; non-target builds
# silently return so the CI matrix keeps passing across every region.
if (NOT (BUILD_REGION STREQUAL "GMS"
         AND BUILD_MAJOR_VERSION EQUAL 83
         AND BUILD_MINOR_VERSION EQUAL 1))
    message(STATUS
        "custom-ui-host: not building for ${BUILD_REGION}_v${BUILD_MAJOR_VERSION}_${BUILD_MINOR_VERSION}")
    return()
endif()

add_edit_dll(custom-ui-host SOURCES
    dllmain.cpp
    host_main.cpp
    abi/custom_ui_abi.cpp
    hooks/process_packet_hook.cpp
    hooks/process_key_hook.cpp
    hooks/stage_dtor_hook.cpp
    hooks/s_update_hook.cpp
    registries/window_registry.cpp
    registries/hotkey_registry.cpp
    registries/packet_registry.cpp
    runtime/custom_ui_wnd.cpp
    runtime/vtable_patch.cpp
    runtime/ini_config.cpp
)

# The public ABI header is part of the source tree; downstream edits
# include it directly (no install step yet).
target_include_directories(custom-ui-host PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
```

- [ ] **Step 2: Add subdirectory registration in root `CMakeLists.txt`**

In root `CMakeLists.txt`, after the line `add_subdirectory(window-mode)`,
add:
```cmake
add_subdirectory(custom-ui-host)
```

Also add `custom-ui-host` to `EDIT_DLL_TARGETS` (the packaging list):
```cmake
set(EDIT_DLL_TARGETS
    bypass
    doom-fix
    enable-minimize
    no-patcher
    no-beginner-party-block
    no-enter-mts-map-restriction
    no-ad-balloon
    redirect
    skip-logo
    window-mode
    custom-ui-host
)
```

- [ ] **Step 3: Configure-test all five matrix combos**

```bash
for v in 83 87 95 111; do
  cmake -S . -B "build-cfg-gms-$v" -DBUILD_REGION=GMS -DBUILD_MAJOR_VERSION=$v -DBUILD_MINOR_VERSION=1 -DBUILD_TESTS=OFF 2>&1 | tail -3
done
cmake -S . -B build-cfg-jms-185 -DBUILD_REGION=JMS -DBUILD_MAJOR_VERSION=185 -DBUILD_MINOR_VERSION=1 -DBUILD_TESTS=OFF 2>&1 | tail -3
```
Expected: GMS v83 configures and emits `add_edit_dll(custom-ui-host)`; the
other four print `-- custom-ui-host: not building for …` and skip. None
should fail.

- [ ] **Step 4: Commit (sources don't exist yet — defer until 3.2)**

Skip this commit; sources land in 3.2 and configure must remain valid
across both commits, so we commit them together at the end of 3.2.

### Task 3.2: Add `dllmain.cpp` + `host_main.cpp` skeleton (mutex, ready flag)

**Files:**
- Create: `custom-ui-host/dllmain.cpp`
- Create: `custom-ui-host/host_main.cpp`
- Create: `custom-ui-host/host_globals.h`

- [ ] **Step 1: Create `custom-ui-host/host_globals.h`**

```cpp
#pragma once
#include <atomic>

namespace custom_ui_host {

// Set true once MainProc has finished hook installation. Consumer DLLs
// poll CustomUI_IsReady() (which reads this) before issuing other calls.
extern std::atomic<bool> g_ready;

// Set true when the host detects it was loaded twice (mutex
// ERROR_ALREADY_EXISTS). The second instance does not install hooks
// and the ABI shims return error sentinels.
extern std::atomic<bool> g_double_load;

} // namespace custom_ui_host
```

- [ ] **Step 2: Create `custom-ui-host/dllmain.cpp`**

```cpp
#include "pch.h"

DWORD WINAPI MainProc(LPVOID lpParam);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);
            CreateThread(nullptr, 0, &MainProc, nullptr, 0, nullptr);
            break;
        default:
            break;
    }
    return TRUE;
}
```

- [ ] **Step 3: Create `custom-ui-host/host_main.cpp` skeleton**

```cpp
#include "pch.h"

#include "host_globals.h"
#include "logger.h"

namespace custom_ui_host {
std::atomic<bool> g_ready{false};
std::atomic<bool> g_double_load{false};
} // namespace custom_ui_host

namespace {

bool AcquireSingletonMutex() {
    HANDLE h = CreateMutexW(nullptr, FALSE, L"Local\\custom-ui-host-singleton");
    if (!h) {
        Log("custom-ui-host: CreateMutexW failed err=%lu", GetLastError());
        return false;  // treat as double-load to be safe
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        Log("custom-ui-host: another host instance is already running -- "
            "this instance will be inert");
        return false;
    }
    return true;
}

} // namespace

DWORD WINAPI MainProc(LPVOID /*lpParam*/) {
    Log("custom-ui-host: MainProc start");

    if (!AcquireSingletonMutex()) {
        custom_ui_host::g_double_load.store(true);
        return 0;
    }

    // Hook installation lands in Phase 7. INI parsing lands in Task 3.3.
    // Vtable cloning lands in Phase 5. For now, just signal ready so the
    // demo's IsReady-poll smoke test passes once Phase 3 lands.
    custom_ui_host::g_ready.store(true);
    Log("custom-ui-host: ready");
    return 0;
}
```

- [ ] **Step 4: Create empty stub `.cpp`s referenced by CMakeLists (so the link succeeds)**

These are intentionally empty (one-line `#include "pch.h"`) and get filled
in by later tasks. Create each with this body:

```cpp
#include "pch.h"
```

Files:
- `custom-ui-host/abi/custom_ui_abi.cpp`
- `custom-ui-host/hooks/process_packet_hook.cpp`
- `custom-ui-host/hooks/process_key_hook.cpp`
- `custom-ui-host/hooks/stage_dtor_hook.cpp`
- `custom-ui-host/hooks/s_update_hook.cpp`
- `custom-ui-host/registries/window_registry.cpp`
- `custom-ui-host/registries/hotkey_registry.cpp`
- `custom-ui-host/registries/packet_registry.cpp`
- `custom-ui-host/runtime/custom_ui_wnd.cpp`
- `custom-ui-host/runtime/vtable_patch.cpp`
- `custom-ui-host/runtime/ini_config.cpp`

- [ ] **Step 5: Configure all five matrix combos**

(Same command set as Task 3.1 Step 3.) Expected: GMS v83 configures
successfully; others skip with the not-building message.

- [ ] **Step 6: Commit Phase 3.1+3.2 together**

```bash
git add custom-ui-host/ CMakeLists.txt
git commit -m "feat(custom-ui-host): scaffold edit DLL with singleton mutex + ready flag"
```

### Task 3.3: Add `host_config.h` + INI parsing

**Files:**
- Create: `custom-ui-host/runtime/host_config.h`
- Modify: `custom-ui-host/runtime/ini_config.cpp`
- Modify: `custom-ui-host/host_main.cpp`

- [ ] **Step 1: Create `runtime/host_config.h`**

```cpp
#pragma once

namespace custom_ui_host {

struct HostConfig {
    bool verbose = false;
    unsigned short inbound_op_min = 0x2000;
    unsigned short inbound_op_max = 0x20FF;
    unsigned short outbound_op_min = 0x0F00;
    unsigned short outbound_op_max = 0x0FFF;
};

extern HostConfig g_config;

// Loads `custom-ui-host.ini` if present next to the host DLL. On parse
// failure or missing file, leaves g_config at defaults and logs.
void LoadHostConfig();

} // namespace custom_ui_host
```

- [ ] **Step 2: Implement `runtime/ini_config.cpp`**

Replace the placeholder body with:
```cpp
#include "pch.h"

#include "host_config.h"

#include "logger.h"
#include "parse_ini.h"

#include <cstdlib>
#include <string>

namespace custom_ui_host {

HostConfig g_config;

namespace {

unsigned short ParseHex16(const std::string &s, unsigned short fallback) {
    if (s.empty()) return fallback;
    try {
        auto v = std::stoul(s, nullptr, 0);
        if (v > 0xFFFF) return fallback;
        return static_cast<unsigned short>(v);
    } catch (...) {
        return fallback;
    }
}

bool ParseBool(const std::string &s, bool fallback) {
    if (s == "true" || s == "True" || s == "1") return true;
    if (s == "false" || s == "False" || s == "0") return false;
    return fallback;
}

const std::string &First(const ms::ini::Parsed &p, const char *key,
                         const std::string &fallback) {
    auto it = p.entries.find(key);
    if (it == p.entries.end() || it->second.empty()) return fallback;
    return it->second.front();
}

} // namespace

void LoadHostConfig() {
    // The host DLL is loaded from the edits/ directory; the INI sits
    // next to it. GetModuleHandle(NULL) is the host process, not the
    // host DLL — we need GetModuleFileNameW on the host's own HMODULE,
    // recoverable via a sentinel symbol address.
    HMODULE self = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                       | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       reinterpret_cast<LPCWSTR>(&LoadHostConfig), &self);

    wchar_t path[MAX_PATH] = {};
    if (!self || !GetModuleFileNameW(self, path, MAX_PATH)) {
        Log("custom-ui-host: cannot resolve own module path -- using defaults");
        return;
    }
    // Replace filename with custom-ui-host.ini.
    std::wstring wpath = path;
    auto slash = wpath.find_last_of(L"\\/");
    if (slash == std::wstring::npos) {
        Log("custom-ui-host: module path has no directory separator");
        return;
    }
    wpath.erase(slash + 1);
    wpath += L"custom-ui-host.ini";

    // Convert wpath -> std::string (ASCII path expected on local builds).
    std::string apath(wpath.begin(), wpath.end());

    ms::ini::Parsed p;
    if (!ms::ini::Parse(apath, p, [](const char *msg) {
            Log("custom-ui-host: ini warning: %s", msg);
        })) {
        Log("custom-ui-host: ini missing at %s -- using defaults", apath.c_str());
        return;
    }

    const std::string empty;
    g_config.verbose = ParseBool(First(p, "host.verbose", empty), false);
    g_config.inbound_op_min =
        ParseHex16(First(p, "host.inbound_opcode_min", empty), 0x2000);
    g_config.inbound_op_max =
        ParseHex16(First(p, "host.inbound_opcode_max", empty), 0x20FF);
    g_config.outbound_op_min =
        ParseHex16(First(p, "host.outbound_opcode_min", empty), 0x0F00);
    g_config.outbound_op_max =
        ParseHex16(First(p, "host.outbound_opcode_max", empty), 0x0FFF);

    Log("custom-ui-host: config inbound=[0x%04X..0x%04X] outbound=[0x%04X..0x%04X] verbose=%d",
        g_config.inbound_op_min, g_config.inbound_op_max,
        g_config.outbound_op_min, g_config.outbound_op_max,
        (int)g_config.verbose);
}

} // namespace custom_ui_host
```

- [ ] **Step 3: Call `LoadHostConfig()` from MainProc**

In `host_main.cpp`, add `#include "runtime/host_config.h"` and insert
between the mutex check and the ready signal:
```cpp
custom_ui_host::LoadHostConfig();
```

Full updated MainProc:
```cpp
DWORD WINAPI MainProc(LPVOID /*lpParam*/) {
    Log("custom-ui-host: MainProc start");

    if (!AcquireSingletonMutex()) {
        custom_ui_host::g_double_load.store(true);
        return 0;
    }

    custom_ui_host::LoadHostConfig();

    // Hook installation lands in Phase 7. Vtable cloning lands in Phase 5.
    custom_ui_host::g_ready.store(true);
    Log("custom-ui-host: ready");
    return 0;
}
```

- [ ] **Step 4: Configure-test**

- [ ] **Step 5: Commit**

```bash
git add custom-ui-host/runtime/host_config.h \
        custom-ui-host/runtime/ini_config.cpp \
        custom-ui-host/host_main.cpp
git commit -m "feat(custom-ui-host): parse custom-ui-host.ini with opcode-range overrides"
```

---

---

## Phase 4 — Registries (TDD on host-buildable C++)

The registries are pure-C++ data structures with no game-side dependency.
They are unit-testable via the existing GoogleTest harness in `tests/`.
This phase follows strict TDD: write the test, see it fail, implement,
see it pass, commit.

### Task 4.1: HotkeyRegistry — types + denylist

**Files:**
- Create: `custom-ui-host/registries/hotkey_registry.h`
- Modify: `custom-ui-host/registries/hotkey_registry.cpp`
- Create: `tests/test_hotkey_registry.cpp`
- Modify: `tests/CMakeLists.txt`

- [ ] **Step 1: Write failing test for denylist rejection**

Create `tests/test_hotkey_registry.cpp`:
```cpp
#include "hotkey_registry.h"

#include <gtest/gtest.h>

using custom_ui_host::HotkeyRegistry;

TEST(HotkeyRegistry, DenylistedKeysReject) {
    HotkeyRegistry reg;
    // VK_ESCAPE=0x1B, VK_RETURN=0x0D, VK_TAB=0x09, arrows 0x25..0x28,
    // mouse 0x01..0x04.
    EXPECT_EQ(reg.Bind(0x1B, 0, /*target=*/42), 0u);  // VK_ESCAPE
    EXPECT_EQ(reg.Bind(0x0D, 0, 42), 0u);             // VK_RETURN
    EXPECT_EQ(reg.Bind(0x09, 0, 42), 0u);             // VK_TAB
    EXPECT_EQ(reg.Bind(0x25, 0, 42), 0u);             // VK_LEFT
    EXPECT_EQ(reg.Bind(0x26, 0, 42), 0u);             // VK_UP
    EXPECT_EQ(reg.Bind(0x27, 0, 42), 0u);             // VK_RIGHT
    EXPECT_EQ(reg.Bind(0x28, 0, 42), 0u);             // VK_DOWN
    EXPECT_EQ(reg.Bind(0x01, 0, 42), 0u);             // VK_LBUTTON
    EXPECT_EQ(reg.Bind(0x02, 0, 42), 0u);             // VK_RBUTTON
    EXPECT_EQ(reg.Bind(0x04, 0, 42), 0u);             // VK_MBUTTON
}
```

- [ ] **Step 2: Wire test into `tests/CMakeLists.txt`**

Append:
```cmake
add_executable(hotkey_registry_tests
    test_hotkey_registry.cpp
    ${CMAKE_SOURCE_DIR}/custom-ui-host/registries/hotkey_registry.cpp)
target_include_directories(hotkey_registry_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/custom-ui-host/registries
    ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(hotkey_registry_tests PRIVATE gtest_main)
gtest_discover_tests(hotkey_registry_tests)
```

- [ ] **Step 3: Run test — confirm it FAILS at compile (header missing)**

```bash
cmake -S . -B build-tests -DBUILD_REGION=GMS -DBUILD_MAJOR_VERSION=83 -DBUILD_MINOR_VERSION=1 -DBUILD_TESTS=ON 2>&1 | tail -5
cmake --build build-tests --target hotkey_registry_tests 2>&1 | tail -10
```
Expected: compile error — `hotkey_registry.h: No such file or directory`.

- [ ] **Step 4: Implement minimal `hotkey_registry.h`**

```cpp
#pragma once
#include <cstdint>
#include <mutex>
#include <vector>

namespace custom_ui_host {

using HotkeyId = std::uint32_t;
using WindowHandle = std::uint32_t;

struct HotkeyBinding {
    HotkeyId id;
    unsigned int vk;
    unsigned int mods;       // CUSTOM_UI_MOD_* bitmask
    WindowHandle target;
};

class HotkeyRegistry {
public:
    // Returns 0 on rejection (denylist hit, vk already mapped on game,
    // already-bound conflict). Returns non-zero opaque id on success.
    HotkeyId Bind(unsigned int vk, unsigned int mods, WindowHandle target);
    bool Unbind(HotkeyId id);
    const HotkeyBinding *Lookup(unsigned int vk, unsigned int mods) const;
    void Clear();
    std::size_t Size() const;

private:
    static bool IsDenylisted(unsigned int vk);

    mutable std::mutex mu_;
    std::vector<HotkeyBinding> bindings_;
    HotkeyId next_id_ = 1;
};

} // namespace custom_ui_host
```

- [ ] **Step 5: Implement minimal `hotkey_registry.cpp` (denylist only)**

Replace the placeholder:
```cpp
#include "hotkey_registry.h"

namespace custom_ui_host {

bool HotkeyRegistry::IsDenylisted(unsigned int vk) {
    switch (vk) {
        case 0x01:  // VK_LBUTTON
        case 0x02:  // VK_RBUTTON
        case 0x04:  // VK_MBUTTON
        case 0x09:  // VK_TAB
        case 0x0D:  // VK_RETURN
        case 0x1B:  // VK_ESCAPE
        case 0x25:  // VK_LEFT
        case 0x26:  // VK_UP
        case 0x27:  // VK_RIGHT
        case 0x28:  // VK_DOWN
            return true;
        default:
            return false;
    }
}

HotkeyId HotkeyRegistry::Bind(unsigned int vk, unsigned int /*mods*/,
                              WindowHandle /*target*/) {
    if (IsDenylisted(vk)) return 0;
    return 0;  // not implemented yet — other failure paths in next task
}

bool HotkeyRegistry::Unbind(HotkeyId /*id*/) { return false; }
const HotkeyBinding *HotkeyRegistry::Lookup(unsigned int /*vk*/,
                                            unsigned int /*mods*/) const {
    return nullptr;
}
void HotkeyRegistry::Clear() {
    std::lock_guard<std::mutex> g(mu_);
    bindings_.clear();
    next_id_ = 1;
}
std::size_t HotkeyRegistry::Size() const {
    std::lock_guard<std::mutex> g(mu_);
    return bindings_.size();
}

} // namespace custom_ui_host
```

- [ ] **Step 6: Run test — confirm PASS**

```bash
cmake --build build-tests --target hotkey_registry_tests
ctest --test-dir build-tests -R HotkeyRegistry --output-on-failure
```
Expected: `1 test passed`.

- [ ] **Step 7: Commit**

```bash
git add custom-ui-host/registries/hotkey_registry.h \
        custom-ui-host/registries/hotkey_registry.cpp \
        tests/test_hotkey_registry.cpp tests/CMakeLists.txt
git commit -m "feat(custom-ui-host): HotkeyRegistry denylist + scaffold"
```

### Task 4.2: HotkeyRegistry — Bind / Lookup / Unbind happy path + double-bind

**Files:**
- Modify: `custom-ui-host/registries/hotkey_registry.cpp`
- Modify: `tests/test_hotkey_registry.cpp`

- [ ] **Step 1: Add failing tests for bind / lookup / unbind / double-bind**

Append to `tests/test_hotkey_registry.cpp`:
```cpp
TEST(HotkeyRegistry, BindAndLookup) {
    HotkeyRegistry reg;
    auto id = reg.Bind(/*vk=*/0x77, /*mods=*/0, /*target=*/123);  // VK_F8
    ASSERT_NE(id, 0u);
    auto *b = reg.Lookup(0x77, 0);
    ASSERT_NE(b, nullptr);
    EXPECT_EQ(b->target, 123u);
    EXPECT_EQ(b->vk, 0x77u);
}

TEST(HotkeyRegistry, BindRejectsConflict) {
    HotkeyRegistry reg;
    auto id1 = reg.Bind(0x77, 0, 1);
    ASSERT_NE(id1, 0u);
    auto id2 = reg.Bind(0x77, 0, 2);     // same vk+mods, different target
    EXPECT_EQ(id2, 0u);                  // second is rejected
    auto *b = reg.Lookup(0x77, 0);
    ASSERT_NE(b, nullptr);
    EXPECT_EQ(b->target, 1u);            // first still wins
}

TEST(HotkeyRegistry, SameVkDifferentModsCoexist) {
    HotkeyRegistry reg;
    auto id1 = reg.Bind(0x77, 0, 1);
    auto id2 = reg.Bind(0x77, /*Ctrl=*/0x02, 2);
    EXPECT_NE(id1, 0u);
    EXPECT_NE(id2, 0u);
    EXPECT_EQ(reg.Lookup(0x77, 0)->target, 1u);
    EXPECT_EQ(reg.Lookup(0x77, 0x02)->target, 2u);
}

TEST(HotkeyRegistry, UnbindRemoves) {
    HotkeyRegistry reg;
    auto id = reg.Bind(0x77, 0, 1);
    ASSERT_NE(id, 0u);
    EXPECT_TRUE(reg.Unbind(id));
    EXPECT_EQ(reg.Lookup(0x77, 0), nullptr);
    EXPECT_FALSE(reg.Unbind(id));        // double unbind is idempotent failure
}

TEST(HotkeyRegistry, ClearWipesAll) {
    HotkeyRegistry reg;
    reg.Bind(0x77, 0, 1);
    reg.Bind(0x78, 0, 2);
    EXPECT_EQ(reg.Size(), 2u);
    reg.Clear();
    EXPECT_EQ(reg.Size(), 0u);
}
```

- [ ] **Step 2: Run tests — confirm new ones FAIL**

```bash
cmake --build build-tests --target hotkey_registry_tests
ctest --test-dir build-tests -R HotkeyRegistry --output-on-failure
```
Expected: denylist test passes; the four new tests fail.

- [ ] **Step 3: Implement Bind/Lookup/Unbind**

Replace the stubs in `hotkey_registry.cpp` with:
```cpp
HotkeyId HotkeyRegistry::Bind(unsigned int vk, unsigned int mods,
                              WindowHandle target) {
    if (IsDenylisted(vk)) return 0;
    std::lock_guard<std::mutex> g(mu_);
    for (const auto &b : bindings_) {
        if (b.vk == vk && b.mods == mods) return 0;
    }
    HotkeyBinding nb{next_id_++, vk, mods, target};
    bindings_.push_back(nb);
    return nb.id;
}

bool HotkeyRegistry::Unbind(HotkeyId id) {
    std::lock_guard<std::mutex> g(mu_);
    for (auto it = bindings_.begin(); it != bindings_.end(); ++it) {
        if (it->id == id) {
            bindings_.erase(it);
            return true;
        }
    }
    return false;
}

const HotkeyBinding *HotkeyRegistry::Lookup(unsigned int vk,
                                            unsigned int mods) const {
    std::lock_guard<std::mutex> g(mu_);
    for (const auto &b : bindings_) {
        if (b.vk == vk && b.mods == mods) return &b;
    }
    return nullptr;
}
```

- [ ] **Step 4: Run tests — confirm all PASS**

- [ ] **Step 5: Commit**

```bash
git add custom-ui-host/registries/hotkey_registry.cpp \
        tests/test_hotkey_registry.cpp
git commit -m "feat(custom-ui-host): HotkeyRegistry bind/lookup/unbind"
```

### Task 4.3: PacketRegistry — TDD

**Files:**
- Create: `custom-ui-host/registries/packet_registry.h`
- Modify: `custom-ui-host/registries/packet_registry.cpp`
- Create: `tests/test_packet_registry.cpp`
- Modify: `tests/CMakeLists.txt`

- [ ] **Step 1: Write failing tests**

`tests/test_packet_registry.cpp`:
```cpp
#include "packet_registry.h"

#include <cstring>
#include <gtest/gtest.h>

using custom_ui_host::PacketRegistry;

namespace {
struct CallSpy {
    int count = 0;
    unsigned short last_op = 0;
    unsigned int last_len = 0;
    std::vector<unsigned char> last_payload;
};

void __cdecl Handler(unsigned short op, const unsigned char *p,
                     unsigned int n, void *user) {
    auto *s = static_cast<CallSpy *>(user);
    s->count++;
    s->last_op = op;
    s->last_len = n;
    s->last_payload.assign(p, p + n);
}
} // namespace

TEST(PacketRegistry, RegisterAndDispatch) {
    PacketRegistry reg(/*min=*/0x2000, /*max=*/0x20FF);
    CallSpy spy;
    auto id = reg.Register(0x2000, &Handler, &spy);
    ASSERT_NE(id, 0u);
    unsigned char payload[] = {0xAA, 0xBB};
    reg.Dispatch(0x2000, payload, sizeof(payload));
    EXPECT_EQ(spy.count, 1);
    EXPECT_EQ(spy.last_op, 0x2000);
    EXPECT_EQ(spy.last_len, 2u);
    EXPECT_EQ(spy.last_payload[0], 0xAA);
}

TEST(PacketRegistry, OutOfRangeRegisterRejected) {
    PacketRegistry reg(0x2000, 0x20FF);
    CallSpy spy;
    EXPECT_EQ(reg.Register(0x1FFF, &Handler, &spy), 0u);
    EXPECT_EQ(reg.Register(0x2100, &Handler, &spy), 0u);
}

TEST(PacketRegistry, DoubleRegisterRejected) {
    PacketRegistry reg(0x2000, 0x20FF);
    CallSpy spy;
    auto id1 = reg.Register(0x2010, &Handler, &spy);
    auto id2 = reg.Register(0x2010, &Handler, &spy);
    EXPECT_NE(id1, 0u);
    EXPECT_EQ(id2, 0u);
}

TEST(PacketRegistry, DispatchOutsideRangeNoop) {
    PacketRegistry reg(0x2000, 0x20FF);
    CallSpy spy;
    reg.Register(0x2000, &Handler, &spy);
    reg.Dispatch(0x1FFF, nullptr, 0);  // outside range
    reg.Dispatch(0x2001, nullptr, 0);  // in range, no handler
    EXPECT_EQ(spy.count, 0);
}

TEST(PacketRegistry, UnregisterStopsDispatch) {
    PacketRegistry reg(0x2000, 0x20FF);
    CallSpy spy;
    auto id = reg.Register(0x2000, &Handler, &spy);
    EXPECT_TRUE(reg.Unregister(id));
    reg.Dispatch(0x2000, nullptr, 0);
    EXPECT_EQ(spy.count, 0);
    EXPECT_FALSE(reg.Unregister(id));  // idempotent failure
}
```

- [ ] **Step 2: Wire into `tests/CMakeLists.txt`**

Append:
```cmake
add_executable(packet_registry_tests
    test_packet_registry.cpp
    ${CMAKE_SOURCE_DIR}/custom-ui-host/registries/packet_registry.cpp)
target_include_directories(packet_registry_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/custom-ui-host/registries
    ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(packet_registry_tests PRIVATE gtest_main)
gtest_discover_tests(packet_registry_tests)
```

- [ ] **Step 3: Run — confirm tests FAIL at compile**

- [ ] **Step 4: Implement `packet_registry.h`**

```cpp
#pragma once
#include <cstdint>
#include <mutex>
#include <unordered_map>

namespace custom_ui_host {

using HandlerId = std::uint32_t;

using PacketHandlerFn = void(__cdecl *)(unsigned short opcode,
                                        const unsigned char *payload,
                                        unsigned int payloadLen,
                                        void *user);

class PacketRegistry {
public:
    PacketRegistry(unsigned short range_min, unsigned short range_max);

    // Returns 0 if opcode is out of range or already registered.
    HandlerId Register(unsigned short opcode, PacketHandlerFn fn, void *user);
    bool Unregister(HandlerId id);

    // Out-of-range opcodes and absent handlers are silent no-ops.
    void Dispatch(unsigned short opcode, const unsigned char *payload,
                  unsigned int payloadLen);

    std::size_t Size() const;

private:
    struct Entry {
        HandlerId id;
        unsigned short opcode;
        PacketHandlerFn fn;
        void *user;
    };

    mutable std::mutex mu_;
    std::unordered_map<unsigned short, Entry> by_opcode_;
    HandlerId next_id_ = 1;
    unsigned short range_min_;
    unsigned short range_max_;
};

} // namespace custom_ui_host
```

- [ ] **Step 5: Implement `packet_registry.cpp`**

Replace placeholder:
```cpp
#include "packet_registry.h"

namespace custom_ui_host {

PacketRegistry::PacketRegistry(unsigned short range_min,
                               unsigned short range_max)
    : range_min_(range_min), range_max_(range_max) {}

HandlerId PacketRegistry::Register(unsigned short opcode, PacketHandlerFn fn,
                                   void *user) {
    if (opcode < range_min_ || opcode > range_max_) return 0;
    std::lock_guard<std::mutex> g(mu_);
    if (by_opcode_.count(opcode) != 0) return 0;
    Entry e{next_id_++, opcode, fn, user};
    by_opcode_.emplace(opcode, e);
    return e.id;
}

bool PacketRegistry::Unregister(HandlerId id) {
    std::lock_guard<std::mutex> g(mu_);
    for (auto it = by_opcode_.begin(); it != by_opcode_.end(); ++it) {
        if (it->second.id == id) {
            by_opcode_.erase(it);
            return true;
        }
    }
    return false;
}

void PacketRegistry::Dispatch(unsigned short opcode,
                              const unsigned char *payload,
                              unsigned int payloadLen) {
    if (opcode < range_min_ || opcode > range_max_) return;
    PacketHandlerFn fn = nullptr;
    void *user = nullptr;
    {
        std::lock_guard<std::mutex> g(mu_);
        auto it = by_opcode_.find(opcode);
        if (it == by_opcode_.end()) return;
        fn = it->second.fn;
        user = it->second.user;
    }
    // Dispatch outside the lock so a misbehaving handler can't deadlock
    // re-entering Register/Unregister.
    fn(opcode, payload, payloadLen, user);
}

std::size_t PacketRegistry::Size() const {
    std::lock_guard<std::mutex> g(mu_);
    return by_opcode_.size();
}

} // namespace custom_ui_host
```

- [ ] **Step 6: Run tests — confirm all PASS**

```bash
cmake --build build-tests --target packet_registry_tests
ctest --test-dir build-tests -R PacketRegistry --output-on-failure
```

- [ ] **Step 7: Commit**

```bash
git add custom-ui-host/registries/packet_registry.h \
        custom-ui-host/registries/packet_registry.cpp \
        tests/test_packet_registry.cpp tests/CMakeLists.txt
git commit -m "feat(custom-ui-host): PacketRegistry with opcode-range gating"
```

### Task 4.4: WindowRegistry — TDD (handle lifecycle only; CustomUIWnd integration in Phase 5)

**Files:**
- Create: `custom-ui-host/registries/window_registry.h`
- Modify: `custom-ui-host/registries/window_registry.cpp`
- Create: `tests/test_window_registry.cpp`
- Modify: `tests/CMakeLists.txt`

The registry stores `CustomUIWnd*` pointers behind opaque handles. For
this task it's a pure handle-table — `CustomUIWnd` doesn't exist yet and
the test treats it as an opaque type via a forward declaration.

- [ ] **Step 1: Write failing tests**

`tests/test_window_registry.cpp`:
```cpp
#include "window_registry.h"

#include <gtest/gtest.h>

namespace custom_ui_host {
struct CustomUIWnd {  // test stub; the real one lands in Phase 5
    int marker;
};
} // namespace custom_ui_host

using custom_ui_host::CustomUIWnd;
using custom_ui_host::WindowRegistry;

TEST(WindowRegistry, RegisterReturnsNonZero) {
    WindowRegistry reg;
    CustomUIWnd w{42};
    auto h = reg.Register(&w);
    EXPECT_NE(h, 0u);
}

TEST(WindowRegistry, LookupReturnsTheWindow) {
    WindowRegistry reg;
    CustomUIWnd w{42};
    auto h = reg.Register(&w);
    EXPECT_EQ(reg.Lookup(h), &w);
    EXPECT_EQ(reg.Lookup(0), nullptr);
    EXPECT_EQ(reg.Lookup(h + 9999), nullptr);
}

TEST(WindowRegistry, UnregisterClearsLookup) {
    WindowRegistry reg;
    CustomUIWnd w{42};
    auto h = reg.Register(&w);
    EXPECT_TRUE(reg.Unregister(h));
    EXPECT_EQ(reg.Lookup(h), nullptr);
    EXPECT_FALSE(reg.Unregister(h));  // double-unreg is idempotent failure
}

TEST(WindowRegistry, HandlesAreUnique) {
    WindowRegistry reg;
    CustomUIWnd a{1}, b{2}, c{3};
    auto ha = reg.Register(&a);
    auto hb = reg.Register(&b);
    auto hc = reg.Register(&c);
    EXPECT_NE(ha, hb);
    EXPECT_NE(hb, hc);
    EXPECT_NE(ha, hc);
}

TEST(WindowRegistry, ForEachIteratesAll) {
    WindowRegistry reg;
    CustomUIWnd a{1}, b{2};
    reg.Register(&a);
    reg.Register(&b);
    int sum = 0;
    reg.ForEach([&](CustomUIWnd *w) { sum += w->marker; });
    EXPECT_EQ(sum, 3);
}
```

- [ ] **Step 2: Wire into `tests/CMakeLists.txt`**

```cmake
add_executable(window_registry_tests
    test_window_registry.cpp
    ${CMAKE_SOURCE_DIR}/custom-ui-host/registries/window_registry.cpp)
target_include_directories(window_registry_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/custom-ui-host/registries
    ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(window_registry_tests PRIVATE gtest_main)
gtest_discover_tests(window_registry_tests)
```

- [ ] **Step 3: Run — confirm FAIL at compile**

- [ ] **Step 4: Implement `window_registry.h`**

```cpp
#pragma once
#include <cstdint>
#include <functional>
#include <mutex>
#include <unordered_map>

namespace custom_ui_host {

// Forward-declared; defined in runtime/custom_ui_wnd.h (Phase 5). Tests
// supply their own stub definition via the same forward-declared type.
struct CustomUIWnd;

using WindowHandle = std::uint32_t;

class WindowRegistry {
public:
    WindowHandle Register(CustomUIWnd *w);
    bool Unregister(WindowHandle h);
    CustomUIWnd *Lookup(WindowHandle h) const;
    void ForEach(const std::function<void(CustomUIWnd *)> &fn) const;
    std::size_t Size() const;

private:
    mutable std::mutex mu_;
    std::unordered_map<WindowHandle, CustomUIWnd *> by_handle_;
    WindowHandle next_handle_ = 1;
};

} // namespace custom_ui_host
```

- [ ] **Step 5: Implement `window_registry.cpp`**

```cpp
#include "window_registry.h"

namespace custom_ui_host {

WindowHandle WindowRegistry::Register(CustomUIWnd *w) {
    std::lock_guard<std::mutex> g(mu_);
    WindowHandle h = next_handle_++;
    by_handle_.emplace(h, w);
    return h;
}

bool WindowRegistry::Unregister(WindowHandle h) {
    std::lock_guard<std::mutex> g(mu_);
    return by_handle_.erase(h) != 0;
}

CustomUIWnd *WindowRegistry::Lookup(WindowHandle h) const {
    std::lock_guard<std::mutex> g(mu_);
    auto it = by_handle_.find(h);
    return it == by_handle_.end() ? nullptr : it->second;
}

void WindowRegistry::ForEach(
    const std::function<void(CustomUIWnd *)> &fn) const {
    std::lock_guard<std::mutex> g(mu_);
    for (const auto &kv : by_handle_) fn(kv.second);
}

std::size_t WindowRegistry::Size() const {
    std::lock_guard<std::mutex> g(mu_);
    return by_handle_.size();
}

} // namespace custom_ui_host
```

- [ ] **Step 6: Run tests — confirm PASS**

- [ ] **Step 7: Commit**

```bash
git add custom-ui-host/registries/window_registry.h \
        custom-ui-host/registries/window_registry.cpp \
        tests/test_window_registry.cpp tests/CMakeLists.txt
git commit -m "feat(custom-ui-host): WindowRegistry handle table"
```

---

---

## Phase 5 — Runtime: vtable patch + CustomUIWnd + SEH dispatcher

This phase introduces the game-side runtime: placement-new wrappers around
`CUIWnd` and `CCtrlButton`, the shared cloned `CUIWnd` vtable, and the
SEH wrapper used to isolate consumer-callback crashes. These pieces can
only be exercised in a Windows build (they call game addresses) — Linux
configure-test is sufficient for plan validation.

### Task 5.1: SEH dispatch header

**Files:**
- Create: `custom-ui-host/runtime/seh_dispatch.h`

- [ ] **Step 1: Create the header**

```cpp
#pragma once
#include <Windows.h>

#include "logger.h"

namespace custom_ui_host {

// Wraps a callable in a __try/__except filtering on EXCEPTION_ACCESS_VIOLATION.
// Other exceptions propagate normally. The callable must not construct or
// destruct C++ objects with non-trivial dtors at the SEH boundary — that
// restriction is enforced by C++ language rules, not by code here.
//
// Usage:
//   SafeDispatch("CustomUI button click", [&]() {
//       consumer_fn(arg);
//   });
template <class F>
inline void SafeDispatch(const char *siteName, F &&fn) noexcept {
    __try {
        fn();
    } __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
                    ? EXCEPTION_EXECUTE_HANDLER
                    : EXCEPTION_CONTINUE_SEARCH) {
        Log("custom-ui-host: AV in consumer callback at site=[%s]", siteName);
    }
}

} // namespace custom_ui_host
```

- [ ] **Step 2: Configure-test (no compilation of host on Linux; just confirm parse)**

The header is included by Phase 5+ TUs; standalone validation isn't
possible until a TU pulls it in. Skip explicit verification here.

- [ ] **Step 3: Commit**

```bash
git add custom-ui-host/runtime/seh_dispatch.h
git commit -m "feat(custom-ui-host): add SafeDispatch SEH wrapper"
```

### Task 5.2: `CustomUIWnd` byte-buffer wrapper + `FrameworkExtras`

**Files:**
- Create: `custom-ui-host/runtime/custom_ui_wnd.h`
- Modify: `custom-ui-host/runtime/custom_ui_wnd.cpp`

- [ ] **Step 1: Create `runtime/custom_ui_wnd.h`**

```cpp
#pragma once

#include "registries/window_registry.h"

#include <cstdint>
#include <vector>

namespace custom_ui_host {

// Distinguishes control kinds for downstream button-click dispatch.
enum class ControlKind {
    Label,
    Button,
    Edit,
};

using CtrlId = std::uint32_t;

struct ControlEntry {
    CtrlId id;
    ControlKind kind;
    void *raw;  // pointer into the per-control byte buffer (CCtrlButton*, etc.)
    // Per-button vtable clone slot (D3 per-instance approach). Unused for
    // Label/Edit.
    void *btn_vtable_clone = nullptr;
    void(__cdecl *on_click)(WindowHandle, CtrlId, void *) = nullptr;
    void *user = nullptr;
};

struct FrameworkExtras {
    WindowHandle handle = 0;
    void *user = nullptr;
    std::vector<ControlEntry> controls;
    bool was_visible = false;
    bool is_visible = false;
};

// CustomUIWnd is *not* a normal C++ class — it's a placement-new'd CUIWnd
// followed by a FrameworkExtras block, with a patched vptr pointing at
// the cloned vtable in runtime/vtable_patch.cpp. Constructor returns
// nullptr on allocation failure or sizeof-mismatch.
struct CustomUIWnd {
    static CustomUIWnd *Create(int x, int y, int w, int h, const char *titleUtf8,
                               void *user);
    static void Destroy(CustomUIWnd *self);

    // Visibility — delegates to game-side CWndMan::RegisterUIWindow /
    // UnregisterUIWindow. Idempotent.
    void Show();
    void Hide();
    bool IsVisible() const;

    // Recover the FrameworkExtras for a given `this` pointer of a placed
    // CUIWnd. Used inside cloned-vtable trampolines.
    static FrameworkExtras *ExtrasOf(void *cuiwnd_self);

    FrameworkExtras &Extras();
    void *GameWnd();  // returns pointer to placed CUIWnd
};

} // namespace custom_ui_host
```

- [ ] **Step 2: Implement `runtime/custom_ui_wnd.cpp` (skeleton — vtable clone wired in Task 5.3)**

```cpp
#include "pch.h"

#include "runtime/custom_ui_wnd.h"

#include "logger.h"
#include "memory_map.h"

#include <new>
#include <string>

namespace custom_ui_host {

namespace {

constexpr std::size_t kCUIWndSize = SIZEOF_C_UI_WND_V83_1;
constexpr std::size_t kBufSize = kCUIWndSize + sizeof(FrameworkExtras);

// UTF-8 -> UCS-2 conversion (game uses unsigned short* strings). Returns
// empty wstring on conversion failure.
std::wstring Utf8ToUcs2(const char *s) {
    if (!s || !*s) return {};
    int needed = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
    if (needed <= 0) return {};
    std::wstring out(needed - 1, L'\0');  // exclude null terminator
    MultiByteToWideChar(CP_UTF8, 0, s, -1, out.data(), needed);
    return out;
}

} // namespace

FrameworkExtras *CustomUIWnd::ExtrasOf(void *cuiwnd_self) {
    auto *p = static_cast<unsigned char *>(cuiwnd_self);
    return reinterpret_cast<FrameworkExtras *>(p + kCUIWndSize);
}

CustomUIWnd *CustomUIWnd::Create(int x, int y, int w, int h,
                                 const char *titleUtf8, void *user) {
    auto *buf = static_cast<unsigned char *>(::operator new(kBufSize));
    if (!buf) return nullptr;
    std::memset(buf, 0, kBufSize);

    // Construct the game-side CUIWnd in place. The thunk in
    // common/CUIWnd.cpp writes the game's vftable pointer at buf[0..3].
    auto title = Utf8ToUcs2(titleUtf8);
    new (buf) ::CUIWnd(x, y, w, h,
                       title.empty() ? nullptr
                                     : reinterpret_cast<const unsigned short *>(
                                           title.c_str()));

    // Patch the vptr to our cloned vtable (vtable_patch.cpp owns the
    // table; wired in Task 5.3).
    extern void *g_cloned_cuiwnd_vtable;
    if (g_cloned_cuiwnd_vtable) {
        *reinterpret_cast<void **>(buf) = g_cloned_cuiwnd_vtable;
    } else {
        Log("custom-ui-host: cloned CUIWnd vtable not initialised");
        ::operator delete(buf);
        return nullptr;
    }

    // Construct FrameworkExtras after the CUIWnd block.
    new (buf + kCUIWndSize) FrameworkExtras();
    auto *fe = reinterpret_cast<FrameworkExtras *>(buf + kCUIWndSize);
    fe->user = user;

    return reinterpret_cast<CustomUIWnd *>(buf);
}

void CustomUIWnd::Destroy(CustomUIWnd *self) {
    if (!self) return;
    auto *buf = reinterpret_cast<unsigned char *>(self);
    auto *fe = reinterpret_cast<FrameworkExtras *>(buf + kCUIWndSize);

    if (fe->is_visible) self->Hide();

    // Destroy per-button vtable clones (their backing storage is in the
    // ControlEntry heap; see Task 6.3 button add).
    for (auto &ctrl : fe->controls) {
        if (ctrl.btn_vtable_clone) {
            ::operator delete(ctrl.btn_vtable_clone);
            ctrl.btn_vtable_clone = nullptr;
        }
        if (ctrl.raw) {
            ::operator delete(ctrl.raw);
            ctrl.raw = nullptr;
        }
    }

    // Run the game-side CUIWnd dtor via the v83.1 dtor address. We can't
    // rely on the virtual destructor through the cloned vtable here,
    // because Destroy is the explicit Free path — we want the original.
    reinterpret_cast<void(__fastcall *)(void *, void *)>(C_UI_WND_DTOR)(buf,
                                                                       nullptr);

    fe->~FrameworkExtras();
    ::operator delete(buf);
}

FrameworkExtras &CustomUIWnd::Extras() {
    auto *buf = reinterpret_cast<unsigned char *>(this);
    return *reinterpret_cast<FrameworkExtras *>(buf + kCUIWndSize);
}

void *CustomUIWnd::GameWnd() {
    return reinterpret_cast<unsigned char *>(this);
}

void CustomUIWnd::Show() {
    auto &fe = Extras();
    if (fe.is_visible) return;
    auto *wnd_man = /* game-side singleton */ nullptr;  // resolved in Task 5.4
    (void)wnd_man;
    // CWndMan singleton accessor wiring lands in Task 5.4. For now this
    // path is unreachable — Phase 6 ABI shims gate Show() on g_ready.
    fe.is_visible = true;
}

void CustomUIWnd::Hide() {
    auto &fe = Extras();
    if (!fe.is_visible) return;
    fe.is_visible = false;
    // Same — CWndMan call in Task 5.4.
}

bool CustomUIWnd::IsVisible() const {
    auto *self = const_cast<CustomUIWnd *>(this);
    return self->Extras().is_visible;
}

} // namespace custom_ui_host
```

- [ ] **Step 3: Configure-test v83.1**

- [ ] **Step 4: Commit**

```bash
git add custom-ui-host/runtime/custom_ui_wnd.h \
        custom-ui-host/runtime/custom_ui_wnd.cpp
git commit -m "feat(custom-ui-host): CustomUIWnd placement-new scaffold"
```

### Task 5.3: Cloned `CUIWnd` vtable in `vtable_patch.cpp`

**Files:**
- Create: `custom-ui-host/runtime/vtable_patch.h`
- Modify: `custom-ui-host/runtime/vtable_patch.cpp`
- Modify: `custom-ui-host/host_main.cpp`

- [ ] **Step 1: Create `runtime/vtable_patch.h`**

```cpp
#pragma once

namespace custom_ui_host {

// Pointer to the shared cloned CUIWnd vtable. Initialized by
// InitCustomUIWndVtable() at host init. Null until ready.
extern void *g_cloned_cuiwnd_vtable;

// Called once from MainProc after the host mutex is acquired and before
// any window is constructed. Returns false on failure (vtable count
// mismatch or allocation failure); host then refuses to install hooks.
bool InitCustomUIWndVtable();

} // namespace custom_ui_host
```

- [ ] **Step 2: Implement `runtime/vtable_patch.cpp`**

Replace placeholder:
```cpp
#include "pch.h"

#include "runtime/vtable_patch.h"

#include "logger.h"
#include "memory_map.h"

#include <cstring>

namespace custom_ui_host {

void *g_cloned_cuiwnd_vtable = nullptr;

namespace {

// Slot replacements. Each one looks up the per-instance FrameworkExtras
// from the `this` pointer (see CustomUIWnd::ExtrasOf) and either invokes
// the consumer's callback under SEH or just chains to the original game
// slot. For the milestone we override only the destructor slot so the
// game's redraw + focus machinery continues to do everything else.
//
// Phase 5 lands the dtor override; OnDraw/OnSetFocus overrides are
// deferred unless integration testing demonstrates a need.

void __fastcall DtorOverride(void *self, void * /*edx*/, char flag) {
    // Chain to the original game dtor. The cloned vtable is what brought
    // us here, so the game's vftable slot is still the right address.
    using DtorT = void(__fastcall *)(void *, void *, char);
    reinterpret_cast<DtorT>(C_UI_WND_DTOR)(self, nullptr, flag);
}

} // namespace

bool InitCustomUIWndVtable() {
    constexpr std::size_t kSlots = C_UI_WND_VTABLE_SLOT_COUNT;
    if (kSlots == 0) {
        Log("custom-ui-host: C_UI_WND_VTABLE_SLOT_COUNT is 0 -- aborting");
        return false;
    }

    auto *src = reinterpret_cast<void **>(C_UI_WND_VFTABLE);
    auto *clone = static_cast<void **>(::operator new(sizeof(void *) * kSlots));
    if (!clone) return false;
    std::memcpy(clone, src, sizeof(void *) * kSlots);

    // Slot 0 on MSVC layouts is the scalar deleting destructor for classes
    // with a virtual dtor. Override it so consumer-Destroy doesn't take
    // the (potentially incompatible) game-side deleting path. The actual
    // dtor body is invoked via C_UI_WND_DTOR explicitly in
    // CustomUIWnd::Destroy.
    clone[0] = reinterpret_cast<void *>(&DtorOverride);

    g_cloned_cuiwnd_vtable = clone;
    Log("custom-ui-host: cloned CUIWnd vtable (%zu slots) at %p", kSlots,
        clone);
    return true;
}

} // namespace custom_ui_host
```

- [ ] **Step 3: Wire `InitCustomUIWndVtable` into MainProc**

In `host_main.cpp`, add `#include "runtime/vtable_patch.h"` near the
existing includes, and insert in `MainProc` after `LoadHostConfig()`:
```cpp
if (!custom_ui_host::InitCustomUIWndVtable()) {
    Log("custom-ui-host: vtable init failed -- staying inert");
    return 0;
}
```

- [ ] **Step 4: Configure-test v83.1**

- [ ] **Step 5: Commit**

```bash
git add custom-ui-host/runtime/vtable_patch.h \
        custom-ui-host/runtime/vtable_patch.cpp \
        custom-ui-host/host_main.cpp
git commit -m "feat(custom-ui-host): clone CUIWnd vtable with dtor override"
```

### Task 5.4: Show/Hide via `CWndMan` singleton

**Files:**
- Modify: `custom-ui-host/runtime/custom_ui_wnd.cpp`
- Modify: `common/CWvsApp.h` (potentially, if singleton accessor missing)

`CWndMan` is owned by `CWvsApp` — look at `common/CWvsApp.h` to find the
appropriate accessor. If `CWvsApp` exposes `GetWndMan()` (it should — check
during execution) use it directly. If not, the manager's instance lives at
a fixed address (typical pattern for `s_pInstance`-style singletons) — add
that address to the memory map as a follow-on micro-task.

- [ ] **Step 1: Inspect `common/CWvsApp.h` for the right accessor**

Run:
```bash
grep -n "CWndMan\|GetWndMan\|m_pWndMan" common/CWvsApp.h
```
Record the result in the implementation. If `m_pWndMan` is a field of the
singleton (likely), the path is
`CWvsApp::GetInstance()->m_pWndMan->RegisterUIWindow(...)`.

- [ ] **Step 2: Implement Show/Hide**

Replace the placeholder bodies in `custom_ui_wnd.cpp`:
```cpp
void CustomUIWnd::Show() {
    auto &fe = Extras();
    if (fe.is_visible) return;
    auto *app = CWvsApp::GetInstance();
    if (!app || !app->m_pWndMan) {
        Log("custom-ui-host: Show without CWvsApp/CWndMan singleton");
        return;
    }
    app->m_pWndMan->RegisterUIWindow(reinterpret_cast<CUIWnd *>(GameWnd()));
    fe.is_visible = true;
}

void CustomUIWnd::Hide() {
    auto &fe = Extras();
    if (!fe.is_visible) return;
    auto *app = CWvsApp::GetInstance();
    if (!app || !app->m_pWndMan) {
        Log("custom-ui-host: Hide without CWvsApp/CWndMan singleton");
        return;
    }
    app->m_pWndMan->UnregisterUIWindow(reinterpret_cast<CUIWnd *>(GameWnd()));
    fe.is_visible = false;
}
```

If `CWvsApp::m_pWndMan` doesn't exist, instead use a per-version
singleton address from the memory map. Concrete fallback: add
`set(C_WND_MAN_INSTANCE_ADDR 0xTBD)` to v83_1.cmake (resolve via IDA),
then `auto *mgr = *reinterpret_cast<CWndMan **>(C_WND_MAN_INSTANCE_ADDR);`.

- [ ] **Step 3: Configure-test**

- [ ] **Step 4: Commit**

```bash
git add custom-ui-host/runtime/custom_ui_wnd.cpp
git commit -m "feat(custom-ui-host): Show/Hide via CWndMan::Register/UnregisterUIWindow"
```

---

## Phase 6 — Controls + ABI

### Task 6.1: Public ABI header `custom_ui_abi.h`

**Files:**
- Create: `custom-ui-host/abi/custom_ui_abi.h`

- [ ] **Step 1: Create the header**

```cpp
// custom_ui_abi.h — public C ABI for the custom-ui-host framework.
// Consumer DLLs include this header AND link nothing; the ABI is resolved
// at runtime via LoadLibrary("custom-ui-host.dll") + GetProcAddress.
//
// ABI version: 1.0.0 (encoded as 0x00010000 by CustomUI_GetAbiVersion).
// All exports are __cdecl extern "C" __declspec(dllexport). Resolve by
// name, never by ordinal. Strings are UTF-8 at the boundary.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef int CustomUI_WindowHandle;   // 0 = invalid
typedef int CustomUI_CtrlId;
typedef int CustomUI_HotkeyId;
typedef int CustomUI_HandlerId;

typedef void(__cdecl *CustomUI_OnClickFn)(CustomUI_WindowHandle w,
                                          CustomUI_CtrlId c, void *user);

typedef void(__cdecl *CustomUI_PacketHandlerFn)(unsigned short opcode,
                                                const unsigned char *payload,
                                                unsigned int payloadLen,
                                                void *user);

#define CUSTOM_UI_MOD_SHIFT 0x01
#define CUSTOM_UI_MOD_CTRL  0x02
#define CUSTOM_UI_MOD_ALT   0x04

/* Lifecycle */
__declspec(dllexport) unsigned int __cdecl CustomUI_GetAbiVersion(void);
__declspec(dllexport) int          __cdecl CustomUI_IsReady(void);

/* Windows */
__declspec(dllexport) CustomUI_WindowHandle __cdecl
    CustomUI_CreateWindow(const char *title, int x, int y, int w, int h,
                          void *user);
__declspec(dllexport) int __cdecl CustomUI_ShowWindow(CustomUI_WindowHandle h);
__declspec(dllexport) int __cdecl CustomUI_HideWindow(CustomUI_WindowHandle h);
__declspec(dllexport) int __cdecl CustomUI_DestroyWindow(CustomUI_WindowHandle h);

/* Controls */
__declspec(dllexport) CustomUI_CtrlId __cdecl
    CustomUI_AddLabel(CustomUI_WindowHandle h, int x, int y, const char *text);
__declspec(dllexport) CustomUI_CtrlId __cdecl
    CustomUI_AddButton(CustomUI_WindowHandle h, int x, int y, int w, int h_,
                       const char *text, CustomUI_OnClickFn onClick);
__declspec(dllexport) CustomUI_CtrlId __cdecl
    CustomUI_AddEdit(CustomUI_WindowHandle h, int x, int y, int w, int h_,
                     const char *initialText);
__declspec(dllexport) int __cdecl
    CustomUI_SetLabelText(CustomUI_WindowHandle h, CustomUI_CtrlId c,
                          const char *text);
__declspec(dllexport) int __cdecl
    CustomUI_GetEditText(CustomUI_WindowHandle h, CustomUI_CtrlId c, char *buf,
                         int bufLen);

/* Hotkeys */
__declspec(dllexport) CustomUI_HotkeyId __cdecl
    CustomUI_BindHotkey(unsigned int vk, unsigned int modifiers,
                        CustomUI_WindowHandle target);
__declspec(dllexport) int __cdecl
    CustomUI_UnbindHotkey(CustomUI_HotkeyId id);

/* Packets */
__declspec(dllexport) int __cdecl
    CustomUI_SendPacket(unsigned short opcode, const void *payload,
                        unsigned int len);
__declspec(dllexport) CustomUI_HandlerId __cdecl
    CustomUI_RegisterPacketHandler(unsigned short opcode,
                                   CustomUI_PacketHandlerFn fn, void *user);
__declspec(dllexport) int __cdecl
    CustomUI_UnregisterPacketHandler(CustomUI_HandlerId id);

/* Debug */
__declspec(dllexport) void __cdecl CustomUI_DumpRegistries(void);

#ifdef __cplusplus
}
#endif
```

- [ ] **Step 2: Commit**

```bash
git add custom-ui-host/abi/custom_ui_abi.h
git commit -m "feat(custom-ui-host): publish C ABI header"
```

### Task 6.2: ABI shim layer — lifecycle + windows

**Files:**
- Create: `custom-ui-host/abi/abi_globals.h` (registry singletons)
- Modify: `custom-ui-host/abi/custom_ui_abi.cpp`
- Modify: `custom-ui-host/host_main.cpp` (construct registries)

- [ ] **Step 1: Create `abi/abi_globals.h`**

```cpp
#pragma once

#include "registries/hotkey_registry.h"
#include "registries/packet_registry.h"
#include "registries/window_registry.h"

#include "runtime/custom_ui_wnd.h"

#include <memory>

namespace custom_ui_host {

// Host-wide registry singletons. Constructed by InitAbiGlobals() from
// MainProc (after LoadHostConfig() so packet range comes from INI).
extern std::unique_ptr<WindowRegistry> g_windows;
extern std::unique_ptr<HotkeyRegistry> g_hotkeys;
extern std::unique_ptr<PacketRegistry> g_packets;

void InitAbiGlobals();

} // namespace custom_ui_host
```

- [ ] **Step 2: Implement registry construction in `runtime/ini_config.cpp`**

Actually — to keep concerns separate, put `InitAbiGlobals` in
`abi/custom_ui_abi.cpp` (it's part of the ABI layer's lifecycle). The
definition lives there; `host_main.cpp` just calls it.

- [ ] **Step 3: Replace `abi/custom_ui_abi.cpp` placeholder with the lifecycle + window exports**

```cpp
#include "pch.h"

#include "abi/custom_ui_abi.h"
#include "abi/abi_globals.h"

#include "host_globals.h"
#include "runtime/host_config.h"
#include "runtime/custom_ui_wnd.h"

#include "logger.h"

#include <string>

namespace custom_ui_host {

std::unique_ptr<WindowRegistry> g_windows;
std::unique_ptr<HotkeyRegistry> g_hotkeys;
std::unique_ptr<PacketRegistry> g_packets;

void InitAbiGlobals() {
    g_windows = std::make_unique<WindowRegistry>();
    g_hotkeys = std::make_unique<HotkeyRegistry>();
    g_packets = std::make_unique<PacketRegistry>(g_config.inbound_op_min,
                                                  g_config.inbound_op_max);
}

namespace {

bool ReadyOrLog(const char *site) {
    if (custom_ui_host::g_double_load.load()) {
        Log("custom-ui-host: %s called on double-load instance -- ignoring", site);
        return false;
    }
    if (!custom_ui_host::g_ready.load()) {
        Log("custom-ui-host: %s called before ready -- ignoring", site);
        return false;
    }
    return true;
}

} // namespace
} // namespace custom_ui_host

extern "C" {

__declspec(dllexport) unsigned int __cdecl CustomUI_GetAbiVersion(void) {
    return 0x00010000u;
}

__declspec(dllexport) int __cdecl CustomUI_IsReady(void) {
    return custom_ui_host::g_ready.load() ? 1 : 0;
}

__declspec(dllexport) CustomUI_WindowHandle __cdecl
CustomUI_CreateWindow(const char *title, int x, int y, int w, int h,
                      void *user) {
    if (!custom_ui_host::ReadyOrLog("CreateWindow")) return 0;
    auto *wnd = custom_ui_host::CustomUIWnd::Create(x, y, w, h, title, user);
    if (!wnd) return 0;
    auto handle = custom_ui_host::g_windows->Register(wnd);
    wnd->Extras().handle = handle;
    return static_cast<CustomUI_WindowHandle>(handle);
}

__declspec(dllexport) int __cdecl
CustomUI_ShowWindow(CustomUI_WindowHandle h) {
    if (!custom_ui_host::ReadyOrLog("ShowWindow")) return 0;
    auto *wnd = custom_ui_host::g_windows->Lookup(static_cast<unsigned int>(h));
    if (!wnd) return 0;
    wnd->Show();
    return 1;
}

__declspec(dllexport) int __cdecl
CustomUI_HideWindow(CustomUI_WindowHandle h) {
    if (!custom_ui_host::ReadyOrLog("HideWindow")) return 0;
    auto *wnd = custom_ui_host::g_windows->Lookup(static_cast<unsigned int>(h));
    if (!wnd) return 0;
    wnd->Hide();
    return 1;
}

__declspec(dllexport) int __cdecl
CustomUI_DestroyWindow(CustomUI_WindowHandle h) {
    if (!custom_ui_host::ReadyOrLog("DestroyWindow")) return 0;
    auto *wnd = custom_ui_host::g_windows->Lookup(static_cast<unsigned int>(h));
    if (!wnd) return 0;
    custom_ui_host::g_windows->Unregister(static_cast<unsigned int>(h));
    custom_ui_host::CustomUIWnd::Destroy(wnd);
    return 1;
}

/* Stubs for controls/hotkeys/packets — implemented in 6.3+. */
__declspec(dllexport) CustomUI_CtrlId __cdecl
CustomUI_AddLabel(CustomUI_WindowHandle, int, int, const char *) { return 0; }
__declspec(dllexport) CustomUI_CtrlId __cdecl
CustomUI_AddButton(CustomUI_WindowHandle, int, int, int, int, const char *,
                   CustomUI_OnClickFn) { return 0; }
__declspec(dllexport) CustomUI_CtrlId __cdecl
CustomUI_AddEdit(CustomUI_WindowHandle, int, int, int, int, const char *) {
    return 0;
}
__declspec(dllexport) int __cdecl
CustomUI_SetLabelText(CustomUI_WindowHandle, CustomUI_CtrlId, const char *) {
    return 0;
}
__declspec(dllexport) int __cdecl
CustomUI_GetEditText(CustomUI_WindowHandle, CustomUI_CtrlId, char *, int) {
    return 0;
}
__declspec(dllexport) CustomUI_HotkeyId __cdecl
CustomUI_BindHotkey(unsigned int, unsigned int, CustomUI_WindowHandle) {
    return 0;
}
__declspec(dllexport) int __cdecl CustomUI_UnbindHotkey(CustomUI_HotkeyId) {
    return 0;
}
__declspec(dllexport) int __cdecl
CustomUI_SendPacket(unsigned short, const void *, unsigned int) { return 0; }
__declspec(dllexport) CustomUI_HandlerId __cdecl
CustomUI_RegisterPacketHandler(unsigned short, CustomUI_PacketHandlerFn,
                               void *) { return 0; }
__declspec(dllexport) int __cdecl
CustomUI_UnregisterPacketHandler(CustomUI_HandlerId) { return 0; }
__declspec(dllexport) void __cdecl CustomUI_DumpRegistries(void) {}

} // extern "C"
```

- [ ] **Step 4: Call `InitAbiGlobals` from MainProc**

In `host_main.cpp`, add `#include "abi/abi_globals.h"` and insert in
`MainProc` after the vtable init:
```cpp
custom_ui_host::InitAbiGlobals();
```

- [ ] **Step 5: Configure-test**

- [ ] **Step 6: Commit**

```bash
git add custom-ui-host/abi/abi_globals.h \
        custom-ui-host/abi/custom_ui_abi.cpp \
        custom-ui-host/host_main.cpp
git commit -m "feat(custom-ui-host): ABI lifecycle + window create/show/hide/destroy"
```

### Task 6.3: ABI — Hotkey binding + game-side conflict probe

**Files:**
- Modify: `custom-ui-host/abi/custom_ui_abi.cpp`

The bind path needs to call `CFuncKeyMappedMan::FuncKeyMapped(vk)` and
reject if `nType != 0`. The struct's first byte is `nType` (see
`common/FunckeyMapped.h`).

- [ ] **Step 1: Replace the `CustomUI_BindHotkey` stub**

```cpp
__declspec(dllexport) CustomUI_HotkeyId __cdecl
CustomUI_BindHotkey(unsigned int vk, unsigned int modifiers,
                    CustomUI_WindowHandle target) {
    if (!custom_ui_host::ReadyOrLog("BindHotkey")) return 0;

    // Reject if the vanilla key map has this vk mapped.
    auto *mgr = CFuncKeyMappedMan::GetInstance();
    if (mgr) {
        constexpr int kMax = 89;
        if (vk < kMax) {
            FUNCKEY_MAPPED fkm = mgr->FuncKeyMapped(static_cast<int>(vk));
            // FUNCKEY_MAPPED is opaque from C++ here (private fields). Read
            // its first byte (nType) by aliasing — layout is #pragma packed
            // so byte 0 is nType.
            unsigned char nType = *reinterpret_cast<unsigned char *>(&fkm);
            if (nType != 0) {
                Log("custom-ui-host: BindHotkey rejected -- vk=0x%02X already mapped (nType=%u)",
                    vk, nType);
                return 0;
            }
        }
    }

    auto *target_wnd = custom_ui_host::g_windows->Lookup(
        static_cast<unsigned int>(target));
    if (!target_wnd) {
        Log("custom-ui-host: BindHotkey rejected -- unknown target window %d", target);
        return 0;
    }

    auto id = custom_ui_host::g_hotkeys->Bind(vk, modifiers,
                                              static_cast<unsigned int>(target));
    return static_cast<CustomUI_HotkeyId>(id);
}

__declspec(dllexport) int __cdecl CustomUI_UnbindHotkey(CustomUI_HotkeyId id) {
    if (!custom_ui_host::ReadyOrLog("UnbindHotkey")) return 0;
    return custom_ui_host::g_hotkeys->Unbind(static_cast<unsigned int>(id))
               ? 1
               : 0;
}
```

Note: this reads the first byte of `FUNCKEY_MAPPED` directly because the
struct's fields are private. If the production v83.1 disasm shows the
returned struct is actually returned in EDX:EAX (small POD on x86), the
first byte of the local `fkm` aliases EAX low — that's `nType`. The cast
is sound for the packed layout in `common/FunckeyMapped.h`.

- [ ] **Step 2: Configure-test**

- [ ] **Step 3: Commit**

```bash
git add custom-ui-host/abi/custom_ui_abi.cpp
git commit -m "feat(custom-ui-host): hotkey bind with FuncKeyMapped conflict probe"
```

### Task 6.4: ABI — Packet send/register/unregister

**Files:**
- Modify: `custom-ui-host/abi/custom_ui_abi.cpp`

- [ ] **Step 1: Replace the packet stubs**

```cpp
__declspec(dllexport) int __cdecl
CustomUI_SendPacket(unsigned short opcode, const void *payload,
                    unsigned int len) {
    if (!custom_ui_host::ReadyOrLog("SendPacket")) return 0;
    if (opcode < custom_ui_host::g_config.outbound_op_min ||
        opcode > custom_ui_host::g_config.outbound_op_max) {
        Log("custom-ui-host: SendPacket rejected -- opcode 0x%04X outside outbound range",
            opcode);
        return 0;
    }
    COutPacket op(static_cast<INT>(opcode));
    if (payload && len) op.EncodeBuffer(payload, len);
    auto *sock = CClientSocket::GetInstance();
    if (!sock) {
        Log("custom-ui-host: SendPacket -- CClientSocket singleton null");
        return 0;
    }
    sock->SendPacket(&op);
    return 1;
}

__declspec(dllexport) CustomUI_HandlerId __cdecl
CustomUI_RegisterPacketHandler(unsigned short opcode,
                               CustomUI_PacketHandlerFn fn, void *user) {
    if (!custom_ui_host::ReadyOrLog("RegisterPacketHandler")) return 0;
    if (!fn) return 0;
    return static_cast<CustomUI_HandlerId>(
        custom_ui_host::g_packets->Register(opcode, fn, user));
}

__declspec(dllexport) int __cdecl
CustomUI_UnregisterPacketHandler(CustomUI_HandlerId id) {
    if (!custom_ui_host::ReadyOrLog("UnregisterPacketHandler")) return 0;
    return custom_ui_host::g_packets->Unregister(static_cast<unsigned int>(id))
               ? 1
               : 0;
}
```

- [ ] **Step 2: Configure-test**

- [ ] **Step 3: Commit**

```bash
git add custom-ui-host/abi/custom_ui_abi.cpp
git commit -m "feat(custom-ui-host): packet send/register/unregister exports"
```

### Task 6.5: ABI — Label / Button / Edit controls

**Files:**
- Modify: `custom-ui-host/abi/custom_ui_abi.cpp`
- Modify: `custom-ui-host/runtime/custom_ui_wnd.cpp` (helper for control insertion)
- Modify: `custom-ui-host/runtime/custom_ui_wnd.h`

Because of the per-button vtable clone requirement (design §8.3), the
button path is more involved than label/edit. We'll:
1. Allocate a per-control byte buffer sized to `sizeof(CCtrlButton)` or
   `sizeof(CCtrlEdit)` (from the memory-map sizeof constants).
2. Placement-new the game control into that buffer.
3. For buttons, clone the vftable, replace the click slot with our shim,
   patch the per-instance vptr.
4. Add a `ControlEntry` to the window's `FrameworkExtras`.

`Label` is implemented as a non-control: we hold the text in
`FrameworkExtras::controls[i]` and rely on a custom OnDraw later (out of
milestone scope per design §4.3 — the milestone uses `CCtrlButton`-style
label rendering or a static text region). The simplest viable milestone
implementation is to use a button with no callback as a "label" surface
and disable its click hit-test via the button's `m_bSelfDisable` field.
That's what the demo does for the "Server says: ?" label.

- [ ] **Step 1: Add control helpers to `runtime/custom_ui_wnd.h`**

Append (after the `CustomUIWnd` struct):
```cpp
// Returns the new ctrl id (>0) on success, 0 on failure. The window
// retains ownership of the control's backing buffer.
CtrlId AddLabel(CustomUIWnd *wnd, int x, int y, const char *textUtf8);

// onClick may be null (button acts as a static surface).
CtrlId AddButton(CustomUIWnd *wnd, int x, int y, int w, int h,
                 const char *textUtf8,
                 void(__cdecl *onClick)(WindowHandle, CtrlId, void *));

CtrlId AddEdit(CustomUIWnd *wnd, int x, int y, int w, int h,
               const char *initialTextUtf8);

bool SetLabelText(CustomUIWnd *wnd, CtrlId id, const char *textUtf8);
int GetEditText(CustomUIWnd *wnd, CtrlId id, char *buf, int bufLen);
```

- [ ] **Step 2: Implement helpers in `runtime/custom_ui_wnd.cpp`**

Append:
```cpp
namespace {

constexpr std::size_t kCCtrlButtonSize = SIZEOF_C_CTRL_BUTTON_V83_1;
constexpr std::size_t kCCtrlEditSize = SIZEOF_C_CTRL_EDIT_V83_1;

// Per-button click trampoline. The cloned button vtable's "click" slot
// points here. The slot is invoked by the game's CWndMan / CCtrlWnd
// dispatch path when the mouse is released over the button. The
// trampoline recovers the ControlEntry via a per-button "callback map"
// keyed on the CCtrlButton* (the trampoline doesn't know its parent
// window directly).
struct ClickCallback {
    WindowHandle window;
    CtrlId ctrl;
    void(__cdecl *fn)(WindowHandle, CtrlId, void *);
    void *user;
};

// Tiny global map: button pointer -> click info. Acquired under mutex.
// (N is tiny — small N favors map over hash for milestone simplicity.)
std::mutex g_btn_mu;
std::unordered_map<void *, ClickCallback> g_btn_callbacks;

void __fastcall ButtonClickShim(void *btn_self, void * /*edx*/) {
    ClickCallback cb{};
    {
        std::lock_guard<std::mutex> g(g_btn_mu);
        auto it = g_btn_callbacks.find(btn_self);
        if (it == g_btn_callbacks.end()) return;
        cb = it->second;
    }
    if (!cb.fn) return;
    SafeDispatch("button-click", [&]() { cb.fn(cb.window, cb.ctrl, cb.user); });
}

} // namespace

CtrlId AddLabel(CustomUIWnd *wnd, int x, int y, const char *textUtf8) {
    return AddButton(wnd, x, y, /*w=*/120, /*h=*/16, textUtf8, /*onClick=*/nullptr);
}

CtrlId AddButton(CustomUIWnd *wnd, int x, int y, int w, int h,
                 const char *textUtf8,
                 void(__cdecl *onClick)(WindowHandle, CtrlId, void *)) {
    if (!wnd || kCCtrlButtonSize == 0) return 0;
    auto title = Utf8ToUcs2(textUtf8);
    auto *buf = static_cast<unsigned char *>(::operator new(kCCtrlButtonSize));
    if (!buf) return 0;
    std::memset(buf, 0, kCCtrlButtonSize);

    CCtrlButton::CREATEPARAM cp{};  // zero-init; safe defaults
    new (buf) ::CCtrlButton(x, y, w, h,
                            title.empty() ? nullptr
                                          : reinterpret_cast<const unsigned short *>(
                                                title.c_str()),
                            &cp);

    // Per-instance vtable clone for the click slot (design §8.3).
    constexpr std::size_t kBtnSlotBytes =
        sizeof(void *) * /*slots*/ 16;  // conservative upper bound; click slot
                                        // is among the first few -- exact slot
                                        // index validated in Phase 1 Task 1.5
                                        // notes. If actual count is larger,
                                        // increase kBtnSlotBytes.
    auto *vclone = static_cast<void **>(::operator new(kBtnSlotBytes));
    std::memcpy(vclone, reinterpret_cast<void **>(C_CTRL_BUTTON_VFTABLE),
                kBtnSlotBytes);
    // TODO_RUNTIME: replace the click slot index with the actual recovered
    // index from IDA -- 0 is a placeholder. Validate by stepping through
    // the click dispatch in CWndMan and recording the offset.
    vclone[0] = reinterpret_cast<void *>(&ButtonClickShim);
    *reinterpret_cast<void **>(buf) = vclone;

    auto &fe = wnd->Extras();
    CtrlId cid = static_cast<CtrlId>(fe.controls.size() + 1);
    ControlEntry entry{cid, ControlKind::Button, buf, vclone, onClick,
                       /*user=*/fe.user};
    fe.controls.push_back(entry);

    if (onClick) {
        ClickCallback cb{fe.handle, cid, onClick, fe.user};
        std::lock_guard<std::mutex> g(g_btn_mu);
        g_btn_callbacks[buf] = cb;
    }

    return cid;
}

CtrlId AddEdit(CustomUIWnd *wnd, int x, int y, int w, int h,
               const char *initialTextUtf8) {
    if (!wnd || kCCtrlEditSize == 0) return 0;
    auto initial = Utf8ToUcs2(initialTextUtf8);
    auto *buf = static_cast<unsigned char *>(::operator new(kCCtrlEditSize));
    if (!buf) return 0;
    std::memset(buf, 0, kCCtrlEditSize);
    new (buf) ::CCtrlEdit(x, y, w, h,
                          initial.empty() ? nullptr
                                          : reinterpret_cast<const unsigned short *>(
                                                initial.c_str()));
    auto &fe = wnd->Extras();
    CtrlId cid = static_cast<CtrlId>(fe.controls.size() + 1);
    fe.controls.push_back({cid, ControlKind::Edit, buf, nullptr, nullptr, nullptr});
    return cid;
}

bool SetLabelText(CustomUIWnd *wnd, CtrlId id, const char *textUtf8) {
    if (!wnd) return false;
    auto &fe = wnd->Extras();
    for (auto &c : fe.controls) {
        if (c.id == id && c.kind == ControlKind::Button) {
            // CCtrlButton text update path requires writing m_sToolTipTitle
            // or invoking a labeling helper. Milestone shortcut: write the
            // text into the CEditCaret-sibling ZXString field at the
            // recovered offset within CCtrlButton. Documented as a known
            // simplification; the demo's "Server says: …" updates this
            // way.
            //
            // The simplest correct implementation requires the v83
            // CCtrlButton text-mutator method address. If unavailable in
            // the memory map, the milestone fallback is to recreate the
            // button (destroy + add at same coords). Use the recreate
            // fallback if no text-mutator is found during IDA work.
            (void)c;
            (void)textUtf8;
            return false;  // executed at Phase 9 acceptance time
        }
    }
    return false;
}

int GetEditText(CustomUIWnd *wnd, CtrlId id, char *buf, int bufLen) {
    if (!wnd) return 0;
    auto &fe = wnd->Extras();
    for (auto &c : fe.controls) {
        if (c.id == id && c.kind == ControlKind::Edit) {
            auto *edit = static_cast<::CCtrlEdit *>(c.raw);
            // Read m_sText directly (design §8.3 fallback).
            const char *src = edit->m_sText.m_pStr ? edit->m_sText.m_pStr : "";
            int needed = static_cast<int>(std::strlen(src)) + 1;
            if (!buf || bufLen <= 0) return needed;
            int n = std::min(bufLen - 1, needed - 1);
            std::memcpy(buf, src, n);
            buf[n] = '\0';
            return needed;
        }
    }
    return 0;
}
```

- [ ] **Step 3: Replace the control ABI stubs in `abi/custom_ui_abi.cpp`**

Replace the stub bodies with:
```cpp
__declspec(dllexport) CustomUI_CtrlId __cdecl
CustomUI_AddLabel(CustomUI_WindowHandle h, int x, int y, const char *text) {
    if (!custom_ui_host::ReadyOrLog("AddLabel")) return 0;
    auto *wnd = custom_ui_host::g_windows->Lookup(static_cast<unsigned int>(h));
    if (!wnd) return 0;
    return static_cast<CustomUI_CtrlId>(custom_ui_host::AddLabel(wnd, x, y, text));
}

__declspec(dllexport) CustomUI_CtrlId __cdecl
CustomUI_AddButton(CustomUI_WindowHandle h, int x, int y, int w, int h_,
                   const char *text, CustomUI_OnClickFn onClick) {
    if (!custom_ui_host::ReadyOrLog("AddButton")) return 0;
    auto *wnd = custom_ui_host::g_windows->Lookup(static_cast<unsigned int>(h));
    if (!wnd) return 0;
    return static_cast<CustomUI_CtrlId>(
        custom_ui_host::AddButton(wnd, x, y, w, h_, text, onClick));
}

__declspec(dllexport) CustomUI_CtrlId __cdecl
CustomUI_AddEdit(CustomUI_WindowHandle h, int x, int y, int w, int h_,
                 const char *initialText) {
    if (!custom_ui_host::ReadyOrLog("AddEdit")) return 0;
    auto *wnd = custom_ui_host::g_windows->Lookup(static_cast<unsigned int>(h));
    if (!wnd) return 0;
    return static_cast<CustomUI_CtrlId>(
        custom_ui_host::AddEdit(wnd, x, y, w, h_, initialText));
}

__declspec(dllexport) int __cdecl
CustomUI_SetLabelText(CustomUI_WindowHandle h, CustomUI_CtrlId c,
                      const char *text) {
    if (!custom_ui_host::ReadyOrLog("SetLabelText")) return 0;
    auto *wnd = custom_ui_host::g_windows->Lookup(static_cast<unsigned int>(h));
    if (!wnd) return 0;
    return custom_ui_host::SetLabelText(wnd, static_cast<unsigned int>(c),
                                         text) ? 1 : 0;
}

__declspec(dllexport) int __cdecl
CustomUI_GetEditText(CustomUI_WindowHandle h, CustomUI_CtrlId c, char *buf,
                     int bufLen) {
    if (!custom_ui_host::ReadyOrLog("GetEditText")) return 0;
    auto *wnd = custom_ui_host::g_windows->Lookup(static_cast<unsigned int>(h));
    if (!wnd) return 0;
    return custom_ui_host::GetEditText(wnd, static_cast<unsigned int>(c), buf,
                                        bufLen);
}
```

- [ ] **Step 4: Implement `CustomUI_DumpRegistries`**

Replace the empty body:
```cpp
__declspec(dllexport) void __cdecl CustomUI_DumpRegistries(void) {
    if (custom_ui_host::g_windows) {
        Log("custom-ui-host: window registry size=%zu",
            custom_ui_host::g_windows->Size());
    }
    if (custom_ui_host::g_hotkeys) {
        Log("custom-ui-host: hotkey registry size=%zu",
            custom_ui_host::g_hotkeys->Size());
    }
    if (custom_ui_host::g_packets) {
        Log("custom-ui-host: packet registry size=%zu",
            custom_ui_host::g_packets->Size());
    }
}
```

- [ ] **Step 5: Configure-test**

- [ ] **Step 6: Commit**

```bash
git add custom-ui-host/runtime/custom_ui_wnd.h \
        custom-ui-host/runtime/custom_ui_wnd.cpp \
        custom-ui-host/abi/custom_ui_abi.cpp
git commit -m "feat(custom-ui-host): AddLabel/Button/Edit + GetEditText + DumpRegistries"
```

---

---

## Phase 7 — Hooks (H1, H2, H3)

Each hook is in its own TU under `custom-ui-host/hooks/`, with one shared
`Install*` function called from `MainProc`. Hook bodies follow the
`bypass/socket_hooks.cpp` pattern: typedefs, `__fastcall` shim, installer
using `INITMAPLEHOOK_OR_RETURN`. Install in MainProc *after* registries
and vtable clone are ready.

### Task 7.1: H2 — `CWndMan::ProcessKey` (lands first, easiest to validate)

**Files:**
- Modify: `custom-ui-host/hooks/process_key_hook.cpp`
- Create: `custom-ui-host/hooks/process_key_hook.h`

- [ ] **Step 1: Create `process_key_hook.h`**

```cpp
#pragma once
#include <Windows.h>

namespace custom_ui_host {
BOOL InstallProcessKeyHook();
} // namespace custom_ui_host
```

- [ ] **Step 2: Replace `process_key_hook.cpp` placeholder**

```cpp
#include "pch.h"

#include "hooks/process_key_hook.h"

#include "abi/abi_globals.h"
#include "abi/custom_ui_abi.h"
#include "runtime/custom_ui_wnd.h"

#include "hooker.h"
#include "logger.h"
#include "memory_map.h"

namespace custom_ui_host {

namespace {

typedef long(__thiscall *ProcessKey_t)(CWndMan *, unsigned int, unsigned int,
                                       long);
ProcessKey_t _ProcessKey = nullptr;

unsigned int SnapshotModifiers() {
    unsigned int m = 0;
    if (GetKeyState(VK_SHIFT) & 0x8000) m |= CUSTOM_UI_MOD_SHIFT;
    if (GetKeyState(VK_CONTROL) & 0x8000) m |= CUSTOM_UI_MOD_CTRL;
    if (GetKeyState(VK_MENU) & 0x8000) m |= CUSTOM_UI_MOD_ALT;
    return m;
}

long __fastcall ProcessKey_Hook(CWndMan *self, void * /*edx*/,
                                unsigned int msg, unsigned int vk,
                                long lParam) {
    // Only rising-edge WM_KEYDOWN / WM_SYSKEYDOWN. lParam bit 30 == 1
    // means the previous key state was down (auto-repeat).
    if ((msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) &&
        (lParam & (1L << 30)) == 0) {
        unsigned int mods = SnapshotModifiers();
        if (g_hotkeys) {
            const auto *binding = g_hotkeys->Lookup(vk, mods);
            if (binding && g_windows) {
                auto *wnd = g_windows->Lookup(binding->target);
                if (wnd) {
                    if (wnd->IsVisible())
                        wnd->Hide();
                    else
                        wnd->Show();
                }
                return 1L;  // consumed
            }
        }
    }
    return _ProcessKey(self, msg, vk, lParam);
}

} // namespace

BOOL InstallProcessKeyHook() {
    INITMAPLEHOOK_OR_RETURN(_ProcessKey, ProcessKey_t, &ProcessKey_Hook,
                            C_WND_MAN_PROCESS_KEY);
    return TRUE;
}

} // namespace custom_ui_host
```

- [ ] **Step 3: Call from MainProc**

In `host_main.cpp`, add `#include "hooks/process_key_hook.h"` and after
`InitAbiGlobals()`:
```cpp
if (!custom_ui_host::InstallProcessKeyHook()) {
    Log("custom-ui-host: ProcessKey hook install failed -- staying inert");
    return 0;
}
```

- [ ] **Step 4: Configure-test**

- [ ] **Step 5: Commit**

```bash
git add custom-ui-host/hooks/process_key_hook.h \
        custom-ui-host/hooks/process_key_hook.cpp \
        custom-ui-host/host_main.cpp
git commit -m "feat(custom-ui-host): hook CWndMan::ProcessKey for hotkey toggles"
```

### Task 7.2: H1 — `CClientSocket::ProcessPacket`

**Files:**
- Modify: `custom-ui-host/hooks/process_packet_hook.cpp`
- Create: `custom-ui-host/hooks/process_packet_hook.h`

- [ ] **Step 1: Create header**

```cpp
#pragma once
#include <Windows.h>

namespace custom_ui_host {
BOOL InstallProcessPacketHook();
} // namespace custom_ui_host
```

- [ ] **Step 2: Replace `process_packet_hook.cpp` placeholder**

```cpp
#include "pch.h"

#include "hooks/process_packet_hook.h"

#include "abi/abi_globals.h"
#include "runtime/host_config.h"

#include "hooker.h"
#include "logger.h"
#include "memory_map.h"

namespace custom_ui_host {

namespace {

typedef void(__thiscall *ProcessPacket_t)(CClientSocket *, CInPacket *);
ProcessPacket_t _ProcessPacket = nullptr;

void __fastcall ProcessPacket_Hook(CClientSocket *self, void * /*edx*/,
                                   CInPacket *iPacket) {
    if (!iPacket) {
        _ProcessPacket(self, iPacket);
        return;
    }
    const unsigned int saved_offset = iPacket->m_uOffset;
    const unsigned short opcode = iPacket->Decode2();

    if (opcode >= g_config.inbound_op_min && opcode <= g_config.inbound_op_max) {
        // Custom range: dispatch to registered handler with the payload
        // bytes that follow the opcode. Vanilla path NOT invoked.
        const unsigned char *payload =
            reinterpret_cast<const unsigned char *>(
                iPacket->m_aRecvBuff.GetBuffer()) +
            iPacket->m_uOffset;
        const unsigned int payloadLen =
            iPacket->m_uDataLen > iPacket->m_uOffset
                ? iPacket->m_uDataLen - iPacket->m_uOffset
                : 0u;
        if (g_packets) g_packets->Dispatch(opcode, payload, payloadLen);
        return;
    }

    // Outside custom range: restore cursor and run vanilla.
    iPacket->m_uOffset = saved_offset;
    _ProcessPacket(self, iPacket);
}

} // namespace

BOOL InstallProcessPacketHook() {
    INITMAPLEHOOK_OR_RETURN(_ProcessPacket, ProcessPacket_t,
                            &ProcessPacket_Hook,
                            C_CLIENT_SOCKET_PROCESS_PACKET);
    return TRUE;
}

} // namespace custom_ui_host
```

Note: `iPacket->m_aRecvBuff.GetBuffer()` returns `void*` (or `char*`) per
`ZArray<unsigned char>`. Verify the accessor's name in `common/ZArray.h`
during execution and adjust if it's `m_aRecvBuff.m_pBuff` or similar.

- [ ] **Step 3: Call from MainProc**

After `InstallProcessKeyHook()`:
```cpp
if (!custom_ui_host::InstallProcessPacketHook()) {
    Log("custom-ui-host: ProcessPacket hook install failed -- staying inert");
    return 0;
}
```

- [ ] **Step 4: Configure-test**

- [ ] **Step 5: Commit**

```bash
git add custom-ui-host/hooks/process_packet_hook.h \
        custom-ui-host/hooks/process_packet_hook.cpp \
        custom-ui-host/host_main.cpp
git commit -m "feat(custom-ui-host): hook ProcessPacket for custom inbound dispatch"
```

### Task 7.3: H3a — `CStage::~CStage` (snapshot on stage end)

**Files:**
- Modify: `custom-ui-host/hooks/stage_dtor_hook.cpp`
- Create: `custom-ui-host/hooks/stage_dtor_hook.h`
- Create: `custom-ui-host/hooks/stage_restore.h` (shared latch flag)
- Modify: `custom-ui-host/runtime/custom_ui_wnd.h` + `.cpp` (snapshot/restore helpers)

- [ ] **Step 1: Add snapshot/restore helpers to `runtime/custom_ui_wnd.h`**

Append after the existing helper declarations:
```cpp
// Walks all registered windows; for each visible one, records the
// visibility in FrameworkExtras::was_visible and calls Hide().
void SnapshotAndSuspendVisibleWindows();

// For each window where was_visible is true, calls Show() and clears the
// flag.
void RestoreSuspendedWindows();
```

- [ ] **Step 2: Implement in `runtime/custom_ui_wnd.cpp`**

Append:
```cpp
void SnapshotAndSuspendVisibleWindows() {
    if (!g_windows) return;
    g_windows->ForEach([](CustomUIWnd *wnd) {
        auto &fe = wnd->Extras();
        fe.was_visible = fe.is_visible;
        if (fe.is_visible) wnd->Hide();
    });
}

void RestoreSuspendedWindows() {
    if (!g_windows) return;
    g_windows->ForEach([](CustomUIWnd *wnd) {
        auto &fe = wnd->Extras();
        if (fe.was_visible) {
            wnd->Show();
            fe.was_visible = false;
        }
    });
}
```

- [ ] **Step 3: Create `hooks/stage_restore.h`**

```cpp
#pragma once
#include <atomic>

namespace custom_ui_host {
extern std::atomic<bool> g_pending_restore;
}
```

- [ ] **Step 4: Define the flag in `hooks/stage_dtor_hook.cpp`**

```cpp
#include "pch.h"

#include "hooks/stage_dtor_hook.h"
#include "hooks/stage_restore.h"

#include "runtime/custom_ui_wnd.h"

#include "hooker.h"
#include "logger.h"
#include "memory_map.h"

namespace custom_ui_host {

std::atomic<bool> g_pending_restore{false};

namespace {

typedef void(__thiscall *StageDtor_t)(CStage *);
StageDtor_t _StageDtor = nullptr;

void __fastcall StageDtor_Hook(CStage *self, void * /*edx*/) {
    SnapshotAndSuspendVisibleWindows();
    g_pending_restore.store(true);
    _StageDtor(self);
}

} // namespace

BOOL InstallStageDtorHook() {
    INITMAPLEHOOK_OR_RETURN(_StageDtor, StageDtor_t, &StageDtor_Hook,
                            C_STAGE_DTOR);
    return TRUE;
}

} // namespace custom_ui_host
```

- [ ] **Step 5: Create `hooks/stage_dtor_hook.h`**

```cpp
#pragma once
#include <Windows.h>

namespace custom_ui_host {
BOOL InstallStageDtorHook();
} // namespace custom_ui_host
```

- [ ] **Step 6: Configure-test**

- [ ] **Step 7: Commit**

```bash
git add custom-ui-host/hooks/stage_dtor_hook.h \
        custom-ui-host/hooks/stage_dtor_hook.cpp \
        custom-ui-host/hooks/stage_restore.h \
        custom-ui-host/runtime/custom_ui_wnd.h \
        custom-ui-host/runtime/custom_ui_wnd.cpp
git commit -m "feat(custom-ui-host): hook CStage dtor + snapshot/restore helpers"
```

### Task 7.4: H3b — `CWndMan::s_Update` restore latch

**Files:**
- Modify: `custom-ui-host/hooks/s_update_hook.cpp`
- Create: `custom-ui-host/hooks/s_update_hook.h`
- Modify: `custom-ui-host/host_main.cpp`

- [ ] **Step 1: Create `s_update_hook.h`**

```cpp
#pragma once
#include <Windows.h>

namespace custom_ui_host {
BOOL InstallSUpdateHook();
} // namespace custom_ui_host
```

- [ ] **Step 2: Replace `s_update_hook.cpp` placeholder**

```cpp
#include "pch.h"

#include "hooks/s_update_hook.h"
#include "hooks/stage_restore.h"

#include "runtime/custom_ui_wnd.h"

#include "hooker.h"
#include "logger.h"
#include "memory_map.h"

namespace custom_ui_host {

namespace {

typedef CWnd **(__cdecl *SUpdate_t)();
SUpdate_t _SUpdate = nullptr;

CWnd **__cdecl SUpdate_Hook() {
    CWnd **rv = _SUpdate();
    if (g_pending_restore.exchange(false)) {
        RestoreSuspendedWindows();
    }
    return rv;
}

} // namespace

BOOL InstallSUpdateHook() {
    INITMAPLEHOOK_OR_RETURN(_SUpdate, SUpdate_t, &SUpdate_Hook,
                            C_WND_MAN_S_UPDATE);
    return TRUE;
}

} // namespace custom_ui_host
```

- [ ] **Step 3: Wire both H3 hooks into MainProc**

In `host_main.cpp`, add the includes and the install calls (after the
ProcessPacket install):
```cpp
#include "hooks/stage_dtor_hook.h"
#include "hooks/s_update_hook.h"
```
and:
```cpp
if (!custom_ui_host::InstallStageDtorHook()) {
    Log("custom-ui-host: StageDtor hook install failed -- staying inert");
    return 0;
}
if (!custom_ui_host::InstallSUpdateHook()) {
    Log("custom-ui-host: s_Update hook install failed -- staying inert");
    return 0;
}
```

- [ ] **Step 4: Configure-test all five matrix combos**

- [ ] **Step 5: Commit**

```bash
git add custom-ui-host/hooks/s_update_hook.h \
        custom-ui-host/hooks/s_update_hook.cpp \
        custom-ui-host/host_main.cpp
git commit -m "feat(custom-ui-host): hook s_Update for stage-restore latch"
```

---

## Phase 8 — Demo edit (`custom-ui-demo`)

### Task 8.1: Demo CMake + scaffold

**Files:**
- Create: `custom-ui-demo/CMakeLists.txt`
- Create: `custom-ui-demo/dllmain.cpp`
- Create: `custom-ui-demo/demo_main.cpp`
- Create: `custom-ui-demo/handlers.cpp`
- Create: `custom-ui-demo/demo_main.h`
- Create: `custom-ui-demo/demo.ini`
- Modify: `CMakeLists.txt` (root, register subdirectory + add to EDIT_DLL_TARGETS)

- [ ] **Step 1: `custom-ui-demo/CMakeLists.txt`**

```cmake
if (NOT (BUILD_REGION STREQUAL "GMS"
         AND BUILD_MAJOR_VERSION EQUAL 83
         AND BUILD_MINOR_VERSION EQUAL 1))
    message(STATUS
        "custom-ui-demo: not building for ${BUILD_REGION}_v${BUILD_MAJOR_VERSION}_${BUILD_MINOR_VERSION}")
    return()
endif()

add_edit_dll(custom-ui-demo SOURCES
    dllmain.cpp
    demo_main.cpp
    handlers.cpp
)

target_include_directories(custom-ui-demo PRIVATE
    ${CMAKE_SOURCE_DIR}/custom-ui-host
)
```

- [ ] **Step 2: `dllmain.cpp`**

```cpp
#include "pch.h"

DWORD WINAPI MainProc(LPVOID lpParam);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID /*reserved*/) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, &MainProc, nullptr, 0, nullptr);
    }
    return TRUE;
}
```

- [ ] **Step 3: `demo_main.h`**

```cpp
#pragma once
#include "abi/custom_ui_abi.h"

namespace custom_ui_demo {

struct ResolvedAbi {
    unsigned int (__cdecl *GetAbiVersion)();
    int (__cdecl *IsReady)();
    CustomUI_WindowHandle (__cdecl *CreateWindow)(const char *, int, int, int,
                                                  int, void *);
    int (__cdecl *ShowWindow)(CustomUI_WindowHandle);
    int (__cdecl *HideWindow)(CustomUI_WindowHandle);
    int (__cdecl *DestroyWindow)(CustomUI_WindowHandle);
    CustomUI_CtrlId (__cdecl *AddLabel)(CustomUI_WindowHandle, int, int,
                                        const char *);
    CustomUI_CtrlId (__cdecl *AddButton)(CustomUI_WindowHandle, int, int, int,
                                         int, const char *,
                                         CustomUI_OnClickFn);
    int (__cdecl *SetLabelText)(CustomUI_WindowHandle, CustomUI_CtrlId,
                                const char *);
    CustomUI_HotkeyId (__cdecl *BindHotkey)(unsigned int, unsigned int,
                                            CustomUI_WindowHandle);
    int (__cdecl *SendPacket)(unsigned short, const void *, unsigned int);
    CustomUI_HandlerId (__cdecl *RegisterPacketHandler)(
        unsigned short, CustomUI_PacketHandlerFn, void *);
};

extern ResolvedAbi g_abi;
extern CustomUI_WindowHandle g_window;
extern CustomUI_CtrlId g_label;
extern int g_ping_count;

void OnPing(CustomUI_WindowHandle, CustomUI_CtrlId, void *);
void OnPong(unsigned short, const unsigned char *, unsigned int, void *);

} // namespace custom_ui_demo
```

- [ ] **Step 4: `demo_main.cpp`**

```cpp
#include "pch.h"

#include "demo_main.h"

#include "logger.h"
#include "parse_ini.h"

#include <string>

namespace custom_ui_demo {

ResolvedAbi g_abi{};
CustomUI_WindowHandle g_window = 0;
CustomUI_CtrlId g_label = 0;
int g_ping_count = 0;

namespace {

struct DemoConfig {
    int vk = 0x77;       // VK_F8
    unsigned int mods = 0;
    int x = 100, y = 100;
};

DemoConfig LoadConfig() {
    DemoConfig c;
    HMODULE self = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                       | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       reinterpret_cast<LPCWSTR>(&LoadConfig), &self);
    wchar_t path[MAX_PATH] = {};
    if (!self || !GetModuleFileNameW(self, path, MAX_PATH)) return c;
    std::wstring wpath = path;
    auto slash = wpath.find_last_of(L"\\/");
    if (slash == std::wstring::npos) return c;
    wpath.erase(slash + 1);
    wpath += L"custom-ui-demo.ini";
    std::string apath(wpath.begin(), wpath.end());
    ms::ini::Parsed p;
    if (!ms::ini::Parse(apath, p)) return c;
    auto find = [&](const char *k, const std::string &def) -> std::string {
        auto it = p.entries.find(k);
        if (it == p.entries.end() || it->second.empty()) return def;
        return it->second.front();
    };
    try { c.vk = std::stoi(find("demo.toggle_hotkey_vk", "119")); } catch (...) {}
    if (find("demo.toggle_hotkey_shift", "false") == "true") c.mods |= CUSTOM_UI_MOD_SHIFT;
    if (find("demo.toggle_hotkey_ctrl",  "false") == "true") c.mods |= CUSTOM_UI_MOD_CTRL;
    if (find("demo.toggle_hotkey_alt",   "false") == "true") c.mods |= CUSTOM_UI_MOD_ALT;
    return c;
}

template <class Fn>
bool Resolve(HMODULE host, const char *name, Fn &out) {
    auto p = GetProcAddress(host, name);
    if (!p) {
        Log("custom-ui-demo: GetProcAddress(%s) failed", name);
        return false;
    }
    out = reinterpret_cast<Fn>(p);
    return true;
}

bool ResolveAbi(HMODULE host) {
    return Resolve(host, "CustomUI_GetAbiVersion", g_abi.GetAbiVersion) &&
           Resolve(host, "CustomUI_IsReady", g_abi.IsReady) &&
           Resolve(host, "CustomUI_CreateWindow", g_abi.CreateWindow) &&
           Resolve(host, "CustomUI_ShowWindow", g_abi.ShowWindow) &&
           Resolve(host, "CustomUI_HideWindow", g_abi.HideWindow) &&
           Resolve(host, "CustomUI_DestroyWindow", g_abi.DestroyWindow) &&
           Resolve(host, "CustomUI_AddLabel", g_abi.AddLabel) &&
           Resolve(host, "CustomUI_AddButton", g_abi.AddButton) &&
           Resolve(host, "CustomUI_SetLabelText", g_abi.SetLabelText) &&
           Resolve(host, "CustomUI_BindHotkey", g_abi.BindHotkey) &&
           Resolve(host, "CustomUI_SendPacket", g_abi.SendPacket) &&
           Resolve(host, "CustomUI_RegisterPacketHandler",
                   g_abi.RegisterPacketHandler);
}

} // namespace
} // namespace custom_ui_demo

DWORD WINAPI MainProc(LPVOID /*lpParam*/) {
    Log("custom-ui-demo: MainProc start");

    HMODULE host = LoadLibraryW(L"custom-ui-host.dll");
    if (!host) {
        Log("custom-ui-demo: host DLL not loaded -- demo inert");
        return 0;
    }
    if (!custom_ui_demo::ResolveAbi(host)) {
        Log("custom-ui-demo: ABI resolve failed -- demo inert");
        return 0;
    }
    unsigned int abi = custom_ui_demo::g_abi.GetAbiVersion();
    if ((abi >> 16) != 1) {
        Log("custom-ui-demo: ABI major mismatch (got 0x%08X) -- demo inert", abi);
        return 0;
    }
    while (!custom_ui_demo::g_abi.IsReady()) Sleep(50);

    auto cfg = custom_ui_demo::LoadConfig();

    custom_ui_demo::g_window = custom_ui_demo::g_abi.CreateWindow(
        "Demo", cfg.x, cfg.y, 240, 80, nullptr);
    if (!custom_ui_demo::g_window) {
        Log("custom-ui-demo: CreateWindow failed");
        return 0;
    }
    custom_ui_demo::g_label = custom_ui_demo::g_abi.AddLabel(
        custom_ui_demo::g_window, 10, 10, "Server says: ?");
    custom_ui_demo::g_abi.AddButton(custom_ui_demo::g_window, 10, 40, 60, 20,
                                    "Ping", &custom_ui_demo::OnPing);
    auto hk = custom_ui_demo::g_abi.BindHotkey(cfg.vk, cfg.mods,
                                               custom_ui_demo::g_window);
    if (!hk) Log("custom-ui-demo: BindHotkey rejected (vk=0x%02X)", cfg.vk);
    custom_ui_demo::g_abi.RegisterPacketHandler(0x2000,
                                                &custom_ui_demo::OnPong,
                                                nullptr);
    Log("custom-ui-demo: window built, ready");
    return 0;
}
```

- [ ] **Step 5: `handlers.cpp`**

```cpp
#include "pch.h"

#include "demo_main.h"

#include "logger.h"

#include <cstdio>

namespace custom_ui_demo {

void OnPing(CustomUI_WindowHandle, CustomUI_CtrlId, void *) {
    Log("custom-ui-demo: OnPing -> SendPacket(0x0F00)");
    g_abi.SendPacket(0x0F00, nullptr, 0);
}

void OnPong(unsigned short opcode, const unsigned char *payload,
            unsigned int len, void *) {
    int seq;
    if (len >= 4) {
        seq = *reinterpret_cast<const int *>(payload);
    } else {
        seq = ++g_ping_count;
    }
    char buf[64];
    std::snprintf(buf, sizeof(buf), "Server says: pong %d", seq);
    Log("custom-ui-demo: OnPong opcode=0x%04X seq=%d", opcode, seq);
    g_abi.SetLabelText(g_window, g_label, buf);
}

} // namespace custom_ui_demo
```

- [ ] **Step 6: `demo.ini`**

```ini
[demo]
toggle_hotkey_vk = 119
toggle_hotkey_shift = false
toggle_hotkey_ctrl = false
toggle_hotkey_alt = false
```

- [ ] **Step 7: Register subdirectory in root `CMakeLists.txt`**

Append after `add_subdirectory(custom-ui-host)`:
```cmake
add_subdirectory(custom-ui-demo)
```
And add to `EDIT_DLL_TARGETS`:
```cmake
    custom-ui-demo
```

- [ ] **Step 8: Configure-test all five matrix combos**

- [ ] **Step 9: Commit**

```bash
git add custom-ui-demo/ CMakeLists.txt
git commit -m "feat(custom-ui-demo): scaffold + Ping/Pong demo against custom-ui-host"
```

---

## Phase 9 — CI matrix + packaging

### Task 9.1: Confirm CI matrix already covers the new edits

**Files:** none (verification).

- [ ] **Step 1: Inspect CI workflow**

```bash
grep -n "matrix" .github/workflows/_build.yml
```
Expected: existing matrix `{GMS 83, 87, 95, 111; JMS 185}` is unchanged.
Because the new edits compile-gate on v83.1 GMS, the existing matrix is
the right coverage with no edit needed — but verify by reading the file.

- [ ] **Step 2: Confirm packaging picks up the new DLLs**

Already added to `EDIT_DLL_TARGETS` in Phase 3 + Phase 8. Verify:
```bash
grep -n "custom-ui-host\|custom-ui-demo" CMakeLists.txt
```
Expected: each name appears under both the `add_subdirectory` block and
the `EDIT_DLL_TARGETS` list.

- [ ] **Step 3: No commit (verification only)**

### Task 9.2: Add `package_dlls` artifact for `custom-ui-host.ini` + `custom-ui-demo.ini`

**Files:**
- Modify: root `CMakeLists.txt` (PACKAGE_COMMANDS block)

- [ ] **Step 1: Add INI files to the package step**

In root `CMakeLists.txt`, find the existing block:
```cmake
list(APPEND PACKAGE_COMMANDS
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:proxy>" "${CMAKE_BINARY_DIR}/artifacts/"
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_BINARY_DIR}/redirect/redirect.ini"
        "${CMAKE_BINARY_DIR}/artifacts/edits/"
)
```
and replace with:
```cmake
list(APPEND PACKAGE_COMMANDS
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:proxy>" "${CMAKE_BINARY_DIR}/artifacts/"
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_BINARY_DIR}/redirect/redirect.ini"
        "${CMAKE_BINARY_DIR}/artifacts/edits/"
)

# Custom UI host / demo INIs (only when those edits actually built).
if (TARGET custom-ui-host)
    list(APPEND PACKAGE_COMMANDS
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/custom-ui-host/custom-ui-host.ini"
            "${CMAKE_BINARY_DIR}/artifacts/edits/"
    )
endif()
if (TARGET custom-ui-demo)
    list(APPEND PACKAGE_COMMANDS
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/custom-ui-demo/demo.ini"
            "${CMAKE_BINARY_DIR}/artifacts/edits/custom-ui-demo.ini"
    )
endif()
```

- [ ] **Step 2: Create default `custom-ui-host/custom-ui-host.ini`**

```ini
[host]
verbose = false

inbound_opcode_min = 0x2000
inbound_opcode_max = 0x20FF
outbound_opcode_min = 0x0F00
outbound_opcode_max = 0x0FFF
```

- [ ] **Step 3: Configure-test v83.1**

- [ ] **Step 4: Commit**

```bash
git add CMakeLists.txt custom-ui-host/custom-ui-host.ini
git commit -m "build(task-005): package custom-ui-host + custom-ui-demo INIs"
```

---

## Phase 10 — Documentation

### Task 10.1: Top-level README `Edits` section

**Files:**
- Modify: `README.md`

- [ ] **Step 1: Add entries**

In `README.md`, in the `## Edits` section, after `### redirect` add:
```markdown
### custom-ui-host
Reusable in-DLL UI framework. Consumer edits load this DLL and call its
C ABI (`CustomUI_*`) to declare hotkey-toggled windows, custom packet
opcodes, and click/handler callbacks. See `custom-ui-host/README.md`.
GMS v83.1 only.

### custom-ui-demo
Reference consumer for `custom-ui-host`. Builds a 240x80 window with a
"Server says: ?" label and a "Ping" button bound to VK_F8 by default;
clicking Ping sends opcode `0x0F00`, the server echoes via `0x2000`,
and the label updates. See `custom-ui-demo/README.md`. GMS v83.1 only.
```

- [ ] **Step 2: Commit**

```bash
git add README.md
git commit -m "docs(task-005): list custom-ui-host + custom-ui-demo in README"
```

### Task 10.2: `custom-ui-host/README.md`

**Files:**
- Create: `custom-ui-host/README.md`

- [ ] **Step 1: Author the host README**

```markdown
# custom-ui-host

Reusable in-DLL UI framework for the MS client edit collection.

## What it does

The host installs three Detours hooks on the client and exposes a stable
C ABI through `GetProcAddress`. Consumer DLLs `LoadLibrary` the host and
call `CustomUI_*` to declare windows, hotkeys, and custom packet
handlers — without installing their own hooks.

## C ABI summary

See `abi/custom_ui_abi.h` for the full surface. Highlights:

| Symbol | Purpose |
|---|---|
| `CustomUI_GetAbiVersion()` | Returns `0x00010000` for the v1.0.0 ABI. Check the major nibble before any other call. |
| `CustomUI_IsReady()` | Polled by consumers from their `MainProc` until non-zero. |
| `CustomUI_CreateWindow / Show / Hide / Destroy` | Window lifecycle. |
| `CustomUI_AddLabel / AddButton / AddEdit` | Add controls to a window. |
| `CustomUI_BindHotkey` | Bind a `VK_*` + modifier mask to toggle a window. Rejected if the vk is in `CFuncKeyMappedMan` or in the framework denylist (ESC, Enter, Tab, arrows, mouse buttons). |
| `CustomUI_SendPacket` | Outbound packet, opcode must be in `0x0F00..0x0FFF`. |
| `CustomUI_RegisterPacketHandler` | Inbound packet, opcode must be in `0x2000..0x20FF`. One handler per opcode; second registration returns 0. |
| `CustomUI_DumpRegistries()` | Debug aid; logs registry sizes. |

All strings at the ABI boundary are UTF-8.

## Hotkey conflict semantics

The host probes `CFuncKeyMappedMan::FuncKeyMapped(vk)` at bind time. If
that returns a mapped function (nType != 0), the bind is rejected. The
host also denies system-level keys (ESC, Enter, Tab, arrows, mouse
buttons) since `CFuncKeyMappedMan` doesn't track them. Consumer DLLs
should avoid those keys regardless.

## Opcode ranges

- Outbound (client → server): `0x0F00..0x0FFF`
- Inbound  (server → client): `0x2000..0x20FF`

Ranges are overridable per-install via `custom-ui-host.ini`.

## Threading

Consumer callbacks (click, packet handler) run on the game UI thread,
the same thread that handles `WM_SOCKET` packet dispatch. Do not block
in callbacks; do not call back into the framework recursively beyond
short, idempotent operations.

## Stability

Callbacks are wrapped in SEH; an access violation inside a consumer
callback is logged and the framework continues. Other exceptions
propagate. Consumer authors should still fix bugs — SEH is a safety
net, not a substitute for correctness.
```

- [ ] **Step 2: Commit**

```bash
git add custom-ui-host/README.md
git commit -m "docs(custom-ui-host): document C ABI, opcode ranges, threading"
```

### Task 10.3: `custom-ui-demo/README.md`

**Files:**
- Create: `custom-ui-demo/README.md`

- [ ] **Step 1: Author the demo README**

```markdown
# custom-ui-demo

Reference consumer for `custom-ui-host`. Demonstrates the minimum work
to ship a window-bearing edit DLL.

## Build a consumer in four steps

1. Add an `add_edit_dll(my-thing SOURCES ...)` entry under your edit
   subdirectory's `CMakeLists.txt`. Include
   `${CMAKE_SOURCE_DIR}/custom-ui-host` so you can include
   `abi/custom_ui_abi.h`.

2. In your `MainProc`, `LoadLibraryW(L"custom-ui-host.dll")` and
   resolve each `CustomUI_*` export via `GetProcAddress` into a
   struct of function pointers (see this demo's `ResolvedAbi`).

3. Spin on `IsReady()` until non-zero, then build your UI:
   ```cpp
   auto w = abi.CreateWindow("Title", x, y, w, h, nullptr);
   abi.AddLabel(w, 10, 10, "Status: idle");
   abi.AddButton(w, 10, 40, 60, 20, "Go", &OnGo);
   abi.BindHotkey(VK_F8, 0, w);
   abi.RegisterPacketHandler(0x2000, &OnReply, nullptr);
   ```

4. Ship your DLL into `edits/` alongside `custom-ui-host.dll`. The
   `ijl15.dll` proxy loads both.

## Server-side test handler

The demo expects the server to register an echo handler for opcode
`0x0F00`. The reference atlas-ms test handler returns opcode `0x2000`
with a 4-byte little-endian counter as the payload:

```go
// Pseudocode for the test server.
on(0x0F00) {
    counter++
    send(0x2000, encode4LE(counter))
}
```

Without that server-side handler the demo can still toggle the window
via F8 and click "Ping", but the label will never update.

## INI

`custom-ui-demo.ini` (ships next to the DLL):

```ini
[demo]
toggle_hotkey_vk = 119
toggle_hotkey_shift = false
toggle_hotkey_ctrl = false
toggle_hotkey_alt = false
```
```

- [ ] **Step 2: Commit**

```bash
git add custom-ui-demo/README.md
git commit -m "docs(custom-ui-demo): consumer-author 4-step guide"
```

---

## Phase 11 — Manual acceptance (executed against a running v83.1 client)

These are not auto-executable; the implementing agent flags them as
**owner-completes** items. Each acceptance criterion from PRD §10.3–10.5
maps to one or more log-line patterns to grep for during a live test
session.

### Task 11.1: Document acceptance log patterns

**Files:**
- Create: `custom-ui-host/docs/acceptance.md`

- [ ] **Step 1: Enumerate acceptance log patterns**

```markdown
# Manual acceptance log patterns

Tail the client's `OutputDebugString` capture (DbgView or similar) and
grep for these patterns to verify each PRD §10.3–10.5 criterion.

| Criterion | Pattern |
|---|---|
| 10.3.a host+demo logged "ready" | `custom-ui-host: ready` AND `custom-ui-demo: window built, ready` |
| 10.3.b F8 toggles window | (visual) window appears/disappears on rising F8 edge |
| 10.3.c "Ping" send | `custom-ui-demo: OnPing -> SendPacket(0x0F00)` |
| 10.3.d label updates | `custom-ui-demo: OnPong opcode=0x2000 seq=N` and visible "Server says: pong N" |
| 10.4.a host absent | `custom-ui-demo: host DLL not loaded -- demo inert` |
| 10.4.b double-load | `custom-ui-host: another host instance is already running` |
| 10.4.c reserved vk rejected | `custom-ui-host: BindHotkey rejected -- vk=0xXX already mapped` |
| 10.4.d out-of-range register | `custom-ui-host: RegisterPacketHandler called …` + return 0 |
| 10.4.e out-of-range send | `custom-ui-host: SendPacket rejected -- opcode 0xXXXX outside outbound range` |
| 10.4.f AV in callback | `custom-ui-host: AV in consumer callback at site=[button-click]` |
| 10.5.a channel change | host log shows hide+show across stage transition (visual) |
| 10.5.c disconnect+reconnect | host re-installs cleanly (no double-hook crash log) |
```

- [ ] **Step 2: Commit**

```bash
git add custom-ui-host/docs/acceptance.md
git commit -m "docs(task-005): list manual acceptance log patterns"
```

---

## Self-review against PRD/design

The plan covers (mapped to PRD §):

- §4.1 Host lifecycle → Phase 3 (mutex, INI, ready flag); Phase 5 (vtable init in MainProc); Phase 7 (hook install in MainProc)
- §4.2 Window lifecycle → Phase 5 + Phase 6.2 (Create/Show/Hide/Destroy)
- §4.3 Controls → Phase 6.5 (Label/Button/Edit + Set/Get text)
- §4.4 Hotkey binding → Phase 4.1–4.2 (registry); Phase 6.3 (ABI bind+conflict probe); Phase 7.1 (H2 hook)
- §4.5 Packets → Phase 4.3 (registry); Phase 6.4 (ABI); Phase 7.2 (H1 hook)
- §4.6 Stage transition → Phase 7.3 (H3a snapshot); Phase 7.4 (H3b restore latch)
- §4.7 ABI versioning → Phase 6.1 (header constant); Phase 6.2 (GetAbiVersion impl); Phase 8.4 (consumer check)
- §4.8 Logging + DumpRegistries → Phase 6.5
- §5 Hooks → Phase 7
- §6 Config → Phase 3.3 (host); Phase 8.1 step 6 (demo)
- §7 Memory map → Phase 1
- §8 NFRs → Phase 2 (common/ thunks lower duplication); Phase 5.1 (SEH); existing build system already provides NFR §8.4 detour parity
- §10 Acceptance → Phase 11

Type/name consistency checks:
- `CustomUI_WindowHandle` is `int` at ABI but `uint32_t` (`WindowHandle`) internally. The ABI cast is consistent everywhere
  (always `static_cast<unsigned int>(h)` going in, `static_cast<CustomUI_WindowHandle>(handle)` coming out).
- `HotkeyId`, `HandlerId`, `CtrlId` are `uint32_t` internally and `int` at the ABI — same cast pattern.
- `FrameworkExtras` field names match between header and `.cpp` (`is_visible`, `was_visible`, `handle`, `user`, `controls`).
- `g_packets`/`g_hotkeys`/`g_windows` references match the singletons declared in `abi_globals.h`.
- `C_UI_WND_DTOR` used in two places (`custom_ui_wnd.cpp::Destroy` + `vtable_patch.cpp::DtorOverride`) — same address, consistent intent.

Placeholders scan:
- `TODO_RUNTIME` appears once at plan line ~2835 (Phase 6.5 Step 2, button click vtable slot index). This is a genuine
  "IDA-determined index" the implementer must record at Phase 6.5 execution time — the surrounding `0` placeholder is
  documented as such. The plan executor must replace the slot index before commit; the comment block instructs them to
  validate by stepping through `CCtrlButton` click dispatch in IDA. Not a plan defect.
- The phrase `0xTBD` appears once at plan line ~2290 as narrative text inside a Task 5.4 *fallback discussion*
  ("if `CWvsApp::m_pWndMan` doesn't exist, Concrete fallback: add `set(C_WND_MAN_INSTANCE_ADDR 0xTBD)` …"). This is not
  a code placeholder the agent should leave in source — it's a description of a contingent recovery path that the agent
  decides during execution. If the fallback fires, the agent resolves the address via IDA in the same task and writes
  the real value. Documented contingency, not a defect.
- Every `0xXXXXXXXX` placeholder in code blocks is inside a Phase 1 IDA-resolution task that explicitly fills it.

---

## Execution

Plan complete and saved to `docs/tasks/task-005-custom-ui-framework/plan.md`.
Context summary saved to `docs/tasks/task-005-custom-ui-framework/context.md`.

Execution is not invoked from this phase. User will run `/clear` and then
`/execute-task task-005` separately to begin implementation.



