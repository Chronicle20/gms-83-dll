# Reusable Function-Identification Signature Catalog

**Purpose.** v84 is not the last version we will port this way. For each
non-trivial function we relocate, this catalog records *how* we found it in a
sparsely-labeled IDB so the next version port can reuse the heuristic instead of
re-discovering it. Prefer version-stable anchors (strings, imports, call-graph,
constants) over raw byte signatures, which break when codegen changes.

This file is populated during the port (it starts mostly empty by design). It is
a durable, version-agnostic artifact — keep entries phrased about the *function*,
not about a specific address.

## How to use

- When you resolve a key, add or update its row below.
- Record the **most stable** anchor that actually worked, plus a fallback.
- Note observed stability across versions you've checked (e.g. "string present in
  v83 and v84"; "byte sig matched v83 but not v87").
- If you also want the heuristic encoded in code/comments, put a short comment at
  the relevant patch site in source referencing this catalog entry.

## Entry schema

```
### <CANONICAL_FUNCTION_NAME>   (memory-map key: <KEY>)
- Primary anchor: <string xref | import call | call-graph | constant | vtable slot | byte sig>
- Detail: <the exact literal / API / parent / constant / slot index used>
- Fallback anchor: <secondary method if primary is absent>
- Cross-version stability: <which versions confirmed; known breakages>
- Notes: <pitfalls, ambiguity, near-duplicates to disambiguate>
```

## Catalogued anchors (populate during port)

### WinMain   (memory-map key: WIN_MAIN)
- Primary anchor: _TBD_
- Detail: _TBD_
- Fallback anchor: _TBD_
- Cross-version stability: _TBD_
- Notes: Entry point per PE header; anchors the whole image. Resolve first.

### CWvsApp::Run   (memory-map key: C_WVS_APP_RUN)
- Primary anchor: _TBD_
- Detail: _TBD_
- Fallback anchor: _TBD_
- Cross-version stability: _TBD_
- Notes: Main message-pump loop; reachable from WinMain.

<!--
Add one section per resolved key. Group loosely by subsystem to mirror
memory-map.md. Leave the two stubs above as worked examples of the schema.
-->

## v84 IDB baseline

Recorded 2026-06-05 via `mcp__ida-pro__survey_binary` with instance routed to port 13341.

| Field | Value |
|---|---|
| Module | `GMS_v84.1_U_DEVM.exe` |
| IDB path | `E:\Programs\Nexon\IDBs_v9\GMS\v84_1\GMS_v84.1_U_DEVM.i64` |
| Image base | `0x00400000` |
| Architecture | x86 32-bit |
| Image size | `0x00BC6000` |
| Entry point (`start`) | `0x00AB01AF` |
| `.text` segment | `0x00401000` – `0x00B41000` (size `0x740000`) |
| Total functions | 59385 (249 named, 550 library, 58586 unnamed) |

WinMain offset math anchor: image base `0x00400000`; subtract from any absolute address
to get the RVA used in `WIN_MAIN` and sibling keys.

## Heuristic playbook (general, version-agnostic)

- **String xrefs are king.** Format strings, file paths ("Data\\…"), registry
  keys, and error literals survive across versions far better than code bytes.
- **Manager singletons** (`*::CreateInstance` / `*_INSTANCE_ADDR`) cluster
  together in the static-init region and are typically called in a fixed order
  from one initializer — find one, the neighbors are adjacent calls.
- **`*_INSTANCE_ADDR` globals** are the `mov [g_xxx], eax` store right after the
  matching `CreateInstance` call returns. Resolve the function, read its
  caller's store target.
- **Packet senders** reference their opcode as a `push <imm>` / `mov` immediate
  into a COutPacket; the opcode constant disambiguates among similar senders.
- **`*_OFFSET` keys** are call-site or branch offsets *within* a host function;
  always re-measure against v84 disassembly.
- **Vtable-derived keys** (`C_LOGO_GET_RTTI`, `*_VFTABLE`): find the class
  vtable via its RTTI/type string, then index the slot.
- When two candidates are indistinguishable, disambiguate by a unique callee or
  by the surrounding string the real one references — don't guess by address
  proximity to v83 alone.
