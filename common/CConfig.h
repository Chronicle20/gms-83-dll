#pragma once

#include "asserts.h"

class CConfig {
public:
    virtual ~CConfig() = default;

    int m_bRememberMailAddr;
    ZXString<char> m_sLastMailAddr;
    ZXString<char> m_sLastConnectWorldName;
#if defined(REGION_JMS)
    int dummy5;
    int dummy1;
#endif
    int m_nScr_RRate;
    int m_bScr_FirstRun;
    int m_nPetConsumeItemID;
    int m_nPetConsumeMPItemID;
    CONFIG_GAMEOPT m_gameOpt;
    CONFIG_SYSOPT m_sysOpt;
    CONFIG_JOYPAD m_joyPad;
    ZArray<unsigned long> m_aListenBlockedFriend;
    ZArray<unsigned long> m_aTalkBlockedFriend;
    ZMap<ZXString<char>, unsigned char, ZXString<char> > m_aFriendGroup;
    int m_bShowOnlineOnly;
    ZArray<ZXString<char> > m_asBlackList;
    ZArray<long> m_aQuestAlarm;
    HKEY__ *m_hKeyGlobal;
    HKEY__ *m_hKeyLastConnected;
    HKEY__ *m_hKeyCharacter;
#if defined(REGION_GMS)
    int m_nUIWnd_X[43];
    int m_nUIWnd_Y[43];
    int m_nUIWnd_LargeX[43];
    int m_nUIWnd_LargeY[43];
    int m_nUIWnd_Option[43];
#elif defined(REGION_JMS)
    int m_nUIWnd_X[35];
    int m_nUIWnd_Y[35];
    int m_nUIWnd_Option[35];
#endif
    ZArray<unsigned char> m_abQuestInfoOption;
#if defined(REGION_GMS)
    FUNCKEY_MAPPED m_aFuncKeyMapped_Reg[89];
#elif defined(REGION_JMS)
    FUNCKEY_MAPPED m_aFuncKeyMapped_Reg[94];
#endif
    int m_aDialogVisible[1];
    int m_nLastConnectedVersion;
    int m_tStartTime;

    CConfig();

    static CConfig *GetInstance();

    INT GetPartnerCode();

    void ApplySysOpt(CONFIG_SYSOPT *pSysOpt, int bApplyVideo);

    void CheckExecPathReg(ZXString<char> sModulePath);
};

// CConfig is allocated with Alloc(sizeof(CConfig)) in CWvsApp::SetUp, then the REAL client
// ctor runs into that block — so our struct must be AT LEAST the real per-version size or
// the ctor overruns the heap (CConfig is a long-lived global, so corruption surfaces later).
// Real sizes (WinMain alloc immediate): v83=1072  v84=1084  v87=1108  v111=3024  (v95/JMS TBD).
// Our struct is v87-shaped (~1108): >= for v83/v84/v87 (safe, no overflow), but FAR too small
// for v111 (3024) -> heap overrun. (Our GMS layout has no per-major gate; the v84/v83 offsets
// are technically v87-shaped, but harmless since our code reads no GMS CConfig data member.)
#if defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 111
static_assert(sizeof(CConfig) >= 3024, "CConfig too small for GMS v111 (need >= 3024) -> WinMain heap overflow");
#elif defined(REGION_GMS) && BUILD_MAJOR_VERSION == 95
// >= not ==: 1592 is the REAL v95 client size; ours need only be at least that
// (no heap overflow). CConfig is never memcpy'd into, so exact layout isn't required.
static_assert(sizeof(CConfig) >= 1592, "CConfig smaller than the v95 real size (1592) -> heap overflow");
#elif defined(REGION_GMS)
// v83/v84/v87: our struct is v95-shaped (arrays ungated) so it's oversized for these (safe,
// no overflow) — only assert no-underflow against the smallest GMS real size.
static_assert(sizeof(CConfig) >= 1072, "CConfig smaller than any GMS real size -> heap overflow");
#endif
