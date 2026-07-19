#pragma once

#include "asserts.h"
#include <cstddef>

/*
00000000 CUIToolTip      struc; (sizeof = 0xA48, align = 0x4, copyof_1535)
00000000 vfptr           dd ? ; offset
00000004 m_nToolTipType  dd ?
00000008 m_nHeight       dd ?
0000000C m_nWidth        dd ?
00000010 m_pLayer        _com_ptr_t<_com_IIID<IWzGr2DLayer, &_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > ?
00000014 m_pLayerAdditional _com_ptr_t<_com_IIID<IWzGr2DLayer, &_GUID_6dc8c7ce_8e81_4420_b4f6_4b60b7d5fcdf> > ?
00000018 m_nLastX        dd ?
0000001C m_nLastY        dd ?
00000020 m_nLineNo       dd ?
00000024 m_aLineInfo     CUIToolTip::CLineInfo 32 dup(? )
000004A4 m_nLineSeparated dd ?
000004A8 m_nOptionLineNo dd ?
000004AC m_aOptionLineInfo CUIToolTip::CLineInfo 32 dup(? )
0000092C m_pFontHL_White _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
00000930 m_pFontHL_Gold  _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
00000934 m_pFontHL_Orange _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
00000938 m_pFontHL_Gray  _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
0000093C m_pFontHL_Green _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
00000940 m_pFontHL_Blue  _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
00000944 m_pFontHL_Violet _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
00000948 m_pFontHL_Green2 _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
0000094C m_pFontHL_Excellent _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
00000950 m_pFontHL_Special _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
00000954 m_pFontGen_White _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
00000958 m_pFontGen_Gray _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
0000095C m_pFontGen_Gray2 _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
00000960 m_pFontGen_Red  _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
00000964 m_pFontGen_Orange _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
00000968 m_pFontGen_Gold _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
0000096C m_pFontGen_Purple _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
00000970 m_pFontGen_Green _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
00000974 m_pFontGen_Yellow _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
00000978 m_pFontGen_Blue _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
0000097C m_pFontGen_Unknown _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
00000980 m_pFontH_White  _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
00000984 m_pFontStan_Prp _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
00000988 m_pFontStan_Dsc _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
0000098C m_pFontStan_Num _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
00000990 m_pFontSkill_Prp _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
00000994 m_pFontSkill_Dsc _com_ptr_t<_com_IIID<IWzFont, &_GUID_2bef046d_ccd6_445a_88c4_929fc35d30ac> > ?
00000998 m_pCanvasEquip_ReqItem _com_ptr_t < _com_IIID<IWzCanvas, &_GUID_7600dc6c_9328_4bff_9624_5b0f5c01179e> > 12 dup(? )
000009C8 m_pNumberCan    _com_ptr_t<_com_IIID<IWzProperty, &_GUID_986515d9_0a0b_4929_8b4f_718682177b92> > ?
000009CC m_pNumberCannot _com_ptr_t<_com_IIID<IWzProperty, &_GUID_986515d9_0a0b_4929_8b4f_718682177b92> > ?
000009D0 m_pCanvasEquip_JobItem _com_ptr_t < _com_IIID<IWzCanvas, &_GUID_7600dc6c_9328_4bff_9624_5b0f5c01179e> > 12 dup(? )
00000A00 m_pCanvasDot    _com_ptr_t < _com_IIID<IWzCanvas, &_GUID_7600dc6c_9328_4bff_9624_5b0f5c01179e> > 3 dup(? )
00000A0C m_pCanvasEquip_GrowthItem _com_ptr_t < _com_IIID<IWzCanvas, &_GUID_7600dc6c_9328_4bff_9624_5b0f5c01179e> > 8 dup(? )
00000A2C m_pNumberGrowthEnable _com_ptr_t<_com_IIID<IWzProperty, &_GUID_986515d9_0a0b_4929_8b4f_718682177b92> > ?
00000A30 m_pNumberGrowthDisable _com_ptr_t<_com_IIID<IWzProperty, &_GUID_986515d9_0a0b_4929_8b4f_718682177b92> > ?
00000A34 m_pCanvasEquip_Durability _com_ptr_t < _com_IIID<IWzCanvas, &_GUID_7600dc6c_9328_4bff_9624_5b0f5c01179e> > 4 dup(? )
00000A44 m_bIngoreWeddingInfo dd ?
00000A48 CUIToolTip      ends
*/
class CUIToolTip
{
    /*
    00000000 CUIToolTip::CLineInfo struc; (sizeof = 0x24, align = 0x4, copyof_1537)
    00000000 m_nWidth        dd ?
    00000004 m_nHeight       dd ?
    00000008 m_nType         dd ?
    0000000C m_sContext      ZXString<char> ?
    00000010 m_nAlign        dd ?
    00000014 m_bMulti        dd ?
    00000018 m_nSubType      dd ?
    0000001C m_sSubContext   ZXString<char> ?
    00000020 m_bUseDotImage  dd ?
    00000024 CUIToolTip::CLineInfo ends
    */
    struct CLineInfo
    {
        int m_nHeight;
        int m_nWidth;
        int m_nType;
        ZXString<char> m_sContext;
        int m_nAlign;
        bool m_bMulti;
        int m_nSubType;
        ZXString<char> m_sSubContext;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
        bool m_bUseDotImage;
#endif
    };

  public:
#if defined(REGION_GMS) && (BUILD_MAJOR_VERSION == 72 || BUILD_MAJOR_VERSION == 79)
    // v79/v72: the binary reserves a dword at offset 0 (IDA struct labels it "vfptr").
    // This class is non-polymorphic and does not otherwise model that slot, leaving
    // the struct 4 bytes short of the binary size. Both ctors (v79 @0x842317,
    // v72 @0x7f9c33) write m_pLayer@0x10 first and the m_aLineInfo array @0x20 --
    // i.e. exactly 4 dwords precede m_pLayer, so the offset-0 slot is real. Gated
    // ==72/==79 so v83/84/87/95/111/JMS stay byte-for-byte unchanged (they share
    // this latent -4; see task-008 report).
    void* vfptr;
#endif
    //vfptr
    int m_nToolTipType;
    int m_nHeight;
    int m_nWidth;
    _com_ptr_t<_com_IIID<IWzGr2DLayer, &IID_IUnknown>> m_pLayer;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 83) || defined(REGION_JMS)
    _com_ptr_t<_com_IIID<IWzGr2DLayer, &IID_IUnknown>> m_pLayerAdditional;
#endif
    int m_nLastX;
    int m_nLastY;
    int m_nLineNo;
    CUIToolTip::CLineInfo  m_aLineInfo[32];
    int m_nLineSeparated;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95) || defined(REGION_JMS)
    int m_nOptionLineNo;
    CUIToolTip::CLineInfo  m_aOptionLineInfo[32];
#endif
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontHL_White;
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontHL_Gold;
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontHL_Orange;
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontHL_Gray;
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontHL_Green;
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontHL_Blue;
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontHL_Violet;
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontHL_Green2;
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontHL_Excellent;
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontHL_Special;
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontGen_White;
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontGen_Gray;
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontGen_Gray2;
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontGen_Red;
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontGen_Orange;
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontGen_Gold;
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontGen_Purple;
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontGen_Green;
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontGen_Yellow;
    // v72: this last base-font slot is ABSENT. The v72 ctor @0x7f9c33 zeroes the
    // font run [esi+424h..esi+46Ch] (19 dwords) whereas the v79 ctor @0x842317
    // zeroes [esi+424h..esi+470h] (20 dwords) — one extra pointer appended at
    // +0x470. Offsets 0x424..0x46C are byte-identical between the two, so the
    // delta is strictly this trailing slot. Absent only for GMS v72 -> CUIToolTip
    // 0x510 (v72) vs 0x514 (v79). task-009.
#if !(defined(REGION_GMS) && BUILD_MAJOR_VERSION == 72)
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontGen_Blue;
#endif
    // v84: m_pFontGen_Unknown present @0x478 — gate split >=87 -> >=84, task-006 (v84=0x52C)
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 84) || defined(REGION_JMS)
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontGen_Unknown;
#endif
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 87) || defined(REGION_JMS)
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontH_White;
#endif
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 87)
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontStan_Prp;
#endif
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 95)
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontStan_Dsc;
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontStan_Num;
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontSkill_Prp;
    _com_ptr_t<_com_IIID<IWzFont, &IID_IUnknown>> m_pFontSkill_Dsc;
#endif
    _com_ptr_t<_com_IIID<IWzCanvas, &IID_IUnknown>> m_pCanvasEquip_ReqItem[6][2];
    _com_ptr_t<_com_IIID<IWzProperty, &IID_IUnknown>> m_pNumberCan;
    _com_ptr_t<_com_IIID<IWzProperty, &IID_IUnknown>> m_pNumberCannot;

#if defined(REGION_GMS)
    _com_ptr_t<_com_IIID<IWzCanvas, &IID_IUnknown>> m_pCanvasEquip_JobItem[6][2];
    _com_ptr_t<_com_IIID<IWzCanvas, &IID_IUnknown>> m_pCanvasDot[3];
#endif
    _com_ptr_t<_com_IIID<IWzCanvas, &IID_IUnknown>> m_pCanvasEquip_GrowthItem[4][2];
    _com_ptr_t<_com_IIID<IWzProperty, &IID_IUnknown>> m_pNumberGrowthEnable;
    _com_ptr_t<_com_IIID<IWzProperty, &IID_IUnknown>> m_pNumberGrowthDisable;
    // v84: present @0x518 — gate >=87 -> >=84, task-006 (v84=0x52C)
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 84) || defined(REGION_JMS)
    _com_ptr_t<_com_IIID<IWzCanvas, &IID_IUnknown>> m_pCanvasEquip_Durability[2][2];
#endif
    bool m_bIngoreWeddingInfo;
};

#if defined(REGION_GMS) && BUILD_MAJOR_VERSION == 79
assert_size(sizeof(CUIToolTip), 0x514); // ctor @0x842317 (GMS_v79_1_DEVM, port 13340)
static_assert(offsetof(CUIToolTip, m_pLayer) == 0x10,
              "v79 CUIToolTip::m_pLayer @0x10 (ctor @0x842317: mov [esi+10h], edi)");
static_assert(offsetof(CUIToolTip, m_pNumberCan) == 0x4A4,
              "v79 CUIToolTip::m_pNumberCan @0x4A4 (ctor @0x842317: lea ecx, [esi+4A4h])");
#elif defined(REGION_GMS) && BUILD_MAJOR_VERSION == 72
assert_size(sizeof(CUIToolTip), 0x510); // ctor @0x7f9c33 (GMS_v72.1_U_DEVM); one fewer base font than v79
static_assert(offsetof(CUIToolTip, m_pLayer) == 0x10,
              "v72 CUIToolTip::m_pLayer @0x10 (ctor @0x7f9c33: mov [esi+10h], edi)");
static_assert(offsetof(CUIToolTip, m_pNumberCan) == 0x4A0,
              "v72 CUIToolTip::m_pNumberCan @0x4A0 (ctor @0x7f9c33: mov [esi+4A0h], edi) -- 4 below v79");
#endif