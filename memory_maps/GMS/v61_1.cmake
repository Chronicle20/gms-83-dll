# GMS v61.1 memory map. Seeded from v72_1.cmake (closest labeled anchor, +11 versions);
# every value is relocated and re-verified against the v61 binary per
# docs/tasks/task-010-gms-v61-support/. v61 sits TWO tiers below the original floor —
# below v72 (task-009), below v79 (task-008), below the v83 base. A value still equal
# to v72 means UNVERIFIED unless its tracking-table row in memory-map.md is marked ✔
# with a signature-catalog entry. There is NO labeled IDB below v61 to triangulate
# against, and the base branch is sometimes the already-twice-diverged v72-reduced
# branch — confirm every value by a v61 signature, never by proximity. (A v48 IDB is
# also loaded; it is NOT an anchor.)

set(VERSION_HEADER 8) # CONFIRMED v72 in OnConnect (0x48528f): `if (v17 != 8)` @0x485520 -> CTerminateException 0x22000007; held == v79 (Task 10). Major-version byte == 0x48 (72) @0x485544 corroborates the build
set(PLAYER_LOGGED_IN 0x14) # CONFIRMED v72: COutPacket(20) @0x48572a, logged-in (non-bLogin, [this+9]==0) OnConnect(0x48528f) branch (then Encode4 charId + Encode1 + Encode1); held == v79 (Task 10)
set(CLIENT_START_ERROR 0x1A) # DRIFT v79 0x19 -> v72 0x1A: COutPacket(26) @0x4856cd, bLogin OnConnect(0x48528f) branch (GetExceptionFileName CFileStream report relay -> Encode2 len + EncodeBuffer report). Confirmed v72 (Task 10; affects redirect/bypass)

set(GET_SE_PRIVILEGE 0x0044989E) # GetSEPrivilege (named); OpenProcessToken+LookupPrivilegeValueA("SeDebugPrivilege")+AdjustTokenPrivileges. v72 confirmed (import quartet + string); DRIFT v79 0x44A48E

set(C_ACTION_MAN_CREATE_INSTANCE_ADDR 0x008F6172) # symbol ?CreateInstance@?$TSingleton@VCActionMan@@; Alloc(0x2A0=672)+ctor 0x406497; instance global ActionManInstanceAddr 0xA9F3F4; called from CWvsApp::SetUp @0x8f2d1b. RELOCATED (was v79 0x946A09)
set(C_ACTION_MAN_INSTANCE_ADDR 0x00A9F3F4) # ActionManInstanceAddr; singleton dword read at top of CreateInstance + SBB store in ctor. RELOCATED (was v79 0xB07804); v72 singleton globals cluster at 0xA9F3Fx
set(C_ACTION_MAN_INIT 0x0040681C) # symbol ?Init@CActionMan@@QAEXXZ; called from SetUp @0x8f2d22 right after CreateInstance. DIRECT (== v79)
set(C_ACTION_MAN_SWEEP_CACHE 0x0040FE89) # symbol ?SweepCache@CActionMan@@QAEXXZ; sole caller CWvsApp::CallUpdate (0x8f4991). DRIFT (was v79 0x40FEEA)

set(C_ANIMATION_DISPLAYER_CREATE_INSTANCE 0x008F61C8) # symbol ?CreateInstance@?$TSingleton@VCAnimationDisplayer@@; Alloc(0x1A8=424)+ctor 0x431b69; instance 0xAA3A8C; called from SetUp @0x8f2d27. RELOCATED (was v79 0x946A5F)

set(C_CLIENT_SOCKET_INSTANCE_ADDR 0x00975054) # g_pClientSocketInstance (dword_975054); SBB-singleton store at top of CClientSocket ctor (0x4727fb) + CreateInstance read/store (0x825ff3). DRIFT vs v72 0xA9F434 (relocated; idiom identical)
set(C_CLIENT_SOCKET_CREATE_INSTANCE 0x00825FF3) # TSingleton<CClientSocket>::CreateInstance (symbol); Alloc(148=0x94)+ctor(0x4727fb)+store g_pClientSocketInstance(0x975054)
set(C_CLIENT_SOCKET_SEND_PACKET 0x00474125) # symbol + MakeBufferList(61)->innoHash->Flush(0x47421c) call-graph + ZSynchronizedHelper<ZFatalSection>@[this+124]/fd[this+8] gate. DRIFT: send-seq immediate 61 (v61 protocol; was 72 in v72, 83 in v83). chain v72(72)->v83(83) holds. needs-main-review
set(C_CLIENT_SOCKET_FLUSH 0x0047421C) # symbol (renamed from sub_47421C) + send-buffer ZList walk ([this+23], InterlockedIncrement) via cloned send slot dword_978224; sole call in SendPacket(0x4741e9); WSAEWOULDBLOCK(10035) via dword_9781F8->OnError. chain v72->v83 holds. needs-main-review
set(C_CLIENT_SOCKET_MANIPULATE_PACKET 0x00474336) # symbol + sole caller of ProcessPacket(0x47440a); recv-stream reassembly (CInPacket::AppendBuffer) + innoHash([this+136]). DRIFT: version/seq XOR check != -62 (~61; was -73=~72, -84=~83). chain v72->v83 holds. needs-main-review
set(C_CLIENT_SOCKET_PROCESS_PACKET 0x0047440A) # symbol + Decode2 + sub-0x10 dispatch (0x10 OnMigrateCommand/0x11 OnAliveReq/0x14 CSecurityClient::OnPacket; 0x1A..0x5B CWvsContext::OnPacket(g_pWvsContext=dword_974EF8) else CStage vtable+8); sole caller ManipulatePacket. DRIFT: opcode range 0x1A..0x5B (was 0x1A..0x71 in v72). needs-main-review
set(C_CLIENT_SOCKET_CLOSE 0x00474108) # symbol + ClearSendReceiveCtx + INLINE closesocket([this+8])+set -1 (v61 inlines ZSocketBase::CloseSocket; no shutdown call, same as v72)
set(C_CLIENT_SOCKET_CLEAR_SEND_RECEIVE_CTX 0x004748B5) # symbol; clears [this+26]/[this+56 word]/[this+30] + double ZList reset (recv [this+60]/send [this+80] via sub_474CFE); called by ctor/Close/OnConnect
set(C_CLIENT_SOCKET_ON_CONNECT 0x00472D42) # symbol; recv-handshake handler (getpeername import 0x8e53c0 xref + ZSocketBuffer::Alloc(0x5B4) + cloned recv slot dword_97822C); version byte==8 / major==0x3D(61); NO 8-byte client key (see catalog). DRIFT: major 0x3D (was 0x48=72 in v72)
set(C_CLIENT_SOCKET_CONNECT_LOGIN 0x00472A0B) # symbol; GetCmdLine(0/1)+rand server pick -> Connect(CONNECTCONTEXT)(0x472c3e); sole caller CWvsApp::ConnectLogin
set(C_CLIENT_SOCKET_CONNECT_CTX 0x00472C3E) # symbol; CONNECTCONTEXT wrapper tail-calling Connect(sockaddr_in)(0x472ca3); callers ConnectLogin + CWvsContext::IssueConnect
set(C_CLIENT_SOCKET_CONNECT_ADR 0x00472CA3) # symbol; sole socket(2,1,0) import(0x8e53c4) caller; cloned connect slot dword_978204(s,addr,16) + WSAAsyncSelect dword_9781E4

set(Z_SOCKET_BASE_CLOSE_SOCKET 0x00000000) # INLINED in v61 (sentinel, confirmed same as v72; was real 0x0048C699 in v79). v61 has NO standalone ZSocketBase::CloseSocket: the -1+closesocket teardown is inlined into Close(0x474108)/Connect(sockaddr_in)(0x472ca3); shutdown is not imported in v61 (imports_query shutdown -> empty); func_query CloseSocket -> empty. FLAG gate/edit owner: bypass/socket_hooks.cpp:324 m_sock.CloseSocket() must inline the close (closesocket + set -1) for v61, or its Connect-Addr hook must be skipped for v61

set(Z_SOCKET_BUFFER_ALLOC 0x00473D71) # symbol; dual ZAllocEx<ZAllocAnonSelector>::Alloc(a1, then 28=0x1C header)+placement ctor sub_473DBE; called by OnConnect(0x472def) with 0x5B4

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

set(C_BATTLE_RECORD_MAN_CREATE_INSTANCE 0x00000000) # CONFIRMED absent in v72: CBattleRecordMan is a v95+ feature; no "BattleRecord" string (find_regex) and not in the complete v72 TSingleton CreateInstance list (Task 7). Confirmed absent (SP-5)

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

set(RESET_LSP 0x00449DC1) # ResetLSP — PRESENT in v72 (symbol-labeled, size 0x1a0). Anchored by TWO string xrefs: "wpclsp.dll"(0xa5a598)@0x449e33 + WinSock2 Protocol_Catalog9 reg path(0xa5a5b8)@0x449dea; sole caller = CWvsApp ctor (0x8f26c7) @0x8f29f2. DRIFT (was v79 0x44A9B1); confirms task-008 v79 "PRESENT" verdict, not v83's stale "does not exist"

set(C_STAGE_ON_MOUSE_ENTER 0x008DF289) # IDB symbol ?OnMouseEnter@CStage@@UAEXH@Z; body = SetCursorState(0) on CInputSystem singleton (dword_AA3E84) guarded by [+0x9B4]; inherited-CStage witness = CLogin IUIMsgHandler secondary vtable (off_9D3120) slot 32 (0x9D31A0)
set(C_STAGE_ON_PACKET 0x006C0C61) # IDB symbol ?OnPacket@CStage@@UAEXJAAVCInPacket@@@Z; also = CLogo OnPacket vtable (off_9D3B44) slot 0

set(C_SYSTEM_INFO 0x0094A6C0) # ctor (symbol ??0CSystemInfo@@QAE@XZ); installs vtable off_9DC404 (size 9); stack-constructed before Init in CLogin::SendCheckPasswordPacket. DRIFT v79 0x99CDB0
set(C_SYSTEM_INFO_INIT 0x0094A700) # symbol ?Init@CSystemInfo@@; Netbios MAC (ncb 0x37/0x32/0x33) + GetVolumeInformationA + RegOpenKeyExA("SOFTWARE\Microsoft\Windows\CurrentVersion") + CxSupportId(16B) + CoCreateGuid fallback. DRIFT v79 0x99CDF0
set(C_SYSTEM_INFO_GET_GAME_ROOM_CLIENT 0x0094AAE0) # symbol ?GetGameRoomClient@CSystemInfo@@ (0x11B4 process-table fn); called from SendCheckPasswordPacket. DRIFT v79 0x99D1D0
set(C_SYSTEM_INFO_GET_MACHINE_ID 0x0094A9E0) # symbol ?GetMachineId@CSystemInfo@@ (size 4); returns cached 16-byte id; EncodeBuffer(id,16) in SendCheckPasswordPacket. DRIFT v79 0x99D0D0

set(C_UI_TITLE_INSTANCE_ADDR 0x00AA5114) # UITitleInstanceAddr (dword_AA5114); read as CUITitle* by CUITitle::EnableLoginCtrl callers (sub_5B1318, SendCheckPasswordPacket) + destroyed via CWnd::Destroy in CLogin teardown (sub_5AF0C7, ForcedEnd path)

set(G_DW_TARGET_OS 0x00000000) # ABSENT in v61 (new v61-only sentinel; was real 0x00A9A164 in v72). WoW64/target-OS marker (mov g_dwTargetOS,7CCh/1996 on OS major<5 OR IsWow64Process true) post-dates v61: v61 ctor (0x822E44) calls GetVersionEx but stores major<6/platformId==1 into a CWvsApp member, NO 7CCh store (byte-search C7 05 ?? ?? ?? ?? CC 07 00 00 = 0 hits), NO IsWow64Process, NO ResetLSP. FLAG gate/edit owner: consuming edit must tolerate 0

set(C_WVS_APP 0x00822E44) # ??0CWvsApp@@QAE@PBD@Z (ctor); symbol + string xrefs WebStart(0x969384)/token(0x96937C)/GameLaunching(0x96936C) + g_CWvsApp SBB store + called from WinMain before SetUp. v61 ctor SIMPLER than v72: NO IsWow64Process/g_dwTargetOS/ResetLSP. needs-main-review
set(C_WVS_APP_INSTANCE_ADDR 0x00970A78) # g_CWvsApp (renamed in IDB); SBB-singleton store at top of ctor (0x822E44); readers = ConnectLogin/SendCheckPasswordPacket/OnSelectWorldResult (login flow). RELOCATED from v72 0xA9F658
set(C_WVS_APP_IS_MSG_PROC 0x00825094) # ?ISMsgProc@CWvsApp@@IAEXIIJ@Z (was sub_825094, labeled this task; size 0x51); per-message dispatch, called only from Run(0x8233CC) both <=2 and >3 branches. RELOCATED from v72 0x8F57AB
set(C_WVS_APP_INITIALIZE_AUTH 0x00000000) # absent in v61 (confirmed, like v72/v79): no InitializeAuth symbol; NMCO/CNMCOClientObject auth subsystem post-dates v61; SetUp(0x823175) makes no auth call
set(C_WVS_APP_INITIALIZE_PCOM 0x008239B0) # symbol ?InitializePCOM@CWvsApp@@IAEXXZ; called from SetUp(0x823175) (call-graph). RELOCATED from v72 0x8F3735
set(C_WVS_APP_CREATE_MAIN_WINDOW 0x008239D0) # symbol ?CreateMainWindow@CWvsApp@@IAEXXZ; called from SetUp. RELOCATED from v72 0x8F3755
set(C_WVS_APP_CONNECT_LOGIN 0x00823B5B) # symbol ?ConnectLogin@CWvsApp@@QAEXXZ; called from SetUp. RELOCATED from v72 0x8F38E5
set(C_WVS_APP_INITIALIZE_RES_MAN 0x00823CA7) # symbol ?InitializeResMan@CWvsApp@@IAEXXZ; called from SetUp. RELOCATED from v72 0x8F3A55
set(C_WVS_APP_INITIALIZE_GR2D 0x00824550) # symbol ?InitializeGr2D@CWvsApp@@IAEXXZ; called from SetUp. RELOCATED from v72 0x8F432B
set(C_WVS_APP_INITIALIZE_INPUT 0x008247C3) # symbol ?InitializeInput@CWvsApp@@IAEXXZ; called from SetUp. RELOCATED from v72 0x8F45D1
set(C_WVS_APP_INITIALIZE_SOUND 0x008248B4) # symbol ?InitializeSound@CWvsApp@@IAEXXZ; called from SetUp. RELOCATED from v72 0x8F493B
set(C_WVS_APP_INITIALIZE_GAME_DATA 0x00824AB5) # symbol ?InitializeGameData@CWvsApp@@IAEXXZ; called from SetUp. RELOCATED from v72 0x8F4E13
set(C_WVS_APP_CREATE_WND_MANAGER 0x00823C44) # symbol ?CreateWndManager@CWvsApp@@IAEXXZ; called from SetUp. RELOCATED from v72 0x8F39F2
set(C_WVS_APP_GET_CMD_LINE 0x00824D80) # symbol ?GetCmdLine@CWvsApp@@QAE?AV?$ZXString@D@@H@Z; called repeatedly from ctor. RELOCATED from v72 0x8F5495
set(C_WVS_APP_DIR_BACK_SLASH_TO_SLASH 0x00824EDB) # symbol ?Dir_BackSlashToSlash@CWvsApp@@SAXPAD@Z; SetUp exec-path normalize step 1. RELOCATED from v72 0x8F55F2 (v61 addr order BackSlashToSlash<SlashToBackSlash<upDir, same as v72)
set(C_WVS_APP_DIR_UP_DIR 0x00824F21) # symbol ?Dir_upDir@CWvsApp@@SAXPAD@Z; SetUp exec-path step 2. RELOCATED from v72 0x8F5638
set(C_WVS_APP_DIR_SLASH_TO_BACK_SLASH 0x00824EFE) # symbol ?Dir_SlashToBackSlash@CWvsApp@@SAXPAD@Z; SetUp exec-path step 3. RELOCATED from v72 0x8F5615
set(C_WVS_APP_GET_EXCEPTION_FILE_NAME 0x008250E5) # symbol ?GetExceptionFileName@CWvsApp@@SAPBDXZ; called from top of WinMain(0x8205EF). RELOCATED from v72 0x8F57FC
set(C_WVS_APP_CALL_UPDATE 0x0082490A) # symbol ?CallUpdate@CWvsApp@@QAEXJ@Z; sole CWndMan::s_Update(0x81652D) caller; called from Run. RELOCATED from v72 0x8F4991
set(C_WVS_APP_RUN 0x008233CC) # symbol ?Run@CWvsApp@@QAEXPAH@Z; msg-pump; exception-TI quad (CPatchException 0x900668 / CDisconnect 0x900678 / CTerminate 0x8F4240 / ZException 0x8F6B60, codes 0x20000000/0x21000000-6/0x22000000-B) + CallUpdate(0x82490A)/Redraw(0x8162B1) pair; loop exits msg type 18. RELOCATED from v72 0x8F2F82. needs-main-review
set(C_WVS_APP_SET_UP 0x00823175) # symbol ?SetUp@CWvsApp@@QAEXXZ; init driver; srand->CSecurityClient CreateInstance/InitModule->InitializePCOM->CreateMainWindow->CClientSocket CreateInstance->ConnectLogin->FuncKey/Quickslot CreateInstance->ResMan->Gr2D->Input->Sound->GameData->CreateWndManager->ApplySysOpt->ActionMan/MapleTV/Quest/MonsterBook->Dir_* exec-path->CLogo->set_stage. NO InitializeAuth, NO DR_init/anti-debug (even cleaner than v72: no meteora/ehsvc/IAT-clone). RELOCATED from v72 0x8F2A7D. needs-main-review

set(C_WVS_CONTEXT_INSTANCE_ADDR 0x00A9F438) # g_pWvsContext; CWvsContext singleton = g_pClientSocketInstance(0xA9F434)+4; loaded as this-arg to OnEnterGame in set_stage(0x6c205e) + read by both party senders + migrate. v72 relocated from v79 0xB07848
set(C_WVS_CONTEXT_ON_ENTER_GAME 0x008FF597) # symbol ?OnEnterGame@CWvsContext@@QAEXXZ; sole caller set_stage(0x6c1fbb)@0x6c2064 GetCharacterData!=0 branch; runs this+0x33xx member-ctor block (v79 +0x34xx; per-version offsets). v72 relocated from v79 0x950297
set(C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET 0x0F) # measured v72: first body instr lea ecx,[esi+3330h] @0x8FF5A6 after EH-prolog+reg-save; delta from base 0x8FF597 (v72 omits the v83 push 1, like v79; coincides with v79 0x0F, re-measured not copied)

set(WIN_MAIN 0x008205EF) # _WinMain@16; string xref "MapleStoryGlobal :: ... Internet Explorer"(0x96932C) + ctor(0x822E44)->SetUp(0x823175)->Run(0x8233CC) chain; sole caller PE entry start(0x87921F). RELOCATED from v72 0x8EF5AD. needs-main-review
set(WIN_MAIN_AD_BALLOON_CONDITIONAL 0x714) # measured v61: jz(74 6D)@0x820D03 guarding ShowADBalloon block (dims 490/190/60 = 0x1EA/0xBE/0x3C, same as v72) then MapleStoryGlobal push; delta from WinMain base 0x8205EF. DRIFT vs v72 0x959 (re-measured at byte level, NOT copied). no-ad-balloon edit flips first byte 0x74->0xEB
set(WIN_MAIN_PATCHER_OFFSET 0x19A) # measured v61: call ?ShowStartUpWndModal@@YAXXZ(E8 0F FE FF FF)@0x820789; delta from WinMain base 0x8205EF. DRIFT vs v72 0x212 (re-measured at byte level, NOT copied). no-patcher edit NOPs these 5 bytes

set(C_WND_MAN_S_UPDATE 0x0081652D) # symbol ?s_Update@CWndMan@@SAXXZ; sole callee of interest inside CallUpdate(0x82490A) (call-graph). RELOCATED from v72 0x8E2D73
set(C_WND_MAN_REDRAW_INVALIDATED_WINDOWS 0x008162B1) # symbol ?RedrawInvalidatedWindows@CWndMan@@SAXXZ; called from Run(0x8233CC) right after CallUpdate. RELOCATED from v72 0x8E2AF7

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

set(DR_CHECK 0x00000000) # CONFIRMED absent in v72: DR/anti-debug subsystem not present (Task 2 R11/R12: SetUp has no DR_init step). Re-confirmed Task 10: no NtGetContextThread/GetContextThread import (imports_query empty). Confirmed absent (SP-5)
set(DR_INIT 0x00000000) # CONFIRMED absent in v72: DR subsystem absent (Task 2 R11/R12: SetUp anti-tamper is only CSecurityClient+meteora+ehsvc+IAT-clone, no DR_init). Re-confirmed Task 10 (no NtGetContextThread import). Confirmed absent (SP-5)
set(CE_TRACER_RUN 0x00000000) # CONFIRMED absent in v72: CeTracer (AhnLab eTracer) is a v95+ feature; no "eTracer"/"CeTracer" string in v72 (find_regex). Confirmed absent (SP-5)
set(SEND_HS_LOG 0x00000000) # ABSENT in v61 (carried sentinel; already absent v72, was real 0x0093F8E0 in v79). No SendHSLog symbol; the AhnLab HShield report log post-dates v61. v61 SetUp(0x823175) has no HShield/EHSvc strings — anti-tamper is only CSecurityClient. FLAG gate/edit owner: consuming edit must tolerate 0

set(C_MOB_C_MOB 0x00611CDB) # symbol ??0CMob@@QAE@PAVCMobTemplate@@@Z (sole caller CreateMob 0x611C9F, ZAllocEx::Alloc(0x4C0=1216) non-zeroing); CLife base(sub_5AB61E) + 3 vtables (off_9D4010/9D3FEC/9D3FE8) + m_pTemplate@this+0x160 + 31/100/24 fuse + _ZtlSecureTear chain + MobStat::SetFrom(0x6D0896) + StringPool(958=0x3BE)/IWzCanvas tail@this+0x484. needs-main-review. DRIFT v79 0x630C2C; alloc 1216 (v79 1304); StringPool 958 (v79 957). DOOM (Task 15): m_bDoomReserved LEFT UNINITIALIZED + doom tail DOES NOT EXIST in v72 (struct only 0x4C0; v84 doom field is at 0x540/0x544, past struct end; highest ctor write ~this+0x4B8) -> v72 on doom-fix (<84) needs-fix side

set(C_SECURITY_CLIENT_ON_PACKET_RET_STUB 0x00000000) # JMS only — JMS185 in-place RET stub (positive re-confirmed real @0xB3B96B, Task 10); GMS v72 hooks CSecurityClient::OnPacket via C_SECURITY_CLIENT_ON_PACKET (0x9422D1). Absent in GMS v72 by design (SP-5)
set(C_SECURITY_CLIENT_ON_PACKET_CHECK 0x00000000) # JMS only — JMS185 integrity-check fn re-confirmed real @0xB3B5F7 (CSecurityClient method (this,u16,COutPacket*), Task 10); no GMS counterpart. Absent in GMS v72 by design (SP-5)
set(C_SECURITY_CLIENT_ON_PACKET_CHECK_OFFSET 0x00000000) # JMS only — JMS185 CHECK+0x19 jz patch offset; no GMS counterpart. Absent in GMS v72 by design (SP-5)
set(C_WVS_APP_INITIALIZE_GR2D_WINDOWED_OFFSET 0x00000000) # JMS only — JMS185 windowed-mode in-place patch offset (0x94); GMS v72 forces windowed via C_CONFIG_SYS_OPT_WINDOWED_MODE (0xAA87AC). Absent in GMS v72 by design (SP-5)
set(WIN_MAIN_LAUNCHER_STUB 0x00000000) # JMS only — JMS185 StartUpDlgClass window proc re-confirmed real @0x7F3CE0 (Task 10); GMS v72 disables patcher via WIN_MAIN_PATCHER_OFFSET NOP. Absent in GMS v72 by design (SP-5)

# --- Faithful client exception dispatch (docs/tasks/exception-dispatch-cleanup) ---
# v72 __TI globals confirmed by IDB RTTI symbol + DOUBLE throw-site anchor:
#   CWvsApp::Run (0x8f2f82) exception-dispatch block (0x8f32ea-0x8f3377) AND OnConnect (0x48528f) handshake throws.
set(C_TI_DISCONNECT_EXCEPTION 0x009E34C0) # __TI3?AVCDisconnectException@@; _CxxThrowException Run @0x8f333d (0x21000000-0x21000006) + OnConnect @0x485314. RELOCATED (was v79 0xA40868)
set(C_TI_TERMINATE_EXCEPTION  0x009DF8C8) # __TI3?AVCTerminateException@@; _CxxThrowException Run @0x8f3366 (0x22000000-0x2200000B) + OnConnect @0x4852f4/0x48553a/0x485594. RELOCATED (was v79 0xA3CC38)
set(C_TI_PATCH_EXCEPTION       0x009ECC20) # __TI3?AVCPatchException@@; _CxxThrowException Run @0x8f3318 (==0x20000000) + OnConnect @0x485575. RELOCATED (was v79 0xA4AAD8)
set(C_TI_ZEXCEPTION            0x009E0048) # __TI1?AVZException@@; default _CxxThrowException Run @0x8f3377 + OnConnect ZException underrun throws. RELOCATED (was v79 0xA3D3B8)
set(C_PATCH_EXCEPTION_BUILDER  0x004FEEB7) # CPatchException_Build (KIND 1 thiscall ctor); *this=0x20000000, *(this+4)=72 (major DRIFT vs v79 79 / v83 83), memset 0x504; reached from Run @0x8f32f5 AND OnConnect @0x485556, each before qmemcpy 1288 + CPatch throw. RELOCATED (was v79 0x50A81B)
set(C_COM_RAISE_ERROR_EX       0x004031B5) # ?_com_issue_error@@YGXJ@Z (1-arg HRESULT raiser); CONFIRMED v72 by symbol + HRESULT-classification body. Run's discrete com call is the 2-arg _com_raise_error(hr,0) @0x8f3022 (?_com_raise_error@@ 0x95254c); this 1-arg form is the structural/semantic FAILED-render analog. DIRECT (v72 VA == v79, re-confirmed by symbol not copied)

# v72 CFileStream relay RECOVERABLE: OnConnect (0x48528f) decompiles CLEAN (not CFG-obfuscated like v83),
# so the CLIENT_START_ERROR report-read helpers resolve as real addresses (decision FR-8a). RESOLVED=1.
set(C_FILE_STREAM_RESOLVED     1)          # RECOVERABLE in v72 (clean OnConnect); relay enabled. Same disposition as v79
set(C_FILE_STREAM_OPEN_INLINE  0)          # out-of-line Open exists (CFileStream_Open 0x485A2A); CreateFileA NOT inlined into OnConnect. Same as v79
set(C_FILE_STREAM_OPEN         0x00485A2A) # CFileStream_Open; OnConnect call sub_485A2A(name,3,128,1,0x80000000,0,0) = OPEN_EXISTING/FILE_ATTRIBUTE_NORMAL/share=1/GENERIC_READ; first Close-resets then CreateFileA via cloned slot dword_AA755C, store handle this[4], OR open-flag this[13]. RELOCATED (was v79 0x48D31C)
set(C_FILE_STREAM_GET_LENGTH   0x00485BB3) # CFileStream_GetLength; thiscall wrapper dispatching object vtable[+60]. RELOCATED (was v79 0x48D4A5)
set(C_FILE_STREAM_READ         0x00485CDE) # CFileStream_Read(this,dst,len); memcpy from mapped view OR ReadFile via cloned slot dword_AA7560 on unmapped path. RELOCATED (was v79 0x48D5D0)
set(C_FILE_STREAM_CLOSE        0x004859CC) # CFileStream_Close; CloseHandle via cloned slot dword_AA7498 + handle this[4]=-1 (also dtor body). RELOCATED (was v79 0x48D2BE)
set(C_FILE_STREAM_VFTABLE      0x009D0914) # CFileStream_vftable (off_9D0914); installed at v35[0] in OnConnect(0x48528f) report-read path @0x485618. RELOCATED (was v79 0xA2CA2C)
