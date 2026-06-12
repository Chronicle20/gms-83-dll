# Memory Map — Socket Connect Fidelity

New per-version entries needed for the `CLIENT_START_ERROR` report read. The load-balancing
fix needs **no** new map entries (it uses our `ZList` + the existing exception `_ThrowInfo`
map values). Add each symbol to `memory_maps/<REGION>/v<MAJOR>_<MINOR>.cmake` and surface it
in `include/memory_map.h.in`.

## Already present (verify, do not re-add)
| Symbol | Where | Notes |
|---|---|---|
| `CLIENT_START_ERROR` | `memory_maps/GMS/v84_1.cmake:10` etc. | `0x19` GMS v83–v87, `0x2F` v111, JMS v185 = confirm |
| `C_WVS_APP_GET_EXCEPTION_FILE_NAME` | `memory_maps/GMS/v84_1.cmake:166` | report path |
| `C_TI_TERMINATE_EXCEPTION` | exception map | used by `RaiseTerminate(0x22000001)` |
| `C_TI_DISCONNECT_EXCEPTION` | exception map | used by `RaiseDisconnect(0x21000001)` |
| `C_CLIENT_SOCKET_ON_CONNECT` | `:29` | hook target |

## New entries (file-stream helper set)
Resolve per region+version. `off_B437BC` is the stream object's vtable; the four method
addresses are reachable through it (slots in order: `[0]=Open-preamble/Close`, `[+4]=Read`,
…), but call them by their flat addresses for clarity.

| Symbol | Meaning | GMS v84.1 | v83.1 | v87.1 | v111.1 | JMS v185.1 |
|---|---|---|---|---|---|---|
| `C_FILE_STREAM_OPEN` | `__thiscall Open(name,access,share,…)` → `CreateFileA` | `0x0049A615` | N/A | `0x004A7710` | N/A | `0x004B0864` |
| `C_FILE_STREAM_GET_LENGTH` | `__thiscall GetLength()` → size | `0x0049A79E` | N/A | `0x004A7899` | N/A | `0x004B09ED` |
| `C_FILE_STREAM_READ` | `__thiscall Read(dst,len)` | `0x0049A8C9` | N/A | `0x004A79C4` | N/A | `0x004B0B18` |
| `C_FILE_STREAM_CLOSE` | `__thiscall Close()` / dtor | `0x0049A5B7` | N/A | `0x004A76B2` | N/A | `0x004B0806` |
| `C_FILE_STREAM_VFTABLE` | stream object vtable | `0x00B437BC` | N/A | `0x00B95EA4` | N/A | `0x00BE4CAC` |
| `C_ZARRAY_BYTE_SETSIZE` | report buffer resize (`sub_49BBBE`) | resolved-via-template (OQ-2) | — | — | — | — |

### Per-version audit anchors (reproducibility)
Each row resolved by decompiling that version's `CClientSocket::OnConnect`
(`C_CLIENT_SOCKET_ON_CONNECT`), locating the `bLogin` branch (the
`if (m_ctxConnect.bLogin)` arm that calls `CWvsApp::GetExceptionFileName`), and reading the
four helpers in call order. `select_instance` + `server_health` confirmed each IDB before
recording.

| Version | Port | `server_health` module | `OnConnect` (cmake `C_CLIENT_SOCKET_ON_CONNECT`) | report opcode pushed to `COutPacket` ctor | cmake `CLIENT_START_ERROR` |
|---|---|---|---|---|---|
| GMS v84.1 | 13341 | `GMS_v84.1_U_DEVM.exe` | `0x00499DCD` | `0x19` (push 19h @ 0x49A2A0) | `0x19` ✅ |
| GMS v83.1 | 13337 | `MapleStory_dump.exe` | `0x00494ED1` | (unrecoverable — CFG-obfuscated) | `0x19` |
| GMS v87.1 | 13338 | `GMSv87_4GB.exe` | `0x004A6E5A` | `0x19` (push 19h @ 0x4A735A) | `0x19` ✅ |
| GMS v111.1 | 13342 | `MapleStory v111.5_dump_SCY.exe` | `0x004DA820` | `0x2F` (push 2Fh @ 0x4DACF2) | `0x2F` ✅ |
| JMS v185.1 | 13340 | `MapleStory_dump_SCY.exe` | `0x004B0066` | **`0x0F`** (push 0Fh @ 0x4B04B4) | **`0x15`** ⚠️ MISMATCH |

> ⚠️ **JMS v185.1 opcode discrepancy (flagged loudly).** The cmake declares
> `CLIENT_START_ERROR 0x15` (`memory_maps/JMS/v185_1.cmake:4`), but the bLogin error-report
> `COutPacket` ctor in `OnConnect` is unambiguously `push 0Fh` (verified at raw
> `0x004B04B4`). The non-login start packet in the same function is `push 7` (0x07). So in
> JMS185 the start-error report opcode is **0x0F**, not 0x15. This does not block the
> file-stream resolution (helpers are fully resolved), but Task 4/Task 5 must NOT trust the
> existing `0x15` for the JMS report relay — either re-derive the opcode or carry a separate
> `CLIENT_START_ERROR` value for JMS185. Recommend a follow-up to reconcile `v185_1.cmake:4`.

### N/A rows — reasons (no guessing; `0x0` placeholders + RESOLVED 0 in Task 4)
- **v83.1 — N/A (OnConnect CFG-obfuscated).** `OnConnect @ 0x00494ED1` is a faithful body
  (same `0xFAC` frame, same `var_A0` stream object, same `var_40/var_70` scratch as v84) but
  it is wrapped by a control-flow obfuscator: the first instructions `jmp loc_D102F3` into a
  relocated/flattened body in the `0x00C22xxx`/`0x00D102xx` region, full of
  `_InterlockedExchange` opaque predicates and `JUMPOUT`s. The linear Open→GetLength→Read→
  Close call sequence is not statically recoverable, and `CWvsApp::GetExceptionFileName`
  (`0x009F9808`) has **no xref into OnConnect** (only `WriteClientLog`, `save_error_log`,
  `_WinMain`). Cannot resolve the helpers without guessing → N/A.
- **v111.1 — N/A (different stream class; no vtable `__thiscall Open`).** The bLogin branch
  (`if (v16[9])`) does NOT use the v84-style vtable stream object. "Open" is the **raw
  `CreateFileA` import thunk** `dword_100F86C(name, 0x80000000, 1, 0, 3, 128, 0)` (handle
  stored at `v55`, throws `ZException` if `== -1`), not a `__thiscall Open(name,access,…)`
  method, and no `off_xxx` vtable is assigned to the stream object. The surrounding helpers
  are a redesigned ZFileStream (`sub_4D86B0` ctor, `sub_4D8D80` open/close,
  `sub_4D8300` GetLength, `sub_4D8330` Read). The `C_FILE_STREAM_*` symbol set is defined
  for the vtable-object model, which v111 does not have → N/A for this symbol set.
  (For the audit only, the v111 analogs are: ctor `0x004D86B0`, open/close `0x004D8D80`,
  GetLength `0x004D8300`, Read `0x004D8330`, CreateFile thunk `0x0100F86C`. These are NOT
  recorded as `C_FILE_STREAM_*` because the shape differs; Task 5's `ClientFileStream` cannot
  faithfully reconstruct them.) COutPacket report opcode `0x2F` confirmed.

## RESOLVED flags (drives Task 4 cmake plumbing)
For each version, `C_FILE_STREAM_RESOLVED = 1` means the four helpers + vtable were verified
and will be emitted to that version's cmake; `= 0` means the relay is gated off (helpers are
`0x0` placeholders in Task 4).

| Version | `C_FILE_STREAM_RESOLVED` | Reason if 0 |
|---|---|---|
| GMS v84.1 | `1` | — (anchor verified) |
| GMS v83.1 | `0` | OnConnect CFG-obfuscated; helpers unrecoverable (see N/A) |
| GMS v87.1 | `1` | — (symbols present, vtable `off_B95EA4`) |
| GMS v111.1 | `0` | different stream class — no vtable `__thiscall Open` (see N/A) |
| JMS v185.1 | `1` | — (vtable `off_BE4CAC`; ⚠️ but report opcode is 0x0F not 0x15) |

## OQ-1 — `ClientFileStream` object size
Settled on v84.1 (active IDB `GMS_v84.1_U_DEVM.exe`). Stream object = stack var `v34`
(`_DWORD[12]` @ `ebp-A0h`), vtable `off_B437BC` stored at `[+0]`. Highest byte offset
**written** by any of the four methods:

| Method | addr | writes (this-relative offsets) |
|---|---|---|
| Open `sub_49A615` | `0x49A615` | `[+8]=0`, `[+0xC]=0`, `[+0x10]=handle`, `[+0x34]` dword (`or [esi+34h],1`; optional `or …,2`) |
| GetLength `sub_49A79E` | `0x49A79E` | none (calls vtable `[*this+0x3C]`, reads position only) |
| Read `sub_49A8C9` | `0x49A8C9` | `[+8]/[+0xC]` 64-bit file position (reads `[+0x18],[+0x20],[+0x28]`; Seek helper `sub_49A74B` writes nothing in the object) |
| Close `sub_49A5B7` | `0x49A5B7` | `[+0x10]=-1`, `[+0x34]` dword (`and [esi+34h],0`) |

- **Highest offset written:** the **dword at `[+0x34]`** → bytes `0x34..0x37`, i.e. **highest
  written byte = `0x37`**.
- **Minimum object size:** **`0x38`** (`0x34` dword + 4).
- **Recommended for Task 5's `ClientFileStream`:** total **`0x40`**, layout
  `{ void* vtbl @ +0; ... ; pad to 0x40 }` → **`pad[0x3C]`** after the vtable pointer
  (`0x40 − 4`). This is the conservative recommendation; the strict minimum is `0x38`
  (`pad[0x34]`). No method writes above `[+0x37]`, so `0x40` is safe with headroom.

## OQ-throw — missing-file behavior → decision **(b)**
Settled on v84.1 from the Open disassembly (`sub_49A615 @ 0x0049A615`):

```
0x49A650  call dword_C49A54          ; CreateFileA wrapper (GENERIC_READ)
0x49A656  cmp  eax, 0FFFFFFFFh        ; INVALID_HANDLE_VALUE ?
0x49A659  mov  [esi+10h], eax         ; store handle
0x49A65C  jnz  short loc_49A675       ; ok
0x49A65E  call dword_C49AA4           ; GetLastError
0x49A664  mov  [ebp+pExceptionObject], eax
0x49A670  call __CxxThrowException@8  ; throw ZException  ← on missing file
```

`C_FILE_STREAM_OPEN` **DOES throw `ZException` when `CreateFileA` returns
`INVALID_HANDLE_VALUE`** — i.e. on a routine missing report file. (It also throws earlier if
the pre-emptive `Close`/`sub_49A5B7` preamble fails, but the missing-file case is the
CreateFile `== -1` path above.) In `OnConnect`'s bLogin branch the `Open`→`GetLength`→`Read`→
`Close` sequence is **NOT wrapped in its own try/catch** that swallows this — the only
enclosing handler is the function-level EH unwind frame (`v53`/`v62` state machine), so a
throw here **escapes** `OnConnect` and unwinds up the stack. This contradicts the requirement
"a routine missing/empty report must produce NO packet and NO exception/crash."

- **Decision: (b).** Task 5 MUST pre-check existence before opening so a normal missing report
  never throws.
- **Required guard (exact):** before constructing/opening the `ClientFileStream`, skip the
  entire relay when the report path does not exist —
  `if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES) return;`
  (equivalently `PathFileExistsA(path)` / a `CreateFileA(...OPEN_EXISTING...)` probe). Only
  call `Open` when the file is known to exist. The existing `(0 < len < 0x2000)` guard on
  `GetLength` handles the empty/oversized cases but does **not** cover the missing-file throw,
  which happens earlier inside `Open`. The pre-existence check is mandatory.

> OQ-throw applies identically to v87.1 and JMS185.1 (same vtable-Open model — Open throws on
> `-1`). v111.1's inlined variant also throws on `CreateFileA == -1` (raw thunk path), so the
> same pre-existence guard is correct there too, even though that version's relay is gated off.

## OQ-2 — `C_ZARRAY_BYTE_SETSIZE` resolved-via-template (NOT a memory-map symbol)
The v84 report-buffer resize (`sub_49BBBE`, the `ZArray<unsigned char>::_Alloc`/grow used as
`v22 = sub_49BBBE(len, &namelen[3])` in OnConnect) is now modeled in `common/ZArray.h` via the
`Alloc`/`SetSize`/`GetData` template (Task 1). It is therefore **NOT added to the cmake files**
and **NOT surfaced in `memory_map.h.in`** — the C++ template reproduces the behavior. The
`C_ZARRAY_BYTE_SETSIZE` row above is kept only for traceability and marked
`resolved-via-template (OQ-2)`; Task 4 should not plumb it.

## Decisions for Task 4 / Task 5 (post-audit, 2026-06-12)

An independent IDA + atlas-ms audit re-derived every RESOLVED-1 address (v87.1, JMS185.1)
and re-confirmed OQ-1 / OQ-throw from v84.1. All addresses, the `0x38` min size, and the
`(b)` throw decision are **CONFIRMED**. The audit further resolved the JMS opcode:

1. **JMS v185.1 `CLIENT_START_ERROR` → change `0x15` to `0x0F`.** The JMS client's bLogin
   report `COutPacket` ctor is unambiguously `push 0Fh` (raw `0x004B04B4`); `0x15` has no
   basis on either wire end (atlas-login's JMS v185 tenant template registers *no* StartError
   handler at `0x0F` or `0x15`). For client fidelity (the goal of task-007) the relay must
   send what the stock client sends. **Task 4 must set `CLIENT_START_ERROR 0x0F` in
   `memory_maps/JMS/v185_1.cmake`** (was `0x15`), with a comment citing this evidence.
   - *Out of scope (flagged for the record):* atlas-login JMS v185 has no `StartErrorHandle`
     binding, so the server currently ignores this packet. That is a server-side gap to fix
     in atlas-ms separately; it does not change the client-fidelity requirement here.

2. **Task 5 needs the missing-file guard (OQ-throw (b)).** Before opening the stream, skip
   the whole relay when the report path does not exist:
   `if (GetFileAttributesA(fileName) == INVALID_FILE_ATTRIBUTES) { /* no report */ }`
   — only call `Open` when the file exists. Applies to every RESOLVED-1 version
   (v84.1/v87.1/JMS185.1); the `(0 < len < 0x2000)` GetLength guard does NOT cover the throw.

3. **Task 5 `ClientFileStream` size:** total `0x40` → `{ void* vftable; char pad[0x3C]; }`
   (min `0x38`, conservative `0x40`).

4. **N/A versions confirmed defensible:** v83.1 (genuine CFG flattening), v111.1 (redesigned
   non-vtable stream class). Both → `C_FILE_STREAM_RESOLVED 0` + `0x0` placeholders in Task 4.

## Resolution recipe (per IDB)
1. `select_instance(port)` then `server_health` to confirm the loaded IDB is the intended version.
2. Decompile the version's `CClientSocket::OnConnect`; find the `bLogin` branch.
3. The call right after `GetExceptionFileName()` with `0x80000000` (GENERIC_READ) is
   `C_FILE_STREAM_OPEN`; the vtable assigned to the stack object just before it is
   `C_FILE_STREAM_VFTABLE`. The subsequent length / resize / read / close calls map to the
   remaining symbols in order.
4. Confirm the `COutPacket` ctor argument equals the version's `CLIENT_START_ERROR` opcode.
5. If helpers are inlined / the stream object is impractical to reconstruct, mark the version
   `N/A` with a one-line reason (`0x0` placeholders + `C_FILE_STREAM_RESOLVED 0` in Task 4).
   Never invent an address.
