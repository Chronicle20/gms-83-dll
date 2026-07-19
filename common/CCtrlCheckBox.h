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

#if defined(REGION_GMS) && (BUILD_MAJOR_VERSION == 72 || BUILD_MAJOR_VERSION == 79)
// v79/v72: CCtrlWnd base 0x34 (v79 ctor @0x4d4378, v72 @0x4cc645 — proven
// identical). m_nCheckBoxState is the first member right after the base;
// m_apCanvasCheckBox[4] is last -> sizeof 0x6C. The >=95 text-offset pair is
// excluded for both, and all members between the base and the array are
// version-invariant, so v72 == v79 == 0x6C. (v72 ctor stripped in the IDB; verdict
// is base-arithmetic anchored to the shared CCtrlWnd base.) task-008/task-009.
assert_size(sizeof(CCtrlCheckBox), 0x6C);
#endif
