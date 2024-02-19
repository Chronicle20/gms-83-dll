#include "ZArray.h"

struct GW_FriendRecord {
    unsigned int dwPairCharacterID;
    char sPairCharacterName[13];
    _LARGE_INTEGER liSN;
    _LARGE_INTEGER liPairSN;
    unsigned int dwFriendItemID;
};
