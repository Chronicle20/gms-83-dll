#pragma once

#include "CStage.h"
#include "ZXString.h"
#include "ZList.h"
#include "ZArray.h"
#include "ZRef.h"
#include "ZMap.h"
#include "ZRefCounted.h"
#include "IWzProperty.h"
#include "IWzGr2DLayer.h"
#include "IWzCanvas.h"
#include <comdef.h>

/*
00000000 CMapLoadable    struc; (sizeof = 0x148, align = 0x4, copyof_3709)
00000000 baseclass_0     CStage ?
00000018 m_nJukeBoxItemID dd ?
0000001C m_tNextMusic    dd ?
00000020 m_bJukeBoxPlaying dd ?
00000024 m_unWeatherSoundCookie dd ?
00000028 m_sChangedBgmUOL ZXString<unsigned short> ?
0000002C m_pPropFieldInfo _com_ptr_t<_com_IIID<IWzProperty, &_GUID_986515d9_0a0b_4929_8b4f_718682177b92> > ?
00000030 m_pPropField    _com_ptr_t<_com_IIID<IWzProperty, &_GUID_986515d9_0a0b_4929_8b4f_718682177b92> > ?
00000034 m_bField        dd ?
00000038 m_pSpace2D      ZRef<CWvsPhysicalSpace2D> ?
00000040 m_lpLayerGen    ZList<_com_ptr_t<_com_IIID<IWzGr2DLayer, &_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > > ?
00000054 m_lpLayerObj    ZList<_com_ptr_t<_com_IIID<IWzGr2DLayer, &_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > > ?
00000068 m_lpLayerTransient ZList<_com_ptr_t<_com_IIID<IWzGr2DLayer, &_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > > ?
0000007C m_lpObstacle    ZList<ZRef<CMapLoadable::OBSTACLE> > ?
00000090 m_lpRefInfo     ZList<ZRef<CMapLoadable::REFLECTION_INFO> > ?
000000A4 m_lVisibleByQuest ZList<CMapLoadable::VISIBLE_BY_QUEST> ?
000000B8 m_mNamedObj     ZMap<char const*, CMapLoadable::CHANGING_OBJECT, ZXString<char> > ?
000000D0 m_mTagedObj     ZMap<char const*, ZRef<ZList<_com_ptr_t<_com_IIID<IWzGr2DLayer, &_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > > >, ZXString<char> > ?
000000E8 m_mlLayerBack   ZMap<long, ZRef<ZList<_com_ptr_t<_com_IIID<IWzGr2DLayer, &_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > > >, long> ?
00000100 m_lpLayerLetterBox ZList<_com_ptr_t<_com_IIID<IWzGr2DLayer, &_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > > ?
00000114 m_nMagLevel_Obj dd ?
00000118 m_nMagLevel_Back dd ?
0000011C m_rcViewRange   tagRECT ?
0000012C m_bSysOptTremble dd ?
00000130 m_bMagLevelModifying dd ?
00000134 m_aObstacleInfo ZArray<CMapLoadable::OBSTACLE_INFO> ?
00000138 m_tRestoreBgmVolume dd ?
0000013C m_nRestoreBgmVolume dd ?
00000140 m_bPlayHoldedBGM dd ?
00000144 m_tPlayHoldedBGM dd ?
00000148 CMapLoadable    ends
*/
class CMapLoadable : CStage {
public:
    /*
    00000000 CMapLoadable::OBSTACLE struc; (sizeof = 0x38, align = 0x4, copyof_3663)
    00000000 baseclass_0     ZRefCounted ?
    0000000C pLayer          _com_ptr_t<_com_IIID<IWzGr2DLayer, &_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > ?
    00000010 bFlip           dd ?
    00000014 nDamage         dd ?
    00000018 nMobDamage      dd ?
    0000001C nDirection      dd ?
    00000020 nMobSkillID     dd ?
    00000024 nSLV            dd ?
    00000028 nForceHP        dd ?
    0000002C sName           ZXString<char> ?
    00000030 dwTargetField   dd ?
    00000034 bSafeZoneByMob  dd ?
    00000038 CMapLoadable::OBSTACLE ends
    */
    struct OBSTACLE :ZRefCounted {
        IWzGr2DLayer *pLayer;
        bool bFlip;
        int nDamage;
        int nMobDamage;
        int nDirection;
        int nMobSkillID;
        int nSLV;
        int nForceHP;
        ZXString<char> sName;
        unsigned int dwTargetField;
        bool bSafeZoneByMob;
    };

    /*
    00000000 CMapLoadable::OBSTACLE_INFO struc; (sizeof = 0x1C, align = 0x4, copyof_3667)
    00000000 rcObs           tagRECT ?
    00000010 vecForce        tagPOINT ?
    00000018 pObstacle       dd ?
    0000001C CMapLoadable::OBSTACLE_INFO ends
    */
    struct OBSTACLE_INFO {
        RECT rcObs;
        POINT vecForce;
        OBSTACLE *pObstacle;
    };

    /*
    00000000 CMapLoadable::OBJECT_STATE struc ; (sizeof=0x10, align=0x4, copyof_3664)
    00000000 nRepeat         dd ?
    00000004 bsSfx           Ztl_bstr_t ?
    00000008 pLayer          _com_ptr_t<_com_IIID<IWzGr2DLayer,&_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > ?
    0000000C bRestartMoving  dd ?
    00000010 CMapLoadable::OBJECT_STATE ends
    */
    struct OBJECT_STATE {
        int nRepeat;
        _bstr_t bsSfx;
        IWzGr2DLayer *pLayer;
        bool bRestartMoving;
    };

    /*
    00000000 CMapLoadable::CHANGING_OBJECT struc; (sizeof = 0xC, align = 0x4, copyof_3666)
    00000000 nState          dd ?
    00000004 dwSN            dd ?
    00000008 aState          ZArray<CMapLoadable::OBJECT_STATE> ?
    0000000C CMapLoadable::CHANGING_OBJECT ends
    */
    struct CHANGING_OBJECT {
        int nState;
        unsigned int dwSN;
        ZArray<CMapLoadable::OBJECT_STATE> aState;
    };

    /*
    00000000 CMapLoadable::REFLECTION_INFO struc; (sizeof = 0x28, align = 0x4, copyof_3668)
    00000000 pLayer          _com_ptr_t<_com_IIID<IWzGr2DLayer, &_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > ?
    00000004 pOriginalCanvas _com_ptr_t<_com_IIID<IWzCanvas, &_GUID_7600dc6c_9328_4bff_9624_5b0f5c01179e> > ?
    00000008 pAvatarCanvas   _com_ptr_t<_com_IIID<IWzCanvas, &_GUID_7600dc6c_9328_4bff_9624_5b0f5c01179e> > ?
    0000000C pRemoverCanvas  _com_ptr_t<_com_IIID<IWzCanvas, &_GUID_7600dc6c_9328_4bff_9624_5b0f5c01179e> > ?
    00000010 rcArea          tagRECT ?
    00000020 nAlpha          dd ?
    00000024 bLastFrameUpdated dd ?
    00000028 CMapLoadable::REFLECTION_INFO ends
    */
    struct REFLECTION_INFO {
        IWzGr2DLayer *pLayer;
        IWzCanvas *pOriginalCanvas;
        IWzCanvas *pAvatarCanvas;
        IWzCanvas *pRemoverCanvas;
        RECT rcArea;
        int Alpha;
        bool bLastFrameUpdated;
    };

    virtual ~CMapLoadable() = default;

    int m_nJukeBoxItemID;
    int m_tNextMusic;
    int m_bJukeBoxPlaying;
    unsigned int m_unWeatherSoundCookie;
    ZXString<unsigned short> m_sChangedBgmUOL;
    IWzProperty *m_pPropFieldInfo;
    IWzProperty *m_pPropField;
    int m_bField;
    int *m_pSpace2D; // ZRef<CWvsPhysicalSpace2D>
    ZList<IWzGr2DLayer *> m_lpLayerGen;
    ZList<IWzGr2DLayer *> m_lpLayerObj;
    ZList<IWzGr2DLayer *> m_lpLayerTransient;
    ZList<ZRef<CMapLoadable::OBSTACLE>> m_lpObstacle;
    ZList<ZRef<CMapLoadable::REFLECTION_INFO> > m_lpRefInfo;
    //ZList<CMapLoadable::VISIBLE_BY_QUEST> m_lVisibleByQuest; not in v83
    ZMap<char const *, CMapLoadable::CHANGING_OBJECT, ZXString<char>> m_mNamedObj;
    ZMap<char const *, ZRef<ZList<IWzGr2DLayer *>>, ZXString<char> > m_mTagedObj;
    ZMap<long, ZRef<ZList<IWzGr2DLayer *>>, long> m_mlLayerBack;
    //ZList<IWzGr2DLayer*> m_lpLayerLetterBox; not in v83
    int m_nMagLevel_Obj;
    int m_nMagLevel_Back;
    tagRECT m_rcViewRange;
    int m_bSysOptTremble;
    int m_bMagLevelModifying;
    ZArray<CMapLoadable::OBSTACLE_INFO> m_aObstacleInfo;
    int m_tRestoreBgmVolume;
    int m_nRestoreBgmVolume;
    //int m_bPlayHoldedBGM; not in v83
    //int m_tPlayHoldedBGM; not in v83
};