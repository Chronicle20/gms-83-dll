#pragma once

struct FamilyInfo {
    int nFamousPoint;
    int nTotalFamousPoint;
    int nTodaySavePoint;
    unsigned __int16 wChildCount;
    unsigned __int16 wChildLimit;
    unsigned __int16 wTotalChildCount;
    // 2 bytes of padding before dwBossID (compiler-inserted)
    unsigned int dwBossID;
    ZXString<char> strFamilyName;
    ZXString<char> strPrecept;
    ZMap<long, long, long> mPrivilegeUse;
    int bInitialized;
};
