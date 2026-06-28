# GMS v61.1 memory map. Seeded from v72_1.cmake (closest labeled anchor, +11 versions);
# every value is relocated and re-verified against the v61 binary per
# docs/tasks/task-010-gms-v61-support/. v61 sits TWO tiers below the original floor —
# below v72 (task-009), below v79 (task-008), below the v83 base. A value still equal
# to v72 means UNVERIFIED unless its tracking-table row in memory-map.md is marked ✔
# with a signature-catalog entry. There is NO labeled IDB below v61 to triangulate
# against, and the base branch is sometimes the already-twice-diverged v72-reduced
# branch — confirm every value by a v61 signature, never by proximity. (A v48 IDB is
# also loaded; it is NOT an anchor.)

set(VERSION_HEADER 8) # CONFIRMED v61 in OnConnect (0x472d42): `if (v18 != 8)` @0x472faf -> CTerminateException 0x22000007. Major-version byte == 0x3D (61) compared @0x472fd3/0x473009 corroborates the build. Held == v72/v79 (still 8)
set(PLAYER_LOGGED_IN 0x14) # CONFIRMED v61: COutPacket(20) @0x4731bb, logged-in (non-bLogin, [this+9]==0) else-branch of OnConnect(0x472d42) (then Encode4 charId=*(g_pWvsContext+8328) + Encode1(0) + Encode1(0)); held == v72/v79
set(CLIENT_START_ERROR 0x19) # DRIFT v72 0x1A(26) -> v61 0x19(25): COutPacket(25) @0x47315d, bLogin branch (m_ctxConnect.bLogin=[this+9]) of OnConnect(0x472d42) (GetExceptionFileName CFileStream report relay -> Encode2 len + EncodeBuffer report). READ v61 immediate, not copied (Task 10; affects redirect/bypass)

set(GET_SE_PRIVILEGE 0x00000000) # absent in v61 — NEW v61-ONLY SENTINEL (was real v72 0x44989E GetSEPrivilege). v61 lacks debug-privilege escalation entirely: 0 "SeDebugPrivilege"/"Privilege" strings, NO OpenProcessToken/LookupPrivilegeValueA/AdjustTokenPrivileges imports (v72 has all four + string @0xA5A564 + GetSEPrivilege@0x44989E). Confirmed both directions. FLAGGED for gate/edit owner (SP-5 backward, R7)

set(C_ACTION_MAN_CREATE_INSTANCE_ADDR 0x00825F46) # symbol ?CreateInstance@?$TSingleton@VCActionMan@@; Alloc(0x2A0=672)+ctor CActionMan::CActionMan(0x405b79); reads/stores ActionManInstanceAddr(0x970830); called from CWvsApp::SetUp(0x823175)@0x8232a5. RELOCATED from v72 0x8F6172
set(C_ACTION_MAN_INSTANCE_ADDR 0x00970830) # ActionManInstanceAddr (dword_970830); singleton dword read at top of CreateInstance(0x825f46) + store in ctor. RELOCATED from v72 0xA9F3F4
set(C_ACTION_MAN_INIT 0x00405EFE) # symbol ?Init@CActionMan@@QAEXXZ; called from SetUp(0x823175)@0x8232ac right after CreateInstance (v6=CreateInstance; CActionMan::Init(v6)). RELOCATED from v72 0x40681C
set(C_ACTION_MAN_SWEEP_CACHE 0x0040F179) # symbol ?SweepCache@CActionMan@@QAEXXZ; sole caller CWvsApp::CallUpdate(0x82490a)@0x824a9f (xrefs_to confirms single xref). RELOCATED from v72 0x40FE89

set(C_ANIMATION_DISPLAYER_CREATE_INSTANCE 0x00825F9C) # symbol ?CreateInstance@?$TSingleton@VCAnimationDisplayer@@; Alloc(408)+ctor CAnimationDisplayer::CAnimationDisplayer(0x4271c4); instance dword_974EAC; called from SetUp(0x823175)@0x8232b1. RELOCATED from v72 0x8F61C8

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

set(C_CONFIG 0x0047921D) # ctor (was sub_47921D, relabeled ??0CConfig@@QAE@XZ in v61); SBB-singleton store -> g_CConfig_pInstance(0x974ED4) + vtable off_8E6760 + StringPool(2497) + RegOpenKeyExA(HKLM 0x80000002 via dword_978168) + memset(this+0x1BC,0x1BD). RELOCATED from v72 0x48C0D3; StringPool 2497 (v72 2530), memset off this+0x1BC (v72 0x234)
set(C_CONFIG_INSTANCE_ADDR 0x00974ED4) # g_CConfig_pInstance (renamed); SBB store in CConfig ctor @0x47921D, returned by TSingleton<CConfig>::GetInstance(0x4EFFB6). RELOCATED from v72 0xAA3AC0
set(C_CONFIG_GET_PARTNER_CODE 0x00564566) # symbol ?GetPartnerCode@CConfig@@; uiWndZ0 key (aUiwndz0) + GetOpt_Int(0,key,0,0x80000000,0x7FFFFFFF) via sub_47B793. RELOCATED from v72 0x5B12BD; anchors ported directly
set(C_CONFIG_APPLY_SYS_OPT 0x0047B28E) # symbol ?ApplySysOpt@CConfig@@; qmemcpy this+0x58(0x30 bytes)+CWvsContext(dword_974EF8) flags @+13088/+13092 + 100*(x+1)/20 volume (imul 0x64/idiv 0x14)->SetBGMVolume/SetSEVolume(dword_974ED0) + InputSystemInstanceAddr+2416. RELOCATED from v72 0x48E7EC; flag offsets 13088/13092 (v72 13572/13576)
set(C_CONFIG_CHECK_EXEC_PATH_REG 0x00479B4D) # symbol ?CheckExecPathReg@CConfig@@; this[38] reg-handle gate + StringPool(3071/3072) + 0x5C backslash + GetFileAttributes(dword_977F04; ==-1||&0x10) + strcmp + GetOpt_String/SetOpt_String. RELOCATED from v72 0x48CBAE; StringPool 3071/3072 (v72 3109/3110), handle gate this[38] (v72 this+0xBC)
set(C_CONFIG_SYS_OPT_WINDOWED_MODE 0x00978E24) # g_CConfig_SysOpt_WindowedMode (renamed); read by CreateMainWindow(0x8239D0 style branch !=0?0x80000000:0x80000 + ?8:0 exstyle) + InitializeGr2D(0x824550). RELOCATED from v72 0xAA87AC. Task 13 windowed-mode global

set(C_FUNC_KEY_MAPPED_MAN 0x0051AA0E) # symbol ??0CFuncKeyMappedMan@@QAE@XZ (ctor); installs vtable CFuncKeyMappedMan_vftable(0x8E7F58), instance dword_975CA0, memcpy DefaultFKM(0x962138) 0x1BD twice (this+4, this+449); called from CreateInstance(0x826038)@0x826068. RELOCATED from v72 0x5512EC
set(C_FUNC_KEY_MAPPED_MAN_VFTABLE 0x008E7F58) # CFuncKeyMappedMan_vftable; installed at *this (`*this = &off_8E7F58`) in the ctor 0x51aa0e. RELOCATED from v72 0x9D22D8
set(C_FUNC_KEY_MAPPED_MAN_INSTANCE_ADDR 0x00975CA0) # FuncKeyMappedManInstanceAddr (dword_975CA0); singleton store in ctor(0x51aa0e) + read at top of CreateInstance(0x826038). RELOCATED from v72 0xAA4CB8
set(C_FUNC_KEY_MAPPED_MAN_CREATE_INSTANCE 0x00826038) # symbol ?CreateInstance@?$TSingleton@VCFuncKeyMappedMan@@; Alloc(0x388=904)+ctor 0x51aa0e; called from SetUp(0x823175)@0x8231fb. Alloc size 0x388 confirms v61 CFuncKeyMappedMan sizing == v72. RELOCATED from v72 0x8F6264

set(DEFAULT_FKM_INSTANCE_ADDR 0x00962138) # DefaultFKMInstanceAddr; 445-byte (0x1BD) FKM default blob, memcpy src in ctor 0x51aa0e (this+4 and this+449) + DefaultFuncKeyMap@CFuncKeyMappedMan(0x51addf) — two independent xrefs (xrefs_to confirms). RELOCATED from v72 0xA5B838
set(DEFAULT_QKM_INSTANCE_ADDR 0x00000000) # ABSENT in v61 (carried; absent v72 too): FKM ctor 0x51aa0e zeroes the quickslot region (*((_DWORD*)this+224)=0; +225=0) — no QKM-default memcpy. Confirmed absent. FLAG gate/edit owner (key_mapped_hooks quickslot memcpy must tolerate 0)

set(C_INPUT_SYSTEM 0x00824817) # symbol ??0CInputSystem@@QAE@XZ (ctor); called from CWvsApp::InitializeInput(0x8247c3)@0x8247ed + CreateInstance(0x825c20)@0x825c50. RELOCATED from v72 0x8F489E
set(C_INPUT_SYSTEM_CREATE_INSTANCE 0x00825C20) # symbol ?CreateInstance@?$TSingleton@VCInputSystem@@; Alloc(0x9D0=2512)+ctor 0x824817; reads/stores InputSystemInstanceAddr(0x975050); called from CWvsApp::InitializeInput(0x8247c3)@0x8247fe. RELOCATED from v72 0x8F5E41
set(C_INPUT_SYSTEM_INSTANCE_ADDR 0x00975050) # InputSystemInstanceAddr (dword_975050); singleton dword read/stored in CreateInstance(0x825c20); cross-confirmed by C_STAGE_ON_MOUSE_ENTER (reads [dword_975050+0x9B4]) AND CLogo::Init ShowCursor(dword_975050). RELOCATED from STALE v72-seed 0xAA3E84 (Task 6 finding: real v61 singleton = dword_975050)
set(C_INPUT_SYSTEM_INIT 0x00524CEB) # symbol ?Init@CInputSystem@@QAEXPAUHWND__@@PAPAX@Z; called from InitializeInput(0x8247c3)@0x824805 on the CreateInstance result. RELOCATED from v72 0x55CBA9
set(C_INPUT_SYSTEM_UPDATE_DEVICE 0x00525113) # UpdateDevice_CInputSystem; if(!a1)UpdateKeyboard(sub_525D9F(1)) else if(a1==1)UpdateMouse(sub_525926); called from Run(0x8233CC) msgtype<=2 branch @0x82348e. body matches v72. RELOCATED from v72 0x55CFD3
set(C_INPUT_SYSTEM_GET_IS_MESSAGE 0x00525130) # GetIsMessage_CInputSystem; this[625] gate, copies 3 dwords from this[626]; Run(0x8233CC) inner drain loop @0x82349d right before ISMsgProc(0x825094). body matches v72. RELOCATED from v72 0x55CFF0
set(C_INPUT_SYSTEM_GENERATE_AUTO_KEY_DOWN 0x005260FA) # GenerateAutoKeyDown_CInputSystem; *a2=256 + GetSpecialKeyFlag(sub_526013); Run(0x8233CC) else-branch @0x8234ce right before CSecurityClient::Update(sub_86354E, SecurityClientInstanceAddr 0x97085C). body matches v72. RELOCATED from v72 0x55DFBC
set(C_INPUT_SYSTEM_SHOW_CURSOR 0x00525162) # symbol ?ShowCursor@CInputSystem@@QAEXH@Z; called from CLogo::Init(0x5951EC) on InputSystemInstanceAddr. RELOCATED from v72 0x55D022

set(C_LOGIN_UPDATE 0x00562EDC) # CLogin primary vtable (off_8E8C44) slot0; body tail-calls SendCheckPasswordPacket/MakeVACDlg/ResetVAC + CWnd::InvalidateRect(this+408,0) + reads CUITitle(0x975FC0). DIVERGES from C_LOGO_UPDATE (0x595670) in v61 (shared 0x5F4C16 only in v83). RELOCATED v72 0x5AFBBE
set(C_LOGIN_SEND_CHECK_PASSWORD_PACKET 0x00564418) # IDB symbol ?SendCheckPasswordPacket@CLogin@@QAEHPBD0@Z; COutPacket(opcode=1, same as v72) + EncodeStr(user)+EncodeStr(pass)+EncodeBuffer(MachineId,16)+Encode4/Encode1x3/Encode4 + SendPacket(0x474125) on g_pClientSocketInstance; reads CUITitle(0x975FC0). RELOCATED v72 0x5B1170

set(C_LOGO 0x005950F4) # IDB symbol ??0CLogo@@QAE@XZ; 4-vtable-write ctor (off_8E9668/8E961C/8E9618/8E9614, same shape as v72), base ctor sub_45BE8F; allocated by CLogo::LogoEnd Alloc+ctor. RELOCATED v72 0x5E11F9
set(C_LOGO_GET_RTTI 0x00595138) # mov eax, offset CLogo CRTTI descriptor (0x975FD4); retn. Unnamed in v61 (relabeled ?GetRTTI@CLogo@@UBEPBVCRTTI@@XZ). DRIFT: now INLINE in CLogo block (after OnSetFocus 0x595132) vs v72's distant stub region 0x421565
set(C_LOGO_IS_KIND_OF 0x0059513E) # = GetRTTI+6; CRTTI-chain walk on the same CLogo descriptor (0x975FD4). Unnamed in v61 (relabeled ?IsKindOf@CLogo@@UBEHPBVCRTTI@@@Z). DRIFT: inline (v72 0x42156B)
set(C_LOGO_UPDATE 0x00595670) # CLogo primary vtable (off_8E9668) slot0; init-once timer [esi+0x24] + GetUpdateTime (dword_9781C4) + elapsed cmp 0x5DC. DIVERGES from C_LOGIN_UPDATE (0x562EDC) in v61. RELOCATED v72 0x5E1789 (was undefined fn, define_func+rename)
set(C_LOGO_ON_MOUSE_BUTTON 0x0059565B) # CLogo IUIMsgHandler vtable (off_8E961C) slot 2; body: cmp [esp+arg_0],202h (WM_LBUTTONUP) then add ecx,-4; call InitNXLogo (0x5952C6). RELOCATED v72 0x5E1774
set(C_LOGO_ON_SET_FOCUS 0x00595132) # CLogo IUIMsgHandler vtable (off_8E961C) slot 1; body: push 1; pop eax; retn 4 (always-true stub). RELOCATED v72 0x5E1237
set(C_LOGO_ON_KEY 0x00595634) # CLogo IUIMsgHandler vtable (off_8E961C) slot 0; body: test [esp+0Bh],80h (key-up filter) then cmp 0Dh/1Bh/20h (13/27/32) then call InitNXLogo (0x5952C6). RELOCATED v72 0x5E174D (was undefined fn, define_func+rename)
set(C_LOGO_LOGO_END 0x0059527C) # call-graph: Alloc(476=0x1DC)+CLogin::CLogin(0x5620D4)+set_stage(0x65B22A); ends logo sequence. CLogin alloc 0x1DC SMALLER than v72 0x23C (and v79 0x258). RELOCATED v72 0x5E1381
set(C_LOGO_FORCED_END 0x0059525A) # CLogo primary vtable (off_8E9668) slot 2; CSoundMan::PlayBGM(...,1000,...) + nullsub_89 tail. RELOCATED v72 0x5E135F
set(C_LOGO_INIT 0x005951EC) # CLogo primary vtable (off_8E9668) slot 1; sub-init(0x595726) + CInputSystem::ShowCursor(dword_975050,0) + CWvsApp::GetCmdLine(3) + conditional LogoEnd(0x59527C) when cmdline non-empty && g_CWvsApp[+0x28]. RELOCATED v72 0x5E12F1
set(C_LOGO_INIT_NX_LOGO 0x005952C6) # init-once guard this[10]=[this+0x28] + GetUpdateTime + StringPool::GetBSTR(id 1380=0x564) NX-logo resource + IWzResMan::GetObjectA; called by OnKey+OnMouseButton. DRIFT StringPool ID 1380 (v72 1386, v79 0x568). RELOCATED v72 0x5E13CB

set(C_MACRO_SYS_MAN_CREATE_INSTANCE 0x00000000) # ABSENT in v61 (carried; also absent v72). CMacroSysMan does not exist as a separate singleton: no CMacroSysMan symbol/func (func_query *CMacroSys* -> empty), no CMacroSysMan string (find_regex -> 0), and it is NOT in the v61 SetUp(0x823175) CreateInstance chain. The macro-sys-data-init role is folded into CWvsContext::OnMacroSysDataInit (0x849bce). FLAG gate/edit owner: consuming edit must tolerate 0

set(C_BATTLE_RECORD_MAN_CREATE_INSTANCE 0x00000000) # CONFIRMED absent in v61: CBattleRecordMan is a v95+ feature; no "BattleRecord" string (find_regex -> 0) and not in the v61 SetUp(0x823175) CreateInstance chain (Task 7). Confirmed absent (SP-5)

set(C_MAPLE_TV_MAN_CREATE_INSTANCE 0x00826124) # symbol ?CreateInstance@?$TSingleton@VCMapleTVMan@@; Alloc(944)+ctor CMapleTVMan::CMapleTVMan(0x59b913); reads/stores MapleTVManInstanceAddr(0x975DDC); called from SetUp(0x823175)@0x8232b6. PRESENT in v61 (confirmed, not a backward sentinel). RELOCATED from v72 0x8F6353
set(C_MAPLE_TV_MAN_INSTANCE_ADDR 0x00975DDC) # MapleTVManInstanceAddr (dword_975DDC); singleton dword read/stored in CreateInstance(0x826124); also drives the scheduled-message path (radio folded into CMapleTVMan). RELOCATED from v72 0xAA4E68
set(C_MAPLE_TV_MAN_INIT 0x0059BB27) # symbol ?Init@CMapleTVMan@@QAEXXZ; called from SetUp(0x823175)@0x8232bd right after CreateInstance (v7=CreateInstance; CMapleTVMan::Init(v7)). RELOCATED from v72 0x5E8B18

set(C_MONSTER_BOOK_MAN_CREATE_INSTANCE 0x00825D13) # symbol ?CreateInstance@?$TSingleton@VCMonsterBookMan@@; Alloc(0xA4=164)+ctor CMonsterBookMan::CMonsterBookMan(0x825d58); reads/stores MonsterBookManInstanceAddr(0x975CF8); called from SetUp(0x823175)@0x8232f2. PRESENT in v61 (confirmed). RELOCATED from v72 0x8F5F3F
set(C_MONSTER_BOOK_MAN_INSTANCE_ADDR 0x00975CF8) # MonsterBookManInstanceAddr (dword_975CF8); singleton dword read/stored in CreateInstance(0x825d13). RELOCATED from v72 0xAA4D24
set(C_MONSTER_BOOK_MAN_LOAD_BOOK 0x005DD687) # symbol ?LoadBook@CMonsterBookMan@@QAEHXZ; called from SetUp(0x823175)@0x8232f9 (failure -> CTerminateException 0x82331d). RELOCATED from v72 0x62F410

set(C_OUT_PACKET 0x005FFC4F) # needs-main-review; symbol ??0COutPacket@@QAE@J@Z (v61 size 0x49); push 100h 256-alloc (helper sub_474B76) + `and [esi+4],0` zero-member + Init tail-call sub_5FFC98 + RemoveAll unwind. DRIFT v72 0x656FA1 (relocated; ctor +3 bytes vs v72 0x46, Init lost symbol)
set(C_OUT_PACKET_ENCODE_1 0x00456CF5) # needs-main-review; symbol Encode1 (0x1e); push 1 + mov [eax+ecx],dl + inc [esi+8] (_BYTE); shared _EnsureCapacity 0x456D13 (5-sibling fan-in). DRIFT v72 0x4062C7 (relocated)
set(C_OUT_PACKET_ENCODE_2 0x0045C250) # needs-main-review; symbol Encode2 (0x21); push 2 + mov [eax+ecx],dx + add [esi+8],2 (_WORD); shared _EnsureCapacity 0x456D13. DRIFT v72 0x424F84 (relocated)
set(C_OUT_PACKET_ENCODE_4 0x00456D52) # needs-main-review; symbol Encode4 (0x1f); push 4 + mov [eax+ecx],edx + add [esi+8],4 (_DWORD); shared _EnsureCapacity 0x456D13. DRIFT v72 0x406324 (relocated)
set(C_OUT_PACKET_ENCODE_STR 0x00458C91) # needs-main-review; symbol EncodeStr (0x66); ZXString len([eax-4])+2 grow + CIOBufferManipulator::EncodeStr(0x458CF7); in 5-sibling fan-in. DRIFT v72 0x468295 (relocated)
set(C_OUT_PACKET_ENCODE_BUFFER 0x00456FBA) # needs-main-review; symbol EncodeBuffer (0x2a); _EnsureCapacity(Size)+memcpy(0x876550)+add [esi+8],edi; retn 8; in 5-sibling fan-in. DRIFT v72 0x465CB2 (relocated)
set(C_OUT_PACKET_MAKE_BUFFER_LIST 0x005FFCE0) # needs-main-review; sole MakeBufferList callee of CClientSocket::SendPacket(0x474125) + 1460/0x5B4 MTU chunk + ^0x13 shuffle (ROR1(v^0x13,3)) + ZSocketBuffer::Alloc; size 0x342 == v72/v83. Was unnamed sub_5FFCE0, renamed. DRIFT v72 0x6570FA (relocated)

set(C_IG_CIPHER_INNO_HASH 0x0086274C) # symbol ?innoHash@CIGCipher@@; no-key seed 0xC65053F2 (== v72) + bShuffle loop (sub_862787). RELOCATED from v72 0x940D7E; seed 0xC65053F2 ported directly (v79 differs 0xC6EF3720)

set(Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_CTOR 0x00402ABA) # symbol ??0?$ZSynchronizedHelper@VZFatalSection@@@@; acquire-loop off_966D68 + Sleep(0) retry (dword_977EAC). RELOCATED from v72 0x402AB8 (+2 bytes); v61-specific globals confirm identity
set(Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_DTOR 0x00402ADF) # ctor+0x25; mov eax,[ecx]; dec [eax+4]; jnz; and [eax],0 (release recursion count). RAII release pair w/ ctor; confirmed by body. RELOCATED from v72 0x402ADD. Listing aliases under ZAllocEx::Alloc (dump aliasing) — not renamed

set(C_QUEST_MAN_CREATE_INSTANCE 0x00825C78) # symbol ?CreateInstance@?$TSingleton@VCQuestMan@@; Alloc(412)+ctor CQuestMan::CQuestMan(0x628c28); reads/stores QuestManInstanceAddr(0x975DFC); called from SetUp(0x823175)@0x8232c2. RELOCATED from v72 0x8F5E99
set(C_QUEST_MAN_INSTANCE_ADDR 0x00975DFC) # QuestManInstanceAddr (dword_975DFC); singleton dword read/stored in CreateInstance(0x825c78). RELOCATED from v72 0xAA4D28
set(C_QUEST_MAN_LOAD_DEMAND 0x00629040) # symbol ?LoadDemand@CQuestMan@@QAEHXZ; called from SetUp(0x823175)@0x8232c9 (failure -> CTerminateException 0x8232ed). RELOCATED from v72 0x683A9D
set(C_QUEST_MAN_LOAD_PARTY_QUEST_INFO 0x00000000) # NEW v61-ONLY SENTINEL (was real v72 0x6887DE). LoadPartyQuestInfo does NOT exist in v61: func_query name_regex "PartyQuest" -> 0 results; full *CQuestMan* symbol set lacks it; v61 SetUp(0x823175) does NOT call it (v72 SetUp did @v72 0x8f2d6e). Party-quest-info load post-dates v61. CONFIRMED absent. FLAG gate/edit owner: consuming edit must tolerate 0
set(C_QUEST_MAN_LOAD_EXCLUSIVE 0x00000000) # NEW v61-ONLY SENTINEL (was real v72 0x689C3E). LoadExclusive does NOT exist in v61: func_query name_regex "Exclusive" -> 0 results; full *CQuestMan* symbol set lacks it; v61 SetUp(0x823175) does NOT call it (v72 SetUp did @v72 0x8f2d79). Exclusive-quest load post-dates v61. CONFIRMED absent. FLAG gate/edit owner: consuming edit must tolerate 0

set(C_QUICKSLOT_KEY_MAPPED_MAN 0x0082608E) # symbol ?CreateInstance@?$TSingleton@VCQuickslotKeyMappedMan@@; Alloc(0x30=48)+ctor CQuickslotKeyMappedMan::CQuickslotKeyMappedMan(0x5972e9); reads/stores QuickslotKeyMappedManInstanceAddr(dword_974EE4); called from SetUp(0x823175)@0x823200. RELOCATED from v72 0x8F62BA

set(C_RADIO_MANAGER_CREATE_INSTANCE 0x00000000) # ABSENT in v61 (carried; also absent v72): no CRadioManager singleton — not in the v61 SetUp(0x823175) CreateInstance chain + no CRadioManager symbol/func/string (func_query *CRadioManager* -> empty, find_regex -> 0). Scheduled-message/radio role folded into CMapleTVMan (0x975DDC). FLAG gate/edit owner (common/CRadioManager.cpp must tolerate 0)
set(C_RADIO_MANAGER_INSTANCE_ADDR 0x00000000) # ABSENT in v61 (carried). RADIO-QUIRK VERDICT: v72 seed of 0 is CORRECT for v61 too — the v83 allocator-selector trap (0xBF0B00 = ZAllocEx selector, not the instance) does NOT apply because the seed was already 0, so nothing wrong was inherited. v61 has no separate radio instance global (no CRadioManager exists).

set(C_SECURITY_CLIENT_CREATE_INSTANCE 0x008260E2) # symbol ?CreateInstance@?$TSingleton@VCSecurityClient@@; Alloc(88)+ctor CSecurityClient::CSecurityClient(0x86333d); reads/stores SecurityClientInstanceAddr(0x97085C); called from SetUp(0x823175)@0x823197 (then InitModule sub_86343D). RELOCATED from v72 0x8F630E
set(C_SECURITY_CLIENT_INSTANCE_ADDR 0x0097085C) # SecurityClientInstanceAddr (dword_97085C); singleton dword read/stored in CreateInstance(0x8260e2) + SetUp early use *(dword_97085C+56) @0x8231ec + Run CSecurityClient::Update(sub_86354E, dword_97085C) @0x8234ed. RELOCATED from v72 0xAA3EE4
set(C_SECURITY_CLIENT_ON_PACKET 0x008637C4) # OnPacket_CSecurityClient (was sub_8637C4, labeled this task; unnamed in v61); body Decode1(a1) -> if !v=>sub_863803(integrity-req) / v==2=>sub_86384D / v==3=>sub_863860; reached from CClientSocket::ProcessPacket(0x47440a) case 0x14. SPOT-CHECKED. needs-main-review (security_hooks boundary). RELOCATED from v72 0x9422D1 (dispatch shape differs: v61 switches on 0/2/3, v72 cmp==4)

set(STAGE_INSTANCE_ADDR 0x00976264) # StageInstanceAddr (dword_976264); zeroed at entry of set_stage (0x65B22A), new stage installed (sub_65B4A9) + dispatched vtable+4=Init; read by CWvsApp::CallUpdate (0x82490A) + CClientSocket::ProcessPacket (0x47440A) stage dispatch. RELOCATED v72 0xAA54D4
set(SET_STAGE 0x0065B22A) # IDB symbol ?set_stage@@YAXPAVCStage@@PAX@Z; clears STAGE_INSTANCE_ADDR (0x976264), calls old_stage->vtable+8=ForcedEnd, stores new stage (sub_65B4A9), calls new_stage->vtable+4=Init; sole set_stage caller of CLogo::LogoEnd. RELOCATED v72 0x6C1FBB

set(GR_INSTANCE_ADDR 0x00978D34) # GrInstanceAddr (dword_978D34); output-arg store via factory sub_826D55(res,&dword_978D34,0) in CWvsApp::InitializeGr2D (0x824550); consumed as IWzGr2D* (CreateCanvas 800x600 vtable+12, SetColor 0xFF000000 vtable+76). RELOCATED v72 0xAA85FC

set(RESET_LSP 0x00000000) # NEW v61-ONLY SENTINEL (was carried v72 seed 0x00449DC1). ResetLSP feature ABSENT in v61: no "wpclsp.dll" string + no "Protocol_Catalog9" WinSock2 reg-path string (find_regex both -> 0; these were v72's TWO anchors), and the v61 CWvsApp ctor (0x822E44) makes NO ResetLSP call (Task 2 / C_WVS_APP: "v61 ctor SIMPLER than v72: NO IsWow64Process/g_dwTargetOS/ResetLSP"). The WSAStartup/LSP-reset machinery post-dates v61. CONFIRMED absent both directions. FLAG gate/edit owner: consuming edit must tolerate 0

set(C_STAGE_ON_MOUSE_ENTER 0x00659F7A) # body = if(arg) if([CInputSystem singleton dword_975050 + 0x9B4]) SetCursorState(0); retn 4. Unnamed in v61 (relabeled ?OnMouseEnter@CStage@@UAEXH@Z). Witness = CLogin IUIMsgHandler vtable (off_8E8BF8) slot 5. RELOCATED v72 0x8DF289
set(C_STAGE_ON_PACKET 0x00659F99) # IDB symbol ?OnPacket@CStage@@UAEXJAAVCInPacket@@@Z; also = CLogo OnPacket vtable (off_8E9618) slot 0. RELOCATED v72 0x6C0C61

set(C_SYSTEM_INFO 0x008658E0) # ctor (symbol ??0CSystemInfo@@QAE@XZ); installs vtable off_8F0E44; stack-constructed before Init in CLogin::SendCheckPasswordPacket. RELOCATED from v72 0x94A6C0
set(C_SYSTEM_INFO_INIT 0x00865920) # symbol ?Init@CSystemInfo@@; Netbios MAC (ncb 0x37/0x32/0x33) + GetVolumeInformationA + RegOpenKeyExA("SOFTWARE\Microsoft\Windows\CurrentVersion") + CxSupportId(16B) + CoCreateGuid fallback. RELOCATED from v72 0x94A700; anchors ported directly
set(C_SYSTEM_INFO_GET_GAME_ROOM_CLIENT 0x00865D00) # symbol ?GetGameRoomClient@CSystemInfo@@ (0x11B4 process-table fn); called from SendCheckPasswordPacket. RELOCATED from v72 0x94AAE0
set(C_SYSTEM_INFO_GET_MACHINE_ID 0x00865C00) # symbol ?GetMachineId@CSystemInfo@@; lea eax,[ecx+0x14] returns cached 16-byte id; EncodeBuffer(id,16) in SendCheckPasswordPacket. RELOCATED from v72 0x94A9E0

set(C_UI_TITLE_INSTANCE_ADDR 0x00975FC0) # UITitleInstanceAddr (dword_975FC0); read as CUITitle* by CLogin::SendCheckPasswordPacket (0x564418) + OnCheckPasswordResult/OnCheckPinCodeResult/login-control fan-in (sub_56363B etc.). RELOCATED v72 0xAA5114

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

set(C_WVS_CONTEXT_INSTANCE_ADDR 0x00974EF8) # g_pWvsContext (renamed from dword_974EF8 this task); CWvsContext singleton loaded as this-arg to OnEnterGame in set_stage(0x65b2cd) + used by both party senders (GetCharacterData/GetPartyMemberNumber) + migrate. DIVERGENCE: NOT g_pClientSocketInstance(0x975054)+4 (would be 0x975058) — the v72 socket+4 layout does NOT hold in v61; confirmed independently by the senders. RELOCATED from v72 0xA9F438
set(C_WVS_CONTEXT_ON_ENTER_GAME 0x0082DD91) # symbol ?OnEnterGame@CWvsContext@@QAEXXZ; sole caller set_stage(0x65b22a)@0x65b2d3 (mov ecx,g_pWvsContext) in GetCharacterData!=0 branch; runs this+0x31xx member-ctor block (v72 +0x33xx; per-version offsets). RELOCATED from v72 0x8FF597
set(C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET 0x0F) # MEASURED v61: first body instr lea ecx,[esi+31A0h] @0x82DDA0 after EH-prolog+reg-save (push ecx/push esi/mov esi,ecx/push edi); delta from base 0x82DD91 (coincides numerically with v72 0x0F but re-measured at byte level, NOT copied)

set(WIN_MAIN 0x008205EF) # _WinMain@16; string xref "MapleStoryGlobal :: ... Internet Explorer"(0x96932C) + ctor(0x822E44)->SetUp(0x823175)->Run(0x8233CC) chain; sole caller PE entry start(0x87921F). RELOCATED from v72 0x8EF5AD. needs-main-review
set(WIN_MAIN_AD_BALLOON_CONDITIONAL 0x714) # measured v61: jz(74 6D)@0x820D03 guarding ShowADBalloon block (dims 490/190/60 = 0x1EA/0xBE/0x3C, same as v72) then MapleStoryGlobal push; delta from WinMain base 0x8205EF. DRIFT vs v72 0x959 (re-measured at byte level, NOT copied). no-ad-balloon edit flips first byte 0x74->0xEB
set(WIN_MAIN_PATCHER_OFFSET 0x19A) # measured v61: call ?ShowStartUpWndModal@@YAXXZ(E8 0F FE FF FF)@0x820789; delta from WinMain base 0x8205EF. DRIFT vs v72 0x212 (re-measured at byte level, NOT copied). no-patcher edit NOPs these 5 bytes

set(C_WND_MAN_S_UPDATE 0x0081652D) # symbol ?s_Update@CWndMan@@SAXXZ; sole callee of interest inside CallUpdate(0x82490A) (call-graph). RELOCATED from v72 0x8E2D73
set(C_WND_MAN_REDRAW_INVALIDATED_WINDOWS 0x008162B1) # symbol ?RedrawInvalidatedWindows@CWndMan@@SAXXZ; called from Run(0x8233CC) right after CallUpdate. RELOCATED from v72 0x8E2AF7

set(Z_ARRAY_REMOVE_ALL 0x0045DBB3) # symbol ?RemoveAll@?$ZArray@E@@; if(*this){ZAllocEx::Free(*this-4);*this=0}; stride-1. RELOCATED from v72 0x425CEC; v61 Free is single-arg (v72 passed selector unk_AA7CB8)

set(Z_X_STRING_GET_BUFFER 0x0045DFA4) # symbol ?_Cat@?$ZXString@D@@ (in-place assign family); empty->GetBuffer(Size,0)(0x415000)+memcpy (==assign), non-empty->grow(*2)+append. needs-main-review: no dedicated pure-assign (same as v72/v79/v84); repo only calls on fresh ZXStrings. RELOCATED from v72 0x425D2B
set(Z_X_STRING_TRIM_RIGHT 0x0045DD5B) # symbol ?TrimRight@?$ZXString@D@@; default " \t\r\n" (asc_96156C) + strchr + inner GetBuffer(0x415000). RELOCATED from v72 0x46C9B4
set(Z_X_STRING_TRIM_LEFT 0x0045DE10) # symbol ?TrimLeft@?$ZXString@D@@; same whitespace literal (asc_96156C) + strchr + memcpy-shift remainder to front; adjacent to/right after TrimRight. RELOCATED from v72 0x46CA69

set(C_FIELD_SEND_JOIN_PARTY_MSG 0x004E8B29) # symbol ?SendJoinPartyMsg@CField@@QAEXABV?$ZXString@D@@@Z; push 70h->COutPacket(0x70)(0x5ffc4f)+Encode1(4=invite)(0x456cf5)+EncodeStr(name)(0x458c91)+SendPacket(0x474125); reads g_pWvsContext(0x974EF8) via GetCharacterData+GetPartyMemberNumber<6; one-arg invitee. OPCODE DRIFT 0x7A(v72)->0x70(v61), read not assumed. RELOCATED from v72 0x514462. needs-main-review
set(C_FIELD_SEND_JOIN_PARTY_MSG_OFFSET 0x5A) # MEASURED v61: level-gate jnb(73 2C)@0x4E8B83 (right after cmp al,0Ah); delta from base 0x4E8B29 (DRIFT vs v72 0x60; same level-gate jnb instr the beginner-party-block edit patches; re-measured, NOT copied)

set(C_FIELD_SEND_CREATE_NEW_PARTY_MSG 0x004E898B) # symbol ?SendCreateNewPartyMsg@CField@@QAEXXZ (no v61 IDB symbol; was sub_4E898B, labeled this task); push 70h->COutPacket(0x70)(0x5ffc4f)+Encode1(1=create)(0x456cf5)+SendPacket(0x474125); reads g_pWvsContext(0x974EF8); nullary, no EncodeStr (vs join Encode1(4)+EncodeStr; vs adjacent leave sub_4E8A90 Encode1(2)+Encode1(0)). OPCODE DRIFT 0x7A(v72)->0x70(v61). RELOCATED from v72 0x5142B0. needs-main-review
set(C_FIELD_SEND_CREATE_NEW_PARTY_MSG_OFFSET 0x8B) # MEASURED v61: level-gate jnb(73 32)@0x4E8A16 (right after cmp al,0Ah); delta from base 0x4E898B (DRIFT vs v72 0x9E; same level-gate jnb instr; re-measured, NOT copied)

set(C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST 0x00839B94) # symbol ?SendMigrateToITCRequest@CWvsContext@@QAEXXZ; "...Guest ID Users." string(aTheMaplestoryT 0x96974c)@0x839BC0 CHATLOG_ADD guest-ID early-out + push 87h->COutPacket(0x87)(0x5ffc4f)+SendPacket(0x474125) reading g_pClientSocketInstance(0x975054); ITC gate get_field()+0x108>>4&1. OPCODE DRIFT 0x9A(v72)->0x87(v61), read not assumed. RELOCATED from v72 0x90C9BD. needs-main-review
set(C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST_OFFSET 0xE2) # MEASURED v61: ITC-gate jz(74 23)@0x839C76 (after shr eax,4; and eax,1; jz to send path loc_839C9B); delta from base 0x839B94 (DRIFT vs v72 0xE9; same ITC-gate jz instr; re-measured at byte level, NOT copied)

set(DR_CHECK 0x00000000) # CONFIRMED absent in v61: DR/anti-debug subsystem not present (Task 2 R11: SetUp(0x823175) has no DR_init step; anti-tamper is ONLY CSecurityClient). Re-confirmed Task 10: no NtGetContextThread/GetThreadContext/*ContextThread* import (imports_query -> empty). Confirmed absent (SP-5)
set(DR_INIT 0x00000000) # CONFIRMED absent in v61: DR subsystem absent (Task 2 R11: SetUp(0x823175) anti-tamper is only CSecurityClient CreateInstance+InitModule — even cleaner than v72, no meteora/ehsvc/IAT-clone, no DR_init). Re-confirmed Task 10 (no NtGetContextThread import). v84 in-field freeze class cannot occur in v61. Confirmed absent (SP-5)
set(CE_TRACER_RUN 0x00000000) # CONFIRMED absent in v61: CeTracer (AhnLab eTracer) is a v95+ feature; no "eTracer"/"CeTracer" string in v61 (find_regex -> 0). Confirmed absent (SP-5)
set(SEND_HS_LOG 0x00000000) # ABSENT in v61 (carried sentinel; already absent v72, was real 0x0093F8E0 in v79). No SendHSLog symbol; the AhnLab HShield report log post-dates v61. v61 SetUp(0x823175) has no HShield/EHSvc strings — anti-tamper is only CSecurityClient. FLAG gate/edit owner: consuming edit must tolerate 0

set(C_MOB_C_MOB 0x005C2128) # symbol ??0CMob@@QAE@PAVCMobTemplate@@@Z (sole caller CreateMob 0x5C20EC, ZAllocEx::Alloc(0x490=1168) non-zeroing); 3 vtables (off_8E9AE8/8E9AC4/8E9AC0) + m_pTemplate@this+0x15C + 31/100/24 fuse + _ZtlSecureTear chain + MobStat::SetFrom(this+368) + StringPool(942)/IWzCanvas tail@this+0x45C. needs-main-review. RELOCATED from v72 0x611CDB; alloc 1168 (v72 1216); StringPool 942 (v72 958). DOOM (Task 15): m_bDoomReserved FIELD DOES NOT EXIST in v61 (struct only 0x490, even smaller than v72 0x4C0; v84 doom field at 0x540/0x544 is past struct end; highest ctor write this+0x47C, IWzCanvas tail this+0x45C; NO doom write) -> v61 on doom-fix (<84) needs-fix side, same as v72/v79

set(C_SECURITY_CLIENT_ON_PACKET_RET_STUB 0x00000000) # JMS only — JMS185 in-place RET stub (positive re-confirmed real @0xB3B96B: CSecurityClient Aossdk_GetMkdS4Object security-init, Task 10 lane 13342); GMS v61 hooks CSecurityClient::OnPacket via C_SECURITY_CLIENT_ON_PACKET (0x8637C4, GMS Decode1->0/2/3 form). v61 has NO Aossdk/MkdS4 (find_regex -> only generic RTTI desc). Absent in GMS v61 by design (SP-5)
set(C_SECURITY_CLIENT_ON_PACKET_CHECK 0x00000000) # JMS only — JMS185 integrity-check fn re-confirmed real @0xB3B5F7 (CSecurityClient (this,u16,COutPacket*): `if(pData%31)` + CRC32 + Init(0x10)/Encode1(1)/Encode4, Task 10 lane 13342); no GMS counterpart (v61 OnPacket has no %31 check). Absent in GMS v61 by design (SP-5)
set(C_SECURITY_CLIENT_ON_PACKET_CHECK_OFFSET 0x00000000) # JMS only — JMS185 CHECK+0x19 jz patch offset; no GMS counterpart. Absent in GMS v61 by design (SP-5)
set(C_WVS_APP_INITIALIZE_GR2D_WINDOWED_OFFSET 0x00000000) # JMS only — JMS185 windowed-mode in-place patch offset (0x94); GMS v61 forces windowed via C_CONFIG_SYS_OPT_WINDOWED_MODE (0x978E24). Absent in GMS v61 by design (SP-5)
set(WIN_MAIN_LAUNCHER_STUB 0x00000000) # JMS only — JMS185 StartUpDlgClass window proc re-confirmed real @0x7F3CE0 (Task 10 lane 13342); GMS v61 disables patcher via WIN_MAIN_PATCHER_OFFSET NOP. Absent in GMS v61 by design (SP-5)

# --- Faithful client exception dispatch (docs/tasks/exception-dispatch-cleanup) ---
# v61 __TI globals confirmed by IDB RTTI symbol + DOUBLE throw-site anchor:
#   CWvsApp::Run (0x8233CC) exception-dispatch block (0x823558-0x8235e9) AND OnConnect (0x472d42) handshake throws.
set(C_TI_DISCONNECT_EXCEPTION 0x00900678) # __TI3?AVCDisconnectException@@; _CxxThrowException Run @0x8235af (0x21000000-0x21000006) + OnConnect @0x472db5. RELOCATED v61 (was v72 0x9E34C0)
set(C_TI_TERMINATE_EXCEPTION  0x008F4240) # __TI3?AVCTerminateException@@; _CxxThrowException Run @0x8235d8 (0x22000000-0x2200000B) + OnConnect @0x472d95/0x472fc9/0x473023. RELOCATED v61 (was v72 0x9DF8C8)
set(C_TI_PATCH_EXCEPTION       0x00900668) # __TI3?AVCPatchException@@; _CxxThrowException Run @0x823586 (==0x20000000) + OnConnect @0x473004. RELOCATED v61 (was v72 0x9ECC20)
set(C_TI_ZEXCEPTION            0x008F6B60) # __TI1?AVZException@@; default _CxxThrowException Run @0x8235e9 + OnConnect ZException underrun throws (@0x472ee7/0x472f41/0x472f62/0x472f84). RELOCATED v61 (was v72 0x9E0048)
set(C_PATCH_EXCEPTION_BUILDER  0x004DC6E4) # CPatchException_Build (labeled this task; thiscall builder); *this=0x20000000, *(this+4)=61 (major DRIFT vs v72 72 / v79 79 / v83 83), memset 0x504; reached from Run @0x823568 AND OnConnect @0x472fe5, each before qmemcpy 1288 + CPatch throw. RELOCATED v61 (was v72 0x4FEEB7)
set(C_COM_RAISE_ERROR_EX       0x004031B7) # ?_com_issue_error@@YGXJ@Z (1-arg HRESULT raiser); CONFIRMED v61 by symbol + HRESULT-classification body (cmp di,3/0Ah/0Bh; push 80070057h). Run's discrete com call is the 2-arg _com_raise_error(hr,0) @0x823465 (?_com_raise_error@@ 0x875f0a); this 1-arg form is the structural/semantic FAILED-render analog. DRIFT v61 +2 (was v72 0x4031B5)

# v61 CFileStream relay RECOVERABLE: OnConnect (0x472d42) decompiles CLEAN (not CFG-obfuscated like v83),
# so the CLIENT_START_ERROR report-read helpers resolve as real addresses (decision FR-8a). RESOLVED=1.
# Backward-sentinel watch (R7): the relay feature is PRESENT in v61 (not a v72-era addition) — relay stays enabled, no flag.
set(C_FILE_STREAM_RESOLVED     1)          # RECOVERABLE in v61 (clean OnConnect); relay enabled. Same disposition as v72/v79
set(C_FILE_STREAM_OPEN_INLINE  0)          # out-of-line Open exists (CFileStream_Open 0x4734A3); CreateFileA NOT inlined into OnConnect. Same as v72/v79
set(C_FILE_STREAM_OPEN         0x004734A3) # CFileStream_Open (labeled this task); OnConnect call sub_4734A3(name,3,128,1,0x80000000,0,0) = OPEN_EXISTING/FILE_ATTRIBUTE_NORMAL/share=1/GENERIC_READ @0x4730c8; first Close-resets then CreateFileA via cloned slot dword_977F0C, store handle this[4], OR open-flags this[13] (|1 open, |2 on GENERIC_READ 0x40000000). RELOCATED v61 (was v72 0x485A2A)
set(C_FILE_STREAM_GET_LENGTH   0x0047362C) # CFileStream_GetLength (labeled this task); thiscall wrapper dispatching object vtable[+60]; OnConnect @0x4730d3. RELOCATED v61 (was v72 0x485BB3)
set(C_FILE_STREAM_READ         0x00473757) # CFileStream_Read(this,dst,len) (labeled this task); memcpy from mapped view OR ReadFile via cloned slot dword_977F10 on unmapped path; OnConnect @0x4730fd. RELOCATED v61 (was v72 0x485CDE)
set(C_FILE_STREAM_CLOSE        0x00473445) # CFileStream_Close (labeled this task); CloseHandle via cloned slot dword_977E48 + handle this[4]=-1, this[13]=0 (also dtor body); OnConnect @0x473108/0x473126. RELOCATED v61 (was v72 0x4859CC)
set(C_FILE_STREAM_VFTABLE      0x008E66BC) # CFileStream_vftable (off_8E66BC, labeled this task); installed at v32[0] in OnConnect(0x472d42) report-read path @0x47308d/0x473116. RELOCATED v61 (was v72 0x9D0914)
