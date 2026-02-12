// Game.h: interface for the CGame class.

#pragma once

// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)

#include "Platform.h"
#include "CommonTypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <memory.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>

#include "winmain.h"
#include "ASIOSocket.h"
#include "Client.h"
#include "Npc.h"
#include "Map.h"
#include "ActionID_Server.h"
#include "UserMessages.h"
#include "NetMessages.h"
#include "Packet/PacketMap.h"
#include "ServerMessages.h"
#include "Misc.h"
#include "NetworkMsg.h"
#include "Magic.h"
#include "Skill.h"
#include "DynamicObject.h"
#include "DelayEvent.h"
#include "Version.h"
#include "Fish.h"
#include "DynamicObject.h"
#include "DynamicObjectID.h"
#include "Portion.h"
#include "Mineral.h"
#include "Quest.h"
#include "BuildItem.h"
#include "TeleportLoc.h"
#include "GlobalDef.h"
#include "TempNpcItem.h"
#include "PartyManager.h"
#include "IOServicePool.h"
#include "ConcurrentMsgQueue.h"

namespace hb::server::config
{
// Infrastructure limits
constexpr int MaxMaps                   = 100;
constexpr int MaxNpcTypes               = 200;
constexpr int ServerSocketBlockLimit    = 300;
constexpr int MaxBanned                 = 500;
constexpr int MaxAdmins                 = 50;
constexpr int MaxNpcItems               = 1000;
constexpr int MaxClients                = 2000;
constexpr int MaxClientLoginSock        = 2000;
constexpr int MaxNpcs                   = 15000;
constexpr int MaxItemTypes              = 5000;
constexpr int MaxDynamicObjects         = 60000;
constexpr int MaxDelayEvents            = 60000;
constexpr int MaxNotifyMsgs             = 300;
constexpr int MaxSkillPoints            = 700;
constexpr int MaxRewardGold             = 99999999;

// Timing constants (milliseconds)
constexpr int ClientTimeout             = 30000;
constexpr int SpUpTime                  = 10000;
constexpr int PoisonTime                = 12000;
constexpr int HpUpTime                  = 15000;
constexpr int MpUpTime                  = 20000;
constexpr int HungerTime                = 60000;
constexpr int NoticeTime                = 80000;
constexpr int SummonTime                = 300000;
constexpr int AutoSaveTime              = 600000;
constexpr int ExpStockTime              = 1000 * 10;
constexpr int AutoExpTime               = 1000 * 60 * 6;
constexpr int NightTime                 = 30;
constexpr int RagProtectionTime         = 7000;

// Game config
constexpr int MsgQueueSize              = 100000;
constexpr int TotalLevelUpPoint         = 3;
constexpr int GuildStartRank            = 12;
constexpr int SsnLimitMultiplyValue     = 2;
constexpr int CharPointLimit            = 1000;
} // namespace hb::server::config

namespace hb::server::config
{
// Attack AI types
namespace AttackAI
{
	enum : int
	{
		Normal          = 1,
		ExchangeAttack  = 2,
		TwoByOneAttack  = 3,
	};
}

// Resource limits
constexpr int MaxFishs                  = 200;
constexpr int MaxMinerals               = 200;
constexpr int MaxEngagingFish           = 30;
constexpr int MaxPortionTypes           = 500;
constexpr int MaxQuestType              = 200;

// Game limits
constexpr int MaxGuilds                 = 1000;
constexpr int MaxConstructNum           = 10;
constexpr int MaxSchedule               = 10;
constexpr int MaxApocalypse             = 7;
constexpr int MaxHeldenian              = 10;
constexpr int MaxFightZone              = 10;

// Combat constants
constexpr int MinimumHitRatio           = 15;
constexpr int MaximumHitRatio           = 99;
constexpr int SpecialEventTime          = 300000;

// Crusade constants
constexpr int GmgManaConsumeUnit        = 15;
constexpr int MaxConstructionPoint      = 30000;
constexpr int MaxSummonPoints           = 30000;
constexpr int MaxWarContribution        = 200000;
} // namespace hb::server::config

// ItemLog values moved to ServerMessages.h -> hb::server::net::ItemLogAction

#define NO_MSGSPEEDCHECK

struct DropEntry
{
	int itemId;
	int weight;
	int minCount;
	int maxCount;
};

struct DropTable
{
	int id;
	char name[64];
	char description[128];
	std::vector<DropEntry> tierEntries[3];
	int totalWeight[3];
};

// Shop system structures
struct NpcShopMapping
{
	int npcType;                    // NPC type (15=ShopKeeper, 24=Blacksmith)
	int shopId;                     // Which shop inventory to use
	char description[64];           // For documentation
};

struct ShopData
{
	int shopId;
	std::vector<int16_t> itemIds;   // List of item IDs available in this shop
};

template <typename T>
static bool In(const T& value, std::initializer_list<T> values) {
	return std::any_of(values.begin(), values.end(),
		[&value](const T& x) { return x == value; });
}

template <typename T>
static bool NotIn(const T& value, std::initializer_list<T> values) {
	return !In(value, values);
}


template <typename T, class = typename enable_if<!is_pointer<T>::value>::type >
static void Push(char*& cp, T value) {
	auto p = (T*)cp;
	*p = (T)value;
	cp += sizeof(T);
}

template <typename T, class = typename enable_if<!is_pointer<T>::value>::type >
static void Pop(char*& cp, T& v) {
	T* p = (T*)cp;
	v = *p;
	cp += sizeof(T);
}

static void Push(char*& dest, const char* src, uint32_t len) {
	memcpy(dest, src, len);
	dest += len;
}

static void Push(char*& dest, const char* src) {

	strcpy(dest, src);
	dest += strlen(src) + 1;
}

static void Push(char*& dest, const string& str) {
	strcpy(dest, str.c_str());
	dest += str.length() + 1;
}

static void Pop(char*& src, char* dest, uint32_t len) {
	memcpy(dest, src, len);
	src += len;
}
static void Pop(char*& src, char* dest) {

	size_t len = strlen(src) + 1;
	memcpy(dest, src, len);
	src += len;
}

static void Pop(char*& src, string& str) {
	str = src;
	src += str.length() + 1;
}


struct LoginClient
{
	LoginClient(asio::io_context& ctx)
	{
		_sock = 0;
		_sock = new class hb::shared::net::ASIOSocket(ctx, hb::server::config::ClientSocketBlockLimit);
		_sock->bInitBufferSize(hb::shared::limits::MsgBufferSize);
		_timeout_tm = 0;
	}

	uint32_t _timeout_tm;
	~LoginClient();
	hb::shared::net::ASIOSocket* _sock;
	char _ip[21];
};

struct AdminEntry {
	char m_cAccountName[hb::shared::limits::AccountNameLen];
	char m_cCharName[hb::shared::limits::CharNameLen];
	char m_cApprovedIP[21];
	int m_iAdminLevel = 1;
};

struct CommandPermission {
	int iAdminLevel = 1000;
	std::string sDescription;
};

class CGame
{
public:


	void RequestNoticementHandler(int iClientH);
	bool bSendClientNpcConfigs(int iClientH);

	LoginClient* _lclients[hb::server::config::MaxClientLoginSock];

	bool bAcceptLogin(hb::shared::net::ASIOSocket* sock);
	bool bAcceptFromAsync(asio::ip::tcp::socket&& peer);
	bool bAcceptLoginFromAsync(asio::ip::tcp::socket&& peer);

	void PartyOperation(char* pData);


	bool _bCheckCharacterData(int iClientH);
	//bool _bDecodeNpcItemConfigFileContents(char * pData, size_t dwMsgSize);
	void GlobalUpdateConfigs(char cConfigType);
	void LocalUpdateConfigs(char cConfigType);

	void ReloadNpcConfigs();
	void SendConfigReloadNotification(bool bItems, bool bMagic, bool bSkills, bool bNpcs);
	void PushConfigReloadToClients(bool bItems, bool bMagic, bool bSkills, bool bNpcs);


	
	


	
	
	// Lists

	void Command_RedBall(int iClientH, char *pData,size_t dwMsgSize);
	void Command_BlueBall(int iClientH, char *pData,size_t dwMsgSize);
	void Command_YellowBall(int iClientH, char* pData, size_t dwMsgSize);

	// Crusade


	// Acidx commands
	


	void SetForceRecallTime(int iClientH);


	//  bool bReadTeleportConfigFile(char * cFn);
	//	void RequestTeleportD2Handler(int iClientH, char * pData);
	
	// Hack Checks
	bool bCheckClientMoveFrequency(int iClientH, uint32_t dwClientTime);

	// bool bCheckClientInvisibility(short iClientH);

	//Hypnotoad functions

	void RequestChangePlayMode(int iClientH);
	
	void StateChangeHandler(int iClientH, char * pData, size_t dwMsgSize);
	
	

	int  iGetMapLocationSide(char * pMapName);
	void ChatMsgHandlerGSM(int iMsgType, int iV1, char * pName, char * pData, size_t dwMsgSize);
	void RemoveClientShortCut(int iClientH);
	bool bAddClientShortCut(int iClientH);

	void RequestHelpHandler(int iClientH);
	
	void CheckConnectionHandler(int iClientH, char *pData, bool bAlreadyResponded = false);

	void AgingMapSectorInfo();
	void UpdateMapSectorInfo();
	void ActivateSpecialAbilityHandler(int iClientH);
	void JoinPartyHandler(int iClientH, int iV1, const char * pMemberName);
	void RequestShopContentsHandler(int iClientH, char * pData);
	void RequestRestartHandler(int iClientH);
	int iRequestPanningMapDataRequest(int iClientH, char * pData);
	void RequestCheckAccountPasswordHandler(char * pData, size_t dwMsgSize);
	void RequestNoticementHandler(int iClientH, char * pData);

	void CheckSpecialEvent(int iClientH);
	char _cGetSpecialAbility(int iKindSA);
	void OnSubLogSocketEvent(UINT message, WPARAM wParam, LPARAM lParam);
	void GetMapInitialPoint(int iMapIndex, short * pX, short * pY, char * pPlayerLocation = 0);
	int  iGetMaxHP(int iClientH);
	int  iGetMaxMP(int iClientH);
	int  iGetMaxSP(int iClientH);
	
	

	
	bool bOnClose();
	void ForceDisconnectAccount(char * pAccountName, uint16_t wCount);
	void ToggleSafeAttackModeHandler(int iClientH);
	void SpecialEventHandler();
	
	int _iForcePlayerDisconect(int iNum);
	int iGetMapIndex(char * pMapName);
	void WhetherProcessor();
	int _iCalcPlayerNum(char cMapIndex, short dX, short dY, char cRadius);
	int iGetExpLevel(uint32_t iExp);
	void ___RestorePlayerRating(int iClientH);
	void CalcExpStock(int iClientH);
	void ResponseSavePlayerDataReplyHandler(char * pData, size_t dwMsgSize);
	void NoticeHandler();
	bool bReadNotifyMsgListFile(char * cFn);
	void SetPlayerReputation(int iClientH, char * pMsg, char cValue, size_t dwMsgSize);
	void CheckDayOrNightMode();
	int _iGetPlayerNumberOnSpot(short dX, short dY, char cMapIndex, char cRange);
	void ___RestorePlayerCharacteristics(int iClientH);
	void GetPlayerProfile(int iClientH, char * pMsg, size_t dwMsgSize);
	void SetPlayerProfile(int iClientH, char * pMsg, size_t dwMsgSize);
	void CheckAndNotifyPlayerConnection(int iClientH, char * pMsg, uint32_t dwSize);
	int iCalcTotalWeight(int iClientH);
	void SendObjectMotionRejectMsg(int iClientH);
	int iGetFollowerNumber(short sOwnerH, char cOwnerType);
	void OnKeyUp(WPARAM wParam, LPARAM lParam);
	void OnKeyDown(WPARAM wParam, LPARAM lParam);
	void RequestFullObjectData(int iClientH, char * pData);
	int _iCalcMaxLoad(int iClientH);
	void RequestCivilRightHandler(int iClientH, char * pData);
	bool bCheckLimitedUser(int iClientH);
	void LevelUpSettingsHandler(int iClientH, char * pData, size_t dwMsgSize);
	bool bCheckLevelUp(int iClientH);
	uint32_t iGetLevelExp(int iLevel);
	void TimeManaPointsUp(int iClientH);
	void TimeStaminarPointsUp(int iClientH);
	void Quit();
	void ReleaseFollowMode(short sOwnerH, char cOwnerType);
	void RequestTeleportHandler(int iClientH, char * pData, char * cMapName = 0, int dX = -1, int dY = -1);
	void ToggleCombatModeHandler(int iClientH);
	void TimeHitPointsUp(int iClientH);
	void OnStartGameSignal();
	uint32_t iDice(uint32_t iThrow, uint32_t iRange);
	bool _bInitNpcAttr(class CNpc * pNpc, int iNpcConfigId, short sClass, char cSA);
	int GetNpcConfigIdByName(const char * pNpcName) const;
	void SendNotifyMsg(int iFromH, int iToH, uint16_t wMsgType, uint32_t sV1, uint32_t sV2, uint32_t sV3, char * pString, uint32_t sV4 = 0, uint32_t sV5 = 0, uint32_t sV6 = 0, uint32_t sV7 = 0, uint32_t sV8 = 0, uint32_t sV9 = 0, char * pString2 = 0);
	void BroadcastServerMessage(const char* pMessage);
	int  iClientMotion_Stop_Handler(int iClientH, short sX, short sY, char cDir);

	
	void ClientCommonHandler(int iClientH, char * pData);
	bool __fastcall bGetMsgQuene(char * pFrom, char * pData, size_t* pMsgSize, int * pIndex, char * pKey);
	void MsgProcess();
	bool __fastcall bPutMsgQuene(char cFrom, char * pData, size_t dwMsgSize, int iIndex, char cKey);
	//int  iCalculateAttackEffect(short sTargetH, char cTargetType, short sAttackerH, char cAttackerType, int tdX, int tdY, int iAttackMode, bool bNearAttack = false);
	bool bGetEmptyPosition(short * pX, short * pY, char cMapIndex);
	char cGetNextMoveDir(short sX, short sY, short dstX, short dstY, char cMapIndex, char cTurn, int * pError);
	int  iClientMotion_Attack_Handler(int iClientH, short sX, short sY, short dX, short dY, short wType, char cDir, uint16_t wTargetObjectID, uint32_t dwClientTime, bool bResponse = true, bool bIsDash = false);
	void ChatMsgHandler(int iClientH, char * pData, size_t dwMsgSize);
	bool IsBlockedBy(int iSenderH, int iReceiverH) const;
	void NpcProcess();
	int bCreateNewNpc(int iNpcConfigId, char * pName, char * pMapName, short sClass, char cSA, char cMoveType, int * poX, int * poY, char * pWaypointList, hb::shared::geometry::GameRectangle * pArea, int iSpotMobIndex, char cChangeSide, bool bHideGenMode, bool bIsSummoned = false, bool bFirmBerserk = false, bool bIsMaster = false, int iGuildGUID = 0, bool bBypassMobLimit = false);
	//bool bCreateNewNpc(char * pNpcName, char * pName, char * pMapName, short sX, short sY);
	int SpawnMapNpcsFromDatabase(struct sqlite3* db, int iMapIndex);
	bool _bGetIsStringIsNumber(char * pStr);
	void GameProcess();
	void InitPlayerData(int iClientH, char * pData, uint32_t dwSize);
	void ResponsePlayerDataHandler(char * pData, uint32_t dwSize);
	void CheckClientResponseTime();
	void OnTimer(char cType);
	int iComposeMoveMapData(short sX, short sY, int iClientH, char cDir, char * pData);
	void SendEventToNearClient_TypeB(uint32_t dwMsgID, uint16_t wMsgType, char cMapIndex, short sX, short sY, short sV1, short sV2, short sV3, short sV4 = 0);
	void SendEventToNearClient_TypeB(uint32_t dwMsgID, uint16_t wMsgType, char cMapIndex, short sX, short sY, short sV1, short sV2, short sV3, uint32_t dwV4 = 0);
	void SendEventToNearClient_TypeA(short sOwnerH, char cOwnerType, uint32_t dwMsgID, uint16_t wMsgType, short sV1, short sV2, short sV3);
	void DeleteClient(int iClientH, bool bSave, bool bNotify, bool bCountLogout = true, bool bForceCloseConn = false);
	int  iComposeInitMapData(short sX, short sY, int iClientH, char * pData);
	void FillPlayerMapObject(hb::net::PacketMapDataObjectPlayer& obj, short sOwnerH, int iViewerH);
	void FillNpcMapObject(hb::net::PacketMapDataObjectNpc& obj, short sOwnerH, int iViewerH);
	void RequestInitDataHandler(int iClientH, char * pData, char cKey, size_t dwMsgSize = 0);
	void RequestInitPlayerHandler(int iClientH, char * pData, char cKey);
	int iClientMotion_Move_Handler(int iClientH, short sX, short sY, char cDir, char cMoveType);
	void ClientMotionHandler(int iClientH, char * pData);
	void OnClientRead(int iClientH);
	bool bInit();
	void OnClientSocketEvent(int iClientH);  // MODERNIZED: Polls socket instead of handling window messages
	bool bAccept(class hb::shared::net::ASIOSocket * pXSock);

	// New 06/05/2004
	// Upgrades

	
	
	//Party Codes
	void RequestCreatePartyHandler(int iClientH);
	void PartyOperationResultHandler(char *pData);
	void PartyOperationResult_Create(int iClientH, char *pName, int iResult, int iPartyID);
	void PartyOperationResult_Join(int iClientH, char *pName, int iResult, int iPartyID);
	void PartyOperationResult_Dismiss(int iClientH, char *pName, int iResult, int iPartyID);
	void PartyOperationResult_Delete(int iPartyID);
	void RequestJoinPartyHandler(int iClientH, char *pData, size_t dwMsgSize);
	void RequestDismissPartyHandler(int iClientH);
	void GetPartyInfoHandler(int iClientH);
	void PartyOperationResult_Info(int iClientH, char * pName, int iTotal, char *pNameList);
	void RequestDeletePartyHandler(int iClientH);
	void RequestAcceptJoinPartyHandler(int iClientH, int iResult);
	void GetExp(int iClientH, uint32_t iExp, bool bIsAttackerOwn = false);

	// New 07/05/2004
	// Guild Codes

	// Item Logs


	// PK Logs

	//HBest code
	void ForceRecallProcess();
	bool IsEnemyZone(int i);

	CGame(HWND hWnd);
	virtual ~CGame();

	// Realm configuration (from realmlist table)
	char m_cRealmName[32];
	char m_cLoginListenIP[16];
	int  m_iLoginListenPort;
	char m_cGameListenIP[16];
	int  m_iGameListenPort;
	char m_cGameConnectionIP[16];   // Optional - for future login->game server connection
	int  m_iGameConnectionPort;     // Optional - for future login->game server connection

	uint32_t  m_iLevelExp20;

//private:
	bool LoadPlayerDataFromDb(int iClientH);
	bool _bRegisterMap(char * pName);

	class CClient * m_pClientList[hb::server::config::MaxClients];
	class CNpc   ** m_pNpcList;  // Pointer to EntityManager's entity array (for backward compatibility)
	class CMap    * m_pMapList[hb::server::config::MaxMaps];
	class CNpcItem * m_pTempNpcItem[hb::server::config::MaxNpcItems];

	class CEntityManager * m_pEntityManager;  // Entity spawn/despawn manager
	class FishingManager * m_pFishingManager;  // Fish spawning/catching manager
	class MiningManager * m_pMiningManager;    // Mineral spawning/mining manager
	class CraftingManager * m_pCraftingManager; // Potion/crafting recipe manager
	class QuestManager * m_pQuestManager;       // Quest assignment/progress manager
	class GuildManager * m_pGuildManager;       // Guild operations manager
	class DelayEventManager * m_pDelayEventManager; // Delay event processor
	class DynamicObjectManager * m_pDynamicObjectManager; // Dynamic object manager
	class LootManager * m_pLootManager; // Kill rewards and penalties
	class CombatManager * m_pCombatManager; // Combat calculations and effects
	class ItemManager * m_pItemManager; // Item management
	class MagicManager * m_pMagicManager; // Magic/spell casting
	class SkillManager * m_pSkillManager; // Skill mastery/profession
	class WarManager * m_pWarManager; // Crusade/Heldenian/Apocalypse/FightZone
	class StatusEffectManager * m_pStatusEffectManager; // Status effect flags

	hb::shared::net::ConcurrentMsgQueue m_msgQueue;
	int             m_iTotalMaps;
	bool			m_bIsGameStarted;
	bool			m_bIsItemAvailable, m_bIsBuildItemAvailable, m_bIsNpcAvailable, m_bIsMagicAvailable;
	bool			m_bIsSkillAvailable, m_bIsPortionAvailable, m_bIsQuestAvailable, m_bIsTeleportAvailable;
	bool			m_bIsDropTableAvailable;
	std::map<int, DropTable> m_DropTables;

	// Shop system - server sends shop contents to client by item IDs
	bool m_bIsShopDataAvailable;
	std::map<int, int> m_NpcShopMappings;        // npc_type  shop_id
	std::map<int, ShopData> m_ShopData;          // shop_id  ShopData
	CItem   * m_pItemConfigList[hb::server::config::MaxItemTypes];
	class CNpc    * m_pNpcConfigList[hb::server::config::MaxNpcTypes];
	class CMagic  * m_pMagicConfigList[hb::shared::limits::MaxMagicType];
	class CSkill  * m_pSkillConfigList[hb::shared::limits::MaxSkillType];
	//class CTeleport * m_pTeleportConfigList[DEF_MAXTELEPORTTYPE];

	uint32_t m_dwConfigHash[4]{};
	void ComputeConfigHashes();

	class hb::shared::net::ASIOSocket* _lsock;

	class PartyManager* m_pPartyManager;

	void OnClientLoginRead(int h);
	void OnLoginClientSocketEvent(int iLoginClientH);  // MODERNIZED: Polls login client socket instead of handling window messages
	void DeleteLoginClient(int h);

	std::vector<LoginClient*> _lclients_disconn;

	char            m_pMsgBuffer[hb::shared::limits::MsgBufferSize+1];

	HWND  m_hWnd;
	int   m_iTotalClients, m_iMaxClients, m_iTotalGameServerClients, m_iTotalGameServerMaxClients;
	int   m_iTotalBots, m_iMaxBots, m_iTotalGameServerBots, m_iTotalGameServerMaxBots;
	SYSTEMTIME m_MaxUserSysTime;

	bool  m_bF1pressed, m_bF4pressed, m_bF12pressed, m_bF5pressed;
	bool  m_bOnExitProcess;
	uint32_t m_dwExitProcessTime;

	uint32_t m_dwWhetherTime, m_dwGameTime1, m_dwGameTime2, m_dwGameTime3, m_dwGameTime4, m_dwGameTime5, m_dwGameTime6;
	
	// Crusade Schedule
	bool m_bIsCrusadeWarStarter;
	bool m_bIsApocalypseStarter;
	int m_iLatestCrusadeDayOfWeek;

	char  m_cDayOrNight;
 	int   m_iSkillSSNpoint[102];

	class CMsg * m_pNoticeMsgList[hb::server::config::MaxNotifyMsgs];
	int   m_iTotalNoticeMsg, m_iPrevSendNoticeMsg;
	uint32_t m_dwNoticeTime, m_dwSpecialEventTime;
	bool  m_bIsSpecialEventTime;
	char  m_cSpecialEventType;

	uint32_t m_iLevelExpTable[1000];	//New 22/10/04


	bool  m_bIsServerShutdowned;
	char  m_cShutDownCode;

	int   m_iMiddlelandMapIndex; 
	int   m_iAresdenMapIndex;
	int	  m_iElvineMapIndex;
	int   m_iBTFieldMapIndex;
	int   m_iGodHMapIndex;
	int   m_iAresdenOccupyTiles;
	int   m_iElvineOccupyTiles;
	int   m_iCurMsgs, m_iMaxMsgs;

	uint32_t m_dwCanFightzoneReserveTime ;

	int  m_iFightZoneReserve[hb::server::config::MaxFightZone] ;
	int  m_iFightzoneNoForceRecall  ;

	struct {
		__int64 iFunds;
		__int64 iCrimes;
		__int64 iWins;

	} m_stCityStatus[3];
	
	int	  m_iStrategicStatus;
	
	int   m_iAutoRebootingCount;

	class CBuildItem * m_pBuildItemList[hb::shared::limits::MaxBuildItems];

	char * m_pNoticementData;
	uint32_t  m_dwNoticementDataSize;

	uint32_t  m_dwMapSectorInfoTime;
	int    m_iMapSectorInfoUpdateCount;

	// Crusade
	int	   m_iCrusadeCount;	
	bool   m_bIsCrusadeMode;		
	bool   m_bIsApocalypseMode;
	struct {
		char cMapName[11];	
		char cType;			
		int  dX, dY;		

	} m_stCrusadeStructures[hb::shared::limits::MaxCrusadeStructures];

	
	int m_iCollectedMana[3];
	int m_iAresdenMana, m_iElvineMana;

	class CTeleportLoc m_pGuildTeleportLoc[hb::server::config::MaxGuilds];

	int m_iLastCrusadeWinner; 	// New 13/05/2004
	struct {
		int iCrashedStructureNum;
		int iStructureDamageAmount;
		int iCasualties;
	} m_stMeteorStrikeResult;

	struct {
		char cType;		
		char cSide;		
		short sX, sY;	
	} m_stMiddleCrusadeStructureInfo[hb::shared::limits::MaxCrusadeStructures];

	struct {
		char m_cBannedIPaddress[21];
	} m_stBannedList[hb::server::config::MaxBanned];

	AdminEntry m_stAdminList[hb::server::config::MaxAdmins];
	int m_iAdminCount = 0;

	int FindAdminByAccount(const char* accountName);
	int FindAdminByCharName(const char* charName);
	bool IsClientAdmin(int iClientH);

	std::unordered_map<std::string, CommandPermission> m_commandPermissions;
	int GetCommandRequiredLevel(const char* cmdName) const;

	// GM command helpers
	int FindClientByName(const char* pName) const;
	bool GMTeleportTo(int iClientH, const char* cDestMap, short sDestX, short sDestY);

	// Crusade Scheduler
	struct {
		int iDay;
		int iHour;
		int iMinute;
	} m_stCrusadeWarSchedule[hb::server::config::MaxSchedule];

	struct {
		int iDay;
		int iHour;
		int iMinute;
	} m_stApocalypseScheduleStart[hb::server::config::MaxApocalypse];

	struct {
		int iDay;
		int StartiHour;
		int StartiMinute;
		int EndiHour;
		int EndiMinute;
	} m_stHeldenianSchedule[hb::server::config::MaxHeldenian];

	struct {
		int iDay;
		int iHour;
		int iMinute;
	} m_stApocalypseScheduleEnd[hb::server::config::MaxApocalypse];

	int m_iTotalMiddleCrusadeStructures;
 
	int m_iClientShortCut[hb::server::config::MaxClients+1];

	int m_iNpcConstructionPoint[hb::server::config::MaxNpcTypes];
	uint32_t m_dwCrusadeGUID;
	short m_sLastCrusadeDate;
	int   m_iCrusadeWinnerSide;

	struct  {
		int iTotalMembers;
		int iIndex[9];
	}m_stPartyInfo[hb::server::config::MaxClients];

	// 09/26/2004
	short m_sSlateSuccessRate;

	// 17/05/2004
	short m_sForceRecallTime;

	// 22/05/2004 - Drop rate multipliers (1.0 = normal, 1.5 = 150%, 0.5 = 50%)
	float m_fPrimaryDropRate;    // Primary item drops (base 10%)
	float m_fGoldDropRate;       // Gold drops (base 30%)
	float m_fSecondaryDropRate;  // Bonus/secondary drops (base 5%)

	// 25/05/2004
	int m_iFinalShutdownCount;

	// New 06/07/2004
	bool m_bEnemyKillMode;
	int m_iEnemyKillAdjust;

	// Configurable Raid Time 
	short m_sRaidTimeMonday; 
	short m_sRaidTimeTuesday; 
	short m_sRaidTimeWednesday; 
	short m_sRaidTimeThursday; 
	short m_sRaidTimeFriday; 
	short m_sRaidTimeSaturday; 
	short m_sRaidTimeSunday; 

	bool m_bManualTime;

	// Apocalypse
	bool	m_bIsApocalyseMode;
	bool	m_bIsHeldenianMode;
	bool	m_bIsHeldenianTeleport;
	char	m_cHeldenianType;

	uint32_t m_dwApocalypseGUID;
	
	// Slate exploit
	int m_sCharPointLimit;

	// Limit Checks
	bool m_bAllow100AllSkill;
	char m_cRepDropModifier;

	// ============================================================================
	// Configurable Settings (loaded from GameConfigs.db)
	// ============================================================================

	// Timing Settings (milliseconds)
	int m_iClientTimeout;           // client-timeout-ms
	int m_iStaminaRegenInterval;    // stamina-regen-interval
	int m_iPoisonDamageInterval;    // poison-damage-interval
	int m_iHealthRegenInterval;     // health-regen-interval
	int m_iManaRegenInterval;       // mana-regen-interval
	int m_iHungerConsumeInterval;   // hunger-consume-interval
	int m_iSummonCreatureDuration;  // summon-creature-duration
	int m_iAutosaveInterval;        // autosave-interval
	int m_iLagProtectionInterval;   // lag-protection-interval

	// Character/Leveling Settings
	int m_iBaseStatValue;           // base-stat-value
	int m_iCreationStatBonus;       // creation-stat-bonus
	int m_iLevelupStatGain;         // levelup-stat-gain
	int m_iMaxLevel;                // max-level (renamed from max-player-level)
	int m_iMaxStatValue;            // calculated: base + creation + (levelup * max_level) + 16

	// Combat Settings
	int m_iMinimumHitRatio;         // minimum-hit-ratio
	int m_iMaximumHitRatio;         // maximum-hit-ratio

	// Gameplay Settings
	int m_iNighttimeDuration;       // nighttime-duration
	int m_iStartingGuildRank;       // starting-guild-rank
	int m_iGrandMagicManaConsumption; // grand-magic-mana-consumption
	int m_iMaxConstructionPoints;   // maximum-construction-points
	int m_iMaxSummonPoints;         // maximum-summon-points
	int m_iMaxWarContribution;      // maximum-war-contribution
	int m_iMaxBankItems;            // max-bank-items

	// ============================================================================

	bool var_89C, var_8A0;
	char m_cHeldenianVictoryType, m_sLastHeldenianWinner, m_cHeldenianModeType;
	int m_iHeldenianAresdenDead, m_iHeldenianElvineDead, var_A38, var_88C;
	int m_iHeldenianAresdenLeftTower, m_iHeldenianElvineLeftTower;
	uint32_t m_dwHeldenianGUID, m_dwHeldenianStartHour, m_dwHeldenianStartMinute, m_dwHeldenianStartTime, m_dwHeldenianFinishTime;
	bool m_bReceivedItemList;
	bool m_bHeldenianInitiated;
	bool m_bHeldenianRunning;

private:

public:

	void CheckForceRecallTime(int iClientH);
	void SetPlayingStatus(int iClientH);
	void ForceChangePlayMode(int iClientH, bool bNotify);
	void ShowVersion(int iClientH);
	void ShowClientMsg(int iClientH, char* pMsg);
	void RequestResurrectPlayer(int iClientH, bool bResurrect);
	void LoteryHandler(int iClientH);
	
	/*void GetAngelMantleHandler(int iClientH,int iItemID,char * pString);
	void CheckAngelUnequip(int iClientH, int iAngelID);
	int iAngelEquip(int iClientH);*/

	void GetAngelHandler(int iClientH, char* pData, size_t dwMsgSize);

	void RequestEnchantUpgradeHandler(int client, uint32_t type, uint32_t lvl, int iType);
	int GetRequiredLevelForUpgrade(uint32_t value);

	//50Cent - Repair All

};
