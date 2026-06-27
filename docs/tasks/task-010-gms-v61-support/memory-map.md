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

- [x] CWvsApp lifecycle (`C_WVS_APP_*`) — incl. `C_WVS_APP_SET_UP` init sequence (Task 2; NO DR_init, NO InitializeAuth)
- [x] CClientSocket / ZSocket send/flush/process/connect (Tasks 4–5; CloseSocket inlined sentinel; CFileStream relay RECOVERABLE)
- [x] COutPacket encode primitives (Task 5; all 7 methods; MakeBufferList relocated by call-graph)
- [x] CLogin / CLogo / CStage / CUITitle login+stage flow (Task 6; GetRTTI/IsKindOf inline drift captured)
- [x] CConfig windowed-mode / sys-opt (Task 8; g_CConfig_SysOpt_WindowedMode standalone global)
- [x] CInputSystem / CFuncKeyMappedMan / quickslot (Task 7; InputSystem instance CORRECTED from stale v72 seed; DEFAULT_QKM absent)
- [x] Manager singletons (`*_CREATE_INSTANCE` / `*_INSTANCE_ADDR`) (Task 7; MacroSysMan + RadioManager absent in v61; BattleRecordMan absent)
- [x] WinMain entry + offsets (Task 2; ad balloon + patcher both PRESENT in v61 — offsets re-measured 0x714 / 0x19A)
- [x] Party / migrate message senders (+ call-site offsets) (Task 9; opcode drift 0x7A→0x70 / 0x9A→0x87; WvsContext layout divergence from socket+4)
- [x] Misc utils (`ZArray::RemoveAll`, `ZXString` trim/get-buffer, fatal section, `CSystemInfo`, `CIGCipher`) (Task 8)
- [x] Exception-dispatch keys (`C_TI_*EXCEPTION`, `C_PATCH_EXCEPTION_BUILDER`, `C_COM_RAISE_ERROR_EX`, `C_FILE_STREAM_*`) (Task 10; double throw-site anchor each)
- [x] Sentinels confirmed (GMS-absent / JMS-only / new v61-absent) (Tasks 2/8/10; 13 confirmed-zero keys)

## Completeness gate

Re-pin at task start:
```
grep -oE '@[A-Z0-9_]+@' include/memory_map.h.in | sort -u | wc -l   # expect 160; minus BUILD_REGION = 159 keys
```
`cmake/CheckMemoryMapKeys.cmake` fails the build if any key is undefined/empty.

**Confirmed count at Task 1 (2026-06-27): 159. Validator: `OK: all 159 keys defined and non-empty for GMS v61.1`.**

---

## 159-Key Tracking Table

Legend: `☐ todo · ◐ located+labeled · ✔ written+catalogued` (includes confirmed-zero sentinels — see key notes for absent/JMS-only rationale)

All v61 values are seeded from v72 (UNVERIFIED). Status `☐` until relocated against the v61 binary.

Keys grouped in the order they appear in `include/memory_map.h.in`.

### Protocol constants

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `VERSION_HEADER` | `8` | `8` | ✔ | OnConnect 0x472faf |
| `PLAYER_LOGGED_IN` | `0x14` | `0x14` | ✔ | OnConnect 0x4731bb |
| `CLIENT_START_ERROR` | `0x1A` | `0x19` | ✔ | OnConnect 0x47315d (DRIFT) |

### Security privilege

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `GET_SE_PRIVILEGE` | `0x0044989E` | `0x00000000` | ✔ | Cluster 3b — NEW v61 SENTINEL (absent; FLAGGED) |

### CActionMan

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_ACTION_MAN_CREATE_INSTANCE_ADDR` | `0x008F6172` | `0x00825F46` | ✔ | catalog (Cluster 5) |
| `C_ACTION_MAN_INSTANCE_ADDR` | `0x00A9F3F4` | `0x00970830` | ✔ | catalog (Cluster 5) |
| `C_ACTION_MAN_INIT` | `0x0040681C` | `0x00405EFE` | ✔ | catalog (Cluster 5) |
| `C_ACTION_MAN_SWEEP_CACHE` | `0x0040FE89` | `0x0040F179` | ✔ | catalog (Cluster 5) |

### CAnimationDisplayer

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_ANIMATION_DISPLAYER_CREATE_INSTANCE` | `0x008F61C8` | `0x00825F9C` | ✔ | catalog (Cluster 5) |

### CClientSocket

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_CLIENT_SOCKET_INSTANCE_ADDR` | `0x00A9F434` | `0x00975054` | ✔ | catalog: g_pClientSocketInstance |
| `C_CLIENT_SOCKET_CREATE_INSTANCE` | `0x008F621F` | `0x00825FF3` | ✔ | catalog: TSingleton<CClientSocket>::CreateInstance |
| `C_CLIENT_SOCKET_SEND_PACKET` | `0x004866AC` | `0x00474125` | ✔ | catalog: CClientSocket::SendPacket (HV) |
| `C_CLIENT_SOCKET_FLUSH` | `0x00486734` | `0x0047421C` | ✔ | catalog: CClientSocket::Flush (HV) |
| `C_CLIENT_SOCKET_MANIPULATE_PACKET` | `0x0048684E` | `0x00474336` | ✔ | catalog: CClientSocket::ManipulatePacket (HV) |
| `C_CLIENT_SOCKET_PROCESS_PACKET` | `0x00486922` | `0x0047440A` | ✔ | catalog: CClientSocket::ProcessPacket (HV) |
| `C_CLIENT_SOCKET_CLOSE` | `0x0048668F` | `0x00474108` | ✔ | catalog: CClientSocket::Close |
| `C_CLIENT_SOCKET_CLEAR_SEND_RECEIVE_CTX` | `0x00486CF0` | `0x004748B5` | ✔ | catalog: CClientSocket::ClearSendReceiveCtx |
| `C_CLIENT_SOCKET_ON_CONNECT` | `0x0048528F` | `0x00472D42` | ✔ | catalog: CClientSocket::OnConnect |
| `C_CLIENT_SOCKET_CONNECT_LOGIN` | `0x00484EA5` | `0x00472A0B` | ✔ | catalog: CClientSocket::ConnectLogin |
| `C_CLIENT_SOCKET_CONNECT_CTX` | `0x004850FC` | `0x00472C3E` | ✔ | catalog: CClientSocket::Connect(CONNECTCONTEXT) |
| `C_CLIENT_SOCKET_CONNECT_ADR` | `0x00485188` | `0x00472CA3` | ✔ | catalog: CClientSocket::Connect(sockaddr_in) |

### ZSocketBase / ZSocketBuffer

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `Z_SOCKET_BASE_CLOSE_SOCKET` | `0x00000000` | `0x00000000` | ✔ | catalog: inlined sentinel (confirmed absent) |
| `Z_SOCKET_BUFFER_ALLOC` | `0x004862F8` | `0x00473D71` | ✔ | catalog: ZSocketBuffer::Alloc |

### CConfig

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_CONFIG` | `0x0048C0D3` | `0x0047921D` | ✔ | Cluster 3 |
| `C_CONFIG_INSTANCE_ADDR` | `0x00AA3AC0` | `0x00974ED4` | ✔ | Cluster 3 |
| `C_CONFIG_GET_PARTNER_CODE` | `0x005B12BD` | `0x00564566` | ✔ | Cluster 3 |
| `C_CONFIG_APPLY_SYS_OPT` | `0x0048E7EC` | `0x0047B28E` | ✔ | Cluster 3 |
| `C_CONFIG_CHECK_EXEC_PATH_REG` | `0x0048CBAE` | `0x00479B4D` | ✔ | Cluster 3 |
| `C_CONFIG_SYS_OPT_WINDOWED_MODE` | `0x00AA87AC` | `0x00978E24` | ✔ | Cluster 3 |

### CFuncKeyMappedMan / DefaultFKM / DefaultQKM

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_FUNC_KEY_MAPPED_MAN` | `0x005512EC` | `0x0051AA0E` | ✔ | catalog (Cluster 5) |
| `C_FUNC_KEY_MAPPED_MAN_VFTABLE` | `0x009D22D8` | `0x008E7F58` | ✔ | catalog (Cluster 5) |
| `C_FUNC_KEY_MAPPED_MAN_INSTANCE_ADDR` | `0x00AA4CB8` | `0x00975CA0` | ✔ | catalog (Cluster 5) |
| `C_FUNC_KEY_MAPPED_MAN_CREATE_INSTANCE` | `0x008F6264` | `0x00826038` | ✔ | catalog (Cluster 5) |
| `DEFAULT_FKM_INSTANCE_ADDR` | `0x00A5B838` | `0x00962138` | ✔ | catalog (Cluster 5) |
| `DEFAULT_QKM_INSTANCE_ADDR` | `0x00000000` | `0x00000000` | ✔ | catalog (Cluster 5) |

### CInputSystem

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_INPUT_SYSTEM` | `0x008F489E` | `0x00824817` | ✔ | catalog (Cluster 5) |
| `C_INPUT_SYSTEM_CREATE_INSTANCE` | `0x008F5E41` | `0x00825C20` | ✔ | catalog (Cluster 5) |
| `C_INPUT_SYSTEM_INSTANCE_ADDR` | `0x00AA3E84` | `0x00975050` | ✔ | catalog (Cluster 5) |
| `C_INPUT_SYSTEM_INIT` | `0x0055CBA9` | `0x00524CEB` | ✔ | catalog (Cluster 5) |
| `C_INPUT_SYSTEM_UPDATE_DEVICE` | `0x0055CFD3` | `0x00525113` | ✔ | catalog (Cluster 5) |
| `C_INPUT_SYSTEM_GET_IS_MESSAGE` | `0x0055CFF0` | `0x00525130` | ✔ | catalog (Cluster 5) |
| `C_INPUT_SYSTEM_GENERATE_AUTO_KEY_DOWN` | `0x0055DFBC` | `0x005260FA` | ✔ | catalog (Cluster 5) |
| `C_INPUT_SYSTEM_SHOW_CURSOR` | `0x0055D022` | `0x00525162` | ✔ | catalog (Cluster 5) |

### CLogin

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_LOGIN_UPDATE` | `0x005AFBBE` | `0x00562EDC` | ✔ | catalog: CLogin::Update |
| `C_LOGIN_SEND_CHECK_PASSWORD_PACKET` | `0x005B1170` | `0x00564418` | ✔ | catalog: CLogin::SendCheckPasswordPacket |

### CLogo

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_LOGO` | `0x005E11F9` | `0x005950F4` | ✔ | catalog: CLogo::CLogo |
| `C_LOGO_GET_RTTI` | `0x00421565` | `0x00595138` | ✔ | catalog: CLogo::GetRTTI |
| `C_LOGO_IS_KIND_OF` | `0x0042156B` | `0x0059513E` | ✔ | catalog: CLogo::IsKindOf |
| `C_LOGO_UPDATE` | `0x005E1789` | `0x00595670` | ✔ | catalog: CLogo::Update |
| `C_LOGO_ON_MOUSE_BUTTON` | `0x005E1774` | `0x0059565B` | ✔ | catalog: CLogo::OnMouseButton |
| `C_LOGO_ON_SET_FOCUS` | `0x005E1237` | `0x00595132` | ✔ | catalog: CLogo::OnSetFocus |
| `C_LOGO_ON_KEY` | `0x005E174D` | `0x00595634` | ✔ | catalog: CLogo::OnKey |
| `C_LOGO_LOGO_END` | `0x005E1381` | `0x0059527C` | ✔ | catalog: CLogo::LogoEnd |
| `C_LOGO_FORCED_END` | `0x005E135F` | `0x0059525A` | ✔ | catalog: CLogo::ForcedEnd |
| `C_LOGO_INIT` | `0x005E12F1` | `0x005951EC` | ✔ | catalog: CLogo::Init |
| `C_LOGO_INIT_NX_LOGO` | `0x005E13CB` | `0x005952C6` | ✔ | catalog: CLogo::InitNXLogo |

### CMacroSysMan / CBattleRecordMan (sentinels)

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_MACRO_SYS_MAN_CREATE_INSTANCE` | `0x00000000` | `0x00000000` | ✔ | catalog (Cluster 5) |
| `C_BATTLE_RECORD_MAN_CREATE_INSTANCE` | `0x00000000` | `0x00000000` | ✔ | absent (no string) |

### CMapleTVMan

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_MAPLE_TV_MAN_CREATE_INSTANCE` | `0x008F6353` | `0x00826124` | ✔ | catalog (Cluster 5) |
| `C_MAPLE_TV_MAN_INSTANCE_ADDR` | `0x00AA4E68` | `0x00975DDC` | ✔ | catalog (Cluster 5) |
| `C_MAPLE_TV_MAN_INIT` | `0x005E8B18` | `0x0059BB27` | ✔ | catalog (Cluster 5) |

### CMonsterBookMan

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_MONSTER_BOOK_MAN_CREATE_INSTANCE` | `0x008F5F3F` | `0x00825D13` | ✔ | catalog (Cluster 5) |
| `C_MONSTER_BOOK_MAN_INSTANCE_ADDR` | `0x00AA4D24` | `0x00975CF8` | ✔ | catalog (Cluster 5) |
| `C_MONSTER_BOOK_MAN_LOAD_BOOK` | `0x0062F410` | `0x005DD687` | ✔ | catalog (Cluster 5) |

### COutPacket

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_OUT_PACKET` | `0x00656FA1` | `0x005FFC4F` | ✔ | catalog: COutPacket ctor (HV) |
| `C_OUT_PACKET_ENCODE_1` | `0x004062C7` | `0x00456CF5` | ✔ | catalog: Encode1 (HV) |
| `C_OUT_PACKET_ENCODE_2` | `0x00424F84` | `0x0045C250` | ✔ | catalog: Encode2 (HV) |
| `C_OUT_PACKET_ENCODE_4` | `0x00406324` | `0x00456D52` | ✔ | catalog: Encode4 (HV) |
| `C_OUT_PACKET_ENCODE_STR` | `0x00468295` | `0x00458C91` | ✔ | catalog: EncodeStr (HV) |
| `C_OUT_PACKET_ENCODE_BUFFER` | `0x00465CB2` | `0x00456FBA` | ✔ | catalog: EncodeBuffer (HV) |
| `C_OUT_PACKET_MAKE_BUFFER_LIST` | `0x006570FA` | `0x005FFCE0` | ✔ | catalog: MakeBufferList (HV) |

### CIGCipher

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_IG_CIPHER_INNO_HASH` | `0x00940D7E` | `0x0086274C` | ✔ | Cluster 3b |

### ZSynchronizedHelper / ZFatalSection

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_CTOR` | `0x00402AB8` | `0x00402ABA` | ✔ | Cluster 3b |
| `Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_DTOR` | `0x00402ADD` | `0x00402ADF` | ✔ | Cluster 3b |

### CQuestMan

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_QUEST_MAN_CREATE_INSTANCE` | `0x008F5E99` | `0x00825C78` | ✔ | catalog (Cluster 5) |
| `C_QUEST_MAN_INSTANCE_ADDR` | `0x00AA4D28` | `0x00975DFC` | ✔ | catalog (Cluster 5) |
| `C_QUEST_MAN_LOAD_DEMAND` | `0x00683A9D` | `0x00629040` | ✔ | catalog (Cluster 5) |
| `C_QUEST_MAN_LOAD_PARTY_QUEST_INFO` | `0x006887DE` | `0x00000000` | ✔ | catalog (Cluster 5) |
| `C_QUEST_MAN_LOAD_EXCLUSIVE` | `0x00689C3E` | `0x00000000` | ✔ | catalog (Cluster 5) |

### CQuickslotKeyMappedMan

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_QUICKSLOT_KEY_MAPPED_MAN` | `0x008F62BA` | `0x0082608E` | ✔ | catalog (Cluster 5) |

### CRadioManager (sentinels)

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_RADIO_MANAGER_CREATE_INSTANCE` | `0x00000000` | `0x00000000` | ✔ | catalog (Cluster 5) |
| `C_RADIO_MANAGER_INSTANCE_ADDR` | `0x00000000` | `0x00000000` | ✔ | catalog (Cluster 5) |

### CSecurityClient

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_SECURITY_CLIENT_CREATE_INSTANCE` | `0x008F630E` | `0x008260E2` | ✔ | catalog (Cluster 5) |
| `C_SECURITY_CLIENT_INSTANCE_ADDR` | `0x00AA3EE4` | `0x0097085C` | ✔ | catalog (Cluster 5) |
| `C_SECURITY_CLIENT_ON_PACKET` | `0x009422D1` | `0x008637C4` | ✔ | catalog (Cluster 5) |

### Stage / set_stage

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `STAGE_INSTANCE_ADDR` | `0x00AA54D4` | `0x00976264` | ✔ | catalog: Stage singleton |
| `SET_STAGE` | `0x006C1FBB` | `0x0065B22A` | ✔ | catalog: set_stage |

### GrInstanceAddr

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `GR_INSTANCE_ADDR` | `0x00AA85FC` | `0x00978D34` | ✔ | catalog: GR singleton |

### ResetLSP

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `RESET_LSP` | `0x00449DC1` | `0x00000000` | ✔ | new v61 sentinel (no wpclsp str) |

### CStage

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_STAGE_ON_MOUSE_ENTER` | `0x008DF289` | `0x00659F7A` | ✔ | catalog: CStage::OnMouseEnter |
| `C_STAGE_ON_PACKET` | `0x006C0C61` | `0x00659F99` | ✔ | catalog: CStage::OnPacket |

### CSystemInfo

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_SYSTEM_INFO` | `0x0094A6C0` | `0x008658E0` | ✔ | Cluster 3b |
| `C_SYSTEM_INFO_INIT` | `0x0094A700` | `0x00865920` | ✔ | Cluster 3b |
| `C_SYSTEM_INFO_GET_GAME_ROOM_CLIENT` | `0x0094AAE0` | `0x00865D00` | ✔ | Cluster 3b |
| `C_SYSTEM_INFO_GET_MACHINE_ID` | `0x0094A9E0` | `0x00865C00` | ✔ | Cluster 3b |

### CUITitle

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_UI_TITLE_INSTANCE_ADDR` | `0x00AA5114` | `0x00975FC0` | ✔ | catalog: CUITitle singleton |

### g_dwTargetOS

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `G_DW_TARGET_OS` | `0x00A9A164` | `0x00000000` | ✔ | cat§Cluster1 globals (NEW SENTINEL) |

### CWvsApp

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_WVS_APP` | `0x008F26C7` | `0x00822E44` | ✔ | cat§Cluster1 ctor |
| `C_WVS_APP_INSTANCE_ADDR` | `0x00A9F658` | `0x00970A78` | ✔ | cat§Cluster1 globals |
| `C_WVS_APP_IS_MSG_PROC` | `0x008F57AB` | `0x00825094` | ✔ | cat§Cluster1 ISMsgProc |
| `C_WVS_APP_INITIALIZE_AUTH` | `0x00000000` | `0x00000000` | ✔ | cat§Cluster1 InitializeAuth (absent) |
| `C_WVS_APP_INITIALIZE_PCOM` | `0x008F3735` | `0x008239B0` | ✔ | cat§Cluster1 Initialize* |
| `C_WVS_APP_CREATE_MAIN_WINDOW` | `0x008F3755` | `0x008239D0` | ✔ | cat§Cluster1 Initialize* |
| `C_WVS_APP_CONNECT_LOGIN` | `0x008F38E5` | `0x00823B5B` | ✔ | cat§Cluster1 Initialize* |
| `C_WVS_APP_INITIALIZE_RES_MAN` | `0x008F3A55` | `0x00823CA7` | ✔ | cat§Cluster1 Initialize* |
| `C_WVS_APP_INITIALIZE_GR2D` | `0x008F432B` | `0x00824550` | ✔ | cat§Cluster1 Initialize* |
| `C_WVS_APP_INITIALIZE_INPUT` | `0x008F45D1` | `0x008247C3` | ✔ | cat§Cluster1 Initialize* |
| `C_WVS_APP_INITIALIZE_SOUND` | `0x008F493B` | `0x008248B4` | ✔ | cat§Cluster1 Initialize* |
| `C_WVS_APP_INITIALIZE_GAME_DATA` | `0x008F4E13` | `0x00824AB5` | ✔ | cat§Cluster1 Initialize* |
| `C_WVS_APP_CREATE_WND_MANAGER` | `0x008F39F2` | `0x00823C44` | ✔ | cat§Cluster1 Initialize* |
| `C_WVS_APP_GET_CMD_LINE` | `0x008F5495` | `0x00824D80` | ✔ | cat§Cluster1 helpers |
| `C_WVS_APP_DIR_BACK_SLASH_TO_SLASH` | `0x008F55F2` | `0x00824EDB` | ✔ | cat§Cluster1 helpers |
| `C_WVS_APP_DIR_UP_DIR` | `0x008F5638` | `0x00824F21` | ✔ | cat§Cluster1 helpers |
| `C_WVS_APP_DIR_SLASH_TO_BACK_SLASH` | `0x008F5615` | `0x00824EFE` | ✔ | cat§Cluster1 helpers |
| `C_WVS_APP_GET_EXCEPTION_FILE_NAME` | `0x008F57FC` | `0x008250E5` | ✔ | cat§Cluster1 helpers |
| `C_WVS_APP_CALL_UPDATE` | `0x008F4991` | `0x0082490A` | ✔ | cat§Cluster1 helpers |
| `C_WVS_APP_RUN` | `0x008F2F82` | `0x008233CC` | ✔ | cat§Cluster1 Run |
| `C_WVS_APP_SET_UP` | `0x008F2A7D` | `0x00823175` | ✔ | cat§Cluster1 SetUp |

### CWvsContext

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_WVS_CONTEXT_INSTANCE_ADDR` | `0x00A9F438` | `0x00974EF8` | ✔ | catalog Cluster 6 (DIVERGENCE: not socket+4) |
| `C_WVS_CONTEXT_ON_ENTER_GAME` | `0x008FF597` | `0x0082DD91` | ✔ | catalog Cluster 6 |
| `C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET` | `0x0F` | `0x0F` | ✔ | catalog Cluster 6 (measured) |

### WinMain

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `WIN_MAIN` | `0x008EF5AD` | `0x008205EF` | ✔ | cat§Cluster1 WinMain |
| `WIN_MAIN_AD_BALLOON_CONDITIONAL` | `0x959` | `0x714` | ✔ | cat§Cluster1 WinMain offsets |
| `WIN_MAIN_PATCHER_OFFSET` | `0x212` | `0x19A` | ✔ | cat§Cluster1 WinMain offsets |

### CWndMan

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_WND_MAN_S_UPDATE` | `0x008E2D73` | `0x0081652D` | ✔ | cat§Cluster1 CWndMan |
| `C_WND_MAN_REDRAW_INVALIDATED_WINDOWS` | `0x008E2AF7` | `0x008162B1` | ✔ | cat§Cluster1 CWndMan |

### ZArray / ZXString

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `Z_ARRAY_REMOVE_ALL` | `0x00425CEC` | `0x0045DBB3` | ✔ | Cluster 3b |
| `Z_X_STRING_GET_BUFFER` | `0x00425D2B` | `0x0045DFA4` | ✔ | Cluster 3b |
| `Z_X_STRING_TRIM_RIGHT` | `0x0046C9B4` | `0x0045DD5B` | ✔ | Cluster 3b |
| `Z_X_STRING_TRIM_LEFT` | `0x0046CA69` | `0x0045DE10` | ✔ | Cluster 3b |

### CField party / CWvsContext migrate

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_FIELD_SEND_JOIN_PARTY_MSG` | `0x00514462` | `0x004E8B29` | ✔ | catalog Cluster 6 (opcode 0x7A→0x70) |
| `C_FIELD_SEND_JOIN_PARTY_MSG_OFFSET` | `0x60` | `0x5A` | ✔ | catalog Cluster 6 (measured) |
| `C_FIELD_SEND_CREATE_NEW_PARTY_MSG` | `0x005142B0` | `0x004E898B` | ✔ | catalog Cluster 6 (opcode 0x7A→0x70) |
| `C_FIELD_SEND_CREATE_NEW_PARTY_MSG_OFFSET` | `0x9E` | `0x8B` | ✔ | catalog Cluster 6 (measured) |
| `C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST` | `0x0090C9BD` | `0x00839B94` | ✔ | catalog Cluster 6 (opcode 0x9A→0x87) |
| `C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST_OFFSET` | `0xE9` | `0xE2` | ✔ | catalog Cluster 6 (measured) |

### DR / CeTracer / HShield sentinels

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `DR_CHECK` | `0x00000000` | `0x00000000` | ✔ | absent (no NtGetContextThread) |
| `DR_INIT` | `0x00000000` | `0x00000000` | ✔ | absent (SetUp no DR_init) |
| `CE_TRACER_RUN` | `0x00000000` | `0x00000000` | ✔ | absent (no eTracer str) |
| `SEND_HS_LOG` | `0x00000000` | `0x00000000` | ✔ | cat§Cluster1 SendHSLog (absent) |

### CMob

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_MOB_C_MOB` | `0x00611CDB` | `0x005C2128` | ✔ | Cluster 3b — needs-main-review; doom-tail ABSENT in v61 |

### JMS-only sentinels

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_SECURITY_CLIENT_ON_PACKET_RET_STUB` | `0x00000000` | `0x00000000` | ✔ | JMS only |
| `C_SECURITY_CLIENT_ON_PACKET_CHECK` | `0x00000000` | `0x00000000` | ✔ | JMS only |
| `C_SECURITY_CLIENT_ON_PACKET_CHECK_OFFSET` | `0x00000000` | `0x00000000` | ✔ | JMS only |
| `C_WVS_APP_INITIALIZE_GR2D_WINDOWED_OFFSET` | `0x00000000` | `0x00000000` | ✔ | JMS only |
| `WIN_MAIN_LAUNCHER_STUB` | `0x00000000` | `0x00000000` | ✔ | JMS only |

### Exception dispatch

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_TI_DISCONNECT_EXCEPTION` | `0x009E34C0` | `0x00900678` | ✔ | Run 0x8235af + OnConnect |
| `C_TI_TERMINATE_EXCEPTION` | `0x009DF8C8` | `0x008F4240` | ✔ | Run 0x8235d8 + OnConnect |
| `C_TI_PATCH_EXCEPTION` | `0x009ECC20` | `0x00900668` | ✔ | Run 0x823586 + OnConnect |
| `C_TI_ZEXCEPTION` | `0x009E0048` | `0x008F6B60` | ✔ | Run 0x8235e9 + OnConnect |
| `C_PATCH_EXCEPTION_BUILDER` | `0x004FEEB7` | `0x004DC6E4` | ✔ | CPatchException_Build |
| `C_COM_RAISE_ERROR_EX` | `0x004031B5` | `0x004031B7` | ✔ | _com_issue_error |

### CFileStream relay

| Key | v72 value | v61 value | status | signature ref |
|---|---|---|---|---|
| `C_FILE_STREAM_RESOLVED` | `1` | `1` | ✔ | recoverable v61 |
| `C_FILE_STREAM_OPEN_INLINE` | `0` | `0` | ✔ | out-of-line |
| `C_FILE_STREAM_OPEN` | `0x00485A2A` | `0x004734A3` | ✔ | CFileStream_Open |
| `C_FILE_STREAM_GET_LENGTH` | `0x00485BB3` | `0x0047362C` | ✔ | CFileStream_GetLength |
| `C_FILE_STREAM_READ` | `0x00485CDE` | `0x00473757` | ✔ | CFileStream_Read |
| `C_FILE_STREAM_CLOSE` | `0x004859CC` | `0x00473445` | ✔ | CFileStream_Close |
| `C_FILE_STREAM_VFTABLE` | `0x009D0914` | `0x008E66BC` | ✔ | CFileStream_vftable |
