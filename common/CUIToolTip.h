#pragma once

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
        bool m_bUseDotImage;
    };

    //vfptr
    int m_nToolTipType;
    int m_nHeight;
    int m_nWidth;
    IWzGr2DLayer* m_pLayer;
    IWzGr2DLayer* m_pLayerAdditional;
    int m_nLastX;
    int m_nLastY;
    int m_nLineNo;
    CUIToolTip::CLineInfo  m_aLineInfo[32];
    int m_nLineSeparated;
    int m_nOptionLineNo;
    CUIToolTip::CLineInfo  m_aOptionLineInfo[32];
    IWzFont* m_pFontHL_White;
    IWzFont* m_pFontHL_Gold;
    IWzFont* m_pFontHL_Orange;
    IWzFont* m_pFontHL_Gray;
    IWzFont* m_pFontHL_Green;
    IWzFont* m_pFontHL_Blue;
    IWzFont* m_pFontHL_Violet;
    IWzFont* m_pFontHL_Green2;
    IWzFont* m_pFontHL_Excellent;
    IWzFont* m_pFontHL_Special;
    IWzFont* m_pFontGen_White;
    IWzFont* m_pFontGen_Gray;
    IWzFont* m_pFontGen_Gray2;
    IWzFont* m_pFontGen_Red;
    IWzFont* m_pFontGen_Orange;
    IWzFont* m_pFontGen_Gold;
    IWzFont* m_pFontGen_Purple;
    IWzFont* m_pFontGen_Green;
    IWzFont* m_pFontGen_Yellow;
    IWzFont* m_pFontGen_Blue;
    IWzFont* m_pFontGen_Unknown;
    IWzFont* m_pFontH_White;
    IWzFont* m_pFontStan_Prp;
    IWzFont* m_pFontStan_Dsc;
    IWzFont* m_pFontStan_Num;
    IWzFont* m_pFontSkill_Prp;
    IWzFont* m_pFontSkill_Dsc;
    IWzCanvas* m_pCanvasEquip_ReqItem[6][2];
    IWzProperty* m_pNumberCan;
    IWzProperty* m_pNumberCannot;
    IWzCanvas* m_pCanvasEquip_JobItem[6][2];
    IWzCanvas* m_pCanvasDot[3];
    IWzCanvas* m_pCanvasEquip_GrowthItem[4][2];
    IWzProperty* m_pNumberGrowthEnable;
    IWzProperty* m_pNumberGrowthDisable;
    IWzCanvas* m_pCanvasEquip_Durability[2][2];
    bool m_bIngoreWeddingInfo;
};