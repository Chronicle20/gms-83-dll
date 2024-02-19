struct CCtrlTab : CCtrlWnd {
    struct TABINFO {
        int bCanvas;
        int nStart;
        int nEnd;
        ZXString<unsigned short> sTabNameNormal;
        ZXString<unsigned short> sTabNameSelected;
        IWzCanvas *pCanvasNormal;
        IWzCanvas *pCanvasSelected;
        int bEnabled;
    };

    int m_bSameWidth;
    int m_nTabWidth;
    int m_bDrawBaseImage;
    int m_nTabSpace;
    int m_nCurTab;
    ZList<CCtrlTab::TABINFO> m_lTabInfo;
    IWzFont *m_pFontNormal;
    IWzFont *m_pFontSelected;
    int m_nInsertPos;
    int m_nType;
    int m_nHeight;
};
