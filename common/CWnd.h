#pragma once

#include "IGObj.h"
#include "IUIMsgHandler.h"
#include "IWzGr2DLayer.h"
#include "IWzCanvas.h"
#include "ZRefCounted.h"
#include "CCtrlWnd.h"
#include "ZList.h"
#include "ZRef.h"
#include "TSecType.h"

/*
00000000 CWnd            struc; (sizeof = 0x80, align = 0x4, copyof_1443)
00000000 baseclass_0     IGObj ?
00000004 baseclass_4     IUIMsgHandler ?
00000008 baseclass_8     ZRefCounted ?
00000014 m_dwWndKey      dd ?
00000018 m_pLayer        _com_ptr_t<_com_IIID<IWzGr2DLayer, &_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > ?
0000001C m_pAnimationLayer _com_ptr_t<_com_IIID<IWzGr2DLayer, &_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > ?
00000020 m_pOverlabLayer _com_ptr_t<_com_IIID<IWzGr2DLayer, &_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > ?
00000024 m_width         dd ?
00000028 m_height        dd ?
0000002C m_rcInvalidated tagRECT ?
0000003C m_bScreenCoord  dd ?
00000040 m_nBackgrndX    dd ?
00000044 m_nBackgrndY    dd ?
00000048 m_ptCursorRel   SECPOINT ?
00000060 m_lpChildren    ZList<ZRef<CCtrlWnd> > ?
00000074 m_pFocusChild   dd ? ; offset
00000078 m_pBackgrnd     _com_ptr_t<_com_IIID<IWzCanvas, &_GUID_7600dc6c_9328_4bff_9624_5b0f5c01179e> > ?
0000007C m_origin        dd ? ; enum CWnd::UIOrigin
00000080 CWnd            ends
*/
class CWnd : public IGObj, public IUIMsgHandler, public ZRefCounted {
    /*
    FFFFFFFF ; enum CWnd::UIOrigin, copyof_38
    FFFFFFFF Origin_LT        = 0
    FFFFFFFF Origin_CT        = 1
    FFFFFFFF Origin_RT        = 2
    FFFFFFFF Origin_LC        = 3
    FFFFFFFF Origin_CC        = 4
    FFFFFFFF Origin_RC        = 5
    FFFFFFFF Origin_LB        = 6
    FFFFFFFF Origin_CB        = 7
    FFFFFFFF Origin_RB        = 8
    FFFFFFFF Origin_NUM       = 9
    */
    enum UIOrigin {
        Origin_LT = 0,
        Origin_CT = 1,
        Origin_RT = 2,
        Origin_LC = 3,
        Origin_CC = 4,
        Origin_RC = 5,
        Origin_LB = 6,
        Origin_CB = 7,
        Origin_RB = 8,
        Origin_NUM = 9
    };

    unsigned long m_dwWndKey; // DWORD?
    IWzGr2DLayer *m_pLayer;
    IWzGr2DLayer *m_pAnimationLayer;
    IWzGr2DLayer *m_pOverlabLayer;
    int m_nBackgrndX;
    int m_nBackgrndY;
    SECPOINT m_ptCursorRel;
    ZList<ZRef<CCtrlWnd>> m_lpChildren;
    CCtrlWnd *m_pFocusChild;
    IWzCanvas *m_pBackgrnd;
    UIOrigin m_origin;
};