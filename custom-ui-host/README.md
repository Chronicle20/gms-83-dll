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
