#pragma once

class CActionMan {

public:
    static void CreateInstance();

    static CActionMan *GetInstance();

    void Init();

    void SweepCache();
};