# GMS v61 — Signature Catalog

Companion to `memory-map.md`. Records the IDB identity anchor, then one entry per
relocated key: the heuristic used, the v61 address found, and any notable observations.
Seeded at Task 1; entries added as Tasks 2–19 relocate each cluster.

---

## IDB identity anchor (Task 1 baseline)

Confirmed via `mcp__ida-pro__survey_binary` on port 13344 (2026-06-27).

| Field | Value |
|---|---|
| Module name | `GMS_v61.1_U_DEVM.exe` |
| IDB path | `E:\Programs\Nexon\IDBs_v9\GMS\v61\GMS_v61.1_U_DEVM.exe.i64` |
| Image base | `0x00400000` |
| Image size | `0x00796000` |
| Architecture | x86 32-bit |
| MD5 | `122419f1ab2607a02d585f0b87ead8ce` |
| SHA256 | `7f0caf8cf7c375c8c91514f40fb21086f362bab463af791ef994a8a7bd59b6cd` |
| Total functions | 35,132 (named: 1,977 / library: 480 / unnamed: 32,675) |
| Total segments | 6 |
| Baseline IDB save | `E:\Programs\Nexon\IDBs_v9\GMS\v61\GMS_v61.1_U_DEVM.exe.i64` (Task 1) |

### Segment layout

| Segment | Start | End | Size | Permissions |
|---|---|---|---|---|
| `.text` | `0x401000` | `0x8e5000` | `0x4e4000` | rx |
| `.idata` | `0x8e5000` | `0x8e5458` | `0x000458` | r |
| `.rdata` | `0x8e5458` | `0x960000` | `0x07aba8` | r |
| `.data` | `0x960000` | `0x97f000` | `0x01f000` | rw |
| `.export` | `0xa2c000` | `0xb94000` | `0x168000` | r |
| `.import` | `0xb94000` | `0xb96000` | `0x002000` | r |

### Entry points

| Address | Name | Ordinal |
|---|---|---|
| `0x820db3` | `ZtlTaskMemAllocImp` | 1 |
| `0x820dc4` | `ZtlTaskMemFreeImp` | 2 |
| `0x820dd5` | `ZtlTaskMemReallocImp` | 3 |
| `0x87921f` | `start` | 8884767 |

### Packing verdict

**pre-Themida** — standard PE binary. Evidence:
- Named, conventional section headers (`.text`, `.rdata`, `.data`, etc.)
- Full, resolved import table (kernel32, user32, ws2_32, advapi32, npkcrypt, wzmss, etc.)
- Named export entry points with known symbols
- No `.themida`, `.oreans`, `.vmp`, or virtualized-code sections
- No `NtGetContextThread` import (consistent with pre-DR era; DR anti-debug absent in v61, same as v72)
- `npkcrypt` imported (NProtect/HackShield era-appropriate for v61)

> Note: The v48 DISTRACTOR (port 13345, `GMS_v48_1_DEVM.exe`) must never be confused
> with this target. Always confirm via `list_instances` / `survey_binary` module name.

---

## Per-key signatures

*(To be filled in as Tasks 2–19 relocate each cluster. One sub-section per cluster.)*

### Cluster 1 — WinMain / CWvsApp lifecycle (Tasks 2–3)

*(pending)*

### Cluster 2 — CClientSocket / ZSocket / COutPacket (Tasks 4–5)

*(pending)*

### Cluster 3 — CConfig / windowed-mode (Task 6)*

*(pending)*

### Cluster 4 — CLogin / CLogo / stage flow (Task 7)

*(pending)*

### Cluster 5 — Manager singletons (Task 8)

*(pending)*

### Cluster 6 — Party / migrate senders (Task 9)

*(pending)*

### Cluster 7 — Misc utils / exception dispatch (Task 10)

*(pending)*

### Cluster 8 — CFileStream relay (Task 11)

*(pending)*

### Cluster 9 — Gate confirm/split (Task 12)

*(pending)*
