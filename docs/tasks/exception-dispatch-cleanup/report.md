# Faithful Client Exception Dispatch — Completion Report

## What changed

The proxy DLL now raises the MapleStory client's **own** typed exceptions (via the
client's `_ThrowInfo`) so the stock `WinMain` handler catches them, replacing the
`CWvsApp::Run` stub (`Log("Do proper …"); return;`) and the wrong-RTTI
`socket_hooks` throws.

- **New module** `bypass/client_exception.{h,cpp}` — `RaiseClientException`,
  `RaiseDisconnect/Terminate/Patch/ZException`, `RaiseComError/ComErrorEx`. Each
  calls our CRT `_CxxThrowException` with the client's `_ThrowInfo`, or the
  client's `__thiscall` CPatch ctor / 1-arg `__stdcall` `_com_error` raiser. Wired
  into `bypass/CMakeLists.txt`.
- **`bypass/app_hooks.cpp`** — `CWvsApp::Run` dispatches `m_hrComErrorCode` →
  `RaiseComError`, `m_hrZExceptionCode` → `RaiseClientException`; both `FAILED(hr)`
  render paths → `RaiseComErrorEx`. `CWvsApp::SetUp`'s render stub likewise. Dead
  commented `_CxxThrowException` stub removed. (The task-006 diagnostic logging in
  these swallow-points is superseded by this migration.)
- **`bypass/socket_hooks.cpp`** — `throw CTerminateException(570425351)` →
  `RaiseTerminate(0x22000007)` (×4); `throw CPatchException()` → `RaisePatch()`
  (×2); include swapped to `client_exception.h`.
- **`common/CTerminateException.h` deleted** — the std::exception `CTerminateException`/
  `CPatchException` classes are retired; no remaining includers.

## Memory-map keys (6 per build target × 6 targets)

| Key | Meaning |
|---|---|
| `C_TI_DISCONNECT_EXCEPTION` / `C_TI_TERMINATE_EXCEPTION` / `C_TI_PATCH_EXCEPTION` / `C_TI_ZEXCEPTION` | client `_ThrowInfo` structures (RTTI-symbol-verified each version) |
| `C_PATCH_EXCEPTION_BUILDER` | `__thiscall` ctor `(buf, version)` building CPatchException |
| `C_COM_RAISE_ERROR_EX` | 1-arg `__stdcall` `_com_error` raiser; serves both COM paths |

Per-version addresses + evidence: `signature-catalog.md`. All six completeness
gates pass at **151 keys** (`cmake -DREGION=<R> -DMAJOR=<M> -DMINOR=1 -P cmake/CheckMemoryMapKeys.cmake`).

## Design simplifications found during execution

The original plan scaffolded **8** keys; discovery + a final per-function cc check
collapsed it to **6**:
- **All six CPatch builders are `__thiscall` ctor(buf, version)** — including v84's
  `sub_527978` (the earlier "free function" reading was a folded-pseudocode
  artifact). So `C_PATCH_EXCEPTION_BUILDER_KIND` was dropped; `RaisePatch` always
  uses the ctor path.
- **The COM raisers are 1-arg `__stdcall`** (`?_com_issue_error@@YGXJ@Z` / v84
  `sub_AABF64`), and one raiser covers both the `m_hrComErrorCode` and render paths
  (the client always passes `perrinfo=0`). The redundant 2-arg `C_COM_RAISE_ERROR`
  was dropped. This also corrected a `__cdecl`→`__stdcall` cc bug that would have
  corrupted the stack.

## Build / verification status

- **Completeness gates:** all six PASS (151 keys). ✅ (only check runnable on this Linux box)
- **Per-version IDA cross-check:** the 4 `_ThrowInfo` are RTTI-symbol-verified; the
  builder + COM raiser are disasm-verified (recorded in `signature-catalog.md`).
- **Full Debug+Release compile of all six targets:** **CI** (MSVC/Win32 cannot build
  on Linux). Pending the pushed PR.
- **Live v84 smoke test (user-run, gates done):** login → **select a world** →
  confirms the `CDisconnectException` migrate path reconnects to the channel server;
  clean exit confirms `CTerminateException`. **Depends on the atlas-ms client-key
  fix landing first** — otherwise the `ZException(38)` underrun still fires upstream.

## Known limitations / flags (acceptable; documented)

- **Render-path COM raiser not verbatim-confirmed in v83/v95/JMS** — their `Run`/
  render path is virtualized, so the canonical 1-arg `_com_issue_error` (which throws
  the same `_com_error`) was recorded as the semantic equivalent. v84/v87/v111 are
  verbatim-confirmed. This path (render `FAILED(hr)`) is rarely hit.
- **Code-range bounds are the per-version union** — disconnect `≤ 0x21000007` (v111),
  terminate `≤ 0x2200000E` (v95/v111). Safe: a code only occurs in the version that
  defines it, so widening never misclassifies another version's code.
- **Two pre-existing "Should throw/issue exception" `Log` stubs left untouched** —
  `app_hooks.cpp` `CWvsApp::ConnectLogin` and `socket_hooks.cpp` ADR-`Connect`. Their
  exact codes need disasm confirmation and changing them risks the working
  connect/login flow; out of scope for this no-regression pass. Follow-up candidate.

## Acceptance vs the goal

The `CWvsApp::Run` exception dispatch is faithful for all six targets; `socket_hooks`
is unified onto the same mechanism; the std::exception classes are retired. Remaining
to call it fully done: green CI (all six compile) + the live v84 smoke test.
