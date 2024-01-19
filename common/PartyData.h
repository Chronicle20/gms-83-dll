#include "ZXString.h"
#include "PartyMember.h"

struct PARTYDATA {
    struct TOWNPORTAL
    {
        unsigned int m_dwTownID;
        unsigned int m_dwFieldID;
        int m_nSKillID;
        tagPOINT m_ptFieldPortal;
    };

    PARTYMEMBER party;
    unsigned int adwFieldID[6];
    PARTYDATA::TOWNPORTAL aTownPortal[6];
    int aPQReward[6];
    int aPQRewardType[6];
    unsigned int dwPQRewardMobTemplateID;
    int bPQReward;
};
