#pragma once

#include "IGObj.h"
#include "IUIMsgHandler.h"
#include "ZRefCounted.h"
#include "IWzVector2D.h"

/*
00000000 CCtrlWnd        struc; (sizeof = 0x34, align = 0x4, copyof_1444)
00000000 baseclass_0     IGObj ?
00000004 baseclass_4     IUIMsgHandler ?
00000008 baseclass_8     ZRefCounted ?
00000014 m_nCtrlId       dd ?
00000018 m_pLTCtrl       _com_ptr_t<_com_IIID<IWzVector2D, &_GUID_f28bd1ed_3deb_4f92_9eec_10ef5a1c3fb4>> ?
0000001C m_width         dd ?
00000020 m_height        dd ?
00000024 m_pParent       dd ? ; offset
00000028 m_bAcceptFocus  dd ?
0000002C m_bEnabled      dd ?
00000030 m_bShown        dd ?
00000034 CCtrlWnd        ends
*/
class CCtrlWnd : IGObj, IUIMsgHandler, ZRefCounted {
    int m_nCtrlId;
    IWzVector2D *m_pLTCtrl;
    int m_width;
    int m_height;
    int *m_pParent;
    bool m_bAcceptFocus;
    bool m_bEnabled;
    bool m_bShown;
};