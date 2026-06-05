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

### CClientSocket::SendPacket   (memory-map key: C_CLIENT_SOCKET_SEND_PACKET)
- Primary anchor: IDB symbol (mangled name)
- Detail: `?SendPacket@CClientSocket@@QAEXABVCOutPacket@@@Z` survives in the v84 IDB. Body grabs the per-socket ZFatalSection (ZSynchronizedHelper<ZFatalSection> ctor = sub_403166, same addr v83→v84), validates the socket fd ([this+8] != 0/-1) and the send-disabled flag ([this+14h]), then pushes the send-seq opcode and calls COutPacket::MakeBufferList -> CIGCipher::innoHash -> CClientSocket::Flush.
- Fallback anchor: call-graph — the callee chain MakeBufferList + innoHash + Flush is a unique fingerprint; the last call in the body IS Flush. Also reachable from the free global `?SendPacket@@YAXABVCOutPacket@@@Z` which calls `CClientSocket::SendPacket(ms_pInstance, pkt)`.
- Cross-version stability: mangled name present in v83 and v84; the ZSynchronizedHelper ctor address (0x403166) is identical across both. The push'd opcode is a per-version constant: 0x53 ('S') in v83, 0x54 ('T') in v84 — do NOT use it as a cross-version anchor.
- Notes: v84 @ 0x0049B28C. HIGH-VALUE (needs-main-review). Two different kinds (symbol + call-graph). Spot-check: confirmed independently via the free `SendPacket` helper at 0x53B0E2 reading the socket singleton.

### CClientSocket::Flush   (memory-map key: C_CLIENT_SOCKET_FLUSH)
- Primary anchor: call-graph
- Detail: The last call inside CClientSocket::SendPacket. Walks the send-buffer ZList (head at [this+5Ch], InterlockedIncrement on each ZRef<ZSocketBuffer>) and writes via the cloned WS2_32 `send` pointer (`call dword_C49D6C`), handling the -1 / WSAEWOULDBLOCK (0x2733 = 10035) result.
- Fallback anchor: structure — the only socket-region function that walks [this+5Ch] ZList and calls the dedicated cloned-send slot dword_C49D6C; distinct from the cloned `connect` (dword_C49D4C) and cloned `recv` (dword_C49D74).
- Cross-version stability: structurally identical v83→v84; v83 used an indirect cloned-send slot too (dword_BF066C). Locate via the SendPacket call-graph + the [this+5Ch] send-buffer walk, not the cloned-pointer address (which moves per build).
- Notes: v84 @ 0x0049B314. HIGH-VALUE (needs-main-review). Two different kinds (call-graph + structure/constant). Spot-check: the WSAEWOULDBLOCK 0x2733 error-path + the dedicated cloned-send slot, independent of the SendPacket call edge.

### CClientSocket::ProcessPacket   (memory-map key: C_CLIENT_SOCKET_PROCESS_PACKET)
- Primary anchor: IDB symbol (mangled name)
- Detail: `?ProcessPacket@CClientSocket@@IAEXAAVCInPacket@@@Z` survives in the v84 IDB. Reads the CStage singleton (STAGE_INSTANCE_ADDR), constructs a ZRef<CStage>, calls CInPacket::Decode2 to read the opcode word, then dispatches via a sub-0x10 jump table.
- Fallback anchor: call-graph — sole callee is `?Decode2@CInPacket@@QAEGXZ`; sole caller is CClientSocket::ManipulatePacket.
- Cross-version stability: mangled name + the Decode2-driven opcode switch stable v83→v84. The stage-singleton global address differs per version (v83 0xBEDED4, v84 0xC474EC).
- Notes: v84 @ 0x0049B502. HIGH-VALUE (needs-main-review). Two different kinds (symbol + call-graph/structure). Spot-check: sole caller is ManipulatePacket (sub_49B42E), independent of the symbol.

### CClientSocket::ManipulatePacket   (memory-map key: C_CLIENT_SOCKET_MANIPULATE_PACKET)
- Primary anchor: call-graph
- Detail: The sole caller of CClientSocket::ProcessPacket. Reassembles the receive stream: calls CInPacket::AppendBuffer ([this+64h]) and ZList<ZRef<ZSocketBuffer>>::RemoveAt ([this+3Ch]), runs the per-packet version check `((dword[this+88h] >> 16) ^ word[this+72h]) == <version-magic>` and the length guard (<= 0x10000), then dispatches each decoded packet through ProcessPacket.
- Fallback anchor: constant — the version-magic compare `cmp ax, 0FFABh` at the [this+88h]>>16 ^ [this+72h] site is a near-unique immediate. v83 used 0xFFAC, v84 uses 0xFFAB (one LOWER). The magic changes per major version but the direction is not assumed from two data points — confirm the v84 value by reading the compare; do NOT byte-search the v83 constant.
- Cross-version stability: the AppendBuffer + RemoveAt + version-XOR + ProcessPacket-dispatch structure is stable; the version-magic immediate (0xFFAC v83 / 0xFFAB v84) is per-version — confirm direction, don't byte-search the v83 value.
- Notes: v84 @ 0x0049B42E (function size 0xD4, same as v83). HIGH-VALUE (needs-main-review). Two different kinds (call-graph + constant). Spot-check: the 0xFFAB version-magic compare, independent of the ProcessPacket call edge.

### CClientSocket::Close   (memory-map key: C_CLIENT_SOCKET_CLOSE)
- Primary anchor: call-graph + structure
- Detail: Tiny 7-instruction function: `push esi; mov esi,ecx; call ClearSendReceiveCtx; lea ecx,[esi+8]; call ZSocketBase::CloseSocket; pop esi; retn`. The only function that calls both ClearSendReceiveCtx and CloseSocket on the embedded ZSocketBase at [this+8].
- Fallback anchor: adjacency — sits immediately before CClientSocket::SendPacket in the image (v83 0x496369 before 0x49637B; v84 0x49B27A before 0x49B28C). Also called at the top of CWvsApp::ConnectLogin.
- Cross-version stability: exact 7-instruction shape held v83→v84.
- Notes: v84 @ 0x0049B27A.

### CClientSocket::ClearSendReceiveCtx   (memory-map key: C_CLIENT_SOCKET_CLEAR_SEND_RECEIVE_CTX)
- Primary anchor: call-graph + structure
- Detail: First call inside CClientSocket::Close (also called from the CClientSocket ctor and the connect path). Zeroes [this+68h] (dword), [this+70h] (word), [this+78h] (dword), then calls ZList<ZRef<ZSocketBuffer>>::RemoveAll twice — on the receive list [this+3Ch] and the send list [this+50h].
- Fallback anchor: structure — the exact 68h/70h(word)/78h zero triple + two RemoveAll calls at +3Ch and +50h.
- Cross-version stability: identical member offsets and call shape v83→v84.
- Notes: v84 @ 0x0049B903.

### CClientSocket::OnConnect   (memory-map key: C_CLIENT_SOCKET_ON_CONNECT)
- Primary anchor: call-graph + structure
- Detail: The WSAAsyncSelect FD_CONNECT/FD_READ event handler driven by the CWvsApp::ConnectLogin message-pump loop. Reads from the cloned WS2_32 `recv` slot (dword_C49D74), parses the host/port/version handshake, validates the protocol version against the literal 84, calls getpeername, and emits the handshake via CClientSocket::SendPacket; throws the CPatchException / CTerminateException / CDisconnectException trio. Recurses as the retry/teardown path `OnConnect(0)`; also tail-called by Connect(sockaddr_in) on error.
- Fallback anchor: the exception-type trio + the version-equality check (==84) + getpeername; reads g_CWvsApp singleton (m_nGameStartMode).
- Cross-version stability: handler role + exception trio stable; the protocol-version literal equals the client major version (84 here) and the cloned-recv slot address are per-version.
- Notes: v84 @ 0x00499DCD (large, 0x612). Disambiguated from the three Connect* variants: it is the recv-side handler (cloned recv) and the SendPacket/getpeername driver, not a `socket`/`connect` caller.

### CClientSocket::Connect(sockaddr_in)   (memory-map key: C_CLIENT_SOCKET_CONNECT_ADR)
- Primary anchor: import call
- Detail: `?Connect@CClientSocket@@IAEXPBUsockaddr_in@@@Z` — the only function calling WS2_32 `socket` (socket(2,1,0) = AF_INET/SOCK_STREAM). Closes any prior socket (ClearSendReceiveCtx + ZSocketBase::CloseSocket), creates the socket, throws ZException on failure, arms WSAAsyncSelect (cloned slot dword_C49D2C, msg 1025), then calls the cloned `connect` slot dword_C49D4C(s, addr, 16) and tolerates WSAEWOULDBLOCK (10035).
- Fallback anchor: structure — socket(2,1,0) + connect(s,addr,16=sizeof sockaddr_in) + WSAEWOULDBLOCK; only caller of the `socket` import.
- Cross-version stability: the `socket` import xref is the most stable anchor; v83/v84 both route connect/send/recv through cloned WS2_32 slots (anti-tamper), so connect itself is an indirect call — anchor on `socket`, not `connect`.
- Notes: v84 @ 0x00499C2B. Disambiguated from the other Connect variants: it is the private sockaddr_in overload and the sole `socket` caller. Its two callers are Connect(CONNECTCONTEXT) and OnConnect.

### CClientSocket::Connect(CONNECTCONTEXT)   (memory-map key: C_CLIENT_SOCKET_CONNECT_CTX)
- Primary anchor: call-graph
- Detail: `?Connect@CClientSocket@@QAEXABUCONNECTCONTEXT@1@@Z` — public overload. Consumes a CONNECTCONTEXT (sub_499C03), stores the resolved address into the socket object, and tail-calls the private Connect(sockaddr_in).
- Fallback anchor: structure — small wrapper (size 0x64) whose only meaningful call is the sockaddr_in Connect; called by CClientSocket::ConnectLogin and by a CWvsContext-side caller.
- Cross-version stability: wrapper-to-sockaddr_in-Connect relationship stable; v83 body is control-flow-virtualized, so anchor on the call-graph (it is one of the two callers of Connect(sockaddr_in)).
- Notes: v84 @ 0x00499B9F. Disambiguated from CONNECT_ADR: this is the public CONNECTCONTEXT overload that *calls* CONNECT_ADR rather than calling `socket` directly.

### CClientSocket::ConnectLogin   (memory-map key: C_CLIENT_SOCKET_CONNECT_LOGIN)
- Primary anchor: call-graph
- Detail: Called exactly once, from CWvsApp::ConnectLogin (0x00A3FFE8, labeled in Task 2). Reads the CWvsApp singleton's m_nGameStartMode (g_CWvsApp+36, ==1 / ==2 branches), pulls login host/port from the command line via CWvsApp::GetCmdLine, builds a CONNECTCONTEXT list (random server pick when no cmd-line host), and finally calls Connect(CONNECTCONTEXT).
- Fallback anchor: structure — the GetCmdLine(arg 0/1/2/3) + m_nGameStartMode branch + final Connect(CONNECTCONTEXT) call.
- Cross-version stability: the unique caller (CWvsApp::ConnectLogin) is a stable call-graph signature; v83 body is virtualized, so anchor on that single caller edge.
- Notes: v84 @ 0x00499824. Distinct from CWvsApp::ConnectLogin (the message-pump driver at 0x00A3FFE8) — this is the CClientSocket method it invokes.

### ZSocketBase::CloseSocket   (memory-map key: Z_SOCKET_BASE_CLOSE_SOCKET)
- Primary anchor: import call
- Detail: The only tiny function calling both WS2_32 `shutdown` and `closesocket`: `if (*this != -1) { shutdown(*this, 2); closesocket(*this); *this = -1; }`. Function size 0x20.
- Fallback anchor: structure — the -1 socket sentinel store + the shutdown(s,2)/closesocket pair; called by CClientSocket::Close ([this+8]) and Connect(sockaddr_in).
- Cross-version stability: shutdown+closesocket import pair is a very stable anchor; v83 0x494857 (size identical).
- Notes: v84 @ 0x0049974A.

### ZSocketBuffer::Alloc   (memory-map key: Z_SOCKET_BUFFER_ALLOC)
- Primary anchor: structure (dual-alloc idiom — durable when symbols are absent)
- Detail: static factory that does TWO ZAllocEx::Alloc calls (the payload of size `a1`, then the 0x1C/28-byte ZSocketBuffer header), then runs the placement ctor on the header and returns it. The mangled symbol `?Alloc@ZSocketBuffer@@SAPAV1@IABVZAllocHelper@@@Z` is present in this IDB and is the fastest anchor when available, but may not survive into a future stripped IDB — hence the structural idiom is the primary.
- Fallback anchor: call-graph — called by OnConnect with the MSS literal 1460 to allocate the receive buffer; the dual ZAllocEx::Alloc with a fixed 28-byte second size is near-unique in the socket region.
- Cross-version stability: the dual-alloc(size, 28) + placement-ctor idiom is stable; v83 0x495FD2 wrapped it in C++ EH, v84 collapsed the EH frame but kept the idiom.
- Notes: v84 @ 0x0049AEE3 (ZAllocEx::Alloc = sub_403065, placement ctor = sub_49AF30).

### globals: CClientSocket singleton / CreateInstance   (memory-map keys: C_CLIENT_SOCKET_INSTANCE_ADDR / C_CLIENT_SOCKET_CREATE_INSTANCE)
- Primary anchor: writer function
- Detail: The CClientSocket singleton is the `(this+1!=0)?this:0` SBB store at the top of the CClientSocket ctor (??0CClientSocket@@QAE@XZ); the ctor also installs the socket vtable, sets the fd member to -1, wires the ZList send/recv heads, and calls ClearSendReceiveCtx. CreateInstance is the TSingleton wrapper that reads the singleton, and on null does `push 94h` -> ZAllocEx::Alloc -> ctor (the 0x94 instance size matches v83).
- Fallback anchor: the singleton is read as the socket `this` by the free `?SendPacket@@YAXABVCOutPacket@@@Z` helper and by CWvsApp::ConnectLogin; CreateInstance is the sole caller of the CClientSocket ctor.
- Cross-version stability: both the SBB-singleton-store idiom and the CreateInstance(0x94-alloc + ctor) shape are stable; locate via the ctor/CreateInstance, not the raw global address.
- Notes: v84 C_CLIENT_SOCKET_INSTANCE_ADDR @ 0x00C40C64; C_CLIENT_SOCKET_CREATE_INSTANCE @ 0x00A43D0B. CreateInstance moved out of the v83 0x9F9Exx singleton cluster into the 0xA43Dxx region — found by call-graph (sole ctor caller), NOT address arithmetic.

### COutPacket::COutPacket (ctor)   (memory-map key: C_OUT_PACKET)
- Primary anchor: IDB symbol (mangled name)
- Detail: `??0COutPacket@@QAE@J@Z` survives in the v84 IDB. Body zeroes the buffer member (`*((DWORD*)this+1) = 0`), calls the ZArray<unsigned char>::_Alloc helper with initial capacity 256 (0x100), then calls COutPacket::Init(a2). The 256-byte initial allocation feeding an Init call is the structural fingerprint.
- Fallback anchor: structure — the `member+4 = 0` then `_Alloc(256)` then Init(seq) shape; the COutPacket layout (buf at +4, len at +8) is shared with the encoders' grow helper.
- Cross-version stability: mangled name present in v83 and v84; the 256-byte initial alloc + Init call shape is structurally identical across both.
- Notes: v84 @ 0x00703CFA (256-alloc = sub_49BBBE, Init = sub_703DD5). HIGH-VALUE (needs-main-review). Two different kinds (symbol + structure). Spot-check: the buffer-grow helper (_EnsureCapacity, sub_40663D) called by every encoder operates on exactly the +4/+8 members this ctor initializes — independent structural tie-in.

### COutPacket::Encode1 / Encode2 / Encode4   (memory-map keys: C_OUT_PACKET_ENCODE_1 / C_OUT_PACKET_ENCODE_2 / C_OUT_PACKET_ENCODE_4)
- Primary anchor: IDB symbol (mangled name)
- Detail: All three retain mangled names in the v84 IDB: `?Encode1@COutPacket@@QAEXE@Z`, `?Encode2@COutPacket@@QAEXG@Z`, `?Encode4@COutPacket@@QAEXK@Z`. Each is a tiny (~0x1E-0x21 byte) function that calls the shared buffer-grow helper with its byte width then stores. **Disambiguate by store width, never by address:** Encode1 appends 1 byte (`mov [eax+ecx], dl` then `inc [esi+8]`); Encode2 appends 2 bytes (`mov [eax+ecx], dx` then `add [esi+8], 2`); Encode4 appends 4 bytes (`mov [eax+ecx], edx` then `add [esi+8], 4`). The push immediate to the grow helper (1/2/4) matches the store width.
- Fallback anchor: call-graph — all three (plus EncodeStr and EncodeBuffer) are the only callers of the COutPacket buffer-grow helper (`_EnsureCapacity`, v84 sub_40663D / v83 sub_406567); an xrefs_to that helper returns exactly the five sibling encoders.
- Cross-version stability: mangled names present in v83 and v84; the grow(width) + width-specific store idiom is structurally identical. The buffer-grow callee address moves per version (v83 0x406567, v84 0x40663D) — anchor on the store width / symbol, not the callee address.
- Notes: v84 Encode1 (C_OUT_PACKET_ENCODE_1) @ 0x0040661F, Encode2 (C_OUT_PACKET_ENCODE_2) @ 0x00428A68, Encode4 (C_OUT_PACKET_ENCODE_4) @ 0x0040667C. HIGH-VALUE (needs-main-review). Two different kinds (symbol + shared-grow-callee call-graph). Spot-check: the actual store width (1/2/4-byte) read from each body, independent of the symbol.

### COutPacket::EncodeStr   (memory-map key: C_OUT_PACKET_ENCODE_STR)
- Primary anchor: IDB symbol (mangled name)
- Detail: `?EncodeStr@COutPacket@@QAEXV?$ZXString@D@@@Z` survives in the v84 IDB. Reads the ZXString length (`*(arg-4)` when non-null, else 0), grows the buffer by `len + 2`, copies the string via a ZXString assign + copy helper, advances the length by the copied count, and finally calls the ZXString destructor (`~ZXString<char>`) in an EH unwind slot.
- Fallback anchor: structure — the `grow(len + 2)` (the +2 length prefix) plus a ZXString construct/copy/destruct triple; the only grow-helper caller that touches a ZXString.
- Cross-version stability: mangled name present in v83 and v84; the len+2 grow + ZXString copy shape is structurally identical.
- Notes: v84 @ 0x00471EB0. HIGH-VALUE (needs-main-review). Two different kinds (symbol + structure). Spot-check: shared-grow-callee call-graph (one of the five sub_40663D callers), independent of the symbol.

### COutPacket::EncodeBuffer   (memory-map key: C_OUT_PACKET_ENCODE_BUFFER)
- Primary anchor: IDB symbol (mangled name)
- Detail: `?EncodeBuffer@COutPacket@@QAEXPBXI@Z` survives in the v84 IDB (note the void* `PBX` form, NOT the `PBE` byte-pointer form). Takes a (Src, Size); grows the buffer by Size, `memcpy(buf + len, Src, Size)`, then `len += Size`. The grow→memcpy→advance triple with a caller-supplied length is the fingerprint.
- Fallback anchor: structure — the only grow-helper caller that does a memcpy of a caller-supplied Size then advances the length by that same Size.
- Cross-version stability: mangled name present in v83 and v84 (the exact signature suffix differs — v83 was a __userpurge `PBX I` too); the grow(Size)+memcpy+advance shape is structurally identical.
- Notes: v84 @ 0x0046E5FE. HIGH-VALUE (needs-main-review). Two different kinds (symbol + structure). Spot-check: shared-grow-callee call-graph (one of the five sub_40663D callers), independent of the symbol. The mangled signature must be `PBXI` (void*, size) — the `PBEI` (unsigned char*) variant does not exist.

### COutPacket::MakeBufferList   (memory-map key: C_OUT_PACKET_MAKE_BUFFER_LIST)
- Primary anchor: call-graph
- Detail: The sole MakeBufferList call inside CClientSocket::SendPacket (the encoder-cluster host call). In v84 SendPacket it is the first call in the send path on the valid-socket branch: `MakeBufferList(this+80, <send-seq opcode>, this+132, 1, m_seqXor)`, immediately followed by CIGCipher::innoHash and CClientSocket::Flush. MakeBufferList itself is large and stripped (no surviving symbol).
- Fallback anchor: constant / structure — the unique triple-pass header-obfuscation loop `v15 ^= seq + ROL(*p,3); *p++ = 71 - ROR(v15, seq--)` and the second pass `v13 ^= seq + ROL(*--p,4); *p = ROR(v13 ^ 0x13, 3)`, the 1460 / 0x5B4 MSS chunking, the ZSocketBuffer::Alloc call (v84 0x0049AEE3), and the CAESCipher::Encrypt-vs-memcpy branch on the encrypt flag.
- Cross-version stability: the 71 / 0x13 / ROL3 / ROL4 obfuscation constants and the 1460/0x5B4 chunking are structurally identical v83→v84; the function's only caller (SendPacket) is a stable call-graph anchor. The send-seq opcode pushed by SendPacket is per-version (0x53 'S' v83, 0x54 'T' v84) — do NOT use it as an anchor.
- Notes: v84 @ 0x00703E53 (named in the IDB during this port; ZSocketBuffer::Alloc = 0x0049AEE3, CAES encrypt = sub_42E520). HIGH-VALUE (needs-main-review). Two different kinds (call-graph + constant/structure). Spot-check: the 71/0x13 obfuscation-constant shuffle loop, independent of the SendPacket call edge.

### CLogin::SendCheckPasswordPacket   (memory-map key: C_LOGIN_SEND_CHECK_PASSWORD_PACKET)
- Primary anchor: IDB symbol (mangled name)
- Detail: `?SendCheckPasswordPacket@CLogin@@QAEHPBD0@Z` survives in the IDB. Body calls `CNMCOClientObject::LoginAuth(id, pw, 201, 0)`, runs the 20000-range NMLoginAuthReplyCode switch, then on success builds the login packet: `COutPacket(seq=1)` followed by EncodeStr(password), EncodeStr(passport), EncodeBuffer(MachineId, 16), Encode4(GameRoomClient), Encode1(m_nGameStartMode = [g_CWvsApp+36]), Encode1(0), Encode1(0), Encode4(PartnerCode), and finally CClientSocket::SendPacket. Every encoder matches the Task-4-resolved COutPacket cluster.
- Fallback anchor: constant + call-graph — the opcode immediate is `1` passed to the COutPacket ctor (stable v83->v84), and the encoder chain ending in SendPacket is a unique fingerprint; sole non-trivial caller is CLogin::Update.
- Cross-version stability: mangled name present in v83 and v84; the COutPacket(1) opcode + EncodeStr/EncodeStr/EncodeBuffer(16)/Encode4/Encode1x3/Encode4/SendPacket layout is structurally identical. The LoginAuth region magic 201 is stable.
- Notes: v84 @ 0x0060B88B. HIGH-VALUE (needs-main-review). Two DIFFERENT kinds (symbol + opcode/encoder-chain). Spot-check: the CNMCOClientObject::LoginAuth(.,.,201,0) call + the 20000-range reply-code switch, independent of both the symbol and the encoder chain.

### CLogin::Update   (memory-map keys: C_LOGIN_UPDATE / C_LOGO_UPDATE)
- Primary anchor: call-graph
- Detail: Sole non-trivial caller of CLogin::SendCheckPasswordPacket (with the cmd-line auto-connect guard `m_aCmd[0] && *m_aCmd[0] && [g_CWvsApp+40] (m_bAutoConnect)`). Body is the m_nLoginStep state machine: OnStepChanged on step-change timeout, EnableLoginStartCtrl(2/17/18/1/0) per step + game-start-mode, the 10000 ms VAC wait, the sub-step UI-alloc cascade (CUINewChar* via the ZAllocEx helper), GotoTitle, and the tail dispatch to SendCheckPasswordPacket / SendLoginPacket (this+544/548) / SendSelectCharPacket (this+552).
- Fallback anchor: structure/constant — the EnableLoginStartCtrl(2/17/18/1/0) ladder + the 10000 ms VAC timeout + the m_nLoginStep (this+380) switch.
- Cross-version stability: the state-machine shape + the SendCheckPasswordPacket caller edge are stable v83->v84.
- Notes: v84 @ 0x00609A9F. **LOGO_UPDATE vs LOGIN_UPDATE determination:** in the repo's v83 mapping BOTH C_LOGIN_UPDATE and C_LOGO_UPDATE are set to 0x005F4C16 = CLogin::Update (they were never two distinct functions in the cmake — C_LOGO_UPDATE deliberately points at CLogin::Update, not at the CLogo vtable Update slot). This convention is carried to v84 unchanged: both keys map to the SAME v84 address 0x00609A9F. They have NOT diverged. For completeness, the *actual* CLogo IGObj-vtable Update method (the NX/WZ logo-timer animator with the 0x5DC/0x215E constants) is a genuinely separate function at v84 0x00644750 (v83 0x0062F2B6) — labeled CLogo__Update in the IDB but NOT used by the C_LOGO_UPDATE key.

### CLogo::CLogo (ctor)   (memory-map key: C_LOGO)
- Primary anchor: structure (member-zero + vtable install)
- Detail: Calls the CStage base ctor, zeroes the seven CLogo members (m_pLayerMain, m_pLogoProp, m_dwTickInitial, m_dwClick, m_bLogoSoundPlayed, m_bWZInit, m_bNXFadeIn), then installs the four CLogo vtables (IGObj / IUIMsgHandler / INetMsgHandler / ZRefCounted). The installed IGObj vtable's slots are the confirmed CLogo methods.
- Fallback anchor: vtable linkage — the vtable cluster it writes contains CLogo::GetRTTI/IsKindOf/Update/Init (IGObj) and CLogo::OnKey/OnSetFocus/OnMouseButton (IUIMsgHandler), with the inherited CStage::OnPacket and CStage::OnMouseEnter slots.
- Cross-version stability: base-ctor + 7-member-zero + 4-vtable-install shape stable v83->v84.
- Notes: v84 @ 0x0064417C (IGObj vtable @ 0x00B4939C; CLogo CRTTI object @ 0x00C470B8). The adjacent CLogo destructor is sub_64420B.

### CLogo vtable methods: GetRTTI / IsKindOf / Update / Init / OnKey / OnSetFocus / OnMouseButton
- (keys: C_LOGO_GET_RTTI / C_LOGO_IS_KIND_OF / C_LOGO_INIT / C_LOGO_ON_KEY / C_LOGO_ON_SET_FOCUS / C_LOGO_ON_MOUSE_BUTTON; slot 2 = the real CLogo::Update which has NO cmake key — see C_LOGO_UPDATE convention)
- Primary anchor: vtable slot (anchor kind 5) confirmed by a SECOND anchor (method body content), matched by slot ORDER against v83's CLogo vtable (not v83 addresses).
- Detail (IGObj vtable @ 0x00B4939C, slot order GetRTTI/IsKindOf/Update/Init):
  - slot 0 GetRTTI = 0x006441C0 — returns &CLogo_CRTTI (0x00C470B8). 2nd anchor: single-instruction `lea/return &g_rtti`.
  - slot 1 IsKindOf = 0x006441C6 — walks the CRTTI m_pPrev chain from the same g_rtti. 2nd anchor: the chain-walk loop.
  - slot 2 Update = 0x00644750 — the logo-timer animator (0x5DC/0x215E timers, DrawNXLogo/DrawWZLogo/InitWZLogo/LogoEnd). NOTE: this is the real CLogo::Update but is NOT the C_LOGO_UPDATE key value (see CLogin::Update entry).
  - slot 3 Init = 0x00644274 — see C_LOGO_INIT entry below.
- Detail (IUIMsgHandler vtable, slot order OnKey/OnSetFocus/OnMouseButton then inherited OnMouseMove/OnMouseWheel/.../OnMouseEnter):
  - slot 0 OnKey = 0x00644714 — `if (lParam>=0 && (wParam==13||27||32)) CLogo::ForcedEnd(this-4)`. 2nd anchor: the 13/27/32 key-code triple + ForcedEnd call.
  - slot 1 OnSetFocus = 0x006441BA — `return 1`. 2nd anchor: trivial constant return (matches v83).
  - slot 2 OnMouseButton = 0x0064473B — `if (msg==514) CLogo::ForcedEnd(this-4)`. 2nd anchor: the 514 (WM_RBUTTONUP) compare + ForcedEnd call.
- Cross-version stability: vtable slot ORDER stable v83->v84; the body fingerprints (13/27/32, 514, return-1, CRTTI chain) are stable. The CRTTI global address is per-version (v83 0x00BEDAB0, v84 0x00C470B8).
- Notes: v84 IGObj vtable @ 0x00B4939C; CLogo CRTTI @ 0x00C470B8. The shared CStage slots in the same vtable cluster are CStage::OnPacket (0x0079894B) and CStage::OnMouseEnter (0x0079892C).

### CLogo::Init   (memory-map key: C_LOGO_INIT)
- Primary anchor: call-graph
- Detail: First call is CLogo::InitNXLogo; then CInputSystem::ShowCursor(0); then CWvsApp::GetCmdLine([g_CWvsApp], &buf, 5 or 3) gated on m_nGameStartMode==1; ends with `if (cmd && *cmd && [g_CWvsApp+40] (m_bAutoConnect)) CLogo::LogoEnd()`.
- Fallback anchor: structure — the GetCmdLine(5/3) game-start-mode branch + the m_bAutoConnect-guarded LogoEnd tail; it is the IGObj-vtable Init slot (slot 3).
- Cross-version stability: structurally identical v83->v84.
- Notes: v84 @ 0x00644274 (InitNXLogo = 0x00644830, ShowCursor = sub_5AA58B, LogoEnd = 0x00644348).

### CLogo::InitNXLogo   (memory-map key: C_LOGO_INIT_NX_LOGO)
- Primary anchor: call-graph + constant
- Detail: First call inside CLogo::Init. Loads the NX-logo image resource via StringPool ID + IWzResMan::GetObjectA into m_pLogoProp, reads the GR singleton (GR_INSTANCE_ADDR) to IWzGr2D::CreateLayer, fills an 800x600 canvas (vtable+140 with 0,0,800,600,-1), RelMoves the layer alpha to 255, then plays the NX-logo BGM via CSoundMan::PlayBGM (StringPool ID).
- Fallback anchor: structure — the GetObjectA(LogoImg) -> CreateLayer(GR) -> 800x600 canvas -> PlayBGM(NxLogoMs) sequence; the GR-singleton read feeding CreateLayer is the same code site v83 uses.
- Cross-version stability: the 800x600 canvas + CreateLayer + PlayBGM structure stable. The StringPool IDs are per-version (v83 LogoImg/Nexon=1384, NxLogoMs=1250; v84 =1386, =1252) — do NOT byte-search the v83 IDs; the table shifted between versions.
- Notes: v84 @ 0x00644830. PlayBGM = sub_43F166.

### CLogo::LogoEnd   (memory-map key: C_LOGO_LOGO_END)
- Primary anchor: structure (alloc-size + callee triple)
- Detail: `p = ZAllocEx::Alloc(0x28C); if (p) CLogin::CLogin(p); set_stage(stage, 0)`. The 0x28C (652-byte) allocation feeding the CLogin ctor, then set_stage, is a near-unique fingerprint.
- Fallback anchor: call-graph — calls the CLogin ctor and set_stage; called by CLogo::Update and CLogo::Init.
- Cross-version stability: the Alloc(0x28C) + CLogin-ctor + set_stage triple stable v83->v84. The CLogin instance size (0x28C) is stable.
- Notes: v84 @ 0x00644348 (CLogin ctor = 0x00608B15, set_stage = 0x00799CF0).

### CLogo::ForcedEnd   (memory-map key: C_LOGO_FORCED_END)
- Primary anchor: call-graph + structure
- Detail: Called by CLogo::OnKey and CLogo::OnMouseButton. Guarded by `if (m_dwClick==0)`; sets m_dwClick = timeGetTime; GetAlpha vtable+144 call (255,255); loads the WIZET logo resource via StringPool ID + GetObjectA; fills an 800x600 canvas; uses an `L"40"` bstr index; draws the final logo frame.
- Fallback anchor: structure — the m_dwClick-zero guard + the WIZET-logo GetObjectA + 800x600 canvas; it is the ForcedEnd target of both OnKey and OnMouseButton.
- Cross-version stability: the m_dwClick guard + WIZET-logo + 800x600 shape stable. The WIZET StringPool ID is per-version (v83 1383, v84 1385).
- Notes: v84 @ 0x00644392.

### set_stage   (memory-map keys: SET_STAGE / STAGE_INSTANCE_ADDR)
- Primary anchor: writer function (global store) + structure
- Detail: The free function that swaps the CStage singleton: zeroes `g_CStage_pInstance` (STAGE_INSTANCE_ADDR), releases the old stage via a ZRef teardown (the `v2+12 then -12` idiom), runs three IsKindOf (vtable+72) checks against the CStage-subclass CRTTI globals, branches on CWvsContext::GetCharacterData to call OnEnterGame / OnLeaveGame / OnGameStageChanged, then stores the new stage and invokes its vtable+4 init.
- Fallback anchor: call-graph — the OnEnterGame/OnLeaveGame/OnGameStageChanged trio + GetCharacterData; the only function that zeroes-then-rewrites STAGE_INSTANCE_ADDR.
- Cross-version stability: the singleton-swap + IsKindOf-trio + character-data branch structure stable v83->v84. STAGE_INSTANCE_ADDR address is per-version (v83 0x00BEDED4, v84 0x00C474EC).
- Notes: v84 SET_STAGE @ 0x00799CF0; STAGE_INSTANCE_ADDR @ 0x00C474EC. STAGE_INSTANCE_ADDR independently corroborated as the global read by the already-labeled CClientSocket::ProcessPacket, CStage::OnSetField, CStage::OnSetCashShop, and CWvsApp::CallUpdate (the Task-4 cross-version hint 0xC474EC verified independently here).

### CStage::OnPacket   (memory-map key: C_STAGE_ON_PACKET)
- Primary anchor: IDB symbol (mangled name)
- Detail: `?OnPacket@CStage@@UAEXJAAVCInPacket@@@Z` survives in the IDB. A 3-case switch dispatching to CStage::OnSetField / CStage::OnSetITC / CStage::OnSetCashShop.
- Fallback anchor: structure/call-graph — the small switch whose three targets are OnSetField/OnSetITC/OnSetCashShop (the first and third already labeled in the IDB).
- Cross-version stability: mangled name + the OnSetField/OnSetITC/OnSetCashShop dispatch stable. The opcode case values are per-version (v83 125/126/127, v84 0x80/0x81/0x82).
- Notes: v84 @ 0x0079894B. It is the INetMsgHandler-vtable slot 0 of every CStage subclass (CField, CLogin, CLogo, ...).

### CStage::OnMouseEnter   (memory-map key: C_STAGE_ON_MOUSE_ENTER)
- Primary anchor: structure
- Detail: Tiny: `if (bEnter) { if (*(CInputSystem_singleton + 2484)) CInputSystem::SetCursorState(singleton, 0); }`. The CInputSystem-singleton member at byte offset 2484 (v83 thiscall member +621 * 4) feeding SetCursorState(0) is near-unique.
- Fallback anchor: adjacency + call-graph — sits immediately before CStage::OnPacket in the image (v83 0x00775FC7 before 0x00775FE6; v84 0x0079892C before 0x0079894B); reads the CInputSystem singleton and calls SetCursorState.
- Cross-version stability: the `if(enter) if([inputsys+cursorstate]) SetCursorState(0)` shape + the adjacency-to-OnPacket are stable. The CInputSystem singleton address is per-version.
- Notes: v84 @ 0x0079892C (CInputSystem singleton = 0x00C456B4, SetCursorState = 0x005AA92C).

### globals: GR_INSTANCE_ADDR / C_UI_TITLE_INSTANCE_ADDR   (memory-map keys: GR_INSTANCE_ADDR / C_UI_TITLE_INSTANCE_ADDR)
- Primary anchor: reader code site
- Detail: GR_INSTANCE_ADDR is the IWzGr2D singleton read by CLogo::InitNXLogo to call IWzGr2D::CreateLayer (the same code site v83 uses) and by the entire CWnd/canvas drawing layer; it is the single most-referenced Gr2D global. C_UI_TITLE_INSTANCE_ADDR is the CUITitle singleton read at the end of CLogin::SendCheckPasswordPacket (`if (g_CUITitle) (g_CUITitle->vftable[52])(...)`) and in CLogin::Update / CLogin::GotoTitle.
- Fallback anchor: GR is written after the Gr2D CreateInstance in CWvsApp::InitializeGr2D (Task-2-labeled); CUITitle is read by the labeled SendCheckPasswordPacket.
- Cross-version stability: the reader idioms are stable; locate via the InitNXLogo CreateLayer site (GR) and the SendCheckPasswordPacket CUITitle read (Title), not the raw addresses.
- Notes: v84 GR_INSTANCE_ADDR @ 0x00C4AB6C; C_UI_TITLE_INSTANCE_ADDR @ 0x00C47064.

<!--
Add one section per resolved key. Group loosely by subsystem to mirror
memory-map.md. Leave the two stubs above as worked examples of the schema.
-->

### Manager singletons: TSingleton<T>::CreateInstance + ms_pInstance pairs
- (keys: C_ACTION_MAN_CREATE_INSTANCE_ADDR / C_ACTION_MAN_INSTANCE_ADDR, C_ANIMATION_DISPLAYER_CREATE_INSTANCE, C_FUNC_KEY_MAPPED_MAN_CREATE_INSTANCE / C_FUNC_KEY_MAPPED_MAN_INSTANCE_ADDR, C_MACRO_SYS_MAN_CREATE_INSTANCE, C_MAPLE_TV_MAN_CREATE_INSTANCE / C_MAPLE_TV_MAN_INSTANCE_ADDR, C_MONSTER_BOOK_MAN_CREATE_INSTANCE / C_MONSTER_BOOK_MAN_INSTANCE_ADDR, C_QUEST_MAN_CREATE_INSTANCE / C_QUEST_MAN_INSTANCE_ADDR, C_RADIO_MANAGER_CREATE_INSTANCE / C_RADIO_MANAGER_INSTANCE_ADDR, C_SECURITY_CLIENT_CREATE_INSTANCE / C_SECURITY_CLIENT_INSTANCE_ADDR, C_QUICKSLOT_KEY_MAPPED_MAN, C_INPUT_SYSTEM_CREATE_INSTANCE / C_INPUT_SYSTEM_INSTANCE_ADDR)
- Primary anchor: constant (allocation size) + call-graph (the placement ctor callee)
- Detail: Every manager CreateInstance is a tiny (~0x45 byte) `TSingleton<T>::CreateInstance` of the shape `if (!ms_pInstance) { v = ZAllocEx::Alloc(<SIZE>); if (v) Ctor(v); }`. They cluster together (v84 0x00A439xx–0x00A43Fxx, the same static-init region that also holds CClientSocket::CreateInstance @ 0x00A43D0B). **Match each manager to v84 by its per-class allocation SIZE** (the `push <imm>` to the allocator) — these are near-unique per-version fingerprints (RE-OBSERVE the size for each new version — they are class-layout-derived and can shift if a struct is refactored; they happened to match verbatim v83→v84): ActionMan 0x2B8(696), AnimationDisplayer 0x1A8(424), FuncKeyMappedMan 0x3C8(968), MacroSysMan 0x30(48), MapleTVMan 0x3E0(992), MonsterBookMan 0xA4(164), QuestMan 0x2A0(672), RadioManager 0x2C(44), SecurityClient 0x13C(316), QuickslotKeyMappedMan 0x44(68), InputSystem 0x9D0(2512). The `*_INSTANCE_ADDR` global is the `mov [g_xxx], <this>` store at the TOP of the matching placement ctor (the `(this+1!=0)?this:0` SBB idiom) — resolve the pair TOGETHER: CreateInstance names the ctor, the ctor names the instance global.
- Fallback anchor: the ctor's distinctive body confirms the class (vtable installed, member-init shape, the StringPool IDs / ZList layout it builds) — a second independent anchor beyond the alloc size.
- Cross-version stability: the alloc-size fingerprint + SBB-singleton-store idiom are stable; v83 used a two-arg `ZAllocEx::Alloc(heapsel, size)` while v84 collapsed to a one-arg `Alloc(size)` — read the size as the sole/last push. The mangled `TSingleton<T>::CreateInstance` names do NOT survive into v84 (only CClientSocket's does), so anchor on size + ctor, NOT on the symbol.
- Notes: v84 addresses — ActionMan CreateInstance 0x00A43C5E / instance 0x00C40C24 / ctor 0x004067EF; AnimationDisplayer CreateInstance 0x00A43CB4 / ctor 0x004360B8 (StringPool IDs 972–980); FuncKeyMappedMan CreateInstance 0x00A43D50 / instance 0x00C46B70 / ctor 0x0059DD00; MacroSysMan CreateInstance 0x00A43DA6 / ctor 0x00646CFC; MapleTVMan CreateInstance 0x00A43E3F / instance 0x00C46D64 / ctor 0x0064C362; MonsterBookMan CreateInstance 0x00A43A2B / instance 0x00C46BE0 / ctor 0x00A43A70; QuestMan CreateInstance 0x00A4397A / instance 0x00C46BE4 / ctor 0x0073A7EB; RadioManager CreateInstance 0x00A43F30 / ctor 0x0074D115; SecurityClient CreateInstance 0x00A43DFA / instance 0x00C4583C / ctor 0x00A9790F; QuickslotKeyMappedMan CreateInstance 0x00A43F83 / ctor 0x00748B91; InputSystem CreateInstance 0x00A43922 / instance 0x00C456B4 / ctor 0x00A41A7B (the CInputSystem singleton cross-confirmed by Task-5's CStage::OnMouseEnter read of [0x00C456B4]).

### CFuncKeyMappedMan ctor / vftable / default-key globals   (keys: C_FUNC_KEY_MAPPED_MAN / C_FUNC_KEY_MAPPED_MAN_VFTABLE / DEFAULT_FKM_INSTANCE_ADDR / DEFAULT_QKM_INSTANCE_ADDR)
- Primary anchor: structure (the ctor's fixed memcpy shape) + vtable install
- Detail: The CFuncKeyMappedMan ctor installs its vftable at `*this`, then runs FOUR memcpys: two of size 445 (0x1BD) from the DEFAULT_FKM key table, and two of size 32 (0x20) from the DEFAULT_QKM key table, into per-slot offsets near the object base (offsets as reported by the decompiler; re-confirm the unit/layout before using for struct work), then zeroes two trailing dword members. The 445/32 memcpy quad is a near-unique fingerprint. The vftable is `*this`; DEFAULT_FKM is the source of the 445-byte copies; DEFAULT_QKM is the source of the 32-byte copies.
- Fallback anchor: call-graph — the ctor is the sole callee of CFuncKeyMappedMan::CreateInstance (alloc size 968).
- Cross-version stability: the 445/32 quad + vtable-at-`*this` shape held v83→v84 byte-for-byte structurally.
- Notes: v84 ctor 0x0059DD00; vftable 0x00B46B08; DEFAULT_FKM 0x00C31C7C; DEFAULT_QKM 0x00C31E3C.

### CInputSystem methods: Init / UpdateDevice / GetIsMessage / GenerateAutoKeyDown / ShowCursor
- (keys: C_INPUT_SYSTEM_INIT / C_INPUT_SYSTEM_UPDATE_DEVICE / C_INPUT_SYSTEM_GET_IS_MESSAGE / C_INPUT_SYSTEM_GENERATE_AUTO_KEY_DOWN / C_INPUT_SYSTEM_SHOW_CURSOR)
- Primary anchor: import / constant / member-offset (per method) + adjacency to the labeled CInputSystem cluster (0x005AAxxx)
- Detail: Init is the sole caller of the `DirectInput8Create` import (also builds the keyboard key-map table and centers the cursor at 400/300). UpdateDevice is the tiny `if(!a1) UpdateKeyboard(1); if(a1==1) UpdateMouse()` switch. GetIsMessage is `if(!this[625]) return 0; copy 3 dwords from this[626]; return 1` (member offsets 625/626 = v83 0x9C4/0x9C8 /4). GenerateAutoKeyDown checks this[2]/[8]/[586], calls timeGetTime, writes `*a2 = 256` (0x100 = WM_KEYDOWN) with the [588]/[589] repeat-rate. ShowCursor is `if(this[606]) (devvtable+224)(dev, bShow?-1:0xFFFFFF)`.
- Fallback anchor: ShowCursor was already cross-referenced by Task-5 (CLogo::Init calls it); the five methods sit adjacent in the image (ShowCursor 0x005AA58B, UpdateDevice/GetIsMessage just before it; GenerateAutoKeyDown in the 0x005ABxxx tail).
- Cross-version stability: DirectInput8Create import, the 0x100 WM_KEYDOWN write, the -1/0xFFFFFF cursor-show constants, and the member offsets are all stable; mangled CInputSystem method names do NOT survive into v84.
- Notes: v84 Init 0x005AA112; UpdateDevice 0x005AA53C; GetIsMessage 0x005AA559; ShowCursor 0x005AA58B; GenerateAutoKeyDown 0x005AB525.

### CActionMan::Init / SweepCache   (keys: C_ACTION_MAN_INIT / C_ACTION_MAN_SWEEP_CACHE)
- Primary anchor: call-graph (Init) / constant (SweepCache)
- Detail: Init calls the `get_action_name_from_code` helper (v84 sub_4AE25C) inside a per-action StringPool table-build loop (skips index 40, iterates to 173 in v84 / 162 in v83, 24-byte stride, dword_C45AF0/AEC/AFC action globals). SweepCache opens with `t = timeGetTime(); if (t - this[173] >= 0xEA60 (60000)) {...}` then a state-machine `switch` over the per-resource cache lists with 300000-ms eviction timeouts (member +0x2B4 = this[173]).
- Fallback anchor: Init is the large StringPool-driven function adjacent to the ActionMan ctor (sub_4067EF); SweepCache is the only 60000+300000 timer state machine reading member +0x2B4.
- Cross-version stability: the get_action_name_from_code call, the 60000/300000 timer constants, and member +0x2B4 are stable; the action-count loop bound shifted (162→173) so do not anchor on it.
- Notes: v84 Init 0x00406B93; SweepCache 0x00411CA9.

### CMapleTVMan::Init   (memory-map key: C_MAPLE_TV_MAN_INIT)
- Primary anchor: call-graph + constant
- Detail: Reads the Gr2D singleton, allocates 56 bytes for the TV object, spawns the MapleTV worker thread (ZThread::BeginThread / v84 sub_42D01F), loads the TV media WZ via StringPool (UI/MapleTVImg/TVMedia) + IWzResMan::GetObjectA, sets this[240]=child count and this[241]=windowed flag; opens with five ZXString buffer ops on this+233..237.
- Fallback anchor: structure — the GR-singleton read + alloc(56) + BeginThread + GetObjectA(TVMedia) sequence; sole non-trivial method adjacent to the MapleTVMan ctor (sub_64C362).
- Cross-version stability: the alloc-56 + BeginThread + GR-singleton + TVMedia-img shape is stable; the StringPool ID shifted (v83 3940 → v84 3943) so do not byte-search the ID.
- Notes: v84 0x0064C578.

### CMonsterBookMan::LoadBook   (memory-map key: C_MONSTER_BOOK_MAN_LOAD_BOOK)
- Primary anchor: structure (3-stage short-circuit wrapper) + string xref (via stage 2)
- Detail: LoadBook is a tiny wrapper `return f1() && f2(this) && f3(this)` (v84 sub_69B498). Stage 1 (sub_69B4C2) parses the item-consume monster-book WZ and computes the monster-id arithmetic `(mobId - 2380000)/1000` and `(mobId - 20704)` — a near-unique pair of immediates. Stage 2 (sub_69BB3F) reads the "String/MonsterBook.img" data and references the wide string "defaultHP" (and "defaultMP").
- Fallback anchor: the "defaultHP"/"defaultMP" wide strings (v84 @ 0x00B49C2C) are referenced only by the LoadBook stage-2 reader; the (x-2380000)/1000 + (x-20704) arithmetic identifies stage 1.
- Cross-version stability: the 3-call wrapper shape, the 2380000/1000 + 20704 monster-id math, and the "defaultHP/defaultMP" strings are stable v83→v84; the StringPool img IDs shift per version (do not byte-search them).
- Notes: v84 wrapper 0x0069B498 (stages 0x0069B4C2 / 0x0069BB3F / 0x0069C473). Equivalent v83 wrapper was 0x0068487C.

### CQuestMan::LoadDemand / LoadPartyQuestInfo / LoadExclusive
- (keys: C_QUEST_MAN_LOAD_DEMAND / C_QUEST_MAN_LOAD_PARTY_QUEST_INFO / C_QUEST_MAN_LOAD_EXCLUSIVE)
- Primary anchor: string xref (the WZ data-path literal each loads)
- Detail: Each method is the sole referencer of its WZ path literal: LoadDemand → "Etc/QuestCategory.img" (wide, v84 @ 0x00B4F054); LoadPartyQuestInfo → "Quest/PQuest.img" (wide, v84 @ 0x00B4F228); LoadExclusive → "Quest/Exclusive.img" (wide, v84 @ 0x00B4F2B4). Each then iterates the property tree via IWzResMan::GetObjectA + GetItem.
- Fallback anchor: LoadPartyQuestInfo additionally calls CQuestMan::LoadPartyQuestRank and references the "mark" key; all three are adjacent in the CQuestMan method region (0x0073xxxx–0x00742xxx).
- Cross-version stability: the WZ data-path literals are extremely stable across versions and are the canonical anchor; mangled CQuestMan method names do NOT survive into v84.
- Notes: v84 LoadDemand 0x0073AE25; LoadPartyQuestInfo 0x007408E6; LoadExclusive 0x00741D46.

### CRadioManager singleton global   (memory-map key: C_RADIO_MANAGER_INSTANCE_ADDR)
- Primary anchor: writer function (the ctor's singleton store)
- Detail: **The v83 mapping for this key (0x00BF0B00) points at the SHARED HEAP-SELECTOR global `dword_BF0B00` — the allocator arg used by EVERY manager CreateInstance — NOT at the real CRadioManager instance.** The genuine instance global is the `mov [g], <this>` store at the top of the CRadioManager ctor (v84 sub_74D115 → 0x00C45848), and it is what the radio-manager methods (v84 0x0074Dxxx) read back. This entry resolves the key to the REAL instance global, anchored to disassembly evidence (ctor store + method reads), and flags the v83 value as a pre-existing quirk.
- Fallback anchor: g_CRadioManager_pInstance (0x00C45848) is read across the radio-manager method cluster (sub_74D1A5, sub_74DE61, …) and written only by the ctor — distinct from the heap selector.
- Cross-version stability: the ctor-store-then-method-read idiom is stable; locate via the ctor, not the v83 raw address. **needs-main-review** — confirm whether the repo's CRadioManager wrapper expects the real instance (this resolution) or must reproduce the v83 heap-selector quirk.
- Notes: v84 real instance 0x00C45848; v84 heap selector (the v83-key referent) is the allocator-arg global, separate.

### CSecurityClient::OnPacket   (memory-map key: C_SECURITY_CLIENT_ON_PACKET)
- Primary anchor: constant + import (CInPacket::Decode1 opcode gate) — HIGH-VALUE, needs-main-review
- Detail: `if (CInPacket::Decode1(pkt) == 4) CSecurityClient::OnCheckClientIntegrityRequest(pkt)` (v84 sub_A97DF1). Decode1 retains its mangled name in v84 (?Decode1@CInPacket@@QAEEXZ @ 0x004066C9); the opcode constant is 4.
- Fallback anchor: call-graph — it is the sole caller of OnCheckClientIntegrityRequest (v84 sub_A97E10), which is uniquely identified by its Decode2/DecodeBuffer + integrity-scan + COutPacket(26)/Encode1(4)/SendPacket reply + CSecurityThreatDetected throw.
- Cross-version stability: the Decode1==4 gate + the OnCheckClientIntegrityRequest dispatch are stable v83→v84; the Security method region relocated (v83 0x00A4Bxxx → v84 0x00A97xxx, the same region as the CSecurityClient ctor 0x00A9790F).
- Notes: v84 0x00A97DF1. **HIGH-VALUE (needs-main-review).** Two DIFFERENT kinds (constant/import gate + call-graph). Spot-check (independent kind): the callee OnCheckClientIntegrityRequest builds the integrity reply with `COutPacket(26); Encode1(4); … SendPacket` — the Encode1(**4**) reply opcode matches the Decode1==4 request gate, and the throw of CSecurityThreatDetected on a bad scan is the security-specific behavior, confirming the function identity independently of the call edge.

### GetSEPrivilege   (memory-map key: GET_SE_PRIVILEGE)
- Primary anchor: import call
- Detail: The only function calling OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES=0x20) -> LookupPrivilegeValueA(0, "SeDebugPrivilege", &luid) -> AdjustTokenPrivileges -> CloseHandle.
- Fallback anchor: string xref — `SeDebugPrivilege` (the only referencer).
- Cross-version stability: the SeDebugPrivilege string + the token-API quartet are stable v83->v84.
- Notes: v84 @ 0x0044FEF9 (v83 0x0044E824). Free function; the v83 cmake value pointed at a different (later) address in this version's space — relocated by string+import, NOT by carrying the v83 value.

### CConfig::CConfig (ctor)   (memory-map keys: C_CONFIG / C_CONFIG_INSTANCE_ADDR)
- Primary anchor: string xref (StringPool) + singleton store
- Detail: The CConfig object is the `a1` passed in (placement ctor; not a ZAllocEx::Alloc result). Installs vtable off_B43860, member-inits the fixed pattern (a1[41]=31, a1[43]=100, a1[44]=24), StringPool::GetString(2548 = SOFTWARE\Wizet\MapleStory; v83 was 2547), RegOpenKeyExA (dword_C49CB0, HKLM root) into a1+48, memset(a1+157, 0, 0x1BD/445), then LoadGlobal + ResetSessionInfo. The singleton store is `g_CConfig_pInstance = (a1+1!=0)?a1:0` (the SBB idiom) = C_CONFIG_INSTANCE_ADDR.
- Fallback anchor: call-graph — it is the sole caller of CConfig::LoadGlobal (the goPartySearch/goAlliance UI-option reader) and ResetSessionInfo; ctor size 0x109 matches v83.
- Cross-version stability: the member-init constants (31/100/24), the SOFTWARE\Wizet StringPool string, the RegOpenKeyExA(HKLM) + memset(445) shape, and the SBB-singleton-store idiom are stable; the StringPool ID shifted (2547->2548) so do NOT byte-search the ID; locate via ctor, not raw global.
- Notes: v84 C_CONFIG @ 0x004A127C; C_CONFIG_INSTANCE_ADDR (g_CConfig_pInstance) @ 0x00C452EC. LoadGlobal = sub_4A14AA; ResetSessionInfo = sub_4A42B8.

### CConfig::GetPartnerCode   (memory-map key: C_CONFIG_GET_PARTNER_CODE)
- Primary anchor: string xref
- Detail: Sole referencer of the literal `uiWndZ0`; builds the ZXString via Format then calls CConfig::GetOpt_Int(0, key, 0, 0x80000000, 0x7FFFFFFF) and releases the temp.
- Fallback anchor: structure — the GetOpt_Int(.,.,0,INT_MIN,INT_MAX) call with the single uiWndZ0 key.
- Cross-version stability: the `uiWndZ0` literal + the GetOpt_Int(min,max) shape are stable v83->v84.
- Notes: v84 @ 0x0060BC34 (v83 0x005F6CFB). GetOpt_Int = sub_4A3FCE; the v84 decompile drops the implicit `this` (it is a CConfig method).

### CConfig::ApplySysOpt   (memory-map key: C_CONFIG_APPLY_SYS_OPT)
- Primary anchor: structure + call-graph
- Detail: `qmemcpy(this+25, a2, 0x30)`; reads the game-start-mode this[35] and writes the two CWvsContext singleton flags at +14376/+14380; calls get_field; computes the BGM/SE volumes as `100*(this[26]+1)/20` (gated on this[27]/this[29]); routes to CRadioManager::SetVolume / CSoundMan::SetBGMVolume / CSoundMan::SetSEVolume; finally writes CInputSystem singleton +2416 = this[31].
- Fallback anchor: call-graph — the SetBGMVolume/SetSEVolume + get_field trio; reads g_CRadioManager_pInstance (0x00C45848, Task-6) and g_CInputSystem_pInstance (0x00C456B4, Task-5).
- Cross-version stability: the game-start-mode->CWvsContext write, the 100*(x+1)/20 volume math, and the sound/radio/input singleton routing are structurally identical v83->v84; the CWvsContext member byte-offsets (14376/14380) and singleton addresses are per-version.
- Notes: v84 @ 0x004A3A9C (v83 0x0049EA33).

### CConfig::CheckExecPathReg   (memory-map key: C_CONFIG_CHECK_EXEC_PATH_REG)
- Primary anchor: string xref (StringPool exec-path pair) + structure
- Detail: Reads this[48] (the reg key handle from the ctor); StringPool::GetString(3138 = ExecPath value, 3139 = MapleStory.exe; v83 were 3135/3136); builds a `"\\"`(92) separator; compares the stored exec path against the running path via strcmp; on mismatch concatenates ExecPath + MapleStory.exe and tests it with GetFileAttributes (dword_C49A4C -> -1 or `& 0x10`); writes the value back via RegSetValueExA wrapper.
- Fallback anchor: structure — the this[48] reg-handle gate + the 92 backslash build + the GetFileAttributes(`==-1 || &0x10`) directory check.
- Cross-version stability: the reg-handle gate + backslash + GetFileAttributes(&0x10) shape is stable; the StringPool IDs shifted (3135/3136 -> 3138/3139) so do NOT byte-search them.
- Notes: v84 @ 0x004A1D5C (v83 0x0049CCF3).

### CConfig sys-opt windowed-mode flag   (memory-map key: C_CONFIG_SYS_OPT_WINDOWED_MODE)
- Primary anchor: reader code site (two already-labeled functions)
- Detail: The global read by BOTH CWvsApp::CreateMainWindow (the `flag != 0 ? 0x80000000 : 720896` window-style branch + the `?8:0` exstyle) AND CWvsApp::InitializeGr2D (the windowed-vs-fullscreen device branch) — exactly the same two-reader pattern as v83's 0xBF1AC8.
- Fallback anchor: the 0x80000000 (WS_POPUP fullscreen) vs 720896 (0x000B0000 windowed) style immediates fed by this flag in CreateMainWindow.
- Cross-version stability: the two-reader (CreateMainWindow + InitializeGr2D) pattern + the 0x80000000/720896 branch are stable; the global address is per-version.
- Notes: v84 @ 0x00C4B150 (g_CConfig_SysOpt_WindowedMode; v83 0xBF1AC8). Both readers were labeled in Task 2.

### CIGCipher::innoHash   (memory-map key: C_IG_CIPHER_INNO_HASH)
- Primary anchor: call-graph (sole callee of CIGCipher::bShuffle; called from CClientSocket::SendPacket between MakeBufferList and Flush)
- Detail: Loops `CIGCipher::bShuffle(v3, buf[i])` over `len` bytes and returns `*v3`; when no key pointer is supplied (`if(!a3) v3=&v6`) it seeds with the no-key default `v6 = -967814158` (0xC6EF3720). Called in CClientSocket::SendPacket between COutPacket::MakeBufferList and CClientSocket::Flush (innoHash(this+132, 4, 0)).
- Fallback anchor: the -967814158 (0xC6EF3720) seed immediate — stable v83->v84 but treat as per-version; confirm by reading the `if(!a3)` branch, do NOT byte-search it.
- Cross-version stability: the 0xC6EF3720 seed + the bShuffle-loop shape + the SendPacket call position are stable v83->v84.
- Notes: v84 @ 0x00A9669E (v83 0x00A4A838); bShuffle = sub_A966D9.

### ZSynchronizedHelper<ZFatalSection> ctor/dtor   (memory-map keys: Z_SYNCHRONIZED_HELPER_Z_FATAL_SECTION_CTOR / _DTOR)
- Primary anchor: structure (critical-section acquire/release) + byte-identity with v83
- Detail: CTOR (0x403166): `push esi; mov esi,ecx; <Enter loop>` — calls the ZFatalSection acquire thunk off_C35A80; while it returns non-zero, Sleep(0) (dword_C499F4) and retry; pop esi; retn. Size 0x25. DTOR (ctor+0x25 = 0x40318B): `mov eax,[ecx]; dec dword[eax+4]; jnz +; and dword[eax],0; retn` — decrements the fatal-section recursion count and clears on last release.
- Fallback anchor: the ctor's Enter/Sleep(0) retry loop is near-unique; the dtor is the tiny adjacent dec-and-clear immediately following the ctor's retn.
- Cross-version stability: both at the same low addresses 0x403166/0x40318B v83->v84 (tiny RAII helper that happened not to move; the surrounding early-.text region DID shift, so confirm the body, don't assume the address). The CTOR identity is confirmed by its near-unique Enter/Sleep(0) retry loop. NOTE: this is a dump IDB with address-aliasing — IDA's function database maps 0x40318B onto an unrelated large EH function (sub_A1DFFC in v84 / sub_9D5DEA in v83), and `get_bytes` operates in a divergent address space here so a raw byte-read of the dtor is NOT reproducible. The DTOR identity is instead confirmed via paired-xref RAII structure: 0x40318B is called as the release half of an acquire/release pair with the ctor 0x403166 from many tiny (~0x51-byte) RAII wrappers. Do NOT trust list_funcs/disasm at 0x40318B in these dumps.
- Notes: v84 CTOR @ 0x00403166, DTOR @ 0x0040318B (both unchanged from v83). The ctor is the SendPacket per-socket-lock acquire (catalog §SendPacket).

### CSystemInfo: ctor / Init / GetMachineId / GetGameRoomClient   (memory-map keys: C_SYSTEM_INFO / C_SYSTEM_INFO_INIT / C_SYSTEM_INFO_GET_MACHINE_ID / C_SYSTEM_INFO_GET_GAME_ROOM_CLIENT)
- Primary anchor: IDB symbol (ctor/Init/GetMachineId retain mangled names) + string xref (Init)
- Detail: ctor `??0CSystemInfo@@QAE@XZ` installs vtable off_B94FC4 (1 instruction). Init `?Init@CSystemInfo@@QAEXXZ` is the machine-id builder: Netbios (NCB_ASTAT/NCB_RESET MAC query), the `SOFTWARE\Microsoft\Windows\CurrentVersion` reg open, the `CxSupportId` value (RegQueryValueExA, 16 bytes), CoCreateGuid fallback; called from CLogin::SendCheckPasswordPacket. GetMachineId `?GetMachineId@CSystemInfo@@QAEPBEXZ` returns this+20 (the cached 16-byte id). GetGameRoomClient is the huge (0x11B4) process-table function referencing `bluenetterm.exe` / `client.exe` / the agent*.bin list, comparing the parent-process exe against the table.
- Fallback anchor: GetGameRoomClient via the `bluenetterm.exe` string xref (sole referencer) + adjacency to GetMachineId; Init via CxSupportId. (The 0x11B4 function size is a per-version descriptor — corroborating only, re-measure per version.)
- Cross-version stability: the three mangled names survived v83->v84; the CxSupportId + SOFTWARE\Microsoft\...\CurrentVersion + Netbios anchors and the bluenetterm.exe process-table are stable.
- Notes: v84 ctor 0x00AA0D10 (vtable off_B94FC4); Init 0x00AA0D50; GetMachineId 0x00AA1030 (this+20); GetGameRoomClient 0x00AA1130. The whole cluster relocated from v83 0xA54Bxx to v84 0xAA0Dxx/0xAA1xxx.

### ZArray<unsigned char>::RemoveAll   (memory-map key: Z_ARRAY_REMOVE_ALL)
- Primary anchor: structure + call-graph
- Detail: `if(*this){ ZAllocEx::Free(*this - 4); *this = 0 }` — frees the array data at (base-4) and nulls the pointer. It is the FIRST call inside ZArray<uchar>::_Alloc (sub_49BBBE), which the COutPacket ctor (0x00703CFA) uses with capacity 256 — i.e. _Alloc clears the old array via RemoveAll before allocating.
- Fallback anchor: the `*this-4` Free + `*this=0` shape; sole "clear" callee at the top of the packet-buffer _Alloc.
- Cross-version stability: structurally identical; v83 used 2-arg `ZAllocEx::Free(heapsel, *this-4)`, v84 collapsed to 1-arg `Free(*this-4)` (read the single arg). This `unsigned char` instantiation has STRIDE 1 (no `imul`); the generic ZArray<T>::RemoveAll variants used as struct-audit tools in later tasks carry an `imul stride, count` element-walk + per-element dtor before the Free — re-derive the stride per element type, do not assume 1.
- Notes: v84 @ 0x004297E5 (v83 0x00428CF1). ZAllocEx::Free = sub_4031ED; _Alloc = sub_49BBBE.

### ZXString<char>::GetBuffer (cstr-assign)   (memory-map key: Z_X_STRING_GET_BUFFER)
- Primary anchor: ABI/structure + call-graph
- Detail: Repo wrapper calls it as `_fastcall(this, NULL, src, size)` (ZXString.h:55) to set the string in place. v83 (0x414617) was `__userpurge(this@ecx, a2@esi, Src, Size)` — pure cstr-assign (inner GetBuffer(bRetain=0) + single memcpy + ReleaseBuffer). v84's nearest in-place primitive with the matching `(this, Src, Size)` ABI is the _Cat-family entry sub_429824: on an empty/zeroed ZXString it does `inner_GetBuffer(this, Size, 0) + memcpy(Src, Size) + ReleaseBuffer` (== assign); on a non-empty string it doubles capacity and appends. Because the repo wrapper is only ever invoked on freshly-managed ZXStrings, it behaves as the v83 assign.
- Fallback anchor: structure — the inner-GetBuffer (sub_417123) + memcpy + ReleaseBuffer (sub_4171CE) sequence operating in place on `this`.
- Cross-version stability: inner GetBuffer (sub_417123 = v83 0x416674's `?GetBuffer@?$ZXString@D@@QAEPADHH@Z`) is byte-stable; the outer assign primitive's identity is the uncertainty.
- Notes: v84 @ 0x00429824. **needs-main-review**: the v83 key was a PURE-assign (always replaces); v84's matched function is the _Cat/append family that assigns only when the target is empty. Confirm the repo never calls GetBuffer on a non-empty ZXString (it does not in the current source) or locate a dedicated pure-assign primitive if one exists.

### ZXString<char>::TrimRight / TrimLeft   (memory-map keys: Z_X_STRING_TRIM_RIGHT / Z_X_STRING_TRIM_LEFT)
- Primary anchor: string literal (whitespace default) + structure
- Detail: Both default `Str` to the literal `" \t\r\n"` (asc_B4337C) when NULL and use `strchr(Str, c)` to test set-membership, calling the inner GetBuffer (sub_417123, bRetain=1). TrimRight (0x4772DD) scans from the last char backward, NUL-terminates after the last non-set char, ReleaseBuffer(newlen). TrimLeft (0x477392) scans forward to the first non-set char, then memcpy-shifts the remainder to the front. They are ADJACENT in the image (same as v83) and call the same empty-result helper sub_415124.
- Fallback anchor: the `" \t\r\n"` literal (shared by exactly these two) + the right-scan vs left-scan+memcpy distinction; sizes 0xB5 / 0xB8 (v83 0xB5 / 0xB4).
- Cross-version stability: the whitespace literal + strchr + inner-GetBuffer(retain) shape are stable v83->v84; the string global address is per-version (v83 asc_AF2244, v84 asc_B4337C).
- Notes: v84 TrimRight @ 0x004772DD (v83 0x00474414); TrimLeft @ 0x00477392 (v83 0x004744C9). Repo calls them `(this, NULL, s)` (ZXString.h:209/214) — the NULL consumes the edx slot; the v84 functions are thiscall(this, Str).

### CMob::CMob (ctor)   (memory-map key: C_MOB_C_MOB)
- Primary anchor: IDB symbol (mangled name) — HIGH-VALUE, needs-main-review (Task-11 doom-fix hook target)
- Detail: `??0CMob@@QAE@PAVCMobTemplate@@@Z` survives in the v84 IDB. Body: CLife base ctor (sub_604FFD); zero-inits the member block; stores `m_pTemplate = pMobTemplate` at this+0x188 (this+98 dword); installs the three CMob vtables at this+0 (IGObj off_B49998), this+1 (ZRefCounted off_B49974), this+2 (off_B49970); runs the `_ZtlSecureTear<>` chain (sub_5F1CCA / sub_417060 / sub_4F05AD / sub_4F0662) over the secured stat members; calls MobStat::SetFrom-equivalent (sub_7AE276) with m_pTemplate; get_update_time (sub_9C7771); ends with StringPool::GetStringW(960 = SP_CANVAS) + PcCreateObject::IWzCanvas (sub_403819) into the HP-indicator canvas at this+1248.
- Fallback anchor (SPOT-CHECK, independent kind): StringPool ID 960 (CANVAS) + the PcCreateObject::IWzCanvas tail call + the CMobTemplate store + the CLife base ctor + the three-vtable install — confirm the ctor identity WITHOUT relying on the symbol.
- Cross-version stability: the CLife-base + 3-vtable-install + CMobTemplate-store + _ZtlSecureTear chain + SP_CANVAS/IWzCanvas tail are stable v83->v84; the StringPool ID 956(v83)->960(v84) shifted and the vtable globals are per-version.
- Notes: v84 @ 0x00678060 (v83 0x006621D9). **HIGH-VALUE / needs-main-review** (Task-11). Two DIFFERENT kinds (symbol + structure/vtable+SP-canvas) + spot-check. **Doom-field observation (Task 11):** the ctor stores `m_pTemplate` at this+0x188 (this+98) and zeroes the adjacent `m_pTemplateByDoom` at this+0x18C (this+99 = 0 at 0x67814c); the doom-reserved template pointer is left NULL on construction (no doom hook installed by the ctor itself), so a doom-fix must populate m_pTemplateByDoom later.

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

### CField::SendCreateNewPartyMsg   (memory-map keys: C_FIELD_SEND_CREATE_NEW_PARTY_MSG / C_FIELD_SEND_CREATE_NEW_PARTY_MSG_OFFSET)
- Primary anchor: constant/opcode + call-graph
- Detail: Nullary CField sender. Reads the CWvsContext singleton, runs the job-id gate (`_ZtlSecureFuse<short>` job read at member+0x39/+0x3D compared against 3E8h/7D0h/7D1h) and the `_ZtlSecureFuse<byte>` level gate (`cmp al, 0Ah; jnb`), and on pass builds `COutPacket(<party-opcode>)` then `Encode1(1)` (the create-party sub-opcode) then `CClientSocket::SendPacket`. Disambiguated from SendJoinPartyMsg by being **nullary** (`retn`, no string arg) and emitting **sub-opcode 1** with no EncodeStr.
- Fallback anchor: caller topology — its sole non-chat caller is a tiny 0xC-byte tail-jump wrapper (CTabParty::OnCreateParty), structurally distinct from JoinParty's OnInvite/OnButtonClicked/HandleRButtonClk callers.
- Cross-version stability: the 3E8/7D0/7D1 job constants + the `cmp al,0Ah` level gate + the Encode1(1)/SendPacket tail are stable v83→v84. **The COutPacket opcode is per-version: v83 0x7C → v84 0x7E** — do NOT byte-search the v83 opcode. StringPool IDs for the error messages also shift per build (v83 0x13F/0x14C1 → v84 0x142/0x14C4).
- Notes: v84 @ 0x0053BE37 (size 0x11F). HIGH-VALUE (needs-main-review). Two DIFFERENT kinds (opcode/encoder-chain + caller call-graph). Spot-check: the 0xC-byte OnCreateParty wrapper caller (sub_944C48), independent of the opcode. **OFFSET re-measured**: the patch-point is the level-gate `jnb short loc_53BF11` at 0x53BEDB (raw bytes `73 34`); the no-beginner-party edit overwrites 0x73 with 0xEB to fall through to the send path. Delta 0x53BEDB−0x53BE37 = **0xA4** (coincides with v83 0xA4 because the prologue+gate preamble length is identical — confirmed by disasm, not copied).

### CField::SendJoinPartyMsg   (memory-map keys: C_FIELD_SEND_JOIN_PARTY_MSG / C_FIELD_SEND_JOIN_PARTY_MSG_OFFSET)
- Primary anchor: constant/opcode + call-graph
- Detail: One-arg CField sender (takes the invitee ZXString, `retn 4`). Same job-id + level gates as SendCreateNewPartyMsg, plus a `strcmp` against the player's own name and a `GetPartyMemberNumber` party-full (`cmp eax, 6`) guard; on pass builds `COutPacket(<party-opcode>)` then `Encode1(4)` (the invite sub-opcode) then `EncodeStr(name)` then `CClientSocket::SendPacket`. Disambiguated from SendCreateNewPartyMsg by the **string argument + EncodeStr** and **sub-opcode 4**.
- Fallback anchor: caller topology — invoked from the UI invite paths (CUIUserInfo::OnButtonClicked, CTabParty::OnInvite, CUserLocal::HandleRButtonClk analogs), distinct from CreateNew's single OnCreateParty wrapper.
- Cross-version stability: the job/level gates + strcmp + GetPartyMemberNumber(==6) + Encode1(4)/EncodeStr/SendPacket shape are stable v83→v84. **The COutPacket opcode is per-version: v83 0x7C → v84 0x7E.** StringPool IDs shift (v83 0x143/0x148/0x144 → v84 0x147/0x14B/0x146/0x1582). **ANTI-PATTERN WARNING: SendJoinPartyMsg and SendCreateNewPartyMsg use the SAME COutPacket opcode (0x7E in v84) — they are NOT distinguishable by opcode byte. Disambiguate by sub-opcode (Encode1 arg: 4=invite vs 1=create) + caller topology, never by searching for two different opcodes.**
- Notes: v84 @ 0x0053C061 (size 0x23B). HIGH-VALUE (needs-main-review). Two DIFFERENT kinds (opcode/encoder-chain + caller call-graph). Spot-check: the GetPartyMemberNumber `cmp eax, 6` party-full guard + the multi-caller invite topology, independent of the opcode. **OFFSET re-measured**: the patch-point is the level-gate `jnb short loc_53C135` at 0x53C101 (raw bytes `73 32`); the edit overwrites 0x73 with 0xEB. Delta 0x53C101−0x53C061 = **0xA0** — this is NOT v83's 0x65: v84 inserts an extra guest-id-check block (StringPool 0x1582) before the level gate, lengthening the preamble. RE-MEASURE per version; never copy 0x65.

### CWvsContext::SendMigrateToITCRequest   (memory-map keys: C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST / C_WVS_CONTEXT_SEND_MIGRATE_TO_ITC_REQUEST_OFFSET)
- Primary anchor: string xref
- Detail: References the unique literal `The MapleStory Trading System is not available for the Guest ID Users.`; runs the guest-ID guard, a throttle (`get_update_time` delta >= 0x1F4) and the ITC-availability gate (`get_field`→[+0x158]→`shr 4`→`and 1`→`jz`), and on pass builds `COutPacket(<itc-opcode>)` then `CClientSocket::SendPacket`.
- Fallback anchor: call-graph/constant — COutPacket ctor + SendPacket tail with no encoder body; reads the CWvsContext singleton members for the throttle timestamps.
- Cross-version stability: the Guest-ID string + the and-1 ITC gate + the get_update_time/0x1F4 throttle are stable v83→v84. **The COutPacket opcode is per-version: v83 0x9C → v84 0xA0.** The Guest-ID message StringPool/format reference is the durable anchor.
- Notes: v84 @ 0x00A5C95F (size 0x163). HIGH-VALUE (needs-main-review). Two DIFFERENT kinds (string xref + opcode/SendPacket call-graph). Spot-check: the get_update_time + 0x1F4 throttle and the get_field [+0x158] ITC-flag read, independent of the string. **OFFSET re-measured**: the patch-point is the ITC-gate `jz short loc_A5CA70` at 0xA5CA48 (raw bytes `74 26`); the no-enter-mts edit overwrites 0x74 with 0xEB. Delta 0xA5CA48−0xA5C95F = **0xE9** (coincides with v83 0xE9 because the preamble length is identical — confirmed by disasm, not copied).

### CWvsContext singleton / CWvsContext::OnEnterGame   (memory-map keys: C_WVS_CONTEXT_INSTANCE_ADDR / C_WVS_CONTEXT_ON_ENTER_GAME / C_WVS_CONTEXT_ON_ENTER_GAME_OFFSET)
- Primary anchor: call-graph (stage-swap trio)
- Detail: The CWvsContext singleton is the global passed as `this` to the OnEnterGame/OnLeaveGame/OnGameStageChanged trio inside SetStage (SET_STAGE @ 0x00799CF0): the GetCharacterData!=0 branch calls `OnEnterGame(g_CWvsContext)`, the CStage-subclass branch calls OnLeaveGame, the else-branch calls OnGameStageChanged. The same global is read by both CField party senders as the CWvsContext object feeding GetCharacterData (sub_4267FF). OnEnterGame itself is the large enter-game initializer that runs the CWvsContext member sub-object ctors at this+0x35xx and zeroes the party/field members.
- Fallback anchor: OnEnterGame is the first of the trio (GetCharacterData!=0 branch) and the only one that constructs the this+0x35xx member block; the singleton is the only global all three handlers + both party senders share as CWvsContext `this`.
- Cross-version stability: the SetStage trio + GetCharacterData branch shape is stable v83→v84; the singleton global address and the CWvsContext member offsets (v83 0x2EDx → v84 0x35xx) are per-version — locate via SetStage's trio, not the raw address. OnEnterGame's body diverged structurally (v83 a memset of the 0x2ED8 region; v84 a sequence of member-ctor calls) because the CWvsContext layout changed.
- Notes: v84 C_WVS_CONTEXT_INSTANCE_ADDR @ 0x00C40C68 (TSingleton<CWvsContext>::ms_pInstance); C_WVS_CONTEXT_ON_ENTER_GAME @ 0x00A4E263. **OFFSET re-measured**: the v83/v87/v95 offset points at the first body instruction after the EH-prolog + register-save block; in v84 that is `lea ecx,[esi+35FCh]` at 0xA4E272 (delta 0xA4E272−0xA4E263 = **0x0F**), shorter than v83 (0x10) and v87/v95 (0x11) because the v84 prologue omits the large `sub esp` / `push 1` setup. Not consumed by any active edit (carried for sibling-map parity) — RE-MEASURE per version; never copy 0x10.

### Protocol constants: VERSION_HEADER / PLAYER_LOGGED_IN / CLIENT_START_ERROR   (memory-map keys: VERSION_HEADER / PLAYER_LOGGED_IN / CLIENT_START_ERROR)
- Primary anchor: read in-place from CClientSocket::OnConnect (the version-handshake recv handler, C_CLIENT_SOCKET_ON_CONNECT).
- Detail: All three live in OnConnect's post-handshake decode. **VERSION_HEADER** is the immediate in the version-header equality guard `cmp byte ptr [decoded-header-byte], <N>` (in v84 `cmp byte ptr [ebp+namelen+3], 8`); the `jz` past it leads to the success path, the fall-through throws CTerminateException 0x22000007. **PLAYER_LOGGED_IN** is the opcode `push <imm>` feeding the COutPacket ctor in the *logged-in* (non-bLogin, `[this+36]==0`) send branch — the packet that then Encode4(charId)+EncodeBuffer(machineId,16)+Encode1×… (matches CLogin::SendCheckPasswordPacket's encoder family). **CLIENT_START_ERROR** is the opcode `push <imm>` feeding the COutPacket ctor in the *bLogin* (`[this+36]!=0`) branch.
- Fallback anchor: VERSION_HEADER is the only single-byte literal compared immediately before the `==<major>` version compare (84 in v84); the two opcodes are the two COutPacket ctor pushes in OnConnect's mutually-exclusive bLogin/else tail.
- Cross-version stability: all three values coincide v83→v84 (8 / 0x14 / 0x19) but MUST be re-read per version from OnConnect, not copied — the version-major compare next to VERSION_HEADER changes per build (84 vs 83) and a send-opcode renumber would silently break a copied value.
- Notes: v84 read sites — VERSION_HEADER=8 @ 0x49A08A; CLIENT_START_ERROR `push 19h` @ 0x49A2A0; PLAYER_LOGGED_IN `push 14h` @ 0x49A2F9. No IDB label needed (immediates).

### RESET_LSP (WinSock LSP reset)   (memory-map key: RESET_LSP)
- Primary anchor: string xref + call-graph from CWvsApp::ctor.
- Detail: The function reads HKLM `SYSTEM\…\WinSock2\Parameters\Protocol_Catalog9\Catalog_Entries\000000000001`, queries `PackedCatalogItem`, and if the value contains `wpclsp.dll` runs `<SysDir>\\netsh.exe winsock reset` via CreateProcessA+WaitForSingleObject. The `wpclsp.dll` + `PackedCatalogItem` + `netsh.exe winsock reset` literal trio is a near-unique fingerprint. It is called from the CWvsApp ctor in the `(dwMajorVersion>=6 && !IsWow64Process)` branch.
- Fallback anchor: call-graph — the CWvsApp ctor's last conditional call (in v84 `call ResetLSP` @ 0xA3DD14, after the IsWow64Process probe + the two g_dwTargetOS=1996 stores).
- Cross-version stability: the wpclsp/netsh string trio is stable across all GMS builds; v87 already names it `ResetLSP` @ 0x451212, v95 @ 0x45ECD0, v111 @ 0x47CB70 (entry-address convention). **v83 was a control-flow-virtualized stub (0x0044ED47 jumps into a VM blob) with a stale "does not exist" comment — v84 is de-virtualized and resolvable.** Anchor on the string trio, not the v83 address.
- Notes: v84 @ 0x004505C5 (IDB-labeled `ResetLSP`). **Surprise vs the carried-forward "does not exist" comment** — the function genuinely exists in GMS v84. The repo's ResetLSP() currently does `*(void**)RESET_LSP` (a pointer-deref, not a call) on Vista+ non-WOW64; the key value is the function entry per GMS convention.

### Sentinel absence confirmations (GMS-absent + JMS-only)   (memory-map keys: C_BATTLE_RECORD_MAN_CREATE_INSTANCE / DR_CHECK / CE_TRACER_RUN + the 5 JMS-only keys)
- Method: SP-5 — confirm each feature's OWN anchor in a version that HAS it, then show that anchor absent in v84. Absence is CONFIRMED, never assumed.
- **C_BATTLE_RECORD_MAN_CREATE_INSTANCE** (GMS v95+ feature): positive anchor = the `CBattleRecordMan` class (many mangled methods @ 0x4701A0+ in v95). v84 has NO `CBattleRecordMan` function name or string → absent. Matches source guard `BUILD_MAJOR_VERSION >= 95`.
- **DR_CHECK** (GMS v87+ feature): positive anchor = `?DR_check@@YAHPAU_DR_INFO@@PAKPAUHINSTANCE__@@@Z` (the AhnLab anti-debug check, @ 0x4A1AD3 in v87). v84 has NO `DR_check` function and NO `_DR_INFO` type/string (note: v84 DOES have the basic `\HShield\EHSvc.dll` / `SOFTWARE\AHNLAB\IOU\HackShield` presence-check strings — those back CWvsApp::SetUp's ehsvc.dll integrity test, NOT DR_check; do not confuse them). Matches source guard `>= 87`.
- **CE_TRACER_RUN** (GMS v95+ feature): positive anchor = `?Run@CeTracer@@QAEXXZ` (the AhnLab eTracer, @ 0x9BF370 in v95). v84 has NO `CeTracer`/`eTracer` function or string → absent. Matches source guard `>= 95`.
- **JMS-only keys** (`C_SECURITY_CLIENT_ON_PACKET_RET_STUB`, `_CHECK`, `_CHECK_OFFSET`, `C_WVS_APP_INITIALIZE_GR2D_WINDOWED_OFFSET`, `WIN_MAIN_LAUNCHER_STUB`): these are JMS-build in-place byte-patch points, confirmed PRESENT in JMS185 — RET_STUB @ 0xB3B96B (push-ebp prologue overwritten with C3), CHECK @ 0xB3B5F7 with CHECK+0x19 = 0xB3B610 bytes `74 04` (jz NOP'd), GR2D windowed offset = 0x94, launcher stub `StartUpDlgClass` window fn @ 0x7F3CE0 (forced ret-1). They have no GMS counterpart by DESIGN, not by relocation: GMS bypasses CSecurityClient::OnPacket via the MapleHook on C_SECURITY_CLIENT_ON_PACKET (0xA97DF1), forces windowed via C_CONFIG_SYS_OPT_WINDOWED_MODE (0xC4B150), and disables the patcher via WIN_MAIN+WIN_MAIN_PATCHER_OFFSET NOP — all selected by `BUILD_REGION=="GMS"`, with the JMS branch never compiled. Carry `0x00000000` for the GMS v84 build.
- Cross-version stability: a sentinel-absent verdict must be re-derived per version against the feature's OWN anchor — a later GMS build that adopts a JMS-style in-place patch, or that backports CeTracer/DR_check earlier, would flip the verdict. Confirm, don't assume.

## Heuristic playbook (general, version-agnostic)

- **String xrefs are king.** Format strings, file paths ("Data\\…"), registry
  keys, and error literals survive across versions far better than code bytes.
- **Manager singletons** (`*::CreateInstance` / `*_INSTANCE_ADDR`): each
  `*::CreateInstance` is its OWN tiny (~0x45 byte) `TSingleton<T>::CreateInstance`
  function — `if (!ms_pInstance) { v = ZAllocEx::Alloc(<SIZE>); if (v) Ctor(v); }`.
  They cluster contiguously in one region of the image (in v84 GMS the cluster is
  ~0x00A439xx–0x00A43Fxx, sharing it with CClientSocket::CreateInstance). **Match a
  manager to its v84 CreateInstance by the per-class allocation SIZE** (the unique
  `push <imm>` to the allocator) — that immediate is carried verbatim across
  versions far more reliably than the address. NOTE: the allocator arity can change
  (v83 `Alloc(heapsel, size)` two-arg → v84 `Alloc(size)` one-arg); read the size
  as the sole/last push.
- **`*_INSTANCE_ADDR` globals** are the `mov [g_xxx], <this>` store at the TOP of
  the class's placement ctor (the `(this+1!=0)?this:0` SBB idiom), NOT a store in
  the CreateInstance caller. Resolve the pair together: CreateInstance names the
  ctor (its single callee), and the ctor names the instance global (its first store).
- **`DEFAULT_*` key-table globals** (e.g. DEFAULT_FKM/DEFAULT_QKM) are the memcpy
  SOURCE operands inside the owning manager's ctor; the copy size disambiguates
  which default table (e.g. 445-byte FKM vs 32-byte QKM copies).
- **Watch for v83-key referent quirks**: a v83 `*_INSTANCE_ADDR` value may point at
  the wrong global (e.g. the shared heap-selector instead of the real instance —
  seen on C_RADIO_MANAGER_INSTANCE_ADDR). Anchor to the ctor's actual store and
  flag the discrepancy rather than carrying the wrong referent forward.
- **Packet senders** reference their opcode as a `push <imm>` / `mov` immediate
  into a COutPacket; the opcode constant disambiguates among similar senders.
- **`*_OFFSET` keys** are call-site or branch offsets *within* a host function;
  always re-measure against v84 disassembly.
- **Vtable-derived keys** (`C_LOGO_GET_RTTI`, `*_VFTABLE`): find the class
  vtable via its RTTI/type string, then index the slot.
- When two candidates are indistinguishable, disambiguate by a unique callee or
  by the surrounding string the real one references — don't guess by address
  proximity to v83 alone.
