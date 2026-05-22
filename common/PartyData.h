#pragma once
#include "PartyMember.h"
#include "ZXString.h"

struct PARTYDATA {
    struct TOWNPORTAL {
        unsigned int m_dwTownID;
        unsigned int m_dwFieldID;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
        int m_nSKillID;
#endif
        tagPOINT m_ptFieldPortal;
    };

    PARTYMEMBER party;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
    unsigned int adwFieldID[6];
#endif
    PARTYDATA::TOWNPORTAL aTownPortal[6];
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
    int aPQReward[6];
    int aPQRewardType[6];
    unsigned int dwPQRewardMobTemplateID;
    int bPQReward;
#endif
};
