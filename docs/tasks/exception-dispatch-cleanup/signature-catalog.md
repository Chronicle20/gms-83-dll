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
