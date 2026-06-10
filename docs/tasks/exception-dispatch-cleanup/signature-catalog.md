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
