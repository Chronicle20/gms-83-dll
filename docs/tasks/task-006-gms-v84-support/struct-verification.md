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

## The 22 version-gated headers

For each, record: v84 size, which gated fields are present/absent in v84, the
disassembly anchor, and the verdict (gate correct / gate needs change).

| Header | Existing thresholds in file | v84 size | Verdict | Evidence |
|---|---|---|---|---|
| common/CWvsApp.h | `>= 87`, `>= 95` | 0x60 (96) — v83-side layout | ✅ all gates correct | ctor @ 0xA3D719 (v83-identical early layout); see Task-12 notes |
| common/CWvsContext.h | `> 83`, `>= 87`, `>= 95` | ~0x3920 (≈14624; upper bound 0x3F70) | ✅ all gates correct | OnConnect @ 0x499DCD; ctor sub_A4BF0B; see Task-12 notes |
| common/CClientSocket.h | `>= 111` | 0x94 (148) | ✅ gate correct | CreateInstance @ 0xA43D0B `push 94h`; see Task-12 notes |
| common/CLogin.h | `>= 95`, `== 83` | 0x28C (652) | ✅ all gates correct | alloc `push 28Ch` @ 0xA6FD1C; ctor @ 0x608B15; see Task-12 notes |
| common/CMapLoadable.h | _audit_ | | ☐ | README-critical |
| common/CLogo.h | _audit_ | | ☐ | |
| common/CMob.h | _audit_ | | ☐ | |
| common/MobStat.h | _audit_ | | ☐ | |
| common/ConfigSysOpt.h | _audit_ | | ☐ | |
| common/CFuncKeyMappedMan.h | _audit_ | | ☐ | |
| common/CFadeWnd.h | `>= 87` (+ `REGION_GMS` `m_bUserAlarm`) | 0xD4 (212) | ✅ all gates correct | v84 ctor sub_52ACD8: `mov [esi+0C8h],eax` (m_sInviter) is highest field → m_dwSN @ 0xCC adjacent (no `>=87` 3-int block). See Task-13 notes. |
| common/CCtrlButton.h | `>= 95` | 0x5BC (1468) — embeds v84 CUIToolTip (0x52C) | ✅ gate correct | v84 m_uiToolTip @ 0x8C (`lea ecx,[esi+8Ch]; call sub_91CA58` @ 0x4C5B0D / 0x8475F9); base CCtrlWnd 0x34; `>=95` `m_sToolTipFromData` absent. See Task-13 notes. |
| common/CCtrlCheckBox.h | `>= 95` | 0x6C (108) — v83-identical | ✅ gate correct | v84 ctor sub_46C894: eh-vector `m_apCanvasCheckBox[4]` @ 0x5C ends 0x6C; nothing written past → `>=95` `m_nTextOffsetX/Y` absent. See Task-13 notes. |
| common/CUIWnd.h | `>= 95` | 0x5C8 (1480) — embeds v84 CUIToolTip (0x52C) | ✅ gate correct | base CWnd 0x6C; m_uiToolTip embed; v83-side trivially (84<95); `>=95` 5-field mid-block absent. See Task-13 notes. |
| common/CWnd.h | `>= 87` (`SECPOINT m_ptCursorRel`), `>= 95` (`UIOrigin m_origin`) | 0x6C (108) | ✅ all gates correct | v84 CWnd ctor sub_A26609 (m_pBackgrnd @ 0x68 last); CDialog ctor sub_4F6020 writes its 1st own member @ 0x6C ⇒ CWnd ends @ 0x6C (no `>=95` m_origin). `>=87` cursor is 8 B either form (layout-neutral). See Task-13 notes. |
| common/CUIToolTip.h | `>= 83`, **`>= 87` (×4)**, `>= 95` (×6) | **0x52C (1324)** | ⚠️ **NEEDS CHANGE** (2 fields mis-gated `>=87`) | v84 ctor sub_91A417: CLineInfo=0x20 (eh-vec `push 20h`), no m_aOptionLineInfo, **21 fonts** (0x428–0x478) incl `m_pFontGen_Unknown`, **`m_pCanvasEquip_Durability[2][2]` PRESENT @ 0x518**. See Task-13 CUIToolTip note. |
| common/CUILoginStart.h | `>= 95` / `REGION_JMS` | 0x100 (256) — v83-identical | ✅ all gates correct | v84 ctor sub_623270: m_pLogin @ 0x7C, m_aBtParam[5] eh-vec @ 0x80 (size 0x10) adjacent ⇒ `>=95`/JMS 2 com_ptrs absent. See Task-13 notes. |
| common/CUITitle.h | `>= 95` (base CFadeWnd + `m_rcRMA`) | 0x604 (1540) — embeds v84 CUIToolTip (0x52C) | ✅ all gates correct | v84 ctor sub_635785: base is CDialog (sub_4F6020, not CFadeWnd); m_pCanvasRMA[2] @ 0x88 → buttons @ 0x90 adjacent (no `m_rcRMA`); m_uiToolTipTitle @ 0xD8. See Task-13 notes. |
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
