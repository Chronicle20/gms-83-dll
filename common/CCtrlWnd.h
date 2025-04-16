#pragma once

class CWnd;

class CCtrlWnd : IGObj, IUIMsgHandler, ZRefCounted {
    int m_nCtrlId;
    _com_ptr_t<_com_IIID<IWzVector2D, &IID_IUnknown>> m_pLTCtrl;
    int m_width;
    int m_height;
    CWnd *m_pParent;
    bool m_bAcceptFocus;
    bool m_bEnabled;
    bool m_bShown;
};