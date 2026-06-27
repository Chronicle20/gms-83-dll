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

### COutPacket encode cluster (Task 5)

> All 7 keys located by surviving IDB mangled symbol (kind 1) + an independent structural
> anchor (kinds 2–4), with the v79→v83 chain confirmed (each fn present at the v79 task-008
> address AND the v83 cmake address with matching name+size). All HIGH-VALUE / needs-main-review.
> Function sizes are identical v83↔v72 (ctor 0x46, Encode1 0x1e, Encode2 0x21, Encode4 0x1f,
> EncodeStr 0x66, EncodeBuffer 0x2a, MakeBufferList 0x342) — strong corroboration the chain holds.

### COutPacket::COutPacket (ctor)   (key: C_OUT_PACKET)   [HIGH-VALUE / needs-main-review]
- v72 address: 0x00656FA1 (size 0x46)
- v79 address (task-008): 0x0067AD6B  |  v83 (cmake): 0x006EC9CE (size 0x46)
- Heuristic: IDB symbol `??0COutPacket@@QAE@J@Z`; structural anchors — (1) `and dword [esi+4],0` (zero buf member) + `push 100h` (256-cap _Alloc via helper sub_486FE4 with `lea ecx,[esi+4]`); (2) tail `call ?Init@COutPacket@@QAEXJ@Z` (0x65707C). The 256-alloc→Init(seq) shape is the fingerprint. EH-prolog ctor; unwind funclet tail-calls ZArray<uchar>::RemoveAll.
- Drift v79→v72: relocated (0x67AD6B→0x656FA1); the _Alloc helper drifted to sub_486FE4 (v79 sub_48E8CB). Init callee + 0x100 immediate held. Chain confirmed v79+v83.
- Label applied: yes (symbol already present, verified verbatim).
- Notes: spot-check independent of symbol — the `push 100h`→helper + Init tail (size 0x46 identical to v83) located the same fn.

### COutPacket::Encode1 / Encode2 / Encode4   (keys: C_OUT_PACKET_ENCODE_1 / _ENCODE_2 / _ENCODE_4)   [HIGH-VALUE / needs-main-review]
- v72 addresses: Encode1 0x004062C7 (0x1e), Encode2 0x00424F84 (0x21), Encode4 0x00406324 (0x1f)
- v79 addresses (task-008): Encode1 0x004062C7, Encode2 0x0042539C, Encode4 0x00406324  |  v83 (cmake): 0x00406549 / 0x00427F74 / 0x004065A6
- Heuristic: IDB symbols `?Encode1@COutPacket@@QAEXE@Z` / `?Encode2@…@QAEXG@Z` / `?Encode4@…@QAEXK@Z`; structural anchors — (1) **WIDTH DISCRIMINANT, verified not transposed:** Encode1 = `push 1`+`mov [eax+ecx],dl`+`inc dword [esi+8]` (arg _BYTE); Encode2 = `push 2`+`mov [eax+ecx],dx`+`add dword [esi+8],2` (arg _WORD); Encode4 = `push 4`+`mov [eax+ecx],edx`+`add dword [esi+8],4` (arg _DWORD). Push-immediate matches store-width matches advance. (2) call-graph — `xrefs_to _EnsureCapacity` (0x4062E5) returns EXACTLY the 5 siblings {Encode1,Encode2,Encode4,EncodeBuffer,EncodeStr} and nothing else.
- Drift v79→v72: Encode1 & Encode4 DIRECT (== v79 addr, both adjacent at 0x4063xx). Encode2 relocated 0x42539C→0x424F84 (still in the distant 0x42xxxx region — confirm by store width, never by adjacency). Shared _EnsureCapacity callee held. Chain confirmed v79+v83.
- Label applied: yes (symbols already present).
- Notes: spot-check — width store byte read from each body (dl/dx/edx) independent of the mangled name; the trio is the 5-sibling _EnsureCapacity fan-in.

### COutPacket::EncodeStr   (key: C_OUT_PACKET_ENCODE_STR)   [HIGH-VALUE / needs-main-review]
- v72 address: 0x00468295 (size 0x66)
- v79 address (task-008): 0x004694DE  |  v83 (cmake): 0x0046F3CF (size 0x66)
- Heuristic: IDB symbol `?EncodeStr@COutPacket@@QAEXV?$ZXString@D@@@Z`; structural anchors — (1) ZXString length read `mov eax,[eax-4]` (null→`xor eax,eax`) then `add eax,2` grow before `_EnsureCapacity`; (2) dispatches to `CIOBufferManipulator::EncodeStr` (0x4682FB) after a ZXString::operator= temp, `add [esi+8],eax`, ZXString dtor in EH unwind slot. Also in the 5-sibling _EnsureCapacity fan-in.
- Drift v79→v72: relocated 0x4694DE→0x468295; length-prefix-minus-4 + (len+2) idiom + CIOBufferManipulator::EncodeStr callee held. Chain confirmed v79+v83.
- Label applied: yes (symbol already present).
- Notes: spot-check — the `[eax-4]`/`+2` grow + CIOBufferManipulator::EncodeStr dispatch, independent of the symbol.

### COutPacket::EncodeBuffer   (key: C_OUT_PACKET_ENCODE_BUFFER)   [HIGH-VALUE / needs-main-review]
- v72 address: 0x00465CB2 (size 0x2a)
- v79 address (task-008): 0x00466AE9  |  v83 (cmake): 0x0046C00C (size 0x2a)
- Heuristic: IDB symbol `?EncodeBuffer@COutPacket@@QAEXPBXI@Z` (void* PBX form); structural anchors — (1) takes (Src,Size), `_EnsureCapacity(Size)` then `_memcpy(buf+len, Src, Size)` (import 0x952B80) + `add [esi+8],edi` advance + `retn 8` (two stack args); (2) member of the 5-sibling _EnsureCapacity fan-in.
- Drift v79→v72: relocated 0x466AE9→0x465CB2; grow→memcpy→advance triple + 8-byte frame held. Chain confirmed v79+v83.
- Label applied: yes (symbol already present).
- Notes: spot-check — the memcpy-grow triple + retn 8, independent of the symbol.

### COutPacket::MakeBufferList   (key: C_OUT_PACKET_MAKE_BUFFER_LIST)   [HIGH-VALUE / needs-main-review]
- v72 address: 0x006570FA (size 0x342)
- v79 address (task-008): 0x0067AEC4  |  v83 (cmake): 0x006ECB27 (size 0x342)
- Heuristic: IDB symbol `?MakeBufferList@COutPacket@@QAE?AV?$ZRef@VZSocketBuffer@@@@HW4SocketSendFlag@@@Z`; structural anchors — (1) call-graph: it is the **sole MakeBufferList callee of CClientSocket::SendPacket** (0x4866AC → {MakeBufferList 0x6570FA, innoHash 0x940D7E, Flush 0x486734}); (2) constant: `mov edi,5B4h` MTU chunk (`v=1460; if(len<0x5B4) v=len`) @0x6572F0 + the `^0x13` payload shuffle (`xor dl,13h` @0x657242).
- Drift v79→v72: relocated 0x67AEC4→0x6570FA. **The v72 body is control-flow-obfuscated** (opaque-predicate `xchg ebp,ebp`/`jo/jno` jump maze, scattered `ror/rol/add dl/not dl/xor dl,13h`) vs the cleaner v79 body — the 0x5B4 chunk constant and ^0x13 shuffle survive the obfuscation. The per-version shuffle byte is `add dl,48h` (72) here (v79 used 71/0x47). v83 carries a DIFFERENT mangled signature (`…@QBEXAAV?$ZList…@GPAKHK@Z`) — same name/role/size 0x342 (v79-retained `ZRef/SocketSendFlag` form ≠ v83 form; chain held by name+role+size, not signature string).
- Label applied: yes (symbol already present).
- Notes: spot-check — the 1460/0x5B4 chunk constant + ^0x13 shuffle, read independent of both the symbol and the SendPacket call edge.

### Login / Stage / Logo / Title (Task 6)

> **CLogo vtables (v72).** The CLogo ctor (0x5E11F9) writes 4 vtable pointers, same shape as
> v79: [this]=primary IStage vtable `off_9D3B94`, [this+4]=IUIMsgHandler vtable `off_9D3B48`,
> [this+8]=IStage::OnPacket vtable `off_9D3B44`, [this+0Ch]=`off_9D3B40`. Slot orders match
> v79/v83 with **NO drift**: primary {slot0=Update, slot1=Init, slot2=ForcedEnd}; IUIMsgHandler
> {slot0=OnKey, slot1=OnSetFocus, slot2=OnMouseButton}; OnPacket {slot0=CStage::OnPacket}.
>
> **Shared-Update verdict (Step-2 note).** In v72 `C_LOGO_UPDATE` (0x5E1789, CLogo primary slot0)
> and `C_LOGIN_UPDATE` (0x5AFBBE, CLogin primary slot0) are **DISTINCT functions → DIVERGED**,
> same as v79. The v83 coincidence (both = 0x5F4C16) does NOT hold below v83. Verified independently.

### CLogin::Update   (key: C_LOGIN_UPDATE)
- v72 address: 0x005AFBBE  |  v79 (task-008): 0x005CA348
- Heuristic: CLogin primary vtable `off_9D316C` slot0 (CLogin ctor 0x5AECED installs it); body anchor — `cmp [esi+15Ch], ebx` guard + InvalidateRect-style call via `dword_AA7624` func-ptr, reads `g_CWvsApp[+0x18]`.
- Drift v79→v72: relocated; the [esi+0x15C] field-guard held. InvalidateRect is now an **indirect call through dword_AA7624** (v79 called CWnd::InvalidateRect 0x48CCA4 directly). DIVERGES from C_LOGO_UPDATE (separate fn), same as v79.
- Label applied: yes (renamed `CLogin__Update`).

### CLogin::SendCheckPasswordPacket   (key: C_LOGIN_SEND_CHECK_PASSWORD_PACKET)
- v72 address: 0x005B1170  |  v79 (task-008): 0x005CBF50
- Heuristic: IDB symbol `?SendCheckPasswordPacket@CLogin@@QAEHPBD0@Z`; structural anchor — `COutPacket::COutPacket(pkt, <opcode>)` then EncodeStr(user) + EncodeStr(pass) + EncodeBuffer(machineId,16) + Encode4(GameRoomClient) + Encode1×3 + Encode4(GetPartnerCode) + `CClientSocket::SendPacket(g_pClientSocketInstance, pkt)` (= the resolved send 0x4866AC).
- Drift v79→v72: relocated; **opcode immediate is 1 in v72 (was 0x05 in v79)** — the COutPacket ctor seq arg drifted. EncodeStr-pair + SendPacket structure held. Reads the CUITitle singleton dword_AA5114.
- Label applied: yes (symbol).
- Notes: cross-confirms STAGE/Title globals + the socket send key.

### CLogo::CLogo (ctor)   (key: C_LOGO)
- v72 address: 0x005E11F9  |  v79 (task-008): 0x005FF8C4
- Heuristic: IDB symbol `??0CLogo@@QAE@XZ`; structural anchor — 4 consecutive vtable-pointer stores ([this]=off_9D3B94, +4=off_9D3B48, +8=off_9D3B44, +0Ch=off_9D3B40); called from CLogo::LogoEnd via Alloc(0x23C)+ctor.
- Drift v79→v72: relocated; 4-vtable-write shape held. CLogin alloc block in LogoEnd is **0x23C (572)** in v72 vs 0x258 (600) in v79.
- Label applied: yes (symbol).

### CLogo::GetRTTI   (key: C_LOGO_GET_RTTI)
- v72 address: 0x00421565  |  v79 (task-008): 0x0042196A
- Heuristic: IDB symbol `?GetRTTI@CLogo@@UBEPBVCRTTI@@XZ`; structural anchor — two-instruction body `mov eax, offset dword_AA4D2C; retn` returning the CLogo RTTI descriptor dword_AA4D2C (the same descriptor IsKindOf walks).
- Drift v79→v72: relocated; **mangling drift** — v72 uses `CRTTI` + `UBE` (const-this), v79 used `CRuntimeClass` + `UAE`. Same role.
- Label applied: yes (symbol).

### CLogo::IsKindOf   (key: C_LOGO_IS_KIND_OF)
- v72 address: 0x0042156B  |  v79 (task-008): 0x00421970
- Heuristic: IDB symbol `?IsKindOf@CLogo@@UBEHPBVCRTTI@@@Z`; structural anchor — body loads `offset dword_AA4D2C` (**the SAME CLogo RTTI descriptor GetRTTI 0x421565 returns**) then walks the base-class chain: `cmp eax,[esp+arg_0]` against the queried descriptor → `mov eax,[eax]` follows the base link → loop, returning `1` on match else `0` (`retn 4`). The shared dword_AA4D2C descriptor + chain-walk shape cross-confirm the GetRTTI/IsKindOf pair, independent of the symbol and of address proximity.
- Drift v79→v72: relocated; same `CRTTI`/`UBE` mangling drift as GetRTTI. Descriptor-chain-walk body + shared dword_AA4D2C held.
- Label applied: yes (symbol).

### CLogo::Update   (key: C_LOGO_UPDATE)
- v72 address: 0x005E1789  |  v79 (task-008): 0x005FFE54
- Heuristic: CLogo primary vtable `off_9D3B94` slot0; body anchor — init-once timer `cmp [esi+24h],0` → `call dword_AA7814` (GetUpdateTime) store-back, then elapsed-time logo body.
- Drift v79→v72: relocated; timer field is **[esi+0x24]** in v72 (v79 used [esi+0x1EC]). Was an undefined function — `define_func` before rename. DIVERGES from C_LOGIN_UPDATE.
- Label applied: yes (renamed `CLogo__Update`).

### CLogo::OnMouseButton   (key: C_LOGO_ON_MOUSE_BUTTON)
- v72 address: 0x005E1774  |  v79 (task-008): 0x005FFE3F
- Heuristic: CLogo IUIMsgHandler vtable `off_9D3B48` slot2; body anchor — `cmp [esp+arg_0], 202h` (WM_LBUTTONUP) → `add ecx,0FFFFFFFCh` (thiscall adjust) → `call CLogo::InitNXLogo` (0x5E13CB).
- Drift v79→v72: relocated; 0x202 + InitNXLogo path held. NOTE: the symbol `?OnMouseButton@CLogo@@UAEXIIJJ@Z` (0x4214BC) is the **CWnd 4-arg stub (`retn 10h`), NOT this handler** — the memory-map key is the IUIMsgHandler slot-2 handler.
- Label applied: yes (renamed `CLogo__OnMouseButton`).

### CLogo::OnSetFocus   (key: C_LOGO_ON_SET_FOCUS)
- v72 address: 0x005E1237  |  v79 (task-008): 0x005FF902
- Heuristic: CLogo IUIMsgHandler vtable `off_9D3B48` slot1; body = `push 1; pop eax; retn 4` (always-true stub).
- Drift v79→v72: relocated; identical stub shape.
- Label applied: yes (renamed `CLogo__OnSetFocus_IUI`).

### CLogo::OnKey   (key: C_LOGO_ON_KEY)
- v72 address: 0x005E174D  |  v79 (task-008): 0x005FFE18
- Heuristic: CLogo IUIMsgHandler vtable `off_9D3B48` slot0; body anchor — key-up filter `test [esp+0Bh],80h` then `cmp …,0Dh/1Bh/20h` (13/27/32) → `add ecx,-4` → `call CLogo::InitNXLogo` (0x5E13CB).
- Drift v79→v72: relocated; the 13/27/32 + InitNXLogo cluster held. Was an undefined function — `define_func` before rename.
- Label applied: yes (renamed `CLogo__OnKey`).

### CLogo::LogoEnd   (key: C_LOGO_LOGO_END)
- v72 address: 0x005E1381  |  v79 (task-008): 0x005FFA4C
- Heuristic: call-graph — body is `Alloc(0x23C)` → `CLogin::CLogin` (0x5AECED) → `set_stage` (0x6C1FBB); the alloc+ctor+set_stage triple is the fingerprint (sole such caller of the CLogin ctor + set_stage in the CLogo range).
- Drift v79→v72: relocated; **alloc size 0x23C (572)** vs v79 0x258. SetStage callee confirmed independently.
- Label applied: yes (renamed `CLogo__LogoEnd`).

### CLogo::ForcedEnd   (key: C_LOGO_FORCED_END)
- v72 address: 0x005E135F  |  v79 (task-008): 0x005FFA2A
- Heuristic: CLogo primary vtable `off_9D3B94` slot2; body = `push 3E8h` (1000) + `CSoundMan::PlayBGM` on the CSoundMan singleton `dword_AA3ABC` + nullsub tail.
- Drift v79→v72: relocated; PlayBGM(0x3E8)-on-singleton idiom held (singleton moved to dword_AA3ABC).
- Label applied: yes (renamed `CLogo__ForcedEnd`).

### CLogo::Init   (key: C_LOGO_INIT)
- v72 address: 0x005E12F1  |  v79 (task-008): 0x005FF9BC
- Heuristic: CLogo primary vtable `off_9D3B94` slot1; body anchor — sub-init helper (sub_5E183F) → `CInputSystem::ShowCursor(0)` → `CWvsApp::GetCmdLine(3)`; if cmdline arg non-empty AND g_CWvsApp[+0x28]!=0 tail-calls LogoEnd (skip-logo shortcut).
- Drift v79→v72: relocated; ShowCursor(0)+GetCmdLine(3)+conditional-LogoEnd shape held.
- Label applied: yes (renamed `CLogo__Init`).

### CLogo::InitNXLogo   (key: C_LOGO_INIT_NX_LOGO)
- v72 address: 0x005E13CB  |  v79 (task-008): 0x005FFA96
- Heuristic: init-once guard `[this+0x28]==0` (this[10]) → `StringPool::GetInstance/GetBSTR(<ID>)` NX-logo resource → IWzResMan::GetObjectA sprite load; callers = CLogo::OnKey + OnMouseButton (+Init/Update timer path).
- Drift v79→v72: relocated; **StringPool ID drifted to 1386 (0x56A)** from v79's 0x568. Guard at [this+0x28] held. Verify via the resolved string, not the numeric ID, in older builds.
- Label applied: yes (renamed `CLogo__InitNXLogo`).

### Stage singleton   (key: STAGE_INSTANCE_ADDR)
- v72 address: 0x00AA54D4  |  v79 (task-008): 0x00B0DADC
- Heuristic: store/clear by `set_stage` (0x6C1FBB) — cleared (`dword_AA54D4=0`) at entry, new stage installed via sub_6C223A, then dispatched (`[*dword_AA54D4+4]` = Init). Second anchor — read by `CWvsApp::CallUpdate` (0x8F4991) and `CClientSocket::ProcessPacket` stage dispatch.
- Drift v79→v72: relocated into the v72 .data window (0xAA54D4 ∈ 0xA9xxxx–0xABxxxx, like the other v72 globals); singleton store/clear pattern held.
- Label applied: yes (renamed `StageInstanceAddr`).

### SetStage   (key: SET_STAGE)
- v72 address: 0x006C1FBB  |  v79 (task-008): 0x006F1AC0
- Heuristic: IDB symbol `?set_stage@@YAXPAVCStage@@PAX@Z`; structural — clears STAGE_INSTANCE_ADDR, calls old-stage ForcedEnd (vtable+8), installs new stage (sub_6C223A), calls new-stage Init (vtable+4 via dword_AA54D4).
- Drift v79→v72: relocated; **v72 symbol is the free function `set_stage` (lowercase)** vs v79's `SetStage`. Store+ForcedEnd+Init triple held.
- Label applied: yes (symbol).

### GR singleton   (key: GR_INSTANCE_ADDR)
- v72 address: 0x00AA85FC  |  v79 (task-008): 0x00B10F74
- Heuristic: output-arg store — `CWvsApp::InitializeGr2D` (0x8F432B) calls factory `sub_8F7257(res, &dword_AA85FC, 0)` which writes the IWzGr2D through the pointer (`*a2 = factory(...)`). Second anchor — consumed as IWzGr2D in InitializeGr2D (vtable[+12] CreateCanvas 800×600, SetColor).
- Drift v79→v72: relocated; factory drifted to sub_8F7257 (v79 sub_947BB8); output-arg→store→IWzGr2D-dispatch chain held.
- Label applied: yes (renamed `GrInstanceAddr`).

### CStage::OnMouseEnter   (key: C_STAGE_ON_MOUSE_ENTER)
- v72 address: 0x008DF289  |  v79 (task-008): 0x0092F3F8
- Heuristic: IDB symbol `?OnMouseEnter@CStage@@UAEXH@Z` (thiscall, one int, void); **structural second anchor** — body loads the CInputSystem singleton `dword_AA3E84`, guards `arg_0 != 0 && [singleton+0x9B4] != 0`, then dispatches `CInputSystem::SetCursorState(0)` (0x55D3C3) and `retn 4`. The SetCursorState-on-singleton-with-+0x9B4-guard shape is the fingerprint, independent of the symbol.
- Vtable cross-ref (third anchor): inherited CStage virtual — appears in CLogin's IUIMsgHandler secondary vtable `off_9D3120` slot 32 (0x9D31A0, get_bytes = 0x008DF289). Use a NON-overriding CStage subclass (CLogin) as the witness: CLogo OVERRIDES OnMouseEnter so its own vtable block doesn't reference 0x8DF289 (same subtlety task-008 flagged in v79).
- Drift v79→v72: relocated; symbol + SetCursorState-dispatch body both held. The witness slot is CLogin secondary-vtable slot 32 in v72 (v79 used the same CLogin-secondary-vtable method; the seed's "CLogo IUIMsgHandler slot 41" was the imprecise CLogo-block guess task-008 superseded).
- Label applied: yes (symbol).

### CStage::OnPacket   (key: C_STAGE_ON_PACKET)
- v72 address: 0x006C0C61  |  v79 (task-008): 0x006F079F
- Heuristic: IDB symbol `?OnPacket@CStage@@UAEXJAAVCInPacket@@@Z`; second anchor — = CLogo OnPacket vtable `off_9D3B44` slot0 (get_bytes confirmed).
- Drift v79→v72: relocated; symbol + CLogo-vtable cross-ref both held.
- Label applied: yes (symbol).

### CUITitle singleton   (key: C_UI_TITLE_INSTANCE_ADDR)
- v72 address: 0x00AA5114  |  v79 (task-008): 0x00B0D738
- Heuristic: read as CUITitle* and passed to `CUITitle::EnableLoginCtrl` (0x5D4AC7) by its CLogin callers (e.g. sub_5B1318: `EnableLoginCtrl((CUITitle*)dword_AA5114, 1)`); also read in SendCheckPasswordPacket for a control dispatch. Second anchor — destroyed via `CWnd::Destroy(dword_AA5114)` in the CLogin teardown sub_5AF0C7 (the ForcedEnd path, alongside the sibling dword_AA5118).
- Drift v79→v72: relocated into the v72 .data window; the CUITitle-singleton + ForcedEnd-destroy pattern held. (Did not isolate the SBB-store ctor; the two read/destroy anchors are sufficient.)
- Label applied: yes (renamed `UITitleInstanceAddr`).

### Config / Input / FuncKeyMappedMan

> Task 8 Config cluster (CInputSystem/CFuncKeyMappedMan resolved in Task 7 above).
> v72 retains the full mangled C++ symbols for every CConfig member — IDB symbol is the
> fast primary anchor; each entry pairs it with a SECOND structural anchor. The CConfig
> singleton + windowed-mode globals were renamed (g_CConfig_pInstance / g_CConfig_SysOpt_WindowedMode).

#### GetSEPrivilege   (key: GET_SE_PRIVILEGE)
- Primary anchor: import call — only fn calling OpenProcessToken -> LookupPrivilegeValueA("SeDebugPrivilege") -> AdjustTokenPrivileges (+ GetCurrentProcess/CloseHandle).
- Fallback anchor: IDB name `GetSEPrivilege` (named); called from CWvsApp::SetUp early (srand->GetSEPrivilege).
- Drift v79->v72: relocated 0x44A48E -> 0x44989E (by string+import, not carry). SeDebugPrivilege string @0xA5A564.
- v72 address: 0x0044989E.

#### CConfig::CConfig (ctor)   (keys: C_CONFIG / C_CONFIG_INSTANCE_ADDR)
- Primary anchor: IDB symbol `??0CConfig@@QAE@XZ` (nullary).
- Detail: SBB-singleton store `g_CConfig_pInstance = (this+4!=0)?this:0` @0xAA3AC0; installs vtable off_9D09B8; 31/100/24 fuse (this+0x98 block: +8=0x1F, +0x10=0x64, +0x14=0x18, vtable off_9D09BC); StringPool::GetString(2530=0x9E2); RegOpenKeyExA(HKLM=0x80000002 via dword_AA77B8) into this+0x9C area; memset(this+0x234, 0, 0x1BD); LoadGlobal(sub_48C301) + ResetSessionInfo(sub_48EFDE).
- Fallback anchor: 31/100/24 fuse + SOFTWARE\Wizet StringPool + RegOpenKeyExA(HKLM) + memset(0x1BD) + SBB-singleton idioms.
- Drift v79->v72: relocated 0x49392C -> 0x48C0D3; g_CConfig_pInstance 0xB0BED0 -> 0xAA3AC0; **StringPool ID 2530 (v79 2532, v83 2547, v84 2548) — DRIFT**; memset target this+0x234 (v79 this+592=0x250) — member layout drift.
- Label applied: globals renamed g_CConfig_pInstance (function symbol pre-present).
- v72 addresses: C_CONFIG 0x0048C0D3; C_CONFIG_INSTANCE_ADDR 0x00AA3AC0.

#### CConfig::GetPartnerCode   (key: C_CONFIG_GET_PARTNER_CODE)
- Primary anchor: IDB symbol `?GetPartnerCode@CConfig@@QAEJXZ`.
- Detail: sole referencer of literal `uiWndZ0` (aUiwndz0 @0xA5C0D4); CHATLOG_ADD(key) then GetOpt_Int(0,key,0,0x80000000,0x7FFFFFFF).
- Fallback anchor: uiWndZ0 string xref + the GetOpt_Int(.,.,0,INT_MIN,INT_MAX) shape.
- Drift v79->v72: relocated 0x5CC09D -> 0x5B12BD; uiWndZ0 + GetOpt(min,max) shape held.
- v72 address: 0x005B12BD.

#### CConfig::ApplySysOpt   (key: C_CONFIG_APPLY_SYS_OPT)
- Primary anchor: IDB symbol `?ApplySysOpt@CConfig@@QAEXPAUCONFIG_SYSOPT@@H@Z`.
- Detail: `rep movsd` this+0x60 (12 dwords=0x30); writes CWvsContext(dword_A9F438) flags @+0x3504/+0x3508 from game-start-mode this+0x88; BGM/SE volumes `100*(x+1)/20` (imul 0x64/idiv 0x14) -> CSoundMan(dword_AA3ABC)::SetBGMVolume(0x48F4D3)/SetSEVolume(0x6BE0AD); writes InputSystemInstanceAddr(0xAA3E84)+0x970.
- Fallback anchor: SetBGMVolume/SetSEVolume + 100*(x+1)/20 volume math + get_field.
- Drift v79->v72: relocated 0x4960F9 -> 0x48E7EC; member byte-offsets per-version.
- v72 address: 0x0048E7EC.

#### CConfig::CheckExecPathReg   (key: C_CONFIG_CHECK_EXEC_PATH_REG)
- Primary anchor: IDB symbol `?CheckExecPathReg@CConfig@@QAEXV?$ZXString@D@@@Z` + StringPool exec-path pair.
- Detail: gates on this+0xBC (the reg key handle from ctor); StringPool::GetString(3109=0xC25 ExecPath value, 3110=0xC26 MapleStory.exe); builds `"\\"`(0x5C) separator via operator=+Right(1); compares stored vs running path via strcmp; on mismatch _Cat + GetFileAttributes(dword_AA7554; ==-1 || &0x10) + SetOpt_String writeback.
- Fallback anchor: this+0xBC reg-handle gate + 0x5C backslash + GetFileAttributes(&0x10).
- Drift v79->v72: relocated 0x49440C -> 0x48CBAE; **StringPool IDs 3109/3110 (v79 3114/3115, v83 3135/3136, v84 3138/3139) — DRIFT**; reg-handle gate this+0xBC (v79 this[48]=0xC0).
- v72 address: 0x0048CBAE.

#### CConfig sys-opt windowed-mode flag   (key: C_CONFIG_SYS_OPT_WINDOWED_MODE)
- Primary anchor: reader code sites — global read by 3 labeled fns: CWvsApp::SetUp (0x8F2C49), CWvsApp::CreateMainWindow (0x8F3850, `flag!=0 ? 0x80000000 : 0x80000` window-style branch + `?8:0` exstyle), CWvsApp::InitializeGr2D (0x8F438B). The two-reader (CreateMainWindow + InitializeGr2D) pattern holds.
- Fallback anchor: the 0x80000000 (fullscreen) vs 0x80000 (windowed) style immediates fed by this flag in CreateMainWindow.
- Drift v79->v72: relocated 0xB11548 -> 0xAA87AC; two-reader pattern + style immediates held. (Note v72 CreateMainWindow computes the style via neg/sbb rather than literal 720896.)
- Label applied: renamed g_CConfig_SysOpt_WindowedMode.
- v72 address: 0x00AA87AC. FOR TASK 13: windowed-mode global the ConfigSysOpt audit cross-checks.

#### CMob::CMob (ctor)   (key: C_MOB_C_MOB)   [HIGH-VALUE / needs-main-review — doom-fix hook target, feeds Task 15]
- Primary anchor: IDB symbol `??0CMob@@QAE@PAVCMobTemplate@@@Z`.
- Detail: placement ctor over a NON-zeroed **1216-byte (0x4C0)** allocation (sole caller CreateMob 0x611C9F -> ZAllocEx<ZAllocAnonSelector>::Alloc(0x4C0), selector unk_AA7CB8). Body: CLife base ctor (sub_5AB61E); installs 3 primary CMob vtables at this+0/+4/+8 (off_9D4010/off_9D3FEC/off_9D3FE8) + several secondary embedded vtables; stores `m_pTemplate = pMobTemplate` at **this+0x160** (v79 this+0x188); 31/100/24 secure-fuse object at this+0x488 (vtable off_9D2F48, +8=0x1F, +0x10=0x64, +0x14=0x18); runs the _ZtlSecureTear chain over the secured stat members; MobStat::SetFrom(this+0x178, m_pTemplate)(0x6D0896); CWvsContext::SetExclRequestSent(0x8900FC); ends with StringPool::GetStringW(958=0x3BE = SP_CANVAS) + PcCreateObject::IWzCanvas into the HP-indicator canvas at this+0x484.
- Fallback anchor (independent kind, spot-checked): sole caller CreateMob (0x611C9F); the CLife-base + 3-vtable install + m_pTemplate store + _ZtlSecureTear chain + SP-958/IWzCanvas tail — confirmed WITHOUT the symbol.
- Drift v79->v72: relocated 0x630C2C -> 0x611CDB; **alloc 1216 (0x4C0) — SMALLER than v79's 1304 (0x518)**; m_pTemplate this+0x160 (v79 0x188); **StringPool 958 (v79 957, v83 956, v84 960) — DRIFT**; HP-canvas this+0x484 (v79 0x4D0). vtable globals per-version.
- **DOOM-FIELD FINDING (Task 15 / doom-fix gate `< 84`):** v72 ctor LEAVES `m_bDoomReserved` UNINITIALIZED **AND the doom tail field DOES NOT EXIST in v72.** Evidence: (1) alloc is non-zeroing ZAllocEx::Alloc(**0x4C0 = 1216**), so the whole object ends at offset 0x4C0; (2) the v84 doom fields sit at this+0x540 (m_bDoomReserved) / this+0x544 (m_bDoomReservedSN) — **past the v72 struct end (0x4C0)**; (3) the ctor's highest member write is ~this+0x4B8 (the secure-fuse object + trailing flags), with NO tail zero-init block reaching any doom offset; (4) v72 is even smaller than v79 (0x4C0 vs 0x518) and lacks both the v84-only fields and the doom field. Verdict: v72 is correctly on the doom-fix needs-fix side (gate `< 84`), same disposition as v79. (Exact pinning of MobStat/CMob sizes is Task 12's read-only struct audit; the verdict rests on the alloc size + v84 contrast + absence of any tail doom write.)
- Label applied: function symbol pre-present (not renamed).
- v72 address: 0x00611CDB.

### Manager singletons (Task 7)

**Cluster anchor — CWvsApp::SetUp (0x008F2A7D).** The whole manager-singleton chain is driven from one
function: `CWvsApp::SetUp`. It calls `TSingleton<X>::CreateInstance()` in a fixed order, and each
`CreateInstance` (an `__EH_prolog` thunk) reads its instance global at the top
(`mov eax, <g>; test eax; jnz ret` — the TSingleton lazy-init guard) and, if null, calls
`ZAllocEx<ZAllocAnonSelector>::Alloc(<size>)` + the class ctor (the SBB-singleton ctor stores `this`
into `<g>`). So **two anchors fall out together for every pair**: (1) the surviving mangled
`?CreateInstance@?$TSingleton@V<Class>@@…` symbol, and (2) the SetUp call site + the `Alloc(<size>)`
immediate that matches the class object size + the instance global the guard reads. v72's complete
TSingleton list is 11 entries (CUIQuestAlarm, CInputSystem, CQuestMan, CMonsterBookMan, CActionMan,
CAnimationDisplayer, CClientSocket, CFuncKeyMappedMan, CQuickslotKeyMappedMan, CSecurityClient,
CMapleTVMan) — **no CMacroSysMan, no CRadioManager** (see Sentinels).

The v72 manager `CreateInstance` cluster relocated to **0x8F5E41–0x8F6353** (the v79 cluster was at
0x946xxx — a wholesale move; never inherit a v79 0x946xxx address). The instance globals relocated to
the v72 singleton .data window **0xA9F3F4 / 0xAA3xxx / 0xAA4xxx** (v79 used 0xB07xxx / 0xB0Cxxx /
0xB0Dxxx). Method functions kept their own addresses (relocated independently). All 14 globals/methods
labeled in the v72 IDB.

| Manager | CreateInstance (key …_CREATE_INSTANCE) | instance global (key …_INSTANCE_ADDR) | Alloc | ctor | SetUp call site |
|---|---|---|---|---|---|
| ActionMan | 0x008F6172 | 0x00A9F3F4 | 0x2A0=672 | 0x406497 | 0x8f2d1b |
| AnimationDisplayer | 0x008F61C8 | 0x00AA3A8C (no key) | 0x1A8=424 | 0x431b69 | 0x8f2d27 |
| MapleTVMan | 0x008F6353 | 0x00AA4E68 | 0x3D0=976 | 0x5e8902 | 0x8f2d2c |
| MonsterBookMan | 0x008F5F3F | 0x00AA4D24 | 0xA4=164 | 0x8f5f84 | 0x8f2d7e |
| QuestMan | 0x008F5E99 | 0x00AA4D28 | 0x258=600 | 0x6834e4 | 0x8f2d38 |
| InputSystem | 0x008F5E41 | 0x00AA3E84 | 0x9D0=2512 | 0x8f489e | InitializeInput 0x8f4626 |
| FuncKeyMappedMan | 0x008F6264 | 0x00AA4CB8 | 0x388=904 | 0x5512ec | 0x8f2c77 |
| QuickslotKeyMappedMan | 0x008F62BA | 0x00AA3D04 (no key) | 0x30=48 | 0x5e385d | 0x8f2c7c |
| SecurityClient | 0x008F630E | 0x00AA3EE4 | 0x13C=316 | 0x941dcf | 0x8f2c2e |

- Drift v79→v72: every CreateInstance + every instance global RELOCATED (wholesale cluster move). The
  TSingleton lazy-init codegen + the `Alloc(size)` immediate held perfectly — Alloc sizes match the
  class identities, giving a robust structural anchor independent of the symbol. MapleTVMan Alloc =
  976 (v79 comment said 992) and QuestMan = 600 (v79 said 648): the v72 objects are slightly smaller
  (older/reduced members), but the symbol + SetUp order made the identity unambiguous.

#### Manager method keys (Task 7 Step 3)
- `C_ACTION_MAN_INIT` 0x0040681C — symbol `?Init@CActionMan@@`; SetUp call @0x8f2d22 (right after ActionMan CreateInstance). **DIRECT (== v79).**
- `C_ACTION_MAN_SWEEP_CACHE` 0x0040FE89 — symbol `?SweepCache@CActionMan@@`; sole caller CWvsApp::CallUpdate (0x8f4991). Drift: 0x40FE89 vs v79 0x40FEEA (the v79 cmake value 0x40FEEA was mid-function — corrected to the function entry).
- `C_MAPLE_TV_MAN_INIT` 0x005E8B18 — symbol `?Init@CMapleTVMan@@`; SetUp call @0x8f2d33.
- `C_MONSTER_BOOK_MAN_LOAD_BOOK` 0x0062F410 — symbol `?LoadBook@CMonsterBookMan@@`; SetUp call @0x8f2d85 (failure throws CTerminateException).
- `C_QUEST_MAN_LOAD_DEMAND` 0x00683A9D — symbol `?LoadDemand@CQuestMan@@`; SetUp call @0x8f2d3f (failure throws CTerminateException).
- `C_QUEST_MAN_LOAD_PARTY_QUEST_INFO` 0x006887DE — symbol `?LoadPartyQuestInfo@CQuestMan@@`; SetUp call @0x8f2d6e on QuestManInstanceAddr (dword_AA4D28).
- `C_QUEST_MAN_LOAD_EXCLUSIVE` 0x00689C3E — symbol `?LoadExclusive@CQuestMan@@`; SetUp call @0x8f2d79 on dword_AA4D28.
- `C_INPUT_SYSTEM_INIT` 0x0055CBA9 — symbol `?Init@CInputSystem@@`; InitializeInput call @0x8f462d.
- `C_INPUT_SYSTEM_SHOW_CURSOR` 0x0055D022 — symbol `?ShowCursor@CInputSystem@@`; caller CLogo::Init @0x5e130c (ShowCursor(0) on the InputSystem singleton).
- `C_SECURITY_CLIENT_ON_PACKET` 0x009422D1 — **[needs-main-review, spot-checked]** symbol `?OnPacket@CSecurityClient@@`; body = `Decode1; cmp al,4; jnz; OnCheckClientIntegrityRequest(0x9422f0)` — matches the documented integrity-check dispatch; reached from CClientSocket::ProcessPacket (0x486922) case 0x14.

#### InputSystem unnamed methods (no surviving symbol — call-graph + body anchors)
The 3 input-pump methods have no mangled symbol in v72; resolved via CWvsApp::Run (0x8F2F82) call sites
+ body shape. The v72 IDB also carried unresolved "v84 name" hints that matched exactly (relabeled).
- `C_INPUT_SYSTEM_UPDATE_DEVICE` 0x0055CFD3 — Run msgtype<=2 branch `sub_55CFD3(v3)` @0x8f304e; body `if(!a1) UpdateKeyboard(0x55dc61); if(a1==1) UpdateMouse(0x55d7e8)`. Two anchors: call-graph (Run) + body shape. (v84 hint: UpdateDevice_CInputSystem.)
- `C_INPUT_SYSTEM_GET_IS_MESSAGE` 0x0055CFF0 — Run inner drain loop `while(sub_55CFF0(&v32)) ISMsgProc(...)` @0x8f305d; body `if(!this[625]) return 0; copy 3 dwords from this[626]`. (v84 hint: GetIsMessage_CInputSystem.)
- `C_INPUT_SYSTEM_GENERATE_AUTO_KEY_DOWN` 0x0055DFBC — Run else-branch `if(sub_55DFBC(&v32)) ISMsgProc(...)` @0x8f3091, immediately before `sub_94222A(SecurityClientInstanceAddr)` = CSecurityClient::Update; body `*a2=256; … GetSpecialKeyFlag(0x55ded5)`. (v84 hint: GenerateAutoKeyDown_CInputSystem.)

#### DEFAULT_FKM_INSTANCE_ADDR / DEFAULT_QKM_INSTANCE_ADDR
- `DEFAULT_FKM_INSTANCE_ADDR` 0x00A5B838 — the 445-byte (0x1BD) FKM default blob. Two anchors: (1) FKM
  ctor 0x5512ec `memcpy(this+4, &unk_A5B838, 0x1BD)` (and a second memcpy to this+449); (2) symbol
  `?DefaultFuncKeyMap@CFuncKeyMappedMan@@` (0x5516c1) `memcpy(this+4, unk_A5B838, 0x1BD)`. Relocated (v79 0xABF99C).
- `DEFAULT_QKM_INSTANCE_ADDR` 0x00000000 — confirmed absent: FKM ctor 0x5512ec does **not** memcpy a
  quickslot-default blob; it zeroes the region (`*((_DWORD*)this+224)=0; +225=0`). Same disposition as
  v79. FLAG: key_mapped_hooks quickslot memcpy must tolerate 0.

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

> **Older-build opcode drift (R8).** All three senders' packet opcodes drifted +1 from v79 in v72:
> party (join+create) **0x79→0x7A**, migrate **0x99→0x9A**. The opcode immediate was
> read from v72 disassembly, not assumed. The opcode is still the disambiguator *between* senders
> only via the sub-opcode (join Encode1(4) vs create Encode1(1)); the base opcode (0x7A) is shared
> by both party senders. (Note: these client→server opcodes are version-local; atlas-ms gates by
> its own server-side enum, so no atlas-ms registry directly pins this client send-opcode value —
> recorded as a pure v72 read.)

### CField::SendJoinPartyMsg   (keys: C_FIELD_SEND_JOIN_PARTY_MSG / _OFFSET)   [HIGH-VALUE / needs-main-review]
- v72 address: 0x00514462   |   v79 address (task-008): 0x0051B4C9
- Primary anchor: IDB symbol `?SendJoinPartyMsg@CField@@QAEXABV?$ZXString@D@@@Z` (one-arg invitee ZXString).
- Structural anchor 1 (opcode immediate): `push 7Ah` @0x5145F4 → `COutPacket::COutPacket(0x7A)` (0x656FA1), then `Encode1(4=invite)` (0x4062C7), `EncodeStr(name)` (0x468295), `CClientSocket::SendPacket` (0x4866AC). Sub-opcode 4 + EncodeStr distinguish from CreateNewParty.
- Structural anchor 2 (call-graph): resolved Task-4/5 encoder+sender chain (COutPacket ctor / Encode1 / EncodeStr / SendPacket all resolved); reads g_pWvsContext (0xA9F438) for GetCharacterData `this`; same job-id `_ZtlSecureFuse<short>` (+0x39/+0x3D vs 0/3E8h) + level `_ZtlSecureFuse<uchar>` (cmp al,0Ah) gates, strcmp-vs-own-name + GetPartyMemberNumber<6 party-full guard. v79→v83 chain held in task-008; same shape in v72.
- **_OFFSET = 0x60** (SP-2): level-gate `jnb short loc_5144F3` @0x5144C2 (bytes `73 2F`, right after `cmp al,0Ah`); delta from base 0x514462. **DRIFT vs v79 0x5E** (same jnb instr, longer v72 preamble); re-measured at byte level.
- Drift v79→v72: address relocated; **opcode 0x79→0x7A** (R8); offset 0x5E→0x60. Encoder idiom + gate shape identical.
- Label applied: yes (symbol already present in v72 IDB).

### CField::SendCreateNewPartyMsg   (keys: C_FIELD_SEND_CREATE_NEW_PARTY_MSG / _OFFSET)   [HIGH-VALUE / needs-main-review]
- v72 address: 0x005142B0   |   v79 address (task-008): 0x0051B318
- **No v72 IDB symbol** — was `sub_5142B0`; located by call-graph + opcode, then labeled `?SendCreateNewPartyMsg@CField@@QAEXXZ` this task.
- Structural anchor 1 (opcode immediate): `push 7Ah` @0x514384 → `COutPacket(0x7A)` (0x656FA1), then `Encode1(1=create)` (0x4062C7), `CClientSocket::SendPacket` (0x4866AC). **Nullary, single Encode1(1), no EncodeStr** — distinguishes from SendJoinPartyMsg (4+EncodeStr) and from the adjacent leave-party sender sub_5143C9 (Encode1(2)+Encode1(0)).
- Structural anchor 2 (call-graph + gates): reads g_pWvsContext (0xA9F438) via GetCharacterData; same job-id `_ZtlSecureFuse<short>` gate + party-exists guard (`cmp [edi+2C98h]`) + level `_ZtlSecureFuse<uchar>` (cmp al,0Ah) gate as v79; resolved encoder+sender chain. v79→v83 chain held in task-008.
- Disambiguation: three sibling senders share opcode 0x7A — create = Encode1(1) nullary; join (0x514462) = Encode1(4)+EncodeStr; leave (sub_5143C9 @0x5143C9) = Encode1(2)+Encode1(0). Chosen by sub-opcode + arg shape, never by proximity.
- **_OFFSET = 0x9E** (SP-2): level-gate `jnb short loc_514384` @0x51434E (bytes `73 34`, after `cmp al,0Ah`); delta from base 0x5142B0. **DRIFT vs v79 0x9D (+1)** (same jnb instr); re-measured.
- Drift v79→v72: address relocated; **opcode 0x79→0x7A** (R8); offset 0x9D→0x9E; symbol absent in v72 (labeled this task).
- Label applied: yes (rename sub_5142B0 + set_type `void __thiscall CField::SendCreateNewPartyMsg(CField *)`).

### CWvsContext::SendMigrateToITCRequest   (keys: C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST / _OFFSET)   [HIGH-VALUE / needs-main-review]
- v72 address: 0x0090C9BD   |   v79 address (task-008): 0x0095DD85
- Primary anchor: IDB symbol `?SendMigrateToITCRequest@CWvsContext@@QAEXXZ`.
- Structural anchor 1 (string xref): "The MapleStory Trading System is not available for the Guest ID Users." (aTheMaplestoryT @0xA9A5C4) pushed @0x90C9E0 into CHATLOG_ADD on the guest-ID early-out.
- Structural anchor 2 (opcode immediate + call-graph): `push 9Ah` @0x90CACE → `COutPacket(0x9A)` (0x656FA1) → `CClientSocket::SendPacket` (0x4866AC) reading g_pClientSocketInstance (0xA9F434); the ITC-availability gate `get_field()->[+0x124] >> 4 & 1`. v79→v83 chain held in task-008.
- **_OFFSET = 0xE9** (SP-2): ITC-gate `jz short loc_90CACE` @0x90CAA6 (bytes `74 26`, after `shr eax,4; and eax,1`); delta from base 0x90C9BD. **Coincides with v79 0xE9** but re-measured at byte level, NOT copied.
- Drift v79→v72: address relocated; **opcode 0x99→0x9A** (R8); offset coincides (0xE9). String + gate + encoder idiom identical.
- Label applied: yes (symbol already present in v72 IDB).

### CWvsContext singleton / CWvsContext::OnEnterGame   (keys: C_WVS_CONTEXT_INSTANCE_ADDR / C_WVS_CONTEXT_ON_ENTER_GAME / _OFFSET)
- v72 addresses: INSTANCE_ADDR 0x00A9F438; ON_ENTER_GAME 0x008FF597 (size 0x1F1).   |   v79: 0x00B07848 / 0x00950297.
- ON_ENTER_GAME primary anchor: IDB symbol `?OnEnterGame@CWvsContext@@QAEXXZ`.
- Structural anchor 1 (call-graph): **sole caller is set_stage** (`?set_stage@@YAXPAVCStage@@PAX@Z` @0x6C1FBB); at 0x6C205E `mov ecx, g_pWvsContext(0xA9F438); call OnEnterGame` in the GetCharacterData!=0 (`cmp byte [arg_0+3]`) branch — the OnEnterGame/OnLeaveGame trio shape.
- Structural anchor 2 (member-ctor block): OnEnterGame body runs the CWvsContext sub-object ctors at **this+0x33xx** (lea ecx,[esi+3330h]/+32E8h/… then a TSecType<long>::SetData run) — v79 was this+0x34xx (per-version member offsets, located via the trio not the raw address).
- INSTANCE_ADDR anchors: (1) loaded as `this` for OnEnterGame in set_stage; also read by both party senders + migrate; (2) **= g_pClientSocketInstance(0xA9F434)+4** — the canonical socket-then-context layout. Renamed g_pWvsContext in the v72 IDB this task (was dword_A9F438).
- **ON_ENTER_GAME_OFFSET = 0x0F** (SP-2): first body instr `lea ecx,[esi+3330h]` @0x8FF5A6 after `__EH_prolog` + reg-save (push ecx/push esi/mov esi,ecx/push edi); delta from base 0x8FF597. v72 omits the v83 `push 1` (like v79/v84) → coincides with v79 0x0F, re-measured not copied.
- Drift v79→v72: both addresses relocated (instance 0xA9F438 vs 0xB07848; fn 0x8FF597 vs 0x950297); member block 0x34xx→0x33xx; offset coincides (0x0F). Caller/trio shape stable.
- Label applied: yes (OnEnterGame symbol present; g_pWvsContext renamed from dword_A9F438).

### Utilities

> Task 8 utilities cluster. v72 retains the mangled symbols; each entry pairs the symbol
> with a structural anchor.

#### CIGCipher::innoHash   (key: C_IG_CIPHER_INNO_HASH)
- Primary anchor: IDB symbol `?innoHash@CIGCipher@@SAKPAEHPAK@Z`.
- Detail: loops `bShuffle(v3, buf[i])` (sub_940DB9) over `len` bytes, returns `*v3`; no-key path (`if(!a3) v3=&var_4`) seeds `var_4 = 0xC65053F2`. Called in CClientSocket::SendPacket between MakeBufferList and Flush.
- Fallback anchor: the 0xC65053F2 seed + bShuffle loop + the SendPacket call position.
- Drift v79->v72: relocated 0x993442 -> 0x940D7E; **no-key seed 0xC65053F2 (v79 0xC6EF3720) — DRIFT** (read it, don't carry).
- v72 address: 0x00940D7E.

#### ZSynchronizedHelper<ZFatalSection> ctor/dtor   (keys: Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_CTOR / _DTOR)
- Primary anchor: CTOR has IDB symbol `??0?$ZSynchronizedHelper@VZFatalSection@@@@QAE@AAVZFatalSection@@@Z`; reached as the SendPacket per-socket lock (CClientSocket::SendPacket calls it on this+124).
- Detail: CTOR (0x402AB8, size 0x25): acquire-loop `result = off_A60AFC()` (the ZFatalSection acquire thunk); while non-zero, `dword_AA74FC(0)` (Sleep(0)) and retry. DTOR (ctor+0x25 = 0x402ADD): `mov eax,[ecx]; dec dword[eax+4]; jnz; and dword[eax],0; retn` — decrements the recursion count, clears on last release.
- Fallback anchor: the acquire-loop/Sleep(0) retry (ctor) + the dec-and-clear (dtor); adjacent acquire/release pair.
- Drift v79->v72: **v72 VAs (0x402AB8 / 0x402ADD) are IDENTICAL to v79** — but confirmed by v72-specific globals (acquire thunk off_A60AFC vs v79 off_AC4ECC; Sleep slot dword_AA74FC vs v79 dword_B0FDE4) + the SendPacket lock-acquire call-edge, NOT by carrying the v79 address. v83/v84 differ (0x403166/0x40318B). The dtor listing is aliased under ZAllocEx::Alloc in this dump (do not trust list_funcs there) — not renamed.
- v72 addresses: CTOR 0x00402AB8, DTOR 0x00402ADD.

#### CSystemInfo: ctor / Init / GetMachineId / GetGameRoomClient   (keys: C_SYSTEM_INFO / _INIT / _GET_MACHINE_ID / _GET_GAME_ROOM_CLIENT)
- Primary anchor: IDB symbols (all four retain mangled names: `??0CSystemInfo@@QAE@XZ`, `?Init@CSystemInfo@@QAEXXZ`, `?GetMachineId@CSystemInfo@@QAEPBEXZ`, `?GetGameRoomClient@CSystemInfo@@QAEKXZ`) + string xref (Init).
- Detail: ctor (0x94A6C0, size 9) installs vtable off_9DC404 (1 instruction). Init (0x94A700, size 0x2C4) = machine-id builder: Netbios (ncb_command 0x37/0x32/0x33 MAC query) + GetVolumeInformationA + RegOpenKeyExA(`SOFTWARE\Microsoft\Windows\CurrentVersion` @0xA9AD80) + `CxSupportId` (@0xA9AD74, RegQueryValueExA 16 bytes) + CoCreateGuid fallback. GetMachineId (0x94A9E0, size 4) returns the cached 16-byte id. GetGameRoomClient (0x94AAE0, size 0x11B4) = the process-table fn.
- Fallback anchor: ctor = the only construct/Init pair in SendCheckPasswordPacket; Init via CxSupportId + Netbios; GetMachineId via EncodeBuffer(id,16) at the call site.
- Drift v79->v72: cluster relocated wholesale (v79 0x99CDxx/0x99Dxxx -> v72 0x94A6xx/0x94Axxx); CxSupportId/CurrentVersion/Netbios anchors held.
- Label applied: all four symbols pre-present (vtable off_9DC404).
- v72 addresses: C_SYSTEM_INFO 0x0094A6C0; C_SYSTEM_INFO_INIT 0x0094A700; C_SYSTEM_INFO_GET_MACHINE_ID 0x0094A9E0; C_SYSTEM_INFO_GET_GAME_ROOM_CLIENT 0x0094AAE0.

#### ZArray<unsigned char>::RemoveAll   (key: Z_ARRAY_REMOVE_ALL)
- Primary anchor: IDB symbol `?RemoveAll@?$ZArray@E@@QAEXXZ`.
- Detail: `if(*this){ ZAllocEx<ZAllocAnonSelector>::Free(*this-4, selector unk_AA7CB8)(0x402B3F); *this=0 }`. Stride-1 (no imul).
- Fallback anchor: the `*this-4` Free + `*this=0` shape; a thunk j_?RemoveAll@?$ZArray@E@@ (0x486D1E) jumps here (seen in the CConfig ctor unwind).
- Drift v79->v72: relocated 0x4260F4 -> 0x425CEC. The generic ZArray<T>::RemoveAll variants (struct-audit tools Tasks 12-16) carry `imul stride,count` element-walks — re-derive stride per element type.
- v72 address: 0x00425CEC.

#### ZXString<char>::GetBuffer (cstr-assign)   (key: Z_X_STRING_GET_BUFFER)   [needs-main-review]
- Primary anchor: IDB symbol `?_Cat@?$ZXString@D@@IAEAAV1@PBDH@Z` (the in-place assign/append primitive; no dedicated pure-assign GetBuffer(PBD,H) symbol exists in v72, same as v79/v84).
- Detail: on an empty/zeroed ZXString does `inner GetBuffer(Size,0)(0x414576) + memcpy(Src,Size) + ReleaseBuffer(0x414621)` (== assign); on a non-empty string doubles capacity and appends.
- Fallback anchor: structure — inner-GetBuffer (0x414576) + memcpy + ReleaseBuffer operating in place on `this`.
- Drift v79->v72: relocated 0x426133 -> 0x425D2B; inner GetBuffer 0x4147BB -> 0x414576. **needs-main-review**: repo only invokes it on freshly-managed ZXStrings, so it behaves as assign — confirm that invariant or locate a dedicated pure-assign if one is added.
- v72 address: 0x00425D2B.

#### ZXString<char>::TrimRight / TrimLeft   (keys: Z_X_STRING_TRIM_RIGHT / Z_X_STRING_TRIM_LEFT)
- Primary anchor: IDB symbols `?TrimRight@?$ZXString@D@@QAEAAV1@PBD@Z` / `?TrimLeft@?$ZXString@D@@QAEAAV1@PBD@Z` + the whitespace literal.
- Detail: both default `Str` to `" \t\r\n"` (asc_A5AAF0 @0xA5AAF0) when NULL and use strchr to test set-membership, calling inner GetBuffer (0x414576). TrimRight scans backward, NUL-terminates after last non-set char; TrimLeft scans forward then memcpy-shifts the remainder to the front. ADJACENT in the image (TrimRight immediately before TrimLeft).
- Fallback anchor: the shared " \t\r\n" literal + right-scan vs left-scan+memcpy distinction.
- Drift v79->v72: relocated TrimRight 0x46DB7E -> 0x46C9B4, TrimLeft 0x46DC33 -> 0x46CA69; whitespace literal asc_ABEDA0 -> asc_A5AAF0. Repo calls them `(this, NULL, s)`.
- v72 addresses: TrimRight 0x0046C9B4; TrimLeft 0x0046CA69.

### Exception dispatch
_(entries: C_TI_*EXCEPTION, C_PATCH_EXCEPTION_BUILDER, C_COM_RAISE_ERROR_EX, C_FILE_STREAM_*)_

### Protocol constants (FR-6)
_(VERSION_HEADER, PLAYER_LOGGED_IN, CLIENT_START_ERROR — record the deciding v72 disasm
site, e.g. the OnConnect `if (hdr != N)` compare and the COutPacket opcode immediates;
note any drift from v79's confirmed VERSION_HEADER=8)_

### Sentinels (confirmed absent in v72)
_(DR_CHECK, DR_INIT, CE_TRACER_RUN, C_BATTLE_RECORD_MAN_CREATE_INSTANCE, JMS-only keys, and
any new v72-only sentinels — record the evidence of absence, not just the `0x0`)_

#### C_MACRO_SYS_MAN_CREATE_INSTANCE — NEW v72-only sentinel (was real 0x00946C88 in v79). **FLAG.**
- Evidence of absence (3 independent): (1) no `CMacroSysMan` function symbol (`*CMacroSysMan*` filter empty);
  (2) no `Macro`/`MacroSysMan` string (find_regex empty); (3) **not** in the COMPLETE v72 TSingleton
  CreateInstance list (11 entries enumerated).
- Where the feature went: the macro-sys-data-init role is folded into **CQuickslotKeyMappedMan** in v72.
  `CWvsContext::OnMacroSysDataInit` (0x92126b) operates on QuickslotKeyMappedManInstanceAddr (0xAA3D04)
  (`sub_5E39CE(dword_AA3D04)`), not on any CMacroSysMan instance.
- Disposition: carry `0x00000000` + `# absent in v72`. FLAG gate/edit owner — consuming edit must tolerate 0.

#### C_RADIO_MANAGER_CREATE_INSTANCE / C_RADIO_MANAGER_INSTANCE_ADDR — confirmed absent (as v79). **FLAG (existing).**
- Evidence: no CRadioManager symbol/string; not in the complete TSingleton list. The scheduled-message/radio
  role is folded into **CMapleTVMan** (instance 0xAA4E68), same as v79.
- **Radio-quirk verdict:** the v79 seed for both keys was already `0x00000000`, so there is NO wrong value
  to inherit. The v83 allocator-selector trap (v83 seed 0xBF0B00 = the `dword ZAllocEx` selector passed as
  the 1st Alloc arg, NOT the instance) does **not** apply to the v72 port because the seed source (v79) was
  already 0. Verdict: carry 0 for both; v79 seed correct; no allocator-selector mis-inheritance.

## Cross-version drift summary (fill at end)

A short table of which heuristic classes survived v79→v72 best, to guide the next (older)
port: e.g. "string xrefs: N/N held; byte sigs: M/K held; constants: …". This is the durable
takeaway that makes the v6x port faster.
</content>
