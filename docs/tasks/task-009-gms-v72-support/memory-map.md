# GMS v72.1 — Memory Map Porting Plan

This is the working plan for producing `memory_maps/GMS/v72_1.cmake`. It defines the
per-key resolution strategy, the IDB-labeling protocol, and the tracking approach for
every key parsed from `include/memory_map.h.in` (159 keys; re-pin at task start). The
**closest labeled anchor is GMS v79** (task-008-labeled, 7 versions above v72); the
**canonical-name anchor is GMS v83** (most complete symbol set). v87 and the v95 PDB are
secondary references for the upper-gate cross-checks only. There is **no labeled IDB below
v72** — confirm every value by signature, never by proximity.

## Resolution method (per key)

1. Look up the symbol in the **v79 reference first** (closest; task-008 labeled it and
   documented the heuristic in its `signature-catalog.md`). Confirm the canonical name and
   prototype against **v83**. Note the function's distinctive anchors: referenced strings,
   called imports, constants, vtable slot, call-graph neighbors.
2. In the **v72 IDB** (confirm with `get_metadata` first — it is sparse/unverified),
   locate the equivalent via a signature in priority order:
   - **String xref** — most robust across versions; a unique format/literal the function
     references. (Older builds may carry a *different* literal — note it.)
   - **Import/API call anchor** — e.g. the only function calling `socket` + `connect` in a
     given module region.
   - **Call-graph anchor** — child/parent of an already-resolved function.
   - **Constant / opcode** — a magic number, a `push <opcode>` immediate.
   - **Byte/structure signature** (`make_signature`) — last resort; least version-stable
     but precise when codegen matches. Reuse task-008's v79 signatures as starting probes.
3. Record the v72 address; **label it in the v72 IDB** (`rename` with the v83 canonical
   name, `set_type` if prototype known).
4. Add the heuristic to `signature-catalog.md`, noting whether task-008's v79 heuristic
   ported directly or drifted.
5. Write the `set(KEY 0x…)` line to `v72_1.cmake`.

For **offset** keys, disassemble the v72 host function and re-measure the offset to the
target instruction/branch — never copy the v79 or v83 offset.

## Seeding

Seed `v72_1.cmake` from **`v79_1.cmake`**, not `v83_1.cmake`: v79 already carries the
below-floor relocations (the reduced-struct clusters) and the finalized sentinel
dispositions (e.g. `RESET_LSP`, CFileStream relay). Every seeded value is **UNVERIFIED**
until its tracking row is marked ✔ with a signature-catalog entry. A value still equal to
v79 means UNVERIFIED unless a v72 signature confirms it.

## IDB labeling protocol

- Always `get_metadata` before a probe; do not infer the connected IDB from conversation
  ([[feedback-verify-ida-target]]). The v72 IDB is sparse/unverified — confirm its identity
  early and re-confirm after any `select_instance`.
- Apply the v83 canonical name verbatim so cross-version greps line up.
- `idb_save` at checkpoints (e.g. every subsystem group) so labels survive a swap/restart.
- Labeling functions/globals is encouraged. **Do not** apply speculative struct *types*
  into the IDB during the struct-verification pass (decompiler leak — see
  `struct-verification.md`).

## Backward-direction sentinel handling

Going from v79 *down* to v72, the sentinel logic continues the backward direction: a
feature v79 *has* may not exist yet in v72. For each v79 sentinel, confirm v72 is also
absent; and stay alert for v79 *real-address* keys whose backing feature is missing in the
older build (those become **new v72 sentinels**, and the consuming gate/edit must tolerate
`0`).

| Key | v79 disposition (start here) | v72 action |
|---|---|---|
| `C_BATTLE_RECORD_MAN_CREATE_INSTANCE` | sentinel (post-dates v83) | Confirm absent — carry `0x00000000` |
| `DR_CHECK` | sentinel ("does not exist") | Confirm absent — carry `0x00000000` |
| `DR_INIT` | sentinel (DR subsystem absent) | Confirm absent — carry `0x00000000` |
| `CE_TRACER_RUN` | sentinel ("does not exist") | Confirm absent — carry `0x00000000` |
| `RESET_LSP` | (use task-008's v79 verdict, not v83's stale comment) | **Verify in v72** — present or absent? |
| `C_SECURITY_CLIENT_ON_PACKET_*` | JMS sentinel | Carry `0x00000000` (GMS build) |
| `C_WVS_APP_INITIALIZE_GR2D_WINDOWED_OFFSET` | JMS sentinel | Carry `0x00000000` |
| `WIN_MAIN_LAUNCHER_STUB` | JMS sentinel | Carry `0x00000000` |
| `C_FILE_STREAM_*` (relay keys) | (use task-008's v79 verdict) | Decide for v72: recoverable or carry `0`. Document. |
| *(any v79 real key absent in v72)* | n/a | If a v79 feature does not exist in v72, the key becomes a new v72 sentinel — flag for the gate/edit owner |

## Key tracking

Maintain a full tracking table (seeded from `v79_1.cmake`, group order following
`include/memory_map.h.in`) with columns: **Key · v79 value · v72 value · status · signature
ref**. Status legend: ☐ todo · ◐ located, IDB labeled · ✔ written to cmake + catalogued.
Fill during the port; the table is the completeness ledger backing the FR-3 / FR-4 / FR-5
acceptance criteria. (This scaffold intentionally omits the 159-row table; generate it from
`v79_1.cmake` at task start so the v79 seed values are exact.)

## Resolution order (highest value first)

1. **WinMain → CWvsApp** — anchor the image, gain call-graph reach.
2. **CClientSocket / ZSocket → COutPacket** — highest value for `redirect` / `bypass`;
   confirm protocol constants (FR-6) here.
3. **CLogin / CStage / CLogo / CUITitle** — login & stage flow.
4. **Manager singletons** (`*_CREATE_INSTANCE` / `*_INSTANCE_ADDR`).
5. **Utilities** (ZArray/ZXString/fatal-section/CSystemInfo/CIGCipher).
6. **Exception-dispatch keys** (`C_TI_*EXCEPTION`, `C_FILE_STREAM_*`).
7. **Sentinels** last (confirm backward-direction absence).

---

## Tracking table (159 rows — generated from v79 seed at task-1 start)

Status legend: ☐ todo · ◐ located, IDB labeled · ✔ written to cmake + catalogued

| # | Key | v79 value | v72 value | status | signature ref |
|---|-----|-----------|-----------|--------|---------------|
| 1 | `VERSION_HEADER` | `8` | `8` | ☐ | |
| 2 | `PLAYER_LOGGED_IN` | `0x14` | `0x14` | ☐ | |
| 3 | `CLIENT_START_ERROR` | `0x19` | `0x19` | ☐ | |
| 4 | `GET_SE_PRIVILEGE` | `0x0044A48E` | `0x0044A48E` | ☐ | |
| 5 | `C_ACTION_MAN_CREATE_INSTANCE_ADDR` | `0x00946A09` | `0x00946A09` | ☐ | |
| 6 | `C_ACTION_MAN_INSTANCE_ADDR` | `0x00B07804` | `0x00B07804` | ☐ | |
| 7 | `C_ACTION_MAN_INIT` | `0x0040681C` | `0x0040681C` | ☐ | |
| 8 | `C_ACTION_MAN_SWEEP_CACHE` | `0x0040FEEA` | `0x0040FEEA` | ☐ | |
| 9 | `C_ANIMATION_DISPLAYER_CREATE_INSTANCE` | `0x00946A5F` | `0x00946A5F` | ☐ | |
| 10 | `C_CLIENT_SOCKET_INSTANCE_ADDR` | `0x00B07844` | `0x00A9F434` | ✔ | sig-cat: g_pClientSocketInstance |
| 11 | `C_CLIENT_SOCKET_CREATE_INSTANCE` | `0x00946AB6` | `0x008F621F` | ✔ | sig-cat: TSingleton<CClientSocket>::CreateInstance |
| 12 | `C_CLIENT_SOCKET_SEND_PACKET` | `0x0048DF93` | `0x004866AC` | ✔ | sig-cat: CClientSocket::SendPacket (needs-main-review) |
| 13 | `C_CLIENT_SOCKET_FLUSH` | `0x0048E01B` | `0x00486734` | ✔ | sig-cat: CClientSocket::Flush (needs-main-review) |
| 14 | `C_CLIENT_SOCKET_MANIPULATE_PACKET` | `0x0048E135` | `0x0048684E` | ✔ | sig-cat: CClientSocket::ManipulatePacket (needs-main-review) |
| 15 | `C_CLIENT_SOCKET_PROCESS_PACKET` | `0x0048E209` | `0x00486922` | ✔ | sig-cat: CClientSocket::ProcessPacket (needs-main-review) |
| 16 | `C_CLIENT_SOCKET_CLOSE` | `0x0048DF81` | `0x0048668F` | ✔ | sig-cat: CClientSocket::Close |
| 17 | `C_CLIENT_SOCKET_CLEAR_SEND_RECEIVE_CTX` | `0x0048E5D7` | `0x00486CF0` | ✔ | sig-cat: CClientSocket::ClearSendReceiveCtx |
| 18 | `C_CLIENT_SOCKET_ON_CONNECT` | `0x0048CB81` | `0x0048528F` | ✔ | sig-cat: CClientSocket::OnConnect (no client key) |
| 19 | `C_CLIENT_SOCKET_CONNECT_LOGIN` | `0x0048C773` | `0x00484EA5` | ✔ | sig-cat: CClientSocket::ConnectLogin |
| 20 | `C_CLIENT_SOCKET_CONNECT_CTX` | `0x0048C9CA` | `0x004850FC` | ✔ | sig-cat: CClientSocket::Connect(CONNECTCONTEXT) |
| 21 | `C_CLIENT_SOCKET_CONNECT_ADR` | `0x0048CA56` | `0x00485188` | ✔ | sig-cat: CClientSocket::Connect(sockaddr_in) |
| 22 | `Z_SOCKET_BASE_CLOSE_SOCKET` | `0x0048C699` | `0x00000000` | ✔ | sig-cat: ZSocketBase::CloseSocket (INLINED — NEW v72 SENTINEL; FLAG) |
| 23 | `Z_SOCKET_BUFFER_ALLOC` | `0x0048DBEA` | `0x004862F8` | ✔ | sig-cat: ZSocketBuffer::Alloc |
| 24 | `C_CONFIG` | `0x0049392C` | `0x0049392C` | ☐ | |
| 25 | `C_CONFIG_INSTANCE_ADDR` | `0x00B0BED0` | `0x00B0BED0` | ☐ | |
| 26 | `C_CONFIG_GET_PARTNER_CODE` | `0x005CC09D` | `0x005CC09D` | ☐ | |
| 27 | `C_CONFIG_APPLY_SYS_OPT` | `0x004960F9` | `0x004960F9` | ☐ | |
| 28 | `C_CONFIG_CHECK_EXEC_PATH_REG` | `0x0049440C` | `0x0049440C` | ☐ | |
| 29 | `C_CONFIG_SYS_OPT_WINDOWED_MODE` | `0x00B11548` | `0x00B11548` | ☐ | |
| 30 | `C_FUNC_KEY_MAPPED_MAN` | `0x00569DE5` | `0x00569DE5` | ☐ | |
| 31 | `C_FUNC_KEY_MAPPED_MAN_VFTABLE` | `0x00A2EB38` | `0x00A2EB38` | ☐ | |
| 32 | `C_FUNC_KEY_MAPPED_MAN_INSTANCE_ADDR` | `0x00B0D2A8` | `0x00B0D2A8` | ☐ | |
| 33 | `C_FUNC_KEY_MAPPED_MAN_CREATE_INSTANCE` | `0x00946AFB` | `0x00946AFB` | ☐ | |
| 34 | `DEFAULT_FKM_INSTANCE_ADDR` | `0x00ABF99C` | `0x00ABF99C` | ☐ | |
| 35 | `DEFAULT_QKM_INSTANCE_ADDR` | `0x00000000` | `0x00000000` | ☐ | |
| 36 | `C_INPUT_SYSTEM` | `0x00945204` | `0x00945204` | ☐ | |
| 37 | `C_INPUT_SYSTEM_CREATE_INSTANCE` | `0x009466CD` | `0x009466CD` | ☐ | |
| 38 | `C_INPUT_SYSTEM_INSTANCE_ADDR` | `0x00B0C29C` | `0x00B0C29C` | ☐ | |
| 39 | `C_INPUT_SYSTEM_INIT` | `0x005757D4` | `0x005757D4` | ☐ | |
| 40 | `C_INPUT_SYSTEM_UPDATE_DEVICE` | `0x00575BFE` | `0x00575BFE` | ☐ | |
| 41 | `C_INPUT_SYSTEM_GET_IS_MESSAGE` | `0x00575C1B` | `0x00575C1B` | ☐ | |
| 42 | `C_INPUT_SYSTEM_GENERATE_AUTO_KEY_DOWN` | `0x00576BE7` | `0x00576BE7` | ☐ | |
| 43 | `C_INPUT_SYSTEM_SHOW_CURSOR` | `0x00575C4D` | `0x00575C4D` | ☐ | |
| 44 | `C_LOGIN_UPDATE` | `0x005CA348` | `0x005CA348` | ☐ | |
| 45 | `C_LOGIN_SEND_CHECK_PASSWORD_PACKET` | `0x005CBF50` | `0x005CBF50` | ☐ | |
| 46 | `C_LOGO` | `0x005FF8C4` | `0x005FF8C4` | ☐ | |
| 47 | `C_LOGO_GET_RTTI` | `0x0042196A` | `0x0042196A` | ☐ | |
| 48 | `C_LOGO_IS_KIND_OF` | `0x00421970` | `0x00421970` | ☐ | |
| 49 | `C_LOGO_UPDATE` | `0x005FFE54` | `0x005FFE54` | ☐ | |
| 50 | `C_LOGO_ON_MOUSE_BUTTON` | `0x005FFE3F` | `0x005FFE3F` | ☐ | |
| 51 | `C_LOGO_ON_SET_FOCUS` | `0x005FF902` | `0x005FF902` | ☐ | |
| 52 | `C_LOGO_ON_KEY` | `0x005FFE18` | `0x005FFE18` | ☐ | |
| 53 | `C_LOGO_LOGO_END` | `0x005FFA4C` | `0x005FFA4C` | ☐ | |
| 54 | `C_LOGO_FORCED_END` | `0x005FFA2A` | `0x005FFA2A` | ☐ | |
| 55 | `C_LOGO_INIT` | `0x005FF9BC` | `0x005FF9BC` | ☐ | |
| 56 | `C_LOGO_INIT_NX_LOGO` | `0x005FFA96` | `0x005FFA96` | ☐ | |
| 57 | `C_MACRO_SYS_MAN_CREATE_INSTANCE` | `0x00946C88` | `0x00946C88` | ☐ | |
| 58 | `C_BATTLE_RECORD_MAN_CREATE_INSTANCE` | `0x00000000` | `0x00000000` | ☐ | |
| 59 | `C_MAPLE_TV_MAN_CREATE_INSTANCE` | `0x00946BEA` | `0x00946BEA` | ☐ | |
| 60 | `C_MAPLE_TV_MAN_INSTANCE_ADDR` | `0x00B0D458` | `0x00B0D458` | ☐ | |
| 61 | `C_MAPLE_TV_MAN_INIT` | `0x006074C7` | `0x006074C7` | ☐ | |
| 62 | `C_MONSTER_BOOK_MAN_CREATE_INSTANCE` | `0x009467D6` | `0x009467D6` | ☐ | |
| 63 | `C_MONSTER_BOOK_MAN_INSTANCE_ADDR` | `0x00B0D314` | `0x00B0D314` | ☐ | |
| 64 | `C_MONSTER_BOOK_MAN_LOAD_BOOK` | `0x00651C1F` | `0x00651C1F` | ☐ | |
| 65 | `C_OUT_PACKET` | `0x0067AD6B` | `0x0067AD6B` | ☐ | |
| 66 | `C_OUT_PACKET_ENCODE_1` | `0x004062C7` | `0x004062C7` | ☐ | |
| 67 | `C_OUT_PACKET_ENCODE_2` | `0x0042539C` | `0x0042539C` | ☐ | |
| 68 | `C_OUT_PACKET_ENCODE_4` | `0x00406324` | `0x00406324` | ☐ | |
| 69 | `C_OUT_PACKET_ENCODE_STR` | `0x004694DE` | `0x004694DE` | ☐ | |
| 70 | `C_OUT_PACKET_ENCODE_BUFFER` | `0x00466AE9` | `0x00466AE9` | ☐ | |
| 71 | `C_OUT_PACKET_MAKE_BUFFER_LIST` | `0x0067AEC4` | `0x0067AEC4` | ☐ | |
| 72 | `C_IG_CIPHER_INNO_HASH` | `0x00993442` | `0x00993442` | ☐ | |
| 73 | `Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_CTOR` | `0x00402AB8` | `0x00402AB8` | ☐ | |
| 74 | `Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_DTOR` | `0x00402ADD` | `0x00402ADD` | ☐ | |
| 75 | `C_QUEST_MAN_CREATE_INSTANCE` | `0x00946725` | `0x00946725` | ☐ | |
| 76 | `C_QUEST_MAN_INSTANCE_ADDR` | `0x00B0D318` | `0x00B0D318` | ☐ | |
| 77 | `C_QUEST_MAN_LOAD_DEMAND` | `0x006A8CD6` | `0x006A8CD6` | ☐ | |
| 78 | `C_QUEST_MAN_LOAD_PARTY_QUEST_INFO` | `0x006AE1F4` | `0x006AE1F4` | ☐ | |
| 79 | `C_QUEST_MAN_LOAD_EXCLUSIVE` | `0x006AF68D` | `0x006AF68D` | ☐ | |
| 80 | `C_QUICKSLOT_KEY_MAPPED_MAN` | `0x00946B51` | `0x00946B51` | ☐ | |
| 81 | `C_RADIO_MANAGER_CREATE_INSTANCE` | `0x00000000` | `0x00000000` | ☐ | |
| 82 | `C_RADIO_MANAGER_INSTANCE_ADDR` | `0x00000000` | `0x00000000` | ☐ | |
| 83 | `C_SECURITY_CLIENT_CREATE_INSTANCE` | `0x00946BA5` | `0x00946BA5` | ☐ | |
| 84 | `C_SECURITY_CLIENT_INSTANCE_ADDR` | `0x00B0C308` | `0x00B0C308` | ☐ | |
| 85 | `C_SECURITY_CLIENT_ON_PACKET` | `0x00994995` | `0x00994995` | ☐ | |
| 86 | `STAGE_INSTANCE_ADDR` | `0x00B0DADC` | `0x00B0DADC` | ☐ | |
| 87 | `SET_STAGE` | `0x006F1AC0` | `0x006F1AC0` | ☐ | |
| 88 | `GR_INSTANCE_ADDR` | `0x00B10F74` | `0x00B10F74` | ☐ | |
| 89 | `RESET_LSP` | `0x0044A9B1` | `0x0044A9B1` | ☐ | |
| 90 | `C_STAGE_ON_MOUSE_ENTER` | `0x0092F3F8` | `0x0092F3F8` | ☐ | |
| 91 | `C_STAGE_ON_PACKET` | `0x006F079F` | `0x006F079F` | ☐ | |
| 92 | `C_SYSTEM_INFO` | `0x0099CDB0` | `0x0099CDB0` | ☐ | |
| 93 | `C_SYSTEM_INFO_INIT` | `0x0099CDF0` | `0x0099CDF0` | ☐ | |
| 94 | `C_SYSTEM_INFO_GET_GAME_ROOM_CLIENT` | `0x0099D1D0` | `0x0099D1D0` | ☐ | |
| 95 | `C_SYSTEM_INFO_GET_MACHINE_ID` | `0x0099D0D0` | `0x0099D0D0` | ☐ | |
| 96 | `C_UI_TITLE_INSTANCE_ADDR` | `0x00B0D738` | `0x00B0D738` | ☐ | |
| 97 | `G_DW_TARGET_OS` | `0x00B0239C` | `0x00A9A164` | ✔ | sig-cat: g_dwTargetOS |
| 98 | `C_WVS_APP` | `0x00942D3B` | `0x008F26C7` | ✔ | sig-cat: CWvsApp::CWvsApp (needs-main-review) |
| 99 | `C_WVS_APP_INSTANCE_ADDR` | `0x00B07A68` | `0x00A9F658` | ✔ | sig-cat: g_CWvsApp |
| 100 | `C_WVS_APP_IS_MSG_PROC` | `0x00946430` | `0x008F57AB` | ✔ | sig-cat: CWvsApp::ISMsgProc |
| 101 | `C_WVS_APP_INITIALIZE_AUTH` | `0x00000000` | `0x00000000` | ✔ | sig-cat: InitializeAuth (sentinel, absent) |
| 102 | `C_WVS_APP_INITIALIZE_PCOM` | `0x0094409B` | `0x008F3735` | ✔ | sig-cat: CWvsApp::InitializePCOM |
| 103 | `C_WVS_APP_CREATE_MAIN_WINDOW` | `0x009440BB` | `0x008F3755` | ✔ | sig-cat: CWvsApp::CreateMainWindow |
| 104 | `C_WVS_APP_CONNECT_LOGIN` | `0x0094424B` | `0x008F38E5` | ✔ | sig-cat: CWvsApp::ConnectLogin |
| 105 | `C_WVS_APP_INITIALIZE_RES_MAN` | `0x009443BB` | `0x008F3A55` | ✔ | sig-cat: CWvsApp::InitializeResMan |
| 106 | `C_WVS_APP_INITIALIZE_GR2D` | `0x00944C91` | `0x008F432B` | ✔ | sig-cat: CWvsApp::InitializeGr2D |
| 107 | `C_WVS_APP_INITIALIZE_INPUT` | `0x00944F37` | `0x008F45D1` | ✔ | sig-cat: CWvsApp::InitializeInput |
| 108 | `C_WVS_APP_INITIALIZE_SOUND` | `0x009452A1` | `0x008F493B` | ✔ | sig-cat: CWvsApp::InitializeSound |
| 109 | `C_WVS_APP_INITIALIZE_GAME_DATA` | `0x00945834` | `0x008F4E13` | ✔ | sig-cat: CWvsApp::InitializeGameData |
| 110 | `C_WVS_APP_CREATE_WND_MANAGER` | `0x00944358` | `0x008F39F2` | ✔ | sig-cat: CWvsApp::CreateWndManager |
| 111 | `C_WVS_APP_GET_CMD_LINE` | `0x0094611A` | `0x008F5495` | ✔ | sig-cat: CWvsApp::GetCmdLine |
| 112 | `C_WVS_APP_DIR_BACK_SLASH_TO_SLASH` | `0x00946277` | `0x008F55F2` | ✔ | sig-cat: CWvsApp::Dir_* |
| 113 | `C_WVS_APP_DIR_UP_DIR` | `0x009462BD` | `0x008F5638` | ✔ | sig-cat: CWvsApp::Dir_* |
| 114 | `C_WVS_APP_DIR_SLASH_TO_BACK_SLASH` | `0x0094629A` | `0x008F5615` | ✔ | sig-cat: CWvsApp::Dir_* |
| 115 | `C_WVS_APP_GET_EXCEPTION_FILE_NAME` | `0x00946481` | `0x008F57FC` | ✔ | sig-cat: CWvsApp::GetExceptionFileName |
| 116 | `C_WVS_APP_CALL_UPDATE` | `0x009454B5` | `0x008F4991` | ✔ | sig-cat: CWvsApp::CallUpdate |
| 117 | `C_WVS_APP_RUN` | `0x00943611` | `0x008F2F82` | ✔ | sig-cat: CWvsApp::Run (needs-main-review) |
| 118 | `C_WVS_APP_SET_UP` | `0x009430F1` | `0x008F2A7D` | ✔ | sig-cat: CWvsApp::SetUp (needs-main-review) |
| 119 | `C_WVS_CONTEXT_INSTANCE_ADDR` | `0x00B07848` | `0x00B07848` | ☐ | |
| 120 | `C_WVS_CONTEXT_ON_ENTER_GAME` | `0x00950297` | `0x00950297` | ☐ | |
| 121 | `C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET` | `0x0F` | `0x0F` | ☐ | |
| 122 | `WIN_MAIN` | `0x0093F9B7` | `0x008EF5AD` | ✔ | sig-cat: WinMain (needs-main-review) |
| 123 | `WIN_MAIN_AD_BALLOON_CONDITIONAL` | `0xA3D` | `0x959` | ✔ | sig-cat: WinMain offsets (DRIFT) |
| 124 | `WIN_MAIN_PATCHER_OFFSET` | `0x212` | `0x212` | ✔ | sig-cat: WinMain offsets (re-measured) |
| 125 | `C_WND_MAN_S_UPDATE` | `0x00932EE2` | `0x008E2D73` | ✔ | sig-cat: CWndMan::s_Update |
| 126 | `C_WND_MAN_REDRAW_INVALIDATED_WINDOWS` | `0x00932C66` | `0x008E2AF7` | ✔ | sig-cat: CWndMan::RedrawInvalidatedWindows |
| 127 | `Z_ARRAY_REMOVE_ALL` | `0x004260F4` | `0x004260F4` | ☐ | |
| 128 | `Z_X_STRING_GET_BUFFER` | `0x00426133` | `0x00426133` | ☐ | |
| 129 | `Z_X_STRING_TRIM_RIGHT` | `0x0046DB7E` | `0x0046DB7E` | ☐ | |
| 130 | `Z_X_STRING_TRIM_LEFT` | `0x0046DC33` | `0x0046DC33` | ☐ | |
| 131 | `C_FIELD_SEND_JOIN_PARTY_MSG` | `0x0051B4C9` | `0x0051B4C9` | ☐ | |
| 132 | `C_FIELD_SEND_JOIN_PARTY_MSG_OFFSET` | `0x5E` | `0x5E` | ☐ | |
| 133 | `C_FIELD_SEND_CREATE_NEW_PARTY_MSG` | `0x0051B318` | `0x0051B318` | ☐ | |
| 134 | `C_FIELD_SEND_CREATE_NEW_PARTY_MSG_OFFSET` | `0x9D` | `0x9D` | ☐ | |
| 135 | `C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST` | `0x0095DD85` | `0x0095DD85` | ☐ | |
| 136 | `C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST_OFFSET` | `0xE9` | `0xE9` | ☐ | |
| 137 | `DR_CHECK` | `0x00000000` | `0x00000000` | ☐ | |
| 138 | `DR_INIT` | `0x00000000` | `0x00000000` | ☐ | |
| 139 | `CE_TRACER_RUN` | `0x00000000` | `0x00000000` | ☐ | |
| 140 | `SEND_HS_LOG` | `0x0093F8E0` | `0x00000000` | ✔ | sig-cat: SendHSLog (NEW v72 SENTINEL — absent; FLAG) |
| 141 | `C_MOB_C_MOB` | `0x00630C2C` | `0x00630C2C` | ☐ | |
| 142 | `C_SECURITY_CLIENT_ON_PACKET_RET_STUB` | `0x00000000` | `0x00000000` | ☐ | |
| 143 | `C_SECURITY_CLIENT_ON_PACKET_CHECK` | `0x00000000` | `0x00000000` | ☐ | |
| 144 | `C_SECURITY_CLIENT_ON_PACKET_CHECK_OFFSET` | `0x00000000` | `0x00000000` | ☐ | |
| 145 | `C_WVS_APP_INITIALIZE_GR2D_WINDOWED_OFFSET` | `0x00000000` | `0x00000000` | ☐ | |
| 146 | `WIN_MAIN_LAUNCHER_STUB` | `0x00000000` | `0x00000000` | ☐ | |
| 147 | `C_TI_DISCONNECT_EXCEPTION` | `0x00A40868` | `0x00A40868` | ☐ | |
| 148 | `C_TI_TERMINATE_EXCEPTION` | `0x00A3CC38` | `0x00A3CC38` | ☐ | |
| 149 | `C_TI_PATCH_EXCEPTION` | `0x00A4AAD8` | `0x00A4AAD8` | ☐ | |
| 150 | `C_TI_ZEXCEPTION` | `0x00A3D3B8` | `0x00A3D3B8` | ☐ | |
| 151 | `C_PATCH_EXCEPTION_BUILDER` | `0x0050A81B` | `0x0050A81B` | ☐ | |
| 152 | `C_COM_RAISE_ERROR_EX` | `0x004031B5` | `0x004031B5` | ☐ | |
| 153 | `C_FILE_STREAM_RESOLVED` | `1` | `1` | ☐ | |
| 154 | `C_FILE_STREAM_OPEN_INLINE` | `0` | `0` | ☐ | |
| 155 | `C_FILE_STREAM_OPEN` | `0x0048D31C` | `0x0048D31C` | ☐ | |
| 156 | `C_FILE_STREAM_GET_LENGTH` | `0x0048D4A5` | `0x0048D4A5` | ☐ | |
| 157 | `C_FILE_STREAM_READ` | `0x0048D5D0` | `0x0048D5D0` | ☐ | |
| 158 | `C_FILE_STREAM_CLOSE` | `0x0048D2BE` | `0x0048D2BE` | ☐ | |
| 159 | `C_FILE_STREAM_VFTABLE` | `0x00A2CA2C` | `0x00A2CA2C` | ☐ | |
</content>
