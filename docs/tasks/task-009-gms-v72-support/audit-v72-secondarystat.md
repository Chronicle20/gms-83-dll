# `SecondaryStat` v72 layout audit (SIZE-CRITICAL)

**Target IDB:** `GMS_v72.1_U_DEVM.exe`, MD5 `05a62ca755b1d3719223426b4eee41a9` (session `eb2a156e`), verified via `survey_binary` before any probe.
**Cross-ref IDB:** `GMS_v79_1_DEVM.exe` (session `88dfa464`) — authoritative v79 layout (0xB88 @ aTemporaryStat 0xB50).
**Methodology:** disassembled (never decompiled) the v72 SecondaryStat ctor, `OnTemporaryStatReset`, and `DecodeForRemote`, and positionally diffed `DecodeForRemote` against v79. Offsets read from raw `[reg+OFFh]`.

## Bottom line

| | v72 (this audit) | v79/v83 base | Δ |
| --- | --- | --- | --- |
| `sizeof(SecondaryStat)` | **0xAB0 (2736)** | 0xB88 (2952) | **−0xD8 (−216)** |
| `offsetof(aTemporaryStat)` | **0xA78** | 0xB50 | −0xD8 |
| embed offset in `CWvsContext` | +0x212C | +0x212C | 0 |

The pre-existing v72 audit's **final size 0xAB0 is CORRECT**, but its stated *"array base 0xA94"* is a **mislabel** — 0xA94 is RideVehicle's pointer field (`aTemporaryStat[3]` + 4), not the array base. The true array base is **0xA78**, and elements are **8 bytes** (pointer at element+4), identical to the v79 model — not the 4-byte/0x1C model the old doc implied. Both models happen to yield 0xAB0.

## Binary anchors (airtight)

**1. Array base + element size (definitive) — ctor `sub_6C70E9`:**
```
6c70f9  push offset sub_6D89E1        ; dtor
6c70fe  push offset sub_6D89F7        ; ctor
6c7103  mov  esi, ecx                 ; esi = this
6c7105  push 7                        ; count
6c7107  lea  eax, [esi+0A78h]         ; aTemporaryStat base = this+0xA78
6c710d  push 8                        ; element size = 8
6c7113  call `eh vector constructor iterator`
```
→ array = 7 × 8 = 0x38 → **sizeof = 0xA78 + 0x38 = 0xAB0.**
Corroborated: `DecodeForRemote` @0x6d0582 iterates the pointer field with `add esi, 0A7Ch` (= 0xA78 + 4, the ZRef pointer at element+4).

**2. Embed @ CWvsContext+0x212C — `OnTemporaryStatReset` @0x918F3C:**
`918f73 add edi, 212Ch` (v79 identical @0x96AB6F). RideVehicle slot `[edi+0A94h]`, GuidedBullet slot `[edi+0AA4h]` → element[3]/element[5] pointer fields (base 0xA78, +0x1C / +0x2C), Δ vs v79 (0xB6C / 0xB7C) = −0xD8. `SecondaryStat::Reset` = 0x6ca91a.

**3. Shared-field anchors (v72 == v79, identical offsets) — `DecodeForRemote` @0x6cfe78:**
- `nDefenseAtt` byte-tear @0x90C (helper `sub_6D8DA5`) — v79 0x90C ✓
- `nDefenseState` byte-tear @0x938 — v79 0x938 ✓
- `nBanMap_` @0x738 — v79 0x738 ✓
- `nWindWalk_` @0xA54 (highest remote-decoded field, **last member before the array in v72**) — v79 0xA54 ✓

## Running-delta map

| v79 offset range | v72 offset range | Δ | Cause |
| --- | --- | --- | --- |
| 0x000 – 0xA77 (through `WindWalk` group, ends 0xA78) | 0x000 – 0xA77 | 0 | byte-identical set (every `DecodeForRemote` field matches) |
| 0xA78 – 0xB4F (`EventRate`..`SmartKnockback`, 18 slots) | — (absent) | — | **v72 lacks these 6 stat groups** |
| 0xB50 (`aTemporaryStat[7]`) | 0xA78 | −0xD8 | array pulled up by the 18 absent slots |
| end 0xB88 | end 0xAB0 | −0xD8 | |

`DecodeForRemote` proof: the two functions emit the **same ordered stat sequence with identical offsets** for all ~30 decoded groups (0x15C … 0xA54). Only the trailing array-loop base differs: v72 `add esi, 0A7Ch` vs v79 `add esi, 0B54h` (Δ 0xD8). No field is shifted → v72 inserts/removes nothing in 0x000–0xA78; the entire −0xD8 is the tail block below.

## The 18 missing slots — PINNED (resolves prior "Task 15" open item)

Header layout walk (v79 config), verified against 3 independent binary offsets (nDefenseAtt@0x90C, nDefenseState_@0x940, nWindWalk_@0xA54):

| header member (group) | slots | v79 offset | in v72? |
| --- | --- | --- | --- |
| `WindWalk` (n/r/t) | 3 | 0xA54–0xA78 | **yes** (last v72 member; array follows at 0xA78) |
| `EventRate` (n/r/t) | 3 | 0xA78 | **no** |
| `ComboAbilityBuff` (n/r/t) | 3 | 0xA9C | **no** |
| `ComboDrain` (n/r/t) | 3 | 0xAC0 | **no** |
| `ComboBarrier` (n/r/t) | 3 | 0xAE4 | **no** |
| `BodyPressure` (n/r/t) | 3 | 0xB08 | **no** |
| `SmartKnockback` (n/r/t) | 3 | 0xB2C–0xB50 | **no** |

6 stats × 3 SecureTear<long> (0xC) = **18 slots = 0xD8**. Arithmetic closes exactly: DefenseState end 0x964 + IncEffectHP/MP(0x60) + Spark/SoulMasterFinal/WindBreakerFinal/ElementalReset(0x90) = 0xA54 (WindWalk) + 0x24 = **0xA78 = v72 array**; v79 continues +6×0x24 = 0xD8 → 0xB50.

These correspond to header lines **566–601** (`_ZtlSecureTear_nEventRate_` … `_ZtlSecureTear_tSmartKnockback__CS`).

## Proposed header gates for `common/SecondaryStat.h`

v72 shares **all** of v79's absences (the `GMS_V79_ABSENT` v95-era blocks) — proven because every `DecodeForRemote` offset is identical, so v72 has zero members v79 lacks in 0x000–0xA78. Currently the header sets `GMS_V79_ABSENT == 1` for v72 (the `#else` arm), which wrongly re-adds the entire v95-era set (~0xE1C). Two edits fix v72:

**1. Widen the `GMS_V79_ABSENT` gate to include v72** (so v72 shares the v79-trimmed base → 0xB88):
```c
#if defined(REGION_GMS) && (BUILD_MAJOR_VERSION == 79 || BUILD_MAJOR_VERSION == 72)
#define GMS_V79_ABSENT 0
#else
#define GMS_V79_ABSENT 1
#endif
```

**2. Add a `GMS_V72_ABSENT` companion** and fence `EventRate`..`SmartKnockback` (removes the final 0xD8 → 0xAB0):
```c
#if defined(REGION_GMS) && BUILD_MAJOR_VERSION == 72
#define GMS_V72_ABSENT 0
#else
#define GMS_V72_ABSENT 1
#endif
```
Wrap header lines 566–601 (currently ungated, between `tWindWalk__CS` and `nComboDrain_`… i.e. the EventRate group through the SmartKnockback group) in `#if GMS_V72_ABSENT` / `#endif`. Add `#undef GMS_V72_ABSENT` next to the existing `#undef GMS_V79_ABSENT`.

Effect matrix (SecondaryStat size):

| build | GMS_V79_ABSENT | GMS_V72_ABSENT | size |
| --- | --- | --- | --- |
| GMS v72 | 0 | 0 | **0xAB0** |
| GMS v79 | 0 | 1 | 0xB88 (unchanged) |
| GMS v83/84/87/95, JMS | 1 | 1 | unchanged |

## Exact `common/v72_layout_guards.h` entry

Create the file (mirroring `common/v79_layout_guards.h`, include at end of `pch.h`):
```c
#pragma once
#include "asserts.h"

#if defined(REGION_GMS) && BUILD_MAJOR_VERSION == 72
// SecondaryStat — anchored to GMS_v72.1_U_DEVM.exe (MD5 05a62ca7…), session eb2a156e.
// ctor sub_6C70E9: eh-vector-ctor `lea eax,[esi+0A78h]; push 8; push 7` -> aTemporaryStat[7]
// @0xA78, 8-byte elems, 0x38 span -> struct ends 0xAB0.
assert_size(sizeof(SecondaryStat), 0xAB0);
static_assert(offsetof(SecondaryStat, aTemporaryStat) == 0xA78,
              "v72 aTemporaryStat @0xA78 (ctor sub_6C70E9 lea [esi+0A78h]; DecodeForRemote add esi,0A7Ch)");
// Shared with v79 (byte-identical through WindWalk); nWindWalk_ is the LAST scalar before the array.
static_assert(offsetof(SecondaryStat, _ZtlSecureTear_nWindWalk_) == 0xA54,
              "v72 nWindWalk_ @0xA54 (DecodeForRemote 0x6cfe78; array 0xA78 follows) — trim point vs v79");
static_assert(offsetof(SecondaryStat, _ZtlSecureTear_nDefenseAtt) == 0x90C,
              "v72 nDefenseAtt byte-tear @0x90C (DecodeForRemote this[..]=sub_6D8DA5(this+0x90C)) — == v79");
static_assert(offsetof(SecondaryStat, _ZtlSecureTear_nDefenseState) == 0x938,
              "v72 nDefenseState byte-tear @0x938 — == v79");
static_assert(offsetof(SecondaryStat, _ZtlSecureTear_nBanMap_) == 0x738,
              "v72 BanMap 4-group @0x738 — == v79 (DecodeForRemote lea [esi+738h])");
#endif
```

## Conflicts with pre-existing v72 numbers (`struct-verification.md`)

- **Final size 0xAB0 — CONFIRMED** (matches old doc).
- **"array base 0xA94 → 0xAB0" — CORRECTED.** 0xA94 is `aTemporaryStat[3]`+4 (RideVehicle pointer field), not the base. True base = **0xA78**; the old "+0x1C" arithmetic used a wrong 4-byte/28-byte array model. Correct model: base 0xA78 + 8-byte×7 = 0x38 = 0xAB0.
- **"18 missing base stats not pinned (Task 15)" — NOW PINNED:** EventRate, ComboAbilityBuff, ComboDrain, ComboBarrier, BodyPressure, SmartKnockback (header lines 566–601).
- Embed @+0x212C, Reset @0x918F3C (`add edi,212Ch`) — CONFIRMED.

## atlas-ms vs binary disagreement (surface for investigation)

`libs/atlas-constants/character/temporary_stat.go` lists **EventRate(73), AranCombo(74), ComboDrain(75), ComboBarrier(76), BodyPressure(77), SmartKnockBack(78)** as base-game stats (its "v95 additions" block only starts at bit 79 `RepeatEffect`). The v72 client binary **lacks all six** — the aTemporaryStat array sits immediately after `WindWalk` (0xA78), leaving no room for them, and they are absent from both the ctor extent and the decode paths. These are Aran combo skills (+ EventRate) introduced to GMS *after* v72. atlas gating that treats them as present for all major versions would over-include six stats for a v72 client — same failure class as the v87 over-inclusion fixed in `Chronicle20/atlas#564`. Recommend a `MajorVersion >= 73/74` (or >= 79) gate for atlas bits 73–78. Everything atlas lists ≤ bit 72 (through WindWalk) matches the v72 binary.

## Open questions

- The six absent stats are confirmed absent by size/extent/decode evidence; their *exact* atlas-version-introduced boundary (v73 vs v74 Aran patch) is inferred historically, not from these binaries. Confirm against a v73 IDB if one is adopted.
- `IncEffectHPPotion`/`IncEffectMPPotion`/`Spark`/`SoulMasterFinal`/`WindBreakerFinal`/`ElementalReset` are inferred present in v72 (fill 0x964–0xA54 identically to v79, and WindWalk@0xA54 is confirmed in v72); they are local-only (not in `DecodeForRemote`) so not individually offset-probed. The nWindWalk_@0xA54 static_assert locks the trim point regardless.
