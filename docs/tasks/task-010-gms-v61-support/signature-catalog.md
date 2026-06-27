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

### Cluster 3 — CConfig / windowed-mode (Task 6)*

*(pending)*

### Cluster 4 — CLogin / CLogo / stage flow (Task 7)

*(pending)*

### Cluster 5 — Manager singletons (Task 8)

*(pending)*

### Cluster 6 — Party / migrate senders (Task 9)

*(pending)*

### Cluster 7 — Misc utils / exception dispatch (Task 10)

*(pending)*

### Cluster 8 — CFileStream relay (Task 11)

*(pending)*

### Cluster 9 — Gate confirm/split (Task 12)

*(pending)*
