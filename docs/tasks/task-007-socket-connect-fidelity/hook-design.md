# Hook Design — Socket Connect Fidelity

Scope: rewrite two regions inside `CClientSocket__OnConnect_Hook` (`bypass/socket_hooks.cpp`).
No new Detours installs. All addresses below are **GMS v84.1** (loaded IDB
`GMS_v84.1_U_DEVM`); `CClientSocket::OnConnect` @ `0x00499DCD`.

---

## 1. Load balancing — `!bSuccess` branch

### Stock behavior (decompiled)
```c
if (!bSuccess) {
    if (!this->m_ctxConnect.posList) {            // [this+32] == NULL  -> list exhausted
        CClientSocket::Close(this);
        if (this->m_ctxConnect.bLogin)            // [this+36]
            throw CTerminateException(0x22000001);  // magic 570425345
        throw CDisconnectException(0x21000001);     // magic 553648129
    }
    // posList != NULL: GetNext(&posList), Connect(currentNode)
    next = posList ? *(posList - 16 + 4) : *(int*)4;   // node->m_pNext
    saved = posList;                                   // current node (addr to try)
    posList = next ? next + 16 : 0;                    // advance cursor
    CClientSocket::Connect(this, saved);
    return 0;
}
```
`posList - 16 + 4` reads the `m_pNext` field of the `ZRefCountedDummy<ZInetAddr>` node
(our `ZList::CastNode` subtracts 16; `+4` is the `m_pNext` slot), then `+16` converts the
node pointer back to the wrapped `ZInetAddr*`. This is exactly `ZList<ZInetAddr>::GetNext`.

### Our implementation
```cpp
if (!bSuccess) {
    if (!pThis->m_ctxConnect.posList) {
        pThis->Close();
        if (pThis->m_ctxConnect.bLogin) {
            Log("CClientSocket::OnConnect connect failed (login) -> RaiseTerminate(0x22000001)");
            RaiseTerminate(0x22000001);   // [[noreturn]]
        }
        Log("CClientSocket::OnConnect connect failed -> RaiseDisconnect(0x21000001)");
        RaiseDisconnect(0x21000001);      // [[noreturn]]
    }

    // Advance the load-balancing cursor and retry the *current* node's address.
    auto* pos = reinterpret_cast<ZInetAddr*>(pThis->m_ctxConnect.posList);
    ZInetAddr* current = pThis->m_ctxConnect.lAddr.GetNext(&pos);
    pThis->m_ctxConnect.posList = reinterpret_cast<__POSITION*>(pos);
    CClientSocket__Connect_Addr_Hook(pThis, edx, reinterpret_cast<const sockaddr_in*>(current));
    return 0;
}
```

Notes
- `RaiseTerminate` / `RaiseDisconnect` already exist (`bypass/client_exception.{h,cpp}`)
  and unwind into the stock WinMain handler. They are `[[noreturn]]`; do not add a trailing
  `return`.
- `m_ctxConnect.posList` is declared `__POSITION*`; `lAddr` is `ZList<ZInetAddr>`. Casting
  `posList` ↔ `ZInetAddr*` mirrors how `CClientSocket__Connect_Ctx_Hook` already stores
  `reinterpret_cast<__POSITION*>(lAddr.GetHeadPosition())`.
- Passing `ZInetAddr*` as `const sockaddr_in*` matches the existing `Connect_Ctx` path,
  which calls `Connect_Addr(&pThis->m_addr)` where `m_addr` is a `ZInetAddr`.
- **Delete** the current line 124 `Connect_Addr_Hook(..., lAddr.GetHeadPosition())` and the
  `// TODO do i really care to do the loadbalancing logic?` comment.

### Magic-number cross-check
| Constant | Hex | Exception |
|---|---|---|
| 570425345 | `0x22000001` | `CTerminateException` |
| 553648129 | `0x21000001` | `CDisconnectException` |
| 570425351 | `0x22000007` | `CTerminateException` (version mismatch — already handled) |

---

## 2. `CLIENT_START_ERROR` relay — `bSuccess && bLogin` branch

### Stock behavior (decompiled, condensed)
```c
if (this->m_ctxConnect.bLogin) {
    char* name = CWvsApp::GetExceptionFileName();
    CFileStream fs;                 // stack object, vtable = off_B437BC
    fs.vftable = &off_B437BC;
    fs.Open(name, /*access*/0x80000000 GENERIC_READ, /*share*/128, 1, /*disp*/0, 0, 0);
    unsigned len = fs.GetLength();  // sub_49A79E
    if (len && len < 0x2000) {
        void* buf = ZArray_SetSize(&report, len);   // sub_49BBBE
        fs.Read(buf, len);                          // sub_49A8C9
    }
    fs.Close();                     // sub_49A5B7  (dtor / handle close)
    if (report && report.len) {
        COutPacket pkt(0x19);       // CLIENT_START_ERROR
        pkt.Encode2(report.len);
        pkt.EncodeBuffer(report.data, report.len);
        CClientSocket::SendPacket(this, &pkt);
        report.RemoveAll();
    }
}
```
`sub_49A615` confirmed `__thiscall CFileStream::Open(this=ecx; name, access, share, arg3,
disp, arg5, arg6)`: closes any prior handle (`sub_49A5B7`), calls `CreateFileA`
(`dword_C49A54`), stores the handle at `[this+0x10]`, throws `ZException` on
`INVALID_HANDLE_VALUE`. Object fields touched: `[+8]`, `[+0xC]` zeroed; `[+0x34] |= 1`,
and `|= 2` when the high access bit (`0x40` of byte 3 of access) is set.

### Our implementation (reuse the client stream)
Stack-construct the file-stream object, install its vtable, and drive it by mapped address.

```cpp
if (pThis->m_ctxConnect.bLogin) {
    Log("CClientSocket::OnConnect relaying CLIENT_START_ERROR [%d]", CLIENT_START_ERROR);
    char* fileName = CWvsApp::GetExceptionFileName();

    ClientFileStream fs{};                       // see struct sketch below
    fs.vftable = reinterpret_cast<void*>(C_FILE_STREAM_VFTABLE);

    using OpenFn = int(__thiscall*)(void* self, const char* name, unsigned access,
                                    unsigned share, int a3, unsigned disp, int a5, int a6);
    using LenFn  = unsigned(__thiscall*)(void* self);
    using ReadFn = int(__thiscall*)(void* self, void* dst, unsigned len);
    using CloseFn= void(__thiscall*)(void* self);

    reinterpret_cast<OpenFn>(C_FILE_STREAM_OPEN)(&fs, fileName, 0x80000000u, 128, 1, 0, 0, 0);
    unsigned len = reinterpret_cast<LenFn>(C_FILE_STREAM_GET_LENGTH)(&fs);

    ZArray<char> report;                         // or call C_ZARRAY_BYTE_SETSIZE by addr (OQ-2)
    if (len && len < 0x2000) {
        char* dst = report.SetSize(len);         // resolve against common/ZArray.h
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

`ClientFileStream` is a thin stack mirror sized to the stock object; only `vftable@[0]`
and `handle@[+0x10]` are semantically meaningful to us, but the buffer must be at least the
full object size so the client methods do not write out of bounds:

```cpp
struct ClientFileStream {
    void* vftable;     // [+0x00]  -> C_FILE_STREAM_VFTABLE
    char  pad[0x30];   // [+0x04]  conservative; confirm exact size (OQ-1)
};
```

Open items folded into PRD §9:
- Confirm exact object size (OQ-1) — size `pad` from the stock stack frame (`v34[12]` =
  0x30 region in v84.1) and any adjacent fields the methods touch.
- Decide `ZArray<char>` reuse vs `C_ZARRAY_BYTE_SETSIZE` by address (OQ-2). Prefer the
  template in `common/ZArray.h` if `SetSize`/`GetData`/`GetCount` already model `sub_49BBBE`.

### Wire format (atlas-login parity)
`serverbound.StartError.Decode`: `length = ReadUint16(); bytes = ReadBytes(length)`.
`Encode2` is little-endian u16, `EncodeBuffer` appends raw bytes — exact match. Handled by
`StartErrorHandleFunc` in
`atlas/services/atlas-login/atlas.com/login/socket/handler/start_error.go`.

---

## 3. Things that must NOT change
- Top guard `if (!pThis->m_ctxConnect.lAddr.GetCount()) return 0;`.
- The handshake decode (`decode_handshake`) and the version/patch ladder
  (`RaiseTerminate(0x22000007)` / `RaisePatch()`).
- The non-login `PLAYER_LOGGED_IN` send branch.
- The `BUILD_MAJOR_VERSION >= 95` `SendPacket` rewrite and its `#if` gate.
