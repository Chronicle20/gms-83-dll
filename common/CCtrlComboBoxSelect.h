#include "CCtrlWnd.h"
#include "FontType.h"
#include "CCtrlComboBox.h"

struct CCtrlComboBoxSelect : CCtrlWnd {
    int m_nSelect;
    int m_nSelectMax;
    int m_bDeleteEnable;
    FONT_TYPE m_fTypeSelect;
    FONT_TYPE m_fTypeSelectFocused;
    IWzGr2DLayer *m_pLayer;
    int m_nTextOffset_Y;
    CCtrlComboBox *m_pComboBox;
};
