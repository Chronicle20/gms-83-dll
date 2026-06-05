#pragma once
struct CUIWnd : CWnd {
    // No C++ constructor: custom windows are built by custom-ui-host via a
    // raw __thiscall thunk to the game ctor (C_UI_WND_CTOR_INT) on a
    // host-owned byte buffer. Declaring a C++ ctor would force MSVC to
    // default-construct the CWnd base (instantiating RE-stub templates that
    // don't compile) and would double-construct the game object.

    ZRef<CCtrlButton> m_pBtClose;
    CUIToolTip m_uiToolTip;
    int m_nUIType;
    int m_nBtCloseType;
    int m_nBtCloseX;
    int m_nBtCloseY;
    int m_nBackgrndX;
    int m_nBackgrndY;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
    int m_nSmallScreenX;
    int m_nSmallScreenY;
    int m_nLargeScreenX;
    int m_nLargeScreenY;
    bool m_bIsLargeMode;
#endif
    bool m_bPosSave;
    bool m_bBackgrnd;
    int m_nOption;
    ZArray<unsigned char> m_abOption;
    ZXString<unsigned short> m_sBackgrndUOL;
};
