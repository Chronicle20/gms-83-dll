# Socket Connect Fidelity — Load Balancing & CLIENT_START_ERROR — Product Requirements Document

Version: v1
Status: Draft
Created: 2026-06-12
---

## 1. Overview

The bypass edit reimplements `CClientSocket::OnConnect` so we control the login/world
handshake and the post-connect packet sent to the server. Two behaviors of the stock
client were left as `TODO`s in `bypass/socket_hooks.cpp` and are currently broken:

1. **Connection load balancing.** When a connect attempt fails, the stock client walks
   the candidate address list (`m_ctxConnect.lAddr`) node-by-node via the `posList`
   cursor, retrying the *next* address each time. When the list is exhausted it `Close()`s
   the socket and raises a typed client exception (`CTerminateException` for a login
   socket, `CDisconnectException` otherwise) so the client surfaces the canonical
   "cannot connect" failure dialog. Our hook instead re-passes `lAddr.GetHeadPosition()`
   on every failure — it never advances the cursor, so it can never exhaust the list and
   never reaches the failure path. The eventual "failed to connect to login socket"
   logic is effectively dead.

2. **`CLIENT_START_ERROR` relay.** When the connecting socket is the login socket
   (`m_ctxConnect.bLogin`), the stock client reads the on-disk exception report
   (path from `CWvsApp::GetExceptionFileName()`), and if non-empty sends a
   `CLIENT_START_ERROR` (opcode `0x19` on GMS v83–v87) packet carrying the report bytes
   so the server can log why the previous session died. Our hook grabs the filename and
   does nothing — the packet is clobbered.

This task restores both behaviors faithfully against the v84.1 disassembly, gated so the
shared hook still compiles and behaves correctly across every configured region+version.

## 2. Goals

Primary goals:
- Restore node-by-node load balancing in the `!bSuccess` path: advance `posList` via the
  `ZList::GetNext` idiom and retry `Connect` against the current node's address.
- On address-list exhaustion, `Close()` and raise the correct typed exception:
  `RaiseTerminate(0x22000001)` when `bLogin`, else `RaiseDisconnect(0x21000001)`.
- Restore `CLIENT_START_ERROR`: read the exception report file via the client's own
  file-stream helpers, and when the report is non-empty send `COutPacket(CLIENT_START_ERROR)`
  with `Encode2(len)` + `EncodeBuffer(bytes, len)`.
- Keep the shared hook compiling and correct for GMS v83.1, v84.1, v87.1, v111.1, and
  JMS v185.1 via existing `#if` gates and the per-version memory map.

Non-goals:
- No new DLL edit — this is an in-place fix to `bypass/socket_hooks.cpp`.
- No rewrite of the handshake decoder (`decode_handshake`) or the version/patch check
  ladder (those already raise correctly via `RaiseTerminate`/`RaisePatch`).
- No change to the `BUILD_MAJOR_VERSION >= 95` `SendPacket` rewrite.
- No new INI configuration surface.

## 3. User Stories

- As a **server operator**, I want the client to send `CLIENT_START_ERROR` after a
  crash-restart so atlas-login's `StartErrorHandleFunc` receives the report and I can
  diagnose client-side failures.
- As a **player**, when the login server is unreachable I want the client to try every
  advertised address and then show the stock "cannot connect" failure rather than hanging
  or silently looping on the first address.
- As a **developer**, I want the bypass `OnConnect` to match the stock client's
  connect/failure/relay semantics so divergence bugs (infinite retry, missing server
  telemetry) stop masking real issues.

## 4. Functional Requirements

### 4.1 Load-balancing advancement (`!bSuccess`, `posList != NULL`)
- FR-1: Read the current cursor `m_ctxConnect.posList`, advance it to the next node using
  the `ZList<ZInetAddr>::GetNext(&pos)` idiom (current node returned, cursor moved to
  `node->m_pNext`), and write the advanced cursor back to `m_ctxConnect.posList`.
- FR-2: Call `CClientSocket__Connect_Addr_Hook` with the **current** (pre-advance) node's
  address — matching the stock client, which passes the saved cursor value to `Connect`.
- FR-3: Remove the current behavior that re-passes `lAddr.GetHeadPosition()` on every
  failure (the bug that prevents cursor advancement / list exhaustion).

### 4.2 List-exhaustion failure (`!bSuccess`, `posList == NULL`)
- FR-4: Call `pThis->Close()`.
- FR-5: If `m_ctxConnect.bLogin` is non-zero, `RaiseTerminate(0x22000001)`
  (stock `CTerminateException`, magic `570425345`). The call does not return.
- FR-6: Otherwise `RaiseDisconnect(0x21000001)` (stock `CDisconnectException`, magic
  `553648129`). The call does not return.
- FR-7: The existing top-of-function guard `if (!m_ctxConnect.lAddr.GetCount()) return 0;`
  is preserved unchanged.

### 4.3 `CLIENT_START_ERROR` relay (`bSuccess`, `bLogin`)
- FR-8: Obtain the report path from `CWvsApp::GetExceptionFileName()`.
- FR-9: Read the file using the client's own file-stream object (open/get-length/read/
  close — see `hook-design.md` §3), capping the read at the stock limit: a report is only
  sent when `0 < length < 0x2000`.
- FR-10: When the report buffer is present and its length is non-zero, construct
  `COutPacket(CLIENT_START_ERROR)`, `Encode2(length)`, `EncodeBuffer(reportBytes, length)`,
  and `CClientSocket::GetInstance()->SendPacket(&pkt)`.
- FR-11: When the file is absent/empty or length is outside `(0, 0x2000)`, send nothing
  (matches stock `if (buf && len)` guard). No exception is raised in this case.
- FR-12: Release any buffer/stream resources before returning (the stock code destructs
  the stream object and the report `ZArray`).

### 4.4 Wire-format parity
- FR-13: The emitted packet must decode cleanly against atlas-login `serverbound.StartError`:
  `ReadUint16 length` then `ReadBytes(length)`. `Encode2` (little-endian u16) + `EncodeBuffer`
  satisfies this.

## 5. Hook / Patch Surface

This task adds no new Detours hooks. It rewrites two regions **inside** the existing
`CClientSocket__OnConnect_Hook` in `bypass/socket_hooks.cpp`. It does add calls to existing
client routines (load-balancing uses only our own `ZList` + exception helpers; the report
read calls into the client's file-stream helpers).

Reference call site (GMS v84.1, `CClientSocket::OnConnect` @ `0x00499DCD`):

| Behavior | Stock site (v84.1) | Notes |
|---|---|---|
| Cursor advance | `node = posList; posList = node->m_pNext; Connect(this, node)` | `posList-16+4` → `m_pNext`, `+16` → `T*` = `ZList::GetNext` |
| Exhaustion terminate | throw `CTerminateException(0x22000001)` (`570425345`) | login socket |
| Exhaustion disconnect | throw `CDisconnectException(0x21000001)` (`553648129`) | non-login socket |
| Report open | `sub_49A615` (`__thiscall CFileStream::Open`, this=ecx) | wraps `CreateFileA`, handle → `[this+0x10]`, `GENERIC_READ 0x80000000` |
| Report length | `sub_49A79E` | returns file size |
| Report buffer resize | `sub_49BBBE` | `ZArray` set-size into report buffer |
| Report read | `sub_49A8C9` | reads `length` bytes into buffer |
| Report close/dtor | `sub_49A5B7` | also reachable as the stream's `Open` preamble |
| Stream vtable | `off_B437BC` | stack object; `vtable` at `[obj+0]` |
| Send | `COutPacket(0x19)` → `Encode2(len)` → `EncodeBuffer(buf,len)` → `SendPacket` | opcode = `CLIENT_START_ERROR` map value |

Per-version addresses for the file-stream helper set + vtable + object size are tabulated
in `memory-map.md`; v83/v87/v111/JMS values must be resolved during implementation.

## 6. Configuration

None. No INI keys are added or changed.

## 7. Memory Map Impact

New per-version `memory_maps/<REGION>/v<MAJOR>_<MINOR>.cmake` entries are required for the
client file-stream helper set, mapped into `include/memory_map.h.in`:

- `C_FILE_STREAM_OPEN` (v84.1 = `0x0049A615`)
- `C_FILE_STREAM_GET_LENGTH` (v84.1 = `0x0049A79E`)
- `C_FILE_STREAM_READ` (v84.1 = `0x0049A8C9`)
- `C_FILE_STREAM_CLOSE` (v84.1 = `0x0049A5B7`)
- `C_FILE_STREAM_VFTABLE` (v84.1 = `0x00B437BC`)
- `C_ZARRAY_BYTE_SETSIZE` (v84.1 = `0x0049BBBE`) — report-buffer resize helper

`CLIENT_START_ERROR` (`0x19` GMS v83–v87, `0x2F` v111), `C_WVS_APP_GET_EXCEPTION_FILE_NAME`,
and the disconnect/terminate `_ThrowInfo` entries already exist. Names/exact offsets are
finalized in `memory-map.md`. If implementation finds the stream object is impractical to
reconstruct on some version, that version's row documents the gap (it must still compile).

## 8. Non-Functional Requirements

- **Stability:** the exhaustion path raises typed client exceptions that unwind into the
  stock WinMain handler (same mechanism as `client_exception.cpp`); it must not return into
  caller code. The report read must tolerate a missing/locked/zero-byte file without
  crashing or raising (stock simply sends nothing).
- **No behavioral regression** for the happy path (successful handshake → `PLAYER_LOGGED_IN`
  for the game socket) or the version/patch check ladder.
- **AV/Themida compatibility:** no new inline patches; we only call existing client routines
  by mapped address, consistent with the rest of the bypass.
- **Resource hygiene:** the report buffer and stream object must be released on every exit
  path from the `bLogin` branch (matches stock destructor sequencing).

## 9. Open Questions

- OQ-1: Exact stack footprint / field layout of the client file-stream object beyond
  `vtable@[0]` and `handle@[+0x10]` (v84.1). Resolve from disassembly during implementation;
  size a local buffer conservatively (as `RaisePatch` does for `CPatchException`).
- OQ-2: Whether `sub_49BBBE` is a generic `ZArray<char>::SetSize` we already model in
  `common/ZArray.h` (preferred — reuse the template) or must be called by address.
- OQ-3: Confirm `CLIENT_START_ERROR` opcode and the file-stream helper addresses for
  v83.1, v87.1, v111.1, and JMS v185.1.

## 10. Acceptance Criteria

- [ ] `bypass/socket_hooks.cpp` `!bSuccess` path advances `posList` via `GetNext` and
      retries the current node's address; the `GetHeadPosition()` re-pass is removed.
- [ ] On `posList == NULL`, the hook `Close()`s then `RaiseTerminate(0x22000001)` (login)
      or `RaiseDisconnect(0x21000001)` (non-login); neither returns.
- [ ] The `bLogin` success branch reads the exception report and, when `0 < len < 0x2000`,
      sends `COutPacket(CLIENT_START_ERROR)` + `Encode2(len)` + `EncodeBuffer(buf,len)`.
- [ ] Missing/empty report → no packet, no exception.
- [ ] Emitted packet decodes under atlas-login `serverbound.StartError` (`u16 len` + bytes).
- [ ] New `memory_maps` entries added for every region+version; `include/memory_map.h.in`
      updated.
- [ ] `scripts/wsl-build.sh` compiles and links all DLLs for **GMS v83.1, v84.1, v87.1,
      v111.1, and JMS v185.1** with no new warnings in the touched TU.
- [ ] Runtime check on GMS v84.1 (loaded IDB): forcing a bad login address exhausts the
      list and shows the stock failure; a seeded exception report produces a
      `CLIENT_START_ERROR` observed by atlas-login.
