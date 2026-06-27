# GMS v61 — Memory Map Resolution Reference

Companion to `prd.md` §5/§7. This is the working reference for relocating the 159
keys of `include/memory_map.h.in` into `memory_maps/GMS/v61_1.cmake`. It is seeded
from the established pattern (task-006 v84, task-008 v79, task-009 v72) and is filled
in during execution.

## Anchor IDBs (confirm each with `get_metadata` before probing)

| Role | Version | IDB binary | MCP port | Label quality |
|---|---|---|---|---|
| Target | **v61.1** | `GMS_v61.1_U_DEVM.exe` | 13344 | sparse — label as we go |
| Closest anchor | v72.1 | `GMS_v72.1_U_DEVM.exe` | 13343 | labeled by task-009 |
| Below-floor x-check | v79.1 | `GMS_v79_1_DEVM.exe` | 13339 | labeled by task-008 |
| Canonical | v83 | `MapleStory_dump.exe` (v83_Me) | 13340 | most complete |
| Upper ref | v84.1 | `GMS_v84.1_U_DEVM.exe` | 13338 | labeled by task-006 |
| Upper ref | v87 | `GMSv87_4GB.exe` | 13341 | labeled |
| Upper ref | v95.0 | `GMS_v95.0_U_DEVM.exe` | 13337 | PDB-derived |
| **NOT an anchor** | v48.1 | `GMS_v48_1_DEVM.exe` | 13345 | **unlabeled — do not triangulate** |

> ⚠️ Two below-floor IDBs (v61 target, v48 distractor) are both loaded. Always
> `get_metadata` to confirm the active instance before any version-specific probe
> ([[feedback_verify_ida_target]]).

## Method (per key)

1. Find the equivalent in **v72** (closest labeled anchor) by its task-009 label.
2. Lift the identifying signature (string xref, import, call-graph anchor, opcode
   pattern, constant, vtable slot) from `signature-catalog.md`.
3. Locate the v61 site by that signature; **re-derive**, do not offset-arithmetic.
4. Confirm canonical name/prototype against **v83**.
5. `rename` (+ `set_type` if prototype known) the v61 site; `idb_save` at checkpoints.
6. Record the v61 address + the heuristic that found it in the catalog.

## Key classes & v61 handling

| Class | How resolved | Notes |
|---|---|---|
| Absolute address | signature relocation from v72 labels | label back into v61 IDB |
| Instruction-relative offset (`*_OFFSET`, `WIN_MAIN_*`, `*_MSG_OFFSET`) | re-derive from v61 codegen | shifts with codegen — never copy |
| Protocol constant (`VERSION_HEADER`, `PLAYER_LOGGED_IN`, `CLIENT_START_ERROR`) | confirm in v61 binary | older protocol; cross-check atlas-ms |
| GMS-absent sentinel (`DR_CHECK`, `DR_INIT`, `CE_TRACER_RUN`, `C_BATTLE_RECORD_MAN_CREATE_INSTANCE`) | confirm absence | `0x00000000` + reason comment |
| JMS-only sentinel | n/a in GMS build | `0x00000000` |
| **v72-present / v61-absent (era)** | investigate each (ad balloon, patcher, MTS, beginner-party) | new v61 sentinel; edit/gate must tolerate `0` |

## Subsystem checklist (mirror of PRD §5 — tick as resolved)

- [ ] CWvsApp lifecycle (`C_WVS_APP_*`) — incl. `C_WVS_APP_SET_UP` init sequence
- [ ] CClientSocket / ZSocket send/flush/process/connect
- [ ] COutPacket encode primitives
- [ ] CLogin / CLogo / CStage / CUITitle login+stage flow
- [ ] CConfig windowed-mode / sys-opt
- [ ] CInputSystem / CFuncKeyMappedMan / quickslot
- [ ] Manager singletons (`*_CREATE_INSTANCE` / `*_INSTANCE_ADDR`)
- [ ] WinMain entry + offsets (ad balloon / patcher — investigate existence)
- [ ] Party / migrate message senders (+ call-site offsets)
- [ ] Misc utils (`ZArray::RemoveAll`, `ZXString` trim/get-buffer, fatal section, `CSystemInfo`, `CIGCipher`)
- [ ] Exception-dispatch keys (`C_TI_*EXCEPTION`, `C_PATCH_EXCEPTION_BUILDER`, `C_COM_RAISE_ERROR_EX`, `C_FILE_STREAM_*`)
- [ ] Sentinels confirmed (GMS-absent / JMS-only / new v61-absent)

## Completeness gate

Re-pin at task start:
```
grep -oE '@[A-Z0-9_]+@' include/memory_map.h.in | sort -u | wc -l   # expect 160; minus BUILD_REGION = 159 keys
```
`cmake/CheckMemoryMapKeys.cmake` fails the build if any key is undefined/empty.

**Confirmed count at Task 1 (2026-06-27): 159. Validator: `OK: all 159 keys defined and non-empty for GMS v61.1`.**

---

## 159-Key Tracking Table

Legend: `☐ todo · ◐ located+labeled · ✔ written+catalogued`

All v61 values are seeded from v72 (UNVERIFIED). Status `☐` until relocated against the v61 binary.

Keys grouped in the order they appear in `include/memory_map.h.in`.

### Protocol constants

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `VERSION_HEADER` | `8` | `8` | ☐ | — |
| `PLAYER_LOGGED_IN` | `0x14` | `0x14` | ☐ | — |
| `CLIENT_START_ERROR` | `0x1A` | `0x1A` | ☐ | — |

### Security privilege

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `GET_SE_PRIVILEGE` | `0x0044989E` | `0x0044989E` | ☐ | — |

### CActionMan

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_ACTION_MAN_CREATE_INSTANCE_ADDR` | `0x008F6172` | `0x008F6172` | ☐ | — |
| `C_ACTION_MAN_INSTANCE_ADDR` | `0x00A9F3F4` | `0x00A9F3F4` | ☐ | — |
| `C_ACTION_MAN_INIT` | `0x0040681C` | `0x0040681C` | ☐ | — |
| `C_ACTION_MAN_SWEEP_CACHE` | `0x0040FE89` | `0x0040FE89` | ☐ | — |

### CAnimationDisplayer

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_ANIMATION_DISPLAYER_CREATE_INSTANCE` | `0x008F61C8` | `0x008F61C8` | ☐ | — |

### CClientSocket

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_CLIENT_SOCKET_INSTANCE_ADDR` | `0x00A9F434` | `0x00A9F434` | ☐ | — |
| `C_CLIENT_SOCKET_CREATE_INSTANCE` | `0x008F621F` | `0x008F621F` | ☐ | — |
| `C_CLIENT_SOCKET_SEND_PACKET` | `0x004866AC` | `0x004866AC` | ☐ | — |
| `C_CLIENT_SOCKET_FLUSH` | `0x00486734` | `0x00486734` | ☐ | — |
| `C_CLIENT_SOCKET_MANIPULATE_PACKET` | `0x0048684E` | `0x0048684E` | ☐ | — |
| `C_CLIENT_SOCKET_PROCESS_PACKET` | `0x00486922` | `0x00486922` | ☐ | — |
| `C_CLIENT_SOCKET_CLOSE` | `0x0048668F` | `0x0048668F` | ☐ | — |
| `C_CLIENT_SOCKET_CLEAR_SEND_RECEIVE_CTX` | `0x00486CF0` | `0x00486CF0` | ☐ | — |
| `C_CLIENT_SOCKET_ON_CONNECT` | `0x0048528F` | `0x0048528F` | ☐ | — |
| `C_CLIENT_SOCKET_CONNECT_LOGIN` | `0x00484EA5` | `0x00484EA5` | ☐ | — |
| `C_CLIENT_SOCKET_CONNECT_CTX` | `0x004850FC` | `0x004850FC` | ☐ | — |
| `C_CLIENT_SOCKET_CONNECT_ADR` | `0x00485188` | `0x00485188` | ☐ | — |

### ZSocketBase / ZSocketBuffer

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `Z_SOCKET_BASE_CLOSE_SOCKET` | `0x00000000` | `0x00000000` | ☐ | — |
| `Z_SOCKET_BUFFER_ALLOC` | `0x004862F8` | `0x004862F8` | ☐ | — |

### CConfig

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_CONFIG` | `0x0048C0D3` | `0x0048C0D3` | ☐ | — |
| `C_CONFIG_INSTANCE_ADDR` | `0x00AA3AC0` | `0x00AA3AC0` | ☐ | — |
| `C_CONFIG_GET_PARTNER_CODE` | `0x005B12BD` | `0x005B12BD` | ☐ | — |
| `C_CONFIG_APPLY_SYS_OPT` | `0x0048E7EC` | `0x0048E7EC` | ☐ | — |
| `C_CONFIG_CHECK_EXEC_PATH_REG` | `0x0048CBAE` | `0x0048CBAE` | ☐ | — |
| `C_CONFIG_SYS_OPT_WINDOWED_MODE` | `0x00AA87AC` | `0x00AA87AC` | ☐ | — |

### CFuncKeyMappedMan / DefaultFKM / DefaultQKM

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_FUNC_KEY_MAPPED_MAN` | `0x005512EC` | `0x005512EC` | ☐ | — |
| `C_FUNC_KEY_MAPPED_MAN_VFTABLE` | `0x009D22D8` | `0x009D22D8` | ☐ | — |
| `C_FUNC_KEY_MAPPED_MAN_INSTANCE_ADDR` | `0x00AA4CB8` | `0x00AA4CB8` | ☐ | — |
| `C_FUNC_KEY_MAPPED_MAN_CREATE_INSTANCE` | `0x008F6264` | `0x008F6264` | ☐ | — |
| `DEFAULT_FKM_INSTANCE_ADDR` | `0x00A5B838` | `0x00A5B838` | ☐ | — |
| `DEFAULT_QKM_INSTANCE_ADDR` | `0x00000000` | `0x00000000` | ☐ | — |

### CInputSystem

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_INPUT_SYSTEM` | `0x008F489E` | `0x008F489E` | ☐ | — |
| `C_INPUT_SYSTEM_CREATE_INSTANCE` | `0x008F5E41` | `0x008F5E41` | ☐ | — |
| `C_INPUT_SYSTEM_INSTANCE_ADDR` | `0x00AA3E84` | `0x00AA3E84` | ☐ | — |
| `C_INPUT_SYSTEM_INIT` | `0x0055CBA9` | `0x0055CBA9` | ☐ | — |
| `C_INPUT_SYSTEM_UPDATE_DEVICE` | `0x0055CFD3` | `0x0055CFD3` | ☐ | — |
| `C_INPUT_SYSTEM_GET_IS_MESSAGE` | `0x0055CFF0` | `0x0055CFF0` | ☐ | — |
| `C_INPUT_SYSTEM_GENERATE_AUTO_KEY_DOWN` | `0x0055DFBC` | `0x0055DFBC` | ☐ | — |
| `C_INPUT_SYSTEM_SHOW_CURSOR` | `0x0055D022` | `0x0055D022` | ☐ | — |

### CLogin

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_LOGIN_UPDATE` | `0x005AFBBE` | `0x005AFBBE` | ☐ | — |
| `C_LOGIN_SEND_CHECK_PASSWORD_PACKET` | `0x005B1170` | `0x005B1170` | ☐ | — |

### CLogo

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_LOGO` | `0x005E11F9` | `0x005E11F9` | ☐ | — |
| `C_LOGO_GET_RTTI` | `0x00421565` | `0x00421565` | ☐ | — |
| `C_LOGO_IS_KIND_OF` | `0x0042156B` | `0x0042156B` | ☐ | — |
| `C_LOGO_UPDATE` | `0x005E1789` | `0x005E1789` | ☐ | — |
| `C_LOGO_ON_MOUSE_BUTTON` | `0x005E1774` | `0x005E1774` | ☐ | — |
| `C_LOGO_ON_SET_FOCUS` | `0x005E1237` | `0x005E1237` | ☐ | — |
| `C_LOGO_ON_KEY` | `0x005E174D` | `0x005E174D` | ☐ | — |
| `C_LOGO_LOGO_END` | `0x005E1381` | `0x005E1381` | ☐ | — |
| `C_LOGO_FORCED_END` | `0x005E135F` | `0x005E135F` | ☐ | — |
| `C_LOGO_INIT` | `0x005E12F1` | `0x005E12F1` | ☐ | — |
| `C_LOGO_INIT_NX_LOGO` | `0x005E13CB` | `0x005E13CB` | ☐ | — |

### CMacroSysMan / CBattleRecordMan (sentinels)

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_MACRO_SYS_MAN_CREATE_INSTANCE` | `0x00000000` | `0x00000000` | ☐ | — |
| `C_BATTLE_RECORD_MAN_CREATE_INSTANCE` | `0x00000000` | `0x00000000` | ☐ | — |

### CMapleTVMan

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_MAPLE_TV_MAN_CREATE_INSTANCE` | `0x008F6353` | `0x008F6353` | ☐ | — |
| `C_MAPLE_TV_MAN_INSTANCE_ADDR` | `0x00AA4E68` | `0x00AA4E68` | ☐ | — |
| `C_MAPLE_TV_MAN_INIT` | `0x005E8B18` | `0x005E8B18` | ☐ | — |

### CMonsterBookMan

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_MONSTER_BOOK_MAN_CREATE_INSTANCE` | `0x008F5F3F` | `0x008F5F3F` | ☐ | — |
| `C_MONSTER_BOOK_MAN_INSTANCE_ADDR` | `0x00AA4D24` | `0x00AA4D24` | ☐ | — |
| `C_MONSTER_BOOK_MAN_LOAD_BOOK` | `0x0062F410` | `0x0062F410` | ☐ | — |

### COutPacket

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_OUT_PACKET` | `0x00656FA1` | `0x00656FA1` | ☐ | — |
| `C_OUT_PACKET_ENCODE_1` | `0x004062C7` | `0x004062C7` | ☐ | — |
| `C_OUT_PACKET_ENCODE_2` | `0x00424F84` | `0x00424F84` | ☐ | — |
| `C_OUT_PACKET_ENCODE_4` | `0x00406324` | `0x00406324` | ☐ | — |
| `C_OUT_PACKET_ENCODE_STR` | `0x00468295` | `0x00468295` | ☐ | — |
| `C_OUT_PACKET_ENCODE_BUFFER` | `0x00465CB2` | `0x00465CB2` | ☐ | — |
| `C_OUT_PACKET_MAKE_BUFFER_LIST` | `0x006570FA` | `0x006570FA` | ☐ | — |

### CIGCipher

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_IG_CIPHER_INNO_HASH` | `0x00940D7E` | `0x00940D7E` | ☐ | — |

### ZSynchronizedHelper / ZFatalSection

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_CTOR` | `0x00402AB8` | `0x00402AB8` | ☐ | — |
| `Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_DTOR` | `0x00402ADD` | `0x00402ADD` | ☐ | — |

### CQuestMan

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_QUEST_MAN_CREATE_INSTANCE` | `0x008F5E99` | `0x008F5E99` | ☐ | — |
| `C_QUEST_MAN_INSTANCE_ADDR` | `0x00AA4D28` | `0x00AA4D28` | ☐ | — |
| `C_QUEST_MAN_LOAD_DEMAND` | `0x00683A9D` | `0x00683A9D` | ☐ | — |
| `C_QUEST_MAN_LOAD_PARTY_QUEST_INFO` | `0x006887DE` | `0x006887DE` | ☐ | — |
| `C_QUEST_MAN_LOAD_EXCLUSIVE` | `0x00689C3E` | `0x00689C3E` | ☐ | — |

### CQuickslotKeyMappedMan

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_QUICKSLOT_KEY_MAPPED_MAN` | `0x008F62BA` | `0x008F62BA` | ☐ | — |

### CRadioManager (sentinels)

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_RADIO_MANAGER_CREATE_INSTANCE` | `0x00000000` | `0x00000000` | ☐ | — |
| `C_RADIO_MANAGER_INSTANCE_ADDR` | `0x00000000` | `0x00000000` | ☐ | — |

### CSecurityClient

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_SECURITY_CLIENT_CREATE_INSTANCE` | `0x008F630E` | `0x008F630E` | ☐ | — |
| `C_SECURITY_CLIENT_INSTANCE_ADDR` | `0x00AA3EE4` | `0x00AA3EE4` | ☐ | — |
| `C_SECURITY_CLIENT_ON_PACKET` | `0x009422D1` | `0x009422D1` | ☐ | — |

### Stage / set_stage

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `STAGE_INSTANCE_ADDR` | `0x00AA54D4` | `0x00AA54D4` | ☐ | — |
| `SET_STAGE` | `0x006C1FBB` | `0x006C1FBB` | ☐ | — |

### GrInstanceAddr

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `GR_INSTANCE_ADDR` | `0x00AA85FC` | `0x00AA85FC` | ☐ | — |

### ResetLSP

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `RESET_LSP` | `0x00449DC1` | `0x00449DC1` | ☐ | — |

### CStage

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_STAGE_ON_MOUSE_ENTER` | `0x008DF289` | `0x008DF289` | ☐ | — |
| `C_STAGE_ON_PACKET` | `0x006C0C61` | `0x006C0C61` | ☐ | — |

### CSystemInfo

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_SYSTEM_INFO` | `0x0094A6C0` | `0x0094A6C0` | ☐ | — |
| `C_SYSTEM_INFO_INIT` | `0x0094A700` | `0x0094A700` | ☐ | — |
| `C_SYSTEM_INFO_GET_GAME_ROOM_CLIENT` | `0x0094AAE0` | `0x0094AAE0` | ☐ | — |
| `C_SYSTEM_INFO_GET_MACHINE_ID` | `0x0094A9E0` | `0x0094A9E0` | ☐ | — |

### CUITitle

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_UI_TITLE_INSTANCE_ADDR` | `0x00AA5114` | `0x00AA5114` | ☐ | — |

### g_dwTargetOS

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `G_DW_TARGET_OS` | `0x00A9A164` | `0x00A9A164` | ☐ | — |

### CWvsApp

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_WVS_APP` | `0x008F26C7` | `0x008F26C7` | ☐ | — |
| `C_WVS_APP_INSTANCE_ADDR` | `0x00A9F658` | `0x00A9F658` | ☐ | — |
| `C_WVS_APP_IS_MSG_PROC` | `0x008F57AB` | `0x008F57AB` | ☐ | — |
| `C_WVS_APP_INITIALIZE_AUTH` | `0x00000000` | `0x00000000` | ☐ | — |
| `C_WVS_APP_INITIALIZE_PCOM` | `0x008F3735` | `0x008F3735` | ☐ | — |
| `C_WVS_APP_CREATE_MAIN_WINDOW` | `0x008F3755` | `0x008F3755` | ☐ | — |
| `C_WVS_APP_CONNECT_LOGIN` | `0x008F38E5` | `0x008F38E5` | ☐ | — |
| `C_WVS_APP_INITIALIZE_RES_MAN` | `0x008F3A55` | `0x008F3A55` | ☐ | — |
| `C_WVS_APP_INITIALIZE_GR2D` | `0x008F432B` | `0x008F432B` | ☐ | — |
| `C_WVS_APP_INITIALIZE_INPUT` | `0x008F45D1` | `0x008F45D1` | ☐ | — |
| `C_WVS_APP_INITIALIZE_SOUND` | `0x008F493B` | `0x008F493B` | ☐ | — |
| `C_WVS_APP_INITIALIZE_GAME_DATA` | `0x008F4E13` | `0x008F4E13` | ☐ | — |
| `C_WVS_APP_CREATE_WND_MANAGER` | `0x008F39F2` | `0x008F39F2` | ☐ | — |
| `C_WVS_APP_GET_CMD_LINE` | `0x008F5495` | `0x008F5495` | ☐ | — |
| `C_WVS_APP_DIR_BACK_SLASH_TO_SLASH` | `0x008F55F2` | `0x008F55F2` | ☐ | — |
| `C_WVS_APP_DIR_UP_DIR` | `0x008F5638` | `0x008F5638` | ☐ | — |
| `C_WVS_APP_DIR_SLASH_TO_BACK_SLASH` | `0x008F5615` | `0x008F5615` | ☐ | — |
| `C_WVS_APP_GET_EXCEPTION_FILE_NAME` | `0x008F57FC` | `0x008F57FC` | ☐ | — |
| `C_WVS_APP_CALL_UPDATE` | `0x008F4991` | `0x008F4991` | ☐ | — |
| `C_WVS_APP_RUN` | `0x008F2F82` | `0x008F2F82` | ☐ | — |
| `C_WVS_APP_SET_UP` | `0x008F2A7D` | `0x008F2A7D` | ☐ | — |

### CWvsContext

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_WVS_CONTEXT_INSTANCE_ADDR` | `0x00A9F438` | `0x00A9F438` | ☐ | — |
| `C_WVS_CONTEXT_ON_ENTER_GAME` | `0x008FF597` | `0x008FF597` | ☐ | — |
| `C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET` | `0x0F` | `0x0F` | ☐ | — |

### WinMain

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `WIN_MAIN` | `0x008EF5AD` | `0x008EF5AD` | ☐ | — |
| `WIN_MAIN_AD_BALLOON_CONDITIONAL` | `0x959` | `0x959` | ☐ | — |
| `WIN_MAIN_PATCHER_OFFSET` | `0x212` | `0x212` | ☐ | — |

### CWndMan

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_WND_MAN_S_UPDATE` | `0x008E2D73` | `0x008E2D73` | ☐ | — |
| `C_WND_MAN_REDRAW_INVALIDATED_WINDOWS` | `0x008E2AF7` | `0x008E2AF7` | ☐ | — |

### ZArray / ZXString

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `Z_ARRAY_REMOVE_ALL` | `0x00425CEC` | `0x00425CEC` | ☐ | — |
| `Z_X_STRING_GET_BUFFER` | `0x00425D2B` | `0x00425D2B` | ☐ | — |
| `Z_X_STRING_TRIM_RIGHT` | `0x0046C9B4` | `0x0046C9B4` | ☐ | — |
| `Z_X_STRING_TRIM_LEFT` | `0x0046CA69` | `0x0046CA69` | ☐ | — |

### CField party / CWvsContext migrate

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_FIELD_SEND_JOIN_PARTY_MSG` | `0x00514462` | `0x00514462` | ☐ | — |
| `C_FIELD_SEND_JOIN_PARTY_MSG_OFFSET` | `0x60` | `0x60` | ☐ | — |
| `C_FIELD_SEND_CREATE_NEW_PARTY_MSG` | `0x005142B0` | `0x005142B0` | ☐ | — |
| `C_FIELD_SEND_CREATE_NEW_PARTY_MSG_OFFSET` | `0x9E` | `0x9E` | ☐ | — |
| `C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST` | `0x0090C9BD` | `0x0090C9BD` | ☐ | — |
| `C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST_OFFSET` | `0xE9` | `0xE9` | ☐ | — |

### DR / CeTracer / HShield sentinels

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `DR_CHECK` | `0x00000000` | `0x00000000` | ☐ | — |
| `DR_INIT` | `0x00000000` | `0x00000000` | ☐ | — |
| `CE_TRACER_RUN` | `0x00000000` | `0x00000000` | ☐ | — |
| `SEND_HS_LOG` | `0x00000000` | `0x00000000` | ☐ | — |

### CMob

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_MOB_C_MOB` | `0x00611CDB` | `0x00611CDB` | ☐ | — |

### JMS-only sentinels

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_SECURITY_CLIENT_ON_PACKET_RET_STUB` | `0x00000000` | `0x00000000` | ☐ | — |
| `C_SECURITY_CLIENT_ON_PACKET_CHECK` | `0x00000000` | `0x00000000` | ☐ | — |
| `C_SECURITY_CLIENT_ON_PACKET_CHECK_OFFSET` | `0x00000000` | `0x00000000` | ☐ | — |
| `C_WVS_APP_INITIALIZE_GR2D_WINDOWED_OFFSET` | `0x00000000` | `0x00000000` | ☐ | — |
| `WIN_MAIN_LAUNCHER_STUB` | `0x00000000` | `0x00000000` | ☐ | — |

### Exception dispatch

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_TI_DISCONNECT_EXCEPTION` | `0x009E34C0` | `0x009E34C0` | ☐ | — |
| `C_TI_TERMINATE_EXCEPTION` | `0x009DF8C8` | `0x009DF8C8` | ☐ | — |
| `C_TI_PATCH_EXCEPTION` | `0x009ECC20` | `0x009ECC20` | ☐ | — |
| `C_TI_ZEXCEPTION` | `0x009E0048` | `0x009E0048` | ☐ | — |
| `C_PATCH_EXCEPTION_BUILDER` | `0x004FEEB7` | `0x004FEEB7` | ☐ | — |
| `C_COM_RAISE_ERROR_EX` | `0x004031B5` | `0x004031B5` | ☐ | — |

### CFileStream relay

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_FILE_STREAM_RESOLVED` | `1` | `1` | ☐ | — |
| `C_FILE_STREAM_OPEN_INLINE` | `0` | `0` | ☐ | — |
| `C_FILE_STREAM_OPEN` | `0x00485A2A` | `0x00485A2A` | ☐ | — |
| `C_FILE_STREAM_GET_LENGTH` | `0x00485BB3` | `0x00485BB3` | ☐ | — |
| `C_FILE_STREAM_READ` | `0x00485CDE` | `0x00485CDE` | ☐ | — |
| `C_FILE_STREAM_CLOSE` | `0x004859CC` | `0x004859CC` | ☐ | — |
| `C_FILE_STREAM_VFTABLE` | `0x009D0914` | `0x009D0914` | ☐ | — |
