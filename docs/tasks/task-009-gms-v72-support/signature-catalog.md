# GMS v72.1 — Signature / Pattern Catalog

A durable, reusable record of how each non-trivial function/global was located in the v72
binary. v72 is not the last version we will port this way, so each entry documents the
heuristic and its robustness across the v83 → v79 → v72 chain. This catalog both **extends**
task-008's v79 catalog and **consumes** it: for each function, start from the v79 heuristic
(`docs/tasks/task-008-gms-v79-support/signature-catalog.md`) and note whether it ported
directly or drifted in the older build.

## How to use this catalog

- One entry per resolved function/global backing a memory-map key (and per non-trivial
  struct-verification anchor).
- Record the **v72 address**, the **heuristic** used, the **v79 starting point**
  (task-008's entry), and a **drift note** (did the string/constant/codegen survive v79→v72?).
- Prefer heuristics in this robustness order: string xref > import/API anchor > call-graph
  anchor > constant/opcode > byte signature.

## Entry template

```
### <CanonicalName> (key: <MEMORY_MAP_KEY>)
- v72 address: 0x________
- v79 address (task-008): 0x________
- Heuristic: <string xref "…" / import anchor / call-graph child of … / push <imm> / byte sig>
- Drift v79→v72: <direct | string changed to "…" | inlined differently | split | not found, see note>
- Label applied: <yes/no> (rename + set_type if proto known)
- Notes: <anything that will help the next, older port>
```

## v72 IDB identity (confirmed at task-1 baseline)

| Field | Value |
|-------|-------|
| Module | `GMS_v72.1_U_DEVM.exe` |
| IDB path | `E:\Programs\Nexon\IDBs_v9\GMS\v72\GMS_v72.1_U_DEVM.exe.i64` |
| IDA port | 13343 |
| Image base | `0x400000` |
| Image size | `0x86f000` |
| MD5 | `05a62ca755b1d3719223426b4eee41a9` |
| SHA256 | `a989875b85668cf1d62ad4eede948da0357b36075740a1db1f6255c943281a96` |
| Total functions | 44,435 (named: 2,289; library: 503; unnamed: 41,643) — sparse/unverified IDB |
| `.text` segment | `0x401000`–`0x9cf000` (`0x5ce000` bytes, rx) |
| `.data` segment | `0xa59000`–`0xaae000` (`0x55000` bytes, rw) |
| Entry `start` | `0x955da3` |

WinMain offset math anchor: image base `0x400000`. All v72 addresses are in the `0x40xxxx`–`0x9cxxxx` range. Baseline IDB saved at task-1 start (no labels applied yet).

## Catalog (fill during the port, grouped by subsystem)

### CWvsApp lifecycle
_(entries: C_WVS_APP_RUN, _INITIALIZE_*, _SET_UP, window/dir helpers, …)_

### CClientSocket / ZSocket
_(entries: instance addr, CreateInstance, SendPacket, Flush, OnConnect, Process, connect path)_

### COutPacket
_(entries: ctor/Init, Encode1/2/4/buffer, …)_

### Login / Stage / Logo / Title
_(entries: CLogin, CStage, CLogo, CUITitle, CUILoginStart, …)_

### Config / Input / FuncKeyMappedMan
_(entries: CConfig ctor + ApplySysOpt, CInputSystem, CFuncKeyMappedMan CreateInstance + ctor)_

### Manager singletons
_(entries: *_CREATE_INSTANCE / *_INSTANCE_ADDR — ActionMan, AnimationDisplayer, etc.)_

### WinMain + offsets
_(entries: WIN_MAIN, AD_BALLOON_CONDITIONAL offset, PATCHER offset, SEND_HS_LOG)_

### Party / migrate senders
_(entries: senders + call-site offsets)_

### Utilities
_(entries: ZArray::RemoveAll, ZXString trim/get-buffer, fatal section ctor/dtor, CSystemInfo, CIGCipher)_

### Exception dispatch
_(entries: C_TI_*EXCEPTION, C_PATCH_EXCEPTION_BUILDER, C_COM_RAISE_ERROR_EX, C_FILE_STREAM_*)_

### Protocol constants (FR-6)
_(VERSION_HEADER, PLAYER_LOGGED_IN, CLIENT_START_ERROR — record the deciding v72 disasm
site, e.g. the OnConnect `if (hdr != N)` compare and the COutPacket opcode immediates;
note any drift from v79's confirmed VERSION_HEADER=8)_

### Sentinels (confirmed absent in v72)
_(DR_CHECK, DR_INIT, CE_TRACER_RUN, C_BATTLE_RECORD_MAN_CREATE_INSTANCE, JMS-only keys, and
any new v72-only sentinels — record the evidence of absence, not just the `0x0`)_

## Cross-version drift summary (fill at end)

A short table of which heuristic classes survived v79→v72 best, to guide the next (older)
port: e.g. "string xrefs: N/N held; byte sigs: M/K held; constants: …". This is the durable
takeaway that makes the v6x port faster.
</content>
