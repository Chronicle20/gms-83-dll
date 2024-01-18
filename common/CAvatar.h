#pragma once
#include "AvatarLook.h"
#include "TSecType.h"
#include "ZArray.h"
#include "IWzVector2D.h"
#include "IWzGr2DLayer.h"

/*
00000000 CAvatar         struc ; (sizeof=0x1940, align=0x4, copyof_1993)
00000000 vfptr           dd ?                    ; offset
00000004 m_avatarLook    AvatarLook ?
00000209 m_avatarLookLast AvatarLook ?
0000040E                 db ? ; undefined
0000040F                 db ? ; undefined
00000410 m_aAvatarHairEquipForced dd 60 dup(?)
00000500 m_bForcingAppearance dd ?
00000504 m_nWeaponItemID dd ?
00000508 m_nSubWeaponItemID dd ?
0000050C m_nShieldItemID dd ?
00000510 m_nWalkType     dd ?
00000514 m_nStandType    dd ?
00000518 m_nAttackActionType dd ?
0000051C _ZtlSecureTear_m_nWeaponAttackSpeed dd 2 dup(?)
00000524 _ZtlSecureTear_m_nWeaponAttackSpeed_CS dd ?
00000528 m_sWeaponAfterimage Ztl_bstr_t ?
0000052C m_bBlinking     dd ?
00000530 m_tNextBlink    dd ?
00000534 m_tEmotionEnd   dd ?
00000538 _ZtlSecureTear_m_nEmotion dd 2 dup(?)
00000540 _ZtlSecureTear_m_nEmotion_CS dd ?
00000544 m_bResetEmotion dd ?
00000548 m_dwMorphTemplateID dd ?
0000054C m_rcMorphBody   tagRECT ?
0000055C m_nGhostIndex   dd ?
00000560 m_nMechanicMode dd ?
00000564 m_bRocketBoosterStart dd ?
00000568 m_bRocketBoosterLoop dd ?
0000056C m_bForcedInvisible dd ?
00000570 m_nRidingVehicleID dd ?
00000574 m_nRidingChairID dd ?
00000578 m_rcTamingMobBody tagRECT ?
00000588 m_nCharacterActionFrame dd ?
0000058C m_ptBodyRelMove SECPOINT ?
000005A4 m_bTamingMobTired dd ?
000005A8 m_nTamingMobOneTimeAction dd ?
000005AC m_nTamingMobAction dd ?
000005B0 m_bDelayedLoad  dd ?
000005B4 m_tAlertRemain  dd ?
000005B8 m_nMoveAction   dd ?
000005BC m_nOneTimeAction dd ?
000005C0 m_aiAction      CAvatar::ACTIONINFO 2 dup(?)
000018A8 m_nDefaultEmotion dd ?
000018AC m_nChairHeight  dd ?
000018B0 m_pRawOrigin    _com_ptr_t<_com_IIID<IWzVector2D,&_GUID_f28bd1ed_3deb_4f92_9eec_10ef5a1c3fb4> > ?
000018B4 m_pOrigin       _com_ptr_t<_com_IIID<IWzVector2D,&_GUID_f28bd1ed_3deb_4f92_9eec_10ef5a1c3fb4> > ?
000018B8 m_pFaceOrigin   _com_ptr_t<_com_IIID<IWzVector2D,&_GUID_f28bd1ed_3deb_4f92_9eec_10ef5a1c3fb4> > ?
000018BC m_pBodyOrigin   _com_ptr_t<_com_IIID<IWzVector2D,&_GUID_f28bd1ed_3deb_4f92_9eec_10ef5a1c3fb4> > ?
000018C0 m_pMuzzleOrigin _com_ptr_t<_com_IIID<IWzVector2D,&_GUID_f28bd1ed_3deb_4f92_9eec_10ef5a1c3fb4> > ?
000018C4 m_pTMNavelOrigin _com_ptr_t<_com_IIID<IWzVector2D,&_GUID_f28bd1ed_3deb_4f92_9eec_10ef5a1c3fb4> > ?
000018C8 m_pTMHeadOrigin _com_ptr_t<_com_IIID<IWzVector2D,&_GUID_f28bd1ed_3deb_4f92_9eec_10ef5a1c3fb4> > ?
000018CC m_pTMMuzzleOrigin _com_ptr_t<_com_IIID<IWzVector2D,&_GUID_f28bd1ed_3deb_4f92_9eec_10ef5a1c3fb4> > ?
000018D0 m_pLayerFace    _com_ptr_t<_com_IIID<IWzGr2DLayer,&_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > ?
000018D4 m_pLayerOverFace _com_ptr_t<_com_IIID<IWzGr2DLayer,&_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > ?
000018D8 m_pLayerUnderFace _com_ptr_t<_com_IIID<IWzGr2DLayer,&_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > ?
000018DC m_pLayerShadowPartner _com_ptr_t<_com_IIID<IWzGr2DLayer,&_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > ?
000018E0 m_pLayerOverCharacter _com_ptr_t<_com_IIID<IWzGr2DLayer,&_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > ?
000018E4 m_pLayerUnderCharacter _com_ptr_t<_com_IIID<IWzGr2DLayer,&_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > ?
000018E8 m_pLayerOverlay _com_ptr_t<_com_IIID<IWzGr2DLayer,&_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > ?
000018EC m_pLayerMuzzle  _com_ptr_t<_com_IIID<IWzGr2DLayer,&_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > ?
000018F0 m_pLayerBarrier _com_ptr_t<_com_IIID<IWzGr2DLayer,&_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > ?
000018F4 m_pLayerCyclone _com_ptr_t<_com_IIID<IWzGr2DLayer,&_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > ?
000018F8 m_pLayerAR01    _com_ptr_t<_com_IIID<IWzGr2DLayer,&_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > ?
000018FC m_ptPos         SECPOINT ?
00001914 m_ptPosPrev     SECPOINT ?
0000192C m_nScale        dd ?
00001930 m_bFlip         dd ?
00001934 m_wLastDayOfWeek dw ?
00001936                 db ? ; undefined
00001937                 db ? ; undefined
00001938 m_tLevitationFlowTime dd ?
0000193C m_tLevitationLastUpdateTime dd ?
00001940 CAvatar         ends
*/
class CAvatar
{
    /*
    00000000 CAvatar::ACTIONINFO struc; (sizeof = 0x974, align = 0x4, copyof_2020)
    00000000 nCurFrameIndex  dd ?
    00000004 tCurFrameRemain dd ?
    00000008 tTotFrameDelay  dd ?
    0000000C aFrameDelay     ZArray<long> ?
    00000010 bCurFrameStop   dd ?
    00000014 nCurTMFrameIndex dd ?
    00000018 tCurTMFrameRemain dd ?
    0000001C tTotTMFrameDelay dd ?
    00000020 aTMFrameDelay   ZArray<long> ?
    00000024 aaAction        ZArray < ZRef<CActionMan::CHARACTERACTIONFRAMEENTRY> > 273 dup(? )
    00000468 aaTamingMobAction ZArray < ZRef<CActionMan::TAMINGMOBACTIONFRAMEENTRY> > 273 dup(? )
    000008AC aaMorphAction   ZArray < ZRef<CActionMan::MORPHACTIONFRAMEENTRY> > 49 dup(? )
    00000970 aSPAction       ZArray<ZList<ZRef<CActionMan::SHADOWPARTNERACTIONFRAMEENTRY> > > ?
    00000974 CAvatar::ACTIONINFO ends
    */
    struct ACTIONINFO
    {
        int nCurFrameIndex;
        int tCurFrameRemain;
        int tTotFrameDelay;
        ZArray<long> aFrameDelay;
        bool bCurFrameStop;
        int nCurTMFrameIndex;
        int tCurTMFrameRemain;
        int tTotTMFrameDelay;
        ZArray<long> aTMFrameDelay;
        //aaAction
        //aaTamingMobAction
        //aaMorphAction
        //aSPAction
    };

    AvatarLook m_avatarLook;
    AvatarLook m_avatarLookLast;
    unsigned int Unknown1;
    unsigned int Unknown2;
    int m_aAvatarHairEquipForced[60];
    bool m_bForcingAppearance;
    int m_nWeaponItemID;
    int m_nSubWeaponItemID;
    int m_nShieldItemID;
    int m_nWalkType;
    int m_nStandType;
    int m_nAttackActionType;
    int _ZtlSecureTear_m_nWeaponAttackSpeed[2];
    unsigned int _ZtlSecureTear_m_nWeaponAttackSpeed_CS;
    _bstr_t m_sWeaponAfterimage;
    bool m_bBlinking;
    int m_tNextBlink;
    int m_tEmotionEnd;
    int _ZtlSecureTear_m_nEmotion[2];
    unsigned int _ZtlSecureTear_m_nEmotion_CS;
    bool m_bResetEmotion;
    unsigned long m_dwMorphTemplateID;
    RECT m_rcMorphBody;
    int m_nGhostIndex;
    int m_nMechanicMode;
    bool m_bRocketBoosterStart;
    bool m_bRocketBoosterLoop;
    bool m_bForcedInvisible;
    int m_nRidingVehicleID;
    int m_nRidingChairID;
    RECT m_rcTamingMobBody;
    int m_nCharacterActionFrame;
    SECPOINT m_ptBodyRelMove;
    bool m_bTamingMobTired;
    int m_nTamingMobOneTimeAction;
    int m_nTamingMobAction;
    bool m_bDelayedLoad;
    int m_tAlertRemain;
    int m_nMoveAction;
    int m_nOneTimeAction;
    ACTIONINFO m_aiAction[2];
    int m_nDefaultEmotion;
    int m_nChairHeight;
    IWzVector2D* m_pRawOrigin;
    IWzVector2D* m_pOrigin;
    IWzVector2D* m_pFaceOrigin;
    IWzVector2D* m_pBodyOrigin;
    IWzVector2D* m_pMuzzleOrigin;
    IWzVector2D* m_pTMNavelOrigin;
    IWzVector2D* m_pTMHeadOrigin;
    IWzVector2D* m_pTMMuzzleOrigin;
    IWzGr2DLayer* m_pLayerFace;
    IWzGr2DLayer* m_pLayerOverFace;
    IWzGr2DLayer* m_pLayerUnderFace;
    IWzGr2DLayer* m_pLayerShadowPartner;
    IWzGr2DLayer* m_pLayerOverCharacter;
    IWzGr2DLayer* m_pLayerUnderCharacter;
    IWzGr2DLayer* m_pLayerOverlay;
    IWzGr2DLayer* m_pLayerMuzzle;
    IWzGr2DLayer* m_pLayerBarrier;
    IWzGr2DLayer* m_pLayerCyclone;
    IWzGr2DLayer* m_pLayerAR01;
    SECPOINT m_ptPos;
    SECPOINT m_ptPosPrev;
    int m_nScale;
    bool m_bFlip;
    short m_wLastDayOfWeek;
    unsigned int Unknown3;
    unsigned int Unknown4;
    int m_tLevitationFlowTime;
    int m_tLevitationLastUpdateTime;
};