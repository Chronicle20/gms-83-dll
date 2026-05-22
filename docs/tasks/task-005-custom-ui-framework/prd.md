# Custom UI Framework — Product Requirements Document

Version: v1
Status: Draft
Created: 2026-05-22
---

## 1. Overview

This task introduces a **reusable in-DLL UI framework** that lets future client-edit DLLs
add custom in-game windows (toggled by configurable hotkeys) that exchange data with the
server over **newly reserved opcodes** — without hijacking any vanilla protocol or
duplicating client-side hooks across multiple edit DLLs.

The framework ships as its own loader DLL — `custom-ui-host` — alongside the other edits
in the `edits/` folder. It installs the small set of client-side hooks once, owns the
registries for windows / hotkeys / inbound handlers, and exposes a stable C ABI through
`GetProcAddress`. Consumer edit DLLs (e.g. a future "guild stash overlay" or "drop-table
viewer") are normal edits that `LoadLibrary` the host, resolve the exports, and call
`CustomUI_*` functions to declare their UI. They never install their own hooks.

The first concrete deliverable beyond the framework itself is a tiny **`custom-ui-demo`**
edit that exercises every layer end-to-end: a hotkey-toggled window with a label and a
button; the button sends a custom outbound opcode; the server replies with a custom
inbound opcode; the label updates from the reply. The demo proves the whole pipeline and
serves as the canonical example for downstream consumer authors.

## 2. Goals

Primary goals:
- Make it possible to add new in-game UI windows from an edit DLL without touching
  the host DLL or the game binary directly.
- Provide a stable C ABI so multiple consumer DLLs can coexist with one another and
  with version updates of the host DLL (semver gating on `CustomUI_GetAbiVersion`).
- Support two new packet opcode ranges (custom inbound and outbound) with a registry
  that dispatches inbound packets to consumer-registered handlers.
- Bind hotkeys to windows with conflict detection against the vanilla key map.
- Deliver a working `custom-ui-demo` edit that exercises every framework primitive.

Non-goals:
- Replacing, restyling, or re-implementing existing game UI.
- Server-driven UI definitions; window layouts are compile-time in consumer DLLs.
- Persisting custom hotkey bindings via the server-synced `CFuncKeyMappedMan` config.
  Custom hotkeys are configured per consumer DLL via INI.
- Rich control set. Milestone supports static text label + push button + edit box.
  Combo box, scrollbar, checkbox, tab, list view: explicit follow-on work, not blocked
  by this PRD.
- Cross-region support. Implementation targets GMS only. Versioning macros remain so
  JMS/CMS can be ported later, but no JMS work is included.
- Animation, skinning, theming, drag-resize. Windows use default `CUIWnd` chrome.

## 3. User Stories

- As a **developer building a custom edit**, I want to declare an in-game window
  programmatically (via `CustomUI_CreateWindow` and `CustomUI_AddLabel`/`AddButton`)
  so that I don't have to re-implement window message routing, redraw scheduling, or
  control hit-testing.
- As a **developer building a custom edit**, I want to send and receive packets over
  reserved custom opcodes (`CustomUI_SendPacket`, `CustomUI_RegisterPacketHandler`)
  so that my UI can talk to my atlas-ms feature without colliding with vanilla
  protocol.
- As a **server operator**, I want to install or omit consumer edits independently
  of the host DLL so that I can mix and match optional UI features per deployment.
- As a **server operator**, I want the framework to refuse hotkeys that conflict
  with the vanilla key map so that installing a consumer DLL doesn't silently break
  an existing player keybind.
- As a **player**, I want custom windows to auto-hide during channel changes / cash
  shop transitions and restore visibility afterward so that the UI doesn't fight the
  loading screen or overlap unrelated UI.

## 4. Functional Requirements

### 4.1 Host DLL lifecycle

- `custom-ui-host.dll` is a normal edit DLL loaded by the existing `ijl15.dll` proxy
  from the `edits/` folder.
- On `DLL_PROCESS_ATTACH`, the host spawns its `MainProc` thread which:
  1. Acquires a named global mutex (`Local\custom-ui-host-singleton`). If already
     held, the host logs a warning, skips hook installation, but still exposes its
     C ABI as no-ops to avoid crashing consumer DLLs. The second-load case must
     not corrupt the running framework instance.
  2. Installs the hooks listed in §5.
  3. Marks the host as ready; consumer DLLs' `MainProc` threads block on a small
     readiness check (a host export `CustomUI_IsReady` returning bool) before
     issuing any registration calls.

### 4.2 Window lifecycle

- `CustomUI_CreateWindow(title, x, y, w, h, user) -> WindowHandle` instantiates a
  framework-owned `CUIWnd` subclass that is **not yet registered** with `CWndMan`
  (i.e. not visible).
- `CustomUI_ShowWindow(handle)` calls `CWndMan::RegisterUIWindow` on the underlying
  `CUIWnd`. `CustomUI_HideWindow(handle)` calls `CWndMan::UnregisterUIWindow`.
  Show/Hide is idempotent (showing a visible window or hiding a hidden one is a
  silent no-op).
- `CustomUI_DestroyWindow(handle)` hides first if visible, then destroys the
  underlying `CUIWnd` and all owned controls, then frees the handle. Subsequent
  calls with the destroyed handle return error.
- Windows survive map changes as objects; their visibility is auto-suspended on
  stage end and auto-restored on next stage if the consumer had them visible. See
  §4.6.

### 4.3 Controls (milestone scope)

- `CustomUI_AddLabel(handle, x, y, text) -> ctrlId` — static text. Position relative
  to the window's client area. Returns a control ID local to the window.
- `CustomUI_AddButton(handle, x, y, w, h, text, onClick) -> ctrlId` — push button.
  `onClick` is invoked from the game's UI message thread when the user releases the
  mouse over the button. Click dispatch routes through the game's existing
  `CCtrlButton` machinery; the framework only translates back to the C callback.
- `CustomUI_AddEdit(handle, x, y, w, h, initialText) -> ctrlId` — single-line edit
  box backed by `CCtrlEdit`.
- `CustomUI_SetLabelText(handle, ctrlId, text)` — updates a label.
- `CustomUI_GetEditText(handle, ctrlId, buf, bufLen) -> int` — returns the edit
  box content as UTF-8 (returns required length if `buf == NULL`).

All text inputs/outputs at the ABI boundary are **UTF-8**. The framework converts to
`unsigned short*` (UCS-2 / `wchar_t` per the game's existing convention) before
calling into `CUIWnd` / `CCtrlButton` / `CCtrlEdit`.

### 4.4 Hotkey binding

- `CustomUI_BindHotkey(vk, modifiers, target) -> hotkeyId` where `vk` is a Windows
  `VK_*` value, `modifiers` is a bitmask (`MOD_SHIFT`, `MOD_CTRL`, `MOD_ALT`), and
  `target` is the `WindowHandle` whose visibility will be toggled.
- Before registration the framework calls `CFuncKeyMappedMan::FuncKeyMapped(vk)` on
  the game singleton. If the returned `FUNCKEY_MAPPED` represents a mapped function
  (type != unmapped sentinel), the bind is **rejected** and returns `0`. If the
  bind would also collide with another consumer's registered hotkey at the same
  `(vk, modifiers)`, it is rejected with the same error.
- On key-down events routed through `CWndMan::ProcessKey`, the framework matches
  registered hotkeys first; on match it toggles the bound window and consumes the
  event (does not call the original `ProcessKey`). On no match the original runs.
- `CustomUI_UnbindHotkey(hotkeyId)` removes the binding. Idempotent.

### 4.5 Custom packet plumbing

- Reserved ranges (claim from atlas-ms):
  - **Outbound (client → server):** `0x0F00` – `0x0FFF`
  - **Inbound (server → client):** `0x2000` – `0x20FF`
- `CustomUI_SendPacket(opcode, payload, len)` — must reject opcodes outside the
  outbound range with a logged error. Internally allocates a `COutPacket`, calls
  `Init(opcode)`, `EncodeBuffer(payload, len)`, then `CClientSocket::SendPacket`
  on the singleton. The `COutPacket` is destructed normally.
- `CustomUI_RegisterPacketHandler(opcode, handler, user) -> handlerId` — must
  reject opcodes outside the inbound range. Registers `handler` for that opcode.
  Two registrations for the same opcode: second returns `0` (error) and the first
  remains active.
- Inbound dispatch: a hook at the top of `CClientSocket::ProcessPacket` peeks the
  opcode via `CInPacket::Decode2` (advancing the cursor; the original
  `ProcessPacket` will re-read it from offset 0 — verify cursor reset semantics
  during implementation). If the opcode is in the inbound custom range, the handler
  is invoked with the **payload bytes following the opcode** and the original
  `ProcessPacket` is **not** called. If outside the custom range, the original is
  called normally with the original packet state.
- `CustomUI_UnregisterPacketHandler(handlerId)` — idempotent.

### 4.6 Stage transition

- The framework hooks the stage-change boundary (specific hook site TBD during
  implementation; candidates are `CStage` dtor, `CWvsApp::SetStage`, or the
  `OnMigrateCommand` path).
- On stage-end: every visible custom window is `UnregisterUIWindow`-ed but the
  framework tracks the prior visibility state.
- On stage-begin (after the new stage is fully constructed): each window that was
  visible pre-transition is `RegisterUIWindow`-ed again.
- Windows that consumer code Show/Hides during the transition window observe the
  new state on the next stage; the framework does not race the consumer.

### 4.7 ABI versioning

- `CustomUI_GetAbiVersion() -> uint32_t` returns `(major << 16) | (minor << 8) | patch`.
  Initial release is `0x00010000` (`1.0.0`).
- Consumer DLLs call this before any registration. If the major version differs from
  what they were built against, they must refuse to call other framework APIs. This
  contract is enforced by convention; the host does not refuse calls from older
  consumers.

### 4.8 Logging and diagnostics

- All framework errors (rejected hotkey binds, opcode-range violations, double-loads,
  handler-already-registered) emit through the existing `Log()` helper. Consumer
  DLLs continue logging through their own loggers.
- A debug export `CustomUI_DumpRegistries()` writes the current window / hotkey /
  packet handler tables to the log. Useful for the demo and future debugging; not
  intended for release-time use by consumers.

## 5. Hook / Patch Surface

The host installs **three** detours via MinHook (the existing `hooker.h` pattern).

| # | Hook target | v95 address | v83.1 address | Purpose |
|---|---|---|---|---|
| 1 | `CClientSocket::ProcessPacket(CInPacket&)` | `0x004b00f0` | `0x004965F1` ✓ (already in memory map as `C_CLIENT_SOCKET_PROCESS_PACKET`) | Inbound custom-opcode dispatch (§4.5). |
| 2 | `CWndMan::ProcessKey(uint, uint, long)` | `0x009b4590` | TBD (memory map addition) | Hotkey toggle dispatch (§4.4). |
| 3 | Stage-end notification | TBD | TBD | Auto-hide / auto-restore across stages (§4.6). Implementation chooses the cleanest hook site; PRD does not pre-commit. |

**No hook** is needed for outbound: `CustomUI_SendPacket` calls the public
`CClientSocket::SendPacket` directly. Drawing, mouse handling, focus, and control
hit-testing are delegated entirely to the game's `CWndMan` / `CUIWnd` machinery via
`RegisterUIWindow`.

### Read-only game APIs called (not hooked)

These functions are called normally and require accurate v83 addresses + signatures
in `common/` headers and the memory map:

- `CUIWnd::CUIWnd(x, y, w, h, name, …)` ctor — base for the framework's window class
- `CUIWnd` vtable: `OnDraw`, `OnSetFocus`, dtor (for our subclass to override correctly)
- `CWnd` base class vtable entries used by `CUIWnd`
- `CCtrlButton::CCtrlButton(…)` ctor + click message handler
- `CCtrlEdit::CCtrlEdit(…)` ctor + accessor for current text
- `CWndMan::RegisterUIWindow(CUIWnd*)`
- `CWndMan::UnregisterUIWindow(CUIWnd*)`
- `CClientSocket::SendPacket(const COutPacket&)`
- `COutPacket::COutPacket()` default ctor + `Init(int)` + `EncodeBuffer(const void*, uint)` + dtor
- `CInPacket::Decode2()` + raw-buffer accessor (for handler payloads)
- `CFuncKeyMappedMan::FuncKeyMapped(int)` — for hotkey conflict detection (§4.4)
- `CFuncKeyMappedMan` singleton accessor `TSingleton<CFuncKeyMappedMan>::GetInstance()`

`FUNCKEY_MAPPED` struct layout: the framework needs to read the "type" field (the
first field, indicating whether the slot is mapped) and treat 0 as "unmapped". Layout
confirmation is part of the v83 port surface for this task.

## 6. Configuration

### 6.1 Host DLL config (`custom-ui-host.ini`)

```ini
[host]
; Whether to log every framework call. Off by default in release.
verbose = false

; Opcode range overrides (rarely needed; documented for completeness).
; If atlas-ms ever shifts the agreed ranges, these can be retuned without rebuild.
inbound_opcode_min = 0x2000
inbound_opcode_max = 0x20FF
outbound_opcode_min = 0x0F00
outbound_opcode_max = 0x0FFF
```

### 6.2 Consumer DLL config

Consumer DLLs carry their own INI files following the existing per-edit pattern.
The framework does not impose a schema; consumer authors typically use INI to
configure hotkey defaults so server operators can rebind without a rebuild.

Example for the demo:

```ini
[demo]
; Window toggle hotkey (Windows VK_* value, decimal)
toggle_hotkey_vk = 119   ; VK_F8
toggle_hotkey_shift = false
toggle_hotkey_ctrl = false
toggle_hotkey_alt = false
```

## 7. Memory Map Impact

### 7.1 New `memory_maps/GMS/v83_1.cmake` entries

```cmake
# CWndMan UI registration + key dispatch
set(C_WND_MAN_REGISTER_UI_WINDOW   0xTBD)  # ::RegisterUIWindow(CUIWnd*)
set(C_WND_MAN_UNREGISTER_UI_WINDOW 0xTBD)  # ::UnregisterUIWindow(CUIWnd*)
set(C_WND_MAN_PROCESS_KEY          0xTBD)  # ::ProcessKey(uint, uint, long)

# CUIWnd + base controls
set(C_UI_WND_CTOR                  0xTBD)  # ::CUIWnd(x,y,w,h,name,...)
set(C_UI_WND_VFTABLE               0xTBD)
set(C_CTRL_BUTTON_CTOR             0xTBD)
set(C_CTRL_BUTTON_VFTABLE          0xTBD)
set(C_CTRL_EDIT_CTOR               0xTBD)
set(C_CTRL_EDIT_VFTABLE            0xTBD)

# COutPacket completion
set(C_OUT_PACKET_INIT              0xTBD)  # ::Init(int opcode)
set(C_OUT_PACKET_DTOR              0xTBD)

# CFuncKeyMappedMan singleton access for conflict detection
set(C_FUNC_KEY_MAPPED_MAN_GET_INSTANCE 0xTBD)
set(C_FUNC_KEY_MAPPED_MAN_FUNC_KEY_MAPPED 0xTBD)

# Stage-end hook site (final choice deferred to implementation)
set(C_STAGE_END_HOOK_SITE          0xTBD)
```

Symbols already present and reused as-is:

- `C_CLIENT_SOCKET_PROCESS_PACKET` (`0x004965F1`)
- `C_CLIENT_SOCKET_SEND_PACKET` (`0x0049637B`)
- `C_OUT_PACKET` (default ctor, `0x006EC9CE`)
- `C_OUT_PACKET_ENCODE_BUFFER` (`0x0046C00C`)
- `C_FUNC_KEY_MAPPED_MAN` (`0x0058DD0D`)

### 7.2 Future region/version coverage

Implementation targets only `GMS/v83_1`. The corresponding `v87_1`, `v95_1`, `v111_1`,
and `JMS/v185_1` maps remain unmodified by this task; cross-version coverage is a
follow-on PRD.

## 8. Non-Functional Requirements

### 8.1 Stability

- Host hooks must be idempotent under any reasonable race (game thread vs. host
  init thread). The mutex check in §4.1 + MinHook's own atomicity is the
  expectation; concurrent hook install from two host instances must be impossible.
- Consumer DLL crashes (segfault in a callback) must not corrupt the host or the
  game. The framework wraps every callback dispatch in a structured exception
  handler that logs and swallows access violations. (Stability over correctness:
  a buggy consumer DLL takes its window out of service, the game keeps running.)

### 8.2 Side effects

- Vanilla UI behavior must be byte-identical with the host loaded when no
  consumer DLLs are present. Specifically:
  - `CWndMan::ProcessKey` original path runs for every key not in the hotkey
    registry.
  - `CClientSocket::ProcessPacket` original path runs for every opcode outside
    the inbound custom range.
  - Stage transitions are observable only as additional `Register`/`Unregister`
    calls on framework-owned windows; vanilla windows are untouched.

### 8.3 Performance

- Hotkey lookup is `O(hotkey count)`; expected count is <10 across all consumers.
- Inbound packet dispatch lookup is `O(1)` on a `std::map` keyed by opcode
  (small N; switching to `unordered_map` is fine but not required).
- Per-frame overhead from the framework must be zero when no windows are visible.

### 8.4 AV / Themida compatibility

- No new IAT patches; all hooks are MinHook detours, matching the existing
  `bypass/` and `enable-minimize/` patterns. Themida tolerance is therefore the
  same as the existing edits.
- No code is written into the game's `.text` section beyond what MinHook itself
  writes.

### 8.5 Testability

- Each framework component (HotkeyRegistry, PacketRegistry, WindowRegistry, ABI
  shims) is independently unit-testable against mocked game-side surfaces. Tests
  live alongside existing tests under `tests/` and run during `cmake --build`.
- Integration testing is manual via the `custom-ui-demo` edit. Acceptance criteria
  in §10 enumerate the required manual scenarios.

## 9. Open Questions

All open questions from the brainstorming phase were resolved (opcode ranges,
hotkey conflict detection, userdata pattern, stage transition). Remaining
questions are implementation-detail decisions deferred to the design phase:

1. **Stage-end hook site** — `CStage` dtor vs. `CWvsApp::SetStage` vs.
   `OnMigrateCommand`. Decided during design after deeper v95 inspection.
2. **`CInPacket` cursor semantics** — does peeking the opcode at the top of
   `ProcessPacket` then calling the original re-read from offset 0, or must we
   reset the cursor explicitly? Verified during implementation; PRD assumes
   reset-or-rewind is achievable.
3. **`FUNCKEY_MAPPED` layout** — exact field offsets for the type/unmapped
   sentinel. Pinned during the v83 port of `CFuncKeyMappedMan`.

## 10. Acceptance Criteria

### 10.1 Build

- [ ] `custom-ui-host/` and `custom-ui-demo/` edits build clean on the
      `BUILD_REGION=GMS BUILD_MAJOR_VERSION=83 BUILD_MINOR_VERSION=1` config.
- [ ] CI workflow matrix is extended to include `custom-ui-host` and
      `custom-ui-demo` for the v83.1 GMS build.
- [ ] No warnings introduced under the project's existing warning level.

### 10.2 Framework unit tests

- [ ] `HotkeyRegistry`: register / lookup-by-vk-and-mods / unbind / double-bind
      rejection / clear-all.
- [ ] `PacketRegistry`: register / lookup / unbind / opcode-range validation /
      double-register rejection.
- [ ] `WindowRegistry`: handle lifecycle / use-after-destroy returns error /
      no leaks under repeated create/destroy.
- [ ] All ABI shims tested via mock game surface.

### 10.3 Demo end-to-end (manual, in-game on v83.1 GMS)

- [ ] With host + demo installed, launch client; both DLLs log "ready".
- [ ] In-game press configured hotkey (default F8): demo window appears at the
      configured position with "Server says: ?" label and "Ping" button.
- [ ] Click "Ping": client log shows outbound `0x0F00`; server (atlas-ms test
      handler) receives it; server replies `0x2000` with "pong N" (N
      incrementing per click).
- [ ] Demo label updates to "Server says: pong N" within one redraw cycle.
- [ ] Press hotkey again: window hides; label state persists (next show shows
      latest text).
- [ ] Repeat 20+ times across at least one map change without crash, leak,
      or stuck UI state.

### 10.4 Negative paths

- [ ] Host DLL absent, demo DLL present: demo logs warning, game runs fine,
      no UI appears.
- [ ] Host DLL loaded twice (forced by duplicating it under a different name):
      second instance logs the mutex check and no-ops; original instance
      continues to serve. No double-hook crash.
- [ ] `BindHotkey` with a vk already mapped in `CFuncKeyMappedMan` (e.g. main
      attack key): returns `0`, no toggle behavior bound, error logged.
- [ ] `RegisterPacketHandler` for opcode outside `0x2000-0x20FF`: returns `0`,
      error logged.
- [ ] `SendPacket` for opcode outside `0x0F00-0x0FFF`: returns without sending,
      error logged.
- [ ] Consumer DLL crashes in `onClick` callback: SEH wrapper logs the AV,
      game continues, window remains usable on next click attempt.

### 10.5 Stage transitions

- [ ] Open demo window. Change channel: window auto-hides during transition,
      re-appears after the new stage loads, with state preserved.
- [ ] Open demo window. Enter cash shop: window auto-hides; on cash shop exit
      window re-appears.
- [ ] Disconnect with window open: no crash; on next login the host re-installs
      cleanly.

### 10.6 Documentation

- [ ] `README.md` "Edits" section gains a `custom-ui-host` and `custom-ui-demo`
      entry pointing to their respective consumer-author docs.
- [ ] `custom-ui-host/README.md` documents the C ABI surface, the opcode
      ranges, and the hotkey conflict semantics.
- [ ] `custom-ui-demo/README.md` walks a new consumer-DLL author through the
      4-step minimum to ship a window of their own.
