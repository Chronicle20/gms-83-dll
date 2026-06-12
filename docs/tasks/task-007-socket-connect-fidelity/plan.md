# Socket Connect Fidelity — Load Balancing & CLIENT_START_ERROR Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Restore the stock client's connect load-balancing (node-by-node address retry + typed-exception exhaustion) and the `CLIENT_START_ERROR` exception-report relay inside the existing `CClientSocket__OnConnect_Hook`, gated to compile and link across GMS v83.1/v84.1/v87.1/v111.1 and JMS v185.1.

**Architecture:** Two in-place rewrites inside one hook (`bypass/socket_hooks.cpp`). The `!bSuccess` block advances the `m_ctxConnect.posList` cursor via the existing `ZList<ZInetAddr>::GetNext` idiom and, on exhaustion, `Close()`s + raises `RaiseTerminate`/`RaiseDisconnect` (both `[[noreturn]]`). The `bSuccess && bLogin` block reconstructs the stock `CFileStream` stack object, drives `Open/GetLength/Read/Close` by mapped address into a `ZArray<char>`, and emits `COutPacket(CLIENT_START_ERROR)`. Five new per-version memory-map symbols plus a compile-time resolution guard plumb the file-stream helpers.

**Tech Stack:** C++ (MSVC/clang-cl, Win32 DLL), CMake `configure_file`-generated `memory_map.h`, IDA Pro MCP for per-version address resolution, `scripts/wsl-build.sh` (clang-cl + xwin) for compile/link verification.

---

## Verification model (read first)

This repo has **no unit-test framework**. "Verification" for a code task means a clean cross-compile + link via `scripts/wsl-build.sh`, plus — for the addresses — IDA disassembly evidence, plus runtime smoke checks against the loaded IDB. Build invocation:

```bash
scripts/wsl-build.sh <REGION> <MAJOR> <MINOR>
# e.g.  scripts/wsl-build.sh GMS 84 1   |   scripts/wsl-build.sh JMS 185 1
```

A clean build ends with all edit DLLs + the proxy linked and exit code 0. The acceptance build matrix is: `GMS 83 1`, `GMS 84 1`, `GMS 87 1`, `GMS 111 1`, `JMS 185 1`.

**Commit after every task.** Never commit to `main` — work on a feature branch (see Task 0).

---

## File Structure

| File | Responsibility | Change |
|---|---|---|
| `common/ZArray.h` | `ZArray<T>` template | Add `SetSize(size_t)` + `GetData()` (report buffer) |
| `bypass/socket_hooks.cpp` | `CClientSocket__OnConnect_Hook` | Rewrite `!bSuccess` load-balancing block; rewrite `bLogin` relay block; add `ClientFileStream` mirror + fn-ptr typedefs |
| `include/memory_map.h.in` | generated-header template | Surface 5 `C_FILE_STREAM_*` defines + `C_FILE_STREAM_RESOLVED` guard |
| `memory_maps/GMS/v83_1.cmake` | per-version symbols | Add the 6 new `set()` entries |
| `memory_maps/GMS/v84_1.cmake` | per-version symbols | Add the 6 new `set()` entries (values known) |
| `memory_maps/GMS/v87_1.cmake` | per-version symbols | Add the 6 new `set()` entries |
| `memory_maps/GMS/v111_1.cmake` | per-version symbols | Add the 6 new `set()` entries |
| `memory_maps/JMS/v185_1.cmake` | per-version symbols | Add the 6 new `set()` entries |
| `docs/tasks/task-007-socket-connect-fidelity/memory-map.md` | resolved-address record | Fill in the TBD table from IDA evidence |

**Hard constraint (CMake):** `cmake/GenerateMemoryMap.cmake` scans `memory_map.h.in` for every `@KEY@` and `FATAL_ERROR`s if the selected version's `.cmake` leaves it **undefined or empty**. Therefore every new `@KEY@` must be `set()` to a **non-empty** value in **all five** `.cmake` files — real address where resolved, or a placeholder (`0x0`) **paired with the `C_FILE_STREAM_RESOLVED 0` guard** so the placeholder is never called at runtime.

---

## Task 0: Branch setup

**Files:** none (git only)

- [ ] **Step 1: Create a feature branch off main**

Run:
```bash
git checkout -b feat/task-007-socket-connect-fidelity
git status
```
Expected: on branch `feat/task-007-socket-connect-fidelity`; only the untracked `docs/tasks/task-007-socket-connect-fidelity/` present.

---

## Task 1: `ZArray<char>` report buffer — `SetSize` / `GetData`

Model the stock `sub_49BBBE` report-buffer resize in the template (design §4, resolves PRD OQ-2) so the relay reads into a `ZArray<char>` instead of calling a by-address helper. These are additive, header-only, non-virtual methods; no existing caller changes.

**Files:**
- Modify: `common/ZArray.h` (add two methods inside `public:`, near `Alloc` / `GetTailPosition`)

- [ ] **Step 1: Add `SetSize` and `GetData`**

Insert these two methods into the `public:` section of `ZArray<T>` (e.g. immediately after `GetTailPosition()` at `common/ZArray.h:275`):

```cpp
	/// <summary>
	/// Allocate exactly n elements, set the count header to n, and return the
	/// data pointer. Models the stock report-buffer resize (sub_49BBBE) for a
	/// fresh read into an empty array: Alloc() frees any prior block, allocates
	/// n*sizeof(T)+sizeof(PVOID), and writes the count header = n.
	/// </summary>
	T* SetSize(size_t n)
	{
		this->Alloc(n);
		return this->a;
	}

	/// <summary>
	/// Raw pointer to element 0 (base of the allocation, after the count header).
	/// Intention-revealing alias of GetTailPosition() for buffer call sites.
	/// </summary>
	T* GetData()
	{
		return this->a;
	}
```

Note: `Alloc(size_t)` already exists at `common/ZArray.h:301` and does exactly "RemoveAll, allocate `n*sizeof(T)+sizeof(PVOID)`, write count header `= n`, point `a` past the header." `GetCount()` then returns `n`. No change to `Alloc`.

- [ ] **Step 2: Build-verify (header-only change compiles)**

Run:
```bash
scripts/wsl-build.sh GMS 84 1
```
Expected: exit code 0, all DLLs linked. (`ZArray.h` is included widely; a clean build proves the additive methods don't break any TU.)

- [ ] **Step 3: Commit**

```bash
git add common/ZArray.h
git commit -m "feat(ZArray): add SetSize/GetData for the report buffer"
```

---

## Task 2: Load-balancing rewrite (`!bSuccess` block)

Replace the dead `!bSuccess` body (`bypass/socket_hooks.cpp:113-126`) — the two `Log(...) return 0` exhaustion stubs and the `GetHeadPosition()` re-pass marked `// TODO do i really care...` — with faithful cursor advancement + typed-exception exhaustion (design §2, hook-design.md §1). This needs **no** new memory-map symbols and applies **unconditionally** to every version.

**Files:**
- Modify: `bypass/socket_hooks.cpp:113-126`

- [ ] **Step 1: Replace the `!bSuccess` block**

Replace exactly these lines:

```cpp
    if (!bSuccess) {
        if (!pThis->m_ctxConnect.posList) {
            pThis->Close();
            if (pThis->m_ctxConnect.bLogin) {
                Log("CClientSocket::OnConnect 570425345");
                return 0;
            }
            Log("CClientSocket::OnConnect 553648129");
            return 0;
        }
        // TODO do i really care to do the loadbalancing logic?
        CClientSocket__Connect_Addr_Hook(pThis, edx, pThis->m_ctxConnect.lAddr.GetHeadPosition());
        return 0;
    }
```

with:

```cpp
    if (!bSuccess) {
        if (!pThis->m_ctxConnect.posList) {
            // Address list exhausted: close and raise the stock typed exception,
            // which unwinds into the client WinMain handler (both [[noreturn]]).
            pThis->Close();
            if (pThis->m_ctxConnect.bLogin) {
                Log("CClientSocket::OnConnect connect failed (login) -> RaiseTerminate(0x22000001)");
                RaiseTerminate(0x22000001);   // CTerminateException, magic 570425345
            }
            Log("CClientSocket::OnConnect connect failed -> RaiseDisconnect(0x21000001)");
            RaiseDisconnect(0x21000001);       // CDisconnectException, magic 553648129
        }

        // Advance the load-balancing cursor (GetNext returns the current node and
        // moves posList to node->m_pNext) and retry the *current* node's address.
        auto* pos = reinterpret_cast<ZInetAddr*>(pThis->m_ctxConnect.posList);
        ZInetAddr* current = pThis->m_ctxConnect.lAddr.GetNext(&pos);
        pThis->m_ctxConnect.posList = reinterpret_cast<__POSITION*>(pos);
        CClientSocket__Connect_Addr_Hook(pThis, edx, current);   // ZInetAddr : sockaddr_in
        return 0;
    }
```

Why this is correct (do not re-derive the pointer math):
- `ZList<ZInetAddr>::GetNext(T** pos)` (`common/ZList.h:333`) returns `*pos` and advances `*pos` to the wrapped `m_pNext` via `CastNode` (subtract 16) — exactly the stock `posList-16+4 -> m_pNext`, `+16 -> ZInetAddr*` idiom in hook-design.md §1.
- `ZInetAddr` is `struct ZInetAddr : sockaddr_in` (`common/ZInetAddr.h:3`), so passing `current` where `CClientSocket__Connect_Addr_Hook` expects `const sockaddr_in*` is a plain base-class conversion — no cast on the argument (mirrors `Connect_Ctx_Hook` passing `&pThis->m_addr`).
- The `posList ↔ __POSITION*` casts mirror the existing `reinterpret_cast<__POSITION*>(...GetHeadPosition())` in `Connect_Ctx_Hook` (`socket_hooks.cpp:282`).
- `RaiseTerminate`/`RaiseDisconnect` are declared `[[noreturn]]` (`bypass/client_exception.h:8-9`); **no trailing `return`** after them — the exhaustion branch cannot fall through into the success path.

- [ ] **Step 2: Build-verify**

Run:
```bash
scripts/wsl-build.sh GMS 84 1
```
Expected: exit code 0, clean link, no new warnings in `socket_hooks.cpp`.

- [ ] **Step 3: Commit**

```bash
git add bypass/socket_hooks.cpp
git commit -m "feat(socket): restore connect load-balancing + typed-exception exhaustion"
```

---

## Task 3: Resolve file-stream helper addresses + v84.1 disassembly facts (IDA)

Resolve the per-version `C_FILE_STREAM_*` addresses and settle the two open disassembly questions (OQ-1 object size, OQ-throw missing-file behavior). This task produces **evidence + an address table**, written into `docs/tasks/task-007-socket-connect-fidelity/memory-map.md`. No source builds here.

**Files:**
- Modify: `docs/tasks/task-007-socket-connect-fidelity/memory-map.md` (fill TBD table + record OQ-1 size and OQ-throw decision)

**Known v84.1 anchors** (loaded IDB `GMS_v84.1_U_DEVM`, `CClientSocket::OnConnect @ 0x00499DCD`):

| Symbol | v84.1 |
|---|---|
| `C_FILE_STREAM_OPEN` | `0x0049A615` |
| `C_FILE_STREAM_GET_LENGTH` | `0x0049A79E` |
| `C_FILE_STREAM_READ` | `0x0049A8C9` |
| `C_FILE_STREAM_CLOSE` | `0x0049A5B7` |
| `C_FILE_STREAM_VFTABLE` | `0x00B437BC` |

- [ ] **Step 1: Confirm the loaded IDB before any probe**

Use `mcp__ida-pro__get_metadata` (or `entity_query`/server_health) to confirm which IDB is loaded. **Do not infer the version from conversation context** (per the verify-IDA-target rule). Record the version string.

- [ ] **Step 2: Resolve OQ-1 (object size) from v84.1**

With the v84.1 IDB loaded, decompile `CClientSocket::OnConnect` (`0x00499DCD`) and the `bLogin` branch. Find the stack object whose vtable is `0x00B437BC` and that `0x0049A615` (`Open`) is called on. Determine the **highest byte offset** any of `Open/GetLength/Read/Close` writes within that object. Known so far: `Open` zeroes `[+8]`,`[+0xC]` and ORs flags into `[+0x34]` (a dword → writes through `[+0x37]`), stores the handle at `[+0x10]`. Record the exact size.

**Note — the design's `pad[0x30]` is too small:** vftable (4 bytes) + `pad[0x30]` = `0x34` total, but a dword write at `[+0x34]` needs the object to be ≥ `0x38` bytes. Size `pad` so the **total object ≥ (highest offset written + 4), rounded up** — e.g. `pad[0x3C]` (total `0x40`) is a safe conservative cover for a `[+0x34]` dword write, matching the RaisePatch "2048 ≥ 1288" conservative-buffer precedent. Record the chosen total size for Task 5.

- [ ] **Step 3: Resolve OQ-throw (missing-file behavior) from v84.1**

Inspect `C_FILE_STREAM_OPEN` (`sub_49A615`) and the `OnConnect` `bLogin` branch: does `Open` throw `ZException` on `INVALID_HANDLE_VALUE`, and if so is that throw caught locally in `OnConnect` or does it escape? Reconcile with FR-11 ("missing/empty report → no packet, no exception"). Record the finding and the required guard in Task 5:
  - If `OnConnect` reaches `GetLength` (returns 0) without `Open` throwing on a routine missing file, the `(0 < len < 0x2000)` guard alone is sufficient (no extra guard).
  - If `Open` **can** throw on a routine missing file, Task 5 must pre-check existence (e.g. skip the whole relay when `GetExceptionFileName()` returns a path that does not exist) so a normal missing report never crashes the client.

- [ ] **Step 4: Resolve the other four versions' addresses**

For each of v83.1, v87.1, v111.1, JMS v185.1: load that IDB (confirm via `get_metadata` first), decompile its `CClientSocket::OnConnect`, find the `bLogin` branch, and map the helpers using the recipe in `memory-map.md` §"Resolution recipe":
  - The call right after `GetExceptionFileName()` taking `0x80000000` (GENERIC_READ) is `C_FILE_STREAM_OPEN`; the vtable assigned to the stack object just before it is `C_FILE_STREAM_VFTABLE`; the subsequent length/read/close calls map in order.
  - Confirm the `COutPacket` ctor argument equals that version's `CLIENT_START_ERROR` opcode (already mapped: `0x19` v83/v84/v87, `0x2F` v111, `0x15` JMS185).
  - If a version's helpers are inlined / the object is impractical to reconstruct, mark that version **`N/A` with a one-line reason** — it will use a `0x0` placeholder + `C_FILE_STREAM_RESOLVED 0` guard in Task 4 (pre-approved by PRD §7 / design §6).

- [ ] **Step 5: Record everything in `memory-map.md`**

Fill the per-version table (replace every `TBD`), record the OQ-1 total object size, the OQ-throw decision, and which versions are `RESOLVED=1` vs `RESOLVED=0`. This file is the source of truth for Tasks 4 and 5.

- [ ] **Step 6: Commit**

```bash
git add docs/tasks/task-007-socket-connect-fidelity/memory-map.md
git commit -m "docs(task-007): resolve file-stream helper addresses + OQ-1/OQ-throw"
```

---

## Task 4: Memory-map plumbing (6 symbols × 5 versions + template)

Surface the file-stream helper set and a compile-time resolution guard. Every `@KEY@` must be non-empty in all five `.cmake` files (CMake hard constraint above).

**Files:**
- Modify: `include/memory_map.h.in` (add 6 defines)
- Modify: `memory_maps/GMS/v83_1.cmake`, `v84_1.cmake`, `v87_1.cmake`, `v111_1.cmake`, `memory_maps/JMS/v185_1.cmake` (add 6 `set()` each)

- [ ] **Step 1: Add the six defines to `memory_map.h.in`**

Append after the exception-dispatch block at the end of `include/memory_map.h.in`:

```c

// --- File-stream report read for CLIENT_START_ERROR (docs/tasks/task-007-socket-connect-fidelity) ---
// C_FILE_STREAM_RESOLVED is 1 when this version's stream helpers were resolved
// from its IDB, 0 when the relay body is gated off (placeholder addresses).
#define C_FILE_STREAM_RESOLVED @C_FILE_STREAM_RESOLVED@
#define C_FILE_STREAM_OPEN @C_FILE_STREAM_OPEN@
#define C_FILE_STREAM_GET_LENGTH @C_FILE_STREAM_GET_LENGTH@
#define C_FILE_STREAM_READ @C_FILE_STREAM_READ@
#define C_FILE_STREAM_CLOSE @C_FILE_STREAM_CLOSE@
#define C_FILE_STREAM_VFTABLE @C_FILE_STREAM_VFTABLE@
```

- [ ] **Step 2: Add the six `set()` entries to `memory_maps/GMS/v84_1.cmake`**

Append (values known and confirmed in Task 3):

```cmake
set(C_FILE_STREAM_RESOLVED     1)
set(C_FILE_STREAM_OPEN         0x0049A615) # __thiscall CFileStream::Open(name,access,share,…) -> CreateFileA
set(C_FILE_STREAM_GET_LENGTH   0x0049A79E) # __thiscall GetLength() -> size
set(C_FILE_STREAM_READ         0x0049A8C9) # __thiscall Read(dst,len)
set(C_FILE_STREAM_CLOSE        0x0049A5B7) # __thiscall Close()/dtor
set(C_FILE_STREAM_VFTABLE      0x00B437BC) # stream object vtable
```

- [ ] **Step 3: Add the six `set()` entries to the other four `.cmake` files**

Using the addresses resolved in Task 3, append the same six-line block to `memory_maps/GMS/v83_1.cmake`, `memory_maps/GMS/v87_1.cmake`, `memory_maps/GMS/v111_1.cmake`, and `memory_maps/JMS/v185_1.cmake` — substituting that version's real addresses and `C_FILE_STREAM_RESOLVED 1`.

For any version marked **`N/A`** in Task 3, use this gated form instead (non-empty placeholders keep CMake happy; the `RESOLVED 0` guard makes Task 5 emit a Log-only no-op so `0x0` is never called):

```cmake
set(C_FILE_STREAM_RESOLVED     0)          # relay gated off: <one-line reason from Task 3>
set(C_FILE_STREAM_OPEN         0x0)
set(C_FILE_STREAM_GET_LENGTH   0x0)
set(C_FILE_STREAM_READ         0x0)
set(C_FILE_STREAM_CLOSE        0x0)
set(C_FILE_STREAM_VFTABLE      0x0)
```

- [ ] **Step 4: Configure-verify every version resolves its keys**

Run each build to prove `GenerateMemoryMap.cmake` finds all keys (a missing/empty key is a `FATAL_ERROR` at configure time):

```bash
scripts/wsl-build.sh GMS 83 1
scripts/wsl-build.sh GMS 84 1
scripts/wsl-build.sh GMS 87 1
scripts/wsl-build.sh GMS 111 1
scripts/wsl-build.sh JMS 185 1
```
Expected: each configures past the memory-map validation and links cleanly (the relay body is added in Task 5; the new symbols are defined but not yet referenced, so this is purely a plumbing check).

- [ ] **Step 5: Commit**

```bash
git add include/memory_map.h.in memory_maps/GMS/v83_1.cmake memory_maps/GMS/v84_1.cmake \
        memory_maps/GMS/v87_1.cmake memory_maps/GMS/v111_1.cmake memory_maps/JMS/v185_1.cmake
git commit -m "feat(memory-map): add file-stream helper set + resolution guard"
```

---

## Task 5: `CLIENT_START_ERROR` relay (`bLogin` block)

Replace the `bLogin` stub (`bypass/socket_hooks.cpp:210-213` — the `// TODO relay CLIENT_START_ERROR` and the dropped `GetExceptionFileName()`) with the faithful report read + send (design §3, hook-design.md §2). The non-login `PLAYER_LOGGED_IN` `else` branch is unchanged.

**Files:**
- Modify: `bypass/socket_hooks.cpp` (add `ClientFileStream` mirror + typedefs near the top file-scope helpers; rewrite the `bLogin` branch body)

- [ ] **Step 1: Add the `ClientFileStream` mirror + fn-ptr typedefs**

In `bypass/socket_hooks.cpp`, just below the existing `// ---- typedefs ----` block (after line 22, before the anonymous namespace), add:

```cpp
// ---- CLIENT_START_ERROR report-read mirror (§3) -------------------------
// Thin stack mirror of the stock CFileStream object. Only vftable@[0] and
// handle@[+0x10] are semantically meaningful to us, but the buffer MUST be at
// least the full stock object size so Open/Read/Close do not write OOB.
// `pad` is sized from the v84.1 stack frame (Task 3 / OQ-1): Open writes a dword
// at [+0x34], so total object >= 0x38; pad[0x3C] (total 0x40) is the conservative
// cover. Adjust only if Task 3 found a larger footprint.
struct ClientFileStream {
    void* vftable;     // [+0x00] -> C_FILE_STREAM_VFTABLE
    char  pad[0x3C];   // [+0x04] conservative; covers the [+0x34] flag write
};

using FileStreamOpenFn  = int(__thiscall*)(void* self, const char* name, unsigned access,
                                           unsigned share, int a3, unsigned disp, int a5, int a6);
using FileStreamLenFn   = unsigned(__thiscall*)(void* self);
using FileStreamReadFn  = int(__thiscall*)(void* self, void* dst, unsigned len);
using FileStreamCloseFn = void(__thiscall*)(void* self);
```

> If Task 3 recorded a total object size larger than `0x40`, set `pad` to `(total size − 4)` instead of `0x3C`.

- [ ] **Step 2: Rewrite the `bLogin` branch body**

Replace exactly:

```cpp
    if (pThis->m_ctxConnect.bLogin) {
        Log("CClientSocket::OnConnect should be sending [%d]", CLIENT_START_ERROR);
        // TODO relay CLIENT_START_ERROR
        char* fileName = CWvsApp::GetExceptionFileName();
    } else {
```

with (the `#if C_FILE_STREAM_RESOLVED` guard makes gated-off versions a Log-only no-op so placeholder `0x0` addresses are never called):

```cpp
    if (pThis->m_ctxConnect.bLogin) {
        Log("CClientSocket::OnConnect relaying CLIENT_START_ERROR [%d]", CLIENT_START_ERROR);
#if C_FILE_STREAM_RESOLVED
        char* fileName = CWvsApp::GetExceptionFileName();

        ClientFileStream fs{};
        fs.vftable = reinterpret_cast<void*>(C_FILE_STREAM_VFTABLE);

        // Open(name, GENERIC_READ=0x80000000, share=128, 1, disp=0, 0, 0)
        reinterpret_cast<FileStreamOpenFn>(C_FILE_STREAM_OPEN)(&fs, fileName, 0x80000000u, 128, 1, 0, 0, 0);
        unsigned len = reinterpret_cast<FileStreamLenFn>(C_FILE_STREAM_GET_LENGTH)(&fs);

        ZArray<char> report;
        if (len && len < 0x2000) {
            char* dst = report.SetSize(len);
            reinterpret_cast<FileStreamReadFn>(C_FILE_STREAM_READ)(&fs, dst, len);
        }
        reinterpret_cast<FileStreamCloseFn>(C_FILE_STREAM_CLOSE)(&fs);

        if (report.GetCount()) {
            COutPacket pkt(CLIENT_START_ERROR);
            pkt.Encode2(static_cast<unsigned short>(report.GetCount()));
            pkt.EncodeBuffer(report.GetData(), report.GetCount());
            CClientSocket::GetInstance()->SendPacket(&pkt);
        }
        // ZArray<char> report frees itself at scope exit; fs is a stack object,
        // its handle released by Close().
#else
        Log("CClientSocket::OnConnect CLIENT_START_ERROR relay gated off for this version");
#endif
    } else {
```

> **If Task 3 found `Open` can throw on a routine missing file (OQ-throw):** guard the read path so a normal missing report never crashes — e.g. wrap the `Open`…`Close` sequence to skip when the file is absent (pre-check the path), keeping `report` empty so nothing is sent. Implement exactly what Task 3 recorded; do not add a guard the disassembly says is unnecessary.

Leave everything after `} else {` (the `PLAYER_LOGGED_IN` branch, lines 214-238) unchanged. The closing `}` of the `else` and the `return 1;` stay as-is.

- [ ] **Step 3: Build-verify the resolved path (v84.1)**

Run:
```bash
scripts/wsl-build.sh GMS 84 1
```
Expected: exit code 0; `socket_hooks.cpp` compiles the `#if C_FILE_STREAM_RESOLVED` (true) path with no new warnings.

- [ ] **Step 4: Build-verify a gated-off path (if any version is `RESOLVED 0`)**

If Task 3 gated any version off, build it to prove the `#else` no-op path compiles, e.g.:
```bash
scripts/wsl-build.sh GMS 111 1   # or whichever version is RESOLVED 0
```
Expected: exit code 0. (Skip this step only if every version is `RESOLVED 1`.)

- [ ] **Step 5: Commit**

```bash
git add bypass/socket_hooks.cpp
git commit -m "feat(socket): relay CLIENT_START_ERROR report on login connect"
```

---

## Task 6: Full build matrix

Prove the touched TUs compile + link across the entire acceptance matrix with no new warnings (PRD §10).

**Files:** none (verification only)

- [ ] **Step 1: Build all five region+version combos**

Run, in order, confirming each exits 0:
```bash
scripts/wsl-build.sh GMS 83 1
scripts/wsl-build.sh GMS 84 1
scripts/wsl-build.sh GMS 87 1
scripts/wsl-build.sh GMS 111 1
scripts/wsl-build.sh JMS 185 1
```
Expected: each links all edit DLLs + the proxy, exit 0, no new warnings in `bypass/socket_hooks.cpp` or `common/ZArray.h`.

- [ ] **Step 2: Record the matrix result**

Note in `docs/tasks/task-007-socket-connect-fidelity/memory-map.md` (or a short `build-matrix.md`) which combos are `RESOLVED 1` vs gated, so the audit phase has the record. Commit if anything was written:
```bash
git add -A docs/tasks/task-007-socket-connect-fidelity/
git commit -m "docs(task-007): record build-matrix result" || true
```

---

## Task 7: Runtime verification (GMS v84.1)

Smoke-test the two restored behaviors against the loaded v84.1 client + atlas-login (PRD §10 acceptance, design §8). These are manual/observational checks; record outcomes.

**Files:** none (runtime only)

- [ ] **Step 1: Load-balancing exhaustion**

Force a bad login address list (e.g. point the login address(es) at an unreachable host) and connect. Confirm the cursor advances node-by-node and, on exhaustion (`posList == NULL`), the client `Close()`s and surfaces the stock "cannot connect" failure (terminate path) rather than looping on the first address.

- [ ] **Step 2: `CLIENT_START_ERROR` relay**

Seed a non-empty exception report at the `CWvsApp::GetExceptionFileName()` path (length `< 0x2000`), connect to login, and observe atlas-login's `StartErrorHandleFunc` (`atlas/services/atlas-login/.../socket/handler/start_error.go`) receive a `CLIENT_START_ERROR` whose `u16 len` + bytes equal the file contents.

- [ ] **Step 3: Missing-report tolerance**

With no report file present, confirm login proceeds, **no** `CLIENT_START_ERROR` is sent, and there is no exception/crash (FR-11).

- [ ] **Step 4: No happy-path regression**

Confirm the non-login game socket still sends `PLAYER_LOGGED_IN` and the version/patch ladder still raises correctly.

- [ ] **Step 5: Record results + finish the branch**

Note the runtime outcomes, then use the `superpowers:finishing-a-development-branch` skill to merge/PR. Do not merge to `main` directly.

---

## Self-Review (completed)

- **Spec coverage:** FR-1..FR-3 → Task 2; FR-4..FR-7 → Task 2 (exhaustion + preserved top guard, which is untouched at `socket_hooks.cpp:110-112`); FR-8..FR-12 → Task 5; FR-13 → Task 5 (`Encode2`+`EncodeBuffer`) verified at runtime in Task 7 Step 2; memory-map additions (PRD §7) → Tasks 3+4; OQ-1/OQ-throw → Task 3; OQ-2 (ZArray reuse) → Task 1; build matrix → Task 6; runtime acceptance → Task 7.
- **Placeholder scan:** every code step shows complete code; the only intentional deferrals are the IDA-resolved addresses (Task 3) and the OQ-driven `pad` size / missing-file guard, which are disassembly facts the executor records, not invented values.
- **Type consistency:** `SetSize`/`GetData`/`GetCount` (Task 1) match the call site in Task 5; `ClientFileStream` / `FileStream{Open,Len,Read,Close}Fn` defined in Task 5 Step 1 and used in Step 2; `C_FILE_STREAM_{RESOLVED,OPEN,GET_LENGTH,READ,CLOSE,VFTABLE}` defined in Task 4 and consumed in Task 5; `RaiseTerminate`/`RaiseDisconnect` signatures match `client_exception.h`.
- **CMake constraint:** every new `@KEY@` is `set()` in all five `.cmake` files (real or guarded placeholder), satisfying `GenerateMemoryMap.cmake`.
