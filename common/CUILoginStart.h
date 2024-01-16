#pragma once
#include "CDialog.h"
#include "CCtrlButton.h"

/*
00000000 CUILoginStart   struc; (sizeof = 0x11C, align = 0x4, copyof_4926)
00000000 baseclass_0     CDialog ?
00000090 m_pLogin        dd ? ; offset
00000094 m_pFont         _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
00000098 m_pCanvasChannelName _com_ptr_t<_com_IIID<IWzCanvas, &_GUID_7600dc6c_9328_4bff_9624_5b0f5c01179e> > ?
0000009C m_aBtParam      CCtrlButton::CREATEPARAM 5 dup(? )
000000EC m_apButton      ZRef < CCtrlButton> 5 dup(? )
00000114 m_nViewWorldButtonType dd ?
00000118 m_bRequestSent  dd ?
0000011C CUILoginStart   ends
*/
class CUILoginStart : CDialog
{
    void* m_pLogin;
    IWzFont* m_pFont;
    IWzCanvas* m_pCanvasChannelName;
    CCtrlButton::CREATEPARAM m_aBtParam[5];
    ZRef<CCtrlButton> m_apButton[5];
    int m_nViewWorldButtonType;
    bool m_bRequestSent;

    void EnableLoginStartCtrl(CUILoginStart*, int);
};