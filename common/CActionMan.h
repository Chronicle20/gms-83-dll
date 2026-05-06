#pragma once

class CActionMan {
public:
    struct MOBACTIONFRAMEENTRY : ZRefCounted {
        _com_ptr_t<_com_IIID<IWzCanvas, &IID_IUnknown> > pCanvas;
        SECRECT rcBody;
        ZArray<SECRECT> arcMultiBody;
        ZArray<SECRECT> arcAttackOnlyBody;
        tagPOINT ptHead;
        int tDelay;
        int a0;
        int a1;
    };

    static void CreateInstance();

    static CActionMan *GetInstance();

    void Init();

    void SweepCache();
};
