# GMS v61.1 — SecondaryStat full layout reconstruction (Task 15c / for Task 17 gate)

Reconstructed READ-ONLY from the v61 binary (`mcp__ida-pro__disasm` only; no struct
types applied). Lane: **v61 = port 13344 `GMS_v61.1_U_DEVM.exe`** (re-confirmed via
`list_instances` after every switch); diff partner **v72 = port 13343
`GMS_v72.1_U_DEVM.exe`**. v48 distractor (13345) never probed.

`sizeof(SecondaryStat)` v61 = **0x970** (anchored by Task 13/15: CWvsContext
m_secondaryStat@+0x2114 .. m_forcedStat@+0x2A84). v72 = **0xAB0** (ctx-delta). Δ = **0x140**.

## Method (what overturns Task 15/15b)

The struct is `TemporaryStatBase` (0xC bytes: vtable + base) followed by an ordered list of
`_ZtlSecureTear<long>` stat slots (each stat = 3–4 contiguous 12-byte tears), ending in a
trailing "two-state base-stat" array. Three independent enumerators were diffed v61↔v72:

- **`SecondaryStat::Reset(UINT128)`** — v61 @0x662704 / v72 @0x6ca91a (both 0xf61). Tests each
  stat's flag mask, and for set stats zeroes its tears via explicit `lea edx,[esi+OFF]` — the
  authoritative **offset** source. 59 mask-gated blocks + a trailing array loop in BOTH versions.
- **`SecondaryStat::DecodeForLocal`** — v61 @0x663665. Enumerates the SAME 59 masks in the SAME
  order, then the trailing two-state loop — proves Reset skips nothing; the 59 masks ARE the
  full per-bit stat set.
- The **flag-mask globals** are a split table whose ADDRESS encodes the atlas wire-bit:
  - Low table `unk_977BD8` descending by 0x10 → **bits 0–49**: `bit = (0x977BD8 − addr)/0x10`.
  - Extension table `unk_977C98` descending by 0x10 (9 entries, interspersed) → **bits 50–58**:
    `bit = 50 + (0x977C98 − addr)/0x10`.
  (The globals are runtime-initialized BSS — statically zero — so the bit is read from the table
  ADDRESS, not the value. v61 `Reset`/`DecodeForLocal` and v72 `Reset` share the identical
  table-jump fingerprint block-for-block, so the 59-stat bit sequence is identical in both.)

Bit→name uses the canonical atlas order (`atlas/libs/atlas-constants/character/temporary_stat.go`
+ `model/character_temporary_stat.go`) and the v95 IDA-verified table
(`atlas-ms .../task-086-mount-system/v95_secondarystat_table.md`). The low table maps 1:1 onto
atlas bits 0–49; the extension table onto atlas bits 50–58.

### KEY FINDING — v61 is NOT "v72 minus trailing stats"
v61 and v72 carry the **identical 59 masked stats (bits 0–58) in identical order** AND a trailing
two-state array. The struct is **byte-for-byte identical v61↔v72 from 0x0 through 0x78C** (bits
0–39, ~83% of the struct). The −0x140 is **concentrated in exactly 4 sites**, all late:

| Site | stat (atlas bit) | v61 footprint | v72 footprint | Δ (v72 extra) |
|---|---|---|---|---|
| A | **ShadowClaw / SpiritJavelin** (bit40) @0x78C | 0x24 (3 tears) | 0x48 | **+0x24** |
| B | **Infinity** (bit41) @0x7B0(v61)/0x7D4(v72) | 0x30 (4 tears) | 0x78 | **+0x48** |
| C | **GhostMorph / Ghost** (bit49) + trailing pre-array region | 0x24 | 0xDC | **+0xB8** |
| D | **trailing two-state array** | 6 × 4B = 0x18 @0x958 | 7 × 8B = 0x38 @0xA7C | **+0x20** |

Σ = 0x24+0x48+0xB8+0x20 = **0x144** (v72-side array arithmetic lands 0xAB4 vs the 0xAB0 ctx-delta;
the 4-byte reconciliation is a v72-side open item and does not affect the exact v61 = 0x970).

This **overturns** Task 15 ("growth interspersed", "≈6–7 trailing stats v61 lacks") and Task 15b
("contiguous-late single block; names blocked"): the divergence is 3 named-stat region expansions
+ the trailing array, NOT missing whole stats.

## v61 SecondaryStat ordered layout (offset → stat), to 0x970

Each row = one masked stat group; "start" = first tear offset (from v61 `Reset` `lea [esi+OFF]`);
"size" = bytes to the next group (footprint). Tears are 12B (`_ZtlSecureTear<long>`). Confidence:
**A** = bit-arithmetic anchored + atlas-canonical; **C(reg)** = extension-table inferred (bits
50–58). v61 and v72 offsets are identical for every row through bit39.

| start | size | atlas bit | atlas / canonical name | header member (`common/SecondaryStat.h`) | conf |
|---|---|---|---|---|---|
| 0x000 | 0x0C | — | `TemporaryStatBase` base (vtable + UINT128 region) | base class | A |
| 0x00C | 0x30 | 0 | WeaponAttack | nPAD group | A |
| 0x03C | 0x30 | 1 | WeaponDefense | nPDD group | A |
| 0x06C | 0x30 | 2 | MagicAttack | nMAD group | A |
| 0x09C | 0x30 | 3 | MagicDefense | nMDD group | A |
| 0x0CC | 0x30 | 4 | Accuracy | nACC group | A |
| 0x0FC | 0x30 | 5 | Avoidability | nEVA group | A |
| 0x12C | 0x30 | 6 | Hands | nCraft group | A |
| 0x15C | 0x30 | 50 | **Barrier** | nBarrier_ | C(reg) |
| 0x18C | 0x24 | 51 | **Confuse** (v95 name ReverseInput) | nReverseInput_ (v61=int) | C(reg) |
| 0x1B0 | 0x24 | 7 | Speed | nSpeed group | A |
| 0x1D4 | 0x30 | 8 | Jump | nJump group | A |
| 0x204 | 0x24 | 9 | MagicGuard | nMagicGuard_ | A |
| 0x228 | 0x24 | 10 | DarkSight | nDarkSight_ | A |
| 0x24C | 0x24 | 11 | Booster | nBooster_ | A |
| 0x270 | 0x24 | 12 | PowerGuard | nPowerGuard_ | A |
| 0x294 | 0x24 | 13 | HyperBodyHP (MaxHP) | nMaxHP_ | A |
| 0x2B8 | 0x24 | 14 | HyperBodyMP (MaxMP) | nMaxMP_ | A |
| 0x2DC | 0x24 | 52 | **ItemUpByItem** | nItemUpByItem group | C(reg) |
| 0x300 | 0x24 | 15 | Invincible | nInvincible_ | A |
| 0x324 | 0x24 | 16 | SoulArrow | nSoulArrow_ | A |
| 0x348 | 0x24 | 17 | Stun | nStun_ | A |
| 0x36C | 0x30 | 18 | Poison | nPoison_ | A |
| 0x39C | 0x24 | 19 | Seal | nSeal_ | A |
| 0x3C0 | 0x24 | 20 | Darkness | nDarkness_ | A |
| 0x3E4 | 0x24 | 21 | Combo | nComboCounter_ | A |
| 0x408 | 0x24 | 22 | WhiteKnightCharge | nWeaponCharge_ | A |
| 0x42C | 0x24 | 23 | DragonBlood | nDragonBlood_ | A |
| 0x450 | 0x24 | 24 | HolySymbol | nHolySymbol_ | A |
| 0x474 | 0x24 | 25 | MesoUp | nMesoUp_ | A |
| 0x498 | 0x24 | 26 | ShadowPartner | nShadowPartner_ | A |
| 0x4BC | 0x24 | 53 | **RespectPImmune** | nRespectPImmune group | C(reg) |
| 0x4E0 | 0x24 | 27 | PickPocket | nPickPocket_ | A |
| 0x504 | 0x24 | 54 | **RespectMImmune** | nRespectMImmune group | C(reg) |
| 0x528 | 0x24 | 55 | **DefenseAttack** | nDefenseAtt group | C(reg) |
| 0x54C | 0x24 | 56 | **DefenseState** | nDefenseState group | C(reg) |
| 0x570 | 0x24 | 28 | MesoGuard | nMesoGuard_ | A |
| 0x594 | 0x24 | 57 | **IncreaseEffectHpPotion** | nIncEffectHPPotion group | C(reg) |
| 0x5B8 | 0x24 | 29 | Thaw | nThaw_ | A |
| 0x5DC | 0x30 | 30 | Weaken (Weakness) | nWeakness_ | A |
| 0x60C | 0x24 | 31 | Curse | nCurse_ | A |
| 0x630 | 0x24 | 58 | **IncreaseEffectMpPotion** | nIncEffectMPPotion group | C(reg) |
| 0x654 | 0x30 | 32 | Slow | nSlow_ | A |
| 0x684 | 0x24 | 33 | Morph | nMorph_ | A |
| 0x6A8 | 0x24 | 34 | Recovery (Regen) | nRegen_ | A |
| 0x6CC | 0x24 | 35 | MapleWarrior (BasicStatUp) | nBasicStatUp_ | A |
| 0x6F0 | 0x24 | 36 | Stance | nStance_ | A |
| 0x714 | 0x24 | 37 | SharpEyes | nSharpEyes_ | A |
| 0x738 | 0x30 | 38 | ManaReflection | nManaReflection_ | A |
| 0x768 | 0x24 | 39 | Seduce (Attract) | nAttract_ | A |
| **0x78C** | **0x24** | **40** | **ShadowClaw (SpiritJavelin)** — SITE A | nSpiritJavelin_ | A |
| **0x7B0** | **0x30** | **41** | **Infinity** — SITE B | nInfinity_ | A |
| 0x7E0 | 0x30 | 42 | HolyShield | nHolyshield_ | A |
| 0x810 | 0x3C | 43 | Hamstring | nHamString_ | A |
| 0x84C | 0x30 | 44 | Blind | nBlind_ | A |
| 0x87C | 0x2C | 45 | Concentrate | nConcentration_ | A |
| 0x8A8 | 0x2C | 46 | BanMap | nBanMap_ | A |
| 0x8D4 | 0x30 | 47 | EchoOfHero (MaxLevelBuff) | nMaxLevelBuff_ | A |
| 0x904 | 0x30 | 48 | MesoUpByItem | nMesoUpByItem group | A |
| **0x934** | **0x24** | **49** | **GhostMorph (Ghost)** — SITE C | nGhost_ | A |
| **0x958** | **0x18** | 122.. | trailing two-state base-stat array — SITE D | aTemporaryStat[] | A/infer |
| 0x970 | — | — | end of struct | | |

(The two `lea` offsets `Reset` clears per block confirm each stat is a 3- or 4-tear group; the
"size" column is the exact gap to the next masked stat = that stat's full footprint.)

### Trailing two-state array (SITE D), v61
`Reset` tail: `add esi, 958h; loop edi=0..5 (cmp edi,6), add esi,4, reset ZRef`. So v61 =
**6 entries × 4 bytes = 0x18** at **0x958..0x970**. v72 tail: `add esi, 0A7Ch; loop edi=0..6
(cmp edi,7), add esi,8` = **7 entries × 8 bytes = 0x38** at 0xA7C. By the v95 two-state order
(bits 122–128: EnergyCharged, Dash_Speed, Dash_Jump, RideVehicle/MonsterRiding,
SpeedInfusion, GuidedBullet, Undead), v61 carries the first **6** and lacks the **7th**
(most-likely **Undead**, the v95-confirmed 7th two-state slot — INFERRED), and each v61 entry is
4 bytes vs v72's 8 bytes.

## Slots v61 OMITS vs v72 (the −0x140), with v72 offsets — for the Task-17 gate

All four sites are LATE; bits 0–39 (0x0–0x78C) are byte-identical, so nothing there is gated.

| site | v61 | v72 | what v72 has that v61 lacks |
|---|---|---|---|
| **A — SpiritJavelin (bit40)** | 3 tears @0x78C (0x24) | 6 tears @0x78C (0x48) | **+0x24 = 3 extra `_ZtlSecureTear<long>`** appended to the SpiritJavelin group (mSpiritJavelin_ + 2 more; individual names not separately resolved — see flag) |
| **B — Infinity (bit41)** | 4 tears @0x7B0 (0x30) | 10 tears @0x7D4 (0x78) | **+0x48 = 6 extra tears** appended to the Infinity group |
| **C — GhostMorph (bit49) trailing** | 0x24 @0x934 | 0xDC @0x9A0 | **+0xB8** of trailing struct members between GhostMorph and the two-state array (the bulk of the delta; not individually named — see flag) |
| **D — two-state array** | 6 × 4B (0x18) @0x958 | 7 × 8B (0x38) @0xA7C | **+0x20**: one extra entry (likely Undead) AND each entry widened 4B→8B |

### Confidence summary
- **Anchored (A):** all 50 low-table stats (atlas bits 0–49) by mask-address arithmetic +
  atlas-canonical names; the exact v61 offsets/footprints of every one of the 59 groups; the
  4-site localization of the −0x140; v61 array = 6×4 = 0x18 ending exactly at 0x970.
- **High-confidence inferred (C-reg):** the 9 extension-table stats are atlas bits **50–58**
  (Barrier…IncreaseEffectMpPotion), from the extension-table descending order + the matching
  9-stat atlas window. Not individually value-confirmed (BSS masks).
- **FLAGGED (individually un-named):** the 3 v72-extra tears in SITE A, the 6 in SITE B, and the
  0xB8 trailing block in SITE C are localized by offset+byte-count and anchored to their named
  neighbour stat, but the individual member names of those v72-only tears are NOT pinned (they are
  reached through per-stat setter helpers in `DecodeForLocal`, not direct `lea [esi+OFF]`).
  Resolving them would require enumerating v72's setter helpers (`sub_667xxx`) — out of scope for
  the v61 reconstruction, which is complete and exact at 0x970.

## Task-17 recommendation
`common/SecondaryStat.h` is a v95+ over-model whose field order/inventory matches NEITHER v61 NOR
v72 (15b: header lands the array ~0xDE4 vs real v72 0xA7C / v61 0x958), so a clean
delete-header-members `<72` gate is not derivable from the header. Because the v61↔v72 delta is
(a) concentrated in 3 late named-stat regions + the trailing array and (b) **LATENT** for v61
(15b: no v61 hook reads any CWvsContext field after m_secondaryStat; no `static_assert` in the
header), the faithful options for Task 17 are:
1. **Preferred:** add a `BUILD_MAJOR_VERSION < 72` (GMS-guarded) arm that models SecondaryStat
   from THIS reconstruction (59 groups at the offsets above + a 6×4-byte two-state array) →
   `sizeof == 0x970`. This requires rebuilding the early portion as a faithful tear list, since
   the header prefix is itself unfaithful — i.e. a self-contained v61 block, not a trim.
2. **Acceptable (non-breaking):** leave the model v95-shaped (mis-sized, model-only) — it is
   latent and carries no size assert. Document the −0x140 as the 4 sites above.
Either way: the bits 0–39 region is shared with v72 and must NOT be gated; only sites A/B/C/D are
v61-vs-v72 divergent.
