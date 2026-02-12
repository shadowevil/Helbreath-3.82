// Client.cpp: implementation of the CClient class.

#include "CommonTypes.h"
#include "Client.h"

using namespace hb::shared::item;
using namespace hb::server::config;

extern char G_cTxt[512];

// Construction/Destruction

CClient::CClient(asio::io_context& ctx)
{
 

	m_pXSock = 0;
	m_pXSock = new class hb::shared::net::ASIOSocket(ctx, hb::server::config::ClientSocketBlockLimit);
	m_pXSock->bInitBufferSize(hb::shared::limits::MsgBufferSize);

	std::memset(m_cProfile, 0, sizeof(m_cProfile));
	strcpy(m_cProfile, "__________");

	std::memset(m_cCharName, 0, sizeof(m_cCharName));
	std::memset(m_cAccountName, 0, sizeof(m_cAccountName));
	std::memset(m_cAccountPassword, 0, sizeof(m_cAccountPassword));

	std::memset(m_cGuildName, 0, sizeof(m_cGuildName));
	std::memset(m_cLocation, 0, sizeof(m_cLocation));
	strcpy(m_cLocation, "NONE");
	m_iGuildRank = -1;
	m_iGuildGUID = -1;

	m_bIsInitComplete = false;
	m_bIsClientConnected = false;
	m_dwLastMsgId = 0;
	m_dwLastMsgTime = 0;
	m_dwLastMsgSize = 0;
	m_dwLastFullObjectId = 0;
	m_dwLastFullObjectTime = 0;
	m_dwAfkActivityTime = 0;

	m_iAngelicStr = m_iAngelicInt = m_iAngelicDex = m_iAngelicMag = 0;

	//m_cLU_Str = m_cLU_Int = m_cLU_Vit = m_cLU_Dex = m_cLU_Mag = m_cLU_Char = 0;
	m_iLU_Pool = 0;
	m_cAura = 0;

	//m_iHitRatio_ItemEffect_SM = 0;
	//m_iHitRatio_ItemEffect_L  = 0;
	m_cVar = 0;
	m_iEnemyKillCount = 0;
	m_iPKCount = 0;
	m_iRewardGold = 0;
	m_iCurWeightLoad = 0;
	m_dwLogoutHackCheck = 0;

	// Charges
	m_iAddTransMana = 0;
	m_iAddChargeCritical = 0;

	m_bIsSafeAttackMode  = false;

	//50Cent - Repair All
	totalItemRepair = 0;
	for(int i = 0; i < hb::shared::limits::MaxItems; i++) {
		m_stRepairAll[i].index = 0;
		m_stRepairAll[i].price = 0;
	}

	for(int i = 0; i < DEF_MAXITEMEQUIPPOS; i++) 
		m_sItemEquipmentStatus[i] = -1;
	
	// Initialize item list 
	for(int i = 0; i < hb::shared::limits::MaxItems; i++) {
		m_pItemList[i]       = 0;
		m_ItemPosList[i].x   = 40;
		m_ItemPosList[i].y   = 30;
		m_bIsItemEquipped[i] = false;
	}
	m_cArrowIndex = -1;

	// Initialize item list.
	for(int i = 0; i < hb::shared::limits::MaxBankItems; i++) {
		m_pItemInBankList[i] = 0;
	}

	// Magic - Skill
	for(int i = 0; i < hb::shared::limits::MaxMagicType; i++)
		m_cMagicMastery[i] = 0;
	
	for(int i = 0; i < hb::shared::limits::MaxSkillType; i++)
		m_cSkillMastery[i] = 0;

	for(int i = 0; i < hb::shared::limits::MaxSkillType; i++) {
		m_bSkillUsingStatus[i] = false;
		m_iSkillUsingTimeID[i] = 0;
	}

	// testcode
	m_cMapIndex = -1;
	m_sX = -1;
	m_sY = -1;
	m_cDir = 5; 
	m_sType   = 0;
	m_sOriginalType = 0;
	m_appearance.Clear();
	m_status.Clear();

	m_cSex  = 0;
	m_cSkin = 0;
	m_cHairStyle  = 0;
	m_cHairColor  = 0;
	m_cUnderwear  = 0;

	m_cAttackDiceThrow_SM = 0;	// @@@@@@@@@@@@@
	m_cAttackDiceRange_SM = 0;
	m_cAttackDiceThrow_L = 0;	// @@@@@@@@@@@@@
	m_cAttackDiceRange_L = 0;
	m_cAttackBonus_SM    = 0;
	m_cAttackBonus_L     = 0;
	
	// NPC   .
	m_cSide = 0;

	m_iHitRatio = 0;
	m_iDefenseRatio = 0;
	
	for(int i = 0; i < DEF_MAXITEMEQUIPPOS; i++) m_iDamageAbsorption_Armor[i]  = 0;
	m_iDamageAbsorption_Shield = 0;

	m_iHPstock = 0;
	m_bIsKilled = false;

	for(int i = 0; i < hb::server::config::MaxMagicEffects; i++) 
		m_cMagicEffectStatus[i]	= 0;

	m_iWhisperPlayerIndex = -1;
	std::memset(m_cWhisperPlayerName, 0, sizeof(m_cWhisperPlayerName));

	m_iHungerStatus  = 100;  // Maximum value is 100
	
	m_bIsWarLocation = false;

	m_bIsPoisoned    = false;
	m_iPoisonLevel   = 0;

	m_iRating          = 0;
	m_iTimeLeft_Rating = 0;
	m_iTimeLeft_ForceRecall  = 0;
	m_iTimeLeft_FirmStaminar = 0;
	
	m_iRecentWalkTime  = 0;
	m_iRecentRunTime   = 0;
	m_sV1			   = 0;

	m_bIsOnServerChange  = false;
	m_bInhibition = false;

	m_iExpStock = 0;

	m_iAllocatedFish = 0;
	m_iFishChance    = 0;

	std::memset(m_cIPaddress, 0, sizeof(m_cIPaddress)); 
	m_bIsOnWaitingProcess = false;

	m_iSuperAttackLeft  = 0;
	m_iSuperAttackCount = 0;

	m_sUsingWeaponSkill = 5;

	m_iManaSaveRatio   = 0;
	m_iAddResistMagic  = 0;
	m_iAddPhysicalDamage = 0;
	m_iAddMagicalDamage  = 0;
	m_bIsLuckyEffect     = false;
	m_iSideEffect_MaxHPdown = 0;

	m_iAddAbsAir   = 0;
	m_iAddAbsEarth = 0;
	m_iAddAbsFire  = 0;
	m_iAddAbsWater = 0;

	m_iComboAttackCount = 0;
	m_iDownSkillIndex   = -1;
	m_bInRecallImpossibleMap = 0;

	m_iMagicDamageSaveItemIndex = -1;

	m_sCharIDnum1 = m_sCharIDnum2 = m_sCharIDnum3 = 0;

	// New 06/05/2004
	m_iPartyID = 0;
	m_iPartyStatus = 0;
	m_iReqJoinPartyClientH = 0;
	std::memset(m_cReqJoinPartyName, 0, sizeof(m_cReqJoinPartyName));

	/*m_iPartyRank = -1; // v1.42
	m_iPartyMemberCount = 0;
	m_iPartyGUID        = 0;

	for(int i = 0; i < hb::shared::limits::MaxPartyMembers; i++) {
		m_stPartyMemberName[i].iIndex = 0;
		std::memset(m_stPartyMemberName[i].cName, 0, sizeof(m_stPartyMemberName[i].cName));
	}*/

	m_iAbuseCount     = 0;
	m_bIsExchangeMode = false;

	//hbest
	isForceSet = false;

    m_iFightZoneTicketNumber =	m_iFightzoneNumber = m_iReserveTime = 0 ;            

	m_iPenaltyBlockYear = m_iPenaltyBlockMonth = m_iPenaltyBlockDay = 0; // v1.4

	m_iExchangeH = 0;
	std::memset(m_cExchangeName, 0, sizeof(m_cExchangeName));
	for(int i = 0; i<4; i++){
		m_sExchangeItemID[i] = 0;
		m_cExchangeItemIndex[i]  = -1;
		m_iExchangeItemAmount[i] = 0;
	}

	m_bIsExchangeConfirm = false;

	m_iQuest		 = 0; // Currently assigned Quest 
	m_iQuestID       = 0; // QuestID
	m_iAskedQuest	 = 0;
	m_iCurQuestCount = 0;

	m_iQuestRewardType	 = 0;
	m_iQuestRewardAmount = 0;

	m_iContribution = 0;			// Contribution 
	m_bQuestMatchFlag_Loc = false;  // Quest location verification flag.
	m_bIsQuestCompleted   = false;

	m_cHeroArmourBonus = 0;

	m_bIsNeutral      = false;
	m_bIsObserverMode = false;

	// 2000.8.1
	m_iSpecialEventID = 200081;

	m_iSpecialWeaponEffectType  = 0;	// : 0-None 1- 2- 3- 4-
	m_iSpecialWeaponEffectValue = 0;

	m_iAddHP = m_iAddSP = m_iAddMP = 0; 
	m_iAddAR = m_iAddPR = m_iAddDR = 0;
	m_iAddAbsPD = m_iAddAbsMD = 0;
	m_iAddCD = m_iAddExp = m_iAddGold = 0;
		
	m_iSpecialAbilityTime = SpecialAbilityTimeSec;		// Special ability can be used once every SpecialAbilityTimeSec seconds.
	m_iSpecialAbilityType = 0;
	m_bIsSpecialAbilityEnabled = false;
	m_iSpecialAbilityLastSec   = 0;

	m_iSpecialAbilityEquipPos  = 0;

	m_iMoveMsgRecvCount   = 0;
	m_iAttackMsgRecvCount = 0;
	m_iRunMsgRecvCount    = 0;
	m_iSkillMsgRecvCount  = 0;

	m_iAlterItemDropIndex = -1;

	m_iAutoExpAmount = 0;
	m_iWarContribution = 0;

	m_dwMoveLAT = m_dwRunLAT = m_dwAttackLAT = 0;

	m_dwInitCCTimeRcv = 0;
	m_dwInitCCTime = 0;

	std::memset(m_cLockedMapName, 0, sizeof(m_cLockedMapName));
	strcpy(m_cLockedMapName, "NONE");
	m_iLockedMapTime = 0;

	m_iCrusadeDuty  = 0;
	m_dwCrusadeGUID = 0;
	m_dwHeldenianGUID = 0;

	for(int i = 0; i < hb::shared::limits::MaxCrusadeStructures; i++) {
		m_stCrusadeStructureInfo[i].cType = 0;
		m_stCrusadeStructureInfo[i].cSide = 0;
		m_stCrusadeStructureInfo[i].sX = 0;
		m_stCrusadeStructureInfo[i].sY = 0;
	}

	m_iCSIsendPoint = 0;

	m_bIsSendingMapStatus = false;
	std::memset(m_cSendingMapName, 0, sizeof(m_cSendingMapName));

	m_iConstructionPoint = 0;

	std::memset(m_cConstructMapName, 0, sizeof(m_cConstructMapName));
	m_iConstructLocX = m_iConstructLocY = -1;

	m_bIsInsideWarehouse = false;
	m_bIsInsideWizardTower = false;
	m_bIsInsideOwnTown = false;
	m_bIsCheckingWhisperPlayer = false;
	m_bIsOwnLocation = false;
	m_pIsProcessingAllowed = false;

	m_cHeroArmorBonus = 0;

	m_bIsBeingResurrected = false;
	m_bMagicConfirm = false;
	m_bMagicItem = false;
	m_iSpellCount = 0;
	m_bMagicPauseTime = false;

}

CClient::~CClient()
{
	if (m_pXSock != 0)
		delete m_pXSock;

	for(int i = 0; i < hb::shared::limits::MaxItems; i++)
	{
		if (m_pItemList[i] != 0) {
			delete m_pItemList[i];
			m_pItemList[i] = 0;
		}
	}

	for(int i = 0; i < hb::shared::limits::MaxBankItems; i++)
	{
		if (m_pItemInBankList[i] != 0) {
			delete m_pItemInBankList[i];
			m_pItemInBankList[i] = 0;
		}
	}
}

bool CClient::bCreateNewParty()
{
 

	if (m_iPartyRank != -1) return false;

	m_iPartyRank = 0;
	m_iPartyMemberCount = 0;
	m_iPartyGUID = (rand() % 999999) + GameClock::GetTimeMS();

	for(int i = 0; i < hb::shared::limits::MaxPartyMembers; i++) {
		m_stPartyMemberName[i].iIndex = 0;
		std::memset(m_stPartyMemberName[i].cName, 0, sizeof(m_stPartyMemberName[i].cName));
	}

	return true;
}

