# Custom UI Framework — Hook Design

Companion to `prd.md`. Captures the per-hook detour layout, call gates, and
known sequencing constraints. Addresses in v95 column are confirmed from the
loaded PDB (`GMS_v95.0_U_DEVM.exe`); v83 column entries marked TBD are
resolved during the implementation port.

## Hook inventory

| ID | Target | v95 addr | Mangled name | Purpose |
|----|--------|----------|--------------|---------|
| H1 | `CClientSocket::ProcessPacket` | `0x004b00f0` | `?ProcessPacket@CClientSocket@@IAEXAAVCInPacket@@@Z` | Custom inbound opcode dispatch |
| H2 | `CWndMan::ProcessKey` | `0x009b4590` | `?ProcessKey@CWndMan@@QAEJIIJ@Z` | Hotkey toggle dispatch |
| H3 | Stage-end notification | TBD | TBD | Auto-hide / restore on stage transitions |

All hooks are MinHook detours using the existing `INITWINHOOK`-style helper
pattern from `common/hooker.h` (adapted for in-process function-address hooks
rather than IAT entries — see `bypass/dllmain.cpp` for an example).

---

## H1: `CClientSocket::ProcessPacket`

### Original signature

```cpp
void __thiscall CClientSocket::ProcessPacket(CClientSocket* this, CInPacket* iPacket);
```

### v95 prologue summary

The function reads `g_pStage`, then calls `CInPacket::Decode2(iPacket)` to
peek the opcode, then runs a small switch (0x10–0x17 cases for auth /
migrate / security / CRC), and falls through to `CWvsContext::OnPacket`
(opcode ≤0x9c) or `CStage::OnPacket` (opcode >0x9c).

### Detour body

```cpp
void __thiscall ProcessPacket_Hook(CClientSocket* self, CInPacket* iPacket) {
    // 1. Snapshot the packet's current cursor position.
    uint32_t savedOffset = iPacket->m_uOffset;  // exact field name TBD

    // 2. Peek opcode without permanently consuming it from the caller's view.
    uint16_t opcode = iPacket->Decode2();

    // 3. Custom inbound range check.
    if (opcode >= 0x2000 && opcode <= 0x20FF) {
        PacketRegistry::Dispatch(opcode, iPacket);  // SEH-wrapped consumer call
        return;  // Vanilla path NOT invoked.
    }

    // 4. Restore cursor so the original sees the packet from the same state.
    iPacket->m_uOffset = savedOffset;
    ProcessPacket_Original(self, iPacket);
}
```

### Known constraints

- **Cursor restoration** is the load-bearing detail. The original
  `ProcessPacket` calls `Decode2` itself; if our peek leaves the cursor
  advanced, the original would read garbage. The exact field name and any
  side-effects of `Decode2` must be confirmed against `CInPacket` v83
  before this hook is shipped. If `CInPacket` exposes a `Peek2()` or a
  manual `Seek()` we prefer those over field-poking.
- **No allocations** in the hook hot path until after the range check
  passes. Vanilla packets must pay zero overhead beyond one branch.

---

## H2: `CWndMan::ProcessKey`

### Original signature

```cpp
long __thiscall CWndMan::ProcessKey(CWndMan* this, unsigned msg, unsigned vk, long lParam);
```

### Detour body

```cpp
long __thiscall ProcessKey_Hook(CWndMan* self, unsigned msg, unsigned vk, long lParam) {
    if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) {
        uint8_t mods = CurrentModifierMask();  // queries GetKeyState(VK_SHIFT/CTRL/MENU)
        if (auto* binding = HotkeyRegistry::Lookup(vk, mods)) {
            ToggleWindow(binding->windowHandle);
            return 1;  // Consumed; vanilla path not invoked.
        }
    }
    return ProcessKey_Original(self, msg, vk, lParam);
}
```

### Known constraints

- **Auto-repeat handling.** Holding the key fires repeated `WM_KEYDOWN`
  events. The framework must debounce so that a held hotkey doesn't toggle
  the window every frame. Implementation choice: only toggle if the
  hotkey's previous-state bit (lParam bit 30) is clear, i.e. transition
  from up to down. This is documented in the implementation, not the PRD.
- **Modifier capture.** Reading `GetKeyState` synchronously inside the
  hook is acceptable because `ProcessKey` runs on the UI thread and
  `lParam` doesn't carry the modifier state. Alternative: track modifier
  presses ourselves; rejected as overengineered.
- **Return value semantics.** The vanilla `ProcessKey` returns non-zero on
  consume; we mirror that.

---

## H3: Stage-end notification (TBD)

### Decision points (deferred to design phase)

The framework needs to know two events:

1. **Stage about to end** — capture which windows are visible, hide them.
2. **Stage now active** — re-show previously-visible windows.

Three candidate hook sites in v95:

#### Option A: `CStage` destructor

- Pros: clean "end" signal; runs unconditionally on every stage tear-down.
- Cons: doesn't tell us when the next stage is ready; need a paired hook
  on the new stage's constructor or first frame.

#### Option B: `CWvsApp::SetStage(CStage*)` or equivalent

- Pros: single call site, both old and new stage are visible.
- Cons: must verify the exact function name and signature in v95; not
  immediately obvious from a `CWvsApp` symbol scan.

#### Option C: `CClientSocket::OnMigrateCommand` (already a case in `ProcessPacket`)

- Pros: triggers on channel/cash-shop transitions specifically.
- Cons: doesn't fire for in-zone transitions (return-from-cash-shop has
  its own flow); too narrow.

### Recommendation for the design phase

Investigate Option B first via IDA. Fall back to Option A + a one-shot
"first redraw after stage change" trigger (via `CWndMan::S_Update` or
similar already-mapped function) if Option B doesn't yield a clean
single-call hook site.

---

## Call gate / original-pointer storage

Each hook stores its `*_Original` pointer in a host-DLL-global. Pattern
matches existing edits exactly:

```cpp
typedef void (__thiscall* ProcessPacket_t)(CClientSocket*, CInPacket*);
ProcessPacket_t ProcessPacket_Original = nullptr;
```

Initialization is done from `custom-ui-host`'s `MainProc` thread after the
singleton mutex check, before any consumer-DLL readiness signal.

## Uninstall

The host DLL does not provide an uninstall path — once loaded, it stays
loaded until process exit. This matches every other edit in the project.
If a future use case demands hot-unload, MinHook supports `MH_DisableHook`
and the cleanup ordering would be: unregister all consumer handlers,
disable H1+H2+H3, then unload. Not in milestone scope.
