#pragma once

class CLogin;

class CUILoginStart : CDialog
{
    CLogin* m_pLogin;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95) || defined(REGION_JMS)
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFont;
    _com_ptr_t<_com_IIID<IWzCanvas, &IID_IUnknown>> m_pCanvasChannelName;
#endif
#if defined(REGION_GMS)
    CCtrlButton::CREATEPARAM m_aBtParam[5];
    ZRef<CCtrlButton> m_apButton[5];
#else
    CCtrlButton::CREATEPARAM m_aBtParam[8];
    ZRef<CCtrlButton> m_apButton[8];
#endif
    int m_nViewWorldButtonType;
    bool m_bRequestSent;

    void EnableLoginStartCtrl(CUILoginStart*, int);
};