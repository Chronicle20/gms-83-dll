#pragma once

#include "GW_CharacterStat.h"
#include "GW_ItemSlotBase.h"
#include "GW_MiniGameRecord.h"
#include "GW_CoupleRecord.h"
#include "GW_FriendRecord.h"
#include "GW_NewYearCardRecord.h"
#include "GW_MarriageRecord.h"
#include "GW_WildHunterInfo.h"
#include "GW_MonsterBookCard.h"
#include "EquippedSetItem.h"
#include "CSimpleStrMap.h"
#include "Additional.h"
#include "ZMap.h"
#include "ZList.h"
#include "ZRef.h"
#include "ZArray.h"
#include "ZPair.h"

struct CharacterData {
    GW_CharacterStat characterStat;
    ZRef <GW_ItemSlotBase> aEquipped[60];
    ZRef <GW_ItemSlotBase> aEquipped2[60];
    ZRef <GW_ItemSlotBase> aDragonEquipped[4];
    ZRef <GW_ItemSlotBase> aMechanicEquipped[5];
    ZArray <ZRef<GW_ItemSlotBase>> aaItemSlot[6];
    _FILETIME aEquipExtExpire[1];
    ZMap<long, EQUIPPED_SETITEM, long> m_mEquippedSetItem;
    int nCombatOrders;
    ZMap<long, long, long> mSkillRecord;
    ZMap<long, long, long> mSkillRecordEx;
    ZMap<long, long, long> mSkillMasterLev;
    ZMap<long, _FILETIME, long> mSkillExpired;
    ZMap<long, unsigned short, long> mSkillCooltime;
    ZMap<unsigned short, _FILETIME, unsigned short> mQuestComplete;
    ZMap<unsigned short, _FILETIME, unsigned short> mQuestCompleteOld;
    ZMap<long, ZRef<GW_MiniGameRecord>, long> mMiniGameRecord;
    int nFriendMax;
    ZList <GW_CoupleRecord> lCoupleRecord;
    ZList <GW_FriendRecord> lFriendRecord;
    ZList <GW_NewYearCardRecord> lNewYearCardRecord;
    ZList <GW_MarriageRecord> lMarriageRecord;
    unsigned int adwMapTransfer[5];
    unsigned int adwMapTransferEx[10];
    int bReachMaxLevel;
    _FILETIME ftReachMaxLevelTime;
    int nItemTotalNumber[5];
    ZMap<long, long, long> mAdminShopCommodityCount;
    ZXString<char> sLinkedCharacter;
    ZRef <GW_WildHunterInfo> pWildHunterInfo;
    ZMap<long, ZRef<GW_MonsterBookCard>, long> mpMonsterBookCard;
    int nMonsterBookCoverID;
    int nMonsterCardNormal;
    int nMonsterCardSpecial;
    ZMap<unsigned short, ZXString<char>, unsigned short> mQuestRecord;
    ZMap<unsigned short, CSimpleStrMap, unsigned short> mQuestRecordEx;
    ZArray <Additional::SKILL> aSkill;
    int aMobCategoryDamage[9];
    int aElemBoost[8];
    Additional::CRITICAL critical;
    Additional::BOSS boss;
    ZMap <ZXString<char>, ZPair<long, long>, ZXString<char>> aUpgradeCountByDamageTheme;
    ZMap<long, long, long> m_mVisitorQuestLog;
};
