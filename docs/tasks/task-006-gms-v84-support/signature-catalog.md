# Reusable Function-Identification Signature Catalog

**Purpose.** v84 is not the last version we will port this way. For each
non-trivial function we relocate, this catalog records *how* we found it in a
sparsely-labeled IDB so the next version port can reuse the heuristic instead of
re-discovering it. Prefer version-stable anchors (strings, imports, call-graph,
constants) over raw byte signatures, which break when codegen changes.

This file is populated during the port (it starts mostly empty by design). It is
a durable, version-agnostic artifact — keep entries phrased about the *function*,
not about a specific address.

## How to use

- When you resolve a key, add or update its row below.
- Record the **most stable** anchor that actually worked, plus a fallback.
- Note observed stability across versions you've checked (e.g. "string present in
  v83 and v84"; "byte sig matched v83 but not v87").
- If you also want the heuristic encoded in code/comments, put a short comment at
  the relevant patch site in source referencing this catalog entry.

## Entry schema

```
### <CANONICAL_FUNCTION_NAME>   (memory-map key: <KEY>)
- Primary anchor: <string xref | import call | call-graph | constant | vtable slot | byte sig>
- Detail: <the exact literal / API / parent / constant / slot index used>
- Fallback anchor: <secondary method if primary is absent>
- Cross-version stability: <which versions confirmed; known breakages>
- Notes: <pitfalls, ambiguity, near-duplicates to disambiguate>
```

## Catalogued anchors (populate during port)

### WinMain   (memory-map key: WIN_MAIN)
- Primary anchor: string xref
- Detail: Two unique startup literals reference WinMain directly: `MapleStoryGlobal :: MapleStory - Microsoft Internet Explorer` (passed to activate_web_site) and `\npkgameuninstnomsg.exe`. xrefs_to either string lands inside the WinMain function body.
- Fallback anchor: call-graph from the PE entry `start` — the only thing `start` calls into the user image is WinMain (single xref).
- Cross-version stability: both strings present and unique in v83 and v84. The IExplorer-title string is the single most reliable WinMain anchor across versions.
- Notes: v84 WinMain @ 0x00A39FA0 (named `_WinMain@16`). Body mirrors v83 (StringPool IDs, single-instance mutex, ShowStartUpWndModal patcher call, CWvsApp ctor/SetUp/Run, ShowADBalloon). High-value; spot-checked via `start` call-graph.

### SendHSLog   (memory-map key: SEND_HS_LOG)
- Primary anchor: string xref
- Detail: References `%s\HShield` and `MapleStory_Global:%s` format strings; final call is the AhnLab `AhnHS_SendHsLogA` thunk.
- Fallback anchor: call-graph adjacency — sits immediately before WinMain in the image.
- Cross-version stability: both format strings present in v83 and v84.
- Notes: v84 @ 0x00A39EC9. Builds `<exe-dir>\HShield` + `MapleStory_Global:<char>` then calls the HS log import.

### CWvsApp::CWvsApp (ctor)   (memory-map key: C_WVS_APP)
- Primary anchor: import call + string
- Detail: References `WebStart`, `GameLaunching`, `kernel32.dll`/`IsWow64Process`; installs the CWvsApp vtable; stores the singleton (`(this+4!=0)?this:0` SBB pattern then `mov [ms_pInstance], ecx`).
- Fallback anchor: call-graph — invoked from WinMain right before SetUp.
- Cross-version stability: command-line keyword strings and the IsWow64Process probe stable v83→v84.
- Notes: v84 @ 0x00A3D719 (IDB-provided mangled name `??0CWvsApp@@QAE@PBD@Z`). The singleton store target is C_WVS_APP_INSTANCE_ADDR. High-value; spot-check = mangled RTTI name independent of the WinMain call-graph.

### CWvsApp::SetUp   (memory-map key: C_WVS_APP_SET_UP)
- Primary anchor: call-graph
- Detail: Called from WinMain immediately after the ctor. In v84 it is NOT virtualized (clean body) and is the init driver: calls InitializeAuth, srand, the `Global\meteora` mutex check, `ehsvc.dll` presence test, the WS2_32 IAT-clone anti-tamper, and the per-subsystem Initialize*/Create* calls in fixed order, finishing with a CRC32 table build.
- Fallback anchor: distinctive strings/constants — `Global\meteora`, `ehsvc.dll`, `ws2_32.dll`, CRC poly init.
- Cross-version stability: v83 SetUp is control-flow-virtualized (junk decompile); v84 is de-virtualized. Anchor on call-graph + the meteora/ehsvc strings, NOT on byte layout.
- Notes: v84 @ 0x00A3DDCC. Walking its callees is the call-graph anchor for the whole Initialize*/Create* cluster. High-value; spot-check = meteora/ehsvc strings.

### CWvsApp::Run   (memory-map key: C_WVS_APP_RUN)
- Primary anchor: call-graph
- Detail: Main message-pump loop; invoked from WinMain right after SetUp. Per iteration pumps the OS message queue, then calls CallUpdate and CWndMan::RedrawInvalidatedWindows; throws CPatchException / CDisconnectException / CTerminateException; loop exits on WM_QUIT (msg type 18).
- Fallback anchor: the three exception type-info refs (`_TI3?AVCPatchException@@`, `...CDisconnectException@@`, `...CTerminateException@@`) are a distinctive trio.
- Cross-version stability: v83 Run is virtualized; v84 is clean. Anchor on WinMain call-graph + the CallUpdate/RedrawInvalidatedWindows callee pair, NOT byte layout.
- Notes: v84 @ 0x00A3E7E8. High-value; spot-check = the exception-type trio independent of WinMain call-graph.

### CWvsApp::InitializeAuth   (memory-map key: C_WVS_APP_INITIALIZE_AUTH)
- Primary anchor: call-graph + import
- Detail: First subsystem call from SetUp; calls `CNMCOClientObject::GetInstance` (mangled name survives in v84) then SetPatchOption / SetLocaleAndRegion(512,201) / Initialize(33563155); throws CTerminateException with codes 0x22000{B,C}.
- Fallback anchor: the locale/region magic args (512, 201) and Initialize arg 33563155.
- Cross-version stability: CNMCOClientObject mangled name and the magic args stable v83→v84.
- Notes: v84 @ 0x00A401E7.

### CWvsApp::InitializePCOM   (memory-map key: C_WVS_APP_INITIALIZE_PCOM)
- Primary anchor: call-graph + import
- Detail: Tiny; calls PcInitialize(0) (the Wz package COM init), raises a COM error on failure, sets this->m_bPCOMInitialized = 1.
- Fallback anchor: smallest CWvsApp Initialize* (a few instructions) setting a bool member.
- Cross-version stability: structurally identical v83→v84.
- Notes: v84 @ 0x00A3FDA2 (PcInitialize = sub_7BA8CC).

### CWvsApp::CreateMainWindow   (memory-map key: C_WVS_APP_CREATE_MAIN_WINDOW)
- Primary anchor: import call + string
- Detail: Registers a window class (cbWndExtra=72, the CWvsApp::WindowProc) and calls CreateWindowExA; uses StringPool IDs for MapleStoryClass / MapleStory; windowed-vs-fullscreen branch on the windowed-mode flag (0x80000000 vs 0x000B0000); throws ZException if hWnd is null.
- Fallback anchor: the CreateWindowExA call + the 0x80000000/720896 style branch.
- Cross-version stability: CreateWindowExA + class/title string IDs stable.
- Notes: v84 @ 0x00A3FDD1; WindowProc = sub_A48A23.

### CWvsApp::ConnectLogin   (memory-map key: C_WVS_APP_CONNECT_LOGIN)
- Primary anchor: call-graph (two independent callers)
- Detail: Called by CLogin::GotoTitle (caller fn size 0x10f) and CWvsContext::ReturnToTitle (caller fn size 0x82) — the same two callers as v83, both size-matched. Also called once from SetUp (initial connect). Body is a socket-connect + message-pump wait loop (socket global, exits on msg 18), throwing on no socket.
- Fallback anchor: the two size-matched callers (GotoTitle 0x10f, ReturnToTitle 0x82).
- Cross-version stability: the GotoTitle/ReturnToTitle caller pair is a stable call-graph signature; v83 ConnectLogin itself is virtualized.
- Notes: v84 @ 0x00A3FFE8.

### CWvsApp::InitializeResMan   (memory-map key: C_WVS_APP_INITIALIZE_RES_MAN)
- Primary anchor: string xref
- Detail: References the WZ data-type literal list `Character, Skill, Reactor, UI, Quest, Item, Effect, String, Morph, TamingMob, Sound` (+ off_ entries for Mob/Npc/Etc/Map) and `Base.wz`; mounts the ResMan/NameSpace COM packages; calls Dir_BackSlashToSlash.
- Fallback anchor: the 15-entry data-type string table iterated in a for(i<15) loop.
- Cross-version stability: the data-type literal list is extremely stable across versions.
- Notes: v84 @ 0x00A402CB.

### CWvsApp::InitializeGr2D   (memory-map key: C_WVS_APP_INITIALIZE_GR2D)
- Primary anchor: constant + COM call
- Detail: Creates the Gr2D DX8 package, calls the device init vtable slot with width 800 / height 600, sets clear color 0xFF000000 (-16777216); branches on the windowed-mode flag.
- Fallback anchor: the 800/600 + 0xFF000000 immediates feeding a COM vtable call.
- Cross-version stability: 800x600 + alpha-clear stable.
- Notes: v84 @ 0x00A4113C.

### CWvsApp::InitializeInput   (memory-map key: C_WVS_APP_INITIALIZE_INPUT)
- Primary anchor: string + call-graph
- Detail: 5th subsystem call from SetUp; verifies `ws2_32.dll` / `getpeername` by mapping the on-disk DLL and comparing the export against the IAT (anti-hook), wires the CInputSystem.
- Fallback anchor: the `\\ws2_32.dll` + getpeername integrity scan + CreateFileMapping/MapViewOfFile.
- Cross-version stability: ws2_32/getpeername integrity pattern stable; v83 InitializeInput is virtualized so use this content.
- Notes: v84 @ 0x00A4153D.

### CWvsApp::InitializeSound   (memory-map key: C_WVS_APP_INITIALIZE_SOUND)
- Primary anchor: constant + call-graph
- Detail: Allocates the CSoundMan (0x64 bytes) and calls CSoundMan::Init(hWnd, 30000, 2, 2, 16).
- Fallback anchor: the literal Init arg sequence 30000, 2, 2, 16.
- Cross-version stability: the (30000,2,2,16) Init args are a stable fingerprint.
- Notes: v84 @ 0x00A41B18.

### CWvsApp::InitializeGameData   (memory-map key: C_WVS_APP_INITIALIZE_GAME_DATA)
- Primary anchor: call-graph + constant
- Detail: Allocates and validates the run of game-data manager singletons (skill/quest/item/etc.), each validated with a Get* returning a pointer or throwing CTerminateException 0x22000006; ends by building several manager tables.
- Fallback anchor: the repeated alloc -> ctor -> validate(throw 0x22000006) pattern.
- Cross-version stability: the 0x22000006 terminate code + manager-validation chain stable; v83 is virtualized.
- Notes: v84 @ 0x00A423F1.

### CWvsApp::CreateWndManager   (memory-map key: C_WVS_APP_CREATE_WND_MANAGER)
- Primary anchor: call-graph + singleton store
- Detail: Constructs the root CWndMan (vtable off_B930A8) and stores it to the CWndMan singleton; then lays out the screen-spanning window (0,0,800,600) with centered (-400,-300) layers.
- Fallback anchor: the 800x600 root-window rect + the CWndMan singleton store.
- Cross-version stability: screen-rect layout stable; v83 is virtualized.
- Notes: v84 @ 0x00A4016D. The CWndMan singleton it writes (dword_C4555C) is the global read by ISMsgProc/s_Update.

### CWvsApp::GetCmdLine   (memory-map key: C_WVS_APP_GET_CMD_LINE)
- Primary anchor: IDB symbol / call-graph
- Detail: v84 retains the mangled name `?GetCmdLine@CWvsApp@@QAE?AV?$ZXString@D@@H@Z`; also called repeatedly from the CWvsApp ctor to parse WebStart/GameLaunching args.
- Fallback anchor: call-graph from the ctor.
- Cross-version stability: mangled name present in both v83 and v84.
- Notes: v84 @ 0x00A430EA.

### CWvsApp::Dir_BackSlashToSlash / Dir_SlashToBackSlash / Dir_UpDir   (memory-map keys: C_WVS_APP_DIR_BACK_SLASH_TO_SLASH / C_WVS_APP_DIR_SLASH_TO_BACK_SLASH / C_WVS_APP_DIR_UP_DIR)
- Primary anchor: constant (path-separator char) + adjacency
- Detail: Three tiny adjacent helpers. BackSlashToSlash replaces '\\'(92)->'/'(47); SlashToBackSlash replaces '/'(47)->'\\'(92); UpDir strips a trailing '/' then truncates at the last '/'. They sit in the same order in v83 and v84.
- Fallback anchor: called from InitializeResMan (BackSlashToSlash) and GetExceptionFileName (SlashToBackSlash).
- Cross-version stability: trivial char-swap loops; the 47/92 immediates identify direction.
- Notes: v84 @ 0x00A43330 (Back->Slash), 0x00A43371 (Slash->Back), 0x00A433B2 (UpDir).

### CWvsApp::GetExceptionFileName   (memory-map key: C_WVS_APP_GET_EXCEPTION_FILE_NAME)
- Primary anchor: call-graph + structure
- Detail: Returns a cached static char* (lazy: `if (!buf[0]) {...}`); builds the dump path with StringPool ID and Dir_SlashToBackSlash. Called from the very top of WinMain.
- Fallback anchor: the static-buffer lazy-init returning char*.
- Cross-version stability: structurally identical; static-cache shape stable.
- Notes: v84 @ 0x00A43663 (static buffer byte_C4B158).

### CWvsApp::CallUpdate   (memory-map key: C_WVS_APP_CALL_UPDATE)
- Primary anchor: call-graph + constant
- Detail: Per-frame catch-up loop advancing the frame clock in 30 ms steps, calling CWndMan::s_Update each step; contains the periodic getpeername heartbeat anti-tamper (30000*v interval) and the ehsvc/v3warpns integrity checks.
- Fallback anchor: the `this[6] += 30` frame-step + s_Update call; it is the (only) caller of s_Update.
- Cross-version stability: 30 ms frame step stable; v83 CallUpdate is virtualized.
- Notes: v84 @ 0x00A41D42 (taking (this, tCurTime)).

### CWvsApp::ISMsgProc   (memory-map key: C_WVS_APP_IS_MSG_PROC)
- Primary anchor: constant + call-graph
- Detail: Dispatches message==0x100 (WM_KEYDOWN) to CWndMan::ProcessKey and 0x200<msg<=0x20A (mouse range) to CWndMan::ProcessMouse, gated on the CWndMan singleton being non-null.
- Fallback anchor: the 0x100 / 0x1FF..0x20A range constants + the CWndMan singleton read.
- Cross-version stability: the message-range constants are stable.
- Notes: v84 @ 0x00A435EA. The v83 cmake value was function-entry+5 (an artifact landing mid-instruction in this IDB); v84 uses the clean function entry — correct for the repo's __fastcall(this, void*, msg, w, l) shim (the void* consumes edx, leaving the 3 stack args correct).

### CWndMan::s_Update   (memory-map key: C_WND_MAN_S_UPDATE)
- Primary anchor: constant + import call
- Detail: Drains the deferred-destroy window list, then on cursor-idle (`timeGetTime() - lastInput > 0x3A98`) hides the cursor and walks the window tree. The 0x3A98 (15000 ms) timeout next to a timeGetTime call is a unique fingerprint.
- Fallback anchor: call-graph — the sole callee of interest inside CallUpdate.
- Cross-version stability: 0x3A98 + timeGetTime stable v83→v84.
- Notes: v84 @ 0x00A2CBEB.

### CWndMan::RedrawInvalidatedWindows   (memory-map key: C_WND_MAN_REDRAW_INVALIDATED_WINDOWS)
- Primary anchor: structure + adjacency
- Detail: COM-variant redraw loop over the invalidated-window list; the inner per-child call is vtable slot +140 with a (0,0,&x,&y,0,0,0,0,...) arg shape, then a rect computed as base + child[7]/child[8], then an invalidate via vtable +32. Function size 0x27C in both v83 and v84.
- Fallback anchor: adjacency — directly precedes s_Update in the image (same as v83).
- Cross-version stability: the vtable+140 redraw call shape and the 0x27C size both held v83->v84.
- Notes: v84 @ 0x00A2C96F.

### globals: CWvsApp singleton / g_dwTargetOS   (memory-map keys: C_WVS_APP_INSTANCE_ADDR / G_DW_TARGET_OS)
- Primary anchor: writer function
- Detail: The CWvsApp singleton is the `mov [global], ecx` store in the CWvsApp ctor (after the `(this+4!=0)?this:0` SBB idiom). g_dwTargetOS is the DWORD set to 1996 (0x7CC) in the ctor when OS major < 5 OR IsWow64Process is true — exactly the two write sites the GMS bypass patch relies on.
- Fallback anchor: g_dwTargetOS reads back as 1996; the singleton is read in CWvsContext::ReturnToTitle's m_nGameStartMode==2 check.
- Cross-version stability: both writer idioms stable; locate via the ctor, not the raw address.
- Notes: v84 C_WVS_APP_INSTANCE_ADDR @ 0x00C40E88; G_DW_TARGET_OS @ 0x00C3C204.

### WinMain offsets   (memory-map keys: WIN_MAIN_AD_BALLOON_CONDITIONAL / WIN_MAIN_PATCHER_OFFSET)
- Primary anchor: re-measured from v84 WinMain disasm (never copied from v83)
- Detail: PATCHER_OFFSET points at `call ShowStartUpWndModal` (sub_A39E77, the launcher.html window) @ 0xA3A1E1 → delta 0x241 from WinMain base 0xA39FA0; the no-patcher edit NOPs these 5 bytes. AD_BALLOON_CONDITIONAL points at the `jz` guarding the ShowADBalloon block (740/300/60 + sub_4236EA) @ 0xA3AA0E → delta 0xA6E; the no-ad-balloon edit overwrites the first byte 0x74 with 0xEB.
- Fallback anchor: the patcher call target references `http://Ingameweb.nexon.net/maplestory/client/launcher.html`; the ad-balloon block is immediately followed by the `MapleStoryGlobal :: ...` string push.
- Cross-version stability: offsets MUST be re-measured per version; v83 were 0x212 / 0xA3D, v84 are 0x241 / 0xA6E.
- Notes: verified bytes — 0xA3A1E1 = E8 91 FC FF FF (call), 0xA3AA0E = 74 6F (jz short).

<!--
Add one section per resolved key. Group loosely by subsystem to mirror
memory-map.md. Leave the two stubs above as worked examples of the schema.
-->

## v84 IDB baseline

Recorded 2026-06-05 via `mcp__ida-pro__survey_binary` with instance routed to port 13341.

| Field | Value |
|---|---|
| Module | `GMS_v84.1_U_DEVM.exe` |
| IDB path | `E:\Programs\Nexon\IDBs_v9\GMS\v84_1\GMS_v84.1_U_DEVM.i64` |
| Image base | `0x00400000` |
| Architecture | x86 32-bit |
| Image size | `0x00BC6000` |
| Entry point (`start`) | `0x00AB01AF` |
| `.text` segment | `0x00401000` – `0x00B41000` (size `0x740000`) |
| Total functions | 59385 (249 named, 550 library, 58586 unnamed) |

WinMain offset math anchor: image base `0x00400000`; subtract from any absolute address
to get the RVA used in `WIN_MAIN` and sibling keys.

## Heuristic playbook (general, version-agnostic)

- **String xrefs are king.** Format strings, file paths ("Data\\…"), registry
  keys, and error literals survive across versions far better than code bytes.
- **Manager singletons** (`*::CreateInstance` / `*_INSTANCE_ADDR`) cluster
  together in the static-init region and are typically called in a fixed order
  from one initializer — find one, the neighbors are adjacent calls.
- **`*_INSTANCE_ADDR` globals** are the `mov [g_xxx], eax` store right after the
  matching `CreateInstance` call returns. Resolve the function, read its
  caller's store target.
- **Packet senders** reference their opcode as a `push <imm>` / `mov` immediate
  into a COutPacket; the opcode constant disambiguates among similar senders.
- **`*_OFFSET` keys** are call-site or branch offsets *within* a host function;
  always re-measure against v84 disassembly.
- **Vtable-derived keys** (`C_LOGO_GET_RTTI`, `*_VFTABLE`): find the class
  vtable via its RTTI/type string, then index the slot.
- When two candidates are indistinguishable, disambiguate by a unique callee or
  by the surrounding string the real one references — don't guess by address
  proximity to v83 alone.
