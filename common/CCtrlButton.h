#pragma once

class CCtrlButton : CCtrlWnd {
public:
    struct CREATEPARAM {
        bool bAcceptFocus;
        bool bDrawBack;
        bool bAnimateOnce;
        ZXString<unsigned short> sUOL;
    };

    int m_nDisplayState;
    int m_nDisplayFrame;
    int m_nAniCount;
    int m_nAniDelay;
    unsigned int m_dwDisplayStarted;
    int m_bMouseEnter;
    int m_nDecClickArea;
    int m_bPressed;
    int m_bPressedByKey;
    int m_bKeyFocused;
    int m_bDrawBack;
    int m_bAnimateOnce;
    _com_ptr_t<_com_IIID<IWzProperty, &IID_IUnknown>> m_pPropFocusFrame;
    _com_ptr_t<_com_IIID<IWzGr2DLayer, &IID_IUnknown>> m_pLayerFocusFrame;
    _com_ptr_t<_com_IIID<IWzProperty, &IID_IUnknown>> m_apPropButton[4];
    int m_bToolTip;
    int m_bToolTipUpDir;
    ZXString<char> m_sToolTipTitle;
    ZXString<char> m_sToolTipDesc;
    CUIToolTip m_uiToolTip;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
    ZXString<char> m_sToolTipFromData;
#endif
    int m_bSelfDisable;
};