# Custom UI Framework — Design

Version: v1
Status: Draft
Created: 2026-05-22
Companion to: [`prd.md`](./prd.md), [`hook-design.md`](./hook-design.md), [`risks.md`](./risks.md)

---

## 0. How to read this document

This design assumes the PRD's goals, scope, and acceptance criteria are
authoritative. The design's job is to commit to **how** the framework is built
inside this codebase: module layout, hook installation pattern, game-class
extension strategy, vtable subclassing scheme, packet/cursor handling, ABI
shape, and CMake/CI integration. Where the PRD or `hook-design.md` lists
"TBD" / "candidates", this document resolves to a single chosen path with a
short rationale and a documented fallback.

The design **corrects two factual errors in the PRD** that surfaced during
context exploration; they are flagged inline in §1.

---

## 1. PRD corrections

### 1.1 Hook framework — Detours, not MinHook

PRD §8.4 and `hook-design.md` state hooks are installed via MinHook. The
project actually uses **Microsoft Detours** uniformly across every existing
edit:

- `common/hooker.h` includes `detours.h` and links `detours.lib`.
- `SetHook(BOOL bInstall, void** ppvTarget, void* pvDetour)` wraps
  `DetourTransactionBegin` / `DetourAttach` / `DetourTransactionCommit`.
- All `bypass/`, `redirect/`, `no-patcher/`, `enable-minimize/` installers
  go through `INITMAPLEHOOK_OR_RETURN`.

This design specifies **Detours via `common/hooker.h`**. No new hook library
is introduced. The PRD's AV / Themida claim still holds — Detours' detour
trampolines are what the existing edits already incur; the host adds no new
detection surface category.

The PRD body and `hook-design.md` should be updated as a small editorial
fix when this design is approved; the implementation plan will list that
edit as a discrete task.

### 1.2 Game-class surface — much of it is unported

PRD §5 lists "read-only game APIs" including `CUIWnd` ctor + vtable,
`CCtrlButton` ctor + click handler, `CCtrlEdit` ctor + text accessor,
`CWndMan::RegisterUIWindow` / `UnregisterUIWindow` / `ProcessKey`,
`CFuncKeyMappedMan::FuncKeyMapped` + singleton accessor, and `CInPacket`
peek. Inspection of `common/` shows the corresponding headers are
**field-only stubs** with no methods or vtables declared in source:

| Class                  | State in `common/`                                                 |
| ---------------------- | ------------------------------------------------------------------ |
| `CUIWnd`               | Struct with fields. No ctor, no vtable type, no methods.            |
| `CWnd`                 | Class with fields + enum. No methods.                              |
| `CCtrlButton`          | Class with fields + `CREATEPARAM` nested. No ctor, no methods.     |
| `CCtrlEdit`            | Struct with fields + `CEditCaret` nested. No ctor, no methods.     |
| `CWndMan`              | Class with `s_Update`, `RedrawInvalidatedWindows` only.            |
| `CFuncKeyMappedMan`    | Class with fields, ctor (thunk), `CreateInstance`. No `FuncKeyMapped`. |
| `CInPacket`            | Struct with fields. No `Decode2`.                                  |
| `COutPacket`           | Class with `Encode*`, `MakeBufferList`. Already usable for §4.5.   |
| `CClientSocket`        | Has `GetInstance`, `SendPacket(COutPacket*)`. Usable as-is.        |
| `CStage`               | Class with `OnMouseEnter`, `OnPacket`. No dtor.                    |

Per the decisions in §2 below, the design adopts a **header-extension
strategy**: extend the affected `common/` headers with the method
declarations the framework needs, with implementations wired to v83
memory-map addresses through the existing thunk pattern used elsewhere in
the codebase (e.g. `CFuncKeyMappedMan::CreateInstance`). This is heavier
than declaring local typedefs inside the host DLL but produces re-usable
surface for future edits, matches how the codebase has been growing, and
avoids duplicating the same trampolines across consumer DLLs.

---

## 2. Decisions locked

| # | Decision | Choice |
|---|----------|--------|
| D1 | Hook installation | Microsoft Detours via `common/hooker.h` + `INITMAPLEHOOK_OR_RETURN`. |
| D2 | Game API surface | Extend `common/` headers with method declarations; implementations are thunks to memory-map addresses. |
| D3 | `CUIWnd` ownership | Shared cloned vtable patched once at host init; every framework window's vptr points at that copy. |
| D4 | Stage-end hook site | Hook `CStage::~CStage` for stage-end signal; latch a one-shot "restore on next `CWndMan::s_Update`" for stage-begin. |
| D5 | ABI calling convention | `__cdecl` `extern "C"` exports. Standard for Windows DLL `GetProcAddress`. |
| D6 | Callback threading | Consumer callbacks run on the thread that delivered the event (game UI thread for hotkey/click/draw; same for inbound packet dispatch since `ProcessPacket` is invoked from the WM_SOCKET pump on the game thread). No marshaling. Documented. |
| D7 | Crash isolation | Every dispatch into consumer code is wrapped with `__try / __except(EXCEPTION_EXECUTE_HANDLER)` filtering on `EXCEPTION_ACCESS_VIOLATION`. Other exceptions propagate. |
| D8 | Logging | Existing `Log(...)` from `common/logger.h`. No new logger. |
| D9 | INI parsing | Existing `parse_ini.h`. No new parser. |
| D10 | Demo packaging | Two edits: `custom-ui-host` + `custom-ui-demo`. Demo `LoadLibrary("custom-ui-host.dll")` and `GetProcAddress`-resolves the C ABI. Both ship in `edits/`. |

---

## 3. Repository layout

Two new top-level edit directories:

```
custom-ui-host/
    CMakeLists.txt
    pch.h / pch.cpp
    dllmain.cpp              # DLL_PROCESS_ATTACH -> spawn MainProc
    host_main.cpp            # MainProc: mutex, hook install, ready flag

    abi/
        custom_ui_abi.h       # Public C ABI header (also installed for consumers)
        custom_ui_abi.cpp     # Exported __cdecl C functions; thin shims

    hooks/
        process_packet_hook.cpp/.h    # H1
        process_key_hook.cpp/.h       # H2
        stage_dtor_hook.cpp/.h        # H3
        s_update_hook.cpp/.h          # H3 restore latch

    registries/
        window_registry.cpp/.h
        hotkey_registry.cpp/.h
        packet_registry.cpp/.h
        handle_table.h                # opaque-handle ID generator (shared)

    runtime/
        custom_ui_wnd.cpp/.h          # framework's CUIWnd subclass (placement-new + cloned vtable)
        vtable_patch.cpp/.h           # shared vtable clone + slot replacement
        seh_dispatch.h                # __try/__except wrappers (header-only)
        ini_config.cpp/.h             # opcode-range overrides from custom-ui-host.ini
        log_helpers.h                 # Log() wrappers with [custom-ui-host] prefix

custom-ui-demo/
    CMakeLists.txt
    pch.h / pch.cpp
    dllmain.cpp
    demo_main.cpp             # LoadLibrary host, wait CustomUI_IsReady, build window
    handlers.cpp              # onClick + packet handler

common/
    # Header extensions (D2). Each one gains *only* the methods the framework
    # calls; existing fields untouched. Implementations live next to existing
    # method thunks (e.g. CFuncKeyMappedMan.cpp) or are added in matching .cpp
    # files where none exists yet.
    CUIWnd.h           (add ctor / dtor / vtable typedef / placement helpers)
    CUIWnd.cpp         (new — placement-new wrapper around v83 ctor address)
    CWndMan.h          (add RegisterUIWindow / UnregisterUIWindow / ProcessKey)
    CWndMan.cpp        (extend; today only declares s_Update / RedrawInvalidatedWindows)
    CCtrlButton.h      (add ctor + click-handler typedef)
    CCtrlButton.cpp    (new)
    CCtrlEdit.h        (add ctor + text getter)
    CCtrlEdit.cpp      (new)
    CInPacket.h        (add Decode2 method + non-mutating Peek2 helper)
    CInPacket.cpp      (new — Decode2 thunks to address; Peek2 is offset-snapshot wrapper)
    CFuncKeyMappedMan.h    (add FuncKeyMapped(int))
    CFuncKeyMappedMan.cpp  (add method body — thunk + small wrapper)
    CStage.h           (add explicit ~CStage typedef for hook target; class already declared)

memory_maps/GMS/v83_1.cmake
    # Net new symbol entries (see §6).
```

Internal naming uses snake_case for files matching `bypass/`'s convention
(`socket_hooks.cpp`, `app_hooks.cpp`); class/method names match the
existing CamelCase used in `common/`.

---

## 4. Host DLL lifecycle

### 4.1 `DllMain` and `MainProc`

`dllmain.cpp` is byte-identical in structure to `bypass/dllmain.cpp`:
spawn `MainProc` on `DLL_PROCESS_ATTACH`, call
`DisableThreadLibraryCalls`, return TRUE.

`host_main.cpp:MainProc` does, in order:

1. Open / create the named mutex `Local\custom-ui-host-singleton` with
   `CreateMutexW`. If `GetLastError() == ERROR_ALREADY_EXISTS`, log a warning
   and set a global `g_doubleLoad = true` flag. Continue execution — the
   ABI shims check `g_doubleLoad` and return error sentinels instead of
   calling into the (uninitialized) registries. Hooks are **not** installed
   in the double-load case. The original host instance is unaffected.

2. Load `custom-ui-host.ini` next to the host DLL via `parse_ini.h`.
   Populate the four opcode-range globals (defaults from §6.1 of the PRD).
   On parse failure, log and use defaults.

3. Clone the `CUIWnd` vtable and patch the slots the framework overrides
   (see §8.2). This must complete before any window is constructed.

4. Install hooks H1–H3 via `INITMAPLEHOOK_OR_RETURN`. On any failure,
   `MainProc` returns `FALSE` and the host stays loaded but inert; the
   ready flag is never set.

5. Set `g_ready = true` (atomic). Consumer DLLs polling
   `CustomUI_IsReady()` then proceed.

The DLL has no uninstall path. Hooks remain through process exit
(matching every other edit).

### 4.2 Ready signaling

`CustomUI_IsReady()` returns `g_ready` directly (atomic load). Consumer
DLLs MUST poll this in their own `MainProc` thread until it returns true
before calling any other `CustomUI_*` function. The host does not provide
a blocking wait API — busy-wait with `Sleep(50)` is the documented pattern.
Rationale: the host can't safely take a consumer-supplied event handle
because the consumer DLL may not be fully initialized yet, and a
synchronous wait could deadlock host init in a multi-consumer load order.

---

## 5. C ABI

### 5.1 Header `custom_ui_abi.h`

```c
#ifdef __cplusplus
extern "C" {
#endif

typedef int CustomUI_WindowHandle;       // 0 = invalid
typedef int CustomUI_CtrlId;             // 0 = invalid (within a window)
typedef int CustomUI_HotkeyId;           // 0 = invalid
typedef int CustomUI_HandlerId;          // 0 = invalid

typedef void (__cdecl *CustomUI_OnClickFn)(CustomUI_WindowHandle w,
                                           CustomUI_CtrlId c,
                                           void* user);

typedef void (__cdecl *CustomUI_PacketHandlerFn)(unsigned short opcode,
                                                 const unsigned char* payload,
                                                 unsigned int payloadLen,
                                                 void* user);

/* Lifecycle */
__declspec(dllexport) unsigned int __cdecl CustomUI_GetAbiVersion(void);
__declspec(dllexport) int          __cdecl CustomUI_IsReady(void);

/* Windows */
__declspec(dllexport) CustomUI_WindowHandle __cdecl
    CustomUI_CreateWindow(const char* title, int x, int y, int w, int h, void* user);
__declspec(dllexport) int  __cdecl CustomUI_ShowWindow(CustomUI_WindowHandle h);
__declspec(dllexport) int  __cdecl CustomUI_HideWindow(CustomUI_WindowHandle h);
__declspec(dllexport) int  __cdecl CustomUI_DestroyWindow(CustomUI_WindowHandle h);

/* Controls */
__declspec(dllexport) CustomUI_CtrlId __cdecl
    CustomUI_AddLabel (CustomUI_WindowHandle h, int x, int y, const char* text);
__declspec(dllexport) CustomUI_CtrlId __cdecl
    CustomUI_AddButton(CustomUI_WindowHandle h, int x, int y, int w, int h_,
                       const char* text, CustomUI_OnClickFn onClick);
__declspec(dllexport) CustomUI_CtrlId __cdecl
    CustomUI_AddEdit  (CustomUI_WindowHandle h, int x, int y, int w, int h_,
                       const char* initialText);
__declspec(dllexport) int __cdecl
    CustomUI_SetLabelText(CustomUI_WindowHandle h, CustomUI_CtrlId c, const char* text);
__declspec(dllexport) int __cdecl
    CustomUI_GetEditText (CustomUI_WindowHandle h, CustomUI_CtrlId c,
                          char* buf, int bufLen);

/* Hotkeys */
__declspec(dllexport) CustomUI_HotkeyId __cdecl
    CustomUI_BindHotkey  (unsigned int vk, unsigned int modifiers,
                          CustomUI_WindowHandle target);
__declspec(dllexport) int __cdecl
    CustomUI_UnbindHotkey(CustomUI_HotkeyId id);

#define CUSTOM_UI_MOD_SHIFT 0x01
#define CUSTOM_UI_MOD_CTRL  0x02
#define CUSTOM_UI_MOD_ALT   0x04

/* Packets */
__declspec(dllexport) int __cdecl
    CustomUI_SendPacket(unsigned short opcode, const void* payload, unsigned int len);
__declspec(dllexport) CustomUI_HandlerId __cdecl
    CustomUI_RegisterPacketHandler  (unsigned short opcode,
                                     CustomUI_PacketHandlerFn fn, void* user);
__declspec(dllexport) int __cdecl
    CustomUI_UnregisterPacketHandler(CustomUI_HandlerId id);

/* Debug */
__declspec(dllexport) void __cdecl CustomUI_DumpRegistries(void);

#ifdef __cplusplus
}
#endif
```

ABI rules:

- All exports are `__cdecl extern "C"` with `__declspec(dllexport)`. Consumer
  DLLs resolve by name via `GetProcAddress`. No ordinals (R4 mitigation).
- Strings at the boundary are UTF-8. The shims convert to UCS-2
  (`ZXString<unsigned short>`) before calling `CUIWnd`/`CCtrlButton`
  internals. Conversion uses `MultiByteToWideChar(CP_UTF8, ...)`.
- All return-value-0 cases mean error. Diagnostics go through `Log()`.
- Pointers passed across the ABI are valid only for the duration of the
  call. The framework copies any data it retains.

### 5.2 ABI version

`CustomUI_GetAbiVersion()` returns `0x00010000` for the initial release
(`major=1, minor=0, patch=0`). Consumer DLLs are expected to reject mismatch
on **major**. The host does not enforce; it logs every API call's caller
module name on debug builds so version mismatches surface in the log.

### 5.3 Shim layer

`custom_ui_abi.cpp` contains nothing but the exported entry points.
Each entry point is short:

1. Check `g_ready` and `g_doubleLoad`; return error if not OK.
2. Argument validation (null pointer checks, range checks for opcodes).
3. UTF-8 → UCS-2 conversion where applicable.
4. Forward to the registries / runtime.
5. Convert internal results to the C ABI error sentinel.

This keeps the public surface trivially auditable and isolates the
exception-safety boundary at exactly one layer.

---

## 6. Memory map additions

### 6.1 v83.1 GMS

```cmake
# Hook targets
set(C_CLIENT_SOCKET_PROCESS_PACKET   0x004965F1)  # already mapped — reused
set(C_WND_MAN_PROCESS_KEY            0xTBD)       # ::ProcessKey(uint, uint, long)
set(C_STAGE_DTOR                     0xTBD)       # ~CStage()
set(C_WND_MAN_S_UPDATE               0xTBD)       # ::s_Update() — used for restore latch

# CWndMan registration
set(C_WND_MAN_REGISTER_UI_WINDOW     0xTBD)
set(C_WND_MAN_UNREGISTER_UI_WINDOW   0xTBD)

# CUIWnd + controls (ctor addresses + vtable addresses)
set(C_UI_WND_CTOR                    0xTBD)
set(C_UI_WND_VFTABLE                 0xTBD)
set(C_UI_WND_DTOR                    0xTBD)
set(C_CTRL_BUTTON_CTOR               0xTBD)
set(C_CTRL_BUTTON_VFTABLE            0xTBD)
set(C_CTRL_EDIT_CTOR                 0xTBD)
set(C_CTRL_EDIT_VFTABLE              0xTBD)

# CInPacket peek
set(C_IN_PACKET_DECODE2              0xTBD)       # CInPacket::Decode2()

# CFuncKeyMappedMan conflict probe
set(C_FUNC_KEY_MAPPED_MAN_GET_INSTANCE      0xTBD)
set(C_FUNC_KEY_MAPPED_MAN_FUNC_KEY_MAPPED   0xTBD)

# Already mapped, reused as-is:
#   C_CLIENT_SOCKET_SEND_PACKET (0x0049637B)
#   C_OUT_PACKET                (0x006EC9CE)
#   C_OUT_PACKET_ENCODE_BUFFER  (0x0046C00C)
#   C_FUNC_KEY_MAPPED_MAN       (0x0058DD0D)
```

Each `0xTBD` is a discrete step in the implementation plan: load v83.1 in
IDA, locate the symbol, write the address, update the `.cmake`, regenerate
`memory_map.h`. The plan groups these so the verifier can iterate
efficiently.

### 6.2 Sizes used at runtime

The framework needs `sizeof(CUIWnd)` (for placement-new), `sizeof(CCtrlButton)`,
`sizeof(CCtrlEdit)` at v83. These are derivable from IDA struct dumps. The
sizes are baked into the host DLL as `constexpr` values (e.g.
`constexpr size_t kSizeofCUIWnd_v83_1 = 0xNN;`) selected per build via
`#if BUILD_MAJOR_VERSION`. Mismatch is detected at host init: a
`static_assert` on the well-known offset of `CUIWnd::m_pBtClose` (the
first declared field after the inherited `CWnd` base layout, see
`common/CUIWnd.h`) plus a runtime sanity check that the field reads as a
plausible pointer after construction. If either check fails the host
logs and refuses to install hooks.

---

## 7. Hook details

### 7.1 H1: `CClientSocket::ProcessPacket`

```cpp
typedef void (__thiscall *ProcessPacket_t)(CClientSocket*, CInPacket*);
ProcessPacket_t _ProcessPacket = nullptr;

void __fastcall ProcessPacket_Hook(CClientSocket* self, void* edx, CInPacket* p) {
    const unsigned int savedOffset = p->m_uOffset;

    // Non-destructive opcode peek: clone the offset, call Decode2 against a
    // temporary CInPacket bound to the same buffer if available — but the
    // simplest, lowest-risk implementation is "advance, branch, restore".
    const unsigned short opcode = p->Decode2();   // mutates m_uOffset

    if (opcode >= g_inboundOpMin && opcode <= g_inboundOpMax) {
        // Custom range: dispatch to consumer-registered handler with the
        // payload bytes that follow the opcode. Cursor is currently past
        // the opcode, which is what consumers expect.
        const unsigned char* payload = p->m_aRecvBuff.GetBuffer() + p->m_uOffset;
        const unsigned int  payloadLen = p->m_uDataLen - p->m_uOffset;
        PacketRegistry::Dispatch(opcode, payload, payloadLen);
        // Vanilla path NOT invoked.
        return;
    }

    // Outside custom range: restore cursor and run vanilla.
    p->m_uOffset = savedOffset;
    _ProcessPacket(self, p);
}
```

**Cursor restoration is the load-bearing line.** Debug-build assertion
sanity-checks that the offset after `Decode2` advanced by exactly 2 bytes;
release builds skip the assertion. Open question §9.2 ("does the original
re-read from offset 0?") is decisively resolved by **always restoring**
— the original reads from `m_uOffset`, which we know because the field is
visible in `CInPacket.h` and is the cursor used by `Decode2`.

There is no non-mutating `Peek2` primitive in the game; we synthesize one
(`CInPacket::Peek2()` declared in `CInPacket.h`, implemented in
`CInPacket.cpp` as save/restore wrapper) for use by other consumers of
`common/` who might want it later, but H1 itself uses inline
save-Decode2-restore for minimal call overhead.

### 7.2 H2: `CWndMan::ProcessKey`

```cpp
typedef long (__thiscall *ProcessKey_t)(CWndMan*, unsigned int, unsigned int, long);
ProcessKey_t _ProcessKey = nullptr;

long __fastcall ProcessKey_Hook(CWndMan* self, void* edx,
                                unsigned int msg, unsigned int vk, long lParam) {
    if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) {
        // Debounce: lParam bit 30 = previous key state. Only fire on rising edge.
        if ((lParam & (1 << 30)) == 0) {
            const unsigned int mods = SnapshotModifiers();
            if (auto* binding = HotkeyRegistry::Lookup(vk, mods)) {
                ToggleVisibility(binding->target);
                return 1;  // consume
            }
        }
    }
    return _ProcessKey(self, msg, vk, lParam);
}

unsigned int SnapshotModifiers() {
    unsigned int m = 0;
    if (GetKeyState(VK_SHIFT)   & 0x8000) m |= CUSTOM_UI_MOD_SHIFT;
    if (GetKeyState(VK_CONTROL) & 0x8000) m |= CUSTOM_UI_MOD_CTRL;
    if (GetKeyState(VK_MENU)    & 0x8000) m |= CUSTOM_UI_MOD_ALT;
    return m;
}
```

Bind-time conflict detection (PRD §4.4) is implemented in `HotkeyRegistry::Bind`:

```cpp
HotkeyId Bind(unsigned int vk, unsigned int mods, WindowHandle target) {
    if (IsDenylisted(vk))                       return 0;   // ESC/Enter/Tab/arrows/mouse
    auto* mgr = CFuncKeyMappedMan::GetInstance();
    if (vk < kVkMapSize && mgr->FuncKeyMapped(vk).nType != 0) return 0;
    if (LookupExisting(vk, mods)) return 0;     // another consumer already bound
    return InsertNew(vk, mods, target);
}
```

`FUNCKEY_MAPPED::nType == 0` is the unmapped sentinel; the layout is in
`common/FunckeyMapped.h` and matches the IDA dump comment (`db ? at +0`).
PRD §9.3 ("FUNCKEY_MAPPED layout") is hereby resolved.

Denylist (R6 mitigation): VK_ESCAPE, VK_RETURN, VK_TAB, VK_LEFT/UP/RIGHT/DOWN,
VK_LBUTTON, VK_RBUTTON, VK_MBUTTON.

### 7.3 H3: `CStage::~CStage` + restore latch

`~CStage` is hooked at address `C_STAGE_DTOR` (v83 TBD). Detour:

```cpp
typedef void (__thiscall *StageDtor_t)(CStage*);
StageDtor_t _StageDtor = nullptr;

void __fastcall StageDtor_Hook(CStage* self, void* edx) {
    // The stage is about to be torn down. Snapshot which framework windows
    // are currently visible, hide them all, set the restore latch.
    WindowRegistry::SnapshotAndSuspendVisible();
    g_pendingRestore = true;
    _StageDtor(self);
}
```

The "stage is now active" signal is the **first invocation of
`CWndMan::s_Update` after `g_pendingRestore` is set**. We hook
`s_Update` for that purpose:

```cpp
typedef CWnd** (__cdecl *SUpdate_t)();
SUpdate_t _SUpdate = nullptr;

CWnd** SUpdate_Hook() {
    CWnd** rv = _SUpdate();
    if (g_pendingRestore) {
        g_pendingRestore = false;
        WindowRegistry::RestoreSnapshotedVisibility();
    }
    return rv;
}
```

Rationale: `~CStage` fires unconditionally on every stage tear-down, and
`s_Update` is the existing per-frame pump that we know runs on the game
thread once the new stage's `CWndMan` is live. The latch keeps the restore
synchronized with the next clean frame, avoiding races against partially
constructed stages (R7 mitigation).

`s_Update` is the one new hook the design adds beyond what `hook-design.md`
explicitly listed; it's a single-call latch with zero per-frame cost in
the steady state (one branch on `g_pendingRestore`).

---

## 8. Window subclassing (CustomUIWnd)

### 8.1 Object layout

The framework's window is a thin C++ wrapper around a raw byte buffer
sized to `sizeof(CUIWnd) + sizeof(FrameworkExtras)`:

```cpp
struct FrameworkExtras {
    CustomUI_WindowHandle handle;
    void* user;
    std::vector<ControlEntry> controls;   // small N, vector is fine
    bool wasVisible;                      // for stage-transition snapshot
};

class CustomUIWnd {
public:
    static CustomUIWnd* Create(int x, int y, int w, int h, const wchar_t* title,
                               void* user);
    void Show();   // -> CWndMan::RegisterUIWindow
    void Hide();   // -> CWndMan::UnregisterUIWindow
    void Destroy();
    CUIWnd*  GameWnd()  { return reinterpret_cast<CUIWnd*>(buf_); }
    FrameworkExtras& Extras() {
        return *reinterpret_cast<FrameworkExtras*>(buf_ + kCUIWndSize);
    }
private:
    alignas(8) unsigned char buf_[kCUIWndSize + sizeof(FrameworkExtras)];
};
```

`Create` does:

1. `new CustomUIWnd()` (raw byte buffer, untyped).
2. Placement-new call to the game's `CUIWnd::CUIWnd(x, y, w, h, name, …)`
   at `C_UI_WND_CTOR` via a thiscall trampoline targeting `buf_`.
3. **Patch the vptr** at `buf_+0` from the game's `CUIWnd` vtable to the
   cloned vtable (see §8.2).
4. Construct `FrameworkExtras` in place at `buf_+kCUIWndSize`.

`Destroy` does the reverse in reverse order; only the byte buffer is heap
memory the host owns, so cleanup is `~FrameworkExtras() + delete[] buf`.

### 8.2 Cloned vtable

At host init (`MainProc` step 3):

1. Read the v95-vs-v83 differences of the `CUIWnd` vtable. For v83 the
   exact slot count and slot order are baked into the design as constants
   derived from IDA inspection; the count is small (PRD §5 enumerates
   `OnDraw`, `OnSetFocus`, dtor as the slots we override, plus pass-through
   on all others).
2. `memcpy` the entire vtable from `C_UI_WND_VFTABLE` into a host-DLL-static
   array `g_customUIWndVftable[kVtableSlots]`.
3. Overwrite the slots we own: `OnDraw`, `OnSetFocus`, `OnMouseLButton*`,
   `dtor`, and (for control delivery) the message-handling slot from
   `IUIMsgHandler`. Each replacement is a free function with `__fastcall`
   shim that looks up our `FrameworkExtras` (recoverable from the `this`
   pointer via `buf_ + kCUIWndSize` arithmetic) and dispatches the
   corresponding consumer callback under SEH.

The cloned vtable is shared by every `CustomUIWnd` instance. Window
instances are distinguished by `this`, not by vtable identity. R7 (UAF on
stage transitions) is mitigated by the framework's discipline: `Destroy`
calls `Hide` first, and `Hide` is the game's own `Unregister`, so the
game's draw scheduler stops dispatching to the unregistered window before
the byte buffer is freed.

### 8.3 Controls

`CCtrlButton` and `CCtrlEdit` are constructed using the same placement-new
pattern but **without** vtable cloning: the framework wants the game's
native button and edit behavior. To receive click events, we hook the
button's vtable slot for the click message handler on a per-button basis
(each button gets a small vtable clone of its own; this is a cheap N=1
indirection per button and avoids a global hook on `CCtrlButton`).

The per-button vtable approach is **the one place we accept per-instance
vtable cloning** (rejected at the window level in §2/D3). Rationale: a
button's identity *is* its click target. Sharing a vtable across all
buttons would force a global dispatch table and a button-to-callback map
lookup inside the hot click path; per-instance cloning makes the callback
identity trivially recoverable.

`CCtrlEdit` does not need a custom vtable — the consumer pulls text out
on demand via `CustomUI_GetEditText` which calls a method we add to
`CCtrlEdit` (`GetText()` thunk to a yet-to-be-located v83 address; if no
single accessor exists, we read `m_sText` directly using the existing
field layout in `common/CCtrlEdit.h`).

---

## 9. Registries

Three registries, all with the same shape: opaque integer IDs minted from
an atomic counter, stored in a `std::unordered_map`, protected by a
`std::mutex`. ID = 0 reserved for "invalid".

| Registry | Key | Value | Operations |
|----------|-----|-------|------------|
| `WindowRegistry` | `WindowHandle` | `CustomUIWnd*` | Create/Destroy, ShowAll/HideAll for stage transitions |
| `HotkeyRegistry` | `HotkeyId` | `{vk, mods, target}` | Bind/Unbind/Lookup(vk, mods) |
| `PacketRegistry` | `HandlerId` | `{opcode, fn, user}` | Register/Unregister/Dispatch(opcode, payload, len) |

Each registry exposes a `Dump(LogSink&)` method for `CustomUI_DumpRegistries`.

`PacketRegistry::Dispatch` enforces "one handler per opcode": registration
keyed on opcode rejects when an entry already exists. Dispatch is `O(1)`
on `unordered_map<unsigned short, …>`.

`HotkeyRegistry::Lookup` is `O(hotkey_count)` linear scan (small N, no
hashing overhead worth the indirection).

`WindowRegistry::SnapshotAndSuspendVisible` walks all windows, records
`wasVisible` in each `FrameworkExtras`, calls `Hide` on visible ones.
`RestoreSnapshotedVisibility` does the inverse on entries where
`wasVisible == true`.

---

## 10. SEH wrapping

`seh_dispatch.h`:

```cpp
template <class F>
inline void SafeDispatch(const char* siteName, F&& fn) noexcept {
    __try {
        fn();
    } __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
                ? EXCEPTION_EXECUTE_HANDLER
                : EXCEPTION_CONTINUE_SEARCH) {
        Log("custom-ui-host: AV in consumer callback at site=[%s]", siteName);
    }
}
```

Used by:

- `PacketRegistry::Dispatch` around the handler call.
- Per-button click thunk around the `CustomUI_OnClickFn` call.
- Each cloned-vtable slot (OnDraw, OnSetFocus) around any consumer-affecting
  work — most slots only call the original game function and don't touch
  consumer code at all.

Lambdas plus templated `SafeDispatch` keep callsites readable. SEH cannot
cross C++ object boundaries inside the same function, so each lambda
captures only POD pointers and primitive arguments. Construction/destruction
of stack objects happens outside the lambda.

R3 (consumer callback fault propagation) is the explicit mitigation.

---

## 11. Demo edit (`custom-ui-demo`)

The demo is the simplest possible exercise of every primitive. Single TU
(`demo_main.cpp`) plus handlers:

```cpp
HMODULE hHost = nullptr;
CustomUI_WindowHandle gWnd = 0;
CustomUI_CtrlId       gLbl = 0;
int                   gPingCount = 0;

DWORD WINAPI MainProc(LPVOID) {
    hHost = LoadLibraryW(L"custom-ui-host.dll");
    if (!hHost) return -1;
    // Resolve exports (omitted for brevity).
    while (!CustomUI_IsReady()) Sleep(50);

    // Load demo INI for hotkey config.
    auto cfg = ParseDemoIni();

    gWnd = CustomUI_CreateWindow("Demo", cfg.x, cfg.y, 240, 80, nullptr);
    gLbl = CustomUI_AddLabel(gWnd, 10, 10, "Server says: ?");
    CustomUI_AddButton(gWnd, 10, 40, 60, 20, "Ping", &OnPing);
    CustomUI_BindHotkey(cfg.vk, cfg.modifiers, gWnd);
    CustomUI_RegisterPacketHandler(0x2000, &OnPong, nullptr);
    return 0;
}

void __cdecl OnPing(CustomUI_WindowHandle, CustomUI_CtrlId, void*) {
    CustomUI_SendPacket(0x0F00, nullptr, 0);
}

void __cdecl OnPong(unsigned short, const unsigned char* p, unsigned int n, void*) {
    char buf[64];
    int seq = (n >= 4) ? *reinterpret_cast<const int*>(p) : ++gPingCount;
    snprintf(buf, sizeof(buf), "Server says: pong %d", seq);
    CustomUI_SetLabelText(gWnd, gLbl, buf);
}
```

INI keys match PRD §6.2. No atlas-ms code lives in this repo; the demo's
end-to-end acceptance criteria (PRD §10.3) assume a small handler is
registered in the test atlas-ms instance for `0x0F00 → 0x2000` echo.
That handler is documented in `custom-ui-demo/README.md` but is not part
of this repo's deliverables.

---

## 12. Build, CMake, CI

### 12.1 CMake

Two new `add_edit_dll(...)` invocations at the top-level `CMakeLists.txt`,
matching how `bypass`, `redirect`, etc. are wired today:

```cmake
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

add_edit_dll(custom-ui-demo SOURCES
    dllmain.cpp
    demo_main.cpp
    handlers.cpp
)
```

The demo links against `custom_ui_abi.h` only (header-only public surface
plus runtime `GetProcAddress`); no static link dependency. This keeps
demo and host genuinely decoupled and proves the consumer-author flow.

### 12.2 CI matrix

`.github/workflows/...` already iterates region × major × minor.
Acceptance criterion (PRD §10.1) requires `BUILD_REGION=GMS
BUILD_MAJOR_VERSION=83 BUILD_MINOR_VERSION=1` to build both edits clean.
Other version/region combos are excluded for this milestone via a
conditional inside the new edits' CMakeLists — uniform with how
`memory_maps/GMS/v83_1.cmake` gates the rest of the build.

The framework's `#if`-guarded compile-out of non-v83-1 builds is:

```cmake
if(NOT (BUILD_REGION STREQUAL "GMS" AND BUILD_MAJOR_VERSION EQUAL 83
        AND BUILD_MINOR_VERSION EQUAL 1))
    message(STATUS "custom-ui-host: not building for ${BUILD_REGION}_v${BUILD_MAJOR_VERSION}_${BUILD_MINOR_VERSION}")
    return()
endif()
```

(Same idiom used elsewhere in the repo.)

---

## 13. Testing strategy

### 13.1 Unit tests (PRD §10.2)

Existing `tests/` runs unit tests via the standard cmake test target.
For this task we add `tests/custom_ui_host/`:

- `hotkey_registry_test.cpp` — bind / lookup / unbind / double-bind / denylist.
- `packet_registry_test.cpp` — register / lookup / unbind / range / double-register.
- `window_registry_test.cpp` — create / destroy / use-after-destroy / no leaks (instrument via a sentinel allocator).
- `seh_dispatch_test.cpp` — Confirms an AV in a callback does not crash. Uses `RaiseException` to simulate. Skipped on non-Windows test runners (the rest are pure C++).

The registries are pure C++ and don't touch any game-side address, so
they can be tested without IDA / a v83 client. The `seh_dispatch` test is
the only one that depends on Windows SEH semantics.

We deliberately do not unit-test the hook bodies themselves — they're
trivial wrappers around library calls and would require mocking
`CClientSocket` / `CInPacket` / `CWndMan`, which buys very little
confidence relative to the manual acceptance scenarios in PRD §10.3.

### 13.2 Manual acceptance (PRD §10.3–10.5)

Already enumerated in the PRD and not re-stated here. The implementation
plan should add to each item the *exact* log-line patterns the host emits
on success so a manual tester can mechanically verify by tailing the log.

---

## 14. Open items deferred to implementation

These are concrete unknowns that need IDA work, not design decisions:

| # | Item | Resolution path |
|---|------|-----------------|
| OI-1 | v83.1 addresses for every `0xTBD` in §6.1 | IDA inspection during implementation; same workflow as the existing `port-class` skill. |
| OI-2 | `sizeof(CUIWnd)`, `sizeof(CCtrlButton)`, `sizeof(CCtrlEdit)` at v83.1 | Same. Used as `constexpr` in `runtime/custom_ui_wnd.cpp`. |
| OI-3 | `CUIWnd` vtable slot count + index of each overridden slot | Same. Drives the cloned-vtable array size + slot patch offsets in `runtime/vtable_patch.cpp`. |
| OI-4 | Exact body of `CUIWnd` ctor (does it self-register with `CWndMan`?) | Affects whether `Create` follows ctor with an explicit `RegisterUIWindow` or whether show/hide is the only registration trigger. The design assumes the latter (ctor does not auto-register); plan validates. |
| OI-5 | Whether `CCtrlEdit` exposes a `GetText` accessor symbol vs. requiring direct field read | Plan checks via IDA; falls back to direct `m_sText` access using the existing field layout. |

Each item is a discrete, narrowly-scoped IDA task. None of them gate the
framework architecture, only its constants.

---

## 15. Implementation-plan handoff

The implementation plan (Phase 3) should split work into these task
clusters, in roughly this order:

1. **PRD/doc corrections** — drop MinHook references in PRD + hook-design.md.
2. **`common/` extensions** — header + implementation thunks for the methods this design needs (CUIWnd, CWnd, CCtrlButton, CCtrlEdit, CWndMan, CFuncKeyMappedMan, CInPacket, CStage). One PR-equivalent task per class.
3. **Memory map v83.1 additions** — resolve OI-1, OI-2, OI-3 in IDA; commit `memory_maps/GMS/v83_1.cmake` additions.
4. **`custom-ui-host` scaffolding** — CMake, pch, dllmain, MainProc skeleton, mutex check, INI parse.
5. **Registries** — three TUs, unit-tested as they land.
6. **Vtable clone runtime** — `runtime/vtable_patch.cpp` + `runtime/custom_ui_wnd.cpp`.
7. **ABI shims** — `abi/custom_ui_abi.cpp`, exports verified with `dumpbin /exports`.
8. **Hooks** — H1 (ProcessPacket), H2 (ProcessKey), H3 (StageDtor + s_Update). Land H2 first; it's the easiest to validate manually.
9. **`custom-ui-demo`** — depends on every other piece; final integration target.
10. **CI matrix** — extend to include the two new edits for v83.1.
11. **Manual acceptance** — execute PRD §10.3–10.5 end-to-end.
12. **README + consumer-author docs** — PRD §10.6.

The plan stage owns the granular task decomposition, owner assignment, and
ordering of subagent dispatches.
