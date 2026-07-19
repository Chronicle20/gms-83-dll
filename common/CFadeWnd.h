#pragma once

#include "asserts.h"
#include <cstddef>

class CFadeWnd : public CDialog
{
  public:
    int m_a0;
    int m_a;
    int m_a1;
    int m_t0;
    int m_t;
    int m_t1;
    POINT m_pt0;
    POINT m_pt;
    POINT m_pt1;
    int m_nPhase;
    int m_tPhase;
    int m_tTimeOver;
#if defined(REGION_GMS) && (BUILD_MAJOR_VERSION == 72 || BUILD_MAJOR_VERSION == 79)
    // v79/v72: these three flags are 4-byte BOOLs in the binary (dwords @0xB0/0xB4/0xB8),
    // not packed C++ bool. CFadeWnd::CFadeWnd (v79 @0x50b6d6, v72 @0x4ffd72 —
    // byte-identical) writes [this+0xB0]=0 (m_bClose) and [this+0xBC]=-1 (m_nType);
    // the 0xC gap between them is exactly 3 dwords. Gated ==72/==79 so other GMS/JMS
    // versions stay byte-for-byte unchanged.
    int m_bClose;
    int m_bUserAlarm;
    int m_bOK;
#else
    bool m_bClose;
#if defined(REGION_GMS)
    bool m_bUserAlarm;
#endif
    bool m_bOK;
#endif
    int m_nType;
    ZXString<char> m_sInviter;
#if (defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 87) || defined(REGION_JMS)
    int m_nLevel;
    int m_nJobCode;
    int m_nExpQuestID;
#endif
    unsigned long m_dwSN;
    unsigned long m_dwFriendID;

    void SetOption(CFadeWnd*, int a0, int a, int a1, POINT pt0, POINT pt, POINT pt1, int t0, int t, int t1);
    void Close(CFadeWnd*, int bOK);
};

#if defined(REGION_GMS) && (BUILD_MAJOR_VERSION == 72 || BUILD_MAJOR_VERSION == 79)
assert_size(sizeof(CFadeWnd), 0xCC); // CDialog(0x74) base; ctor v79 @0x50b6d6 / v72 @0x4ffd72
static_assert(offsetof(CFadeWnd, m_nType) == 0xBC,
              "CFadeWnd::m_nType @0xBC (ctor: or [this+0xBC], -1; v79 @0x50b6d6, v72 @0x4ffd72)");
static_assert(offsetof(CFadeWnd, m_dwSN) == 0xC4,
              "CFadeWnd::m_dwSN @0xC4 (>=87 m_nLevel/m_nJobCode/m_nExpQuestID block absent)");
#endif