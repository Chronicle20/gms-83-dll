#pragma once
struct PARTYMEMBER {
    unsigned int adwCharacterID[6];
    char asCharacterName[6][13];
    int anJob[6];
    int anLevel[6];
    int anChannelID[6];
    unsigned int dwPartyBossCharacterID;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION < 95)
    // v83 keeps the per-member home-field array inside PARTYMEMBER; v95+ promoted it to PARTYDATA.
    unsigned int adwFieldID[6];
#endif
};
