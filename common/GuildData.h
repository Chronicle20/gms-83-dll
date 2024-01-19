#include "ZXString.h"
#include "ZArray.h"
#include "ZMap.h"
#include "GuildMember.h"

struct GUILDDATA {
    struct SKILLENTRY {
        __int16 nLevel;
        _FILETIME dateExpire;
        ZXString<char> strBuyCharacterName;
    };

    int nGuildID;
    ZXString<char> sGuildName;
    ZArray<ZXString<char>> asGradeName;
    ZArray<unsigned long> adwCharacterID;
    ZArray<GUILDMEMBER> aMemberData;
    int nMaxMemberNum;
    unsigned __int16 nMarkBg;
    unsigned __int8 nMarkBgColor;
    unsigned __int16 nMark;
    unsigned __int8 nMarkColor;
    ZXString<char> sNotice;
    int nPoint;
    int nAllianceID;
    int nLevel;
    ZMap<long, GUILDDATA::SKILLENTRY, long> mSkillRecord;
    ZArray<long> aSkillRecordOnlyID;
};
