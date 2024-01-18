#pragma once
#include "ZRefCounted.h"
#include "FunckeyMapped.h"
#include "IUIMsgHandler.h"

/*
00000000 IDraggable      struc; (sizeof = 0x18, align = 0x4, copyof_1424)
00000000 baseclass_0     ZRefCounted ?
0000000C m_pLayer        _com_ptr_t<_com_IIID<IWzGr2DLayer, &_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > ?
00000010 m_OldIcon       FUNCKEY_MAPPED ?
00000015                 db ? ; undefined
00000016                 db ? ; undefined
00000017                 db ? ; undefined
00000018 IDraggable      ends
*/
class IDraggable : ZRefCounted
{
    //m_pLayer
    FUNCKEY_MAPPED m_OldIcon;
    int Unknown1;
    int Unknown2;
    int Unknown3;

    virtual int OnDoubleClicked(IDraggable*) = 0;
    virtual int OnDropped(IDraggable*, IUIMsgHandler* pFrom, IUIMsgHandler* pTo, int rx, int ry) = 0;
};