#pragma once

/*
00000000 CFadeWnd        struc ; (sizeof=0xF4, align=0x4, copyof_3078)
00000000 baseclass_0     CDialog ?
00000090 m_a0            dd ?
00000094 m_a             dd ?
00000098 m_a1            dd ?
0000009C m_t0            dd ?
000000A0 m_t             dd ?
000000A4 m_t1            dd ?
000000A8 m_pt0           tagPOINT ?
000000B0 m_pt            tagPOINT ?
000000B8 m_pt1           tagPOINT ?
000000C0 m_nPhase        dd ?
000000C4 m_tPhase        dd ?
000000C8 m_tTimeOver     dd ?
000000CC m_bClose        dd ?
000000D0 m_bUserAlarm    dd ?
000000D4 m_bOK           dd ?
000000D8 m_nType         dd ?
000000DC m_sInviter      ZXString<char> ?
000000E0 m_nLevel        dd ?
000000E4 m_nJobCode      dd ?
000000E8 m_nExpQuestID   dd ?
000000EC m_dwSN          dd ?
000000F0 m_dwFriendID    dd ?
000000F4 CFadeWnd        ends
*/
class CFadeWnd : CDialog
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
    int m_nLevel;
    int m_nJobCode;
    int m_nExpQuestID;
    unsigned long m_dwSN;
    unsigned long m_dwFriendID;

    void SetOption(CFadeWnd*, int a0, int a, int a1, POINT pt0, POINT pt, POINT pt1, int t0, int t, int t1);
    void Close(CFadeWnd*, int bOK);
};