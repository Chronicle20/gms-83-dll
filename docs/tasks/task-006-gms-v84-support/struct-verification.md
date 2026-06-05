# GMS v84.1 — Struct Size/Layout Verification & Boundary-Gate Audit

Scope (per confirmed PRD): verify **all 22 version-gated `common/*.h` headers**
against the v84 binary, and empirically verify/correct every 83/84 boundary gate.

Follow the read-only verification discipline from
`docs/version-porting-workflow.md`: anchor every verdict to v84 disassembly, and
do **not** apply speculative struct types into the IDB during verification
(decompiler leak). Confirm the connected IDB with `get_metadata` first
([[feedback-verify-ida-target]]). Do not defer a settleable question with "out of
scope"/"unlikely to matter" ([[feedback-prefer-confirmation]]).

## The 83/84 boundary gates (verify first — highest risk)

These already branch at the 83/84 line and currently assume v84 takes the
*newer* path. Each is a hypothesis until proven against v84.

| Site | Current gate | v84 truth value (current) | Must confirm |
|---|---|---|---|
| `doom-fix/dllmain.cpp:25` | `BUILD_MAJOR_VERSION < 84` | false → v87+ branch | Does v84 belong on the `>= 84` side here? |
| `common/CWvsContext.h:98` | `BUILD_MAJOR_VERSION > 83` | true → v87+ layout | Does v84 CWvsContext have the `> 83` field(s)? |
| `bypass/socket_hooks.cpp:233` | `BUILD_MAJOR_VERSION > 83` | true → v87+ behavior | Does v84's socket path match the `> 83` form? |
| `common/CLogin.h:235` | `BUILD_MAJOR_VERSION == 83` | false | Is the v83-only member truly absent in v84? |
| `common/CUIToolTip.h:92` | `BUILD_MAJOR_VERSION >= 83` | true | Confirm v84 still on this side |

For each: disassemble the relevant v84 function(s), determine the actual
behavior/layout, and either (a) confirm the current gate is correct for v84, or
(b) rewrite the gate to a correct boundary. If v84 needs to split from both v83
and v87, introduce explicit `== 84` / `>= 84` boundaries. **Every rewrite must be
re-validated against v83/v87/v95/v111/JMS185** so no other version's compiled
branch flips incorrectly (FR-13).

## The 22 version-gated headers

For each, record: v84 size, which gated fields are present/absent in v84, the
disassembly anchor, and the verdict (gate correct / gate needs change).

| Header | Existing thresholds in file | v84 size | Verdict | Evidence |
|---|---|---|---|---|
| common/CWvsApp.h | _audit_ | | ☐ | |
| common/CWvsContext.h | `> 83`, `>= 95` | | ☐ | boundary gate |
| common/CClientSocket.h | _audit_ | | ☐ | |
| common/CLogin.h | `== 83` | | ☐ | boundary gate |
| common/CMapLoadable.h | _audit_ | | ☐ | README-critical |
| common/CLogo.h | _audit_ | | ☐ | |
| common/CMob.h | _audit_ | | ☐ | |
| common/MobStat.h | _audit_ | | ☐ | |
| common/ConfigSysOpt.h | _audit_ | | ☐ | |
| common/CFuncKeyMappedMan.h | _audit_ | | ☐ | |
| common/CFadeWnd.h | _audit_ | | ☐ | |
| common/CCtrlButton.h | _audit_ | | ☐ | |
| common/CCtrlCheckBox.h | _audit_ | | ☐ | |
| common/CUIWnd.h | _audit_ | | ☐ | |
| common/CWnd.h | _audit_ | | ☐ | |
| common/CUIToolTip.h | `>= 83` | | ☐ | boundary gate |
| common/CUILoginStart.h | _audit_ | | ☐ | |
| common/CUITitle.h | _audit_ | | ☐ | |
| common/GuildData.h | `>= 95` | | ☐ | |
| common/PartyMember.h | `< 95` | | ☐ | |
| common/PartyData.h | `>= 95` | | ☐ | |
| common/SecondaryStat.h | `>= 95` | | ☐ | embedded UDT; size-critical |

(Threshold column is filled in during the audit by grepping each file; the four
boundary-gate rows above are pre-flagged.)

## How to find a v84 struct size (from the workflow runbook)

- A `Decode`/`DecodeBuffer(this, <N>)` literal byte count = struct size.
- `ZArray<T>::RemoveAll` uses `imul stride, <N>` where `<N>` = `sizeof(T)`.
- Destructor unwind tables bound the total layout extent.
- Verify **embedded UDT** sizes independently (they often shrink pre-v95);
  don't assume v84 == v87 == v95.

## Output

A completed verdict table above plus, for any corrected gate, a short rationale
with the disassembly line that forced the change. Spot-check boundary cases in
the main session before trusting any subagent's summary numbers (per the
workflow's "subagent cross-validation discipline").
