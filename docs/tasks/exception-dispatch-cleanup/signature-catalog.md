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
