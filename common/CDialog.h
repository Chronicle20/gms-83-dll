#pragma once

/*
00000000 CDialog         struc; (sizeof = 0x90, align = 0x4, copyof_1529)
00000000 baseclass_0     CWnd ?
00000080 m_nRet          dd ?
00000084 m_bTerminate    dd ?
00000088 m_pChildModal   ZRef<CDialog> ?
00000090 CDialog         ends
*/
class CDialog : public CWnd
{
    int m_nRet;
    bool m_bTerminate;
    ZRef<CDialog> m_pChildModal;

    void SetRet(CDialog*, int nRet);
};