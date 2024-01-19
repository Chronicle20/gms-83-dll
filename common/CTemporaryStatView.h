#include "UInt128.h"

class CTemporaryStatView {
public:
    ~CTemporaryStatView() = default;

    struct TEMPORARY_STAT : ZRefCounted {
        UINT128 uFlagTemp;
        int nType;
        int nID;
        int nSubID;
        ZXString<char> sToolTip;
        IWzGr2DLayer *pLayer;
        IWzGr2DLayer *pLayerShadow;
        int nIndexShadow;
        int tHideTime;
        int bNoShadow;
        int tLeft;
        int tLeftUnit;
    };

    ZList<ZRef<CTemporaryStatView::TEMPORARY_STAT>> m_lTemporaryStat;
};
