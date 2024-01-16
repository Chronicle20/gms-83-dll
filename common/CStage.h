#pragma once

#include "IGObj.h"
#include "IUIMsgHandler.h"
#include "INetMsgHandler.h"
#include "ZRefCounted.h"

/*
00000000 CStage          struc; (sizeof = 0x18, align = 0x4, copyof_3252)
00000000 baseclass_0     IGObj ?
00000004 baseclass_4     IUIMsgHandler ?
00000008 baseclass_8     INetMsgHandler ?
0000000C baseclass_c     ZRefCounted ?
00000018 CStage          ends
*/
class CStage : IGObj, IUIMsgHandler, INetMsgHandler, public ZRefCounted {
public:
    virtual ~CStage() = default;
};