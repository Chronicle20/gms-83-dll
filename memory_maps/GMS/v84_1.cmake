# GMS v84.1 memory map. Seeded from v83_1.cmake; every value is relocated and
# re-verified against the v84 binary per docs/tasks/task-006-gms-v84-support/.
# A value still equal to v83 means UNVERIFIED unless its tracking-table row in
# memory-map.md is marked ✔ with a signature-catalog entry.

# Protocol constants read from the v84 CClientSocket::OnConnect send path (0x499DCD),
# not copied from v83 — all three coincide with v83.
set(VERSION_HEADER 8)        # cmp byte ptr [ebp+namelen+3], 8 @ 0x49A08A (version-header guard; else CTerminateException 0x22000007)
set(PLAYER_LOGGED_IN 0x14)   # push 14h @ 0x49A2F9 -> COutPacket ctor in the logged-in (non-bLogin) send branch
set(CLIENT_START_ERROR 0x19) # push 19h @ 0x49A2A0 -> COutPacket ctor in the bLogin send branch

set(GET_SE_PRIVILEGE 0x0044FEF9)

set(C_ACTION_MAN_CREATE_INSTANCE_ADDR 0x00A43C5E)
set(C_ACTION_MAN_INSTANCE_ADDR 0x00C40C24)
set(C_ACTION_MAN_INIT 0x00406B93)
set(C_ACTION_MAN_SWEEP_CACHE 0x00411CA9)

set(C_ANIMATION_DISPLAYER_CREATE_INSTANCE 0x00A43CB4)

set(C_CLIENT_SOCKET_INSTANCE_ADDR 0x00C40C64)
set(C_CLIENT_SOCKET_CREATE_INSTANCE 0x00A43D0B)
set(C_CLIENT_SOCKET_SEND_PACKET 0x0049B28C)
set(C_CLIENT_SOCKET_FLUSH 0x0049B314)
set(C_CLIENT_SOCKET_MANIPULATE_PACKET 0x0049B42E)
set(C_CLIENT_SOCKET_PROCESS_PACKET 0x0049B502)
set(C_CLIENT_SOCKET_CLOSE 0x0049B27A)
set(C_CLIENT_SOCKET_CLEAR_SEND_RECEIVE_CTX 0x0049B903)
set(C_CLIENT_SOCKET_ON_CONNECT 0x00499DCD)
set(C_CLIENT_SOCKET_CONNECT_LOGIN 0x00499824)
set(C_CLIENT_SOCKET_CONNECT_CTX 0x00499B9F)
set(C_CLIENT_SOCKET_CONNECT_ADR 0x00499C2B)

set(Z_SOCKET_BASE_CLOSE_SOCKET 0x0049974A)

set(Z_SOCKET_BUFFER_ALLOC 0x0049AEE3)

set(C_CONFIG 0x004A127C)
set(C_CONFIG_INSTANCE_ADDR 0x00C452EC)
set(C_CONFIG_GET_PARTNER_CODE 0x0060BC34)
set(C_CONFIG_APPLY_SYS_OPT 0x004A3A9C)
set(C_CONFIG_CHECK_EXEC_PATH_REG 0x004A1D5C)
set(C_CONFIG_SYS_OPT_WINDOWED_MODE 0x00C4B150) # sys-opt windowed-mode flag; read by CreateMainWindow + InitializeGr2D (0x80000000 vs 720896 branch)

set(C_FUNC_KEY_MAPPED_MAN 0x0059DD00)
set(C_FUNC_KEY_MAPPED_MAN_VFTABLE 0x00B46B08)
set(C_FUNC_KEY_MAPPED_MAN_INSTANCE_ADDR 0x00C46B70)
set(C_FUNC_KEY_MAPPED_MAN_CREATE_INSTANCE 0x00A43D50)

set(DEFAULT_FKM_INSTANCE_ADDR 0x00C31C7C)
set(DEFAULT_QKM_INSTANCE_ADDR 0x00C31E3C)

set(C_INPUT_SYSTEM 0x00A41A7B)
set(C_INPUT_SYSTEM_CREATE_INSTANCE 0x00A43922)
set(C_INPUT_SYSTEM_INSTANCE_ADDR 0x00C456B4)
set(C_INPUT_SYSTEM_INIT 0x005AA112)
set(C_INPUT_SYSTEM_UPDATE_DEVICE 0x005AA53C)
set(C_INPUT_SYSTEM_GET_IS_MESSAGE 0x005AA559)
set(C_INPUT_SYSTEM_GENERATE_AUTO_KEY_DOWN 0x005AB525)
set(C_INPUT_SYSTEM_SHOW_CURSOR 0x005AA58B)

set(C_LOGIN_UPDATE 0x00609A9F)
set(C_LOGIN_SEND_CHECK_PASSWORD_PACKET 0x0060B88B)

set(C_LOGO 0x0064417C)
set(C_LOGO_GET_RTTI 0x006441C0)
set(C_LOGO_IS_KIND_OF 0x006441C6)
set(C_LOGO_UPDATE 0x00609A9F) # deliberate: points at CLogin::Update (== C_LOGIN_UPDATE), NOT the real CLogo::Update @ 0x00644750 — v83 convention; see signature-catalog.md §CLogin::Update
set(C_LOGO_ON_MOUSE_BUTTON 0x0064473B)
set(C_LOGO_ON_SET_FOCUS 0x006441BA)
set(C_LOGO_ON_KEY 0x00644714)
set(C_LOGO_LOGO_END 0x00644348)
set(C_LOGO_FORCED_END 0x00644392)
set(C_LOGO_INIT 0x00644274)
set(C_LOGO_INIT_NX_LOGO 0x00644830)

set(C_MACRO_SYS_MAN_CREATE_INSTANCE 0x00A43DA6)

# confirmed absent in v84: no CBattleRecordMan class fn/string (positive class confirmed in v95 @ 0x4701A0+); GMS v95+ feature
set(C_BATTLE_RECORD_MAN_CREATE_INSTANCE 0x00000000)

set(C_MAPLE_TV_MAN_CREATE_INSTANCE 0x00A43E3F)
set(C_MAPLE_TV_MAN_INSTANCE_ADDR 0x00C46D64)
set(C_MAPLE_TV_MAN_INIT 0x0064C578)

set(C_MONSTER_BOOK_MAN_CREATE_INSTANCE 0x00A43A2B)
set(C_MONSTER_BOOK_MAN_INSTANCE_ADDR 0x00C46BE0)
set(C_MONSTER_BOOK_MAN_LOAD_BOOK 0x0069B498)

set(C_OUT_PACKET 0x00703CFA)
set(C_OUT_PACKET_ENCODE_1 0x0040661F)
set(C_OUT_PACKET_ENCODE_2 0x00428A68)
set(C_OUT_PACKET_ENCODE_4 0x0040667C)
set(C_OUT_PACKET_ENCODE_STR 0x00471EB0)
set(C_OUT_PACKET_ENCODE_BUFFER 0x0046E5FE)
set(C_OUT_PACKET_MAKE_BUFFER_LIST 0x00703E53)

set(C_IG_CIPHER_INNO_HASH 0x00A9669E)

# byte-identical to v83; ctor = critical-section Enter loop (off_C35A80 + Sleep dword_C499F4);
# dtor (ctor+0x25) = mov eax,[ecx]; dec [eax+4]; jnz; and [eax],0; retn (verified via raw bytes —
# IDA func-db can't represent 0x40318B due to dump address-aliasing). See signature-catalog.md.
set(Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_CTOR 0x00403166)
set(Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_DTOR 0x0040318B)

set(C_QUEST_MAN_CREATE_INSTANCE 0x00A4397A)
set(C_QUEST_MAN_INSTANCE_ADDR 0x00C46BE4)
set(C_QUEST_MAN_LOAD_DEMAND 0x0073AE25)
set(C_QUEST_MAN_LOAD_PARTY_QUEST_INFO 0x007408E6)
set(C_QUEST_MAN_LOAD_EXCLUSIVE 0x00741D46)

set(C_QUICKSLOT_KEY_MAPPED_MAN 0x00A43F83)

set(C_RADIO_MANAGER_CREATE_INSTANCE 0x00A43F30)
# v83 mapped this key to the shared heap-selector global (0x00BF0B00), not the real
# CRadioManager singleton. Resolved here to the genuine instance global written by the
# ctor/CreateInstance pair. See signature-catalog.md §CRadioManager. needs-main-review.
set(C_RADIO_MANAGER_INSTANCE_ADDR 0x00C45848)

set(C_SECURITY_CLIENT_CREATE_INSTANCE 0x00A43DFA)
set(C_SECURITY_CLIENT_INSTANCE_ADDR 0x00C4583C)
# HIGH-VALUE: security integrity-check dispatch (Decode1==4 → integrity reply Encode1(4)).
# needs-main-review. See signature-catalog.md §CSecurityClient::OnPacket.
set(C_SECURITY_CLIENT_ON_PACKET 0x00A97DF1)

set(STAGE_INSTANCE_ADDR 0x00C474EC)
set(SET_STAGE 0x00799CF0)

set(GR_INSTANCE_ADDR 0x00C4AB6C)

# WinSock LSP-reset (wpclsp.dll detect -> netsh winsock reset). v84 has it as a clean,
# de-virtualized function (v83's 0x0044ED47 was an obfuscated stub + stale "does not exist"
# comment). Called from CWvsApp::ctor in the (OS>=6 && !WOW64) branch (call @ 0xA3DD14).
# IDB-labeled ResetLSP. Matches sibling GMS maps (v87 0x451212 / v95 0x45ECD0), entry convention.
set(RESET_LSP 0x004505C5)

set(C_STAGE_ON_MOUSE_ENTER 0x0079892C)
set(C_STAGE_ON_PACKET 0x0079894B)

set(C_SYSTEM_INFO 0x00AA0D10)
set(C_SYSTEM_INFO_INIT 0x00AA0D50)
set(C_SYSTEM_INFO_GET_GAME_ROOM_CLIENT 0x00AA1130)
set(C_SYSTEM_INFO_GET_MACHINE_ID 0x00AA1030)

set(C_UI_TITLE_INSTANCE_ADDR 0x00C47064)

set(G_DW_TARGET_OS 0x00C3C204) # g_dwTargetOS; set to 1996 in CWvsApp::ctor when OS<5 or WOW64

set(C_WVS_APP 0x00A3D719)
set(C_WVS_APP_INSTANCE_ADDR 0x00C40E88)
set(C_WVS_APP_IS_MSG_PROC 0x00A435EA) # clean function entry; v83 cmake was entry+5 (0x009F97BC) — IDB artifact, not a calling-convention offset
set(C_WVS_APP_INITIALIZE_AUTH 0x00A401E7)
set(C_WVS_APP_INITIALIZE_PCOM 0x00A3FDA2)
set(C_WVS_APP_CREATE_MAIN_WINDOW 0x00A3FDD1)
set(C_WVS_APP_CONNECT_LOGIN 0x00A3FFE8)
set(C_WVS_APP_INITIALIZE_RES_MAN 0x00A402CB)
set(C_WVS_APP_INITIALIZE_GR2D 0x00A4113C)
set(C_WVS_APP_INITIALIZE_INPUT 0x00A4153D)
set(C_WVS_APP_INITIALIZE_SOUND 0x00A41B18)
set(C_WVS_APP_INITIALIZE_GAME_DATA 0x00A423F1)
set(C_WVS_APP_CREATE_WND_MANAGER 0x00A4016D)
set(C_WVS_APP_GET_CMD_LINE 0x00A430EA)
set(C_WVS_APP_DIR_BACK_SLASH_TO_SLASH 0x00A43330)
set(C_WVS_APP_DIR_UP_DIR 0x00A433B2)
set(C_WVS_APP_DIR_SLASH_TO_BACK_SLASH 0x00A43371)
set(C_WVS_APP_GET_EXCEPTION_FILE_NAME 0x00A43663)
set(C_WVS_APP_CALL_UPDATE 0x00A41D42)
set(C_WVS_APP_RUN 0x00A3E7E8)
set(C_WVS_APP_SET_UP 0x00A3DDCC)

set(C_WVS_CONTEXT_INSTANCE_ADDR 0x00C40C68)
# CWvsContext::OnEnterGame: enter-game handler invoked by SetStage on the CWvsContext
# singleton in the GetCharacterData!=0 branch (OnEnterGame/OnLeaveGame/OnGameStageChanged trio).
set(C_WVS_CONTEXT_ON_ENTER_GAME 0x00A4E263)
# Re-measured from v84 OnEnterGame disasm: first body instruction after EH-prolog +
# register-save block (lea ecx,[esi+35FCh] @ 0xA4E272). v84 prologue is shorter than v83
# (no large sub esp / push 1), so 0x0F, NOT v83's 0x10 (cf. v87/v95 = 0x11). Not consumed
# by any active edit; carried for parity with sibling maps.
set(C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET 0x0F)

set(WIN_MAIN 0x00A39FA0)
set(WIN_MAIN_AD_BALLOON_CONDITIONAL 0xA6E) # jz @ 0xA3AA0E (guards ShowADBalloon); patch 74->EB
set(WIN_MAIN_PATCHER_OFFSET 0x241) # call ShowStartUpWndModal @ 0xA3A1E1; NOP 5 bytes

set(C_WND_MAN_S_UPDATE 0x00A2CBEB)
set(C_WND_MAN_REDRAW_INVALIDATED_WINDOWS 0x00A2C96F)

set(Z_ARRAY_REMOVE_ALL 0x004297E5) # ZArray<uchar>::RemoveAll: if(*this){ZAllocEx::Free(*this-4); *this=0}; stride-1 (uchar)

# cstr-assign primitive (in-place this=src[0..size)); v84 nearest in-place ABI match is the
# _Cat-family entry which assigns on empty / appends on non-empty. Repo wrapper is always called
# on a fresh/managed ZXString so behaves as assign. needs-main-review. See signature-catalog.md.
set(Z_X_STRING_GET_BUFFER 0x00429824)

# TrimRight/TrimLeft below are NOT under review (no assign/append ambiguity) — anchored on the " \t\r\n" set.
set(Z_X_STRING_TRIM_RIGHT 0x004772DD)
set(Z_X_STRING_TRIM_LEFT 0x00477392)

# CField::SendJoinPartyMsg(name): COutPacket(0x7E)+Encode1(4)+EncodeStr(name)+SendPacket.
# needs-main-review. OFFSET = jnb @ 0x53C101 (the level<10 check, jnb short loc_53C135)
# re-measured: 0x53C101-0x53C061 = 0xA0 (NOT v83's 0x65 — v84 adds a guest-id-check block
# before the level check). Edit overwrites the jnb (0x73) with 0xEB to reach the send path.
set(C_FIELD_SEND_JOIN_PARTY_MSG 0x0053C061)
set(C_FIELD_SEND_JOIN_PARTY_MSG_OFFSET 0xA0) # level-gate jnb @ host+0xA0 (0x53C101, bytes 73 32); edit patches 73->EB. NOT v83 0x65 (added guest-id block)

# CField::SendCreateNewPartyMsg(): COutPacket(0x7E)+Encode1(1)+SendPacket. needs-main-review.
# OFFSET = jnb @ 0x53BEDB (level<10 check, jnb short loc_53BF11) re-measured:
# 0x53BEDB-0x53BE37 = 0xA4 (coincides with v83 0xA4; preamble length identical).
# Edit overwrites the jnb (0x73) with 0xEB to reach the send path.
set(C_FIELD_SEND_CREATE_NEW_PARTY_MSG 0x0053BE37)
set(C_FIELD_SEND_CREATE_NEW_PARTY_MSG_OFFSET 0xA4) # level-gate jnb @ host+0xA4 (0x53BEDB, bytes 73 34); edit patches 73->EB. Coincides w/ v83 (re-measured, not copied)

# CWvsContext::SendMigrateToITCRequest(): COutPacket(0xA0)+SendPacket. needs-main-review.
# OFFSET = jz @ 0xA5CA48 (ITC-restriction and-1 check, jz short loc_A5CA70) re-measured:
# 0xA5CA48-0xA5C95F = 0xE9 (coincides with v83 0xE9; preamble length identical).
# Edit overwrites the jz (0x74) with 0xEB to reach the send path.
set(C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST 0x00A5C95F)
set(C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST_OFFSET 0xE9) # ITC-gate jz @ host+0xE9 (0xA5CA48, bytes 74 26); edit patches 74->EB. Coincides w/ v83 (re-measured, not copied)

# confirmed absent in v84: no DR_check fn / _DR_INFO type (positive ?DR_check@@YAHPAU_DR_INFO@@... confirmed in v87 @ 0x4A1AD3); GMS v87+ feature
set(DR_CHECK 0x00000000)
# DR_init (?DR_init@@YAXXZ): anti-debug setup. Called from CWvsApp::SetUp (@0xA3DE0F) on the CLEAN
# v84 client. Resolves NtGetContextThread into dword_C457FC; the in-field DR check (sub_496208,
# reached via CVecCtrlUser::EndUpdateActive movement path) does call(dword_C45808 ^ dword_C457FC).
# Our SetUp reimpl MUST call this — omitting it leaves the pointer NULL -> call(0) -> AV at 0x0
# ~2-3s into the field. v84 does NOT hook DR_check, so this call is the fix. task-006.
set(DR_INIT 0x00495942)
# confirmed absent in v84: no CeTracer/eTracer fn/string (positive ?Run@CeTracer@@QAEXXZ confirmed in v95 @ 0x9BF370); GMS v95+ feature
set(CE_TRACER_RUN 0x00000000)
set(SEND_HS_LOG 0x00A39EC9)

set(C_MOB_C_MOB 0x00678060) # CMob::CMob ctor (doom-fix hook target, Task-11). m_pTemplate=this+0x188, m_pTemplateByDoom(this+0x18C)=0. needs-main-review

# JMS only: in-place OnPacket ret-stub byte patch (positive JMS185 @ 0xB3B96B, push ebp prologue overwritten with C3).
# GMS v84 bypasses CSecurityClient::OnPacket via the MapleHook on C_SECURITY_CLIENT_ON_PACKET (0xA97DF1) instead.
set(C_SECURITY_CLIENT_ON_PACKET_RET_STUB 0x00000000)
# JMS only: in-place OnPacket integrity-check NOP patch base (positive JMS185 @ 0xB3B5F7; CHECK+0x19 = 0xB3B610 bytes 74 04).
set(C_SECURITY_CLIENT_ON_PACKET_CHECK 0x00000000)
set(C_SECURITY_CLIENT_ON_PACKET_CHECK_OFFSET 0x00000000) # JMS only (positive JMS185 = 0x19); stays 0 for GMS
# JMS only: InitializeGr2D windowed-mode in-place patch offset (positive JMS185 = 0x94).
# GMS v84 forces windowed via C_CONFIG_SYS_OPT_WINDOWED_MODE (0xC4B150) instead; stays 0.
set(C_WVS_APP_INITIALIZE_GR2D_WINDOWED_OFFSET 0x00000000)
# JMS only: launcher/StartUpDlgClass stub forced to ret-1 (positive JMS185 @ 0x7F3CE0).
# GMS v84 no-patcher NOPs WIN_MAIN+WIN_MAIN_PATCHER_OFFSET (0x241) instead; stays 0.
set(WIN_MAIN_LAUNCHER_STUB 0x00000000)

# --- Faithful client exception dispatch (docs/tasks/exception-dispatch-cleanup) ---
set(C_TI_DISCONNECT_EXCEPTION 0x00B9C7B8) # __TI3?AVCDisconnectException@@
set(C_TI_TERMINATE_EXCEPTION  0x00B986C0) # __TI3?AVCTerminateException@@
set(C_TI_PATCH_EXCEPTION       0x00BA72F0) # __TI3?AVCPatchException@@
set(C_TI_ZEXCEPTION            0x00B98E40) # __TI1?AVZException@@
set(C_PATCH_EXCEPTION_BUILDER  0x00527978) # builds CPatchException obj from m_nTargetVersion (Run: v3=sub_527978(this[16]))
set(C_COM_RAISE_ERROR_EX       0x00AABF64) # _com_raise_errorex(hr)  (Run render-fail: sub_AABF64(hr))

set(C_FILE_STREAM_RESOLVED     1)
set(C_FILE_STREAM_OPEN         0x0049A615) # __thiscall CFileStream::Open(name,access,share,…) -> CreateFileA
set(C_FILE_STREAM_GET_LENGTH   0x0049A79E) # __thiscall GetLength() -> size
set(C_FILE_STREAM_READ         0x0049A8C9) # __thiscall Read(dst,len)
set(C_FILE_STREAM_CLOSE        0x0049A5B7) # __thiscall Close()/dtor
set(C_FILE_STREAM_VFTABLE      0x00B437BC) # stream object vtable
