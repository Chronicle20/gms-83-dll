#pragma once

class CChatBalloon {
public:
    virtual ~CChatBalloon() = default;

    _com_ptr_t<_com_IIID<IWzGr2DLayer, &IID_IUnknown> > m_pLayerChat;
    _com_ptr_t<_com_IIID<IWzGr2DLayer, &IID_IUnknown> > m_pLayerAD;
    _com_ptr_t<_com_IIID<IWzGr2DLayer, &IID_IUnknown> > m_pLayerGameState;
    _com_ptr_t<_com_IIID<IWzGr2DLayer, &IID_IUnknown> > m_pLayerShop;
    _com_ptr_t<_com_IIID<IWzGr2DLayer, &IID_IUnknown> > m_pLayerBack;
    _com_ptr_t<_com_IIID<IWzGr2DLayer, &IID_IUnknown> > m_pLayerEffect;
    _com_ptr_t<_com_IIID<IWzCanvas, &IID_IUnknown> > m_pButton[4];
    int m_nADPosX;
    int m_nADPosY;
    int m_nADButtonPosX;
    int m_nADButtonPosY;
    int m_bADButtonPressed;
    int m_tChatBegin;
    int m_tTimeOut;
    int m_nPosX;
    int m_nPosY;
    int m_nHeight;
    int m_bMiniRoomBalloon;
    int m_bAbbreviated;
    int m_nCanvasWidth;
    int m_nCanvasHeight;
    int m_tFadeDalay;
};
