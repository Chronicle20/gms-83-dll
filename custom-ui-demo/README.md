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
