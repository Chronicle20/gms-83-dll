#pragma once
#include "IWzSerialize.h"
#include "IUnknown2.h"

/*
00000000 IWzProperty     struc; (sizeof = 0x4, align = 0x4, copyof_1523)
00000000 baseclass_0     IWzSerialize ?
00000004 IWzProperty     ends
*/
class IWzProperty : IWzSerialize
{
};