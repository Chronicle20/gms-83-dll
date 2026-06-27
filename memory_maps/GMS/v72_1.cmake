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

set(GET_SE_PRIVILEGE 0x0044989E) # GetSEPrivilege (named); OpenProcessToken+LookupPrivilegeValueA("SeDebugPrivilege")+AdjustTokenPrivileges. v72 confirmed (import quartet + string); DRIFT v79 0x44A48E

set(C_ACTION_MAN_CREATE_INSTANCE_ADDR 0x008F6172) # symbol ?CreateInstance@?$TSingleton@VCActionMan@@; Alloc(0x2A0=672)+ctor 0x406497; instance global ActionManInstanceAddr 0xA9F3F4; called from CWvsApp::SetUp @0x8f2d1b. RELOCATED (was v79 0x946A09)
set(C_ACTION_MAN_INSTANCE_ADDR 0x00A9F3F4) # ActionManInstanceAddr; singleton dword read at top of CreateInstance + SBB store in ctor. RELOCATED (was v79 0xB07804); v72 singleton globals cluster at 0xA9F3Fx
set(C_ACTION_MAN_INIT 0x0040681C) # symbol ?Init@CActionMan@@QAEXXZ; called from SetUp @0x8f2d22 right after CreateInstance. DIRECT (== v79)
set(C_ACTION_MAN_SWEEP_CACHE 0x0040FE89) # symbol ?SweepCache@CActionMan@@QAEXXZ; sole caller CWvsApp::CallUpdate (0x8f4991). DRIFT (was v79 0x40FEEA)

set(C_ANIMATION_DISPLAYER_CREATE_INSTANCE 0x008F61C8) # symbol ?CreateInstance@?$TSingleton@VCAnimationDisplayer@@; Alloc(0x1A8=424)+ctor 0x431b69; instance 0xAA3A8C; called from SetUp @0x8f2d27. RELOCATED (was v79 0x946A5F)

set(C_CLIENT_SOCKET_INSTANCE_ADDR 0x00A9F434) # g_pClientSocketInstance; SBB-singleton store at top of CClientSocket ctor (0x484c95) + CreateInstance store; read by whole send subsystem (CField/CCashShop senders). DRIFT vs v79 0xB07844 (CWvsContext singleton = +4 = 0xA9F438)
set(C_CLIENT_SOCKET_CREATE_INSTANCE 0x008F621F) # TSingleton<CClientSocket>::CreateInstance (symbol); Alloc(148=0x94)+ctor(0x484c95)+store g_pClientSocketInstance
set(C_CLIENT_SOCKET_SEND_PACKET 0x004866AC) # symbol + MakeBufferList(72)->innoHash->Flush call-graph. DRIFT: send-seq immediate 72 (v72 protocol; was 79 in v79)
set(C_CLIENT_SOCKET_FLUSH 0x00486734) # symbol + send-buffer ZList walk ([this+23]) via cloned send slot dword_AA7874; last call in SendPacket; WSAEWOULDBLOCK(10035) via dword_AA7848->OnError
set(C_CLIENT_SOCKET_MANIPULATE_PACKET 0x0048684E) # symbol + sole caller of ProcessPacket; recv-stream reassembly + innoHash([this+136]). DRIFT: version/seq XOR check != -73 (~72; was -80=~79 in v79)
set(C_CLIENT_SOCKET_PROCESS_PACKET 0x00486922) # symbol + Decode2 + sub-0x10 dispatch (0x10 OnMigrateCommand/0x11 OnAliveReq/0x14 CSecurityClient::OnPacket; 0x1A..0x71 CWvsContext::OnPacket(g_pWvsContext=0xA9F438) else CStage vtable+8); sole caller ManipulatePacket
set(C_CLIENT_SOCKET_CLOSE 0x0048668F) # symbol + ClearSendReceiveCtx + INLINE closesocket([this+8])+set -1 (v72 inlines ZSocketBase::CloseSocket; no shutdown call)
set(C_CLIENT_SOCKET_CLEAR_SEND_RECEIVE_CTX 0x00486CF0) # symbol; clears [this+26]/[this+30] + double ZList reset (recv [this+60]/send [this+80] via sub_48716C); called by ctor/Close/OnConnect
set(C_CLIENT_SOCKET_ON_CONNECT 0x0048528F) # symbol; recv-handshake handler (getpeername import xref + ZSocketBuffer::Alloc(0x5B4) + cloned recv slot dword_AA787C); version byte==8 / major==0x48(72); NO 8-byte client key (see catalog). DRIFT: major 0x48 (was 0x4F in v79)
set(C_CLIENT_SOCKET_CONNECT_LOGIN 0x00484EA5) # symbol; GetCmdLine(0/1)+rand server pick -> Connect(CONNECTCONTEXT); sole caller CWvsApp::ConnectLogin(0x8f38e5)
set(C_CLIENT_SOCKET_CONNECT_CTX 0x004850FC) # symbol; CONNECTCONTEXT wrapper tail-calling Connect(sockaddr_in)(0x485188); callers ConnectLogin + CWvsContext::IssueConnect
set(C_CLIENT_SOCKET_CONNECT_ADR 0x00485188) # symbol; sole socket(2,1,0) import(0x9cf35c) caller; cloned connect slot dword_AA7854(s,addr,16) + WSAAsyncSelect dword_AA7834

set(Z_SOCKET_BASE_CLOSE_SOCKET 0x00000000) # INLINED in v72 (new v72-only sentinel; was real 0x0048C699 in v79). v72 has NO standalone ZSocketBase::CloseSocket: the -1+closesocket teardown is inlined into Close(0x48668f)/Connect(sockaddr_in)(0x485188)/~CClientSocket(0x484e0b); shutdown is not even imported in v72 (no shutdown(s,2) call). closesocket import (0x9cf344) has exactly 4 callers, none standalone; the closesocket thunk (0x952570) has 0 callers. FLAG gate/edit owner: bypass/socket_hooks.cpp:324 m_sock.CloseSocket() must inline the close (closesocket + set -1) for v72, or its Connect-Addr hook must be skipped for v72

set(Z_SOCKET_BUFFER_ALLOC 0x004862F8) # symbol; dual ZAllocEx<ZAllocAnonSelector>::Alloc(a1, then 28=0x1C header)+placement ctor sub_486345; called by OnConnect with 0x5B4

set(C_CONFIG 0x0048C0D3) # symbol ??0CConfig@@QAE@XZ; SBB-singleton store -> g_CConfig_pInstance(AA3AC0) + 31/100/24 fuse + StringPool(2530=0x9E2) + RegOpenKeyExA(HKLM via dword_AA77B8) + memset(this+0x234,0x1BD). DRIFT v79 0x49392C; StringPool 2530 (v79 2532)
set(C_CONFIG_INSTANCE_ADDR 0x00AA3AC0) # g_CConfig_pInstance (renamed); SBB store in CConfig ctor @0x48C0D3, read by SendCheckPasswordPacket -> GetPartnerCode(g_CConfig). DRIFT v79 0xB0BED0
set(C_CONFIG_GET_PARTNER_CODE 0x005B12BD) # symbol ?GetPartnerCode@CConfig@@; uiWndZ0 key (aUiwndz0) + GetOpt_Int(0,key,0,INT_MIN=0x80000000,INT_MAX=0x7FFFFFFF). DRIFT v79 0x5CC09D
set(C_CONFIG_APPLY_SYS_OPT 0x0048E7EC) # symbol ?ApplySysOpt@CConfig@@; rep movsd this+0x60(12 dwords=0x30)+CWvsContext(dword_A9F438) flags @+0x3504/+0x3508 + 100*(x+1)/20 volume (imul 0x64/idiv 0x14)->SetBGMVolume/SetSEVolume(dword_AA3ABC) + InputSystemInstanceAddr+0x970. DRIFT v79 0x4960F9
set(C_CONFIG_CHECK_EXEC_PATH_REG 0x0048CBAE) # symbol ?CheckExecPathReg@CConfig@@; this+0xBC reg-handle gate + StringPool(3109/3110 = 0xC25/0xC26) + 0x5C backslash + GetFileAttributes(dword_AA7554; ==-1||&0x10) + strcmp. DRIFT v79 0x49440C; StringPool 3109/3110 (v79 3114/3115)
set(C_CONFIG_SYS_OPT_WINDOWED_MODE 0x00AA87AC) # g_CConfig_SysOpt_WindowedMode (renamed); read by 3 sites: SetUp(0x8F2C49), CreateMainWindow(0x8F3850 style branch 0x80000000/0x80000 + ?8:0 exstyle), InitializeGr2D(0x8F438B). DRIFT v79 0xB11548. Task 13 windowed-mode global

set(C_FUNC_KEY_MAPPED_MAN 0x005512EC) # symbol ??0CFuncKeyMappedMan@@QAE@XZ (ctor); installs vtable CFuncKeyMappedMan_vftable 0x9D22D8, instance 0xAA4CB8, memcpy DefaultFKM 0xA5B838 (0x1BD); called from CreateInstance @0x8f6294. RELOCATED (was v79 0x569DE5)
set(C_FUNC_KEY_MAPPED_MAN_VFTABLE 0x009D22D8) # CFuncKeyMappedMan_vftable; installed at *this (`*this = &off_9D22D8`) in the ctor 0x5512ec. RELOCATED (was v79 0xA2EB38)
set(C_FUNC_KEY_MAPPED_MAN_INSTANCE_ADDR 0x00AA4CB8) # FuncKeyMappedManInstanceAddr; SBB-singleton store in ctor + read at top of CreateInstance. RELOCATED (was v79 0xB0D2A8)
set(C_FUNC_KEY_MAPPED_MAN_CREATE_INSTANCE 0x008F6264) # symbol ?CreateInstance@?$TSingleton@VCFuncKeyMappedMan@@; Alloc(0x388=904)+ctor 0x5512ec; called from SetUp @0x8f2c77. Alloc size 0x388 confirms Task 2 CFuncKeyMappedMan sizing. RELOCATED (was v79 0x946AFB)

set(DEFAULT_FKM_INSTANCE_ADDR 0x00A5B838) # DefaultFKMInstanceAddr; 445-byte (0x1BD) FKM default blob, memcpy src in ctor 0x5512ec (this+4 and this+449) + DefaultFuncKeyMap 0x5516c1. RELOCATED (was v79 0xABF99C)
set(DEFAULT_QKM_INSTANCE_ADDR 0x00000000) # ABSENT in v72 (as v79): FKM ctor 0x5512ec zeroes the quickslot region (*((_DWORD*)this+224)=0; +225=0) — no QKM-default memcpy. Confirmed absent. FLAG gate/edit owner (key_mapped_hooks quickslot memcpy must tolerate 0)

set(C_INPUT_SYSTEM 0x008F489E) # symbol ??0CInputSystem@@QAE@XZ (ctor); called from InitializeInput @0x8f4615 + CreateInstance @0x8f5e71. RELOCATED (was v79 0x945204)
set(C_INPUT_SYSTEM_CREATE_INSTANCE 0x008F5E41) # symbol ?CreateInstance@?$TSingleton@VCInputSystem@@; Alloc(0x9D0=2512)+ctor 0x8f489e; called from CWvsApp::InitializeInput @0x8f4626. RELOCATED (was v79 0x9466CD)
set(C_INPUT_SYSTEM_INSTANCE_ADDR 0x00AA3E84) # InputSystemInstanceAddr; singleton dword read at top of CreateInstance; cross-confirmed by C_STAGE_ON_MOUSE_ENTER (SetCursorState on dword_AA3E84). RELOCATED (was v79 0xB0C29C)
set(C_INPUT_SYSTEM_INIT 0x0055CBA9) # symbol ?Init@CInputSystem@@QAEXPAUHWND__@@PAPAX@Z; called from InitializeInput @0x8f462d. RELOCATED (was v79 0x5757D4)
set(C_INPUT_SYSTEM_UPDATE_DEVICE 0x0055CFD3) # UpdateDevice_CInputSystem; if(!a1)UpdateKeyboard(0x55dc61) else if(a1==1)UpdateMouse(0x55d7e8); called from Run msgtype<=2 branch @0x8f304e (v84-name hint in IDB). RELOCATED (was v79 0x575BFE)
set(C_INPUT_SYSTEM_GET_IS_MESSAGE 0x0055CFF0) # GetIsMessage_CInputSystem; this[625] gate, copies 3 dwords from this[626]; Run inner drain loop w/ ISMsgProc @0x8f305d (v84-name hint). RELOCATED (was v79 0x575C1B)
set(C_INPUT_SYSTEM_GENERATE_AUTO_KEY_DOWN 0x0055DFBC) # GenerateAutoKeyDown_CInputSystem; *a2=256 + GetSpecialKeyFlag(0x55ded5); Run else-branch @0x8f3091 right before CSecurityClient::Update(sub_94222A,SecurityClientInstanceAddr) (v84-name hint). RELOCATED (was v79 0x576BE7)
set(C_INPUT_SYSTEM_SHOW_CURSOR 0x0055D022) # symbol ?ShowCursor@CInputSystem@@QAEXH@Z; called from CLogo::Init @0x5e130c. RELOCATED (was v79 0x575C4D)

set(C_LOGIN_UPDATE 0x005AFBBE) # CLogin primary vtable (off_9D316C) slot0; body uses [esi+0x15C] + InvalidateRect via dword_AA7624 ptr — DIVERGES from C_LOGO_UPDATE in v72 (0x5E1789); shared 0x5F4C16 in v83
set(C_LOGIN_SEND_CHECK_PASSWORD_PACKET 0x005B1170) # IDB symbol ?SendCheckPasswordPacket@CLogin@@QAEHPBD0@Z; COutPacket(opcode=1, was 0x05 in v79) + EncodeStr x2 + SendPacket(0x4866AC)

set(C_LOGO 0x005E11F9) # IDB symbol ??0CLogo@@QAE@XZ; 4-vtable-write ctor (off_9D3B94/9D3B48/9D3B44/9D3B40); Alloc(0x23C)+ctor pattern from CLogo::LogoEnd
set(C_LOGO_GET_RTTI 0x00421565) # IDB symbol ?GetRTTI@CLogo@@UBEPBVCRTTI@@XZ (v72 mangling uses CRTTI/UBE, not CRuntimeClass)
set(C_LOGO_IS_KIND_OF 0x0042156B) # IDB symbol ?IsKindOf@CLogo@@UBEHPBVCRTTI@@@Z (= GetRTTI+6)
set(C_LOGO_UPDATE 0x005E1789) # CLogo primary vtable (off_9D3B94) slot0; timer body [esi+0x24] + dword_AA7814; DIVERGES from C_LOGIN_UPDATE (0x5AFBBE) in v72
set(C_LOGO_ON_MOUSE_BUTTON 0x005E1774) # CLogo IUIMsgHandler vtable (off_9D3B48) slot 2; body: cmp [esp+arg_0],202h (WM_LBUTTONUP) then call InitNXLogo (0x5E13CB)
set(C_LOGO_ON_SET_FOCUS 0x005E1237) # CLogo IUIMsgHandler vtable (off_9D3B48) slot 1; body: push 1; pop eax; retn 4 (returns 1)
set(C_LOGO_ON_KEY 0x005E174D) # CLogo IUIMsgHandler vtable (off_9D3B48) slot 0; body: checks wParam==13||27||32 then call InitNXLogo (0x5E13CB)
set(C_LOGO_LOGO_END 0x005E1381) # call-graph: Alloc(0x23C)+CLogin_ctor(0x5AECED)+set_stage(0x6C1FBB); ends logo sequence
set(C_LOGO_FORCED_END 0x005E135F) # CLogo primary vtable (off_9D3B94) slot 2; PlayBGM(0x3E8) on CSoundMan singleton (dword_AA3ABC) + nullsub tail
set(C_LOGO_INIT 0x005E12F1) # CLogo primary vtable (off_9D3B94) slot 1; sub-init + CInputSystem::ShowCursor(0) + CWvsApp::GetCmdLine(3) + conditional LogoEnd
set(C_LOGO_INIT_NX_LOGO 0x005E13CB) # StringPool ID 1386 (0x56A; was 0x568 in v79) NX-logo resource + init-once guard on [this+0x28]

set(C_MACRO_SYS_MAN_CREATE_INSTANCE 0x00000000) # ABSENT in v72 (new v72-only sentinel; was real 0x00946C88 in v79). CMacroSysMan does not exist as a separate singleton in v72: no CMacroSysMan symbol, no "Macro" string, and it is NOT in the complete TSingleton CreateInstance list. The macro-sys-data-init role is folded into CQuickslotKeyMappedMan — CWvsContext::OnMacroSysDataInit (0x92126b) operates on QuickslotKeyMappedManInstanceAddr (0xAA3D04). FLAG gate/edit owner: consuming edit must tolerate 0

set(C_BATTLE_RECORD_MAN_CREATE_INSTANCE 0x00000000) # absent in v79: CBattleRecordMan is a v95+ feature; no CBattleRecordMan symbol or string in v79. Confirmed absent

set(C_MAPLE_TV_MAN_CREATE_INSTANCE 0x008F6353) # symbol ?CreateInstance@?$TSingleton@VCMapleTVMan@@; Alloc(0x3D0=976)+ctor 0x5e8902; instance 0xAA4E68; called from SetUp @0x8f2d2c. RELOCATED (was v79 0x946BEA)
set(C_MAPLE_TV_MAN_INSTANCE_ADDR 0x00AA4E68) # MapleTVManInstanceAddr; singleton dword read at top of CreateInstance; also drives the scheduled-message path (v83's radio role — radio is folded into CMapleTVMan). RELOCATED (was v79 0xB0D458)
set(C_MAPLE_TV_MAN_INIT 0x005E8B18) # symbol ?Init@CMapleTVMan@@QAEXXZ; called from SetUp @0x8f2d33 right after CreateInstance. RELOCATED (was v79 0x6074C7)

set(C_MONSTER_BOOK_MAN_CREATE_INSTANCE 0x008F5F3F) # symbol ?CreateInstance@?$TSingleton@VCMonsterBookMan@@; Alloc(0xA4=164)+ctor 0x8f5f84; instance 0xAA4D24; called from SetUp @0x8f2d7e. RELOCATED (was v79 0x9467D6)
set(C_MONSTER_BOOK_MAN_INSTANCE_ADDR 0x00AA4D24) # MonsterBookManInstanceAddr; singleton dword read at top of CreateInstance. RELOCATED (was v79 0xB0D314)
set(C_MONSTER_BOOK_MAN_LOAD_BOOK 0x0062F410) # symbol ?LoadBook@CMonsterBookMan@@QAEHXZ; called from SetUp @0x8f2d85 (failure -> CTerminateException). RELOCATED (was v79 0x651C1F)

set(C_OUT_PACKET 0x00656FA1) # symbol ??0COutPacket@@QAE@J@Z; _Alloc(256/push 100h)+Init(0x65707C) structure. DRIFT v79 0x67AD6B (helper sub_486FE4)
set(C_OUT_PACKET_ENCODE_1 0x004062C7) # symbol Encode1; push 1 + mov [eax+ecx],dl + inc; shared _EnsureCapacity 0x4062E5. DIRECT (== v79)
set(C_OUT_PACKET_ENCODE_2 0x00424F84) # symbol Encode2; push 2 + mov [eax+ecx],dx + add 2; shared _EnsureCapacity 0x4062E5. DRIFT v79 0x42539C
set(C_OUT_PACKET_ENCODE_4 0x00406324) # symbol Encode4; push 4 + mov [eax+ecx],edx + add 4; shared _EnsureCapacity 0x4062E5. DIRECT (== v79)
set(C_OUT_PACKET_ENCODE_STR 0x00468295) # symbol EncodeStr; ZXString len([eax-4])+2 + CIOBufferManipulator::EncodeStr. DRIFT v79 0x4694DE
set(C_OUT_PACKET_ENCODE_BUFFER 0x00465CB2) # symbol EncodeBuffer; _EnsureCapacity(Size)+memcpy+len+=Size; retn 8. DRIFT v79 0x466AE9
set(C_OUT_PACKET_MAKE_BUFFER_LIST 0x006570FA) # symbol; sole MakeBufferList callee in CClientSocket::SendPacket(0x4866AC) + 1460/0x5B4 chunk + ^0x13 shuffle (CFG-obfuscated). DRIFT v79 0x67AEC4

set(C_IG_CIPHER_INNO_HASH 0x00940D7E) # symbol ?innoHash@CIGCipher@@; no-key seed 0xC65053F2 + bShuffle loop (sub_940DB9); called from SendPacket between MakeBufferList+Flush. DRIFT v79 0x993442; seed 0xC65053F2 (v79 0xC6EF3720)

set(Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_CTOR 0x00402AB8) # symbol ??0?$ZSynchronizedHelper@VZFatalSection@@@@; acquire-loop off_A60AFC + Sleep(0) retry (dword_AA74FC); called from SendPacket (this+124). v72 == v79 VA (same 0x402AB8) but v72-specific globals confirm identity; DRIFT v83 0x403166
set(Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_DTOR 0x00402ADD) # ctor+0x25; mov eax,[ecx]; dec [eax+4]; jnz; and [eax],0 (release recursion count). RAII release pair w/ ctor. v72 confirmed by body; v72 == v79 VA. DRIFT v83 0x40318B. Listing aliases under ZAllocEx::Alloc (dump aliasing) — not renamed

set(C_QUEST_MAN_CREATE_INSTANCE 0x008F5E99) # symbol ?CreateInstance@?$TSingleton@VCQuestMan@@; Alloc(0x258=600)+ctor 0x6834e4; instance 0xAA4D28; called from SetUp @0x8f2d38. RELOCATED (was v79 0x946725)
set(C_QUEST_MAN_INSTANCE_ADDR 0x00AA4D28) # QuestManInstanceAddr; singleton dword read at top of CreateInstance + SetUp LoadPartyQuestInfo((CQuestMan*)dword_AA4D28) @0x8f2d6e. RELOCATED (was v79 0xB0D318)
set(C_QUEST_MAN_LOAD_DEMAND 0x00683A9D) # symbol ?LoadDemand@CQuestMan@@QAEHXZ; called from SetUp @0x8f2d3f (failure -> CTerminateException). RELOCATED (was v79 0x6A8CD6)
set(C_QUEST_MAN_LOAD_PARTY_QUEST_INFO 0x006887DE) # symbol ?LoadPartyQuestInfo@CQuestMan@@QAEXXZ; called from SetUp @0x8f2d6e on QuestManInstanceAddr. RELOCATED (was v79 0x6AE1F4)
set(C_QUEST_MAN_LOAD_EXCLUSIVE 0x00689C3E) # symbol ?LoadExclusive@CQuestMan@@QAEXXZ; called from SetUp @0x8f2d79 on QuestManInstanceAddr. RELOCATED (was v79 0x6AF68D)

set(C_QUICKSLOT_KEY_MAPPED_MAN 0x008F62BA) # symbol ?CreateInstance@?$TSingleton@VCQuickslotKeyMappedMan@@; Alloc(0x30=48)+ctor 0x5e385d; instance 0xAA3D04; called from SetUp @0x8f2c7c. Also hosts the v72 macro-sys role (OnMacroSysDataInit 0x92126b). RELOCATED (was v79 0x946B51)

set(C_RADIO_MANAGER_CREATE_INSTANCE 0x00000000) # ABSENT in v72 (as v79): no CRadioManager singleton — confirmed absent from the COMPLETE v72 TSingleton CreateInstance list (11 entries, none CRadioManager) + no CRadioManager symbol/string. The scheduled-message/radio role is folded into CMapleTVMan (0xAA4E68). FLAG gate/edit owner (common/CRadioManager.cpp must tolerate 0)
set(C_RADIO_MANAGER_INSTANCE_ADDR 0x00000000) # ABSENT in v72 (see above). RADIO-QUIRK VERDICT: the v79 seed of 0 is CORRECT for v72 — the v83 allocator-selector trap (0xBF0B00 = dword ZAllocEx selector, not the instance) does NOT apply here because the v79 seed was already 0, so nothing wrong was inherited. v72 has no separate radio instance global.

set(C_SECURITY_CLIENT_CREATE_INSTANCE 0x008F630E) # symbol ?CreateInstance@?$TSingleton@VCSecurityClient@@; Alloc(0x13C=316)+ctor 0x941dcf; instance 0xAA3EE4; called from SetUp @0x8f2c2e (then InitModule/StartModule). RELOCATED (was v79 0x946BA5)
set(C_SECURITY_CLIENT_INSTANCE_ADDR 0x00AA3EE4) # SecurityClientInstanceAddr; singleton dword read at top of CreateInstance + SetUp InitModule/StartModule on dword_AA3EE4 + Run CSecurityClient::Update(sub_94222A,dword_AA3EE4). RELOCATED (was v79 0xB0C308)
set(C_SECURITY_CLIENT_ON_PACKET 0x009422D1) # symbol ?OnPacket@CSecurityClient@@QAEXAAVCInPacket@@@Z; body Decode1; cmp al,4; jnz; OnCheckClientIntegrityRequest(0x9422f0); reached from CClientSocket::ProcessPacket case 0x14. SPOT-CHECKED. needs-main-review (security_hooks boundary). RELOCATED (was v79 0x994995)

set(STAGE_INSTANCE_ADDR 0x00AA54D4) # StageInstanceAddr (dword_AA54D4); zeroed then written by set_stage (0x6C1FBB); read by CWvsApp::CallUpdate (0x8F4991) + CClientSocket::ProcessPacket stage dispatch
set(SET_STAGE 0x006C1FBB) # IDB symbol ?set_stage@@YAXPAVCStage@@PAX@Z; clears STAGE_INSTANCE_ADDR, calls old_stage->vtable[2]=ForcedEnd, stores new stage, calls new_stage->vtable[1]=Init

set(GR_INSTANCE_ADDR 0x00AA85FC) # GrInstanceAddr (dword_AA85FC); output-arg store via factory sub_8F7257 in CWvsApp::InitializeGr2D (0x8F432B); consumed as IWzGr2D* (CreateCanvas 800x600)

set(RESET_LSP 0x0044A9B1) # ResetLSP — PRESENT in v79 (stale v83 "does not exist" comment corrected). WinSock2 Protocol_Catalog9 reg read + "wpclsp.dll" check + "netsh winsock reset" via CreateProcessA; sole xref = CWvsApp ctor @0x943066

set(C_STAGE_ON_MOUSE_ENTER 0x008DF289) # IDB symbol ?OnMouseEnter@CStage@@UAEXH@Z; body = SetCursorState(0) on CInputSystem singleton (dword_AA3E84) guarded by [+0x9B4]; inherited-CStage witness = CLogin IUIMsgHandler secondary vtable (off_9D3120) slot 32 (0x9D31A0)
set(C_STAGE_ON_PACKET 0x006C0C61) # IDB symbol ?OnPacket@CStage@@UAEXJAAVCInPacket@@@Z; also = CLogo OnPacket vtable (off_9D3B44) slot 0

set(C_SYSTEM_INFO 0x0094A6C0) # ctor (symbol ??0CSystemInfo@@QAE@XZ); installs vtable off_9DC404 (size 9); stack-constructed before Init in CLogin::SendCheckPasswordPacket. DRIFT v79 0x99CDB0
set(C_SYSTEM_INFO_INIT 0x0094A700) # symbol ?Init@CSystemInfo@@; Netbios MAC (ncb 0x37/0x32/0x33) + GetVolumeInformationA + RegOpenKeyExA("SOFTWARE\Microsoft\Windows\CurrentVersion") + CxSupportId(16B) + CoCreateGuid fallback. DRIFT v79 0x99CDF0
set(C_SYSTEM_INFO_GET_GAME_ROOM_CLIENT 0x0094AAE0) # symbol ?GetGameRoomClient@CSystemInfo@@ (0x11B4 process-table fn); called from SendCheckPasswordPacket. DRIFT v79 0x99D1D0
set(C_SYSTEM_INFO_GET_MACHINE_ID 0x0094A9E0) # symbol ?GetMachineId@CSystemInfo@@ (size 4); returns cached 16-byte id; EncodeBuffer(id,16) in SendCheckPasswordPacket. DRIFT v79 0x99D0D0

set(C_UI_TITLE_INSTANCE_ADDR 0x00AA5114) # UITitleInstanceAddr (dword_AA5114); read as CUITitle* by CUITitle::EnableLoginCtrl callers (sub_5B1318, SendCheckPasswordPacket) + destroyed via CWnd::Destroy in CLogin teardown (sub_5AF0C7, ForcedEnd path)

set(G_DW_TARGET_OS 0x00A9A164) # g_dwTargetOS; =1996 on OS major<5 or IsWow64Process, in CWvsApp ctor (DRIFT vs v79 0xB0239C)

set(C_WVS_APP 0x008F26C7) # ??0CWvsApp@@QAE@PBD@Z (ctor); WebStart/GameLaunching/token + IsWow64Process + g_CWvsApp/g_dwTargetOS writers + ResetLSP tail; sole WebStart referrer. needs-main-review
set(C_WVS_APP_INSTANCE_ADDR 0x00A9F658) # g_CWvsApp; SBB-singleton store at top of ctor (DRIFT vs v79 0xB07A68)
set(C_WVS_APP_IS_MSG_PROC 0x008F57AB) # symbol ?ISMsgProc@CWvsApp@@IAEXIIJ@Z; per-message dispatch, called from Run
set(C_WVS_APP_INITIALIZE_AUTH 0x00000000) # absent in v72 (confirmed, like v79): NMCO/CNMCOClientObject auth subsystem post-dates v72; SetUp makes no auth call
set(C_WVS_APP_INITIALIZE_PCOM 0x008F3735) # symbol ?InitializePCOM@CWvsApp@@IAEXXZ; called from SetUp (call-graph)
set(C_WVS_APP_CREATE_MAIN_WINDOW 0x008F3755) # symbol ?CreateMainWindow@CWvsApp@@IAEXXZ; called from SetUp
set(C_WVS_APP_CONNECT_LOGIN 0x008F38E5) # symbol ?ConnectLogin@CWvsApp@@QAEXXZ; called from SetUp
set(C_WVS_APP_INITIALIZE_RES_MAN 0x008F3A55) # symbol ?InitializeResMan@CWvsApp@@IAEXXZ; called from SetUp
set(C_WVS_APP_INITIALIZE_GR2D 0x008F432B) # symbol ?InitializeGr2D@CWvsApp@@IAEXXZ; called from SetUp
set(C_WVS_APP_INITIALIZE_INPUT 0x008F45D1) # symbol ?InitializeInput@CWvsApp@@IAEXXZ; called from SetUp
set(C_WVS_APP_INITIALIZE_SOUND 0x008F493B) # symbol ?InitializeSound@CWvsApp@@IAEXXZ; called from SetUp
set(C_WVS_APP_INITIALIZE_GAME_DATA 0x008F4E13) # symbol ?InitializeGameData@CWvsApp@@IAEXXZ; called from SetUp
set(C_WVS_APP_CREATE_WND_MANAGER 0x008F39F2) # symbol ?CreateWndManager@CWvsApp@@IAEXXZ; called from SetUp
set(C_WVS_APP_GET_CMD_LINE 0x008F5495) # symbol ?GetCmdLine@CWvsApp@@QAE?AV?$ZXString@D@@H@Z; called repeatedly from ctor
set(C_WVS_APP_DIR_BACK_SLASH_TO_SLASH 0x008F55F2) # symbol ?Dir_BackSlashToSlash@CWvsApp@@SAXPAD@Z; SetUp exec-path normalize step 1
set(C_WVS_APP_DIR_UP_DIR 0x008F5638) # symbol ?Dir_upDir@CWvsApp@@SAXPAD@Z; SetUp exec-path step 2
set(C_WVS_APP_DIR_SLASH_TO_BACK_SLASH 0x008F5615) # symbol ?Dir_SlashToBackSlash@CWvsApp@@SAXPAD@Z; SetUp exec-path step 3
set(C_WVS_APP_GET_EXCEPTION_FILE_NAME 0x008F57FC) # symbol ?GetExceptionFileName@CWvsApp@@SAPBDXZ; called from top of WinMain
set(C_WVS_APP_CALL_UPDATE 0x008F4991) # symbol ?CallUpdate@CWvsApp@@QAEXJ@Z; 30ms frame step + sole CWndMan::s_Update caller; called from Run
set(C_WVS_APP_RUN 0x008F2F82) # symbol ?Run@CWvsApp@@QAEXPAH@Z; msg-pump; exception-TI quad (Patch/Disconnect/Terminate/ZException) + CallUpdate/Redraw pair. needs-main-review
set(C_WVS_APP_SET_UP 0x008F2A7D) # symbol ?SetUp@CWvsApp@@QAEXXZ; init driver; Global\meteora+ehsvc+ws2_32 strings; walks Initialize*/Create* cluster; NO DR_init. needs-main-review

set(C_WVS_CONTEXT_INSTANCE_ADDR 0x00A9F438) # g_pWvsContext; CWvsContext singleton = g_pClientSocketInstance(0xA9F434)+4; loaded as this-arg to OnEnterGame in set_stage(0x6c205e) + read by both party senders + migrate. v72 relocated from v79 0xB07848
set(C_WVS_CONTEXT_ON_ENTER_GAME 0x008FF597) # symbol ?OnEnterGame@CWvsContext@@QAEXXZ; sole caller set_stage(0x6c1fbb)@0x6c2064 GetCharacterData!=0 branch; runs this+0x33xx member-ctor block (v79 +0x34xx; per-version offsets). v72 relocated from v79 0x950297
set(C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET 0x0F) # measured v72: first body instr lea ecx,[esi+3330h] @0x8FF5A6 after EH-prolog+reg-save; delta from base 0x8FF597 (v72 omits the v83 push 1, like v79; coincides with v79 0x0F, re-measured not copied)

set(WIN_MAIN 0x008EF5AD) # _WinMain@16; symbol + two startup literals (\npkgameuninstnomsg.exe / "MapleStoryGlobal :: ... Internet Explorer") + ctor->SetUp->Run chain; sole caller PE entry start(0x955DA3). needs-main-review
set(WIN_MAIN_AD_BALLOON_CONDITIONAL 0x959) # measured v72: jz(74 6F)@0x8EFF06 guarding ShowADBalloon block (490/190/60 — DRIFT vs v79 740/300/60) then MapleStoryGlobal push; delta from WinMain base 0x8EF5AD (DRIFT vs v79 0xA3D)
set(WIN_MAIN_PATCHER_OFFSET 0x212) # measured v72: call ShowStartUpWndModal(E8 97 FD FF FF)@0x8EF7BF; delta from WinMain base 0x8EF5AD (coincides with v79 0x212; re-measured at byte level, NOT copied)

set(C_WND_MAN_S_UPDATE 0x008E2D73) # symbol ?s_Update@CWndMan@@SAXXZ; sole callee of interest inside CallUpdate (call-graph)
set(C_WND_MAN_REDRAW_INVALIDATED_WINDOWS 0x008E2AF7) # symbol ?RedrawInvalidatedWindows@CWndMan@@SAXXZ; called from Run right after CallUpdate

set(Z_ARRAY_REMOVE_ALL 0x00425CEC) # symbol ?RemoveAll@?$ZArray@E@@; if(*this){ZAllocEx::Free(*this-4, selector unk_AA7CB8);*this=0}; stride-1. DRIFT v79 0x4260F4

set(Z_X_STRING_GET_BUFFER 0x00425D2B) # symbol ?_Cat@?$ZXString@D@@ (in-place assign family); empty->GetBuffer(Size,0)(0x414576)+memcpy+ReleaseBuffer (==assign), non-empty->grow+append. needs-main-review: no dedicated pure-assign in v72 (same as v79/v84); repo only calls on fresh ZXStrings. DRIFT v79 0x426133
set(Z_X_STRING_TRIM_RIGHT 0x0046C9B4) # symbol ?TrimRight@?$ZXString@D@@; default " \t\r\n" (asc_A5AAF0) + strchr + inner GetBuffer(0x414576). DRIFT v79 0x46DB7E
set(Z_X_STRING_TRIM_LEFT 0x0046CA69) # symbol ?TrimLeft@?$ZXString@D@@; same whitespace literal (asc_A5AAF0) + strchr + memcpy-shift remainder to front; adjacent to/right after TrimRight. DRIFT v79 0x46DC33

set(C_FIELD_SEND_JOIN_PARTY_MSG 0x00514462) # symbol ?SendJoinPartyMsg@CField@@QAEXABV?$ZXString@D@@@Z; COutPacket(0x7A)+Encode1(4=invite)+EncodeStr(name)+SendPacket(0x4866ac); reads g_pWvsContext(0xA9F438); one-arg invitee. OPCODE DRIFT 0x79(v79)->0x7A(v72). v72 relocated from v79 0x51B4C9
set(C_FIELD_SEND_JOIN_PARTY_MSG_OFFSET 0x60) # measured v72: level-gate jnb(73 2F)@0x5144C2 (cmp al,0Ah); delta from base 0x514462 (DRIFT vs v79 0x5E; same jnb instr, longer v72 preamble)

set(C_FIELD_SEND_CREATE_NEW_PARTY_MSG 0x005142B0) # symbol ?SendCreateNewPartyMsg@CField@@QAEXXZ (no v72 IDB symbol; labeled this task); COutPacket(0x7A)+Encode1(1=create)+SendPacket(0x4866ac); reads g_pWvsContext(0xA9F438); nullary, no EncodeStr. OPCODE DRIFT 0x79(v79)->0x7A(v72). v72 relocated from v79 0x51B318
set(C_FIELD_SEND_CREATE_NEW_PARTY_MSG_OFFSET 0x9E) # measured v72: level-gate jnb(73 34)@0x51434E (cmp al,0Ah); delta from base 0x5142B0 (DRIFT vs v79 0x9D by +1; same jnb instr)

set(C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST 0x0090C9BD) # symbol ?SendMigrateToITCRequest@CWvsContext@@QAEXXZ; "...Guest ID Users." string(0xA9A5C4)@0x90C9E0 + COutPacket(0x9A)+SendPacket(0x4866ac); reads g_pClientSocketInstance(0xA9F434). OPCODE DRIFT 0x99(v79)->0x9A(v72). v72 relocated from v79 0x95DD85
set(C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST_OFFSET 0xE9) # measured v72: ITC-gate jz(74 26)@0x90CAA6 (get_field+0x124>>4&1); delta from base 0x90C9BD (coincides with v79 0xE9; re-measured at byte level, NOT copied)

set(DR_CHECK 0x00000000) # absent in v79: DR/anti-debug subsystem not present (Task 2 R11: SetUp has no DR_init step; no NtGetContextThread import). Confirmed absent
set(DR_INIT 0x00000000) # absent in v79: DR subsystem absent (Task 2 R11: SetUp anti-tamper is only CSecurityClient+meteora+ehsvc+IAT-clone, no DR_init). Confirmed absent
set(CE_TRACER_RUN 0x00000000) # absent in v79: CeTracer (AhnLab eTracer) is a v95+ feature; no CeTracer/eTracer symbol or string in v79. Confirmed absent
set(SEND_HS_LOG 0x00000000) # ABSENT in v72 (new v72-only sentinel; was real 0x0093F8E0 in v79). The AhnLab HShield report log (sprintf "%s\HShield" + "MapleStory_Global:%s" -> EHSvc.dll ordinal-10 report) post-dates v72: those format strings + the report thunk are absent, and CConfig::GetSessionCharacterName (0x89065E) has no SendHSLog caller. v72 HackShield init folded into CSecurityClient::InitModule. FLAG gate/edit owner: consuming edit must tolerate 0

set(C_MOB_C_MOB 0x00611CDB) # symbol ??0CMob@@QAE@PAVCMobTemplate@@@Z (sole caller CreateMob 0x611C9F, ZAllocEx::Alloc(0x4C0=1216) non-zeroing); CLife base(sub_5AB61E) + 3 vtables (off_9D4010/9D3FEC/9D3FE8) + m_pTemplate@this+0x160 + 31/100/24 fuse + _ZtlSecureTear chain + MobStat::SetFrom(0x6D0896) + StringPool(958=0x3BE)/IWzCanvas tail@this+0x484. needs-main-review. DRIFT v79 0x630C2C; alloc 1216 (v79 1304); StringPool 958 (v79 957). DOOM (Task 15): m_bDoomReserved LEFT UNINITIALIZED + doom tail DOES NOT EXIST in v72 (struct only 0x4C0; v84 doom field is at 0x540/0x544, past struct end; highest ctor write ~this+0x4B8) -> v72 on doom-fix (<84) needs-fix side

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
