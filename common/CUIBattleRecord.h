#include "CUIWnd.h"
#include "CCtrlTab.h"
#include "CCtrlCheckBox.h"

struct CUIBattleRecord : CUIWnd
{
unsigned int m_tNextUpdate;
int m_bExtended;
ZRef<CCtrlTab> m_pTab;
ZRef<CCtrlButton> m_pBtTabClear;
ZRef<CCtrlButton> m_pBtAllClear;
ZRef<CCtrlButton> m_pBtTimerSet;
ZRef<CCtrlButton> m_pBtSave;
ZRef<CCtrlButton> m_pBtRecentSaveView;
ZRef<CCtrlButton> m_pBtRecentSaveSel;
ZRef<CCtrlButton> m_pBtFold;
ZRef<CCtrlButton> m_pBtOnOff;
ZRef<CCtrlButton> m_pBtTimerStop;
ZRef<CCtrlCheckBox> m_pCBIncludeDot;
ZRef<CCtrlCheckBox> m_pCBIncludeSummon;
CUIToolTip m_uiBtToolTip;
int m_nCurSelRecentSave;
unsigned int m_tSetTime;
unsigned int m_tStopRemainTime;
int m_bToggle;
};
