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
    // m_uiToolTip is the LAST member for GMS v72 and v79: the alloc immediates
    // (v72 push 59Ch @CUIFadeYesNo::OnCreate 0x500921; v79 push 5A0h @0x50c293)
    // equal exactly 0x8C(embed) + sizeof(CUIToolTip) [0x510 v72 / 0x514 v79], and
    // both ctors (v72 0x422954, v79 0x422d59) emit no write past the m_uiToolTip
    // embed. So m_bSelfDisable is absent below v83. task-009 (also corrects the
    // v79 task-008 value: true v79 = 0x5A0, not 0x5A4).
#if !(defined(REGION_GMS) && BUILD_MAJOR_VERSION <= 79)
    int m_bSelfDisable;
#endif
};

#if defined(REGION_GMS) && BUILD_MAJOR_VERSION == 79
// v79: CCtrlWnd base 0x34 + CUIToolTip m_uiToolTip 0x514 @0x8C (last member).
// alloc @0x50c293 push 5A0h -> 0x8C+0x514 = 0x5A0. (task-008 stated 0x5A4 by
// CCtrlWnd+tooltip arithmetic that double-counted a nonexistent m_bSelfDisable;
// the alloc immediate proves 0x5A0. task-009.)
assert_size(sizeof(CCtrlButton), 0x5A0);
#elif defined(REGION_GMS) && BUILD_MAJOR_VERSION == 72
// v72: CCtrlWnd base 0x34 + CUIToolTip m_uiToolTip 0x510 @0x8C (last member).
// alloc @0x500921 push 59Ch -> 0x8C+0x510 = 0x59C; ctor @0x422954.
assert_size(sizeof(CCtrlButton), 0x59C);
#endif