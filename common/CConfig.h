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
#if defined(REGION_GMS) && BUILD_MAJOR_VERSION >= 111
    // v111's real CConfig is 3024 bytes vs our v95-shaped layout (>= 1592, guaranteed by the
    // passing v95 build). We read NO CConfig data member and it's never memcpy'd, so exact v111
    // layout is unnecessary — only no-underflow matters (the real ctor runs into Alloc(sizeof)).
    // Pad the 1592 floor up to the v111 real size so the ctor can't overrun the heap. If our
    // struct is already > 1592 this lands harmlessly above 3024 (the gate is >=, not ==).
    char m_v111Pad[3024 - 1592];
#endif

    CConfig();

    static CConfig *GetInstance();

    INT GetPartnerCode();

    void ApplySysOpt(CONFIG_SYSOPT *pSysOpt, int bApplyVideo);

    void CheckExecPathReg(ZXString<char> sModulePath);
};

// CConfig is allocated with Alloc(sizeof(CConfig)) in CWvsApp::SetUp, then the REAL client
// ctor runs into that block — so our struct must be AT LEAST the real per-version size or
// the ctor overruns the heap (CConfig is a long-lived global, so corruption surfaces later).
// Real sizes (WinMain alloc immediate): v79=1048 (0x418)  v83=1072  v84=1084  v87=1108  v95=1592
// v111=3024 (JMS TBD). NOTE: v79 (1048) is the true GMS floor — below the 1072 in the assert below;
// the >= assert is still safe because our struct is v95-shaped (>= 1592 >> 1048). verified task-008
// Our GMS layout is v95-shaped (>= 1592: the [43] UI-window arrays + FUNCKEY[89]); it has no
// per-major gate, so the v83/v84/v87 offsets are technically v95-shaped — harmless, since our code
// reads no GMS CConfig data member and it's never memcpy'd. The v111 branch adds m_v111Pad to lift
// that 1592 floor up to the v111 real size (3024) so its larger ctor can't overrun the heap.
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
