#pragma once

class CFadeWnd : public CDialog
{
    int m_a0;
    int m_a;
    int m_a1;
    int m_t0;
    int m_t;
    int m_t1;
    POINT m_pt0;
    POINT m_pt;
    POINT m_pt1;
    int m_nPhase;
    int m_tPhase;
    int m_tTimeOver;
    bool m_bClose;
    bool m_bUserAlarm;
    bool m_bOK;
    int m_nType;
    ZXString<char> m_sInviter;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 87) || defined(REGION_JMS)
    int m_nLevel;
    int m_nJobCode;
    int m_nExpQuestID;
#endif
    unsigned long m_dwSN;
    unsigned long m_dwFriendID;

    void SetOption(CFadeWnd*, int a0, int a, int a1, POINT pt0, POINT pt, POINT pt1, int t0, int t, int t1);
    void Close(CFadeWnd*, int bOK);
};