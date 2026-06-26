# GMS v72.1 memory map. Seeded from v79_1.cmake (closest labeled anchor, +7 versions);
# every value is relocated and re-verified against the v72 binary per
# docs/tasks/task-009-gms-v72-support/. v72 sits BELOW the new floor (79) — a value
# still equal to v79 means UNVERIFIED unless its tracking-table row in memory-map.md is
# marked ✔ with a signature-catalog entry. There is no labeled IDB below v72 to
# triangulate against, and the base branch is sometimes the already-diverged v79-reduced
# branch — confirm every value by a v72 signature, never by proximity.

set(VERSION_HEADER 8) # confirmed v79 in OnConnect (0x48cb81): `if (v17 != 8)` @0x48ce12 -> CTerminateException 0x22000007 (Task 4 socket path)

set(PLAYER_LOGGED_IN 0x14) # confirmed v79: COutPacket(20) @0x48d01c, logged-in (non-bLogin) OnConnect branch
set(CLIENT_START_ERROR 0x19) # confirmed v79: COutPacket(25) @0x48cfbf, bLogin OnConnect branch (CFileStream report relay)

set(GET_SE_PRIVILEGE 0x0044A48E) # GetSEPrivilege (named); OpenProcessToken+LookupPrivilegeValueA("SeDebugPrivilege")+AdjustTokenPrivileges

set(C_ACTION_MAN_CREATE_INSTANCE_ADDR 0x00946A09) # TSingleton<CActionMan>::CreateInstance; Alloc(672)+ctor; instance global 0xB07804
set(C_ACTION_MAN_INSTANCE_ADDR 0x00B07804) # ActionManInstanceAddr; the mov [g],eax store in CreateInstance
set(C_ACTION_MAN_INIT 0x0040681C) # symbol ?Init@CActionMan@@QAEXXZ
set(C_ACTION_MAN_SWEEP_CACHE 0x0040FEEA) # symbol ?SweepCache@CActionMan@@QAEXXZ

set(C_ANIMATION_DISPLAYER_CREATE_INSTANCE 0x00946A5F) # TSingleton<CAnimationDisplayer>::CreateInstance; Alloc(424)+ctor; instance 0xB0BE9C

set(C_CLIENT_SOCKET_INSTANCE_ADDR 0x00B07844) # g_pClientSocketInstance; SBB-singleton store at top of CClientSocket ctor, read by free SendPacket helper
set(C_CLIENT_SOCKET_CREATE_INSTANCE 0x00946AB6) # TSingleton<CClientSocket>::CreateInstance; Alloc(148=0x94)+ctor (out of v83 0x9F9Exx cluster)
set(C_CLIENT_SOCKET_SEND_PACKET 0x0048DF93) # symbol + MakeBufferList(79)->innoHash->Flush call-graph
set(C_CLIENT_SOCKET_FLUSH 0x0048E01B) # symbol + send-buffer ZList walk via cloned send slot dword_B1015C; last call in SendPacket
set(C_CLIENT_SOCKET_MANIPULATE_PACKET 0x0048E135) # symbol + sole caller of ProcessPacket; recv-stream reassembly + innoHash
set(C_CLIENT_SOCKET_PROCESS_PACKET 0x0048E209) # symbol + Decode2 + sub-0x10 dispatch (OnMigrateCommand/OnAliveReq/CWvsContext::OnPacket)
set(C_CLIENT_SOCKET_CLOSE 0x0048DF81) # symbol + ClearSendReceiveCtx + ZSocketBase::CloseSocket([this+8])
set(C_CLIENT_SOCKET_CLEAR_SEND_RECEIVE_CTX 0x0048E5D7) # symbol + double ZList RemoveAll (recv [this+60]/send [this+80])
set(C_CLIENT_SOCKET_ON_CONNECT 0x0048CB81) # symbol; recv-handshake handler, version byte==8/major==0x4F(79); NO 8-byte client key (see catalog)
set(C_CLIENT_SOCKET_CONNECT_LOGIN 0x0048C773) # symbol; GetCmdLine(0/1)+random server pick -> Connect(CONNECTCONTEXT); sole caller CWvsApp::ConnectLogin
set(C_CLIENT_SOCKET_CONNECT_CTX 0x0048C9CA) # symbol; CONNECTCONTEXT wrapper tail-calling Connect(sockaddr_in)
set(C_CLIENT_SOCKET_CONNECT_ADR 0x0048CA56) # symbol; sole socket(2,1,0) caller; cloned connect slot dword_B1013C(s,addr,16)

set(Z_SOCKET_BASE_CLOSE_SOCKET 0x0048C699) # symbol; -1 sentinel + shutdown(s,2)/closesocket pair

set(Z_SOCKET_BUFFER_ALLOC 0x0048DBEA) # symbol; dual ZAllocEx::Alloc(a1, then 28=0x1C header); called by OnConnect with 0x5B4

set(C_CONFIG 0x0049392C) # symbol ??0CConfig@@QAE@XZ; SBB-singleton store -> g_CConfig_pInstance + 31/100/24 fuse + StringPool(2532) + RegOpenKeyExA(HKLM) + memset(this+592,0x1BD)
set(C_CONFIG_INSTANCE_ADDR 0x00B0BED0) # g_CConfig_pInstance; SBB store in ctor, read by SendCheckPasswordPacket -> GetPartnerCode(g_CConfig)
set(C_CONFIG_GET_PARTNER_CODE 0x005CC09D) # symbol ?GetPartnerCode@CConfig@@; uiWndZ0 key + GetOpt_Int(0,key,0,INT_MIN,INT_MAX)
set(C_CONFIG_APPLY_SYS_OPT 0x004960F9) # symbol ?ApplySysOpt@CConfig@@; qmemcpy(this+100,a2,0x30)+CWvsContext flags @+13868/+13872 + 100*(x+1)/20 volume routing
set(C_CONFIG_CHECK_EXEC_PATH_REG 0x0049440C) # symbol ?CheckExecPathReg@CConfig@@; this[48] reg handle + StringPool(3114/3115) + 92 backslash + GetFileAttributes(==-1||&0x10)
set(C_CONFIG_SYS_OPT_WINDOWED_MODE 0x00B11548) # g_CConfig_SysOpt_WindowedMode; CreateMainWindow style branch (?0x80000000:720896 / ?8:0) + InitializeGr2D device reader (Task 13 windowed-mode offset)

set(C_FUNC_KEY_MAPPED_MAN 0x00569DE5) # symbol ??0CFuncKeyMappedMan@@QAE@XZ (ctor); installs vtable 0xA2EB38, instance 0xB0D2A8
set(C_FUNC_KEY_MAPPED_MAN_VFTABLE 0x00A2EB38) # off_A2EB38; installed at *this in the ctor
set(C_FUNC_KEY_MAPPED_MAN_INSTANCE_ADDR 0x00B0D2A8) # dword_B0D2A8; SBB-singleton store in ctor + read by CreateInstance
set(C_FUNC_KEY_MAPPED_MAN_CREATE_INSTANCE 0x00946AFB) # TSingleton<CFuncKeyMappedMan>::CreateInstance; Alloc(904)+ctor

set(DEFAULT_FKM_INSTANCE_ADDR 0x00ABF99C) # DefaultFKMInstanceAddr (unk_ABF99C); 445-byte FKM default blob, memcpy src in ctor + DefaultFuncKeyMap
set(DEFAULT_QKM_INSTANCE_ADDR 0x00000000) # ABSENT in v79: FKM ctor zeroes the quickslot region (no QKM-default memcpy); v83 32-byte blob has no v79 byte-match. FLAG gate/edit owner (key_mapped_hooks quickslot memcpy must tolerate 0)

set(C_INPUT_SYSTEM 0x00945204) # symbol ??0CInputSystem@@QAE@XZ (ctor); CreateInstance allocs 2512, instance 0xB0C29C
set(C_INPUT_SYSTEM_CREATE_INSTANCE 0x009466CD) # TSingleton<CInputSystem>::CreateInstance; Alloc(2512)+ctor
set(C_INPUT_SYSTEM_INSTANCE_ADDR 0x00B0C29C) # InputSystemInstanceAddr (dword_B0C29C); read by ApplySysOpt + CWvsApp::Run input pump
set(C_INPUT_SYSTEM_INIT 0x005757D4) # symbol ?Init@CInputSystem@@QAEXPAUHWND__@@PAPAX@Z
set(C_INPUT_SYSTEM_UPDATE_DEVICE 0x00575BFE) # UpdateDevice_CInputSystem; if(!a1)UpdateKeyboard else if(a1==1)UpdateMouse; called from Run (msgtype<=2 branch)
set(C_INPUT_SYSTEM_GET_IS_MESSAGE 0x00575C1B) # GetIsMessage_CInputSystem; this[625] gate, copies 3 dwords; Run inner drain loop w/ ISMsgProc
set(C_INPUT_SYSTEM_GENERATE_AUTO_KEY_DOWN 0x00576BE7) # GenerateAutoKeyDown_CInputSystem; *a2=256 + GetSpecialKeyFlag; Run else-branch before CSecurityClient::Update
set(C_INPUT_SYSTEM_SHOW_CURSOR 0x00575C4D) # symbol ?ShowCursor@CInputSystem@@QAEXH@Z

set(C_LOGIN_UPDATE 0x005CA348) # vtable[0] of CLogin primary vtable (0xA2F9EC); body uses [esi+0x15C], CWnd::InvalidateRect — diverges from C_LOGO_UPDATE in v79 (shared 0x5F4C16 in v83)
set(C_LOGIN_SEND_CHECK_PASSWORD_PACKET 0x005CBF50) # IDB symbol ??

set(C_LOGO 0x005FF8C4) # IDB symbol ??0CLogo@@QAE@XZ; Alloc(0x258)+ctor pattern from CLogo::LogoEnd
set(C_LOGO_GET_RTTI 0x0042196A) # IDB symbol ?GetRTTI@CLogo@@UAEPBVCRuntimeClass@@XZ; CLogo vtable slot 53 of CWnd iface vtable
set(C_LOGO_IS_KIND_OF 0x00421970) # IDB symbol ?IsKindOf@CLogo@@UBEHPBVCRuntimeClass@@Z; CLogo vtable slot 54
set(C_LOGO_UPDATE 0x005FFE54) # vtable[0] of CLogo primary vtable (0xA307BC); 1500ms timer body; DIVERGES from C_LOGIN_UPDATE in v79
set(C_LOGO_ON_MOUSE_BUTTON 0x005FFE3F) # IUIMsgHandler vtable (0xA30770) slot 2; body: cmp [esp+arg_0],202h (WM_LBUTTONUP) then call InitNXLogo
set(C_LOGO_ON_SET_FOCUS 0x005FF902) # IUIMsgHandler vtable (0xA30770) slot 1; body: push 1; pop eax; retn 4 (returns 1)
set(C_LOGO_ON_KEY 0x005FFE18) # IUIMsgHandler vtable (0xA30770) slot 0; body: checks wParam==13||27||32 then call InitNXLogo
set(C_LOGO_LOGO_END 0x005FFA4C) # call-graph: Alloc(0x258)+CLogin_ctor+SetStage; ends logo sequence
set(C_LOGO_FORCED_END 0x005FFA2A) # vtable[2] of CLogo primary vtable (A307C4); called by SET_STAGE at 6f1b14 via [vtable+8]; stops BGM
set(C_LOGO_INIT 0x005FF9BC) # vtable[1] of CLogo primary vtable (A307C0); called by SET_STAGE at 6f1c2c via [vtable+4]
set(C_LOGO_INIT_NX_LOGO 0x005FFA96) # StringPool::GetBSTR(0x568) NX-logo resource + init-once guard on [this+0x28]

set(C_MACRO_SYS_MAN_CREATE_INSTANCE 0x00946C88) # CreateInstance_TSingleton_CMacroSysMan; Alloc(80)+ctor 0x6CBBFC; instance 0xB0C118 (matches v83 macro-instance usage by UseFuncKeyMapped+NotifyAvatarModified); called from InitializeGameData tail

set(C_BATTLE_RECORD_MAN_CREATE_INSTANCE 0x00000000) # absent in v79: CBattleRecordMan is a v95+ feature; no CBattleRecordMan symbol or string in v79. Confirmed absent

set(C_MAPLE_TV_MAN_CREATE_INSTANCE 0x00946BEA) # TSingleton<CMapleTVMan>::CreateInstance; Alloc(992)+ctor 0x6072B1; instance 0xB0D458
set(C_MAPLE_TV_MAN_INSTANCE_ADDR 0x00B0D458) # MapleTVManInstanceAddr (dword_B0D458); also drives the scheduled-message path in CWvsContext::Update (v83's radio role)
set(C_MAPLE_TV_MAN_INIT 0x006074C7) # symbol ?Init@CMapleTVMan@@QAEXXZ

set(C_MONSTER_BOOK_MAN_CREATE_INSTANCE 0x009467D6) # TSingleton<CMonsterBookMan>::CreateInstance; Alloc(164)+ctor 0x94681B; instance 0xB0D314
set(C_MONSTER_BOOK_MAN_INSTANCE_ADDR 0x00B0D314) # MonsterBookManInstanceAddr (dword_B0D314)
set(C_MONSTER_BOOK_MAN_LOAD_BOOK 0x00651C1F) # symbol ?LoadBook@CMonsterBookMan@@QAEHXZ

set(C_OUT_PACKET 0x0067AD6B) # symbol ??0COutPacket@@QAE@J@Z; _Alloc(256)+Init structure
set(C_OUT_PACKET_ENCODE_1 0x004062C7) # symbol Encode1; push 1 + mov [eax+ecx],dl + inc; shared _EnsureCapacity
set(C_OUT_PACKET_ENCODE_2 0x0042539C) # symbol Encode2; push 2 + mov [eax+ecx],dx + add 2; shared _EnsureCapacity
set(C_OUT_PACKET_ENCODE_4 0x00406324) # symbol Encode4; push 4 + mov [eax+ecx],edx + add 4; shared _EnsureCapacity
set(C_OUT_PACKET_ENCODE_STR 0x004694DE) # symbol EncodeStr; ZXString len([eax-4])+2 + CIOBufferManipulator::EncodeStr
set(C_OUT_PACKET_ENCODE_BUFFER 0x00466AE9) # symbol EncodeBuffer; _EnsureCapacity(Size)+memcpy+len+=Size; retn 8
set(C_OUT_PACKET_MAKE_BUFFER_LIST 0x0067AEC4) # symbol; sole call in CClientSocket::SendPacket + 1460/0x5B4 chunk const

set(C_IG_CIPHER_INNO_HASH 0x00993442) # symbol ?innoHash@CIGCipher@@; seed -967814158 (0xC6EF3720) + bShuffle loop (sub_99347D); called from SendPacket between MakeBufferList+Flush

set(Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_CTOR 0x00402AB8) # symbol ??0?$ZSynchronizedHelper@VZFatalSection@@@@; acquire-loop off_AC4ECC + Sleep(0) retry (dword_B0FDE4); called from SendPacket (this+124). DRIFT: v83 0x403166
set(Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_DTOR 0x00402ADD) # ctor+0x25; dec [eax+4]; jnz; and [eax],0 (release recursion count). RAII release pair w/ ctor (inlined in SendPacket; jmp from loc_9BD033). DRIFT: v83 0x40318B. Listing aliases under ZAllocEx::Alloc (dump aliasing)

set(C_QUEST_MAN_CREATE_INSTANCE 0x00946725) # TSingleton<CQuestMan>::CreateInstance; Alloc(648)+ctor 0x6A86CD; instance 0xB0D318
set(C_QUEST_MAN_INSTANCE_ADDR 0x00B0D318) # QuestManInstanceAddr (dword_B0D318)
set(C_QUEST_MAN_LOAD_DEMAND 0x006A8CD6) # symbol ?LoadDemand@CQuestMan@@QAEHXZ
set(C_QUEST_MAN_LOAD_PARTY_QUEST_INFO 0x006AE1F4) # symbol ?LoadPartyQuestInfo@CQuestMan@@QAEXXZ
set(C_QUEST_MAN_LOAD_EXCLUSIVE 0x006AF68D) # symbol ?LoadExclusive@CQuestMan@@QAEXXZ

set(C_QUICKSLOT_KEY_MAPPED_MAN 0x00946B51) # TSingleton<CQuickslotKeyMappedMan>::CreateInstance; Alloc(48)+ctor 0x602158; instance 0xB0C114

set(C_RADIO_MANAGER_CREATE_INSTANCE 0x00000000) # ABSENT in v79: no CRadioManager singleton (not in the 0x946xxx cluster); the scheduled-message/radio role is folded into CMapleTVMan in v79. FLAG gate/edit owner (common/CRadioManager.cpp must tolerate 0)
set(C_RADIO_MANAGER_INSTANCE_ADDR 0x00000000) # ABSENT in v79 (see above). v83 seed 0xBF0B00 was ALWAYS WRONG: it is the dword_BF0B00 ZAllocEx allocator-selector (1st Alloc arg), not the instance; the real v83 instance was 0xBEC3B4. v79 has no separate radio instance.

set(C_SECURITY_CLIENT_CREATE_INSTANCE 0x00946BA5) # TSingleton<CSecurityClient>::CreateInstance; Alloc(316)+ctor 0x994493; instance 0xB0C308
set(C_SECURITY_CLIENT_INSTANCE_ADDR 0x00B0C308) # SecurityClientInstanceAddr (dword_B0C308)
set(C_SECURITY_CLIENT_ON_PACKET 0x00994995) # OnPacket_CSecurityClient; Decode1==4 -> OnCheckClientIntegrityRequest; reached from CClientSocket::ProcessPacket case 0x14. needs-main-review (security_hooks boundary)

set(STAGE_INSTANCE_ADDR 0x00B0DADC) # dword_B0DADC; zeroed then written by SET_STAGE (6f1aec); read by CWvsApp::CallUpdate (945501)
set(SET_STAGE 0x006F1AC0) # IDB comment; clears STAGE_INSTANCE_ADDR, calls old_stage->vtable[2]=ForcedEnd, stores new stage, calls new_stage->vtable[1]=Init

set(GR_INSTANCE_ADDR 0x00B10F74) # dword_B10F74; stored by sub_947BB8 in CWvsApp::InitializeGr2D (944cca); read as IWzGr2D* at 944d5a

set(RESET_LSP 0x0044A9B1) # ResetLSP — PRESENT in v79 (stale v83 "does not exist" comment corrected). WinSock2 Protocol_Catalog9 reg read + "wpclsp.dll" check + "netsh winsock reset" via CreateProcessA; sole xref = CWvsApp ctor @0x943066

set(C_STAGE_ON_MOUSE_ENTER 0x0092F3F8) # IDB symbol ?OnMouseEnter@CStage@@UAEXH@Z; in CLogo IUIMsgHandler vtable (A30770) slot 41
set(C_STAGE_ON_PACKET 0x006F079F) # IDB symbol ?OnPacket@CStage@@UAEXJAAVCInPacket@@@Z; dispatched from ProcessPacket(48e275) via [stage+8 vtable][0]

set(C_SYSTEM_INFO 0x0099CDB0) # ctor (renamed ??0CSystemInfo@@QAE@XZ); installs vtable off_A396E4; stack-constructed in CLogin::SendCheckPasswordPacket (sub_99CDB0/sub_99CDE0 ctor/dtor pair)
set(C_SYSTEM_INFO_INIT 0x0099CDF0) # symbol ?Init@CSystemInfo@@; Netbios MAC + SOFTWARE\Microsoft\Windows\CurrentVersion + CxSupportId(16B) + CoCreateGuid fallback
set(C_SYSTEM_INFO_GET_GAME_ROOM_CLIENT 0x0099D1D0) # symbol ?GetGameRoomClient@CSystemInfo@@ (0x11B4 process-table fn); called from SendCheckPasswordPacket
set(C_SYSTEM_INFO_GET_MACHINE_ID 0x0099D0D0) # symbol ?GetMachineId@CSystemInfo@@; returns cached 16-byte id; EncodeBuffer(id,16) in SendCheckPasswordPacket

set(C_UI_TITLE_INSTANCE_ADDR 0x00B0D738) # UITitleInstanceAddr; stored by CLogo-range ctor sub_5F652C (installs vtables A30698/A3064C/A30648); cleared by dtor at loc_5FD04E

set(G_DW_TARGET_OS 0x00B0239C) # g_dwTargetOS; =1996 on OS major<5 or IsWow64Process, in CWvsApp ctor

set(C_WVS_APP 0x00942D3B)
set(C_WVS_APP_INSTANCE_ADDR 0x00B07A68) # g_CWvsApp singleton store in ctor
set(C_WVS_APP_IS_MSG_PROC 0x00946430)
set(C_WVS_APP_INITIALIZE_AUTH 0x00000000) # absent in v79: NMCO/CNMCOClientObject auth subsystem post-dates v79 (new v79 sentinel)
set(C_WVS_APP_INITIALIZE_PCOM 0x0094409B)
set(C_WVS_APP_CREATE_MAIN_WINDOW 0x009440BB)
set(C_WVS_APP_CONNECT_LOGIN 0x0094424B)
set(C_WVS_APP_INITIALIZE_RES_MAN 0x009443BB)
set(C_WVS_APP_INITIALIZE_GR2D 0x00944C91)
set(C_WVS_APP_INITIALIZE_INPUT 0x00944F37)
set(C_WVS_APP_INITIALIZE_SOUND 0x009452A1)
set(C_WVS_APP_INITIALIZE_GAME_DATA 0x00945834)
set(C_WVS_APP_CREATE_WND_MANAGER 0x00944358)
set(C_WVS_APP_GET_CMD_LINE 0x0094611A)
set(C_WVS_APP_DIR_BACK_SLASH_TO_SLASH 0x00946277)
set(C_WVS_APP_DIR_UP_DIR 0x009462BD)
set(C_WVS_APP_DIR_SLASH_TO_BACK_SLASH 0x0094629A)
set(C_WVS_APP_GET_EXCEPTION_FILE_NAME 0x00946481)
set(C_WVS_APP_CALL_UPDATE 0x009454B5)
set(C_WVS_APP_RUN 0x00943611)
set(C_WVS_APP_SET_UP 0x009430F1)

set(C_WVS_CONTEXT_INSTANCE_ADDR 0x00B07848) # g_pWvsContext; CWvsContext singleton (g_pClientSocketInstance+4); this-arg to OnEnterGame in SetStage + read by both party senders + ProcessPacket
set(C_WVS_CONTEXT_ON_ENTER_GAME 0x00950297) # symbol ?OnEnterGame@CWvsContext@@QAEXXZ; sole caller SetStage(6f1ac0) GetCharacterData!=0 branch; runs this+0x34xx member-ctor block
set(C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET 0x0F) # measured v79: first body instr lea ecx,[esi+3424h] @0x9502A6 after EH-prolog+reg-save; delta from base 0x950297 (v83 0x10 = push 1; v79 omits it like v84)

set(WIN_MAIN 0x0093F9B7)
set(WIN_MAIN_AD_BALLOON_CONDITIONAL 0xA3D) # measured v79: jz(74 6F)@0x9403F4 guarding ShowADBalloon(740/300/60); delta from WinMain base 0x93F9B7
set(WIN_MAIN_PATCHER_OFFSET 0x212) # measured v79: call ShowStartUpWndModal(E8 C0 FC FF FF)@0x93FBC9; delta from WinMain base 0x93F9B7

set(C_WND_MAN_S_UPDATE 0x00932EE2)
set(C_WND_MAN_REDRAW_INVALIDATED_WINDOWS 0x00932C66)

set(Z_ARRAY_REMOVE_ALL 0x004260F4) # symbol ?RemoveAll@?$ZArray@E@@; if(*this){Free(*this-4);*this=0}; first call in ZArray<uchar>::_Alloc (sub_48E8CB) used by COutPacket ctor

set(Z_X_STRING_GET_BUFFER 0x00426133) # symbol ?_Cat@?$ZXString@D@@ (in-place assign family); empty->GetBuffer(Size,0)+memcpy+ReleaseBuffer (==assign), non-empty->grow+append. needs-main-review: no dedicated pure-assign in v79 (same as v84); repo only calls on fresh ZXStrings
set(Z_X_STRING_TRIM_RIGHT 0x0046DB7E) # symbol ?TrimRight@?$ZXString@D@@; default " \t\r\n" (asc_ABEDA0) + strchr + inner GetBuffer(0x4147bb)
set(Z_X_STRING_TRIM_LEFT 0x0046DC33) # symbol ?TrimLeft@?$ZXString@D@@; same whitespace literal + strchr + memcpy-shift remainder to front

set(C_FIELD_SEND_JOIN_PARTY_MSG 0x0051B4C9) # symbol ?SendJoinPartyMsg@CField@@QAEXABV?$ZXString@D@@@Z; COutPacket(0x79)+Encode1(4)+EncodeStr(name)+SendPacket; one-arg invitee
set(C_FIELD_SEND_JOIN_PARTY_MSG_OFFSET 0x5E) # measured v79: level-gate jnb(73 2F)@0x51B527 (cmp al,0Ah); delta from base 0x51B4C9 (v83 0x65 = same jnb instr, shorter v79 preamble)

set(C_FIELD_SEND_CREATE_NEW_PARTY_MSG 0x0051B318) # symbol ?SendCreateNewPartyMsg@CField@@QAEXXZ; COutPacket(0x79)+Encode1(1)+SendPacket; nullary
set(C_FIELD_SEND_CREATE_NEW_PARTY_MSG_OFFSET 0x9D) # measured v79: level-gate jnb(73 34)@0x51B3B5 (cmp al,0Ah); delta from base 0x51B318 (v83 0xA4 = same jnb instr, shorter v79 preamble)

set(C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST 0x0095DD85) # symbol ?SendMigrateToITCRequest@CWvsContext@@QAEXXZ; "Guest ID Users" string + COutPacket(0x99)+SendPacket
set(C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST_OFFSET 0xE9) # measured v79: ITC-gate jz(74 26)@0x95DE6E (get_field+0x124>>4&1); delta from base 0x95DD85 (coincides with v83 0xE9)

set(DR_CHECK 0x00000000) # absent in v79: DR/anti-debug subsystem not present (Task 2 R11: SetUp has no DR_init step; no NtGetContextThread import). Confirmed absent
set(DR_INIT 0x00000000) # absent in v79: DR subsystem absent (Task 2 R11: SetUp anti-tamper is only CSecurityClient+meteora+ehsvc+IAT-clone, no DR_init). Confirmed absent
set(CE_TRACER_RUN 0x00000000) # absent in v79: CeTracer (AhnLab eTracer) is a v95+ feature; no CeTracer/eTracer symbol or string in v79. Confirmed absent
set(SEND_HS_LOG 0x0093F8E0)

set(C_MOB_C_MOB 0x00630C2C) # symbol ??0CMob@@QAE@PAVCMobTemplate@@@Z (sole caller CreateMob 0x630BF0, ZAllocEx::Alloc(1304) non-zeroing); CLife base + 3 vtables (off_A30C48/A30C24/A30C20) + m_pTemplate@this+0x188 + _ZtlSecureTear chain + MobStat::SetFrom + StringPool(957)/IWzCanvas tail. needs-main-review. m_bDoomReserved LEFT UNINITIALIZED (ctor's highest member write = this+325/0x514; doom field past it near struct end) -> v79 on doom-fix (<84) needs-fix side

set(C_SECURITY_CLIENT_ON_PACKET_RET_STUB 0x00000000) # JMS only — JMS185 in-place RET stub (positive @0xB3B96B); GMS hooks CSecurityClient::OnPacket via C_SECURITY_CLIENT_ON_PACKET (0x994995). Absent in GMS v79 by design
set(C_SECURITY_CLIENT_ON_PACKET_CHECK 0x00000000) # JMS only — JMS185 integrity-check fn @0xB3B5F7 (confirmed real: CSecurityClient method (this,u16,COutPacket*)); no GMS counterpart. Absent in GMS v79 by design
set(C_SECURITY_CLIENT_ON_PACKET_CHECK_OFFSET 0x00000000) # JMS only — JMS185 CHECK+0x19 jz patch offset; no GMS counterpart. Absent in GMS v79 by design
set(C_WVS_APP_INITIALIZE_GR2D_WINDOWED_OFFSET 0x00000000) # JMS only — JMS185 windowed-mode in-place patch offset (0x94); GMS forces windowed via C_CONFIG_SYS_OPT_WINDOWED_MODE (0xB11548). Absent in GMS v79 by design
set(WIN_MAIN_LAUNCHER_STUB 0x00000000) # JMS only — JMS185 StartUpDlgClass window proc @0x7F3CE0 (confirmed real); GMS disables patcher via WIN_MAIN_PATCHER_OFFSET NOP. Absent in GMS v79 by design

# --- Faithful client exception dispatch (docs/tasks/exception-dispatch-cleanup) ---
# v79 __TI globals confirmed by IDB RTTI symbol + the CWvsApp::Run (0x943611) throw sites.
set(C_TI_DISCONNECT_EXCEPTION 0x00A40868) # __TI3?AVCDisconnectException@@; _CxxThrowException @0x943c95 (0x21000000-0x21000006)
set(C_TI_TERMINATE_EXCEPTION  0x00A3CC38) # __TI3?AVCTerminateException@@; _CxxThrowException @0x943cbe (0x22000000-0x2200000B)
set(C_TI_PATCH_EXCEPTION       0x00A4AAD8) # __TI3?AVCPatchException@@; _CxxThrowException @0x943c6c (==0x20000000)
set(C_TI_ZEXCEPTION            0x00A3D3B8) # __TI1?AVZException@@; default _CxxThrowException @0x943ccf
set(C_PATCH_EXCEPTION_BUILDER  0x0050A81B) # CPatchException_Build (KIND 1 thiscall ctor); *this=0x20000000, *(this+4)=79 (major DRIFT vs v83 83), memset 0x504; called @0x943c48 before qmemcpy 1288 + CPatch throw
set(C_COM_RAISE_ERROR_EX       0x004031B5) # ?_com_issue_error@@YGXJ@Z (1-arg HRESULT raiser); Run's discrete com call is the 2-arg _com_raise_error(hr,0) @0x9a84bc, this 1-arg form is the structural/semantic FAILED-render analog (same as v83/v95)

# v79 CFileStream relay RECOVERABLE: OnConnect (0x48cb81) is CLEAN (not CFG-obfuscated like v83),
# so the CLIENT_START_ERROR report-read helpers resolve as real addresses (decision FR-8a).
set(C_FILE_STREAM_RESOLVED     1)          # recoverable in v79 (clean OnConnect); relay enabled
set(C_FILE_STREAM_OPEN_INLINE  0)          # out-of-line Open exists (CFileStream_Open below); CreateFileA NOT inlined into OnConnect
set(C_FILE_STREAM_OPEN         0x0048D31C) # CFileStream_Open; OnConnect call sub_48D31C(name,3,128,1,0x80000000,0,0) = OPEN_EXISTING/FILE_ATTRIBUTE_NORMAL/share=1/GENERIC_READ; CreateFileA via cloned slot dword_B0FE44
set(C_FILE_STREAM_GET_LENGTH   0x0048D4A5) # CFileStream_GetLength; thiscall wrapper dispatching vtable[+60]
set(C_FILE_STREAM_READ         0x0048D5D0) # CFileStream_Read(this,dst,len); memcpy/ReadFile(cloned slot dword_B0FE48)
set(C_FILE_STREAM_CLOSE        0x0048D2BE) # CFileStream_Close; CloseHandle(cloned slot dword_B0FD80) + handle=-1
set(C_FILE_STREAM_VFTABLE      0x00A2CA2C) # CFileStream_vftable (off_A2CA2C); installed at v31[0] in OnConnect report-read path
