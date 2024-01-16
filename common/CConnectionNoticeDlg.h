#pragma once
#include "CDialog.h"
#include "CCtrlButton.h"
#include "CLogin.h"

class CConnectionNoticeDlg : CDialog
{
public:
    CLogin* m_pLogin;
    ZRef<CCtrlButton> m_pBtCancel;
    IWzGr2DLayer* m_pLayerAnimationBar;

    CConnectionNoticeDlg(CLogin* pLogin);
    ~CConnectionNoticeDlg();
    void OnCreate(CConnectionNoticeDlg *, void *);
    void OnButtonClicked(CConnectionNoticeDlg *, unsigned int);
};