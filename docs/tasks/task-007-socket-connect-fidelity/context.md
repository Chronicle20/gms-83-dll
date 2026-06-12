# Context — Socket Connect Fidelity (task-007)

Quick reference for agents executing `plan.md`. Source of truth is `prd.md`, `design.md`,
`hook-design.md`, `memory-map.md`.

## What's being changed

Two in-place rewrites inside one hook — `CClientSocket__OnConnect_Hook` in
`bypass/socket_hooks.cpp`. No new Detours installs, no new DLL edit, no INI surface.

1. **Load balancing** (`!bSuccess` block, `socket_hooks.cpp:113-126`) — advance the
   `posList` cursor via `ZList<ZInetAddr>::GetNext`, retry the current node; on exhaustion
   `Close()` + `RaiseTerminate(0x22000001)` (login) / `RaiseDisconnect(0x21000001)`.
2. **CLIENT_START_ERROR relay** (`bSuccess && bLogin` block, `socket_hooks.cpp:210-213`) —
   read the exception report via the stock `CFileStream` helpers into a `ZArray<char>`,
   send `COutPacket(CLIENT_START_ERROR)` + `Encode2(len)` + `EncodeBuffer(buf,len)`.

## Key files

| File | Role |
|---|---|
| `bypass/socket_hooks.cpp` | the hook; both rewrites + `ClientFileStream` mirror + fn-ptr typedefs |
| `common/ZArray.h` | add `SetSize(size_t)` / `GetData()` (report buffer) |
| `common/ZList.h` | `GetNext(T** pos)` @ `:333` — the cursor-advance idiom (already exists) |
| `common/ZInetAddr.h` | `struct ZInetAddr : sockaddr_in` — base-cast to `const sockaddr_in*` |
| `common/CClientSocket.h` | `CONNECTCONTEXT { ZList<ZInetAddr> lAddr; __POSITION* posList; int bLogin; }` |
| `common/COutPacket.h` | `COutPacket(INT)`, `Encode2(unsigned short)`, `EncodeBuffer(const void*, unsigned)` |
| `common/CWvsApp.cpp:92` | `CWvsApp::GetExceptionFileName()` — `static char* __cdecl`, returns report path |
| `bypass/client_exception.h` | `[[noreturn]] RaiseTerminate(int)` / `RaiseDisconnect(int)` (impl in `.cpp`) |
| `include/memory_map.h.in` | add 6 `@KEY@` defines (template) |
| `memory_maps/{GMS,JMS}/v*_*.cmake` | per-version `set()` for the 6 keys |
| `cmake/GenerateMemoryMap.cmake` | validates every `@KEY@` is defined + non-empty per version |

## Region/version matrix (acceptance)

`GMS 83 1`, `GMS 84 1`, `GMS 87 1`, `GMS 111 1`, `JMS 185 1`.
Build: `scripts/wsl-build.sh <REGION> <MAJOR> <MINOR>` (clang-cl + xwin; exit 0 = clean).

## Hook addresses (GMS v84.1, loaded IDB `GMS_v84.1_U_DEVM`)

`CClientSocket::OnConnect` @ `0x00499DCD`. File-stream helper set (known v84.1):

| Symbol | v84.1 | Meaning |
|---|---|---|
| `C_FILE_STREAM_OPEN` | `0x0049A615` | `__thiscall Open(name,access,share,…)` → `CreateFileA`; handle → `[this+0x10]` |
| `C_FILE_STREAM_GET_LENGTH` | `0x0049A79E` | `__thiscall GetLength()` |
| `C_FILE_STREAM_READ` | `0x0049A8C9` | `__thiscall Read(dst,len)` |
| `C_FILE_STREAM_CLOSE` | `0x0049A5B7` | `__thiscall Close()`/dtor |
| `C_FILE_STREAM_VFTABLE` | `0x00B437BC` | stream object vtable |

v83.1 / v87.1 / v111.1 / JMS v185.1 = **resolve in Task 3** (recipe in `memory-map.md`).
Already mapped (do not re-add): `CLIENT_START_ERROR` (`0x19` v83/84/87, `0x2F` v111, `0x15`
JMS185), `C_WVS_APP_GET_EXCEPTION_FILE_NAME`, `C_TI_TERMINATE_EXCEPTION`,
`C_TI_DISCONNECT_EXCEPTION`, `C_CLIENT_SOCKET_ON_CONNECT`.

## Magic numbers

| Hex | Decimal | Exception |
|---|---|---|
| `0x22000001` | 570425345 | `CTerminateException` (login exhaustion) |
| `0x21000001` | 553648129 | `CDisconnectException` (non-login exhaustion) |
| `0x22000007` | 570425351 | `CTerminateException` (version/patch ladder — already handled, unchanged) |

## Open questions resolved in Task 3 (disassembly, not preference)

- **OQ-1 object size:** `ClientFileStream` total must cover the highest offset the four
  methods write. `Open` writes a dword at `[+0x34]` → object ≥ `0x38`. **Design's `pad[0x30]`
  (total `0x34`) is too small by 4 bytes.** Use `pad[0x3C]` (total `0x40`) unless Task 3 finds
  a larger footprint; size `pad = total − 4`.
- **OQ-throw missing-file:** confirm whether `Open` throws on `INVALID_HANDLE_VALUE` for a
  routine missing report and whether `OnConnect` catches it. FR-11 requires missing/empty →
  no packet, no exception. Guard the read path only if the disassembly shows `Open` can throw
  on a normal missing file.
- **OQ-2 (resolved):** model `SetSize`/`GetData` in `ZArray.h`; no `C_ZARRAY_BYTE_SETSIZE`
  symbol.

## Gating mechanism

New `C_FILE_STREAM_RESOLVED` (1/0) per version. The relay body is wrapped in
`#if C_FILE_STREAM_RESOLVED … #else Log-only no-op #endif`. A version whose helpers can't be
resolved gets `0x0` placeholder addresses + `C_FILE_STREAM_RESOLVED 0` so the build stays
green and the placeholder is never called. Load-balancing (Task 2) is unconditional — it
needs no new symbols.

## Must NOT change (hook-design.md §3)

- Top guard `if (!pThis->m_ctxConnect.lAddr.GetCount()) return 0;` (`socket_hooks.cpp:110-112`).
- `decode_handshake` and the version/patch ladder (`RaiseTerminate(0x22000007)`/`RaisePatch()`).
- The non-login `PLAYER_LOGGED_IN` send branch (`socket_hooks.cpp:214-238`).
- The `BUILD_MAJOR_VERSION >= 95` `SendPacket` rewrite + its `#if` gate.

## Wire format (atlas-login parity)

`serverbound.StartError.Decode`: `length = ReadUint16(); bytes = ReadBytes(length)`.
`Encode2` = little-endian u16, `EncodeBuffer` = raw bytes → exact match. Handled by
`StartErrorHandleFunc` in `atlas/services/atlas-login/atlas.com/login/socket/handler/start_error.go`.

## Dependencies / ordering

Task 0 (branch) → 1 (ZArray, independent) → 2 (load-balancing, independent) →
3 (IDA resolve, gates 4+5) → 4 (memory-map plumbing) → 5 (relay, needs 1+3+4) →
6 (build matrix) → 7 (runtime). Commit after each task; never commit to `main`.
