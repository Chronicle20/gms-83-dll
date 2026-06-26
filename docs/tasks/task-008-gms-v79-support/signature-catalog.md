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
- Fallback anchor: call-graph/adjacency — sits immediately before WinMain (0x93F8E0 + 0xD7 = 0x93F9B7) and is called 3× from WinMain.
- Cross-version stability: both format strings present in v79, v83, v84. IDB pre-named `SendHSLog`.
- v79 address: 0x0093F8E0 (size 0xD7)
- Notes: clean transfer from the v84 anchor.

### CWvsApp::CWvsApp (ctor)   (memory-map key: C_WVS_APP)
- Primary anchor: IDB symbol (`??0CWvsApp@@QAE@PBD@Z`)
- Detail: References `WebStart` (@0xB023D8), `GameLaunching` (@0xB023C0), `token` (@0xB023D0), `kernel32.dll`/`IsWow64Process`; installs the CWvsApp vtable off_A385C0; SBB-stores the singleton (`(a1+1!=0)?a1:0` → g_CWvsApp); writes g_dwTargetOS=1996 on OS major<5 / WoW64; calls ResetLSP at the tail.
- Fallback anchor: call-graph — invoked from WinMain right before SetUp.
- Cross-version stability: command-line keyword strings + IsWow64Process probe stable v79→v84. Singleton + g_dwTargetOS writer idioms identical.
- v79 address: 0x00942D3B (size 0x353)
- Notes: HIGH-VALUE (needs-main-review). Two kinds (symbol + WebStart/IsWow64Process string cluster). Spot-check: WebStart xref → same fn, independent of symbol.

### CWvsApp::SetUp   (memory-map key: C_WVS_APP_SET_UP)
- Primary anchor: IDB symbol (`?SetUp@CWvsApp@@QAEXXZ`)
- Detail: Init driver, called from WinMain right after the ctor. v79 NOT virtualized (clean body). References `Global\meteora` (strcpy literal) + `ehsvc.dll` (@0xB023E8) + `ws2_32.dll`/`getpeername`/`WSAStartup`; builds the CRC32 table (poly 0xDD04D9C1). Walking its callees is the call-graph anchor for the whole Initialize*/Create* cluster.
- Fallback anchor: distinctive strings — meteora/ehsvc/getpeername; the WS2_32 IAT-clone integrity scan.
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
