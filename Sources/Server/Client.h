// Client.h: interface for the CClient class.

#pragma once

// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)

#include "Platform.h"
#include "CommonTypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ASIOSocket.h"
#include "Item.h"
#include "GuildsMan.h"
#include "Magic.h"
#include "GlobalDef.h"
#include "NetConstants.h"
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include "GameGeometry.h"
#include "Appearance.h"
#include "PlayerStatusData.h"
using namespace std;

namespace hb::server::config { constexpr int ClientSocketBlockLimit = 2000; } // Queue size per client

// hb::shared::limits::MaxMagicType and hb::shared::limits::MaxSkillType are defined in NetConstants.h

namespace hb::server::config { constexpr int SpecialAbilityTimeSec = 1200; }

class CClient  
{
public:

	int m_iAngelicStr, m_iAngelicInt, m_iAngelicDex, m_iAngelicMag;
		
	char m_cVar;
	int m_iRecentWalkTime;
	int m_iRecentRunTime;
	short m_sV1;
	char m_cHeroArmourBonus;

	bool bCreateNewParty();

	// Hack Checkers
	uint32_t m_dwMagicFreqTime, m_dwMoveFreqTime, m_dwAttackFreqTime;
	bool m_bIsMoveBlocked, m_bMagicItem;
	uint32_t dwClientTime;
	bool m_bMagicConfirm;
	int m_iSpellCount;
	bool m_bMagicPauseTime;
	//int m_iUninteruptibleCheck;
	//char m_cConnectionCheck;

	bool m_bIsClientConnected;
	uint32_t m_dwLastMsgId;
	uint32_t m_dwLastMsgTime;
	size_t m_dwLastMsgSize;
	uint32_t m_dwLastFullObjectId;
	uint32_t m_dwLastFullObjectTime;

	CClient(asio::io_context& ctx);
	virtual ~CClient();

	char m_cCharName[hb::shared::limits::CharNameLen];
	char m_cAccountName[hb::shared::limits::AccountNameLen];
	char m_cAccountPassword[hb::shared::limits::AccountPassLen];

	bool  m_bIsInitComplete;
	bool  m_bIsMsgSendAvailable;

	char  m_cMapName[11];
	char  m_cMapIndex;
	short m_sX, m_sY;
	
	char  m_cGuildName[21];
	char  m_cLocation[11];
	int   m_iGuildRank;
	int   m_iGuildGUID;
	
	char  m_cDir;
	short m_sType;
	short m_sOriginalType;
	hb::shared::entity::PlayerAppearance m_appearance;
	hb::shared::entity::PlayerStatus m_status;

	uint32_t m_dwTime, m_dwHPTime, m_dwMPTime, m_dwSPTime, m_dwAutoSaveTime, m_dwHungerTime, m_dwWarmEffectTime;
	uint32_t m_dwAfkActivityTime;
	// Player

	char m_cSex, m_cSkin, m_cHairStyle, m_cHairColor, m_cUnderwear;

	int  m_iHP;						// Hit Point
	int  m_iHPstock;
	int  m_iMP;
	int  m_iSP;
	uint32_t  m_iExp;
	uint32_t m_iNextLevelExp;
	bool m_bIsKilled;

	int  m_iDefenseRatio;		// Defense Ratio
	int  m_iHitRatio;			// Hit Ratio

	// int  m_iHitRatio_ItemEffect_SM; //    HitRatio
	//int  m_iHitRatio_ItemEffect_L;

	int  m_iDamageAbsorption_Armor[hb::shared::item::DEF_MAXITEMEQUIPPOS];		// Damage
	int  m_iDamageAbsorption_Shield;	// Parrying ?  Damage

	int  m_iLevel;
	int  m_iStr, m_iInt, m_iVit, m_iDex, m_iMag, m_iCharisma;
	// char m_cLU_Str, m_cLU_Int, m_cLU_Vit, m_cLU_Dex, m_cLU_Mag, m_cLU_Char;   //  ?  .
	int  m_iLuck; 
	int  m_iLU_Pool;
	char m_cAura;
	//MOG var - 3.51
	int m_iGizonItemUpgradeLeft;

	int m_iAddTransMana, m_iAddChargeCritical;


	uint32_t m_iRewardGold;
	int  m_iEnemyKillCount, m_iPKCount;
	int  m_iCurWeightLoad;

	char m_cSide;
	
	bool m_bInhibition;

	//50Cent - Repair All
	short totalItemRepair;
	struct
	{
		char index;
		short price;
	} m_stRepairAll[hb::shared::limits::MaxItems];

	char m_cAttackDiceThrow_SM;
	char m_cAttackDiceRange_SM;
	char m_cAttackDiceThrow_L;
	char m_cAttackDiceRange_L;
	char m_cAttackBonus_SM;
	char m_cAttackBonus_L;

	CItem * m_pItemList[hb::shared::limits::MaxItems];
	hb::shared::geometry::GamePoint m_ItemPosList[hb::shared::limits::MaxItems];
	CItem * m_pItemInBankList[hb::shared::limits::MaxBankItems];
	
	bool  m_bIsItemEquipped[hb::shared::limits::MaxItems];
	short m_sItemEquipmentStatus[hb::shared::item::DEF_MAXITEMEQUIPPOS];
	char  m_cArrowIndex;		// ?   .  -1( )

	char           m_cMagicMastery[hb::shared::limits::MaxMagicType];
	unsigned char  m_cSkillMastery[hb::shared::limits::MaxSkillType]; // v1.4

	int   m_iSkillSSN[hb::shared::limits::MaxSkillType];
	bool  m_bSkillUsingStatus[hb::shared::limits::MaxSkillType];
	int   m_iSkillUsingTimeID[hb::shared::limits::MaxSkillType]; //v1.12

	char  m_cMagicEffectStatus[hb::server::config::MaxMagicEffects];

	int   m_iWhisperPlayerIndex;
	char  m_cProfile[256];

	int   m_iHungerStatus;

	uint32_t m_dwWarBeginTime;
	bool  m_bIsWarLocation;

	bool  m_bIsPoisoned;
	int   m_iPoisonLevel;
	uint32_t m_dwPoisonTime;
	
	int   m_iPenaltyBlockYear, m_iPenaltyBlockMonth, m_iPenaltyBlockDay; // v1.4

	int   m_iFightzoneNumber , m_iReserveTime, m_iFightZoneTicketNumber ; 

	class hb::shared::net::ASIOSocket * m_pXSock;

	int   m_iRating;

	int   m_iTimeLeft_Rating;
	int   m_iTimeLeft_ForceRecall;
	int   m_iTimeLeft_FirmStaminar;

	bool isForceSet;   //hbest
	time_t m_iForceStart;

	bool  m_bIsOnServerChange;

	uint32_t   m_iExpStock;
	uint32_t m_dwExpStockTime;		 // ExpStock ? .

	uint32_t   m_iAutoExpAmount;		 // Auto-Exp
	uint32_t m_dwAutoExpTime;		 // Auto-Exp ? .

	uint32_t m_dwRecentAttackTime;

	int   m_iAllocatedFish;
	int   m_iFishChance;
	
	char  m_cIPaddress[21];		 // IP address
	bool  m_bIsSafeAttackMode;

	bool  m_bIsOnWaitingProcess;
	
	int   m_iSuperAttackLeft;
	int   m_iSuperAttackCount;

	short m_sUsingWeaponSkill;

	int   m_iManaSaveRatio;
	
	bool  m_bIsLuckyEffect;
	int   m_iSideEffect_MaxHPdown;

	int   m_iComboAttackCount;
	int   m_iDownSkillIndex;

	int   m_iMagicDamageSaveItemIndex;

	short m_sCharIDnum1, m_sCharIDnum2, m_sCharIDnum3;

	int   m_iAbuseCount;
	
	// bool  m_bIsExchangeMode;		//    ??
	// int   m_iExchangeH;				//  ?
	// char  m_cExchangeName[11];		//  ?
	// char  m_cExchangeItemName[21];	//
	// char  m_cExchangeItemIndex;  //
	// int   m_iExchangeItemAmount; //
	// bool  m_bIsExchangeConfirm;  //

	bool  m_bIsExchangeMode;			// Is In Exchange Mode? 
	int   m_iExchangeH;					// Client ID to Exchanging with 
	char  m_cExchangeName[hb::shared::limits::CharNameLen];	// Name of Client to Exchanging with
	short m_sExchangeItemID[4];	// Item ID to validate exchange hasn't been tampered

	char  m_cExchangeItemIndex[4];		// ItemID to Exchange
	int   m_iExchangeItemAmount[4];		// Ammount to exchange with

	bool  m_bIsExchangeConfirm;			// Has the user hit confirm? 
	int	  iExchangeCount;				//Keeps track of items which are on list

	int   m_iQuest;				 // ? Quest
	int   m_iQuestID;			 // ? Quest ID
	int   m_iAskedQuest;
	int   m_iCurQuestCount;
	
	int   m_iQuestRewardType;
	int   m_iQuestRewardAmount;

	int   m_iContribution;

	bool  m_bQuestMatchFlag_Loc;
	bool  m_bIsQuestCompleted;

	int   m_iCustomItemValue_Attack;
	int   m_iCustomItemValue_Defense;

	int   m_iMinAP_SM;			// Custom-Item    AP
	int   m_iMinAP_L;

	int   m_iMaxAP_SM;			// Custom-Item    AP
	int   m_iMaxAP_L;

	bool  m_bIsNeutral;
	bool  m_bIsObserverMode;

	int   m_iSpecialEventID;

	int   m_iSpecialWeaponEffectType;
	int   m_iSpecialWeaponEffectValue;
	// 0-None 1-? 2- 3-
	// 5- 6- 7- 8- 9-

	// v1.42
	int   m_iAddHP, m_iAddSP, m_iAddMP; 
	int   m_iAddAR, m_iAddPR, m_iAddDR;
	int   m_iAddMR, m_iAddAbsPD, m_iAddAbsMD; 
	int   m_iAddCD, m_iAddExp, m_iAddGold;

	int   m_iAddResistMagic;
	int   m_iAddPhysicalDamage;
	int   m_iAddMagicalDamage;	

	int   m_iAddAbsAir;
	int   m_iAddAbsEarth;
	int   m_iAddAbsFire;
	int   m_iAddAbsWater;
	
	int   m_iLastDamage;

	int   m_iMoveMsgRecvCount, m_iAttackMsgRecvCount, m_iRunMsgRecvCount, m_iSkillMsgRecvCount;
	uint32_t m_dwMoveLAT, m_dwRunLAT, m_dwAttackLAT;

	int   m_iSpecialAbilityTime;
	bool  m_bIsSpecialAbilityEnabled;
	uint32_t m_dwSpecialAbilityStartTime;
	int   m_iSpecialAbilityLastSec;

	int   m_iSpecialAbilityType;
	int   m_iSpecialAbilityEquipPos;
	int   m_iAlterItemDropIndex;

	int   m_iWarContribution;

	uint32_t m_dwSpeedHackCheckTime;
	int   m_iSpeedHackCheckExp;		
	uint32_t m_dwLogoutHackCheck;

	uint32_t m_dwInitCCTimeRcv;
	uint32_t m_dwInitCCTime;

	char  m_cLockedMapName[11];
	int   m_iLockedMapTime;

	int   m_iCrusadeDuty;						// : 1-. 2-. 3-
	uint32_t m_dwCrusadeGUID;						// GUID
	uint32_t m_dwHeldenianGUID;
	bool m_bInRecallImpossibleMap;

	struct {
		char cType;
		char cSide;
		short sX, sY;
	} m_stCrusadeStructureInfo[hb::shared::limits::MaxCrusadeStructures];
	int m_iCSIsendPoint;

	char m_cSendingMapName[11];
	bool m_bIsSendingMapStatus;

	int  m_iConstructionPoint;

	char m_cConstructMapName[11];
	int  m_iConstructLocX, m_iConstructLocY;
	
	// 2.06
	bool m_bIsPlayerCivil;
	bool m_bIsAttackModeChange;

	// New 06/05/2004
	// Party Stuff
	int m_iPartyID;
	int m_iPartyStatus;
	int m_iReqJoinPartyClientH;
	char m_cReqJoinPartyName[hb::shared::limits::CharNameLen];

	int   m_iPartyRank;										// Party . -1 . 1  . 12 ?
	int   m_iPartyMemberCount;
	int   m_iPartyGUID;										// v1.42 Party GUID
	struct {
	int  iIndex;
	char cName[hb::shared::limits::CharNameLen];
	} m_stPartyMemberName[hb::shared::limits::MaxPartyMembers];

	// New 07/05/2004
	uint32_t m_dwLastActionTime;
	int m_iDeadPenaltyTime;

	// New 16/05/2004
	char m_cWhisperPlayerName[hb::shared::limits::CharNameLen];
	bool m_bIsInsideWarehouse;
	bool m_bIsInsideWizardTower;
	bool m_bIsInsideOwnTown;
	bool m_bIsCheckingWhisperPlayer;
	bool m_bIsOwnLocation;
	bool m_pIsProcessingAllowed;

	// Updated 10/11/2004 - 24/05/2004
	char m_cHeroArmorBonus;

	// New 25/05/2004
	bool m_bIsBeingResurrected;

	uint32_t m_dwFightzoneDeadTime;
	char m_cSaveCount;

	uint32_t m_dwLastConfigRequestTime = 0;
	uint32_t m_dwLastDamageTakenTime = 0;

	// Admin / GM
	bool m_bIsGMMode = false;
	bool m_bIsAdminInvisible = false;
	uint32_t m_dwLastGMImmuneNotifyTime = 0;
	int m_iAdminIndex = -1;
	int m_iAdminLevel = 0;

	// Block list
	struct CaseInsensitiveLess {
		bool operator()(const std::string& a, const std::string& b) const {
			return _stricmp(a.c_str(), b.c_str()) < 0;
		}
	};
	std::set<std::string, CaseInsensitiveLess> m_BlockedAccounts;
	std::vector<std::pair<std::string, std::string>> m_BlockedAccountsList;
	bool m_bBlockListDirty = false;

};
