#pragma once

#include "asserts.h"

class CWnd;

class CCtrlWnd : IGObj, IUIMsgHandler, ZRefCounted {
    int m_nCtrlId;
    _com_ptr_t<_com_IIID<IWzVector2D, &IID_IUnknown>> m_pLTCtrl;
    int m_width;
    int m_height;
    CWnd *m_pParent;
#if (defined(REGION_GMS) && (BUILD_MAJOR_VERSION == 61 || BUILD_MAJOR_VERSION == 72 || BUILD_MAJOR_VERSION == 79))
    // v61/v79/v72: the 3 trailing flags are 4-byte in the binary. CCtrlWnd::CCtrlWnd
    // (v61 @0x4b2783, v79 @0x4d4378, v72 @0x4cc645 — byte-identical) writes
    // *((_DWORD*)this+10/11/12)=1 at 0x28/0x2C/0x30 -> sizeof 0x34. Modeling them
    // as packed bool (below) yields 0x2C (-8), which shifts every CCtrlWnd-derived
    // class (CCtrlButton, CCtrlCheckBox). task-008 (v79); task-009 (v72); task-010 (v61).
    // (v83+ likely share this modeling error; unverified here -> follow-up.)
    int m_bAcceptFocus;
    int m_bEnabled;
    int m_bShown;
#else
    bool m_bAcceptFocus;
    bool m_bEnabled;
    bool m_bShown;
#endif
};

#if (defined(REGION_GMS) && (BUILD_MAJOR_VERSION == 61 || BUILD_MAJOR_VERSION == 72 || BUILD_MAJOR_VERSION == 79))
assert_size(sizeof(CCtrlWnd),
            0x34); // ctor v61 @0x4b2783 (task-010), v79 @0x4d4378 (task-008), v72 @0x4cc645 (task-009)
#endif