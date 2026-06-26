#pragma once

#include "asserts.h"

class CFuncKeyMappedMan {
public:
    virtual ~CFuncKeyMappedMan() = default;

#if defined(REGION_GMS)
    FUNCKEY_MAPPED m_aFuncKeyMapped[89];
    FUNCKEY_MAPPED m_aFuncKeyMapped_Old[89];
#elif defined(REGION_JMS)
    FUNCKEY_MAPPED m_aFuncKeyMapped[94];
    FUNCKEY_MAPPED m_aFuncKeyMapped_Old[94];
#endif
    // v79: lacks the two quickslot int arrays (0x40 below-floor member shift; sizeof 0x388 vs
    // v83 0x3C8) — present in v83+ and JMS. Gate excludes ONLY v79. verified task-008
#if defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 83 || defined(REGION_JMS)
    int m_aQuickslotKeyMapped[8];
    int m_aQuickslotKeyMapped_Old[8];
#endif
    int m_nPetConsumeItemID;
    int m_nPetConsumeMPItemID;
#if defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 111 || defined(REGION_JMS)
    int dummy1;
#endif
#if defined(REGION_JMS)
    int dummy2;
#endif
#if defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95
    int m_nNormalAttackCode;
#endif

    CFuncKeyMappedMan();

    static void CreateInstance();
};

// CFuncKeyMappedMan: TSingleton-allocated, and key_mapped_hooks memcpy's into m_aFuncKeyMapped /
// m_aQuickslotKeyMapped — member offsets must be EXACT (a wrong layout corrupts it / adjacent
// memory). Real sizes (size sweep): v83/v84/v87 = 0x3C8, v111 = 0x3D0, JMS = 0x400 (v95 TBD).
// v79 is DEFERRED (task-008): measured sizeof == 0x388 (904) — Alloc(0x388) @CreateInstance
// 0x946AFB + ctor 0x569DE5 field-init extent (vtable@0, 2x memcpy 0x1BD @+4/@+0x1C1 ending @0x37E,
// dwords zeroed @+0x380/+0x384). That is 0x40 BELOW the header's computed base layout (0x3C8) — a
// below-floor MEMBER shift (the two m_aQuickslotKeyMapped[8] int arrays do not exist in v79), NOT a
// size-assert fix. A bare assert_size(...,0x388) would FAIL to compile (header still lays out 0x3C8).
// No v79 branch added here on purpose: fixing the member gate belongs to the struct audit (Task 12/16).
// v79: with the quickslot pair now gated out, the header computes 0x388 — restores the guard
// deferred in Task 3 (World-B closure). verified task-008
#if defined(REGION_GMS) && BUILD_MAJOR_VERSION == 79
assert_size(sizeof(CFuncKeyMappedMan), 0x388)
#elif defined(REGION_GMS) && (BUILD_MAJOR_VERSION == 83 || BUILD_MAJOR_VERSION == 84 || BUILD_MAJOR_VERSION == 87)
assert_size(sizeof(CFuncKeyMappedMan), 0x3C8)
#elif defined(REGION_GMS) && BUILD_MAJOR_VERSION == 95
assert_size(sizeof(CFuncKeyMappedMan), 0x3CC)
#elif defined(REGION_GMS) && BUILD_MAJOR_VERSION == 111
assert_size(sizeof(CFuncKeyMappedMan), 0x3D0)
#elif defined(REGION_JMS)
assert_size(sizeof(CFuncKeyMappedMan), 0x400)
#endif
