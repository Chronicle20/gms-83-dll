#include "CCtrlWnd.h"
#include "ZXString.h"

struct CCtrlScrollBar : CCtrlWnd {
    int m_nWheelRange;
    int m_nCurPos;
    int m_nScrollRange;
    int m_nLastHT;
    unsigned int m_dwLastHT;
    int m_bCapture;
    int m_grid;
    int m_length;
    int m_hv;
    int m_type;
    int m_thumbPos;
    int m_thumbX;
    int m_thumbY;
    int m_bHideThumb;
    int m_bTranslucent;
    ZXString<unsigned short> m_sUOL;
};
