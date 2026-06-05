# Custom UI Framework — Risks

Companion to `prd.md`. Captures stability, security, and detection risks
specific to this feature.

## R1 — Inbound packet cursor corruption

**Risk:** the `CInPacket` peek-and-restore pattern in H1 mis-restores the
cursor, leaving the original `ProcessPacket` to read mid-payload bytes as
an opcode. Result: client crash or, worse, silent state corruption.

**Mitigation:**
- Confirm `CInPacket` field layout (offset / length / buffer pointer) in
  v83 before the hook is wired in.
- Prefer a non-mutating peek primitive if one exists in `CInPacket`
  (e.g. `Peek2` or a const-method opcode read).
- Add a debug-build sanity check that the cursor offset after the hook
  equals the offset before, for opcodes outside the custom range.

## R2 — Hook ordering conflict with other edits

**Risk:** another edit (current: `bypass`, `redirect`, `no-patcher`)
installs its own detour on `CClientSocket::ProcessPacket` or
`CWndMan::ProcessKey` and the load order with `custom-ui-host` produces
a broken trampoline chain.

**Mitigation:**
- Inventory existing edits for hooks on these two functions. Current
  audit (2026-05-22): none of the shipped edits hook either function.
- The framework's hooks are non-destructive: vanilla path always runs
  for opcodes / keys outside the framework's domain. If another edit
  later wraps the same function, MinHook's trampoline chaining handles
  the standard case; the framework's behavior is independent of order.

## R3 — Consumer-DLL callback fault propagation

**Risk:** a consumer DLL's `onClick` or packet handler segfaults; without
guarding, the AV propagates to the game thread and crashes the client.

**Mitigation:**
- Every callback dispatch is wrapped in a structured exception handler
  (`__try/__except` with `EXCEPTION_EXECUTE_HANDLER` on access-violation
  codes). On AV, the framework logs the offending callback's registered
  name and continues. The window or handler is **not** auto-unregistered
  — the consumer DLL author is expected to fix the bug.

## R4 — ABI drift across host versions

**Risk:** host DLL ships v1.1 ABI; consumer DLL was built against v1.0;
new function pointer in v1.1's export table layout shifts existing
ordinals (if exports are by ordinal) or adds new names (if by name).
Consumer breaks at `GetProcAddress` time.

**Mitigation:**
- Always export by **name**, never by ordinal. Adding a new export is
  backward-compatible.
- Semver discipline: minor version bumps for additive changes, major
  version bumps for any signature or behavior change. Consumer DLLs
  must check `CustomUI_GetAbiVersion`'s major equals their build-time
  major before issuing any other call.
- Existing exports must never change signature without a major bump.

## R5 — Detection (anti-cheat / Themida heuristics)

**Risk:** installing detours on `CClientSocket::ProcessPacket` is more
behaviorally visible than the current edits, which mostly hook USER32
APIs. A cheat-detection system could flag the trampoline.

**Mitigation:**
- This project targets emulated server stacks (atlas-ms is the
  cross-validation oracle); Nexon's anti-cheat is not in the threat
  model. If the framework is ever used against an official server,
  the host DLL should not be loaded.
- The hook itself doesn't add detection surface beyond what the
  existing `bypass` and `no-patcher` edits already incur.

## R6 — Hotkey conflict detection false negatives

**Risk:** `CFuncKeyMappedMan::FuncKeyMapped(vk)` returns "unmapped" for a
key that the game still treats as having a hard-coded function (e.g.
ESC, Enter, function keys reserved for in-game system UI not routed
through `CFuncKeyMappedMan`).

**Mitigation:**
- Document the limitation in `custom-ui-host/README.md`: conflict
  detection covers user-rebindable keys only. Consumer DLL authors
  must avoid system-level keys (ESC, Tab, Enter, arrow keys).
- Maintain a small framework-side **denylist** of always-rejected vks
  (ESC, Enter, Tab, arrow keys, mouse buttons). Documented and unit-
  tested.

## R7 — Stage transition race

**Risk:** stage transition fires mid-frame; the framework has hidden
windows but a queued draw event for one of them runs against an
already-`Unregister`-ed `CUIWnd`. Use-after-free.

**Mitigation:**
- `RegisterUIWindow` / `UnregisterUIWindow` are the game's own
  registration calls; the game's own redraw scheduler observes
  registration state. As long as we don't free the `CUIWnd` object on
  hide (we don't — Destroy is a separate operation), there's no UAF.
- Defensive: never call `delete` on a `CUIWnd` while it's registered.
  `DestroyWindow` first calls `HideWindow`, then frees.
