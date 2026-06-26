# GMS v79 — Function-Identification Signature Catalog

**Purpose.** v79 is not the last version we will port this way — and as the first
*below-floor* port it is the template for porting even older builds. For each
non-trivial function we relocate, this catalog records *how* we found it in a
sparsely-labeled v79 IDB so the next (possibly older) port can reuse the heuristic
instead of re-discovering it. Prefer version-stable anchors (strings, imports,
call-graph, constants) over raw byte signatures, which break when codegen
changes — and the v79↔v83 gap is larger than the v83↔v84 gap, so byte sigs are
*less* reliable here.

This file is populated during the port (it starts mostly empty by design). It is
a durable, version-agnostic artifact — keep entries phrased about the *function*,
not about a specific address. Where a v83 anchor (string/constant) has drifted in
the older v79 build, **record the v79-specific anchor** so the next backward port
benefits.

The shared, version-agnostic anchors discovered during the v84 port (WinMain via
the IExplorer-title string, SendHSLog via `%s\HShield`, CWvsApp::CWvsApp via the
`WebStart`/`IsWow64Process` cluster, etc.) live in
`docs/tasks/task-006-gms-v84-support/signature-catalog.md`. **Consult it first** —
most entries should transfer to v79; this file records the v79 deltas and any new
anchors needed when the v83/v84 heuristic fails on the older build.

## How to use

- When you resolve a key, add or update its row below.
- Record the **most stable** anchor that actually worked, plus a fallback.
- Note observed stability across versions you've checked (e.g. "string present in
  v79, v83, and v84"; "byte sig matched v83 but not v79").
- If you also want the heuristic encoded in code/comments, put a short comment at
  the relevant patch site in source referencing this catalog entry.

## Entry schema

```
### <CANONICAL_FUNCTION_NAME>   (memory-map key: <KEY>)
- Primary anchor: <string xref | import call | call-graph | constant | vtable slot | byte sig>
- Detail: <the exact literal / API / parent / constant / slot index used>
- Fallback anchor: <secondary method if primary is absent>
- Cross-version stability: <which versions confirmed; known breakages; v79-vs-v83 drift>
- v79 address: <0x… (named in IDB)>
- Notes: <pitfalls, ambiguity, near-duplicates to disambiguate>
```

## v79 IDB baseline

Confirmed at task-1 setup (2026-06-26):

- **Module:** `GMS_v79_1_DEVM.exe`
- **IDB path:** `E:\Programs\Nexon\IDBs_v9\GMS\v79\GMS_v79_1_DEVM.exe.i64`
- **Image base:** `0x400000`
- **Image size:** `0x990000`
- **MD5:** `718a1f2d3493acce0e81637f2faa9e99`
- **SHA-256:** `07cc813351ba2d288c0dec096a761ad990e2af8c8f909a8254c8e3aa1776f970`
- **Segments:** `.text` 0x401000–0xa2b000 (rx), `.rdata` 0xa2b3a4–0xabd000 (r), `.data` 0xabd000–0xb18000 (rw)
- **Named functions:** 2372 of 47317 total
- **IDA MCP port:** 13339

WinMain offset math reference: `WIN_MAIN = image_base + RVA`. With base `0x400000`,
a WinMain at RVA `0x5B19F2` would be VA `0x9B19F2` (adjust once confirmed in later tasks).
Baseline IDB saved clean before any renaming or annotation.

## Catalogued anchors (populate during port)

_(empty — fill as keys are resolved; port the v84 catalog's entries forward and
record v79 deltas here)_
</content>
