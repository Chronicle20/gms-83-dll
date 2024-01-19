#include "ZXString.h"
#include "ZArray.h"

struct ALLIANCEDATA {
    int nAllianceID;
    ZXString<char> sAllianceName;
    ZArray<ZXString<char>> asGradeName;
    ZArray<unsigned long> adwGuildID;
    int nMaxMemberNum;
    ZXString<char> sNotice;
};
