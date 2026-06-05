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

### VERDICTS — Task 12 read-only audit (4 core layout headers)

v84 IDB = `GMS_v84.1_U_DEVM.exe` (port 13341, confirmed active before every probe).
Cross-anchored vs v83 (port 13337). All gates in these 4 headers resolve to v84
taking the **v83-side branch** (build 84 < 87/95/111; the `>83` key is present;
the `==83` member is absent). **No gate needs a boundary change.**

Sizes confirmed identical v83↔v84 for CClientSocket (0x94), CLogin (0x28C), and the
early CWvsApp layout (0x60) — exactly what the gates predict when v84 takes the
v83 side. (CWvsApp/CWvsContext are static/stack objects, not heap-allocated, so
their size comes from the ctor/dtor extent, not a `new` immediate.)

#### CWvsApp.h — gates `>= 87`, `>= 95` → both FALSE for v84
- **Layout note:** the `>=87` fields (`m_tNextSecurityCheck`, `m_pBackupBuffer`
  ZArray, `m_dwBackupBufferSize`) are trailing. The `>=95` OS-version block
  (lines 12-18: `m_nOSVersion … m_b64BitInfo`) is **mid-struct** (between
  `m_bWin9x` and `m_tUpdateTime`), NOT trailing; the remaining `>=95` fields
  (`m_bEnabledDX9`, `m_dwClearStackLog`, `m_bWindowActive`) are trailing. All are
  correctly gated `>=87`/`>=95` → compiled out for v84 (build < 87), so no shift
  occurs in the v84 build. (Heads-up for any future v84-side enable: the
  mid-struct `>=95` block WOULD shift later fields — the v84 safety here rests on
  the gate excluding it, confirmed by the contiguous-ctor evidence below, not on
  it being a tail member.)
- **v84 anchor:** ctor `CWvsApp::CWvsApp` @ 0x00A3D719 stores the singleton
  (`dword_C40E88`) and zero-inits the early scalar block through `this+0x38`
  (`*((_DWORD*)this+14)=0`), byte-for-byte matching the v83 ctor
  (`??0CWvsApp@@QAE@PBD@Z`). Both versions **stack-construct** CWvsApp in WinMain
  (v84 `lea ecx,[ebp+var_FC]` @ 0xA3A224; v83 `lea ecx,[ebp+var_F8]` @ 0x9F1C4E) —
  no alloc immediate exists. Highest CWvsApp field touched via the singleton is
  `+0x40` (`*(g_CWvsApp_ms_pInstance+64)=84`, the version stamp, in OnConnect
  @ 0x49A0F8) — consistent with a 0x60 v83-side struct.
- **Verdict:** `>=87` CORRECT (v84 absent / v87+ present); `>=95` CORRECT
  (v84 absent / v95+ present).

#### CWvsContext.h — gates `> 83`, `>= 87`, `>= 95`
- **`>83` `m_aClientKey[8]` — PRESENT in v84 (was Gate 2/3 in Task 11).**
  v84 OnConnect @ 0x00499DCD else-branch (PLAYER_LOGGED_IN, opcode 20):
  `Encode4(*(dword_C40C68 + 8360))` = `m_dwCharacterId` @ singleton+0x20A8;
  `EncodeBuffer((dword_C40C68 + 8352), 8u)` = `m_aClientKey[8]` @ +0x20A0.
- **`>= 87` `m_bTesterAccount` — ABSENT in v84 (the trap).** The key ends at
  0x20A0+8 = 0x20A8, and `m_dwCharacterId` is exactly at 0x20A8 — a 0-byte gap.
  If the `>=87` `int m_bTesterAccount` existed between them, `m_dwCharacterId`
  would be at 0x20AC. It is not → the `>=87` field is excluded. (The `>=87`
  `m_bPetHelpPopUpShown` and the `>=87` equip-array widening, plus all `>=95`
  blocks, are likewise compiled out for build 84.) Same OnConnect anchor.
- **v84 size:** CWvsContext is a **static object** @ 0xC4B2A8 (the dtor sub_A4C8CD
  is invoked with `ecx = offset unk_C4B2A8`). Its ctor (sub_A4BF0B) inits fields
  up to `[esi+0x391C]` (last field @ 0x391C → struct ≥ 0x3920); the dtor
  destroys members up to `+0x38D0`. Next named global is `hObject` @ 0xC4F218
  (gap 0x3F70), an upper bound that includes trailing/adjacent unnamed BSS.
  Best estimate **~0x3920 (≈14624 B)**; SetUp reads `+14400` (0x3840) and OnEnterGame
  reads `this+3560` (0x37A0), both within range.
- **Verdict:** `>83` CORRECT (present); `>=87` CORRECT (absent / v87+ present);
  `>=95` CORRECT (absent / v95+ present).

#### CClientSocket.h — gate `>= 111` → FALSE for v84
- **`>= 111` `int dummy1` — ABSENT in v84.** This field sits mid-struct (between
  `m_addr` and `m_tTimeout`); had v84 carried it, the struct would be 0x98.
  v84 `TSingleton<CClientSocket>::CreateInstance` @ 0x00A43D0B allocates with
  `push 94h` (0x94 = 148 B) immediately before `call ??0CClientSocket@@QAE@XZ`
  (ctor @ 0x499610). v83 CreateInstance @ 0x9F9E53 also pushes `0x94` — **sizes
  identical**, proving the `>=111` field is excluded in v84.
- **Verdict:** `>=111` CORRECT (v84 absent / v111+ present).

#### CLogin.h — gates `>= 95`, `== 83`
- **`== 83` member — ABSENT in v84 (was Gate 4 in Task 11).** v84 ctor @ 0x608B15:
  `mov [esi+1B0h], ebx` = `m_abOnFamily` (ZArray, 4 B) @ 0x1B0, immediately
  followed by `lea eax,[esi+1B4h]; mov [eax], offset off_B4811C` = `m_lNewEquip`
  (ZList) @ 0x1B4 — adjacent, no 20-byte member between. Contrast v83 ctor
  @ 0x5F3C59, which builds a **20-byte ZList @ 0x1A0** (vtable + members at
  +8/+0xC/+0x10) before the `m_lNewEquip` ZList @ 0x1B4 — that v83-only ZList is
  exactly what the header's `#if ==83 int unk3[5]` (20 B) models. In both versions
  `m_lNewEquip` lands at 0x1B4 and the total is 0x28C, so removing the member in
  v84 keeps the layout consistent.
- **`>= 95` blocks — ABSENT in v84.** v84 and v83 CLogin are both **0x28C** (652 B):
  v84 alloc `push 28Ch` @ 0xA6FD1C (before `call CLogin::CLogin`); v83 alloc
  `push 28Ch` @ 0x62EED5 / 0xA2469B. The `>=95` fields (`m_pLayerBook`,
  `m_nFadeOutLoginStep`, `m_nBuyCharCount`, `m_nCurSelectedSubJob`, `m_bCharSale`,
  `m_nCharSaleJob`, `m_bCanHaveExtraChar`) are excluded for build 84.
- **Verdict:** `==83` CORRECT (absent / v83 present); `>=95` CORRECT (absent /
  v95+ present). (Task-11 modeling note stands: the v83-only member is a 20-byte
  ZList, not literally `int[5]`; same size, cosmetic only.)

### VERDICTS — Task 13 read-only audit (UI/control family — 8 headers)

v84 IDB = `GMS_v84.1_U_DEVM.exe` (port 13341, confirmed active via `list_instances`
before every probe). Cross-anchored vs v83 (port 13337) and v87 (port 13338).
Read-only: `disasm`/`decompile` + `type_inspect` on the v83/v87 IDBs as oracles;
**no `set_type`/`declare_type`/rename applied to any IDB**. The v83 IDB carries
prebuilt UDTs (`CWnd`, `CDialog`, `CCtrlWnd`, `CUIToolTip`, `CUIWnd`, `CCtrlButton`,
`CCtrlCheckBox`, `CFadeWnd`, `CUITitle`) used purely as a layout oracle; the v87 IDB
likewise carries `CUIToolTip`. Every v84 number below is anchored to a v84 disasm line.

#### Base-class sizes (load-bearing — anchored carefully)

| Base | v84 size | How anchored (v84) | Matches v83-side? |
|---|---|---|---|
| **CWnd** | **0x6C (108)** | ctor `sub_A26609`: vtables @ 0/4/8, 3 layer com_ptrs @ 0x18/0x1C/0x20, ZList `m_lpChildren` @ 0x50 (vtable `off_B92FE4`), `m_pBackgrnd` @ 0x68 = last (dtor unwind table). **CDialog ctor `sub_4F6020` writes its first own member @ 0x6C** ⇒ CWnd ends exactly @ 0x6C (no room for `>=95` `m_origin`). | ✅ v83 UDT = 108 |
| **CCtrlWnd** | **0x34 (52)** | v83 oracle UDT (no boundary gate at 83/84; control base unchanged); v84 ctor `sub_4E6D14` invoked by every CCtrl ctor with derived members starting @ 0x34. | ✅ v83 UDT = 52 |
| **CDialog** | **0x7C (124)** | = CWnd(0x6C) + m_nRet/m_bTerminate/m_pChildModal[2]. v84 chain `sub_4F63D7`→`sub_4F6020`→`sub_A26609` writes CDialog members @ 0x6C/0x70/0x74/0x78. | ✅ v83 UDT = 124 |
| **CUIToolTip** | **0x52C (1324)** | embedded base for CUIWnd/CCtrlButton/CUITitle — see dedicated note below. | ⚠️ **v84-specific** (v83=0x514, v87=0x534) |

CWnd's `>=87` gate is `SECPOINT m_ptCursorRel` vs two ints `m_ptCursorRel_x/_y` —
**both forms are 8 bytes @ 0x48/0x4C, so the gate is layout-neutral**; v84 (build<87)
compiles the two-int form, v87+ the SECPOINT. No size impact either way; gate correct.

#### CUIToolTip.h — the size-critical embedded UDT (⚠️ NEEDS CHANGE)

**v84 CUIToolTip = 0x52C (1324).** v83 = 0x514 (1300), v87 = 0x534 (1332).
v84 is a genuine **intermediate** between v83 and v87 — it does NOT equal v87.
Reconciliation: `v84 = v83(0x514) + m_pLayerAdditional(4) + m_pFontGen_Unknown(4)
+ m_pCanvasEquip_Durability[2][2](16) = 0x52C`. And `v87(0x534) = v84(0x52C)
+ m_pFontH_White(4) + m_pFontStan_Prp(4)`.

Per-gate findings (v84 ctor `CUIToolTip::CUIToolTip` @ **0x91A417**; embed cross-check
`0x7C + 0x52C = 0x5A8` = field after m_uiToolTip in dialog ctors sub_46A98C/sub_5AE105):

- **`>=95` `m_bUseDotImage` in `CLineInfo` — ABSENT.** v84 eh-vector ctor `lea eax,[esi+24h];
  push 20h; push 20h` (@ 0x91A445) ⇒ **CLineInfo stride = 0x20** (not 0x24); array @ 0x24
  ends @ 0x424. ✅ gate correct.
- **`>=83` `m_pLayerAdditional` — PRESENT @ 0x14** (Task 11): `mov [esi+10h],edi` (m_pLayer)
  `mov [esi+14h],edi` (@ 0x91A42E/34). ✅ gate correct. *(Note: the v83_Me IDB UDT lacks
  this field — that binary predates `>=83`; v84/v87 both have it, consistent with the gate.)*
- **`>=95` `m_nOptionLineNo` + `m_aOptionLineInfo[32]` — ABSENT.** After `m_nLineSeparated`
  @ 0x424 the font block begins immediately @ 0x428 (`mov [esi+428h],edi` @ 0x91A452);
  no 0x488-byte option-array. ✅ gate correct.
- **`>=87` `m_pFontGen_Unknown` — PRESENT in v84 @ 0x478. ⚠️ NEEDS CHANGE → `>=84`.**
  v84 zero-inits **21** font com_ptrs (0x428…0x478 inclusive); v83 has 20 (last
  `m_pFontGen_Blue`), v87 has 23. The single extra v84 font is `m_pFontGen_Unknown`.
- **`>=87` `m_pFontH_White` — ABSENT in v84 (stays `>=87`).** Canvas eh-vector for
  `m_pCanvasEquip_ReqItem[12]` is @ **0x47C** (`lea eax,[esi+47Ch]; push 0Ch; push 4`
  @ 0x91A4DD) — i.e. the font block stops at 0x478; v87 has FontH_White @ 0x47C. ✅ correct.
- **`>=87` `m_pFontStan_Prp` — ABSENT in v84 (stays `>=87`).** Same anchor (canvas @ 0x47C,
  not 0x484 as in v87). ✅ correct.
- **`>=87` `m_pCanvasEquip_Durability[2][2]` — PRESENT in v84 @ 0x518. ⚠️ NEEDS CHANGE → `>=84`.**
  v84 ctor: eh-vector `lea eax,[esi+518h]; push 4; push 4` (@ 0x91A555, count 4 = [2][2]),
  immediately followed by `m_bIngoreWeddingInfo` @ 0x528 ⇒ size 0x52C. v83 lacks it
  (m_bIngoreWeddingInfo @ 0x510); v87 has it @ 0x520.
- **`>=95` `m_pFontStan_Dsc/Num/Skill_Prp/Skill_Dsc` — ABSENT** (font block ends @ Gen_Unknown). ✅ correct.

**VERDICT: NEEDS CHANGE.** Two `>=87` gates fire too late for v84:
`m_pFontGen_Unknown` and `m_pCanvasEquip_Durability` must move to **`>=84`**, while
`m_pFontH_White` and `m_pFontStan_Prp` must STAY `>=87`. This SPLITS the header's
single `#if >=87 { m_pFontGen_Unknown; m_pFontH_White; }` block. As written, the
header compiles CUIToolTip at **0x518 for v84 (20 B too small)** — it drops
`m_pFontGen_Unknown` (-4) and `m_pCanvasEquip_Durability` (-16). **Task-16 must fix this**;
the error propagates into every embedder (CUIWnd, CCtrlButton, CUITitle, and all
CDialog/CWnd-derived UI windows embedding a CUIToolTip).

#### CWnd.h — gates `>=87` (cursor), `>=95` (UIOrigin/m_origin)
- v84 size **0x6C**. `>=87` cursor layout-neutral (8 B). `>=95` `m_origin` ABSENT
  (CDialog first member @ 0x6C). **VERDICT: all gates correct.**

#### CUIWnd.h — gate `>=95` (5 fields, mid-struct)
- v84 size **0x5C8** (= v83 0x5B0 + CUIToolTip delta 0x18). base CWnd 0x6C; embeds one
  CUIToolTip. The `>=95` block (`m_nSmallScreenX…m_bIsLargeMode`) sits between
  `m_nBackgrndY` and `m_bPosSave`; build 84 < 95 ⇒ compiled out (no v87 trap — only gate
  is `>=95`). **VERDICT: gate correct.** *(Size depends on the corrected CUIToolTip.)*

#### CCtrlButton.h — gate `>=95` (`m_sToolTipFromData`)
- v84 size **0x5BC** (= v83 0x5A4 + CUIToolTip delta 0x18). base CCtrlWnd 0x34;
  m_uiToolTip @ 0x8C (v84 methods `lea ecx,[esi+8Ch]; call sub_91CA58` @ 0x4C5B0D,
  0x8475F9). `m_sToolTipFromData` (`>=95`, between m_uiToolTip and m_bSelfDisable)
  ABSENT for build 84 ⇒ m_bSelfDisable @ 0x5B8. **VERDICT: gate correct.**
  *(Size depends on the corrected CUIToolTip.)*

#### CCtrlCheckBox.h — gate `>=95` (`m_nTextOffsetX/Y`, trailing)
- v84 size **0x6C**, v83-identical (no CUIToolTip embed). v84 ctor `sub_46C894`:
  CCtrlWnd ctor + eh-vector `m_apCanvasCheckBox[4]` @ 0x5C ending @ 0x6C; nothing
  written past ⇒ trailing `>=95` ints absent. **VERDICT: gate correct.**

#### CFadeWnd.h — gates `>=87` (3 ints), `REGION_GMS` (`m_bUserAlarm`)
- v84 size **0xD4**, v83-identical. v84 ctor `sub_52ACD8` (byte-isomorphic to v83
  `??0CFadeWnd@@QAE@XZ` @ 0x51F887): highest member `mov [esi+0C8h],eax`
  (`m_sInviter`), then `m_dwSN` @ 0xCC / `m_dwFriendID` @ 0xD0 — **no 3-int
  `m_nLevel/m_nJobCode/m_nExpQuestID` block** between m_sInviter and m_dwSN (would push
  m_dwSN to 0xD8). `m_bUserAlarm` present @ 0xBC (REGION_GMS, true). **VERDICT: all gates
  correct.** *(This is a `>=87` trap that was checked, not assumed — v84 sits with v83.)*

#### CUITitle.h — gate `>=95` (base CFadeWnd vs CDialog; `m_rcRMA`)
- v84 size **0x604** (= v83 0x5EC + CUIToolTip delta 0x18). v84 ctor `sub_635785`
  (singleton `g_CUITitle_pInstance`): **base is CDialog** (`sub_4F6020`, not CFadeWnd);
  `m_pCanvasRMA[2]` @ 0x88 followed immediately by buttons @ 0x90 (no 16-byte `m_rcRMA`);
  `m_uiToolTipTitle` @ 0xD8. Both `>=95` gates ABSENT. **VERDICT: all gates correct.**
  *(Size depends on the corrected CUIToolTip.)*

#### CUILoginStart.h — gate `>=95` / `REGION_JMS` (2 com_ptrs, mid-struct)
- v84 size **0x100**, v83-identical (no CUIToolTip embed). v84 ctor `sub_623270`:
  CDialog base; `m_pLogin` @ 0x7C; `m_aBtParam[5]` eh-vector @ 0x80 (stride 0x10) directly
  after m_pLogin — no `>=95`/JMS `m_pFont` + `m_pCanvasChannelName` between. `m_apButton[5]`
  @ 0xD0, `m_nViewWorldButtonType` @ 0xF8, `m_bRequestSent` @ 0xFC. **VERDICT: all gates correct.**

**Task-13 Task-16 cross-check flags:**
- **CUIToolTip.h (NEEDS CHANGE):** split the `>=87` font block — `m_pFontGen_Unknown` and
  `m_pCanvasEquip_Durability[2][2]` must become **`>=84`** (present in v84); `m_pFontH_White`
  and `m_pFontStan_Prp` stay `>=87`. Re-validate v83 (0x514, neither extra), v87/v95
  (0x534+, all four) so no other version flips. The fix must restore v84 size to **0x52C**
  and correspondingly all embedders (CUIWnd 0x5C8, CCtrlButton 0x5BC, CUITitle 0x604).
  **JMS caveat (verified in spec review):** the current font-block gates carry a
  `|| defined(REGION_JMS)` clause. JMS has a wholly different CUIToolTip layout
  (option-line array present, canvas @ 0x884). When splitting, the new `>=84` gate
  must RETAIN the JMS clause (`>=84 || defined(REGION_JMS)` for Gen_Unknown +
  Durability), and `m_pFontH_White` keeps `>=87 || JMS`; do NOT silently drop JMS
  coverage. (`m_pFontStan_Prp` stays `>=87` GMS-only, as today.)
- All other 7 headers: gates correct, no change.

### VERDICTS — Task 14 read-only audit (Mob/stat family — 4 headers)

v84 IDB = `GMS_v84.1_U_DEVM.exe` (port 13341, confirmed active via `list_instances`
before every probe). Cross-anchored vs v83 (port 13337) and v87 (port 13338).
Read-only: `disasm`/`decompile` + v83 `type_inspect` (UDT oracle) + `find_bytes`
(eh-vector locator); **no `set_type`/`declare_type`/rename applied to any IDB**.

**Headline:** TWO of the four headers NEED CHANGE. v84's CMob and SecondaryStat are
genuine **intermediates** (larger than v83, smaller than v87) — v84 carries a
SUBSET of the `>=87` fields, so the blanket `>=87` gates are wrong for v84.
CMapLoadable's `>=87` field is also present in v84. MobStat is clean.

#### Size anchors (load-bearing)

| Struct | v83 | **v84** | v87 | v84 anchor |
|---|---|---|---|---|
| **CMob** | 0x548 (1352) | **0x560 (1376)** | 0x588 (1416) | `CreateMob` @ 0x678024: `push 560h` before `call CMob::CMob` (0x678060). v83 `push 548h` @ 0x6621a8; v87 `push 588h` @ 0x69c5c5. |
| **MobStat** | 0x208 (520) | **0x208 (520)** | 0x208 (520) | embedded `m_stat` @ CMob+0x1A0; next field m_rgHorz/m_nTeamForMCarnival @ 0x3A8/0x3B0 ⇒ 0x3A8−0x1A0 = 0x208. v83 UDT = 520. No 83/84/87 gates. |
| **SecondaryStat** | 0xCD8 (3288) | **0xD20 (3360)** | 0xD74 (3444) | ctor `sub_79ED3E` eh-vector `lea eax,[esi+0CE8h]; push 8; push 7` (@ 0x79ed5c) ⇒ aTemporaryStat[7] @ 0xCE8 ⇒ size 0xCE8+0x38 = 0xD20. v83 @ 0xCA0 (ctor 0x77c25f); v87 @ 0xD3C (ctor 0x7ca8d2). |
| **CMapLoadable** | 0x114 (276) | **0x128 (296)** | — | ctor `sub_64EAB3`; first CLogin-own field @ 0x12C (= base+4 of m_pConnectionDlg com_ptr) ⇒ base 0x128. v83 ctor 0x639401, CLogin field @ 0x118 ⇒ base 0x114 (UDT confirms). +0x14 = one ZList (m_lVisibleByQuest). |

#### CMob.h — gates `<95`, `>=87` (×5), `>=95` (×3) — ⚠️ **NEEDS CHANGE**

- **`<95` `unknown1` ZList — PRESENT in v84** (84<95 → true). ✅ correct.
  (v83/v84/v87 all carry it; in v83/v87 IDA names it `dummy1` @ the 4th of the
  four post-`m_lpLayerASIcon` ZLists.)
- **`>=87` `m_aMultiTargetForBall` (ZArray, 4B) — PRESENT in v84 @ 0x528. ⚠️ NEEDS
  CHANGE → `>=84`.**
- **`>=87`||JMS `m_aRandTimeforAreaAttack` (ZArray, 4B) + `m_delaySkill` (DelaySkill,
  16B) — PRESENT in v84 @ 0x52C / 0x530-0x53C. ⚠️ NEEDS CHANGE → `>=84`||JMS.**
  v84 ctor doom region (disasm @ 0x67826c-0x6782b3): m_nHPpercentage=100 @ 0x520,
  m_bWaitingToBeSetTossed @ 0x524, **m_aMultiTargetForBall @ 0x528, m_aRandTimeforAreaAttack
  @ 0x52C, m_delaySkill[4] @ 0x530/534/538/53C**, m_bDoomReserved @ 0x540, m_bDoomReservedSN
  @ 0x544, m_lpStatChangeReserved ZList @ 0x548. **Contrast v83 ctor @ 0x6621d9:**
  m_nHPpercentage @ 0x520, m_bWaitingToBeSetTossed @ 0x524, m_bDoomReservedSN @ 0x52C,
  m_lpStatChangeReserved @ 0x530 — **NO m_aMultiTargetForBall / m_aRandTimeforAreaAttack
  / m_delaySkill**. The inserted 24 bytes (4+4+16) exactly equal the v84−v83 size delta
  (0x560−0x548 = 0x18).
- **`>=87` `SECPOINT m_ptPos`/`m_ptPosPrev` (else: 4 ints) — v84 takes the ELSE (2-int)
  form, layout-neutral.** ✅ gate correct (build 84<87). m_ptPos region (0x510-0x51F,
  16B) is byte-identical size in v83 and v84 (both lead to m_nHPpercentage @ 0x520);
  no secure-coordinate init (no `sub_42B612` pairs) in v84, unlike v87.
- **`>=87`||JMS `m_bChasing` (TSecType<int>, 12B) — ABSENT in v84.** ✅ gate correct.
  m_lpStatChangeReserved ZList ends @ 0x55C; struct = 0x560 (4 bytes left) — no room
  for a 12B TSecType. v84 ctor has no `sub_6AD8AE`(m_bChasing) call (v87 @ 0x69c8af does).
- **`>=95` `m_nPhase` / `m_arcMultiBody*` / the `>=95`||JMS doom-bomb tail block — ABSENT.**
  ✅ correct (build 84<95).
- **Doom field (Task 11 confirmed):** m_bDoomReserved @ 0x540 zero-inited (`mov [esi+540h],ebx`
  @ 0x6782ad), m_bDoomReservedSN @ 0x544 (`mov [esi+544h],bl`). v84 reproduces the v87 fix.

**VERDICT: NEEDS CHANGE.** The `>=87` gates on `m_aMultiTargetForBall` (line 228-230) and
`m_aRandTimeforAreaAttack`+`m_delaySkill` (line 231-234) fire too late for v84 — they must
become **`>=84`** (and `>=84`||JMS for the JMS-shared block). As written, the header compiles
v84 CMob at **0x548 (24 B too small)**, dropping those 3 fields and shifting m_bDoomReserved
back to 0x528 — which would corrupt the doom-fix offset and every field after. All other
CMob gates correct. **Flag for Task 16.**

#### MobStat.h — gates `>=95`||JMS, `>=95` (×2) — ✅ all gates correct

No gate sits at the 83/84/87 boundary, so v84 = v83 layout. **v84 size 0x208 (520)**,
anchored by the embedded `m_stat` extent inside CMob (0x1A0 … 0x3A8) and the v83 UDT
(`MobStat` = 520). The `>=95`||JMS TimeBomb block (4 fields), the `>=95` MagicCrash/
DamagedElemAttr/HealByDamage block (9 fields), and `>=95` `bCannotEvade` are all
compiled out for build 84. **VERDICT: all gates correct.**

#### SecondaryStat.h — gates `>=87`/`==87`/`>=95` (size-critical embedded UDT) — ⚠️ **NEEDS CHANGE**

**v84 SecondaryStat = 0xD20 (3360).** v83 = 0xCD8 (3288), v87 = 0xD74 (3444). v84 is a
genuine **intermediate** — it does NOT equal v83 and does NOT equal v87. Anchored
INDEPENDENTLY from the ctor's `aTemporaryStat[7]` eh-vector (the array is the always-present
last member): v84 `lea eax,[esi+0CE8h]` ⇒ array @ 0xCE8 ⇒ size 0xCE8 + 7×8 = 0xD20.

Per-gate findings (v84 ctor `sub_79ED3E`; v84 Clear `sub_79F3A1`, cross-diffed dword-for-dword
against v83 Clear `SecondaryStat::Clear` @ 0x77c8a4):

- **`>=87`||JMS `nDojangShield_` (3 secure-tears, MID-struct, after nBarrier) — ABSENT in
  v84.** ✅ gate correct. v84 Clear's mid-struct dword indices match v83 EXACTLY (both jump
  nBarrier → nReverseInput with no insertion); a mid-struct insertion would shift every
  later index. (build 84<87)
- **`==87`||JMS byte-variants `nReverseInput_` / `rDojangBerserk_` (char vs int) — v84 takes
  the ELSE (int, 12B) form**, same as v83. ✅ gate correct (the `==87`-EXCLUSIVE byte form
  is v87-only; v84 sides with v83). Confirmed by matching Clear indices through dword 807.
- **`>=87`||JMS post-SoulStone block (4 stats: nFlying/nFrozen/nAssistCharge/nEnrage,
  12 secure-tears) — PARTIALLY PRESENT in v84.** ⚠️ **NEEDS CHANGE / SPLIT.** v84 carries
  ONLY the first **2 stats** = `nFlying_` + `nFrozen_` (6 secure-tears = 72 B) at dwords
  808-825 (bytes 0xCA0-0xCE7, immediately after v83's old end-of-struct, immediately before
  aTemporaryStat). `nAssistCharge_` and `nEnrage_` are **ABSENT** in v84. Evidence: v84 Clear
  has exactly 6 extra secure-tears past v83's last field (nSoulStone @ dword ~805): bases @
  808/811/814/817/820/823 (each = int[2]+CS), then the aTemporaryStat loop @ dword 826
  (0xCE8). 6×12 B = 0x48 = the v84−v83 size delta. (Atlas oracle lists all 4 as one `>=87`
  block — that is the v87 server wire layout; the v84 CLIENT disasm is authoritative and
  shows only 2.)
- **`>=95`||JMS SuddenDeath-onward block + `>=95` rEMDD/Guard/Mine/SafetyDamage/… tail —
  ABSENT in v84.** ✅ correct (build 84<95). No fields between Flying/Frozen and aTemporaryStat.

**VERDICT: NEEDS CHANGE — size-critical.** The header's single `>=87`||JMS block
(`nFlying_`/`nFrozen_`/`nAssistCharge_`/`nEnrage_`, file lines 622-649) must be **SPLIT**:
`nFlying_` + `nFrozen_` move to **`>=84`** (present in v84), while `nAssistCharge_` +
`nEnrage_` STAY `>=87` (absent in v84). `nDojangShield_` and the `==87` byte-variants stay
as-is (correctly excluded from v84). The fix must restore v84 SecondaryStat to **0xD20**.
**Because SecondaryStat is embedded (in CUser, and any CWvsContext-adjacent stat holder), a
wrong size shifts every following field** — as written, the header compiles v84 at 0xCD8
(72 B too small). Re-validate v83 (0xCD8, neither Flying nor Frozen), v87 (0xD74, all four)
so no other version flips. **Flag for Task 16.** *(JMS caveat: the existing block carries an
`||JMS` clause; the new `>=84` gate for Flying/Frozen must RETAIN `|| defined(REGION_JMS)`,
and AssistCharge/Enrage keep `>=87 || JMS` — do not drop JMS coverage.)*

#### CMapLoadable.h — gates `>=95` (×3), `>=87`||JMS (×1) — ⚠️ **NEEDS CHANGE**

**v84 CMapLoadable = 0x128 (296).** v83 = 0x114 (276). Delta +0x14 = exactly one ZList.

- **`>=87`||JMS `m_lVisibleByQuest` (ZList, 0x14) — PRESENT in v84 @ 0xA0. ⚠️ NEEDS CHANGE
  → `>=84`||JMS.** v84 ctor `sub_64EAB3` has **THREE** ZLists before the three ZMaps:
  vtables @ 0x78 (m_lpObstacle), 0x8C (m_lpRefInfo), **0xA0 (m_lVisibleByQuest)**, then
  m_mNamedObj ZMap @ 0xB4. **Contrast v83 ctor @ 0x639401:** only TWO ZLists — m_lpObstacle
  @ 0x78, m_lpRefInfo @ 0x8C, then m_mNamedObj ZMap @ **0xA0** (no third ZList). The extra
  ZList @ 0xA0 in v84 shifts m_mNamedObj/m_mTagedObj/m_mlLayerBack and all later fields +0x14.
- **`>=95` `m_bField` (@ 0x34 in v95) — ABSENT in v84.** ✅ correct. v84/v83 both place
  m_pSpace2D directly after m_pPropField; no m_bField. (The header's IDA comment block
  showing sizeof 0x148 with m_bField @ 0x34 is a v95 dump, not v84.)
- **`>=95` `m_lpLayerLetterBox` (ZList) — ABSENT in v84.** ✅ correct.
- **`>=95` `m_bPlayHoldedBGM` / `m_tPlayHoldedBGM` (trailing 2 ints) — ABSENT in v84.**
  ✅ correct. The v84 size (0x128 = v83 0x114 + only the m_lVisibleByQuest ZList) leaves no
  room for any `>=95` field.

**VERDICT: NEEDS CHANGE.** `m_lVisibleByQuest`'s `>=87`||JMS gate (line 153) fires too late
for v84 — must become **`>=84`||JMS** (present in v84). As written, the header compiles v84
CMapLoadable at **0x114 (20 B too small)**, dropping the ZList and shifting the three ZMaps
and everything after by 0x14. All `>=95` gates correct. **Flag for Task 16.** README flags
CMapLoadable as minimally-important, but the size IS wrong for v84 and CMapLoadable is the
base of CLogin/CField, so the shift propagates.

**Task-14 Task-16 cross-check flags (3 of 4 headers NEED CHANGE):**
- **CMob.h:** move `m_aMultiTargetForBall` and `m_aRandTimeforAreaAttack`+`m_delaySkill`
  from `>=87`(`||JMS`) to **`>=84`**(`||JMS`). Restores v84 CMob to 0x560 and the doom field
  to 0x540. `m_ptPos` SECPOINT (`>=87`), `m_bChasing` (`>=87`||JMS), and all `>=95` gates
  stay (correctly excluded from v84).
- **SecondaryStat.h (size-critical):** SPLIT the `>=87`||JMS post-SoulStone block — move
  `nFlying_`+`nFrozen_` (6 tears) to **`>=84`**(`||JMS`); keep `nAssistCharge_`+`nEnrage_`
  at `>=87`||JMS. Restores v84 to 0xD20. `nDojangShield_` (`>=87`||JMS, mid-struct) and the
  `==87` byte-variants stay (excluded from v84).
- **CMapLoadable.h:** move `m_lVisibleByQuest` from `>=87`||JMS to **`>=84`**||JMS. Restores
  v84 to 0x128.
- **MobStat.h:** gates correct, no change.
- Every rewrite must be re-validated against v83 (smaller, none present), v87 (larger, all
  present) and v95/v111/JMS185 so no other version's branch flips (FR-13).

**SecondaryStat ⊂ CWvsContext — embedding & blast radius (verified in spec review):**
`SecondaryStat m_secondaryStat` is embedded BY VALUE in CWvsContext (CWvsContext.h ~line 110),
**AFTER** `m_aClientKey` (line 99, Task-11-verified @0x20A0). Source order between them:
m_aClientKey → m_bTesterAccount(`>=87`) → m_dwCharacterId → … → m_basicStat → m_secondaryStat.
- **m_aClientKey is SAFE:** growing SecondaryStat by +0x48 for v84 does NOT shift m_aClientKey
  or anything upstream of m_secondaryStat. The current (pre-fix) header is NOT wrong about the
  client key for v84 — no compensating factor needed.
- **Downstream blast radius (Task-16 MUST re-verify):** every CWvsContext field AFTER
  m_secondaryStat shifts +0x48 in v84 once the fix lands (m_forcedStat, m_temporaryStatView,
  m_townPortal, m_party, guild/alliance, … ). After applying the SecondaryStat split, confirm
  these downstream CWvsContext offsets still match the v84 binary.
- **v87 reconciliation flag (pre-existing, NOT a v84 issue):** the SecondaryStat header's v87
  path appears ~0xC short of the binary v87 size (multiple `>=87`/`==87` gated regions). v83/v84
  math is exact; the v87 gap is a separate concern worth a dedicated v87 pass, but does not
  affect any v84 verdict here.

### VERDICTS — Task 15 read-only audit (Party/Guild/Config/misc — 6 headers)

v84 IDB = `GMS_v84.1_U_DEVM.exe` (port 13341, confirmed active via `list_instances`
before every probe). Cross-anchored vs v83 (port 13337) and v95 (port 13339, as the
oracle where every `>=95` gate flips ON). Read-only: `disasm`/`decompile` +
`find_bytes` (cross-version function locator) + v95 `type_inspect` (UDT oracle for
GUILDDATA); **no `set_type`/`declare_type`/rename applied to any IDB**.

**Headline:** ALL 6 headers are CORRECT for v84 — no rewrites required. Unlike the
Mob/UI families (Tasks 13/14), NONE of these 6 headers has a `>=87` gate, so v84
cannot be an "intermediate": every gate here is `<95`/`>=95`/`>=111`, which resolves
identically for build 83 and build 84. Each gate was nonetheless verified against a
concrete v84 disasm anchor (not assumed). The PartyMember `<95` block is **PRESENT**
in v84 (confirmed via the v84↔v95 PARTYDATA size delta, not assumed).

#### PartyMember.h — gate `<95` (adwFieldID[6]) — ✅ PRESENT in v84, gate correct
- **`<95` `adwFieldID[6]` (24 B) — PRESENT in v84** (84 < 95 → true). PARTYMEMBER is
  not flat-decoded standalone in OnPartyResult; it rides inside PARTYDATA's single
  `DecodeBuffer(this, 0x12A)` blob. v84 PARTYDATA = 0x12A (298) = **PARTYMEMBER(202,
  incl adwFieldID) + aTownPortal[6](6×16 = 96)**. The 202 figure REQUIRES adwFieldID
  (core PARTYMEMBER without it = 178 = 0xB2; with it = 202).
- **Decisive `<95` proof (v84↔v95 delta):** v95 PARTYDATA::Decode @ 0x4F2B00 =
  `push 17Ah` = 0x17A (378), exactly v84's 0x12A + 0x50. The +0x50 = v95 moving
  adwFieldID PARTYMEMBER→PARTYDATA (net 0, but reshuffled) + adding TOWNPORTAL.m_nSKillID
  (6×4=24) + PQReward tail (adwFieldID[6]+aPQReward[6]+aPQRewardType[6]+dwPQRewardMobTemplateID
  +bPQReward = 24+24+24+4+4 = 80). 24+80 − 24 (adwFieldID removed from member) = 80 = 0x50. ✅
  This proves v84 keeps adwFieldID inside PARTYMEMBER (the `<95` side).
- **VERDICT: gate correct.** (PARTYMEMBER size note for embedders: 202 B per member;
  it appears once by value inside PARTYDATA.)

#### PartyData.h — gates `>=95` (×3 regions) — ✅ all gates correct
- **v84 size 0x12A (298).** Anchor: v84 PARTYDATA::Decode @ 0x4EB819
  `push 12Ah; push ecx; mov ecx,[esp+8]; call DecodeBuffer (0x432EBE); retn 4` —
  identical literal to v83 PARTYDATA::Decode (0x12A @ 0x4E43E8). Confirmed as PARTYDATA
  by its caller: CWvsContext::OnPartyResult (sub_A89CF3, the big party packet handler).
  The matching Encode stub @ 0x4EB807 also pushes 0x12A.
- **`>=95` `TOWNPORTAL.m_nSKillID` — ABSENT** (TOWNPORTAL = 16 B in v84: 4+4+8, no skill).
- **`>=95` `adwFieldID[6]` (in PARTYDATA) — ABSENT** (it's still in PARTYMEMBER for v84).
- **`>=95` PQReward tail (aPQReward/aPQRewardType/dwPQRewardMobTemplateID/bPQReward) — ABSENT.**
  All three blocks together account for the +0x50 jump to v95's 0x17A. ✅
- **VERDICT: all gates correct** (v84 = v83 = 0x12A).

#### GuildData.h — gates `>=95` (×3: nLevel, mSkillRecord ZMap, aSkillRecordOnlyID) — ✅ correct
- **v84 size 0x2A (42).** GUILDDATA is field-by-field decoded (ZXString/ZArray members),
  so size comes from the ctor/Clear extent, not a buffer literal. v84 GUILDDATA::Clear
  @ 0xA4C703 (located by byte-identical prologue `53 56 8B F1 33 DB 8D 4E 04 89 1E 89
  5E 22` to v83 Clear @ 0xA024DD) resets members up to **nAllianceID @ 0x26**
  (`*(this+38)=0`); the highest member is nAllianceID ⇒ size 0x2A. It does NOT touch
  nLevel(0x2A)/mSkillRecord/aSkillRecordOnlyID.
- **`>=95` block — ABSENT in v84.** Cross-check: v95 GUILDDATA UDT (type_inspect) =
  0x4A (74), with nLevel @ 0x2A, mSkillRecord ZMap @ 0x2E (24 B), aSkillRecordOnlyID @
  0x46 (4 B) — exactly the trailing +0x20 the gate adds. v95 ctor @ 0x9E5750 explicitly
  builds the ZMap (`_m_uTableSize=31`, `_CalcAutoGrow(…,0x64)`); v84 ctor/Clear has no
  such ZMap init. Struct is byte-packed (mark fields at unaligned 0x18-0x1D). ✅
- **VERDICT: gate correct** (v84 = v83 = 0x2A).

#### ConfigSysOpt.h — gate `>=95` (×2: bSysOpt_LargeScreen, bSysOpt_WindowedMode) — ✅ correct
- **v84 size 0x30 (48).** Anchor: CConfig::ApplySysOpt @ 0x4A3A9C does
  `qmemcpy(this+25, a2, 0x30u)` — copies the entire CONFIG_SYSOPT (`a2`) = **12 ints
  (0x30)** into the CConfig object at this[25..36]. The 12 fields used downstream map
  1:1 to the header: this[25]=nSysOpt_Video … this[31]=nSysOpt_MouseSpeed (`g_CInputSystem
  +2416 = this[31]`), this[34]=bSysOpt_Tremble, this[35]=nSysOpt_MobInfo (tested ==1/2/3
  for HP/MP bars → CWvsContext+14376/14380), this[36]=bSysOpt_Minimap_Normal
  (`sub_A2C4E1(this[36])`). Exactly 12 ints, no 13th/14th.
- **`>=95` `bSysOpt_LargeScreen`/`bSysOpt_WindowedMode` — ABSENT** (would make 0x38).
- **Windowed-mode cross-check (Task 7):** `C_CONFIG_SYS_OPT_WINDOWED_MODE` @ 0xC4B150 is a
  STANDALONE global (`g_CConfig_SysOpt_WindowedMode`, set to 0x10 in CWvsApp::SetUp @
  0xA3E1BB; xref'd only by SetUp/CreateMainWindow/InitializeGr2D), NOT a CONFIG_SYSOPT
  member. This is exactly consistent with `bSysOpt_WindowedMode` being `>=95`-gated-out of
  the struct for v84 — the windowed-mode setting lives outside the struct in this build. ✅
- **VERDICT: gate correct** (v84 = v83 = 0x30).

#### CFuncKeyMappedMan.h — gates `>=111`||JMS (dummy1), `>=95` (m_nNormalAttackCode) — ✅ correct
- **v84 size 0x3C8 (968).** Anchor: CreateInstance @ 0xA43D50 → `sub_403065(968)` then
  ctor @ 0x59DD00. Cross-check vs Task-6: ctor stores vftable @ 0xB46B08; the two
  445-byte (0x1BD) memcpys from DEFAULT_FKM @ 0xC31C7C populate m_aFuncKeyMapped[89] and
  m_aFuncKeyMapped_Old[89] (89×5 B = 445 ⇒ FUNCKEY_MAPPED = 5 B); the two 0x20 memcpys
  from DEFAULT_QKM @ 0xC31E3C populate m_aQuickslotKeyMapped[8]/_Old[8].
- **Field map:** +4 m_aFuncKeyMapped(445), +449 _Old(445), +896 m_aQuickslotKeyMapped(32),
  +928 _Old(32), +960 m_nPetConsumeItemID, +964 m_nPetConsumeMPItemID. Ctor STOPS at
  a1[241] (+964) ⇒ no further field. 964+4 = 968 = alloc size. ✅
- **`>=111`||JMS `dummy1` — ABSENT; `>=95` `m_nNormalAttackCode` — ABSENT** (both would push
  size past 968). GMS build 84 < 95 < 111, not JMS ⇒ both correctly excluded.
- **VERDICT: all gates correct.**

#### CLogo.h — gates `>=95` (×4), `>=95`||JMS (m_bNXFadeOut), `>=111` (dummy1) — ✅ all correct
- **v84 size 0x38 (56).** Anchor: alloc `push 38h` @ 0xA3E6B8 in CWvsApp::SetUp,
  immediately before `call CLogo::CLogo` @ 0x64417C (Task-5 ctor). Ctor zero-inits
  this[6]=m_pLayerMain (com_ptr), this[7]=m_pLogoProp (com_ptr), then this[9..13] =
  m_dwTickInitial/m_dwClick/m_bLogoSoundPlayed/m_bWZInit/m_bNXFadeIn (this[8]=m_nLogoCount
  left uninit as a plain int). Last field this[13] @ 0x34 ⇒ 0x38. CStage base = 0x18
  (this[0..5], vtables @ 0x0-0xC set in ctor).
- **`>=95` `m_pLayerBackground` (leading com_ptr) — ABSENT** (first own field is m_pLayerMain
  @ this[6]/0x18; a leading `m_pLayerBackground` would push everything +4 and the doc count
  would not close at 0x38). **`>=95`||JMS `m_bNXFadeOut` — ABSENT** (build 84<95, not JMS;
  would be this[14]/0x38). **`>=95` `m_bVideoMode`/`m_videoState` — ABSENT.** **`>=111`
  `dummy1` — ABSENT.** All resolve to the v83 side. ✅
- **VERDICT: all gates correct.**

**Task-15 Task-16 cross-check flags:** NONE. All 6 headers' gates are correct for v84
(no `>=87`-style intermediate traps exist in this family). No source changes for Task 16.

### 22/22 COMPLETION — final audit roll-up (Tasks 11–15)

**All 22 version-gated headers now carry a v84 size + per-gate verdict + disasm anchor
(no ☐ rows remain).** Tally across the full audit:

- **CORRECT (18 of 22 table rows):** CWvsApp, CWvsContext, CClientSocket, CLogin,
  MobStat, CFadeWnd, CCtrlButton, CCtrlCheckBox, CUIWnd, CWnd, CUILoginStart, CUITitle,
  PartyMember, PartyData, GuildData, ConfigSysOpt, CFuncKeyMappedMan, CLogo.
  *(The 5 Task-11 boundary-gate SITES — doom-fix dllmain, CWvsContext key, socket_hooks,
  CLogin ==83 member, CUIToolTip >=83 — were also all CORRECT; those sites are folded into
  the CWvsContext/CLogin/CUIToolTip header rows.)*
- **NEEDS CHANGE (4):** **CUIToolTip.h**, **CMob.h**, **SecondaryStat.h**,
  **CMapLoadable.h** — all flagged for Task 16, all are `>=87` gates firing too late for
  v84 (v84 is an intermediate carrying a SUBSET of the `>=87` fields):
  - **CUIToolTip.h:** move `m_pFontGen_Unknown` + `m_pCanvasEquip_Durability[2][2]` to
    `>=84`(||JMS for Gen_Unknown+Durability); keep `m_pFontH_White`/`m_pFontStan_Prp` at
    `>=87`. Restores v84 to 0x52C (propagates to CUIWnd 0x5C8, CCtrlButton 0x5BC, CUITitle 0x604).
  - **CMob.h:** move `m_aMultiTargetForBall` and `m_aRandTimeforAreaAttack`+`m_delaySkill`
    from `>=87`(||JMS) to `>=84`(||JMS). Restores v84 CMob to 0x560.
  - **SecondaryStat.h (size-critical):** SPLIT the `>=87`||JMS post-SoulStone block —
    `nFlying_`+`nFrozen_` → `>=84`(||JMS); keep `nAssistCharge_`+`nEnrage_` at `>=87`||JMS.
    Restores v84 to 0xD20. Re-verify downstream CWvsContext offsets after m_secondaryStat (+0x48).
  - **CMapLoadable.h:** move `m_lVisibleByQuest` from `>=87`||JMS to `>=84`||JMS. Restores v84 to 0x128.
  - Every rewrite must be re-validated against v83/v87/v95/v111/JMS185 (FR-13) so no other
    version's compiled branch flips.

## The 22 version-gated headers

For each, record: v84 size, which gated fields are present/absent in v84, the
disassembly anchor, and the verdict (gate correct / gate needs change).

| Header | Existing thresholds in file | v84 size | Verdict | Evidence |
|---|---|---|---|---|
| common/CWvsApp.h | `>= 87`, `>= 95` | 0x60 (96) — v83-side layout | ✅ all gates correct | ctor @ 0xA3D719 (v83-identical early layout); see Task-12 notes |
| common/CWvsContext.h | `> 83`, `>= 87`, `>= 95` | ~0x3920 (≈14624; upper bound 0x3F70) | ✅ all gates correct | OnConnect @ 0x499DCD; ctor sub_A4BF0B; see Task-12 notes |
| common/CClientSocket.h | `>= 111` | 0x94 (148) | ✅ gate correct | CreateInstance @ 0xA43D0B `push 94h`; see Task-12 notes |
| common/CLogin.h | `>= 95`, `== 83` | 0x28C (652) | ✅ all gates correct | alloc `push 28Ch` @ 0xA6FD1C; ctor @ 0x608B15; see Task-12 notes |
| common/CMapLoadable.h | `>= 95` (×3), `>= 87`\|\|JMS (×1) | 0x128 (296) | ⚠️ **NEEDS CHANGE** (`m_lVisibleByQuest` mis-gated `>=87`) | v84 ctor sub_64EAB3: THREE ZLists @ 0x78/0x8C/**0xA0** (m_lVisibleByQuest present) before ZMaps @ 0xB4; v83 ctor 0x639401 has only TWO (ZMap @ 0xA0). `>=95` all absent. See Task-14 notes. |
| common/CLogo.h | `>= 95` (×4), `>= 95`\|\|JMS (×1), `>= 111` (×1) | 0x38 (56) | ✅ all gates correct | alloc `push 38h` @ 0xA3E6B8 (CWvsApp::SetUp, before CLogo::CLogo @ 0x64417C); ctor zero-inits this[6..7] (m_pLayerMain/m_pLogoProp com_ptrs) + this[9..13] (m_dwTickInitial…m_bNXFadeIn), last field @ 0x34 ⇒ 0x38. CStage base = 0x18. `m_pLayerBackground`/`m_bNXFadeOut`/`m_bVideoMode`/`m_videoState`/`dummy1` all ABSENT. See Task-15 notes. |
| common/CMob.h | `<95`, `>=87` (×5), `>=95` (×3) | 0x560 (1376) | ⚠️ **NEEDS CHANGE** (3 fields mis-gated `>=87`) | CreateMob @ 0x678024 `push 560h`; v84 ctor @ 0x678060 has m_aMultiTargetForBall @ 0x528, m_aRandTimeforAreaAttack @ 0x52C, m_delaySkill @ 0x530-0x53C (absent in v83 ctor 0x6621d9); m_bDoomReserved @ 0x540. m_bChasing & SECPOINT m_ptPos & all `>=95` absent. See Task-14 notes. |
| common/MobStat.h | `>=95`\|\|JMS, `>=95` (×2) | 0x208 (520) — v83-identical | ✅ all gates correct | embedded m_stat @ CMob+0x1A0 … 0x3A8 (m_nTeamForMCarnival @ 0x3B0); v83 UDT MobStat=520; no 83/84/87 gate. See Task-14 notes. |
| common/ConfigSysOpt.h | `>= 95` (×2) | 0x30 (48) | ✅ gate correct | CConfig::ApplySysOpt @ 0x4A3A9C: `qmemcpy(this+25, a2, 0x30u)` copies exactly **12 ints** (0x30) = fields nSysOpt_Video…bSysOpt_Minimap_Normal; `>=95` `bSysOpt_LargeScreen`/`bSysOpt_WindowedMode` ABSENT (would make 0x38). Windowed-mode is a SEPARATE global `g_CConfig_SysOpt_WindowedMode` @ 0xC4B150 (not a struct member in v84), consistent with the gate. See Task-15 notes. |
| common/CFuncKeyMappedMan.h | `>= 111`\|\|JMS (×1), `>= 95` (×1) (+ JMS-only `dummy2`) | 0x3C8 (968) | ✅ all gates correct | CreateInstance @ 0xA43D50 `sub_403065(968)`; ctor @ 0x59DD00: vftable @ 0xB46B08, memcpy(+4,DEFAULT_FKM,0x1BD=445)=m_aFuncKeyMapped[89]×5B, memcpy(+449,…,445)=_Old, memcpy(+896,DEFAULT_QKM,0x20)=m_aQuickslotKeyMapped[8], memcpy(+928,…,0x20)=_Old, +960/+964=m_nPetConsumeItemID/MP. Ctor stops @ 964+4=968 ⇒ `dummy1`(`>=111`) & `m_nNormalAttackCode`(`>=95`) ABSENT. See Task-15 notes. |
| common/CFadeWnd.h | `>= 87` (+ `REGION_GMS` `m_bUserAlarm`) | 0xD4 (212) | ✅ all gates correct | v84 ctor sub_52ACD8: `mov [esi+0C8h],eax` (m_sInviter) is highest field → m_dwSN @ 0xCC adjacent (no `>=87` 3-int block). See Task-13 notes. |
| common/CCtrlButton.h | `>= 95` | 0x5BC (1468) — embeds v84 CUIToolTip (0x52C) | ✅ gate correct | v84 m_uiToolTip @ 0x8C (`lea ecx,[esi+8Ch]; call sub_91CA58` @ 0x4C5B0D / 0x8475F9); base CCtrlWnd 0x34; `>=95` `m_sToolTipFromData` absent. See Task-13 notes. |
| common/CCtrlCheckBox.h | `>= 95` | 0x6C (108) — v83-identical | ✅ gate correct | v84 ctor sub_46C894: eh-vector `m_apCanvasCheckBox[4]` @ 0x5C ends 0x6C; nothing written past → `>=95` `m_nTextOffsetX/Y` absent. See Task-13 notes. |
| common/CUIWnd.h | `>= 95` | 0x5C8 (1480) — embeds v84 CUIToolTip (0x52C) | ✅ gate correct | base CWnd 0x6C; m_uiToolTip embed; v83-side trivially (84<95); `>=95` 5-field mid-block absent. See Task-13 notes. |
| common/CWnd.h | `>= 87` (`SECPOINT m_ptCursorRel`), `>= 95` (`UIOrigin m_origin`) | 0x6C (108) | ✅ all gates correct | v84 CWnd ctor sub_A26609 (m_pBackgrnd @ 0x68 last); CDialog ctor sub_4F6020 writes its 1st own member @ 0x6C ⇒ CWnd ends @ 0x6C (no `>=95` m_origin). `>=87` cursor is 8 B either form (layout-neutral). See Task-13 notes. |
| common/CUIToolTip.h | `>= 83`, **`>= 87` (×4)**, `>= 95` (×6) | **0x52C (1324)** | ⚠️ **NEEDS CHANGE** (2 fields mis-gated `>=87`) | v84 ctor sub_91A417: CLineInfo=0x20 (eh-vec `push 20h`), no m_aOptionLineInfo, **21 fonts** (0x428–0x478) incl `m_pFontGen_Unknown`, **`m_pCanvasEquip_Durability[2][2]` PRESENT @ 0x518**. See Task-13 CUIToolTip note. |
| common/CUILoginStart.h | `>= 95` / `REGION_JMS` | 0x100 (256) — v83-identical | ✅ all gates correct | v84 ctor sub_623270: m_pLogin @ 0x7C, m_aBtParam[5] eh-vec @ 0x80 (size 0x10) adjacent ⇒ `>=95`/JMS 2 com_ptrs absent. See Task-13 notes. |
| common/CUITitle.h | `>= 95` (base CFadeWnd + `m_rcRMA`) | 0x604 (1540) — embeds v84 CUIToolTip (0x52C) | ✅ all gates correct | v84 ctor sub_635785: base is CDialog (sub_4F6020, not CFadeWnd); m_pCanvasRMA[2] @ 0x88 → buttons @ 0x90 adjacent (no `m_rcRMA`); m_uiToolTipTitle @ 0xD8. See Task-13 notes. |
| common/GuildData.h | `>= 95` (×3: nLevel, mSkillRecord ZMap, aSkillRecordOnlyID) | 0x2A (42) | ✅ gate correct | v84 GUILDDATA::Clear @ 0xA4C703 (byte-identical to v83 Clear @ 0xA024DD) touches members up to nAllianceID @ 0x26 ⇒ size 0x2A; NO nLevel/mSkillRecord/aSkillRecordOnlyID. v95 GUILDDATA UDT = 0x4A (those 3 fields add +0x20 @ 0x2A onward). `>=95` block ABSENT in v84. See Task-15 notes. |
| common/PartyMember.h | `< 95` (adwFieldID[6]) | 202 (0xCA) — embedded in PARTYDATA | ✅ gate correct (`<95` block PRESENT) | adwFieldID[6] is part of v84's flat PARTYDATA decode (0x12A): PARTYMEMBER(202, incl adwFieldID) + aTownPortal[6](96) = 298. Proven by v95 delta: v95 moves adwFieldID PARTYMEMBER→PARTYDATA & adds PQ block ⇒ v95 PARTYDATA = 0x17A (=0x12A+0x50). `<95` true for v84 ⇒ adwFieldID stays in PARTYMEMBER. See Task-15 notes. |
| common/PartyData.h | `>= 95` (×3 blocks: TOWNPORTAL.m_nSKillID, adwFieldID[6], PQReward tail) | 0x12A (298) | ✅ all gates correct | PARTYDATA::Decode @ 0x4EB819 `push 12Ah; call DecodeBuffer` ⇒ 298 (called from CWvsContext::OnPartyResult sub_A89CF3); IDENTICAL to v83 (0x12A @ 0x4E43E8). 298 = PARTYMEMBER(202,with adwFieldID) + aTownPortal[6](6×16, no m_nSKillID). v95 = 0x17A (all `>=95` present). `>=95` blocks ABSENT in v84. See Task-15 notes. |
| common/SecondaryStat.h | `>=87`/`==87`/`>=95` | **0xD20 (3360)** (v83=0xCD8, v87=0xD74) | ⚠️ **NEEDS CHANGE** — size-critical (intermediate; `>=87` block half-present) | ctor sub_79ED3E eh-vector `lea eax,[esi+0CE8h]` ⇒ aTemporaryStat[7] @ 0xCE8 ⇒ 0xD20. v84 carries nFlying_+nFrozen_ (6 tears @ 0xCA0-0xCE7) but NOT nAssistCharge_/nEnrage_; nDojangShield_ & `==87` byte-variants absent. See Task-14 SecondaryStat note. |

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
