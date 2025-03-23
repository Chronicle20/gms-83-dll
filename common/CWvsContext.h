#pragma once

class CWvsContext {
public:
    virtual ~CWvsContext() = default;

    struct CFriend {
        ZArray<GW_Friend> m_aFriend;
        ZArray<int> m_aInShop;
        ZArray<int> m_aListenBlocked;
        ZArray<int> m_aTalkBlocked;
    };

    struct Massacre {
        int nHit;
        int nMiss;
        int nCool;
        int nSkill;
    };

#if defined(REGION_GMS) && MAJOR_VERSION >= 95
    int m_bFirstUserLoad;
#endif
    int m_bAvatarMegaphone;
    int m_tAM_LastUpdate;
#if defined(REGION_GMS)
    int m_nTargetPosition_X;
    int m_nTargetPosition_Y;
    int m_bChaseEnable;
#endif
#if defined(REGION_GMS) && MAJOR_VERSION >= 87
    int m_bPetHelpPopUpShown;
#endif
    WEBCOOKIE m_Cookie;
    int m_bIsCookieValid;
    int m_bCookieLoadedByArgString;
    int m_nLoginBaseStep;
    unsigned int m_dwAccountId;
    int m_nGender;
    TSecType<unsigned char> m_nGradeCode;
#if defined(REGION_GMS)
    TSecType<unsigned char> m_nSubGradeCode;
#elif defined(REGION_JMS)
    int unk1;
    int unk2;
    int m_nNumOfCharacter;
#endif
    ZXString<char> m_sEMailAccount;
    ZXString<char> m_sNexonClubID;
#if defined(REGION_GMS)
    unsigned __int8 m_nCountryID;
#endif
    unsigned __int8 m_nPurchaseExp;
    int m_nWorldID;
    int m_nChannelID;
    int m_bPremium;
    unsigned int m_ulPremiumArgument;
    unsigned __int8 m_nChatBlockReason;
    _SYSTEMTIME m_dtChatUnblockDate;
#if defined(REGION_GMS)
    _SYSTEMTIME m_dtRegisterDate;
    int m_nNumOfCharacter;
    int m_bThisAccountJustCreatedCharacter;
    int m_bIsGuestAccount;
    int m_bManagerAccount;
    int m_nCharacterCount;
    int m_nSlotCount;
#elif defined(REGION_JMS)
    int m_bManagerAccount;
    int m_bTesterAccount;
    int unk3;
#endif
#if (defined(REGION_GMS) && MAJOR_VERSION > 83) || (defined(REGION_JMS))
    unsigned __int8 m_aClientKey[8];
#endif
#if defined(REGION_GMS) && MAJOR_VERSION >= 87
    int m_bTesterAccount;
#endif
    unsigned int m_dwCharacterId;
    int m_bExclRequestSent;
    int m_tExclRequestSent;
    int m_tExclRequestSentQ[2];
    // TODO this is as far as has been verified.
    ZRef<CharacterData> m_pCharacterData;
    BasicStat m_basicStat;
    SecondaryStat m_secondaryStat;
    ForcedStat m_forcedStat;
    CTemporaryStatView m_temporaryStatView;
    PARTYDATA::TOWNPORTAL m_townPortal;
    int m_nActiveEffectItemID;
    int m_nPartyID;
    PARTYDATA m_party;
    ZRef<CWvsContext::CFriend> m_pFriendArray;
    unsigned int m_dwMarriedPartnerCurFieldID;
    int m_nMarriedPartnerID;
    PARTYSERACH_SETTING m_PartySearch_Setting;
    int m_nPartySearch_State;
    int m_bKeep_PartySearch;
    int m_bWasRadioUICleared;
    int m_bWasMute;
    ZRef<CPartySearch_RemoCon> m_pPartySearch_Remocon;
    IWzGr2DLayer *m_pPartySearchLayer_Searching;
    IWzGr2DLayer *m_pPartySearchLayer_Holding;
    GUILDDATA m_guild;
    ZXString<unsigned short> m_sGuildBoardAuthkey;
    unsigned int m_dwGuildBoardAuthkeyLastUpdated;
    ZXString<unsigned short> m_sConsultAuthkey;
    unsigned int m_dwConsultAuthkeyLastUpdated;
    ZXString<unsigned short> m_sClassCompetitionAuthkey;
    unsigned int m_dwClassCompetitionAuthkeyLastUpdated;
    ZXString<unsigned short> m_sWebBoardAuthKey[1];
    unsigned int m_dwWebBoardAuthkeyLastUpdated[1];
    ALLIANCEDATA m_alliance;
    ZArray<GUILDDATA> m_AllianceMember;
    int m_bDirectionMode;
    int m_bStandAloneMode;
    ZXString<char> m_sBattleTeamName;
    unsigned __int8 m_nClaimSvrOpenTime;
    unsigned __int8 m_nClaimSvrCloseTime;
    int m_bClaimSvrConnected;
    int m_bPersonalShopOpen;
    int m_bADBoard;
    ZXString<char> m_sADBoard;
    ZRef<GW_ItemSlotBase> m_aRealEquip[60];
    ZRef<GW_ItemSlotBase> m_aRealEquip2[60];
    ZRef<GW_ItemSlotBase> m_aRealDragonEquip[4];
    ZRef<GW_ItemSlotBase> m_aRealMechanicEquip[5];
    CalcDamage m_CalcDamage;
    TSecType<long> m_tRestForHPDuration;
    TSecType<long> m_tRestForMPDuration;
    TSecType<long> m_tRestForMPDurationOnPortableChair;
    TSecType<long> m_tRestForHPDurationOnPortableChair;
    TSecType<long> m_tRestForHPDurationItemOption;
    TSecType<long> m_tRestForMPDurationItemOption;
    int m_tReviveDialog;
    int m_tLastGivePopularity;
    int m_tLastEmotionChange;
    int m_tLastEffectItemChange;
    int m_tLastStatResetRequest;
    int m_tLastFollowCharacterRequest;
    unsigned int m_dwOldDriverID;
    unsigned int m_dwFollowRequesterID;
    CRand32 m_RndActionMan;
    ZXString<char> m_sWeekEventMessage;
    int m_bWeekEventMessagePrinted;
    int m_nPotionDiscountRate;
    int m_tLastSueCharacter;
    int *m_pUserPool; // ZRef<CUserPool>
    int *m_pSummonedPool; // ZRef<CSummonedPool>
    int *m_pMobPool; // ZRef<CMobPool>
    int *m_pNpcPool; // ZRef<CNpcPool>
    int *m_pEmployeePool; // ZRef<CEmployeePool>
    int *m_pDropPool; // ZRef<CDropPool>
    int *m_pMessageBoxPool; // ZRef<CMessageBoxPool>
    int *m_pAffectedAreaPool; // ZRef<CAffectedAreaPool>
    int *m_pTownPortalPool; // ZRef<CTownPortalPool>
    int *m_pOpenGatePool; // ZRef<COpenGatePool>
    int *m_pReactorPool; // ZRef<CReactorPool>
    int *m_pPortalList; // ZRef<CPortalList>
    int *m_pUIItem; // ZRef<CUIItem>
    int *m_pUIEquip; // ZRef<CUIEquip>
    int *m_pUIStat; // ZRef<CUIStat>
    int *m_pUISkill; // ZRef<CUISkill>
    int *m_pUISkillEx; // ZRef<CUISkillEx>
    int *m_pUIKeyConfig; // ZRef<CUIKeyConfig>
    int *m_pUIUserList; // ZRef<CUIUserList>
    int *m_pUIQuestInfo; // ZRef<CUIQuestInfo>
    int *m_pUIMedalQuestInfo; // ZRef<CUIMedalQuestInfo>
    int *m_pUIUserInfo; // ZRef<CUIUserInfo>
    int *m_pUIQuestAlarm; // ZRef<CUIQuestAlarm>
    int *m_pUIGuildBBS; // ZRef<CUIGuildBBS>
    int *m_pAvatarMegaphone; // ZRef<CAvatarMegaphone>
    int *m_pUIMonsterCarnival; // ZRef<CUIMonsterCarnival>
    int *m_pUIEnergyBar; // ZRef<CUIEnergyBar>
    int *m_pUIRaiseManager; // ZRef<CUIRaiseManager>
    int *m_pUIMonsterBook; // ZRef<CUIMonsterBook>
    int *m_pUIPartySearch; // ZRef<CUIPartySearch>
    int *m_pUIItemMaker; // ZRef<CUIItemMaker>
    int *m_pUIRanking; // ZRef<CUIRanking>
    int *m_pUIFamily; // ZRef<CUIFamily>
    int *m_pUIFamilyChart; // ZRef<CUIFamilyChart>
    int *m_pUIOperatorBoard; // ZRef<CUIOperatorBoard>
    int *m_pUIOperatorBoardState; // ZRef<CUIOpBoardState>
    int *m_pUIDragonBox; // ZRef<CUIDragonBox>
    bool m_bIsOperatorBoardState;
    int m_nWebOpBoardIndex;
    ZRef<CUIBattleRecord> m_pUIBattleRecord;
    ZXString<char> m_sWebOpBoardURL;
//    ZRef<CUIAccountMoreInfo> m_pUIAccountMoreInfo;
//    ZRef<CUIFindFriend> m_pUIFindFriend;
//    ZArray<ZRef<CUIFadeYesNo>> m_apFadeWnd;
//    ZRef<CNoticeQuestProgress> m_pNoticeQuestProgress;
    int m_bShowUI;
//    ZList<GW_Memo> m_lReceivedMemo;
    int m_bOnReadingMemo;
    int m_bIsExistMemo_NotLoaded;
//    ZList<ZRef<CUIQuestTimer>> m_lpUIQuestTimer;
//    CTips m_tips;
//    ZRef<CClock> m_pClock;
    int m_nLastMobBonusEventPercentage;
    ZArray<ZXString<char>> m_aChannelName;
    ZArray<int> m_aAdultChannel;
//    ZArray<ZRef<CS_COMMODITY>> m_aOriginalCommodity;
//    ZArray<ZRef<CS_COMMODITY>> m_aCommodity;
//    ZArray<CS_LIMITGOODS> m_aLimitGoods;
    int m_bMigrateFromWishItem;
    int m_nCommSN;
    ZXString<char> m_sGiveTo;
    ZXString<char> m_sMapTransferTargetUserName;
    int m_nPreStartQuestCount;
    int m_tRemainAntiMacroQuestion;
    int m_tRemainInitialQuiz;
    int m_nEmployeeItemPos;
    int m_nEmployeeItemID;
    int m_tNextNoticePlaytime;
    int m_nPlaytimeHour;
    ZMap<long, long, long> m_mSkillCooltimeOver;
    int m_nDarkForceDamage;
    int m_nDarkForcePddr;
    int m_nDragonFury;
    ZMap<unsigned short, ZXString<char>, unsigned short> m_mQuestMatesName;
    int m_bTvVisionRegion;
    int m_bCurTvView;
    ZMap<unsigned short, int, unsigned short> m_mAutoStartQuestPreStart;
    ZMap<unsigned short, int, unsigned short> m_mAutoAcceptQuestRequest;
    ZMap<unsigned short, int, unsigned short> m_mAutoCompleteQuestInProgress;
    int m_bNewPreStartQuest;
    ZList<unsigned short> m_lNewPreStartQuestIDs;
    int m_bLevelUpAutoQuestRequetSent;
    unsigned int m_tAutoAcceptQuestRequest;
    ZList<unsigned short> m_lAutoCompletionAlertQuest;
    int m_bNewAutoCompletionAlertQuest;
    int m_nQuestDeliveryItemPos;
    unsigned __int16 m_usDeliveryQuestID;
    int m_nTamingMobLevel;
    int m_nTamingMobExp;
    int m_nTamingMobFatigue;
    int m_bCommodityLoadedCompletely;
    IWzProperty *m_pCommodity;
    IWzProperty *m_pCashPackage;
    int m_bShowMobInfoName;
    int m_bShowMobInfoHP;
    ZArray<CUIWnd *> m_apStackForTab;
    int m_bPredictQuit;
    int m_anLogoutGiftCommoditySN[3];
    int m_bIsFakeGMNotice;
    int m_bRecentPickUpEntrance;
    int m_bKillMobFromEnterField;
    int m_bADSpaceON;
    int m_tNextUrgingForLevelUp;
    int m_bMiniMapOnOff;
    int m_nCashShopInitialItem;
    int m_nEnergy;
    //ZArray<ZRef<PrivilegeItem>> m_apPrivilege;
    //Privilege m_privilege;
    //FamilyInfo m_FamilyInfo;
    ZXString<char> m_sUnregisterCharacterName;
    ZMap<unsigned long, ZRef<ZList<_FILETIME> >, unsigned long>
            m_mExpireProtectingCheckedItem;
    int m_tExpireProtectingItemChecked;
    int m_nCookieHousePoint;
    int m_bBuyEquipExt;
    ZMap<long, ZXString<char>, long> m_mCashPackageName;
    int m_tLastUpdateFileTime;
    _FILETIME m_ftLastUpdate;
    CWvsContext::Massacre m_Massacre;
    //PartyRaidTeam m_nTeamForPartyRaid;
    int m_nPartyRaidStageMine;
    int m_nPartyRaidStageOther;
    int m_nPartyRaidPoint;
    int m_nLastestGetItemID;
    int m_nLastestGetItemPos;
    int m_bBambooUsed;
    ZXString<char> m_strImpactNextBySessionValueKey;
    ZXString<char> m_strImpactNextBySessionValue;
    long double m_dImpactNextBySessionValueVX;
    long double m_dImpactNextBySessionValueVY;
    int m_nScreenWidth;
    int m_nScreenHeight;
    int m_nAdjustCenterY;
    bool m_bIsLargeScreen;
    unsigned __int8 m_nDoubleJumpChatCtrl;
    int m_aPasssiveSkillBuffing[22];
    //ZList<CWvsContext::ITEMMSG> m_lItemMsg;
    int m_tNextCheckItemMsg;
    unsigned __int16 m_usWorldMapQuestID;
    ZList<unsigned long> m_lWorldMapQuestMobList;
    //ZArray<WORLDMAPQUESTDEMANDITEM> m_aWorldMapQuestDemandItem;
    int m_bShowOnlyWorthyQuests;

    //TODO
    static CWvsContext *GetInstance();
};
