#pragma once

class CLife : IGObj, IVecCtrlOwner, ZRefCounted {
public:
    CChatBalloon m_chatBalloon;
    _com_ptr_t<_com_IIID<IWzGr2DLayer,&IID_IUnknown> > m_pLayerNameTag[3];
};
