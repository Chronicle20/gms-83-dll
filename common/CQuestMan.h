#pragma once

class CQuestMan {

public:
    static void CreateInstance();

    static CQuestMan *GetInstance();

    int LoadDemand();

    void LoadPartyQuestInfo();

    void LoadExclusive();
};
