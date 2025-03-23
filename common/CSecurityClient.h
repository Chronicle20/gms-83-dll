#pragma once

struct CSecurityClient {
public:
    
    int dummy1;
    HWND__ *m_hMainWnd;

    static void CreateInstance();

    static CSecurityClient *GetInstance();
};