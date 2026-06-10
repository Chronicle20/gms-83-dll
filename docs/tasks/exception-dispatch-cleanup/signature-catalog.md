# Exception-dispatch keys — per-version address catalog

Anchor: each version's `CWvsApp::Run` exception-dispatch block. Re-derived per
binary, never copied. Verify the IDB with `get_metadata` before recording.

## GMS v84.1 (port 13341) — Run @ 0xA3E7E8
| Key | Address | Anchor |
|---|---|---|
| C_TI_DISCONNECT_EXCEPTION | 0x00B9C7B8 | __TI3?AVCDisconnectException@@; throw @0x21000000-range |
| C_TI_TERMINATE_EXCEPTION  | 0x00B986C0 | __TI3?AVCTerminateException@@; throw @0x22000000-range |
| C_TI_PATCH_EXCEPTION       | 0x00BA72F0 | __TI3?AVCPatchException@@; throw @==0x20000000 |
| C_TI_ZEXCEPTION            | 0x00B98E40 | __TI1?AVZException@@; default throw |
| C_PATCH_EXCEPTION_BUILDER  | 0x00527978 | v3=sub_527978(this[16]); qmemcpy 1288 then throw CPatch |
| C_COM_RAISE_ERROR          | 0x00AAC743 | sub_AAC743(v37,0) on m_hrComErrorCode |
| C_COM_RAISE_ERROR_EX       | 0x00AABF64 | sub_AABF64(hr) on FAILED(render hr) |

Ranges (v84): Patch ==0x20000000 · Disconnect 0x21000000–0x21000006 · Terminate 0x22000000–0x2200000D · else ZException.

## GMS v83.1 (port 13337) — Run body @ 0x9F5FD9 (real dispatch; named `?Run@CWvsApp@@` @0x9F5C50 is a virtualized thunk into the 0xC22xxx VM)
| Key | Address | Anchor |
|---|---|---|
| C_TI_DISCONNECT_EXCEPTION | 0x00B48858 | __TI3?AVCDisconnectException@@; `_CxxThrowException(&code,&CDisconnect_TI)` @0x9F620E (0x21000000–0x21000006) |
| C_TI_TERMINATE_EXCEPTION  | 0x00B44760 | __TI3?AVCTerminateException@@; `_CxxThrowException(&code,&CTerminate_TI)` @0x9F6270 (0x22000000–0x2200000D) |
| C_TI_PATCH_EXCEPTION       | 0x00B52FC8 | __TI3?AVCPatchException@@ (struct: CTA@0xB52FB8→CT@0xB52F98→TypeDesc 0xBD8660); throw @0x9F61AC (==0x20000000) |
| C_TI_ZEXCEPTION            | 0x00B44EE0 | __TI1?AVZException@@; default `_CxxThrowException(&code,&ZException_TI)` @0x9F6299 |
| C_PATCH_EXCEPTION_BUILDER  | 0x0051E834 | `*(a2-3384)=sub_51E834((a2-3304),*(*(a2-3376)+64));` @0x9F6177 — sets *this=0x20000000, *(this+4)=83 (major), *(this+3)=version, memset 1284 → qmemcpy 0x508 then throw CPatch |
| C_COM_RAISE_ERROR          | 0x00A605C3 | `_com_raise_error(*(a2-172),0);` @0x9F6103 on `*(app+56)` (m_hrComErrorCode) — call e8 BB A4 06 00 |
| C_COM_RAISE_ERROR_EX       | 0x00A5FDE4 | `_com_issue_error(hr)` = `_com_raise_error(hr,0)`; v83's single-HRESULT raiser (analog of v84 `_com_raise_errorex`). See concern below. |

Ranges (v83, IDENTICAL to v84): Patch `==0x20000000` · Disconnect `0x21000000–0x21000006` · Terminate `0x22000000–0x2200000D` · else ZException.

Notes:
- v83 `CWvsApp::Run` and `CWvsApp::CallUpdate` are virtualized (thunk at 0x9F5C50 / 0x9F84D0 jumps into the shared 0xC22xxx VM dispatcher). The faithful exception dispatch + message-pump body is the contiguous non-virtualized chunk at **0x9F5FD9** (defined as a function during this analysis; lies in the gap between `?Run@CWvsApp@@`+0x58 and `?CleanUp@CWvsApp@@` @0x9F69B7).
- CONCERN on C_COM_RAISE_ERROR_EX: unlike v84 (which has a *distinct* client `sub_AABF64` for the FAILED-render path), v83's Run body contains only ONE com-raise call — `_com_raise_error(hr,0)` (the standard comdef helper) for `m_hrComErrorCode`. The render-FAILED HRESULT path lives inside the obfuscated `CallUpdate` VM and could not be resolved to a discrete call site. `_com_issue_error` @0xA5FDE4 (= `_com_raise_error(hr,0)`, the 1-arg HRESULT raiser) is recorded as the structural/semantic v83 analog of v84's `_com_raise_errorex(hr)`. If the faithful port needs the exact render-path entry, it is NOT independently confirmed in v83.

## GMS v87.1 (port 13338) — Run @ 0xA88B81
| Key | Address | Anchor |
|---|---|---|
| C_TI_DISCONNECT_EXCEPTION | 0x00BF0B80 | __TI3?AVCDisconnectException@@; `_CxxThrowException(&v10,&_TI3_AVCDisconnectException__)` @0xa892f8 (0x21000000–0x21000006) |
| C_TI_TERMINATE_EXCEPTION  | 0x00BEC9D0 | __TI3?AVCTerminateException@@; `_CxxThrowException(&v8,&_TI3_AVCTerminateException__)` @0xa8935f (0x22000000–0x2200000D) |
| C_TI_PATCH_EXCEPTION       | 0x00BFC420 | __TI3?AVCPatchException@@; `_CxxThrowException(pExceptionObject,&_TI3_AVCPatchException__)` @0xa89291 (==0x20000000) |
| C_TI_ZEXCEPTION            | 0x00BED150 | __TI1?AVZException@@; default `_CxxThrowException(&v6,&_TI1_AVZException__)` @0xa89388 |
| C_PATCH_EXCEPTION_BUILDER  | 0x0054154E | `??0CPatchException@@QAE@J@Z`; `CPatchException::CPatchException(v11, this->m_nTargetVersion)` @0xa89257 → qmemcpy 1288B @0xa89283 then throw CPatch |
| C_COM_RAISE_ERROR          | 0x00AFA623 | `?_com_raise_error@@YGXJPAUIErrorInfo@@@Z`; `_com_raise_error(m_hrComErrorCode, 0)` @0xa891e8 on this->m_hrComErrorCode |
| C_COM_RAISE_ERROR_EX       | 0x00AF9E44 | `?_com_issue_error@@YGXJ@Z`; `_com_issue_error(-2147467261)` @0xa89465 on FAILED render path (`if(!dword_CA4128)` after CallUpdate/RedrawInvalidatedWindows, before RenderFrame) |

Ranges (v87, IDENTICAL to v84/v83): Patch `==0x20000000` · Disconnect `0x21000000–0x21000006` · Terminate `0x22000000–0x2200000D` · else ZException.

Notes:
- v87 `CWvsApp::Run` @0xA88B81 (size 0x1028) is fully non-virtualized; the entire exception-dispatch block (m_hrComErrorCode + m_hrZExceptionCode checks + four `_CxxThrowException` calls) decompiles cleanly with all callees symbol-resolved. No VM thunk indirection (unlike v83).
- C_COM_RAISE_ERROR_EX is VERBATIM-CONFIRMED in v87: the FAILED-render HRESULT raiser is a discrete call `_com_issue_error(-2147467261)` @0xa89465, guarded by `if(!dword_CA4128)` between `CWvsApp::CallUpdate`/`CWndMan::RedrawInvalidatedWindows` and `IWzGr2D::RenderFrame`. This is NOT the v83 limitation — the render path is in Run directly, not buried in the CallUpdate VM.

## GMS v95.1 (port 13339) — Run @ 0x9C5F00
| Key | Address | Anchor |
|---|---|---|
| C_TI_DISCONNECT_EXCEPTION | 0x00BBD474 | __TI3?AVCDisconnectException@@; `push offset __TI3?AVCDisconnectException@@` @0x9c67a5 → `call __CxxThrowException@8` @0x9c67b1 (0x21000000–0x21000006) |
| C_TI_TERMINATE_EXCEPTION  | 0x00BB8F64 | __TI3?AVCTerminateException@@; `push offset __TI3?AVCTerminateException@@` @0x9c6819 → `call __CxxThrowException@8` @0x9c6825 (0x22000000–0x2200000E) |
| C_TI_PATCH_EXCEPTION       | 0x00BC9A34 | __TI3?AVCPatchException@@; `push offset __TI3?AVCPatchException@@` @0x9c6731 → `call __CxxThrowException@8` @0x9c673d (==0x20000000) |
| C_TI_ZEXCEPTION            | 0x00BB9228 | __TI1?AVZException@@; default `push offset __TI1?AVZException@@` @0x9c6856 → `call __CxxThrowException@8` @0x9c6862 |
| C_PATCH_EXCEPTION_BUILDER  | 0x00520FA0 | `??0CPatchException@@QAE@J@Z`; `mov ecx,[eax+54h]` (nTargetVersion) `push ecx` / `lea ecx,[ebp+var_CD8]` / `call ??0CPatchException@@QAE@J@Z` @0x9c6700 → `rep movsd` (0x142 dwords = 0x508B) @0x9c672f then throw CPatch |
| C_COM_RAISE_ERROR          | 0x00A2FDA0 | `?_com_raise_error@@YGXJPAUIErrorInfo@@@Z`; `push 0`/`push hr`/`call ?_com_raise_error@@...` @0x9c66c5 on ExtractComErrorCode/m_hrComErrorCode path |
| C_COM_RAISE_ERROR_EX       | 0x00A2FD00 | `?_com_issue_error@@YGXJ@Z` (= 1-arg HRESULT raiser); NOT a discrete call in Run. See concern below. |

Builder KIND: **1** — `??0CPatchException@@QAE@J@Z` is a `__thiscall` constructor invoked as `ctor(buffer=[ebp+var_CD8], version)` (`lea ecx,[ebp+var_CD8]; push <version>; call ??0CPatchException@@QAE@J@Z` @0x9c6700). It builds into a caller-owned stack buffer (var_CD8, size 0x508), which is then `rep movsd`-copied into pExceptionObject before the throw. Not a free function returning a pointer.

Ranges (v95): Patch `==0x20000000` · Disconnect `0x21000000–0x21000006` · Terminate `0x22000000–0x2200000E` · else ZException.
- NOTE: Terminate upper bound is **0x2200000E** in v95 (`cmp [ebp+hr],2200000Eh; jg`), vs 0x2200000D in v83/v84/v87. All other ranges identical.

Notes:
- v95 `CWvsApp::Run` @0x9C5F00 (size 0x1586) Hex-Rays decompilation FAILS, but the function body is fully non-virtualized and disassembles cleanly. The exception-dispatch block (ExtractComErrorCode→_com_raise_error, then ExtractZExceptionCode→4-way range dispatch with four `__CxxThrowException@8` calls) is contiguous at 0x9c66a6–0x9c6867 with all callees symbol-resolved. All four `_ThrowInfo` operands and all builder/ctor/dtor calls are verbatim from the listing.
- CONCERN on C_COM_RAISE_ERROR_EX (FLAGGED, same limitation as v83): v95 Run does NOT contain a discrete FAILED-render `_com_issue_error(hr)` call site. The render path is `CWvsApp::CallUpdate` @0x9c5360 (called from Run @0x9c6946); CallUpdate's callee list contains no `_com_issue_error`/`_com_raise_error*` — the FAILED-HRESULT raise is emitted inside the `_com_ptr_t<IWzGr2D>`/`UpdateCurrentTime` COM wrapper, not in Run/CallUpdate directly. `?_com_issue_error@@YGXJ@Z` @0xA2FD00 (the binary's canonical 1-arg HRESULT raiser, `_com_issue_error(hr)`) is recorded as the structural/semantic equivalent. This is NOT independently confirmed at a render-path call site in v95 (unlike v87, where it was verbatim).

## JMS v185.1 (port 13340) — Run @ 0x00AD8328 (virtualized; dispatch actually in CClientSocket::OnConnect @ 0x004B0066)

**MAJOR STRUCTURAL DIVERGENCE FROM GMS.** JMS v185 does NOT dispatch exceptions from `CWvsApp::Run` by an `m_hrZExceptionCode` range. `CWvsApp::Run` @0x00AD8328 is virtualized (jumps into the 0x00D2xxxx VM). The faithful version/login exception dispatch lives in `CClientSocket::OnConnect` @0x004B0066, keyed on the **version word decoded from the login-handshake packet**, not on an HRESULT range.

| Key | Address | Anchor |
|---|---|---|
| C_TI_DISCONNECT_EXCEPTION | 0x00BFB678 | __TI3?AVCDisconnectException@@; `push offset __TI3?AVCDisconnectException@@` → `call __CxxThrowException@8` @0x4b00d9 (code 0x21000001, connect-fail/no-more-addrs path) |
| C_TI_TERMINATE_EXCEPTION  | 0x00BF74B8 | __TI3?AVCTerminateException@@; throws @0x4b00b9 (code 0x22000001, bLogin path), @0x4b02e0 (0x22000007, recv-flag!=3), @0x4b0384 (0x22000007, version!=185) |
| C_TI_PATCH_EXCEPTION       | 0x00C07518 | __TI3?AVCPatchException@@; `push offset __TI3?AVCPatchException@@` → `call __CxxThrowException@8` @0x4b0320 (version>185) and @0x4b0362 (version==185 & data present) |
| C_TI_ZEXCEPTION            | 0x00BF7C38 | __TI1?AVZException@@; multiple buffer-underrun throws (code 38) @0x4b020b,0x4b0259,0x4b027a,0x4b029c and getpeername-fail @0x4b03cc |
| C_PATCH_EXCEPTION_BUILDER  | 0x0055127C | __thiscall builder; `movzx eax,ax; push eax; lea ecx,[ebp+var_FB0]; call sub_55127C` @0x4b02fc / @0x4b033e. Writes `*this=0x20000000`, `*(this+4)=185`, `*(this+3)=version`, fills strings; returns `this`. Result `rep movsd`-copied (0x142 dwords = 0x508B) into pExceptionObject before throw. |
| C_COM_RAISE_ERROR          | 0x00B4DCEF | ?_com_raise_error@@YGXJPAUIErrorInfo@@@Z — standard 2-arg `_com_raise_error(hr, IErrorInfo*=0)` comdef helper |
| C_COM_RAISE_ERROR_EX       | 0x00B4D510 | ?_com_issue_error@@YGXJ@Z — canonical 1-arg HRESULT raiser (`_com_issue_error(hr)`, tail-calls _com_raise_error(hr,0)). See concern below. |

Builder KIND: **1** — `sub_55127C` is a `__thiscall` invoked as `ctor(buffer, version)`: buffer/object pointer loaded into ECX (`lea ecx,[ebp+var_FB0]`) and version pushed on the stack (`push eax`), at 0x4b02fc and 0x4b033e. It returns `this` (the buffer). IDA types it `char *__thiscall sub_55127C(char *this, __int16 a2)`. Although it returns the object pointer, the calling convention is thiscall-ctor (buffer in ECX, not a stack arg), matching v83/v87/v95 KIND=1 — not the v84 free-function form.

Ranges (JMS185): **DIFFERENT SCHEME — dispatch is by VERSION WORD, not by HRESULT range.**
- `cmp ax, 0xB9` (0xB9 = 185): **version > 185** → CPatchException (`sub_55127C(version)`, code tag 0x20000000) @0x4b0320
- **version == 185** AND handshake data present (`Src != 0`) → CPatchException (`sub_55127C(185)`) @0x4b0362; sets `CWvsApp::m_nTargetVersion = 185` @0x4b0334
- **version != 185** (and not >185, i.e. version < 185) → CTerminateException (code 0x22000007) @0x4b0384
- recv control byte `!= 3` → CTerminateException (code 0x22000007) @0x4b02e0
- connect failure, login session → CTerminateException (code 0x22000001) @0x4b00b9; otherwise → CDisconnectException (code 0x21000001) @0x4b00d9
- packet buffer underruns / getpeername failure → ZException (code 38 / WSA error)

The embedded exception **code tags** (for any union/HRESULT calc): CPatch=0x20000000, CDisconnect=0x21000001, CTerminate=0x22000001 and 0x22000007, ZException uses 38 (0x26) or a raw WSA error code. NOTE: unlike GMS these are NOT contiguous ranges — they are discrete literals at fixed throw sites.

Notes / flags:
- **JMS-DIVERGENT (flag):** There is no `m_hrZExceptionCode`/`m_hrComErrorCode` range dispatch in a clean `CWvsApp::Run`. The 4 exception types still exist with identical C++ mangled names and the 4 `_ThrowInfo` globals are present and verbatim-confirmed by RTTI symbol. The PatchException builder and KIND are verbatim-confirmed from the OnConnect disassembly.
- **CONCERN on C_COM_RAISE_ERROR / C_COM_RAISE_ERROR_EX (FLAGGED):** Because `CWvsApp::Run` and the CallUpdate/render path are virtualized (0x00D2xxxx VM), the specific `m_hrComErrorCode` and FAILED-render call sites could NOT be resolved to discrete instructions in a Run body. Recorded values are the binary's canonical comdef helpers: 2-arg `?_com_raise_error@@YGXJPAUIErrorInfo@@@Z` @0x00B4DCEF (key 7, semantic `_com_raise_error(hr,0)`) and 1-arg `?_com_issue_error@@YGXJ@Z` @0x00B4D510 (key 8, the 1-arg HRESULT raiser). These are the correct semantic/structural equivalents but are NOT independently confirmed at a render-path call site in JMS v185 (same limitation as v83/v95, more severe here because Run itself is virtualized).

## GMS v111.1 (port 13342) — Run @ 0x00C07360

`CWvsApp::Run` @0x00C07360 (size 0x21BE) decompiles cleanly and is fully non-virtualized. The exception dispatch is contiguous at 0xc076f1–0xc077f4: `sub_C043E0(&hr)` extracts m_hrZExceptionCode, then a 4-way range dispatch with four `__CxxThrowException@8` calls. The m_hrComErrorCode path (`sub_C04410`/`sub_C91D40`) is just above at 0xc076d3–0xc076e2. All four `_ThrowInfo` operands, the builder ctor call, and the com raiser are verbatim from the listing.

| Key | Address | Anchor |
|---|---|---|
| C_TI_DISCONNECT_EXCEPTION | 0x00F20BB8 | __TI3?AVCDisconnectException@@; `push offset __TI3?AVCDisconnectException@@` @0xc07770 → `call __CxxThrowException@8` @0xc0777c (0x21000000–0x21000007) |
| C_TI_TERMINATE_EXCEPTION  | 0x00F1B8A4 | __TI3?AVCTerminateException@@; `push offset __TI3?AVCTerminateException@@` @0xc077b5 → `call __CxxThrowException@8` @0xc077c1 (0x22000000–0x2200000E) |
| C_TI_PATCH_EXCEPTION       | 0x00F30A04 | __TI3?AVCPatchException@@; `push offset __TI3?AVCPatchException@@` @0xc0772b → `call __CxxThrowException@8` @0xc07737 (==0x20000000) |
| C_TI_ZEXCEPTION            | 0x00F1BB68 | __TI1?AVZException@@; default `push offset __TI1?AVZException@@` @0xc077e8 → `call __CxxThrowException@8` @0xc077f4 |
| C_PATCH_EXCEPTION_BUILDER  | 0x00564AE0 | `sub_564AE0` __thiscall ctor; `mov edx,[var_E18]; mov eax,[edx+54h]` (m_nTargetVersion) `push eax` @0xc07710 / `lea ecx,[ebp+var_DD8]` @0xc07711 / `call sub_564AE0` @0xc07717 → `mov esi,eax; rep movsd` (0x142 dwords = 0x508B) @0xc07729 then throw CPatch. Body: `CMSException::CMSException(this,0x20000000)`, `*(this+4)=111`, `*(this+6)=version`. |
| C_COM_RAISE_ERROR          | 0x00C91D40 | `sub_C91D40` = `_com_raise_error(hr, IErrorInfo*)`; throws `_com_error` via `_CxxThrowException(&[off_E84CD0,hr,perrinfo,0], &__TI1?AV_com_error@@)`. Called `sub_C91D40(v48, 0)` @0xc076e2 on the m_hrComErrorCode path (`if(sub_C04410(&v48)) sub_C91D40(v48,0)`; sub_C04410 reads this[19]=m_hrComErrorCode). |
| C_COM_RAISE_ERROR_EX       | 0x00C91CC0 | `_com_issue_errorex` (3-arg); the FAILED-render raiser: `IWzGr2D::RenderFrame` @0x811420 does `result=(vtbl+28)(this); if(result<0) com_issue_errorex(result,this,...)` @0x811439. See flag. |

Builder KIND: **1** — `sub_564AE0` is a `__thiscall` constructor invoked as `ctor(buffer, version)`: buffer pointer loaded into ECX (`lea ecx,[ebp+var_DD8]` @0xc07711) and the version pushed on the stack (`push eax` @0xc07710, eax = `[m_nTargetVersion]`), then `call sub_564AE0` @0xc07717. It constructs the CPatchException/CMSException in place (`CMSException::CMSException(this,0x20000000)`) and returns `this` in EAX, which is `rep movsd`-copied (0x142 dwords) into `pExceptionObject` before the throw. Buffer-in-ECX + version-on-stack = thiscall-ctor form (matches v83/v87/v95/JMS KIND=1), NOT the v84 free-function form. IDA types it `int __thiscall sub_564AE0(int this, __int16 a2)`.

Ranges (v111): Patch `==0x20000000` (`cmp [var_6C],20000000h; jnz` @0xc076ff) · Disconnect `0x21000000–0x21000007` (`cmp 21000000h; jl` @0xc07747 / `cmp 21000007h; jg` @0xc07750) · Terminate `0x22000000–0x2200000E` (`cmp 22000000h; jl` @0xc0778c / `cmp 2200000Eh; jg` @0xc07795) · else ZException. All bounds read verbatim from disasm.
- NOTE: Terminate upper bound is **0x2200000E** in v111 (same as v95; vs 0x2200000D in v83/v84/v87). Disconnect upper bound is **0x21000007** (v95 was 0x21000006). Patch and lower bounds identical across all versions.

Flags:
- **C_COM_RAISE_ERROR_EX (FLAG):** Unlike v83/v95/JMS, v111 HAS a verbatim-confirmed FAILED-render raiser call site — `IWzGr2D::RenderFrame` @0x811420 (reached from Run @0xc078d8) calls it on `result < 0`. However the callee is the **3-arg** `_com_issue_errorex` @0x00C91CC0 (`com_issue_errorex(hr, pSource, pIID)`), NOT the 1-arg `_com_issue_error` used in other GMS versions. The binary's canonical 1-arg `_com_issue_error` exists at **0x00C91CA0** (`com_issue_error(hr) → _com_raise_error(hr,0)`); both tail into `sub_C91D40` (off_1000AE8). Recorded the actual render-path callee (0x00C91CC0) since the task key is "the raiser on the FAILED-render path"; if strict 1-arg semantics are preferred, swap to 0x00C91CA0.
- C_COM_RAISE_ERROR (key 7) is verbatim-confirmed at a discrete Run call site (`sub_C91D40(v48,0)` @0xc076e2) — no concern, unlike v83/v95/JMS.
