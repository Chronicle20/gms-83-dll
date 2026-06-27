# GMS v72.1 — Signature / Pattern Catalog

A durable, reusable record of how each non-trivial function/global was located in the v72
binary. v72 is not the last version we will port this way, so each entry documents the
heuristic and its robustness across the v83 → v79 → v72 chain. This catalog both **extends**
task-008's v79 catalog and **consumes** it: for each function, start from the v79 heuristic
(`docs/tasks/task-008-gms-v79-support/signature-catalog.md`) and note whether it ported
directly or drifted in the older build.

## How to use this catalog

- One entry per resolved function/global backing a memory-map key (and per non-trivial
  struct-verification anchor).
- Record the **v72 address**, the **heuristic** used, the **v79 starting point**
  (task-008's entry), and a **drift note** (did the string/constant/codegen survive v79→v72?).
- Prefer heuristics in this robustness order: string xref > import/API anchor > call-graph
  anchor > constant/opcode > byte signature.

## Entry template

```
### <CanonicalName> (key: <MEMORY_MAP_KEY>)
- v72 address: 0x________
- v79 address (task-008): 0x________
- Heuristic: <string xref "…" / import anchor / call-graph child of … / push <imm> / byte sig>
- Drift v79→v72: <direct | string changed to "…" | inlined differently | split | not found, see note>
- Label applied: <yes/no> (rename + set_type if proto known)
- Notes: <anything that will help the next, older port>
```

## v72 IDB identity (confirmed at task-1 baseline)

| Field | Value |
|-------|-------|
| Module | `GMS_v72.1_U_DEVM.exe` |
| IDB path | `E:\Programs\Nexon\IDBs_v9\GMS\v72\GMS_v72.1_U_DEVM.exe.i64` |
| IDA port | 13343 |
| Image base | `0x400000` |
| Image size | `0x86f000` |
| MD5 | `05a62ca755b1d3719223426b4eee41a9` |
| SHA256 | `a989875b85668cf1d62ad4eede948da0357b36075740a1db1f6255c943281a96` |
| Total functions | 44,435 (named: 2,289; library: 503; unnamed: 41,643) — sparse/unverified IDB |
| `.text` segment | `0x401000`–`0x9cf000` (`0x5ce000` bytes, rx) |
| `.data` segment | `0xa59000`–`0xaae000` (`0x55000` bytes, rw) |
| Entry `start` | `0x955da3` |

WinMain offset math anchor: image base `0x400000`. All v72 addresses are in the `0x40xxxx`–`0x9cxxxx` range. Baseline IDB saved at task-1 start (no labels applied yet).

## Catalog (fill during the port, grouped by subsystem)

### CWvsApp lifecycle
_(entries: C_WVS_APP_RUN, _INITIALIZE_*, _SET_UP, window/dir helpers, …)_

> **Task 2 key finding:** the v72 IDB retains the **full mangled C++ symbols** for the
> entire CWvsApp class, CWndMan s_Update/RedrawInvalidatedWindows, WinMain, the exception
> type-info, and CFuncKeyMappedMan — exactly like the v79 IDB. So the v79 string/call-graph
> heuristics all ported directly, and the surviving IDB symbol is the fastest primary anchor;
> each entry below still records a second *structural* anchor (two-anchor rule). v72 image
> base 0x400000. The v72 cluster is a near-verbatim relocation of v79's (same SetUp init
> order, same reduced FuncKeyMappedMan size) — the only divergences are SEND_HS_LOG (absent
> in v72) and the ad-balloon dims/offset drift.

### WinMain   (key: WIN_MAIN)   [HIGH-VALUE / needs-main-review]
- v72 address: 0x008EF5AD (size 0xA0A)
- v79 address (task-008): 0x0093F9B7
- Heuristic: IDB symbol `_WinMain@16`; two structural anchors — (1) string xrefs to the two unique startup literals `\npkgameuninstnomsg.exe` (aNpkgameuninstn @0xA9A14C) and `MapleStoryGlobal :: MapleStory - Microsoft Internet Explorer` (aMaplestoryglob @0xA9A0D4), both referenced in-body; (2) the ctor->SetUp->Run call chain (calls 0x8F26C7 / 0x8F2A7D / 0x8F2F82). v79→v83 chain: same literals + chain in v79 (0x93F9B7) and v83.
- Drift v79→v72: direct (both literals present + identical). Ad-balloon block dims drifted (490/190/60 vs v79 740/300/60) — see offsets entry.
- Label applied: yes (pre-named `_WinMain@16`)
- Spot-check: xrefs_to WinMain → sole caller is the PE entry `start` (0x955DA3) — independent call-graph anchor, holds.

### SendHSLog   (key: SEND_HS_LOG)   — ABSENT / NEW v72 SENTINEL
- v72 address: 0x00000000 (was real 0x0093F8E0 in v79)
- v79 address (task-008): 0x0093F8E0
- Heuristic: confirmed-absent (SP-5 backward direction). v79 SendHSLog = sprintf "%s\HShield" + "MapleStory_Global:%s" -> AhnLab EHSvc.dll ordinal-10 report thunk (sub_9A7906, `%s\EHSvc.dll` + `/ec:%08x /gc:%08x /id:%s /se:%d`), called 3× from WinMain. In v72 ALL of these are absent: no `%s\HShield`/`MapleStory_Global`/`%s\EHSvc.dll`/`/ec:` strings; the only `\HShield` strings (0xA9AA74 / `\HShield\EHSvc.dll` 0xA9AA60) are referenced solely by CSecurityClient::InitModule (0x941ED9), not a logger; and CConfig::GetSessionCharacterName (0x89065E) has only two v72 callers (the crash-report writer sub_89059C @ "ver.%d CharacterName..." + ZtlExceptionHandler) — no SendHSLog. v72 HackShield init is folded into CSecurityClient::InitModule.
- Drift v79→v72: not found — feature post-dates v72.
- Label applied: n/a
- Notes: **FLAG for gate/edit owner** — the consuming edit must tolerate SEND_HS_LOG==0.

### CWvsApp::CWvsApp (ctor)   (key: C_WVS_APP)   [HIGH-VALUE / needs-main-review]
- v72 address: 0x008F26C7 (size 0x353)
- v79 address (task-008): 0x00942D3B
- Heuristic: IDB symbol `??0CWvsApp@@QAE@PBD@Z`; two structural anchors — (1) command-line keyword string xrefs `WebStart`(0xA9A1A0)/`GameLaunching`(0xA9A188)/`token`(0xA9A198) + `kernel32.dll`/`IsWow64Process` probe; (2) call-graph (called from WinMain right before SetUp). Installs vtable off_9DB2E0; SBB-stores the singleton g_CWvsApp (0xA9F658); writes g_dwTargetOS=1996 on OS major<5 / WoW64; ResetLSP (0x449DC1) tail call. v79→v83 chain holds.
- Drift v79→v72: direct (idioms identical). ResetLSP IS present in v72 (same as v79).
- Label applied: yes (symbol)
- Spot-check: xrefs_to `WebStart` (0xA9A1A0) → sole referrer is this ctor — independent of the symbol + WinMain edge.

### CWvsApp::SetUp   (key: C_WVS_APP_SET_UP)   [HIGH-VALUE / needs-main-review]
- v72 address: 0x008F2A7D (size 0x505)
- v79 address (task-008): 0x009430F1
- Heuristic: IDB symbol `?SetUp@CWvsApp@@QAEXXZ`; two structural anchors — (1) string xrefs `Global\meteora`(strcpy)+`ehsvc.dll`(0xA9A1B0)+`ws2_32.dll`/`WSAStartup`/`getpeername`; (2) call-graph (called from WinMain after ctor; walks the whole Initialize*/Create* cluster). v72 NOT virtualized (clean body, like v79). v79→v83 chain: v83 SetUp is CFG-virtualized, v72/v79 clean — anchor on call-graph + meteora/ehsvc.
- **R12 / init-call order (v72):** srand → GetSEPrivilege → GetTickCount → WS2_32 IAT-clone memcpy(0x200) → InitSafeDll loop → WSAStartup/getpeername integrity → HideDll loop(6×) → getpeername IAT memcmp(unk_A9A1BC) → CSecurityClient CreateInstance/InitModule/StartModule → InitializePCOM → CreateMainWindow → CClientSocket::CreateInstance → ConnectLogin → CFuncKeyMappedMan/CQuickslotKeyMappedMan CreateInstance → InitializeResMan → InitializeGr2D → InitializeInput → InitializeSound → InitializeGameData → CreateWndManager → CConfig::ApplySysOpt → CActionMan/CAnimationDisplayer/CMapleTVMan/CQuestMan/CMonsterBookMan inits → Global\meteora + ehsvc + IAT-clone anti-tamper → CheckExecPathReg → CLogo ctor → set_stage. **NO InitializeAuth (NMCO absent). NO DR/anti-debug DR_init step** — anti-tamper is only CSecurityClient + meteora + ehsvc + IAT-clone, no NtGetContextThread. Confirms v72 lacks the DR subsystem (feeds Task 10). Identical in shape to v79.
- Drift v79→v72: direct.
- Label applied: yes (symbol)
- Spot-check: xrefs_to `ehsvc.dll` (0xA9A1B0) → sole referrer is SetUp — independent of the symbol + WinMain edge.

### CWvsApp::Run   (key: C_WVS_APP_RUN)   [HIGH-VALUE / needs-main-review]
- v72 address: 0x008F2F82 (size 0x418)
- v79 address (task-008): 0x00943611
- Heuristic: IDB symbol `?Run@CWvsApp@@QAEXPAH@Z`; two structural anchors — (1) the four exception type-info refs `__TI3?AVCPatchException@@`(0x9ECC20) / `...CDisconnectException@@`(0x9E34C0) / `...CTerminateException@@`(0x9DF8C8) / `__TI1?AVZException@@`(0x9E0048), a distinctive quad thrown via _CxxThrowException on codes 0x20000000 / 0x21000000-6 / 0x22000000-B; (2) the CallUpdate(0x8F4991)/RedrawInvalidatedWindows(0x8E2AF7) callee pair + ISMsgProc dispatch. Loop exits on msg type 18 (WM_QUIT). v79→v83 chain holds.
- Drift v79→v72: direct.
- Label applied: yes (symbol)
- Spot-check: xrefs_to CPatchException-TI (0x9ECC20) → Run (+ OnConnect, a distinct consumer) — independent of the symbol + WinMain edge.

### CWvsApp::ISMsgProc   (key: C_WVS_APP_IS_MSG_PROC)
- v72 address: 0x008F57AB (size 0x51)
- v79 address (task-008): 0x00946430
- Heuristic: IDB symbol `?ISMsgProc@CWvsApp@@IAEXIIJ@Z`; second anchor call-graph (called per-message from Run, both the <=2 and >3 message branches).
- Drift v79→v72: direct.
- Label applied: yes (symbol)

### CWvsApp Initialize*/Create*/ConnectLogin (keys: C_WVS_APP_INITIALIZE_PCOM/_CREATE_MAIN_WINDOW/_CONNECT_LOGIN/_INITIALIZE_RES_MAN/_INITIALIZE_GR2D/_INITIALIZE_INPUT/_INITIALIZE_SOUND/_INITIALIZE_GAME_DATA/_CREATE_WND_MANAGER)
- v72 addresses: InitializePCOM 0x008F3735, CreateMainWindow 0x008F3755, ConnectLogin 0x008F38E5, InitializeResMan 0x008F3A55, InitializeGr2D 0x008F432B, InitializeInput 0x008F45D1, InitializeSound 0x008F493B, InitializeGameData 0x008F4E13, CreateWndManager 0x008F39F2
- v79 addresses (task-008): 0x0094409B / 0x009440BB / 0x0094424B / 0x009443BB / 0x00944C91 / 0x00944F37 / 0x009452A1 / 0x00945834 / 0x00944358
- Heuristic: each carries its IDB mangled symbol (`?InitializePCOM@CWvsApp@@IAEXXZ`, etc.); second anchor = the SetUp call-graph (anchor 1) corroborated by each function's own distinctive content (ResMan: WZ data-type string list + Base.wz; Gr2D: 800/600 + 0xFF000000; Input: ws2_32/getpeername IAT scan; Sound: CSoundMan::Init(30000,2,2,16); GameData: alloc/validate + CTerminateException 0x22000006; CreateMainWindow: CreateWindowExA + style branch; CreateWndManager: CWndMan singleton store).
- Drift v79→v72: direct (all present, same structure).
- Label applied: yes (symbols)

### CWvsApp::GetCmdLine / Dir_BackSlashToSlash / Dir_upDir / Dir_SlashToBackSlash / GetExceptionFileName / CallUpdate
- v72 addresses: GetCmdLine 0x008F5495, Dir_BackSlashToSlash 0x008F55F2, Dir_SlashToBackSlash 0x008F5615, Dir_upDir 0x008F5638, GetExceptionFileName 0x008F57FC, CallUpdate 0x008F4991
- v79 addresses (task-008): 0x0094611A / 0x00946277 / 0x0094629A / 0x009462BD / 0x00946481 / 0x009454B5
- Heuristic: IDB mangled symbols. Second anchors — GetCmdLine: call-graph from ctor (WebStart/token/GameLaunching parse); the three Dir_* statics: char-swap '\\'<->'/' loops, all three called from SetUp on the exec-path (BackSlashToSlash->upDir->SlashToBackSlash); GetExceptionFileName: called from top of WinMain (static-cache); CallUpdate: 30ms frame step + sole CWndMan::s_Update caller + getpeername heartbeat + 3N.mhe/v3warp ehsvc integrity, called from Run.
- Drift v79→v72: direct. v72 Dir_* address order is BackSlashToSlash < SlashToBackSlash < upDir (same as v79).
- Label applied: yes (symbols)

### CWndMan::s_Update / RedrawInvalidatedWindows   (keys: C_WND_MAN_S_UPDATE / C_WND_MAN_REDRAW_INVALIDATED_WINDOWS)
- v72 addresses: s_Update 0x008E2D73, RedrawInvalidatedWindows 0x008E2AF7
- v79 addresses (task-008): 0x00932EE2 / 0x00932C66
- Heuristic: IDB symbols `?s_Update@CWndMan@@SAXXZ` / `?RedrawInvalidatedWindows@CWndMan@@SAXXZ`; second anchor call-graph — s_Update is the sole callee of interest inside CallUpdate; RedrawInvalidatedWindows is called from Run right after CallUpdate.
- Drift v79→v72: direct.
- Label applied: yes (symbols)

### globals: g_CWvsApp / g_dwTargetOS   (keys: C_WVS_APP_INSTANCE_ADDR / G_DW_TARGET_OS)
- v72 addresses: C_WVS_APP_INSTANCE_ADDR 0x00A9F658, G_DW_TARGET_OS 0x00A9A164
- v79 addresses (task-008): 0x00B07A68 / 0x00B0239C
- Heuristic: located via their writer, the CWvsApp ctor (0x8F26C7). g_CWvsApp = the `(a1+1!=0)?a1:0` SBB store at the top of the ctor (xrefs_to confirms ctor is the writer + ConnectLogin/login-flow readers). g_dwTargetOS = the DWORD set to 1996 (0x7CC) on OS major<5 (OSVERSIONINFOEX cb=148) OR IsWow64Process true.
- Drift v79→v72: address relocated (both in .data, drifted down ~0x6E000); writer idioms identical.
- Label applied: yes (renamed g_CWvsApp / g_dwTargetOS in v72 IDB)

### CWvsApp::InitializeAuth   (key: C_WVS_APP_INITIALIZE_AUTH) — ABSENT (confirmed, like v79)
- v72 address: 0x00000000 (sentinel)
- Heuristic: confirmed-absent. SetUp makes no auth call; no CNMCOClientObject/CNMManager. The NMCO/Nexon-Passport auth subsystem post-dates v72 (same verdict as v79).
- Notes: already a sentinel in the v79 seed; carried forward, comment updated to v72.

### CClientSocket / ZSocket
_(entries: instance addr, CreateInstance, SendPacket, Flush, OnConnect, Process, connect path)_

> **Task 4 key finding:** the v72 IDB retains the **full mangled C++ symbols** for the
> entire socket cluster (`?SendPacket@CClientSocket@@…`, `?OnConnect@CClientSocket@@…`,
> etc.) — same as v79 — so each function's primary anchor is its surviving IDB symbol;
> a second *structural* anchor is recorded per the two-anchor rule. v72 routes
> send/recv/connect/WSAAsyncSelect through **cloned import slots** (anti-tamper, same idiom
> as v79/v83): dword_AA7874=send, dword_AA787C=recv, dword_AA7854=connect,
> dword_AA7834=WSAAsyncSelect, dword_AA7848=WSAGetLastError, dword_AA74FC=Sleep,
> dword_AA787C(recv). Anchor on the surviving `socket`/`closesocket`/`getpeername` *imports*
> (0x9cf35c / 0x9cf344 / 0x9cf360), not the cloned slots. **Protocol version drift: v72=72**
> — SendPacket MakeBufferList immediate 72, OnConnect major-version compare 0x48, the
> ManipulatePacket version/seq XOR check `!= -73` (~72). **One backward divergence:**
> ZSocketBase::CloseSocket is *inlined* in v72 (no standalone fn; shutdown not imported).

### g_pClientSocketInstance (key: C_CLIENT_SOCKET_INSTANCE_ADDR)
- v72 address: 0x00A9F434
- v79 address (task-008): 0x00B07844
- Heuristic: writer = CClientSocket ctor (0x484c95) SBB-singleton store `g = (this+4!=0)?this:0` at top; also stored/read by CreateInstance (0x8f621f). Reader set = the whole send subsystem (CField::Send*, CCashShop senders, free SendPacket helpers). Two anchors: ctor writer idiom + the broad reader fan-in.
- Drift v79→v72: address relocated (.data, down ~0x68000); writer idiom identical. CWvsContext singleton is +4 = 0xA9F438 (cross-check for C_WVS_CONTEXT_INSTANCE_ADDR).
- Label applied: yes (renamed g_pClientSocketInstance)

### TSingleton<CClientSocket>::CreateInstance (key: C_CLIENT_SOCKET_CREATE_INSTANCE)
- v72 address: 0x008F621F
- v79 address (task-008): 0x00946AB6
- Heuristic: IDB symbol `?CreateInstance@?$TSingleton@VCClientSocket@@@@SAPAVCClientSocket@@XZ`; second anchor = ZAllocEx::Alloc(148=0x94) + ctor(0x484c95) + store to g_pClientSocketInstance (0xA9F434).
- Drift v79→v72: direct (alloc size 148 identical).
- Label applied: yes (symbol)

### CClientSocket::SendPacket (key: C_CLIENT_SOCKET_SEND_PACKET)   [HIGH-VALUE / needs-main-review]
- v72 address: 0x004866AC (size 0x88)
- v79 address (task-008): 0x0048DF93
- Heuristic: IDB symbol `?SendPacket@CClientSocket@@QAEXABVCOutPacket@@@Z`; two structural anchors — (1) the MakeBufferList(this+80, **72**, this+132, 1, m_seqSnd=[this+33])->CIGCipher::innoHash(this+132,4,0)->CClientSocket::Flush callee triple (Flush is the last call); (2) ZSynchronizedHelper<ZFatalSection> lock-ctor at [this+124] + fd([this+8])/send-disabled([this+20]) gate. v79→v83 chain: same triple in v79 (0x48DF93) + v83.
- Drift v79→v72: send-seq immediate **72** (was 79 in v79, 83/84 above). Read it, never copy.
- Label applied: yes (symbol)
- Spot-check: reached via OnConnect's post-handshake send (0x48570b/0x485776) — independent of the symbol.

### CClientSocket::Flush (key: C_CLIENT_SOCKET_FLUSH)   [HIGH-VALUE / needs-main-review]
- v72 address: 0x00486734 (size 0x11A)
- v79 address (task-008): 0x0048E01B
- Heuristic: IDB symbol `?Flush@CClientSocket@@QAEXXZ`; two structural anchors — (1) the send-buffer ZList walk over [this+23] (InterlockedIncrement per node) writing via the cloned send slot dword_AA7874(fd,buf[+16],len[+12],0); (2) it is the tail call inside SendPacket (0x48670d) and the only socket-region fn touching dword_AA7874 + WSAEWOULDBLOCK(10035 via dword_AA7848)->OnError. v79→v83 chain holds.
- Drift v79→v72: direct (cloned send slot relocated dword_B1015C→dword_AA7874).
- Label applied: yes (symbol)
- Spot-check: reached as SendPacket's last call, independent of the symbol.

### CClientSocket::ManipulatePacket (key: C_CLIENT_SOCKET_MANIPULATE_PACKET)   [HIGH-VALUE / needs-main-review]
- v72 address: 0x0048684E (size 0xD4)
- v79 address (task-008): 0x0048E135
- Heuristic: IDB symbol `?ManipulatePacket@CClientSocket@@QAEXXZ`; two structural anchors — (1) `while([this+17])` receive reassembly via CInPacket::AppendBuffer, version/seq check `(word[this+57] ^ HIWORD([this+34])) != -73`, on full packet (ret==2) re-seeds m_seqRcv=[this+34]=innoHash(this+136,4,0); (2) **sole caller** of ProcessPacket (0x4868f5). v79→v83 chain holds.
- Drift v79→v72: version/seq XOR constant **-73** (~72; was -80=~79 in v79).
- Label applied: yes (symbol)
- Spot-check: callers are the FD_READ dispatcher + CWvsApp::Run path — corroborate independent of the symbol.

### CClientSocket::ProcessPacket (key: C_CLIENT_SOCKET_PROCESS_PACKET)   [HIGH-VALUE / needs-main-review]
- v72 address: 0x00486922 (size 0xC0)
- v79 address (task-008): 0x0048E209
- Heuristic: IDB symbol `?ProcessPacket@CClientSocket@@IAEXAAVCInPacket@@@Z`; two structural anchors — (1) reads CStage(dword_AA54D4) + CInPacket::Decode2 opcode then the sub-0x10 switch (0x10 OnMigrateCommand, 0x11 OnAliveReq, 0x12 OnAuthenCodeChanged, 0x13 OnAuthenMessage, 0x14 CSecurityClient::OnPacket(dword_AA3EE4), else 0x1A..0x71 CWvsContext::OnPacket(g_pWvsContext=0xA9F438) else CStage vtable+8); (2) **sole caller is ManipulatePacket**, sole non-trivial callee is Decode2. v79→v83 chain holds.
- Drift v79→v72: CWvsContext opcode range 0x1A..0x71 (was 0x1A..0x75 in v79).
- Label applied: yes (symbol)
- Spot-check: sole caller ManipulatePacket confirms independent of the symbol.

### CClientSocket::OnConnect (key: C_CLIENT_SOCKET_ON_CONNECT)
- v72 address: 0x0048528F (size 0x565)
- v79 address (task-008): 0x0048CB81
- Heuristic: IDB symbol `?OnConnect@CClientSocket@@QAEHH@Z` (int arg = bSuccess); two anchors — (1) `getpeername` import (0x9cf360) xref lands here (kind 2); (2) the only fn calling ZSocketBuffer::Alloc(0x5B4) + cloned recv slot dword_AA787C + the version-header(==8 @0x485520)/major(==0x48 @0x485544) guard trio throwing the CTerminate(0x22000007)/CPatch pair. Tail-calls Connect(v32) on the retry/teardown path.
- Drift v79→v72: major-version compared = **0x48 (72)** (was 0x4F=79 in v79). VERSION_HEADER byte still 8.
- Label applied: yes (symbol)
- Notes: disambiguated from the three Connect* variants — recv-side handler (getpeername + cloned recv + SendPacket driver), takes bSuccess.

### 8-byte client-key finding (feeds Task 13 / §5.6; CWvsContext.h `>83` gate, bypass/socket_hooks.cpp:310-312)
- **Verdict: ABSENT in v72 (no-key form) — confirms the `BUILD_MAJOR_VERSION > 83` gate prediction (v72 < 83 → no key, matching v79).**
- Evidence: OnConnect's post-handshake send has two mutually-exclusive branches on `[this+9]` (m_ctxConnect.bLogin):
  - else / logged-in branch @0x48572a: `COutPacket(20=0x14=PLAYER_LOGGED_IN); Encode4(charId=*(g_pWvsContext+8352)); Encode1(0|1 per TSecType bit *(g_pWvsContext+8252)&0x80); Encode1(0); CClientSocket::SendPacket;` then returns. The encoder sequence is exactly **Encode4/Encode1/Encode1** — there is **no `EncodeBuffer(m_aClientKey, 8)`** anywhere, and CWvsContext has no m_aClientKey member below v83. v72 has neither the 16-byte machineId (v84+) nor any 8-byte client key in this path.
  - bLogin branch @0x4856cd: builds the GetExceptionFileName report (CFileStream read) and `COutPacket(26); Encode2(len); EncodeBuffer(report,len)` — unrelated to a client key.
- Verified against the v72 binary (decompile of OnConnect 0x48528f), not a server round-trip.

### CClientSocket::ConnectLogin (key: C_CLIENT_SOCKET_CONNECT_LOGIN)
- v72 address: 0x00484EA5 (size 0x19D)
- v79 address (task-008): 0x0048C773
- Heuristic: IDB symbol `?ConnectLogin@CClientSocket@@QAEXXZ`; second anchor = GetCmdLine(0)/GetCmdLine(1) (0x8f5495) + rand server-table pick (ZArray<long> shuffle) -> Connect(CONNECTCONTEXT)(0x4850fc); **sole caller is CWvsApp::ConnectLogin (0x8f38e5)**.
- Drift v79→v72: direct. Distinct from CWvsApp::ConnectLogin (the message-pump driver) — this is the CClientSocket method it invokes.
- Label applied: yes (symbol)

### CClientSocket::Connect(CONNECTCONTEXT) (key: C_CLIENT_SOCKET_CONNECT_CTX)
- v72 address: 0x004850FC (size 0x64)
- v79 address (task-008): 0x0048C9CA
- Heuristic: IDB symbol `?Connect@CClientSocket@@QAEXABUCONNECTCONTEXT@1@@Z` (public); second anchor = its only meaningful call is the private Connect(sockaddr_in)(0x485188); callers are ConnectLogin(0x484ea5) + CWvsContext::IssueConnect(0x8fea69).
- Drift v79→v72: direct.
- Label applied: yes (symbol)
- Notes: the CONNECTCONTEXT overload that *calls* CONNECT_ADR, not a `socket` caller.

### CClientSocket::Connect(sockaddr_in) (key: C_CLIENT_SOCKET_CONNECT_ADR)
- v72 address: 0x00485188 (size 0x107)
- v79 address (task-008): 0x0048CA56
- Heuristic: IDB symbol `?Connect@CClientSocket@@IAEXPBUsockaddr_in@@@Z` (private) — the **sole `socket` import (0x9cf35c) caller** (`socket(2,1,0)`=AF_INET/SOCK_STREAM); second anchor = inline ClearSendReceiveCtx + closesocket teardown, then WSAAsyncSelect via cloned dword_AA7834(s,hwnd,1025,51) then cloned connect dword_AA7854(s,addr,16) tolerating WSAEWOULDBLOCK(10035), OnConnect(this,0) on the synchronous paths.
- Drift v79→v72: direct; the `socket` import xref is the most stable anchor; connect/select indirected through cloned slots in v72 just like v79/v83. The close step is now inlined (was a ZSocketBase::CloseSocket call in v79).
- Label applied: yes (symbol)

### ZSocketBase::CloseSocket (key: Z_SOCKET_BASE_CLOSE_SOCKET) — INLINED / NEW v72 SENTINEL
- v72 address: 0x00000000 (was real 0x0048C699 in v79)
- v79 address (task-008): 0x0048C699
- Heuristic: confirmed-inlined (SP-5 backward direction). v79 had a standalone `?CloseSocket@ZSocketBase@@QAEXXZ` (shutdown(s,2)+closesocket+ -1 sentinel, size 0x20) called by Close + Connect(sockaddr_in). In v72 there is **no standalone function**: the close teardown is inlined into CClientSocket::Close(0x48668f), Connect(sockaddr_in)(0x485188) and ~CClientSocket(0x484e0b), and `shutdown` is **not imported at all** (v72 ws2_32 imports lack shutdown → the v72 inline close is closesocket-only). The `closesocket` import (0x9cf344) has exactly 4 callers (ctor-EH, dtor, Connect, Close), none a standalone CloseSocket; the closesocket thunk (0x952570) has 0 callers.
- Drift v79→v72: not found as a discrete function — inlined; behavior also dropped the shutdown(s,2) step.
- Label applied: n/a
- Notes: **FLAG for gate/edit owner** — bypass/socket_hooks.cpp:324 (`pThis->m_sock.CloseSocket()` inside CClientSocket__Connect_Addr_Hook) calls Z_SOCKET_BASE_CLOSE_SOCKET as a function pointer. For v72 the consuming hook must inline the close (closesocket(m_sock._m_hSocket); _m_hSocket=-1) instead of calling 0, or its Connect-Addr reimplementation must be skipped for v72. The consuming edit must tolerate Z_SOCKET_BASE_CLOSE_SOCKET==0.

### ZSocketBuffer::Alloc (key: Z_SOCKET_BUFFER_ALLOC)
- v72 address: 0x004862F8 (size 0x4D)
- v79 address (task-008): 0x0048DBEA
- Heuristic: IDB symbol `?Alloc@ZSocketBuffer@@SAPAV1@IABVZAllocHelper@@@Z`; second anchor = static factory doing TWO ZAllocEx<ZAllocAnonSelector>::Alloc calls (payload size a1, then the placement ctor sub_486345) and called by OnConnect with the 0x5B4 receive-buffer size.
- Drift v79→v72: direct (the dual-alloc + 0x5B4 OnConnect caller idiom holds).
- Label applied: yes (symbol)

### CClientSocket::Close (key: C_CLIENT_SOCKET_CLOSE)
- v72 address: 0x0048668F (size 0x1D)
- v79 address (task-008): 0x0048DF81
- Heuristic: IDB symbol `?Close@CClientSocket@@QAEXXZ`; second anchor = the 2-step body `ClearSendReceiveCtx(this); if([this+8]!=-1){closesocket([this+8]); [this+8]=-1;}` — the close is now **inlined** (v79 called ZSocketBase::CloseSocket). Also called at the top of OnConnect's failure path.
- Drift v79→v72: ZSocketBase::CloseSocket inlined (see that key); otherwise direct.
- Label applied: yes (symbol)

### CClientSocket::ClearSendReceiveCtx (key: C_CLIENT_SOCKET_CLEAR_SEND_RECEIVE_CTX)
- v72 address: 0x00486CF0 (size 0x21)
- v79 address (task-008): 0x0048E5D7
- Heuristic: IDB symbol `?ClearSendReceiveCtx@CClientSocket@@IAEXXZ`; second anchor = zeroes [this+26]/[this+56(word)]/[this+30] then resets the recv ([this+60]) and send ([this+80]) ZLists via sub_48716C; called by the ctor, Close, and OnConnect.
- Drift v79→v72: direct.
- Label applied: yes (symbol)

### COutPacket
_(entries: ctor/Init, Encode1/2/4/buffer, …)_

### Login / Stage / Logo / Title
_(entries: CLogin, CStage, CLogo, CUITitle, CUILoginStart, …)_

### Config / Input / FuncKeyMappedMan
_(entries: CConfig ctor + ApplySysOpt, CInputSystem, CFuncKeyMappedMan CreateInstance + ctor)_

### Manager singletons
_(entries: *_CREATE_INSTANCE / *_INSTANCE_ADDR — ActionMan, AnimationDisplayer, etc.)_

### WinMain + offsets
_(entries: WIN_MAIN, AD_BALLOON_CONDITIONAL offset, PATCHER offset, SEND_HS_LOG)_
_(WIN_MAIN + SEND_HS_LOG entries are in the "CWvsApp lifecycle" section above.)_

### WinMain offsets   (keys: WIN_MAIN_AD_BALLOON_CONDITIONAL / WIN_MAIN_PATCHER_OFFSET)
- Host: v72 WinMain 0x008EF5AD. Both re-measured at the byte level (SP-2), never copied.
- **PATCHER_OFFSET = 0x212**: `call ?ShowStartUpWndModal@@YAXXZ` @0x8EF7BF, bytes `E8 97 FD FF FF`; delta from WinMain base 0x8EF5AD. (Coincides with v79 0x212, but re-measured — the no-patcher edit NOPs these 5 bytes.)
- **AD_BALLOON_CONDITIONAL = 0x959**: `jz short loc_8EFF77` @0x8EFF06, bytes `74 6F`, guarding the `if(v47)` ShowADBalloon block (dims **490/190/60** — DRIFT vs v79 740/300/60) immediately followed by the MapleStoryGlobal string push (sub_89090F). Delta from base 0x8EF5AD. **DRIFT vs v79 0xA3D.** The no-ad-balloon edit overwrites the first byte 0x74->0xEB (always skip).
- Drift v79→v72: PATCHER offset coincides (0x212); AD_BALLOON offset drifted (0x959 vs 0xA3D) and the balloon dims drifted. Re-measure every version.

### CFuncKeyMappedMan (Step 5 — class anchor + v72 size, for Task 3)   (keys resolved fully in Task 7)
- v72 ctor `??0CFuncKeyMappedMan@@QAE@XZ` @0x005512EC installs vtable off_9D22D8 (@0x009D22D8); singleton global dword_AA4CB8 (@0x00AA4CB8); CreateInstance (0x008F6264) allocates **904 (0x388)** bytes; default-FKM blob src unk_A5B838.
- **v72 size = 0x388 (904) — IDENTICAL to v79's reduced 0x388.** Confirmed two ways: (1) Alloc(904) in CreateInstance; (2) ctor field-init extent — vtable@0 + memcpy 0x1BD->this+4 + memcpy 0x1BD->this+449 + dwords @+896/+900 → object ends at 904. The quickslot pair is already gated out in v72 just as in v79, so **v72 is on the SAME (reduced) side as v79** for Task 3's `CFuncKeyMappedMan.h:50` amendment (Category-B header). No struct type applied (read-only).
- Drift v79→v72: size direct (0x388 = 0x388). vtable/instance addresses relocated (vtable 0x9D22D8 vs v79 0xA2EB38; instance 0xAA4CB8 vs 0xB0D2A8).

### Party / migrate senders
_(entries: senders + call-site offsets)_

### Utilities
_(entries: ZArray::RemoveAll, ZXString trim/get-buffer, fatal section ctor/dtor, CSystemInfo, CIGCipher)_

### Exception dispatch
_(entries: C_TI_*EXCEPTION, C_PATCH_EXCEPTION_BUILDER, C_COM_RAISE_ERROR_EX, C_FILE_STREAM_*)_

### Protocol constants (FR-6)
_(VERSION_HEADER, PLAYER_LOGGED_IN, CLIENT_START_ERROR — record the deciding v72 disasm
site, e.g. the OnConnect `if (hdr != N)` compare and the COutPacket opcode immediates;
note any drift from v79's confirmed VERSION_HEADER=8)_

### Sentinels (confirmed absent in v72)
_(DR_CHECK, DR_INIT, CE_TRACER_RUN, C_BATTLE_RECORD_MAN_CREATE_INSTANCE, JMS-only keys, and
any new v72-only sentinels — record the evidence of absence, not just the `0x0`)_

## Cross-version drift summary (fill at end)

A short table of which heuristic classes survived v79→v72 best, to guide the next (older)
port: e.g. "string xrefs: N/N held; byte sigs: M/K held; constants: …". This is the durable
takeaway that makes the v6x port faster.
</content>
