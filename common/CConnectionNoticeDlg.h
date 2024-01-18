#pragma once

class CConnectionNoticeDlg : CDialog
{
public:
    char gap90;
    CLogin* m_pLogin;
    ZRef<CCtrlButton> m_pBtCancel;
    _com_ptr_t<_com_IIID<IWzGr2DLayer, &IID_IUnknown>> m_pLayerAnimationBar;
};