#pragma once
#include "ZList.h"
#include "SPSet.h"

/*
00000000 ExtendSP        struc; (sizeof = 0x14, align = 0x4, copyof_1763)
00000000 lSPSet          ZList<SPSet> ?
00000014 ExtendSP        ends
*/
struct ExtendSP
{
    ZList<SPSet> lSPSet;
};