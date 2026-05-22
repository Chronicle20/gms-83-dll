#pragma once

class CTips {
public:
    struct TIPS_INFO {
        int nLevelMin;
        int nLevelMax;
        int nInterval;
        int nJob;
        int nAll;
        ZXString<unsigned short> sTipPath;
    };

    virtual ~CTips() = default;

    ZArray<TIPS_INFO> m_aTipsInfo;
    unsigned int m_tLastTip;
};
