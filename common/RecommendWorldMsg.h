#pragma once
#include "ZXString.h"

/*/
00000000 RECOMMENDWORLDMSG struc; (sizeof = 0x8, align = 0x4, copyof_4932)
00000000 nWorldID        dd ?
00000004 sMessage        ZXString<char> ?
00000008 RECOMMENDWORLDMSG ends
*/
struct RecommendWorldMsg
{
    int nWorldID;
    ZXString<char> sMessage;
};