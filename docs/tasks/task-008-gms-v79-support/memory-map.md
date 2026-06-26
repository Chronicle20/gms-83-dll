# GMS v79.1 — Memory Map Porting Plan

This is the working plan for producing `memory_maps/GMS/v79_1.cmake`. It defines
the per-key resolution strategy, the IDB-labeling protocol, and a tracking table
for every key parsed from `include/memory_map.h.in`. The closest labeled anchor
is **GMS v83** (the only nearby reference, and it sits *above* v79 — there is no
labeled IDB below v79); **v87** and the **v95 PDB** are secondary references for
the upper-gate cross-checks only.

## Resolution method (per key)

1. Look up the symbol name / address in the **v83 reference** (most completely
   labeled). Note the function's distinctive anchors: referenced strings, called
   imports, constants, vtable slot, call-graph neighbors.
2. In the **v79 IDB** (confirm with `get_metadata` first), locate the equivalent
   via a signature in priority order:
   - **String xref** — most robust across versions; a unique format/literal the
     function references. (Older builds may carry a *different* literal — note it.)
   - **Import/API call anchor** — e.g. the only function calling `socket` +
     `connect` in a given module region.
   - **Call-graph anchor** — child/parent of an already-resolved function.
   - **Constant / opcode** — a magic number, a `push <opcode>` immediate.
   - **Byte/structure signature** (`make_signature`) — last resort; least
     version-stable but precise when codegen matches.
3. Record the v79 address; **label it in the v79 IDB** (`rename`, `set_type` if
   prototype known).
4. Add the heuristic to `signature-catalog.md`.
5. Write the `set(KEY 0x…)` line to `v79_1.cmake`.

For **offset** keys, disassemble the v79 host function and re-measure the offset
to the target instruction/branch — never copy the v83 offset.

## IDB labeling protocol

- Always `get_metadata` before a probe; do not infer the connected IDB from
  conversation ([[feedback-verify-ida-target]]).
- Apply the v83 canonical name verbatim so cross-version greps line up.
- `idb_save` at checkpoints (e.g. every subsystem group) so labels survive a
  swap/restart.
- Labeling functions/globals is encouraged. **Do not** apply speculative struct
  *types* into the IDB during the struct-verification pass (decompiler leak —
  see `struct-verification.md`).

## Backward-direction sentinel handling

Going from v83 *down* to v79, the sentinel logic inverts relative to the v84
port: a feature that v83 *has* may not exist yet in v79. For each v83 sentinel,
confirm v79 is also absent; and stay alert for v83 *real-address* keys whose
backing feature is missing in the older build (those become **new v79 sentinels**,
and the consuming gate/edit must tolerate `0`).

| Key | v83 disposition | v79 action |
|---|---|---|
| `C_BATTLE_RECORD_MAN_CREATE_INSTANCE` | sentinel (v95+ feature) | Confirm absent (almost certainly — it post-dates v83) — carry `0x00000000` |
| `DR_CHECK` | sentinel ("does not exist") | Confirm absent — carry `0x00000000` |
| `DR_INIT` | sentinel (DR subsystem absent in v83) | Confirm absent — carry `0x00000000` |
| `CE_TRACER_RUN` | sentinel ("does not exist") | Confirm absent — carry `0x00000000` |
| `RESET_LSP` | real addr `0x0044ED47` (`# does not exist` comment is stale; v84 found it present) | **Verify in v79** — present or absent? Resolve, don't inherit the stale comment |
| `C_SECURITY_CLIENT_ON_PACKET_RET_STUB` | JMS sentinel | Carry `0x00000000` (GMS build) |
| `C_SECURITY_CLIENT_ON_PACKET_CHECK` | JMS sentinel | Carry `0x00000000` |
| `C_SECURITY_CLIENT_ON_PACKET_CHECK_OFFSET` | JMS sentinel | Carry `0x00000000` |
| `C_WVS_APP_INITIALIZE_GR2D_WINDOWED_OFFSET` | JMS sentinel | Carry `0x00000000` |
| `WIN_MAIN_LAUNCHER_STUB` | JMS sentinel | Carry `0x00000000` |
| `C_FILE_STREAM_*` (7 keys) | relay gated off (`0`) in v83 — helpers unrecoverable | Decide for v79: recoverable or carry `0` like v83. Document. |
| *(any v83 real key absent in v79)* | n/a | If a v83 feature does not exist in v79, the key becomes a new sentinel — flag for the gate/edit owner |

## Key tracking (full set, seeded from `v83_1.cmake`)

Status legend: ☐ todo · ◐ located, IDB labeled · ✔ written to cmake +
catalogued. Group order follows `include/memory_map.h.in`. The **v83 value**
column is the *seed* — every row must be re-confirmed against v79 by signature,
never copied blind.

| Key | Class | v83 value | Status |
|---|---|---|---|
| VERSION_HEADER | constant | 8 | ☐ |
| PLAYER_LOGGED_IN | opcode | 0x14 | ☐ |
| CLIENT_START_ERROR | opcode | 0x19 | ☐ |
| GET_SE_PRIVILEGE | addr | 0x0044E824 | ☐ |
| C_ACTION_MAN_CREATE_INSTANCE_ADDR | addr | 0x009F9DA6 | ☐ |
| C_ACTION_MAN_INSTANCE_ADDR | addr | 0x00BE78D4 | ☐ |
| C_ACTION_MAN_INIT | addr | 0x00406ABD | ☐ |
| C_ACTION_MAN_SWEEP_CACHE | addr | 0x00411BBB | ☐ |
| C_ANIMATION_DISPLAYER_CREATE_INSTANCE | addr | 0x009F9DFC | ☐ |
| C_CLIENT_SOCKET_INSTANCE_ADDR | addr | 0x00BE7914 | ✔ (v79 0x00B07844; SBB-singleton store in ctor, g_pClientSocketInstance) |
| C_CLIENT_SOCKET_CREATE_INSTANCE | addr | 0x009F9E53 | ✔ (v79 0x00946AB6; TSingleton::CreateInstance Alloc(0x94)+ctor) |
| C_CLIENT_SOCKET_SEND_PACKET | addr | 0x0049637B | ✔ (v79 0x0048DF93; symbol+MakeBufferList(79)->innoHash->Flush) |
| C_CLIENT_SOCKET_FLUSH | addr | 0x00496403 | ✔ (v79 0x0048E01B; symbol+send-ZList via cloned send slot) |
| C_CLIENT_SOCKET_MANIPULATE_PACKET | addr | 0x0049651D | ✔ (v79 0x0048E135; symbol+sole ProcessPacket caller) |
| C_CLIENT_SOCKET_PROCESS_PACKET | addr | 0x004965F1 | ✔ (v79 0x0048E209; symbol+Decode2 jump-table) |
| C_CLIENT_SOCKET_CLOSE | addr | 0x00496369 | ✔ (v79 0x0048DF81; symbol+ClearCtx+CloseSocket) |
| C_CLIENT_SOCKET_CLEAR_SEND_RECEIVE_CTX | addr | 0x004969EE | ✔ (v79 0x0048E5D7; symbol+double ZList RemoveAll) |
| C_CLIENT_SOCKET_ON_CONNECT | addr | 0x00494ED1 | ✔ (v79 0x0048CB81; symbol; ver byte==8/major==0x4F; NO 8-byte client key) |
| C_CLIENT_SOCKET_CONNECT_LOGIN | addr | 0x00494931 | ✔ (v79 0x0048C773; symbol; GetCmdLine+server-pick->Connect(CTX)) |
| C_CLIENT_SOCKET_CONNECT_CTX | addr | 0x00494CA3 | ✔ (v79 0x0048C9CA; symbol; CONNECTCONTEXT wrapper) |
| C_CLIENT_SOCKET_CONNECT_ADR | addr | 0x00494D2F | ✔ (v79 0x0048CA56; symbol; sole socket(2,1,0) caller) |
| Z_SOCKET_BASE_CLOSE_SOCKET | addr | 0x00494857 | ✔ (v79 0x0048C699; symbol; shutdown+closesocket pair) |
| Z_SOCKET_BUFFER_ALLOC | addr | 0x00495FD2 | ✔ (v79 0x0048DBEA; symbol; dual Alloc(a1,28)) |
| C_CONFIG | addr | 0x0049C213 | ☐ |
| C_CONFIG_INSTANCE_ADDR | addr | 0x00BEBF9C | ☐ |
| C_CONFIG_GET_PARTNER_CODE | addr | 0x005F6CFB | ☐ |
| C_CONFIG_APPLY_SYS_OPT | addr | 0x0049EA33 | ☐ |
| C_CONFIG_CHECK_EXEC_PATH_REG | addr | 0x0049CCF3 | ☐ |
| C_CONFIG_SYS_OPT_WINDOWED_MODE | addr | 0x00BF1AC8 | ☐ |
| C_FUNC_KEY_MAPPED_MAN | addr | 0x0058DD0D | ☐ |
| C_FUNC_KEY_MAPPED_MAN_VFTABLE | addr | 0x00AF5650 | ☐ |
| C_FUNC_KEY_MAPPED_MAN_INSTANCE_ADDR | addr | 0x00BED5A0 | ☐ |
| C_FUNC_KEY_MAPPED_MAN_CREATE_INSTANCE | addr | 0x009F9E98 | ☐ |
| DEFAULT_FKM_INSTANCE_ADDR | addr | 0x00BD8BCC | ☐ |
| DEFAULT_QKM_INSTANCE_ADDR | addr | 0x00BD8D8C | ☐ |
| C_INPUT_SYSTEM | addr | 0x009F821F | ☐ |
| C_INPUT_SYSTEM_CREATE_INSTANCE | addr | 0x009F9A6A | ☐ |
| C_INPUT_SYSTEM_INSTANCE_ADDR | addr | 0x00BEC33C | ☐ |
| C_INPUT_SYSTEM_INIT | addr | 0x00599EBF | ☐ |
| C_INPUT_SYSTEM_UPDATE_DEVICE | addr | 0x0059A2E9 | ☐ |
| C_INPUT_SYSTEM_GET_IS_MESSAGE | addr | 0x0059A306 | ☐ |
| C_INPUT_SYSTEM_GENERATE_AUTO_KEY_DOWN | addr | 0x0059B2D2 | ☐ |
| C_INPUT_SYSTEM_SHOW_CURSOR | addr | 0x59A338 | ☐ |
| C_LOGIN_UPDATE | addr | 0x005F4C16 | ✔ (v79 0x005CA348; vtable[0] CLogin primary vtable 0xA2F9EC; body: [esi+0x15C]+CWnd::InvalidateRect; DIVERGES from C_LOGO_UPDATE in v79) |
| C_LOGIN_SEND_CHECK_PASSWORD_PACKET | addr | 0x005F6952 | ✔ (v79 0x005CBF50; IDB symbol) |
| C_LOGO | addr | 0x0062ECE2 | ✔ (v79 0x005FF8C4; IDB symbol ??0CLogo@@QAE@XZ; Alloc(0x258)+ctor in LogoEnd) |
| C_LOGO_GET_RTTI | addr | 0x0062ED26 | ✔ (v79 0x0042196A; IDB symbol; CLogo vtable CWnd-iface slot 53) |
| C_LOGO_IS_KIND_OF | addr | 0x0062ED2C | ✔ (v79 0x00421970; IDB symbol; CLogo vtable CWnd-iface slot 54) |
| C_LOGO_UPDATE | addr | 0x005F4C16 | ✔ (v79 0x005FFE54; vtable[0] CLogo primary vtable 0xA307BC; 1500ms timer body; DIVERGES from C_LOGIN_UPDATE in v79; v83 primary vtable off_AF7C80 slot0=0x62F2B6 ?Update@CLogo@@ — slot order matches v79: no drift) |
| C_LOGO_ON_MOUSE_BUTTON | addr | 0x0062F2A1 | ✔ (v79 0x005FFE3F; IUIMsgHandler vtable 0xA30770 slot 2; body: cmp 0x202(WM_LBUTTONUP)+InitNXLogo; v83 IUIMsgHandler vtable off_AF7C34 slot2=0x62F2A1 — slot order matches v79: no drift) |
| C_LOGO_ON_SET_FOCUS | addr | 0x0062ED20 | ✔ (v79 0x005FF902; IUIMsgHandler vtable 0xA30770 slot 1; body: push1;pop eax;retn 4; v83 IUIMsgHandler vtable off_AF7C34 slot1=0x62ED20 — slot order matches v79: no drift) |
| C_LOGO_ON_KEY | addr | 0x0062F27A | ✔ (v79 0x005FFE18; IUIMsgHandler vtable 0xA30770 slot 0; body: wParam==13/27/32+InitNXLogo; v83 IUIMsgHandler vtable off_AF7C34 slot0=0x62F27A — slot order matches v79: no drift) |
| C_LOGO_LOGO_END | addr | 0x0062EEAE | ✔ (v79 0x005FFA4C; call-graph: Alloc(0x258)+CLogin_ctor+SetStage) |
| C_LOGO_FORCED_END | addr | 0x0062EEF8 | ✔ (v79 0x005FFA2A; vtable[2] CLogo primary 0xA307C4; SET_STAGE calls [vtable+8] at 6f1b14; body: CSoundMan::PlayBGM(0x3E8); v83 primary vtable off_AF7C80 slot2=0x62EE8C, same PlayBGM(0x3E8) idiom (historical seed 0x62EEF8 imprecise) — slot order matches v79: no drift) |
| C_LOGO_INIT | addr | 0x0062EDDA | ✔ (v79 0x005FF9BC; vtable[1] CLogo primary 0xA307C0; SET_STAGE calls [vtable+4] at 6f1c2c; v83 primary vtable off_AF7C80 slot1=0x62EDDA — slot order matches v79: no drift) |
| C_LOGO_INIT_NX_LOGO | addr | 0x0062F396 | ✔ (v79 0x005FFA96; StringPool::GetBSTR(0x568) NX-logo path; init-once guard [this+0x28]) |
| C_MACRO_SYS_MAN_CREATE_INSTANCE | addr | 0x009F9EEE | ☐ |
| C_BATTLE_RECORD_MAN_CREATE_INSTANCE | sentinel | 0x00000000 | ☐ |
| C_MAPLE_TV_MAN_CREATE_INSTANCE | addr | 0x009F9F87 | ☐ |
| C_MAPLE_TV_MAN_INSTANCE_ADDR | addr | 0x00BED76C | ☐ |
| C_MAPLE_TV_MAN_INIT | addr | 0x00636F4E | ☐ |
| C_MONSTER_BOOK_MAN_CREATE_INSTANCE | addr | 0x009F9B73 | ☐ |
| C_MONSTER_BOOK_MAN_INSTANCE_ADDR | addr | 0x00BED610 | ☐ |
| C_MONSTER_BOOK_MAN_LOAD_BOOK | addr | 0x0068487C | ☐ |
| C_OUT_PACKET | addr | 0x006EC9CE | ✔ (v79 0x0067AD6B; symbol+_Alloc(256)+Init) |
| C_OUT_PACKET_ENCODE_1 | addr | 0x00406549 | ✔ (v79 0x004062C7; push1+store dl+inc; shared _EnsureCapacity) |
| C_OUT_PACKET_ENCODE_2 | addr | 0x00427F74 | ✔ (v79 0x0042539C; push2+store dx+add2; shared _EnsureCapacity) |
| C_OUT_PACKET_ENCODE_4 | addr | 0x004065A6 | ✔ (v79 0x00406324; push4+store edx+add4; shared _EnsureCapacity) |
| C_OUT_PACKET_ENCODE_STR | addr | 0x0046F3CF | ✔ (v79 0x004694DE; ZXString len+2+CIOBufferManipulator::EncodeStr) |
| C_OUT_PACKET_ENCODE_BUFFER | addr | 0x0046C00C | ✔ (v79 0x00466AE9; _EnsureCapacity+memcpy+len+=Size) |
| C_OUT_PACKET_MAKE_BUFFER_LIST | addr | 0x006ECB27 | ✔ (v79 0x0067AEC4; symbol+sole SendPacket call+1460/0x5B4 chunk) |
| C_IG_CIPHER_INNO_HASH | addr | 0x00A4A838 | ☐ |
| Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_CTOR | addr | 0x00403166 | ☐ |
| Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_DTOR | addr | 0x0040318B | ☐ |
| C_QUEST_MAN_CREATE_INSTANCE | addr | 0x009F9AC2 | ☐ |
| C_QUEST_MAN_INSTANCE_ADDR | addr | 0x00BED614 | ☐ |
| C_QUEST_MAN_LOAD_DEMAND | addr | 0x0071D8DF | ☐ |
| C_QUEST_MAN_LOAD_PARTY_QUEST_INFO | addr | 0x00723341 | ☐ |
| C_QUEST_MAN_LOAD_EXCLUSIVE | addr | 0x007247A1 | ☐ |
| C_QUICKSLOT_KEY_MAPPED_MAN | addr | 0x009FA0CB | ☐ |
| C_RADIO_MANAGER_CREATE_INSTANCE | addr | 0x009FA078 | ☐ |
| C_RADIO_MANAGER_INSTANCE_ADDR | addr | 0x00BF0B00 | ☐ (v84 found v83's value pointed at the allocator selector, not the instance — verify the real v79 instance global) |
| C_SECURITY_CLIENT_CREATE_INSTANCE | addr | 0x009F9F42 | ☐ |
| C_SECURITY_CLIENT_INSTANCE_ADDR | addr | 0x00BEC3A8 | ☐ |
| C_SECURITY_CLIENT_ON_PACKET | addr | 0x00A4BF03 | ☐ |
| STAGE_INSTANCE_ADDR | addr | 0x00BEDED4 | ✔ (v79 0x00B0DADC; written by SET_STAGE at 6f1aec; xrefs_to confirmed) |
| SET_STAGE | addr | 0x00777347 | ✔ (v79 0x006F1AC0; IDB symbol SetStage; stores to B0DADC+vtable dispatch) |
| GR_INSTANCE_ADDR | addr | 0x00BF14EC | ✔ (v79 0x00B10F74; stored by sub_947BB8 in InitializeGr2D at 944cca; mov ebx,dword_B10F74+vtable dispatch) |
| RESET_LSP | addr/sentinel | 0x0044ED47 | ☐ (resolve present/absent in v79; stale v83 comment) |
| C_STAGE_ON_MOUSE_ENTER | addr | 0x00775FC7 | ✔ (v79 0x0092F3F8; IDB symbol ?OnMouseEnter@CStage@@UAEXH@Z; direct v79 read: CLogin secondary vtable 0xA2FA08 slot 6 (0xA2FA20)=0x0092F3F8 — inherited, CLogin does not override) |
| C_STAGE_ON_PACKET | addr | 0x00775FE6 | ✔ (v79 0x006F079F; IDB symbol ?OnPacket@CStage@@UAEXJAAVCInPacket@@@Z; CLogo [this+8] vtable 0xA3076C slot0=0x006F079F) |
| C_SYSTEM_INFO | addr | 0x00A54B90 | ☐ |
| C_SYSTEM_INFO_INIT | addr | 0x00A54BD0 | ☐ |
| C_SYSTEM_INFO_GET_GAME_ROOM_CLIENT | addr | 0x00A54FB0 | ☐ |
| C_SYSTEM_INFO_GET_MACHINE_ID | addr | 0x00A54EB0 | ☐ |
| C_UI_TITLE_INSTANCE_ADDR | addr | 0x00BEDA60 | ✔ (v79 0x00B0D738; sub_5F652C (CUITitle ctor) stores this via SBB null-check; dtor at loc_5FD04E clears it; CLogin::ForcedEnd destroys it) |
| G_DW_TARGET_OS | addr | 0x00BE2EBC | ✔ (v79 0x00B0239C; g_dwTargetOS=1996 writer in ctor) |
| C_WVS_APP | addr | 0x009F4FDA | ✔ (v79 0x00942D3B; symbol+WebStart/IsWow64Process) |
| C_WVS_APP_INSTANCE_ADDR | addr | 0x00BE7B38 | ✔ (v79 0x00B07A68; g_CWvsApp singleton store) |
| C_WVS_APP_IS_MSG_PROC | addr | 0x009F97BC | ✔ (v79 0x00946430; msg 0x100/0x1FF-0x20A dispatch) |
| C_WVS_APP_INITIALIZE_AUTH | addr | 0x009F7097 | ✔ SENTINEL (v79 0x0: NMCO auth absent — FLAG gate owner) |
| C_WVS_APP_INITIALIZE_PCOM | addr | 0x009F6D77 | ✔ (v79 0x0094409B) |
| C_WVS_APP_CREATE_MAIN_WINDOW | addr | 0x009F6D97 | ✔ (v79 0x009440BB) |
| C_WVS_APP_CONNECT_LOGIN | addr | 0x009F6F27 | ✔ (v79 0x0094424B) |
| C_WVS_APP_INITIALIZE_RES_MAN | addr | 0x009F7159 | ✔ (v79 0x009443BB) |
| C_WVS_APP_INITIALIZE_GR2D | addr | 0x009F7A3B | ✔ (v79 0x00944C91) |
| C_WVS_APP_INITIALIZE_INPUT | addr | 0x009F7CE1 | ✔ (v79 0x00944F37) |
| C_WVS_APP_INITIALIZE_SOUND | addr | 0x009F82BC | ✔ (v79 0x009452A1) |
| C_WVS_APP_INITIALIZE_GAME_DATA | addr | 0x009F8B61 | ✔ (v79 0x00945834) |
| C_WVS_APP_CREATE_WND_MANAGER | addr | 0x009F7034 | ✔ (v79 0x00944358) |
| C_WVS_APP_GET_CMD_LINE | addr | 0x009F94A1 | ✔ (v79 0x0094611A) |
| C_WVS_APP_DIR_BACK_SLASH_TO_SLASH | addr | 0x009F95FE | ✔ (v79 0x00946277) |
| C_WVS_APP_DIR_UP_DIR | addr | 0x009F9644 | ✔ (v79 0x009462BD) |
| C_WVS_APP_DIR_SLASH_TO_BACK_SLASH | addr | 0x009F9621 | ✔ (v79 0x0094629A) |
| C_WVS_APP_GET_EXCEPTION_FILE_NAME | addr | 0x009F9808 | ✔ (v79 0x00946481) |
| C_WVS_APP_CALL_UPDATE | addr | 0x009F84D0 | ✔ (v79 0x009454B5) |
| C_WVS_APP_RUN | addr | 0x009F5C50 | ✔ (v79 0x00943611; symbol+exception-TI quad) |
| C_WVS_APP_SET_UP | addr | 0x009F5239 | ✔ (v79 0x009430F1; NO DR_init present — R11 confirmed) |
| C_WVS_CONTEXT_INSTANCE_ADDR | addr | 0x00BE7918 | ☐ |
| C_WVS_CONTEXT_ON_ENTER_GAME | addr | 0x00A03935 | ☐ |
| C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET | offset | 0x10 | ☐ (re-measure) |
| WIN_MAIN | addr | 0x009F19F2 | ✔ (v79 0x0093F9B7; title-string+start call-graph) |
| WIN_MAIN_AD_BALLOON_CONDITIONAL | offset | 0xA3D | ✔ (re-measured v79 0xA3D; jz@0x9403F4) |
| WIN_MAIN_PATCHER_OFFSET | offset | 0x212 | ✔ (re-measured v79 0x212; call ShowStartUpWndModal@0x93FBC9) |
| C_WND_MAN_S_UPDATE | addr | 0x009E47C3 | ✔ (v79 0x00932EE2) |
| C_WND_MAN_REDRAW_INVALIDATED_WINDOWS | addr | 0x009E4547 | ✔ (v79 0x00932C66) |
| Z_ARRAY_REMOVE_ALL | addr | 0x00428CF1 | ☐ |
| Z_X_STRING_GET_BUFFER | addr | 0x00414617 | ☐ |
| Z_X_STRING_TRIM_RIGHT | addr | 0x00474414 | ☐ |
| Z_X_STRING_TRIM_LEFT | addr | 0x004744C9 | ☐ |
| C_FIELD_SEND_JOIN_PARTY_MSG | addr | 0x0052FECF | ☐ |
| C_FIELD_SEND_JOIN_PARTY_MSG_OFFSET | offset | 0x65 | ☐ (re-measure) |
| C_FIELD_SEND_CREATE_NEW_PARTY_MSG | addr | 0x52FCE1 | ☐ |
| C_FIELD_SEND_CREATE_NEW_PARTY_MSG_OFFSET | offset | 0xA4 | ☐ (re-measure) |
| C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST | addr | 0x00A12522 | ☐ |
| C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST_OFFSET | offset | 0xE9 | ☐ (re-measure) |
| DR_CHECK | sentinel | 0x00000000 | ☐ (confirm absent) |
| DR_INIT | sentinel | 0x00000000 | ☐ (confirm absent) |
| CE_TRACER_RUN | sentinel | 0x00000000 | ☐ (confirm absent) |
| SEND_HS_LOG | addr | 0x009F191B | ✔ (v79 0x0093F8E0; %s\HShield) |
| C_MOB_C_MOB | addr | 0x006621D9 | ☐ |
| C_SECURITY_CLIENT_ON_PACKET_RET_STUB | JMS sentinel | 0x00000000 | ☐ |
| C_SECURITY_CLIENT_ON_PACKET_CHECK | JMS sentinel | 0x00000000 | ☐ |
| C_SECURITY_CLIENT_ON_PACKET_CHECK_OFFSET | JMS sentinel | 0x00000000 | ☐ |
| C_WVS_APP_INITIALIZE_GR2D_WINDOWED_OFFSET | JMS sentinel | 0x00000000 | ☐ |
| WIN_MAIN_LAUNCHER_STUB | JMS sentinel | 0x00000000 | ☐ |
| C_TI_DISCONNECT_EXCEPTION | addr | 0x00B48858 | ☐ |
| C_TI_TERMINATE_EXCEPTION | addr | 0x00B44760 | ☐ |
| C_TI_PATCH_EXCEPTION | addr | 0x00B52FC8 | ☐ |
| C_TI_ZEXCEPTION | addr | 0x00B44EE0 | ☐ |
| C_PATCH_EXCEPTION_BUILDER | addr | 0x0051E834 | ☐ |
| C_COM_RAISE_ERROR_EX | addr | 0x00A5FDE4 | ☐ |
| C_FILE_STREAM_RESOLVED | gate | 0 | ☐ (decide v79 relay disposition) |
| C_FILE_STREAM_OPEN_INLINE | gate | 0 | ☐ |
| C_FILE_STREAM_OPEN | addr | 0x0 | ☐ |
| C_FILE_STREAM_GET_LENGTH | addr | 0x0 | ☐ |
| C_FILE_STREAM_READ | addr | 0x0 | ☐ |
| C_FILE_STREAM_CLOSE | addr | 0x0 | ☐ |
| C_FILE_STREAM_VFTABLE | addr | 0x0 | ☐ |

> **Completeness is CI-enforced.** Whatever the exact templated-key count parsed
> from `include/memory_map.h.in`, the key-completeness check fails the build if any
> required key is undefined or empty. Reconcile this table against the live header
> at the start of the task (`grep -oE '@[A-Z_]+@' include/memory_map.h.in`) so no
> newly-added key is missed — the set grows between ports (the exception-dispatch
> and CFileStream keys were added after the v84 port).

## Completion (completeness gate)

- Every tracking row marked `✔` (written to `v79_1.cmake` + catalogued); zero
  `☐`/`◐` remain.
- The CMake key-completeness check for GMS 79.1 reports all required keys defined
  and non-empty.
- `signature-catalog.md` carries a determination for every absolute key (offsets
  record host fn + target instruction + measured delta; sentinels record
  absent/present + evidence).
- v79 IDB labelled for all resolved functions/globals and `idb_save`'d.
- Any latent v83-map issues surfaced during the port (e.g. the
  `C_RADIO_MANAGER_INSTANCE_ADDR` / `RESET_LSP` quirks the v84 port flagged) are
  recorded as findings for the user, not silently propagated.

## Suggested resolution order

Per README §"Adding a new version": **WinMain → CWvsApp** (anchor the image,
gain call-graph reach), then **CClientSocket/ZSocket** and **COutPacket**
(highest value for redirect/bypass), then login/stage/logo, then the manager
singletons, then utilities, then exception-dispatch keys, then sentinels last.
</content>
