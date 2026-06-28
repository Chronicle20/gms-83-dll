#pragma once
#include "TemporaryStatBase.h"
#include "asserts.h"

struct SecondaryStat {
#if defined(REGION_GMS) && BUILD_MAJOR_VERSION < 72
    // ============================================================================
    // v61 faithful tail — transcribed from docs/tasks/task-010-gms-v61-support/
    // v61_secondarystat_layout.md (full rebuild, user decision task-010).
    // Members named where the binary pinned them; _ZtlSecureTear_unkNNN_ placeholders
    // (offset + size commented) where a v61 tear is not individually named. Reaches
    // exact sizeof 0x970 — locked by the assert_size below. v61 and v72 are byte-
    // identical 0x0..0x78C; the divergence is concentrated in 4 late sites
    // (A=SpiritJavelin@0x78C, B=Infinity@0x7B0, C=GhostMorph@0x934, D=array@0x958),
    // but the over-model #else prefix is itself unfaithful to v61 (different tear
    // counts/order), so the whole v61 body is rebuilt self-contained here.
    //
    // Base region @0x000 size 0xC (vtable + stat-set mask head; nPAD starts at 0xC).
    void* _vfptr;                                                              // 0x000
    unsigned int _uStatHead[2];                                                // 0x004..0x00C
    // bit0 WeaponAttack @0x00C (0x30, 4 tears)
    int _ZtlSecureTear_nPAD[2];        unsigned int _ZtlSecureTear_nPAD_CS;    // 0x00C
    int _ZtlSecureTear_nPAD_[2];       unsigned int _ZtlSecureTear_nPAD__CS;   // 0x018
    int _ZtlSecureTear_rPAD_[2];       unsigned int _ZtlSecureTear_rPAD__CS;   // 0x024
    int _ZtlSecureTear_tPAD_[2];       unsigned int _ZtlSecureTear_tPAD__CS;   // 0x030
    // bit1 WeaponDefense @0x03C (0x30, 4)
    int _ZtlSecureTear_nPDD[2];        unsigned int _ZtlSecureTear_nPDD_CS;    // 0x03C
    int _ZtlSecureTear_nPDD_[2];       unsigned int _ZtlSecureTear_nPDD__CS;   // 0x048
    int _ZtlSecureTear_rPDD_[2];       unsigned int _ZtlSecureTear_rPDD__CS;   // 0x054
    int _ZtlSecureTear_tPDD_[2];       unsigned int _ZtlSecureTear_tPDD__CS;   // 0x060
    // bit2 MagicAttack @0x06C (0x30, 4)
    int _ZtlSecureTear_nMAD[2];        unsigned int _ZtlSecureTear_nMAD_CS;    // 0x06C
    int _ZtlSecureTear_nMAD_[2];       unsigned int _ZtlSecureTear_nMAD__CS;   // 0x078
    int _ZtlSecureTear_rMAD_[2];       unsigned int _ZtlSecureTear_rMAD__CS;   // 0x084
    int _ZtlSecureTear_tMAD_[2];       unsigned int _ZtlSecureTear_tMAD__CS;   // 0x090
    // bit3 MagicDefense @0x09C (0x30, 4)
    int _ZtlSecureTear_nMDD[2];        unsigned int _ZtlSecureTear_nMDD_CS;    // 0x09C
    int _ZtlSecureTear_nMDD_[2];       unsigned int _ZtlSecureTear_nMDD__CS;   // 0x0A8
    int _ZtlSecureTear_rMDD_[2];       unsigned int _ZtlSecureTear_rMDD__CS;   // 0x0B4
    int _ZtlSecureTear_tMDD_[2];       unsigned int _ZtlSecureTear_tMDD__CS;   // 0x0C0
    // bit4 Accuracy @0x0CC (0x30, 4)
    int _ZtlSecureTear_nACC[2];        unsigned int _ZtlSecureTear_nACC_CS;    // 0x0CC
    int _ZtlSecureTear_nACC_[2];       unsigned int _ZtlSecureTear_nACC__CS;   // 0x0D8
    int _ZtlSecureTear_rACC_[2];       unsigned int _ZtlSecureTear_rACC__CS;   // 0x0E4
    int _ZtlSecureTear_tACC_[2];       unsigned int _ZtlSecureTear_tACC__CS;   // 0x0F0
    // bit5 Avoidability @0x0FC (0x30, 4)
    int _ZtlSecureTear_nEVA[2];        unsigned int _ZtlSecureTear_nEVA_CS;    // 0x0FC
    int _ZtlSecureTear_nEVA_[2];       unsigned int _ZtlSecureTear_nEVA__CS;   // 0x108
    int _ZtlSecureTear_rEVA_[2];       unsigned int _ZtlSecureTear_rEVA__CS;   // 0x114
    int _ZtlSecureTear_tEVA_[2];       unsigned int _ZtlSecureTear_tEVA__CS;   // 0x120
    // bit6 Hands @0x12C (0x30, 4)
    int _ZtlSecureTear_nCraft[2];      unsigned int _ZtlSecureTear_nCraft_CS;  // 0x12C
    int _ZtlSecureTear_nCraft_[2];     unsigned int _ZtlSecureTear_nCraft__CS; // 0x138
    int _ZtlSecureTear_rCraft_[2];     unsigned int _ZtlSecureTear_rCraft__CS; // 0x144
    int _ZtlSecureTear_tCraft_[2];     unsigned int _ZtlSecureTear_tCraft__CS; // 0x150
    // bit50 Barrier @0x15C (0x30, 4) [ext-table inferred]
    int _ZtlSecureTear_nBarrier_[2];   unsigned int _ZtlSecureTear_nBarrier__CS;  // 0x15C
    int _ZtlSecureTear_rBarrier_[2];   unsigned int _ZtlSecureTear_rBarrier__CS;  // 0x168
    int _ZtlSecureTear_tBarrier_[2];   unsigned int _ZtlSecureTear_tBarrier__CS;  // 0x174
    int _ZtlSecureTear_unk180_[2];     unsigned int _ZtlSecureTear_unk180__CS;    // 0x180 v61 Barrier 4th tear, name unresolved
    // bit51 Confuse/ReverseInput @0x18C (0x24, 3) [ext-table inferred]
    int _ZtlSecureTear_nReverseInput_[2]; unsigned int _ZtlSecureTear_nReverseInput__CS; // 0x18C
    int _ZtlSecureTear_rReverseInput_[2]; unsigned int _ZtlSecureTear_rReverseInput__CS; // 0x198
    int _ZtlSecureTear_tReverseInput_[2]; unsigned int _ZtlSecureTear_tReverseInput__CS; // 0x1A4
    // bit7 Speed @0x1B0 (0x24, 3)
    int _ZtlSecureTear_nSpeed_[2];     unsigned int _ZtlSecureTear_nSpeed__CS;    // 0x1B0
    int _ZtlSecureTear_rSpeed_[2];     unsigned int _ZtlSecureTear_rSpeed__CS;    // 0x1BC
    int _ZtlSecureTear_tSpeed_[2];     unsigned int _ZtlSecureTear_tSpeed__CS;    // 0x1C8
    // bit8 Jump @0x1D4 (0x30, 4)
    int _ZtlSecureTear_nJump[2];       unsigned int _ZtlSecureTear_nJump_CS;      // 0x1D4
    int _ZtlSecureTear_nJump_[2];      unsigned int _ZtlSecureTear_nJump__CS;     // 0x1E0
    int _ZtlSecureTear_rJump_[2];      unsigned int _ZtlSecureTear_rJump__CS;     // 0x1EC
    int _ZtlSecureTear_tJump_[2];      unsigned int _ZtlSecureTear_tJump__CS;     // 0x1F8
    // bit9 MagicGuard @0x204 (0x24, 3)
    int _ZtlSecureTear_nMagicGuard_[2]; unsigned int _ZtlSecureTear_nMagicGuard__CS; // 0x204
    int _ZtlSecureTear_rMagicGuard_[2]; unsigned int _ZtlSecureTear_rMagicGuard__CS; // 0x210
    int _ZtlSecureTear_tMagicGuard_[2]; unsigned int _ZtlSecureTear_tMagicGuard__CS; // 0x21C
    // bit10 DarkSight @0x228 (0x24, 3)
    int _ZtlSecureTear_nDarkSight_[2]; unsigned int _ZtlSecureTear_nDarkSight__CS; // 0x228
    int _ZtlSecureTear_rDarkSight_[2]; unsigned int _ZtlSecureTear_rDarkSight__CS; // 0x234
    int _ZtlSecureTear_tDarkSight_[2]; unsigned int _ZtlSecureTear_tDarkSight__CS; // 0x240
    // bit11 Booster @0x24C (0x24, 3)
    int _ZtlSecureTear_nBooster_[2];   unsigned int _ZtlSecureTear_nBooster__CS;  // 0x24C
    int _ZtlSecureTear_rBooster_[2];   unsigned int _ZtlSecureTear_rBooster__CS;  // 0x258
    int _ZtlSecureTear_tBooster_[2];   unsigned int _ZtlSecureTear_tBooster__CS;  // 0x264
    // bit12 PowerGuard @0x270 (0x24, 3)
    int _ZtlSecureTear_nPowerGuard_[2]; unsigned int _ZtlSecureTear_nPowerGuard__CS; // 0x270
    int _ZtlSecureTear_rPowerGuard_[2]; unsigned int _ZtlSecureTear_rPowerGuard__CS; // 0x27C
    int _ZtlSecureTear_tPowerGuard_[2]; unsigned int _ZtlSecureTear_tPowerGuard__CS; // 0x288
    // bit13 HyperBodyHP (MaxHP) @0x294 (0x24, 3)
    int _ZtlSecureTear_nMaxHP_[2];     unsigned int _ZtlSecureTear_nMaxHP__CS;    // 0x294
    int _ZtlSecureTear_rMaxHP_[2];     unsigned int _ZtlSecureTear_rMaxHP__CS;    // 0x2A0
    int _ZtlSecureTear_tMaxHP_[2];     unsigned int _ZtlSecureTear_tMaxHP__CS;    // 0x2AC
    // bit14 HyperBodyMP (MaxMP) @0x2B8 (0x24, 3)
    int _ZtlSecureTear_nMaxMP_[2];     unsigned int _ZtlSecureTear_nMaxMP__CS;    // 0x2B8
    int _ZtlSecureTear_rMaxMP_[2];     unsigned int _ZtlSecureTear_rMaxMP__CS;    // 0x2C4
    int _ZtlSecureTear_tMaxMP_[2];     unsigned int _ZtlSecureTear_tMaxMP__CS;    // 0x2D0
    // bit52 ItemUpByItem @0x2DC (0x24, 3) [ext-table inferred]
    int _ZtlSecureTear_nItemUpByItem_[2]; unsigned int _ZtlSecureTear_nItemUpByItem__CS; // 0x2DC
    int _ZtlSecureTear_rItemUpByItem_[2]; unsigned int _ZtlSecureTear_rItemUpByItem__CS; // 0x2E8
    int _ZtlSecureTear_tItemUpByItem_[2]; unsigned int _ZtlSecureTear_tItemUpByItem__CS; // 0x2F4
    // bit15 Invincible @0x300 (0x24, 3)
    int _ZtlSecureTear_nInvincible_[2]; unsigned int _ZtlSecureTear_nInvincible__CS; // 0x300
    int _ZtlSecureTear_rInvincible_[2]; unsigned int _ZtlSecureTear_rInvincible__CS; // 0x30C
    int _ZtlSecureTear_tInvincible_[2]; unsigned int _ZtlSecureTear_tInvincible__CS; // 0x318
    // bit16 SoulArrow @0x324 (0x24, 3)
    int _ZtlSecureTear_nSoulArrow_[2]; unsigned int _ZtlSecureTear_nSoulArrow__CS; // 0x324
    int _ZtlSecureTear_rSoulArrow_[2]; unsigned int _ZtlSecureTear_rSoulArrow__CS; // 0x330
    int _ZtlSecureTear_tSoulArrow_[2]; unsigned int _ZtlSecureTear_tSoulArrow__CS; // 0x33C
    // bit17 Stun @0x348 (0x24, 3)
    int _ZtlSecureTear_nStun_[2];      unsigned int _ZtlSecureTear_nStun__CS;     // 0x348
    int _ZtlSecureTear_rStun_[2];      unsigned int _ZtlSecureTear_rStun__CS;     // 0x354
    int _ZtlSecureTear_tStun_[2];      unsigned int _ZtlSecureTear_tStun__CS;     // 0x360
    // bit18 Poison @0x36C (0x30, 4)
    int _ZtlSecureTear_nPoison_[2];    unsigned int _ZtlSecureTear_nPoison__CS;   // 0x36C
    int _ZtlSecureTear_rPoison_[2];    unsigned int _ZtlSecureTear_rPoison__CS;   // 0x378
    int _ZtlSecureTear_tPoison_[2];    unsigned int _ZtlSecureTear_tPoison__CS;   // 0x384
    int _ZtlSecureTear_unk390_[2];     unsigned int _ZtlSecureTear_unk390__CS;    // 0x390 v61 Poison 4th tear, name unresolved
    // bit19 Seal @0x39C (0x24, 3)
    int _ZtlSecureTear_nSeal_[2];      unsigned int _ZtlSecureTear_nSeal__CS;     // 0x39C
    int _ZtlSecureTear_rSeal_[2];      unsigned int _ZtlSecureTear_rSeal__CS;     // 0x3A8
    int _ZtlSecureTear_tSeal_[2];      unsigned int _ZtlSecureTear_tSeal__CS;     // 0x3B4
    // bit20 Darkness @0x3C0 (0x24, 3)
    int _ZtlSecureTear_nDarkness_[2];  unsigned int _ZtlSecureTear_nDarkness__CS; // 0x3C0
    int _ZtlSecureTear_rDarkness_[2];  unsigned int _ZtlSecureTear_rDarkness__CS; // 0x3CC
    int _ZtlSecureTear_tDarkness_[2];  unsigned int _ZtlSecureTear_tDarkness__CS; // 0x3D8
    // bit21 Combo @0x3E4 (0x24, 3)
    int _ZtlSecureTear_nComboCounter_[2]; unsigned int _ZtlSecureTear_nComboCounter__CS; // 0x3E4
    int _ZtlSecureTear_rComboCounter_[2]; unsigned int _ZtlSecureTear_rComboCounter__CS; // 0x3F0
    int _ZtlSecureTear_tComboCounter_[2]; unsigned int _ZtlSecureTear_tComboCounter__CS; // 0x3FC
    // bit22 WhiteKnightCharge (WeaponCharge) @0x408 (0x24, 3)
    int _ZtlSecureTear_nWeaponCharge_[2]; unsigned int _ZtlSecureTear_nWeaponCharge__CS; // 0x408
    int _ZtlSecureTear_rWeaponCharge_[2]; unsigned int _ZtlSecureTear_rWeaponCharge__CS; // 0x414
    int _ZtlSecureTear_tWeaponCharge_[2]; unsigned int _ZtlSecureTear_tWeaponCharge__CS; // 0x420
    // bit23 DragonBlood @0x42C (0x24, 3)
    int _ZtlSecureTear_nDragonBlood_[2]; unsigned int _ZtlSecureTear_nDragonBlood__CS; // 0x42C
    int _ZtlSecureTear_rDragonBlood_[2]; unsigned int _ZtlSecureTear_rDragonBlood__CS; // 0x438
    int _ZtlSecureTear_tDragonBlood_[2]; unsigned int _ZtlSecureTear_tDragonBlood__CS; // 0x444
    // bit24 HolySymbol @0x450 (0x24, 3)
    int _ZtlSecureTear_nHolySymbol_[2]; unsigned int _ZtlSecureTear_nHolySymbol__CS; // 0x450
    int _ZtlSecureTear_rHolySymbol_[2]; unsigned int _ZtlSecureTear_rHolySymbol__CS; // 0x45C
    int _ZtlSecureTear_tHolySymbol_[2]; unsigned int _ZtlSecureTear_tHolySymbol__CS; // 0x468
    // bit25 MesoUp @0x474 (0x24, 3)
    int _ZtlSecureTear_nMesoUp_[2];    unsigned int _ZtlSecureTear_nMesoUp__CS;   // 0x474
    int _ZtlSecureTear_rMesoUp_[2];    unsigned int _ZtlSecureTear_rMesoUp__CS;   // 0x480
    int _ZtlSecureTear_tMesoUp_[2];    unsigned int _ZtlSecureTear_tMesoUp__CS;   // 0x48C
    // bit26 ShadowPartner @0x498 (0x24, 3)
    int _ZtlSecureTear_nShadowPartner_[2]; unsigned int _ZtlSecureTear_nShadowPartner__CS; // 0x498
    int _ZtlSecureTear_rShadowPartner_[2]; unsigned int _ZtlSecureTear_rShadowPartner__CS; // 0x4A4
    int _ZtlSecureTear_tShadowPartner_[2]; unsigned int _ZtlSecureTear_tShadowPartner__CS; // 0x4B0
    // bit53 RespectPImmune @0x4BC (0x24, 3) [ext-table inferred]
    int _ZtlSecureTear_nRespectPImmune_[2]; unsigned int _ZtlSecureTear_nRespectPImmune__CS; // 0x4BC
    int _ZtlSecureTear_rRespectPImmune_[2]; unsigned int _ZtlSecureTear_rRespectPImmune__CS; // 0x4C8
    int _ZtlSecureTear_tRespectPImmune_[2]; unsigned int _ZtlSecureTear_tRespectPImmune__CS; // 0x4D4
    // bit27 PickPocket @0x4E0 (0x24, 3)
    int _ZtlSecureTear_nPickPocket_[2]; unsigned int _ZtlSecureTear_nPickPocket__CS; // 0x4E0
    int _ZtlSecureTear_rPickPocket_[2]; unsigned int _ZtlSecureTear_rPickPocket__CS; // 0x4EC
    int _ZtlSecureTear_tPickPocket_[2]; unsigned int _ZtlSecureTear_tPickPocket__CS; // 0x4F8
    // bit54 RespectMImmune @0x504 (0x24, 3) [ext-table inferred]
    int _ZtlSecureTear_nRespectMImmune_[2]; unsigned int _ZtlSecureTear_nRespectMImmune__CS; // 0x504
    int _ZtlSecureTear_rRespectMImmune_[2]; unsigned int _ZtlSecureTear_rRespectMImmune__CS; // 0x510
    int _ZtlSecureTear_tRespectMImmune_[2]; unsigned int _ZtlSecureTear_tRespectMImmune__CS; // 0x51C
    // bit55 DefenseAttack @0x528 (0x24, 3) [ext-table inferred]
    int _ZtlSecureTear_nDefenseAtt_[2]; unsigned int _ZtlSecureTear_nDefenseAtt__CS; // 0x528
    int _ZtlSecureTear_rDefenseAtt_[2]; unsigned int _ZtlSecureTear_rDefenseAtt__CS; // 0x534
    int _ZtlSecureTear_tDefenseAtt_[2]; unsigned int _ZtlSecureTear_tDefenseAtt__CS; // 0x540
    // bit56 DefenseState @0x54C (0x24, 3) [ext-table inferred]
    int _ZtlSecureTear_nDefenseState_[2]; unsigned int _ZtlSecureTear_nDefenseState__CS; // 0x54C
    int _ZtlSecureTear_rDefenseState_[2]; unsigned int _ZtlSecureTear_rDefenseState__CS; // 0x558
    int _ZtlSecureTear_tDefenseState_[2]; unsigned int _ZtlSecureTear_tDefenseState__CS; // 0x564
    // bit28 MesoGuard @0x570 (0x24, 3)
    int _ZtlSecureTear_nMesoGuard_[2]; unsigned int _ZtlSecureTear_nMesoGuard__CS; // 0x570
    int _ZtlSecureTear_rMesoGuard_[2]; unsigned int _ZtlSecureTear_rMesoGuard__CS; // 0x57C
    int _ZtlSecureTear_tMesoGuard_[2]; unsigned int _ZtlSecureTear_tMesoGuard__CS; // 0x588
    // bit57 IncreaseEffectHpPotion @0x594 (0x24, 3) [ext-table inferred]
    int _ZtlSecureTear_nIncEffectHPPotion_[2]; unsigned int _ZtlSecureTear_nIncEffectHPPotion__CS; // 0x594
    int _ZtlSecureTear_rIncEffectHPPotion_[2]; unsigned int _ZtlSecureTear_rIncEffectHPPotion__CS; // 0x5A0
    int _ZtlSecureTear_tIncEffectHPPotion_[2]; unsigned int _ZtlSecureTear_tIncEffectHPPotion__CS; // 0x5AC
    // bit29 Thaw @0x5B8 (0x24, 3)
    int _ZtlSecureTear_nThaw_[2];      unsigned int _ZtlSecureTear_nThaw__CS;     // 0x5B8
    int _ZtlSecureTear_rThaw_[2];      unsigned int _ZtlSecureTear_rThaw__CS;     // 0x5C4
    int _ZtlSecureTear_tThaw_[2];      unsigned int _ZtlSecureTear_tThaw__CS;     // 0x5D0
    // bit30 Weaken (Weakness) @0x5DC (0x30, 4)
    int _ZtlSecureTear_nWeakness_[2];  unsigned int _ZtlSecureTear_nWeakness__CS; // 0x5DC
    int _ZtlSecureTear_rWeakness_[2];  unsigned int _ZtlSecureTear_rWeakness__CS; // 0x5E8
    int _ZtlSecureTear_tWeakness_[2];  unsigned int _ZtlSecureTear_tWeakness__CS; // 0x5F4
    int _ZtlSecureTear_unk600_[2];     unsigned int _ZtlSecureTear_unk600__CS;    // 0x600 v61 Weakness 4th tear, name unresolved
    // bit31 Curse @0x60C (0x24, 3)
    int _ZtlSecureTear_nCurse_[2];     unsigned int _ZtlSecureTear_nCurse__CS;    // 0x60C
    int _ZtlSecureTear_rCurse_[2];     unsigned int _ZtlSecureTear_rCurse__CS;    // 0x618
    int _ZtlSecureTear_tCurse_[2];     unsigned int _ZtlSecureTear_tCurse__CS;    // 0x624
    // bit58 IncreaseEffectMpPotion @0x630 (0x24, 3) [ext-table inferred]
    int _ZtlSecureTear_nIncEffectMPPotion_[2]; unsigned int _ZtlSecureTear_nIncEffectMPPotion__CS; // 0x630
    int _ZtlSecureTear_rIncEffectMPPotion_[2]; unsigned int _ZtlSecureTear_rIncEffectMPPotion__CS; // 0x63C
    int _ZtlSecureTear_tIncEffectMPPotion_[2]; unsigned int _ZtlSecureTear_tIncEffectMPPotion__CS; // 0x648
    // bit32 Slow @0x654 (0x30, 4)
    int _ZtlSecureTear_nSlow_[2];      unsigned int _ZtlSecureTear_nSlow__CS;     // 0x654
    int _ZtlSecureTear_rSlow_[2];      unsigned int _ZtlSecureTear_rSlow__CS;     // 0x660
    int _ZtlSecureTear_tSlow_[2];      unsigned int _ZtlSecureTear_tSlow__CS;     // 0x66C
    int _ZtlSecureTear_unk678_[2];     unsigned int _ZtlSecureTear_unk678__CS;    // 0x678 v61 Slow 4th tear, name unresolved
    // bit33 Morph @0x684 (0x24, 3)
    int _ZtlSecureTear_nMorph_[2];     unsigned int _ZtlSecureTear_nMorph__CS;    // 0x684
    int _ZtlSecureTear_rMorph_[2];     unsigned int _ZtlSecureTear_rMorph__CS;    // 0x690
    int _ZtlSecureTear_tMorph_[2];     unsigned int _ZtlSecureTear_tMorph__CS;    // 0x69C
    // bit34 Recovery (Regen) @0x6A8 (0x24, 3)
    int _ZtlSecureTear_nRegen_[2];     unsigned int _ZtlSecureTear_nRegen__CS;    // 0x6A8
    int _ZtlSecureTear_rRegen_[2];     unsigned int _ZtlSecureTear_rRegen__CS;    // 0x6B4
    int _ZtlSecureTear_tRegen_[2];     unsigned int _ZtlSecureTear_tRegen__CS;    // 0x6C0
    // bit35 MapleWarrior (BasicStatUp) @0x6CC (0x24, 3)
    int _ZtlSecureTear_nBasicStatUp_[2]; unsigned int _ZtlSecureTear_nBasicStatUp__CS; // 0x6CC
    int _ZtlSecureTear_rBasicStatUp_[2]; unsigned int _ZtlSecureTear_rBasicStatUp__CS; // 0x6D8
    int _ZtlSecureTear_tBasicStatUp_[2]; unsigned int _ZtlSecureTear_tBasicStatUp__CS; // 0x6E4
    // bit36 Stance @0x6F0 (0x24, 3)
    int _ZtlSecureTear_nStance_[2];    unsigned int _ZtlSecureTear_nStance__CS;   // 0x6F0
    int _ZtlSecureTear_rStance_[2];    unsigned int _ZtlSecureTear_rStance__CS;   // 0x6FC
    int _ZtlSecureTear_tStance_[2];    unsigned int _ZtlSecureTear_tStance__CS;   // 0x708
    // bit37 SharpEyes @0x714 (0x24, 3)
    int _ZtlSecureTear_nSharpEyes_[2]; unsigned int _ZtlSecureTear_nSharpEyes__CS; // 0x714
    int _ZtlSecureTear_rSharpEyes_[2]; unsigned int _ZtlSecureTear_rSharpEyes__CS; // 0x720
    int _ZtlSecureTear_tSharpEyes_[2]; unsigned int _ZtlSecureTear_tSharpEyes__CS; // 0x72C
    // bit38 ManaReflection @0x738 (0x30, 4)
    int _ZtlSecureTear_nManaReflection_[2]; unsigned int _ZtlSecureTear_nManaReflection__CS; // 0x738
    int _ZtlSecureTear_rManaReflection_[2]; unsigned int _ZtlSecureTear_rManaReflection__CS; // 0x744
    int _ZtlSecureTear_tManaReflection_[2]; unsigned int _ZtlSecureTear_tManaReflection__CS; // 0x750
    int _ZtlSecureTear_unk75C_[2];     unsigned int _ZtlSecureTear_unk75C__CS;    // 0x75C v61 ManaReflection 4th tear, name unresolved
    // bit39 Seduce (Attract) @0x768 (0x24, 3)
    int _ZtlSecureTear_nAttract_[2];   unsigned int _ZtlSecureTear_nAttract__CS;  // 0x768
    int _ZtlSecureTear_rAttract_[2];   unsigned int _ZtlSecureTear_rAttract__CS;  // 0x774
    int _ZtlSecureTear_tAttract_[2];   unsigned int _ZtlSecureTear_tAttract__CS;  // 0x780
    // bit40 ShadowClaw (SpiritJavelin) @0x78C (0x24, 3) — SITE A (v72 has 6 tears here)
    int _ZtlSecureTear_nSpiritJavelin_[2]; unsigned int _ZtlSecureTear_nSpiritJavelin__CS; // 0x78C
    int _ZtlSecureTear_rSpiritJavelin_[2]; unsigned int _ZtlSecureTear_rSpiritJavelin__CS; // 0x798
    int _ZtlSecureTear_tSpiritJavelin_[2]; unsigned int _ZtlSecureTear_tSpiritJavelin__CS; // 0x7A4
    // bit41 Infinity @0x7B0 (0x30, 4) — SITE B (v72 has 10 tears here)
    int _ZtlSecureTear_nInfinity_[2];  unsigned int _ZtlSecureTear_nInfinity__CS; // 0x7B0
    int _ZtlSecureTear_rInfinity_[2];  unsigned int _ZtlSecureTear_rInfinity__CS; // 0x7BC
    int _ZtlSecureTear_tInfinity_[2];  unsigned int _ZtlSecureTear_tInfinity__CS; // 0x7C8
    int _ZtlSecureTear_tUpdateInfinity_[2]; unsigned int _ZtlSecureTear_tUpdateInfinity__CS; // 0x7D4
    // bit42 HolyShield @0x7E0 (0x30, 4)
    int _ZtlSecureTear_nHolyshield_[2]; unsigned int _ZtlSecureTear_nHolyshield__CS; // 0x7E0
    int _ZtlSecureTear_rHolyshield_[2]; unsigned int _ZtlSecureTear_rHolyshield__CS; // 0x7EC
    int _ZtlSecureTear_tHolyshield_[2]; unsigned int _ZtlSecureTear_tHolyshield__CS; // 0x7F8
    int _ZtlSecureTear_unk804_[2];     unsigned int _ZtlSecureTear_unk804__CS;    // 0x804 v61 HolyShield 4th tear, name unresolved
    // bit43 Hamstring @0x810 (0x3C, 5)
    int _ZtlSecureTear_nHamString_[2]; unsigned int _ZtlSecureTear_nHamString__CS; // 0x810
    int _ZtlSecureTear_rHamString_[2]; unsigned int _ZtlSecureTear_rHamString__CS; // 0x81C
    int _ZtlSecureTear_tHamString_[2]; unsigned int _ZtlSecureTear_tHamString__CS; // 0x828
    int _ZtlSecureTear_unk834_[2];     unsigned int _ZtlSecureTear_unk834__CS;    // 0x834 v61 Hamstring 4th tear, name unresolved
    int _ZtlSecureTear_unk840_[2];     unsigned int _ZtlSecureTear_unk840__CS;    // 0x840 v61 Hamstring 5th tear, name unresolved
    // bit44 Blind @0x84C (0x30, 4)
    int _ZtlSecureTear_nBlind_[2];     unsigned int _ZtlSecureTear_nBlind__CS;    // 0x84C
    int _ZtlSecureTear_rBlind_[2];     unsigned int _ZtlSecureTear_rBlind__CS;    // 0x858
    int _ZtlSecureTear_tBlind_[2];     unsigned int _ZtlSecureTear_tBlind__CS;    // 0x864
    int _ZtlSecureTear_unk870_[2];     unsigned int _ZtlSecureTear_unk870__CS;    // 0x870 v61 Blind 4th tear, name unresolved
    // bit45 Concentrate @0x87C (0x2C, 3 long-tears + 1 byte-tear)
    int _ZtlSecureTear_nConcentration_[2]; unsigned int _ZtlSecureTear_nConcentration__CS; // 0x87C
    int _ZtlSecureTear_rConcentration_[2]; unsigned int _ZtlSecureTear_rConcentration__CS; // 0x888
    int _ZtlSecureTear_tConcentration_[2]; unsigned int _ZtlSecureTear_tConcentration__CS; // 0x894
    char _ZtlSecureTear_unk8A0_[2];    unsigned int _ZtlSecureTear_unk8A0__CS;    // 0x8A0 v61 Concentrate trailing byte-tear (8B), name unresolved
    // bit46 BanMap @0x8A8 (0x2C, 3 long-tears + 1 byte-tear)
    int _ZtlSecureTear_nBanMap_[2];    unsigned int _ZtlSecureTear_nBanMap__CS;   // 0x8A8
    int _ZtlSecureTear_rBanMap_[2];    unsigned int _ZtlSecureTear_rBanMap__CS;   // 0x8B4
    int _ZtlSecureTear_tBanMap_[2];    unsigned int _ZtlSecureTear_tBanMap__CS;   // 0x8C0
    char _ZtlSecureTear_mBanMap_[2];   unsigned int _ZtlSecureTear_mBanMap__CS;   // 0x8CC v61 BanMap trailing byte-tear (8B); name = over-model mBanMap (inferred)
    // bit47 EchoOfHero (MaxLevelBuff) @0x8D4 (0x30, 4)
    int _ZtlSecureTear_nMaxLevelBuff_[2]; unsigned int _ZtlSecureTear_nMaxLevelBuff__CS; // 0x8D4
    int _ZtlSecureTear_rMaxLevelBuff_[2]; unsigned int _ZtlSecureTear_rMaxLevelBuff__CS; // 0x8E0
    int _ZtlSecureTear_tMaxLevelBuff_[2]; unsigned int _ZtlSecureTear_tMaxLevelBuff__CS; // 0x8EC
    int _ZtlSecureTear_unk8F8_[2];     unsigned int _ZtlSecureTear_unk8F8__CS;    // 0x8F8 v61 MaxLevelBuff 4th tear, name unresolved
    // bit48 MesoUpByItem @0x904 (0x30, 4)
    int _ZtlSecureTear_nMesoUpByItem[2];  unsigned int _ZtlSecureTear_nMesoUpByItem_CS;  // 0x904
    int _ZtlSecureTear_nMesoUpByItem_[2]; unsigned int _ZtlSecureTear_nMesoUpByItem__CS; // 0x910
    int _ZtlSecureTear_rMesoUpByItem_[2]; unsigned int _ZtlSecureTear_rMesoUpByItem__CS; // 0x91C
    int _ZtlSecureTear_tMesoUpByItem_[2]; unsigned int _ZtlSecureTear_tMesoUpByItem__CS; // 0x928
    // bit49 GhostMorph (Ghost) @0x934 (0x24, 3) — SITE C (v72 has 0xDC trailing here)
    int _ZtlSecureTear_nGhost_[2];     unsigned int _ZtlSecureTear_nGhost__CS;    // 0x934
    int _ZtlSecureTear_rGhost_[2];     unsigned int _ZtlSecureTear_rGhost__CS;    // 0x940
    int _ZtlSecureTear_tGhost_[2];     unsigned int _ZtlSecureTear_tGhost__CS;    // 0x94C
    // SITE D — trailing two-state base-stat array: v61 = 6 entries x 4B = 0x18 @0x958..0x970.
    // (v72 = 7 x 8B = 0x38.) 4-byte entry on 32-bit ⇒ raw pointer, not the 8-byte ZRef.
    TemporaryStatBase<long>* aTemporaryStat[6];                                   // 0x958..0x970
#else
    int _ZtlSecureTear_nPAD[2];
    unsigned int _ZtlSecureTear_nPAD_CS;
    int _ZtlSecureTear_nPAD_[2];
    unsigned int _ZtlSecureTear_nPAD__CS;
    int _ZtlSecureTear_rPAD_[2];
    unsigned int _ZtlSecureTear_rPAD__CS;
    int _ZtlSecureTear_tPAD_[2];
    unsigned int _ZtlSecureTear_tPAD__CS;
    int _ZtlSecureTear_nItemPADR[2];
    unsigned int _ZtlSecureTear_nItemPADR_CS;
    int _ZtlSecureTear_nPDD[2];
    unsigned int _ZtlSecureTear_nPDD_CS;
    int _ZtlSecureTear_nPDD_[2];
    unsigned int _ZtlSecureTear_nPDD__CS;
    int _ZtlSecureTear_rPDD_[2];
    unsigned int _ZtlSecureTear_rPDD__CS;
    int _ZtlSecureTear_tPDD_[2];
    unsigned int _ZtlSecureTear_tPDD__CS;
    int _ZtlSecureTear_nItemPDDR[2];
    unsigned int _ZtlSecureTear_nItemPDDR_CS;
    int _ZtlSecureTear_nMAD[2];
    unsigned int _ZtlSecureTear_nMAD_CS;
    int _ZtlSecureTear_nMAD_[2];
    unsigned int _ZtlSecureTear_nMAD__CS;
    int _ZtlSecureTear_rMAD_[2];
    unsigned int _ZtlSecureTear_rMAD__CS;
    int _ZtlSecureTear_tMAD_[2];
    unsigned int _ZtlSecureTear_tMAD__CS;
    int _ZtlSecureTear_nItemMADR[2];
    unsigned int _ZtlSecureTear_nItemMADR_CS;
    int _ZtlSecureTear_nMDD[2];
    unsigned int _ZtlSecureTear_nMDD_CS;
    int _ZtlSecureTear_nMDD_[2];
    unsigned int _ZtlSecureTear_nMDD__CS;
    int _ZtlSecureTear_rMDD_[2];
    unsigned int _ZtlSecureTear_rMDD__CS;
    int _ZtlSecureTear_tMDD_[2];
    unsigned int _ZtlSecureTear_tMDD__CS;
    int _ZtlSecureTear_nItemMDDR[2];
    unsigned int _ZtlSecureTear_nItemMDDR_CS;
    int _ZtlSecureTear_nACC[2];
    unsigned int _ZtlSecureTear_nACC_CS;
    int _ZtlSecureTear_nACC_[2];
    unsigned int _ZtlSecureTear_nACC__CS;
    int _ZtlSecureTear_rACC_[2];
    unsigned int _ZtlSecureTear_rACC__CS;
    int _ZtlSecureTear_tACC_[2];
    unsigned int _ZtlSecureTear_tACC__CS;
    int _ZtlSecureTear_nItemACCR[2];
    unsigned int _ZtlSecureTear_nItemACCR_CS;
    int _ZtlSecureTear_nEVA[2];
    unsigned int _ZtlSecureTear_nEVA_CS;
    int _ZtlSecureTear_nEVA_[2];
    unsigned int _ZtlSecureTear_nEVA__CS;
    int _ZtlSecureTear_rEVA_[2];
    unsigned int _ZtlSecureTear_rEVA__CS;
    int _ZtlSecureTear_tEVA_[2];
    unsigned int _ZtlSecureTear_tEVA__CS;
    int _ZtlSecureTear_nItemEVAR[2];
    unsigned int _ZtlSecureTear_nItemEVAR_CS;
    int _ZtlSecureTear_nCraft[2];
    unsigned int _ZtlSecureTear_nCraft_CS;
    int _ZtlSecureTear_nCraft_[2];
    unsigned int _ZtlSecureTear_nCraft__CS;
    int _ZtlSecureTear_rCraft_[2];
    unsigned int _ZtlSecureTear_rCraft__CS;
    int _ZtlSecureTear_tCraft_[2];
    unsigned int _ZtlSecureTear_tCraft__CS;
    int _ZtlSecureTear_nSpeed[2];
    unsigned int _ZtlSecureTear_nSpeed_CS;
    int _ZtlSecureTear_nSpeed_[2];
    unsigned int _ZtlSecureTear_nSpeed__CS;
    int _ZtlSecureTear_rSpeed_[2];
    unsigned int _ZtlSecureTear_rSpeed__CS;
    int _ZtlSecureTear_tSpeed_[2];
    unsigned int _ZtlSecureTear_tSpeed__CS;
    int _ZtlSecureTear_nJump[2];
    unsigned int _ZtlSecureTear_nJump_CS;
    int _ZtlSecureTear_nJump_[2];
    unsigned int _ZtlSecureTear_nJump__CS;
    int _ZtlSecureTear_rJump_[2];
    unsigned int _ZtlSecureTear_rJump__CS;
    int _ZtlSecureTear_tJump_[2];
    unsigned int _ZtlSecureTear_tJump__CS;
    int _ZtlSecureTear_nMagicGuard_[2];
    unsigned int _ZtlSecureTear_nMagicGuard__CS;
    int _ZtlSecureTear_rMagicGuard_[2];
    unsigned int _ZtlSecureTear_rMagicGuard__CS;
    int _ZtlSecureTear_tMagicGuard_[2];
    unsigned int _ZtlSecureTear_tMagicGuard__CS;
    int _ZtlSecureTear_nDarkSight_[2];
    unsigned int _ZtlSecureTear_nDarkSight__CS;
    int _ZtlSecureTear_rDarkSight_[2];
    unsigned int _ZtlSecureTear_rDarkSight__CS;
    int _ZtlSecureTear_tDarkSight_[2];
    unsigned int _ZtlSecureTear_tDarkSight__CS;
    int _ZtlSecureTear_mDarkSight_[2];
    unsigned int _ZtlSecureTear_mDarkSight__CS;
    int _ZtlSecureTear_pDarkSight_[2];
    unsigned int _ZtlSecureTear_pDarkSight__CS;
    int _ZtlSecureTear_nBooster_[2];
    unsigned int _ZtlSecureTear_nBooster__CS;
    int _ZtlSecureTear_rBooster_[2];
    unsigned int _ZtlSecureTear_rBooster__CS;
    int _ZtlSecureTear_tBooster_[2];
    unsigned int _ZtlSecureTear_tBooster__CS;
    int _ZtlSecureTear_nPowerGuard_[2];
    unsigned int _ZtlSecureTear_nPowerGuard__CS;
    int _ZtlSecureTear_rPowerGuard_[2];
    unsigned int _ZtlSecureTear_rPowerGuard__CS;
    int _ZtlSecureTear_tPowerGuard_[2];
    unsigned int _ZtlSecureTear_tPowerGuard__CS;
    int _ZtlSecureTear_nMechanic_[2];
    unsigned int _ZtlSecureTear_nMechanic__CS;
    int _ZtlSecureTear_rMechanic_[2];
    unsigned int _ZtlSecureTear_rMechanic__CS;
    int _ZtlSecureTear_tMechanic_[2];
    unsigned int _ZtlSecureTear_tMechanic__CS;
    int _ZtlSecureTear_nMaxHP_[2];
    unsigned int _ZtlSecureTear_nMaxHP__CS;
    int _ZtlSecureTear_rMaxHP_[2];
    unsigned int _ZtlSecureTear_rMaxHP__CS;
    int _ZtlSecureTear_tMaxHP_[2];
    unsigned int _ZtlSecureTear_tMaxHP__CS;
    int _ZtlSecureTear_nMaxMP_[2];
    unsigned int _ZtlSecureTear_nMaxMP__CS;
    int _ZtlSecureTear_rMaxMP_[2];
    unsigned int _ZtlSecureTear_rMaxMP__CS;
    int _ZtlSecureTear_tMaxMP_[2];
    unsigned int _ZtlSecureTear_tMaxMP__CS;
    int _ZtlSecureTear_nInvincible_[2];
    unsigned int _ZtlSecureTear_nInvincible__CS;
    int _ZtlSecureTear_rInvincible_[2];
    unsigned int _ZtlSecureTear_rInvincible__CS;
    int _ZtlSecureTear_tInvincible_[2];
    unsigned int _ZtlSecureTear_tInvincible__CS;
    int _ZtlSecureTear_nSoulArrow_[2];
    unsigned int _ZtlSecureTear_nSoulArrow__CS;
    int _ZtlSecureTear_rSoulArrow_[2];
    unsigned int _ZtlSecureTear_rSoulArrow__CS;
    int _ZtlSecureTear_tSoulArrow_[2];
    unsigned int _ZtlSecureTear_tSoulArrow__CS;
    int _ZtlSecureTear_nStun_[2];
    unsigned int _ZtlSecureTear_nStun__CS;
    int _ZtlSecureTear_rStun_[2];
    unsigned int _ZtlSecureTear_rStun__CS;
    int _ZtlSecureTear_tStun_[2];
    unsigned int _ZtlSecureTear_tStun__CS;
    int _ZtlSecureTear_nPoison_[2];
    unsigned int _ZtlSecureTear_nPoison__CS;
    int _ZtlSecureTear_rPoison_[2];
    unsigned int _ZtlSecureTear_rPoison__CS;
    int _ZtlSecureTear_tPoison_[2];
    unsigned int _ZtlSecureTear_tPoison__CS;
    int _ZtlSecureTear_nSeal_[2];
    unsigned int _ZtlSecureTear_nSeal__CS;
    int _ZtlSecureTear_rSeal_[2];
    unsigned int _ZtlSecureTear_rSeal__CS;
    int _ZtlSecureTear_tSeal_[2];
    unsigned int _ZtlSecureTear_tSeal__CS;
    int _ZtlSecureTear_nDarkness_[2];
    unsigned int _ZtlSecureTear_nDarkness__CS;
    int _ZtlSecureTear_rDarkness_[2];
    unsigned int _ZtlSecureTear_rDarkness__CS;
    int _ZtlSecureTear_tDarkness_[2];
    unsigned int _ZtlSecureTear_tDarkness__CS;
    int _ZtlSecureTear_nComboCounter_[2];
    unsigned int _ZtlSecureTear_nComboCounter__CS;
    int _ZtlSecureTear_rComboCounter_[2];
    unsigned int _ZtlSecureTear_rComboCounter__CS;
    int _ZtlSecureTear_tComboCounter_[2];
    unsigned int _ZtlSecureTear_tComboCounter__CS;
    int _ZtlSecureTear_mComboCounter_[2];
    unsigned int _ZtlSecureTear_mComboCounter__CS;
    int _ZtlSecureTear_nWeaponCharge_[2];
    unsigned int _ZtlSecureTear_nWeaponCharge__CS;
    int _ZtlSecureTear_rWeaponCharge_[2];
    unsigned int _ZtlSecureTear_rWeaponCharge__CS;
    int _ZtlSecureTear_tWeaponCharge_[2];
    unsigned int _ZtlSecureTear_tWeaponCharge__CS;
    int _ZtlSecureTear_nDragonBlood_[2];
    unsigned int _ZtlSecureTear_nDragonBlood__CS;
    int _ZtlSecureTear_rDragonBlood_[2];
    unsigned int _ZtlSecureTear_rDragonBlood__CS;
    int _ZtlSecureTear_tDragonBlood_[2];
    unsigned int _ZtlSecureTear_tDragonBlood__CS;
    int _ZtlSecureTear_nHolySymbol_[2];
    unsigned int _ZtlSecureTear_nHolySymbol__CS;
    int _ZtlSecureTear_rHolySymbol_[2];
    unsigned int _ZtlSecureTear_rHolySymbol__CS;
    int _ZtlSecureTear_tHolySymbol_[2];
    unsigned int _ZtlSecureTear_tHolySymbol__CS;
    int _ZtlSecureTear_nMesoUp_[2];
    unsigned int _ZtlSecureTear_nMesoUp__CS;
    int _ZtlSecureTear_rMesoUp_[2];
    unsigned int _ZtlSecureTear_rMesoUp__CS;
    int _ZtlSecureTear_tMesoUp_[2];
    unsigned int _ZtlSecureTear_tMesoUp__CS;
    int _ZtlSecureTear_nShadowPartner_[2];
    unsigned int _ZtlSecureTear_nShadowPartner__CS;
    int _ZtlSecureTear_rShadowPartner_[2];
    unsigned int _ZtlSecureTear_rShadowPartner__CS;
    int _ZtlSecureTear_tShadowPartner_[2];
    unsigned int _ZtlSecureTear_tShadowPartner__CS;
    int _ZtlSecureTear_nPickPocket_[2];
    unsigned int _ZtlSecureTear_nPickPocket__CS;
    int _ZtlSecureTear_rPickPocket_[2];
    unsigned int _ZtlSecureTear_rPickPocket__CS;
    int _ZtlSecureTear_tPickPocket_[2];
    unsigned int _ZtlSecureTear_tPickPocket__CS;
    int _ZtlSecureTear_nMesoGuard_[2];
    unsigned int _ZtlSecureTear_nMesoGuard__CS;
    int _ZtlSecureTear_rMesoGuard_[2];
    unsigned int _ZtlSecureTear_rMesoGuard__CS;
    int _ZtlSecureTear_tMesoGuard_[2];
    unsigned int _ZtlSecureTear_tMesoGuard__CS;
    int _ZtlSecureTear_nThaw_[2];
    unsigned int _ZtlSecureTear_nThaw__CS;
    int _ZtlSecureTear_rThaw_[2];
    unsigned int _ZtlSecureTear_rThaw__CS;
    int _ZtlSecureTear_tThaw_[2];
    unsigned int _ZtlSecureTear_tThaw__CS;
    int _ZtlSecureTear_nWeakness_[2];
    unsigned int _ZtlSecureTear_nWeakness__CS;
    int _ZtlSecureTear_rWeakness_[2];
    unsigned int _ZtlSecureTear_rWeakness__CS;
    int _ZtlSecureTear_tWeakness_[2];
    unsigned int _ZtlSecureTear_tWeakness__CS;
    int _ZtlSecureTear_nCurse_[2];
    unsigned int _ZtlSecureTear_nCurse__CS;
    int _ZtlSecureTear_rCurse_[2];
    unsigned int _ZtlSecureTear_rCurse__CS;
    int _ZtlSecureTear_tCurse_[2];
    unsigned int _ZtlSecureTear_tCurse__CS;
    int _ZtlSecureTear_nSlow_[2];
    unsigned int _ZtlSecureTear_nSlow__CS;
    int _ZtlSecureTear_rSlow_[2];
    unsigned int _ZtlSecureTear_rSlow__CS;
    int _ZtlSecureTear_tSlow_[2];
    unsigned int _ZtlSecureTear_tSlow__CS;
    int _ZtlSecureTear_nMorph_[2];
    unsigned int _ZtlSecureTear_nMorph__CS;
    int _ZtlSecureTear_rMorph_[2];
    unsigned int _ZtlSecureTear_rMorph__CS;
    int _ZtlSecureTear_tMorph_[2];
    unsigned int _ZtlSecureTear_tMorph__CS;
    int _ZtlSecureTear_nGhost_[2];
    unsigned int _ZtlSecureTear_nGhost__CS;
    int _ZtlSecureTear_rGhost_[2];
    unsigned int _ZtlSecureTear_rGhost__CS;
    int _ZtlSecureTear_tGhost_[2];
    unsigned int _ZtlSecureTear_tGhost__CS;
    int _ZtlSecureTear_nRegen_[2];
    unsigned int _ZtlSecureTear_nRegen__CS;
    int _ZtlSecureTear_rRegen_[2];
    unsigned int _ZtlSecureTear_rRegen__CS;
    int _ZtlSecureTear_tRegen_[2];
    unsigned int _ZtlSecureTear_tRegen__CS;
    int _ZtlSecureTear_nBasicStatUp_[2];
    unsigned int _ZtlSecureTear_nBasicStatUp__CS;
    int _ZtlSecureTear_rBasicStatUp_[2];
    unsigned int _ZtlSecureTear_rBasicStatUp__CS;
    int _ZtlSecureTear_tBasicStatUp_[2];
    unsigned int _ZtlSecureTear_tBasicStatUp__CS;
    int _ZtlSecureTear_nStance_[2];
    unsigned int _ZtlSecureTear_nStance__CS;
    int _ZtlSecureTear_rStance_[2];
    unsigned int _ZtlSecureTear_rStance__CS;
    int _ZtlSecureTear_tStance_[2];
    unsigned int _ZtlSecureTear_tStance__CS;
    int _ZtlSecureTear_nSharpEyes_[2];
    unsigned int _ZtlSecureTear_nSharpEyes__CS;
    int _ZtlSecureTear_rSharpEyes_[2];
    unsigned int _ZtlSecureTear_rSharpEyes__CS;
    int _ZtlSecureTear_tSharpEyes_[2];
    unsigned int _ZtlSecureTear_tSharpEyes__CS;
    int _ZtlSecureTear_mSharpEyes_[2];
    unsigned int _ZtlSecureTear_mSharpEyes__CS;
    int _ZtlSecureTear_nManaReflection_[2];
    unsigned int _ZtlSecureTear_nManaReflection__CS;
    int _ZtlSecureTear_rManaReflection_[2];
    unsigned int _ZtlSecureTear_rManaReflection__CS;
    int _ZtlSecureTear_tManaReflection_[2];
    unsigned int _ZtlSecureTear_tManaReflection__CS;
    int _ZtlSecureTear_nAttract_[2];
    unsigned int _ZtlSecureTear_nAttract__CS;
    int _ZtlSecureTear_rAttract_[2];
    unsigned int _ZtlSecureTear_rAttract__CS;
    int _ZtlSecureTear_tAttract_[2];
    unsigned int _ZtlSecureTear_tAttract__CS;
    int _ZtlSecureTear_nSpiritJavelin_[2];
    unsigned int _ZtlSecureTear_nSpiritJavelin__CS;
    int _ZtlSecureTear_rSpiritJavelin_[2];
    unsigned int _ZtlSecureTear_rSpiritJavelin__CS;
    int _ZtlSecureTear_tSpiritJavelin_[2];
    unsigned int _ZtlSecureTear_tSpiritJavelin__CS;
    int _ZtlSecureTear_mSpiritJavelin_[2];
    unsigned int _ZtlSecureTear_mSpiritJavelin__CS;
    int _ZtlSecureTear_nInfinity_[2];
    unsigned int _ZtlSecureTear_nInfinity__CS;
    int _ZtlSecureTear_rInfinity_[2];
    unsigned int _ZtlSecureTear_rInfinity__CS;
    int _ZtlSecureTear_tInfinity_[2];
    unsigned int _ZtlSecureTear_tInfinity__CS;
    int _ZtlSecureTear_tUpdateInfinity_[2];
    unsigned int _ZtlSecureTear_tUpdateInfinity__CS;
    int _ZtlSecureTear_nHolyshield_[2];
    unsigned int _ZtlSecureTear_nHolyshield__CS;
    int _ZtlSecureTear_rHolyshield_[2];
    unsigned int _ZtlSecureTear_rHolyshield__CS;
    int _ZtlSecureTear_tHolyshield_[2];
    unsigned int _ZtlSecureTear_tHolyshield__CS;
    int _ZtlSecureTear_nHamString_[2];
    unsigned int _ZtlSecureTear_nHamString__CS;
    int _ZtlSecureTear_rHamString_[2];
    unsigned int _ZtlSecureTear_rHamString__CS;
    int _ZtlSecureTear_tHamString_[2];
    unsigned int _ZtlSecureTear_tHamString__CS;
    int _ZtlSecureTear_nBlind_[2];
    unsigned int _ZtlSecureTear_nBlind__CS;
    int _ZtlSecureTear_rBlind_[2];
    unsigned int _ZtlSecureTear_rBlind__CS;
    int _ZtlSecureTear_tBlind_[2];
    unsigned int _ZtlSecureTear_tBlind__CS;
    int _ZtlSecureTear_nConcentration_[2];
    unsigned int _ZtlSecureTear_nConcentration__CS;
    int _ZtlSecureTear_rConcentration_[2];
    unsigned int _ZtlSecureTear_rConcentration__CS;
    int _ZtlSecureTear_tConcentration_[2];
    unsigned int _ZtlSecureTear_tConcentration__CS;
    int _ZtlSecureTear_nBanMap_[2];
    unsigned int _ZtlSecureTear_nBanMap__CS;
    int _ZtlSecureTear_rBanMap_[2];
    unsigned int _ZtlSecureTear_rBanMap__CS;
    int _ZtlSecureTear_tBanMap_[2];
    unsigned int _ZtlSecureTear_tBanMap__CS;
    int _ZtlSecureTear_mBanMap_[2];
    unsigned int _ZtlSecureTear_mBanMap__CS;
    int _ZtlSecureTear_nMaxLevelBuff_[2];
    unsigned int _ZtlSecureTear_nMaxLevelBuff__CS;
    int _ZtlSecureTear_rMaxLevelBuff_[2];
    unsigned int _ZtlSecureTear_rMaxLevelBuff__CS;
    int _ZtlSecureTear_tMaxLevelBuff_[2];
    unsigned int _ZtlSecureTear_tMaxLevelBuff__CS;
    int _ZtlSecureTear_nAura_[2];
    unsigned int _ZtlSecureTear_nAura__CS;
    int _ZtlSecureTear_rAura_[2];
    unsigned int _ZtlSecureTear_rAura__CS;
    int _ZtlSecureTear_tAura_[2];
    unsigned int _ZtlSecureTear_tAura__CS;
    int _ZtlSecureTear_tUpdateAura_[2];
    unsigned int _ZtlSecureTear_tUpdateAura__CS;
    int _ZtlSecureTear_nSuperBody_[2];
    unsigned int _ZtlSecureTear_nSuperBody__CS;
    int _ZtlSecureTear_rSuperBody_[2];
    unsigned int _ZtlSecureTear_rSuperBody__CS;
    int _ZtlSecureTear_tSuperBody_[2];
    unsigned int _ZtlSecureTear_tSuperBody__CS;
    int _ZtlSecureTear_nDarkAura_[2];
    unsigned int _ZtlSecureTear_nDarkAura__CS;
    int _ZtlSecureTear_rDarkAura_[2];
    unsigned int _ZtlSecureTear_rDarkAura__CS;
    int _ZtlSecureTear_tDarkAura_[2];
    unsigned int _ZtlSecureTear_tDarkAura__CS;
    int _ZtlSecureTear_cDarkAura_[2];
    unsigned int _ZtlSecureTear_cDarkAura__CS;
    int _ZtlSecureTear_nBlueAura_[2];
    unsigned int _ZtlSecureTear_nBlueAura__CS;
    int _ZtlSecureTear_rBlueAura_[2];
    unsigned int _ZtlSecureTear_rBlueAura__CS;
    int _ZtlSecureTear_tBlueAura_[2];
    unsigned int _ZtlSecureTear_tBlueAura__CS;
    int _ZtlSecureTear_cBlueAura_[2];
    unsigned int _ZtlSecureTear_cBlueAura__CS;
    int _ZtlSecureTear_nYellowAura_[2];
    unsigned int _ZtlSecureTear_nYellowAura__CS;
    int _ZtlSecureTear_rYellowAura_[2];
    unsigned int _ZtlSecureTear_rYellowAura__CS;
    int _ZtlSecureTear_tYellowAura_[2];
    unsigned int _ZtlSecureTear_tYellowAura__CS;
    int _ZtlSecureTear_cYellowAura_[2];
    unsigned int _ZtlSecureTear_cYellowAura__CS;
    int _ZtlSecureTear_nBarrier_[2];
    unsigned int _ZtlSecureTear_nBarrier__CS;
    int _ZtlSecureTear_tBarrier_[2];
    unsigned int _ZtlSecureTear_tBarrier__CS;
    int _ZtlSecureTear_rBarrier_[2];
    unsigned int _ZtlSecureTear_rBarrier__CS;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 87) || defined(REGION_JMS)
    int _ZtlSecureTear_nDojangShield_[2];
    unsigned int _ZtlSecureTear_nDojangShield__CS;
    int _ZtlSecureTear_tDojangShield_[2];
    unsigned int _ZtlSecureTear_tDojangShield__CS;
    int _ZtlSecureTear_rDojangShield_[2];
    unsigned int _ZtlSecureTear_rDojangShield__CS;
#endif
    // v87 (GMS) and v185 (JMS) both encode this as a SecureTear<byte> (8B slot
    // = 2B payload + 2B pad + 4B CS), verified via disassembly. v95 GMS uses the
    // standard SecureTear<long> 12B slot. v83 layout is structurally different
    // in this region and unconfirmed in detail; falls into the 12B branch.
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION == 87) || defined(REGION_JMS)
    char _ZtlSecureTear_nReverseInput_[2];
    unsigned int _ZtlSecureTear_nReverseInput__CS;
#else
    int _ZtlSecureTear_nReverseInput_[2];
    unsigned int _ZtlSecureTear_nReverseInput__CS;
#endif
    int _ZtlSecureTear_rReverseInput_[2];
    unsigned int _ZtlSecureTear_rReverseInput__CS;
    int _ZtlSecureTear_tReverseInput_[2];
    unsigned int _ZtlSecureTear_tReverseInput__CS;
    int _ZtlSecureTear_nDojangBerserk_[2];
    unsigned int _ZtlSecureTear_nDojangBerserk__CS;
    // Same byte-vs-long delta as nReverseInput_ above.
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION == 87) || defined(REGION_JMS)
    char _ZtlSecureTear_rDojangBerserk_[2];
    unsigned int _ZtlSecureTear_rDojangBerserk__CS;
#else
    int _ZtlSecureTear_rDojangBerserk_[2];
    unsigned int _ZtlSecureTear_rDojangBerserk__CS;
#endif
    int _ZtlSecureTear_tDojangBerserk_[2];
    unsigned int _ZtlSecureTear_tDojangBerserk__CS;
    int _ZtlSecureTear_nDojangInvincible_[2];
    unsigned int _ZtlSecureTear_nDojangInvincible__CS;
    int _ZtlSecureTear_rDojangInvincible_[2];
    unsigned int _ZtlSecureTear_rDojangInvincible__CS;
    int _ZtlSecureTear_tDojangInvincible_[2];
    unsigned int _ZtlSecureTear_tDojangInvincible__CS;
    int _ZtlSecureTear_nMesoUpByItem[2];
    unsigned int _ZtlSecureTear_nMesoUpByItem_CS;
    int _ZtlSecureTear_nMesoUpByItem_[2];
    unsigned int _ZtlSecureTear_nMesoUpByItem__CS;
    int _ZtlSecureTear_rMesoUpByItem_[2];
    unsigned int _ZtlSecureTear_rMesoUpByItem__CS;
    int _ZtlSecureTear_tMesoUpByItem_[2];
    unsigned int _ZtlSecureTear_tMesoUpByItem__CS;
    int _ZtlSecureTear_nItemUpByItem[2];
    unsigned int _ZtlSecureTear_nItemUpByItem_CS;
    int _ZtlSecureTear_nItemUpByItem_[2];
    unsigned int _ZtlSecureTear_nItemUpByItem__CS;
    int _ZtlSecureTear_rItemUpByItem_[2];
    unsigned int _ZtlSecureTear_rItemUpByItem__CS;
    int _ZtlSecureTear_tItemUpByItem_[2];
    unsigned int _ZtlSecureTear_tItemUpByItem__CS;
    int _ZtlSecureTear_xItemUpByItem_[2];
    unsigned int _ZtlSecureTear_xItemUpByItem__CS;
    int _ZtlSecureTear_nRespectPImmune[2];
    unsigned int _ZtlSecureTear_nRespectPImmune_CS;
    int _ZtlSecureTear_nRespectPImmune_[2];
    unsigned int _ZtlSecureTear_nRespectPImmune__CS;
    int _ZtlSecureTear_rRespectPImmune_[2];
    unsigned int _ZtlSecureTear_rRespectPImmune__CS;
    int _ZtlSecureTear_tRespectPImmune_[2];
    unsigned int _ZtlSecureTear_tRespectPImmune__CS;
    int _ZtlSecureTear_nRespectMImmune[2];
    unsigned int _ZtlSecureTear_nRespectMImmune_CS;
    int _ZtlSecureTear_nRespectMImmune_[2];
    unsigned int _ZtlSecureTear_nRespectMImmune__CS;
    int _ZtlSecureTear_rRespectMImmune_[2];
    unsigned int _ZtlSecureTear_rRespectMImmune__CS;
    int _ZtlSecureTear_tRespectMImmune_[2];
    unsigned int _ZtlSecureTear_tRespectMImmune__CS;
    char _ZtlSecureTear_nDefenseAtt[2];
    unsigned int _ZtlSecureTear_nDefenseAtt_CS;
    int _ZtlSecureTear_nDefenseAtt_[2];
    unsigned int _ZtlSecureTear_nDefenseAtt__CS;
    int _ZtlSecureTear_rDefenseAtt_[2];
    unsigned int _ZtlSecureTear_rDefenseAtt__CS;
    int _ZtlSecureTear_tDefenseAtt_[2];
    unsigned int _ZtlSecureTear_tDefenseAtt__CS;
    char _ZtlSecureTear_nDefenseState[2];
    unsigned int _ZtlSecureTear_nDefenseState_CS;
    int _ZtlSecureTear_nDefenseState_[2];
    unsigned int _ZtlSecureTear_nDefenseState__CS;
    int _ZtlSecureTear_rDefenseState_[2];
    unsigned int _ZtlSecureTear_rDefenseState__CS;
    int _ZtlSecureTear_tDefenseState_[2];
    unsigned int _ZtlSecureTear_tDefenseState__CS;
    int _ZtlSecureTear_nIncEffectHPPotion[2];
    unsigned int _ZtlSecureTear_nIncEffectHPPotion_CS;
    int _ZtlSecureTear_nIncEffectHPPotion_[2];
    unsigned int _ZtlSecureTear_nIncEffectHPPotion__CS;
    int _ZtlSecureTear_rIncEffectHPPotion_[2];
    unsigned int _ZtlSecureTear_rIncEffectHPPotion__CS;
    int _ZtlSecureTear_tIncEffectHPPotion_[2];
    unsigned int _ZtlSecureTear_tIncEffectHPPotion__CS;
    int _ZtlSecureTear_nIncEffectMPPotion[2];
    unsigned int _ZtlSecureTear_nIncEffectMPPotion_CS;
    int _ZtlSecureTear_nIncEffectMPPotion_[2];
    unsigned int _ZtlSecureTear_nIncEffectMPPotion__CS;
    int _ZtlSecureTear_rIncEffectMPPotion_[2];
    unsigned int _ZtlSecureTear_rIncEffectMPPotion__CS;
    int _ZtlSecureTear_tIncEffectMPPotion_[2];
    unsigned int _ZtlSecureTear_tIncEffectMPPotion__CS;
    int _ZtlSecureTear_nSpark_[2];
    unsigned int _ZtlSecureTear_nSpark__CS;
    int _ZtlSecureTear_rSpark_[2];
    unsigned int _ZtlSecureTear_rSpark__CS;
    int _ZtlSecureTear_tSpark_[2];
    unsigned int _ZtlSecureTear_tSpark__CS;
    int _ZtlSecureTear_nSoulMasterFinal_[2];
    unsigned int _ZtlSecureTear_nSoulMasterFinal__CS;
    int _ZtlSecureTear_rSoulMasterFinal_[2];
    unsigned int _ZtlSecureTear_rSoulMasterFinal__CS;
    int _ZtlSecureTear_tSoulMasterFinal_[2];
    unsigned int _ZtlSecureTear_tSoulMasterFinal__CS;
    int _ZtlSecureTear_nWindBreakerFinal_[2];
    unsigned int _ZtlSecureTear_nWindBreakerFinal__CS;
    int _ZtlSecureTear_rWindBreakerFinal_[2];
    unsigned int _ZtlSecureTear_rWindBreakerFinal__CS;
    int _ZtlSecureTear_tWindBreakerFinal_[2];
    unsigned int _ZtlSecureTear_tWindBreakerFinal__CS;
    int _ZtlSecureTear_nElementalReset_[2];
    unsigned int _ZtlSecureTear_nElementalReset__CS;
    int _ZtlSecureTear_rElementalReset_[2];
    unsigned int _ZtlSecureTear_rElementalReset__CS;
    int _ZtlSecureTear_tElementalReset_[2];
    unsigned int _ZtlSecureTear_tElementalReset__CS;
    int _ZtlSecureTear_nWindWalk_[2];
    unsigned int _ZtlSecureTear_nWindWalk__CS;
    int _ZtlSecureTear_rWindWalk_[2];
    unsigned int _ZtlSecureTear_rWindWalk__CS;
    int _ZtlSecureTear_tWindWalk_[2];
    unsigned int _ZtlSecureTear_tWindWalk__CS;
    int _ZtlSecureTear_nEventRate_[2];
    unsigned int _ZtlSecureTear_nEventRate__CS;
    int _ZtlSecureTear_rEventRate_[2];
    unsigned int _ZtlSecureTear_rEventRate__CS;
    int _ZtlSecureTear_tEventRate_[2];
    unsigned int _ZtlSecureTear_tEventRate__CS;
    int _ZtlSecureTear_nComboAbilityBuff_[2];
    unsigned int _ZtlSecureTear_nComboAbilityBuff__CS;
    int _ZtlSecureTear_rComboAbilityBuff_[2];
    unsigned int _ZtlSecureTear_rComboAbilityBuff__CS;
    int _ZtlSecureTear_tComboAbilityBuff_[2];
    unsigned int _ZtlSecureTear_tComboAbilityBuff__CS;
    int _ZtlSecureTear_nComboDrain_[2];
    unsigned int _ZtlSecureTear_nComboDrain__CS;
    int _ZtlSecureTear_rComboDrain_[2];
    unsigned int _ZtlSecureTear_rComboDrain__CS;
    int _ZtlSecureTear_tComboDrain_[2];
    unsigned int _ZtlSecureTear_tComboDrain__CS;
    int _ZtlSecureTear_nComboBarrier_[2];
    unsigned int _ZtlSecureTear_nComboBarrier__CS;
    int _ZtlSecureTear_rComboBarrier_[2];
    unsigned int _ZtlSecureTear_rComboBarrier__CS;
    int _ZtlSecureTear_tComboBarrier_[2];
    unsigned int _ZtlSecureTear_tComboBarrier__CS;
    int _ZtlSecureTear_nBodyPressure_[2];
    unsigned int _ZtlSecureTear_nBodyPressure__CS;
    int _ZtlSecureTear_rBodyPressure_[2];
    unsigned int _ZtlSecureTear_rBodyPressure__CS;
    int _ZtlSecureTear_tBodyPressure_[2];
    unsigned int _ZtlSecureTear_tBodyPressure__CS;
    int _ZtlSecureTear_nSmartKnockback_[2];
    unsigned int _ZtlSecureTear_nSmartKnockback__CS;
    int _ZtlSecureTear_rSmartKnockback_[2];
    unsigned int _ZtlSecureTear_rSmartKnockback__CS;
    int _ZtlSecureTear_tSmartKnockback_[2];
    unsigned int _ZtlSecureTear_tSmartKnockback__CS;
    int _ZtlSecureTear_nRepeatEffect_[2];
    unsigned int _ZtlSecureTear_nRepeatEffect__CS;
    int _ZtlSecureTear_rRepeatEffect_[2];
    unsigned int _ZtlSecureTear_rRepeatEffect__CS;
    int _ZtlSecureTear_tRepeatEffect_[2];
    unsigned int _ZtlSecureTear_tRepeatEffect__CS;
    int _ZtlSecureTear_nExpBuffRate_[2];
    unsigned int _ZtlSecureTear_nExpBuffRate__CS;
    int _ZtlSecureTear_rExpBuffRate_[2];
    unsigned int _ZtlSecureTear_rExpBuffRate__CS;
    int _ZtlSecureTear_tExpBuffRate_[2];
    unsigned int _ZtlSecureTear_tExpBuffRate__CS;
    int _ZtlSecureTear_nStopPortion_[2];
    unsigned int _ZtlSecureTear_nStopPortion__CS;
    int _ZtlSecureTear_rStopPortion_[2];
    unsigned int _ZtlSecureTear_rStopPortion__CS;
    int _ZtlSecureTear_tStopPortion_[2];
    unsigned int _ZtlSecureTear_tStopPortion__CS;
    int _ZtlSecureTear_nStopMotion_[2];
    unsigned int _ZtlSecureTear_nStopMotion__CS;
    int _ZtlSecureTear_rStopMotion_[2];
    unsigned int _ZtlSecureTear_rStopMotion__CS;
    int _ZtlSecureTear_tStopMotion_[2];
    unsigned int _ZtlSecureTear_tStopMotion__CS;
    int _ZtlSecureTear_nFear_[2];
    unsigned int _ZtlSecureTear_nFear__CS;
    int _ZtlSecureTear_rFear_[2];
    unsigned int _ZtlSecureTear_rFear__CS;
    int _ZtlSecureTear_tFear_[2];
    unsigned int _ZtlSecureTear_tFear__CS;
    int _ZtlSecureTear_nEvanSlow_[2];
    unsigned int _ZtlSecureTear_nEvanSlow__CS;
    int _ZtlSecureTear_rEvanSlow_[2];
    unsigned int _ZtlSecureTear_rEvanSlow__CS;
    int _ZtlSecureTear_tEvanSlow_[2];
    unsigned int _ZtlSecureTear_tEvanSlow__CS;
    int _ZtlSecureTear_nMagicShield_[2];
    unsigned int _ZtlSecureTear_nMagicShield__CS;
    int _ZtlSecureTear_rMagicShield_[2];
    unsigned int _ZtlSecureTear_rMagicShield__CS;
    int _ZtlSecureTear_tMagicShield_[2];
    unsigned int _ZtlSecureTear_tMagicShield__CS;
    int _ZtlSecureTear_mMagicShield_[2];
    unsigned int _ZtlSecureTear_mMagicShield__CS;
    int _ZtlSecureTear_nMagicResistance_[2];
    unsigned int _ZtlSecureTear_nMagicResistance__CS;
    int _ZtlSecureTear_rMagicResistance_[2];
    unsigned int _ZtlSecureTear_rMagicResistance__CS;
    int _ZtlSecureTear_tMagicResistance_[2];
    unsigned int _ZtlSecureTear_tMagicResistance__CS;
    int _ZtlSecureTear_nSoulStone_[2];
    unsigned int _ZtlSecureTear_nSoulStone__CS;
    int _ZtlSecureTear_rSoulStone_[2];
    unsigned int _ZtlSecureTear_rSoulStone__CS;
    int _ZtlSecureTear_tSoulStone_[2];
    unsigned int _ZtlSecureTear_tSoulStone__CS;
    // Atlas bits 82-85 (Flying / Frozen / AssistCharge / MirrorImage [v95 PDB name: nEnrage_]).
    // v87 confirmed via Reset stat-group mapping (docs/tasks/cwvscontext-port/v87_secondarystat_reset_mapping.md).
    // task-006: v84 carries ONLY Flying+Frozen (@0xCA0, 6 tears) — gate split >=87 -> >=84 here;
    // AssistCharge+Enrage stay >=87 (absent in v84). Restores v84=0xD20.
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 84) || defined(REGION_JMS)
    int _ZtlSecureTear_nFlying_[2];
    unsigned int _ZtlSecureTear_nFlying__CS;
    int _ZtlSecureTear_rFlying_[2];
    unsigned int _ZtlSecureTear_rFlying__CS;
    int _ZtlSecureTear_tFlying_[2];
    unsigned int _ZtlSecureTear_tFlying__CS;
    int _ZtlSecureTear_nFrozen_[2];
    unsigned int _ZtlSecureTear_nFrozen__CS;
    int _ZtlSecureTear_rFrozen_[2];
    unsigned int _ZtlSecureTear_rFrozen__CS;
    int _ZtlSecureTear_tFrozen_[2];
    unsigned int _ZtlSecureTear_tFrozen__CS;
#endif
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 87) || defined(REGION_JMS)
    // Atlas bits 84-85 (AssistCharge / MirrorImage [v95 PDB name: nEnrage_]) — absent in v84 (kept >=87; task-006).
    int _ZtlSecureTear_nAssistCharge_[2];
    unsigned int _ZtlSecureTear_nAssistCharge__CS;
    int _ZtlSecureTear_rAssistCharge_[2];
    unsigned int _ZtlSecureTear_rAssistCharge__CS;
    int _ZtlSecureTear_tAssistCharge_[2];
    unsigned int _ZtlSecureTear_tAssistCharge__CS;
    int _ZtlSecureTear_nEnrage_[2];
    unsigned int _ZtlSecureTear_nEnrage__CS;
    int _ZtlSecureTear_rEnrage_[2];
    unsigned int _ZtlSecureTear_rEnrage__CS;
    int _ZtlSecureTear_tEnrage_[2];
    unsigned int _ZtlSecureTear_tEnrage__CS;
#endif
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95) || defined(REGION_JMS)
    // Atlas bits 86+ (SuddenDeath onwards) — v87 lacks these.
    int _ZtlSecureTear_nSuddenDeath_[2];
    unsigned int _ZtlSecureTear_nSuddenDeath__CS;
    int _ZtlSecureTear_rSuddenDeath_[2];
    unsigned int _ZtlSecureTear_rSuddenDeath__CS;
    int _ZtlSecureTear_tSuddenDeath_[2];
    unsigned int _ZtlSecureTear_tSuddenDeath__CS;
    int _ZtlSecureTear_mSuddenDeath_[2];
    unsigned int _ZtlSecureTear_mSuddenDeath__CS;
    int _ZtlSecureTear_nNotDamaged_[2];
    unsigned int _ZtlSecureTear_nNotDamaged__CS;
    int _ZtlSecureTear_rNotDamaged_[2];
    unsigned int _ZtlSecureTear_rNotDamaged__CS;
    int _ZtlSecureTear_tNotDamaged_[2];
    unsigned int _ZtlSecureTear_tNotDamaged__CS;
    int _ZtlSecureTear_nFinalCut_[2];
    unsigned int _ZtlSecureTear_nFinalCut__CS;
    int _ZtlSecureTear_rFinalCut_[2];
    unsigned int _ZtlSecureTear_rFinalCut__CS;
    int _ZtlSecureTear_tFinalCut_[2];
    unsigned int _ZtlSecureTear_tFinalCut__CS;
    int _ZtlSecureTear_nThornsEffect_[2];
    unsigned int _ZtlSecureTear_nThornsEffect__CS;
    int _ZtlSecureTear_rThornsEffect_[2];
    unsigned int _ZtlSecureTear_rThornsEffect__CS;
    int _ZtlSecureTear_tThornsEffect_[2];
    unsigned int _ZtlSecureTear_tThornsEffect__CS;
    int _ZtlSecureTear_nSwallowAttackDamage_[2];
    unsigned int _ZtlSecureTear_nSwallowAttackDamage__CS;
    int _ZtlSecureTear_rSwallowAttackDamage_[2];
    unsigned int _ZtlSecureTear_rSwallowAttackDamage__CS;
    int _ZtlSecureTear_tSwallowAttackDamage_[2];
    unsigned int _ZtlSecureTear_tSwallowAttackDamage__CS;
    int _ZtlSecureTear_nMorewildDamageUp_[2];
    unsigned int _ZtlSecureTear_nMorewildDamageUp__CS;
    int _ZtlSecureTear_rMorewildDamageUp_[2];
    unsigned int _ZtlSecureTear_rMorewildDamageUp__CS;
    int _ZtlSecureTear_tMorewildDamageUp_[2];
    unsigned int _ZtlSecureTear_tMorewildDamageUp__CS;
    int _ZtlSecureTear_nEMHP[2];
    unsigned int _ZtlSecureTear_nEMHP_CS;
    int _ZtlSecureTear_nEMHP_[2];
    unsigned int _ZtlSecureTear_nEMHP__CS;
    int _ZtlSecureTear_rEMHP_[2];
    unsigned int _ZtlSecureTear_rEMHP__CS;
    int _ZtlSecureTear_tEMHP_[2];
    unsigned int _ZtlSecureTear_tEMHP__CS;
    int _ZtlSecureTear_nEMMP[2];
    unsigned int _ZtlSecureTear_nEMMP_CS;
    int _ZtlSecureTear_nEMMP_[2];
    unsigned int _ZtlSecureTear_nEMMP__CS;
    int _ZtlSecureTear_rEMMP_[2];
    unsigned int _ZtlSecureTear_rEMMP__CS;
    int _ZtlSecureTear_tEMMP_[2];
    unsigned int _ZtlSecureTear_tEMMP__CS;
    int _ZtlSecureTear_nEPAD[2];
    unsigned int _ZtlSecureTear_nEPAD_CS;
    int _ZtlSecureTear_nEPAD_[2];
    unsigned int _ZtlSecureTear_nEPAD__CS;
    int _ZtlSecureTear_rEPAD_[2];
    unsigned int _ZtlSecureTear_rEPAD__CS;
    int _ZtlSecureTear_tEPAD_[2];
    unsigned int _ZtlSecureTear_tEPAD__CS;
    int _ZtlSecureTear_nEPDD[2];
    unsigned int _ZtlSecureTear_nEPDD_CS;
    int _ZtlSecureTear_nEPDD_[2];
    unsigned int _ZtlSecureTear_nEPDD__CS;
    int _ZtlSecureTear_rEPDD_[2];
    unsigned int _ZtlSecureTear_rEPDD__CS;
    int _ZtlSecureTear_tEPDD_[2];
    unsigned int _ZtlSecureTear_tEPDD__CS;
    int _ZtlSecureTear_nEMDD[2];
    unsigned int _ZtlSecureTear_nEMDD_CS;
    int _ZtlSecureTear_nEMDD_[2];
    unsigned int _ZtlSecureTear_nEMDD__CS;
#endif
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
    // v95-only tail: v185 JMS truncates SecondaryStat at nEMDD_ (above), then places
    // aTemporaryStat[7] at v185 0x1054.
    int _ZtlSecureTear_rEMDD_[2];
    unsigned int _ZtlSecureTear_rEMDD__CS;
    int _ZtlSecureTear_tEMDD_[2];
    unsigned int _ZtlSecureTear_tEMDD__CS;
    int _ZtlSecureTear_nGuard_[2];
    unsigned int _ZtlSecureTear_nGuard__CS;
    int _ZtlSecureTear_rGuard_[2];
    unsigned int _ZtlSecureTear_rGuard__CS;
    int _ZtlSecureTear_tGuard_[2];
    unsigned int _ZtlSecureTear_tGuard__CS;
    int _ZtlSecureTear_nMine_[2];
    unsigned int _ZtlSecureTear_nMine__CS;
    int _ZtlSecureTear_rMine_[2];
    unsigned int _ZtlSecureTear_rMine__CS;
    int _ZtlSecureTear_tMine_[2];
    unsigned int _ZtlSecureTear_tMine__CS;
    int _ZtlSecureTear_nSafetyDamage_[2];
    unsigned int _ZtlSecureTear_nSafetyDamage__CS;
    int _ZtlSecureTear_rSafetyDamage_[2];
    unsigned int _ZtlSecureTear_rSafetyDamage__CS;
    int _ZtlSecureTear_tSafetyDamage_[2];
    unsigned int _ZtlSecureTear_tSafetyDamage__CS;
    int _ZtlSecureTear_nSafetyAbsorb_[2];
    unsigned int _ZtlSecureTear_nSafetyAbsorb__CS;
    int _ZtlSecureTear_rSafetyAbsorb_[2];
    unsigned int _ZtlSecureTear_rSafetyAbsorb__CS;
    int _ZtlSecureTear_tSafetyAbsorb_[2];
    unsigned int _ZtlSecureTear_tSafetyAbsorb__CS;
    int _ZtlSecureTear_nCyclone_[2];
    unsigned int _ZtlSecureTear_nCyclone__CS;
    int _ZtlSecureTear_rCyclone_[2];
    unsigned int _ZtlSecureTear_rCyclone__CS;
    int _ZtlSecureTear_tCyclone_[2];
    unsigned int _ZtlSecureTear_tCyclone__CS;
    int _ZtlSecureTear_nSwallowCritical_[2];
    unsigned int _ZtlSecureTear_nSwallowCritical__CS;
    int _ZtlSecureTear_rSwallowCritical_[2];
    unsigned int _ZtlSecureTear_rSwallowCritical__CS;
    int _ZtlSecureTear_tSwallowCritical_[2];
    unsigned int _ZtlSecureTear_tSwallowCritical__CS;
    int _ZtlSecureTear_nSwallowMaxMP_[2];
    unsigned int _ZtlSecureTear_nSwallowMaxMP__CS;
    int _ZtlSecureTear_rSwallowMaxMP_[2];
    unsigned int _ZtlSecureTear_rSwallowMaxMP__CS;
    int _ZtlSecureTear_tSwallowMaxMP_[2];
    unsigned int _ZtlSecureTear_tSwallowMaxMP__CS;
    int _ZtlSecureTear_nSwallowDefence_[2];
    unsigned int _ZtlSecureTear_nSwallowDefence__CS;
    int _ZtlSecureTear_rSwallowDefence_[2];
    unsigned int _ZtlSecureTear_rSwallowDefence__CS;
    int _ZtlSecureTear_tSwallowDefence_[2];
    unsigned int _ZtlSecureTear_tSwallowDefence__CS;
    int _ZtlSecureTear_nSwallowEvasion_[2];
    unsigned int _ZtlSecureTear_nSwallowEvasion__CS;
    int _ZtlSecureTear_rSwallowEvasion_[2];
    unsigned int _ZtlSecureTear_rSwallowEvasion__CS;
    int _ZtlSecureTear_tSwallowEvasion_[2];
    unsigned int _ZtlSecureTear_tSwallowEvasion__CS;
    int _ZtlSecureTear_tSwallowBuffTime_[2];
    unsigned int _ZtlSecureTear_tSwallowBuffTime__CS;
    int _ZtlSecureTear_nConversion_[2];
    unsigned int _ZtlSecureTear_nConversion__CS;
    int _ZtlSecureTear_rConversion_[2];
    unsigned int _ZtlSecureTear_rConversion__CS;
    int _ZtlSecureTear_tConversion_[2];
    unsigned int _ZtlSecureTear_tConversion__CS;
    int _ZtlSecureTear_nRevive_[2];
    unsigned int _ZtlSecureTear_nRevive__CS;
    int _ZtlSecureTear_rRevive_[2];
    unsigned int _ZtlSecureTear_rRevive__CS;
    int _ZtlSecureTear_tRevive_[2];
    unsigned int _ZtlSecureTear_tRevive__CS;
    int _ZtlSecureTear_nSneak_[2];
    unsigned int _ZtlSecureTear_nSneak__CS;
    int _ZtlSecureTear_rSneak_[2];
    unsigned int _ZtlSecureTear_rSneak__CS;
    int _ZtlSecureTear_tSneak_[2];
    unsigned int _ZtlSecureTear_tSneak__CS;
    int _ZtlSecureTear_nMorewildMaxHP_[2];
    unsigned int _ZtlSecureTear_nMorewildMaxHP__CS;
    int _ZtlSecureTear_rMorewildMaxHP_[2];
    unsigned int _ZtlSecureTear_rMorewildMaxHP__CS;
    int _ZtlSecureTear_tMorewildMaxHP_[2];
    unsigned int _ZtlSecureTear_tMorewildMaxHP__CS;
    int _ZtlSecureTear_nDice_[2];
    unsigned int _ZtlSecureTear_nDice__CS;
    int _ZtlSecureTear_rDice_[2];
    unsigned int _ZtlSecureTear_rDice__CS;
    int _ZtlSecureTear_tDice_[2];
    unsigned int _ZtlSecureTear_tDice__CS;
    int aDiceInfo[22];
    int _ZtlSecureTear_nBlessingArmor_[2];
    unsigned int _ZtlSecureTear_nBlessingArmor__CS;
    int _ZtlSecureTear_rBlessingArmor_[2];
    unsigned int _ZtlSecureTear_rBlessingArmor__CS;
    int _ZtlSecureTear_tBlessingArmor_[2];
    unsigned int _ZtlSecureTear_tBlessingArmor__CS;
    int _ZtlSecureTear_nBlessingArmorIncPAD_[2];
    unsigned int _ZtlSecureTear_nBlessingArmorIncPAD__CS;
    int _ZtlSecureTear_nDamR_[2];
    unsigned int _ZtlSecureTear_nDamR__CS;
    int _ZtlSecureTear_rDamR_[2];
    unsigned int _ZtlSecureTear_rDamR__CS;
    int _ZtlSecureTear_tDamR_[2];
    unsigned int _ZtlSecureTear_tDamR__CS;
    int _ZtlSecureTear_nTeleportMasteryOn_[2];
    unsigned int _ZtlSecureTear_nTeleportMasteryOn__CS;
    int _ZtlSecureTear_rTeleportMasteryOn_[2];
    unsigned int _ZtlSecureTear_rTeleportMasteryOn__CS;
    int _ZtlSecureTear_tTeleportMasteryOn_[2];
    unsigned int _ZtlSecureTear_tTeleportMasteryOn__CS;
    int _ZtlSecureTear_nCombatOrders_[2];
    unsigned int _ZtlSecureTear_nCombatOrders__CS;
    int _ZtlSecureTear_rCombatOrders_[2];
    unsigned int _ZtlSecureTear_rCombatOrders__CS;
    int _ZtlSecureTear_tCombatOrders_[2];
    unsigned int _ZtlSecureTear_tCombatOrders__CS;
    int _ZtlSecureTear_nBeholder_[2];
    unsigned int _ZtlSecureTear_nBeholder__CS;
    int _ZtlSecureTear_rBeholder_[2];
    unsigned int _ZtlSecureTear_rBeholder__CS;
    int _ZtlSecureTear_tBeholder_[2];
    unsigned int _ZtlSecureTear_tBeholder__CS;
    int _ZtlSecureTear_nSummonBomb_[2];
    unsigned int _ZtlSecureTear_nSummonBomb__CS;
    int _ZtlSecureTear_rSummonBomb_[2];
    unsigned int _ZtlSecureTear_rSummonBomb__CS;
    int _ZtlSecureTear_tSummonBomb_[2];
    unsigned int _ZtlSecureTear_tSummonBomb__CS;
    int _ZtlSecureTear_lSummonBomb_[2];
    unsigned int _ZtlSecureTear_lSummonBomb__CS;
#endif
    // v72+/v79/v83/v87/v95/v111/JMS185 over-model trailing two-state array: 7 x 8B ZRef = 0x38.
    // (The v61 layout — including its 6 x 4B array — lives in the self-contained #if<72 branch above;
    // this single tail gate absorbs the former Site-D [6]/[7] split. verified task-010)
    ZRef<TemporaryStatBase<long>> aTemporaryStat[7];
#endif // v61 vs v72+ SecondaryStat body gate (BUILD_MAJOR_VERSION < 72)
};

#if defined(REGION_GMS) && BUILD_MAJOR_VERSION < 72
assert_size(sizeof(SecondaryStat), 0x970) // v61 faithful layout — full rebuild, task-010
#endif
