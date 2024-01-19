#pragma once

/*
00000000 CCtrlButton     struc; (sizeof = 0xADC, align = 0x4, copyof_1539)
00000000 baseclass_0     CCtrlWnd ?
00000034 m_nDisplayState dd ?
00000038 m_nDisplayFrame dd ?
0000003C m_nAniCount     dd ?
00000040 m_nAniDelay     dd ?
00000044 m_dwDisplayStarted dd ?
00000048 m_bMouseEnter   dd ?
0000004C m_nDecClickArea dd ?
00000050 m_bPressed      dd ?
00000054 m_bPressedByKey dd ?
00000058 m_bKeyFocused   dd ?
0000005C m_bDrawBack     dd ?
00000060 m_bAnimateOnce  dd ?
00000064 m_pPropFocusFrame _com_ptr_t<_com_IIID<IWzProperty, &_GUID_986515d9_0a0b_4929_8b4f_718682177b92> > ?
00000068 m_pLayerFocusFrame _com_ptr_t<_com_IIID<IWzGr2DLayer, &_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > ?
0000006C m_apPropButton  _com_ptr_t < _com_IIID<IWzProperty, &_GUID_986515d9_0a0b_4929_8b4f_718682177b92> > 4 dup(? )
0000007C m_bToolTip      dd ?
00000080 m_bToolTipUpDir dd ?
00000084 m_sToolTipTitle ZXString<char> ?
00000088 m_sToolTipDesc  ZXString<char> ?
0000008C m_uiToolTip     CUIToolTip ?
00000AD4 m_sToolTipFromData ZXString<char> ?
00000AD8 m_bSelfDisable  dd ?
00000ADC CCtrlButton     ends
*/
class CCtrlButton : CCtrlWnd
{
public:
    /*
    00000000 CCtrlButton::CREATEPARAM struc ; (sizeof=0x10, align=0x4, copyof_1685)
    00000000 bAcceptFocus    dd ?
    00000004 bDrawBack       dd ?
    00000008 bAnimateOnce    dd ?
    0000000C sUOL            ZXString<unsigned short> ?
    00000010 CCtrlButton::CREATEPARAM ends
    */
    struct CREATEPARAM
    {
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
    bool m_bMouseEnter;
    int m_nDecClickArea;
    bool m_bPressed;
    bool m_bPressedByKey;
    bool m_bKeyFocused;
    bool m_bDrawBack;
    bool m_bAnimateOnce;
    IWzProperty* m_pPropFocusFrame;
    IWzGr2DLayer* m_pLayerFocusFrame;
    IWzProperty* m_apPropButton[4];
    bool m_bToolTip;
    bool m_bToolTipUpDir;
    ZXString<char> m_sToolTipTitle;
    ZXString<char> m_sToolTipDesc;
    CUIToolTip m_uiToolTip;
    ZXString<char> m_sToolTipFromData;
    bool m_bSelfDisable;
};