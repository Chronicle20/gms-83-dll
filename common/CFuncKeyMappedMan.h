#pragma once

class CFuncKeyMappedMan {
public:


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
#if defined(REGION_GMS) && MAJOR_VERSION >= 111 || defined(REGION_JMS)
    int dummy1;
#endif
#if defined(REGION_JMS)
    int dummy2;
#endif
#if defined(REGION_GMS) && MAJOR_VERSION >= 95
    int m_nNormalAttackCode;
#endif

    static void CreateInstance();
};
