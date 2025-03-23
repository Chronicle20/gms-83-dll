#pragma once

struct CSecurityClient {
public:
    

    static void CreateInstance();

    static CSecurityClient *GetInstance();
};