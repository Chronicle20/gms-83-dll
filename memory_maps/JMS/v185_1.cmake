set(VERSION_HEADER 3)

set(PLAYER_LOGGED_IN 0x07)
set(CLIENT_START_ERROR 0x15)

set(GET_SE_PRIVILEGE 0x00000000) # Does not exist in JMS

set(C_ACTION_MAN_CREATE_INSTANCE_ADDR 0x00ADCCCB)
set(C_ACTION_MAN_INSTANCE_ADDR 0x00CD11AC)
set(C_ACTION_MAN_INIT 0x0040705D)
set(C_ACTION_MAN_SWEEP_CACHE 0x004123D3)

set(C_ANIMATION_DISPLAYER_CREATE_INSTANCE 0x00ADCD21)

set(C_CLIENT_SOCKET_INSTANCE_ADDR 0x00CD11F8)
set(C_CLIENT_SOCKET_CREATE_INSTANCE 0x00ADCD78)
set(C_CLIENT_SOCKET_SEND_PACKET 0x004B14F7)
set(C_CLIENT_SOCKET_MANIPULATE_PACKET 0x004B1717)
set(C_CLIENT_SOCKET_PROCESS_PACKET 0x004B17EB)
set(C_CLIENT_SOCKET_CLOSE 0x004B14E5)
set(C_CLIENT_SOCKET_CLEAR_SEND_RECEIVE_CTX 0x004B1CD5)
set(C_CLIENT_SOCKET_ON_CONNECT 0x004B0066)
set(C_CLIENT_SOCKET_CONNECT_LOGIN 0x004AFD61)
set(C_CLIENT_SOCKET_CONNECT_CTX 0x004AFF6C)
set(C_CLIENT_SOCKET_CONNECT_ADR 0x004AFFD1)

set(Z_SOCKET_BASE_CLOSE_SOCKET 0x004AFC92)

set(Z_SOCKET_BUFFER_ALLOC 0x004B1132)

set(C_CONFIG 0x004B8CA5)
set(C_CONFIG_INSTANCE_ADDR 0x00CD5690)
set(C_CONFIG_GET_PARTNER_CODE 0x000000) # TODO
set(C_CONFIG_APPLY_SYS_OPT 0x004BB741)
set(C_CONFIG_CHECK_EXEC_PATH_REG 0x004B98CD)
set(C_CONFIG_SYS_OPT_WINDOWED_MODE 0x00000000) # this does not exist in JMS

set(C_FUNC_KEY_MAPPED_MAN 0x005E78F7)
set(C_FUNC_KEY_MAPPED_MAN_VFTABLE 0x00BE7A80)
set(C_FUNC_KEY_MAPPED_MAN_INSTANCE_ADDR 0x00CD72E8)
set(C_FUNC_KEY_MAPPED_MAN_CREATE_INSTANCE 0x00ADCDBD)

set(DEFAULT_FKM_INSTANCE_ADDR 0x00C8F0D0)
set(DEFAULT_QKM_INSTANCE_ADDR 0x00C8F2A8)

set(C_INPUT_SYSTEM 0x00ADAD17)
set(C_INPUT_SYSTEM_CREATE_INSTANCE 0x00ADC984)
set(C_INPUT_SYSTEM_INSTANCE_ADDR 0x00CD5A80)
set(C_INPUT_SYSTEM_INIT 0x005F98BA)
set(C_INPUT_SYSTEM_UPDATE_DEVICE 0x005F9E49)
set(C_INPUT_SYSTEM_GET_IS_MESSAGE 0x005F9E70)
set(C_INPUT_SYSTEM_GENERATE_AUTO_KEY_DOWN 0x005FAE83)
set(C_INPUT_SYSTEM_SHOW_CURSOR 0x005F9EA2)

set(C_LOGIN_UPDATE 0x0066BC8C)
set(C_LOGIN_SEND_CHECK_PASSWORD_PACKET 0x0066DA6A)

set(C_LOGO 0x006A0BC2)
set(C_LOGO_GET_RTTI 0x006A0BFD)
set(C_LOGO_IS_KIND_OF 0x006A0C03)
set(C_LOGO_UPDATE 0x006A1069)
set(C_LOGO_ON_MOUSE_BUTTON 0x006A1054)
set(C_LOGO_ON_SET_FOCUS 0x006A0BF7)
set(C_LOGO_ON_KEY 0x006A102D)
set(C_LOGO_LOGO_END 0x006A0D41)
set(C_LOGO_FORCED_END 0x006A0D8B)
set(C_LOGO_INIT 0x006A0CB1)
set(C_LOGO_INIT_NX_LOGO 0x006A10DD)

set(C_MACRO_SYS_MAN_CREATE_INSTANCE 0x00ADCE13)

set(C_BATTLE_RECORD_MAN_CREATE_INSTANCE 0x00000000)

set(C_MAPLE_TV_MAN_CREATE_INSTANCE 0x00ADCF23)
set(C_MAPLE_TV_MAN_INSTANCE_ADDR 0x00CD7578)
set(C_MAPLE_TV_MAN_INIT 0x006AAE8F)

set(C_MONSTER_BOOK_MAN_CREATE_INSTANCE 0x00ADCA98)
set(C_MONSTER_BOOK_MAN_INSTANCE_ADDR 0x00CD739C)
set(C_MONSTER_BOOK_MAN_LOAD_BOOK 0x0070530E)

set(C_OUT_PACKET 0x0074B68D)
set(C_OUT_PACKET_ENCODE_1 0x00406A4D)
set(C_OUT_PACKET_ENCODE_2 0x0042A5CC)
set(C_OUT_PACKET_ENCODE_4 0x00406AAA)
set(C_OUT_PACKET_ENCODE_STR 0x0047C19F)
set(C_OUT_PACKET_ENCODE_BUFFER 0x0047C252)

set(C_QUEST_MAN_CREATE_INSTANCE 0x00ADC9DC)
set(C_QUEST_MAN_INSTANCE_ADDR 0x00CD73A0)
set(C_QUEST_MAN_LOAD_DEMAND 0x00784129)
set(C_QUEST_MAN_LOAD_PARTY_QUEST_INFO 0x00789D21)
set(C_QUEST_MAN_LOAD_EXCLUSIVE 0x0078B181)

set(C_QUICKSLOT_KEY_MAPPED_MAN 0x00ADCFDF)

set(C_RADIO_MANAGER_CREATE_INSTANCE 0x00ADCF8C)
set(C_RADIO_MANAGER_INSTANCE_ADDR 0x00CD5C58)

set(C_SECURITY_CLIENT_CREATE_INSTANCE 0x00ADCE67)
set(C_SECURITY_CLIENT_INSTANCE_ADDR 0x00CD5C44)
set(C_SECURITY_CLIENT_ON_PACKET 0x00B3B5AB)

set(STAGE_INSTANCE_ADDR 0x00CD7F04)
set(SET_STAGE 0x007EFFC0)

set(GR_INSTANCE_ADDR 0x00CDB7E0)

set(RESET_LSP 0x00000000) # TODO this does not exist in JMS?

set(C_STAGE_ON_MOUSE_ENTER 0x007EEA10)
set(C_STAGE_ON_PACKET 0x007EEA2F)

set(C_SYSTEM_INFO 0x00B3EB20)
set(C_SYSTEM_INFO_INIT 0x00B3EB60)
set(C_SYSTEM_INFO_GET_GAME_ROOM_CLIENT 0x00B3EF40)
set(C_SYSTEM_INFO_GET_MACHINE_ID 0x00B3EE40)

set(C_UI_TITLE_INSTANCE_ADDR 0x00CD79A0)

set(G_DW_TARGET_OS 0x00000000) # TODO this does not exist in JMS?

set(C_WVS_APP 0x00AD73D7)
set(C_WVS_APP_INSTANCE_ADDR 0x00CD5C40)
set(C_WVS_APP_IS_MSG_PROC 0x00ADBE51)
set(C_WVS_APP_INITIALIZE_AUTH 0x00000000)
set(C_WVS_APP_INITIALIZE_PCOM 0x00AD9498)
set(C_WVS_APP_CREATE_MAIN_WINDOW 0x00AD94C7)
set(C_WVS_APP_CONNECT_LOGIN 0x00AD96D0)
set(C_WVS_APP_INITIALIZE_RES_MAN 0x00AD98A5)
set(C_WVS_APP_INITIALIZE_GR2D 0x00ADA8D7)
set(C_WVS_APP_INITIALIZE_INPUT 0x00ADACA5)
set(C_WVS_APP_INITIALIZE_SOUND 0x00ADADBA)
set(C_WVS_APP_INITIALIZE_GAME_DATA 0x00ADAFA1)
set(C_WVS_APP_CREATE_WND_MANAGER 0x00AD982B)
set(C_WVS_APP_GET_CMD_LINE 0x00ADB951)
set(C_WVS_APP_DIR_BACK_SLASH_TO_SLASH 0x00ADBB97)
set(C_WVS_APP_DIR_UP_DIR 0x00ADBC19)
set(C_WVS_APP_DIR_SLASH_TO_BACK_SLASH 0x00ADBBD8)
set(C_WVS_APP_GET_EXCEPTION_FILE_NAME 0x00ADBF06)
set(C_WVS_APP_CALL_UPDATE 0x00000000) # TODO this is fully virtualized
set(C_WVS_APP_RUN 0x00AD8328)
set(C_WVS_APP_SET_UP 0x00AD7D58)

set(C_WVS_CONTEXT_INSTANCE_ADDR 0x00CD11B4)
set(C_WVS_CONTEXT_ON_ENTER_GAME 0x00AE7C9F) # TODO do we still need these
set(C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET 0x10) # TODO do we still need these

set(WIN_MAIN 0x00AD3033)
set(WIN_MAIN_AD_BALLOON_CONDITIONAL 0xA03)
set(WIN_MAIN_PATCHER_OFFSET 0x00) # This isn't the same between GMS and JMS

set(C_WND_MAN_S_UPDATE 0x00AC5202)
set(C_WND_MAN_REDRAW_INVALIDATED_WINDOWS 0x00AC4F86)

set(Z_ARRAY_REMOVE_ALL 0x0042B28B)

set(Z_X_STRING_GET_BUFFER 0x00419364)
set(Z_X_STRING_TRIM_RIGHT 0x004CC5EE)
set(Z_X_STRING_TRIM_LEFT 0x004CC6A3)

set(C_FIELD_SEND_JOIN_PARTY_MSG 0x0056CCE9)
set(C_FIELD_SEND_JOIN_PARTY_MSG_OFFSET 0x9E)

set(C_FIELD_SEND_CREATE_NEW_PARTY_MSG 0x0056CA8B)
set(C_FIELD_SEND_CREATE_NEW_PARTY_MSG_OFFSET 0xA1)

set(C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST 0x00AA7F49)
set(C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST_OFFSET 0xE9)

set(DR_CHECK 0x004A9617)
set(CE_TRACER_RUN 0x00000000) # does not exist
set(SEND_HS_LOG 0x00000000) # does not exist