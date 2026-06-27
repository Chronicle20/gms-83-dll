# GMS v61.1 — Struct Size/Layout Verification & Below-Floor Gate Audit

Scope: verify the version-gated `common/*.h` headers against the v61 binary and amend every
enumerated gate that has no catch-all `#else` so v61 selects a defined, measured branch.

IDB lane discipline (R9): v61 = port 13344 `GMS_v61.1_U_DEVM.exe` (active confirmed via
`list_instances`); v48 = port 13345 DISTRACTOR (never confuse); v72 = 13343. Read-only `disasm`
only; no struct types applied to any IDB (R10). v61 retains full mangled C++ symbols.

## Task 3 (Category A) — early gate restoration record

Live Category-A sites re-grepped (`grep -rn "BUILD_MAJOR_VERSION ==" common/*.h`):
`CWvsApp.h:98`, `CFuncKeyMappedMan.h:52`, `CLogin.h:235`. Other `== N` hits — `CLogo.h:93`
(==95), `CConfig.h:84` (==95), `SecondaryStat.h:405/419` (==87||JMS) — are upper-version member
gates **false for v61** that resolve onto the base/excluded branch (Category C, later tasks),
NOT amended here.

### Per-header verdict log

| Header | v61 size | v61 vs v72 | Gate verdict | Deciding v61 evidence |
|---|---|---|---|---|
| CWvsApp.h | **0x58** (88) | **DIVERGES**: v61 0x58 vs v72 0x60 (−0x8) | **branch-added + member-gate** (new `#elif ==61` 0x58 :103; gated `m_tLastServerIPCheck2`+`m_tLastGGHookingAPICheck` behind `>= 72`) | Two size anchors: (a) WinMain stack-ctor `lea ecx,[ebp+var_CC]` @0x8207B3 → object base var_CC(frame 0x330); first post-object local var_74(0x388) ⇒ Δ=**0x58**. (b) `m_ahInput` @**+0x4C** via TWO independent reads — `InitializeInput` 0x8247F9 `add esi,4Ch; push esi` → `CInputSystem::Init(HWND,void**)`, and `Run` 0x823411 `lea eax,[esi+4Ch]` → input poll. Field pins: `m_nTargetVersion`@+0x40 (Run 0x82355A `push [esi+40h]`); `m_tLastServerIPCheck`@+0x44 (CallUpdate 0x82493B/0x8249D3 — getpeername + 30000ms server-IP recheck timer). Only ONE int slot at +0x48 between +0x44 and m_ahInput@+0x4C ⇒ it is the unconditional `m_tLastSecurityCheck` (forced — it cannot be dropped); the two GMS GameGuard-era timers `m_tLastServerIPCheck2`/`m_tLastGGHookingAPICheck` are ABSENT in v61 (added v72). ctor 0x822E44 field-init extent reaches +0x38 (== v72/v79). |
| CFuncKeyMappedMan.h | **0x388** (904) | == v72 (both 0x388) | **branch-added** (`==61` joined the `(==72\|\|==79)` 0x388 assert :52) | Independent re-derivation (not just Task 2 trust): `TSingleton<CFuncKeyMappedMan>::CreateInstance` @0x826038 `push 388h` → `ZAllocEx::Alloc` → ctor 0x51AA0E ⇒ sizeof = **0x388**. Member gate :19 (`>= 83 \|\| JMS`) excludes v61 → quickslot pair absent; :25 (`>= 111`)/:31 (`>= 95`) excluded → header computes 0x388 for v61. Assert compiles & fires (build OK). |
| CLogin.h | 0x1D4+ (one-ZList shape) | == v72 (one ZList; v72 0x23C) | **unchanged** (no edit; `==83` correctly excludes v61) | `CLogin::CLogin` @0x5620D4 builds exactly ONE ZList-vtable object (`off_8E8C54` @this+0x16C = `m_lNewEquip`), then `m_aCmd[5]` ZXString vector-ctor @+0x1C0. NO second ZList ⇒ the v83-only `unk3` ZList (header `int unk3[5]`, 0x14) is ABSENT. Matches v72/v79 (one ZList). `==83` gate excludes v61 correctly. |

(A5: each branch assignment is an **evidence verdict** from measured size, not "oldest ⇒ v83".)

### Edits applied (house `#if/#elif/#endif` form; one-line `task-010` comment each)

- `common/CWvsApp.h` — (1) member-gated `m_tLastServerIPCheck2` + `m_tLastGGHookingAPICheck`
  behind `#if BUILD_MAJOR_VERSION >= 72` (absent in v61; the 0x8 shrink); (2) added
  `#elif … == 61 → assert_size(sizeof(CWvsApp), 0x58)`. The member gate is **required for hook
  correctness** (see below), not just to satisfy the assert.
- `common/CFuncKeyMappedMan.h` — `(==72||==79)` → `(==61||==72||==79)`, asserting `0x388`;
  refreshed the `:16` member-gate comment.
- `common/CLogin.h` — **no edit** (`==83` correctly excludes v61; `unk3[5]` absent).
- `bypass/app_hooks.cpp` — downstream consumer: gated the two `m_tLast…` writes in
  `CWvsApp__CallUpdate_Hook` behind `>= 72` to mirror the header (else v61 fails to compile, and
  more importantly would write to wrong offsets on the real 0x58 object).

### Why the CWvsApp member gate is mandatory (not deferrable to the audit)

CWvsApp is **stack-constructed by the real v61 client's WinMain** (0x58 bytes); our hooks
(`CallUpdate`, `SetUp`, …) receive that `pThis` and access fields **by header offset**. If the
header modelled 0x60 (the two extra ints present), `m_tLastSecurityCheck` and `m_ahInput` would
resolve to +0x50/+0x54 instead of the real v61 +0x48/+0x4C → our hooks would corrupt the object.
So modelling v61's true layout (member gate → 0x58) is a correctness requirement; the matching
size assert just locks it. (Contrast the v79 CFuncKeyMappedMan deferral, where no hook indexed the
shifted region.)

### FR-13 cross-version truth table — preprocessor-selected branch (verbatim from `gcc -E`)

| Version | CWvsApp.h | Changed? | CFuncKeyMappedMan.h | Changed? | CLogin.h unk3[5] | Changed? |
|---|---|---|---|---|---|---|
| GMS 61 | 0x58 | **NEW (was none)** | 0x388 | **NEW (was none)** | absent | unchanged |
| GMS 72 | 0x60 | unchanged | 0x388 | unchanged | absent | unchanged |
| GMS 79 | 0x60 | unchanged | 0x388 | unchanged | absent | unchanged |
| GMS 83 | 0x60 | unchanged | 0x3C8 | unchanged | **present** | unchanged |
| GMS 84 | 0x60 | unchanged | 0x3C8 | unchanged | absent | unchanged |
| GMS 87 | 0x6C | unchanged | 0x3C8 | unchanged | absent | unchanged |
| GMS 95 | 0x8C | unchanged | 0x3CC | unchanged | absent | unchanged |
| GMS 111 | 0x8C | unchanged | 0x3D0 | unchanged | absent | unchanged |
| JMS 185 | 0x64 | unchanged | 0x400 | unchanged | absent | unchanged |

`== 61` and `>= 72` are disjoint from every other supported version's selector (no supported GMS
version equals 61, and 72/79/83/84/87/95/111 all satisfy `>= 72`; JMS is `REGION_JMS`-gated, the
GMS-only ints never apply). v61 moves from "no branch selected (silent guard loss / silent member
shift)" to a defined, measured layout + assert. **v111 IDB not loaded** — validated from build
constants (`>= 95` → 0x8C, `== 111` → 0x3D0); unaffected because the v61 edits are strictly
additive/disjoint and never touch a `>= 95`/`== 111` selector.

### Empirical verification (exactly what was run)

- `scripts/wsl-build.sh GMS 61 1` → `>> OK` (compiles **and links** all edit DLLs) — the now-selected
  `assert_size(...,0x58)` and `assert_size(...,0x388)` **fire and hold** against the real header
  layout (the member gate makes the header compute 0x58).
- `scripts/wsl-build.sh GMS 72 1` → `>> OK` — neighbor, no regression.
- `gcc -E -DREGION_{GMS,JMS} -DBUILD_MAJOR_VERSION={61,72,79,83,84,87,95,111,185}` on each amended
  header → the full matrix above (selected `static_assert` size + `unk3[5]` presence per version).

**Finding:** v61 was not previously *blocked* from compiling CWvsApp.h/CFuncKeyMappedMan.h (the
gates had no `#else`, so v61 silently selected no `assert_size`). The amendment restores the guard.
The genuinely load-bearing discovery is the CWvsApp **0x58 below-floor member shift** (−2 GMS
security ints vs v72), which both sizes the object and fixes the offsets our hooks use on v61.
