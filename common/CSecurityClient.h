#pragma once

struct CSecurityClient {
public:

#if defined(REGION_GMS)
    TSecType<int> m_bInitModule;
    TSecType<int> m_bStartModule;
#endif
    int m_nThreatCode;
#if defined(REGION_GMS)
    TSecType<long> m_nThreatParamSize;
    void *m_pThreatParam;
    unsigned int m_dwCallbackTime;
    char m_szHShieldPath[260];
#endif
    HWND__ *m_hMainWnd;

    static void CreateInstance();

    static CSecurityClient *GetInstance();
};