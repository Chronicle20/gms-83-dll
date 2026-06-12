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
    int m_aQuickslotKeyMapped[8];
    int m_aQuickslotKeyMapped_Old[8];
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
#if defined(REGION_GMS) && (BUILD_MAJOR_VERSION == 83 || BUILD_MAJOR_VERSION == 84 || BUILD_MAJOR_VERSION == 87)
assert_size(sizeof(CFuncKeyMappedMan), 0x3C8)
#elif defined(REGION_GMS) && BUILD_MAJOR_VERSION == 95
assert_size(sizeof(CFuncKeyMappedMan), 0x3CC)
#elif defined(REGION_GMS) && BUILD_MAJOR_VERSION == 111
assert_size(sizeof(CFuncKeyMappedMan), 0x3D0)
#elif defined(REGION_JMS)
assert_size(sizeof(CFuncKeyMappedMan), 0x400)
#endif
