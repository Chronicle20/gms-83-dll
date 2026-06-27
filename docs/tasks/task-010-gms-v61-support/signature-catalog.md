# GMS v61 — Signature Catalog

Companion to `memory-map.md`. Records the IDB identity anchor, then one entry per
relocated key: the heuristic used, the v61 address found, and any notable observations.
Seeded at Task 1; entries added as Tasks 2–19 relocate each cluster.

---

## IDB identity anchor (Task 1 baseline)

Confirmed via `mcp__ida-pro__survey_binary` on port 13344 (2026-06-27).

| Field | Value |
|---|---|
| Module name | `GMS_v61.1_U_DEVM.exe` |
| IDB path | `E:\Programs\Nexon\IDBs_v9\GMS\v61\GMS_v61.1_U_DEVM.exe.i64` |
| Image base | `0x00400000` |
| Image size | `0x00796000` |
| Architecture | x86 32-bit |
| MD5 | `122419f1ab2607a02d585f0b87ead8ce` |
| SHA256 | `7f0caf8cf7c375c8c91514f40fb21086f362bab463af791ef994a8a7bd59b6cd` |
| Total functions | 35,132 (named: 1,977 / library: 480 / unnamed: 32,675) |
| Total segments | 6 |
| Baseline IDB save | `E:\Programs\Nexon\IDBs_v9\GMS\v61\GMS_v61.1_U_DEVM.exe.i64` (Task 1) |

### Segment layout

| Segment | Start | End | Size | Permissions |
|---|---|---|---|---|
| `.text` | `0x401000` | `0x8e5000` | `0x4e4000` | rx |
| `.idata` | `0x8e5000` | `0x8e5458` | `0x000458` | r |
| `.rdata` | `0x8e5458` | `0x960000` | `0x07aba8` | r |
| `.data` | `0x960000` | `0x97f000` | `0x01f000` | rw |
| `.export` | `0xa2c000` | `0xb94000` | `0x168000` | r |
| `.import` | `0xb94000` | `0xb96000` | `0x002000` | r |

### Entry points

| Address | Name | Ordinal |
|---|---|---|
| `0x820db3` | `ZtlTaskMemAllocImp` | 1 |
| `0x820dc4` | `ZtlTaskMemFreeImp` | 2 |
| `0x820dd5` | `ZtlTaskMemReallocImp` | 3 |
| `0x87921f` | `start` | 8884767 |

### Packing verdict

**pre-Themida** — standard PE binary. Evidence:
- Named, conventional section headers (`.text`, `.rdata`, `.data`, etc.)
- Full, resolved import table (kernel32, user32, ws2_32, advapi32, npkcrypt, wzmss, etc.)
- Named export entry points with known symbols
- No `.themida`, `.oreans`, `.vmp`, or virtualized-code sections
- No `NtGetContextThread` import (consistent with pre-DR era; DR anti-debug absent in v61, same as v72)
- `npkcrypt` imported (NProtect/HackShield era-appropriate for v61)

> Note: The v48 DISTRACTOR (port 13345, `GMS_v48_1_DEVM.exe`) must never be confused
> with this target. Always confirm via `list_instances` / `survey_binary` module name.

---

## Per-key signatures

*(To be filled in as Tasks 2–19 relocate each cluster. One sub-section per cluster.)*

### Cluster 1 — WinMain / CWvsApp lifecycle (Task 2)

> **Task 2 key finding:** like v72/v79, the **v61 IDB retains the full mangled C++ symbols**
> for the entire CWvsApp class, CWndMan s_Update/RedrawInvalidatedWindows, WinMain, the
> exception type-info, and CFuncKeyMappedMan. So each function's surviving IDB symbol is the
> fastest primary anchor; each entry still records a second *structural* anchor (two-anchor
> rule). v61 image base 0x400000, image size 0x796000 (smaller than v72's 0x86f000 — older
> build, all addresses lower). The cluster is a near-verbatim relocation of v72's, with TWO
> notable backward divergences: **(1) g_dwTargetOS absent** (the WoW64/target-OS marker is not
> in the v61 ctor), and **(2) the SetUp init driver is even cleaner than v72** — no
> meteora/ehsvc/IAT-clone anti-tamper, only CSecurityClient. SEND_HS_LOG / InitializeAuth
> remain absent (same as v72).

#### WinMain (key: WIN_MAIN) [HIGH-VALUE / needs-main-review]
- v61 address: 0x008205EF (fn `_WinMain@16`, size 0x7A1)
- v72 address (task-009): 0x008EF5AD (size 0xA0A)
- Heuristic: IDB symbol `_WinMain@16`; two structural anchors — (1) string xref `aMaplestoryglob` "MapleStoryGlobal :: MapleStory - Microsoft Internet Explorer" (0x96932C) pushed @0x820D72; (2) the ctor(0x822E44)->SetUp(0x823175)->Run(0x8233CC) call chain @0x8207B3/0x8207C2/0x82081D.
- Drift v72→v61: relocated; smaller body (0x7A1 vs 0xA0A) — the HShield/EHSvc logging present in v79 (absent already in v72) stays absent. Ad-balloon dims 490/190/60 identical to v72. The `\npkgameuninstnomsg.exe` literal anchor used in v72 was not separately re-verified in v61 (MapleStoryGlobal + call-chain sufficed).
- Chain trace (high-value): v72 0x8EF5AD + v83 — same MapleStoryGlobal literal + ctor/SetUp/Run chain at both; v61 corroborated.
- Label applied: yes (pre-named `_WinMain@16`).
- Spot-check: `xrefs_to WinMain` → sole caller is PE entry `start` (0x87921F) @0x8792FA — independent call-graph anchor, holds.

#### WinMain offsets (keys: WIN_MAIN_AD_BALLOON_CONDITIONAL / WIN_MAIN_PATCHER_OFFSET) [SP-2]
- Host: v61 WinMain 0x8205EF. Both re-measured at byte level (never copied).
- **PATCHER_OFFSET = 0x19A**: `call ?ShowStartUpWndModal@@YAXXZ` @0x820789, bytes `E8 0F FE FF FF`; delta from base 0x8205EF. DRIFT vs v72 0x212. (no-patcher edit NOPs these 5 bytes.)
- **AD_BALLOON_CONDITIONAL = 0x714**: `jz short loc_820D72` @0x820D03, bytes `74 6D`, guarding the ShowADBalloon block (dims var_EC=490/var_E8=190/var_E4=60 = 0x1EA/0xBE/0x3C, same as v72) → sub_41E8B5, immediately followed by the MapleStoryGlobal push (sub_7CFA9C). Delta from base 0x8205EF. DRIFT vs v72 0x959. (no-ad-balloon edit flips first byte 0x74->0xEB.)

#### CWvsApp::CWvsApp ctor (key: C_WVS_APP) [HIGH-VALUE / needs-main-review]
- v61 address: 0x00822E44 (`??0CWvsApp@@QAE@PBD@Z`, size 0x2CE)
- v72 address (task-009): 0x008F26C7 (size 0x353)
- Heuristic: IDB symbol; two structural anchors — (1) command-line keyword string xrefs `WebStart`(0x969384)/`token`(0x96937C)/`GameLaunching`(0x96936C); (2) call-graph (called from WinMain right before SetUp). Installs vtable off_8EFEC8; SBB-stores singleton g_CWvsApp (0x970A78).
- Drift v72→v61: relocated, smaller (0x2CE vs 0x353). **v61 ctor is SIMPLER than v72:** NO IsWow64Process/kernel32 probe, NO g_dwTargetOS=1996 store, NO ResetLSP tail call. It calls GetVersionEx and stores major<6/platformId==1 into CWvsApp members (a1[5], a1[9]) only.
- Chain trace (high-value): v72 0x8F26C7 + v83 — same WebStart/token/GameLaunching idioms + WinMain edge.
- Label applied: yes (symbol).
- Spot-check: `xrefs_to WebStart`(0x969384) referenced from this ctor; g_CWvsApp(0x970A78) written here.

#### CWvsApp::SetUp (key: C_WVS_APP_SET_UP) [HIGH-VALUE / needs-main-review]
- v61 address: 0x00823175 (`?SetUp@CWvsApp@@QAEXXZ`, size 0x257)
- v72 address (task-009): 0x008F2A7D (size 0x505)
- Heuristic: IDB symbol; two structural anchors — (1) call-graph: walks the whole Initialize*/Create* cluster (anchor for the rest of the cluster); (2) the CTerminateException throw (`__TI3?AVCTerminateException@@` 0x8F4240) via `_CxxThrowException` on the QuestMan/MonsterBook load-fail paths (code 0x22000006).
- **R11 / init-call order (v61):** srand(rand) → CSecurityClient::CreateInstance + InitModule(sub_86343D) → ZAllocEx Alloc(896)+sub_47921D → InitializePCOM → CreateMainWindow → CClientSocket::CreateInstance → ConnectLogin → CFuncKeyMappedMan::CreateInstance → CQuickslotKeyMappedMan::CreateInstance → InitializeResMan → InitializeGr2D → InitializeInput → InitializeSound → InitializeGameData → CreateWndManager → CConfig::ApplySysOpt → CActionMan::Init / CAnimationDisplayer / CMapleTVMan::Init / CQuestMan::LoadDemand / CMonsterBookMan::LoadBook → exec-path Dir_BackSlashToSlash/upDir/SlashToBackSlash + CheckExecPathReg → CLogo ctor → set_stage. **NO InitializeAuth (NMCO absent). NO DR/anti-debug DR_init step.** Anti-tamper is ONLY CSecurityClient (CreateInstance + InitModule) — **even cleaner than v72**, which additionally had Global\meteora + ehsvc + IAT-clone + HideDll loops + WSAStartup/getpeername integrity. No `NtGetContextThread`. Confirms v61 LACKS the DR subsystem (feeds Task 10).
- Drift v72→v61: relocated, much smaller (0x257 vs 0x505) — anti-tamper machinery reduced.
- Chain trace (high-value): v72 0x8F2A7D (clean, not virtualized) + v83 (CFG-virtualized) — anchored on call-graph + the Initialize* walk; v61 corroborated.
- Label applied: yes (symbol).

#### CWvsApp::Run (key: C_WVS_APP_RUN) [HIGH-VALUE / needs-main-review]
- v61 address: 0x008233CC (`?Run@CWvsApp@@QAEXPAH@Z`, size 0x240)
- v72 address (task-009): 0x008F2F82 (size 0x418)
- Heuristic: IDB symbol; two structural anchors — (1) the four exception type-info refs `__TI3?AVCPatchException@@`(0x900668) / `__TI3?AVCDisconnectException@@`(0x900678) / `__TI3?AVCTerminateException@@`(0x8F4240) / `__TI1?AVZException@@`(0x8F6B60), thrown via `_CxxThrowException` on codes 0x20000000 / 0x21000000-6 / 0x22000000-B; (2) the CallUpdate(0x82490A)/RedrawInvalidatedWindows(0x8162B1) callee pair + ISMsgProc(0x825094) dispatch. Loop exits on msg type 18 (WM_QUIT).
- NOTE: the v61 `__TI` exception-type-info addresses (0x900668/0x900678/0x8F4240/0x8F6B60) DIFFER from the cmake seed (v72's 0x9ECC20/0x9E34C0/0x9DF8C8/0x9E0048) — those C_TI_* keys are Task 10's, recorded here for the consumer.
- Chain trace (high-value): v72 0x8F2F82 + v83 — same exception quad + CallUpdate/Redraw pair.
- Label applied: yes (symbol).

#### CWvsApp::ISMsgProc (key: C_WVS_APP_IS_MSG_PROC)
- v61 address: 0x00825094 (was `sub_825094`, size 0x51 — matches v72's 0x51 exactly; **labeled `?ISMsgProc@CWvsApp@@IAEXIIJ@Z` this task**)
- v72 address (task-009): 0x008F57AB
- Heuristic: NO IDB symbol in v61 (v72 had it). Located by call-graph: called per-message from Run(0x8233CC) at 0x8234B5 and 0x8234E2 — both the <=2 and >3 message branches; `xrefs_to` confirms Run is the sole caller. Size 0x51 matches v72.
- Drift v72→v61: symbol stripped in v61 (labeled this task); structure identical.
- Label applied: yes (rename sub_825094).

#### CWvsApp Initialize*/Create*/ConnectLogin (keys: C_WVS_APP_INITIALIZE_PCOM / _CREATE_MAIN_WINDOW / _CONNECT_LOGIN / _INITIALIZE_RES_MAN / _INITIALIZE_GR2D / _INITIALIZE_INPUT / _INITIALIZE_SOUND / _INITIALIZE_GAME_DATA / _CREATE_WND_MANAGER)
- v61 addresses: InitializePCOM 0x008239B0, CreateMainWindow 0x008239D0, ConnectLogin 0x00823B5B, CreateWndManager 0x00823C44, InitializeResMan 0x00823CA7, InitializeGr2D 0x00824550, InitializeInput 0x008247C3, InitializeSound 0x008248B4, InitializeGameData 0x00824AB5
- v72 addresses (task-009): 0x008F3735 / 0x008F3755 / 0x008F38E5 / 0x008F39F2 / 0x008F3A55 / 0x008F432B / 0x008F45D1 / 0x008F493B / 0x008F4E13
- Heuristic: each carries its IDB mangled symbol (`?InitializePCOM@CWvsApp@@IAEXXZ`, etc.); second anchor = the SetUp(0x823175) call-graph (each is a direct callee).
- Drift v72→v61: direct (all present, same call order in SetUp).
- Label applied: yes (symbols).

#### CWvsApp::GetCmdLine / Dir_BackSlashToSlash / Dir_upDir / Dir_SlashToBackSlash / GetExceptionFileName / CallUpdate
- v61 addresses: GetCmdLine 0x00824D80, Dir_BackSlashToSlash 0x00824EDB, Dir_SlashToBackSlash 0x00824EFE, Dir_upDir 0x00824F21, GetExceptionFileName 0x008250E5, CallUpdate 0x0082490A
- v72 addresses (task-009): 0x008F5495 / 0x008F55F2 / 0x008F5615 / 0x008F5638 / 0x008F57FC / 0x008F4991
- Heuristic: IDB mangled symbols. Second anchors — GetCmdLine: call-graph from ctor (WebStart/token/GameLaunching parse); the three Dir_* statics called from SetUp on the exec-path (BackSlashToSlash→upDir→SlashToBackSlash) — addr order BackSlashToSlash(0x824EDB) < SlashToBackSlash(0x824EFE) < upDir(0x824F21), same as v72; GetExceptionFileName: called from top of WinMain (0x82060D); CallUpdate: sole CWndMan::s_Update(0x81652D) caller, called from Run.
- Drift v72→v61: direct.
- Label applied: yes (symbols).

#### CWndMan::s_Update / RedrawInvalidatedWindows (keys: C_WND_MAN_S_UPDATE / C_WND_MAN_REDRAW_INVALIDATED_WINDOWS)
- v61 addresses: s_Update 0x0081652D, RedrawInvalidatedWindows 0x008162B1
- v72 addresses (task-009): 0x008E2D73 / 0x008E2AF7
- Heuristic: IDB symbols `?s_Update@CWndMan@@SAXXZ` / `?RedrawInvalidatedWindows@CWndMan@@SAXXZ`; second anchor call-graph — s_Update is the sole callee of interest inside CallUpdate(0x82490A); RedrawInvalidatedWindows is called from Run right after CallUpdate (0x823509).
- Drift v72→v61: direct.
- Label applied: yes (symbols).

#### globals: g_CWvsApp / g_dwTargetOS (keys: C_WVS_APP_INSTANCE_ADDR / G_DW_TARGET_OS)
- **g_CWvsApp (C_WVS_APP_INSTANCE_ADDR) = 0x00970A78** (v72: 0x00A9F658). Located via writer (ctor 0x822E44): the `(a1+1!=0)?a1:0` SBB store at top of ctor. Second anchor: `xrefs_to 0x970A78` readers = CClientSocket::ConnectLogin(0x472A0B), CLogin::SendCheckPasswordPacket(0x564418), CLogin::OnSelectWorldResult(0x567CCB) — the login-flow readers (matches v72 catalog). Label applied: yes (renamed g_CWvsApp).
- **g_dwTargetOS (G_DW_TARGET_OS) = ABSENT → new v61-only sentinel (0x00000000).** v72: real 0x00A9A164, written only in ctor via `mov ebx,7CCh`(1996) on OS-major<5 OR IsWow64Process true, alongside ResetLSP. The v61 ctor (0x822E44) has NONE of this: no 7CCh store (byte-search `C7 05 ?? ?? ?? ?? CC 07 00 00` = 0 hits in v61), no IsWow64Process probe, no ResetLSP. The WoW64/target-OS marker post-dates v61. **FLAGGED for gate/edit owner — the consuming edit must tolerate 0.**

#### CWvsApp::InitializeAuth (key: C_WVS_APP_INITIALIZE_AUTH) — ABSENT (confirmed, like v72/v79)
- v61 address: 0x00000000 (sentinel, carried from v72 seed)
- Heuristic: confirmed-absent. No `*InitializeAuth*` symbol in v61; SetUp(0x823175) makes no auth call; no CNMCOClientObject/CNMManager. The NMCO/Nexon-Passport auth subsystem post-dates v61 (same verdict as v72/v79).

#### SendHSLog (key: SEND_HS_LOG) — ABSENT (carried sentinel, already absent in v72)
- v61 address: 0x00000000 (sentinel)
- Heuristic: confirmed-absent (SP-5 backward). No `*HSLog*`/`*SendHSLog*` symbol; v61 SetUp has no HShield/EHSvc report strings — anti-tamper is only CSecurityClient. The AhnLab HShield report log (real 0x0093F8E0 in v79) post-dates v61. **FLAGGED — consuming edit must tolerate 0** (already a sentinel in the v72 seed).

#### CFuncKeyMappedMan (Step 5 — class anchor + v61 size, for Task 3) (full key set resolved in Task 7)
- v61 ctor `??0CFuncKeyMappedMan@@QAE@XZ` @0x0051AA0E installs vtable `off_8E7F58` (@0x008E7F58 — `C_FUNC_KEY_MAPPED_MAN_VFTABLE` candidate); singleton global dword_975CA0 (@0x00975CA0 — `_INSTANCE_ADDR` candidate); CreateInstance (0x00826038) allocates **904 (0x388)** bytes.
- **v61 size = 0x388 (904) — IDENTICAL to v72/v79 reduced 0x388.** Confirmed two ways: (1) `Alloc(904)` in CreateInstance(0x826038); (2) ctor field-init extent — vtable@0 + memcpy 0x1BD→this+4 + memcpy 0x1BD→this+449 + dwords @ this+896 (`*((_DWORD*)this+224)=0`) / this+900 (`+225=0`) → object ends at 904. The quickslot pair is gated out in v61 just as in v72/v79, so **v61 is on the SAME (reduced) side as v72** for Task 3's `CFuncKeyMappedMan.h:52` amendment (add `== 61` to the existing 0x388 branch). No struct type applied (read-only).
- Drift v72→v61: size direct (0x388 = 0x388). vtable/instance/ctor addresses relocated (vtable 0x8E7F58 vs v72 0x9D22D8; instance 0x975CA0 vs 0xAA4CB8; ctor 0x51AA0E vs 0x5512EC; CreateInstance 0x826038 vs 0x8F6264).

### Cluster 2 — CClientSocket / ZSocket / COutPacket (Tasks 4–5)

> **Task 4 key finding:** the v61 IDB retains the **full mangled C++ symbols** for the
> entire socket cluster (`?SendPacket@CClientSocket@@…`, `?OnConnect@CClientSocket@@…`,
> etc.) — same as v72/v79 — so each function's primary anchor is its surviving IDB symbol;
> a second *structural* anchor is recorded per the two-anchor rule (two structural kinds
> for the four high-value keys). v61 routes send/recv/connect/WSAAsyncSelect through
> **cloned import slots** (anti-tamper, same idiom as v72/v79/v83): dword_978224=send,
> dword_97822C=recv, dword_978204=connect, dword_9781E4=WSAAsyncSelect, dword_9781F8=
> WSAGetLastError, dword_977EAC=Sleep, dword_9781C4=GetTickCount. Anchor on the surviving
> `socket`/`closesocket`/`getpeername` *imports* (0x8e53c4 / 0x8e53dc / 0x8e53c0), not the
> cloned slots. **Protocol version drift: v61=61** — SendPacket MakeBufferList immediate 61,
> OnConnect major-version compare 0x3D, ManipulatePacket version/seq XOR check `!= -62`
> (~61). **One backward divergence carried from v72:** ZSocketBase::CloseSocket is *inlined*
> in v61 (no standalone fn; shutdown not imported). FLUSH is a real (unnamed) function
> `sub_47421C` — NOT inlined (renamed to ?Flush@CClientSocket@@QAEXXZ). g_pWvsContext =
> dword_974EF8 (cross-check for C_WVS_CONTEXT_INSTANCE_ADDR).

### g_pClientSocketInstance (key: C_CLIENT_SOCKET_INSTANCE_ADDR)
- v61 address: 0x00975054 (dword_975054, renamed g_pClientSocketInstance)
- v72 address (task-009): 0x00A9F434
- Heuristic: two anchors — (1) writer = CClientSocket ctor (0x4727fb) SBB-singleton store `g = (this+4!=0)?this:0` at top; (2) read+stored by TSingleton<CClientSocket>::CreateInstance (0x825ff3, the `if(!dword_975054)` guard).
- Drift v72→v61: address relocated (.data, down ~0x12A000); writer idiom identical.
- Label applied: yes (renamed g_pClientSocketInstance)

### TSingleton<CClientSocket>::CreateInstance (key: C_CLIENT_SOCKET_CREATE_INSTANCE)
- v61 address: 0x00825FF3 (size 0x45)
- v72 address (task-009): 0x008F621F
- Heuristic: IDB symbol `?CreateInstance@?$TSingleton@VCClientSocket@@@@SAPAVCClientSocket@@XZ`; second anchor = `if(!g_pClientSocketInstance)` guard → ZAllocEx::Alloc(148=0x94) → ctor(0x4727fb) → return. Alloc size 148 identical to v72/v79.
- Drift v72→v61: direct (alloc size 148 identical).
- Label applied: yes (symbol)

### CClientSocket::SendPacket (key: C_CLIENT_SOCKET_SEND_PACKET)   [HIGH-VALUE / needs-main-review]
- v61 address: 0x00474125 (size 0xf7)
- v72 address (task-009): 0x004866AC  |  v83 (cmake): 0x0049637B
- Heuristic: IDB symbol `?SendPacket@CClientSocket@@QAEXABVCOutPacket@@@Z`; two structural anchors — (1) **call-graph + constant**: MakeBufferList(this+80, **61**, this+132, 1, m_seqSnd=[this+33]) [as sub_5FFCE0] → CIGCipher::innoHash(this+132,4,0) → CClientSocket::Flush(sub_47421C @0x4741e9) triple (Flush is the last call); (2) **structure**: ZSynchronizedHelper<ZFatalSection> lock-ctor at [this+124] + fd([this+8])/send-disabled([this+20]) gate.
- Drift v72→v61: send-seq immediate **61** (was 72 in v72, 83 in v83). Read it, never copy.
- Chain trace (high-value): v72 0x4866AC (immediate 72, task-009) → v83 0x49637B (immediate 83, verified — MakeBufferList→innoHash→Flush triple identical). v61 lands at 61 with identical structure.
- Label applied: yes (symbol)
- Spot-check: reached via OnConnect's post-handshake send (0x47319b / 0x4731ef) — independent of the symbol.

### CClientSocket::Flush (key: C_CLIENT_SOCKET_FLUSH)   [HIGH-VALUE / needs-main-review]
- v61 address: 0x0047421C (size 0x102; was unnamed sub_47421C, renamed ?Flush@CClientSocket@@QAEXXZ)
- v72 address (task-009): 0x00486734  |  v83 (cmake): 0x00496403
- Heuristic: two structural anchors — (1) **structure + constant**: send-buffer ZList walk over [this+23] (InterlockedIncrement per node) writing via the cloned send slot dword_978224(fd,buf[+16],len[+12],0), WSAEWOULDBLOCK(**10035**) via dword_9781F8 → CClientSocket::OnError; (2) **call-graph**: it is the sole call inside SendPacket (0x4741e9) and the only socket-region fn touching dword_978224.
- Drift v72→v61: lost its IDB symbol (was symboled in v72 as ?Flush@…). Re-identified structurally + renamed. Cloned send slot relocated dword_AA7874→dword_978224.
- Chain trace (high-value): v72 0x486734 (last call in SendPacket) → v83 0x496403 (?Flush@CClientSocket@@, called by SendPacket 0x4963dc — verified). v61 same role (SendPacket's tail call).
- Label applied: yes (renamed — symbol re-applied)
- Notes: **drift** — unlike v72/v79/v83 which retained the Flush symbol, the v61 IDB had it as sub_47421C. Located by the send-slot/WSAEWOULDBLOCK structure + SendPacket call edge, then renamed.

### CClientSocket::ManipulatePacket (key: C_CLIENT_SOCKET_MANIPULATE_PACKET)   [HIGH-VALUE / needs-main-review]
- v61 address: 0x00474336 (size 0xd4)
- v72 address (task-009): 0x0048684E  |  v83 (cmake): 0x0049651D
- Heuristic: IDB symbol `?ManipulatePacket@CClientSocket@@QAEXXZ`; two structural anchors — (1) **constant + structure**: `while([this+17])` receive reassembly via CInPacket::AppendBuffer, version/seq check `(word[this+57] ^ HIWORD([this+34])) != -62`, on full packet (ret==2) re-seeds m_seqRcv=[this+34]=innoHash(this+136,4,0); (2) **call-graph**: **sole caller** of ProcessPacket (0x47440a).
- Drift v72→v61: version/seq XOR constant **-62** (~61; was -73=~72, -84=~83).
- Chain trace (high-value): v72 0x48684E (XOR -73) → v83 0x49651D (XOR -84, sole caller of ProcessPacket 0x4965f1 — verified). v61 XOR -62, identical structure.
- Label applied: yes (symbol)

### CClientSocket::ProcessPacket (key: C_CLIENT_SOCKET_PROCESS_PACKET)   [HIGH-VALUE / needs-main-review]
- v61 address: 0x0047440A (size 0xcd)
- v72 address (task-009): 0x00486922  |  v83 (cmake): 0x004965F1
- Heuristic: IDB symbol `?ProcessPacket@CClientSocket@@IAEXAAVCInPacket@@@Z`; two structural anchors — (1) **constant + call-graph**: CInPacket::Decode2 opcode then the sub-0x10 switch (0x10 OnMigrateCommand, 0x11 OnAliveReq, 0x12 OnAuthenCodeChanged, 0x13 OnAuthenMessage, 0x14 CSecurityClient::OnPacket(sub_8637C4), 0x15 sub_4747E2, else 0x1A..0x5B CWvsContext::OnPacket(g_pWvsContext=dword_974EF8) else CStage(dword_976264) vtable+8); (2) **call-graph**: **sole caller is ManipulatePacket**, sole non-trivial callee is Decode2.
- Drift v72→v61: CWvsContext opcode range **0x1A..0x5B** (was 0x1A..0x71 in v72, 0x1A..0x75 in v79).
- Chain trace (high-value): v72 0x486922 (sole caller ManipulatePacket) → v83 0x4965F1 (?ProcessPacket@CClientSocket@@, Decode2+switch — verified). v61 same role.
- Label applied: yes (symbol)
- Spot-check: sole caller ManipulatePacket(0x474336) confirms independent of the symbol.

### CClientSocket::OnConnect (key: C_CLIENT_SOCKET_ON_CONNECT)
- v61 address: 0x00472D42 (size 0x52b)
- v72 address (task-009): 0x0048528F
- Heuristic: IDB symbol `?OnConnect@CClientSocket@@QAEHH@Z` (int arg = bSuccess); two anchors — (1) **import**: `getpeername` import (0x8e53c0) xref lands here (kind 2); (2) **structure + constant**: the only fn calling ZSocketBuffer::Alloc(0x5B4) + cloned recv slot dword_97822C + the version-header(byte==8 @0x472faf)/major(==0x3D @0x472fd3-0x473009) guard trio throwing the CTerminate(0x22000007)/CPatch pair. Tail-calls Connect(sockaddr_in) on the retry/teardown path.
- Drift v72→v61: major-version compared = **0x3D (61)** (was 0x48=72 in v72, 0x4F=79 in v79). VERSION_HEADER byte still 8.
- Label applied: yes (symbol)
- Notes: disambiguated from the three Connect* variants — recv-side handler (getpeername + cloned recv + SendPacket driver), takes bSuccess.

### 8-byte client-key finding (Step 3 — feeds Task 13 / §5.6; CWvsContext.h `>83` gate, bypass/socket_hooks.cpp:310-312)
- **Verdict: ABSENT in v61 (no-key form) — confirms the `BUILD_MAJOR_VERSION > 83` gate prediction (v61 < 83 → no key, matching v72/v79).**
- Evidence (decompile of OnConnect 0x472d42, v61 binary — not a server round-trip): OnConnect's post-handshake send has two mutually-exclusive branches on `[this+9]` (m_ctxConnect.bLogin):
  - else / logged-in branch @0x4731bb: `COutPacket(20=0x14=PLAYER_LOGGED_IN); Encode4(charId=*(dword_974EF8+8328)); Encode1(0); Encode1(0); CClientSocket::SendPacket;` then returns. The encoder sequence is exactly **Encode4/Encode1/Encode1** — there is **no `EncodeBuffer(m_aClientKey, 8)`** anywhere, and CWvsContext has no m_aClientKey member below v83. v61 has neither the 16-byte machineId (v84+) nor any 8-byte client key in this path. (Minor drift vs v72: v72's 2nd Encode1 carried a TSecType bit `*(g_pWvsContext+8252)&0x80`; v61 emits a literal Encode1(0).)
  - bLogin branch @0x47315d: builds the GetExceptionFileName report (CFileStream read) and `COutPacket(25); Encode2(len); EncodeBuffer(report,len)` — unrelated to a client key (note: opcode 25 in v61 vs 26 in v72).
- Verified against the v61 binary.

### CClientSocket::ConnectLogin (key: C_CLIENT_SOCKET_CONNECT_LOGIN)
- v61 address: 0x00472A0B (size 0x179)
- v72 address (task-009): 0x00484EA5
- Heuristic: IDB symbol `?ConnectLogin@CClientSocket@@QAEXXZ`; second anchor = CWvsApp::GetCmdLine(0)/GetCmdLine(1) (0x824d80) + rand server-table pick (ZArray<long> shuffle over dword_965284 / unk_976160) → Connect(CONNECTCONTEXT)(0x472c3e). Reads g_CWvsApp(0x970a78).
- Drift v72→v61: direct. Distinct from CWvsApp::ConnectLogin (the message-pump driver) — this is the CClientSocket method it invokes.
- Label applied: yes (symbol)

### CClientSocket::Connect(CONNECTCONTEXT) (key: C_CLIENT_SOCKET_CONNECT_CTX)
- v61 address: 0x00472C3E (size 0x3d)
- v72 address (task-009): 0x004850FC
- Heuristic: IDB symbol `?Connect@CClientSocket@@QAEXABUCONNECTCONTEXT@1@@Z` (public); second anchor = its only meaningful call is the private Connect(sockaddr_in)(0x472ca3); caller is ConnectLogin(0x472a0b) (+ CWvsContext::IssueConnect).
- Drift v72→v61: direct.
- Label applied: yes (symbol)
- Notes: the CONNECTCONTEXT overload that *calls* CONNECT_ADR, not a `socket` caller.

### CClientSocket::Connect(sockaddr_in) (key: C_CLIENT_SOCKET_CONNECT_ADR)
- v61 address: 0x00472CA3 (size 0x9f)
- v72 address (task-009): 0x00485188
- Heuristic: IDB symbol `?Connect@CClientSocket@@IAEXPBUsockaddr_in@@@Z` (private) — the **sole `socket` import (0x8e53c4) caller** (`socket(2,1,0)`=AF_INET/SOCK_STREAM); second anchor = inline ClearSendReceiveCtx + closesocket([this+2]) teardown, then WSAAsyncSelect via cloned dword_9781E4(s,hwnd,1025,51) then cloned connect dword_978204(s,addr,16) tolerating WSAEWOULDBLOCK(10035), OnConnect(this,0) on the synchronous paths.
- Drift v72→v61: direct; the `socket` import xref is the most stable anchor; connect/select indirected through cloned slots in v61 just like v72. Close step inlined (no ZSocketBase::CloseSocket).
- Label applied: yes (symbol)

### ZSocketBase::CloseSocket (key: Z_SOCKET_BASE_CLOSE_SOCKET) — INLINED / SENTINEL (confirmed, same as v72)
- v61 address: 0x00000000 (was real 0x0048C699 in v79)
- v72 address (task-009): 0x00000000 (inlined)
- Heuristic: confirmed-inlined (SP-5 backward direction). No standalone function in v61: `func_query` for `CloseSocket` → empty; the close teardown is inlined into CClientSocket::Close(0x474108) and Connect(sockaddr_in)(0x472ca3) as `if([this+2]!=-1){closesocket([this+2]); [this+2]=-1;}`; `shutdown` is **not imported** (imports_query shutdown → empty), so the v61 inline close is closesocket-only (no shutdown(s,2)). closesocket import = 0x8e53dc.
- Drift v72→v61: still inlined (no discrete fn); same disposition as v72.
- Label applied: n/a
- Notes: **FLAG for gate/edit owner** — bypass/socket_hooks.cpp:324 (`pThis->m_sock.CloseSocket()`) calls Z_SOCKET_BASE_CLOSE_SOCKET as a function pointer. For v61 the consuming hook must inline the close (closesocket(m_sock._m_hSocket); _m_hSocket=-1) instead of calling 0, or its Connect-Addr reimplementation must be skipped for v61. The consuming edit must tolerate Z_SOCKET_BASE_CLOSE_SOCKET==0.

### ZSocketBuffer::Alloc (key: Z_SOCKET_BUFFER_ALLOC)
- v61 address: 0x00473D71 (size 0x4d)
- v72 address (task-009): 0x004862F8
- Heuristic: IDB symbol `?Alloc@ZSocketBuffer@@SAPAV1@IABVZAllocHelper@@@Z`; second anchor = static factory doing TWO ZAllocEx<ZAllocAnonSelector>::Alloc calls (payload size a1, then the placement ctor sub_473DBE) and called by OnConnect(0x472def) with the 0x5B4 receive-buffer size.
- Drift v72→v61: direct (the dual-alloc + 0x5B4 OnConnect caller idiom holds).
- Label applied: yes (symbol)

### CClientSocket::Close (key: C_CLIENT_SOCKET_CLOSE)
- v61 address: 0x00474108 (size 0x1d)
- v72 address (task-009): 0x0048668F
- Heuristic: IDB symbol `?Close@CClientSocket@@QAEXXZ`; second anchor = the 2-step body `ClearSendReceiveCtx(this); if([this+2]!=-1){closesocket([this+2]); [this+2]=-1;}` — the close is **inlined** (no ZSocketBase::CloseSocket call). Also called at the top of OnConnect's failure path.
- Drift v72→v61: direct (ZSocketBase::CloseSocket inlined, same as v72).
- Label applied: yes (symbol)

### CClientSocket::ClearSendReceiveCtx (key: C_CLIENT_SOCKET_CLEAR_SEND_RECEIVE_CTX)
- v61 address: 0x004748B5 (size 0x21)
- v72 address (task-009): 0x00486CF0
- Heuristic: IDB symbol `?ClearSendReceiveCtx@CClientSocket@@IAEXXZ`; second anchor = zeroes [this+26]/[this+56(word)]/[this+30] then resets the recv ([this+60]) and send ([this+80]) ZLists via sub_474CFE; called by the ctor(0x4727fb), Close, and OnConnect.
- Drift v72→v61: direct (ZList-reset helper relocated sub_48716C→sub_474CFE).
- Label applied: yes (symbol)

### COutPacket encode cluster (Task 5)

> **Key finding:** the v61 IDB retains the full mangled C++ symbols for 6 of the 7 COutPacket
> methods (ctor, Encode1/2/4, EncodeStr, EncodeBuffer) — same as v72/v79/v83 — so each carries
> its surviving IDB symbol as the primary anchor plus TWO structural anchors per the high-value
> rule. **MakeBufferList was the lone exception: unnamed (`sub_5FFCE0`) in v61** — re-identified
> structurally + by the SendPacket call edge, then renamed. All 7 are high-value / `needs-main-review`.
> The shared `COutPacket::_EnsureCapacity` callee is at **0x456D13** in v61 and its xref fan-in is
> EXACTLY the 5 grow-siblings {Encode1, Encode2, Encode4, EncodeStr, EncodeBuffer} and nothing else —
> a single call-graph anchor that corroborates all five at once.
>
> **v61 sizes vs v72/v83 (chain corroboration):** Encode1 0x1e, Encode2 0x21, Encode4 0x1f,
> EncodeStr 0x66, EncodeBuffer 0x2a, MakeBufferList 0x342 — all BYTE-IDENTICAL across v61/v72/v83.
> Only the ctor drifted: v61 0x49 vs v72/v83 0x46 (+3 bytes; different inlined alloc helper).
> v72→v83 chain confirmed in task-009 and re-confirmed here at v83 (port 13340): all 7 v83 cmake
> addresses resolve to the correctly-named COutPacket symbols with the sizes above.

### COutPacket::COutPacket (ctor)   (key: C_OUT_PACKET)   [HIGH-VALUE / needs-main-review]
- v61 address: 0x005FFC4F (size 0x49)
- v72 address (task-009): 0x00656FA1  |  v83 (cmake): 0x006EC9CE
- Heuristic: IDB symbol `??0COutPacket@@QAE@J@Z`; structural anchors — (1) `and dword [esi+4],0` (zero buf member) + `push 100h` (256-cap _Alloc via helper sub_474B76 with `lea ecx,[esi+4]`); (2) tail `call sub_5FFC98` = Init(seq). EH-prolog ctor; unwind funclet tail-calls `ZArray<uchar>::RemoveAll` (0x4748E3).
- Drift v72→v61: relocated; ctor +3 bytes (0x49 vs 0x46). **Alloc helper drifted sub_486FE4→sub_474B76; Init lost its symbol** (v72 had `?Init@COutPacket` 0x65707C; v61 = unnamed sub_5FFC98). The 256-alloc→Init shape held.
- Chain trace (high-value): v72 0x656FA1 → v83 0x6EC9CE (`??0COutPacket@@QAE@J@Z`, size 0x46 — re-confirmed). v61 same fingerprint.
- Label applied: yes (symbol present).

### COutPacket::Encode1 / Encode2 / Encode4   (keys: C_OUT_PACKET_ENCODE_1 / _ENCODE_2 / _ENCODE_4)   [HIGH-VALUE / needs-main-review]
- v61 addresses: Encode1 0x00456CF5 (0x1e), Encode2 0x0045C250 (0x21), Encode4 0x00456D52 (0x1f)
- v72 addresses (task-009): 0x004062C7 / 0x00424F84 / 0x00406324  |  v83 (cmake): 0x00406549 / 0x00427F74 / 0x004065A6
- Heuristic: IDB symbols `?Encode1@COutPacket@@QAEXE@Z` / `?Encode2@…@QAEXG@Z` / `?Encode4@…@QAEXK@Z`; structural anchors — (1) **WIDTH DISCRIMINANT, verified not transposed:** Encode1 = `push 1`+`mov [eax+ecx],dl`+`inc dword [esi+8]` (arg _BYTE); Encode2 = `push 2`+`mov [eax+ecx],dx`+`add dword [esi+8],2` (arg _WORD); Encode4 = `push 4`+`mov [eax+ecx],edx`+`add dword [esi+8],4` (arg _DWORD). Push-immediate matches store-width matches advance. (2) call-graph — all three call shared `_EnsureCapacity` (0x456D13) whose xref fan-in is EXACTLY the 5 grow-siblings {Encode1,Encode2,Encode4,EncodeBuffer,EncodeStr}.
- Drift v72→v61: all three **relocated** (Encode1/4 adjacent at 0x456Cxx–0x456Dxx; Encode2 distant at 0x45C250). Confirm by store width, never by adjacency. Shared _EnsureCapacity callee held. Sizes byte-identical to v72/v83.
- Chain trace (high-value): v72 → v83 (0x406549/0x427F74/0x4065A6, same symbols + sizes 0x1e/0x21/0x1f — re-confirmed). v61 same width-discriminant.
- Label applied: yes (symbols present).
- Notes: spot-check — the per-width store pattern + the 5-sibling fan-in identify each independently of the symbol.

### COutPacket::EncodeStr   (key: C_OUT_PACKET_ENCODE_STR)   [HIGH-VALUE / needs-main-review]
- v61 address: 0x00458C91 (size 0x66)
- v72 address (task-009): 0x00468295  |  v83 (cmake): 0x0046F3CF
- Heuristic: IDB symbol `?EncodeStr@COutPacket@@QAEXV?$ZXString@D@@@Z`; structural anchors — (1) ZXString length read `mov eax,[eax-4]` (null→`xor eax,eax`) then `add eax,2` grow before `_EnsureCapacity` (0x456D13); (2) dispatches to `CIOBufferManipulator::EncodeStr` (0x458CF7) after a `ZXString::operator=` temp, `add [esi+8],eax`, ZXString dtor in the EH unwind funclet. Also in the 5-sibling _EnsureCapacity fan-in.
- Drift v72→v61: relocated; `[eax-4]`/`+2` idiom + CIOBufferManipulator::EncodeStr callee held; size 0x66 byte-identical.
- Chain trace (high-value): v72 0x468295 → v83 0x46F3CF (same symbol, size 0x66 — re-confirmed).
- Label applied: yes (symbol present).
- Notes: spot-check — the `[eax-4]`/`+2` grow + CIOBufferManipulator::EncodeStr dispatch, independent of the symbol.

### COutPacket::EncodeBuffer   (key: C_OUT_PACKET_ENCODE_BUFFER)   [HIGH-VALUE / needs-main-review]
- v61 address: 0x00456FBA (size 0x2a)
- v72 address (task-009): 0x00465CB2  |  v83 (cmake): 0x0046C00C
- Heuristic: IDB symbol `?EncodeBuffer@COutPacket@@QAEXPBXI@Z` (void* PBX form); structural anchors — (1) takes (Src,Size), `_EnsureCapacity(Size)` (0x456D13) then `_memcpy(buf+len, Src, Size)` (import 0x876550) + `add [esi+8],edi` advance + `retn 8` (two stack args); (2) member of the 5-sibling _EnsureCapacity fan-in.
- Drift v72→v61: relocated; memcpy-grow + retn 8 shape + fan-in held; size 0x2a byte-identical.
- Chain trace (high-value): v72 0x465CB2 → v83 0x46C00C (same symbol, size 0x2a — re-confirmed).
- Label applied: yes (symbol present).

### COutPacket::MakeBufferList   (key: C_OUT_PACKET_MAKE_BUFFER_LIST)   [HIGH-VALUE / needs-main-review]
- v61 address: 0x005FFCE0 (size 0x342; **was unnamed sub_5FFCE0**, renamed `?MakeBufferList@COutPacket@@QAE?AV?$ZRef@VZSocketBuffer@@@@HW4SocketSendFlag@@@Z`)
- v72 address (task-009): 0x006570FA  |  v83 (cmake): 0x006ECB27
- Heuristic: structural anchors — (1) call-graph: it is the **sole MakeBufferList callee of CClientSocket::SendPacket** (0x474125; both xrefs to sub_5FFCE0 originate there, `more:false`); (2) constants: `v16 = 1460; if (len < 0x5B4u) v16 = len` MTU chunk + the `^0x13` payload shuffle (`*v14 = __ROR1__(v13 ^ 0x13, 3)`) + ZSocketBuffer::Alloc(0x473D71). Size 0x342 identical to v72/v83.
- Drift v72→v61: relocated; **lost its IDB symbol** (v72/v83 retained a MakeBufferList symbol; v61 had `sub_5FFCE0`). Re-identified by the SendPacket call edge + the 1460/0x5B4 chunk + ^0x13 shuffle, then renamed to the v72-form canonical symbol for grep alignment. Note: v72 and v83 carry **different** MakeBufferList mangled signatures (v72 `QAE?AV?$ZRef@…@HW4SocketSendFlag@@@Z` vs v83 `QBEXAAV?$ZList@…@GPAKHK@Z`); applied the v72 form (closest anchor + task-009 lineage).
- Chain trace (high-value): v72 0x6570FA → v83 0x6ECB27 (`?MakeBufferList@COutPacket@@…`, size 0x342 — re-confirmed). v61 same role + constants.
- Label applied: yes (renamed sub_5FFCE0 → canonical symbol).
- Notes: spot-check — the 1460/0x5B4 MTU chunk + ^0x13 shuffle + SendPacket call edge identify it independent of any symbol (there was none to trust).

### Cluster 3 — CConfig / windowed-mode (Task 8)

> All CConfig method symbols are present (mangled) in v61. The ctor symbol `??0CConfig@@QAE@XZ`
> was NOT present (was `sub_47921D`); relabeled this task. Two structural anchors confirm each.

#### CConfig::CConfig ctor (key: C_CONFIG)
- v61 0x0047921D (v72 0x48C0D3). Was `sub_47921D` → relabeled `??0CConfig@@QAE@XZ`.
- Anchor 1: SBB-singleton store `dword_974ED4 = (a1+1!=0) ? a1 : 0` (g_CConfig_pInstance) + vtable `off_8E6760` at `*a1`.
- Anchor 2: `memset(this+0x1BC, 0, 0x1BD)` (distinctive 0x1BD size) + StringPool(2497) + RegOpenKeyExA(HKLM 0x80000002 via dword_978168).
- Drift v72→v61: StringPool 2497 (v72 2530); memset offset this+0x1BC (v72 0x234); size 0x1BD ported directly.

#### g_CConfig_pInstance (key: C_CONFIG_INSTANCE_ADDR)
- v61 0x00974ED4 (v72 0xAA3AC0). Stored in ctor (above); returned by `TSingleton<CConfig>::GetInstance`(0x4EFFB6). Renamed from `dword_974ED4`.

#### CConfig::GetPartnerCode (key: C_CONFIG_GET_PARTNER_CODE)
- v61 0x00564566 (v72 0x5B12BD). Symbol `?GetPartnerCode@CConfig@@`.
- Anchor 1: string `uiWndZ0` (aUiwndz0 @0x962620). Anchor 2: GetOpt_Int(0,key,0,0x80000000,0x7FFFFFFF) via sub_47B793. Ported directly.

#### CConfig::ApplySysOpt (key: C_CONFIG_APPLY_SYS_OPT)
- v61 0x0047B28E (v72 0x48E7EC). Symbol `?ApplySysOpt@CConfig@@`.
- Anchor 1: qmemcpy(this+0x58, a2, 0x30) + dual CWvsContext flag writes `dword_974EF8 + 13088/13092` (v==1||3 / v==2||3).
- Anchor 2: `100*(x+1)/20` volume formula → SetBGMVolume/SetSEVolume(dword_974ED0) + InputSystemInstanceAddr+2416.
- Drift: flag offsets 13088/13092 (v72 13572/13576).

#### CConfig::CheckExecPathReg (key: C_CONFIG_CHECK_EXEC_PATH_REG)
- v61 0x00479B4D (v72 0x48CBAE). Symbol `?CheckExecPathReg@CConfig@@`.
- Anchor 1: StringPool(3071/3072) + 0x5C backslash char (92) + Right(1)/Left compare.
- Anchor 2: GetFileAttributes(dword_977F04; `==-1 || &0x10`) + strcmp + GetOpt_String/SetOpt_String.
- Drift: StringPool 3071/3072 (v72 3109/3110); reg-handle gate this[38] (v72 this+0xBC).

#### g_CConfig_SysOpt_WindowedMode (key: C_CONFIG_SYS_OPT_WINDOWED_MODE)
- v61 0x00978E24 (v72 0xAA87AC). Renamed from `dword_978E24`. Standalone global (NOT a config-struct member).
- Anchor 1: CreateMainWindow(0x8239D0) window-style branch `dword_978E24 != 0 ? 0x80000000 : 0x80000` + exStyle `?8:0`.
- Anchor 2: read in InitializeGr2D(0x824550) `pvargSrc.lVal = dword_978E24`.
- Note: only 2 read sites in v61 (v72 had 3 incl. SetUp). Feeds Task 13.

### Cluster 3b — IGCipher / SystemInfo / sync / ZXString / ZArray / CMob utils (Task 8)

#### GetSEPrivilege (key: GET_SE_PRIVILEGE) — NEW v61-ONLY SENTINEL (absent)
- v61 0x00000000 (was real v72 0x44989E). Confirmed absent both directions (SP-5):
  - v61: `find_regex "Privilege"` → 0 strings; `imports_query *Token*/*Privilege*` → 0; advapi32 import table fully enumerated (22 entries) lacks OpenProcessToken/LookupPrivilegeValueA/AdjustTokenPrivileges.
  - v72 (cross-check, port 13343): string "SeDebugPrivilege"@0xA5A564 PRESENT; OpenProcessToken/AdjustTokenPrivileges/LookupPrivilegeValueA imports PRESENT; GetSEPrivilege@0x44989E (size 0x6A) PRESENT.
- Debug-privilege escalation post-dates v61. **FLAGGED for gate/edit owner** — consuming edit must tolerate 0.

#### CIGCipher::innoHash (key: C_IG_CIPHER_INNO_HASH)
- v61 0x0086274C (v72 0x940D7E). Symbol `?innoHash@CIGCipher@@`.
- Anchor 1: no-key seed `0xC65053F2` (v6 = -967814158) — identical to v72 (v79 differs: 0xC6EF3720).
- Anchor 2: bShuffle per-byte loop sub_862787. Ported directly.

#### ZSynchronizedHelper<ZFatalSection> ctor/dtor (keys: ..._CTOR / ..._DTOR)
- CTOR v61 0x00402ABA (v72 0x402AB8, +2 bytes). Symbol present. Anchors: acquire-loop calling off_966D68 (TryEnter) + Sleep(0) retry via dword_977EAC.
- DTOR v61 0x00402ADF (= ctor+0x25; v72 0x402ADD). Body `mov eax,[ecx]; dec dword[eax+4]; jnz; and dword[eax],0; retn` (RAII release of recursion count). Listing aliases under ZAllocEx::Alloc (dump aliasing) — not renamed.

#### CSystemInfo ctor + Init + GetGameRoomClient + GetMachineId (keys: C_SYSTEM_INFO[_INIT/_GET_GAME_ROOM_CLIENT/_GET_MACHINE_ID])
- ctor v61 0x008658E0 (v72 0x94A6C0): `mov [eax], off_8F0E44; retn` (vtable install). Symbol present.
- Init v61 0x00865920 (v72 0x94A700): Anchor 1 = Netbios MAC (ncb_command 0x37/0x32/0x33) + GetVolumeInformationA. Anchor 2 = RegOpenKeyExA(HKLM, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion") + "CxSupportId" value (16B) + CoCreateGuid fallback. Ported directly.
- GetGameRoomClient v61 0x00865D00 (v72 0x94AAE0): symbol + 0x11B4 process-table fn + call-graph adjacency (consecutive, same class) + SendCheckPasswordPacket caller.
- GetMachineId v61 0x00865C00 (v72 0x94A9E0): `lea eax,[ecx+0x14]; retn` returns cached 16-byte id (stored at this+0x14 in Init). Symbol + body.

#### ZArray<E>::RemoveAll (key: Z_ARRAY_REMOVE_ALL)
- v61 0x0045DBB3 (v72 0x425CEC). Symbol `?RemoveAll@?$ZArray@E@@`.
- Anchor 1: `if(*this){ZAllocEx::Free(*this-4); *this=0}` shape. Anchor 2: byte-stride (`@E@` = unsigned char element), `*this-4` header back-step.
- Drift: v61 ZAllocEx::Free is single-arg (v72 passed selector unk_AA7CB8).

#### ZXString<char>::_Cat / TrimRight / TrimLeft (keys: Z_X_STRING_GET_BUFFER / _TRIM_RIGHT / _TRIM_LEFT)
- _Cat v61 0x0045DFA4 (v72 0x425D2B): empty→GetBuffer(Size,0)+memcpy (==assign); non-empty→capacity grow (`v8*=2`)+append. needs-main-review (no dedicated pure-assign; repo calls on fresh ZXStrings only).
- TrimRight v61 0x0045DD5B (v72 0x46C9B4): default whitespace `asc_96156C " \t\r\n"` + strchr from tail + inner GetBuffer(0x415000).
- TrimLeft v61 0x0045DE10 (v72 0x46CA69): same `asc_96156C` literal + strchr from head + memcpy-shift remainder to front; adjacent right after TrimRight. All symbol + body confirmed.

#### CMob::CMob ctor (key: C_MOB_C_MOB) [needs-main-review — DOOM-FIX HOOK TARGET]
- v61 0x005C2128 (v72 0x611CDB). Symbol `??0CMob@@QAE@PAVCMobTemplate@@@Z`.
- Anchor 1: sole caller `CreateMob`(0x5C20EC) does `ZAllocEx::Alloc(1168)` (0x490, non-zeroing) then CMob::CMob.
- Anchor 2: 3 vtables off_8E9AE8/8E9AC4/8E9AC0 at this+0/1/2 + m_pTemplate@this+0x15C + 31/100/24 fuse (this+282/284/285) + _ZtlSecureTear chain + MobStat::SetFrom(this+368) + StringPool(942)/PcCreateObject<IWzCanvas> tail@this+0x45C.
- Drift v72→v61: alloc 1168 (v72 1216, v79 1304); StringPool 942 (v72 958); m_pTemplate this+0x15C (v72 0x160).
- **DOOM finding (feeds Task 15 gate audit):** the `m_bDoomReserved` / doom-tail field **DOES NOT EXIST in v61.** Struct is only 0x490 bytes (even smaller than v72's 0x4C0). The v84 doom field sits at 0x540/0x544 — far past the v61 struct end. The ctor's highest write is this+0x47C (a field) and the IWzCanvas com_ptr at this+0x45C; there is **NO write anywhere near 0x540**, and no doom field can exist (past allocation). So v61 lands on the doom-fix `< 84` "needs-fix" side, identical disposition to v72/v79 (field absent, not merely uninitialized).

### Cluster 4 — Login / Stage / Logo / Title (Task 6)

> **CLogo vtables (v61).** The CLogo ctor (0x5950F4) writes 4 vtable pointers — SAME shape as
> v72/v79: [this]=primary IStage `off_8E9668`, [this+4]=IUIMsgHandler `off_8E961C`,
> [this+8]=IStage::OnPacket `off_8E9618`, [this+0Ch]=`off_8E9614`. Base ctor sub_45BE8F
> installs the CStage CWnd-rooted vtable (off_8E65E4) before the override. **Slot order matches
> v72 with NO drift**: primary {slot0=Update 0x595670, slot1=Init 0x5951EC, slot2=ForcedEnd 0x59525A};
> IUIMsgHandler {slot0=OnKey 0x595634, slot1=OnSetFocus 0x595132, slot2=OnMouseButton 0x59565B};
> OnPacket {slot0=CStage::OnPacket 0x659F99}.
>
> **GetRTTI/IsKindOf slot-order DRIFT (FR-14).** Unlike v72 (which retained the mangled
> `?GetRTTI@CLogo@@`/`?IsKindOf@CLogo@@` symbols in a distant stub region 0x42156x), v61 **strips
> these two symbols entirely** (func_query for GetRTTI/IsKindOf returns 0 binary-wide) and places
> them **inline in the CLogo translation-unit block**: GetRTTI=0x595138 (`mov eax, offset 0x975FD4; retn`)
> and IsKindOf=0x59513E (= GetRTTI+6, CRTTI-chain walk on the same descriptor 0x975FD4), immediately
> after CLogo::OnSetFocus (0x595132). The CLogo CRTTI descriptor 0x975FD4 is a content-less zero
> global (no name string, like v72's dword_AA4D2C) referenced only by the CLogo descriptor-init thunk
> sub_5950E9 (just before the ctor) + this GetRTTI/IsKindOf pair. They are NOT runtime-vtable slot0/1
> of the CLogo object (its [this+0] is the IStage Update vtable); they live in a CWnd-rooted vtable
> shared with CLogo subclasses (same situation as v72's 0x9cf9e4/0x9daed8 blocks).
>
> **Shared-Update verdict (Step-2 note).** In v61 `C_LOGO_UPDATE` (0x595670, CLogo primary slot0,
> init-once timer body) and `C_LOGIN_UPDATE` (0x562EDC, CLogin primary slot0, login-form body) are
> **DISTINCT functions → DIVERGED**, same as v72/v79. The v83 coincidence (both = 0x5F4C16) does NOT
> hold below v83. Verified independently (separate vtables, separate bodies).
>
> **No v61-only sentinels in this cluster** — NX logo, all CStage handlers, the title manager, the GR
> singleton all exist in v61.

#### CLogin::Update   (key: C_LOGIN_UPDATE)
- v61 address: 0x00562EDC  |  v72 (task-009): 0x005AFBBE
- Heuristic: two anchors — (1) CLogin primary vtable `off_8E8C44` slot0 (CLogin ctor 0x5620D4 installs it); (2) body tail-dispatches to CLogin::SendCheckPasswordPacket (0x564418) + MakeVACDlg/ResetVAC/RemoveNoticeConnecting (all CLogin methods), reads CUITitle singleton (dword_975FC0), and InvalidateRect-s the form CWnd at this+408.
- Drift v72→v61: relocated; **InvalidateRect is now a DIRECT `CWnd::InvalidateRect` (0x813331) call** (v72 used the indirect func-ptr dword_AA7624); member offsets shifted (this+316/320/408). DIVERGES from C_LOGO_UPDATE — same verdict as v72.
- Label applied: yes (renamed `CLogin__Update`).

#### CLogin::SendCheckPasswordPacket   (key: C_LOGIN_SEND_CHECK_PASSWORD_PACKET)
- v61 address: 0x00564418  |  v72 (task-009): 0x005B1170
- Heuristic: two anchors — (1) IDB symbol `?SendCheckPasswordPacket@CLogin@@QAEHPBD0@Z`; (2) `COutPacket::COutPacket(pkt, 1)` (opcode/seq=1) then EncodeStr(user)+EncodeStr(pass) (0x458C91) + EncodeBuffer(CSystemInfo::GetMachineId,16) + Encode4(GameRoomClient)+Encode1×3+Encode4(GetPartnerCode) + `CClientSocket::SendPacket(g_pClientSocketInstance, pkt)` = the resolved send **0x474125**.
- Drift v72→v61: relocated; **opcode immediate is 1 (== v72; was 0x05 in v79)** — read, not copied. EncodeStr-pair + SendPacket(0x474125) structure held. Cross-confirms the CUITitle global (dword_975FC0) + the socket send key.
- Label applied: yes (symbol present).

#### CLogo::CLogo (ctor)   (key: C_LOGO)
- v61 address: 0x005950F4 (size 0x3e)  |  v72 (task-009): 0x005E11F9
- Heuristic: two anchors — (1) IDB symbol `??0CLogo@@QAE@XZ`; (2) 4 consecutive vtable-pointer stores ([this]=off_8E9668, +4=off_8E961C, +8=off_8E9618, +0Ch=off_8E9614) after base ctor sub_45BE8F + the member-zero block; allocated by CLogo::LogoEnd (0x59527C) Alloc+ctor.
- Drift v72→v61: relocated; 4-vtable-write shape held identical to v72.
- Label applied: yes (symbol present).

#### CLogo::GetRTTI   (key: C_LOGO_GET_RTTI)
- v61 address: 0x00595138 (size 0x6)  |  v72 (task-009): 0x00421565
- Heuristic: two anchors — (1) body `mov eax, offset 0x975FD4 (CLogo CRTTI descriptor); retn` returning the same descriptor IsKindOf walks; (2) located inline in the CLogo TU block immediately after CLogo::OnSetFocus (0x595132) and adjacent to IsKindOf (0x59513E), sharing descriptor 0x975FD4 (also referenced by the CLogo descriptor-init thunk sub_5950E9 right before the CLogo ctor 0x5950F4).
- Drift v72→v61: **symbol stripped in v61** (v72 had `?GetRTTI@CLogo@@UBEPBVCRTTI@@XZ`); relocated from the distant RTTI-stub region (v72 0x42156x) to **inline in the CLogo block** (slot-index/placement drift). Located by byte-sig + descriptor linkage, then relabeled to the v72-form canonical symbol.
- Label applied: yes (relabeled `?GetRTTI@CLogo@@UBEPBVCRTTI@@XZ`).

#### CLogo::IsKindOf   (key: C_LOGO_IS_KIND_OF)
- v61 address: 0x0059513E (size 0x21)  |  v72 (task-009): 0x0042156B
- Heuristic: two anchors — (1) the CRTTI base-chain walk body (`v2=&desc; while(v2!=arg){v2=*v2; if(!v2)return 0;} return 1`) on descriptor 0x975FD4 — the SAME descriptor GetRTTI returns; (2) = GetRTTI+6, the adjacent stub pair in the CLogo TU block. Body byte-sig matched the wildcarded v72 IsKindOf pattern.
- Drift v72→v61: symbol stripped; inline placement drift (same as GetRTTI). Descriptor-chain-walk + shared 0x975FD4 + GetRTTI adjacency held.
- Label applied: yes (relabeled `?IsKindOf@CLogo@@UBEHPBVCRTTI@@@Z`).

#### CLogo::Update   (key: C_LOGO_UPDATE)
- v61 address: 0x00595670  |  v72 (task-009): 0x005E1789
- Heuristic: two anchors — (1) CLogo primary vtable `off_8E9668` slot0; (2) init-once timer body `cmp [esi+24h],0 → call dword_9781C4 (GetUpdateTime) → mov [esi+24h],eax`, then elapsed-time logo body (cmp 0x5DC).
- Drift v72→v61: relocated; timer field [esi+0x24] held (== v72). Was an undefined function — `define_func` before rename. DIVERGES from C_LOGIN_UPDATE.
- Label applied: yes (renamed `CLogo__Update`).

#### CLogo::OnMouseButton   (key: C_LOGO_ON_MOUSE_BUTTON)
- v61 address: 0x0059565B  |  v72 (task-009): 0x005E1774
- Heuristic: two anchors — (1) CLogo IUIMsgHandler vtable `off_8E961C` slot2; (2) body `cmp [esp+arg_0], 202h (WM_LBUTTONUP) → add ecx,-4 (thiscall adjust) → call CLogo::InitNXLogo (0x5952C6)`.
- Drift v72→v61: relocated; 0x202 + InitNXLogo path held identical.
- Label applied: yes (renamed `CLogo__OnMouseButton`).

#### CLogo::OnSetFocus   (key: C_LOGO_ON_SET_FOCUS)
- v61 address: 0x00595132  |  v72 (task-009): 0x005E1237
- Heuristic: two anchors — (1) CLogo IUIMsgHandler vtable `off_8E961C` slot1; (2) body `push 1; pop eax; retn 4` (always-true stub).
- Drift v72→v61: relocated; identical stub shape.
- Label applied: yes (renamed `CLogo__OnSetFocus_IUI`).

#### CLogo::OnKey   (key: C_LOGO_ON_KEY)
- v61 address: 0x00595634  |  v72 (task-009): 0x005E174D
- Heuristic: two anchors — (1) CLogo IUIMsgHandler vtable `off_8E961C` slot0; (2) body `test byte [esp+0Bh],80h (key-up filter) → cmp [esp+4],0Dh/1Bh/20h (13/27/32) → add ecx,-4 → call CLogo::InitNXLogo (0x5952C6)`.
- Drift v72→v61: relocated; the 13/27/32 + InitNXLogo cluster held. Was an undefined function — `define_func` before rename.
- Label applied: yes (renamed `CLogo__OnKey`).

#### CLogo::LogoEnd   (key: C_LOGO_LOGO_END)
- v61 address: 0x0059527C  |  v72 (task-009): 0x005E1381
- Heuristic: two anchors — (1) call-graph fingerprint `ZAllocEx::Alloc(476) → CLogin::CLogin (0x5620D4) → set_stage (0x65B22A)` (sole such caller of the CLogin ctor + set_stage); (2) reached as the conditional skip-logo tail of CLogo::Init (0x5951EC).
- Drift v72→v61: relocated; **CLogin alloc size 0x1DC (476) — SMALLER than v72 0x23C (572) and v79 0x258 (600)**. set_stage callee confirmed independently (0x65B22A).
- Label applied: yes (renamed `CLogo__LogoEnd`).

#### CLogo::ForcedEnd   (key: C_LOGO_FORCED_END)
- v61 address: 0x0059525A  |  v72 (task-009): 0x005E135F
- Heuristic: two anchors — (1) CLogo primary vtable `off_8E9668` slot2; (2) body `CSoundMan::PlayBGM(...,1000,...)` + nullsub_89 tail.
- Drift v72→v61: relocated; PlayBGM(1000=0x3E8) + nullsub-tail idiom held.
- Label applied: yes (renamed `CLogo__ForcedEnd`).

#### CLogo::Init   (key: C_LOGO_INIT)
- v61 address: 0x005951EC  |  v72 (task-009): 0x005E12F1
- Heuristic: two anchors — (1) CLogo primary vtable `off_8E9668` slot1; (2) body sub-init (0x595726) → `CInputSystem::ShowCursor(dword_975050, 0)` → `CWvsApp::GetCmdLine(3)`; if cmdline non-empty AND g_CWvsApp[+0x28]!=0 tail-calls LogoEnd (0x59527C) skip-logo shortcut.
- Drift v72→v61: relocated; ShowCursor(0)+GetCmdLine(3)+conditional-LogoEnd shape held. CInputSystem singleton = dword_975050 (cross-confirms the real v61 InputSystem global — note the seed C_INPUT_SYSTEM_INSTANCE_ADDR is still the v72 value 0xAA3E84, to be corrected in Task 7).
- Label applied: yes (renamed `CLogo__Init`).

#### CLogo::InitNXLogo   (key: C_LOGO_INIT_NX_LOGO)
- v61 address: 0x005952C6  |  v72 (task-009): 0x005E13CB
- Heuristic: two anchors — (1) init-once guard `this[10]==0` (=[this+0x28]) → GetUpdateTime (dword_9781C4) store-back → `StringPool::GetBSTR(&unk_9787F0, …, 1380)` NX-logo resource → IWzResMan::GetObjectA sprite load; (2) callers = CLogo::OnKey (0x595634) + CLogo::OnMouseButton (0x59565B).
- Drift v72→v61: relocated; **StringPool ID drifted to 1380 (0x564)** (v72 1386/0x56A, v79 0x568). Guard at [this+0x28] held. Verify via the resolved resource, not the numeric ID, in older builds.
- Label applied: yes (renamed `CLogo__InitNXLogo`).

#### Stage singleton   (key: STAGE_INSTANCE_ADDR)
- v61 address: 0x00976264  |  v72 (task-009): 0x00AA54D4
- Heuristic: two anchors — (1) store/clear by `set_stage` (0x65B22A) — `dword_976264 = 0` at entry, new stage installed via sub_65B4A9, then dispatched `(*(*dword_976264+4))(dword_976264, a2)` = Init (vtable+4); (2) read by `CWvsApp::CallUpdate` (0x82490A) and `CClientSocket::ProcessPacket` (0x47440A) stage dispatch (xrefs_to confirms the writer/reader split).
- Drift v72→v61: relocated into the v61 .data window (0x97xxxx); singleton store/clear pattern held.
- Label applied: yes (renamed `StageInstanceAddr`).

#### set_stage   (key: SET_STAGE)
- v61 address: 0x0065B22A (size 0x18a)  |  v72 (task-009): 0x006C1FBB
- Heuristic: two anchors — (1) IDB symbol `?set_stage@@YAXPAVCStage@@PAX@Z`; (2) clears STAGE_INSTANCE_ADDR (0x976264), calls old-stage ForcedEnd (vtable+8), installs new stage (sub_65B4A9), calls new-stage Init (vtable+4 via dword_976264). Also the sole set_stage caller from CLogo::LogoEnd.
- Drift v72→v61: relocated; **v61 symbol is the free function `set_stage` (lowercase, like v72)**. Store+ForcedEnd+Init triple held.
- Label applied: yes (symbol present).

#### GR singleton   (key: GR_INSTANCE_ADDR)
- v61 address: 0x00978D34  |  v72 (task-009): 0x00AA85FC
- Heuristic: two anchors — (1) output-arg store — `CWvsApp::InitializeGr2D` (0x824550) calls factory `sub_826D55(res, &dword_978D34, 0)` which writes the IWzGr2D through the pointer; (2) consumed as IWzGr2D — CreateCanvas 800×600 (vtable+12) + SetColor(0xFF000000) (vtable+76) in InitializeGr2D.
- Drift v72→v61: relocated; factory drifted to sub_826D55 (v72 sub_8F7257); output-arg→store→IWzGr2D-dispatch chain held.
- Label applied: yes (renamed `GrInstanceAddr`).

#### CStage::OnMouseEnter   (key: C_STAGE_ON_MOUSE_ENTER)
- v61 address: 0x00659F7A (size 0x1f)  |  v72 (task-009): 0x008DF289
- Heuristic: two anchors — (1) body `if(arg) if([CInputSystem singleton dword_975050 + 0x9B4 (2484)]) CInputSystem::SetCursorState(dword_975050, 0); retn 4` — the SetCursorState-on-singleton-with-+0x9B4-guard fingerprint; (2) vtable witness — appears as an inherited CStage virtual in CLogin's IUIMsgHandler vtable `off_8E8BF8` slot 5 (get_bytes = 0x659F7A).
- Drift v72→v61: **symbol stripped in v61** (v72 had `?OnMouseEnter@CStage@@UAEXH@Z`); relocated next to the CStage OnPacket cluster (0x659xxx). SetCursorState (0x525501) + [+0x9B4] guard body held. The CLogo block overrides OnMouseEnter, so use a non-overriding CStage subclass (CLogin) vtable as the witness (same subtlety task-008/009 flagged).
- Label applied: yes (relabeled `?OnMouseEnter@CStage@@UAEXH@Z`).

#### CStage::OnPacket   (key: C_STAGE_ON_PACKET)
- v61 address: 0x00659F99 (size 0x3a)  |  v72 (task-009): 0x006C0C61
- Heuristic: two anchors — (1) IDB symbol `?OnPacket@CStage@@UAEXJAAVCInPacket@@@Z`; (2) = CLogo OnPacket vtable `off_8E9618` slot0 (get_bytes confirmed).
- Drift v72→v61: relocated; symbol + CLogo-vtable cross-ref both held.
- Label applied: yes (symbol present).

#### CUITitle singleton   (key: C_UI_TITLE_INSTANCE_ADDR)
- v61 address: 0x00975FC0  |  v72 (task-009): 0x00AA5114
- Heuristic: two anchors — (1) read as CUITitle* by CLogin::SendCheckPasswordPacket (0x564418) — `(*(*(dword_975FC0+4)+52))(dword_975FC0+4)` control dispatch; (2) the broad CLogin login-control fan-in via xrefs_to — OnCheckPasswordResult, OnGuestIDLoginResult, OnCheckPinCodeResult, and the CLogin ctor/teardown helpers (sub_562452/sub_56363B).
- Drift v72→v61: relocated into the v61 .data window; the CUITitle-singleton read/dispatch pattern held.
- Label applied: yes (renamed `UITitleInstanceAddr`).

### Cluster 5 — Manager singletons (Task 7)

**Static-init walk.** Every `*::CreateInstance` is reached from `CWvsApp::SetUp` (v61 0x823175,
fully symbol-decompiled). The TSingleton<T>::CreateInstance template instantiations cluster at
v61 0x825c78–0x826124 (right after WinMain/ctor/SetUp). Each `CreateInstance` reads/stores its
instance global and tail-calls the ctor (Alloc(size)+ctor). SetUp call-order:
CSecurityClient → CClientSocket → CFuncKeyMappedMan → CQuickslotKeyMappedMan → (InitializeInput:
CInputSystem) → CActionMan → CAnimationDisplayer → CMapleTVMan → CQuestMan → CMonsterBookMan.
Anchor 1 for every CreateInstance/global = its SetUp call-site (call-graph); anchor 2 = the
TSingleton symbol + Alloc-size/ctor body. Cross-checks proving v61 lane: ISMsgProc(0x825094),
ProcessPacket default range 0x1A..0x5B, CClientSocket::CreateInstance(0x825ff3) all match the
already-relocated v61 keys.

#### ActionMan   (keys: C_ACTION_MAN_CREATE_INSTANCE_ADDR / _INSTANCE_ADDR / _INIT / _SWEEP_CACHE)
- CreateInstance 0x00825F46 (v72 0x8F6172); instance global ActionManInstanceAddr 0x00970830 (v72 0xA9F3F4); Alloc(672)+ctor 0x405b79.
- Init 0x00405EFE (sym ?Init@CActionMan@@; SetUp@0x8232ac on CreateInstance result). SweepCache 0x0040F179 (sym ?SweepCache@CActionMan@@; xrefs_to → sole caller CallUpdate 0x82490a@0x824a9f).
- Drift v72→v61: all relocated; symbols + call-graph held.

#### AnimationDisplayer   (key: C_ANIMATION_DISPLAYER_CREATE_INSTANCE)
- 0x00825F9C (v72 0x8F61C8); Alloc(408)+ctor 0x4271c4; instance dword_974EAC; SetUp@0x8232b1.

#### FuncKeyMappedMan / Quickslot   (keys: C_FUNC_KEY_MAPPED_MAN / _VFTABLE / _INSTANCE_ADDR / _CREATE_INSTANCE / DEFAULT_FKM / DEFAULT_QKM / C_QUICKSLOT_KEY_MAPPED_MAN)
- ctor C_FUNC_KEY_MAPPED_MAN 0x0051AA0E (sym ??0CFuncKeyMappedMan@@); installs vftable off_8E7F58 (C_FUNC_KEY_MAPPED_MAN_VFTABLE 0x008E7F58), stores instance FuncKeyMappedManInstanceAddr 0x00975CA0, memcpy DefaultFKM(0x00962138) 0x1BD ×2.
- CreateInstance 0x00826038 (v72 0x8F6264); Alloc(0x388=904) — v61 size == v72.
- DEFAULT_FKM_INSTANCE_ADDR 0x00962138: two xrefs (ctor 0x51aa25 + DefaultFuncKeyMap@CFuncKeyMappedMan 0x51addf).
- DEFAULT_QKM_INSTANCE_ADDR = 0 (carried sentinel): ctor zeroes quickslot region (this+224/+225=0), no QKM memcpy — same as v72. FLAGGED.
- C_QUICKSLOT_KEY_MAPPED_MAN 0x0082608E (v72 0x8F62BA); Alloc(0x30=48)+ctor 0x5972e9; instance dword_974EE4; SetUp@0x823200.

#### InputSystem   (keys: C_INPUT_SYSTEM / _CREATE_INSTANCE / _INSTANCE_ADDR / _INIT / _UPDATE_DEVICE / _GET_IS_MESSAGE / _GENERATE_AUTO_KEY_DOWN / _SHOW_CURSOR)
- Reached via CWvsApp::InitializeInput (v61 0x8247c3). ctor C_INPUT_SYSTEM 0x00824817; CreateInstance 0x00825C20; Init 0x00524CEB; ShowCursor 0x00525162 (all symbol-named).
- **C_INPUT_SYSTEM_INSTANCE_ADDR = 0x00975050 (CORRECTION).** v72 seed 0xAA3E84 was STALE/WRONG for v61. Real v61 singleton = dword_975050, confirmed three ways: read/stored in CreateInstance(0x825c20); C_STAGE_ON_MOUSE_ENTER reads [dword_975050+0x9B4]; CLogo::Init calls ShowCursor(dword_975050). (Task 6 originally surfaced this.)
- Three unnamed methods found by Run(0x8233CC) position + body (renamed this task): UPDATE_DEVICE 0x00525113 (Run <=2 branch; if(!a1)UpdateKeyboard else if(a1==1)UpdateMouse); GET_IS_MESSAGE 0x00525130 (inner ISMsgProc loop; this[625] gate, copies 3 dwords from this[626]); GENERATE_AUTO_KEY_DOWN 0x005260FA (else-branch before CSecurityClient::Update; *a2=256 + GetSpecialKeyFlag). All bodies match v72 structure.

#### MapleTVMan   (keys: C_MAPLE_TV_MAN_CREATE_INSTANCE / _INSTANCE_ADDR / _INIT) — PRESENT in v61
- CreateInstance 0x00826124 (v72 0x8F6353); Alloc(944)+ctor 0x59b913; instance MapleTVManInstanceAddr 0x00975DDC; Init 0x0059BB27 (sym; SetUp@0x8232bd). NOT a backward sentinel — confirmed present.

#### MonsterBookMan   (keys: C_MONSTER_BOOK_MAN_CREATE_INSTANCE / _INSTANCE_ADDR / _LOAD_BOOK) — PRESENT in v61
- CreateInstance 0x00825D13 (v72 0x8F5F3F); Alloc(164)+ctor 0x825d58; instance 0x00975CF8; LoadBook 0x005DD687 (sym; SetUp@0x8232f9). Confirmed present.

#### QuestMan   (keys: C_QUEST_MAN_CREATE_INSTANCE / _INSTANCE_ADDR / _LOAD_DEMAND / _LOAD_PARTY_QUEST_INFO / _LOAD_EXCLUSIVE)
- CreateInstance 0x00825C78 (v72 0x8F5E99); Alloc(412)+ctor 0x628c28; instance 0x00975DFC; LoadDemand 0x00629040 (sym; SetUp@0x8232c9).
- **LOAD_PARTY_QUEST_INFO = 0 and LOAD_EXCLUSIVE = 0 — NEW v61-ONLY SENTINELS** (were real v72 0x6887DE / 0x689C3E). Absence evidence: func_query name_regex "PartyQuest"/"Exclusive" → 0 results across all of v61; full *CQuestMan* symbol set lacks both; v61 SetUp does NOT call them (v72 SetUp did). Features post-date v61. FLAGGED for gate/edit owner.

#### MacroSysMan / RadioManager — ABSENT in v61 (carried sentinels, also absent v72)
- C_MACRO_SYS_MAN_CREATE_INSTANCE = 0: no CMacroSysMan symbol/func/string; macro role folded into CWvsContext::OnMacroSysDataInit (0x849bce). FLAGGED.
- C_RADIO_MANAGER_CREATE_INSTANCE / _INSTANCE_ADDR = 0: no CRadioManager symbol/func/string; not in SetUp chain; radio folded into CMapleTVMan. **Radio-quirk:** v72 seed already 0, so the v83 allocator-selector trap (0xBF0B00) never applied — nothing wrong inherited; v61 also 0. FLAGGED.

#### SecurityClient   (keys: C_SECURITY_CLIENT_CREATE_INSTANCE / _INSTANCE_ADDR / _ON_PACKET)
- CreateInstance 0x008260E2 (v72 0x8F630E); Alloc(88)+ctor 0x86333d; instance SecurityClientInstanceAddr 0x0097085C (also used in SetUp@0x8231ec *(g+56) + Run CSecurityClient::Update sub_86354E@0x8234ed).
- C_SECURITY_CLIENT_ON_PACKET 0x008637C4 (unnamed in v61, renamed OnPacket_CSecurityClient): reached from ProcessPacket(0x47440a) case 0x14; body Decode1 → integrity-handler dispatch (0/2/3). **needs-main-review** (security_hooks boundary). Dispatch shape differs from v72 (v61 switches 0/2/3, v72 cmp==4) — relocated by call-site role, not byte match.

### Cluster 6 — Party / migrate senders (Task 9)

*(pending)*

### Cluster 7 — Misc utils / exception dispatch (Task 10)

*(pending)*

### Cluster 8 — CFileStream relay (Task 11)

*(pending)*

### Cluster 9 — Gate confirm/split (Task 12)

*(pending)*
