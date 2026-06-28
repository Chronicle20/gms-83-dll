#pragma once

class CLife : IGObj, IVecCtrlOwner, ZRefCounted {
  public:
    CChatBalloon m_chatBalloon;
    // v61: CLife base is -0x4 vs v72 (sizeof 0x80 vs 0x84) — one trailing 4-byte member absent.
    // Task 15b byte-located it at the END of the CLife base (CMob's first own member
    // m_nMobChargeCount @v61 0x80 / v72 0x84); the member is NOT symbol-pinned, so it is modeled
    // here as the last m_pLayerNameTag element (the contiguous 4B at the base tail). The v72+/JMS
    // branch (#else) keeps [3], byte-identical. byte-region split — FLAGGED, verified task-010
#if defined(REGION_GMS) && BUILD_MAJOR_VERSION < 72
    _com_ptr_t<_com_IIID<IWzGr2DLayer, &IID_IUnknown>> m_pLayerNameTag[2];
#else
    _com_ptr_t<_com_IIID<IWzGr2DLayer, &IID_IUnknown>> m_pLayerNameTag[3];
#endif
#if (defined(REGION_JMS))
    int m_effectAttacktStart;
    int m_effectAttackbLeft;
#endif
};
