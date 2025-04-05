#pragma once

class CWnd : public IGObj, public IUIMsgHandler, public ZRefCounted {
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95) || defined(REGION_JMS)
    enum UIOrigin {
        Origin_LT = 0,
        Origin_CT = 1,
        Origin_RT = 2,
        Origin_LC = 3,
        Origin_CC = 4,
        Origin_RC = 5,
        Origin_LB = 6,
        Origin_CB = 7,
        Origin_RB = 8,
        Origin_NUM = 9
    };
#endif

    unsigned long m_dwWndKey;
    IWzGr2DLayer *m_pLayer;
    IWzGr2DLayer *m_pAnimationLayer;
    IWzGr2DLayer *m_pOverlabLayer;
    int m_width;
    int m_height;
    tagRECT m_rcInvalidated;
    int m_bScreenCoord;
    int m_nBackgrndX;
    int m_nBackgrndY;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 87) || defined(REGION_JMS)
    SECPOINT m_ptCursorRel;
#else
    int m_ptCursorRel_x;
    int m_ptCursorRel_y;
#endif
    ZList<ZRef<CCtrlWnd>> m_lpChildren;
    CCtrlWnd *m_pFocusChild;
    IWzCanvas *m_pBackgrnd;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95) || defined(REGION_JMS)
    UIOrigin m_origin;
#endif
};