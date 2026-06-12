# Design — Socket Connect Fidelity (Load Balancing & CLIENT_START_ERROR)

Status: Draft for review
Created: 2026-06-12
Inputs: `prd.md`, `hook-design.md`, `memory-map.md` (same folder)

---

## 1. Scope & Decisions

This task rewrites two regions **inside** the existing `CClientSocket__OnConnect_Hook`
in `bypass/socket_hooks.cpp`. No new Detours installs, no new DLL edit, no INI surface.

Two design forks were resolved during brainstorming:

| Fork | Decision | Consequence |
|---|---|---|
| How to read the exception report file | **Reconstruct the client `CFileStream` stack object and drive `Open/GetLength/Read/Close` by mapped address** (faithful to stock; matches this repo's RE-fidelity ethos) | Adds 5 new per-version memory-map symbols; OQ-1 (object size) resolved from disassembly |
| How to model the report buffer | **Add `SetSize`/`GetData` to `common/ZArray.h`** and read into a `ZArray<char>` as the stock does | No `C_ZARRAY_BYTE_SETSIZE` symbol needed (modeled in the template); small shared-header surface added |

The **load-balancing** rewrite needs no new map symbols — it uses our existing
`ZList<ZInetAddr>::GetNext` and the existing `RaiseTerminate`/`RaiseDisconnect` helpers.

Everything in PRD §"Things that must NOT change" / `hook-design.md` §3 is preserved:
the top guard `if (!lAddr.GetCount()) return 0;`, the handshake decode, the version/patch
ladder, the non-login `PLAYER_LOGGED_IN` branch, and the `BUILD_MAJOR_VERSION >= 95`
`SendPacket` rewrite + its `#if` gate.

---

## 2. Component A — Load-balancing rewrite (`!bSuccess`)

Replaces the current dead code at `socket_hooks.cpp:113-126` (the two `Log(...) return 0`
stubs and the `GetHeadPosition()` re-pass marked `// TODO do i really care...`).

### Behavior

```cpp
if (!bSuccess) {
    if (!pThis->m_ctxConnect.posList) {            // list exhausted
        pThis->Close();
        if (pThis->m_ctxConnect.bLogin) {
            Log("CClientSocket::OnConnect connect failed (login) -> RaiseTerminate(0x22000001)");
            RaiseTerminate(0x22000001);            // [[noreturn]] CTerminateException, magic 570425345
        }
        Log("CClientSocket::OnConnect connect failed -> RaiseDisconnect(0x21000001)");
        RaiseDisconnect(0x21000001);               // [[noreturn]] CDisconnectException, magic 553648129
    }

    // Advance the cursor; retry the *current* (pre-advance) node's address.
    ZInetAddr* pos = reinterpret_cast<ZInetAddr*>(pThis->m_ctxConnect.posList);
    ZInetAddr* current = pThis->m_ctxConnect.lAddr.GetNext(&pos);
    pThis->m_ctxConnect.posList = reinterpret_cast<__POSITION*>(pos);
    CClientSocket__Connect_Addr_Hook(pThis, edx, current);   // ZInetAddr : sockaddr_in (base cast)
    return 0;
}
```

### Why this is correct / low-risk

- `ZList<ZInetAddr>::GetNext(T** pos)` already exists (`common/ZList.h`) and implements the
  exact stock idiom: it returns the current node and advances `*pos` to `node->m_pNext`
  (via `CastNode`, which subtracts 16 — matching the disassembly's `posList-16+4` → `m_pNext`,
  `+16` → wrapped `ZInetAddr*`). We do **not** re-derive the pointer math by hand.
- `ZInetAddr` is declared `struct ZInetAddr : sockaddr_in`, so passing a node where the hook
  expects `const sockaddr_in*` is a plain base-class conversion — no `reinterpret_cast` on the
  argument (mirrors `Connect_Ctx_Hook`, which passes `&pThis->m_addr`).
- The `posList ↔ __POSITION*` casts mirror the existing
  `reinterpret_cast<__POSITION*>(lAddr.GetHeadPosition())` already in `Connect_Ctx_Hook`.
- `RaiseTerminate`/`RaiseDisconnect` are `[[noreturn]]` (`bypass/client_exception.cpp`) and
  unwind into the stock WinMain handler via the client's own `_ThrowInfo`. **No trailing
  `return`** after them — they do not return, and the exhaustion branch therefore cannot fall
  through into the success path below.

### Magic-number cross-check

| Constant | Hex | Exception |
|---|---|---|
| 570425345 | `0x22000001` | `CTerminateException` (login exhaustion) |
| 553648129 | `0x21000001` | `CDisconnectException` (non-login exhaustion) |
| 570425351 | `0x22000007` | `CTerminateException` (version mismatch — already handled, unchanged) |

---

## 3. Component B — `CLIENT_START_ERROR` relay (`bSuccess && bLogin`)

Replaces the `bLogin` stub at `socket_hooks.cpp:210-213` (the `// TODO relay
CLIENT_START_ERROR` and the dangling `GetExceptionFileName()` whose result is dropped). The
non-login `else` branch (`PLAYER_LOGGED_IN`) is unchanged.

### Data flow

```
CWvsApp::GetExceptionFileName()  ->  fileName
        │
        ▼
ClientFileStream fs;  fs.vftable = C_FILE_STREAM_VFTABLE
        │  Open(fileName, GENERIC_READ, share=128, 1, 0, 0, 0)   [C_FILE_STREAM_OPEN]
        ▼
len = GetLength()                                                [C_FILE_STREAM_GET_LENGTH]
        │  guard: 0 < len < 0x2000
        ▼
report.SetSize(len) -> dst ;  Read(dst, len)                    [ZArray<char> ; C_FILE_STREAM_READ]
        │
        ▼
Close()                                                          [C_FILE_STREAM_CLOSE]
        │  if report.GetCount():
        ▼
COutPacket(CLIENT_START_ERROR) . Encode2(len) . EncodeBuffer(report.GetData(), len)
        │
        ▼
CClientSocket::GetInstance()->SendPacket(&pkt)
```

### Implementation

```cpp
if (pThis->m_ctxConnect.bLogin) {
    Log("CClientSocket::OnConnect relaying CLIENT_START_ERROR [%d]", CLIENT_START_ERROR);
    char* fileName = CWvsApp::GetExceptionFileName();

    ClientFileStream fs{};
    fs.vftable = reinterpret_cast<void*>(C_FILE_STREAM_VFTABLE);

    using OpenFn  = int(__thiscall*)(void*, const char*, unsigned, unsigned, int, unsigned, int, int);
    using LenFn   = unsigned(__thiscall*)(void*);
    using ReadFn  = int(__thiscall*)(void*, void*, unsigned);
    using CloseFn = void(__thiscall*)(void*);

    reinterpret_cast<OpenFn>(C_FILE_STREAM_OPEN)(&fs, fileName, 0x80000000u, 128, 1, 0, 0, 0);
    unsigned len = reinterpret_cast<LenFn>(C_FILE_STREAM_GET_LENGTH)(&fs);

    ZArray<char> report;
    if (len && len < 0x2000) {
        char* dst = report.SetSize(len);
        reinterpret_cast<ReadFn>(C_FILE_STREAM_READ)(&fs, dst, len);
    }
    reinterpret_cast<CloseFn>(C_FILE_STREAM_CLOSE)(&fs);

    if (report.GetCount()) {
        COutPacket pkt(CLIENT_START_ERROR);
        pkt.Encode2(static_cast<unsigned short>(report.GetCount()));
        pkt.EncodeBuffer(report.GetData(), report.GetCount());
        CClientSocket::GetInstance()->SendPacket(&pkt);
    }
} else {
    /* existing PLAYER_LOGGED_IN branch — unchanged */
}
```

### `ClientFileStream` mirror (OQ-1)

```cpp
struct ClientFileStream {
    void* vftable;     // [+0x00] -> C_FILE_STREAM_VFTABLE
    char  pad[0x30];   // [+0x04] conservative; size from the v84.1 stack frame
};
```

Only `vftable@[0]` and `handle@[+0x10]` are semantically meaningful to us, but the buffer
**must** be at least the full stock object size so `Open`/`Read`/`Close` do not write out of
bounds (`Open` zeroes `[+8]`,`[+0xC]` and ORs flags into `[+0x34]`). **Resolution (OQ-1):**
read the exact stack footprint of the stream local in v84.1 `CClientSocket::OnConnect`
(the decompile shows a `v34[12]` 0x30 region; confirm the highest offset any of the four
methods touch) and size `pad` to cover it, rounding up. This is the same "size a local buffer
conservatively from disassembly" pattern `RaisePatch` uses for `CPatchException` (2048-byte
buffer ≥ the 1288-byte object). Anchor the final size to disassembly evidence, not a guess.

### Resource hygiene & error tolerance

- `ClientFileStream` is a stack object; `Close()` releases the OS handle. There is no heap
  stream to free. `ZArray<char> report` frees itself in its destructor at scope exit
  (`RemoveAll` → `Z_ARRAY_REMOVE_ALL`).
- Missing / locked / zero-byte file: stock `Open` throws `ZException` on
  `INVALID_HANDLE_VALUE`. **Open question for implementation (see §7):** confirm whether the
  stock `OnConnect` swallows that throw or relies on the outer handler; our reimplementation
  must match — if stock tolerates a missing file by sending nothing (the PRD's stated
  contract, FR-11), we must not let an `Open` throw escape as a crash. Default: if the file is
  absent, `GetLength` returns 0 and the `(0,0x2000)` guard skips the read, `report` stays
  empty, and nothing is sent — no exception raised (matches stock `if (buf && len)`).
- The `(0 < len < 0x2000)` guard bounds the allocation and the wire payload identically to
  stock.

### Wire-format parity (FR-13)

`Encode2` is little-endian u16; `EncodeBuffer` appends raw bytes. atlas-login
`serverbound.StartError.Decode` reads `ReadUint16 length` then `ReadBytes(length)` and
dispatches to `StartErrorHandleFunc`
(`atlas/services/atlas-login/.../socket/handler/start_error.go`). Exact match.

---

## 4. `common/ZArray.h` — `SetSize` / `GetData`

The stock report buffer is a `ZArray<char>` resized by `sub_49BBBE`. We model that in the
template rather than calling it by address.

```cpp
// Allocate exactly n elements, set the count header to n, return the data pointer.
T* SetSize(size_t n) {
    this->Alloc(n);     // existing: RemoveAll + alloc (n*sizeof(T)+sizeof(PVOID)); head = n
    return this->a;
}

// Raw pointer to element 0 (base of the allocation, after the count header).
T* GetData() {
    return this->a;
}
```

- `Alloc(size_t)` already exists and does exactly "free old, allocate `n`, write count header
  `= n`, point `a` at the first element." For a fresh report read (no prior contents to
  preserve) this is the correct semantics. **Verify against `sub_49BBBE`'s disassembly during
  implementation:** if the stock helper preserves contents or uses a doubling/Realloc path,
  adjust `SetSize` accordingly; for a single read into an empty array the observable result
  (a buffer of exactly `len` bytes, count `= len`) is the same.
- `GetData()` returns `this->a` — the same base `GetTailPosition()` already returns;
  `GetData` is added as the intention-revealing name the call site uses.
- These are additive, non-virtual, header-only methods on an existing template. No existing
  caller changes. Risk to other TUs using `ZArray<T>` is nil (no signature/layout change).

---

## 5. Memory-map additions

New per-version symbols (file-stream helper set). Added to each
`memory_maps/<REGION>/v<MAJOR>_<MINOR>.cmake` and surfaced in `include/memory_map.h.in`.
`C_ZARRAY_BYTE_SETSIZE` is **not** added (modeled in `ZArray.h` per §4).

| Symbol | Meaning | GMS v84.1 |
|---|---|---|
| `C_FILE_STREAM_OPEN` | `__thiscall Open(name,access,share,…)` → `CreateFileA` | `0x0049A615` |
| `C_FILE_STREAM_GET_LENGTH` | `__thiscall GetLength()` → size | `0x0049A79E` |
| `C_FILE_STREAM_READ` | `__thiscall Read(dst,len)` | `0x0049A8C9` |
| `C_FILE_STREAM_CLOSE` | `__thiscall Close()` / dtor | `0x0049A5B7` |
| `C_FILE_STREAM_VFTABLE` | stream object vtable | `0x00B437BC` |

Already present (verify, do not re-add): `CLIENT_START_ERROR`,
`C_WVS_APP_GET_EXCEPTION_FILE_NAME`, `C_TI_TERMINATE_EXCEPTION`,
`C_TI_DISCONNECT_EXCEPTION`, `C_CLIENT_SOCKET_ON_CONNECT`.

Per-IDB resolution recipe (from `memory-map.md` §"Resolution recipe"):
1. `get_metadata` to confirm the loaded IDB version (per the verify-IDA-target rule).
2. Decompile that version's `CClientSocket::OnConnect`; find the `bLogin` branch.
3. The call after `GetExceptionFileName()` taking `0x80000000` (GENERIC_READ) is
   `C_FILE_STREAM_OPEN`; the vtable assigned to the stack object just before it is
   `C_FILE_STREAM_VFTABLE`; subsequent length/read/close calls map in order.
4. Confirm the `COutPacket` ctor argument equals that version's `CLIENT_START_ERROR` opcode
   (already mapped: `0x19` v83/v84/v87, `0x1A` v95, `0x2F` v111, `0x15` JMS185).

---

## 6. Per-version gating strategy

Targets: GMS v83.1, v84.1, v87.1, v111.1, JMS v185.1 (must compile + link, per acceptance).

- The five `C_FILE_STREAM_*` symbols are resolved per version from each IDB. v84.1 values are
  known; the rest are `TBD` in `memory-map.md` and resolved during implementation.
- **If a version cannot faithfully resolve the stream helpers** (helper inlined, vtable
  ambiguous, or object impractical to reconstruct): gate that version's relay body off behind
  a documented `#if`, leaving the `bLogin` branch as a `Log(...)`-only no-op for that version,
  and mark the symbol row `N/A` with a one-line reason in `memory-map.md`. The build must
  still succeed and the load-balancing fix (which needs no new symbols) still applies. This is
  pre-approved by PRD §7.
- The load-balancing rewrite (§2) is **unconditional** across all versions — it depends only
  on `ZList`/exception helpers that already exist for every configured version.

---

## 7. Open questions carried into implementation

These are disassembly-resolved, not user-preference forks; listed so the plan tracks them.

- **OQ-1 — `ClientFileStream` object size.** Resolve the exact stack footprint from v84.1
  `CClientSocket::OnConnect`; size `pad` conservatively to cover the highest offset any of the
  four methods writes (`[+0x34]` flags seen so far). Anchor to disassembly evidence.
- **OQ-Open-throw — missing-file behavior.** Confirm from the v84.1 disassembly whether stock
  `CFileStream::Open` throwing on a missing report is caught locally or escapes. Reconcile
  with FR-11 ("missing/empty → no packet, no exception"). If `Open` can throw on a normal
  missing-file case, the design must guard the path (e.g. pre-check existence, or confirm
  `GetLength`-returns-0 path is reached without a throw) so a routine missing report never
  crashes the client.
- **Per-version `C_FILE_STREAM_*` addresses** for v83.1, v87.1, v111.1, JMS v185.1 — resolve
  per the §5 recipe; gate off per §6 if any version can't be resolved faithfully.

(OQ-2 from the PRD — ZArray reuse vs call-by-address — is **resolved**: model `SetSize`/
`GetData` in `ZArray.h` per §4; no `C_ZARRAY_BYTE_SETSIZE` symbol.)

---

## 8. Testing & verification

Per the repo's build/verify conventions (`scripts/wsl-build.sh`) and PRD §10 acceptance:

1. **Build matrix.** `scripts/wsl-build.sh` compiles + links all DLLs for GMS v83.1, v84.1,
   v87.1, v111.1, and JMS v185.1 with no new warnings in `bypass/socket_hooks.cpp` or
   `common/ZArray.h`.
2. **Load-balancing runtime (GMS v84.1, loaded IDB).** Force a bad login address list so the
   first node fails: confirm the cursor advances node-by-node and, on exhaustion, the client
   `Close()`s and shows the stock "cannot connect" failure (terminate path) rather than
   looping on the first address.
3. **`CLIENT_START_ERROR` runtime (GMS v84.1).** Seed a non-empty exception report at the
   `GetExceptionFileName()` path, connect to login, and observe atlas-login's
   `StartErrorHandleFunc` receive a `CLIENT_START_ERROR` whose payload equals the file bytes.
4. **Missing-report tolerance.** With no report file present, confirm login proceeds with no
   `CLIENT_START_ERROR` sent and no exception/crash.
5. **No happy-path regression.** Non-login game socket still sends `PLAYER_LOGGED_IN`; the
   version/patch ladder still raises correctly.

---

## 9. Files touched (summary)

| File | Change |
|---|---|
| `bypass/socket_hooks.cpp` | Rewrite `!bSuccess` load-balancing block (§2) and `bLogin` relay block (§3); add the `ClientFileStream` mirror struct + fn-ptr typedefs |
| `common/ZArray.h` | Add `SetSize(size_t)` and `GetData()` (§4) |
| `memory_maps/GMS/v83_1.cmake`, `v84_1.cmake`, `v87_1.cmake`, `v111_1.cmake`, `memory_maps/JMS/v185_1.cmake` | Add `C_FILE_STREAM_{OPEN,GET_LENGTH,READ,CLOSE,VFTABLE}` (§5) |
| `include/memory_map.h.in` | Surface the five new `C_FILE_STREAM_*` defines |

No new files. No new DLL edit. No INI changes.
