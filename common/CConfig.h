#pragma once

class CConfig {
public:
    virtual ~CConfig() = default;

    int m_bRememberMailAddr;
    ZXString<char> m_sLastMailAddr;
    ZXString<char> m_sLastConnectWorldName;
    int dummy5;
    int m_nScr_RRate;
    int dummy1;
    int dummy2;
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
    int m_nUIWnd_X[35];
    int m_nUIWnd_Y[35];
    int m_nUIWnd_Option[35];
    ZArray<unsigned char> m_abQuestInfoOption;
    FUNCKEY_MAPPED m_aFuncKeyMapped_Reg[94];
    int m_aDialogVisible[1];
    int m_nLastConnectedVersion;
    int m_tStartTime;

    CConfig();

    static CConfig *GetInstance();

    INT GetPartnerCode();

    void ApplySysOpt(int *pSysOpt, int bApplyVideo);

    void CheckExecPathReg(ZXString<char> sModulePath);
};
