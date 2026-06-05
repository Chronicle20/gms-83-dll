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

### VERDICTS — Task 11 read-only audit (v84 IDB = `GMS_v84.1_U_DEVM.exe`, port 13341)

All five gates audited against v84 disassembly, cross-anchored vs v83 (port 13337)
and v87 (port 13338). **All five gates are CORRECT for v84 — no rewrites required.**
v84 sides with v87 on gates 2/3/4/5 and with v87 (the fixed side) on gate 1.

| # | Site | v84 finding | Deciding v84 disasm line | Verdict |
|---|---|---|---|---|
| 1 | `doom-fix/dllmain.cpp:25` (`< 84`) | v84 `CMob::CMob` **zero-inits `m_bDoomReserved`** (int @ this+0x540), like v87. The v83 ctor SKIPS it (the bug). The manual `m_bDoomReserved=0` fixup is NOT needed in v84. | `0x6782ad  mov [esi+540h], ebx` (ebx=0 → m_bDoomReserved=0), immediately followed by `0x6782b3  mov [esi+544h], bl` (m_bDoomReservedSN=0). CMob::CMob @ 0x00678060. | **CORRECT** — v84 on the fixed (`>=84`/no-fix) side. Header comment "fixed in v84" is accurate. |
| 2 | `common/CWvsContext.h:98` (`> 83`) | v84 CWvsContext **carries `m_aClientKey[8]`** at the singleton + 0x20A0 (8352). `m_dwCharacterId` follows at +0x20A8 (8360), confirming the 8-byte key precedes it (matches header order). | (shared with #3) v84 OnConnect @ 0x00499DCD: `EncodeBuffer((void *)(dword_C40C68 + 8352), 8u)`; `dword_C40C68` = `CWvsContext::ms_pInstance`. | **CORRECT** — `>83` is true for v84 → key present. |
| 3 | `bypass/socket_hooks.cpp:233` (`> 83`) | **SAME FACT as #2.** v84's connect-hello (PLAYER_LOGGED_IN, opcode 20) encodes the 8-byte client key after Encode4(characterId)+Encode1(subgrade)+Encode1(0), exactly the `>83` form. | `CClientSocket::OnConnect` @ 0x00499DCD, else-branch: `COutPacket(...,20)` → `Encode4(+8360)` → `Encode1` → `Encode1(0)` → `EncodeBuffer((dword_C40C68 + 8352), 8u)` → `SendPacket`. | **CORRECT** — `>83` true → encodes key. Reconciled with #2 (one layout fact). |
| 4 | `common/CLogin.h:235` (`== 83`) | The v83-only 20-byte member between `m_abOnFamily` and `m_lNewEquip` is **ABSENT in v84**. v84: `m_abOnFamily` @ this+0x1B0, `m_lNewEquip` ZList @ this+0x1B4 (4-byte gap = adjacent). v83 has an extra 20-byte member (a ZList, IDA-named `unk`, not literally `int[5]`) between them; v87 does not. v84 matches v87. | v84 `CLogin::CLogin` @ 0x00608B15: m_abOnFamily store + ZList init `0x608bf7 mov dword ptr [eax], offset off_B4811C` at this+0x1B4 (eax=this+0x1B4); unwind entries loc_AE5DBD(+0x1B0)/loc_AE5DCB(+0x1B4 → ZList dtor). Contrast v83 ctor @ 0x5F3C59 which inits `unk[0..4]` between m_abOnFamily and m_lNewEquip. | **CORRECT** — `==83` false for v84 → member excluded. (Modeling note for Task 16: the v83-only member is a 20-byte ZList, not `int unk3[5]`; same size, so no offset error — but rename for accuracy if touched.) |
| 5 | `common/CUIToolTip.h:92` (`>= 83`) | v84 CUIToolTip **has `m_pLayerAdditional`** at this+0x14 (com_ptr immediately after `m_pLayer` @ this+0x10). Ctor prologue is byte-identical to v87. | v84 `CUIToolTip::CUIToolTip` @ 0x0091A417: `0x91a42e mov [esi+10h], edi` (m_pLayer=0), `0x91a434 mov [esi+14h], edi` (m_pLayerAdditional=0), then eh-vector ctor for `m_aLineInfo[32]` at this+0x24 (`0x91a445 lea eax,[esi+24h]`, count 0x20). | **CORRECT** — `>=83` true for v84 → field present. |

**Doom-field identity (settled):** Task 7's `this+0x18C` (this+99) is `m_pTemplateByDoom`
(the pointer adjacent to `m_pTemplate` @ this+0x188) — NOT the doom-fix target.
The doom-fix's `m_bDoomReserved` is a distinct, much-later int field @ this+0x540,
sitting between `m_bWaitingToBeSetTossed` and `m_bDoomReservedSN`. Confirmed by the
v83 ctor (@ 0x5F3C59) jumping `m_bWaitingToBeSetTossed=0` → `m_bDoomReservedSN=0`,
skipping `m_bDoomReserved` (the v83 bug), and the v87 ctor (@ 0x69C5F6) explicitly
`this->m_bDoomReserved = 0;` right after `m_delaySkill[3]=0` (the fix). v84 reproduces
the v87 fix at offset 0x540.

**Task-16 cross-check flags:** None of the 5 gates need a boundary change.
The only follow-up is cosmetic: Gate 4's v83-only member should be remodeled in the
header as a 20-byte ZList rather than `int unk3[5]` (size is correct either way; not
a gate-correctness issue).

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
