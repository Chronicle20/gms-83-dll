# GMS v84.1 — Memory Map Porting Plan

This is the working plan for producing `memory_maps/GMS/v84_1.cmake`. It defines
the per-key resolution strategy, the IDB-labeling protocol, and a tracking table
for all 145 keys. The closest labeled anchor is **GMS v83**; **v87** and the
**v95 PDB** are secondary references.

## Resolution method (per key)

1. Look up the symbol name / address in the **v83 reference** (it is the most
   completely labeled). Note the function's distinctive anchors: referenced
   strings, called imports, constants, vtable slot, call-graph neighbors.
2. In the **v84 IDB** (confirm with `get_metadata` first), locate the equivalent
   via a signature in priority order:
   - **String xref** — most robust across versions; a unique format/literal the
     function references.
   - **Import/API call anchor** — e.g. the only function calling `socket` +
     `connect` in a given module region.
   - **Call-graph anchor** — child/parent of an already-resolved function.
   - **Constant / opcode** — a magic number, a `push <opcode>` immediate.
   - **Byte/structure signature** (`make_signature`) — last resort; least
     version-stable but precise when codegen matches.
3. Record the v84 address; **label it in the v84 IDB** (`rename`, `set_type` if
   prototype known).
4. Add the heuristic to `signature-catalog.md`.
5. Write the `set(KEY 0x…)` line to `v84_1.cmake`.

For **offset** keys, disassemble the v84 host function and re-measure the offset
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

## Sentinel handling

These v83 keys are `0x00000000`. For each, confirm the v84 disposition before
carrying the sentinel forward:

| Key | v83 note | v84 action |
|---|---|---|
| `C_BATTLE_RECORD_MAN_CREATE_INSTANCE` | absent | Confirm absent; else locate |
| `RESET_LSP` | "does not exist" | Confirm |
| `DR_CHECK` | "does not exist" | Confirm |
| `CE_TRACER_RUN` | "does not exist" | Confirm |
| `C_SECURITY_CLIENT_ON_PACKET_RET_STUB` | JMS only | Carry `0x00000000` |
| `C_SECURITY_CLIENT_ON_PACKET_CHECK` | JMS only | Carry `0x00000000` |
| `C_SECURITY_CLIENT_ON_PACKET_CHECK_OFFSET` | JMS only | Carry `0x00000000` |
| `C_WVS_APP_INITIALIZE_GR2D_WINDOWED_OFFSET` | JMS only | Carry `0x00000000` |
| `WIN_MAIN_LAUNCHER_STUB` | JMS only | Carry `0x00000000` |

## Key tracking (all 145)

Status legend: ☐ todo · ◐ located, IDB labeled · ✔ written to cmake +
catalogued. Group order follows `include/memory_map.h.in`.

| Key | Class | v83 value | Status |
|---|---|---|---|
| VERSION_HEADER | constant | 8 | ☐ |
| PLAYER_LOGGED_IN | opcode | 0x14 | ☐ |
| CLIENT_START_ERROR | opcode | 0x19 | ☐ |
| GET_SE_PRIVILEGE | addr | 0x0044E824 | ✔ 0x0044FEF9 |
| C_ACTION_MAN_CREATE_INSTANCE_ADDR | addr | 0x009F9DA6 | ✔ 0x00A43C5E |
| C_ACTION_MAN_INSTANCE_ADDR | addr | 0x00BE78D4 | ✔ 0x00C40C24 |
| C_ACTION_MAN_INIT | addr | 0x00406ABD | ✔ 0x00406B93 |
| C_ACTION_MAN_SWEEP_CACHE | addr | 0x00411BBB | ✔ 0x00411CA9 |
| C_ANIMATION_DISPLAYER_CREATE_INSTANCE | addr | 0x009F9DFC | ✔ 0x00A43CB4 |
| C_CLIENT_SOCKET_INSTANCE_ADDR | addr | 0x00BE7914 | ✔ 0x00C40C64 |
| C_CLIENT_SOCKET_CREATE_INSTANCE | addr | 0x009F9E53 | ✔ 0x00A43D0B |
| C_CLIENT_SOCKET_SEND_PACKET | addr | 0x0049637B | ✔ 0x0049B28C |
| C_CLIENT_SOCKET_FLUSH | addr | 0x00496403 | ✔ 0x0049B314 |
| C_CLIENT_SOCKET_MANIPULATE_PACKET | addr | 0x0049651D | ✔ 0x0049B42E |
| C_CLIENT_SOCKET_PROCESS_PACKET | addr | 0x004965F1 | ✔ 0x0049B502 |
| C_CLIENT_SOCKET_CLOSE | addr | 0x00496369 | ✔ 0x0049B27A |
| C_CLIENT_SOCKET_CLEAR_SEND_RECEIVE_CTX | addr | 0x004969EE | ✔ 0x0049B903 |
| C_CLIENT_SOCKET_ON_CONNECT | addr | 0x00494ED1 | ✔ 0x00499DCD |
| C_CLIENT_SOCKET_CONNECT_LOGIN | addr | 0x00494931 | ✔ 0x00499824 |
| C_CLIENT_SOCKET_CONNECT_CTX | addr | 0x00494CA3 | ✔ 0x00499B9F |
| C_CLIENT_SOCKET_CONNECT_ADR | addr | 0x00494D2F | ✔ 0x00499C2B |
| Z_SOCKET_BASE_CLOSE_SOCKET | addr | 0x00494857 | ✔ 0x0049974A |
| Z_SOCKET_BUFFER_ALLOC | addr | 0x00495FD2 | ✔ 0x0049AEE3 |
| C_CONFIG | addr | 0x0049C213 | ✔ 0x004A127C |
| C_CONFIG_INSTANCE_ADDR | addr | 0x00BEBF9C | ✔ 0x00C452EC |
| C_CONFIG_GET_PARTNER_CODE | addr | 0x005F6CFB | ✔ 0x0060BC34 |
| C_CONFIG_APPLY_SYS_OPT | addr | 0x0049EA33 | ✔ 0x004A3A9C |
| C_CONFIG_CHECK_EXEC_PATH_REG | addr | 0x0049CCF3 | ✔ 0x004A1D5C |
| C_CONFIG_SYS_OPT_WINDOWED_MODE | addr | 0x00BF1AC8 | ✔ 0x00C4B150 |
| C_FUNC_KEY_MAPPED_MAN | addr | 0x0058DD0D | ✔ 0x0059DD00 |
| C_FUNC_KEY_MAPPED_MAN_VFTABLE | addr | 0x00AF5650 | ✔ 0x00B46B08 |
| C_FUNC_KEY_MAPPED_MAN_INSTANCE_ADDR | addr | 0x00BED5A0 | ✔ 0x00C46B70 |
| C_FUNC_KEY_MAPPED_MAN_CREATE_INSTANCE | addr | 0x009F9E98 | ✔ 0x00A43D50 |
| DEFAULT_FKM_INSTANCE_ADDR | addr | 0x00BD8BCC | ✔ 0x00C31C7C |
| DEFAULT_QKM_INSTANCE_ADDR | addr | 0x00BD8D8C | ✔ 0x00C31E3C |
| C_INPUT_SYSTEM | addr | 0x009F821F | ✔ 0x00A41A7B |
| C_INPUT_SYSTEM_CREATE_INSTANCE | addr | 0x009F9A6A | ✔ 0x00A43922 |
| C_INPUT_SYSTEM_INSTANCE_ADDR | addr | 0x00BEC33C | ✔ 0x00C456B4 |
| C_INPUT_SYSTEM_INIT | addr | 0x00599EBF | ✔ 0x005AA112 |
| C_INPUT_SYSTEM_UPDATE_DEVICE | addr | 0x0059A2E9 | ✔ 0x005AA53C |
| C_INPUT_SYSTEM_GET_IS_MESSAGE | addr | 0x0059A306 | ✔ 0x005AA559 |
| C_INPUT_SYSTEM_GENERATE_AUTO_KEY_DOWN | addr | 0x0059B2D2 | ✔ 0x005AB525 |
| C_INPUT_SYSTEM_SHOW_CURSOR | addr | 0x59A338 | ✔ 0x005AA58B |
| C_LOGIN_UPDATE | addr | 0x005F4C16 | ✔ 0x00609A9F |
| C_LOGIN_SEND_CHECK_PASSWORD_PACKET | addr | 0x005F6952 | ✔ 0x0060B88B |
| C_LOGO | addr | 0x0062ECE2 | ✔ 0x0064417C |
| C_LOGO_GET_RTTI | addr | 0x0062ED26 | ✔ 0x006441C0 |
| C_LOGO_IS_KIND_OF | addr | 0x0062ED2C | ✔ 0x006441C6 |
| C_LOGO_UPDATE | addr | 0x005F4C16 | ✔ 0x00609A9F |
| C_LOGO_ON_MOUSE_BUTTON | addr | 0x0062F2A1 | ✔ 0x0064473B |
| C_LOGO_ON_SET_FOCUS | addr | 0x0062ED20 | ✔ 0x006441BA |
| C_LOGO_ON_KEY | addr | 0x0062F27A | ✔ 0x00644714 |
| C_LOGO_LOGO_END | addr | 0x0062EEAE | ✔ 0x00644348 |
| C_LOGO_FORCED_END | addr | 0x0062EEF8 | ✔ 0x00644392 |
| C_LOGO_INIT | addr | 0x0062EDDA | ✔ 0x00644274 |
| C_LOGO_INIT_NX_LOGO | addr | 0x0062F396 | ✔ 0x00644830 |
| C_MACRO_SYS_MAN_CREATE_INSTANCE | addr | 0x009F9EEE | ✔ 0x00A43DA6 |
| C_BATTLE_RECORD_MAN_CREATE_INSTANCE | sentinel | 0x00000000 | ☐ |
| C_MAPLE_TV_MAN_CREATE_INSTANCE | addr | 0x009F9F87 | ✔ 0x00A43E3F |
| C_MAPLE_TV_MAN_INSTANCE_ADDR | addr | 0x00BED76C | ✔ 0x00C46D64 |
| C_MAPLE_TV_MAN_INIT | addr | 0x00636F4E | ✔ 0x0064C578 |
| C_MONSTER_BOOK_MAN_CREATE_INSTANCE | addr | 0x009F9B73 | ✔ 0x00A43A2B |
| C_MONSTER_BOOK_MAN_INSTANCE_ADDR | addr | 0x00BED610 | ✔ 0x00C46BE0 |
| C_MONSTER_BOOK_MAN_LOAD_BOOK | addr | 0x0068487C | ✔ 0x0069B498 |
| C_OUT_PACKET | addr | 0x006EC9CE | ✔ 0x00703CFA |
| C_OUT_PACKET_ENCODE_1 | addr | 0x00406549 | ✔ 0x0040661F |
| C_OUT_PACKET_ENCODE_2 | addr | 0x00427F74 | ✔ 0x00428A68 |
| C_OUT_PACKET_ENCODE_4 | addr | 0x004065A6 | ✔ 0x0040667C |
| C_OUT_PACKET_ENCODE_STR | addr | 0x0046F3CF | ✔ 0x00471EB0 |
| C_OUT_PACKET_ENCODE_BUFFER | addr | 0x0046C00C | ✔ 0x0046E5FE |
| C_OUT_PACKET_MAKE_BUFFER_LIST | addr | 0x006ECB27 | ✔ 0x00703E53 |
| C_IG_CIPHER_INNO_HASH | addr | 0x00A4A838 | ✔ 0x00A9669E |
| Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_CTOR | addr | 0x00403166 | ✔ 0x00403166 |
| Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_DTOR | addr | 0x0040318B | ✔ 0x0040318B |
| C_QUEST_MAN_CREATE_INSTANCE | addr | 0x009F9AC2 | ✔ 0x00A4397A |
| C_QUEST_MAN_INSTANCE_ADDR | addr | 0x00BED614 | ✔ 0x00C46BE4 |
| C_QUEST_MAN_LOAD_DEMAND | addr | 0x0071D8DF | ✔ 0x0073AE25 |
| C_QUEST_MAN_LOAD_PARTY_QUEST_INFO | addr | 0x00723341 | ✔ 0x007408E6 |
| C_QUEST_MAN_LOAD_EXCLUSIVE | addr | 0x007247A1 | ✔ 0x00741D46 |
| C_QUICKSLOT_KEY_MAPPED_MAN | addr | 0x009FA0CB | ✔ 0x00A43F83 |
| C_RADIO_MANAGER_CREATE_INSTANCE | addr | 0x009FA078 | ✔ 0x00A43F30 |
| C_RADIO_MANAGER_INSTANCE_ADDR | addr | 0x00BF0B00 | ✔ 0x00C45848 (v83 key pointed at heap selector; resolved to real instance global — needs-main-review) |
| C_SECURITY_CLIENT_CREATE_INSTANCE | addr | 0x009F9F42 | ✔ 0x00A43DFA |
| C_SECURITY_CLIENT_INSTANCE_ADDR | addr | 0x00BEC3A8 | ✔ 0x00C4583C |
| C_SECURITY_CLIENT_ON_PACKET | addr | 0x00A4BF03 | ✔ 0x00A97DF1 (needs-main-review; spot-checked) |
| STAGE_INSTANCE_ADDR | addr | 0x00BEDED4 | ✔ 0x00C474EC |
| SET_STAGE | addr | 0x00777347 | ✔ 0x00799CF0 |
| GR_INSTANCE_ADDR | addr | 0x00BF14EC | ✔ 0x00C4AB6C |
| RESET_LSP | sentinel | 0x0044ED47 | ☐ |
| C_STAGE_ON_MOUSE_ENTER | addr | 0x00775FC7 | ✔ 0x0079892C |
| C_STAGE_ON_PACKET | addr | 0x00775FE6 | ✔ 0x0079894B |
| C_SYSTEM_INFO | addr | 0x00A54B90 | ✔ 0x00AA0D10 |
| C_SYSTEM_INFO_INIT | addr | 0x00A54BD0 | ✔ 0x00AA0D50 |
| C_SYSTEM_INFO_GET_GAME_ROOM_CLIENT | addr | 0x00A54FB0 | ✔ 0x00AA1130 |
| C_SYSTEM_INFO_GET_MACHINE_ID | addr | 0x00A54EB0 | ✔ 0x00AA1030 |
| C_UI_TITLE_INSTANCE_ADDR | addr | 0x00BEDA60 | ✔ 0x00C47064 |
| G_DW_TARGET_OS | addr | 0x00BE2EBC | ✔ 0x00C3C204 |
| C_WVS_APP | addr | 0x009F4FDA | ✔ 0x00A3D719 |
| C_WVS_APP_INSTANCE_ADDR | addr | 0x00BE7B38 | ✔ 0x00C40E88 |
| C_WVS_APP_IS_MSG_PROC | addr | 0x009F97BC | ✔ 0x00A435EA |
| C_WVS_APP_INITIALIZE_AUTH | addr | 0x009F7097 | ✔ 0x00A401E7 |
| C_WVS_APP_INITIALIZE_PCOM | addr | 0x009F6D77 | ✔ 0x00A3FDA2 |
| C_WVS_APP_CREATE_MAIN_WINDOW | addr | 0x009F6D97 | ✔ 0x00A3FDD1 |
| C_WVS_APP_CONNECT_LOGIN | addr | 0x009F6F27 | ✔ 0x00A3FFE8 |
| C_WVS_APP_INITIALIZE_RES_MAN | addr | 0x009F7159 | ✔ 0x00A402CB |
| C_WVS_APP_INITIALIZE_GR2D | addr | 0x009F7A3B | ✔ 0x00A4113C |
| C_WVS_APP_INITIALIZE_INPUT | addr | 0x009F7CE1 | ✔ 0x00A4153D |
| C_WVS_APP_INITIALIZE_SOUND | addr | 0x009F82BC | ✔ 0x00A41B18 |
| C_WVS_APP_INITIALIZE_GAME_DATA | addr | 0x009F8B61 | ✔ 0x00A423F1 |
| C_WVS_APP_CREATE_WND_MANAGER | addr | 0x009F7034 | ✔ 0x00A4016D |
| C_WVS_APP_GET_CMD_LINE | addr | 0x009F94A1 | ✔ 0x00A430EA |
| C_WVS_APP_DIR_BACK_SLASH_TO_SLASH | addr | 0x009F95FE | ✔ 0x00A43330 |
| C_WVS_APP_DIR_UP_DIR | addr | 0x009F9644 | ✔ 0x00A433B2 |
| C_WVS_APP_DIR_SLASH_TO_BACK_SLASH | addr | 0x009F9621 | ✔ 0x00A43371 |
| C_WVS_APP_GET_EXCEPTION_FILE_NAME | addr | 0x009F9808 | ✔ 0x00A43663 |
| C_WVS_APP_CALL_UPDATE | addr | 0x009F84D0 | ✔ 0x00A41D42 |
| C_WVS_APP_RUN | addr | 0x009F5C50 | ✔ 0x00A3E7E8 |
| C_WVS_APP_SET_UP | addr | 0x009F5239 | ✔ 0x00A3DDCC |
| C_WVS_CONTEXT_INSTANCE_ADDR | addr | 0x00BE7918 | ☐ |
| C_WVS_CONTEXT_ON_ENTER_GAME | addr | 0x00A03935 | ☐ |
| C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET | offset | 0x10 | ☐ |
| WIN_MAIN | addr | 0x009F19F2 | ✔ 0x00A39FA0 |
| WIN_MAIN_AD_BALLOON_CONDITIONAL | offset | 0xA3D | ✔ 0xA6E |
| WIN_MAIN_PATCHER_OFFSET | offset | 0x212 | ✔ 0x241 |
| C_WND_MAN_S_UPDATE | addr | 0x009E47C3 | ✔ 0x00A2CBEB |
| C_WND_MAN_REDRAW_INVALIDATED_WINDOWS | addr | 0x009E4547 | ✔ 0x00A2C96F |
| Z_ARRAY_REMOVE_ALL | addr | 0x00428CF1 | ✔ 0x004297E5 |
| Z_X_STRING_GET_BUFFER | addr | 0x00414617 | ✔ 0x00429824 (needs-main-review: cstr-assign vs _Cat) |
| Z_X_STRING_TRIM_RIGHT | addr | 0x00474414 | ✔ 0x004772DD |
| Z_X_STRING_TRIM_LEFT | addr | 0x004744C9 | ✔ 0x00477392 |
| C_FIELD_SEND_JOIN_PARTY_MSG | addr | 0x0052FECF | ☐ |
| C_FIELD_SEND_JOIN_PARTY_MSG_OFFSET | offset | 0x65 | ☐ |
| C_FIELD_SEND_CREATE_NEW_PARTY_MSG | addr | 0x52FCE1 | ☐ |
| C_FIELD_SEND_CREATE_NEW_PARTY_MSG_OFFSET | offset | 0xA4 | ☐ |
| C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST | addr | 0x00A12522 | ☐ |
| C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST_OFFSET | offset | 0xE9 | ☐ |
| DR_CHECK | sentinel | 0x00000000 | ☐ |
| CE_TRACER_RUN | sentinel | 0x00000000 | ☐ |
| SEND_HS_LOG | addr | 0x009F191B | ✔ 0x00A39EC9 |
| C_MOB_C_MOB | addr | 0x006621D9 | ✔ 0x00678060 (needs-main-review; spot-checked) |
| C_SECURITY_CLIENT_ON_PACKET_RET_STUB | JMS sentinel | 0x00000000 | ☐ |
| C_SECURITY_CLIENT_ON_PACKET_CHECK | JMS sentinel | 0x00000000 | ☐ |
| C_SECURITY_CLIENT_ON_PACKET_CHECK_OFFSET | JMS sentinel | 0x00000000 | ☐ |
| C_WVS_APP_INITIALIZE_GR2D_WINDOWED_OFFSET | JMS sentinel | 0x00000000 | ☐ |
| WIN_MAIN_LAUNCHER_STUB | JMS sentinel | 0x00000000 | ☐ |

## Suggested resolution order

Per README §"Adding a new version": start with **WinMain → CWvsApp** (they
anchor the whole image and give you call-graph reach into most other subsystems),
then **CClientSocket/ZSocket** and **COutPacket** (highest-value for the
redirect/bypass edits), then login/stage/logo, then the manager singletons, then
utilities, then resolve sentinels last.
