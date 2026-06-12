# Build Matrix — Socket Connect Fidelity (task-007)

Full acceptance matrix via `scripts/wsl-build.sh <REGION> <MAJOR> <MINOR>` (clang-cl + xwin,
WSL). Each row built with a clean `build-wsl/` dir. All five exit 0 with the proxy + every
edit DLL linked.

| Region/Version | Build | `C_FILE_STREAM_RESOLVED` | Relay path compiled | Notes |
|---|---|---|---|---|
| GMS 83 1 | ✅ exit 0 | 0 | `#else` Log-only no-op | OnConnect CFG-obfuscated → gated off |
| GMS 84 1 | ✅ exit 0 | 1 | resolved relay | anchor version; addresses audited |
| GMS 87 1 | ✅ exit 0 | 1 | resolved relay | addresses audited |
| GMS 111 1 | ✅ exit 0 | 0 | `#else` Log-only no-op | redesigned non-vtable stream class → gated off |
| JMS 185 1 | ✅ exit 0 | 1 | resolved relay | major 185 ≥ 95 → also exercises the gated SendPacket-hook typedef; opcode corrected 0x15→0x0F |

## New-warning scan (touched files only)

`grep -iE "warning|error"` over all five build logs, filtered to `socket_hooks.cpp` /
`ZArray.h`: **no warnings on any edited region.** The only `socket_hooks.cpp` warnings present
are pre-existing and live in the hook-install/Detours section (lines ~319, 363–376):
`WSAAsyncSelect` deprecation and `-Wmicrosoft-cast` (function-pointer ↔ object-pointer) — both
predate task-007 and were not introduced or moved by this work. `common/ZArray.h` produced no
warnings in any build.

## Mapping to the task's edits

- `common/ZArray.h` — `SetSize`/`GetData` (Task 1): clean in all 5.
- `bypass/socket_hooks.cpp` `!bSuccess` load-balancing (Task 2): clean, unconditional, all 5.
- `bypass/socket_hooks.cpp` `ClientFileStream` mirror + relay (Task 5): compiles the resolved
  path on GMS 84/87 + JMS 185, the `#else` no-op on GMS 83/111.
- `memory_maps/*` + `include/memory_map.h.in` (Task 4): every version resolves all six
  `C_FILE_STREAM_*` keys (GenerateMemoryMap validation passes — a missing/empty key is a
  configure-time FATAL_ERROR, so a clean configure is itself the proof).
