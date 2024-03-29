
class CUITitle : public CFadeWnd {
    CLogin *m_pLogin;
    int m_bRememberMailAddr;
    IWzCanvas* m_pCanvasRMA[2];
    tagRECT m_rcRMA;
    ZRef <CCtrlButton> m_pBtLogin;
    ZRef <CCtrlButton> m_pBtEmailSave;
    ZRef <CCtrlButton> m_pBtEmailLost;
    ZRef <CCtrlButton> m_pBtPasswdLost;
    ZRef <CCtrlButton> m_pBtNew;
    ZRef <CCtrlButton> m_pBtHomePage;
    ZRef <CCtrlButton> m_pBtQuit;
    ZRef <CCtrlEdit> m_pEditID;
    ZRef <CCtrlEdit> m_pEditPasswd;
    CUIToolTip m_uiToolTipTitle;

public:
    static CUITitle* GetInstance();
};
