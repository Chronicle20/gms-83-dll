# GMS v79 — Function-Identification Signature Catalog

**Purpose.** v79 is not the last version we will port this way — and as the first
*below-floor* port it is the template for porting even older builds. For each
non-trivial function we relocate, this catalog records *how* we found it in a
sparsely-labeled v79 IDB so the next (possibly older) port can reuse the heuristic
instead of re-discovering it. Prefer version-stable anchors (strings, imports,
call-graph, constants) over raw byte signatures, which break when codegen
changes — and the v79↔v83 gap is larger than the v83↔v84 gap, so byte sigs are
*less* reliable here.

This file is populated during the port (it starts mostly empty by design). It is
a durable, version-agnostic artifact — keep entries phrased about the *function*,
not about a specific address. Where a v83 anchor (string/constant) has drifted in
the older v79 build, **record the v79-specific anchor** so the next backward port
benefits.

The shared, version-agnostic anchors discovered during the v84 port (WinMain via
the IExplorer-title string, SendHSLog via `%s\HShield`, CWvsApp::CWvsApp via the
`WebStart`/`IsWow64Process` cluster, etc.) live in
`docs/tasks/task-006-gms-v84-support/signature-catalog.md`. **Consult it first** —
most entries should transfer to v79; this file records the v79 deltas and any new
anchors needed when the v83/v84 heuristic fails on the older build.

## How to use

- When you resolve a key, add or update its row below.
- Record the **most stable** anchor that actually worked, plus a fallback.
- Note observed stability across versions you've checked (e.g. "string present in
  v79, v83, and v84"; "byte sig matched v83 but not v79").
- If you also want the heuristic encoded in code/comments, put a short comment at
  the relevant patch site in source referencing this catalog entry.

## Entry schema

```
### <CANONICAL_FUNCTION_NAME>   (memory-map key: <KEY>)
- Primary anchor: <string xref | import call | call-graph | constant | vtable slot | byte sig>
- Detail: <the exact literal / API / parent / constant / slot index used>
- Fallback anchor: <secondary method if primary is absent>
- Cross-version stability: <which versions confirmed; known breakages; v79-vs-v83 drift>
- v79 address: <0x… (named in IDB)>
- Notes: <pitfalls, ambiguity, near-duplicates to disambiguate>
```

## v79 IDB baseline

Confirmed at task-1 setup (2026-06-26):

- **Module:** `GMS_v79_1_DEVM.exe`
- **IDB path:** `E:\Programs\Nexon\IDBs_v9\GMS\v79\GMS_v79_1_DEVM.exe.i64`
- **Image base:** `0x400000`
- **Image size:** `0x990000`
- **MD5:** `718a1f2d3493acce0e81637f2faa9e99`
- **SHA-256:** `07cc813351ba2d288c0dec096a761ad990e2af8c8f909a8254c8e3aa1776f970`
- **Segments:** `.text` 0x401000–0xa2b000 (rx), `.rdata` 0xa2b3a4–0xabd000 (r), `.data` 0xabd000–0xb18000 (rw)
- **Named functions:** 2372 of 47317 total
- **IDA MCP port:** 13339

WinMain offset math reference: `WIN_MAIN = image_base + RVA`. With base `0x400000`,
a WinMain at RVA `0x5B19F2` would be VA `0x9B19F2` (adjust once confirmed in later tasks).
Baseline IDB saved clean before any renaming or annotation.

## Catalogued anchors (populate during port)

> Task 2 cluster (WinMain + CWvsApp + window-manager). **Key finding: the v79 IDB
> retains C++ mangled symbols** for nearly the whole CWvsApp class, CWndMan
> s_Update/RedrawInvalidatedWindows, WinMain, SendHSLog, and the exception
> type-info — so the v84 string/call-graph heuristics all transferred cleanly and
> the IDB symbol is the fastest primary anchor. Each entry below still records a
> second structural anchor (per the two-anchor rule). v79 image base 0x400000.

### WinMain   (memory-map key: WIN_MAIN)
- Primary anchor: string xref
- Detail: Same two unique startup literals as v84 — `MapleStoryGlobal :: MapleStory - Microsoft Internet Explorer` (@0xB0230C) and `\npkgameuninstnomsg.exe` (@0xB02384) — both xref into the WinMain body.
- Fallback anchor: call-graph — the PE entry `start` (0x9ABD95) makes exactly one call into the user image, to WinMain.
- Cross-version stability: both literals present and unique in v79, v83, v84. IDB pre-named `_WinMain@16`. Body mirrors v83/v84 (StringPool IDs, single-instance mutex check==183, ShowStartUpWndModal patcher, CConfig/CWvsApp ctor→SetUp→Run, ShowADBalloon block).
- v79 address: 0x0093F9B7 (size 0xAEE)
- Notes: HIGH-VALUE (needs-main-review). Two structural anchors of different kinds (string xref + start call-graph). Spot-check: re-probed via `\npkgameuninstnomsg.exe` xref (independent literal) → same fn.

### SendHSLog   (memory-map key: SEND_HS_LOG)
- Primary anchor: string xref
- Detail: References `%s\HShield` (@0xB02260) and `MapleStory_Global:%s` (@0xB02248); final call is the AhnLab HS-log thunk.
- Fallback anchor: call-graph — called 3× from WinMain
- Cross-version stability: both format strings present in v79, v83, v84. IDB pre-named `SendHSLog`.
- v79 address: 0x0093F8E0 (size 0xD7)
- Notes: clean transfer from the v84 anchor.

### CWvsApp::CWvsApp (ctor)   (memory-map key: C_WVS_APP)
- Primary anchor: IDB symbol (`??0CWvsApp@@QAE@PBD@Z`)
- Detail: References `WebStart` (@0xB023D8), `GameLaunching` (@0xB023C0), `token` (@0xB023D0), `kernel32.dll`/`IsWow64Process`; installs the CWvsApp vtable off_A385C0; SBB-stores the singleton (`(a1+1!=0)?a1:0` → g_CWvsApp); writes g_dwTargetOS=1996 on OS major<5 / WoW64; calls ResetLSP at the tail.
- Fallback anchor: call-graph — invoked from WinMain right before SetUp.
- Cross-version stability: command-line keyword strings + IsWow64Process probe stable v79→v84. Singleton + g_dwTargetOS writer idioms identical.
- v79 address: 0x00942D3B (size 0x353)
- Notes: HIGH-VALUE (needs-main-review). Two kinds (IDB symbol [kind 1] + call-graph from WinMain [kind 3]); WebStart xref used as independent spot-check.

### CWvsApp::SetUp   (memory-map key: C_WVS_APP_SET_UP)
- Primary anchor: IDB symbol (`?SetUp@CWvsApp@@QAEXXZ`)
- Detail: Init driver, called from WinMain right after the ctor. v79 NOT virtualized (clean body). References `Global\meteora` (strcpy literal) + `ehsvc.dll` (@0xB023E8) + `ws2_32.dll`/`getpeername`/`WSAStartup`; builds the CRC32 table (poly 0xDD04D9C1). Walking its callees is the call-graph anchor for the whole Initialize*/Create* cluster.
- Fallback anchor: call-graph — called from WinMain right after the CWvsApp ctor (WinMain → ctor → SetUp call-edge); this call-graph is also the primary anchor for the entire Initialize*/Create* cluster walkdown
- Cross-version stability: v83 SetUp is control-flow-virtualized; v79 (like v84) is de-virtualized/clean — anchor on call-graph + meteora/ehsvc, not byte layout.
- v79 address: 0x009430F1 (size 0x520)
- Notes: HIGH-VALUE (needs-main-review). **R11 / init-call order (v79): srand → GetSEPrivilege → WS2_32 IAT-clone memcpy → InitSafeDll loop → WSAStartup/getpeername integrity → HideDll loop → CSecurityClient CreateInstance/InitModule/StartModule → InitializePCOM → CreateMainWindow → CClientSocket::CreateInstance → ConnectLogin → CFuncKeyMappedMan/CQuickslotKeyMappedMan CreateInstance → InitializeResMan → InitializeGr2D → InitializeInput → InitializeSound → InitializeGameData → CreateWndManager → ApplySysOpt → CActionMan/CAnimationDisplayer/CMapleTVMan/CQuestMan/CMonsterBookMan inits → Global\meteora + ehsvc + IAT-clone anti-tamper → CheckExecPathReg → CLogo ctor → CRC32 table build.** NO InitializeAuth call (NMCO absent, see below). **NO DR/anti-debug DR_init step present** — anti-tamper is only CSecurityClient + meteora + ehsvc + IAT-clone (confirms v79 lacks the DR subsystem that caused the v84 freeze; feeds Task 10). Drift vs v84: v84 calls InitializeAuth first + srand; v79 omits InitializeAuth and orders srand first. Spot-check: ehsvc.dll xref → same fn, independent of WinMain call edge.

### CWvsApp::Run   (memory-map key: C_WVS_APP_RUN)
- Primary anchor: IDB symbol (`?Run@CWvsApp@@QAEXPAH@Z`)
- Detail: Main message-pump loop; invoked from WinMain after SetUp. Per iteration pumps the queue, calls ISMsgProc (sub_946430→named) for each drained message, then on tick calls CWvsApp::CallUpdate + CWndMan::RedrawInvalidatedWindows; raises CPatchException(0x20000000)/CDisconnectException(0x21000000..6)/CTerminateException(0x22000000..B)/ZException via _CxxThrowException; loop exits on msg type 18 (WM_QUIT).
- Fallback anchor: the four exception type-info refs — `__TI3?AVCPatchException@@`(0xA4AAD8), `...CDisconnectException@@`(0xA40868), `...CTerminateException@@`(0xA3CC38), `__TI1?AVZException@@`(0xA3D3B8) — a distinctive quad; plus the CallUpdate/RedrawInvalidatedWindows callee pair.
- Cross-version stability: v83 Run virtualized; v79/v84 clean. Anchor on the exception quad + CallUpdate/Redraw pair, not byte layout.
- v79 address: 0x00943611 (size 0x6EA)
- Notes: HIGH-VALUE (needs-main-review). Two kinds (symbol + exception-TI quad/callee-pair structure). Spot-check: CPatchException-TI xref → same fn, independent of the WinMain call edge.

### CWvsApp::ISMsgProc   (memory-map key: C_WVS_APP_IS_MSG_PROC)
- Primary anchor: constant + call-graph
- Detail: Dispatches message==0x100 (WM_KEYDOWN) → CWndMan::ProcessKey (sub_932810) and 0x1FF<msg<=0x20A (mouse range) → CWndMan::ProcessMouse (sub_93221A), each gated on the CWndMan singleton (dword_B0C12C) being non-null. Called per-message from Run.
- Fallback anchor: the 0x100 / 0x1FF..0x20A range constants + the CWndMan-singleton gate.
- Cross-version stability: message-range constants stable v79→v84. v83 IDB names it `?ISMsgProc@CWvsApp@@IAEXIIJ@Z` — applied verbatim to the v79 fn (was unnamed sub_946430).
- v79 address: 0x00946430 (size 0x4E)
- Notes: clean v79 function entry (no entry+5 artifact). Renamed in IDB.

### CWvsApp::InitializeAuth   (memory-map key: C_WVS_APP_INITIALIZE_AUTH) — ABSENT / NEW v79 SENTINEL
- Primary anchor: confirmed-absent (SP-5 backward direction)
- Detail: In v83 InitializeAuth (0x9F7097) is the first SetUp subsystem call: CNMManager::GetInstance → CNMCOClientObject::GetInstance/SetPatchOption/SetLocaleAndRegion(512,201)/Initialize(33563155), throwing CTerminateException 0x2200000B/C. In v79 NONE of this exists: no `CNMCOClientObject`/`CNMManager`/`NMLOCALEID`/`NMREGIONCODE` RTTI strings, no `Initialize(33563155)` constant (byte `13 20 00 02` absent), and v79 SetUp makes no auth call. The NMCO/Nexon-Passport auth subsystem post-dates v79.
- Fallback anchor: n/a (feature absent)
- Cross-version stability: present v83→v84, ABSENT v79. This is a backward-direction new sentinel — the consuming gate/edit must tolerate 0.
- v79 address: 0x00000000 (sentinel, flagged for gate/edit owner)
- Notes: FLAG — previously-real key zeroed because the backing feature does not exist in v79.

### CWvsApp::InitializePCOM   (memory-map key: C_WVS_APP_INITIALIZE_PCOM)
- Primary anchor: IDB symbol (`?InitializePCOM@CWvsApp@@IAEXXZ`)
- Detail: Tiny (0x20); the Wz-package COM init, sets a bool member. Called from SetUp.
- Fallback anchor: smallest CWvsApp Initialize*; call-graph from SetUp.
- Cross-version stability: structurally identical v79→v84.
- v79 address: 0x0094409B (size 0x20)

### CWvsApp::CreateMainWindow   (memory-map key: C_WVS_APP_CREATE_MAIN_WINDOW)
- Primary anchor: IDB symbol (`?CreateMainWindow@CWvsApp@@IAEXXZ`)
- Detail: Registers the window class + CreateWindowExA (MapleStoryClass/MapleStory StringPool IDs); windowed-vs-fullscreen style branch; ZException if hWnd null. Called from SetUp.
- Fallback anchor: CreateWindowExA call + the style branch; call-graph from SetUp.
- Cross-version stability: CreateWindowExA + class/title IDs stable.
- v79 address: 0x009440BB (size 0x190)

### CWvsApp::ConnectLogin   (memory-map key: C_WVS_APP_CONNECT_LOGIN)
- Primary anchor: IDB symbol (`?ConnectLogin@CWvsApp@@QAEXXZ`)
- Detail: socket-connect + message-pump wait loop (exits on msg 18), throwing on no socket. Called from SetUp (initial connect) and from CLogin::GotoTitle / CWvsContext::ReturnToTitle.
- Fallback anchor: call-graph from SetUp; the socket-singleton + pump-loop structure.
- Cross-version stability: caller set stable; v83 body virtualized, v79/v84 clean.
- v79 address: 0x0094424B (size 0x10D)

### CWvsApp::InitializeResMan   (memory-map key: C_WVS_APP_INITIALIZE_RES_MAN)
- Primary anchor: IDB symbol (`?InitializeResMan@CWvsApp@@IAEXXZ`)
- Detail: References the WZ data-type literal list (Character, Skill, Reactor, UI, Quest, Item, Effect, …) + `Base.wz`; mounts ResMan/NameSpace COM packages; calls Dir_BackSlashToSlash. Called from SetUp.
- Fallback anchor: the data-type string-table loop.
- Cross-version stability: data-type list extremely stable across versions.
- v79 address: 0x009443BB (size 0x7B1)

### CWvsApp::InitializeGr2D   (memory-map key: C_WVS_APP_INITIALIZE_GR2D)
- Primary anchor: IDB symbol (`?InitializeGr2D@CWvsApp@@IAEXXZ`)
- Detail: Creates the Gr2D DX8 package, device init with width 800/height 600, clear color 0xFF000000; windowed-mode branch. Called from SetUp.
- Fallback anchor: 800/600 + 0xFF000000 immediates feeding a COM vtable call.
- Cross-version stability: 800x600 + alpha-clear stable.
- v79 address: 0x00944C91 (size 0x25F)

### CWvsApp::InitializeInput   (memory-map key: C_WVS_APP_INITIALIZE_INPUT)
- Primary anchor: IDB symbol (`?InitializeInput@CWvsApp@@IAEXXZ`)
- Detail: ws2_32.dll/getpeername IAT-vs-disk integrity scan (anti-hook), wires CInputSystem. Called from SetUp.
- Fallback anchor: the getpeername integrity pattern + CreateFileMapping/MapViewOfFile.
- Cross-version stability: ws2_32/getpeername integrity pattern stable.
- v79 address: 0x00944F37 (size 0x2CD)

### CWvsApp::InitializeSound   (memory-map key: C_WVS_APP_INITIALIZE_SOUND)
- Primary anchor: IDB symbol (`?InitializeSound@CWvsApp@@IAEXXZ`)
- Detail: Allocates CSoundMan and calls CSoundMan::Init(hWnd, 30000, 2, 2, 16). Called from SetUp.
- Fallback anchor: the (30000,2,2,16) Init arg sequence.
- Cross-version stability: the Init args are a stable fingerprint.
- v79 address: 0x009452A1 (size 0x56)

### CWvsApp::InitializeGameData   (memory-map key: C_WVS_APP_INITIALIZE_GAME_DATA)
- Primary anchor: IDB symbol (`?InitializeGameData@CWvsApp@@IAEXXZ`)
- Detail: Allocates/validates the game-data manager singletons; CQuestMan::LoadDemand / CMonsterBookMan::LoadBook validated with CTerminateException 0x22000006 on failure. Called from SetUp.
- Fallback anchor: the alloc→ctor→validate(throw 0x22000006) pattern.
- Cross-version stability: the 0x22000006 terminate code + chain stable.
- v79 address: 0x00945834 (size 0x201)

### CWvsApp::CreateWndManager   (memory-map key: C_WVS_APP_CREATE_WND_MANAGER)
- Primary anchor: IDB symbol (`?CreateWndManager@CWvsApp@@IAEXXZ`)
- Detail: Constructs the root CWndMan and stores it to the CWndMan singleton (dword_B0C12C, the global read by ISMsgProc/s_Update); 800x600 screen layout. Called from SetUp.
- Fallback anchor: the screen-rect layout + the CWndMan singleton store.
- Cross-version stability: layout stable; v83 virtualized, v79/v84 clean.
- v79 address: 0x00944358 (size 0x63)

### CWvsApp::GetCmdLine   (memory-map key: C_WVS_APP_GET_CMD_LINE)
- Primary anchor: IDB symbol (`?GetCmdLine@CWvsApp@@QAE?AV?$ZXString@D@@H@Z`)
- Detail: Parses WebStart/GameLaunching/token args; called repeatedly from the CWvsApp ctor.
- Fallback anchor: call-graph from the ctor.
- Cross-version stability: mangled name present v79→v84.
- v79 address: 0x0094611A (size 0x15D)

### CWvsApp::Dir_BackSlashToSlash / Dir_SlashToBackSlash / Dir_upDir   (keys: C_WVS_APP_DIR_BACK_SLASH_TO_SLASH / C_WVS_APP_DIR_SLASH_TO_BACK_SLASH / C_WVS_APP_DIR_UP_DIR)
- Primary anchor: IDB symbols (`?Dir_BackSlashToSlash@CWvsApp@@SAXPAD@Z`, `?Dir_SlashToBackSlash@CWvsApp@@SAXPAD@Z`, `?Dir_upDir@CWvsApp@@SAXPAD@Z`)
- Detail: Three tiny adjacent static helpers (char-swap '\\'↔'/' and up-dir truncate). All three called from SetUp (Dir_BackSlashToSlash→Dir_upDir→Dir_SlashToBackSlash on the exec-path).
- Fallback anchor: 47/92 char immediates; adjacency (0x946277 / 0x9462BD / 0x9462... note v79 order is BackSlashToSlash, SlashToBackSlash, upDir by address).
- Cross-version stability: trivial char-swap loops; symbols present.
- v79 addresses: Dir_BackSlashToSlash 0x00946277, Dir_SlashToBackSlash 0x0094629A, Dir_upDir 0x009462BD
- Notes: v79 address order is BackSlashToSlash < SlashToBackSlash < upDir (slightly different ordering than the v84 catalog's Back/Slash/UpDir listing — confirm by symbol, not adjacency order).

### CWvsApp::GetExceptionFileName   (memory-map key: C_WVS_APP_GET_EXCEPTION_FILE_NAME)
- Primary anchor: IDB symbol (`?GetExceptionFileName@CWvsApp@@SAPBDXZ`)
- Detail: Returns a cached static char*; called from the very top of WinMain.
- Fallback anchor: call-graph from WinMain; the static-cache lazy-init shape.
- Cross-version stability: structurally identical v79→v84.
- v79 address: 0x00946481 (size 0x8B)

### CWvsApp::CallUpdate   (memory-map key: C_WVS_APP_CALL_UPDATE)
- Primary anchor: IDB symbol (`?CallUpdate@CWvsApp@@QAEXJ@Z`)
- Detail: Per-frame catch-up loop advancing the frame clock in 30 ms steps (this[6]+=30), calling CWndMan::s_Update each step; contains the periodic getpeername heartbeat (30000*(rand%6+1)) and the "3N.mhe"/ehsvc integrity check. Called from Run; is the sole caller of s_Update.
- Fallback anchor: the 30ms frame step + s_Update call.
- Cross-version stability: 30ms step stable; v83 virtualized, v79/v84 clean.
- v79 address: 0x009454B5 (size 0x373)

### CWndMan::s_Update   (memory-map key: C_WND_MAN_S_UPDATE)
- Primary anchor: IDB symbol (`?s_Update@CWndMan@@SAXXZ`)
- Detail: Drains the deferred-destroy window list, cursor-idle hide on timeout, walks the window tree. Sole callee of interest inside CallUpdate.
- Fallback anchor: call-graph from CallUpdate; adjacency (directly after RedrawInvalidatedWindows: 0x932C66 < 0x932EE2).
- Cross-version stability: symbol present v79→v84.
- v79 address: 0x00932EE2
- Notes: v83 cmake value (0x9E47C3) is a different VA — confirmed by symbol + call-graph, not proximity.

### CWndMan::RedrawInvalidatedWindows   (memory-map key: C_WND_MAN_REDRAW_INVALIDATED_WINDOWS)
- Primary anchor: IDB symbol (`?RedrawInvalidatedWindows@CWndMan@@SAXXZ`)
- Detail: COM-variant redraw loop over the invalidated-window list. Called from Run (right after CallUpdate).
- Fallback anchor: call-graph from Run; adjacency — directly precedes s_Update in the image.
- Cross-version stability: symbol present; redraw-loop shape held v79→v84.
- v79 address: 0x00932C66

### globals: CWvsApp singleton / g_dwTargetOS   (keys: C_WVS_APP_INSTANCE_ADDR / G_DW_TARGET_OS)
- Primary anchor: writer function (CWvsApp ctor)
- Detail: The CWvsApp singleton is the `(a1+1!=0)?a1:0` SBB store at the top of the ctor → g_CWvsApp @ 0xB07A68. g_dwTargetOS is the DWORD set to 1996 (0x7CC) when OS major<5 (OSVERSIONINFOEX, cb=148) OR IsWow64Process is true → @ 0xB0239C — exactly the two write sites the GMS bypass relies on.
- Fallback anchor: g_dwTargetOS reads back as 1996; both renamed in the v79 IDB (g_CWvsApp / g_dwTargetOS).
- Cross-version stability: both writer idioms stable; locate via the ctor, not the raw address.
- v79 addresses: C_WVS_APP_INSTANCE_ADDR 0x00B07A68; G_DW_TARGET_OS 0x00B0239C
- Notes: Drift — the ctor tail calls ResetLSP (0x44A9B1) when OS major>=6 and not WoW64, so **ResetLSP IS present in v79** (relevant to the RESET_LSP key / stale "# does not exist" comment — finding for that key's owner).

### WinMain offsets   (keys: WIN_MAIN_AD_BALLOON_CONDITIONAL / WIN_MAIN_PATCHER_OFFSET)
- Primary anchor: re-measured from v79 WinMain disasm (never copied from v83)
- Detail: PATCHER_OFFSET points at `call ShowStartUpWndModal` (E8 C0 FC FF FF) @ 0x93FBC9 → delta 0x212 from WinMain base 0x93F9B7; the no-patcher edit NOPs these 5 bytes. AD_BALLOON_CONDITIONAL points at the `jz` (74 6F) @ 0x9403F4 guarding the ShowADBalloon block (v32=740/v33=300/v34=60, then the MapleStoryGlobal string push) → delta 0xA3D; the no-ad-balloon edit overwrites the first byte 0x74→0xEB.
- Fallback anchor: the patcher call target is ShowStartUpWndModal (0x93F88E); the ad-balloon block is immediately followed by the `MapleStoryGlobal :: …` string push (sub_8DD924).
- Cross-version stability: offsets MUST be re-measured per version. v79 measured 0x212 / 0xA3D — coincidentally identical to v83 (v84 differed: 0x241 / 0xA6E). Confirmed at the instruction/byte level in v79, NOT inherited.
- v79 addresses: patcher call @ 0x93FBC9; ad-balloon jz @ 0x9403F4
- Notes: verified bytes — 0x93FBC9 = E8 C0 FC FF FF (call), 0x9403F4 = 74 6F (jz short).

### CFuncKeyMappedMan (class anchor + v79 size, for Task 3)   (memory-map keys: C_FUNC_KEY_MAPPED_MAN_VFTABLE etc. — full set is Task 7)
- Primary anchor: IDB symbol + CreateInstance alloc size
- Detail: ctor `??0CFuncKeyMappedMan@@QAE@XZ` @ 0x569DE5 installs vtable off_A2EB38 (@0xA2EB38). TSingleton<CFuncKeyMappedMan>::CreateInstance (0x946AFB) allocates **904 (0x388)** bytes. Singleton global dword_B0D2A8 (@0xB0D2A8).
- Fallback anchor: ctor field-init extent — vtable@0 + memcpy 0x1BD→this+4 + memcpy 0x1BD→this+449 + dwords @+896/+900 → object ends at 904. Size confirmed two independent ways (Alloc(904) and the ctor layout).
- Cross-version stability: located via the CFuncKeyMappedMan ctor symbol; size measured per version.
- v79 address: vtable 0x00A2EB38; class size 904
- Notes: Step 5 scope only — no struct type applied (read-only). Full FuncKeyMapped key resolution is Task 7. Recorded in struct-verification.md.
</content>

---

## Task 4 — CClientSocket / ZSocket cluster

> v79 IDB carries the **full mangled C++ symbols** for the entire socket cluster
> (`?SendPacket@CClientSocket@@…`, `?OnConnect@CClientSocket@@…`, etc.), so every
> function's primary anchor is its surviving IDB symbol; a second *structural*
> anchor is recorded per the two-anchor rule. The connect/send/recv WS2_32 calls
> route through **cloned import slots** (anti-tamper, same idiom as v83/v84):
> dword_B1011C=WSAAsyncSelect, dword_B1013C=connect, dword_B1015C=send,
> dword_B10164=recv, dword_B10130=WSAGetLastError, dword_B0FDE4=Sleep,
> dword_B100FC=timeGetTime. Anchor on the surviving `socket`/`shutdown`/
> `closesocket`/`getpeername` *imports*, not on the cloned slots.

### CClientSocket::SendPacket   (memory-map key: C_CLIENT_SOCKET_SEND_PACKET)   [HIGH-VALUE / needs-main-review]
- Primary anchor: IDB symbol `?SendPacket@CClientSocket@@QAEXABVCOutPacket@@@Z`.
- Detail: acquires the per-socket ZSynchronizedHelper<ZFatalSection> lock ([this+124]), validates fd ([this+8] != 0/-1) and the send-disabled flag ([this+5]==0), then `MakeBufferList(this+80, 79, this+132, 1, m_seqSnd=[this+33])` -> `CIGCipher::innoHash(this+132,4,0)` -> `CClientSocket::Flush`. The MakeBufferList send-seq immediate **79** is the v79 protocol version (was 84 in v84, 83 in v83 — drift, read it, don't copy).
- Fallback anchor: call-graph — the MakeBufferList -> innoHash -> Flush callee triple is a unique fingerprint; last call in the body IS Flush. Also reached from the free `?SendPacket@@YAXABVCOutPacket@@@Z` (0x51a618) which reads the singleton g_pClientSocketInstance and calls this.
- Cross-version stability: structure stable v79→v84; the lock-ctor (ZSynchronizedHelper) + MakeBufferList/innoHash/Flush triple holds.
- v79 address: 0x0048DF93 (size 0x88)
- Notes: Two structural anchor kinds (symbol + call-graph). Spot-check: the free SendPacket helper confirms independently of the symbol.

### CClientSocket::Flush   (memory-map key: C_CLIENT_SOCKET_FLUSH)   [HIGH-VALUE / needs-main-review]
- Primary anchor: IDB symbol `?Flush@CClientSocket@@QAEXXZ`.
- Detail: walks the send-buffer ZList (head [this+22]/[this+23], InterlockedIncrement per node), writes via the cloned WS2_32 send slot `dword_B1015C(fd, buf, len, 0)`, handles -1 / WSAEWOULDBLOCK (10035 via dword_B10130) and routes errors to CClientSocket::OnError. The only socket-region fn that walks [this+23] and calls dword_B1015C.
- Fallback anchor: call-graph/structure — it is the last call inside SendPacket (0x48dff4); the [this+23] ZList walk + dedicated cloned-send slot is distinct from connect (dword_B1013C) and recv (dword_B10164).
- Cross-version stability: send-buffer-walk structure stable v79→v84.
- v79 address: 0x0048E01B (size 0x11A)
- Notes: Two structural anchor kinds (symbol + structure/call-from-SendPacket). Spot-check: reached as SendPacket's tail call, independent of the symbol.

### CClientSocket::ManipulatePacket   (memory-map key: C_CLIENT_SOCKET_MANIPULATE_PACKET)   [HIGH-VALUE / needs-main-review]
- Primary anchor: IDB symbol `?ManipulatePacket@CClientSocket@@QAEXXZ`.
- Detail: `while([this+17])` receive-stream reassembly loop; runs the per-packet version/seq check `(word[this+57*2] ^ HIWORD(dword[this+34])) != -80`, on a full packet (ret==2) re-seeds `m_seqRcv=[this+34]=CIGCipher::innoHash(this+136,4,0)` and dispatches through ProcessPacket; bad packets -> OnError(0).
- Fallback anchor: call-graph — the **sole caller** of CClientSocket::ProcessPacket (0x48e1dc); its own callers are the FD_READ dispatcher sub_48DD14 and CWvsApp::Run.
- Cross-version stability: reassembly + innoHash + sole-ProcessPacket-caller relationship stable v79→v84.
- v79 address: 0x0048E135 (size 0xD4)
- Notes: Two structural anchor kinds (symbol + call-graph). Spot-check: two independent callers (sub_48DD14, CWvsApp::Run) corroborate, independent of the symbol.

### CClientSocket::ProcessPacket   (memory-map key: C_CLIENT_SOCKET_PROCESS_PACKET)   [HIGH-VALUE / needs-main-review]
- Primary anchor: IDB symbol `?ProcessPacket@CClientSocket@@IAEXAAVCInPacket@@@Z`.
- Detail: reads the CWvsContext singleton (dword_B07848), calls `CInPacket::Decode2` to read the opcode word, then a switch: 0x10=OnMigrateCommand, 0x11=OnAliveReq, 0x12/0x13/0x14 internal handlers, else if 0x1A..0x75 -> CWvsContext::OnPacket, else the CStage(dword_B0DADC) vtable slot+8 dispatch.
- Fallback anchor: call-graph — sole non-trivial callee is `?Decode2@CInPacket@@QAEGXZ`; **sole caller is CClientSocket::ManipulatePacket**.
- Cross-version stability: Decode2 + sub-0x10 jump-table dispatch stable v79→v84 (the 0x1A..0x75 CWvsContext range is the v79 form).
- v79 address: 0x0048E209 (size 0xC0)
- Notes: Two structural anchor kinds (symbol + call-graph). Spot-check: sole caller ManipulatePacket confirms independent of the symbol.

### CClientSocket::OnConnect   (memory-map key: C_CLIENT_SOCKET_ON_CONNECT)
- Primary anchor: IDB symbol `?OnConnect@CClientSocket@@QAEHH@Z` (int arg = bSuccess).
- Detail: the recv-side FD_CONNECT/FD_READ handshake handler driven by the connect message-pump. Allocs a 0x5B4 receive buffer (ZSocketBuffer::Alloc), reads via the cloned recv slot dword_B10164, parses host/port, validates the **version-header byte == 8** (`if (v17 != 8) throw CTerminateException 0x22000007`) and the **major version == 0x4F (79)** (`>0x4F`->CPatchException, `<0x4F`->CTerminateException), then ClearSendReceiveCtx + getpeername([this+40]) and emits the post-handshake packet via CClientSocket::SendPacket. Recurses OnConnect(this,0) as the retry/teardown path; also tail-called by Connect(sockaddr_in) on error.
- Fallback anchor: structure — the only fn calling ZSocketBuffer::Alloc(0x5B4) + the cloned recv slot + getpeername + the version-header(8)/major(0x4F) guard trio that throws the CTerminate/CPatch exception pair.
- Cross-version stability: handshake-validation structure stable; the **major-version compared changes per build** (0x4F=79 here, 84 in v84, 83 in v83) — re-read it, never inherit.
- v79 address: 0x0048CB81 (size 0x565)
- Notes: Disambiguated from the three Connect* variants — it is the recv-side handler (cloned recv slot + getpeername + SendPacket driver), not a `socket`/`connect` caller, and takes the bSuccess int arg.

### 8-byte client-key finding (feeds Task 13 / §5.5; CWvsContext.h:98 `>83` gate, bypass/socket_hooks.cpp:311)
- **Verdict: ABSENT in v79 (no-key form) — confirms the `BUILD_MAJOR_VERSION > 83` gate prediction.**
- Evidence: OnConnect's post-handshake send has two mutually-exclusive branches keyed on `[this+9]` (m_ctxConnect.bLogin):
  - else / logged-in branch (PLAYER_LOGGED_IN, opcode 20=0x14) @ 0x48d01c:
    `COutPacket(20); Encode4(charId=[dword_B07848+8352]); Encode1(0|1 per TSecType bit [+8252]&0x80); Encode1(0); CClientSocket::SendPacket;` — and then returns. There is **no `EncodeBuffer(m_aClientKey, 8)`** anywhere in OnConnect (and CWvsContext has no m_aClientKey member below v83). The encoder sequence is exactly Encode4/Encode1/Encode1 — no 8-byte buffer push.
  - bLogin branch (CLIENT_START_ERROR, opcode 25=0x19) @ 0x48cfbf: builds the GetExceptionFileName report and EncodeBuffer(report,len) — unrelated to a client key.
- This matches the v84 catalog's observation pattern but the v79 logged-in packet is *shorter* (v84 added machineId(16)); v79 has neither the 16-byte machineId nor any 8-byte client key in this path.

### CClientSocket::ConnectLogin   (memory-map key: C_CLIENT_SOCKET_CONNECT_LOGIN)
- Primary anchor: IDB symbol `?ConnectLogin@CClientSocket@@QAEXXZ`.
- Detail: stores m_nGameStartMode from g_CWvsApp+4; pulls login host/port via `CWvsApp::GetCmdLine(0)` / `GetCmdLine(1)`; if both present builds one CONNECTCONTEXT from them, else iterates the server table (dword_AC32B0 count, ZArray shuffle, `rand()`-picks unk_B0D9E0+16*i) building a CONNECTCONTEXT list; finally `CClientSocket::Connect(CONNECTCONTEXT)`.
- Fallback anchor: structure/call-graph — the GetCmdLine(0/1) + random-server-pick + final Connect(CONNECTCONTEXT); its sole caller is CWvsApp::ConnectLogin.
- Cross-version stability: GetCmdLine + Connect(CONNECTCONTEXT) tail stable v79→v84.
- v79 address: 0x0048C773 (size 0x19D)
- Notes: Distinct from CWvsApp::ConnectLogin (the message-pump driver, 0x0094424B in v79) — this is the CClientSocket method it invokes. Disambiguator: it is the only Connect* variant reading the command line + server table.

### CClientSocket::Connect(CONNECTCONTEXT)   (memory-map key: C_CLIENT_SOCKET_CONNECT_CTX)
- Primary anchor: IDB symbol `?Connect@CClientSocket@@QAEXABUCONNECTCONTEXT@1@@Z` (public).
- Detail: small wrapper — consumes the CONNECTCONTEXT (sub_48CA2E), advances the address list cursor ([this+6]/[this+8]), tail-calls the private Connect(sockaddr_in).
- Fallback anchor: structure — its only meaningful call is Connect(sockaddr_in) (0x48ca56); callers are ConnectLogin and a CWvsContext-side caller.
- Cross-version stability: wrapper-to-sockaddr_in relationship stable v79→v84.
- v79 address: 0x0048C9CA (size 0x64)
- Notes: Disambiguated from CONNECT_ADR — this is the public CONNECTCONTEXT overload that *calls* CONNECT_ADR rather than calling `socket` directly.

### CClientSocket::Connect(sockaddr_in)   (memory-map key: C_CLIENT_SOCKET_CONNECT_ADR)
- Primary anchor: IDB symbol `?Connect@CClientSocket@@IAEXPBUsockaddr_in@@@Z` (private) — the **sole `socket` import caller** (`socket(2,1,0)` = AF_INET/SOCK_STREAM).
- Detail: ClearSendReceiveCtx + ZSocketBase::CloseSocket([this+8]), then socket(2,1,0) -> throws ZException on -1, arms WSAAsyncSelect via cloned slot dword_B1011C(s, hwnd, 1025, 51=FD_CONNECT|FD_READ|FD_CLOSE), then the cloned connect slot dword_B1013C(s, addr, 16=sizeof sockaddr_in), tolerating WSAEWOULDBLOCK (10035); calls OnConnect(this,0) on the synchronous paths.
- Fallback anchor: structure — socket(2,1,0) + connect(s,addr,16) + WSAEWOULDBLOCK; sole caller of the `socket` import.
- Cross-version stability: the `socket` import xref is the most stable anchor; connect/select are indirected through cloned slots in all of v79/v83/v84.
- v79 address: 0x0048CA56 (size 0x12B)
- Notes: Disambiguated as the private sockaddr_in overload and the only `socket` caller. Its callers are Connect(CONNECTCONTEXT) and OnConnect.

### ZSocketBase::CloseSocket   (memory-map key: Z_SOCKET_BASE_CLOSE_SOCKET)
- Primary anchor: IDB symbol `?CloseSocket@ZSocketBase@@QAEXXZ`.
- Detail: `if (*this != -1) { shutdown(*this, 2); closesocket(*this); *this = -1; }` — the only tiny fn calling both `shutdown` and `closesocket` imports.
- Fallback anchor: structure — the -1 sentinel store + the shutdown(s,2)/closesocket pair; called by CClientSocket::Close ([this+8]) and Connect(sockaddr_in).
- Cross-version stability: shutdown+closesocket import pair is a very stable anchor (size 0x20, identical shape v79→v84).
- v79 address: 0x0048C699 (size 0x20)
- Notes: a thunk `j_?CloseSocket@ZSocketBase@@QAEXXZ` exists at 0x48c658 — the real body is 0x48c699.

### ZSocketBuffer::Alloc   (memory-map key: Z_SOCKET_BUFFER_ALLOC)
- Primary anchor: IDB symbol `?Alloc@ZSocketBuffer@@SAPAV1@IABVZAllocHelper@@@Z`.
- Detail: static factory doing TWO `ZAllocEx<ZAllocAnonSelector>::Alloc` calls — the payload of size `a1`, then the 28-byte (0x1C) ZSocketBuffer header — then runs the placement ctor (sub_48DC37) and returns it.
- Fallback anchor: call-graph/structure — the dual ZAllocEx::Alloc with a fixed 28-byte second size is near-unique; called by OnConnect with the 0x5B4 receive-buffer size.
- Cross-version stability: dual-alloc(payload, 28) idiom stable v79→v84.
- v79 address: 0x0048DBEA (size 0x4D)
- Notes: the v79 signature collapsed the ZAllocEx call to one arg (anon selector); read the second alloc size (28) as the fingerprint.

### CClientSocket::Close   (memory-map key: C_CLIENT_SOCKET_CLOSE)
- Primary anchor: IDB symbol `?Close@CClientSocket@@QAEXXZ`.
- Detail: 2-call body `ClearSendReceiveCtx(this); ZSocketBase::CloseSocket(this+8);` — the only fn calling both on the embedded ZSocketBase at [this+8].
- Fallback anchor: structure — the ClearSendReceiveCtx+CloseSocket pair; also called at the top of OnConnect's failure path.
- Cross-version stability: tiny 2-call shape stable v79→v84.
- v79 address: 0x0048DF81 (size 0x12)

### CClientSocket::ClearSendReceiveCtx   (memory-map key: C_CLIENT_SOCKET_CLEAR_SEND_RECEIVE_CTX)
- Primary anchor: IDB symbol `?ClearSendReceiveCtx@CClientSocket@@IAEXXZ`.
- Detail: zeros [this+26]/[this+56(word)]/[this+30], then calls the ZList<ZRef<ZSocketBuffer>>::RemoveAll helper (sub_48EA53) twice — on the receive list [this+60] and the send list [this+80].
- Fallback anchor: call-graph — called from the CClientSocket ctor, CClientSocket::Close, OnConnect, and Connect(sockaddr_in); the double-RemoveAll on two list heads is the structural idiom.
- Cross-version stability: double-RemoveAll shape stable v79→v84.
- v79 address: 0x0048E5D7 (size 0x21)

### globals: CClientSocket singleton / CreateInstance   (keys: C_CLIENT_SOCKET_INSTANCE_ADDR / C_CLIENT_SOCKET_CREATE_INSTANCE)
- Primary anchor: writer (CClientSocket ctor) + IDB symbol (CreateInstance).
- Detail: the singleton is the `(this+4 != 0)? this : 0` SBB store at the TOP of `??0CClientSocket@@QAE@XZ` (0x48c55f) into g_pClientSocketInstance @ 0xB07844; the ctor installs vtable off_A2CA20, sets fd=-1, wires the ZList heads, and tail-calls ClearSendReceiveCtx. `?CreateInstance@?$TSingleton@VCClientSocket@@@@SAPAVCClientSocket@@XZ` (0x946ab6) reads the singleton and on null does `ZAllocEx::Alloc(148=0x94)` -> ctor (the 0x94 instance size matches v83/v84).
- Fallback anchor: the singleton is read as the socket `this` by the free `?SendPacket@@YAXABVCOutPacket@@@Z` (0x51a618), CWvsApp::ConnectLogin, and hundreds of CField/CCashShop senders; CreateInstance is the sole caller of the ctor.
- Cross-version stability: SBB-singleton store + Alloc(0x94) idiom stable; located via the ctor, not address arithmetic.
- v79 addresses: C_CLIENT_SOCKET_INSTANCE_ADDR 0x00B07844 (renamed g_pClientSocketInstance in IDB); C_CLIENT_SOCKET_CREATE_INSTANCE 0x00946AB6
- Notes: CreateInstance moved OUT of the v83 0x9F9Exx singleton cluster into the 0x946Axx static-init region (same region as the other manager CreateInstances) — found by call-graph (sole ctor caller), NOT proximity. v83 seed INSTANCE_ADDR (0xBE7914) and CREATE_INSTANCE (0x9F9E53) are unrelated VAs.

## COutPacket encode cluster (Task 5)

### COutPacket::COutPacket (ctor)   (memory-map key: C_OUT_PACKET)
- Primary anchor: IDB symbol `??0COutPacket@@QAE@J@Z`.
- Detail: EH-prolog ctor that zeroes the buffer member (`*((DWORD*)this+1) = 0`), calls the ZArray<uchar>::_Alloc helper with initial capacity 256 (`sub_48E8CB(256, &v4)`), then calls `COutPacket::Init(a2)` (?Init@COutPacket@@QAEXJ@Z @ 0x67AE46). The 256-byte initial allocation feeding an Init(seq) call is the structural fingerprint.
- Fallback anchor: constant + call-graph — the 0x100 alloc immediate (constant) and the COutPacket::Init callee (call-graph). Two different kinds besides the symbol.
- Cross-version stability: member+4=0 / _Alloc(256) / Init shape stable v79→v84; layout (buf at +4, len at +8) shared with the encoders' grow helper.
- v79 address: 0x0067AD6B (size span to ~0x67AE.. ; entry at 0x67AD6B)
- Notes: HIGH-VALUE (needs-main-review). Spot-check (independent kind): the 256 immediate to _Alloc plus the Init tail-call, read independent of the mangled name.

### COutPacket::Encode1 / Encode2 / Encode4   (memory-map keys: C_OUT_PACKET_ENCODE_1 / C_OUT_PACKET_ENCODE_2 / C_OUT_PACKET_ENCODE_4)
- Primary anchor: IDB symbols `?Encode1@COutPacket@@QAEXE@Z`, `?Encode2@COutPacket@@QAEXG@Z`, `?Encode4@COutPacket@@QAEXK@Z`.
- Detail: three tiny (0x1E/0x21/0x1F byte) functions, each pushes its byte-width to the shared `COutPacket::_EnsureCapacity` (?_EnsureCapacity@COutPacket@@AAEXI@Z @ 0x4062E5) then stores. **WIDTH DISCRIMINANT (do not swap):** Encode1 = `push 1` + `mov [eax+ecx], dl` + `inc dword [esi+8]`; Encode2 = `push 2` + `mov [eax+ecx], dx` + `add dword [esi+8], 2`; Encode4 = `push 4` + `mov [eax+ecx], edx` + `add dword [esi+8], 4`. The push immediate matches the store width.
- Fallback anchor: call-graph — `xrefs_to _EnsureCapacity` returns EXACTLY the five sibling encoders (Encode1/2/4 + EncodeStr + EncodeBuffer) and nothing else; the grow→store→advance shape with width immediate is the second (constant) kind. Two structural kinds besides the symbol.
- Cross-version stability: store-width idiom and shared _EnsureCapacity callee stable v79→v84.
- v79 addresses: Encode1 0x004062C7, Encode2 0x0042539C, Encode4 0x00406324
- Notes: HIGH-VALUE (needs-main-review). Encode2 lives in a distant region (0x4253xx) vs Encode1/Encode4 adjacent at 0x4063xx — confirm by store width, never by adjacency. Spot-check (independent kind): width store byte read from each body, independent of symbol.

### COutPacket::EncodeStr   (memory-map key: C_OUT_PACKET_ENCODE_STR)
- Primary anchor: IDB symbol `?EncodeStr@COutPacket@@QAEXV?$ZXString@D@@@Z`.
- Detail: reads the ZXString length (`mov eax,[eax-4]` when ptr non-null, else 0), grows by `len + 2` via _EnsureCapacity, copies through `CIOBufferManipulator::EncodeStr(ZXString, buf)` (?EncodeStr@CIOBufferManipulator@@... @ 0x469544) after a ZXString::operator= into a temp, advances len by the returned count, and runs the ZXString dtor in an EH unwind slot.
- Fallback anchor: call-graph (shared _EnsureCapacity + CIOBufferManipulator::EncodeStr callee) + structure (the `[eax-4]` length read + `+2` grow). Two structural kinds besides the symbol.
- Cross-version stability: ZXString length-prefix + CIOBufferManipulator::EncodeStr shape stable v79→v84.
- v79 address: 0x004694DE (size 0x66)
- Notes: HIGH-VALUE (needs-main-review). Spot-check (independent kind): the 0x4694DE body reads the ZXString length via `mov eax,[eax-4]` (with the null→`xor eax,eax` branch) and grows by `add eax,2` before _EnsureCapacity, then dispatches to CIOBufferManipulator::EncodeStr (0x469544) — the length-prefix-minus-4 + (len+2) idiom, observed independent of the mangled name; the address also appears in the exact `xrefs_to _EnsureCapacity` 5-sibling set.

### COutPacket::EncodeBuffer   (memory-map key: C_OUT_PACKET_ENCODE_BUFFER)
- Primary anchor: IDB symbol `?EncodeBuffer@COutPacket@@QAEXPBXI@Z` (void* PBX form, not the byte-pointer PBE form).
- Detail: takes (Src, Size); grows the buffer by Size via _EnsureCapacity(Size), `memcpy(buf + len, Src, Size)`, then `len += Size`; `retn 8`. The grow→memcpy→advance triple with a caller-supplied length is the fingerprint.
- Fallback anchor: call-graph (shared _EnsureCapacity) + import (_memcpy). Two structural kinds besides the symbol.
- Cross-version stability: grow/memcpy/advance shape stable v79→v84.
- v79 address: 0x00466AE9 (size 0x2A)
- Notes: HIGH-VALUE (needs-main-review). Spot-check (independent kind): the 0x466AE9 body takes a caller-supplied Size, calls `_EnsureCapacity(Size)` then the `_memcpy` import (0x9A8AF0) into `buf+len`, advances `len += Size`, and ends with `retn 8` (two stack args) — the memcpy-grow triple + 8-byte arg frame, observed independent of the mangled name; the address also appears in the exact `xrefs_to _EnsureCapacity` 5-sibling set.

### COutPacket::MakeBufferList   (memory-map key: C_OUT_PACKET_MAKE_BUFFER_LIST)
- Primary anchor: IDB symbol `?MakeBufferList@COutPacket@@QAE?AV?$ZRef@VZSocketBuffer@@@@HW4SocketSendFlag@@@Z` (retained in v79; NOT stripped as in v84).
- Detail: builds the encrypted ZSocketBuffer chunk list. Distinctive constant: MTU chunking at **1460 / 0x5B4** (`v16=1460; if(len<0x5B4) v16=len`) driving repeated `ZSocketBuffer::Alloc(v16)`; the in-place COutPacket shuffle uses `71 - __ROR1__(...)` and `^0x13` over the payload. Sole MakeBufferList call inside the resolved `CClientSocket::SendPacket` (0x48DF93), invoked as `MakeBufferList(this+80, 79, this+132, 1, m_seqXor)` between the ZSynchronizedHelper ctor and CIGCipher::innoHash / CClientSocket::Flush.
- Fallback anchor: call-graph (parent = SendPacket; callee = resolved `ZSocketBuffer::Alloc` 0x48DBEA / Z_SOCKET_BUFFER_ALLOC) + constant (1460/0x5B4 chunk size + 71/0x13 shuffle). Two structural kinds besides the symbol.
- Cross-version stability: 1460-byte chunking + 71/0x13 shuffle + ZSocketBuffer::Alloc loop stable v79→v84.
- v79 address: 0x0067AEC4
- Notes: HIGH-VALUE (needs-main-review). Spot-check (independent kind): the 1460/0x5B4 chunk constant and the 71-ROR/^0x13 shuffle loop, read independent of both the symbol and the SendPacket call edge.

## Login / Stage / Logo / Title cluster (Task 6)

### CLogin::Update   (memory-map key: C_LOGIN_UPDATE)
- Primary anchor: vtable slot — vtable[0] of CLogin primary vtable at 0xA2F9EC (get_int read confirmed). Slot 0 of a stage-derived class vtable always maps to Update in v79/v83/v84 calling convention.
- Detail: body references `[esi+0x15C]` timer counter incremented each tick, then calls `CWnd::InvalidateRect` to force a repaint. The timer walk + InvalidateRect combination with a 1500 ms threshold is the structural fingerprint.
- Fallback anchor: call-graph — `CLogin::Update` is the sole function in the 0x5CA000-0x5CC000 range that calls `CWnd::InvalidateRect` (0x48CCA4) and reads `[esi+0x15C]`.
- Cross-version stability: **DIVERGES from C_LOGO_UPDATE in v79**. In v83, both keys share address 0x005F4C16 (single function). In v79 the two stages each have their own Update implementation. Always verify independently; never assume they still alias.
- v79 address: 0x005CA348 (labeled CLogin__Update in IDB)
- Notes: The v83 seed value (0x005F4C16) must not be copied blindly to v79. C_LOGIN_UPDATE and C_LOGO_UPDATE are different functions in this version.

### CLogin::SendCheckPasswordPacket   (memory-map key: C_LOGIN_SEND_CHECK_PASSWORD_PACKET)
- Primary anchor: IDB symbol (retained in v79 IDB from DEVM build).
- Detail: constructs a COutPacket with opcode byte 0x05 (CheckPassword), encodes the username/password ZXStrings via Encode4/EncodeStr, then dispatches via CClientSocket::SendPacket. The 0x05 immediate + encode-pair + SendPacket call is the structural fingerprint.
- Fallback anchor: call-graph (callee = COutPacket::COutPacket + resolved CClientSocket::SendPacket) + constant (opcode 0x05 immediate).
- Cross-version stability: opcode 0x05 CheckPassword stable v79→v83; encode-pair shape stable. Symbol also present in v83.
- v79 address: 0x005CBF50
- Notes: Located directly via IDB symbol — no address arithmetic needed. Confirm with spot-check on the 0x05 opcode immediate and the EncodeStr+SendPacket call-graph.

### CLogo::CLogo (ctor)   (memory-map key: C_LOGO)
- Primary anchor: IDB symbol `??0CLogo@@QAE@XZ` (retained in v79 DEVM build).
- Detail: nullary ctor (no args besides `this`). Writes 4 vtable pointers: primary IStage vtable at A307BC, IUIMsgHandler vtable at A30770, CStage::OnPacket vtable slot at A3076C, and a 4th at A30768. Allocates member `0x258` (CUIObject subobject). Caller in C_LOGO_LOGO_END uses `Alloc(0x258)` before invoking the ctor — the two-step Alloc+ctor idiom is a fallback anchor.
- Fallback anchor: call-graph (called from C_LOGO_LOGO_END / 0x005FFA4C: the sole caller is the alloc-then-ctor sequence) + vtable-write pattern (4 vtable stores at the first instructions of the ctor).
- Cross-version stability: Alloc(0x258)+4-vtable-write pattern stable v79→v83; RTTI header at A307C8 (`0xFDE04000`) distinguishes CLogo from CLogin primary vtable.
- v79 address: 0x005FF8C4 (labeled CLogo__ctor in IDB)
- Notes: The RTTI header bytes (0xFDE04000 at A307C8, 0x014F373B at A307CC) are version-specific. Use the 4-vtable-write shape + Alloc(0x258) caller for forward ports.

### CLogo::GetRTTI   (memory-map key: C_LOGO_GET_RTTI)
- Primary anchor: IDB symbol (retained in v79 IDB).
- Detail: one-liner returning a pointer to the CLogo RTTI descriptor; appears at a fixed slot offset in the CWnd interface vtable.
- Fallback anchor: vtable slot (CWnd-iface vtable slot 53 from CLogo primary vtable start).
- Cross-version stability: GetRTTI slot position stable v79→v83→v84.
- v79 address: 0x0042196A
- Notes: Consecutive with IsKindOf at 0x00421970 — if GetRTTI is found, IsKindOf is always at +6 bytes (next function).

### CLogo::IsKindOf   (memory-map key: C_LOGO_IS_KIND_OF)
- Primary anchor: IDB symbol (retained in v79 IDB).
- Detail: compares the passed RTTI descriptor pointer against the CLogo descriptor chain. CWnd-iface vtable slot 54.
- Fallback anchor: vtable slot (immediately follows GetRTTI at slot 54); call-graph (shared RTTI descriptor xrefs).
- Cross-version stability: slot stable v79→v83→v84.
- v79 address: 0x00421970
- Notes: Address is exactly +6 from C_LOGO_GET_RTTI (0x0042196A). A tight pair — confirm both simultaneously from the vtable read.

### CLogo::Update   (memory-map key: C_LOGO_UPDATE)
- Primary anchor: vtable slot — vtable[0] of CLogo primary vtable at 0xA307BC (confirmed via `get_int`).
- Detail: 1500 ms logo timer body: checks `[esi+0x1EC]` (or similar timer field), triggers the stage transition by calling `LogoEnd` when elapsed. **DIVERGES from C_LOGIN_UPDATE in v79** — they are two separate functions.
- Fallback anchor: call-graph (body calls C_LOGO_LOGO_END / 0x005FFA4C after timer check) + constant (1500 ms threshold immediate).
- Cross-version stability: Timer-threshold + LogoEnd call stable v79→v83. In v83 this was a shared address with CLogin::Update — that coincidence is gone in v79. Treat as independent. **v83 slot-order check (read 2026-06-26, v83 MapleStory_dump.exe port 13340):** CLogo primary vtable off_AF7C80 slot0 (get_int) = 0x0062F2B6 = `?Update@CLogo@@UAEXXZ` (symbol-confirmed). v79 0xA307BC slot0 = 0x005FFE54. Slot 0 = Update in both → slot order matches v83: no drift.
- v79 address: 0x005FFE54 (labeled CLogo__Update in IDB; required `define_func` before rename would accept it)
- Notes: IDA had this as `loc_5FFE54` (not recognised as a function head). Used `define_func` on [0x5FFE54, 0x5FFE54+length] to promote it, then rename succeeded.

### CLogo::OnMouseButton   (memory-map key: C_LOGO_ON_MOUSE_BUTTON)
- Primary anchor: vtable slot — IUIMsgHandler vtable at 0xA30770, slot 2 (= `[this+4]` in CLogo object, offset 8 into the IUIMsgHandler vtable block).
- Detail: first comparison is `cmp [esp+arg_0], 202h` (WM_LBUTTONUP = 0x202). On match, applies `add ecx, 0FFFFFFFCh` (thiscall `this` adjustment for IUIMsgHandler) then calls `CLogo::InitNXLogo`. The 0x202 immediate + thiscall-adjust + InitNXLogo call is the structural fingerprint.
- Fallback anchor: call-graph (callee = C_LOGO_INIT_NX_LOGO / 0x005FFA96) + constant (0x202 WM_LBUTTONUP).
- Cross-version stability: WM_LBUTTONUP constant + InitNXLogo call-path stable v79→v83. **v83 slot-order check (read 2026-06-26, v83 port 13340):** CLogo IUIMsgHandler vtable off_AF7C34 slot2 (get_int) = 0x0062F2A1; v79 0xA30770 slot2 = 0x005FFE3F. Slot 2 = OnMouseButton in both → slot order matches v83: no drift.
- v79 address: 0x005FFE3F (labeled CLogo__OnMouseButton in IDB)
- Notes: IUIMsgHandler vtable is at `[this+4]` in CLogo (the second vtable pointer). Slot layout: [A30770]=OnKey, [A30774]=OnSetFocus, [A30778]=OnMouseButton.

### CLogo::OnSetFocus   (memory-map key: C_LOGO_ON_SET_FOCUS)
- Primary anchor: vtable slot — IUIMsgHandler vtable at 0xA30770, slot 1 (address A30774).
- Detail: trivial stub: `push 1; pop eax; retn 4` — always returns 1 (true). Three-instruction body is unambiguous.
- Fallback anchor: function size (3 instructions, ~4 bytes) + return-1 pattern. Both independent of the vtable read.
- Cross-version stability: always-true stub shape stable v79→v83. **v83 slot-order check (read 2026-06-26, v83 port 13340):** CLogo IUIMsgHandler vtable off_AF7C34 slot1 (get_int) = 0x0062ED20; v79 0xA30770 slot1 = 0x005FF902. Slot 1 = OnSetFocus in both → slot order matches v83: no drift.
- v79 address: 0x005FF902 (labeled CLogo__OnSetFocus_IUI in IDB)
- Notes: An earlier session incorrectly assigned 0x0092F599 (the CWnd override version). Correct address is the IUIMsgHandler vtable version at 0x005FF902. Always read the IUIMsgHandler vtable directly — the CWnd vtable has a different OnSetFocus that does real work.

### CLogo::OnKey   (memory-map key: C_LOGO_ON_KEY)
- Primary anchor: vtable slot — IUIMsgHandler vtable at 0xA30770, slot 0 (address A30770 itself).
- Detail: checks wParam against three key codes in sequence: 13 (VK_RETURN), 27 (VK_ESCAPE), 32 (VK_SPACE). Any match triggers `add ecx, 0FFFFFFFCh` (thiscall adjust) + `CLogo::InitNXLogo`. The three-key-constant cluster is the structural fingerprint.
- Fallback anchor: call-graph (callee = C_LOGO_INIT_NX_LOGO / 0x005FFA96) + constants (13/27/32 key-code immediates).
- Cross-version stability: key-code triple (13/27/32) + InitNXLogo dispatch stable v79→v83. **v83 slot-order check (read 2026-06-26, v83 port 13340):** CLogo IUIMsgHandler vtable off_AF7C34 slot0 (get_int) = 0x0062F27A; v79 0xA30770 slot0 = 0x005FFE18. Slot 0 = OnKey in both → slot order matches v83: no drift.
- v79 address: 0x005FFE18 (labeled CLogo__OnKey in IDB; defined via define_func before rename)
- Notes: Both OnKey and OnMouseButton call InitNXLogo after a thiscall this-adjustment. If either is found, the other is derivable from the same vtable block.

### CLogo::LogoEnd   (memory-map key: C_LOGO_LOGO_END)
- Primary anchor: call-graph — body is `Alloc(0x258)` followed by `CLogo::CLogo(ctor)` (0x005FF8C4) followed by `SetStage` (0x006F1AC0). This three-call sequence allocates and installs CLogin as the next stage.
- Detail: the Alloc(0x258) + CLogin ctor + SetStage triple is the structural fingerprint; no other function in the binary calls all three together.
- Fallback anchor: constant (0x258 = sizeof CLogin allocation block) + call-graph (SetStage callee confirmed independently).
- Cross-version stability: Alloc+ctor+SetStage pattern stable v79→v83. Size 0x258 may shift if CLogin layout changes.
- v79 address: 0x005FFA4C (labeled CLogo__LogoEnd in IDB)
- Notes: This is the "transition to login" function. SetStage is called with the freshly allocated CLogin pointer, which means LogoEnd is always a parent of SetStage for the login-stage transition.

### CLogo::ForcedEnd   (memory-map key: C_LOGO_FORCED_END)
- Primary anchor: vtable slot — CLogo primary vtable at 0xA307BC, slot 2 (address A307C4, read via `get_int`).
- Detail: body is `push 3E8h` (1000) + `CSoundMan::PlayBGM(...)` on the CSoundMan singleton (dword_B0BEC8) then a `nullsub` tail — it (re)issues the logo BGM via the 0x3E8 idiom, NOT a Stop call (earlier "stops BGM" wording was imprecise). SET_STAGE calls `[eax+8]` (vtable slot 2) on the stage at 0x6F1B14. The PlayBGM(0x3E8)-on-singleton + vtable-slot-2 position is the structural fingerprint.
- Fallback anchor: call-graph (SET_STAGE parent — `xrefs_to` confirms 0x006F1B14 dispatches this slot) + vtable position (primary vtable slot 2 = ForcedEnd in v79/v83 CLogo).
- Cross-version stability: vtable slot 2 = ForcedEnd convention stable v79→v83; PlayBGM(0x3E8)-on-CSoundMan-singleton body stable. **v83 slot-order check (read 2026-06-26, v83 port 13340):** CLogo primary vtable off_AF7C80 slot2 (get_int) = 0x0062EE8C; its body is the identical `push 3E8h` + `CSoundMan::PlayBGM` idiom (dword_BEBF94 singleton) — same virtual as v79 0x005FFA2A. v79 0xA307C4 slot2 = 0x005FFA2A. Slot 2 = ForcedEnd in both → slot order matches v83: no drift. (NOTE: the memory-map column-3 v83 SEED 0x0062EEF8 was imprecise — the real v83 slot-2 function is 0x0062EE8C; v79 value 0x005FFA2A is unaffected and confirmed.)
- v79 address: 0x005FFA2A (labeled CLogo__ForcedEnd in IDB)
- Notes: vtable slot order: [A307BC]=Update, [A307C0]=Init, [A307C4]=ForcedEnd — confirmed by direct v79 get_int and matched against the v83 CLogo primary vtable off_AF7C80 (same 3-slot order).

### CLogo::Init   (memory-map key: C_LOGO_INIT)
- Primary anchor: vtable slot — CLogo primary vtable at 0xA307BC, slot 1 (address A307C0).
- Detail (own body, EH-prolog): calls a sub-init helper (sub_5FFF34), then `CInputSystem::ShowCursor(0)` (hides the cursor for the logo stage), then reads `CWvsApp::GetCmdLine(3)`; if that cmdline arg is non-empty AND `g_CWvsApp[+0x28]!=0` it tail-calls `CLogo::LogoEnd` (the skip-logo shortcut). The ShowCursor(0) + GetCmdLine(3) + conditional-LogoEnd shape is the body-level fingerprint, independent of the vtable slot.
- Detail (dispatch anchor): SET_STAGE calls `[eax+4]` (vtable slot 1) on the new stage at 0x6F1C2C to initialise it after install.
- Fallback anchor: call-graph (SET_STAGE parent at 0x006F1C2C dispatches this slot; callees CInputSystem::ShowCursor + CWvsApp::GetCmdLine) + vtable position (slot 1 = Init convention).
- Cross-version stability: vtable slot 1 = Init convention stable v79→v83. **v83 slot-order check (read 2026-06-26, v83 port 13340):** CLogo primary vtable off_AF7C80 slot1 (get_int) = 0x0062EDDA (matches the historical v83 seed); v79 0xA307C0 slot1 = 0x005FF9BC. Slot 1 = Init in both → slot order matches v83: no drift.
- v79 address: 0x005FF9BC (labeled CLogo__Init in IDB)
- Notes: Slot 1 dispatch in SET_STAGE is at 0x6F1C2C (`call dword ptr [eax+4]`). Confirmed by tracing SET_STAGE body.

### CLogo::InitNXLogo   (memory-map key: C_LOGO_INIT_NX_LOGO)
- Primary anchor: string xref — references StringPool ID 0x568, which resolves to the NX-logo resource path (e.g. `Map/Obj/login.img/Logo/logo/0`). StringPool::GetBSTR(0x568) is the first real operation of the function.
- Detail: init-once guard at `[this+0x28]` — if already non-zero, returns immediately (idempotent). Otherwise calls StringPool::GetBSTR(0x568), stores the result, and loads/plays the NX logo sprite. The guard + 0x568 constant + StringPool call is the structural fingerprint.
- Fallback anchor: call-graph (callee = StringPool::GetBSTR; callers = CLogo::OnKey + CLogo::OnMouseButton + CLogo::Update timer path) + constant (0x568 StringPool ID).
- Cross-version stability: StringPool ID 0x568 = NX-logo path confirmed in v79. If the ID drifts in older versions, search for the NX-logo path string directly and re-derive the ID from that xref.
- v79 address: 0x005FFA96 (labeled CLogo__InitNXLogo in IDB)
- Notes: 0x568 is the v79 StringPool ID for the NX-logo UOL path. This ID may differ in earlier or later versions — always verify via the actual string content, not the numeric ID alone.

### Stage singleton (memory-map key: STAGE_INSTANCE_ADDR)
- Primary anchor: store-after-SetStage — SET_STAGE (0x006F1AC0) writes the new stage pointer to this global at offset +0x2C from its entry (instruction at ~0x6F1AEC). `xrefs_to 0x00B0DADC` confirms this is the sole writer outside of static init.
- Detail: `mov dword ptr [B0DADC], <stage_ptr>` immediately after the SetStage prolog; all stage-dispatch callers read this global for vtable dispatch.
- Fallback anchor: call-graph (read by CWvsApp main loop and the stage-dispatch helper) + .data range check (0xB0DADC ∈ [0xABD000, 0xB18000] for v79; any 0xBExxxx address is the v83 seed and invalid for v79).
- Cross-version stability: singleton store pattern stable; the actual address WILL differ — always trace from SetStage body, never copy from v83.
- v79 address: 0x00B0DADC (labeled StageInstanceAddr in IDB)
- Notes: v83 seed was 0x00BEDED4 — invalid for v79 (v79 .data ends at 0xB18000). All stage/GR/UITitle globals verified to be in the 0xB0xxxx–0xB1xxxx range.

### SetStage   (memory-map key: SET_STAGE)
- Primary anchor: IDB symbol `SetStage` (retained in v79 DEVM build).
- Detail: stores the new stage pointer to STAGE_INSTANCE_ADDR (0xB0DADC), then calls ForcedEnd on the old stage (`[eax+8]`, vtable slot 2) and Init on the new stage (`[eax+4]`, vtable slot 1). The store-then-ForcedEnd-then-Init triple is the structural fingerprint.
- Fallback anchor: call-graph (callee of C_LOGO_LOGO_END; dispatches vtable slots 1 and 2 on stage objects) + constant (STAGE_INSTANCE_ADDR 0xB0DADC).
- Cross-version stability: store+ForcedEnd+Init triple stable v79→v83→v84.
- v79 address: 0x006F1AC0 (labeled SetStage in IDB)
- Notes: The IDB symbol made this trivial. For versions without the symbol, the Alloc(stage_size)+ctor+SetStage call-graph from LogoEnd functions is the fastest path.

### GR singleton (memory-map key: GR_INSTANCE_ADDR)
- Primary anchor: store pattern — `CWvsApp::InitializeGr2D` (at 0x944CCA) calls `sub_947BB8` with the address of `dword_B10F74` as an output argument. The store is `mov [B10F74], eax` inside sub_947BB8.
- Detail: after the store, `CWvsApp` reads `mov ebx, dword_B10F74` and dispatches through its vtable. The output-arg→store→vtable-dispatch chain through sub_947BB8 is the structural fingerprint.
- Fallback anchor: call-graph (parent = InitializeGr2D; consumer = CWvsApp's Gr vtable dispatch) + .data range check (0xB10F74 ∈ v79 .data range 0xABD000–0xB18000).
- Cross-version stability: GR singleton output-arg store pattern stable; address WILL differ from v83 (0x00BF14EC is the invalid v83 seed).
- v79 address: 0x00B10F74 (labeled GrInstanceAddr in IDB)
- Notes: Found by tracing `CWvsApp::InitializeGr2D` → `call sub_947BB8` with the address pushed as the output arg. The output-arg idiom (push &global; call factory) is the reliable cross-version heuristic.

### CStage::OnMouseEnter   (memory-map key: C_STAGE_ON_MOUSE_ENTER)
- Primary anchor: IDB symbol `?OnMouseEnter@CStage@@UAEXH@Z` (retained in v79 DEVM build).
- Detail: virtual function taking a single `int` (the hit region); called from the input dispatch when the cursor enters a stage region. The mangled name encodes `__thiscall`, one `int` arg, `void` return.
- Fallback anchor (DIRECT v79 vtable read, 2026-06-26, v79 port 13339): CStage::OnMouseEnter (0x0092F3F8) is inherited unchanged by CLogin (CLogin does not override it), so it appears in CLogin's secondary CWnd/IUIMsgHandler vtable. `get_int` at 0x00A2FA20 = 0x0092F3F8 — that is slot 6 (offset +0x18) of the secondary vtable beginning at 0x00A2FA08, which matches the IDB symbol address exactly. This is a genuine v79 structural anchor independent of the symbol. (CLogo's own vtable block 0xA307xx does NOT reference 0x0092F3F8 because CLogo overrides OnMouseEnter — so use a non-overriding CStage subclass like CLogin as the witness; the reviewer's hypothesised slot 0xA30814 is in CLogo's block and lands on an RTTI header 0x014F37BB, not a function pointer.)
- Cross-version stability: symbol retained v79→v84; the OnMouseEnter slot lives in the CWnd/IUIMsgHandler secondary vtable of CStage subclasses. Not asserting v83/v84 slot indices here without a read.
- v79 address: 0x0092F3F8
- Notes: Symbol made the primary anchor trivial; the second anchor is the direct CLogin secondary-vtable slot read above (0x00A2FA20 = 0x0092F3F8), NOT a cross-version assumption. For versions without the symbol, locate via a CStage subclass's secondary vtable (the slot whose value is shared by many subclasses = the inherited CStage::OnMouseEnter) or via the input dispatch path.

### CStage::OnPacket   (memory-map key: C_STAGE_ON_PACKET)
- Primary anchor: IDB symbol `?OnPacket@CStage@@UAEXJAAVCInPacket@@@Z` (retained in v79 DEVM build).
- Detail: virtual function taking `long` opcode + `CInPacket&`; dispatches to sub-handlers. The mangled name encodes `__thiscall`, `long`+`CInPacket&` args, `void` return.
- Fallback anchor: vtable slot (CStage vtable; CLogo's IStage::OnPacket vtable at 0xA3076C points here) + call-graph (sole dispatch target from CLogo's vtable slot 3).
- Cross-version stability: symbol retained v79→v84; vtable slot and CInPacket arg shape stable.
- v79 address: 0x006F079F
- Notes: CLogo's vtable at [this+8] = 0xA3076C, slot 0 = 0x006F079F (confirmed via get_int). Both the IDB symbol and the CLogo vtable cross-reference confirm this independently.

### CUITitle singleton (memory-map key: C_UI_TITLE_INSTANCE_ADDR)
- Primary anchor: store-after-ctor — `sub_5F652C` (the CUITitle ctor, a CWnd-derived nullary ctor) stores `this` to 0x00B0D738 via a SBB null-check idiom: `sbb eax, eax; and eax, ecx; mov [B0D738], eax`.
- Detail: the dtor (at loc_5FD04E) clears 0x00B0D738 (`mov dword ptr [B0D738], 0`), and `CLogin::ForcedEnd` calls the dtor to destroy the title UI. The ctor-store + dtor-clear + ForcedEnd-destroy triple is the structural fingerprint.
- Fallback anchor: call-graph (ctor called from the CLogin init path in the 0x5C0000–0x600000 range; dtor called from ForcedEnd) + .data range check (0xB0D738 ∈ v79 .data range 0xABD000–0xB18000).
- Cross-version stability: SBB-singleton store + CWnd-derived ctor + ForcedEnd-destroy pattern stable; address WILL differ (v83 seed 0x00BEDA60 is invalid for v79).
- v79 address: 0x00B0D738 (labeled UITitleInstanceAddr in IDB)
- Notes: Found via `py_eval` scanning all global stores from functions in the CLogin/CLogo code range (0x5C0000–0x600000) that target the v79 .data window (0xB00000–0xB18000). The SBB idiom is `mov eax, this; sbb eax, eax; and eax, ecx; mov [global], eax` — it stores `this` if non-null, else 0. Recognise this pattern in future ports.

---

## Task 7 — Manager-singleton cluster (~37 keys)

> **Key structural fact (v79).** All TSingleton<T>::CreateInstance functions moved
> into the static-init region **0x9466CD–0x946C88** (out of v83's 0x9F9xxx cluster),
> using the single-arg `ZAllocEx<ZAllocAnonSelector>::Alloc(size)` form. There are
> exactly **10 named** TSingleton CreateInstances + **1 unnamed** (CMacroSysMan,
> sub_946C88). Each CreateInstance trivially reveals its `*_INSTANCE_ADDR` global
> (the `result = dword_Bxxxxx; if(!dword_Bxxxxx){...}` read) and its class size
> (the Alloc immediate) — decompile the CreateInstance and read both. The cluster is
> driven from **CWvsApp::SetUp** (0x9430F1, R11 walk) for most managers and from
> **CWvsApp::InitializeGameData** (0x945834) for the game-data managers + CMacroSysMan
> (its tail call). The v79 IDB retains mangled symbols for the manager classes and
> their methods, so the method keys (Init/SweepCache/LoadBook/LoadDemand/…) are
> direct IDB-symbol lookups; only CMacroSysMan, CSecurityClient::OnPacket and the 3
> CInputSystem message-pump methods were unnamed and needed structural anchors.

### TSingleton<T>::CreateInstance + instance globals (the CREATE_INSTANCE / INSTANCE_ADDR pairs)
- Primary anchor: each CreateInstance is a tiny (0x42–0x45) fn `result=dword_G; if(!dword_G){v=Alloc(N); if(v)return Ctor(v); else return 0;} return result;` — decompile it; `dword_G` is the INSTANCE_ADDR, `N` the class size, `Ctor` the class ctor.
- Fallback anchor: the CreateInstance is the sole caller of the class ctor (call-graph), and the cluster is contiguous in 0x9466CD–0x946C88.
- v79 addresses (CreateInstance fn / instance global / Alloc size):
  - CActionMan: 0x00946A09 / 0x00B07804 / 672
  - CAnimationDisplayer: 0x00946A5F / 0x00B0BE9C / 424
  - CInputSystem: 0x009466CD / 0x00B0C29C / 2512
  - CQuestMan: 0x00946725 / 0x00B0D318 / 648
  - CMonsterBookMan: 0x009467D6 / 0x00B0D314 / 164
  - CFuncKeyMappedMan: 0x00946AFB / 0x00B0D2A8 / 904
  - CQuickslotKeyMappedMan: 0x00946B51 / 0x00B0C114 / 48
  - CSecurityClient: 0x00946BA5 / 0x00B0C308 / 316
  - CMapleTVMan: 0x00946BEA / 0x00B0D458 / 992
  - CMacroSysMan: 0x00946C88 (unnamed) / 0x00B0C118 / 80
- Notes: ActionMan key naming — `C_ACTION_MAN_CREATE_INSTANCE_ADDR` is the CreateInstance **function** (0x946A09); `C_ACTION_MAN_INSTANCE_ADDR` is the **global** (0xB07804). Same convention as v83.

### CFuncKeyMappedMan::CFuncKeyMappedMan (ctor)   (memory-map key: C_FUNC_KEY_MAPPED_MAN)
- Primary anchor: IDB symbol `??0CFuncKeyMappedMan@@QAE@XZ` (0x569DE5).
- Detail: SBB-stores singleton dword_B0D2A8, installs vtable off_A2EB38, then `memcpy(this+4, unk_ABF99C, 0x1BD)` (m_aFuncKeyMapped) and `memcpy(this+449, unk_ABF99C, 0x1BD)` (m_aFuncKeyMapped_Old), then zeroes this+896/this+900.
- Fallback anchor: call-graph — the sole callee of the CFuncKeyMappedMan CreateInstance (0x946AFB); the dual-memcpy-from-unk_ABF99C + the off_A2EB38 vtable store is the structural fingerprint (kind 3 + constant, independent of the symbol).
- Cross-version stability: SBB-singleton + vtable-install + default-keymap-memcpy idiom stable v79→v84; address differs (v83 ctor 0x58DD0D).
- v79 address: 0x00569DE5.

### CFuncKeyMappedMan vftable   (memory-map key: C_FUNC_KEY_MAPPED_MAN_VFTABLE)
- Primary anchor: writer — off_A2EB38 is the value installed at `*this` (first dword) by the CFuncKeyMappedMan ctor (0x569DE5) and matches the Task-2/Task-3 class-anchor finding.
- Fallback anchor: cross-confirmed by Task 2 Step 5 (carry-forward) where the CFuncKeyMappedMan vtable was located independently for the size-gate measurement.
- v79 address: 0x00A2EB38 (off_A2EB38).

### CFuncKeyMappedMan singleton   (memory-map key: C_FUNC_KEY_MAPPED_MAN_INSTANCE_ADDR)
- Primary anchor: writer — the SBB-singleton store at the top of the ctor (0x569DE5) writes dword_B0D2A8.
- Fallback anchor: reader — the CFuncKeyMappedMan CreateInstance (0x946AFB) reads dword_B0D2A8 as the null-check; CFuncKeyMappedMan::SaveFuncKeyMap (0x569FE4) also references it. .data range check (0xB0D2A8 ∈ v79 .data 0xABD000–0xB18000).
- v79 address: 0x00B0D2A8.

### DefaultFuncKeyMap blob   (memory-map key: DEFAULT_FKM_INSTANCE_ADDR)
- Primary anchor: the 445-byte (0x1BD) default-func-key-map data blob unk_ABF99C, used as the memcpy SOURCE in the ctor (into this+4 and this+449).
- Fallback anchor: re-referenced (xrefs_to confirm exactly two referrers) by `CFuncKeyMappedMan::DefaultFuncKeyMap` (0x56A1C9), whose entire body is `memcpy(this+4, unk_ABF99C, 0x1BD)` — a second independent function pointing at the same blob.
- Cross-version stability: a default-keymap data blob exists in v79/v83/v84; address differs (v83 0xBD8BCC). The key_mapped_hooks.cpp edit memcpy's from this into m_aFuncKeyMapped / m_aFuncKeyMapped_Old.
- v79 address: 0x00ABF99C (DefaultFKMInstanceAddr).

### DefaultQuickslotKeyMap blob — ABSENT in v79 (memory-map key: DEFAULT_QKM_INSTANCE_ADDR) [FLAG]
- Primary anchor: confirmed-absent (SP-5 backward direction). v83's FKM ctor (0x58DD0D) memcpy'd a SEPARATE 32-byte quickslot-default blob (dword_BD8D8C) into this+224/this+232; **the v79 FKM ctor has no such memcpy** — it only zeroes this+896/this+900 (the 904-byte object is too small to embed a 32-byte quickslot array; quickslot lives in the separate CQuickslotKeyMappedMan, whose ctor 0x602158 also just zeroes).
- Fallback anchor: byte search — the v83 32-byte QKM default-key sequence (0x2A,0x52,0x47,0x49,0x1D,0x53,0x4F,0x51) has **no match anywhere in v79** (find_bytes, dword- or byte-form); there is no referenced quickslot-default data global in v79.
- Cross-version stability: present v83→v84, ABSENT v79 → new backward sentinel. The key_mapped_hooks.cpp quickslot memcpy must tolerate 0 / be struct-gated for v79.
- v79 address: 0x00000000 (sentinel, flagged for gate/edit owner).

### CInputSystem message-pump methods (keys: C_INPUT_SYSTEM_UPDATE_DEVICE / _GET_IS_MESSAGE / _GENERATE_AUTO_KEY_DOWN)
- Primary anchor: call-graph from **CWvsApp::Run** (0x943611). All three are called from Run's message dispatch (v83 chunk sub_9F5FD9), in three distinguishable branches keyed on the wait-result of `MsgWaitForMultipleObjects`-style `dword_B0FFDC(...)`:
  - `if (type<=2)`: `UpdateDevice(inst,type)` then `while(GetIsMessage(inst,&msg)) ISMsgProc(...)`.
  - `else (auto-key)`: `if(GenerateAutoKeyDown(inst,&msg)) ISMsgProc(...); CSecurityClient::Update(inst2)`.
- Fallback anchor: exact body structure (independent of the unnamed symbols):
  - UpdateDevice (0x575BFE, size 0x1d): `if(!a1) UpdateKeyboard(1); else if(a1==1) UpdateMouse;`.
  - GetIsMessage (0x575C1B, size 0x32): `if(!this[625])return 0; copy 3 dwords from this[626]; helper(this[626]); return 1;`.
  - GenerateAutoKeyDown (0x576BE7): `gate this[2]/[8]/[586]; *a2=256; a2[2]=…|GetSpecialKeyFlag(this); return 1;`.
  Sizes/bodies match v83 (UpdateDevice 0x59A2E9, GetIsMessage sub_59A306, GenerateAutoKeyDown sub_59B2D2) exactly.
- v79 addresses: 0x00575BFE / 0x00575C1B / 0x00576BE7. (Init 0x005757D4, ShowCursor 0x00575C4D are IDB symbols.)
- Notes: these 3 were unnamed in v83 too (manual-RE seeds) — anchor on Run-branch position + body shape, never address.

### CMacroSysMan::CreateInstance (key: C_MACRO_SYS_MAN_CREATE_INSTANCE)
- Primary anchor: usage cross-check. v79 instance dword_B0C118 (read by sub_946C88) is read by the SAME functions as v83's CMacroSysMan instance 0xBEC1F4 — `CUserLocal::UseFuncKeyMapped` and `CAvatar::NotifyAvatarModified` (and CUserLocal::Update).
- Fallback anchor: structure/call-graph — sub_946C88 is the TSingleton CreateInstance idiom (read-global → Alloc(80) → ctor sub_6CBBFC installing vtable off_A31638) and is called from `CWvsApp::InitializeGameData`'s tail (the `return sub_946C88();` at 0x945A28). Two structural kinds independent of any symbol.
- Cross-version drift: class GREW v79→v83 backward? — v79 allocs 80 (0x50, 4 vtables) vs v83 48 (0x30, 2 vtables). Do NOT match Macro by size; match by the UseFuncKeyMapped/NotifyAvatarModified instance-usage anchor.
- v79 address: 0x00946C88 (CreateInstance_TSingleton_CMacroSysMan); instance 0x00B0C118; ctor 0x6CBBFC (CMacroSysMan_ctor).

### CSecurityClient::OnPacket (key: C_SECURITY_CLIENT_ON_PACKET) [needs-main-review]
- Primary anchor: call-graph — reached as `CClientSocket::ProcessPacket` (0x48E209) **case 0x14** dispatch (`sub_994995(pkt)`).
- Fallback anchor: body structure (independent of the call edge) — `result = CInPacket::Decode1(a1); if(result==4) return OnCheckClientIntegrityRequest(a1); return result;` (size 0x1f); the Decode1==4 → OnCheckClientIntegrityRequest (sub_9949B4) shape exactly matches v83 OnPacket. Region check: sits in the CSecurityClient code block (0x994xxx, adjacent to ctor/InitModule/StartModule).
- Detail: exact match to v83 OnPacket (0xA4BF03, also size 0x1f, Decode1==4 → OnCheckClientIntegrityRequest).
- Cross-version drift: v83 placed OnPacket at 0xA4BF03 (IGCipher region); v79 at 0x994995 (CSecurityClient region). Anchor on the ProcessPacket-case-0x14 edge + the Decode1==4 body, not address.
- v79 address: 0x00994995 (OnPacket_CSecurityClient). Spot-check (needs-main-review): the security instance dword_B0C308 is also consumed by CSecurityClient::Update (sub_9948EE) in CWvsApp::Run's auto-key branch — independent corroboration of the security cluster.

### CRadioManager — ABSENT in v79 (keys: C_RADIO_MANAGER_CREATE_INSTANCE / _INSTANCE_ADDR) [FLAG]
- Primary anchor: confirmed-absent (SP-5 backward direction). The static-init singleton cluster (0x9466CD–0x946C88) contains every manager EXCEPT radio (v83's cluster had `sub_9FA078` for radio); v79 has no 11th CreateInstance allocating the radio size, and no "Radio" string literal exists (find_regex). (Note: RTTI-string byte search is useless here — v79 class names come from a symbol/PDB source, not RTTI type descriptors; even "FuncKeyMappedMan" find_bytes returns 0.)
- Fallback anchor: usage displacement — `CConfig::ApplySysOpt` (0x4960F9) reads CSoundMan/CInputSystem/CWvsContext but **no** radio global (v83's ApplySysOpt read radio instance 0xBEC3B4 at two sites); and `CWvsContext::Update` (0x94F766) drives the scheduled-message ("radio") feature off **CMapleTVMan** (dword_B0D458: sub_607BC5/sub_607BBE/[+964]) — the role v83 served from radio 0xBEC3B4. The feature is folded into CMapleTVMan in v79.
- Cross-version stability: present v83→v84, ABSENT v79 → new backward sentinel; the common/CRadioManager.cpp edit must tolerate 0.
- v79 address: 0x00000000 / 0x00000000 (sentinel, flagged for gate/edit owner).
- Notes — **Radio quirk determination (the v84-flagged trap):** v83's map seed `C_RADIO_MANAGER_INSTANCE_ADDR = 0xBF0B00` is **WRONG** — confirmed by v83 disasm: `sub_9FA078` does `Alloc(dword_BF0B00, 0x2C)`, so 0xBF0B00 is the `dword_BF0B00` ZAllocEx **allocator-selector** (1st Alloc arg), while the real v83 instance global is **dword_BEC3B4** (the SBB store in ctor sub_72FC30). In v79 the manager is absent entirely.

## Task-7 review — manager method keys: second structural anchors (direct v79 probes)

> The 9 manager method keys originally rested on the surviving mangled symbol (kind 1)
> alone. Each now carries a SECOND structural anchor read directly from the v79 IDB:
> the caller address (call-graph) AND a distinctive body constant/string. All probes
> performed on v79 (port 13339, active-flag confirmed). None of the 9 is inlined/absent.

### CActionMan::Init   (memory-map key: C_ACTION_MAN_INIT)
- Primary anchor: IDB symbol `?Init@CActionMan@@QAEXXZ`.
- Fallback anchor: call-graph — sole caller is `CWvsApp::SetUp` at **0x943396**. Body constant: starts with `CActionMan::GetCharacterImgEntry(this, 2000)` then iterates a fixed 143-entry action table (`while v3 < 143`, special-cases index 40 and range 101..108) writing the action-table globals dword_B0C4F8 / unk_B0C4FC. The 2000 img-entry id + the 143-entry build loop + the B0C4F8 table are the structural fingerprint.
- v79 address: 0x0040681C.

### CActionMan::SweepCache   (memory-map key: C_ACTION_MAN_SWEEP_CACHE)
- Primary anchor: IDB symbol `?SweepCache@CActionMan@@QAEXXZ`.
- Fallback anchor: call-graph — sole caller is `CWvsApp::CallUpdate` at **0x945812** (per-frame). Body constant: a phase state-machine keyed on the global `dword_B07800` cycling 0→1→2→3→4→5→0, each phase walking a ZList<ZRef> at a this+offset and dropping entries idle ≥ **300000 ms**, gated by a **60000 ms** outer throttle (`dword_B100FC()` timeGetTime). The B07800 phase counter + 300000/60000 thresholds are the fingerprint.
- v79 address: 0x0040FEEA.

### CMapleTVMan::Init   (memory-map key: C_MAPLE_TV_MAN_INIT)
- Primary anchor: IDB symbol `?Init@CMapleTVMan@@QAEXXZ`.
- Fallback anchor: call-graph — sole caller is `CWvsApp::SetUp` at **0x9433A7**. Body constant: 5× `ZXString::ReleaseBuffer(&byte_B0C24C)`, `ZThread::BeginThread`, `Alloc(56)` (sub_605614), and `StringPool::GetInstance(_, 3919)` (the MapleTV WZ resource id). The StringPool id **3919** + the byte_B0C24C string-buffer cluster + BeginThread are the fingerprint.
- v79 address: 0x006074C7.

### CMonsterBookMan::LoadBook   (memory-map key: C_MONSTER_BOOK_MAN_LOAD_BOOK)
- Primary anchor: IDB symbol `?LoadBook@CMonsterBookMan@@QAEHXZ`.
- Fallback anchor: call-graph (both directions). Caller: `CWvsApp::SetUp` at **0x9433F9**. Callee triple (body): `return LoadCard(this) && LoadStringA(this) && LoadBookIcon(this);` — the three private CMonsterBookMan loaders (0x651C49 / 0x6522C6 / 0x652BFA) short-circuited in one expression are a unique fingerprint.
- v79 address: 0x00651C1F.

### CQuestMan::LoadDemand   (memory-map key: C_QUEST_MAN_LOAD_DEMAND)
- Primary anchor: IDB symbol `?LoadDemand@CQuestMan@@QAEHXZ`.
- Fallback anchor: call-graph + WZ-path constant. Caller: `CWvsApp::SetUp` at **0x9433B3**. Body: opens a WZ property via `get_int32(off_AC2FF8)` (the QuestInfo path global), enumerates demand props, uses StringPool id 3199, and **tail-calls `CQuestMan::LoadQuestInfo` (0x6AD76C)**. The off_AC2FF8 path global + the LoadQuestInfo tail-call distinguish it from the sibling Load* methods.
- v79 address: 0x006A8CD6.

### CQuestMan::LoadPartyQuestInfo   (memory-map key: C_QUEST_MAN_LOAD_PARTY_QUEST_INFO)
- Primary anchor: IDB symbol `?LoadPartyQuestInfo@CQuestMan@@QAEXXZ`.
- Fallback anchor: call-graph + WZ-key strings. Caller: `CWvsApp::SetUp` at **0x9433E2**. Body: opens WZ via `get_int32(off_AC31D8)` and reads the property keys **"rank"** (aRank_0 @0xAC31CC), **"ranks"** (aRanks @0xAC31C0), **"mark"** (aMark @0xAC31B4), inserting party-quest entries via sub_6AE7C9. The rank/ranks/mark key strings are unique to this method.
- v79 address: 0x006AE1F4.

### CQuestMan::LoadExclusive   (memory-map key: C_QUEST_MAN_LOAD_EXCLUSIVE)
- Primary anchor: IDB symbol `?LoadExclusive@CQuestMan@@QAEXXZ`.
- Fallback anchor: call-graph + WZ-path constant + structure. Caller: `CWvsApp::SetUp` at **0x9433ED**. Body: opens WZ via `get_int32(off_AC3264)` and runs a **nested double IEnumVARIANT** enumeration (exclusive-quest groups) building a `ZArray<unsigned short>` with an O(n²) cross-dedup loop (sub_6B0D99). The off_AC3264 path + nested-enum dedup structure distinguish it from LoadDemand/LoadPartyQuestInfo.
- v79 address: 0x006AF68D.

### CInputSystem::Init   (memory-map key: C_INPUT_SYSTEM_INIT)
- Primary anchor: IDB symbol `?Init@CInputSystem@@QAEXPAUHWND__@@PAPAX@Z`.
- Fallback anchor: call-graph + import + constant. Caller: `CWvsApp::InitializeInput` at **0x944F93**. Body: `DirectInput8Create(...&riidltf...)` (DirectInput device init), a 3-iteration device-create loop, `StringPool::GetInstance(_, 959)`, and sets the default cursor position `this[590]=400 / this[591]=300` before `SetCursorState(0)`. The DirectInput8Create import + the 400/300 default-pos constants are the fingerprint.
- v79 address: 0x005757D4.

### CInputSystem::ShowCursor   (memory-map key: C_INPUT_SYSTEM_SHOW_CURSOR)
- Primary anchor: IDB symbol `?ShowCursor@CInputSystem@@QAEXH@Z`.
- Fallback anchor: call-graph + body constant. Callers include `CWndMan::s_Update` at **0x932F48** (cursor-idle-hide path) and CWndMan::ProcessMouse (0x93221A) / CLogo::Init. Body: reads the cursor object `this[606]`, calls its vtable+224 with the show/hide selector **`a2 ? -1 : 0xFFFFFF`**, error-routes to `CWnd::CoverBackgrnd`. The `-1 / 0xFFFFFF` cursor-show/hide ternary is the fingerprint.
- v79 address: 0x00575C4D.

## Task 8 — Config / SystemInfo / IGCipher / utilities cluster

> v79 retains mangled C++ symbols for nearly every function in this cluster
> (CConfig members, CIGCipher::innoHash, CSystemInfo Init/GetMachineId/GetGameRoomClient,
> ZArray<uchar>::RemoveAll, ZXString TrimRight/TrimLeft/_Cat, CMob ctor, GetSEPrivilege,
> the ZSynchronizedHelper<ZFatalSection> ctor) — IDB symbol is the fastest primary
> anchor; each entry pairs it with a SECOND structural anchor. The CConfig singleton
> and the windowed-mode global were renamed (g_CConfig_pInstance / g_CConfig_SysOpt_WindowedMode);
> the CSystemInfo ctor (was sub_99CDB0) was renamed ??0CSystemInfo@@QAE@XZ.

### GetSEPrivilege   (memory-map key: GET_SE_PRIVILEGE)
- Primary anchor: import call — the only fn calling OpenProcessToken (IAT 0xA2B018) -> LookupPrivilegeValueA (IAT 0xA2B01C, "SeDebugPrivilege") -> AdjustTokenPrivileges.
- Fallback anchor: IDB name `GetSEPrivilege` (already applied); called from CWvsApp::SetUp early (srand→GetSEPrivilege per the R11 init order).
- Cross-version stability: SeDebugPrivilege string + token-API quartet stable v79→v84.
- v79 address: 0x0044A48E (v83 seed 0x0044E824, v84 0x0044FEF9 — relocated by string+import, not by carry).

### CConfig::CConfig (ctor)   (memory-map keys: C_CONFIG / C_CONFIG_INSTANCE_ADDR)
- Primary anchor: IDB symbol `??0CConfig@@QAE@XZ` (nullary in v79; v84 was the PBD overload).
- Detail: SBB-singleton store `g_CConfig_pInstance = (this+4!=0)?this:0` (= C_CONFIG_INSTANCE_ADDR 0xB0BED0); installs vtable off_A2CAD0; member-inits the fixed 31/100/24 secure-fuse pattern (this[41]=31, this[43]=100, this[44]=24); StringPool::GetString(2532 = SOFTWARE\Wizet\MapleStory — v83 2547/v84 2548, DRIFT); RegOpenKeyExA(HKLM=0x80000002 via dword_B100A0) into this+192; memset(this+592, 0, 0x1BD/445); LoadGlobal (sub_493B5A) + ResetSessionInfo (sub_4968EB).
- Fallback anchor: call-graph — sole caller of LoadGlobal/ResetSessionInfo; ctor size 0x109 (matches v83/v84). C_CONFIG_INSTANCE_ADDR is read as `GetPartnerCode(g_CConfig_pInstance)` in CLogin::SendCheckPasswordPacket.
- Cross-version stability: 31/100/24 + SOFTWARE\Wizet StringPool + RegOpenKeyExA(HKLM)+memset(445) + SBB-singleton idioms stable; StringPool ID drifts per build.
- v79 address: C_CONFIG 0x0049392C; C_CONFIG_INSTANCE_ADDR 0x00B0BED0 (g_CConfig_pInstance).

### CConfig::GetPartnerCode   (memory-map key: C_CONFIG_GET_PARTNER_CODE)
- Primary anchor: IDB symbol `?GetPartnerCode@CConfig@@QAEJXZ`.
- Detail: sole referencer of literal `uiWndZ0` (aUiwndz0 @0xAC02B8); builds ZXString via CHATLOG_ADD then GetOpt_Int(0, key, 0, 0x80000000, 0x7FFFFFFF).
- Fallback anchor: structure — the GetOpt_Int(.,.,0,INT_MIN,INT_MAX) call with the single uiWndZ0 key; called from CLogin::SendCheckPasswordPacket.
- Cross-version stability: uiWndZ0 + GetOpt_Int(min,max) shape stable v79→v84.
- v79 address: 0x005CC09D.

### CConfig::ApplySysOpt   (memory-map key: C_CONFIG_APPLY_SYS_OPT)
- Primary anchor: IDB symbol `?ApplySysOpt@CConfig@@QAEXPAUCONFIG_SYSOPT@@H@Z`.
- Detail: `qmemcpy(this+100, a2, 0x30)`; writes the two CWvsContext singleton flags at +13868/+13872 from the game-start-mode this[35]; computes BGM/SE volumes `100*(this[26]+1)/20` (gated this[27]/this[29]); routes to CSoundMan::SetBGMVolume/SetSEVolume (dword_B0BEC8) and writes CInputSystem singleton +2416 = this[31] (InputSystemInstanceAddr 0xB0C29C).
- Fallback anchor: call-graph — SetBGMVolume/SetSEVolume + get_field trio; the 100*(x+1)/20 volume math.
- Cross-version stability: structurally identical v79→v84; member byte-offsets are per-version.
- v79 address: 0x004960F9.

### CConfig::CheckExecPathReg   (memory-map key: C_CONFIG_CHECK_EXEC_PATH_REG)
- Primary anchor: IDB symbol `?CheckExecPathReg@CConfig@@QAEXV?$ZXString@D@@@Z` + string xref (StringPool exec-path pair).
- Detail: gates on this[48] (the reg key handle from the ctor); StringPool::GetString(3114 = ExecPath value, 3115 = MapleStory.exe — v83 3135/3136, v84 3138/3139, DRIFT); builds a `"\\"`(92) separator via ZXString::operator=(92)+Right(1); compares stored vs running path via strcmp; on mismatch concatenates + GetFileAttributes (dword_B0FE3C -> ==-1 || &0x10) directory check; writes back via the reg-set wrapper sub_4965A9.
- Fallback anchor: structure — this[48] reg-handle gate + 92 backslash + GetFileAttributes(&0x10).
- Cross-version stability: reg-handle gate + backslash + GetFileAttributes(&0x10) stable; StringPool IDs drift.
- v79 address: 0x0049440C.

### CConfig sys-opt windowed-mode flag   (memory-map key: C_CONFIG_SYS_OPT_WINDOWED_MODE)
- Primary anchor: reader code site (two labeled functions). The global read by BOTH CWvsApp::CreateMainWindow (0x9440BB — the `flag!=0 ? 0x80000000 : 720896` window-style branch + the `?8:0` exstyle) AND CWvsApp::InitializeGr2D (0x944C91 — `pvargSrc.lVal = <flag>` feeding the Gr2D device init) — exactly the v83/v84 two-reader pattern.
- Fallback anchor: the 0x80000000 (fullscreen) vs 720896 (0x000B0000 windowed) style immediates fed by this flag in CreateMainWindow.
- Cross-version stability: the two-reader pattern + style immediates stable; global address per-version.
- v79 address: 0x00B11548 (renamed g_CConfig_SysOpt_WindowedMode; v83 0xBF1AC8, v84 0xC4B150). FOR TASK 13: this is the windowed-mode global the ConfigSysOpt audit cross-checks.

### CIGCipher::innoHash   (memory-map key: C_IG_CIPHER_INNO_HASH)
- Primary anchor: IDB symbol `?innoHash@CIGCipher@@SAKPAEHPAK@Z`.
- Detail: loops `bShuffle(v3, buf[i])` (sub_99347D) over `len` bytes, returns `*v3`; with no key (`if(!a3) v3=&v6`) seeds `v6 = -967814158` (0xC6EF3720). Called in CClientSocket::SendPacket between MakeBufferList and Flush as innoHash(this+132,4,0).
- Fallback anchor: the 0xC6EF3720 seed + bShuffle loop + the SendPacket call position.
- Cross-version stability: seed + bShuffle shape + SendPacket position stable v79→v84.
- v79 address: 0x00993442 (v83 0xA4A838, v84 0xA9669E).

### ZSynchronizedHelper<ZFatalSection> ctor/dtor   (memory-map keys: Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_CTOR / _DTOR)
- Primary anchor: CTOR has IDB symbol `??0?$ZSynchronizedHelper@VZFatalSection@@@@QAE@AAVZFatalSection@@@Z`; reached by call-graph as the SendPacket per-socket lock (CClientSocket::SendPacket calls it on this+124).
- Detail: CTOR (0x402AB8, size 0x25): `result = off_AC4ECC()` (the ZFatalSection acquire thunk); while non-zero, `dword_B0FDE4(0)` (Sleep(0)) and retry. DTOR (ctor+0x25 = 0x402ADD): `mov eax,[ecx]; dec dword[eax+4]; jnz; and dword[eax],0; retn` — decrements the recursion count, clears on last release; inlined as the release half in SendPacket and reached via `jmp loc_402ADD` from RAII unwind stub loc_9BD033.
- Fallback anchor: the acquire-loop/Sleep(0) retry (ctor) and the dec-and-clear (dtor) are near-unique; ctor+dtor are an adjacent acquire/release pair.
- Cross-version stability: **DRIFT — v79 relocated to 0x402AB8/0x402ADD vs v83/v84's stable 0x403166/0x40318B.** Confirm by the SendPacket lock-acquire call-edge + body, NOT by carrying the v83 address. The dtor listing is aliased under ZAllocEx::Alloc in this dump (do not trust list_funcs there); identity confirmed via the RAII pair + body bytes.
- v79 addresses: CTOR 0x00402AB8, DTOR 0x00402ADD.

### CSystemInfo: ctor / Init / GetMachineId / GetGameRoomClient   (keys: C_SYSTEM_INFO / _INIT / _GET_MACHINE_ID / _GET_GAME_ROOM_CLIENT)
- Primary anchor: IDB symbols (Init/GetMachineId/GetGameRoomClient retain mangled names) + string xref (Init). The CTOR had no symbol (was sub_99CDB0) — renamed ??0CSystemInfo@@QAE@XZ; it is the stack-construct call right before CSystemInfo::Init in CLogin::SendCheckPasswordPacket (sub_99CDB0/Init/GetMachineId/GetGameRoomClient/sub_99CDE0 dtor sequence over the same v9 stack object).
- Detail: ctor (0x99CDB0) installs vtable off_A396E4 (1 instruction). Init (0x99CDF0) = machine-id builder: Netbios (ncb_command 55/50/51 MAC query) + GetVolumeInformationA + RegOpenKeyExA `SOFTWARE\Microsoft\Windows\CurrentVersion` (aSoftwareMicros @0xB02FB8) + `CxSupportId` (aCxsupportid @0xB02FAC, RegQueryValueExA 16 bytes) + CoCreateGuid fallback. GetMachineId (0x99D0D0) returns the cached 16-byte id. GetGameRoomClient (0x99D1D0) = the 0x11B4 process-table fn.
- Fallback anchor: the ctor is the only construct/Init pair in SendCheckPasswordPacket; Init via CxSupportId; GetMachineId via EncodeBuffer(id,16) at the call site.
- Cross-version stability: the three mangled names + CxSupportId/CurrentVersion/Netbios anchors stable v79→v84; the cluster relocated wholesale (v83 0xA54Bxx → v79 0x99CDxx/0x99Dxxx).
- v79 addresses: C_SYSTEM_INFO 0x0099CDB0; C_SYSTEM_INFO_INIT 0x0099CDF0; C_SYSTEM_INFO_GET_MACHINE_ID 0x0099D0D0; C_SYSTEM_INFO_GET_GAME_ROOM_CLIENT 0x0099D1D0.

### ZArray<unsigned char>::RemoveAll   (memory-map key: Z_ARRAY_REMOVE_ALL)
- Primary anchor: IDB symbol `?RemoveAll@?$ZArray@E@@QAEXXZ`.
- Detail: `if(*this){ ZAllocEx::Free(*this-4); *this=0 }`. It is the FIRST call inside ZArray<uchar>::_Alloc (sub_48E8CB), which the COutPacket ctor uses with capacity 256.
- Fallback anchor: the `*this-4` Free + `*this=0` shape; clear-callee at the top of the packet-buffer _Alloc.
- Cross-version stability: stride-1 (no imul) instantiation; the generic ZArray<T>::RemoveAll variants used as struct-audit tools (Tasks 12–16) carry `imul stride,count` element-walks — re-derive stride per element type.
- v79 address: 0x004260F4 (v83 0x428CF1, v84 0x4297E5).

### ZXString<char>::GetBuffer (cstr-assign)   (memory-map key: Z_X_STRING_GET_BUFFER)
- Primary anchor: ABI/structure + call-graph. Repo wrapper (ZXString.h:55) calls it as `_fastcall(this, NULL, src, size)` (the edx slot is unused) — a cstr-assign. v79 has NO dedicated pure-assign GetBuffer(PBD,H) symbol; the matching in-place primitive is `?_Cat@?$ZXString@D@@IAEAAV1@PBDH@Z`.
- Detail: on an empty/zeroed ZXString it does `inner GetBuffer(Size,0)(0x4147bb) + memcpy(Src,Size) + ReleaseBuffer(Size)` (== assign); on a non-empty string it doubles capacity and appends — identical behavior to the v84 match.
- Fallback anchor: structure — inner-GetBuffer (0x4147bb) + memcpy + ReleaseBuffer operating in place on `this`.
- Cross-version stability: inner GetBuffer(HH) byte-stable; the outer assign primitive's identity is the carried uncertainty.
- v79 address: 0x00426133. **needs-main-review**: v83 key (0x414617) was a PURE-assign; v79 (like v84) matches the _Cat/append family that assigns only when the target is empty. Repo only invokes it on freshly-managed ZXStrings, so it behaves as assign — confirm that invariant or locate a dedicated pure-assign if one is added.

### ZXString<char>::TrimRight / TrimLeft   (keys: Z_X_STRING_TRIM_RIGHT / Z_X_STRING_TRIM_LEFT)
- Primary anchor: IDB symbols `?TrimRight@?$ZXString@D@@QAEAAV1@PBD@Z` / `?TrimLeft@?$ZXString@D@@QAEAAV1@PBD@Z` + the whitespace literal.
- Detail: both default `Str` to `" \t\r\n"` (asc_ABEDA0 @0xABEDA0) when NULL and use strchr to test set-membership, calling inner GetBuffer (0x4147bb). TrimRight scans backward, NUL-terminates after the last non-set char; TrimLeft scans forward then memcpy-shifts the remainder to the front. ADJACENT in the image.
- Fallback anchor: the shared " \t\r\n" literal + right-scan vs left-scan+memcpy distinction.
- Cross-version stability: whitespace literal + strchr + inner-GetBuffer shape stable v79→v84; the literal's address is per-version (v79 asc_ABEDA0). Repo calls them `(this, NULL, s)` (ZXString.h:209/214).
- v79 addresses: TrimRight 0x0046DB7E; TrimLeft 0x0046DC33.

### CMob::CMob (ctor)   (memory-map key: C_MOB_C_MOB)   [HIGH-VALUE / needs-main-review — doom-fix hook target, feeds Task 12]
- Primary anchor: IDB symbol `??0CMob@@QAE@PAVCMobTemplate@@@Z`.
- Detail: placement ctor over a NON-zeroed 1304-byte (0x518) allocation (sole caller CreateMob 0x630BF0 → ZAllocEx<ZAllocAnonSelector>::Alloc(1304)). Body: CLife base ctor (sub_5C5D12); zero-inits the member block; stores `m_pTemplate = pMobTemplate` at this+0x188 (this+98 dword) and zeroes the adjacent m_pTemplateByDoom at this+0x18C (this+99); installs the three CMob vtables at this+0/+1/+2 (off_A30C48/off_A30C24/off_A30C20); runs the _ZtlSecureTear chain over the secured stat members (vtable off_A2F7B4 + 31/100/24 fuse); MobStat::SetFrom(this+416, m_pTemplate); CWvsContext::SetExclRequestSent; ends with StringPool::GetStringW(957 = SP_CANVAS; v83 956/v84 960, DRIFT) + PcCreateObject::IWzCanvas into the HP-indicator canvas at this+0x4D0.
- Fallback anchor (spot-check, independent kind): sole caller CreateMob (0x630BF0); the CLife-base + 3-vtable install + m_pTemplate store + _ZtlSecureTear chain + SP-957/IWzCanvas tail — confirm WITHOUT the symbol.
- Cross-version stability: CLife-base + 3-vtable + CMobTemplate-store + secure-tear + SP_CANVAS/IWzCanvas tail stable v79→v84; StringPool ID and vtable globals per-version.
- v79 address: 0x00630C2C (v83 0x6621D9, v84 0x678060).
- **DOOM-FIELD FINDING (Task 12 / doom-fix gate `< 84`):** the v79 ctor LEAVES `m_bDoomReserved` UNINITIALIZED → v79 is correctly on the doom-fix needs-fix side. Evidence: (1) the allocation is non-zeroing (ZAllocEx::Alloc(1304), not calloc); (2) the ctor's highest member write is the secure-tear backing dword at this+325 (0x514) and the HP-canvas at this+0x4D0 — there is NO trailing zero-init block reaching a doom field. Contrast v84 (no-fix side, gate >=84): its ctor at 0x678060 explicitly zero-writes `*((_DWORD*)this+336)=0` (0x540 = m_bDoomReserved) and `*((_BYTE*)this+1348)=0` (0x544 = m_bDoomReservedSN) as a contiguous tail block right after the v84-only fields (m_aMultiTargetForBall/m_aRandTimeforAreaAttack/m_delaySkill, all `>=84`). v79 lacks both those `>=84` fields AND the corresponding doom zero-init. NOTE: the exact v79 byte offset of m_bDoomReserved was not pinned (embedded MobStat may be smaller in v79 than v84, so arithmetic from v84's 0x540 is unsafe) — Task 12's read-only struct audit will measure MobStat/CMob sizes and pin it; the uninitialized verdict stands on the v84 contrast + non-zeroing alloc + absence of any tail doom write.

---

## Task 9 — Party / migrate / context senders + offsets

> The v79 IDB retains the full mangled C++ symbols for all four functions in this
> cluster (`?SendJoinPartyMsg@CField@@…`, `?SendCreateNewPartyMsg@CField@@…`,
> `?SendMigrateToITCRequest@CWvsContext@@…`, `?OnEnterGame@CWvsContext@@…`), so the
> IDB symbol is the fast primary anchor; each entry records a second STRUCTURAL
> anchor (opcode/encoder-chain, string xref, or SetStage-trio call-graph) per the
> two-anchor rule. **Opcode drift (R7): the v79 party opcode is 0x79 (v83 0x7C) and
> the v79 ITC opcode is 0x99 (v83 0x9C) — read, never copied.** All four offsets
> were re-measured from v79 disasm and cross-checked against the v83 host at the
> v83 offset (same patch-point instruction kind in both).

### CField::SendCreateNewPartyMsg   (keys: C_FIELD_SEND_CREATE_NEW_PARTY_MSG / _OFFSET)   [HIGH-VALUE / needs-main-review]
- Primary anchor: IDB symbol `?SendCreateNewPartyMsg@CField@@QAEXXZ` (nullary).
- Detail: reads the CWvsContext singleton (g_pWvsContext @0xB07848), runs the job-id gate (`_ZtlSecureFuse<short>` job at +0x39/+0x3D vs 0/3E8h/7D0h) and the `_ZtlSecureFuse<uchar>` level gate (`cmp al,0Ah; jnb`), then on pass `COutPacket(0x79)` -> `Encode1(1)` (create sub-opcode) -> `CClientSocket::SendPacket`. Nullary + sub-opcode 1, no EncodeStr — distinguishes it from SendJoinPartyMsg.
- Fallback anchor (2nd kind): opcode/encoder-chain — the COutPacket(0x79) ctor + Encode1(1) + SendPacket tail with no string arg.
- Cross-version stability: the 3E8/7D0 job constants + `cmp al,0Ah` level gate + Encode1(1)/SendPacket tail stable v83→v84→v79. **OPCODE per-version: v83 0x7C → v84 0x7E → v79 0x79.** StringPool error IDs shift (v79 320/324).
- v79 address: 0x0051B318 (size 0x118).
- **OFFSET (C_FIELD_SEND_CREATE_NEW_PARTY_MSG_OFFSET):** patch-point = level-gate `jnb short loc_51B3EB` @0x51B3B5 (bytes `73 34`); the no-beginner-party edit overwrites 0x73→0xEB. Delta 0x51B3B5−0x51B318 = **0x9D** — DIFFERS from v83 0xA4 (v79 preamble is 7 bytes shorter; same jnb instruction confirmed in v83 @0x52FD85 = `73 34`).

### CField::SendJoinPartyMsg   (keys: C_FIELD_SEND_JOIN_PARTY_MSG / _OFFSET)   [HIGH-VALUE / needs-main-review]
- Primary anchor: IDB symbol `?SendJoinPartyMsg@CField@@QAEXABV?$ZXString@D@@@Z` (one-arg invitee ZXString).
- Detail: same job-id + level gates as SendCreateNewPartyMsg, plus a `strcmp` vs the player's own name and a `GetPartyMemberNumber < 6` party-full guard; on pass `COutPacket(0x79)` -> `Encode1(4)` (invite sub-opcode) -> `EncodeStr(name)` -> `CClientSocket::SendPacket`. String arg + EncodeStr + sub-opcode 4 distinguish it from CreateNewParty.
- Fallback anchor (2nd kind): opcode/encoder-chain — COutPacket(0x79)+Encode1(4)+EncodeStr+SendPacket; reads g_pWvsContext. **Both party senders share opcode 0x79 — disambiguate by sub-opcode (4=invite vs 1=create) + EncodeStr, never by a second opcode.**
- Cross-version stability: gates + strcmp + GetPartyMemberNumber(<6) + Encode1(4)/EncodeStr/SendPacket stable v83→v84→v79. **OPCODE per-version: v83 0x7C → v84 0x7E → v79 0x79.** StringPool IDs shift (v79 324/325/326/330).
- v79 address: 0x0051B4C9 (size 0x1EB).
- **OFFSET (C_FIELD_SEND_JOIN_PARTY_MSG_OFFSET):** patch-point = level-gate `jnb short loc_51B558` @0x51B527 (bytes `73 2F`); edit overwrites 0x73→0xEB. Delta 0x51B527−0x51B4C9 = **0x5E** — DIFFERS from v83 0x65 (v79 preamble shorter; the v79 guest-id/own-name checks come AFTER the level gate, unlike v84's pre-gate guest-id block; same jnb instruction confirmed in v83 @0x52FF34 = `73 2F`).

### CWvsContext::SendMigrateToITCRequest   (keys: C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST / _OFFSET)   [HIGH-VALUE / needs-main-review]
- Primary anchor: string xref — the unique literal `The MapleStory Trading System is not available for the Guest ID Users.` (aTheMaplestoryT @0xB027FC).
- Detail: guest-ID guard ([this+0x2090]), a `>= 0x1F4` throttle (SetExclRequestSent delta), and the ITC-availability gate (`get_field()`->[+0x124]->`shr 4`->`and 1`->`jz`), then on pass `COutPacket(0x99)` -> `CClientSocket::SendPacket` (no encoder body).
- Fallback anchor (2nd kind): opcode/SendPacket call-graph — COutPacket(0x99) ctor + SendPacket tail with no encoders; reads CWvsContext member throttle timestamps at this+0x20A4/+0x20A8.
- Cross-version stability: Guest-ID string + and-1 ITC gate + 0x1F4 throttle stable v83→v84→v79. **OPCODE per-version: v83 0x9C → v84 0xA0 → v79 0x99.**
- v79 address: 0x0095DD85 (size 0x163).
- **OFFSET (C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST_OFFSET):** patch-point = ITC-gate `jz short loc_95DE96` @0x95DE6E (bytes `74 26`); the no-enter-mts edit overwrites 0x74→0xEB to always take the send path. Delta 0x95DE6E−0x95DD85 = **0xE9** — coincides with v83 0xE9 (same jz instruction confirmed in v83 @0xA1260B = `74 26`; identical preamble length).

### CWvsContext singleton / CWvsContext::OnEnterGame   (keys: C_WVS_CONTEXT_INSTANCE_ADDR / C_WVS_CONTEXT_ON_ENTER_GAME / _OFFSET)
- Primary anchor: call-graph (SetStage trio) — SetStage (SET_STAGE @0x6F1AC0) at 0x6F1B63 does `mov ecx, g_pWvsContext; call OnEnterGame` in its GetCharacterData!=0 branch (the OnEnterGame/OnLeaveGame/OnGameStageChanged trio). OnEnterGame itself runs the CWvsContext member sub-object ctors at this+0x34xx.
- Fallback anchor: IDB symbol `?OnEnterGame@CWvsContext@@QAEXXZ`; the singleton (g_pWvsContext @0xB07848) is the only global shared by the SetStage trio AND both CField party senders (as GetCharacterData `this`) AND CClientSocket::ProcessPacket (Task 4) — and it sits at g_pClientSocketInstance(0xB07844)+4, the canonical v83 socket-then-context layout.
- Cross-version stability: SetStage-trio + GetCharacterData branch shape stable v83→v84→v79; the singleton VA and CWvsContext member offsets are per-version (v83 0x2EDx → v84 0x35xx → v79 0x34xx) — locate via SetStage's trio, not the raw address. The singleton has no TSingleton<CWvsContext> CreateInstance symbol in v79 (plain global, renamed g_pWvsContext in the IDB).
- v79 addresses: C_WVS_CONTEXT_INSTANCE_ADDR 0x00B07848; C_WVS_CONTEXT_ON_ENTER_GAME 0x00950297 (size 0x1F1).
- **OFFSET (C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET):** the first body instruction after the EH-prolog + register-save block (`push ecx/push esi/mov esi,ecx/push edi`) — in v79 that is `lea ecx,[esi+3424h]` @0x9502A6 (the first this+0x34xx member-ctor). Delta 0x9502A6−0x950297 = **0x0F** — DIFFERS from v83 0x10 (whose +0x10 is `push 1` = `6A 01`; v79 omits the `push 1` setup, matching v84's 0x0F). Not consumed by an active edit (carried for sibling-map parity).
