#pragma once


class CMob : CLife {
public:
    struct ATTACKEFFECT {
        int tStart;
        int bLeft;
        _bstr_t sEffect;
    };

    struct AFFECTEDSKILLENTRY {
        int nSkillID;
        int tStart;
        __POSITION *posList;
        int bIcon;
        int bFlip;
        _com_ptr_t<_com_IIID<IWzGr2DLayer, &IID_IUnknown> > pLayer;
    };

    struct ATTACKENTRY {
        int tTime;
        int nType;
        int ___u2[4];
        int bLeft;
        int nAttackIdx;
    };

    struct DAMAGEINFO {
        int _ZtlSecureTear_tDelayedProcess[2];
        unsigned int _ZtlSecureTear_tDelayedProcess_CS;
        unsigned int _ZtlSecureTear_dwCharacterId[2];
        unsigned int _ZtlSecureTear_dwCharacterId_CS;
        int _ZtlSecureTear_nSkillID[2];
        unsigned int _ZtlSecureTear_nSkillID_CS;
        int _ZtlSecureTear_nHitAction[2];
        unsigned int _ZtlSecureTear_nHitAction_CS;
        int _ZtlSecureTear_bLeft[2];
        unsigned int _ZtlSecureTear_bLeft_CS;
        int _ZtlSecureTear_nDamage[2];
        unsigned int _ZtlSecureTear_nDamage_CS;
        int _ZtlSecureTear_bCriticalAttack[2];
        unsigned int _ZtlSecureTear_bCriticalAttack_CS;
        int _ZtlSecureTear_nAttackIdx[2];
        unsigned int _ZtlSecureTear_nAttackIdx_CS;
        int _ZtlSecureTear_bChase[2];
        unsigned int _ZtlSecureTear_bChase_CS;
        int _ZtlSecureTear_nMoveType[2];
        unsigned int _ZtlSecureTear_nMoveType_CS;
        int _ZtlSecureTear_nBulletCashItemID[2];
        unsigned int _ZtlSecureTear_nBulletCashItemID_CS;
        int _ZtlSecureTear_nMoveEndingPosX[2];
        unsigned int _ZtlSecureTear_nMoveEndingPosX_CS;
        int _ZtlSecureTear_nMoveEndingPosY[2];
        unsigned int _ZtlSecureTear_nMoveEndingPosY_CS;
        int _ZtlSecureTear_bMoveLeft[2];
        unsigned int _ZtlSecureTear_bMoveLeft_CS;
    };

    struct HITEFFECT {
        int tHit;
        int nSkillID;
        int bLeft;
        int bSfx;
        _bstr_t sHitUOL;
        tagPOINT ptRel;
    };

    struct DROPPICKUP {
        unsigned int dwDropID;
        int tLastTry;
    };

    struct DelaySkill {
        int tSkillDelayTime;
        int nSkillID;
        int nSLV;
        int nOption;
    };

    struct ReservedPacket {
        int bSet;
        UINT128 uFlag;
        CInPacket iPacket;
    };

    struct MobBullet : CFadeoutBullet {
    public:
        struct Container : BulletContainer<MobBullet> {
        };

        int m_nZ;
        int m_bLeft;
        _bstr_t m_sBallUOL;
        int m_nAttackIdx;
    };


    int m_nMobChargeCount;
    int m_bAttackReady;
    int m_nAngerGaugeCount;
    int m_nUpdateTime;
    int m_nCurrIndicatorIndex;
    int m_bFullChargeEffectTime;
    int m_bFullChargeEffectStartTime;
    ATTACKEFFECT m_effectAttack;
    ZList<AFFECTEDSKILLENTRY> m_lAffectedSkillEntry;
    ZList<ATTACKENTRY> m_lAttackEntry;
    ZList<_com_ptr_t<_com_IIID<IWzGr2DLayer, &IID_IUnknown> > > m_lpLayerASAni;
    ZList<_com_ptr_t<_com_IIID<IWzGr2DLayer, &IID_IUnknown> > > m_lpLayerASIcon;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION < 95) || defined(REGION_JMS)
    ZList<_com_ptr_t<_com_IIID<IWzGr2DLayer, &IID_IUnknown> > > unknown1;
#endif
    __POSITION *m_posAffectedGuidedBullet;
    _com_ptr_t<_com_IIID<IWzVector2D, &IID_IUnknown> > m_pvc;
    _com_ptr_t<_com_IIID<IWzVector2D, &IID_IUnknown> > m_pvcActive;
    _com_ptr_t<_com_IIID<IWzVector2D, &IID_IUnknown> > m_pvcHead;
    int _ZtlSecureTear_m_bInViewSplit[2];
    unsigned int _ZtlSecureTear_m_bInViewSplit_CS;
    int _ZtlSecureTear_m_nMobCtrlState[2];
    unsigned int _ZtlSecureTear_m_nMobCtrlState_CS;
    __int16 _ZtlSecureTear_m_nMobCtrlSN[2];
    unsigned int _ZtlSecureTear_m_nMobCtrlSN_CS;
    int m_nSkillCommand;
    int m_nSLV;
    int m_nSummonType;
    int m_tSummonEffect;
    int m_tDoomEffectEnd;
    int _ZtlSecureTear_m_tLastApplyCtrl[2];
    unsigned int _ZtlSecureTear_m_tLastApplyCtrl_CS;
    int _ZtlSecureTear_m_tLastTryPickUpDrop[2];
    unsigned int _ZtlSecureTear_m_tLastTryPickUpDrop_CS;
    int m_tLastAreaAttack;
    int m_bDoFirstAttack;
    unsigned int m_uLayerStateCounter;
    unsigned int _ZtlSecureTear_m_dwMobID[2];
    unsigned int _ZtlSecureTear_m_dwMobID_CS;
    CMobTemplate *m_pTemplate;
    CMobTemplate *m_pTemplateByDoom;
    int _ZtlSecureTear_m_nMP[2];
    unsigned int _ZtlSecureTear_m_nMP_CS;
    MobStat m_stat;
    RANGE m_rgHorz;
    int m_nTeamForMCarnival;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95) || defined(REGION_JMS)
    int m_nPhase;
#endif
    int _ZtlSecureTear_m_nMoveAction[2];
    unsigned int _ZtlSecureTear_m_nMoveAction_CS;
    int _ZtlSecureTear_m_nOneTimeAction[2];
    unsigned int _ZtlSecureTear_m_nOneTimeAction_CS;
    int _ZtlSecureTear_m_tHitExpire[2];
    unsigned int _ZtlSecureTear_m_tHitExpire_CS;
    int _ZtlSecureTear_m_tLastHitExpire[2];
    unsigned int _ZtlSecureTear_m_tLastHitExpire_CS;
    __POSITION *m_posFrame;
    int _ZtlSecureTear_m_tCurFrameRemain[2];
    unsigned int _ZtlSecureTear_m_tCurFrameRemain_CS;
    int _ZtlSecureTear_m_tNextFramesRemain[2];
    unsigned int _ZtlSecureTear_m_tNextFramesRemain_CS;
    int _ZtlSecureTear_m_tActionDelay[2];
    unsigned int _ZtlSecureTear_m_tActionDelay_CS;
    tagRECT m_rcBody;
    tagRECT m_rcBodyFlip;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95) || defined(REGION_JMS)
    ZArray<tagRECT> m_arcMultiBody;
    ZArray<tagRECT> m_arcMultiBodyFlip;
#endif
    ZArray<tagRECT> m_arcAttackBody;
    ZArray<tagRECT> m_arcAttackBodyFlip;
    ZArray<ZList<ZRef<CActionMan::MOBACTIONFRAMEENTRY> > > m_aAction;
    int _ZtlSecureTear_m_tInitDelay[2];
    unsigned int _ZtlSecureTear_m_tInitDelay_CS;
    int _ZtlSecureTear_m_nDeadType[2];
    unsigned int _ZtlSecureTear_m_nDeadType_CS;
    int _ZtlSecureTear_m_nSuspended[2];
    unsigned int _ZtlSecureTear_m_nSuspended_CS;
    int m_tLastPoisonDamage;
    int m_tLastVenomDamage;
    int m_tLastAmbushDamage;
    int m_tLastObstacleDamage;
    int m_tLastHitByMob;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95) || defined(REGION_JMS)
    int m_tLastHitDazzledMob;
#endif
    ZList<unsigned long> m_ldwRevive;
    ZList<DAMAGEINFO> m_lDamageInfo;
    ZList<HITEFFECT> m_lHitEffect;
    ZList<DROPPICKUP> m_lDropPickUpLog;
    _com_ptr_t<_com_IIID<IWzGr2DLayer, &IID_IUnknown> > m_pLayerAction;
    int m_nCalcDamageStatIndex;
    _com_ptr_t<_com_IIID<IWzGr2DLayer, &IID_IUnknown> > m_pLayerHPTag;
    _com_ptr_t<_com_IIID<IWzGr2DLayer, &IID_IUnknown> > m_pEffectLayer;
    _com_ptr_t<_com_IIID<IWzGr2DLayer, &IID_IUnknown> > m_pLayerAngerTag;
    int m_bNeedToUpdateCrc;
    unsigned int m_dwMobCrc;
    int m_tLastHitted;
    _com_ptr_t<_com_IIID<IWzCanvas, &IID_IUnknown> > m_pCanvasHPIndicator;
    ZMap<long, long, long> m_mDelayedHPIndicator;
    ZArray<long> m_pCanvasAngerIndicatorArrayCount;
    ZArray<ZArray<_com_ptr_t<_com_IIID<IWzCanvas, &IID_IUnknown> > > > m_pCanvasAngerIndicatorArray;
    int m_nGaugeCount;
    ZRef<CAttrShoe> m_pAttrShoe;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95) || defined(REGION_JMS)
    SECPOINT m_ptPos;
    SECPOINT m_ptPosPrev;
#else
    int m_ptPoxX;
    int m_ptPoxY;
    int m_ptPoxXPrev;
    int m_ptPoxYPrev;
#endif
    int m_nHPpercentage;
    int m_bWaitingToBeSetTossed;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95) || defined(REGION_JMS)
    ZArray<tagPOINT> m_aMultiTargetForBall;
    ZArray<long> m_aRandTimeforAreaAttack;
    DelaySkill m_delaySkill;
#endif
    int m_bDoomReserved;
    unsigned __int8 m_bDoomReservedSN;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95) || defined(REGION_JMS)
    ZList<ZRef<ReservedPacket> > m_lpStatChangeReserved;
    TSecType<int> m_bChasing;
    int m_tTimeBomb;
    unsigned int m_dwSwallowCharacterID;
    unsigned int m_dwTargetMobID;
    int m_tEscortStopActTime;
    int m_nEscortStopAct;
    int m_nDamagedByMobHPBarState;
    int m_nRushAttackIdx;
    int m_tRushAttackEnd;
    MobBullet::Container m_Bullets;
#endif
};
