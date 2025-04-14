#pragma once

class CDialog : public CWnd
{
    int m_nRet;
    bool m_bTerminate;
    ZRef<CDialog> m_pChildModal;

    void SetRet(CDialog*, int nRet);
};