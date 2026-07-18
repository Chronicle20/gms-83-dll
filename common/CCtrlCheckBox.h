#pragma once

class CCtrlCheckBox : CCtrlWnd {
public:
    int m_nCheckBoxState;
    int m_nArrange;
    int m_nBackColor;
    int m_nFontColor;
    ZXString<char> m_sText;
    int m_bChecked;
    IWzFont *m_pFont;
    IWzFont *m_pFontDisabled;
    int m_nFontHeight;
    int m_bDrawLineAtFocus;
    IWzCanvas *m_apCanvasCheckBox[4];
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
    int m_nTextOffsetX;
    int m_nTextOffsetY;
#endif
};

#if defined(REGION_GMS) && BUILD_MAJOR_VERSION == 79
// v79: CCtrlWnd base 0x34 (fixed, task-008). m_nCheckBoxState is the first member
// right after the base; m_apCanvasCheckBox[4] is last -> sizeof 0x6C.
assert_size(sizeof(CCtrlCheckBox), 0x6C);
#endif
