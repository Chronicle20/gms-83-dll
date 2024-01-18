#pragma once

class CMonsterBookMan {

public:
    static void CreateInstance();

    static CMonsterBookMan *GetInstance();

    bool LoadBook();
};
