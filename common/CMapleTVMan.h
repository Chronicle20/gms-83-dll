#pragma once

class CMapleTVMan {

public:
    static void CreateInstance();

    static CMapleTVMan *GetInstance();

#if defined(REGION_GMS)
    void Init();
#elif defined(REGION_JMS)
    void Init(int something, int somethingElse);
#endif
};
