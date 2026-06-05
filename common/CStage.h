#pragma once

/*
00000000 CStage          struc; (sizeof = 0x18, align = 0x4, copyof_3252)
00000000 baseclass_0     IGObj ?
00000004 baseclass_4     IUIMsgHandler ?
00000008 baseclass_8     INetMsgHandler ?
0000000C baseclass_c     ZRefCounted ?
00000018 CStage          ends
*/
// CStage::~CStage() is hooked by edits via the memory-map address
// C_STAGE_DTOR. We deliberately do not declare ~CStage here — the
// virtual destruction slot is inherited from ZRefCounted and the hook
// is installed against the recovered v83.1 address directly.
class CStage : public IGObj, public IUIMsgHandler, public INetMsgHandler, public ZRefCounted {
public:
    void OnMouseEnter(int) override;

    void OnPacket(int, CInPacket *) override;
};