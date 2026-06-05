# Plan Audit — task-005-custom-ui-framework

**Plan Path:** docs/tasks/task-005-custom-ui-framework/plan.md
**Audit Date:** 2026-06-05
**Branch:** worktree-task-005-custom-ui-framework (HEAD = `264b82f`)
**Base Branch:** main (`origin/main..HEAD` = 44 commits)

## Executive Summary

The task-005 Custom UI Framework plan was **faithfully implemented**. Every phase (0–11)
landed; the Phase 5/6 deviations called out in the three committed findings docs
(`v83_cuiwnd_construction.md`, `v83_cwndman_funckey.md`, `v83_controls.md`) are coherently
implemented and evidence-anchored, not silent skips. All 5 CMake matrix combos configure clean
(the two new edits correctly print "not building for …" on non-GMS83), and all 29 unit tests
across the 5 test binaries pass. The Windows DLL compile + live-client COM/runtime behavior are
CI- and manual-acceptance-validated and out of scope for this Linux audit. **One genuine minor
gap:** `CustomUI_DumpRegistries` shipped as an empty `{}` stub instead of the registry-size
logging body the plan's Task 6.5 Step 4 specified.

## Task Completion

Phases collapsed per the user's "intent as spec" directive; status is per task group.

| # | Task | Status | Evidence / Notes |
|---|------|--------|------------------|
| 0.1 | MinHook → Detours in prd.md + hook-design.md | DONE | `grep -i minhook` on both docs returns nothing; commits `7c0ff2f`, `73d5da3` |
| 1.1 | `C_WND_MAN_PROCESS_KEY` | DONE (adapted) | v83_1.cmake:185 = `0x009E40F1`; thiscall confirmed in funckey findings doc |
| 1.2 | `C_STAGE_DTOR` | DONE | v83_1.cmake:120 = `0x00775F5F` |
| 1.3 | Register/Unregister UI window | DONE (adapted) | v83_1.cmake:187/189 = `0x009E43FF`/`0x009E44BA`; documented as `__cdecl` static |
| 1.4 | CUIWnd ctor/vftable/dtor/size/slots | DONE (adapted) | v83_1.cmake:131-136; **two** ctors recorded (`C_UI_WND_CTOR` string + `C_UI_WND_CTOR_INT` int-only); slot count 14, size 1456 |
| 1.5 | CCtrlButton ctor/vftable/size | DONE (adapted) | v83_1.cmake:138-140; nullary ctor `0x004258E4`, size 1444 |
| 1.6 | CCtrlEdit ctor/vftable/size | DONE (adapted) | v83_1.cmake:142-144; nullary ctor `0x004C9C72`, size 184 |
| 1.7 | `C_IN_PACKET_DECODE2` | DONE | v83_1.cmake:83 = `0x0042470C` |
| 1.8 | `C_FUNC_KEY_MAPPED_MAN_FUNC_KEY_MAPPED` | DONE (adapted-by-removal) | Symbol intentionally **removed** — fully inlined in v83.1 (funckey doc Inv. 2). `grep` confirms absent from all maps/headers. Resolved via direct member-array read; NOT a skip. Commit `ff94367` |
| 1.9 | Memory-map sanity sweep | DONE | All 5 combos configure with no "Missing keys" |
| 2.1 | `CInPacket::Decode2` thunk | DONE | common/CInPacket.cpp |
| 2.2 | CWndMan Register/Unregister/ProcessKey | DONE (adapted) | common/CWndMan.cpp: Register/Unregister as `__cdecl(pWnd)`, ProcessKey thiscall `(msg,vk,lParam)` — matches funckey doc reconciliation. Commit `8721402` |
| 2.3 | CFuncKeyMappedMan FuncKeyMapped + GetInstance | DONE (adapted) | common/CFuncKeyMappedMan.cpp: `FuncKeyMapped` = `return m_aFuncKeyMapped[vk];` (inline mirror); `GetInstance` reads `0x00BED5A0` |
| 2.4 | CCtrlButton ctor | DONE (adapted) | common/CCtrlButton.cpp: nullary ctor thunk. Commit `963b3cf` |
| 2.5 | CCtrlEdit ctor | DONE (adapted) | common/CCtrlEdit.cpp: nullary ctor thunk. Commit `4c7b1da` |
| 2.6 | CUIWnd ctor | DONE (adapted) | common/CUIWnd.cpp: 7-int+UOL ctor thunk (string ctor). Commit `0406a1e` |
| 2.7 | Annotate CStage dtor hook target | DONE | commit `1bc201a` |
| 2.8 | common/ compile-link verify | DEFERRED (expected) | Windows-only; CI-validated. Plan explicitly defers Linux verification |
| 3.1 | host CMakeLists + register subdir | DONE | CMakeLists.txt:79,94; v83 gate + skip message verified |
| 3.2 | dllmain + host_main skeleton (mutex/ready) | DONE | host_main.cpp: `AcquireSingletonMutex`, `g_ready`, `g_double_load` |
| 3.3 | host_config.h + INI parsing | DONE | runtime/host_config.h, ini_config.cpp |
| 4.1 | HotkeyRegistry denylist | DONE | registries/hotkey_registry.*; **6 tests PASS** |
| 4.2 | HotkeyRegistry bind/lookup/unbind | DONE | hotkey_registry_tests PASS |
| 4.3 | PacketRegistry | DONE | registries/packet_registry.*; **5 tests PASS** |
| 4.4 | WindowRegistry handle table | DONE | registries/window_registry.*; **5 tests PASS** |
| 5.1 | SEH dispatch header | DONE | runtime/seh_dispatch.h; used in vtable_patch.cpp:39 |
| 5.2 | CustomUIWnd byte-buffer wrapper | DONE (adapted) | custom_ui_wnd.cpp: uses `C_UI_WND_CTOR_INT` (0x0092C0BF), plain `C_UI_WND_DTOR` for teardown — matches construction doc Q1/Q2/Q5. Commit `6e5559f` |
| 5.3 | Cloned CUIWnd vtable | DONE (adapted) | vtable_patch.cpp: overrides slot 8 (OnButtonClicked), 11 (Draw), 13 (OnCreate no-op) — NOT slot 0/dtor as the plan text wrongly said. Adds text_draw module. Commit `f448800` |
| 5.4 | Show/Hide via CWndMan | DONE (adapted) | custom_ui_wnd.cpp: first-show via `C_WND_CREATE_WND`; re-show/hide via `__cdecl` Register/RemoveWindow free fns. Commit `77aa837` |
| 6.1 | Public ABI header | DONE | abi/custom_ui_abi.h |
| 6.2 | ABI lifecycle + windows | DONE | custom_ui_abi.cpp: GetAbiVersion/IsReady/Create/Show/Hide/Destroy + InitAbiGlobals |
| 6.3 | Hotkey bind + FuncKeyMapped conflict probe | DONE (adapted) | custom_ui_abi.cpp `CustomUI_BindHotkey`: reads `m_aFuncKeyMapped[vk]` nType byte via direct array. Commit `d504462` |
| 6.4 | Packet send/register/unregister | DONE | custom_ui_abi.cpp: outbound-range gate on send, inbound-range gate on register (range check added per commit `264b82f`) |
| 6.5 | Label/Button/Edit controls + text + DumpRegistries | **PARTIAL** | Add{Label,Button,Edit}/Set/Get all implemented (id-based, image-only buttons, framework-drawn labels per controls doc). **`CustomUI_DumpRegistries` is an empty `{}` stub** (custom_ui_abi.cpp:385) — plan Step 4 specified logging the 3 registry sizes. Commit `ac44109` |
| 7.1 | H2 ProcessKey hook | DONE | hooks/process_key_hook.cpp; rising-edge filter, mod snapshot, toggle |
| 7.2 | H1 ProcessPacket hook | DONE | hooks/process_packet_hook.cpp; uses `m_aRecvBuff.GetTailPosition()` (verified accessor) |
| 7.3 | H3a CStage dtor snapshot | DONE | hooks/stage_dtor_hook.cpp + stage_restore.h |
| 7.4 | H3b s_Update restore latch | DONE | hooks/s_update_hook.cpp; `g_pending_restore.exchange(false)` |
| 8.1 | Demo scaffold + Ping/Pong | DONE | custom-ui-demo/*; LoadLibrary+GetProcAddress, IsReady poll, OnPing→0x0F00, OnPong←0x2000 |
| 9.1 | CI matrix coverage | DONE | new edits v83-gated; existing matrix unchanged |
| 9.2 | Package INIs | DONE | CMakeLists.txt:119-130 guarded by `TARGET` checks; custom-ui-host.ini present |
| 10.1 | Top-level README | DONE | README.md:43-53 |
| 10.2 | host README | DONE | custom-ui-host/README.md |
| 10.3 | demo README | DONE | custom-ui-demo/README.md |
| 11.1 | Acceptance log patterns | DONE | custom-ui-host/docs/acceptance.md |

**Completion Rate:** ~43/44 task groups fully done (≈98%)
**Skipped without approval:** 0
**Partial implementations:** 1 (Task 6.5 — DumpRegistries stub)

## Skipped / Deferred Tasks

- **Task 6.5 (PARTIAL) — `CustomUI_DumpRegistries` empty stub.** The exported function exists
  and is declared in the ABI header, but its body is `{}` (custom_ui_abi.cpp:385). The plan's
  Task 6.5 Step 4 specified it should `Log()` the window/hotkey/packet registry sizes. Impact:
  **low** — it is a debug aid only; no acceptance criterion in `acceptance.md` depends on its
  output (criterion 10.4.d relies on the per-call rejection logs, which are implemented). No
  consumer (including the demo) calls it. Git history (`git log -S`) confirms it was never
  populated, so this is a quiet drop rather than a regression.
- **Task 2.8 (DEFERRED, expected) — common_lib Windows compile-link.** Plan explicitly defers
  to CI; not a Linux-checkable item.

## Build Results

Linux host: configure-only for DLL targets (Windows `windows.h` compile runs in CI). Unit tests
build and run natively.

| Region | Version | Configure | Build (tests) | Notes |
|--------|---------|-----------|---------------|-------|
| GMS | 83.1 | PASS | PASS | both new edits configured; 29/29 tests pass |
| GMS | 87.1 | PASS | N/A | custom-ui-host/demo print "not building for GMS_v87_1" |
| GMS | 95.1 | PASS | N/A | both edits skip |
| GMS | 111.1 | PASS | N/A | both edits skip |
| JMS | 185.1 | PASS | N/A | both edits skip |

Unit test detail (configure `-DBUILD_TESTS=ON`, GMS 83.1):

| Test binary | Result |
|---|---|
| hotkey_registry_tests | 6/6 PASS |
| packet_registry_tests | 5/5 PASS |
| window_registry_tests | 5/5 PASS |
| parse_ini_tests | 9/9 PASS |
| byte_ops_tests | 4/4 PASS |

Out of scope (CI / manual acceptance only): the Windows DLL compile of `common_lib` +
`custom-ui-host` + `custom-ui-demo`, and all live-client COM/runtime behavior (window display,
DrawTextA label rendering, hook firing, packet round-trip). Acceptance log patterns are
enumerated in `custom-ui-host/docs/acceptance.md`.

## Manual Verification Required

Per `custom-ui-host/docs/acceptance.md` (run against a live v83.1 client, tail OutputDebugString):

1. 10.3.a — `custom-ui-host: ready` AND `custom-ui-demo: window built, ready`
2. 10.3.b — F8 toggles the window (visual)
3. 10.3.c — `custom-ui-demo: OnPing -> SendPacket(0x0F00)`
4. 10.3.d — `custom-ui-demo: OnPong opcode=0x2000 seq=N` + visible "Server says: pong N"
5. 10.4.a — `custom-ui-demo: host DLL not loaded -- demo inert`
6. 10.4.b — `custom-ui-host: another host instance is already running`
7. 10.4.c — `custom-ui-host: BindHotkey rejected -- vk=0xXX already mapped`
8. 10.4.e — `custom-ui-host: SendPacket rejected -- opcode 0xXXXX outside outbound range`
9. 10.4.f — `custom-ui-host: AV in consumer callback at site=[CustomUI button click]`
10. 10.5.a/c — channel change hide+show; disconnect/reconnect re-installs cleanly

## Overall Assessment

- **Plan Adherence:** MOSTLY_COMPLETE (all intent met; one debug-only stub left empty)
- **Recommendation:** READY_TO_MERGE (the single gap is a non-functional debug aid; flag it for
  the human reviewer but it does not block the milestone)

## Action Items

1. **(Optional, low priority)** Implement `CustomUI_DumpRegistries` body to log
   `g_windows->Size()`, `g_hotkeys->Size()`, `g_packets->Size()` as specified in plan Task 6.5
   Step 4 — or, if intentionally dropped, remove it from the ABI header and acceptance doc to
   keep the surface honest.
2. **(Verification)** Confirm on Windows CI that `common_lib`, `custom-ui-host`, and
   `custom-ui-demo` compile and link clean for GMS 83.1 (Task 2.8 deferred item).
3. **(Manual acceptance)** Owner runs the 10 acceptance patterns above against a live v83.1
   client before declaring the milestone shipped.
