#include "CCtrlWnd.h"
#include "ZXString.h"
#include "ZArray.h"
#include "IWzFont.h"
#include "IWzCanvas.h"
#include "CWnd.h"

class CCtrlComboBox{};

struct CCtrlEdit : CCtrlWnd {
    struct CEditCaret {
        int m_l;
        int m_t;
        unsigned int m_dwLastSet;
        int m_bInit;
        int m_height;
        IWzGr2DLayer *m_pLayerCaret;
    };

    ZXString<char> m_sText;
    tagPOINT m_ptText;
    tagPOINT m_ptCaret;
    int m_col;
    int m_ext;
    ZArray<unsigned long> m_adwIMECompClause;
    int m_nCurClause;
    int m_nCaretX;
    int m_selCol;
    int m_nViewportX;
    int m_nViewportWidth;
    IWzFont *m_pFont;
    IWzFont *m_pFontDisabled;
    IWzFont *m_pFontSel;
    IWzFont *m_pFontCand;
    IWzFont *m_pFontCandSel;
    int m_nFontHeight;
    int m_nBackColor;
    int m_bPasswd;
    int m_bReadOnly;
    int m_nHorzMax;
    int m_bNumber;
    IWzCanvas *m_pCanvasEmptyText;
    CCtrlEdit::CEditCaret m_editCaret;
    ZRef<CWnd> m_pIMECandWnd;
    CCtrlComboBox *m_pParentComboBox;
};
