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

#if defined(REGION_GMS) && (BUILD_MAJOR_VERSION == 61 || BUILD_MAJOR_VERSION == 72 || BUILD_MAJOR_VERSION == 79)
// v61/v79/v72: CCtrlWnd base 0x34 (v61 ctor @0x4b2783, v79 @0x4d4378, v72 @0x4cc645 —
// proven identical). m_nCheckBoxState is the first member right after the base;
// m_apCanvasCheckBox[4] is last -> sizeof 0x6C. The >=95 text-offset pair is
// excluded for all three, and all members between the base and the array are
// version-invariant. v61 anchored by alloc @0x45501b push 6Ch (task-010; corrects the
// audit's ≈0x9C estimate). task-008/task-009/task-010.
assert_size(sizeof(CCtrlCheckBox), 0x6C);
#endif
