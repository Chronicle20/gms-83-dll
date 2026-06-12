# Faithful Client Exception Dispatch — Design

## Goal

Make the proxy DLL raise the MapleStory client's **own** typed exceptions so the
client's original `WinMain` handler catches and acts on them. Today the
`CWvsApp::Run` hook stubs the exception dispatch (`Log("Do proper
_com_raise_error"); return;`), which silently kills the message pump, and
`socket_hooks.cpp` throws `common/`-defined `std::exception` subclasses whose
RTTI the client's `catch` can never match. Replace both with one faithful,
shared mechanism.

## Background / evidence

`WinMain` (v84 `@0x00A39FA0`) wraps `CWvsApp::SetUp`/`Run` in MSVC C++ EH (the
`v41` state machine + `__CxxFrameHandler`), with catch funclets for the client's
`CPatchException` / `CDisconnectException` / `CTerminateException` / `ZException`.
We do **not** reimplement `WinMain`, so those catches are live.

The real `CWvsApp::Run` (v84 `@0x00A3E7E8`) dispatches the two error codes:

- `m_hrComErrorCode` (`this+0x38`) set → `sub_AAC743(hr, 0)`  (`_com_raise_error`)
- `m_hrZExceptionCode` (`this+0x34`) set → throw by code range:
  - `== 0x20000000` → `CPatchException` (object built by `sub_527978(m_nTargetVersion)`, 1288 bytes)
  - `0x21000000`–`0x21000006` → `CDisconnectException` (object = `int` code)
  - `0x22000000`–`0x2200000D` → `CTerminateException` (object = `int` code)
  - else → `ZException` (object = `int` code)

The render-failure paths raise COM errors via `sub_AABF64(hr)`
(`_com_raise_errorex`): `if (FAILED(hr)) sub_AABF64(hr);`.

`CDisconnectException` is the **server-migration** signal (login→channel). The
stubbed dispatch is why selecting a world/channel kills the client even when the
packet flow is correct.

## Throw mechanism

Call our **own** CRT `_CxxThrowException` with the **client's** `_ThrowInfo`
pointer (a memory-map address):

```cpp
extern "C" void __stdcall _CxxThrowException(void* pObject, void* pThrowInfo);
int code = ...;
_CxxThrowException(&code, reinterpret_cast<void*>(C_TI_DISCONNECT_EXCEPTION));
```

The MSVC C++ EH ABI is process-wide (`0xE06D7363` magic; identical `_ThrowInfo`/
`CatchableType`/`TypeDescriptor` layouts), so the client's `__CxxFrameHandler`
matches the catch against its own `TypeDescriptor` — the same pointer our raised
`_ThrowInfo` references. We never construct a `_ThrowInfo`; we only pass the
client's. For the three `int`-wrapped types the object is a single stack `int`.

Rejected alternatives: calling the *client's* `_CxxThrowException` (adds a key,
identical effect); hand-rolling `RaiseException` (reinvents the CRT).

`CPatchException`: call the client's `sub_527978(version)` (memory-map key) to
get the 1288-byte object pointer, copy it to a local buffer, then
`_CxxThrowException(buf, C_TI_PATCH_EXCEPTION)`. `version` = `CWvsApp::GetInstance()->m_nTargetVersion`.

COM raises: call the client's `sub_AAC743(hr, 0)` (`_com_raise_error`) and
`sub_AABF64(hr)` (`_com_raise_errorex`) via memory-map keys so the client throws
its own `_com_error` RTTI.

## Architecture

New shared module **`bypass/client_exception.h` / `client_exception.cpp`** — the
single place that touches the EH/COM-raise machinery. Public API:

```cpp
// Maps an m_hrZExceptionCode value to the client's typed throw (mirrors Run's ranges).
[[noreturn]] void RaiseClientException(int code);

// Convenience wrappers (readability for call sites); same underlying raises.
[[noreturn]] void RaiseDisconnect(int code);   // 0x21000000-range
[[noreturn]] void RaiseTerminate(int code);    // 0x22000000-range
[[noreturn]] void RaisePatch();                // builds object via client builder
[[noreturn]] void RaiseZException(int code);

// COM raises (HRESULT failure paths).
[[noreturn]] void RaiseComError(HRESULT hr);    // _com_raise_error   (sub_AAC743)
[[noreturn]] void RaiseComErrorEx(HRESULT hr);  // _com_raise_errorex (sub_AABF64)
```

`RaiseClientException` is the authority for the code→type ranges; the typed
wrappers exist for self-documenting call sites.

### Call-site changes

- **`app_hooks.cpp` `CWvsApp__Run_Hook`**: replace the two
  `Log("Do proper _com_raise_error"); return;` blocks with
  `RaiseComError(m_hrComErrorCode)` and `RaiseClientException(m_hrZExceptionCode)`
  (after the existing zero-of-fields). Replace the two `FAILED(hr)`
  `Log("Do proper _com_raise_errorex"); return;` blocks with `RaiseComErrorEx(hr)`.
- **`app_hooks.cpp` `CWvsApp__SetUp_Hook`** (`>=95`/JMS render block, line ~275):
  replace its `errorex` stub with `RaiseComErrorEx(hr)`.
- **`socket_hooks.cpp`**: replace `throw CTerminateException(570425351)` →
  `RaiseTerminate(0x22000007)` and `throw CPatchException()` → `RaisePatch()`.
  Remove the diagnostic-only `Log("...Should throw an exception here.")` stub at
  the ADR `Connect` path by raising the correct exception (confirm which against
  disasm during implementation).
- **`common/CTerminateException.h`**: delete. Remove its `#include`s.
- **`app_hooks.cpp` lines 140-142**: delete the commented-out `_CxxThrowException`
  stub (superseded).

## Memory-map keys (7 new, × all 6 build targets)

| Key | Source (v84) | Meaning |
|---|---|---|
| `C_TI_DISCONNECT_EXCEPTION` | `__TI3?AVCDisconnectException@@` `@0xB9C7B8` | `_ThrowInfo` |
| `C_TI_TERMINATE_EXCEPTION` | `__TI3?AVCTerminateException@@` `@0xB986C0` | `_ThrowInfo` |
| `C_TI_PATCH_EXCEPTION` | `__TI3?AVCPatchException@@` `@0xBA72F0` | `_ThrowInfo` |
| `C_TI_ZEXCEPTION` | `__TI1?AVZException@@` `@0xB98E40` | `_ThrowInfo` |
| `C_PATCH_EXCEPTION_BUILDER` | `sub_527978` | builds 1288-byte CPatch object |
| `C_COM_RAISE_ERROR` | `sub_AAC743` | `_com_raise_error(hr, 0)` |
| `C_COM_RAISE_ERROR_EX` | `sub_AABF64` | `_com_raise_errorex(hr)` |

Each must be located per build via its live IDB (GMS v83/v84/v87/v95, JMS185) and
derived from source for GMS v111. Add placeholders to `include/memory_map.h.in`
and values to every `memory_maps/<REGION>/<v>.cmake`. The completeness gate
(`GenerateMemoryMap` / `CheckMemoryMapKeys`) enforces non-empty for all targets.

Per-version confirmation (do not copy v84 values): re-derive each `_ThrowInfo`
from the type-info symbol or the type's throw sites; confirm `sub_527978` reads
the version arg and returns the patch object; confirm the two COM helpers take an
HRESULT and raise. Record each in a per-version signature catalog. The code
**ranges** (`0x20/0x21/0x22…`) must also be confirmed stable per version (they may
differ — re-read each `Run`).

## Error handling / edge cases

- `_CxxThrowException` is `[[noreturn]]`; the wrappers are `noreturn`. Call sites
  that currently `return` after the stub will instead unwind — verify no required
  cleanup is skipped (the surrounding RAII / client frames handle it; `WinMain`
  owns teardown).
- Object lifetime: the thrown `int` lives on the helper's stack; `_CxxThrowException`
  copies it into the catch frame before the helper unwinds — safe. The CPatch
  buffer is copied likewise.
- Reentrancy: do not raise while already unwinding; the Run dispatch only raises
  at the top of a loop iteration with no in-flight exception.
- Themida: these are ordinary `RaiseException`/EH flows the client already uses;
  no new anti-tamper surface.

## Testing

- **CI (authoritative for build):** all six targets compile Debug+Release with the
  new keys and module. This is the only check runnable on this Linux box.
- **Static cross-check:** for each version, the chosen `_ThrowInfo`/helper
  addresses match the type-info symbol / `Run` disasm (recorded in the signature
  catalog).
- **Live smoke (user-run, gates done):** v84 client → login → **select a world**
  → confirms the `CDisconnectException` migrate path now reconnects to the channel
  server (world list → channel select proceeds, no `ZException(38)`/silent death).
  Clean exit confirms `CTerminateException`. (Requires the atlas-ms client-key fix
  landed first, else the `ZException(38)` underrun still fires upstream.)

## Out of scope

- The atlas-ms `auth_success` client-key gate fix (server-side; separate).
- Reimplementing `WinMain` or its catch funclets (we rely on the stock ones).
- The task-006 diagnostic logging commit (`5d750b5`); fold/remove separately.

## Risks

- **Cross-module C++ EH assumption** — if a target's client uses a different EH
  model or a custom frame handler, our `_ThrowInfo` may not match. Mitigation:
  the static cross-check + the live smoke test per version; v84 proven first.
- **Wrong `_ThrowInfo`/helper address** on any version → crash on the (rare) raise
  path. Mitigation: per-version re-derivation, never copy v84.
- **CPatch object size/layout drift** across versions (1288 is v84-specific) —
  confirm the copy size per version from each `Run`'s `qmemcpy`.
