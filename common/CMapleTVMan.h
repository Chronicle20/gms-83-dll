#pragma once

class CMapleTVMan {

public:
    static void CreateInstance();

    static CMapleTVMan *GetInstance();

    //void Init();
    void Init(int something, int somethingElse);
};
