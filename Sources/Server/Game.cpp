// Game.cpp: implementation of the CGame class.

#include "CommonTypes.h"
#include "Game.h"
#include "AdminLevel.h"
#include "LoginServer.h"
#include "EntityManager.h"
#include "FishingManager.h"
#include "MiningManager.h"
#include "AccountSqliteStore.h"
#include "GameConfigSqliteStore.h"
#include "MapInfoSqliteStore.h"
#include "sqlite3.h"
#include "Packet/SharedPackets.h"
#include "CRC32.h"
#include "SharedCalculations.h"
#include "Item/ItemAttributes.h"
#include "ChatLog.h"
#include "ItemLog.h"
#include "ObjectIDRange.h"
#include "DirectionHelpers.h"
#include "GameChatCommand.h"
#include "CraftingManager.h"
#include "QuestManager.h"
#include "GuildManager.h"
#include "DelayEventManager.h"
#include "DynamicObjectManager.h"
#include "LootManager.h"
#include "CombatManager.h"
#include "ItemManager.h"
#include "MagicManager.h"
#include "SkillManager.h"
#include "WarManager.h"
#include "StatusEffectManager.h"



using namespace hb::shared::net;
using namespace hb::server::net;
using namespace hb::server::config;
using namespace hb::server::npc;
using namespace hb::server::skill;
using namespace hb::server::msg;
namespace smap = hb::server::map;
namespace squest = hb::server::quest;
namespace sdelay = hb::server::delay_event;
namespace sock = hb::shared::net::socket;
namespace dynamic_object = hb::shared::dynamic_object;

using namespace hb::shared::action;

using namespace hb::shared::item;

class CDebugWindow* DbgWnd;

extern void PutLogList(char* cMsg);
extern void PutLogListLevel(int level, const char* cMsg);
extern char G_cTxt[512];
extern char	G_cData50000[50000];

extern void PutLogFileList(char* cStr);
extern void PutLogEventFileList(char* cStr);
extern void PutHackLogFileList(char* cStr);
extern void PutPvPLogFileList(char* cStr);

// extern void PutDebugMsg(char * cStr);	// 2002-09-09 #2

extern FILE* pLogFile;
extern HWND	G_hWnd;

// Move location tables  auto-calculated from hb::shared::view::InitDataTilesX/Y.
// Each direction lists the (X,Y) tile offsets revealed when the player
// moves one step in that direction, terminated by -1.
int _tmp_iMoveLocX[9][hb::shared::view::MoveLocMaxEntries];
int _tmp_iMoveLocY[9][hb::shared::view::MoveLocMaxEntries];

static void BuildMoveLocTables()
{
	const int MAX_X = hb::shared::view::InitDataTilesX - 1;
	const int MAX_Y = hb::shared::view::InitDataTilesY - 1;

	// Zero-fill everything first
	memset(_tmp_iMoveLocX, 0, sizeof(_tmp_iMoveLocX));
	memset(_tmp_iMoveLocY, 0, sizeof(_tmp_iMoveLocY));

	// Helper: write an entry pair and advance the index
	auto put = [](int dir, int& idx, int x, int y) {
		_tmp_iMoveLocX[dir][idx] = x;
		_tmp_iMoveLocY[dir][idx] = y;
		idx++;
	};
	auto sentinel = [](int dir, int idx) {
		_tmp_iMoveLocX[dir][idx] = -1;
		_tmp_iMoveLocY[dir][idx] = -1;
	};

	int n;

	// Direction 0: unused
	sentinel(0, 0);

	// Direction 1 (North): top row, sweep X 0..MAX_X, Y=0
	n = 0;
	for(int x = 0; x <= MAX_X; x++) put(1, n, x, 0);
	sentinel(1, n);

	// Direction 2 (NE): top row X 0..MAX_X at Y=0, then right column Y=1..MAX_Y at X=MAX_X
	n = 0;
	for(int x = 0; x <= MAX_X; x++) put(2, n, x, 0);
	for(int y = 1; y <= MAX_Y; y++) put(2, n, MAX_X, y);
	sentinel(2, n);

	// Direction 3 (East): right column, sweep Y 0..MAX_Y, X=MAX_X
	n = 0;
	for(int y = 0; y <= MAX_Y; y++) put(3, n, MAX_X, y);
	sentinel(3, n);

	// Direction 4 (SE): right column Y 0..MAX_Y at X=MAX_X, then bottom row X=MAX_X-1..0 at Y=MAX_Y
	n = 0;
	for(int y = 0; y <= MAX_Y; y++) put(4, n, MAX_X, y);
	for(int x = MAX_X - 1; x >= 0; x--) put(4, n, x, MAX_Y);
	sentinel(4, n);

	// Direction 5 (South): bottom row, sweep X 0..MAX_X, Y=MAX_Y
	n = 0;
	for(int x = 0; x <= MAX_X; x++) put(5, n, x, MAX_Y);
	sentinel(5, n);

	// Direction 6 (SW): left column Y 0..MAX_Y at X=0, then bottom row X=1..MAX_X at Y=MAX_Y
	n = 0;
	for(int y = 0; y <= MAX_Y; y++) put(6, n, 0, y);
	for(int x = 1; x <= MAX_X; x++) put(6, n, x, MAX_Y);
	sentinel(6, n);

	// Direction 7 (West): left column, sweep Y 0..MAX_Y, X=0
	n = 0;
	for(int y = 0; y <= MAX_Y; y++) put(7, n, 0, y);
	sentinel(7, n);

	// Direction 8 (NW): top row X 0..MAX_X at Y=0, then left column Y=1..MAX_Y at X=0
	n = 0;
	for(int x = 0; x <= MAX_X; x++) put(8, n, x, 0);
	for(int y = 1; y <= MAX_Y; y++) put(8, n, 0, y);
	sentinel(8, n);
}


char _tmp_cTmpDirX[9] = { 0,0,1,1,1,0,-1,-1,-1 };
char _tmp_cTmpDirY[9] = { 0,-1,-1,0,1,1,1,0,-1 };

// Construction/Destruction

extern bool	G_bIsThread;
extern void ThreadProc(void* ch);

CGame::CGame(HWND hWnd)
{
	int x;

	BuildMoveLocTables();

	m_bIsGameStarted = false;
	m_hWnd = hWnd;
	_lsock = 0;
	g_login = new LoginServer;

	// Initialize configurable settings with defaults
	// Timing Settings (milliseconds)
	m_iClientTimeout = ClientTimeout;
	m_iStaminaRegenInterval = SpUpTime;
	m_iPoisonDamageInterval = PoisonTime;
	m_iHealthRegenInterval = HpUpTime;
	m_iManaRegenInterval = MpUpTime;
	m_iHungerConsumeInterval = HungerTime;
	m_iSummonCreatureDuration = SummonTime;
	m_iAutosaveInterval = AutoSaveTime;
	m_iLagProtectionInterval = RagProtectionTime;

	// Character/Leveling Settings
	m_iBaseStatValue = 10;
	m_iCreationStatBonus = 4;
	m_iLevelupStatGain = TotalLevelUpPoint;
	m_iMaxLevel = hb::shared::limits::PlayerMaxLevel;
	m_iMaxStatValue = 0; // Calculated after config load

	// Combat Settings
	m_iMinimumHitRatio = MinimumHitRatio;
	m_iMaximumHitRatio = MaximumHitRatio;

	// Gameplay Settings
	m_iNighttimeDuration = NightTime;
	m_iStartingGuildRank = m_iStartingGuildRank;
	m_iGrandMagicManaConsumption = GmgManaConsumeUnit;
	m_iMaxConstructionPoints = m_iMaxConstructionPoints;
	m_iMaxSummonPoints = MaxSummonPoints;
	m_iMaxWarContribution = m_iMaxWarContribution;
	m_iMaxBankItems = 200; // Default soft cap

	m_bIsDropTableAvailable = false;
	m_DropTables.clear();

	for(int i = 0; i < MaxClients; i++)
		m_pClientList[i] = 0;

	for(int i = 0; i < MaxMaps; i++)
		m_pMapList[i] = 0;

	for(int i = 0; i < MaxItemTypes; i++)
		m_pItemConfigList[i] = 0;

	for(int i = 0; i < MaxNpcTypes; i++)
		m_pNpcConfigList[i] = 0;

	// Initialize Entity Manager (MUST be before any entity operations)
	m_pEntityManager = new CEntityManager();

	// Get reference to EntityManager's entity array for backward compatibility
	// This allows existing code to access entities via m_pNpcList
	m_pNpcList = m_pEntityManager->GetEntityArray();
	m_pEntityManager->SetMapList(m_pMapList, MaxMaps);
	m_pEntityManager->SetGame(this);

	// Initialize Gathering Managers
	m_pFishingManager = new FishingManager();
	m_pFishingManager->SetGame(this);
	m_pMiningManager = new MiningManager();
	m_pMiningManager->SetGame(this);
	m_pCraftingManager = new CraftingManager();
	m_pCraftingManager->SetGame(this);
	m_pQuestManager = new QuestManager();
	m_pQuestManager->SetGame(this);
	m_pGuildManager = new GuildManager();
	m_pGuildManager->SetGame(this);
	m_pDelayEventManager = new DelayEventManager();
	m_pDelayEventManager->SetGame(this);
	m_pDelayEventManager->InitArrays();
	m_pDynamicObjectManager = new DynamicObjectManager();
	m_pDynamicObjectManager->SetGame(this);
	m_pDynamicObjectManager->InitArrays();
	m_pLootManager = new LootManager();
	m_pLootManager->SetGame(this);
	m_pCombatManager = new CombatManager();
	m_pItemManager = new ItemManager();
	m_pMagicManager = new MagicManager();
	m_pSkillManager = new SkillManager();
	m_pWarManager = new WarManager();
	m_pStatusEffectManager = new StatusEffectManager();
	m_pCombatManager->SetGame(this);
	m_pItemManager->SetGame(this);
	m_pMagicManager->SetGame(this);
	m_pSkillManager->SetGame(this);
	m_pWarManager->SetGame(this);
	m_pStatusEffectManager->SetGame(this);

	for(int i = 0; i < hb::shared::limits::MaxMagicType; i++)
		m_pMagicConfigList[i] = 0;

	for(int i = 0; i < hb::shared::limits::MaxSkillType; i++)
		m_pSkillConfigList[i] = 0;




	for(int i = 0; i < MaxNotifyMsgs; i++)
		m_pNoticeMsgList[i] = 0;




	//	/for(int i = 0; i < DEF_MAXTELEPORTTYPE; i++)
	//		m_pTeleportConfigList[i] = 0;

	for(int i = 0; i < hb::shared::limits::MaxBuildItems; i++)
		m_pBuildItemList[i] = 0;

	// New 06/05/2004
	for(int i = 0; i < MaxClients; i++) {
		m_stPartyInfo[i].iTotalMembers = 0;
		for (x = 0; x < hb::shared::limits::MaxPartyMembers; x++)
			m_stPartyInfo[i].iIndex[x] = 0;
	}

	m_iTotalClients = 0;
	m_iMaxClients = 0;
	m_iTotalMaps = 0;

	m_iTotalGameServerClients = 0;
	m_iTotalGameServerMaxClients = 0;

	m_MaxUserSysTime.wHour = 0;
	m_MaxUserSysTime.wMinute = 0;

	m_bIsServerShutdowned = false;
	m_cShutDownCode = 0;

	m_iMiddlelandMapIndex = -1;
	m_iAresdenOccupyTiles = 0;
	m_iElvineOccupyTiles = 0;

	m_iCurMsgs = 0;
	m_iMaxMsgs = 0;

	m_stCityStatus[1].iCrimes = 0;
	m_stCityStatus[1].iFunds = 0;
	m_stCityStatus[1].iWins = 0;

	m_stCityStatus[2].iCrimes = 0;
	m_stCityStatus[2].iFunds = 0;
	m_stCityStatus[2].iWins = 0;

	m_iAutoRebootingCount = 0;
	m_bEnemyKillMode = false;
	m_iEnemyKillAdjust = 1;
	m_sRaidTimeMonday = 0;
	m_sRaidTimeTuesday = 0;
	m_sRaidTimeWednesday = 0;
	m_sRaidTimeThursday = 0;
	m_sRaidTimeFriday = 0;
	m_sRaidTimeSaturday = 0;
	m_sRaidTimeSunday = 0;
	m_sCharPointLimit = 0;
	m_sSlateSuccessRate = 0;
	m_sForceRecallTime = 0;
	m_fPrimaryDropRate = 1.0f;    // 1.0 = normal (10% base), 1.5 = 150%, etc.
	m_fGoldDropRate = 1.0f;       // 1.0 = normal (30% base), 1.5 = 150%, etc.
	m_fSecondaryDropRate = 1.0f;  // 1.0 = normal (5% base), 1.5 = 150%, etc.

	//Show Debug hb::shared::render::Window
	//DbgWnd = new CDebugWindow();
	//DbgWnd->Startup();
	//DbgWnd->AddEventMsg("CGame Startup");
	// 2002-09-09 #1
	m_bReceivedItemList = false;

	for(int i = 0; i < MaxClientLoginSock; i++)
		_lclients[i] = nullptr;

	m_pPartyManager = new class PartyManager(this);

}

CGame::~CGame()
{
	//DbgWnd->Shutdown();
	//delete DbgWnd;

	for(int i = 0; i < MaxClientLoginSock; i++)
	{
		if (_lclients[i])
		{
			delete _lclients[i];
			_lclients[i] = nullptr;
		}
	}

	delete m_pPartyManager;

	// Cleanup Entity Manager
	if (m_pEntityManager != NULL) {
		delete m_pEntityManager;
		m_pEntityManager = NULL;
	}

	// Cleanup Gathering Managers
	if (m_pFishingManager != nullptr) {
		delete m_pFishingManager;
		m_pFishingManager = nullptr;
	}
	if (m_pMiningManager != nullptr) {
		delete m_pMiningManager;
		m_pMiningManager = nullptr;
	}
	if (m_pCraftingManager != nullptr) {
		delete m_pCraftingManager;
		m_pCraftingManager = nullptr;
	}
	if (m_pQuestManager != nullptr) {
		delete m_pQuestManager;
		m_pQuestManager = nullptr;
	}
	if (m_pGuildManager != nullptr) {
		delete m_pGuildManager;
		m_pGuildManager = nullptr;
	}
	if (m_pDelayEventManager != nullptr) {
		m_pDelayEventManager->CleanupArrays();
		delete m_pDelayEventManager;
		m_pDelayEventManager = nullptr;
	}
	if (m_pDynamicObjectManager != nullptr) {
		m_pDynamicObjectManager->CleanupArrays();
		delete m_pDynamicObjectManager;
		m_pDynamicObjectManager = nullptr;
	}
	if (m_pLootManager != nullptr) {
		delete m_pLootManager;
		m_pLootManager = nullptr;
	}
	if (m_pCombatManager != nullptr) {
		delete m_pCombatManager;
	delete m_pItemManager;
	delete m_pMagicManager;
	delete m_pSkillManager;
	delete m_pWarManager;
	delete m_pStatusEffectManager;
		m_pCombatManager = nullptr;
	}
}

bool CGame::bAcceptLogin(hb::shared::net::ASIOSocket* sock)
{
	if (m_bIsGameStarted == false)
	{
		PutLogList("Closed Connection, not initialized");
		goto CLOSE_ANYWAY;
	}

	for(int i = 0; i < MaxClientLoginSock; i++)
	{
		auto& p = _lclients[i];
		if (!p)
		{
			p = new LoginClient(G_pIOPool->GetContext());
			sock->bAccept(p->_sock);  // MODERNIZED: Removed WM_USER_BOT_ACCEPT message ID
			std::memset(p->_ip, 0, sizeof(p->_ip));
			p->_sock->iGetPeerAddress(p->_ip);
			return true;
		}
	}

CLOSE_ANYWAY:

	// MODERNIZED: Removed m_hWnd parameter
	auto pTmpSock = new hb::shared::net::ASIOSocket(G_pIOPool->GetContext(), ServerSocketBlockLimit);
	sock->bAccept(pTmpSock);
	delete pTmpSock;

	return false;
}

bool CGame::bAccept(class hb::shared::net::ASIOSocket* pXSock)
{
	int iTotalip = 0, a;
	class hb::shared::net::ASIOSocket* pTmpSock;
	char cIPtoBan[21];
	FILE* pFile;

	if (m_bIsGameStarted == false)
		goto CLOSE_ANYWAY;

	for(int i = 1; i < MaxClients; i++)
		if (m_pClientList[i] == 0) {

			m_pClientList[i] = new class CClient(G_pIOPool->GetContext());
			bAddClientShortCut(i);
			m_pClientList[i]->m_dwSPTime = m_pClientList[i]->m_dwMPTime =
				m_pClientList[i]->m_dwHPTime = m_pClientList[i]->m_dwAutoSaveTime =
				m_pClientList[i]->m_dwTime = m_pClientList[i]->m_dwHungerTime = m_pClientList[i]->m_dwExpStockTime =
				m_pClientList[i]->m_dwRecentAttackTime = m_pClientList[i]->m_dwAutoExpTime = m_pClientList[i]->m_dwSpeedHackCheckTime =
				m_pClientList[i]->m_dwAfkActivityTime = GameClock::GetTimeMS();

			pXSock->bAccept(m_pClientList[i]->m_pXSock);

			std::memset(m_pClientList[i]->m_cIPaddress, 0, sizeof(m_pClientList[i]->m_cIPaddress));
			m_pClientList[i]->m_pXSock->iGetPeerAddress(m_pClientList[i]->m_cIPaddress);

			a = i;

			for(int v = 0; v < MaxBanned; v++)
			{
				if (strcmp(m_stBannedList[v].m_cBannedIPaddress, m_pClientList[i]->m_cIPaddress) == 0)
				{
					goto CLOSE_CONN;
				}
			}
			//centu: Anti-Downer
			for(int j = 0; j < MaxClients; j++) {
				if (m_pClientList[j] != 0) {
					if (strcmp(m_pClientList[j]->m_cIPaddress, m_pClientList[i]->m_cIPaddress) == 0) iTotalip++;
				}
			}
			if (iTotalip > 9) {
				std::memset(cIPtoBan, 0, sizeof(cIPtoBan));
				strcpy(cIPtoBan, m_pClientList[i]->m_cIPaddress);
				//opens cfg file
				pFile = fopen("GameConfigs/BannedList.cfg", "a");
				//shows log
				std::snprintf(G_cTxt, sizeof(G_cTxt), "<%d> IP Banned: (%s)", i, cIPtoBan);
				PutLogList(G_cTxt);
				//modifys cfg file
				fprintf(pFile, "banned-ip = %s", cIPtoBan);
				fprintf(pFile, "\n");
				fclose(pFile);

				//updates BannedList.cfg on the server
				for(int x = 0; x < MaxBanned; x++)
					if (strlen(m_stBannedList[x].m_cBannedIPaddress) == 0)
						strcpy(m_stBannedList[x].m_cBannedIPaddress, cIPtoBan);

				goto CLOSE_CONN;
			}

			std::snprintf(G_cTxt, sizeof(G_cTxt), "<%d> Client Connected: (%s)", i, m_pClientList[i]->m_cIPaddress);
			PutLogList(G_cTxt);

			m_iTotalClients++;

			if (m_iTotalClients > m_iMaxClients) {
				m_iMaxClients = m_iTotalClients;
				//GetLocalTime(&m_MaxUserSysTime);
				//std::snprintf(cTxt, sizeof(cTxt), "Maximum Players: %d", m_iMaxClients);
				//PutLogFileList(cTxt);
			}

			// m_pClientList[iClientH]->m_bIsInitComplete   .
			return true;
		}

CLOSE_ANYWAY:

	pTmpSock = new class hb::shared::net::ASIOSocket(G_pIOPool->GetContext(), ServerSocketBlockLimit);
	pXSock->bAccept(pTmpSock);
	delete pTmpSock;

	return false;

CLOSE_CONN:
	delete m_pClientList[a];
	m_pClientList[a] = 0;
	RemoveClientShortCut(a);
	return false;
}

bool CGame::bAcceptLoginFromAsync(asio::ip::tcp::socket&& peer)
{
	if (m_bIsGameStarted == false) return false;

	for(int i = 0; i < MaxClientLoginSock; i++)
	{
		auto& p = _lclients[i];
		if (!p)
		{
			p = new LoginClient(G_pIOPool->GetContext());
			p->_sock->bAcceptFromSocket(std::move(peer));
			std::memset(p->_ip, 0, sizeof(p->_ip));
			p->_sock->iGetPeerAddress(p->_ip);
			return true;
		}
	}

	return false;
}

bool CGame::bAcceptFromAsync(asio::ip::tcp::socket&& peer)
{
	int iTotalip = 0, a;
	char cIPtoBan[21];
	FILE* pFile;

	if (m_bIsGameStarted == false) return false;

	for(int i = 1; i < MaxClients; i++)
		if (m_pClientList[i] == 0) {

			m_pClientList[i] = new class CClient(G_pIOPool->GetContext());
			bAddClientShortCut(i);
			m_pClientList[i]->m_dwSPTime = m_pClientList[i]->m_dwMPTime =
				m_pClientList[i]->m_dwHPTime = m_pClientList[i]->m_dwAutoSaveTime =
				m_pClientList[i]->m_dwTime = m_pClientList[i]->m_dwHungerTime = m_pClientList[i]->m_dwExpStockTime =
				m_pClientList[i]->m_dwRecentAttackTime = m_pClientList[i]->m_dwAutoExpTime = m_pClientList[i]->m_dwSpeedHackCheckTime =
				m_pClientList[i]->m_dwAfkActivityTime = GameClock::GetTimeMS();

			m_pClientList[i]->m_pXSock->bAcceptFromSocket(std::move(peer));

			std::memset(m_pClientList[i]->m_cIPaddress, 0, sizeof(m_pClientList[i]->m_cIPaddress));
			m_pClientList[i]->m_pXSock->iGetPeerAddress(m_pClientList[i]->m_cIPaddress);

			a = i;

			for(int v = 0; v < MaxBanned; v++)
			{
				if (strcmp(m_stBannedList[v].m_cBannedIPaddress, m_pClientList[i]->m_cIPaddress) == 0)
				{
					goto CLOSE_CONN_ASYNC;
				}
			}

			for(int j = 0; j < MaxClients; j++) {
				if (m_pClientList[j] != 0) {
					if (strcmp(m_pClientList[j]->m_cIPaddress, m_pClientList[i]->m_cIPaddress) == 0) iTotalip++;
				}
			}
			if (iTotalip > 9) {
				std::memset(cIPtoBan, 0, sizeof(cIPtoBan));
				strcpy(cIPtoBan, m_pClientList[i]->m_cIPaddress);
				pFile = fopen("GameConfigs/BannedList.cfg", "a");
				std::snprintf(G_cTxt, sizeof(G_cTxt), "<%d> IP Banned: (%s)", i, cIPtoBan);
				PutLogList(G_cTxt);
				fprintf(pFile, "banned-ip = %s", cIPtoBan);
				fprintf(pFile, "\n");
				fclose(pFile);

				for(int x = 0; x < MaxBanned; x++)
					if (strlen(m_stBannedList[x].m_cBannedIPaddress) == 0)
						strcpy(m_stBannedList[x].m_cBannedIPaddress, cIPtoBan);

				goto CLOSE_CONN_ASYNC;
			}

			std::snprintf(G_cTxt, sizeof(G_cTxt), "<%d> Client Connected: (%s)", i, m_pClientList[i]->m_cIPaddress);
			PutLogList(G_cTxt);

			m_iTotalClients++;

			if (m_iTotalClients > m_iMaxClients) {
				m_iMaxClients = m_iTotalClients;
			}

			// Set up async callbacks and start async read
			m_pClientList[i]->m_pXSock->SetSocketIndex(i);
			{
				hb::shared::net::ASIOSocket* pSock = m_pClientList[i]->m_pXSock;
				m_pClientList[i]->m_pXSock->SetCallbacks(
					[this, pSock](int idx, const char* data, size_t size, char key) {
						// Fast-track ping: respond immediately from I/O thread for accurate latency
						if (size >= sizeof(hb::net::PacketCommandCheckConnection)) {
							const auto* hdr = reinterpret_cast<const hb::net::PacketHeader*>(data);
							if (hdr->msg_id == MsgId::CommandCheckConnection) {
								const auto* ping = reinterpret_cast<const hb::net::PacketCommandCheckConnection*>(data);
								hb::net::PacketCommandCheckConnection resp{};
								resp.header.msg_id = MsgId::CommandCheckConnection;
								resp.header.msg_type = MsgType::Confirm;
								resp.time_ms = ping->time_ms;
								pSock->iSendMsg(reinterpret_cast<char*>(&resp), sizeof(resp));
								// Still queue it so MsgProcess updates m_dwTime and timeout tracking
							}
						}
						bPutMsgQuene(Source::Client, (char*)data, size, idx, key);
					},
					[](int idx, int err) {
						extern hb::shared::net::ConcurrentQueue<hb::shared::net::SocketErrorEvent> G_errorQueue;
						G_errorQueue.Push(hb::shared::net::SocketErrorEvent{idx, err});
					}
				);
			}
			m_pClientList[i]->m_pXSock->StartAsyncRead();

			return true;
		}

	return false;

CLOSE_CONN_ASYNC:
	delete m_pClientList[a];
	m_pClientList[a] = 0;
	RemoveClientShortCut(a);
	return false;
}

// MODERNIZED: No longer uses window messages, directly polls socket
void CGame::OnClientSocketEvent(int iClientH)
{
	int iRet;
	uint32_t dwTime = GameClock::GetTimeMS();

	if (iClientH <= 0) return;

	if (m_pClientList[iClientH] == 0) return;

	iRet = m_pClientList[iClientH]->m_pXSock->Poll();  // MODERNIZED: Poll() instead of iOnSocketEvent()

	switch (iRet) {
	case sock::Event::ReadComplete:
		OnClientRead(iClientH);
		m_pClientList[iClientH]->m_dwTime = GameClock::GetTimeMS();
		break;

	case sock::Event::Block:
		std::snprintf(G_cTxt, sizeof(G_cTxt), "[WARN] Client %d Socket BLOCKED (send buffer full)", iClientH);
		PutLogList(G_cTxt);
		break;

	case sock::Event::ConfirmCodeNotMatch:
		std::snprintf(G_cTxt, sizeof(G_cTxt), "<%d> Confirmcode notmatch!", iClientH);
		PutLogList(G_cTxt);
		DeleteClient(iClientH, false, true);
		break;

	case sock::Event::MsgSizeTooLarge:
		std::snprintf(G_cTxt, sizeof(G_cTxt), "<%d> Client Disconnected! MSGSIZETOOLARGE (%s)", iClientH, m_pClientList[iClientH]->m_cIPaddress);
		PutLogList(G_cTxt);
		DeleteClient(iClientH, true, true);
		break;

	case sock::Event::SocketError:
		std::snprintf(G_cTxt, sizeof(G_cTxt),
			"<%d> Client Disconnected! SOCKETERROR (%s) WSA=%d LastMsg=0x%08X LastMsgAge=%dms LastMsgSize=%zu CharName=%s",
			iClientH, m_pClientList[iClientH]->m_cIPaddress, m_pClientList[iClientH]->m_pXSock->m_WSAErr,
			m_pClientList[iClientH]->m_dwLastMsgId,
			dwTime - m_pClientList[iClientH]->m_dwLastMsgTime,
			m_pClientList[iClientH]->m_dwLastMsgSize,
			m_pClientList[iClientH]->m_cCharName);
		PutLogList(G_cTxt);
		DeleteClient(iClientH, true, true);
		break;

	case sock::Event::SocketClosed:
		std::snprintf(G_cTxt, sizeof(G_cTxt),
			"<%d> Client Disconnected! SOCKETCLOSED (%s) WSA=%d TimeSinceLastPacket=%dms LastMsg=0x%08X LastMsgAge=%dms LastMsgSize=%zu CharName=%s",
			iClientH, m_pClientList[iClientH]->m_cIPaddress, m_pClientList[iClientH]->m_pXSock->m_WSAErr,
			dwTime - m_pClientList[iClientH]->m_dwTime,
			m_pClientList[iClientH]->m_dwLastMsgId,
			dwTime - m_pClientList[iClientH]->m_dwLastMsgTime,
			m_pClientList[iClientH]->m_dwLastMsgSize,
			m_pClientList[iClientH]->m_cCharName);
		PutLogList(G_cTxt);
		if ((dwTime - m_pClientList[iClientH]->m_dwLogoutHackCheck) < 1000) {
			try
			{
				std::snprintf(G_cTxt, sizeof(G_cTxt), "Logout Hack: (%s) Player: (%s) - disconnected within 10 seconds of most recent damage. Hack? Lag?", m_pClientList[iClientH]->m_cIPaddress, m_pClientList[iClientH]->m_cCharName);
				PutHackLogFileList(G_cTxt);
			}
			catch (...)
			{
			}
		}

		DeleteClient(iClientH, true, true);
		break;

	case sock::Event::CriticalError:
		std::snprintf(G_cTxt, sizeof(G_cTxt),
			"<%d> Client Disconnected! CRITICALERROR (%s) WSA=%d LastMsg=0x%08X LastMsgAge=%dms LastMsgSize=%zu CharName=%s",
			iClientH, m_pClientList[iClientH]->m_cIPaddress, m_pClientList[iClientH]->m_pXSock->m_WSAErr,
			m_pClientList[iClientH]->m_dwLastMsgId,
			dwTime - m_pClientList[iClientH]->m_dwLastMsgTime,
			m_pClientList[iClientH]->m_dwLastMsgSize,
			m_pClientList[iClientH]->m_cCharName);
		PutLogList(G_cTxt);
		DeleteClient(iClientH, true, true);
		break;
	}
}

bool CGame::bInit()
{
	SYSTEMTIME SysTime;
	uint32_t dwTime = GameClock::GetTimeMS();

	//CMisc::Temp();

	PutLogListLevel(LOG_LEVEL_NOTICE, "Initializing game server...");

	for(int i = 0; i < MaxClients + 1; i++)
		m_iClientShortCut[i] = 0;

	if (_lsock != 0)
		delete _lsock;

	for(int i = 0; i < MaxClients; i++)
		if (m_pClientList[i] != 0) delete m_pClientList[i];

	for(int i = 0; i < MaxNpcs; i++)
		if (m_pNpcList[i] != 0) delete m_pNpcList[i];

	for(int i = 0; i < MaxMaps; i++)
		if (m_pMapList[i] != 0) delete m_pMapList[i];

	for(int i = 0; i < MaxItemTypes; i++)
		if (m_pItemConfigList[i] != 0) delete m_pItemConfigList[i];

	for(int i = 0; i < MaxNpcTypes; i++)
		if (m_pNpcConfigList[i] != 0) delete m_pNpcConfigList[i];

	for(int i = 0; i < hb::shared::limits::MaxMagicType; i++)
		if (m_pMagicConfigList[i] != 0) delete m_pMagicConfigList[i];

	for(int i = 0; i < hb::shared::limits::MaxSkillType; i++)
		if (m_pSkillConfigList[i] != 0) delete m_pSkillConfigList[i];




	for(int i = 0; i < MaxNotifyMsgs; i++)
		if (m_pNoticeMsgList[i] != 0) delete m_pNoticeMsgList[i];




	//	for(int i = 0; i < DEF_MAXTELEPORTTYPE; i++)
	//	if (m_pTeleportConfigList[i] != 0) delete m_pTeleportConfigList[i];

	for(int i = 0; i < hb::shared::limits::MaxBuildItems; i++)
		if (m_pBuildItemList[i] != 0) delete m_pBuildItemList[i];

	for(int i = 0; i < MaxNpcTypes; i++)
		m_iNpcConstructionPoint[i] = 0;


	for(int i = 0; i < MaxSchedule; i++) {
		m_stCrusadeWarSchedule[i].iDay = -1;
		m_stCrusadeWarSchedule[i].iHour = -1;
		m_stCrusadeWarSchedule[i].iMinute = -1;
	}

	for(int i = 0; i < MaxApocalypse; i++) {
		m_stApocalypseScheduleStart[i].iDay = -1;
		m_stApocalypseScheduleStart[i].iHour = -1;
		m_stApocalypseScheduleStart[i].iMinute = -1;
	}

	for(int i = 0; i < MaxHeldenian; i++) {
		m_stHeldenianSchedule[i].iDay = -1;
		m_stHeldenianSchedule[i].StartiHour = -1;
		m_stHeldenianSchedule[i].StartiMinute = -1;
		m_stHeldenianSchedule[i].EndiHour = -1;
		m_stHeldenianSchedule[i].EndiMinute = -1;
	}

	for(int i = 0; i < MaxApocalypse; i++) {
		m_stApocalypseScheduleEnd[i].iDay = -1;
		m_stApocalypseScheduleEnd[i].iHour = -1;
		m_stApocalypseScheduleEnd[i].iMinute = -1;
	}

	m_iNpcConstructionPoint[1] = 100;
	m_iNpcConstructionPoint[2] = 100;
	m_iNpcConstructionPoint[3] = 100;
	m_iNpcConstructionPoint[4] = 100;
	m_iNpcConstructionPoint[5] = 100;
	m_iNpcConstructionPoint[6] = 100;

	m_iNpcConstructionPoint[43] = 1000; // LWB
	m_iNpcConstructionPoint[44] = 2000; // GHK
	m_iNpcConstructionPoint[45] = 3000; // GHKABS
	m_iNpcConstructionPoint[46] = 2000;
	m_iNpcConstructionPoint[47] = 3000;
	m_iNpcConstructionPoint[51] = 1500; // Catapult

	m_bIsGameStarted = false;

	_lsock = 0;

	for(int i = 0; i < MaxClients; i++)
		m_pClientList[i] = 0;

	for(int i = 0; i < MaxMaps; i++)
		m_pMapList[i] = 0;

	for(int i = 0; i < MaxItemTypes; i++)
		m_pItemConfigList[i] = 0;

	for(int i = 0; i < MaxNpcTypes; i++)
		m_pNpcConfigList[i] = 0;

	for(int i = 0; i < MaxNpcs; i++)
		m_pNpcList[i] = 0;

	for(int i = 0; i < hb::shared::limits::MaxMagicType; i++)
		m_pMagicConfigList[i] = 0;

	for(int i = 0; i < hb::shared::limits::MaxSkillType; i++)
		m_pSkillConfigList[i] = 0;




	for(int i = 0; i < MaxNotifyMsgs; i++)
		m_pNoticeMsgList[i] = 0;




	//	for(int i = 0; i < DEF_MAXTELEPORTTYPE; i++)
	//		m_pTeleportConfigList[i] = 0;

	for(int i = 0; i < hb::shared::limits::MaxBuildItems; i++)
		m_pBuildItemList[i] = 0;

	for(int i = 0; i < hb::shared::limits::MaxCrusadeStructures; i++) {
		std::memset(m_stCrusadeStructures[i].cMapName, 0, sizeof(m_stCrusadeStructures[i].cMapName));
		m_stCrusadeStructures[i].cType = 0;
		m_stCrusadeStructures[i].dX = 0;
		m_stCrusadeStructures[i].dY = 0;
	}

	for(int i = 0; i < MaxBanned; i++) {
		std::memset(m_stBannedList[i].m_cBannedIPaddress, 0, sizeof(m_stBannedList[i].m_cBannedIPaddress));
	}

	for(int i = 0; i < MaxGuilds; i++)
		m_pGuildTeleportLoc[i].m_iV1 = 0;

	for(int i = 0; i < hb::shared::limits::MaxCrusadeStructures; i++) {
		m_stMiddleCrusadeStructureInfo[i].cType = 0;
		m_stMiddleCrusadeStructureInfo[i].cSide = 0;
		m_stMiddleCrusadeStructureInfo[i].sX = 0;
		m_stMiddleCrusadeStructureInfo[i].sY = 0;
	}
	m_iTotalMiddleCrusadeStructures = 0;

	m_pNoticementData = 0;

	m_iTotalClients = 0;
	m_iMaxClients = 0;
	m_iTotalMaps = 0;

	m_iTotalGameServerClients = 0;
	m_iTotalGameServerMaxClients = 0;

	m_MaxUserSysTime.wHour = 0;
	m_MaxUserSysTime.wMinute = 0;

	m_bIsServerShutdowned = false;
	m_cShutDownCode = 0;

	m_iMiddlelandMapIndex = -1;
	m_iAresdenMapIndex = -1;
	m_iElvineMapIndex = -1;
	m_iGodHMapIndex = -1;
	m_iBTFieldMapIndex = -1;

	m_iAresdenOccupyTiles = 0;
	m_iElvineOccupyTiles = 0;

	m_iCurMsgs = 0;
	m_iMaxMsgs = 0;

	m_stCityStatus[1].iCrimes = 0;
	m_stCityStatus[1].iFunds = 0;
	m_stCityStatus[1].iWins = 0;

	m_stCityStatus[2].iCrimes = 0;
	m_stCityStatus[2].iFunds = 0;
	m_stCityStatus[2].iWins = 0;

	m_iStrategicStatus = 0;

	m_iCollectedMana[0] = 0;
	m_iCollectedMana[1] = 0;
	m_iCollectedMana[2] = 0;

	m_iAresdenMana = 0;
	m_iElvineMana = 0;

	if (m_pFishingManager != nullptr) m_pFishingManager->m_dwFishTime = dwTime;
	m_dwSpecialEventTime = m_dwWhetherTime = m_dwGameTime1 =
		m_dwGameTime2 = m_dwGameTime3 = m_dwGameTime4 = m_dwGameTime5 = m_dwGameTime6 = dwTime;

	m_bIsSpecialEventTime = false;

	GetLocalTime(&SysTime);
	m_dwCanFightzoneReserveTime = dwTime - ((SysTime.wHour % 2) * 60 * 60 + SysTime.wMinute * 60) * 1000;

	for(int i = 0; i < MaxFightZone; i++)
		m_iFightZoneReserve[i] = 0;

	m_iFightzoneNoForceRecall = 0;

	for(int i = 1; i < 1000; i++) {
		m_iLevelExpTable[i] = iGetLevelExp(i);
		//testcode
		//std::snprintf(G_cTxt, sizeof(G_cTxt), "Level:%d --- Exp:%d", i, m_iLevelExpTable[i]);
		//PutLogFileList(G_cTxt);
	}

	m_iLevelExp20 = m_iLevelExpTable[20];

	sqlite3* configDb = nullptr;
	std::string configDbPath;
	bool configDbCreated = false;
	PutLogListLevel(LOG_LEVEL_NOTICE, "Validating GameConfigs.db...");
	bool configDbReady = EnsureGameConfigDatabase(&configDb, configDbPath, &configDbCreated);
	if (!configDbReady) {
		PutLogList(" ");
		PutLogList("(!!!) CRITICAL ERROR! Cannot execute server! GameConfigs.db unavailable.");
		return false;
	}
	if (configDbCreated) {
		PutLogList(" ");
		PutLogList("(!!!) CRITICAL ERROR! Cannot execute server! GameConfigs.db missing configuration data.");
		CloseGameConfigDatabase(configDb);
		return false;
	}

	if (!HasGameConfigRows(configDb, "realmlist") || !HasGameConfigRows(configDb, "active_maps") ||
		!LoadRealmConfig(configDb, this)) {
		PutLogList(" ");
		PutLogList("(!!!) CRITICAL ERROR! Cannot execute server! Program configs missing in GameConfigs.db.");
		CloseGameConfigDatabase(configDb);
		return false;
	}

	if (!HasGameConfigRows(configDb, "settings") || !LoadSettingsConfig(configDb, this)) {
		PutLogList(" ");
		PutLogList("(!!!) CRITICAL ERROR! Cannot execute server! Settings configs missing in GameConfigs.db.");
		CloseGameConfigDatabase(configDb);
		return false;
	}

	// Calculate m_iMaxStatValue after settings are loaded
	// Formula: base + creation + (levelup * max_level) + angelic_max(16)
	m_iMaxStatValue = m_iBaseStatValue + m_iCreationStatBonus + (m_iLevelupStatGain * m_iMaxLevel) + 16;
	std::snprintf(G_cTxt, sizeof(G_cTxt), "(!) Max stat value calculated: %d", m_iMaxStatValue);
	PutLogList(G_cTxt);

	if (!LoadBannedListConfig(configDb, this)) {
		PutLogList(" ");
		PutLogList("(!!!) CRITICAL ERROR! Cannot execute server! Banned list unavailable in GameConfigs.db.");
		CloseGameConfigDatabase(configDb);
		return false;
	}

	if (!LoadAdminConfig(configDb, this)) {
		PutLogList("(!) Warning: Could not load admin config from GameConfigs.db. Admin list empty.");
	}
	else {
		std::snprintf(G_cTxt, sizeof(G_cTxt), "(!) Loaded %d admin(s) from GameConfigs.db.", m_iAdminCount);
		PutLogList(G_cTxt);
	}

	if (!LoadCommandPermissions(configDb, this)) {
		PutLogList("(!) Warning: Could not load command permissions from GameConfigs.db.");
	}
	else if (!m_commandPermissions.empty()) {
		std::snprintf(G_cTxt, sizeof(G_cTxt), "(!) Loaded %d command permission override(s) from GameConfigs.db.", (int)m_commandPermissions.size());
		PutLogList(G_cTxt);
	}

	srand((unsigned)time(0));

	_lsock = new class hb::shared::net::ASIOSocket(G_pIOPool->GetContext(), ServerSocketBlockLimit);
	_lsock->bConnect(m_cLoginListenIP, m_iLoginListenPort);
	_lsock->bInitBufferSize(hb::shared::limits::MsgBufferSize);

	m_bF1pressed = m_bF4pressed = m_bF12pressed = m_bF5pressed = false;

	m_bOnExitProcess = false;

	for(int i = 0; i <= 100; i++) {
		m_iSkillSSNpoint[i] = m_pSkillManager->_iCalcSkillSSNpoint(i);
	}

	GetLocalTime(&SysTime);
	if (SysTime.wMinute >= m_iNighttimeDuration)
		m_cDayOrNight = 2;
	else m_cDayOrNight = 1;

	bReadNotifyMsgListFile("GameConfigs/notice.txt");
	m_dwNoticeTime = dwTime;

	m_pNoticementData = 0;
	m_dwNoticementDataSize = 0;

	m_dwMapSectorInfoTime = dwTime;
	m_iMapSectorInfoUpdateCount = 0;

	m_iCrusadeCount = 0;
	m_bIsCrusadeMode = false;
	m_bIsApocalypseMode = false;
	m_dwCrusadeGUID = 0;
	m_iCrusadeWinnerSide = 0;
	m_iLastCrusadeWinner = 0;
	m_sLastHeldenianWinner = 0;
	m_sLastCrusadeDate = -1;
	m_iFinalShutdownCount = 0;
	m_bIsCrusadeWarStarter = false;
	m_bIsApocalypseStarter = false;
	m_iLatestCrusadeDayOfWeek = -1;

	m_bHeldenianInitiated = false;
	m_cHeldenianType = false;
	m_bIsHeldenianMode = false;
	m_bHeldenianRunning = false;
	m_iHeldenianAresdenLeftTower = 0;
	m_cHeldenianModeType = -1;
	m_sLastHeldenianWinner = -1;
	m_iHeldenianAresdenLeftTower = 0;
	m_iHeldenianElvineLeftTower = 0;
	m_iHeldenianAresdenDead = 0;
	m_iHeldenianElvineDead = 0;

	int dwMsgSize = 0;
	m_bIsItemAvailable = false;
	if (HasGameConfigRows(configDb, "items")) {
		m_bIsItemAvailable = LoadItemConfigs(configDb, m_pItemConfigList, MaxItemTypes);
	}
	if (!m_bIsItemAvailable) {
		PutLogList(" ");
		PutLogList("(!!!) CRITICAL ERROR! Cannot execute server! Item configs missing in GameConfigs.db.");
		CloseGameConfigDatabase(configDb);
		return false;
	}

	m_bIsBuildItemAvailable = false;
	if (HasGameConfigRows(configDb, "builditem_configs")) {
		m_bIsBuildItemAvailable = LoadBuildItemConfigs(configDb, this);
	}
	if (!m_bIsBuildItemAvailable) {
		PutLogList(" ");
		PutLogList("(!!!) CRITICAL ERROR! Cannot execute server! BuildItem configs missing in GameConfigs.db.");
		CloseGameConfigDatabase(configDb);
		return false;
	}

	m_bIsNpcAvailable = false;
	if (HasGameConfigRows(configDb, "npc_configs")) {
		m_bIsNpcAvailable = LoadNpcConfigs(configDb, this);
	}
	if (!m_bIsNpcAvailable) {
		PutLogList(" ");
		PutLogList("(!!!) CRITICAL ERROR! Cannot execute server! NPC configs missing in GameConfigs.db.");
		CloseGameConfigDatabase(configDb);
		return false;
	}

	m_bIsDropTableAvailable = false;
	if (HasGameConfigRows(configDb, "drop_tables") && HasGameConfigRows(configDb, "drop_entries")) {
		m_bIsDropTableAvailable = LoadDropTables(configDb, this);
	}
	if (!m_bIsDropTableAvailable) {
		PutLogList(" ");
		PutLogList("(!!!) CRITICAL ERROR! Cannot execute server! Drop tables missing in GameConfigs.db.");
		CloseGameConfigDatabase(configDb);
		return false;
	}

	{
		int missingDrops = 0;
		for(int i = 0; i < MaxNpcTypes; i++) {
			const CNpc* npc = m_pNpcConfigList[i];
			if (npc == nullptr) {
				continue;
			}
			if (npc->m_iDropTableId == 0 &&
				(npc->m_iExpDiceMax > 0 || npc->m_iGoldDiceMax > 0)) {
				char logMsg[256] = {};
				std::snprintf(logMsg, sizeof(logMsg),
					"NPC missing drop table: %s (exp %u-%u, gold %u-%u)",
					npc->m_cNpcName,
					static_cast<unsigned int>(npc->m_iExpDiceMin),
					static_cast<unsigned int>(npc->m_iExpDiceMax),
					static_cast<unsigned int>(npc->m_iGoldDiceMin),
					static_cast<unsigned int>(npc->m_iGoldDiceMax));
				PutLogListLevel(LOG_LEVEL_WARNING, logMsg);
				missingDrops++;
			}
		}
		if (missingDrops > 0) {
			char logMsg[128] = {};
			std::snprintf(logMsg, sizeof(logMsg), "NPCs missing drop tables: %d", missingDrops);
			PutLogListLevel(LOG_LEVEL_WARNING, logMsg);
		}
	}

	// Load shop configurations (optional - server works without shops)
	m_bIsShopDataAvailable = false;
	if (HasGameConfigRows(configDb, "npc_shop_mapping") || HasGameConfigRows(configDb, "shop_items")) {
		LoadShopConfigs(configDb, this);
	}
	if (!m_bIsShopDataAvailable) {
		PutLogList("(!) Shop data not configured - NPCs will not have shop inventories.");
	}

	m_bIsMagicAvailable = false;
	if (HasGameConfigRows(configDb, "magic_configs")) {
		m_bIsMagicAvailable = LoadMagicConfigs(configDb, this);
	}
	if (!m_bIsMagicAvailable) {
		PutLogList(" ");
		PutLogList("(!!!) CRITICAL ERROR! Cannot execute server! Magic configs missing in GameConfigs.db.");
		CloseGameConfigDatabase(configDb);
		return false;
	}

	m_bIsSkillAvailable = false;
	if (HasGameConfigRows(configDb, "skill_configs")) {
		m_bIsSkillAvailable = LoadSkillConfigs(configDb, this);
	}
	if (!m_bIsSkillAvailable) {
		PutLogList(" ");
		PutLogList("(!!!) CRITICAL ERROR! Cannot execute server! Skill configs missing in GameConfigs.db.");
		CloseGameConfigDatabase(configDb);
		return false;
	}

	ComputeConfigHashes();

	m_bIsQuestAvailable = false;
	if (HasGameConfigRows(configDb, "quest_configs")) {
		m_bIsQuestAvailable = LoadQuestConfigs(configDb, this);
	}
	if (!m_bIsQuestAvailable) {
		PutLogList(" ");
		PutLogList("(!!!) CRITICAL ERROR! Cannot execute server! Quest configs missing in GameConfigs.db.");
		CloseGameConfigDatabase(configDb);
		return false;
	}

	m_bIsPortionAvailable = false;
	if (HasGameConfigRows(configDb, "potion_configs") || HasGameConfigRows(configDb, "crafting_configs")) {
		m_bIsPortionAvailable = LoadPortionConfigs(configDb, this);
	}
	if (!m_bIsPortionAvailable) {
		PutLogList(" ");
		PutLogList("(!!!) CRITICAL ERROR! Cannot execute server! Potion/Crafting configs missing in GameConfigs.db.");
		CloseGameConfigDatabase(configDb);
		return false;
	}

	CloseGameConfigDatabase(configDb);

	return true;
}

void CGame::OnClientRead(int iClientH)
{
	char* pData, cKey;
	size_t  dwMsgSize;

	if (m_pClientList[iClientH] == 0) return;

	pData = m_pClientList[iClientH]->m_pXSock->pGetRcvDataPointer(&dwMsgSize, &cKey); // v1.4

	const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(
		pData, sizeof(hb::net::PacketHeader));
	if (header) {
		m_pClientList[iClientH]->m_dwLastMsgId = header->msg_id;
		m_pClientList[iClientH]->m_dwLastMsgTime = GameClock::GetTimeMS();
		m_pClientList[iClientH]->m_dwLastMsgSize = dwMsgSize;

		// Fast-track ping responses: bypass the message queue (which only drains every 300ms)
		// so latency measurement reflects actual round-trip time
		if (header->msg_id == MsgId::CommandCheckConnection) {
			CheckConnectionHandler(iClientH, pData);
			return;
		}
	}

	if (bPutMsgQuene(Source::Client, pData, dwMsgSize, iClientH, cKey) == false) {
		PutLogList("@@@@@@ CRITICAL ERROR in MsgQuene!!! @@@@@@");
	}
}


void CGame::ClientMotionHandler(int iClientH, char* pData)
{
	uint32_t dwClientTime;
	uint16_t wCommand, wTargetObjectID = 0;
	short sX, sY, dX, dY, wType;
	char cDir;
	int   iRet, iTemp;

	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_bIsInitComplete == false) return;
	if (m_pClientList[iClientH]->m_bIsKilled) return;

	const auto* base = hb::net::PacketCast<hb::net::PacketCommandMotionBase>(
		pData, sizeof(hb::net::PacketCommandMotionBase));
	if (!base) return;
	wCommand = base->header.msg_type;
	sX = base->x;
	sY = base->y;
	cDir = static_cast<char>(base->dir);
	dX = base->dx;
	dY = base->dy;
	wType = base->type;

	if ((wCommand == Type::Attack) || (wCommand == Type::AttackMove)) { // v1.4
		const auto* pkt = hb::net::PacketCast<hb::net::PacketCommandMotionAttack>(
			pData, sizeof(hb::net::PacketCommandMotionAttack));
		if (!pkt) return;
		wTargetObjectID = pkt->target_id;
		dwClientTime = pkt->time_ms;
	}
	else {
		const auto* pkt = hb::net::PacketCast<hb::net::PacketCommandMotionSimple>(
			pData, sizeof(hb::net::PacketCommandMotionSimple));
		if (!pkt) return;
		dwClientTime = pkt->time_ms;
	}

	switch (wCommand) {
	case Type::Stop:
		iRet = iClientMotion_Stop_Handler(iClientH, sX, sY, cDir);
		if (iRet == 1) {
			SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventMotion, Type::Stop, 0, 0, 0);
		}
		else if (iRet == 2) SendObjectMotionRejectMsg(iClientH);
		break;

	case Type::Run:
		iRet = iClientMotion_Move_Handler(iClientH, sX, sY, cDir, 1);
		if (iRet == 1) {
			SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventMotion, Type::Run, 0, 0, 0);
		}
		else if (iRet == 2) SendObjectMotionRejectMsg(iClientH);
		if ((m_pClientList[iClientH] != 0) && (m_pClientList[iClientH]->m_iHP <= 0)) m_pCombatManager->ClientKilledHandler(iClientH, 0, 0, 1); // v1.4
		// v2.171
		bCheckClientMoveFrequency(iClientH, dwClientTime);
		break;

	case Type::Move:
		iRet = iClientMotion_Move_Handler(iClientH, sX, sY, cDir, 2);
		if (iRet == 1) {
			SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventMotion, Type::Move, 0, 0, 0);
		}
		else if (iRet == 2) SendObjectMotionRejectMsg(iClientH);
		if ((m_pClientList[iClientH] != 0) && (m_pClientList[iClientH]->m_iHP <= 0)) m_pCombatManager->ClientKilledHandler(iClientH, 0, 0, 1); // v1.4
		// v2.171
		bCheckClientMoveFrequency(iClientH, dwClientTime);
		break;

	case Type::DamageMove:
		iRet = iClientMotion_Move_Handler(iClientH, sX, sY, cDir, 0);
		if (iRet == 1) {
			SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventMotion, Type::DamageMove, m_pClientList[iClientH]->m_iLastDamage, 0, 0);
		}
		else if (iRet == 2) SendObjectMotionRejectMsg(iClientH);
		if ((m_pClientList[iClientH] != 0) && (m_pClientList[iClientH]->m_iHP <= 0)) m_pCombatManager->ClientKilledHandler(iClientH, 0, 0, 1); // v1.4
		break;

	case Type::AttackMove:
		iRet = iClientMotion_Move_Handler(iClientH, sX, sY, cDir, 0);
		if ((iRet == 1) && (m_pClientList[iClientH] != 0)) {
			SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventMotion, Type::AttackMove, 0, 0, 0);
			iClientMotion_Attack_Handler(iClientH, m_pClientList[iClientH]->m_sX, m_pClientList[iClientH]->m_sY, dX, dY, wType, cDir, wTargetObjectID, dwClientTime, false, true); // v1.4
		}
		else if (iRet == 2) SendObjectMotionRejectMsg(iClientH);
		if ((m_pClientList[iClientH] != 0) && (m_pClientList[iClientH]->m_iHP <= 0)) m_pCombatManager->ClientKilledHandler(iClientH, 0, 0, 1); // v1.4
		// v2.171
		m_pCombatManager->bCheckClientAttackFrequency(iClientH, dwClientTime);
		break;

	case Type::Attack:
		m_pCombatManager->_CheckAttackType(iClientH, &wType);
		iRet = iClientMotion_Attack_Handler(iClientH, sX, sY, dX, dY, wType, cDir, wTargetObjectID, dwClientTime); // v1.4
		if (iRet == 1) {
			if (wType >= 20) {
				m_pClientList[iClientH]->m_iSuperAttackLeft--;
				if (m_pClientList[iClientH]->m_iSuperAttackLeft < 0) m_pClientList[iClientH]->m_iSuperAttackLeft = 0;
				SendNotifyMsg(0, iClientH, Notify::SuperAttackLeft, 0, 0, 0, 0);
			}

			SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventMotion, Type::Attack, dX, dY, wType);
		}
		else if (iRet == 2) SendObjectMotionRejectMsg(iClientH);
		// v2.171
		m_pCombatManager->bCheckClientAttackFrequency(iClientH, dwClientTime);
		break;

	case Type::GetItem:
		iRet = m_pItemManager->iClientMotion_GetItem_Handler(iClientH, sX, sY, cDir);
		if (iRet == 1) {
			SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventMotion, Type::GetItem, 0, 0, 0);
		}
		else if (iRet == 2) SendObjectMotionRejectMsg(iClientH);
		break;

	case Type::Magic:
		iRet = m_pMagicManager->iClientMotion_Magic_Handler(iClientH, sX, sY, cDir);
		if (iRet == 1) {
			m_pClientList[iClientH]->m_bMagicPauseTime = true;
			iTemp = 10;
			SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventMotion, Type::Magic, dX, iTemp, 0);
			m_pClientList[iClientH]->m_iSpellCount++;
			m_pMagicManager->bCheckClientMagicFrequency(iClientH, dwClientTime);
		}
		else if (iRet == 2) SendObjectMotionRejectMsg(iClientH);
		break;

	default:
		break;
	}
}

int CGame::iClientMotion_Move_Handler(int iClientH, short sX, short sY, char cDir, char cMoveType)
{
	char moveMapData[3000];
	class CTile* pTile;
	DWORD dwTime;
	WORD  wObjectID;
	short dX, dY, sDOtype, pTopItem;
	int   iRet, iSize, iDamage;
	bool  bRet, bIsBlocked = false;
	hb::net::PacketWriter writer;

	if (m_pClientList[iClientH] == 0) return 0;
	if ((cDir <= 0) || (cDir > 8))       return 0;
	if (m_pClientList[iClientH]->m_bIsKilled) return 0;
	if (m_pClientList[iClientH]->m_bIsInitComplete == false) return 0;

	if ((sX != m_pClientList[iClientH]->m_sX) || (sY != m_pClientList[iClientH]->m_sY)) return 2;

	//locobans
	dwTime = timeGetTime();
	/*m_pClientList[iClientH]->m_dwLastActionTime = dwTime;
	if (cMoveType == 2) {
		if (m_pClientList[iClientH]->m_iRecentWalkTime > dwTime) {
			m_pClientList[iClientH]->m_iRecentWalkTime = dwTime;
			if (m_pClientList[iClientH]->m_sV1 < 1) {
				if (m_pClientList[iClientH]->m_iRecentWalkTime < dwTime) {
					m_pClientList[iClientH]->m_sV1++;
				}
				else {
					bIsBlocked = true;
					m_pClientList[iClientH]->m_sV1 = 0;
				}
			}
		m_pClientList[iClientH]->m_iRecentWalkTime = dwTime;
		}
		if (bIsBlocked == false) m_pClientList[iClientH]->m_iMoveMsgRecvCount++;
		if (m_pClientList[iClientH]->m_iMoveMsgRecvCount >= 3) {
			if (m_pClientList[iClientH]->m_dwMoveLAT != 0) {
				if ((dwTime - m_pClientList[iClientH]->m_dwMoveLAT) < (590)) {
					//wsprintf(G_cTxt, "3.51 Walk Speeder: (%s) Player: (%s) walk difference: %d. Speed Hack?", m_pClientList[iClientH]->m_cIPaddress, m_pClientList[iClientH]->m_cCharName, dwTime - m_pClientList[iClientH]->m_dwMoveLAT);
					//PutHackLogFileList(G_cTxt);
					bIsBlocked = true;
				}
			}
			m_pClientList[iClientH]->m_dwMoveLAT = dwTime;
			m_pClientList[iClientH]->m_iMoveMsgRecvCount = 0;
		}
	}
	else if (cMoveType == 1) {
		if (m_pClientList[iClientH]->m_iRecentRunTime > dwTime) {
			m_pClientList[iClientH]->m_iRecentRunTime = dwTime;
			if (m_pClientList[iClientH]->m_sV1 < 1) {
				if (m_pClientList[iClientH]->m_iRecentRunTime < dwTime) {
					m_pClientList[iClientH]->m_sV1++;
				}
				else {
					bIsBlocked = true;
					m_pClientList[iClientH]->m_sV1 = 0;
				}
			}
		m_pClientList[iClientH]->m_iRecentRunTime = dwTime;
		}
		if (bIsBlocked == false) m_pClientList[iClientH]->m_iRunMsgRecvCount++;
		if (m_pClientList[iClientH]->m_iRunMsgRecvCount >= 3) {
			if (m_pClientList[iClientH]->m_dwRunLAT != 0) {
				if ((dwTime - m_pClientList[iClientH]->m_dwRunLAT) < (290)) {
					//wsprintf(G_cTxt, "3.51 Run Speeder: (%s) Player: (%s) run difference: %d. Speed Hack?", m_pClientList[iClientH]->m_cIPaddress, m_pClientList[iClientH]->m_cCharName, dwTime - m_pClientList[iClientH]->m_dwRunLAT);
					//PutHackLogFileList(G_cTxt);
					bIsBlocked = true;
				}
			}
			m_pClientList[iClientH]->m_dwRunLAT	= dwTime;
			m_pClientList[iClientH]->m_iRunMsgRecvCount = 0;
		}
	}*/

	int iStX, iStY;
	if (m_pMapList[m_pClientList[iClientH]->m_cMapIndex] != 0) {
		iStX = m_pClientList[iClientH]->m_sX / 20;
		iStY = m_pClientList[iClientH]->m_sY / 20;
		m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_stTempSectorInfo[iStX][iStY].iPlayerActivity++;

		switch (m_pClientList[iClientH]->m_cSide) {
		case 0: m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_stTempSectorInfo[iStX][iStY].iNeutralActivity++; break;
		case 1: m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_stTempSectorInfo[iStX][iStY].iAresdenActivity++; break;
		case 2: m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_stTempSectorInfo[iStX][iStY].iElvineActivity++;  break;
		}
	}

	m_pSkillManager->ClearSkillUsingStatus(iClientH);

	dX = m_pClientList[iClientH]->m_sX;
	dY = m_pClientList[iClientH]->m_sY;
	hb::shared::direction::ApplyOffset(cDir, dX, dY);

	pTopItem = 0;
	bRet = m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->bGetMoveable(dX, dY, &sDOtype, &pTopItem);

	if (m_pClientList[iClientH]->m_cMagicEffectStatus[hb::shared::magic::HoldObject] != 0)
		bRet = false;

	if ((bRet) && (bIsBlocked == false)) {
		if (m_pClientList[iClientH]->m_iQuest != 0) m_pQuestManager->_bCheckIsQuestCompleted(iClientH);

		m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->ClearOwner(1, iClientH, hb::shared::owner_class::Player, m_pClientList[iClientH]->m_sX, m_pClientList[iClientH]->m_sY);

		m_pClientList[iClientH]->m_sX = dX;
		m_pClientList[iClientH]->m_sY = dY;
		m_pClientList[iClientH]->m_cDir = cDir;

		m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->SetOwner((short)iClientH,
			hb::shared::owner_class::Player,
			dX, dY);

		if (sDOtype == dynamic_object::Spike) {
			if ((m_pClientList[iClientH]->m_bIsNeutral) && (!m_pClientList[iClientH]->m_appearance.bIsWalking)) {

			}
			else {
				iDamage = iDice(2, 4);

				m_pClientList[iClientH]->m_iHP -= iDamage;
			}
		}

		if (m_pClientList[iClientH]->m_iHP <= 0) m_pClientList[iClientH]->m_iHP = 0;

		writer.Reset();
		auto* pkt = writer.Append<hb::net::PacketResponseMotionMoveConfirm>();
		pkt->header.msg_id = MsgId::ResponseMotion;
		pkt->header.msg_type = Confirm::MoveConfirm;
		pkt->x = static_cast<std::int16_t>(dX - hb::shared::view::CenterX);
		pkt->y = static_cast<std::int16_t>(dY - hb::shared::view::CenterY);
		pkt->dir = static_cast<std::uint8_t>(cDir);
		pkt->stamina_cost = 0;
		if (cMoveType == 1) {
			if (m_pClientList[iClientH]->m_iSP > 0) {
				if (m_pClientList[iClientH]->m_iTimeLeft_FirmStaminar == 0) {
					m_pClientList[iClientH]->m_iSP--;
					pkt->stamina_cost = 1;
				}
			}
		}

		pTile = (class CTile*)(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_pTile + dX + dY * m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_sSizeY);
		pkt->occupy_status = static_cast<std::uint8_t>(pTile->m_iOccupyStatus);
		pkt->hp = m_pClientList[iClientH]->m_iHP;

		iSize = iComposeMoveMapData((short)(dX - hb::shared::view::CenterX), (short)(dY - hb::shared::view::CenterY), iClientH, cDir, moveMapData);
		writer.AppendBytes(moveMapData, static_cast<std::size_t>(iSize));

		iRet = m_pClientList[iClientH]->m_pXSock->iSendMsg(writer.Data(), static_cast<int>(writer.Size()));
		switch (iRet) {
		case sock::Event::QueueFull:
		case sock::Event::SocketError:
		case sock::Event::CriticalError:
		case sock::Event::SocketClosed:
			DeleteClient(iClientH, true, true);
			return 0;
		}
	}
	else {
		m_pClientList[iClientH]->m_bIsMoveBlocked = true;

		m_pClientList[iClientH]->m_dwAttackLAT = 0;

		wObjectID = (WORD)iClientH;
		writer.Reset();
		auto* pkt = writer.Append<hb::net::PacketResponseMotionMoveReject>();
		pkt->header.msg_id = MsgId::ResponseMotion;
		pkt->header.msg_type = Confirm::MoveReject;
		pkt->object_id = static_cast<std::uint16_t>(wObjectID);
		pkt->x = m_pClientList[wObjectID]->m_sX;
		pkt->y = m_pClientList[wObjectID]->m_sY;
		pkt->type = m_pClientList[wObjectID]->m_sType;
		pkt->dir = static_cast<std::uint8_t>(m_pClientList[wObjectID]->m_cDir);
		std::memcpy(pkt->name, m_pClientList[wObjectID]->m_cCharName, sizeof(pkt->name));
		pkt->appearance = m_pClientList[wObjectID]->m_appearance;
		{
			auto pktStatus = m_pClientList[wObjectID]->m_status;
			pktStatus.bPK = (m_pClientList[wObjectID]->m_iPKCount != 0) ? 1 : 0;
			pktStatus.bCitizen = (m_pClientList[wObjectID]->m_cSide != 0) ? 1 : 0;
			pktStatus.bAresden = (m_pClientList[wObjectID]->m_cSide == 1) ? 1 : 0;
			pktStatus.bHunter = m_pClientList[wObjectID]->m_bIsPlayerCivil ? 1 : 0;
			pktStatus.iRelationship = m_pCombatManager->GetPlayerRelationship(wObjectID, iClientH);
			pkt->status = pktStatus;
		}
		pkt->padding = 0;

		iRet = m_pClientList[iClientH]->m_pXSock->iSendMsg(writer.Data(), static_cast<int>(writer.Size()));

		switch (iRet) {
		case sock::Event::QueueFull:
		case sock::Event::SocketError:
		case sock::Event::CriticalError:
		case sock::Event::SocketClosed:
			DeleteClient(iClientH, true, true);
			return 0;
		}
		// locobans
		SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventMotion, Type::NullAction, 0, 0, 0);
		return 0;
	}

	return 1;
}

void CGame::RequestInitPlayerHandler(int iClientH, char* pData, char cKey)
{
	
	char cCharName[hb::shared::limits::CharNameLen], cAccountName[hb::shared::limits::AccountNameLen], cAccountPassword[hb::shared::limits::AccountPassLen], cTxt[120];
	bool bIsObserverMode;

	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_bIsInitComplete) return;


	ZeroMemory(cCharName, sizeof(cCharName));
	ZeroMemory(cAccountName, sizeof(cAccountName));
	ZeroMemory(cAccountPassword, sizeof(cAccountPassword));

	ZeroMemory(m_pClientList[iClientH]->m_cCharName, sizeof(m_pClientList[iClientH]->m_cCharName));
	ZeroMemory(m_pClientList[iClientH]->m_cAccountName, sizeof(m_pClientList[iClientH]->m_cAccountName));
	ZeroMemory(m_pClientList[iClientH]->m_cAccountPassword, sizeof(m_pClientList[iClientH]->m_cAccountPassword));

	const auto* req = hb::net::PacketCast<hb::net::PacketRequestInitPlayer>(
		pData, sizeof(hb::net::PacketRequestInitPlayer));
	if (!req) return;

	memcpy(cCharName, req->player, hb::shared::limits::CharNameLen - 1);

	ZeroMemory(cTxt, sizeof(cTxt)); // v1.4
	memcpy(cTxt, cCharName, hb::shared::limits::CharNameLen - 1);
	ZeroMemory(cCharName, sizeof(cCharName));
	memcpy(cCharName, cTxt, hb::shared::limits::CharNameLen - 1);

	//testcode
	if (strlen(cTxt) == 0) PutLogList("RIPH - cTxt: Char 0!");

	memcpy(cAccountName, req->account, hb::shared::limits::AccountNameLen - 1);

	ZeroMemory(cTxt, sizeof(cTxt)); // v1.4
	memcpy(cTxt, cAccountName, hb::shared::limits::AccountNameLen - 1);
	ZeroMemory(cAccountName, sizeof(cAccountName));
	memcpy(cAccountName, cTxt, hb::shared::limits::AccountNameLen - 1);

	// Lowercase account name to match how it was stored during account creation
	for (int ci = 0; ci < 10 && cAccountName[ci] != '\0'; ci++)
		cAccountName[ci] = static_cast<char>(::tolower(static_cast<unsigned char>(cAccountName[ci])));

	memcpy(cAccountPassword, req->password, hb::shared::limits::AccountPassLen - 1);

	ZeroMemory(cTxt, sizeof(cTxt)); // v1.4
	memcpy(cTxt, cAccountPassword, hb::shared::limits::AccountPassLen - 1);
	ZeroMemory(cAccountPassword, sizeof(cAccountPassword));
	memcpy(cAccountPassword, cTxt, hb::shared::limits::AccountPassLen - 1);

	bIsObserverMode = (req->is_observer != 0);

	for(int i = 1; i < MaxClients; i++)
		if ((m_pClientList[i] != 0) && (iClientH != i) && (_strnicmp(m_pClientList[i]->m_cAccountName, cAccountName, hb::shared::limits::AccountNameLen - 1) == 0)) {
			if (memcmp(m_pClientList[i]->m_cAccountPassword, cAccountPassword, 10) == 0) {
				wsprintf(G_cTxt, "<%d> Duplicate account player! Deleted with data save : CharName(%s) AccntName(%s) IP(%s)", i, m_pClientList[i]->m_cCharName, m_pClientList[i]->m_cAccountName, m_pClientList[i]->m_cIPaddress);
				PutLogList(G_cTxt);
				//PutLogFileList(G_cTxt);
				DeleteClient(i, true, true, false);
			}
			else {
				memcpy(m_pClientList[iClientH]->m_cCharName, cCharName, hb::shared::limits::CharNameLen - 1);
				memcpy(m_pClientList[iClientH]->m_cAccountName, cAccountName, hb::shared::limits::AccountNameLen - 1);
				memcpy(m_pClientList[iClientH]->m_cAccountPassword, cAccountPassword, hb::shared::limits::AccountPassLen - 1);

				DeleteClient(iClientH, false, false, false);
				return;
			}
		}

	for(int i = 1; i < MaxClients; i++)
		if ((m_pClientList[i] != 0) && (iClientH != i) && (_strnicmp(m_pClientList[i]->m_cCharName, cCharName, hb::shared::limits::CharNameLen - 1) == 0)) {
			if (memcmp(m_pClientList[i]->m_cAccountPassword, cAccountPassword, 10) == 0) {
				wsprintf(G_cTxt, "<%d> Duplicate player! Deleted with data save : CharName(%s) IP(%s)", i, m_pClientList[i]->m_cCharName, m_pClientList[i]->m_cIPaddress);
				PutLogList(G_cTxt);
				//PutLogFileList(G_cTxt);
				DeleteClient(i, true, true, false);
			}
			else {
				memcpy(m_pClientList[iClientH]->m_cCharName, cCharName, hb::shared::limits::CharNameLen - 1);
				memcpy(m_pClientList[iClientH]->m_cAccountName, cAccountName, hb::shared::limits::AccountNameLen - 1);
				memcpy(m_pClientList[iClientH]->m_cAccountPassword, cAccountPassword, hb::shared::limits::AccountPassLen - 1);

				DeleteClient(iClientH, false, false);
				return;
			}
		}

	memcpy(m_pClientList[iClientH]->m_cCharName, cCharName, hb::shared::limits::CharNameLen - 1);
	memcpy(m_pClientList[iClientH]->m_cAccountName, cAccountName, hb::shared::limits::AccountNameLen - 1);
	memcpy(m_pClientList[iClientH]->m_cAccountPassword, cAccountPassword, hb::shared::limits::AccountPassLen - 1);

	m_pClientList[iClientH]->m_bIsObserverMode = bIsObserverMode;

	// Admin validation
	m_pClientList[iClientH]->m_iAdminIndex = -1;
	m_pClientList[iClientH]->m_iAdminLevel = 0;
	m_pClientList[iClientH]->m_bIsGMMode = false;
	int iAdminIdx = FindAdminByAccount(cAccountName);
	if (iAdminIdx != -1 && _stricmp(m_stAdminList[iAdminIdx].m_cCharName, cCharName) == 0)
	{
		if (strcmp(m_stAdminList[iAdminIdx].m_cApprovedIP, "0.0.0.0") == 0)
		{
			strncpy(m_stAdminList[iAdminIdx].m_cApprovedIP, m_pClientList[iClientH]->m_cIPaddress, 20);
			m_stAdminList[iAdminIdx].m_cApprovedIP[20] = '\0';
			std::snprintf(G_cTxt, sizeof(G_cTxt), "(!) Admin IP auto-set for account %s to %s", cAccountName, m_pClientList[iClientH]->m_cIPaddress);
			PutLogList(G_cTxt);

			sqlite3* configDb = nullptr;
			std::string dbPath;
			if (EnsureGameConfigDatabase(&configDb, dbPath, nullptr))
			{
				SaveAdminConfig(configDb, this);
				CloseGameConfigDatabase(configDb);
			}

			m_pClientList[iClientH]->m_iAdminIndex = iAdminIdx;
			m_pClientList[iClientH]->m_iAdminLevel = m_stAdminList[iAdminIdx].m_iAdminLevel;
		}
		else if (strcmp(m_stAdminList[iAdminIdx].m_cApprovedIP, m_pClientList[iClientH]->m_cIPaddress) == 0)
		{
			m_pClientList[iClientH]->m_iAdminIndex = iAdminIdx;
			m_pClientList[iClientH]->m_iAdminLevel = m_stAdminList[iAdminIdx].m_iAdminLevel;
		}
		else
		{
			std::snprintf(G_cTxt, sizeof(G_cTxt), "(!!!) SECURITY: Admin IP mismatch for account %s (expected %s, got %s)",
				cAccountName, m_stAdminList[iAdminIdx].m_cApprovedIP, m_pClientList[iClientH]->m_cIPaddress);
			PutLogList(G_cTxt);
			DeleteClient(iClientH, false, false, false);
			return;
		}
	}

	InitPlayerData(iClientH, 0, 0); //bSendMsgToLS(ServerMsgId::RequestPlayerData, iClientH);
}

// 05/22/2004 - Hypnotoad - sends client to proper location after dieing
void CGame::RequestInitDataHandler(int iClientH, char* pData, char cKey, size_t dwMsgSize)
{
	char cPlayerName[hb::shared::limits::CharNameLen], cTxt[120];
	int sSummonPoints;
	int iTotalItemA, iTotalItemB, iSize, iRet, iStats;
	SYSTEMTIME SysTime;
	hb::net::PacketWriter writer;
	char initMapData[hb::shared::limits::MsgBufferSize + 1];

	if (m_pClientList[iClientH] == 0) return;

	const auto* req = hb::net::PacketCast<hb::net::PacketRequestInitPlayer>(
		pData, sizeof(hb::net::PacketRequestInitPlayer));
	if (!req) {
		return;
	}

	ZeroMemory(cPlayerName, sizeof(cPlayerName));
	memcpy(cPlayerName, req->player, hb::shared::limits::CharNameLen - 1);

	ZeroMemory(cTxt, sizeof(cTxt)); // v1.4
	memcpy(cTxt, cPlayerName, hb::shared::limits::CharNameLen - 1);
	ZeroMemory(cPlayerName, sizeof(cPlayerName));
	memcpy(cPlayerName, cTxt, hb::shared::limits::CharNameLen - 1);

	if (_strnicmp(m_pClientList[iClientH]->m_cCharName, cPlayerName, hb::shared::limits::CharNameLen - 1) != 0) {
		DeleteClient(iClientH, false, true);
		return;
	}

	// Send configs FIRST so the client has item/magic/skill definitions
	// before receiving player data that references them.
	uint32_t clientItemHash = 0, clientMagicHash = 0, clientSkillHash = 0, clientNpcHash = 0;
	if (dwMsgSize >= sizeof(hb::net::PacketRequestInitDataEx)) {
		const auto* exReq = reinterpret_cast<const hb::net::PacketRequestInitDataEx*>(pData);
		clientItemHash = exReq->itemConfigHash;
		clientMagicHash = exReq->magicConfigHash;
		clientSkillHash = exReq->skillConfigHash;
		clientNpcHash = exReq->npcConfigHash;
	}

	bool bItemCacheValid  = (clientItemHash != 0 && clientItemHash == m_dwConfigHash[0]);
	bool bMagicCacheValid = (clientMagicHash != 0 && clientMagicHash == m_dwConfigHash[1]);
	bool bSkillCacheValid = (clientSkillHash != 0 && clientSkillHash == m_dwConfigHash[2]);
	bool bNpcCacheValid   = (clientNpcHash != 0 && clientNpcHash == m_dwConfigHash[3]);

	{
		hb::net::PacketResponseConfigCacheStatus cacheStatus{};
		cacheStatus.header.msg_id = MSGID_RESPONSE_CONFIGCACHESTATUS;
		cacheStatus.header.msg_type = MsgType::Confirm;
		cacheStatus.itemCacheValid = bItemCacheValid ? 1 : 0;
		cacheStatus.magicCacheValid = bMagicCacheValid ? 1 : 0;
		cacheStatus.skillCacheValid = bSkillCacheValid ? 1 : 0;
		cacheStatus.npcCacheValid = bNpcCacheValid ? 1 : 0;
		m_pClientList[iClientH]->m_pXSock->iSendMsg(
			reinterpret_cast<char*>(&cacheStatus), sizeof(cacheStatus));
	}

	if (!bItemCacheValid)  m_pItemManager->bSendClientItemConfigs(iClientH);
	if (!bMagicCacheValid) m_pMagicManager->bSendClientMagicConfigs(iClientH);
	if (!bSkillCacheValid) m_pSkillManager->bSendClientSkillConfigs(iClientH);
	if (!bNpcCacheValid)   bSendClientNpcConfigs(iClientH);

	// Now send player data (configs are guaranteed loaded on client)
	writer.Reset();
	auto* char_pkt = writer.Append<hb::net::PacketResponsePlayerCharacterContents>();
	char_pkt->header.msg_id = MsgId::PlayerCharacterContents;
	char_pkt->header.msg_type = MsgType::Confirm;
	char_pkt->hp = m_pClientList[iClientH]->m_iHP;
	char_pkt->mp = m_pClientList[iClientH]->m_iMP;
	char_pkt->sp = m_pClientList[iClientH]->m_iSP;
	char_pkt->ac = m_pClientList[iClientH]->m_iDefenseRatio;
	char_pkt->thac0 = m_pClientList[iClientH]->m_iHitRatio;
	char_pkt->level = m_pClientList[iClientH]->m_iLevel;
	char_pkt->str = m_pClientList[iClientH]->m_iStr;
	char_pkt->intel = m_pClientList[iClientH]->m_iInt;
	char_pkt->vit = m_pClientList[iClientH]->m_iVit;
	char_pkt->dex = m_pClientList[iClientH]->m_iDex;
	char_pkt->mag = m_pClientList[iClientH]->m_iMag;
	char_pkt->chr = m_pClientList[iClientH]->m_iCharisma;

	iStats = (m_pClientList[iClientH]->m_iStr + m_pClientList[iClientH]->m_iDex + m_pClientList[iClientH]->m_iVit +
		m_pClientList[iClientH]->m_iInt + m_pClientList[iClientH]->m_iMag + m_pClientList[iClientH]->m_iCharisma);

	m_pClientList[iClientH]->m_iLU_Pool = (m_pClientList[iClientH]->m_iLevel - 1) * 3 - (iStats - 70);
	char_pkt->lu_point = static_cast<std::uint16_t>(m_pClientList[iClientH]->m_iLU_Pool);
	char_pkt->lu_unused[0] = static_cast<std::uint8_t>(m_pClientList[iClientH]->m_cVar);
	char_pkt->lu_unused[1] = 0;
	char_pkt->lu_unused[2] = 0;
	char_pkt->lu_unused[3] = 0;
	char_pkt->lu_unused[4] = 0;
	char_pkt->exp = m_pClientList[iClientH]->m_iExp;
	char_pkt->enemy_kills = m_pClientList[iClientH]->m_iEnemyKillCount;
	char_pkt->pk_count = m_pClientList[iClientH]->m_iPKCount;
	char_pkt->reward_gold = m_pClientList[iClientH]->m_iRewardGold;
	std::memcpy(char_pkt->location, m_pClientList[iClientH]->m_cLocation, sizeof(char_pkt->location));
	std::memcpy(char_pkt->guild_name, m_pClientList[iClientH]->m_cGuildName, sizeof(char_pkt->guild_name));
	char_pkt->guild_rank = m_pClientList[iClientH]->m_iGuildRank;
	char_pkt->super_attack_left = static_cast<std::uint8_t>(m_pClientList[iClientH]->m_iSuperAttackLeft);
	char_pkt->fightzone_number = m_pClientList[iClientH]->m_iFightzoneNumber;
	char_pkt->max_stats = m_iMaxStatValue;
	char_pkt->max_level = m_iMaxLevel;
	char_pkt->max_bank_items = m_iMaxBankItems;

	//hbest
	m_pClientList[iClientH]->isForceSet = false;
	m_pClientList[iClientH]->m_iPartyID = 0;
	m_pClientList[iClientH]->m_iPartyStatus = PartyStatus::Null;
	m_pClientList[iClientH]->m_iReqJoinPartyClientH = 0;

	iRet = m_pClientList[iClientH]->m_pXSock->iSendMsg(writer.Data(), static_cast<int>(writer.Size()));
	switch (iRet) {
	case sock::Event::QueueFull:
	case sock::Event::SocketError:
	case sock::Event::CriticalError:
	case sock::Event::SocketClosed:
		DeleteClient(iClientH, true, true);
		return;
	}

	writer.Reset();
	auto* item_header = writer.Append<hb::net::PacketResponseItemListHeader>();
	item_header->header.msg_id = MsgId::PlayerItemListContents;
	item_header->header.msg_type = MsgType::Confirm;

	iTotalItemA = 0;
	for(int i = 0; i < hb::shared::limits::MaxItems; i++)
		if (m_pClientList[iClientH]->m_pItemList[i] != 0)
			iTotalItemA++;

	item_header->item_count = static_cast<std::uint8_t>(iTotalItemA);

	for(int i = 0; i < iTotalItemA; i++) {
		// ### ERROR POINT!!!
		if (m_pClientList[iClientH]->m_pItemList[i] == 0) {
			wsprintf(G_cTxt, "RequestInitDataHandler error: Client(%s) Item(%d)", m_pClientList[iClientH]->m_cCharName, i);
			PutLogFileList(G_cTxt);

			DeleteClient(iClientH, false, true);
			return;
		}
		auto* entry = writer.Append<hb::net::PacketResponseItemListEntry>();
		std::memcpy(entry->name, m_pClientList[iClientH]->m_pItemList[i]->m_cName, sizeof(entry->name));
		entry->count = m_pClientList[iClientH]->m_pItemList[i]->m_dwCount;
		entry->item_type = m_pClientList[iClientH]->m_pItemList[i]->m_cItemType;
		entry->equip_pos = m_pClientList[iClientH]->m_pItemList[i]->m_cEquipPos;
		entry->is_equipped = static_cast<std::uint8_t>(m_pClientList[iClientH]->m_bIsItemEquipped[i]);
		entry->level_limit = m_pClientList[iClientH]->m_pItemList[i]->m_sLevelLimit;
		entry->gender_limit = m_pClientList[iClientH]->m_pItemList[i]->m_cGenderLimit;
		entry->cur_lifespan = m_pClientList[iClientH]->m_pItemList[i]->m_wCurLifeSpan;
		entry->weight = m_pClientList[iClientH]->m_pItemList[i]->m_wWeight;
		entry->sprite = m_pClientList[iClientH]->m_pItemList[i]->m_sSprite;
		entry->sprite_frame = m_pClientList[iClientH]->m_pItemList[i]->m_sSpriteFrame;
		entry->item_color = m_pClientList[iClientH]->m_pItemList[i]->m_cItemColor;
		entry->spec_value2 = static_cast<std::uint8_t>(m_pClientList[iClientH]->m_pItemList[i]->m_sItemSpecEffectValue2);
		entry->attribute = m_pClientList[iClientH]->m_pItemList[i]->m_dwAttribute;
		entry->item_id = m_pClientList[iClientH]->m_pItemList[i]->m_sIDnum;
		entry->max_lifespan = m_pClientList[iClientH]->m_pItemList[i]->m_wMaxLifeSpan;
	}

	iTotalItemB = 0;
	for(int i = 0; i < hb::shared::limits::MaxBankItems; i++)
		if (m_pClientList[iClientH]->m_pItemInBankList[i] != 0)
			iTotalItemB++;

	auto* bank_header = writer.Append<hb::net::PacketResponseBankItemListHeader>();
	bank_header->bank_item_count = static_cast<std::uint16_t>(iTotalItemB);

	for(int i = 0; i < iTotalItemB; i++) {
		if (m_pClientList[iClientH]->m_pItemInBankList[i] == 0) {
			wsprintf(G_cTxt, "RequestInitDataHandler error: Client(%s) Bank-Item(%d)", m_pClientList[iClientH]->m_cCharName, i);
			PutLogFileList(G_cTxt);

			DeleteClient(iClientH, false, true);
			return;
		}
		auto* entry = writer.Append<hb::net::PacketResponseBankItemEntry>();
		std::memcpy(entry->name, m_pClientList[iClientH]->m_pItemInBankList[i]->m_cName, sizeof(entry->name));
		entry->count = m_pClientList[iClientH]->m_pItemInBankList[i]->m_dwCount;
		entry->item_type = m_pClientList[iClientH]->m_pItemInBankList[i]->m_cItemType;
		entry->equip_pos = m_pClientList[iClientH]->m_pItemInBankList[i]->m_cEquipPos;
		entry->level_limit = m_pClientList[iClientH]->m_pItemInBankList[i]->m_sLevelLimit;
		entry->gender_limit = m_pClientList[iClientH]->m_pItemInBankList[i]->m_cGenderLimit;
		entry->cur_lifespan = m_pClientList[iClientH]->m_pItemInBankList[i]->m_wCurLifeSpan;
		entry->weight = m_pClientList[iClientH]->m_pItemInBankList[i]->m_wWeight;
		entry->sprite = m_pClientList[iClientH]->m_pItemInBankList[i]->m_sSprite;
		entry->sprite_frame = m_pClientList[iClientH]->m_pItemInBankList[i]->m_sSpriteFrame;
		entry->item_color = m_pClientList[iClientH]->m_pItemInBankList[i]->m_cItemColor;
		entry->spec_value2 = static_cast<std::uint8_t>(m_pClientList[iClientH]->m_pItemInBankList[i]->m_sItemSpecEffectValue2);
		entry->attribute = m_pClientList[iClientH]->m_pItemInBankList[i]->m_dwAttribute;
		entry->item_id = m_pClientList[iClientH]->m_pItemInBankList[i]->m_sIDnum;
		entry->max_lifespan = m_pClientList[iClientH]->m_pItemInBankList[i]->m_wMaxLifeSpan;
	}

	auto* mastery = writer.Append<hb::net::PacketResponseMasteryData>();
	std::memcpy(mastery->magic_mastery, m_pClientList[iClientH]->m_cMagicMastery, hb::shared::limits::MaxMagicType);
	std::memcpy(mastery->skill_mastery, m_pClientList[iClientH]->m_cSkillMastery, hb::shared::limits::MaxSkillType);

	iRet = m_pClientList[iClientH]->m_pXSock->iSendMsg(writer.Data(), static_cast<int>(writer.Size()));
	switch (iRet) {
	case sock::Event::QueueFull:
	case sock::Event::SocketError:
	case sock::Event::CriticalError:
	case sock::Event::SocketClosed:
		DeleteClient(iClientH, true, true);
		return;
	}

	// Send item positions right after item list so positions are applied
	// while items are still freshly initialized
	SendNotifyMsg(0, iClientH, Notify::ItemPosList, 0, 0, 0, 0);

	writer.Reset();
	auto* init_header = writer.Append<hb::net::PacketResponseInitDataHeader>();
	init_header->header.msg_id = MsgId::ResponseInitData;
	init_header->header.msg_type = MsgType::Confirm;

	if (m_pClientList[iClientH]->m_bIsObserverMode == false)
		bGetEmptyPosition(&m_pClientList[iClientH]->m_sX, &m_pClientList[iClientH]->m_sY, m_pClientList[iClientH]->m_cMapIndex);
	else GetMapInitialPoint(m_pClientList[iClientH]->m_cMapIndex, &m_pClientList[iClientH]->m_sX, &m_pClientList[iClientH]->m_sY);

	init_header->player_object_id = static_cast<std::int16_t>(iClientH);
	init_header->pivot_x = static_cast<std::int16_t>(m_pClientList[iClientH]->m_sX - hb::shared::view::PlayerPivotOffsetX);
	init_header->pivot_y = static_cast<std::int16_t>(m_pClientList[iClientH]->m_sY - hb::shared::view::PlayerPivotOffsetY);
	init_header->player_type = m_pClientList[iClientH]->m_sType;
	init_header->appearance = m_pClientList[iClientH]->m_appearance;
	init_header->status = m_pClientList[iClientH]->m_status;
	std::memcpy(init_header->map_name, m_pClientList[iClientH]->m_cMapName, sizeof(init_header->map_name));
	std::memcpy(init_header->cur_location, m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cLocationName, sizeof(init_header->cur_location));

	if (m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_bIsFixedDayMode)
		init_header->sprite_alpha = 1;
	else init_header->sprite_alpha = m_cDayOrNight;

	if (m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_bIsFixedDayMode)
		init_header->weather_status = 0;
	else init_header->weather_status = m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cWhetherStatus;

	init_header->contribution = m_pClientList[iClientH]->m_iContribution;

	if (m_pClientList[iClientH]->m_bIsObserverMode == false) {
		m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->SetOwner(iClientH,
			hb::shared::owner_class::Player,
			m_pClientList[iClientH]->m_sX,
			m_pClientList[iClientH]->m_sY);
	}

	init_header->observer_mode = static_cast<std::uint8_t>(m_pClientList[iClientH]->m_bIsObserverMode);
	init_header->rating = m_pClientList[iClientH]->m_iRating;
	init_header->hp = m_pClientList[iClientH]->m_iHP;
	init_header->discount = 0;

	iSize = iComposeInitMapData(m_pClientList[iClientH]->m_sX - hb::shared::view::CenterX, m_pClientList[iClientH]->m_sY - hb::shared::view::CenterY, iClientH, initMapData);
	writer.AppendBytes(initMapData, static_cast<std::size_t>(iSize));

	iRet = m_pClientList[iClientH]->m_pXSock->iSendMsg(writer.Data(), static_cast<int>(writer.Size()));
	switch (iRet) {
	case sock::Event::QueueFull:
	case sock::Event::SocketError:
	case sock::Event::CriticalError:
	case sock::Event::SocketClosed:
		DeleteClient(iClientH, true, true);
		return;
	}

	SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventLog, MsgType::Confirm, 0, 0, 0);

	// v2.13 
	if ((memcmp(m_pClientList[iClientH]->m_cLocation, "are", 3) == 0) &&
		(memcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cLocationName, "elvine", 6) == 0)
		) {

		m_pClientList[iClientH]->m_dwWarBeginTime = timeGetTime();
		m_pClientList[iClientH]->m_bIsWarLocation = true;
		// v2.17 2002-7-15
		SetForceRecallTime(iClientH);
	}
	// v2.13 
	else if ((memcmp(m_pClientList[iClientH]->m_cLocation, "elv", 3) == 0) &&
		(memcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cLocationName, "aresden", 7) == 0)
		) {

		m_pClientList[iClientH]->m_dwWarBeginTime = timeGetTime();
		m_pClientList[iClientH]->m_bIsWarLocation = true;

		// v2.17 2002-7-15
		SetForceRecallTime(iClientH);
	}
	else if (((memcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cLocationName, "arejail", 7) == 0) ||
		(memcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cLocationName, "elvjail", 7) == 0))
		) {
		m_pClientList[iClientH]->m_bIsWarLocation = true;
		m_pClientList[iClientH]->m_dwWarBeginTime = timeGetTime();

		// v2.17 2002-7-15 
		if (m_pClientList[iClientH]->m_iTimeLeft_ForceRecall == 0) {
			m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 20 * 5;
		}
		else if (m_pClientList[iClientH]->m_iTimeLeft_ForceRecall > 20 * 5) {
			m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 20 * 5;
		}

	}
	else if ((m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_bIsFightZone) &&
		(m_iFightzoneNoForceRecall == 0) ) {

		m_pClientList[iClientH]->m_dwWarBeginTime = timeGetTime();
		m_pClientList[iClientH]->m_bIsWarLocation = true;

		GetLocalTime(&SysTime);
		m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 2 * 60 * 20 - ((SysTime.wHour % 2) * 20 * 60 + SysTime.wMinute * 20) - 2 * 20;
	}
	else
	{
		m_pClientList[iClientH]->m_bIsWarLocation = false;
		// v1.42
		m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 0;
		// 06/11/2004
		SetForceRecallTime(iClientH);
	}

	// v2.17 2002-7-15
	//hbest...
	if ((m_pClientList[iClientH]->m_iTimeLeft_ForceRecall > 0) && (m_pClientList[iClientH]->m_bIsWarLocation) && IsEnemyZone(iClientH)) {
		SendNotifyMsg(0, iClientH, Notify::ForceRecallTime, m_pClientList[iClientH]->m_iTimeLeft_ForceRecall, 0, 0, 0);
		//wsprintf(G_cTxt,"(!) Game Server Force Recall Time  %d (%d)min", m_pClientList[iClientH]->m_iTimeLeft_ForceRecall, m_pClientList[iClientH]->m_iTimeLeft_ForceRecall/20) ;
		//PutLogList(G_cTxt) ;
	}

	if (m_pClientList[iClientH]->m_iGizonItemUpgradeLeft < 0) {
		m_pClientList[iClientH]->m_iGizonItemUpgradeLeft = 0;
	}

	// No entering enemy shops
	int iMapside, iMapside2;

	iMapside = iGetMapLocationSide(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cName);
	if (iMapside > 3) iMapside2 = iMapside - 2;
	else iMapside2 = iMapside;
	m_pClientList[iClientH]->m_bIsInsideOwnTown = false;
	if ((m_pClientList[iClientH]->m_cSide != iMapside2) && (iMapside != 0)) {
		if ((iMapside <= 2)) {
			if (m_pClientList[iClientH]->m_cSide != 0) {
				m_pClientList[iClientH]->m_dwWarBeginTime = timeGetTime();
				m_pClientList[iClientH]->m_bIsWarLocation = true;
				m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 1;
				m_pClientList[iClientH]->m_bIsInsideOwnTown = true;
			}
		}
	}
	else {
		if (m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_bIsFightZone &&
			m_iFightzoneNoForceRecall == false) {
			m_pClientList[iClientH]->m_dwWarBeginTime = timeGetTime();
			m_pClientList[iClientH]->m_bIsWarLocation = true;
			GetLocalTime(&SysTime);
			m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 2 * 60 * 20 - ((SysTime.wHour % 2) * 20 * 60 + SysTime.wMinute * 20) - 2 * 20;
		}
		else {
			if (memcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cLocationName, "arejail", 7) == 0 ||
				memcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cLocationName, "elvjail", 7) == 0) {
				m_pClientList[iClientH]->m_bIsWarLocation = true;
				m_pClientList[iClientH]->m_dwWarBeginTime = timeGetTime();
				if (m_pClientList[iClientH]->m_iTimeLeft_ForceRecall == 0)
					m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 100;
				else if (m_pClientList[iClientH]->m_iTimeLeft_ForceRecall > 100)
					m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 100;
			}
		}
	}

	/*if ((m_pClientList[iClientH]->m_iTimeLeft_ForceRecall > 0) &&
		(m_pClientList[iClientH]->m_bIsWarLocation )) {
		SendNotifyMsg(0, iClientH, Notify::ForceRecallTime, m_pClientList[iClientH]->m_iTimeLeft_ForceRecall, 0, 0, 0);
	}*/

	SendNotifyMsg(0, iClientH, Notify::SafeAttackMode, 0, 0, 0, 0);
	SendNotifyMsg(0, iClientH, Notify::DownSkillIndexSet, m_pClientList[iClientH]->m_iDownSkillIndex, 0, 0, 0);

	m_pQuestManager->_SendQuestContents(iClientH);
	m_pQuestManager->_CheckQuestEnvironment(iClientH);

	// v1.432
	if (m_pClientList[iClientH]->m_iSpecialAbilityTime == 0) {
		SendNotifyMsg(0, iClientH, Notify::SpecialAbilityEnabled, 0, 0, 0, 0);
	}

	// Crusade 
	if (m_bIsCrusadeMode) {
		if (m_pClientList[iClientH]->m_dwCrusadeGUID == 0) {
			m_pClientList[iClientH]->m_iCrusadeDuty = 0;
			m_pClientList[iClientH]->m_iConstructionPoint = 0;
			m_pClientList[iClientH]->m_dwCrusadeGUID = m_dwCrusadeGUID;
		}
		else if (m_pClientList[iClientH]->m_dwCrusadeGUID != m_dwCrusadeGUID) {
			m_pClientList[iClientH]->m_iCrusadeDuty = 0;
			m_pClientList[iClientH]->m_iConstructionPoint = 0;
			m_pClientList[iClientH]->m_iWarContribution = 0;
			m_pClientList[iClientH]->m_dwCrusadeGUID = m_dwCrusadeGUID;
			SendNotifyMsg(0, iClientH, Notify::Crusade, (DWORD)m_bIsCrusadeMode, 0, 0, 0, -1);
		}
		m_pClientList[iClientH]->m_cVar = 1;
		SendNotifyMsg(0, iClientH, Notify::Crusade, (DWORD)m_bIsCrusadeMode, m_pClientList[iClientH]->m_iCrusadeDuty, 0, 0);
	}
	else if (m_bIsHeldenianMode) {
		sSummonPoints = m_pClientList[iClientH]->m_iCharisma * 300;
		if (sSummonPoints > m_iMaxSummonPoints) sSummonPoints = m_iMaxSummonPoints;
		if (m_pClientList[iClientH]->m_dwHeldenianGUID == 0) {
			m_pClientList[iClientH]->m_dwHeldenianGUID = m_dwHeldenianGUID;
			m_pClientList[iClientH]->m_iConstructionPoint = sSummonPoints;
		}
		else if (m_pClientList[iClientH]->m_dwHeldenianGUID != m_dwHeldenianGUID) {
			m_pClientList[iClientH]->m_iConstructionPoint = sSummonPoints;
			m_pClientList[iClientH]->m_iWarContribution = 0;
			m_pClientList[iClientH]->m_dwHeldenianGUID = m_dwHeldenianGUID;
		}
		m_pClientList[iClientH]->m_cVar = 2;
		if (m_bIsHeldenianMode) {
			SendNotifyMsg(0, iClientH, Notify::Crusade, 0, 0, 0, 0);
			if (m_bHeldenianInitiated == false) {
				SendNotifyMsg(0, iClientH, Notify::HeldenianStart, 0, 0, 0, 0);
			}
			SendNotifyMsg(0, iClientH, Notify::ConstructionPoint, m_pClientList[iClientH]->m_iConstructionPoint, m_pClientList[iClientH]->m_iWarContribution, 0, 0);
			m_pWarManager->UpdateHeldenianStatus();
		}
	}
	else if ((m_pClientList[iClientH]->m_cVar == 1) && (m_pClientList[iClientH]->m_dwCrusadeGUID == m_dwCrusadeGUID)) {
		m_pClientList[iClientH]->m_iCrusadeDuty = 0;
		m_pClientList[iClientH]->m_iConstructionPoint = 0;
	}
	else {
		if (m_pClientList[iClientH]->m_dwCrusadeGUID == m_dwCrusadeGUID) {
			if (m_pClientList[iClientH]->m_cVar == 1) {
				SendNotifyMsg(0, iClientH, Notify::Crusade, (DWORD)m_bIsCrusadeMode, 0, 0, 0, -1);
			}
		}
		else {
			SendNotifyMsg(0, iClientH, Notify::Crusade, (DWORD)m_bIsCrusadeMode, 0, 0, 0, -1);
			m_pClientList[iClientH]->m_dwCrusadeGUID = 0;
			m_pClientList[iClientH]->m_iWarContribution = 0;
			m_pClientList[iClientH]->m_dwCrusadeGUID = 0;
		}
	}

	// v1.42
	if (memcmp(m_pClientList[iClientH]->m_cMapName, "fightzone", 9) == 0) {
		wsprintf(G_cTxt, "Char(%s)-Enter(%s) Observer(%d)", m_pClientList[iClientH]->m_cCharName, m_pClientList[iClientH]->m_cMapName, m_pClientList[iClientH]->m_bIsObserverMode);
		PutLogEventFileList(G_cTxt);
	}

	if (m_bIsHeldenianMode) SendNotifyMsg(0, iClientH, Notify::HeldenianTeleport, 0, 0, 0, 0, 0);
	if (m_bHeldenianInitiated) SendNotifyMsg(0, iClientH, Notify::HeldenianStart, 0, 0, 0, 0, 0);

	// Crusade
	SendNotifyMsg(0, iClientH, Notify::ConstructionPoint, m_pClientList[iClientH]->m_iConstructionPoint, m_pClientList[iClientH]->m_iWarContribution, 1, 0);
	//Fix Sprite Bug
	//			SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventMotion, Type::NullAction, 0, 0, 0);
	//Gizon point lefT???
	SendNotifyMsg(0, iClientH, Notify::GizonItemUpgradeLeft, m_pClientList[iClientH]->m_iGizonItemUpgradeLeft, 0, 0, 0);

	if ((m_bIsApocalypseMode) && (m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_bIsApocalypseMap)) {
		RequestTeleportHandler(iClientH, "1   ");
	}

	if (m_bIsApocalypseMode) {
		SendNotifyMsg(0, iClientH, Notify::ApocGateStartMsg, 0, 0, 0, 0, 0);
	}

	SendNotifyMsg(0, iClientH, Notify::Hunger, m_pClientList[iClientH]->m_iHungerStatus, 0, 0, 0);
	SendNotifyMsg(0, iClientH, Notify::SuperAttackLeft, 0, 0, 0, 0);

	RequestNoticementHandler(iClientH); // send noticement when log in
}




bool CGame::bSendClientNpcConfigs(int iClientH)
{
	if (m_pClientList[iClientH] == 0) {
		return false;
	}

	constexpr size_t maxPacketSize = 7000;
	constexpr size_t headerSize = sizeof(hb::net::PacketNpcConfigHeader);
	constexpr size_t entrySize = sizeof(hb::net::PacketNpcConfigEntry);
	constexpr size_t maxEntriesPerPacket = (maxPacketSize - headerSize) / entrySize;

	// Count total NPCs
	int totalNpcs = 0;
	for(int i = 0; i < MaxNpcTypes; i++) {
		if (m_pNpcConfigList[i] != 0) {
			totalNpcs++;
		}
	}

	// Send NPCs in packets
	int npcsSent = 0;
	int packetIndex = 0;

	while (npcsSent < totalNpcs) {
		std::memset(G_cData50000, 0, sizeof(G_cData50000));

		auto* pktHeader = reinterpret_cast<hb::net::PacketNpcConfigHeader*>(G_cData50000);
		pktHeader->header.msg_id = MsgId::NpcConfigContents;
		pktHeader->header.msg_type = MsgType::Confirm;
		pktHeader->totalNpcs = static_cast<uint16_t>(totalNpcs);
		pktHeader->packetIndex = static_cast<uint16_t>(packetIndex);

		auto* entries = reinterpret_cast<hb::net::PacketNpcConfigEntry*>(G_cData50000 + headerSize);

		uint16_t entriesInPacket = 0;
		int skipped = 0;

		for(int i = 0; i < MaxNpcTypes && entriesInPacket < maxEntriesPerPacket; i++) {
			if (m_pNpcConfigList[i] == 0) {
				continue;
			}

			if (skipped < npcsSent) {
				skipped++;
				continue;
			}

			const CNpc* npc = m_pNpcConfigList[i];
			auto& entry = entries[entriesInPacket];

			entry.npcId = static_cast<int16_t>(i);
			entry.npcType = npc->m_sType;
			std::memset(entry.name, 0, sizeof(entry.name));
			std::snprintf(entry.name, sizeof(entry.name), "%s", npc->m_cNpcName);

			entriesInPacket++;
		}

		pktHeader->npcCount = entriesInPacket;
		size_t packetSize = headerSize + (entriesInPacket * entrySize);

		int iRet = m_pClientList[iClientH]->m_pXSock->iSendMsg(G_cData50000, static_cast<int>(packetSize));
		switch (iRet) {
		case sock::Event::QueueFull:
		case sock::Event::SocketError:
		case sock::Event::CriticalError:
		case sock::Event::SocketClosed:
			std::snprintf(G_cTxt, sizeof(G_cTxt),
				"Failed to send NPC configs: Client(%d) Packet(%d)",
				iClientH, packetIndex);
			PutLogList(G_cTxt);
			DeleteClient(iClientH, true, true);
			delete m_pClientList[iClientH];
			m_pClientList[iClientH] = 0;
			return false;
		}

		npcsSent += entriesInPacket;
		packetIndex++;
	}

	return true;
}

void CGame::ComputeConfigHashes()
{
	// Compute CRC32 for item configs
	{
		constexpr size_t headerSize = sizeof(hb::net::PacketItemConfigHeader);
		constexpr size_t entrySize = sizeof(hb::net::PacketItemConfigEntry);
		constexpr size_t maxEntriesPerPacket = (7000 - headerSize) / entrySize;

		std::vector<uint8_t> allData;
		int totalItems = 0;
		for(int i = 0; i < MaxItemTypes; i++) {
			if (m_pItemConfigList[i] != 0) totalItems++;
		}

		int itemsSent = 0;
		int packetIndex = 0;
		while (itemsSent < totalItems) {
			char buf[7000]{};
			auto* pktHeader = reinterpret_cast<hb::net::PacketItemConfigHeader*>(buf);
			pktHeader->header.msg_id = MsgId::ItemConfigContents;
			pktHeader->header.msg_type = MsgType::Confirm;
			pktHeader->totalItems = static_cast<uint16_t>(totalItems);
			pktHeader->packetIndex = static_cast<uint16_t>(packetIndex);

			auto* entries = reinterpret_cast<hb::net::PacketItemConfigEntry*>(buf + headerSize);
			uint16_t entriesInPacket = 0;
			int skipped = 0;

			for(int i = 0; i < MaxItemTypes && entriesInPacket < maxEntriesPerPacket; i++) {
				if (m_pItemConfigList[i] == 0) continue;
				if (skipped < itemsSent) { skipped++; continue; }

				const CItem* item = m_pItemConfigList[i];
				auto& entry = entries[entriesInPacket];
				entry.itemId = item->m_sIDnum;
				std::memset(entry.name, 0, sizeof(entry.name));
				std::snprintf(entry.name, sizeof(entry.name), "%s", item->m_cName);
				entry.itemType = item->m_cItemType;
				entry.equipPos = item->m_cEquipPos;
				entry.effectType = item->m_sItemEffectType;
				entry.effectValue1 = item->m_sItemEffectValue1;
				entry.effectValue2 = item->m_sItemEffectValue2;
				entry.effectValue3 = item->m_sItemEffectValue3;
				entry.effectValue4 = item->m_sItemEffectValue4;
				entry.effectValue5 = item->m_sItemEffectValue5;
				entry.effectValue6 = item->m_sItemEffectValue6;
				entry.maxLifeSpan = item->m_wMaxLifeSpan;
				entry.specialEffect = item->m_sSpecialEffect;
				entry.sprite = item->m_sSprite;
				entry.spriteFrame = item->m_sSpriteFrame;
				entry.price = item->m_bIsForSale ? static_cast<int32_t>(item->m_wPrice) : -static_cast<int32_t>(item->m_wPrice);
				entry.weight = item->m_wWeight;
				entry.apprValue = item->m_cApprValue;
				entry.speed = item->m_cSpeed;
				entry.levelLimit = item->m_sLevelLimit;
				entry.genderLimit = item->m_cGenderLimit;
				entry.specialEffectValue1 = item->m_sSpecialEffectValue1;
				entry.specialEffectValue2 = item->m_sSpecialEffectValue2;
				entry.relatedSkill = item->m_sRelatedSkill;
				entry.category = item->m_cCategory;
				entry.itemColor = item->m_cItemColor;
				entriesInPacket++;
			}

			pktHeader->itemCount = entriesInPacket;
			size_t packetSize = headerSize + (entriesInPacket * entrySize);

			uint16_t len = static_cast<uint16_t>(packetSize);
			const uint8_t* lenBytes = reinterpret_cast<const uint8_t*>(&len);
			allData.push_back(lenBytes[0]);
			allData.push_back(lenBytes[1]);
			allData.insert(allData.end(), reinterpret_cast<uint8_t*>(buf), reinterpret_cast<uint8_t*>(buf) + packetSize);

			itemsSent += entriesInPacket;
			packetIndex++;
		}
		m_dwConfigHash[0] = allData.empty() ? 0 : hb::shared::util::hb_crc32(allData.data(), allData.size());
	}

	// Compute CRC32 for magic configs
	{
		constexpr size_t headerSize = sizeof(hb::net::PacketMagicConfigHeader);
		constexpr size_t entrySize = sizeof(hb::net::PacketMagicConfigEntry);
		constexpr size_t maxEntriesPerPacket = (7000 - headerSize) / entrySize;

		std::vector<uint8_t> allData;
		int totalMagics = 0;
		for(int i = 0; i < hb::shared::limits::MaxMagicType; i++) {
			if (m_pMagicConfigList[i] != 0) totalMagics++;
		}

		int magicsSent = 0;
		int packetIndex = 0;
		while (magicsSent < totalMagics) {
			char buf[7000]{};
			auto* pktHeader = reinterpret_cast<hb::net::PacketMagicConfigHeader*>(buf);
			pktHeader->header.msg_id = MsgId::MagicConfigContents;
			pktHeader->header.msg_type = MsgType::Confirm;
			pktHeader->totalMagics = static_cast<uint16_t>(totalMagics);
			pktHeader->packetIndex = static_cast<uint16_t>(packetIndex);

			auto* entries = reinterpret_cast<hb::net::PacketMagicConfigEntry*>(buf + headerSize);
			uint16_t entriesInPacket = 0;
			int skipped = 0;

			for(int i = 0; i < hb::shared::limits::MaxMagicType && entriesInPacket < maxEntriesPerPacket; i++) {
				if (m_pMagicConfigList[i] == 0) continue;
				if (skipped < magicsSent) { skipped++; continue; }

				const CMagic* magic = m_pMagicConfigList[i];
				auto& entry = entries[entriesInPacket];
				entry.magicId = static_cast<int16_t>(i);
				std::memset(entry.name, 0, sizeof(entry.name));
				std::snprintf(entry.name, sizeof(entry.name), "%s", magic->m_cName);
				entry.manaCost = magic->m_sValue1;
				entry.intLimit = magic->m_sIntLimit;
				entry.goldCost = magic->m_iGoldCost;
				entry.isVisible = (magic->m_iGoldCost >= 0) ? 1 : 0;
				entry.magicType = magic->m_sType;
				entry.aoeRadiusX = magic->m_sValue2;
				entry.aoeRadiusY = magic->m_sValue3;
				entry.dynamicPattern = magic->m_sValue11;
				entry.dynamicRadius = magic->m_sValue12;
				entriesInPacket++;
			}

			pktHeader->magicCount = entriesInPacket;
			size_t packetSize = headerSize + (entriesInPacket * entrySize);

			uint16_t len = static_cast<uint16_t>(packetSize);
			const uint8_t* lenBytes = reinterpret_cast<const uint8_t*>(&len);
			allData.push_back(lenBytes[0]);
			allData.push_back(lenBytes[1]);
			allData.insert(allData.end(), reinterpret_cast<uint8_t*>(buf), reinterpret_cast<uint8_t*>(buf) + packetSize);

			magicsSent += entriesInPacket;
			packetIndex++;
		}
		m_dwConfigHash[1] = allData.empty() ? 0 : hb::shared::util::hb_crc32(allData.data(), allData.size());
	}

	// Compute CRC32 for skill configs
	{
		constexpr size_t headerSize = sizeof(hb::net::PacketSkillConfigHeader);
		constexpr size_t entrySize = sizeof(hb::net::PacketSkillConfigEntry);
		constexpr size_t maxEntriesPerPacket = (7000 - headerSize) / entrySize;

		std::vector<uint8_t> allData;
		int totalSkills = 0;
		for(int i = 0; i < hb::shared::limits::MaxSkillType; i++) {
			if (m_pSkillConfigList[i] != 0) totalSkills++;
		}

		int skillsSent = 0;
		int packetIndex = 0;
		while (skillsSent < totalSkills) {
			char buf[7000]{};
			auto* pktHeader = reinterpret_cast<hb::net::PacketSkillConfigHeader*>(buf);
			pktHeader->header.msg_id = MsgId::SkillConfigContents;
			pktHeader->header.msg_type = MsgType::Confirm;
			pktHeader->totalSkills = static_cast<uint16_t>(totalSkills);
			pktHeader->packetIndex = static_cast<uint16_t>(packetIndex);

			auto* entries = reinterpret_cast<hb::net::PacketSkillConfigEntry*>(buf + headerSize);
			uint16_t entriesInPacket = 0;
			int skipped = 0;

			for(int i = 0; i < hb::shared::limits::MaxSkillType && entriesInPacket < maxEntriesPerPacket; i++) {
				if (m_pSkillConfigList[i] == 0) continue;
				if (skipped < skillsSent) { skipped++; continue; }

				const CSkill* skill = m_pSkillConfigList[i];
				auto& entry = entries[entriesInPacket];
				entry.skillId = static_cast<int16_t>(i);
				std::memset(entry.name, 0, sizeof(entry.name));
				std::snprintf(entry.name, sizeof(entry.name), "%s", skill->m_cName);
				entry.isUseable = skill->m_bIsUseable ? 1 : 0;
				entry.useMethod = skill->m_cUseMethod;
				entriesInPacket++;
			}

			pktHeader->skillCount = entriesInPacket;
			size_t packetSize = headerSize + (entriesInPacket * entrySize);

			uint16_t len = static_cast<uint16_t>(packetSize);
			const uint8_t* lenBytes = reinterpret_cast<const uint8_t*>(&len);
			allData.push_back(lenBytes[0]);
			allData.push_back(lenBytes[1]);
			allData.insert(allData.end(), reinterpret_cast<uint8_t*>(buf), reinterpret_cast<uint8_t*>(buf) + packetSize);

			skillsSent += entriesInPacket;
			packetIndex++;
		}
		m_dwConfigHash[2] = allData.empty() ? 0 : hb::shared::util::hb_crc32(allData.data(), allData.size());
	}

	// Compute CRC32 for NPC configs
	{
		constexpr size_t headerSize = sizeof(hb::net::PacketNpcConfigHeader);
		constexpr size_t entrySize = sizeof(hb::net::PacketNpcConfigEntry);
		constexpr size_t maxEntriesPerPacket = (7000 - headerSize) / entrySize;

		std::vector<uint8_t> allData;
		int totalNpcs = 0;
		for(int i = 0; i < MaxNpcTypes; i++) {
			if (m_pNpcConfigList[i] != 0) totalNpcs++;
		}

		int npcsSent = 0;
		int packetIndex = 0;
		while (npcsSent < totalNpcs) {
			char buf[7000]{};
			auto* pktHeader = reinterpret_cast<hb::net::PacketNpcConfigHeader*>(buf);
			pktHeader->header.msg_id = MsgId::NpcConfigContents;
			pktHeader->header.msg_type = MsgType::Confirm;
			pktHeader->totalNpcs = static_cast<uint16_t>(totalNpcs);
			pktHeader->packetIndex = static_cast<uint16_t>(packetIndex);

			auto* entries = reinterpret_cast<hb::net::PacketNpcConfigEntry*>(buf + headerSize);
			uint16_t entriesInPacket = 0;
			int skipped = 0;

			for(int i = 0; i < MaxNpcTypes && entriesInPacket < maxEntriesPerPacket; i++) {
				if (m_pNpcConfigList[i] == 0) continue;
				if (skipped < npcsSent) { skipped++; continue; }

				const CNpc* npc = m_pNpcConfigList[i];
				auto& entry = entries[entriesInPacket];
				entry.npcId = static_cast<int16_t>(i);
				entry.npcType = npc->m_sType;
				std::memset(entry.name, 0, sizeof(entry.name));
				std::snprintf(entry.name, sizeof(entry.name), "%s", npc->m_cNpcName);
				entriesInPacket++;
			}

			pktHeader->npcCount = entriesInPacket;
			size_t packetSize = headerSize + (entriesInPacket * entrySize);

			uint16_t len = static_cast<uint16_t>(packetSize);
			const uint8_t* lenBytes = reinterpret_cast<const uint8_t*>(&len);
			allData.push_back(lenBytes[0]);
			allData.push_back(lenBytes[1]);
			allData.insert(allData.end(), reinterpret_cast<uint8_t*>(buf), reinterpret_cast<uint8_t*>(buf) + packetSize);

			npcsSent += entriesInPacket;
			packetIndex++;
		}
		m_dwConfigHash[3] = allData.empty() ? 0 : hb::shared::util::hb_crc32(allData.data(), allData.size());
	}

	std::snprintf(G_cTxt, sizeof(G_cTxt), "Config hashes computed - Items: 0x%08X, Magic: 0x%08X, Skills: 0x%08X, Npcs: 0x%08X",
		m_dwConfigHash[0], m_dwConfigHash[1], m_dwConfigHash[2], m_dwConfigHash[3]);
	PutLogList(G_cTxt);
}


void CGame::FillPlayerMapObject(hb::net::PacketMapDataObjectPlayer& obj, short sOwnerH, int iViewerH)
{
	auto* client = m_pClientList[sOwnerH];
	obj.base.object_id = static_cast<uint16_t>(sOwnerH);
	obj.type = client->m_sType;
	obj.dir = client->m_cDir;
	obj.appearance = client->m_appearance;
	obj.status = client->m_status;
	obj.status.bPK = (client->m_iPKCount != 0) ? 1 : 0;
	obj.status.bCitizen = (client->m_cSide != 0) ? 1 : 0;
	obj.status.bAresden = (client->m_cSide == 1) ? 1 : 0;
	obj.status.bHunter = client->m_bIsPlayerCivil ? 1 : 0;
	obj.status.iRelationship = m_pCombatManager->GetPlayerRelationship(sOwnerH, iViewerH);
	std::memcpy(obj.name, client->m_cCharName, sizeof(obj.name));
}

void CGame::FillNpcMapObject(hb::net::PacketMapDataObjectNpc& obj, short sOwnerH, int iViewerH)
{
	auto* npc = m_pNpcList[sOwnerH];
	obj.base.object_id = static_cast<uint16_t>(sOwnerH + hb::shared::object_id::NpcMin);
	obj.config_id = npc->m_iNpcConfigId;
	obj.dir = npc->m_cDir;
	obj.appearance = npc->m_appearance;
	obj.status = npc->m_status;
	obj.status.iRelationship = m_pEntityManager->GetNpcRelationship(sOwnerH, iViewerH);
	std::memcpy(obj.name, npc->m_cName, sizeof(obj.name));
}

int CGame::iComposeInitMapData(short sX, short sY, int iClientH, char* pData)
{
	int iSize, iTileExists;
	class CTile* pTile;
	unsigned char ucHeader;
	char* cp;

	if (m_pClientList[iClientH] == 0) return 0;

	cp = pData + sizeof(hb::net::PacketMapDataHeader);
	iSize = sizeof(hb::net::PacketMapDataHeader);
	iTileExists = 0;

	for(int iy = 0; iy < hb::shared::view::InitDataTilesY; iy++)
		for(int ix = 0; ix < hb::shared::view::InitDataTilesX; ix++) {

			if (((sX + ix) == 100) && ((sY + iy) == 100))
				sX = sX;

			pTile = (class CTile*)(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_pTile + (sX + ix) + (sY + iy) * m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_sSizeY);

			//If player not same side and is invied (Beholder Hack)
			/*if ((m_pClientList[pTile->m_sOwner] != 0) && (pTile->m_sOwner != iClientH))
				if ((m_pClientList[pTile->m_sOwner]->m_cSide != 0) &&
					(m_pClientList[pTile->m_sOwner]->m_cSide != m_pClientList[iClientH]->m_cSide) &&
					(m_pClientList[pTile->m_sOwner]->m_status.bInvisibility)) {
					continue;
				}*/

			if ((pTile->m_sOwner != 0) || (pTile->m_sDeadOwner != 0) ||
				(pTile->m_pItem[0] != 0) || (pTile->m_sDynamicObjectType != 0)) {
				iTileExists++;

				ucHeader = 0;
				if (pTile->m_sOwner != 0) {
					if (pTile->m_cOwnerClass == hb::shared::owner_class::Player) {
						if (m_pClientList[pTile->m_sOwner] != 0) {
							if (m_pClientList[pTile->m_sOwner]->m_bIsAdminInvisible &&
								pTile->m_sOwner != iClientH &&
								m_pClientList[iClientH]->m_iAdminLevel <= m_pClientList[pTile->m_sOwner]->m_iAdminLevel)
							{
							}
							else
							{
								ucHeader = ucHeader | 0x01;
							}
						}
						else {
							std::snprintf(G_cTxt, sizeof(G_cTxt), "Empty player handle: %d", pTile->m_sOwner);
							pTile->m_sOwner = 0;
						}
					}

					if (pTile->m_cOwnerClass == hb::shared::owner_class::Npc) {
						if (m_pNpcList[pTile->m_sOwner] != 0) ucHeader = ucHeader | 0x01;
						else pTile->m_sOwner = 0;
					}
				}
				if (pTile->m_sDeadOwner != 0) {
					if (pTile->m_cDeadOwnerClass == hb::shared::owner_class::Player) {
						if (m_pClientList[pTile->m_sDeadOwner] != 0) ucHeader = ucHeader | 0x02;
						else pTile->m_sDeadOwner = 0;
					}
					if (pTile->m_cDeadOwnerClass == hb::shared::owner_class::Npc) {
						if (m_pNpcList[pTile->m_sDeadOwner] != 0) ucHeader = ucHeader | 0x02;
						else pTile->m_sDeadOwner = 0;
					}
				}
				if (pTile->m_pItem[0] != 0)				ucHeader = ucHeader | 0x04;
				if (pTile->m_sDynamicObjectType != 0)    ucHeader = ucHeader | 0x08;

				hb::net::PacketMapDataEntryHeader entryHeader{};
				entryHeader.x = static_cast<int16_t>(ix);
				entryHeader.y = static_cast<int16_t>(iy);
				entryHeader.flags = ucHeader;
				std::memcpy(cp, &entryHeader, sizeof(entryHeader));
				cp += sizeof(entryHeader);
				iSize += sizeof(entryHeader);

				if ((ucHeader & 0x01) != 0) {
					switch (pTile->m_cOwnerClass) {
					case hb::shared::owner_class::Player:
					{
						hb::net::PacketMapDataObjectPlayer obj{};
						FillPlayerMapObject(obj, pTile->m_sOwner, iClientH);
						if (m_pClientList[pTile->m_sOwner]->m_bIsAdminInvisible) {
							obj.status.bInvisibility = true;
							obj.status.bGMMode = true;
						}
						std::memcpy(cp, &obj, sizeof(obj));
						cp += sizeof(obj);
						iSize += sizeof(obj);
						break;
					}
					case hb::shared::owner_class::Npc:
					{
						hb::net::PacketMapDataObjectNpc obj{};
						FillNpcMapObject(obj, pTile->m_sOwner, iClientH);
						std::memcpy(cp, &obj, sizeof(obj));
						cp += sizeof(obj);
						iSize += sizeof(obj);
						break;
					}
					}
				}

				if ((ucHeader & 0x02) != 0) {
					switch (pTile->m_cDeadOwnerClass) {
					case hb::shared::owner_class::Player:
					{
						hb::net::PacketMapDataObjectPlayer obj{};
						FillPlayerMapObject(obj, pTile->m_sDeadOwner, iClientH);
						std::memcpy(cp, &obj, sizeof(obj));
						cp += sizeof(obj);
						iSize += sizeof(obj);
						break;
					}
					case hb::shared::owner_class::Npc:
					{
						hb::net::PacketMapDataObjectNpc obj{};
						FillNpcMapObject(obj, pTile->m_sDeadOwner, iClientH);
						std::memcpy(cp, &obj, sizeof(obj));
						cp += sizeof(obj);
						iSize += sizeof(obj);
						break;
					}
					}
				}

				if (pTile->m_pItem[0] != 0) {
					hb::net::PacketMapDataItem itemObj{};
					itemObj.item_id = pTile->m_pItem[0]->m_sIDnum;
					itemObj.color = pTile->m_pItem[0]->m_cItemColor;
					itemObj.attribute = pTile->m_pItem[0]->m_dwAttribute;
					std::memcpy(cp, &itemObj, sizeof(itemObj));
					cp += sizeof(itemObj);
					iSize += sizeof(itemObj);
				}

				if (pTile->m_sDynamicObjectType != 0) {
					hb::net::PacketMapDataDynamicObject dynObj{};
					dynObj.object_id = pTile->m_wDynamicObjectID;
					dynObj.type = pTile->m_sDynamicObjectType;
					std::memcpy(cp, &dynObj, sizeof(dynObj));
					cp += sizeof(dynObj);
					iSize += sizeof(dynObj);
				}
			} // Big if
		} // while(1)

	hb::net::PacketMapDataHeader header{};
	header.total = static_cast<int16_t>(iTileExists);
	std::memcpy(pData, &header, sizeof(header));
	return iSize;
}

void CGame::DeleteClient(int iClientH, bool bSave, bool bNotify, bool bCountLogout, bool bForceCloseConn)
{
	int iExH;
	char cTmpMap[30];

	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_bIsInitComplete) {
		if (memcmp(m_pClientList[iClientH]->m_cMapName, "fight", 5) == 0) {
			std::snprintf(G_cTxt, sizeof(G_cTxt), "Char(%s)-Exit(%s)", m_pClientList[iClientH]->m_cCharName, m_pClientList[iClientH]->m_cMapName);
			PutLogEventFileList(G_cTxt);
		}

		if (m_pClientList[iClientH]->m_bIsExchangeMode) {
			iExH = m_pClientList[iClientH]->m_iExchangeH;
			m_pItemManager->_ClearExchangeStatus(iExH);
			m_pItemManager->_ClearExchangeStatus(iClientH);
		}

		m_pFishingManager->ReleaseFishEngagement(iClientH);

		if (bNotify)
			SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventLog, MsgType::Reject, 0, 0, 0);

		m_pCombatManager->RemoveFromTarget(iClientH, hb::shared::owner_class::Player);

		// Delete all summoned NPCs belonging to this player
		for (int i = 0; i < MaxNpcs; i++)
			if (m_pNpcList[i] != 0) {
				if ((m_pNpcList[i]->m_bIsSummoned) &&
					(m_pNpcList[i]->m_iFollowOwnerIndex == iClientH) &&
					(m_pNpcList[i]->m_cFollowOwnerType == hb::shared::owner_class::Player)) {
					m_pEntityManager->DeleteEntity(i);
				}
			}

		for(int i = 1; i < MaxClients; i++)
			if ((m_pClientList[i] != 0) && (m_pClientList[i]->m_iWhisperPlayerIndex == iClientH)) {
				m_pClientList[i]->m_iWhisperPlayerIndex = -1;
				SendNotifyMsg(0, i, Notify::WhisperModeOff, 0, 0, 0, m_pClientList[iClientH]->m_cCharName);
			}


		m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->ClearOwner(2, iClientH, hb::shared::owner_class::Player,
			m_pClientList[iClientH]->m_sX,
			m_pClientList[iClientH]->m_sY);

		m_pDelayEventManager->bRemoveFromDelayEventList(iClientH, hb::shared::owner_class::Player, 0);
	}

	if ((bSave) && (m_pClientList[iClientH]->m_bIsOnServerChange == false)) {

		if (m_pClientList[iClientH]->m_bIsKilled) {
			m_pClientList[iClientH]->m_sX = -1;
			m_pClientList[iClientH]->m_sY = -1;

			strcpy(cTmpMap, m_pClientList[iClientH]->m_cMapName);

			std::memset(m_pClientList[iClientH]->m_cMapName, 0, sizeof(m_pClientList[iClientH]->m_cMapName));

			if (m_pClientList[iClientH]->m_cSide == 0) {
				strcpy(m_pClientList[iClientH]->m_cMapName, "default");
			}
			else {
				if (memcmp(m_pClientList[iClientH]->m_cLocation, "are", 3) == 0) {
					if (m_bIsCrusadeMode) {
						if (m_pClientList[iClientH]->m_iDeadPenaltyTime > 0) {
							std::memset(m_pClientList[iClientH]->m_cLockedMapName, 0, sizeof(m_pClientList[iClientH]->m_cLockedMapName));
							strcpy(m_pClientList[iClientH]->m_cLockedMapName, "aresden");
							m_pClientList[iClientH]->m_iLockedMapTime = 60 * 5;
							m_pClientList[iClientH]->m_iDeadPenaltyTime = 60 * 10;
						}
						else {
							m_pClientList[iClientH]->m_iDeadPenaltyTime = 60 * 10;
						}
					}

					if (strcmp(cTmpMap, "elvine") == 0) {
						strcpy(m_pClientList[iClientH]->m_cLockedMapName, "elvjail");
						m_pClientList[iClientH]->m_iLockedMapTime = 60 * 3;
						memcpy(m_pClientList[iClientH]->m_cMapName, "elvjail", 7);
					}
					else if (m_pClientList[iClientH]->m_iLevel > 80)
						memcpy(m_pClientList[iClientH]->m_cMapName, "resurr1", 7);
					else memcpy(m_pClientList[iClientH]->m_cMapName, "arefarm", 7);
				}
				else {
					if (m_bIsCrusadeMode) {
						if (m_pClientList[iClientH]->m_iDeadPenaltyTime > 0) {
							std::memset(m_pClientList[iClientH]->m_cLockedMapName, 0, sizeof(m_pClientList[iClientH]->m_cLockedMapName));
							strcpy(m_pClientList[iClientH]->m_cLockedMapName, "elvine");
							m_pClientList[iClientH]->m_iLockedMapTime = 60 * 5;
							m_pClientList[iClientH]->m_iDeadPenaltyTime = 60 * 10;
						}
						else {
							m_pClientList[iClientH]->m_iDeadPenaltyTime = 60 * 10;
						}
					}
					if (strcmp(cTmpMap, "aresden") == 0) {
						strcpy(m_pClientList[iClientH]->m_cLockedMapName, "arejail");
						m_pClientList[iClientH]->m_iLockedMapTime = 60 * 3;
						memcpy(m_pClientList[iClientH]->m_cMapName, "arejail", 7);

					}
					else if (m_pClientList[iClientH]->m_iLevel > 80)
						memcpy(m_pClientList[iClientH]->m_cMapName, "resurr2", 7);
					else memcpy(m_pClientList[iClientH]->m_cMapName, "elvfarm", 7);
				}
			}
		}
		else if (bForceCloseConn) {
			std::memset(m_pClientList[iClientH]->m_cMapName, 0, sizeof(m_pClientList[iClientH]->m_cMapName));
			memcpy(m_pClientList[iClientH]->m_cMapName, "bisle", 5);
			m_pClientList[iClientH]->m_sX = -1;
			m_pClientList[iClientH]->m_sY = -1;

			std::memset(m_pClientList[iClientH]->m_cLockedMapName, 0, sizeof(m_pClientList[iClientH]->m_cLockedMapName));
			strcpy(m_pClientList[iClientH]->m_cLockedMapName, "bisle");
			m_pClientList[iClientH]->m_iLockedMapTime = 10 * 60;
		}

		if (m_pClientList[iClientH]->m_bIsObserverMode) {
			std::memset(m_pClientList[iClientH]->m_cMapName, 0, sizeof(m_pClientList[iClientH]->m_cMapName));
			if (m_pClientList[iClientH]->m_cSide == 0) {
				switch (iDice(1, 2)) {
				case 1:
					memcpy(m_pClientList[iClientH]->m_cMapName, "aresden", 7);
					break;
				case 2:
					memcpy(m_pClientList[iClientH]->m_cMapName, "elvine", 6);
					break;
				}
			}
			else {
				memcpy(m_pClientList[iClientH]->m_cMapName, m_pClientList[iClientH]->m_cLocation, 10);
			}
			m_pClientList[iClientH]->m_sX = -1;
			m_pClientList[iClientH]->m_sY = -1;
		}

		if (memcmp(m_pClientList[iClientH]->m_cMapName, "fight", 5) == 0) {
			std::memset(m_pClientList[iClientH]->m_cMapName, 0, sizeof(m_pClientList[iClientH]->m_cMapName));
			if (m_pClientList[iClientH]->m_cSide == 0) {
				switch (iDice(1, 2)) {
				case 1:
					memcpy(m_pClientList[iClientH]->m_cMapName, "aresden", 7);
					break;
				case 2:
					memcpy(m_pClientList[iClientH]->m_cMapName, "elvine", 6);
					break;
				}
			}
			else {
				memcpy(m_pClientList[iClientH]->m_cMapName, m_pClientList[iClientH]->m_cLocation, 10);
			}
			m_pClientList[iClientH]->m_sX = -1;
			m_pClientList[iClientH]->m_sY = -1;
		}

		if (m_pClientList[iClientH]->m_bIsInitComplete) {
			if (m_pClientList[iClientH]->m_iPartyID != 0) {
				hb::net::PartyOpPayload partyOp{};
				partyOp.op_type = 4;
				partyOp.client_h = static_cast<uint16_t>(iClientH);
				std::memcpy(partyOp.name, m_pClientList[iClientH]->m_cCharName, sizeof(partyOp.name));
				partyOp.party_id = static_cast<uint16_t>(m_pClientList[iClientH]->m_iPartyID);
				PartyOperation(reinterpret_cast<char*>(&partyOp));
			}
		}
		g_login->LocalSavePlayerData(iClientH);
	}
	else {
		if (m_pClientList[iClientH]->m_bIsOnServerChange == false) {
			if (m_pClientList[iClientH]->m_iPartyID != 0) {
				hb::net::PartyOpPayload partyOp{};
				partyOp.op_type = 4;
				partyOp.client_h = static_cast<uint16_t>(iClientH);
				std::memcpy(partyOp.name, m_pClientList[iClientH]->m_cCharName, sizeof(partyOp.name));
				partyOp.party_id = static_cast<uint16_t>(m_pClientList[iClientH]->m_iPartyID);
				PartyOperation(reinterpret_cast<char*>(&partyOp));
			}
		}
		else {
			if (m_pClientList[iClientH]->m_iPartyID != 0) {
				hb::net::PartyOpPayload partyOp{};
				partyOp.op_type = 7;
				partyOp.client_h = 0;
				std::memcpy(partyOp.name, m_pClientList[iClientH]->m_cCharName, sizeof(partyOp.name));
				partyOp.party_id = static_cast<uint16_t>(m_pClientList[iClientH]->m_iPartyID);
				PartyOperation(reinterpret_cast<char*>(&partyOp));
			}
		}
	}

	if (m_pClientList[iClientH]->m_iPartyID != 0) {
		for(int i = 0; i < hb::shared::limits::MaxPartyMembers; i++)
			if (m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iIndex[i] == iClientH) {
				m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iIndex[i] = 0;
				m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iTotalMembers--;
				m_pClientList[iClientH]->m_iPartyID = 0;
				m_pClientList[iClientH]->m_iPartyStatus = PartyStatus::Null;
				m_pClientList[iClientH]->m_iReqJoinPartyClientH = 0;
				std::snprintf(G_cTxt, sizeof(G_cTxt), "PartyID:%d member:%d Out(Delete) Total:%d", m_pClientList[iClientH]->m_iPartyID, iClientH, m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iTotalMembers);
				PutLogList(G_cTxt);
				goto DC_LOOPBREAK1;
			}
	DC_LOOPBREAK1:
		for(int i = 0; i < hb::shared::limits::MaxPartyMembers - 1; i++)
			if ((m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iIndex[i] == 0) && (m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iIndex[i + 1] != 0)) {
				m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iIndex[i] = m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iIndex[i + 1];
				m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iIndex[i + 1] = 0;
			}
	}


	m_iTotalClients--;

	// Cancel async operations before freeing the socket
	if (m_pClientList[iClientH]->m_pXSock != 0)
		m_pClientList[iClientH]->m_pXSock->CancelAsync();

	delete m_pClientList[iClientH];
	m_pClientList[iClientH] = 0;

	RemoveClientShortCut(iClientH);
}


void CGame::SendEventToNearClient_TypeA(short sOwnerH, char cOwnerType, uint32_t dwMsgID, uint16_t wMsgType, short sV1, short sV2, short sV3)
{
	int iRet, iShortCutIndex;
	bool bFlag;
	int sRange;
	short sX, sY;
	bool cOwnerSend;

	if ((dwMsgID == MsgId::EventLog) || (wMsgType == Type::Move) || (wMsgType == Type::Run) ||
		(wMsgType == Type::AttackMove) || (wMsgType == Type::DamageMove) || (wMsgType == Type::Dying))
		sRange = hb::shared::view::RangeBuffer;
	else sRange = 0;

	if (cOwnerType == hb::shared::owner_class::Player) {
		if (m_pClientList[sOwnerH] == 0) return;

		sX = m_pClientList[sOwnerH]->m_sX;
		sY = m_pClientList[sOwnerH]->m_sY;

		switch (wMsgType) {
		case Type::NullAction:
		case Type::Damage:
		case Type::Dying:
		case MsgType::Confirm:
			cOwnerSend = true;
			break;
		default:
			cOwnerSend = false;
			break;
		}

		hb::net::PacketEventMotionPlayer base_all{};
		base_all.header.msg_id = dwMsgID;
		base_all.header.msg_type = wMsgType;
		base_all.object_id = static_cast<std::uint16_t>(sOwnerH);
		base_all.x = sX;
		base_all.y = sY;
		base_all.type = m_pClientList[sOwnerH]->m_sType;
		base_all.dir = static_cast<std::uint8_t>(m_pClientList[sOwnerH]->m_cDir);
		std::memcpy(base_all.name, m_pClientList[sOwnerH]->m_cCharName, sizeof(base_all.name));
		base_all.appearance = m_pClientList[sOwnerH]->m_appearance;
		base_all.status = m_pClientList[sOwnerH]->m_status;
		base_all.loc = 0;
		if (wMsgType == Type::NullAction) {
			base_all.loc = m_pClientList[sOwnerH]->m_bIsKilled ? 1 : 0;
		}

		hb::net::PacketEventMotionShort pkt_short{};
		pkt_short.header.msg_id = dwMsgID;
		pkt_short.header.msg_type = wMsgType;
		pkt_short.object_id = static_cast<std::uint16_t>(sOwnerH + hb::shared::object_id::NearbyOffset);
		pkt_short.dir = static_cast<std::uint8_t>(m_pClientList[sOwnerH]->m_cDir);
		pkt_short.v1 = static_cast<std::int16_t>(sV1);
		pkt_short.v2 = static_cast<std::uint8_t>(sV2);

		hb::net::PacketEventMotionMove pkt_move{};
		pkt_move.header.msg_id = dwMsgID;
		pkt_move.header.msg_type = wMsgType;
		pkt_move.object_id = static_cast<std::uint16_t>(sOwnerH + hb::shared::object_id::NearbyOffset);
		pkt_move.dir = static_cast<std::uint8_t>(m_pClientList[sOwnerH]->m_cDir);
		pkt_move.v1 = static_cast<std::uint8_t>(sV1);
		pkt_move.v2 = static_cast<std::uint8_t>(sV2);
		pkt_move.x = sX;
		pkt_move.y = sY;

		hb::net::PacketEventMotionAttack pkt_attack{};
		pkt_attack.header.msg_id = dwMsgID;
		pkt_attack.header.msg_type = wMsgType;
		pkt_attack.object_id = static_cast<std::uint16_t>(sOwnerH + hb::shared::object_id::NearbyOffset);
		pkt_attack.dir = static_cast<std::uint8_t>(m_pClientList[sOwnerH]->m_cDir);
		pkt_attack.v1 = static_cast<std::int8_t>(sV1 - sX);
		pkt_attack.v2 = static_cast<std::int8_t>(sV2 - sY);
		pkt_attack.v3 = static_cast<std::int16_t>(sV3);

		hb::net::PacketEventMotionDirOnly pkt_dir{};
		pkt_dir.header.msg_id = dwMsgID;
		pkt_dir.header.msg_type = wMsgType;
		pkt_dir.object_id = static_cast<std::uint16_t>(sOwnerH + hb::shared::object_id::NearbyOffset);
		pkt_dir.dir = static_cast<std::uint8_t>(m_pClientList[sOwnerH]->m_cDir);

		// Per-viewer status filtering is handled at packet build time

		const char cKey = static_cast<char>((rand() % 255) + 1);

		bFlag = true;
		iShortCutIndex = 0;

		while (bFlag) {
			int i = m_iClientShortCut[iShortCutIndex];
			iShortCutIndex++;
			if (i == 0) bFlag = false;

			if ((bFlag) && (m_pClientList[i] != 0) && (m_pClientList[i]->m_bIsInitComplete))
				if ((m_pClientList[i]->m_cMapIndex == m_pClientList[sOwnerH]->m_cMapIndex) &&
					(m_pClientList[i]->m_sX >= m_pClientList[sOwnerH]->m_sX - hb::shared::view::RangeX - sRange) &&
					(m_pClientList[i]->m_sX <= m_pClientList[sOwnerH]->m_sX + hb::shared::view::RangeX + sRange) &&
					(m_pClientList[i]->m_sY >= m_pClientList[sOwnerH]->m_sY - hb::shared::view::RangeY - sRange) &&
					(m_pClientList[i]->m_sY <= m_pClientList[sOwnerH]->m_sY + hb::shared::view::RangeY + sRange)) {

					// Admin invisibility filtering: skip clients that shouldn't see this player
					if (m_pClientList[sOwnerH]->m_bIsAdminInvisible && i != sOwnerH &&
						m_pClientList[i]->m_iAdminLevel <= m_pClientList[sOwnerH]->m_iAdminLevel)
					{
						// Don't send any packet to this client
					}
					else
					{

					auto pkt_all = base_all;
					{
						auto viewerStatus = m_pClientList[sOwnerH]->m_status;
						viewerStatus.bPK = (m_pClientList[sOwnerH]->m_iPKCount != 0) ? 1 : 0;
						viewerStatus.bCitizen = (m_pClientList[sOwnerH]->m_cSide != 0) ? 1 : 0;
						viewerStatus.bAresden = (m_pClientList[sOwnerH]->m_cSide == 1) ? 1 : 0;
						viewerStatus.bHunter = m_pClientList[sOwnerH]->m_bIsPlayerCivil ? 1 : 0;
						viewerStatus.iRelationship = m_pCombatManager->GetPlayerRelationship(sOwnerH, i);
						if (m_pClientList[sOwnerH]->m_cSide != m_pClientList[i]->m_cSide && i != sOwnerH) {
							viewerStatus.bPoisoned = false;
							viewerStatus.bIllusion = false;
						}
						if (m_pClientList[sOwnerH]->m_bIsAdminInvisible) {
							viewerStatus.bInvisibility = true;
							viewerStatus.bGMMode = true;
						}
						pkt_all.status = viewerStatus;
					}

					auto send_packet = [&](const void* packet, std::size_t size) -> int {
						return m_pClientList[i]->m_pXSock->iSendMsg(reinterpret_cast<char*>(const_cast<void*>(packet)), static_cast<int>(size), cKey);
						};

					const bool is_near = (m_pClientList[i]->m_sX >= m_pClientList[sOwnerH]->m_sX - (hb::shared::view::CenterX - 1)) &&
						(m_pClientList[i]->m_sX <= m_pClientList[sOwnerH]->m_sX + (hb::shared::view::CenterX - 1)) &&
						(m_pClientList[i]->m_sY >= m_pClientList[sOwnerH]->m_sY - hb::shared::view::CenterY) &&
						(m_pClientList[i]->m_sY <= m_pClientList[sOwnerH]->m_sY + hb::shared::view::CenterY);

					if (is_near) {
						switch (wMsgType) {
						case MsgType::Confirm:
						case MsgType::Reject:
						case Type::NullAction:
							if (cOwnerSend)
								iRet = send_packet(&pkt_all, sizeof(pkt_all));
							else if (i != sOwnerH)
								iRet = send_packet(&pkt_all, sizeof(pkt_all));
							break;

						case Type::Attack:
						case Type::AttackMove:
							if (cOwnerSend)
								iRet = send_packet(&pkt_attack, sizeof(pkt_attack));
							else if (i != sOwnerH)
								iRet = send_packet(&pkt_attack, sizeof(pkt_attack));
							break;

						case Type::Magic:
						case Type::Damage:
						case Type::DamageMove:
							if (cOwnerSend)
								iRet = send_packet(&pkt_short, sizeof(pkt_short));
							else if (i != sOwnerH)
								iRet = send_packet(&pkt_short, sizeof(pkt_short));
							break;

						case Type::Dying:
							if (cOwnerSend)
								iRet = send_packet(&pkt_move, sizeof(pkt_move));
							else if (i != sOwnerH)
								iRet = send_packet(&pkt_move, sizeof(pkt_move));
							break;

						default:
							if (cOwnerSend)
								iRet = send_packet(&pkt_dir, sizeof(pkt_dir));
							else if (i != sOwnerH)
								iRet = send_packet(&pkt_dir, sizeof(pkt_dir));
							break;
						}
					}
					else {
						switch (wMsgType) {
						case MsgType::Confirm:
						case MsgType::Reject:
						case Type::NullAction:
							if (cOwnerSend)
								iRet = send_packet(&pkt_all, sizeof(pkt_all));
							else if (i != sOwnerH)
								iRet = send_packet(&pkt_all, sizeof(pkt_all));
							break;

						case Type::Attack:
						case Type::AttackMove:
							if (cOwnerSend)
								iRet = send_packet(&pkt_attack, sizeof(pkt_attack));
							else if (i != sOwnerH)
								iRet = send_packet(&pkt_attack, sizeof(pkt_attack));
							break;

						case Type::Magic:
						case Type::Damage:
						case Type::DamageMove:
							if (cOwnerSend)
								iRet = send_packet(&pkt_short, sizeof(pkt_short));
							else if (i != sOwnerH)
								iRet = send_packet(&pkt_short, sizeof(pkt_short));
							break;

						case Type::Dying:
							if (cOwnerSend)
								iRet = send_packet(&pkt_move, sizeof(pkt_move));
							else if (i != sOwnerH)
								iRet = send_packet(&pkt_move, sizeof(pkt_move));
							break;

						default:
							if (cOwnerSend)
								iRet = send_packet(&pkt_all, sizeof(pkt_all));
							else if (i != sOwnerH)
								iRet = send_packet(&pkt_all, sizeof(pkt_all));
							break;
						}

						if ((iRet == sock::Event::QueueFull) || (iRet == sock::Event::SocketError) ||
							(iRet == sock::Event::SocketClosed) || (iRet == sock::Event::CriticalError)) {
							static uint32_t s_dwLastNetWarn = 0;
							uint32_t dwNow = GameClock::GetTimeMS();
							if (dwNow - s_dwLastNetWarn > 5000) {
								std::snprintf(G_cTxt, sizeof(G_cTxt), "[NETWARN] SendEventToNearClient_TypeA: client=%d ret=%d ownerType=%d msgType=0x%X",
									i, iRet, cOwnerType, wMsgType);
								PutLogList(G_cTxt);
								s_dwLastNetWarn = dwNow;
							}
						}
					}
					} // end admin invis else
				}
		}
	}
	else {
		if (m_pNpcList[sOwnerH] == 0) return;

		sX = m_pNpcList[sOwnerH]->m_sX;
		sY = m_pNpcList[sOwnerH]->m_sY;

		hb::net::PacketEventMotionNpc base_all{};
		base_all.header.msg_id = dwMsgID;
		base_all.header.msg_type = wMsgType;
		base_all.object_id = static_cast<std::uint16_t>(sOwnerH + hb::shared::object_id::NpcMin);
		base_all.x = sX;
		base_all.y = sY;
		base_all.config_id = m_pNpcList[sOwnerH]->m_iNpcConfigId;
		base_all.dir = static_cast<std::uint8_t>(m_pNpcList[sOwnerH]->m_cDir);
		std::memcpy(base_all.name, m_pNpcList[sOwnerH]->m_cName, sizeof(base_all.name));
		base_all.appearance = m_pNpcList[sOwnerH]->m_appearance;
		base_all.status = m_pNpcList[sOwnerH]->m_status;
		base_all.loc = 0;
		if (wMsgType == Type::NullAction) {
			base_all.loc = m_pNpcList[sOwnerH]->m_bIsKilled ? 1 : 0;
		}

		hb::net::PacketEventMotionShort pkt_short{};
		pkt_short.header.msg_id = dwMsgID;
		pkt_short.header.msg_type = wMsgType;
		pkt_short.object_id = static_cast<std::uint16_t>(sOwnerH + 40000);
		pkt_short.dir = static_cast<std::uint8_t>(m_pNpcList[sOwnerH]->m_cDir);
		pkt_short.v1 = static_cast<std::int16_t>(sV1);
		pkt_short.v2 = static_cast<std::uint8_t>(sV2);

		hb::net::PacketEventMotionMove pkt_move{};
		pkt_move.header.msg_id = dwMsgID;
		pkt_move.header.msg_type = wMsgType;
		pkt_move.object_id = static_cast<std::uint16_t>(sOwnerH + 40000);
		pkt_move.dir = static_cast<std::uint8_t>(m_pNpcList[sOwnerH]->m_cDir);
		pkt_move.v1 = static_cast<std::uint8_t>(sV1);
		pkt_move.v2 = static_cast<std::uint8_t>(sV2);
		pkt_move.x = sX;
		pkt_move.y = sY;

		hb::net::PacketEventMotionAttack pkt_attack{};
		pkt_attack.header.msg_id = dwMsgID;
		pkt_attack.header.msg_type = wMsgType;
		pkt_attack.object_id = static_cast<std::uint16_t>(sOwnerH + 40000);
		pkt_attack.dir = static_cast<std::uint8_t>(m_pNpcList[sOwnerH]->m_cDir);
		pkt_attack.v1 = static_cast<std::int8_t>(sV1 - sX);
		pkt_attack.v2 = static_cast<std::int8_t>(sV2 - sY);
		pkt_attack.v3 = static_cast<std::int16_t>(sV3);

		hb::net::PacketEventMotionDirOnly pkt_dir{};
		pkt_dir.header.msg_id = dwMsgID;
		pkt_dir.header.msg_type = wMsgType;
		pkt_dir.object_id = static_cast<std::uint16_t>(sOwnerH + 40000);
		pkt_dir.dir = static_cast<std::uint8_t>(m_pNpcList[sOwnerH]->m_cDir);

		const char cKey = static_cast<char>((rand() % 255) + 1);

		bFlag = true;
		iShortCutIndex = 0;

		while (bFlag) {

			int i = m_iClientShortCut[iShortCutIndex];
			iShortCutIndex++;
			if (i == 0) bFlag = false;

			if ((bFlag) && (m_pClientList[i] != 0))

				if ((m_pClientList[i]->m_cMapIndex == m_pNpcList[sOwnerH]->m_cMapIndex) &&
					(m_pClientList[i]->m_sX >= m_pNpcList[sOwnerH]->m_sX - hb::shared::view::RangeX - sRange) &&
					(m_pClientList[i]->m_sX <= m_pNpcList[sOwnerH]->m_sX + hb::shared::view::RangeX + sRange) &&
					(m_pClientList[i]->m_sY >= m_pNpcList[sOwnerH]->m_sY - hb::shared::view::RangeY - sRange) &&
					(m_pClientList[i]->m_sY <= m_pNpcList[sOwnerH]->m_sY + hb::shared::view::RangeY + sRange)) {

					auto pkt_all = base_all;
					pkt_all.status.iRelationship = m_pEntityManager->GetNpcRelationship(sOwnerH, i);

					auto send_packet = [&](const void* packet, std::size_t size) -> int {
						return m_pClientList[i]->m_pXSock->iSendMsg(reinterpret_cast<char*>(const_cast<void*>(packet)), static_cast<int>(size), cKey);
						};

					const bool is_near = (m_pClientList[i]->m_sX >= m_pNpcList[sOwnerH]->m_sX - (hb::shared::view::CenterX - 1)) &&
						(m_pClientList[i]->m_sX <= m_pNpcList[sOwnerH]->m_sX + (hb::shared::view::CenterX - 1)) &&
						(m_pClientList[i]->m_sY >= m_pNpcList[sOwnerH]->m_sY - hb::shared::view::CenterY) &&
						(m_pClientList[i]->m_sY <= m_pNpcList[sOwnerH]->m_sY + hb::shared::view::CenterY);

					if (is_near) {
						switch (wMsgType) {
						case MsgType::Confirm:
						case MsgType::Reject:
						case Type::NullAction:
							iRet = send_packet(&pkt_all, sizeof(pkt_all));
							break;

						case Type::Stop:
							iRet = send_packet(&pkt_all, sizeof(pkt_all));
							break;

						case Type::Dying:
							iRet = send_packet(&pkt_move, sizeof(pkt_move));
							break;

						case Type::Damage:
						case Type::DamageMove:
							iRet = send_packet(&pkt_short, sizeof(pkt_short));
							break;

						case Type::Attack:
						case Type::AttackMove:
							iRet = send_packet(&pkt_attack, sizeof(pkt_attack));
							break;

						case Type::Move:
						case Type::Run:
							// Send full position data to prevent desync for nearby players
							iRet = send_packet(&pkt_all, sizeof(pkt_all));
							break;

						default:
							iRet = send_packet(&pkt_dir, sizeof(pkt_dir));
							break;

						}

						if ((iRet == sock::Event::QueueFull) || (iRet == sock::Event::SocketError) ||
							(iRet == sock::Event::SocketClosed) || (iRet == sock::Event::CriticalError)) {
							static uint32_t s_dwLastNetWarnNpc = 0;
							uint32_t dwNow = GameClock::GetTimeMS();
							if (dwNow - s_dwLastNetWarnNpc > 5000) {
								std::snprintf(G_cTxt, sizeof(G_cTxt), "[NETWARN] SendEventToNearClient_TypeA(NPC): client=%d ret=%d ownerType=%d msgType=0x%X",
									i, iRet, cOwnerType, wMsgType);
								PutLogList(G_cTxt);
								s_dwLastNetWarnNpc = dwNow;
							}
						}
					}
					else {
						switch (wMsgType) {
						case MsgType::Confirm:
						case MsgType::Reject:
						case Type::NullAction:
							iRet = send_packet(&pkt_all, sizeof(pkt_all));
							break;

						case Type::Dying:
							iRet = send_packet(&pkt_move, sizeof(pkt_move));
							break;

						case Type::Damage:
						case Type::DamageMove:
							iRet = send_packet(&pkt_short, sizeof(pkt_short));
							break;

						case Type::Attack:
						case Type::AttackMove:
							iRet = send_packet(&pkt_attack, sizeof(pkt_attack));
							break;

						default:
							iRet = send_packet(&pkt_all, sizeof(pkt_all));
							break;

						} //Switch

						if ((iRet == sock::Event::QueueFull) || (iRet == sock::Event::SocketError) ||
							(iRet == sock::Event::SocketClosed) || (iRet == sock::Event::CriticalError)) {
							static uint32_t s_dwLastNetWarnNpcFar = 0;
							uint32_t dwNow = GameClock::GetTimeMS();
							if (dwNow - s_dwLastNetWarnNpcFar > 5000) {
								std::snprintf(G_cTxt, sizeof(G_cTxt), "[NETWARN] SendEventToNearClient_TypeA(NPC-far): client=%d ret=%d ownerType=%d msgType=0x%X",
									i, iRet, cOwnerType, wMsgType);
								PutLogList(G_cTxt);
								s_dwLastNetWarnNpcFar = dwNow;
							}
						}
					}
				}
		}
	} // else - NPC
}


int CGame::iComposeMoveMapData(short sX, short sY, int iClientH, char cDir, char* pData)
{
	int ix, iy, iSize, iTileExists, iIndex;
	class CTile* pTile;
	unsigned char ucHeader;
	char* cp;

	if (m_pClientList[iClientH] == 0) return 0;

	cp = pData + sizeof(hb::net::PacketMapDataHeader);
	iSize = sizeof(hb::net::PacketMapDataHeader);
	iTileExists = 0;

	iIndex = 0;

	while (1) {
		ix = _tmp_iMoveLocX[cDir][iIndex];
		iy = _tmp_iMoveLocY[cDir][iIndex];
		if ((ix == -1) || (iy == -1)) break;

		iIndex++;

		pTile = (class CTile*)(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_pTile + (sX + ix) + (sY + iy) * m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_sSizeY);

		//If player not same side and is invied (Beholder Hack)
		// there is another person on the tiles, and the owner is not the player
//xxxxxx
		/*if ((m_pClientList[pTile->m_sOwner] != 0) && (pTile->m_sOwner != iClientH))
			if ((m_pClientList[pTile->m_sOwner]->m_cSide != 0) &&
				(m_pClientList[pTile->m_sOwner]->m_cSide != m_pClientList[iClientH]->m_cSide) &&
				(m_pClientList[pTile->m_sOwner]->m_status.bInvisibility)) {
				continue;
			}*/

		if ((pTile->m_sOwner != 0) || (pTile->m_sDeadOwner != 0) ||
			(pTile->m_pItem[0] != 0) || (pTile->m_sDynamicObjectType != 0)) {

			iTileExists++;

			ucHeader = 0;
			if (pTile->m_sOwner != 0) {
				if (pTile->m_cOwnerClass == hb::shared::owner_class::Player) {
					if (m_pClientList[pTile->m_sOwner] != 0) {
						if (m_pClientList[pTile->m_sOwner]->m_bIsAdminInvisible &&
							pTile->m_sOwner != iClientH &&
							m_pClientList[iClientH]->m_iAdminLevel <= m_pClientList[pTile->m_sOwner]->m_iAdminLevel)
						{
						}
						else
						{
							ucHeader = ucHeader | 0x01;
						}
					}
					else pTile->m_sOwner = 0;
				}
				if (pTile->m_cOwnerClass == hb::shared::owner_class::Npc) {
					if (m_pNpcList[pTile->m_sOwner] != 0) ucHeader = ucHeader | 0x01;
					else pTile->m_sOwner = 0;
				}
			}
			if (pTile->m_sDeadOwner != 0) {
				if (pTile->m_cDeadOwnerClass == hb::shared::owner_class::Player) {
					if (m_pClientList[pTile->m_sDeadOwner] != 0)	ucHeader = ucHeader | 0x02;
					else pTile->m_sDeadOwner = 0;
				}
				if (pTile->m_cDeadOwnerClass == hb::shared::owner_class::Npc) {
					if (m_pNpcList[pTile->m_sDeadOwner] != 0) ucHeader = ucHeader | 0x02;
					else pTile->m_sDeadOwner = 0;
				}
			}
			if (pTile->m_pItem[0] != 0)				ucHeader = ucHeader | 0x04;
			if (pTile->m_sDynamicObjectType != 0)    ucHeader = ucHeader | 0x08;

			hb::net::PacketMapDataEntryHeader entryHeader{};
			entryHeader.x = static_cast<int16_t>(ix);
			entryHeader.y = static_cast<int16_t>(iy);
			entryHeader.flags = ucHeader;
			std::memcpy(cp, &entryHeader, sizeof(entryHeader));
			cp += sizeof(entryHeader);
			iSize += sizeof(entryHeader);

			if ((ucHeader & 0x01) != 0) {
				switch (pTile->m_cOwnerClass) {
				case hb::shared::owner_class::Player:
				{
					hb::net::PacketMapDataObjectPlayer obj{};
					FillPlayerMapObject(obj, pTile->m_sOwner, iClientH);
					if (m_pClientList[iClientH]->m_cSide != m_pClientList[pTile->m_sOwner]->m_cSide && iClientH != pTile->m_sOwner) {
						obj.status.bPoisoned = false;
						obj.status.bIllusion = false;
					}
					if (m_pClientList[pTile->m_sOwner]->m_bIsAdminInvisible) {
						obj.status.bInvisibility = true;
						obj.status.bGMMode = true;
					}
					std::memcpy(cp, &obj, sizeof(obj));
					cp += sizeof(obj);
					iSize += sizeof(obj);
					break;
				}
				case hb::shared::owner_class::Npc:
				{
					hb::net::PacketMapDataObjectNpc obj{};
					FillNpcMapObject(obj, pTile->m_sOwner, iClientH);
					std::memcpy(cp, &obj, sizeof(obj));
					cp += sizeof(obj);
					iSize += sizeof(obj);
					break;
				}
				}
			}

			if ((ucHeader & 0x02) != 0) {
				switch (pTile->m_cDeadOwnerClass) {
				case hb::shared::owner_class::Player:
				{
					hb::net::PacketMapDataObjectPlayer obj{};
					FillPlayerMapObject(obj, pTile->m_sDeadOwner, iClientH);
					if (m_pClientList[iClientH]->m_cSide != m_pClientList[pTile->m_sDeadOwner]->m_cSide && iClientH != pTile->m_sDeadOwner) {
						obj.status.bPoisoned = false;
						obj.status.bIllusion = false;
					}
					std::memcpy(cp, &obj, sizeof(obj));
					cp += sizeof(obj);
					iSize += sizeof(obj);
					break;
				}
				case hb::shared::owner_class::Npc:
				{
					hb::net::PacketMapDataObjectNpc obj{};
					FillNpcMapObject(obj, pTile->m_sDeadOwner, iClientH);
					std::memcpy(cp, &obj, sizeof(obj));
					cp += sizeof(obj);
					iSize += sizeof(obj);
					break;
				}
				}
			}

			if (pTile->m_pItem[0] != 0) {
				hb::net::PacketMapDataItem itemObj{};
				itemObj.item_id = pTile->m_pItem[0]->m_sIDnum;
				itemObj.color = pTile->m_pItem[0]->m_cItemColor;
				itemObj.attribute = pTile->m_pItem[0]->m_dwAttribute;
				std::memcpy(cp, &itemObj, sizeof(itemObj));
				cp += sizeof(itemObj);
				iSize += sizeof(itemObj);
			}

			if (pTile->m_sDynamicObjectType != 0) {
				hb::net::PacketMapDataDynamicObject dynObj{};
				dynObj.object_id = pTile->m_wDynamicObjectID;
				dynObj.type = pTile->m_sDynamicObjectType;
				std::memcpy(cp, &dynObj, sizeof(dynObj));
				cp += sizeof(dynObj);
				iSize += sizeof(dynObj);
			}

		} //(pTile->m_sOwner != 0)
	} // end While(1)

	hb::net::PacketMapDataHeader header{};
	header.total = static_cast<int16_t>(iTileExists);
	std::memcpy(pData, &header, sizeof(header));
	return iSize;
}

void CGame::CheckClientResponseTime()
{
	int iPlusTime, iMaxSuperAttack, iValue;
	uint32_t dwTime;
	short sItemIndex;
	static uint32_t s_dwLastIdleLog = 0;
	//locobans
	//int iMapside, iMapside2;
	//SYSTEMTIME SysTime;

	   /*
	   GetLocalTime(&SysTime);
	   switch (SysTime.wDayOfWeek) {
	   case 1:	iWarPeriod = 30; break;
	   case 2:	iWarPeriod = 30; break;
	   case 3:	iWarPeriod = 60; break;
	   case 4:	iWarPeriod = 60*2;  break;
	   case 5:	iWarPeriod = 60*5;  break;
	   case 6:	iWarPeriod = 60*10; break;
	   case 0:	iWarPeriod = 60*20; break;
	   }
	   */

	dwTime = GameClock::GetTimeMS();

	for(int i = 1; i < MaxClients; i++) {
		if (m_pClientList[i] != 0) {

			if ((dwTime - m_pClientList[i]->m_dwTime) > (uint32_t)m_iClientTimeout) {
				if (m_pClientList[i]->m_bIsInitComplete) {
					//Testcode
					std::snprintf(G_cTxt, sizeof(G_cTxt), "Client Timeout: %s", m_pClientList[i]->m_cIPaddress);
					PutLogList(G_cTxt);

					DeleteClient(i, true, true);
				}
				else if ((dwTime - m_pClientList[i]->m_dwTime) > (uint32_t)m_iClientTimeout) {
					DeleteClient(i, false, false);
				}
			}
			else if (m_pClientList[i]->m_bIsInitComplete) {
				uint32_t dwIdle = dwTime - m_pClientList[i]->m_dwTime;
				if (dwIdle > 5000 && (dwTime - m_pClientList[i]->m_dwLastMsgTime) > 5000 &&
					(dwTime - s_dwLastIdleLog) > 5000) {
					std::snprintf(G_cTxt, sizeof(G_cTxt),
						"[NET] IDLE slot=%d idle=%ums lastmsg=0x%08X lastage=%ums size=%zu char=%s ip=%s",
						i, dwIdle, m_pClientList[i]->m_dwLastMsgId,
						dwTime - m_pClientList[i]->m_dwLastMsgTime,
						m_pClientList[i]->m_dwLastMsgSize,
						m_pClientList[i]->m_cCharName,
						m_pClientList[i]->m_cIPaddress);
					PutLogList(G_cTxt);
					s_dwLastIdleLog = dwTime;
				}

				// AFK detection: 3 minutes of no meaningful activity
				constexpr uint32_t AFK_TIMEOUT_MS = 180000;
				bool bWasAfk = m_pClientList[i]->m_status.bAfk;
				bool bNowAfk = (dwTime - m_pClientList[i]->m_dwAfkActivityTime) > AFK_TIMEOUT_MS;
				if (bWasAfk != bNowAfk) {
					m_pClientList[i]->m_status.bAfk = bNowAfk;
					SendEventToNearClient_TypeA(i, hb::shared::owner_class::Player,
						MsgId::EventMotion, Type::NullAction, 0, 0, 0);
				}

				m_pClientList[i]->m_iTimeLeft_Rating--;
				if (m_pClientList[i]->m_iTimeLeft_Rating < 0) m_pClientList[i]->m_iTimeLeft_Rating = 0;

				if (((dwTime - m_pClientList[i]->m_dwHungerTime) > (uint32_t)m_iHungerConsumeInterval) && (m_pClientList[i]->m_bIsKilled == false)) {
					m_pClientList[i]->m_iHungerStatus--;
					if (m_pClientList[i]->m_iHungerStatus <= 0) m_pClientList[i]->m_iHungerStatus = 0;
					m_pClientList[i]->m_dwHungerTime = dwTime;

					SendNotifyMsg(0, i, Notify::Hunger, m_pClientList[i]->m_iHungerStatus, 0, 0, 0);
				}

				if (_bCheckCharacterData(i) == false) {
					DeleteClient(i, true, true);
					break;
				}

				if ((m_pClientList[i]->m_iHungerStatus <= 30) && (m_pClientList[i]->m_iHungerStatus >= 0))
					iPlusTime = (30 - m_pClientList[i]->m_iHungerStatus) * 1000;
				else iPlusTime = 0;

				iPlusTime = abs(iPlusTime);

				if ((dwTime - m_pClientList[i]->m_dwHPTime) > (uint32_t)(m_iHealthRegenInterval + iPlusTime)) {
					TimeHitPointsUp(i);
					m_pClientList[i]->m_dwHPTime = dwTime;
				}

				if ((dwTime - m_pClientList[i]->m_dwMPTime) > (uint32_t)(m_iManaRegenInterval + iPlusTime)) {
					TimeManaPointsUp(i);
					m_pClientList[i]->m_dwMPTime = dwTime;
				}

				if ((dwTime - m_pClientList[i]->m_dwSPTime) > (uint32_t)(m_iStaminaRegenInterval + iPlusTime)) {
					TimeStaminarPointsUp(i);
					m_pClientList[i]->m_dwSPTime = dwTime;
				}

				if ((m_pClientList[i]->m_bIsPoisoned) && ((dwTime - m_pClientList[i]->m_dwPoisonTime) > (uint32_t)m_iPoisonDamageInterval)) {
					m_pCombatManager->PoisonEffect(i, 0);
					m_pClientList[i]->m_dwPoisonTime = dwTime;
				}

				if ((m_pMapList[m_pClientList[i]->m_cMapIndex]->m_bIsFightZone == false) &&
					((dwTime - m_pClientList[i]->m_dwAutoSaveTime) > (uint32_t)m_iAutosaveInterval)) {
					g_login->LocalSavePlayerData(i); //bSendMsgToLS(ServerMsgId::RequestSavePlayerData, i);
					m_pClientList[i]->m_dwAutoSaveTime = dwTime;
				}

				// ExpStock
				if ((dwTime - m_pClientList[i]->m_dwExpStockTime) > (uint32_t)ExpStockTime) {
					m_pClientList[i]->m_dwExpStockTime = dwTime;
					CalcExpStock(i);
					m_pItemManager->CheckUniqueItemEquipment(i);
					m_pWarManager->CheckCrusadeResultCalculation(i);
					m_pWarManager->CheckHeldenianResultCalculation(i);
				}

				// AutoExe
				if ((dwTime - m_pClientList[i]->m_dwAutoExpTime) > (uint32_t)AutoExpTime) {
					iValue = (m_pClientList[i]->m_iLevel / 2);
					if (iValue <= 0) iValue = 1;
					uint32_t iValueDw = static_cast<uint32_t>(iValue);
					if (m_pClientList[i]->m_iAutoExpAmount < iValueDw) {
						if ((m_pClientList[i]->m_iExp + iValueDw) < m_iLevelExpTable[m_pClientList[i]->m_iLevel + 1]) {
							//m_pClientList[i]->m_iExpStock += iValue;
							GetExp(i, iValueDw, false);
							CalcExpStock(i);
						}
					}

					m_pClientList[i]->m_iAutoExpAmount = 0;
					m_pClientList[i]->m_dwAutoExpTime = dwTime;
				}

				// v1.432
				if (m_pClientList[i]->m_iSpecialAbilityTime == 3) {
					SendNotifyMsg(0, i, Notify::SpecialAbilityEnabled, 0, 0, 0, 0);
					// New 25/05/2004
					// After the time up, add magic back
					sItemIndex = m_pClientList[i]->m_sItemEquipmentStatus[ToInt(EquipPos::RightHand)];
					if (sItemIndex != -1) {
						if ((m_pClientList[i]->m_pItemList[sItemIndex]->m_sIDnum == 865) || (m_pClientList[i]->m_pItemList[sItemIndex]->m_sIDnum == 866)) {
							if ((m_pClientList[i]->m_iInt + m_pClientList[i]->m_iAngelicInt) > 99 && (m_pClientList[i]->m_iMag + m_pClientList[i]->m_iAngelicMag) > 99) {
								m_pClientList[i]->m_cMagicMastery[94] = true;
								SendNotifyMsg(0, i, Notify::StateChangeSuccess, 0, 0, 0, 0);
							}
						}
					}
				}
				m_pClientList[i]->m_iSpecialAbilityTime -= 3;
				if (m_pClientList[i]->m_iSpecialAbilityTime < 0) m_pClientList[i]->m_iSpecialAbilityTime = 0;

				// v1.432
				if (m_pClientList[i]->m_bIsSpecialAbilityEnabled) {
					uint32_t elapsedSec = (dwTime - m_pClientList[i]->m_dwSpecialAbilityStartTime) / 1000;
					if (elapsedSec > static_cast<uint32_t>(m_pClientList[i]->m_iSpecialAbilityLastSec)) {
						SendNotifyMsg(0, i, Notify::SpecialAbilityStatus, 3, 0, 0, 0);
						m_pClientList[i]->m_bIsSpecialAbilityEnabled = false;
						m_pClientList[i]->m_iSpecialAbilityTime = SpecialAbilityTimeSec;
						m_pClientList[i]->m_appearance.iEffectType = 0;
						SendEventToNearClient_TypeA(i, hb::shared::owner_class::Player, MsgId::EventMotion, Type::NullAction, 0, 0, 0);
					}
				}

				//Crusade
				m_pClientList[i]->m_iLockedMapTime -= 3;
				if (m_pClientList[i]->m_iLockedMapTime < 0) {
					m_pClientList[i]->m_iLockedMapTime = 0;
					std::memset(m_pClientList[i]->m_cLockedMapName, 0, sizeof(m_pClientList[i]->m_cLockedMapName));
					strcpy(m_pClientList[i]->m_cLockedMapName, "NONE");
				}

				// v2.04
				m_pClientList[i]->m_iDeadPenaltyTime -= 3;
				if (m_pClientList[i]->m_iDeadPenaltyTime < 0) m_pClientList[i]->m_iDeadPenaltyTime = 0;

				if ((m_pClientList[i]->m_bIsWarLocation) && IsEnemyZone(i)) {
					// Crusade
					if (m_bIsCrusadeMode == false)
						if (m_pClientList[i]->m_bIsInsideOwnTown == false)
							m_pClientList[i]->m_iTimeLeft_ForceRecall--;

					if (m_pClientList[i]->m_iTimeLeft_ForceRecall <= 0) {
						m_pClientList[i]->m_iTimeLeft_ForceRecall = 0;
						m_pClientList[i]->m_dwWarBeginTime = dwTime;
						m_pClientList[i]->m_bIsWarLocation = false;

						SendNotifyMsg(0, i, Notify::ToBeRecalled, 0, 0, 0, 0);
						RequestTeleportHandler(i, "1   ");
					}
				}

				if ((m_bIsHeldenianMode) && (m_pMapList[m_pClientList[i]->m_cMapIndex] != 0)) {
					if (m_pWarManager->bCheckHeldenianMap(i, m_iBTFieldMapIndex, hb::shared::owner_class::Player) == 1) {
						m_pStatusEffectManager->SetHeroFlag(i, hb::shared::owner_class::Player, true);
					}
					else {
						m_pStatusEffectManager->SetHeroFlag(i, hb::shared::owner_class::Player, false);
					}
				}

				if (m_pClientList[i] == 0) break;
				if (m_pClientList[i]->m_iSkillMsgRecvCount >= 2) {
					// std::snprintf(G_cTxt, sizeof(G_cTxt), "(!)  (%s)  ", m_pClientList[i]->m_cCharName);
					//PutLogFileList(G_cTxt);
					DeleteClient(i, true, true);
				}
				else {
					m_pClientList[i]->m_iSkillMsgRecvCount = 0;
				}

				if (m_pClientList[i] == 0) break;
				//if (m_pClientList[i]->m_iLevel < m_pMapList[m_pClientList[i]->m_cMapIndex]->m_iLevelLimit) {
				if ((m_pClientList[i]->m_iLevel < m_pMapList[m_pClientList[i]->m_cMapIndex]->m_iLevelLimit)) {
					SendNotifyMsg(0, i, Notify::ToBeRecalled, 0, 0, 0, 0);
					RequestTeleportHandler(i, "0   ");
				}

				if (m_pClientList[i] == 0) break;
				//if ( (m_pMapList[m_pClientList[i]->m_cMapIndex]->m_iUpperLevelLimit != 0) &&
				//	 (m_pClientList[i]->m_iLevel > m_pMapList[m_pClientList[i]->m_cMapIndex]->m_iUpperLevelLimit) ) {
				if ((m_pMapList[m_pClientList[i]->m_cMapIndex]->m_iUpperLevelLimit != 0) &&
					(m_pClientList[i]->m_iLevel > m_pMapList[m_pClientList[i]->m_cMapIndex]->m_iUpperLevelLimit)) {
					SendNotifyMsg(0, i, Notify::ToBeRecalled, 0, 0, 0, 0);
					if (m_pClientList[i]->m_cSide == 1) {
						RequestTeleportHandler(i, "2   ", "aresden", -1, -1);
					}
					else if (m_pClientList[i]->m_cSide == 2) {
						RequestTeleportHandler(i, "2   ", "elvine", -1, -1);
					}
				}

				if (m_pClientList[i] == 0) break;
				if ((strcmp(m_pClientList[i]->m_cLocation, "elvine") != 0) &&
					(strcmp(m_pClientList[i]->m_cLocation, "elvhunter") != 0) &&
					(strcmp(m_pClientList[i]->m_cLocation, "arehunter") != 0) &&
					(strcmp(m_pClientList[i]->m_cLocation, "aresden") != 0) &&
					(m_pClientList[i]->m_iLevel >= 20)) {
					std::snprintf(G_cTxt, sizeof(G_cTxt), "Traveller Hack: (%s) Player: (%s) is a traveller and is greater than level 19.", m_pClientList[i]->m_cIPaddress, m_pClientList[i]->m_cCharName);
					PutHackLogFileList(G_cTxt);
					DeleteClient(i, true, true);
				}

				if (m_pClientList[i] == 0) break;
				if ((m_pMapList[m_pClientList[i]->m_cMapIndex]->m_bIsApocalypseMap) &&
					(m_bIsApocalypseMode == false))
				{
					RequestTeleportHandler(i, "1   ");
				}

				//(m_bIsCrusadeMode )
				/*
				if (m_pClientList[i] == 0) break;
				if (((memcmp(m_pClientList[i]->m_cLocation, "aresden",7) == 0) || (memcmp(m_pClientList[i]->m_cLocation, "elvine",6) == 0)) ) {
					iMapside = iGetMapLocationSide(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cName);
					if (iMapside > 3) iMapside2 = iMapside - 2;
					else iMapside2 = iMapside;

					if ((m_pClientList[i]->m_cSide != iMapside2) && (iMapside != 0)) {
						if ((iMapside <= 2)) {
							if (m_pClientList[i]->m_cSide != 0) {
								m_pClientList[i]->m_dwWarBeginTime = GameClock::GetTimeMS();
								m_pClientList[i]->m_bIsWarLocation = true;
								m_pClientList[i]->m_iTimeLeft_ForceRecall = 1;

								RequestTeleportHandler(i, "1   ");
								SendNotifyMsg(0, i, Notify::ToBeRecalled, 0, 0, 0, 0);
							}
						}
					}
				}*/

				if (m_pClientList[i] == 0) break;
				if (((memcmp(m_pClientList[i]->m_cLocation, "arehunter", 9) == 0) || (memcmp(m_pClientList[i]->m_cLocation, "elvhunter", 9) == 0)) &&
					((strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cName, "2ndmiddle") == 0) || (strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cName, "middleland") == 0))) {
					SendNotifyMsg(0, i, Notify::ToBeRecalled, 0, 0, 0, 0);
					RequestTeleportHandler(i, "1   ");
				}

				if (m_bIsApocalypseMode) {
					if (memcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cName, "abaddon", 7) == 0)
						SendNotifyMsg(0, i, Notify::ApocGateOpen, 167, 169, 0, m_pClientList[i]->m_cMapName);
					else if (memcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cName, "icebound", 8) == 0)
						SendNotifyMsg(0, i, Notify::ApocGateOpen, 89, 31, 0, m_pClientList[i]->m_cMapName);
				}

				if (m_pClientList[i] == 0) break;
				if ((m_bIsApocalypseMode) &&
					(memcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cName, "icebound", 8) == 0) &&
					((m_pClientList[i]->m_sX == 89 && m_pClientList[i]->m_sY == 31) ||
						(m_pClientList[i]->m_sX == 89 && m_pClientList[i]->m_sY == 32) ||
						(m_pClientList[i]->m_sX == 90 && m_pClientList[i]->m_sY == 31) ||
						(m_pClientList[i]->m_sX == 90 && m_pClientList[i]->m_sY == 32))) {
					RequestTeleportHandler(i, "2   ", "druncncity", -1, -1);
				}

				if (m_pClientList[i] == 0) break;
				if ((memcmp(m_pClientList[i]->m_cLocation, "are", 3) == 0) &&
					(strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cName, "elvfarm") == 0)) {
					SendNotifyMsg(0, i, Notify::ToBeRecalled, 0, 0, 0, 0);
					RequestTeleportHandler(i, "0   ");
				}

				if (m_pClientList[i] == 0) break;
				if ((memcmp(m_pClientList[i]->m_cLocation, "elv", 3) == 0) &&
					(strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cName, "arefarm") == 0)) {
					SendNotifyMsg(0, i, Notify::ToBeRecalled, 0, 0, 0, 0);
					RequestTeleportHandler(i, "0   ");
				}

				if (m_pClientList[i] == 0) break;
				if ((strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cName, "middleland") == 0)
					&& (strcmp(m_pClientList[i]->m_cLocation, "NONE") == 0)) {
					SendNotifyMsg(0, i, Notify::ToBeRecalled, 0, 0, 0, 0);
					RequestTeleportHandler(i, "0   ");
				}

				if ((m_pClientList[i]->m_bInRecallImpossibleMap)) {
					m_pClientList[i]->m_iTimeLeft_ForceRecall--;
					if (m_pClientList[i]->m_iTimeLeft_ForceRecall <= 0) {
						m_pClientList[i]->m_iTimeLeft_ForceRecall = 0;
						m_pClientList[i]->m_bInRecallImpossibleMap = false;
						SendNotifyMsg(0, i, Notify::ToBeRecalled, 0, 0, 0, 0);
						RequestTeleportHandler(i, "0   ");
					}
				}

				if (m_pClientList[i] == 0) break;
				m_pClientList[i]->m_iSuperAttackCount++;
				if (m_pClientList[i]->m_iSuperAttackCount > 12) {

					m_pClientList[i]->m_iSuperAttackCount = 0;
					iMaxSuperAttack = (m_pClientList[i]->m_iLevel / 10);
					if (m_pClientList[i]->m_iSuperAttackLeft < iMaxSuperAttack) m_pClientList[i]->m_iSuperAttackLeft++;

					// v1.12
					SendNotifyMsg(0, i, Notify::SuperAttackLeft, 0, 0, 0, 0);
				}

				// v1.42
				m_pClientList[i]->m_iTimeLeft_FirmStaminar--;
				if (m_pClientList[i]->m_iTimeLeft_FirmStaminar < 0) m_pClientList[i]->m_iTimeLeft_FirmStaminar = 0;

				// Crusade
				if (m_pClientList[i] == 0) break;
				if (m_pClientList[i]->m_bIsSendingMapStatus) m_pWarManager->_SendMapStatus(i);

				if (m_pClientList[i]->m_iConstructionPoint > 0) {
					m_pWarManager->CheckCommanderConstructionPoint(i);
				}
			}
		}
	}
}


void CGame::ResponsePlayerDataHandler(char* pData, uint32_t dwSize)
{
	char* cp, cCharName[hb::shared::limits::CharNameLen], cTxt[120];

	std::memset(cCharName, 0, sizeof(cCharName));
	const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(pData, sizeof(hb::net::PacketHeader));
	if (!header) return;
	cp = (char*)(pData + sizeof(hb::net::PacketHeader));

	memcpy(cCharName, cp, hb::shared::limits::CharNameLen);
	cp += hb::shared::limits::CharNameLen;

	for(int i = 1; i < MaxClients; i++)
		if (m_pClientList[i] != 0) {
			if (_strnicmp(m_pClientList[i]->m_cCharName, cCharName, hb::shared::limits::CharNameLen - 1) == 0) {
				switch (header->msg_type) {
				case LogResMsg::Confirm:
					InitPlayerData(i, pData, dwSize);
					break;

				case LogResMsg::Reject:
					std::snprintf(G_cTxt, sizeof(G_cTxt), "(HACK?) Not existing character(%s) data request! Rejected!", m_pClientList[i]->m_cCharName);
					PutLogList(G_cTxt);
					//PutLogFileList(G_cTxt); // v1.4

					DeleteClient(i, false, false);
					break;

				default:
					break;
				}

				return;
			}
		}

	std::snprintf(cTxt, sizeof(cTxt), "(!)Non-existing player data received from Log server: CharName(%s)", cCharName);
	PutLogList(cTxt);
}

bool CGame::LoadPlayerDataFromDb(int iClientH)
{
	if (m_pClientList[iClientH] == 0) return false;

	sqlite3* db = nullptr;
	std::string dbPath;
	if (!EnsureAccountDatabase(m_pClientList[iClientH]->m_cAccountName, &db, dbPath)) {
		return false;
	}

	AccountDbCharacterState state = {};
	if (!LoadCharacterState(db, m_pClientList[iClientH]->m_cCharName, state)) {
		CloseAccountDatabase(db);
		return false;
	}

	std::memset(m_pClientList[iClientH]->m_cProfile, 0, sizeof(m_pClientList[iClientH]->m_cProfile));
	std::snprintf(m_pClientList[iClientH]->m_cProfile, sizeof(m_pClientList[iClientH]->m_cProfile), "%s", state.profile);

	std::memset(m_pClientList[iClientH]->m_cLocation, 0, sizeof(m_pClientList[iClientH]->m_cLocation));
	std::snprintf(m_pClientList[iClientH]->m_cLocation, sizeof(m_pClientList[iClientH]->m_cLocation), "%s", state.location);

	std::memset(m_pClientList[iClientH]->m_cGuildName, 0, sizeof(m_pClientList[iClientH]->m_cGuildName));
	std::snprintf(m_pClientList[iClientH]->m_cGuildName, sizeof(m_pClientList[iClientH]->m_cGuildName), "%s", state.guildName);
	m_pClientList[iClientH]->m_iGuildGUID = state.guildGuid;
	m_pClientList[iClientH]->m_iGuildRank = state.guildRank;

	std::memset(m_pClientList[iClientH]->m_cMapName, 0, sizeof(m_pClientList[iClientH]->m_cMapName));
	std::snprintf(m_pClientList[iClientH]->m_cMapName, sizeof(m_pClientList[iClientH]->m_cMapName), "%s", state.mapName);
	m_pClientList[iClientH]->m_cMapIndex = -1;
	for(int i = 0; i < MaxMaps; i++) {
		if ((m_pMapList[i] != 0) && (memcmp(m_pMapList[i]->m_cName, m_pClientList[iClientH]->m_cMapName, 10) == 0)) {
			m_pClientList[iClientH]->m_cMapIndex = (char)i;
			break;
		}
	}
	if (m_pClientList[iClientH]->m_cMapIndex == -1) {
		std::snprintf(G_cTxt, sizeof(G_cTxt), "(!) Player(%s) tries to enter unknown map : %s", m_pClientList[iClientH]->m_cCharName, m_pClientList[iClientH]->m_cMapName);
		PutLogList(G_cTxt);
		CloseAccountDatabase(db);
		return false;
	}

	m_pClientList[iClientH]->m_sX = (short)state.mapX;
	m_pClientList[iClientH]->m_sY = (short)state.mapY;
	m_pClientList[iClientH]->m_iHP = state.hp;
	m_pClientList[iClientH]->m_iMP = state.mp;
	m_pClientList[iClientH]->m_iSP = state.sp;
	m_pClientList[iClientH]->m_iLevel = state.level;
	m_pClientList[iClientH]->m_iRating = state.rating;
	m_pClientList[iClientH]->m_iStr = state.str;
	m_pClientList[iClientH]->m_iInt = state.intl;
	m_pClientList[iClientH]->m_iVit = state.vit;
	m_pClientList[iClientH]->m_iDex = state.dex;
	m_pClientList[iClientH]->m_iMag = state.mag;
	m_pClientList[iClientH]->m_iCharisma = state.chr;
	m_pClientList[iClientH]->m_iLuck = state.luck;
	m_pClientList[iClientH]->m_iExp = state.exp;
	m_pClientList[iClientH]->m_iLU_Pool = state.luPool;
	m_pClientList[iClientH]->m_iEnemyKillCount = state.enemyKillCount;
	m_pClientList[iClientH]->m_iPKCount = state.pkCount;
	m_pClientList[iClientH]->m_iRewardGold = state.rewardGold;
	m_pClientList[iClientH]->m_iDownSkillIndex = state.downSkillIndex;
	m_pClientList[iClientH]->m_sCharIDnum1 = (short)state.idnum1;
	m_pClientList[iClientH]->m_sCharIDnum2 = (short)state.idnum2;
	m_pClientList[iClientH]->m_sCharIDnum3 = (short)state.idnum3;
	m_pClientList[iClientH]->m_cSex = (char)state.sex;
	m_pClientList[iClientH]->m_cSkin = (char)state.skin;
	m_pClientList[iClientH]->m_cHairStyle = (char)state.hairStyle;
	m_pClientList[iClientH]->m_cHairColor = (char)state.hairColor;
	m_pClientList[iClientH]->m_cUnderwear = (char)state.underwear;
	m_pClientList[iClientH]->m_iHungerStatus = state.hungerStatus;
	m_pClientList[iClientH]->m_iTimeLeft_Rating = state.timeleftRating;
	m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = state.timeleftForceRecall;
	m_pClientList[iClientH]->m_iTimeLeft_FirmStaminar = state.timeleftFirmStaminar;
	m_pClientList[iClientH]->m_iPenaltyBlockYear = state.penaltyBlockYear;
	m_pClientList[iClientH]->m_iPenaltyBlockMonth = state.penaltyBlockMonth;
	m_pClientList[iClientH]->m_iPenaltyBlockDay = state.penaltyBlockDay;
	m_pClientList[iClientH]->m_iQuest = state.questNumber;
	m_pClientList[iClientH]->m_iQuestID = state.questId;
	m_pClientList[iClientH]->m_iCurQuestCount = state.currentQuestCount;
	m_pClientList[iClientH]->m_iQuestRewardType = state.questRewardType;
	m_pClientList[iClientH]->m_iQuestRewardAmount = state.questRewardAmount;
	m_pClientList[iClientH]->m_iContribution = state.contribution;
	m_pClientList[iClientH]->m_iWarContribution = state.warContribution;
	m_pClientList[iClientH]->m_bIsQuestCompleted = (state.questCompleted != 0);
	m_pClientList[iClientH]->m_iSpecialEventID = state.specialEventId;
	m_pClientList[iClientH]->m_iSuperAttackLeft = state.superAttackLeft;
	m_pClientList[iClientH]->m_iFightzoneNumber = state.fightzoneNumber;
	m_pClientList[iClientH]->m_iReserveTime = state.reserveTime;
	m_pClientList[iClientH]->m_iFightZoneTicketNumber = state.fightzoneTicketNumber;
	m_pClientList[iClientH]->m_iSpecialAbilityTime = state.specialAbilityTime;
	std::memset(m_pClientList[iClientH]->m_cLockedMapName, 0, sizeof(m_pClientList[iClientH]->m_cLockedMapName));
	std::snprintf(m_pClientList[iClientH]->m_cLockedMapName, sizeof(m_pClientList[iClientH]->m_cLockedMapName), "%s", state.lockedMapName);
	m_pClientList[iClientH]->m_iLockedMapTime = state.lockedMapTime;
	m_pClientList[iClientH]->m_iCrusadeDuty = state.crusadeJob;
	m_pClientList[iClientH]->m_dwCrusadeGUID = state.crusadeGuid;
	m_pClientList[iClientH]->m_iConstructionPoint = state.constructPoint;
	m_pClientList[iClientH]->m_iDeadPenaltyTime = state.deadPenaltyTime;
	m_pClientList[iClientH]->m_iPartyID = state.partyId;
	m_pClientList[iClientH]->m_iGizonItemUpgradeLeft = state.gizonItemUpgradeLeft;
	m_pClientList[iClientH]->m_appearance = state.appearance;

	for(int i = 0; i < hb::shared::limits::MaxItems; i++) {
		if (m_pClientList[iClientH]->m_pItemList[i] != 0) {
			delete m_pClientList[iClientH]->m_pItemList[i];
			m_pClientList[iClientH]->m_pItemList[i] = 0;
		}
		m_pClientList[iClientH]->m_ItemPosList[i].x = 40;
		m_pClientList[iClientH]->m_ItemPosList[i].y = 30;
		m_pClientList[iClientH]->m_bIsItemEquipped[i] = false;
	}

	for(int i = 0; i < hb::shared::limits::MaxBankItems; i++) {
		if (m_pClientList[iClientH]->m_pItemInBankList[i] != 0) {
			delete m_pClientList[iClientH]->m_pItemInBankList[i];
			m_pClientList[iClientH]->m_pItemInBankList[i] = 0;
		}
	}

	std::vector<AccountDbIndexedValue> positionsX;
	std::vector<AccountDbIndexedValue> positionsY;
	LoadCharacterItemPositions(db, m_pClientList[iClientH]->m_cCharName, positionsX, positionsY);
	for (size_t i = 0; i < positionsX.size(); i++) {
		int slot = positionsX[i].index;
		if (slot >= 0 && slot < hb::shared::limits::MaxItems) {
			m_pClientList[iClientH]->m_ItemPosList[slot].x = positionsX[i].value;
			m_pClientList[iClientH]->m_ItemPosList[slot].y = positionsY[i].value;
		}
	}

	std::vector<AccountDbItemRow> items;
	LoadCharacterItems(db, m_pClientList[iClientH]->m_cCharName, items);
	for (const auto& item : items) {
		if (item.slot < 0 || item.slot >= hb::shared::limits::MaxItems) {
			continue;
		}
		if (m_pClientList[iClientH]->m_pItemList[item.slot] != 0) {
			delete m_pClientList[iClientH]->m_pItemList[item.slot];
		}
		m_pClientList[iClientH]->m_pItemList[item.slot] = new CItem;
		if (m_pItemManager->_bInitItemAttr(m_pClientList[iClientH]->m_pItemList[item.slot], item.itemId) == false) {
			delete m_pClientList[iClientH]->m_pItemList[item.slot];
			m_pClientList[iClientH]->m_pItemList[item.slot] = 0;
			continue;
		}
		m_pClientList[iClientH]->m_pItemList[item.slot]->m_dwCount = item.count;
		m_pClientList[iClientH]->m_pItemList[item.slot]->m_sTouchEffectType = item.touchEffectType;
		m_pClientList[iClientH]->m_pItemList[item.slot]->m_sTouchEffectValue1 = item.touchEffectValue1;
		m_pClientList[iClientH]->m_pItemList[item.slot]->m_sTouchEffectValue2 = item.touchEffectValue2;
		m_pClientList[iClientH]->m_pItemList[item.slot]->m_sTouchEffectValue3 = item.touchEffectValue3;
		m_pClientList[iClientH]->m_pItemList[item.slot]->m_cItemColor = item.itemColor;
		m_pClientList[iClientH]->m_pItemList[item.slot]->m_sItemSpecEffectValue1 = item.specEffectValue1;
		m_pClientList[iClientH]->m_pItemList[item.slot]->m_sItemSpecEffectValue2 = item.specEffectValue2;
		m_pClientList[iClientH]->m_pItemList[item.slot]->m_sItemSpecEffectValue3 = item.specEffectValue3;
		m_pClientList[iClientH]->m_pItemList[item.slot]->m_wCurLifeSpan = (short)item.curLifeSpan;
		m_pClientList[iClientH]->m_pItemList[item.slot]->m_dwAttribute = item.attribute;

		if ((m_pClientList[iClientH]->m_pItemList[item.slot]->m_dwAttribute & 0x00000001) != 0) {
			m_pClientList[iClientH]->m_pItemList[item.slot]->m_wMaxLifeSpan = m_pClientList[iClientH]->m_pItemList[item.slot]->m_sItemSpecEffectValue1;
		}
		m_pItemManager->_AdjustRareItemValue(m_pClientList[iClientH]->m_pItemList[item.slot]);
		if (m_pClientList[iClientH]->m_pItemList[item.slot]->m_wCurLifeSpan > m_pClientList[iClientH]->m_pItemList[item.slot]->m_wMaxLifeSpan) {
			m_pClientList[iClientH]->m_pItemList[item.slot]->m_wCurLifeSpan = m_pClientList[iClientH]->m_pItemList[item.slot]->m_wMaxLifeSpan;
		}
		m_pItemManager->bCheckAndConvertPlusWeaponItem(iClientH, item.slot);
	}

	std::vector<AccountDbBankItemRow> bankItems;
	LoadCharacterBankItems(db, m_pClientList[iClientH]->m_cCharName, bankItems);
	for (const auto& item : bankItems) {
		if (item.slot < 0 || item.slot >= hb::shared::limits::MaxBankItems) {
			continue;
		}
		if (m_pClientList[iClientH]->m_pItemInBankList[item.slot] != 0) {
			delete m_pClientList[iClientH]->m_pItemInBankList[item.slot];
		}
		m_pClientList[iClientH]->m_pItemInBankList[item.slot] = new CItem;
		if (m_pItemManager->_bInitItemAttr(m_pClientList[iClientH]->m_pItemInBankList[item.slot], item.itemId) == false) {
			delete m_pClientList[iClientH]->m_pItemInBankList[item.slot];
			m_pClientList[iClientH]->m_pItemInBankList[item.slot] = 0;
			continue;
		}
		m_pClientList[iClientH]->m_pItemInBankList[item.slot]->m_dwCount = item.count;
		m_pClientList[iClientH]->m_pItemInBankList[item.slot]->m_sTouchEffectType = item.touchEffectType;
		m_pClientList[iClientH]->m_pItemInBankList[item.slot]->m_sTouchEffectValue1 = item.touchEffectValue1;
		m_pClientList[iClientH]->m_pItemInBankList[item.slot]->m_sTouchEffectValue2 = item.touchEffectValue2;
		m_pClientList[iClientH]->m_pItemInBankList[item.slot]->m_sTouchEffectValue3 = item.touchEffectValue3;
		m_pClientList[iClientH]->m_pItemInBankList[item.slot]->m_cItemColor = item.itemColor;
		m_pClientList[iClientH]->m_pItemInBankList[item.slot]->m_sItemSpecEffectValue1 = item.specEffectValue1;
		m_pClientList[iClientH]->m_pItemInBankList[item.slot]->m_sItemSpecEffectValue2 = item.specEffectValue2;
		m_pClientList[iClientH]->m_pItemInBankList[item.slot]->m_sItemSpecEffectValue3 = item.specEffectValue3;
		m_pClientList[iClientH]->m_pItemInBankList[item.slot]->m_wCurLifeSpan = (short)item.curLifeSpan;
		m_pClientList[iClientH]->m_pItemInBankList[item.slot]->m_dwAttribute = item.attribute;
		if ((m_pClientList[iClientH]->m_pItemInBankList[item.slot]->m_dwAttribute & 0x00000001) != 0) {
			m_pClientList[iClientH]->m_pItemInBankList[item.slot]->m_wMaxLifeSpan = m_pClientList[iClientH]->m_pItemInBankList[item.slot]->m_sItemSpecEffectValue1;
		}
		m_pItemManager->_AdjustRareItemValue(m_pClientList[iClientH]->m_pItemInBankList[item.slot]);
		if (m_pClientList[iClientH]->m_pItemInBankList[item.slot]->m_wCurLifeSpan > m_pClientList[iClientH]->m_pItemInBankList[item.slot]->m_wMaxLifeSpan) {
			m_pClientList[iClientH]->m_pItemInBankList[item.slot]->m_wCurLifeSpan = m_pClientList[iClientH]->m_pItemInBankList[item.slot]->m_wMaxLifeSpan;
		}
	}

	std::vector<AccountDbIndexedValue> equips;
	LoadCharacterItemEquips(db, m_pClientList[iClientH]->m_cCharName, equips);
	for (const auto& equip : equips) {
		if (equip.index >= 0 && equip.index < hb::shared::limits::MaxItems) {
			m_pClientList[iClientH]->m_bIsItemEquipped[equip.index] = (equip.value != 0);
		}
	}

	int packedIndex = 0;
	for(int i = 0; i < hb::shared::limits::MaxItems; i++) {
		if (m_pClientList[iClientH]->m_pItemList[i] == 0) {
			continue;
		}
		if (i != packedIndex) {
			m_pClientList[iClientH]->m_pItemList[packedIndex] = m_pClientList[iClientH]->m_pItemList[i];
			m_pClientList[iClientH]->m_pItemList[i] = 0;
			m_pClientList[iClientH]->m_ItemPosList[packedIndex] = m_pClientList[iClientH]->m_ItemPosList[i];
			m_pClientList[iClientH]->m_bIsItemEquipped[packedIndex] = m_pClientList[iClientH]->m_bIsItemEquipped[i];
		}
		packedIndex++;
	}
	for(int i = packedIndex; i < hb::shared::limits::MaxItems; i++) {
		m_pClientList[iClientH]->m_ItemPosList[i].x = 40;
		m_pClientList[iClientH]->m_ItemPosList[i].y = 30;
		m_pClientList[iClientH]->m_bIsItemEquipped[i] = false;
	}

	packedIndex = 0;
	for(int i = 0; i < hb::shared::limits::MaxBankItems; i++) {
		if (m_pClientList[iClientH]->m_pItemInBankList[i] == 0) {
			continue;
		}
		if (i != packedIndex) {
			m_pClientList[iClientH]->m_pItemInBankList[packedIndex] = m_pClientList[iClientH]->m_pItemInBankList[i];
			m_pClientList[iClientH]->m_pItemInBankList[i] = 0;
		}
		packedIndex++;
	}
	for(int i = packedIndex; i < hb::shared::limits::MaxBankItems; i++) {
		m_pClientList[iClientH]->m_pItemInBankList[i] = 0;
	}

	for(int i = 0; i < DEF_MAXITEMEQUIPPOS; i++) {
		m_pClientList[iClientH]->m_sItemEquipmentStatus[i] = -1;
	}

	for(int i = 0; i < hb::shared::limits::MaxItems; i++) {
		if ((m_pClientList[iClientH]->m_pItemList[i] != 0) && m_pClientList[iClientH]->m_bIsItemEquipped[i]) {
			if (m_pClientList[iClientH]->m_pItemList[i]->GetItemType() == ItemType::Equip) {
				if (m_pItemManager->bEquipItemHandler(iClientH, i) == false) {
					m_pClientList[iClientH]->m_bIsItemEquipped[i] = false;
				}
			}
			else {
				m_pClientList[iClientH]->m_bIsItemEquipped[i] = false;
			}
		}
	}

	for(int i = 0; i < hb::shared::limits::MaxMagicType; i++) {
		m_pClientList[iClientH]->m_cMagicMastery[i] = 0;
	}
	std::vector<AccountDbIndexedValue> magicMastery;
	LoadCharacterMagicMastery(db, m_pClientList[iClientH]->m_cCharName, magicMastery);
	for (const auto& entry : magicMastery) {
		if (entry.index >= 0 && entry.index < hb::shared::limits::MaxMagicType) {
			m_pClientList[iClientH]->m_cMagicMastery[entry.index] = (char)entry.value;
		}
	}

	for(int i = 0; i < hb::shared::limits::MaxSkillType; i++) {
		m_pClientList[iClientH]->m_cSkillMastery[i] = 0;
		m_pClientList[iClientH]->m_iSkillSSN[i] = 0;
	}
	std::vector<AccountDbIndexedValue> skillMastery;
	LoadCharacterSkillMastery(db, m_pClientList[iClientH]->m_cCharName, skillMastery);
	for (const auto& entry : skillMastery) {
		if (entry.index >= 0 && entry.index < hb::shared::limits::MaxSkillType) {
			m_pClientList[iClientH]->m_cSkillMastery[entry.index] = (unsigned char)entry.value;
		}
	}

	std::vector<AccountDbIndexedValue> skillSsn;
	LoadCharacterSkillSSN(db, m_pClientList[iClientH]->m_cCharName, skillSsn);
	for (const auto& entry : skillSsn) {
		if (entry.index >= 0 && entry.index < hb::shared::limits::MaxSkillType) {
			m_pClientList[iClientH]->m_iSkillSSN[entry.index] = entry.value;
		}
	}

	short sTmpType = 0;
	if (m_pClientList[iClientH]->m_cSex == 1) {
		sTmpType = 1;
	}
	else if (m_pClientList[iClientH]->m_cSex == 2) {
		sTmpType = 4;
	}
	switch (m_pClientList[iClientH]->m_cSkin) {
	case 1:
		break;
	case 2:
		sTmpType += 1;
		break;
	case 3:
		sTmpType += 2;
		break;
	}
	m_pClientList[iClientH]->m_sType = sTmpType;
	m_pClientList[iClientH]->m_appearance.iHairStyle = m_pClientList[iClientH]->m_cHairStyle;
	m_pClientList[iClientH]->m_appearance.iHairColor = m_pClientList[iClientH]->m_cHairColor;
	m_pClientList[iClientH]->m_appearance.iUnderwearType = m_pClientList[iClientH]->m_cUnderwear;

	if (m_pClientList[iClientH]->m_sCharIDnum1 == 0) {
		int temp1 = 1;
		int temp2 = 1;
		for(int i = 0; i < 10; i++) {
			temp1 += m_pClientList[iClientH]->m_cCharName[i];
			temp2 += abs(m_pClientList[iClientH]->m_cCharName[i] ^ m_pClientList[iClientH]->m_cCharName[i]);
		}
		m_pClientList[iClientH]->m_sCharIDnum1 = (short)GameClock::GetTimeMS();
		m_pClientList[iClientH]->m_sCharIDnum2 = (short)temp1;
		m_pClientList[iClientH]->m_sCharIDnum3 = (short)temp2;
	}

	m_pClientList[iClientH]->m_iSpeedHackCheckExp = m_pClientList[iClientH]->m_iExp;
	if (memcmp(m_pClientList[iClientH]->m_cLocation, "NONE", 4) == 0) {
		m_pClientList[iClientH]->m_bIsNeutral = true;
	}

	// Load block list
	m_pClientList[iClientH]->m_BlockedAccounts.clear();
	m_pClientList[iClientH]->m_BlockedAccountsList.clear();
	m_pClientList[iClientH]->m_bBlockListDirty = false;
	std::vector<std::pair<std::string, std::string>> blocks;
	if (LoadBlockList(db, blocks)) {
		for (const auto& entry : blocks) {
			m_pClientList[iClientH]->m_BlockedAccounts.insert(entry.first);
			m_pClientList[iClientH]->m_BlockedAccountsList.push_back(entry);
		}
	}

	CloseAccountDatabase(db);
	return true;
}

void CGame::InitPlayerData(int iClientH, char* pData, uint32_t dwSize)
{
	char cTxt[256], cQuestRemain;
	int     iRet, iTotalPoints;
	bool    bRet, bGuildStatus;

	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_bIsInitComplete) return;

	// Log Server
	//cp = (char *)(pData + hb::shared::net::MessageOffsetType + 2);

	//std::memset(cName, 0, sizeof(cName));
	//memcpy(cName, cp, hb::shared::limits::CharNameLen - 1);
	//cp += 10;

	////m_pClientList[iClientH]->m_cAccountStatus = *cp;
	//cp++;

	//cGuildStatus = *cp;
	//cp++;

	m_pClientList[iClientH]->m_iHitRatio = 0;
	m_pClientList[iClientH]->m_iDefenseRatio = 0;
	m_pClientList[iClientH]->m_cSide = 0;

	bRet = LoadPlayerDataFromDb(iClientH);
	if (bRet == false) {
		std::snprintf(G_cTxt, sizeof(G_cTxt), "(HACK?) Character(%s) data error!", m_pClientList[iClientH]->m_cCharName);
		DeleteClient(iClientH, false, true);
		return;
	}

	___RestorePlayerCharacteristics(iClientH);

	___RestorePlayerRating(iClientH);

	if ((m_pClientList[iClientH]->m_sX == -1) && (m_pClientList[iClientH]->m_sY == -1)) {
		GetMapInitialPoint(m_pClientList[iClientH]->m_cMapIndex, &m_pClientList[iClientH]->m_sX, &m_pClientList[iClientH]->m_sY, m_pClientList[iClientH]->m_cLocation);
	}

	// New 17/05/2004
	SetPlayingStatus(iClientH);
	// Set faction/identity status fields from player data
	m_pClientList[iClientH]->m_status.bPK = (m_pClientList[iClientH]->m_iPKCount != 0) ? 1 : 0;
	m_pClientList[iClientH]->m_status.bCitizen = (m_pClientList[iClientH]->m_cSide != 0) ? 1 : 0;
	m_pClientList[iClientH]->m_status.bAresden = (m_pClientList[iClientH]->m_cSide == 1) ? 1 : 0;
	m_pClientList[iClientH]->m_status.bHunter = m_pClientList[iClientH]->m_bIsPlayerCivil ? 1 : 0;

	if (m_pClientList[iClientH]->m_iLevel > 100)
		if (m_pClientList[iClientH]->m_bIsPlayerCivil)
			ForceChangePlayMode(iClientH, false);

	m_pClientList[iClientH]->m_iNextLevelExp = m_iLevelExpTable[m_pClientList[iClientH]->m_iLevel + 1]; //iGetLevelExp(m_pClientList[iClientH]->m_iLevel + 1);

	m_pItemManager->CalcTotalItemEffect(iClientH, -1, true); //false
	iCalcTotalWeight(iClientH);

	iTotalPoints = 0;
	for(int i = 0; i < hb::shared::limits::MaxSkillType; i++)
		iTotalPoints += m_pClientList[iClientH]->m_cSkillMastery[i];
	if ((iTotalPoints - 21 > MaxSkillPoints) ) {
		try
		{
			std::snprintf(G_cTxt, sizeof(G_cTxt), "Packet Editing: (%s) Player: (%s) - has more than allowed skill points (%d).", m_pClientList[iClientH]->m_cIPaddress, m_pClientList[iClientH]->m_cCharName, iTotalPoints);
			PutHackLogFileList(G_cTxt);
			DeleteClient(iClientH, true, true);
		}
		catch (...)
		{
		}
		return;
	}

	CheckSpecialEvent(iClientH);
	m_pMagicManager->bCheckMagicInt(iClientH);

	SendNotifyMsg(0, iClientH, Notify::Hunger, m_pClientList[iClientH]->m_iHungerStatus, 0, 0, 0);

	if (strcmp(m_pClientList[iClientH]->m_cGuildName, "NONE") != 0) {
		char cFn[112] = {};
		std::memset(cFn, 0, sizeof(cFn));
		std::snprintf(cFn, sizeof(cFn), "Guilds/AscII%d/%s.txt", m_pClientList[iClientH]->m_cGuildName[0], m_pClientList[iClientH]->m_cGuildName);
		HANDLE  hFile = CreateFile(cFn, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
		auto dwFileSize = GetFileSize(hFile, 0);

		bGuildStatus = !(hFile == INVALID_HANDLE_VALUE);
		CloseHandle(hFile);
		// GuildName
		if ((!bGuildStatus) && (memcmp(m_pClientList[iClientH]->m_cGuildName, "NONE", 4) != 0)) {
			std::memset(m_pClientList[iClientH]->m_cGuildName, 0, sizeof(m_pClientList[iClientH]->m_cGuildName));
			strcpy(m_pClientList[iClientH]->m_cGuildName, "NONE");
			m_pClientList[iClientH]->m_iGuildRank = -1;
			m_pClientList[iClientH]->m_iGuildGUID = -1;

			SendNotifyMsg(0, iClientH, Notify::GuildDisbanded, 0, 0, 0, m_pClientList[iClientH]->m_cGuildName);
		}
	}

	if (m_pClientList[iClientH]->m_iQuest > 0) {
		// Validate quest number to prevent out-of-bounds access
		if(m_pClientList[iClientH]->m_iQuest >= hb::server::config::MaxQuestType || !m_pQuestManager->m_pQuestConfigList[m_pClientList[iClientH]->m_iQuest])
		{
			m_pClientList[iClientH]->m_iQuest = 0;
			m_pClientList[iClientH]->m_iCurQuestCount = 0;
			m_pClientList[iClientH]->m_iQuestRewardAmount = 0;
			m_pClientList[iClientH]->m_iQuestRewardType = 0;
			m_pClientList[iClientH]->m_bIsQuestCompleted = false;
		}
		else {
			cQuestRemain = (m_pQuestManager->m_pQuestConfigList[m_pClientList[iClientH]->m_iQuest]->m_iMaxCount - m_pClientList[iClientH]->m_iCurQuestCount);
			SendNotifyMsg(0, iClientH, Notify::QuestCounter, cQuestRemain, 0, 0, 0);
			m_pQuestManager->_bCheckIsQuestCompleted(iClientH);
		}
	}


	if (m_pClientList[iClientH] == 0) {
		std::snprintf(cTxt, sizeof(cTxt), "<%d> InitPlayerData error - Socket error! Disconnected.", iClientH);
		PutLogList(cTxt);
		return;
	}

	hb::net::PacketResponseInitPlayer pkt{};
	pkt.header.msg_id = MsgId::ResponseInitPlayer;
	pkt.header.msg_type = MsgType::Confirm;

	iRet = m_pClientList[iClientH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	switch (iRet) {
	case sock::Event::QueueFull:
	case sock::Event::SocketError:
	case sock::Event::CriticalError:
	case sock::Event::SocketClosed:
		// ## BUG POINT!!!
		std::snprintf(cTxt, sizeof(cTxt), "<%d> InitPlayerData - Socket error! Disconnected.", iClientH);
		PutLogList(cTxt);

		DeleteClient(iClientH, false, true);
		return;
	}


	m_pClientList[iClientH]->m_bIsInitComplete = true;
}

void CGame::GameProcess()
{
	// MODERNIZED: Socket polling moved to EventLoop (wmain.cpp) for continuous responsiveness
	// This function now handles only game logic processing
	NpcProcess();
	MsgProcess();
	ForceRecallProcess();
	m_pDelayEventManager->DelayEventProcess();
}




// Helper function to normalize item name for comparison (removes spaces and underscores)
static void NormalizeItemName(const char* src, char* dst, size_t dstSize)
{
	size_t j = 0;
	for (size_t i = 0; src[i] && j < dstSize - 1; ++i) {
		if (src[i] != ' ' && src[i] != '_') {
			dst[j++] = src[i];
		}
	}
	dst[j] = '\0';
}



bool CGame::_bGetIsStringIsNumber(char* pStr)
{
	
	for(int i = 0; i < (int)strlen(pStr); i++)
		if ((pStr[i] != '-') && ((pStr[i] < (char)'0') || (pStr[i] > (char)'9'))) return false;

	return true;
}




int CGame::bCreateNewNpc(int iNpcConfigId, char* pName, char* pMapName, short sClass, char cSA, char cMoveType, int* poX, int* poY, char* pWaypointList, hb::shared::geometry::GameRectangle* pArea, int iSpotMobIndex, char cChangeSide, bool bHideGenMode, bool bIsSummoned, bool bFirmBerserk, bool bIsMaster, int iGuildGUID, bool bBypassMobLimit)
{
	if (m_pEntityManager == 0)
		return false;

	return (m_pEntityManager->CreateEntity(
		iNpcConfigId, pName, pMapName, sClass, cSA, cMoveType,
		poX, poY, pWaypointList, pArea, iSpotMobIndex, cChangeSide,
		bHideGenMode, bIsSummoned, bFirmBerserk, bIsMaster, iGuildGUID, bBypassMobLimit) > 0);
}

int CGame::SpawnMapNpcsFromDatabase(sqlite3* db, int iMapIndex)
{
	if (db == nullptr || m_pMapList[iMapIndex] == nullptr)
		return 0;

	const char* sql =
		"SELECT npc_config_id, move_type, waypoint_list, name_prefix"
		" FROM map_npcs WHERE map_name = ? COLLATE NOCASE;";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return 0;
	}

	sqlite3_bind_text(stmt, 1, m_pMapList[iMapIndex]->m_cName, -1, SQLITE_STATIC);

	int npcCount = 0;
	char cNpcWaypointIndex[12];
	char cNamePrefix;
	char cNpcMoveType;
	char cName[8];
	int iNamingValue;

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		// Get NPC config ID directly
		int iNpcConfigId = sqlite3_column_int(stmt, 0);
		if (iNpcConfigId < 0 || iNpcConfigId >= MaxNpcTypes || m_pNpcConfigList[iNpcConfigId] == nullptr) {
			std::snprintf(G_cTxt, sizeof(G_cTxt), "(!) Invalid npc_config_id %d in map_npcs for map %s", iNpcConfigId, m_pMapList[iMapIndex]->m_cName);
			PutLogList(G_cTxt);
			continue;
		}

		// Get move type
		cNpcMoveType = static_cast<char>(sqlite3_column_int(stmt, 1));

		// Get waypoint list (comma-separated string like "0,0,0,0,0,0,0,0,0,0")
		std::memset(cNpcWaypointIndex, 0, sizeof(cNpcWaypointIndex));
		const char* waypointList = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
		if (waypointList != nullptr && strlen(waypointList) > 0) {
			char waypointCopy[64];
			std::snprintf(waypointCopy, sizeof(waypointCopy), "%s", waypointList);

			char* token = strtok(waypointCopy, ",");
			int wpIndex = 0;
			while (token != nullptr && wpIndex < 10) {
				cNpcWaypointIndex[wpIndex] = static_cast<char>(atoi(token));
				token = strtok(nullptr, ",");
				wpIndex++;
			}
		}

		// Get name prefix
		const char* prefix = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
		cNamePrefix = (prefix != nullptr && strlen(prefix) > 0) ? prefix[0] : '_';

		// Get a naming value for this NPC
		iNamingValue = m_pMapList[iMapIndex]->iGetEmptyNamingValue();
		if (iNamingValue == -1) {
			continue;
		}

		// Construct the NPC instance name
		std::memset(cName, 0, sizeof(cName));
		std::snprintf(cName, sizeof(cName), "XX%d", iNamingValue);
		cName[0] = cNamePrefix;
		cName[1] = static_cast<char>(iMapIndex + 65);

		// Spawn the NPC
		if (bCreateNewNpc(iNpcConfigId, cName, m_pMapList[iMapIndex]->m_cName, 0, 0, cNpcMoveType, 0, 0, cNpcWaypointIndex, 0, 0, -1, false) == false) {
			m_pMapList[iMapIndex]->SetNamingValueEmpty(iNamingValue);
		}
		else {
			npcCount++;
		}
	}

	sqlite3_finalize(stmt);

	if (npcCount > 0) {
		char cTxt[128];
		std::snprintf(cTxt, sizeof(cTxt), "  - Spawned %d static NPCs for map: %s", npcCount, m_pMapList[iMapIndex]->m_cName);
		PutLogList(cTxt);
	}

	return npcCount;
}

void CGame::NpcProcess()
{
	if (m_pEntityManager != 0)
		m_pEntityManager->ProcessEntities();
}


void CGame::BroadcastServerMessage(const char* pMessage)
{
	if (pMessage == nullptr || pMessage[0] == '\0')
		return;

	// Build a chat packet: header(6) + x(2) + y(2) + name(10) + chat_type(1) + message + null
	char pkt[256];
	std::memset(pkt, 0, sizeof(pkt));

	size_t msgLen = std::strlen(pMessage);
	if (msgLen > sizeof(pkt) - 22) msgLen = sizeof(pkt) - 22;

	auto& chatPkt = *reinterpret_cast<hb::net::PacketChatMsg*>(pkt);
	chatPkt.header.msg_id = MsgId::CommandChatMsg;
	chatPkt.header.msg_type = 0;
	chatPkt.reserved1 = 0;
	chatPkt.reserved2 = 0;
	std::memcpy(chatPkt.name, "Server", 6);
	chatPkt.chat_type = 10;

	// Message text follows immediately after the chat header
	constexpr size_t chat_header_size = sizeof(hb::net::PacketChatMsg);
	std::memcpy(pkt + chat_header_size, pMessage, msgLen);
	pkt[chat_header_size + msgLen] = '\0';

	uint32_t dwSize = static_cast<uint32_t>(chat_header_size + msgLen + 1);

	for(int i = 1; i < MaxClients; i++)
	{
		if (m_pClientList[i] != nullptr && m_pClientList[i]->m_bIsInitComplete)
			m_pClientList[i]->m_pXSock->iSendMsg(pkt, dwSize);
	}

	ChatLog::Get().Write(10, "Server", "", pMessage);
}

bool CGame::IsBlockedBy(int iSenderH, int iReceiverH) const
{
	if (m_pClientList[iSenderH] == nullptr || m_pClientList[iReceiverH] == nullptr)
		return false;
	return m_pClientList[iReceiverH]->m_BlockedAccounts.count(
		m_pClientList[iSenderH]->m_cAccountName) > 0;
}

// 05/29/2004 - Hypnotoad - GM chat tweak
void CGame::ChatMsgHandler(int iClientH, char* pData, size_t dwMsgSize)
{
	int iRet;
	char* cp;
	char cSendMode = 0;

	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_bIsInitComplete == false) return;
	if (dwMsgSize > 83 + 30) return;

	auto* header = hb::net::PacketCast<hb::net::PacketHeader>(pData, sizeof(hb::net::PacketHeader));
	if (!header) return;
	const auto* pkt = hb::net::PacketCast<hb::net::PacketCommandChatMsgHeader>(pData, sizeof(hb::net::PacketCommandChatMsgHeader));
	if (!pkt) return;
	char* message = pData + sizeof(hb::net::PacketCommandChatMsgHeader);

	if (_strnicmp(pkt->name, m_pClientList[iClientH]->m_cCharName, strlen(m_pClientList[iClientH]->m_cCharName)) != 0) return;

	if (m_pClientList[iClientH]->m_bIsObserverMode) return;

	// v1.432-2
	int iStX, iStY;
	if (m_pMapList[m_pClientList[iClientH]->m_cMapIndex] != 0) {
		iStX = m_pClientList[iClientH]->m_sX / 20;
		iStY = m_pClientList[iClientH]->m_sY / 20;
		m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_stTempSectorInfo[iStX][iStY].iPlayerActivity++;

		switch (m_pClientList[iClientH]->m_cSide) {
		case 0: m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_stTempSectorInfo[iStX][iStY].iNeutralActivity++; break;
		case 1: m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_stTempSectorInfo[iStX][iStY].iAresdenActivity++; break;
		case 2: m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_stTempSectorInfo[iStX][iStY].iElvineActivity++;  break;
		}
	}
	cp = message;

	switch (*cp) {
	case '@':
		*cp = 32;

		if ((m_pClientList[iClientH]->m_iLevel > 1) &&
			(m_pClientList[iClientH]->m_iSP >= 3)) {
			if (m_pClientList[iClientH]->m_iTimeLeft_FirmStaminar == 0) {
				m_pClientList[iClientH]->m_iSP -= 3;
				SendNotifyMsg(0, iClientH, Notify::Sp, 0, 0, 0, 0);
			}
			cSendMode = 1;
		}
		else cSendMode = 0;
		break;

		// New 08/05/2004
		// Party chat
	case '$':
		*cp = 32;

		if (m_pClientList[iClientH]->m_iSP >= 3) {
			if (m_pClientList[iClientH]->m_iTimeLeft_FirmStaminar == 0) {
				m_pClientList[iClientH]->m_iSP -= 3;
				SendNotifyMsg(0, iClientH, Notify::Sp, 0, 0, 0, 0);
			}
			cSendMode = 4;
		}
		else {
			cSendMode = 0;
		}
		break;

	case '^':
		*cp = 32;

		if ((m_pClientList[iClientH]->m_iLevel > 10) &&
			(m_pClientList[iClientH]->m_iSP > 5) && m_pClientList[iClientH]->m_iGuildRank != -1) {
			if (m_pClientList[iClientH]->m_iTimeLeft_FirmStaminar == 0) {
				m_pClientList[iClientH]->m_iSP -= 3;
				SendNotifyMsg(0, iClientH, Notify::Sp, 0, 0, 0, 0);
			}
			cSendMode = 1;
		}
		else cSendMode = 0;

		// v1.4334
		if (m_pClientList[iClientH]->m_iHP < 0) cSendMode = 0;

		break;

	case '!':
		*cp = 32;

		if ((m_pClientList[iClientH]->m_iLevel > 10) &&
			(m_pClientList[iClientH]->m_iSP >= 5)) {
			if (m_pClientList[iClientH]->m_iTimeLeft_FirmStaminar == 0) {
				m_pClientList[iClientH]->m_iSP -= 5;
				SendNotifyMsg(0, iClientH, Notify::Sp, 0, 0, 0, 0);
			}
			cSendMode = 2;
		}
		else cSendMode = 0;

		// v1.4334
		if (m_pClientList[iClientH]->m_iHP <= 0) cSendMode = 0;

		break;

	case '~':
		*cp = 32;
		if ((m_pClientList[iClientH]->m_iLevel > 1) &&
			(m_pClientList[iClientH]->m_iSP >= 3)) {
			if (m_pClientList[iClientH]->m_iTimeLeft_FirmStaminar == 0) {
				m_pClientList[iClientH]->m_iSP -= 3;
				SendNotifyMsg(0, iClientH, Notify::Sp, 0, 0, 0, 0);
			}
			cSendMode = 3;
		}
		else cSendMode = 0;
		// v1.4334
		if (m_pClientList[iClientH]->m_iHP <= 0) cSendMode = 0;
		break;

	case '/':
		if (GameChatCommandManager::Get().ProcessCommand(iClientH, cp, dwMsgSize - sizeof(hb::net::PacketCommandChatMsgHeader)))
			return;
		// Not a recognized command - fall through as normal chat
		break;
	}

	pData[dwMsgSize - 1] = 0;

	if ((m_pClientList[iClientH]->m_cMagicEffectStatus[hb::shared::magic::Confuse] == 1) && (iDice(1, 3) != 2)) {
		// Confuse Language
		cp = message;

		while (*cp != 0) {
			if ((cp[0] != 0) && (cp[0] != ' ') && (cp[1] != 0) && (cp[1] != ' ')) {
				switch (iDice(1, 3)) {
				case 1:	memcpy(cp, "¿ö", 2); break;
				case 2:	memcpy(cp, "¿ì", 2); break;
				case 3:	memcpy(cp, "¿ù", 2); break;
				}
				cp += 2;
			}
			else cp++;
		}
	}

	cp = message;

	if ((cSendMode == 0) && (m_pClientList[iClientH]->m_iWhisperPlayerIndex != -1)) {
		cSendMode = 20;

		if (*cp == '#') cSendMode = 0;
	}

	// Refresh AFK timer for non-whisper chat (whispers don't break AFK)
	if (cSendMode != 20) {
		m_pClientList[iClientH]->m_dwAfkActivityTime = GameClock::GetTimeMS();
	}

	ChatLog::Get().Write(cSendMode, m_pClientList[iClientH]->m_cCharName,
		m_pClientList[iClientH]->m_cMapName, message,
		(cSendMode == 20) ? m_pClientList[iClientH]->m_cWhisperPlayerName : nullptr);

	header->msg_type = (uint16_t)iClientH;
	// Write chat send mode into the packet's chat_type field before relaying to clients
	pData[offsetof(hb::net::PacketCommandChatMsgHeader, chat_type)] = cSendMode;

	if (cSendMode != 20) {
		for(int i = 1; i < MaxClients; i++)
			if (m_pClientList[i] != 0) {
				if (IsBlockedBy(iClientH, i)) continue;
				switch (cSendMode) {
				case 0:
					if (m_pClientList[i]->m_bIsInitComplete == false) break;

					if ((m_pClientList[i]->m_cMapIndex == m_pClientList[iClientH]->m_cMapIndex) &&
						(m_pClientList[i]->m_sX > m_pClientList[iClientH]->m_sX - hb::shared::view::CenterX) &&
						(m_pClientList[i]->m_sX < m_pClientList[iClientH]->m_sX + hb::shared::view::CenterX) &&
						(m_pClientList[i]->m_sY > m_pClientList[iClientH]->m_sY - hb::shared::view::CenterY) &&
						(m_pClientList[i]->m_sY < m_pClientList[iClientH]->m_sY + hb::shared::view::CenterY)) {

						// Crusade
						if (m_bIsCrusadeMode) {
							if ((m_pClientList[iClientH]->m_cSide != 0) && (m_pClientList[i]->m_cSide != 0) &&
								(m_pClientList[i]->m_cSide != m_pClientList[iClientH]->m_cSide)) {
							}
							else iRet = m_pClientList[i]->m_pXSock->iSendMsg(pData, dwMsgSize);
						}
						else iRet = m_pClientList[i]->m_pXSock->iSendMsg(pData, dwMsgSize);
					}
					break;

				case 1:
					if (m_pClientList[i]->m_bIsInitComplete == false) break;

					if ((memcmp(m_pClientList[i]->m_cGuildName, m_pClientList[iClientH]->m_cGuildName, 20) == 0) &&
						(memcmp(m_pClientList[i]->m_cGuildName, "NONE", 4) != 0)) {

						// Crusade
						if (m_bIsCrusadeMode) {
							if ((m_pClientList[iClientH]->m_cSide != 0) && (m_pClientList[i]->m_cSide != 0) &&
								(m_pClientList[i]->m_cSide != m_pClientList[iClientH]->m_cSide)) {
							}
							else iRet = m_pClientList[i]->m_pXSock->iSendMsg(pData, dwMsgSize);
						}
						else iRet = m_pClientList[i]->m_pXSock->iSendMsg(pData, dwMsgSize);
					}
					break;

				case 2:
				case 10:
					// Crusade
					if (m_bIsCrusadeMode) {
						if ((m_pClientList[iClientH]->m_cSide != 0) && (m_pClientList[i]->m_cSide != 0) &&
							(m_pClientList[i]->m_cSide != m_pClientList[iClientH]->m_cSide)) {
						}
						else iRet = m_pClientList[i]->m_pXSock->iSendMsg(pData, dwMsgSize);
					}
					else iRet = m_pClientList[i]->m_pXSock->iSendMsg(pData, dwMsgSize);
					break;

				case 3:
					if (m_pClientList[i]->m_bIsInitComplete == false) break;

					if ((m_pClientList[i]->m_cSide == m_pClientList[iClientH]->m_cSide))
						iRet = m_pClientList[i]->m_pXSock->iSendMsg(pData, dwMsgSize);
					break;

				case 4:
					if (m_pClientList[i]->m_bIsInitComplete == false) break;
					if ((m_pClientList[i]->m_iPartyID != 0) && (m_pClientList[i]->m_iPartyID == m_pClientList[iClientH]->m_iPartyID))
						iRet = m_pClientList[i]->m_pXSock->iSendMsg(pData, dwMsgSize);
					break;
				}

				switch (iRet) {
				case sock::Event::QueueFull:
				case sock::Event::SocketError:
				case sock::Event::CriticalError:
				case sock::Event::SocketClosed:
					//DeleteClient(i, true, true);
					break;
				}
			}
	}
	else {
		// New 16/05/2004
		iRet = m_pClientList[iClientH]->m_pXSock->iSendMsg(pData, dwMsgSize);
		{
			int whisperTarget = m_pClientList[iClientH]->m_iWhisperPlayerIndex;
			if (m_pClientList[whisperTarget] != 0 &&
				_stricmp(m_pClientList[iClientH]->m_cWhisperPlayerName, m_pClientList[whisperTarget]->m_cCharName) == 0) {
				if (!IsBlockedBy(iClientH, whisperTarget)) {
					iRet = m_pClientList[whisperTarget]->m_pXSock->iSendMsg(pData, dwMsgSize);
				}
			}
		}

		switch (iRet) {
		case sock::Event::QueueFull:
		case sock::Event::SocketError:
		case sock::Event::CriticalError:
		case sock::Event::SocketClosed:
			//DeleteClient(i, true, true);
			break;
		}
	}
}


void CGame::ChatMsgHandlerGSM(int iMsgType, int iV1, char* pName, char* pData, size_t dwMsgSize)
{
	int iRet;
	char cTemp[256], cSendMode = 0;

	std::memset(cTemp, 0, sizeof(cTemp));

	auto& chatPkt = *reinterpret_cast<hb::net::PacketChatMsg*>(cTemp);
	chatPkt.header.msg_id = MsgId::CommandChatMsg;
	chatPkt.header.msg_type = 0;
	chatPkt.reserved1 = 0;
	chatPkt.reserved2 = 0;
	std::memcpy(chatPkt.name, pName, sizeof(chatPkt.name));
	chatPkt.chat_type = static_cast<uint8_t>(iMsgType);

	char* cp = cTemp + sizeof(hb::net::PacketChatMsg);
	memcpy(cp, pData, dwMsgSize);
	cp += dwMsgSize;

	switch (iMsgType) {
	case 1:
		for(int i = 1; i < MaxClients; i++)
			if (m_pClientList[i] != 0) {
				if (m_pClientList[i]->m_bIsInitComplete == false) break;
				if ((m_pClientList[i]->m_iGuildGUID == iV1) && (m_pClientList[i]->m_iGuildGUID != 0)) {
					iRet = m_pClientList[i]->m_pXSock->iSendMsg(cTemp, dwMsgSize + 22);
				}
			}
		break;

	case 2:
	case 10:
		for(int i = 1; i < MaxClients; i++)
			if (m_pClientList[i] != 0) {
				iRet = m_pClientList[i]->m_pXSock->iSendMsg(cTemp, dwMsgSize + 22);
			}
		break;
	}
}

//  int CGame::iClientMotion_Attack_Handler(int iClientH, short sX, short sY, short dX, short dY, short wType, char cDir, uint16_t wTargetObjectID, bool bResponse, bool bIsDash)
//  description			:: controls player attack
//	return value		:: int
//  last updated		:: October 29, 2004; 8:06 PM; Hypnotoad
//  commentary			:: - contains attack hack detection
//						   - added checks for Firebow and Directionbow to see if player is m_bIsInsideWarehouse, m_bIsInsideWizardTower, m_bIsInsideOwnTown 
//						   - added ability to attack moving object
//						   - fixed attack unmoving object
// Incomplete: 
//			- Direction Bow damage disabled
int CGame::iClientMotion_Attack_Handler(int iClientH, short sX, short sY, short dX, short dY, short wType, char cDir, uint16_t wTargetObjectID, uint32_t dwClientTime, bool bResponse, bool bIsDash)
{
	uint32_t dwTime, iExp;
	int     iRet, tdX = 0, tdY = 0;
	short   sOwner, sAbsX, sAbsY;
	char    cOwnerType;
	bool    bNearAttack = false, var_AC = false;
	short sItemIndex;
	int tX, tY, iErr, iStX, iStY;

	if (m_pClientList[iClientH] == 0) return 0;
	if ((cDir <= 0) || (cDir > 8))       return 0;
	if (m_pClientList[iClientH]->m_bIsInitComplete == false) return 0;
	if (m_pClientList[iClientH]->m_bIsKilled) return 0;

	dwTime = GameClock::GetTimeMS();
	m_pClientList[iClientH]->m_dwLastActionTime = dwTime;
	m_pClientList[iClientH]->m_iAttackMsgRecvCount++;
	if (m_pClientList[iClientH]->m_iAttackMsgRecvCount >= 7) {
		if (m_pClientList[iClientH]->m_dwAttackLAT != 0) {
			// Compute expected time for 7 consecutive attacks from weapon speed and status.
			// Uses client time to avoid false positives from TCP buffering/network jitter.
			// Must match client-side animation timing (PlayerAnim::Attack: sMaxFrame=7, frames 0-7 = 8 durations @ 78ms base).
			constexpr int ATTACK_FRAME_DURATIONS = 8;
			constexpr int BASE_FRAME_TIME = 78;
			constexpr int RUN_FRAME_TIME = 39;
			constexpr int BATCH_TOLERANCE_MS = 100;

			const auto& status = m_pClientList[iClientH]->m_status;
			int iAttackDelay = status.iAttackDelay;
			bool bHaste = status.bHaste;
			bool bFrozen = status.bFrozen;

			int effectiveFrameTime = BASE_FRAME_TIME + (iAttackDelay * 12);
			if (bFrozen) effectiveFrameTime += BASE_FRAME_TIME >> 2;
			if (bHaste)  effectiveFrameTime -= static_cast<int>(RUN_FRAME_TIME / 2.3);

			int singleSwingTime = ATTACK_FRAME_DURATIONS * effectiveFrameTime;
			int batchThreshold = 7 * singleSwingTime - BATCH_TOLERANCE_MS;
			if (batchThreshold < 2800) batchThreshold = 2800;

			uint32_t dwClientGap = dwClientTime - m_pClientList[iClientH]->m_dwAttackLAT;
			if (dwClientGap < static_cast<uint32_t>(batchThreshold)) {
				std::snprintf(G_cTxt, sizeof(G_cTxt), "Batch Swing Hack: (%s) Player: (%s) - 7 attacks in %ums, Min: %dms",
					m_pClientList[iClientH]->m_cIPaddress, m_pClientList[iClientH]->m_cCharName,
					dwClientGap, batchThreshold);
				PutHackLogFileList(G_cTxt);
				DeleteClient(iClientH, true, true, true);
				return 0;
			}
		}
		m_pClientList[iClientH]->m_dwAttackLAT = dwClientTime;
		m_pClientList[iClientH]->m_iAttackMsgRecvCount = 0;
	}

	if ((wTargetObjectID != 0) && (wType != 2)) {
		if (wTargetObjectID < MaxClients) {
			if (m_pClientList[wTargetObjectID] != 0) {
				tdX = m_pClientList[wTargetObjectID]->m_sX;
				tdY = m_pClientList[wTargetObjectID]->m_sY;
			}
		}
		else if (hb::shared::object_id::IsNpcID(wTargetObjectID) && (hb::shared::object_id::ToNpcIndex(wTargetObjectID) < MaxNpcs)) {
			if (m_pNpcList[hb::shared::object_id::ToNpcIndex(wTargetObjectID)] != 0) {
				tdX = m_pNpcList[hb::shared::object_id::ToNpcIndex(wTargetObjectID)]->m_sX;
				tdY = m_pNpcList[hb::shared::object_id::ToNpcIndex(wTargetObjectID)]->m_sY;
			}
		}

		m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->GetOwner(&sOwner, &cOwnerType, dX, dY);
		if ((sOwner == hb::shared::object_id::ToNpcIndex(wTargetObjectID)) && (m_pNpcList[sOwner] != 0)) {
			tdX = m_pNpcList[sOwner]->m_sX;
			dX = tdX;
			tdY = m_pNpcList[sOwner]->m_sY;
			dY = tdY;
			bNearAttack = false;
			var_AC = true;
		}
		if (var_AC != true) {
			if ((tdX == dX) && (tdY == dY)) {
				bNearAttack = false;
			}
			else if ((abs(tdX - dX) <= 1) && (abs(tdY - dY) <= 1)) {
				dX = tdX;
				dY = tdY;
				bNearAttack = true;
			}
		}
	}

	if ((dX < 0) || (dX >= m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_sSizeX) ||
		(dY < 0) || (dY >= m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_sSizeY)) return 0;

	if ((sX != m_pClientList[iClientH]->m_sX) || (sY != m_pClientList[iClientH]->m_sY)) return 2;

	if (m_pMapList[m_pClientList[iClientH]->m_cMapIndex] != 0) {
		iStX = m_pClientList[iClientH]->m_sX / 20;
		iStY = m_pClientList[iClientH]->m_sY / 20;
		m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_stTempSectorInfo[iStX][iStY].iPlayerActivity++;

		switch (m_pClientList[iClientH]->m_cSide) {
		case 0: m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_stTempSectorInfo[iStX][iStY].iNeutralActivity++; break;
		case 1: m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_stTempSectorInfo[iStX][iStY].iAresdenActivity++; break;
		case 2: m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_stTempSectorInfo[iStX][iStY].iElvineActivity++;  break;
		}
	}

	sAbsX = abs(sX - dX);
	sAbsY = abs(sY - dY);
	if ((wType != 2) && (wType < 20)) {
		if (var_AC == false) {
			sItemIndex = m_pClientList[iClientH]->m_sItemEquipmentStatus[ToInt(EquipPos::TwoHand)];
			if (sItemIndex != -1) {
				if (m_pClientList[iClientH]->m_pItemList[sItemIndex] == 0) return 0;
				if (m_pClientList[iClientH]->m_pItemList[sItemIndex]->m_sIDnum == 845) {
					if ((sAbsX > 4) || (sAbsY > 4)) wType = 0;
				}
				else {
					if ((sAbsX > 1) || (sAbsY > 1)) wType = 0;
				}
			}
			else {
				if ((sAbsX > 1) || (sAbsY > 1)) wType = 0;
			}
		}
		else {
			cDir = CMisc::cGetNextMoveDir(sX, sY, dX, dY);
			if ((m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->bCheckFlySpaceAvailable(
				sX, static_cast<char>(sY), cDir, sOwner)))
				wType = 0;
		}
	}
	else if (wType >= 20) {
		short sMaxRange;
		switch (wType) {
		case 30: sMaxRange = 5; break; // StormBlade critical
		case 22: sMaxRange = 4; break; // Esterk
		case 23: sMaxRange = 3; break; // Long Sword
		case 24: sMaxRange = 2; break; // Axe
		case 26: sMaxRange = 2; break; // Hammer
		case 27: sMaxRange = 2; break; // Wand
		case 25: sMaxRange = 0; break; // Bow - ranged, no melee limit
		default: sMaxRange = 1; break; // Boxing (20), Dagger/SS (21)
		}
		if ((sMaxRange > 0) && ((sAbsX > sMaxRange) || (sAbsY > sMaxRange))) wType = 0;
	}

	m_pSkillManager->ClearSkillUsingStatus(iClientH);
	m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->ClearOwner(0, iClientH, hb::shared::owner_class::Player, sX, sY);
	m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->SetOwner(iClientH, hb::shared::owner_class::Player, sX, sY);

	m_pClientList[iClientH]->m_cDir = cDir;

	iExp = 0;
	m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->GetOwner(&sOwner, &cOwnerType, dX, dY);

	if (sOwner != 0) {
		if ((wType != 0) && ((dwTime - m_pClientList[iClientH]->m_dwRecentAttackTime) > 100)) {
			if ((m_pClientList[iClientH]->m_pIsProcessingAllowed == false) && (m_pClientList[iClientH]->m_bIsInsideWarehouse == false)
				&& (m_pClientList[iClientH]->m_bIsInsideWizardTower == false) && (m_pClientList[iClientH]->m_bIsInsideOwnTown == false)) {

				uint32_t dwType1 = 0, dwType2, dwValue1, dwValue2;
				if (m_pClientList[iClientH]->m_sItemEquipmentStatus[ToInt(EquipPos::RightHand)] != -1) {
					sItemIndex = m_pClientList[iClientH]->m_sItemEquipmentStatus[ToInt(EquipPos::RightHand)];
				}
				else if (m_pClientList[iClientH]->m_sItemEquipmentStatus[ToInt(EquipPos::TwoHand)] != -1) {
					sItemIndex = m_pClientList[iClientH]->m_sItemEquipmentStatus[ToInt(EquipPos::TwoHand)];
				}
				else sItemIndex = -1;

				if (sItemIndex != -1 && m_pClientList[iClientH]->m_pItemList[sItemIndex] != 0) {
					if ((m_pClientList[iClientH]->m_pItemList[sItemIndex]->m_dwAttribute & 0x00F00000) != 0) {
						dwType1 = (m_pClientList[iClientH]->m_pItemList[sItemIndex]->m_dwAttribute & 0x00F00000) >> 20;
						dwValue1 = (m_pClientList[iClientH]->m_pItemList[sItemIndex]->m_dwAttribute & 0x000F0000) >> 16;
						dwType2 = (m_pClientList[iClientH]->m_pItemList[sItemIndex]->m_dwAttribute & 0x0000F000) >> 12;
						dwValue2 = (m_pClientList[iClientH]->m_pItemList[sItemIndex]->m_dwAttribute & 0x00000F00) >> 8;
					}

					if (dwType1 == 2) {
						// Centuu - fix for poison
						switch (cOwnerType) {
						case hb::shared::owner_class::Player:
							if (!m_pClientList[sOwner]->m_bIsPoisoned && !m_pCombatManager->bCheckResistingPoisonSuccess(sOwner, cOwnerType))
							{
								m_pClientList[sOwner]->m_bIsPoisoned = true;
								m_pClientList[sOwner]->m_iPoisonLevel = dwValue1 * 5;
								m_pClientList[sOwner]->m_dwPoisonTime = dwTime;
								m_pStatusEffectManager->SetPoisonFlag(sOwner, cOwnerType, true);
								SendNotifyMsg(0, sOwner, Notify::MagicEffectOn, hb::shared::magic::Poison, m_pClientList[sOwner]->m_iPoisonLevel, 0, 0);
							}
							break;
						case hb::shared::owner_class::Npc:
							break;
						}
					}
				}

				sItemIndex = m_pClientList[iClientH]->m_sItemEquipmentStatus[ToInt(EquipPos::TwoHand)];
				if (sItemIndex != -1 && m_pClientList[iClientH]->m_pItemList[sItemIndex] != 0) {
					if (m_pClientList[iClientH]->m_pItemList[sItemIndex]->m_sIDnum == 874) { // Directional bow
						for(int i = 2; i < 10; i++) {
							iErr = 0;
							CMisc::GetPoint2(sX, sY, dX, dY, &tX, &tY, &iErr, i);
							m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->GetOwner(&sOwner, &cOwnerType, tX, tY);
							//iExp += m_pCombatManager->iCalculateAttackEffect(sOwner, cOwnerType, iClientH, hb::shared::owner_class::Player, tX, tY, wType, bNearAttack, bIsDash, true); // 1
							if ((abs(tdX - dX) <= 1) && (abs(tdY - dY) <= 1)) {
								m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->GetOwner(&sOwner, &cOwnerType, dX, dY);
								//iExp += m_pCombatManager->iCalculateAttackEffect(sOwner, cOwnerType, iClientH, hb::shared::owner_class::Player, dX, dY, wType, bNearAttack, bIsDash, false); // 0
							}
						}
					}
					else if (m_pClientList[iClientH]->m_pItemList[sItemIndex]->m_sIDnum == 873) { // Firebow
						if (m_pClientList[iClientH]->m_appearance.bIsWalking) {
							if (m_bHeldenianInitiated != 1) {
								m_pDynamicObjectManager->iAddDynamicObjectList(iClientH, hb::shared::owner_class::PlayerIndirect, dynamic_object::Fire3, m_pClientList[iClientH]->m_cMapIndex, dX, dY, (iDice(1, 7) + 3) * 1000, 8);
							}
							iExp += m_pCombatManager->iCalculateAttackEffect(sOwner, cOwnerType, iClientH, hb::shared::owner_class::Player, dX, dY, wType, bNearAttack, bIsDash, false);
						}
					}
					else {
						iExp += m_pCombatManager->iCalculateAttackEffect(sOwner, cOwnerType, iClientH, hb::shared::owner_class::Player, dX, dY, wType, bNearAttack, bIsDash, false);
					}
				}
				else {
					iExp += m_pCombatManager->iCalculateAttackEffect(sOwner, cOwnerType, iClientH, hb::shared::owner_class::Player, dX, dY, wType, bNearAttack, bIsDash, false);
				}
			}
			else {
				iExp += m_pCombatManager->iCalculateAttackEffect(sOwner, cOwnerType, iClientH, hb::shared::owner_class::Player, dX, dY, wType, bNearAttack, bIsDash, false);
			}
			if (m_pClientList[iClientH] == 0) return 0;
			m_pClientList[iClientH]->m_dwRecentAttackTime = dwTime;
		}
	}
	else m_pMiningManager->_CheckMiningAction(iClientH, dX, dY);

	if (iExp != 0) {
		GetExp(iClientH, iExp, true);
	}

	if (bResponse) {
		hb::net::PacketResponseMotionHeader pkt{};
		pkt.header.msg_id = MsgId::ResponseMotion;
		pkt.header.msg_type = Confirm::MotionAttackConfirm;
		iRet = m_pClientList[iClientH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		switch (iRet) {
		case sock::Event::QueueFull:
		case sock::Event::SocketError:
		case sock::Event::CriticalError:
		case sock::Event::SocketClosed:
			DeleteClient(iClientH, true, true);
			return 0;
		}
	}

	return 1;
}

char CGame::cGetNextMoveDir(short sX, short sY, short dstX, short dstY, char cMapIndex, char cTurn, int* pError)
{
	char  cDir, cTmpDir;
	int   aX, aY, dX, dY;
	int   iResX, iResY;

	if ((sX == dstX) && (sY == dstY)) return 0;

	dX = sX;
	dY = sY;

	if ((abs(dX - dstX) <= 1) && (abs(dY - dstY) <= 1)) {
		iResX = dstX;
		iResY = dstY;
	}
	else CMisc::GetPoint(dX, dY, dstX, dstY, &iResX, &iResY, pError);

	cDir = CMisc::cGetNextMoveDir(dX, dY, iResX, iResY);

	if (cTurn == 0)
		for(int i = cDir; i <= cDir + 7; i++) {
			cTmpDir = i;
			if (cTmpDir > 8) cTmpDir -= 8;
			aX = _tmp_cTmpDirX[cTmpDir];
			aY = _tmp_cTmpDirY[cTmpDir];
			if (m_pMapList[cMapIndex]->bGetMoveable(dX + aX, dY + aY)) return cTmpDir;
		}

	if (cTurn == 1)
		for(int i = cDir; i >= cDir - 7; i--) {
			cTmpDir = i;
			if (cTmpDir < 1) cTmpDir += 8;
			aX = _tmp_cTmpDirX[cTmpDir];
			aY = _tmp_cTmpDirY[cTmpDir];
			if (m_pMapList[cMapIndex]->bGetMoveable(dX + aX, dY + aY)) return cTmpDir;
		}

	return 0;
}


char _tmp_cEmptyPosX[] = { 0, 1, 1, 0, -1, -1, -1, 0 ,1, 2, 2, 2, 2, 1, 0, -1, -2, -2, -2, -2, -2, -1, 0, 1, 2 };
char _tmp_cEmptyPosY[] = { 0, 0, 1, 1, 1, 0, -1, -1, -1, -1, 0, 1, 2, 2, 2, 2, 2, 1, 0, -1, -2, -2, -2, -2, -2 };

bool CGame::bGetEmptyPosition(short* pX, short* pY, char cMapIndex)
{
	short sX, sY;

	for(int i = 0; i < 25; i++)
		if ((m_pMapList[cMapIndex]->bGetMoveable(*pX + _tmp_cEmptyPosX[i], *pY + _tmp_cEmptyPosY[i])) &&
			(m_pMapList[cMapIndex]->bGetIsTeleport(*pX + _tmp_cEmptyPosX[i], *pY + _tmp_cEmptyPosY[i]) == false)) {
			sX = *pX + _tmp_cEmptyPosX[i];
			sY = *pY + _tmp_cEmptyPosY[i];
			*pX = sX;
			*pY = sY;
			return true;
		}

	GetMapInitialPoint(cMapIndex, &sX, &sY);
	*pX = sX;
	*pY = sY;

	return false;
}




void CGame::MsgProcess()
{
	char* pData, cFrom, cKey;
	size_t    dwMsgSize;
	int      iClientH;
	uint32_t dwTime = GameClock::GetTimeMS();

	if ((m_bF5pressed) && (m_bF1pressed)) {
		PutLogList("(XXX) RELOADING CONFIGS MANUALY...");
		for(int i = 1; i < MaxClients; i++)
			if ((m_pClientList[i] != 0) && (m_pClientList[i]->m_bIsInitComplete)) {
				g_login->LocalSavePlayerData(i); //bSendMsgToLS(ServerMsgId::RequestSavePlayerData, i);
			}
		bInit();
	}

	if ((m_bF1pressed) && (m_bF4pressed) && (m_bOnExitProcess == false)) {
		m_cShutDownCode = 2;
		m_bOnExitProcess = true;
		m_dwExitProcessTime = GameClock::GetTimeMS();
		PutLogList("(!) GAME SERVER SHUTDOWN PROCESS BEGIN(by Local command)!!!");
		//bSendMsgToLS(ServerMsgId::GameServerShutdowned, 0);


		return;
	}

	std::memset(m_pMsgBuffer, 0, hb::shared::limits::MsgBufferSize + 1);
	pData = (char*)m_pMsgBuffer;

	m_iCurMsgs = 0;
	while (bGetMsgQuene(&cFrom, pData, &dwMsgSize, &iClientH, &cKey)) {

		//v1.31
		m_iCurMsgs++;
		if (m_iCurMsgs > m_iMaxMsgs) m_iMaxMsgs = m_iCurMsgs;

		switch (cFrom) {
		case Source::Client: {
			if (m_pClientList[iClientH] == nullptr) break;

			// Update activity tracking (async reads bypass OnClientSocketEvent)
			m_pClientList[iClientH]->m_dwTime = dwTime;

			const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(
				pData, sizeof(hb::net::PacketHeader));
			if (!header) break;

			m_pClientList[iClientH]->m_dwLastMsgId = header->msg_id;
			m_pClientList[iClientH]->m_dwLastMsgTime = dwTime;
			m_pClientList[iClientH]->m_dwLastMsgSize = dwMsgSize;

			// Update AFK activity timer for all messages except keepalive and chat
			// (chat is handled separately in ChatMsgHandler to exclude whispers)
			if (header->msg_id != MsgId::CommandCheckConnection && header->msg_id != MsgId::CommandChatMsg) {
				m_pClientList[iClientH]->m_dwAfkActivityTime = dwTime;
			}

			switch (header->msg_id) {

			case MsgId::RequestAngel: // Angels by Snoopy...
				GetAngelHandler(iClientH, pData, dwMsgSize);
				break;

			case MsgId::RequestResurrectYes:
				RequestResurrectPlayer(iClientH, true);
				break;

			case MsgId::RequestResurrectNo:
				RequestResurrectPlayer(iClientH, false);
				break;

			case MsgId::RequestSellItemList:
				m_pItemManager->RequestSellItemListHandler(iClientH, pData);
				break;

			case MsgId::RequestRestart:
				RequestRestartHandler(iClientH);
				break;

			case MsgId::RequestPanning:
				iRequestPanningMapDataRequest(iClientH, pData);
				break;

			case MsgId::RequestNoticement:
				//RequestNoticementHandler(iClientH, pData);
				break;

			case MsgId::RequestSetItemPos:
				m_pItemManager->_SetItemPos(iClientH, pData);
				break;

			case MsgId::RequestFullObjectData:
				RequestFullObjectData(iClientH, pData);
				break;

			case MsgId::RequestRetrieveItem:
				m_pItemManager->RequestRetrieveItemHandler(iClientH, pData);
				break;

			case MsgId::RequestCivilRight:
				RequestCivilRightHandler(iClientH, pData);
				break;

			case MsgId::RequestTeleport:
				RequestTeleportHandler(iClientH, pData);
				break;

			case MsgId::RequestInitPlayer:
				RequestInitPlayerHandler(iClientH, pData, cKey);
				break;

			case MsgId::RequestInitData:
				if (m_pClientList[iClientH] == nullptr) break;
				if (m_pClientList[iClientH]->m_bIsClientConnected) {
					std::snprintf(G_cTxt, sizeof(G_cTxt), "(!!!) Client (%s) connection closed!. Sniffer suspect!.", m_pClientList[iClientH]->m_cCharName);
					PutLogList(G_cTxt);					m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->ClearOwner(2, iClientH, hb::shared::owner_class::Player, m_pClientList[iClientH]->m_sX, m_pClientList[iClientH]->m_sY);
					m_pDelayEventManager->bRemoveFromDelayEventList(iClientH, hb::shared::owner_class::Player, 0);
					g_login->LocalSavePlayerData(iClientH); //bSendMsgToLS(ServerMsgId::RequestSavePlayerDataLogout, iClientH, false);
					if ((dwTime - m_dwGameTime2) > 3000) { // 3 segs
						m_pClientList[iClientH]->m_bIsClientConnected = false;
						DeleteClient(iClientH, true, true, true, true);
					}
					break;
				}
				else {
					m_pClientList[iClientH]->m_bIsClientConnected = true;
					RequestInitDataHandler(iClientH, pData, cKey, dwMsgSize);
				}
				break;

			case MsgId::CommandCommon:
				ClientCommonHandler(iClientH, pData);
				break;

			case MsgId::CommandMotion:
				ClientMotionHandler(iClientH, pData);
				break;

			case MsgId::CommandCheckConnection:
				// Ping response already sent from I/O thread for accurate latency.
				// Still run handler for speedhack detection (skip duplicate send).
				CheckConnectionHandler(iClientH, pData, true);
				break;

			case MsgId::CommandChatMsg:
				ChatMsgHandler(iClientH, pData, dwMsgSize);
				break;

			case MsgId::RequestCreateNewGuild:
				m_pGuildManager->RequestCreateNewGuildHandler(iClientH, pData, dwMsgSize);
				break;

			case MsgId::RequestDisbandGuild:
				m_pGuildManager->RequestDisbandGuildHandler(iClientH, pData, dwMsgSize);
				break;

			case MsgId::RequestFightZoneReserve:
				m_pWarManager->FightzoneReserveHandler(iClientH, pData, dwMsgSize);
				break;

			case MsgId::LevelUpSettings:
				LevelUpSettingsHandler(iClientH, pData, dwMsgSize);
				break;

			case MsgId::StateChangePoint:
				StateChangeHandler(iClientH, pData, dwMsgSize);
				break;

			case ServerMsgId::RequestHeldenianTeleport:
				m_pWarManager->RequestHeldenianTeleport(iClientH, pData, dwMsgSize);
				break;


			case ServerMsgId::RequestCityHallTeleport:
				if (memcmp(m_pClientList[iClientH]->m_cLocation, "aresden", 7) == 0) {
					RequestTeleportHandler(iClientH, "2   ", "dglv2", 263, 258);
				}
				else if (memcmp(m_pClientList[iClientH]->m_cLocation, "elvine", 6) == 0) {
					RequestTeleportHandler(iClientH, "2   ", "dglv2", 209, 258);
				}
				break;

			case MSGID_REQUEST_SHOP_CONTENTS:
				RequestShopContentsHandler(iClientH, pData);
				break;

			case MsgId::RequestConfigData:
			{
				const auto* reqPkt = hb::net::PacketCast<hb::net::PacketRequestConfigData>(
					pData, sizeof(hb::net::PacketRequestConfigData));
				if (!reqPkt) break;
				if (dwTime - m_pClientList[iClientH]->m_dwLastConfigRequestTime < 30000) break;
				m_pClientList[iClientH]->m_dwLastConfigRequestTime = dwTime;
				if (reqPkt->requestItems)  m_pItemManager->bSendClientItemConfigs(iClientH);
				if (reqPkt->requestMagic)  m_pMagicManager->bSendClientMagicConfigs(iClientH);
				if (reqPkt->requestSkills) m_pSkillManager->bSendClientSkillConfigs(iClientH);
				if (reqPkt->requestNpcs)   bSendClientNpcConfigs(iClientH);
			}
			break;

			default:
				if (m_pClientList[iClientH] != 0)  // Snoopy: Anti-crash check !
				{
					std::snprintf(G_cTxt, sizeof(G_cTxt), "Unknown message received: (0x%.8X) PC(%s) - (Delayed). \tIP(%s)"
						, header->msg_id
						, m_pClientList[iClientH]->m_cCharName
						, m_pClientList[iClientH]->m_cIPaddress);
					//DelayedDeleteClient(iClientH, true, true, true, true);
				}
				else
				{
					std::snprintf(G_cTxt, sizeof(G_cTxt), "Unknown message received: (0x%.8X) PC(unknown).", header->msg_id);
				}
				PutLogList(G_cTxt);
				PutHackLogFileList(G_cTxt);
				PutHackLogFileList(m_pMsgBuffer);
				break;
			}
			break;
		}

		case Source::LogServer: {
			const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(
				pData, sizeof(hb::net::PacketHeader));
			if (!header) break;

			switch (header->msg_id) {
			case MsgId::RequestCreateNewAccount:
				g_login->CreateNewAccount(iClientH, pData);
				break;
			case MsgId::RequestLogin:
				g_login->RequestLogin(iClientH, pData);
				break;
			case MsgId::RequestCreateNewCharacter: //message from client
				g_login->ResponseCharacter(iClientH, pData);
				break;
			case MsgId::RequestDeleteCharacter:
				g_login->DeleteCharacter(iClientH, pData);
				break;
			case MsgId::RequestChangePassword:
				g_login->ChangePassword(iClientH, pData);
				break;
			case MsgId::RequestEnterGame:
				g_login->RequestEnterGame(iClientH, pData);
				break;
			default:
				std::snprintf(G_cTxt, sizeof(G_cTxt), "Unknown login message received! (0x%.8X) Delete Client", header->msg_id);
				PutLogList(G_cTxt);
				break;
			}
			DeleteLoginClient(iClientH);
		}
								  break;
		}

	}
}

bool CGame::bPutMsgQuene(char cFrom, char* pData, size_t dwMsgSize, int iIndex, char cKey)
{
	return m_msgQueue.Push(cFrom, pData, dwMsgSize, iIndex, cKey);
}


bool CGame::bGetMsgQuene(char* pFrom, char* pData, size_t* pMsgSize, int* pIndex, char* pKey)
{
	return m_msgQueue.Pop(pFrom, pData, pMsgSize, pIndex, pKey);
}


void CGame::ClientCommonHandler(int iClientH, char* pData)
{
	uint16_t wCommand;
	short sX, sY;
	int iV1, iV2, iV3, iV4;
	char cDir;
	const char* pString;

	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_bIsInitComplete == false) return;
	if (m_pClientList[iClientH]->m_bIsKilled) return;

	const auto* req = hb::net::PacketCast<hb::net::PacketCommandCommonWithString>(
		pData, sizeof(hb::net::PacketCommandCommonWithString));
	if (!req) return;
	wCommand = req->base.header.msg_type;
	sX = req->base.x;
	sY = req->base.y;
	cDir = static_cast<char>(req->base.dir);
	iV1 = req->v1;
	iV2 = req->v2;
	iV3 = req->v3;
	pString = req->text;
	iV4 = req->v4;
	switch (wCommand) {

		//50Cent - Repair All
	case CommonType::ReqRepairAll:
		m_pItemManager->RequestRepairAllItemsHandler(iClientH);
		break;
	case CommonType::ReqRepairAllDelete:
		m_pItemManager->RequestRepairAllItemsDeleteHandler(iClientH, iV1);
		break;
	case CommonType::ReqRepairAllConfirm:
		m_pItemManager->RequestRepairAllItemsConfirmHandler(iClientH);
		break;

		// Crafting
	case CommonType::CraftItem:
		m_pCraftingManager->ReqCreateCraftingHandler(iClientH, pData);
		break;

		// New 15/05/2004
	case CommonType::ReqCreateSlate:
		m_pItemManager->ReqCreateSlateHandler(iClientH, pData);
		break;

		// 2.06 - by KLKS
	case CommonType::RequestHuntMode:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> MsgId::RequestCivilRight");
		RequestChangePlayMode(iClientH);
		break;

	case CommonType::SetGuildTeleportLoc:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::SetGuildTeleportLoc");
		m_pWarManager->RequestSetGuildTeleportLocHandler(iClientH, iV1, iV2, m_pClientList[iClientH]->m_iGuildGUID, "middleland");
		break;

	case CommonType::SetGuildConstructLoc:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::SetGuildConstructLoc");
		m_pWarManager->RequestSetGuildConstructLocHandler(iClientH, iV1, iV2, m_pClientList[iClientH]->m_iGuildGUID, pString);
		break;

	case CommonType::GuildTeleport:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::GuildTeleport");
		m_pWarManager->RequestGuildTeleportHandler(iClientH);
		break;

	case CommonType::SummonWarUnit:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::SummonWarUnit");
		m_pWarManager->RequestSummonWarUnitHandler(iClientH, sX, sY, iV1, iV2, iV3);
		break;

	case CommonType::RequestHelp:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::RequestHelp");
		RequestHelpHandler(iClientH);
		break;

	case CommonType::RequestMapStatus:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::RequestMapStatus");
		m_pWarManager->MapStatusHandler(iClientH, iV1, pString);
		break;

	case CommonType::RequestSelectCrusadeDuty:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::RequestSelectCrusadeDuty");
		m_pWarManager->SelectCrusadeDutyHandler(iClientH, iV1);
		break;

	case CommonType::RequestCancelQuest:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::RequestCancelQuest");
		m_pQuestManager->CancelQuestHandler(iClientH);
		break;

	case CommonType::RequestActivateSpecAbility:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::RequestActivateSpecAbility");
		ActivateSpecialAbilityHandler(iClientH);
		break;

	case CommonType::RequestJoinParty:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::RequestJoinParty");
		JoinPartyHandler(iClientH, iV1, pString);
		break;

	case CommonType::GetMagicAbility:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::GetMagicAbility");
		m_pMagicManager->GetMagicAbilityHandler(iClientH);
		break;

	case CommonType::BuildItem:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::BuildItem");
		m_pItemManager->BuildItemHandler(iClientH, pData);
		break;

	case CommonType::QuestAccepted:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::QuestAccepted");
		m_pQuestManager->QuestAcceptedHandler(iClientH);
		break;

	case CommonType::CancelExchangeItem:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::CancelExchangeItem");
		m_pItemManager->CancelExchangeItem(iClientH);
		break;

	case CommonType::ConfirmExchangeItem:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::ConfirmExchangeItem");
		m_pItemManager->ConfirmExchangeItem(iClientH);
		break;

	case CommonType::SetExchangeItem:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::SetExchangeItem");
		m_pItemManager->SetExchangeItem(iClientH, iV1, iV2);
		break;

	case CommonType::ReqGetHeroMantle:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::ReqGetHeroMantle");
		m_pItemManager->GetHeroMantleHandler(iClientH, iV1, pString);
		break;

	case CommonType::ReqGetOccupyFlag:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::ReqGetOccupyFlag");
		m_pWarManager->GetOccupyFlagHandler(iClientH);
		break;

	case CommonType::ReqSetDownSkillIndex:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::ReqSetDownSkillIndex");
		m_pSkillManager->SetDownSkillIndexHandler(iClientH, iV1);
		break;

	case CommonType::TalkToNpc:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::TalkToNpc");
		// works for client, but for npc it returns middleland
		// if ((m_pMapList[m_pNpcList[iV1]->m_cMapIndex]->m_cName) != (m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cName)) break;
		m_pQuestManager->NpcTalkHandler(iClientH, iV1);
		break;

	case CommonType::ReqCreatePortion:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::ReqCreatePortion");
		m_pCraftingManager->ReqCreatePortionHandler(iClientH, pData);
		break;

	case CommonType::ReqGetFishThisTime:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::ReqGetFishThisTime");
		m_pFishingManager->ReqGetFishThisTimeHandler(iClientH);
		break;

	case CommonType::ReqRepairItemConfirm:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::ReqRepairItemConfirm");
		m_pItemManager->ReqRepairItemCofirmHandler(iClientH, iV1, pString);
		break;

	case CommonType::ReqRepairItem:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::ReqRepairItem");
		m_pItemManager->ReqRepairItemHandler(iClientH, iV1, iV2, pString);
		break;

	case CommonType::ReqSellItemConfirm:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::ReqSellItemConfirm");
		m_pItemManager->ReqSellItemConfirmHandler(iClientH, iV1, iV2, pString);
		break;

	case CommonType::ReqSellItem:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::ReqSellItem");
		m_pItemManager->ReqSellItemHandler(iClientH, iV1, iV2, iV3, pString);
		break;

	case CommonType::ReqUseSkill:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::ReqUseSkill");
		m_pSkillManager->UseSkillHandler(iClientH, iV1, iV2, iV3);
		break;

	case CommonType::ReqUseItem:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::ReqUseItem");
		m_pItemManager->UseItemHandler(iClientH, iV1, iV2, iV3, iV4);
		break;

	case CommonType::ReqGetRewardMoney:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::ReqGetRewardMoney");
		m_pLootManager->GetRewardMoneyHandler(iClientH);
		break;

	case CommonType::ItemDrop:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::ItemDrop");
		m_pItemManager->DropItemHandler(iClientH, iV1, iV2, pString, true);
		break;

	case CommonType::EquipItem:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::EquipItem");
		m_pItemManager->bEquipItemHandler(iClientH, iV1);
		break;

	case CommonType::ReqPurchaseItem:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::ReqPurchaseItem");
		m_pItemManager->RequestPurchaseItemHandler(iClientH, pString, iV1, iV2);
		break;

	case CommonType::ReqStudyMagic:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::ReqStudyMagic");
		m_pMagicManager->RequestStudyMagicHandler(iClientH, pString);
		break;

	case CommonType::ReqTrainSkill:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::ReqTrainSkill");
		//RequestTrainSkillHandler(iClientH, pString);
		break;

	case CommonType::GiveItemToChar:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::GiveItemToChar");
		m_pItemManager->GiveItemHandler(iClientH, cDir, iV1, iV2, iV3, iV4, pString);
		break;

	case CommonType::ExchangeItemToChar:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::ExchangeItemToChar");
		m_pItemManager->ExchangeItemHandler(iClientH, cDir, iV1, iV2, iV3, iV4, pString);
		break;

	case CommonType::JoinGuildApprove:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::JoinGuildApprove");
		m_pGuildManager->JoinGuildApproveHandler(iClientH, pString);
		break;

	case CommonType::JoinGuildReject:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::JoinGuildReject");
		m_pGuildManager->JoinGuildRejectHandler(iClientH, pString);
		break;

	case CommonType::DismissGuildApprove:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::DismissGuildApprove");
		m_pGuildManager->DismissGuildApproveHandler(iClientH, pString);
		break;

	case CommonType::DismissGuildReject:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::DismissGuildReject");
		m_pGuildManager->DismissGuildRejectHandler(iClientH, pString);
		break;

	case CommonType::ReleaseItem:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::ReleaseItem");
		m_pItemManager->ReleaseItemHandler(iClientH, iV1, true);
		break;

	case CommonType::ToggleCombatMode:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::ToggleCombatMode");
		ToggleCombatModeHandler(iClientH);
		break;

	case CommonType::Magic:
	{
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::Magic");
		// Parse as PacketCommandCommonWithTime to get target object ID from time_ms field
		const auto* magicReq = hb::net::PacketCast<hb::net::PacketCommandCommonWithTime>(
			pData, sizeof(hb::net::PacketCommandCommonWithTime));
		uint16_t targetObjectID = magicReq ? static_cast<uint16_t>(magicReq->time_ms) : 0;
		m_pMagicManager->PlayerMagicHandler(iClientH, iV1, iV2, (iV3 - 100), false, 0, targetObjectID);
	}
	break;

	case CommonType::ToggleSafeAttackMode:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::ToggleSafeAttackMode");
		ToggleSafeAttackModeHandler(iClientH);
		break;

	case CommonType::ReqGetOccupyFightZoneTicket:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::ReqGetOccupyFightZoneTicket");
		m_pWarManager->GetFightzoneTicketHandler(iClientH);
		break;

		// Upgrade Item
	case CommonType::UpgradeItem:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::UpgradeItem");
		m_pItemManager->RequestItemUpgradeHandler(iClientH, iV1);
		break;

	case CommonType::ReqGuildName:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::ReqGuildName");
		m_pGuildManager->RequestGuildNameHandler(iClientH, iV1, iV2);
		break;

	case CommonType::RequestAcceptJoinParty:
		//DbgWnd->AddEventMsg("RECV -> Source::Client -> MsgId::CommandCommon -> CommonType::RequestAcceptJoinParty");
		RequestAcceptJoinPartyHandler(iClientH, iV1);
		break;

	default:
		std::snprintf(G_cTxt, sizeof(G_cTxt), "Unknown message received! (0x%.8X)", wCommand);
		PutLogList(G_cTxt);
		break;
	}
}

// New 07/05/2004

//  int CGame::iClientMotion_GetItem_Handler(int iClientH, short sX, short sY, char cDir)
//  description			:: check if player is dropping item or picking up item
//  last updated		:: October 29, 2004; 7:12 PM; Hypnotoad
//	return value		:: int




void CGame::SendEventToNearClient_TypeB(uint32_t dwMsgID, uint16_t wMsgType, char cMapIndex, short sX, short sY, short sV1, short sV2, short sV3, short sV4)
{
	int iRet, iShortCutIndex;
	bool bFlag;

	// OPTIMIZATION FIX #2: Early exit if no clients online
	if (m_iClientShortCut[0] == 0) return;

	// OPTIMIZATION FIX #2: Pre-check if any clients are in range before building packet
	bool bHasNearbyClients = false;
	iShortCutIndex = 0;
	while (m_iClientShortCut[iShortCutIndex] != 0) {
		int i = m_iClientShortCut[iShortCutIndex];
		if ((m_pClientList[i] != 0) &&
			(m_pClientList[i]->m_cMapIndex == cMapIndex) &&
			(m_pClientList[i]->m_sX >= sX - hb::shared::view::CenterX) &&
			(m_pClientList[i]->m_sX <= sX + hb::shared::view::CenterX) &&
			(m_pClientList[i]->m_sY >= sY - (hb::shared::view::CenterY + 1)) &&
			(m_pClientList[i]->m_sY <= sY + (hb::shared::view::CenterY + 1))) {
			bHasNearbyClients = true;
			break;
		}
		iShortCutIndex++;
	}

	// TEMPORARILY DISABLED FOR TESTING - Early exit if no clients in range
	if (false && !bHasNearbyClients) return;

	hb::net::PacketEventNearTypeBShort pkt{};
	pkt.header.msg_id = dwMsgID;
	pkt.header.msg_type = wMsgType;
	pkt.x = sX;
	pkt.y = sY;
	pkt.v1 = sV1;
	pkt.v2 = sV2;
	pkt.v3 = sV3;
	pkt.v4 = sV4;

	//for(int i = 1; i < MaxClients; i++)
	bFlag = true;
	iShortCutIndex = 0;
	while (bFlag) {
		// MaxClients 
		int i = m_iClientShortCut[iShortCutIndex];
		iShortCutIndex++;
		if (i == 0) bFlag = false;

		if ((bFlag) && (m_pClientList[i] != 0)) {
			if ((m_pClientList[i]->m_cMapIndex == cMapIndex) &&
				(m_pClientList[i]->m_sX >= sX - hb::shared::view::CenterX) &&
				(m_pClientList[i]->m_sX <= sX + hb::shared::view::CenterX) &&
				(m_pClientList[i]->m_sY >= sY - (hb::shared::view::CenterY + 1)) &&
				(m_pClientList[i]->m_sY <= sY + (hb::shared::view::CenterY + 1))) {

				iRet = m_pClientList[i]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
			}
		}
	}
}

void CGame::SendEventToNearClient_TypeB(uint32_t dwMsgID, uint16_t wMsgType, char cMapIndex, short sX, short sY, short sV1, short sV2, short sV3, uint32_t dwV4)
{
	int iRet, iShortCutIndex;
	bool bFlag;

	hb::net::PacketEventNearTypeBDword pkt{};
	pkt.header.msg_id = dwMsgID;
	pkt.header.msg_type = wMsgType;
	pkt.x = sX;
	pkt.y = sY;
	pkt.v1 = sV1;
	pkt.v2 = sV2;
	pkt.v3 = sV3;
	pkt.v4 = dwV4;

	//for(int i = 1; i < MaxClients; i++)
	bFlag = true;
	iShortCutIndex = 0;
	while (bFlag) {
		// MaxClients 
		int i = m_iClientShortCut[iShortCutIndex];
		iShortCutIndex++;
		if (i == 0) bFlag = false;

		if ((bFlag) && (m_pClientList[i] != 0)) {
			if ((m_pClientList[i]->m_cMapIndex == cMapIndex) &&
				(m_pClientList[i]->m_sX >= sX - hb::shared::view::CenterX) &&
				(m_pClientList[i]->m_sX <= sX + hb::shared::view::CenterX) &&
				(m_pClientList[i]->m_sY >= sY - (hb::shared::view::CenterY + 1)) &&
				(m_pClientList[i]->m_sY <= sY + (hb::shared::view::CenterY + 1))) {

				iRet = m_pClientList[i]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
			}
		}
	}
}

//  int CGame::iClientMotion_Stop_Handler(int iClientH, short sX, short sY, char cDir)
//  description			:: checks if player is stopped
//  last updated		:: October 29, 2004; 6:46 PM; Hypnotoad
//	return value		:: int
int CGame::iClientMotion_Stop_Handler(int iClientH, short sX, short sY, char cDir)
{
	int     iRet;
	short   sOwnerH;
	char    cOwnerType;

	if (m_pClientList[iClientH] == 0) return 0;
	if ((cDir <= 0) || (cDir > 8))       return 0;
	if (m_pClientList[iClientH]->m_bIsKilled) return 0;
	if (m_pClientList[iClientH]->m_bIsInitComplete == false) return 0;

	if ((sX != m_pClientList[iClientH]->m_sX) || (sY != m_pClientList[iClientH]->m_sY)) return 2;

	if (m_pClientList[iClientH]->m_bSkillUsingStatus[19]) {
		m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->GetOwner(&sOwnerH, &cOwnerType, sX, sY);
		if (sOwnerH != 0) {
			DeleteClient(iClientH, true, true);
			return 0;
		}
	}

	m_pSkillManager->ClearSkillUsingStatus(iClientH);

	int iStX, iStY;
	if (m_pMapList[m_pClientList[iClientH]->m_cMapIndex] != 0) {
		iStX = m_pClientList[iClientH]->m_sX / 20;
		iStY = m_pClientList[iClientH]->m_sY / 20;
		m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_stTempSectorInfo[iStX][iStY].iPlayerActivity++;

		switch (m_pClientList[iClientH]->m_cSide) {
		case 0: m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_stTempSectorInfo[iStX][iStY].iNeutralActivity++; break;
		case 1: m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_stTempSectorInfo[iStX][iStY].iAresdenActivity++; break;
		case 2: m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_stTempSectorInfo[iStX][iStY].iElvineActivity++;  break;
		}
	}

	m_pClientList[iClientH]->m_cDir = cDir;

	m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->ClearOwner(0, iClientH, hb::shared::owner_class::Player, sX, sY);
	m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->SetOwner(iClientH, hb::shared::owner_class::Player, sX, sY);

	{
		hb::net::PacketResponseMotionHeader pkt{};
		pkt.header.msg_id = MsgId::ResponseMotion;
		pkt.header.msg_type = Confirm::MotionConfirm;
		iRet = m_pClientList[iClientH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	switch (iRet) {
	case sock::Event::QueueFull:
	case sock::Event::SocketError:
	case sock::Event::CriticalError:
	case sock::Event::SocketClosed:
		DeleteClient(iClientH, true, true);
		return 0;
	}

	return 1;
}


// 05/29/2004 - Hypnotoad - Purchase Dicount updated to take charisma into consideration


void CGame::SendNotifyMsg(int iFromH, int iToH, uint16_t wMsgType, uint32_t sV1, uint32_t sV2, uint32_t sV3, char* pString, uint32_t sV4, uint32_t sV5, uint32_t sV6, uint32_t sV7, uint32_t sV8, uint32_t sV9, char* pString2)
{
	int iRet = 0;

	if (m_pClientList[iToH] == 0) return;

	// !!! sV1, sV2, sV3 DWORD .
	switch (wMsgType) {
	case Notify::CurLifeSpan:
	{
		hb::net::PacketNotifyCurLifeSpan pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.item_index = static_cast<int32_t>(sV1);
		pkt.cur_lifespan = static_cast<int32_t>(sV2);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}
	case Notify::HeldenianCount:
	{
		hb::net::PacketNotifyHeldenianCount pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.aresden_tower_left = static_cast<int16_t>(sV1);
		pkt.elvine_tower_left = static_cast<int16_t>(sV2);
		pkt.aresden_flags = static_cast<int16_t>(sV3);
		pkt.elvine_flags = static_cast<int16_t>(sV4);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::NoMoreAgriculture:
	case Notify::AgricultureSkillLimit:
	case Notify::AgricultureNoArea:
	{
		hb::net::PacketNotifyEmpty pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	// New 18/05/2004
	case Notify::SpawnEvent:
	{
		hb::net::PacketNotifySpawnEvent pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.monster_id = static_cast<uint8_t>(sV3);
		pkt.x = static_cast<int16_t>(sV1);
		pkt.y = static_cast<int16_t>(sV2);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::QuestCounter:
	{
		hb::net::PacketNotifyQuestCounter pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.current_count = static_cast<int32_t>(sV1);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::ApocGateClose:
	case Notify::ApocGateOpen:
	{
		hb::net::PacketNotifyApocGateOpen pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.gate_x = static_cast<int32_t>(sV1);
		pkt.gate_y = static_cast<int32_t>(sV2);
		if (pString != 0) {
			memcpy(pkt.map_name, pString, sizeof(pkt.map_name));
		}
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::AbaddonKilled:
	{
		hb::net::PacketNotifyAbaddonKilled pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		memcpy(pkt.killer_name, m_pClientList[iFromH]->m_cCharName, sizeof(pkt.killer_name));
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::ApocForceRecallPlayers:
	case Notify::ApocGateStartMsg:
	case Notify::ApocGateEndMsg:
	case Notify::NoRecall:
	{
		hb::net::PacketNotifyEmpty pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::ForceRecallTime:
	{
		hb::net::PacketNotifyForceRecallTime pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.seconds_left = static_cast<uint16_t>(sV1);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	// New 16/05/2004
	//0xB4E2, 0xBEB
	case Notify::MonsterCount:
	case Notify::SlateStatus:
		if (wMsgType == Notify::MonsterCount) {
			hb::net::PacketNotifyMonsterCount pkt{};
			pkt.header.msg_id = MsgId::Notify;
			pkt.header.msg_type = wMsgType;
			pkt.count = static_cast<int16_t>(sV1);
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		}
		else {
			hb::net::PacketNotifySimpleShort pkt{};
			pkt.header.msg_id = MsgId::Notify;
			pkt.header.msg_type = wMsgType;
			pkt.value = static_cast<int16_t>(sV1);
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		}
		break;

		//0x0BE5, 0x0BE7, 0x0BE8, 0x0BEA
	case Notify::Unknown0BE8:
	case Notify::HeldenianTeleport:
	case Notify::HeldenianEnd:
	case Notify::ResurrectPlayer:
	case Notify::SlateExp:
	case Notify::SlateMana:
	case Notify::SlateInvincible:
	{
		hb::net::PacketNotifyEmpty pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::SlateCreateFail:
	{
		hb::net::PacketNotifyEmpty pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::SlateCreateSuccess:
	{
		hb::net::PacketNotifySimpleInt pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.value = static_cast<int32_t>(sV1);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	// New 07/05/2004
	// Party Notify Msg's
	case Notify::Party:
		switch (sV1) {
		case 4:
		case 6:
		{
			hb::net::PacketNotifyPartyName pkt{};
			pkt.header.msg_id = MsgId::Notify;
			pkt.header.msg_type = wMsgType;
			pkt.type = static_cast<int16_t>(sV1);
			pkt.v2 = static_cast<int16_t>(sV2);
			pkt.v3 = static_cast<int16_t>(sV3);
			if (pString != 0) {
				memcpy(pkt.name, pString, sizeof(pkt.name));
			}
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
			break;
		}
		case 5:
		{
			hb::net::PacketWriter writer;
			writer.Reserve(sizeof(hb::net::PacketNotifyPartyList) + (sV3 * 11));

			auto* pkt = writer.Append<hb::net::PacketNotifyPartyList>();
			pkt->header.msg_id = MsgId::Notify;
			pkt->header.msg_type = wMsgType;
			pkt->type = static_cast<int16_t>(sV1);
			pkt->v2 = static_cast<int16_t>(sV2);
			pkt->count = static_cast<int16_t>(sV3);

			if (pString != 0 && sV3 > 0) {
				writer.AppendBytes(pString, sV3 * 11);
			}

			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(writer.Data(), static_cast<int>(writer.Size()));
			break;
		}
		default:
		{
			hb::net::PacketNotifyPartyBasic pkt{};
			pkt.header.msg_id = MsgId::Notify;
			pkt.header.msg_type = wMsgType;
			pkt.type = static_cast<int16_t>(sV1);
			pkt.v2 = static_cast<int16_t>(sV2);
			pkt.v3 = static_cast<int16_t>(sV3);
			pkt.v4 = static_cast<int16_t>(sV4);
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
			break;
		}
		}
		break;

	case Notify::ReqGuildNameAnswer:
	{
		hb::net::PacketNotifyReqGuildNameAnswer pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.guild_rank = static_cast<int16_t>(sV1);
		pkt.index = static_cast<int16_t>(sV2);
		if (pString != 0) {
			memcpy(pkt.guild_name, pString, sizeof(pkt.guild_name));
		}
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	// New 06/05/2004
	// Upgrade Notify Msg's
	case Notify::ItemUpgradeFail:
	{
		hb::net::PacketNotifyItemUpgradeFail pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.reason = static_cast<int16_t>(sV1);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::ItemAttributeChange:
	case Notify::GizonItemUpgradeLeft:
	{
		hb::net::PacketNotifyItemAttributeChange pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.item_index = static_cast<int16_t>(sV1);
		pkt.attribute = sV2;
		pkt.spec_value1 = sV3;
		pkt.spec_value2 = sV4;
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::GizoneItemChange:
	{
		hb::net::PacketNotifyGizonItemChange pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.item_index = static_cast<int16_t>(sV1);
		pkt.item_type = static_cast<uint8_t>(sV2);
		pkt.cur_lifespan = static_cast<int16_t>(sV3);
		pkt.sprite = static_cast<int16_t>(sV4);
		pkt.sprite_frame = static_cast<int16_t>(sV5);
		pkt.item_color = static_cast<uint8_t>(sV6);
		pkt.spec_value2 = static_cast<uint8_t>(sV7);
		pkt.attribute = sV8;
		pkt.item_id = static_cast<int16_t>(sV9);
		if (pString != 0) {
			memcpy(pkt.item_name, pString, sizeof(pkt.item_name));
		}
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	// 2.06 - by KLKS
	case Notify::ChangePlayMode:
	{
		hb::net::PacketNotifyChangePlayMode pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		if (pString != 0) {
			memcpy(pkt.location, pString, sizeof(pkt.location));
		}
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::TcLoc:
	{
		hb::net::PacketNotifyTCLoc pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.dest_x = static_cast<int16_t>(sV1);
		pkt.dest_y = static_cast<int16_t>(sV2);
		if (pString != 0) {
			memcpy(pkt.teleport_map, pString, sizeof(pkt.teleport_map));
		}
		pkt.construct_x = static_cast<int16_t>(sV4);
		pkt.construct_y = static_cast<int16_t>(sV5);
		if (pString2 != 0) {
			memcpy(pkt.construct_map, pString2, sizeof(pkt.construct_map));
		}
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	//New 11/05/2004
	case Notify::GrandMagicResult:
	{
		hb::net::PacketWriter writer;
		writer.Reserve(sizeof(hb::net::PacketNotifyGrandMagicResultHeader) + (sV9 * 2));

		auto* pkt = writer.Append<hb::net::PacketNotifyGrandMagicResultHeader>();
		pkt->header.msg_id = MsgId::Notify;
		pkt->header.msg_type = wMsgType;
		pkt->crashed_structures = static_cast<uint16_t>(sV1);
		pkt->structure_damage = static_cast<uint16_t>(sV2);
		pkt->casualities = static_cast<uint16_t>(sV3);
		if (pString != 0) {
			memcpy(pkt->map_name, pString, sizeof(pkt->map_name));
		}
		pkt->active_structure = static_cast<uint16_t>(sV4);
		pkt->value_count = static_cast<uint16_t>(sV9);

		if (sV9 > 0 && pString2 != 0) {
			writer.AppendBytes(pString2 + 2, sV9 * 2);
		}

		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(writer.Data(), static_cast<int>(writer.Size()));
	}
	break;

	case Notify::MapStatusNext:
	{
		hb::net::PacketWriter writer;
		writer.Reserve(sizeof(hb::net::PacketHeader) + sV1);

		auto* pkt = writer.Append<hb::net::PacketHeader>();
		pkt->msg_id = MsgId::Notify;
		pkt->msg_type = wMsgType;

		if (pString != 0 && sV1 > 0) {
			writer.AppendBytes(pString, sV1);
		}

		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(writer.Data(), static_cast<int>(writer.Size()));
	}
	break;

	case Notify::MapStatusLast:
	{
		hb::net::PacketWriter writer;
		writer.Reserve(sizeof(hb::net::PacketHeader) + sV1);

		auto* pkt = writer.Append<hb::net::PacketHeader>();
		pkt->msg_id = MsgId::Notify;
		pkt->msg_type = wMsgType;

		if (pString != 0 && sV1 > 0) {
			writer.AppendBytes(pString, sV1);
		}

		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(writer.Data(), static_cast<int>(writer.Size()));
	}
	break;

	case Notify::LockedMap:
	{
		hb::net::PacketNotifyLockedMap pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.seconds_left = static_cast<int16_t>(sV1);
		if (pString != 0) {
			memcpy(pkt.map_name, pString, sizeof(pkt.map_name));
		}
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::BuildItemSuccess:
	case Notify::BuildItemFail:
	{
		hb::net::PacketNotifyBuildItemResult pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		if (sV1 >= 0) {
			pkt.item_id = static_cast<int16_t>(sV1);
		}
		else {
			pkt.item_id = static_cast<int16_t>(sV1 + 10000);
		}
		pkt.item_count = static_cast<int16_t>(sV2);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::Help:
	case Notify::QuestReward:
	{
		hb::net::PacketNotifyQuestReward pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.who = static_cast<int16_t>(sV1);
		pkt.flag = static_cast<int16_t>(sV2);
		pkt.amount = static_cast<int32_t>(sV3);
		if (pString != 0) {
			memcpy(pkt.reward_name, pString, sizeof(pkt.reward_name));
		}
		pkt.contribution = static_cast<int32_t>(sV4);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::CannotConstruct:
	{
		hb::net::PacketNotifyCannotConstruct pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.reason = static_cast<int16_t>(sV1);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}
	case Notify::MeteorStrikeComing:
	{
		hb::net::PacketNotifyMeteorStrikeComing pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.phase = static_cast<int16_t>(sV1);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}
	case Notify::ObserverMode:
	{
		hb::net::PacketNotifyObserverMode pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.enabled = static_cast<int16_t>(sV1);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}
	case Notify::SpellInterrupted:
	{
		hb::net::PacketNotifyEmpty pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}
	case Notify::MeteorStrikeHit:
	case Notify::HelpFailed:
	case Notify::SpecialAbilityEnabled:
	case Notify::ForceDisconn:
	case Notify::QuestCompleted:
	case Notify::QuestAborted:
		if (wMsgType == Notify::ForceDisconn) {
			hb::net::PacketNotifyForceDisconn pkt{};
			pkt.header.msg_id = MsgId::Notify;
			pkt.header.msg_type = wMsgType;
			pkt.seconds = static_cast<uint16_t>(sV1);
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		}
		else {
			hb::net::PacketNotifySimpleShort pkt{};
			pkt.header.msg_id = MsgId::Notify;
			pkt.header.msg_type = wMsgType;
			pkt.value = static_cast<int16_t>(sV1);
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		}
		break;

	case Notify::QuestContents:
	{
		hb::net::PacketNotifyQuestContents pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.who = static_cast<int16_t>(sV1);
		pkt.quest_type = static_cast<int16_t>(sV2);
		pkt.contribution = static_cast<int16_t>(sV3);
		pkt.target_type = static_cast<int16_t>(sV4);
		pkt.target_count = static_cast<int16_t>(sV5);
		pkt.x = static_cast<int16_t>(sV6);
		pkt.y = static_cast<int16_t>(sV7);
		pkt.range = static_cast<int16_t>(sV8);
		pkt.is_completed = static_cast<int16_t>(sV9);
		if (pString2 != 0) {
			memcpy(pkt.target_name, pString2, sizeof(pkt.target_name));
		}
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::EnergySphereCreated:
	{
		hb::net::PacketNotifyEnergySphereCreated pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.x = static_cast<int16_t>(sV1);
		pkt.y = static_cast<int16_t>(sV2);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;
	case Notify::ItemColorChange:
	{
		hb::net::PacketNotifyItemColorChange pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.item_index = static_cast<int16_t>(sV1);
		pkt.item_color = static_cast<int16_t>(sV2);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::NoMoreCrusadeStructure:
	case Notify::ExchangeItemComplete:
	case Notify::CancelExchangeItem:
	{
		hb::net::PacketNotifyEmpty pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::SetExchangeItem:
	{
		hb::net::PacketNotifyExchangeItem pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.dir = static_cast<int16_t>(sV1);
		pkt.sprite = static_cast<int16_t>(sV2);
		pkt.sprite_frame = static_cast<int16_t>(sV3);
		pkt.amount = static_cast<int32_t>(sV4);
		pkt.color = static_cast<uint8_t>(sV5);
		pkt.cur_life = static_cast<int16_t>(sV6);
		pkt.max_life = static_cast<int16_t>(sV7);
		pkt.performance = static_cast<int16_t>(sV8);
		if (pString != 0) {
			memcpy(pkt.item_name, pString, sizeof(pkt.item_name));
		}
		memcpy(pkt.char_name, m_pClientList[iFromH]->m_cCharName, sizeof(pkt.char_name));
		pkt.attribute = static_cast<uint32_t>(sV9);
		pkt.item_id = static_cast<int16_t>(reinterpret_cast<intptr_t>(pString2));
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::OpenExchangeWindow:
	{
		hb::net::PacketNotifyExchangeItem pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.dir = static_cast<int16_t>(sV1);
		pkt.sprite = static_cast<int16_t>(sV2);
		pkt.sprite_frame = static_cast<int16_t>(sV3);
		pkt.amount = static_cast<int32_t>(sV4);
		pkt.color = static_cast<uint8_t>(sV5);
		pkt.cur_life = static_cast<int16_t>(sV6);
		pkt.max_life = static_cast<int16_t>(sV7);
		pkt.performance = static_cast<int16_t>(sV8);
		if (pString != 0) {
			memcpy(pkt.item_name, pString, sizeof(pkt.item_name));
		}
		memcpy(pkt.char_name, m_pClientList[iFromH]->m_cCharName, sizeof(pkt.char_name));
		pkt.attribute = static_cast<uint32_t>(sV9);
		pkt.item_id = static_cast<int16_t>(reinterpret_cast<intptr_t>(pString2));
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::NotFlagSpot:
	{
		hb::net::PacketNotifyEmpty pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::ItemPosList:
	{
		hb::net::PacketNotifyItemPosList pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		for(int i = 0; i < hb::shared::limits::MaxItems; i++) {
			pkt.positions[i * 2] = static_cast<int16_t>(m_pClientList[iToH]->m_ItemPosList[i].x);
			pkt.positions[i * 2 + 1] = static_cast<int16_t>(m_pClientList[iToH]->m_ItemPosList[i].y);
		}
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::EnemyKills:
	{
		hb::net::PacketNotifyEnemyKills pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.count = static_cast<int32_t>(sV1);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::Crusade:
	{
		hb::net::PacketNotifyCrusade pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.crusade_mode = static_cast<int32_t>(sV1);
		pkt.crusade_duty = static_cast<int32_t>(sV2);
		pkt.v3 = static_cast<int32_t>(sV3);
		pkt.v4 = static_cast<int32_t>(sV4);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::ConstructionPoint:
	{
		hb::net::PacketNotifyConstructionPoint pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.construction_point = static_cast<int16_t>(sV1);
		pkt.war_contribution = static_cast<int16_t>(sV2);
		pkt.notify_type = static_cast<int16_t>(sV3);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::SpecialAbilityStatus:
	{
		hb::net::PacketNotifySpecialAbilityStatus pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.status_type = static_cast<int16_t>(sV1);
		pkt.ability_type = static_cast<int16_t>(sV2);
		pkt.seconds_left = static_cast<int16_t>(sV3);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::DamageMove:
	{
		hb::net::PacketNotifyDamageMove pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.dir = static_cast<uint8_t>(sV1);
		pkt.amount = static_cast<int16_t>(sV2);
		pkt.weapon = static_cast<uint8_t>(sV3);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::DownSkillIndexSet:
	case Notify::ResponseCreateNewParty:
		if (wMsgType == Notify::ResponseCreateNewParty) {
			hb::net::PacketNotifyResponseCreateNewParty pkt{};
			pkt.header.msg_id = MsgId::Notify;
			pkt.header.msg_type = wMsgType;
			pkt.result = static_cast<int16_t>(sV1);
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		}
		else {
			hb::net::PacketNotifyDownSkillIndexSet pkt{};
			pkt.header.msg_id = MsgId::Notify;
			pkt.header.msg_type = wMsgType;
			pkt.skill_index = static_cast<uint16_t>(sV1);
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		}
		break;

	case Notify::HeldenianStart:
	case Notify::NpcTalk:
	{
		hb::net::PacketNotifyNpcTalk pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.type = static_cast<int16_t>(sV1);
		pkt.response = static_cast<int16_t>(sV2);
		pkt.amount = static_cast<int16_t>(sV3);
		pkt.contribution = static_cast<int16_t>(sV4);
		pkt.target_type = static_cast<int16_t>(sV5);
		pkt.target_count = static_cast<int16_t>(sV6);
		pkt.x = static_cast<int16_t>(sV7);
		pkt.y = static_cast<int16_t>(sV8);
		pkt.range = static_cast<int16_t>(sV9);
		if (pString != 0) {
			memcpy(pkt.reward_name, pString, sizeof(pkt.reward_name));
		}
		if (pString2 != 0) {
			memcpy(pkt.target_name, pString2, sizeof(pkt.target_name));
		}
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	// Crafting
	case Notify::CraftingFail:		//reversed by Snoopy: 0x0BF1:
	{
		hb::net::PacketNotifyCraftingFail pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.reason = static_cast<int32_t>(sV1);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::CraftingSuccess:		//reversed by Snoopy: 0x0BF0
	case Notify::PortionSuccess:
	case Notify::LowPortionSkill:
	case Notify::PortionFail:
	case Notify::NoMatchingPortion:
	{
		hb::net::PacketNotifyEmpty pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::SuperAttackLeft:
	{
		hb::net::PacketNotifySuperAttackLeft pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.left = static_cast<int16_t>(m_pClientList[iToH]->m_iSuperAttackLeft);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::SafeAttackMode:
	{
		hb::net::PacketNotifySafeAttackMode pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.enabled = m_pClientList[iToH]->m_bIsSafeAttackMode ? 1 : 0;
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::QueryJoinParty:
	case Notify::IpAccountInfo:
		if (wMsgType == Notify::QueryJoinParty) {
			hb::net::PacketWriter writer;
			writer.Reserve(sizeof(hb::net::PacketHeader) + 11);

			auto* pkt = writer.Append<hb::net::PacketHeader>();
			pkt->msg_id = MsgId::Notify;
			pkt->msg_type = wMsgType;

			std::size_t name_len = 0;
			if (pString != 0) {
				name_len = std::strlen(pString);
				if (name_len > 10) {
					name_len = 10;
				}
				writer.AppendBytes(pString, name_len);
			}
			writer.AppendBytes("", 1);
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(writer.Data(), static_cast<int>(writer.Size()));
		}
		else {
			hb::net::PacketWriter writer;
			writer.Reserve(sizeof(hb::net::PacketHeader) + 510);

			auto* pkt = writer.Append<hb::net::PacketHeader>();
			pkt->msg_id = MsgId::Notify;
			pkt->msg_type = wMsgType;

			std::size_t text_len = 0;
			if (pString != 0) {
				text_len = std::strlen(pString);
				if (text_len >= 509) {
					text_len = 509;
				}
				writer.AppendBytes(pString, text_len);
			}
			writer.AppendBytes("", 1);
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(writer.Data(), static_cast<int>(writer.Size()));
		}
		break;

	case Notify::RewardGold:
	{
		hb::net::PacketNotifyRewardGold pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.gold = static_cast<uint32_t>(m_pClientList[iToH]->m_iRewardGold);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::ServerShutdown:
	{
		hb::net::PacketNotifyServerShutdown pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.mode = static_cast<uint8_t>(sV1);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::GlobalAttackMode:
	case Notify::WhetherChange:
		if (wMsgType == Notify::GlobalAttackMode) {
			hb::net::PacketNotifyGlobalAttackMode pkt{};
			pkt.header.msg_id = MsgId::Notify;
			pkt.header.msg_type = wMsgType;
			pkt.mode = static_cast<uint8_t>(sV1);
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		}
		else {
			hb::net::PacketNotifyWhetherChange pkt{};
			pkt.header.msg_id = MsgId::Notify;
			pkt.header.msg_type = wMsgType;
			pkt.status = static_cast<uint8_t>(sV1);
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		}
		break;

	case Notify::FishCanceled:
	case Notify::FishSuccess:
	case Notify::FishFail:
		if (wMsgType == Notify::FishCanceled) {
			hb::net::PacketNotifyFishCanceled pkt{};
			pkt.header.msg_id = MsgId::Notify;
			pkt.header.msg_type = wMsgType;
			pkt.reason = static_cast<uint16_t>(sV1);
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		}
		else {
			hb::net::PacketNotifySimpleShort pkt{};
			pkt.header.msg_id = MsgId::Notify;
			pkt.header.msg_type = wMsgType;
			pkt.value = static_cast<int16_t>(sV1);
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		}
		break;

	case Notify::DebugMsg:
	{
		hb::net::PacketNotifySimpleShort pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.value = static_cast<int16_t>(sV1);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::FishChance:
	{
		hb::net::PacketNotifyFishChance pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.chance = static_cast<uint16_t>(sV1);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::EnergySphereGoalIn:
	case Notify::EventFishMode:
		if (wMsgType == Notify::EnergySphereGoalIn) {
			hb::net::PacketNotifyEnergySphereGoalIn pkt{};
			pkt.header.msg_id = MsgId::Notify;
			pkt.header.msg_type = wMsgType;
			pkt.result = static_cast<int16_t>(sV1);
			pkt.side = static_cast<int16_t>(sV2);
			pkt.goal = static_cast<int16_t>(sV3);
			if (pString != 0) {
				memcpy(pkt.name, pString, sizeof(pkt.name));
			}
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		}
		else {
			hb::net::PacketNotifyEventFishMode pkt{};
			pkt.header.msg_id = MsgId::Notify;
			pkt.header.msg_type = wMsgType;
			pkt.price = static_cast<uint16_t>(sV1);
			pkt.sprite = static_cast<uint16_t>(sV2);
			pkt.sprite_frame = static_cast<uint16_t>(sV3);
			if (pString != 0) {
				memcpy(pkt.name, pString, sizeof(pkt.name));
			}
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		}
		break;

	case Notify::NoticeMsg:
	{
		char buf[sizeof(hb::net::PacketHeader) + 256]{};
		auto* pkt = reinterpret_cast<hb::net::PacketNotifyNoticeMsg*>(buf);
		pkt->header.msg_id = MsgId::Notify;
		pkt->header.msg_type = wMsgType;
		std::size_t msg_len = 0;
		if (pString != 0) {
			msg_len = std::strlen(pString);
			if (msg_len > 255) msg_len = 255;
			memcpy(pkt->text, pString, msg_len);
		}
		pkt->text[msg_len] = '\0';
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(buf,
			static_cast<int>(sizeof(hb::net::PacketHeader) + msg_len + 1));
		break;
	}

	case Notify::StatusText:
	{
		hb::net::PacketNotifyStatusText pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		if (pString != nullptr) {
			strncpy(pkt.text, pString, sizeof(pkt.text) - 1);
			pkt.text[sizeof(pkt.text) - 1] = '\0';
		}
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::CannotRating:
	{
		hb::net::PacketNotifyCannotRating pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.time_left = static_cast<uint16_t>(sV1);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::RatingPlayer:
	{
		hb::net::PacketNotifyRatingPlayer pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.result = static_cast<uint8_t>(sV1);
		if (pString != 0) {
			memcpy(pkt.name, pString, sizeof(pkt.name));
		}
		pkt.rating = m_pClientList[iToH]->m_iRating;
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;


	case Notify::TimeChange:
	{
		hb::net::PacketNotifyTimeChange pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.sprite_alpha = static_cast<uint8_t>(sV1);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::ToBeRecalled:
	{
		hb::net::PacketNotifyEmpty pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::Hunger:
	{
		hb::net::PacketNotifyHunger pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.hunger = static_cast<uint8_t>(sV1);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::PlayerProfile:
	{
		hb::net::PacketNotifyPlayerProfile pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		std::size_t send_len = 0;
		if (pString != 0) {
			send_len = std::strlen(pString);
			if (send_len >= sizeof(pkt.text)) {
				send_len = sizeof(pkt.text) - 1;
			}
			std::size_t copy_len = send_len;
			if (copy_len > 100) {
				copy_len = 100;
			}
			memcpy(pkt.text, pString, copy_len);
		}
		pkt.text[send_len] = '\0';
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt),
			static_cast<int>(sizeof(hb::net::PacketHeader) + send_len + 1));
		break;
	}

	// New 10/05/2004 Changed
	case Notify::WhisperModeOn:
	case Notify::WhisperModeOff:
	case Notify::PlayerNotOnGame:
	{
		hb::net::PacketNotifyPlayerNotOnGame pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		if (pString != 0) {
			memcpy(pkt.name, pString, sizeof(pkt.name));
		}
		std::memset(pkt.filler, ' ', sizeof(pkt.filler));
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	// New 15/05/2004 Changed
	case Notify::PlayerOnGame:
	{
		hb::net::PacketNotifyPlayerOnGame pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		if (pString != 0) {
			memcpy(pkt.name, pString, sizeof(pkt.name));
		}
		if (pString != 0 && pString[0] != 0 && pString2 != 0) {
			memcpy(pkt.map_name, pString2, sizeof(pkt.map_name));
		}
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	// New 06/05/2004
	case Notify::ItemSold:
	case Notify::ItemRepaired:
	{
		hb::net::PacketNotifyItemRepaired pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.item_id = static_cast<uint32_t>(sV1);
		pkt.life = static_cast<uint32_t>(sV2);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	// New 06/05/2004
	case Notify::RepairItemPrice:
	case Notify::SellItemPrice:
	{
		if (wMsgType == Notify::RepairItemPrice) {
			hb::net::PacketNotifyRepairItemPrice pkt{};
			pkt.header.msg_id = MsgId::Notify;
			pkt.header.msg_type = wMsgType;
			pkt.v1 = static_cast<uint32_t>(sV1);
			pkt.v2 = static_cast<uint32_t>(sV2);
			pkt.v3 = static_cast<uint32_t>(sV3);
			pkt.v4 = static_cast<uint32_t>(sV4);
			if (pString != 0) {
				memcpy(pkt.item_name, pString, sizeof(pkt.item_name));
			}
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		}
		else {
			hb::net::PacketNotifySellItemPrice pkt{};
			pkt.header.msg_id = MsgId::Notify;
			pkt.header.msg_type = wMsgType;
			pkt.v1 = static_cast<uint32_t>(sV1);
			pkt.v2 = static_cast<uint32_t>(sV2);
			pkt.v3 = static_cast<uint32_t>(sV3);
			pkt.v4 = static_cast<uint32_t>(sV4);
			if (pString != 0) {
				memcpy(pkt.item_name, pString, sizeof(pkt.item_name));
			}
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		}
		break;
	}

	case Notify::CannotRepairItem:
	case Notify::CannotSellItem:
		if (wMsgType == Notify::CannotRepairItem) {
			hb::net::PacketNotifyCannotRepairItem pkt{};
			pkt.header.msg_id = MsgId::Notify;
			pkt.header.msg_type = wMsgType;
			pkt.item_index = static_cast<uint16_t>(sV1);
			pkt.reason = static_cast<uint16_t>(sV2);
			if (pString != 0) {
				memcpy(pkt.name, pString, sizeof(pkt.name));
			}
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		}
		else {
			hb::net::PacketNotifyCannotSellItem pkt{};
			pkt.header.msg_id = MsgId::Notify;
			pkt.header.msg_type = wMsgType;
			pkt.item_index = static_cast<uint16_t>(sV1);
			pkt.reason = static_cast<uint16_t>(sV2);
			if (pString != 0) {
				memcpy(pkt.name, pString, sizeof(pkt.name));
			}
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		}
		break;

	case Notify::ShowMap:
	{
		hb::net::PacketNotifyShowMap pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.map_id = static_cast<uint16_t>(sV1);
		pkt.map_type = static_cast<uint16_t>(sV2);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::SkillUsingEnd:
	{
		hb::net::PacketNotifySkillUsingEnd pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.result = static_cast<uint16_t>(sV1);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::TotalUsers:
	{
		hb::net::PacketNotifyTotalUsers pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.total = static_cast<uint16_t>(m_iTotalGameServerClients);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::MagicEffectOff:
	case Notify::MagicEffectOn:
	{
		hb::net::PacketNotifyMagicEffect pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.magic_type = static_cast<uint16_t>(sV1);
		pkt.effect = static_cast<uint32_t>(sV2);
		pkt.owner = static_cast<uint32_t>(sV3);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::CannotItemToBank:
	{
		hb::net::PacketNotifyEmpty pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::ServerChange:
	{
		hb::net::PacketNotifyServerChange pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		memcpy(pkt.map_name, m_pClientList[iToH]->m_cMapName, sizeof(pkt.map_name));
		memcpy(pkt.log_server_addr, m_cLoginListenIP, sizeof(pkt.log_server_addr));
		pkt.log_server_port = m_iLoginListenPort;
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::Skill:
	{
		hb::net::PacketNotifySkill pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.skill_index = static_cast<uint16_t>(sV1);
		pkt.skill_value = static_cast<uint16_t>(sV2);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::SetItemCount:
	{
		hb::net::PacketNotifySetItemCount pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.item_index = static_cast<uint16_t>(sV1);
		pkt.count = static_cast<uint32_t>(sV2);
		pkt.notify = static_cast<uint8_t>(sV3);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::ItemDepletedEraseItem:
	{
		hb::net::PacketNotifyItemDepletedEraseItem pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.item_index = static_cast<uint16_t>(sV1);
		pkt.use_result = static_cast<uint16_t>(sV2);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::DropItemFinCountChanged:
	{
		hb::net::PacketNotifyDropItemFinCountChanged pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.item_index = static_cast<uint16_t>(sV1);
		pkt.amount = static_cast<int32_t>(sV2);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::DropItemFinEraseItem:
	{
		hb::net::PacketNotifyDropItemFinEraseItem pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.item_index = static_cast<uint16_t>(sV1);
		pkt.amount = static_cast<int32_t>(sV2);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::CannotGiveItem:
	case Notify::GiveItemFinCountChanged:
		if (wMsgType == Notify::GiveItemFinCountChanged) {
			hb::net::PacketNotifyGiveItemFinCountChanged pkt{};
			pkt.header.msg_id = MsgId::Notify;
			pkt.header.msg_type = wMsgType;
			pkt.item_index = static_cast<uint16_t>(sV1);
			pkt.amount = static_cast<int32_t>(sV2);
			if (pString != 0) {
				memcpy(pkt.name, pString, sizeof(pkt.name));
			}
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		}
		else {
			hb::net::PacketNotifyCannotGiveItem pkt{};
			pkt.header.msg_id = MsgId::Notify;
			pkt.header.msg_type = wMsgType;
			pkt.item_index = static_cast<uint16_t>(sV1);
			pkt.amount = static_cast<int32_t>(sV2);
			if (pString != 0) {
				memcpy(pkt.name, pString, sizeof(pkt.name));
			}
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		}
		break;

	case Notify::GiveItemFinEraseItem:
	{
		hb::net::PacketNotifyGiveItemFinEraseItem pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.item_index = static_cast<uint16_t>(sV1);
		pkt.amount = static_cast<int32_t>(sV2);
		if (pString != 0) {
			memcpy(pkt.name, pString, sizeof(pkt.name));
		}
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::EnemyKillReward:
	{
		hb::net::PacketNotifyEnemyKillReward pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.exp = static_cast<uint32_t>(m_pClientList[iToH]->m_iExp);
		pkt.kill_count = static_cast<uint32_t>(m_pClientList[iToH]->m_iEnemyKillCount);
		memcpy(pkt.killer_name, m_pClientList[sV1]->m_cCharName, sizeof(pkt.killer_name));
		memcpy(pkt.killer_guild, m_pClientList[sV1]->m_cGuildName, sizeof(pkt.killer_guild));
		pkt.killer_rank = static_cast<int16_t>(m_pClientList[sV1]->m_iGuildRank);
		pkt.war_contribution = static_cast<int16_t>(m_pClientList[iToH]->m_iWarContribution);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::PkCaptured:
	{
		hb::net::PacketNotifyPKcaptured pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.pk_count = static_cast<uint16_t>(sV1);
		pkt.victim_pk_count = static_cast<uint16_t>(sV2);
		if (pString != 0) {
			memcpy(pkt.victim_name, pString, sizeof(pkt.victim_name));
		}
		pkt.reward_gold = static_cast<uint32_t>(m_pClientList[iToH]->m_iRewardGold);
		pkt.exp = static_cast<uint32_t>(m_pClientList[iToH]->m_iExp);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::PkPenalty:
	{
		hb::net::PacketNotifyPKpenalty pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.exp = static_cast<uint32_t>(m_pClientList[iToH]->m_iExp);
		pkt.str = static_cast<uint32_t>(m_pClientList[iToH]->m_iStr);
		pkt.vit = static_cast<uint32_t>(m_pClientList[iToH]->m_iVit);
		pkt.dex = static_cast<uint32_t>(m_pClientList[iToH]->m_iDex);
		pkt.intel = static_cast<uint32_t>(m_pClientList[iToH]->m_iInt);
		pkt.mag = static_cast<uint32_t>(m_pClientList[iToH]->m_iMag);
		pkt.chr = static_cast<uint32_t>(m_pClientList[iToH]->m_iCharisma);
		pkt.pk_count = static_cast<uint32_t>(m_pClientList[iToH]->m_iPKCount);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::RepairAllPrices:
	{
		hb::net::PacketNotifyRepairAllPricesHeader header{};
		header.header.msg_id = MsgId::Notify;
		header.header.msg_type = wMsgType;
		int total = m_pClientList[iToH]->totalItemRepair;
		if (total < 0) total = 0;
		header.total = static_cast<int16_t>(total);

		hb::net::PacketWriter writer;
		writer.Reserve(sizeof(hb::net::PacketNotifyRepairAllPricesHeader) +
			(total * sizeof(hb::net::PacketNotifyRepairAllPricesEntry)));
		writer.AppendBytes(&header, sizeof(header));

		for(int i = 0; i < total; i++) {
			hb::net::PacketNotifyRepairAllPricesEntry entry{};
			entry.index = static_cast<uint8_t>(m_pClientList[iToH]->m_stRepairAll[i].index);
			entry.price = static_cast<int16_t>(m_pClientList[iToH]->m_stRepairAll[i].price);
			writer.AppendBytes(&entry, sizeof(entry));
		}

		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(writer.Data(), static_cast<int>(writer.Size()));
	}
	break;

	case Notify::TravelerLimitedLevel:
	case Notify::LimitedLevel:
	{
		hb::net::PacketNotifySimpleInt pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.value = static_cast<int32_t>(m_pClientList[iToH]->m_iExp);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::ItemReleased:
	{
		hb::net::PacketNotifyItemReleased pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.equip_pos = static_cast<int16_t>(sV1);
		pkt.item_index = static_cast<int16_t>(sV2);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}
	case Notify::ItemLifeSpanEnd:
	{
		hb::net::PacketNotifyItemLifeSpanEnd pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.equip_pos = static_cast<int16_t>(sV1);
		pkt.item_index = static_cast<int16_t>(sV2);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::Killed:
	{
		hb::net::PacketNotifyKilled pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		if (pString != 0) {
			memcpy(pkt.attacker_name, pString, sizeof(pkt.attacker_name));
		}
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::Exp:
	{
		hb::net::PacketNotifyExp pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.exp = static_cast<uint32_t>(m_pClientList[iToH]->m_iExp);
		pkt.rating = static_cast<int32_t>(m_pClientList[iToH]->m_iRating);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::Hp:
	{
		hb::net::PacketNotifyHP pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.hp = static_cast<uint32_t>(m_pClientList[iToH]->m_iHP);
		pkt.hunger = static_cast<uint32_t>(m_pClientList[iToH]->m_iHungerStatus);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::Mp:
	{
		hb::net::PacketNotifyMP pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.mp = static_cast<uint32_t>(m_pClientList[iToH]->m_iMP);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::Sp:
	{
		hb::net::PacketNotifySP pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.sp = static_cast<uint32_t>(m_pClientList[iToH]->m_iSP);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::Charisma:
	{
		hb::net::PacketNotifyCharisma pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.charisma = static_cast<uint32_t>(m_pClientList[iToH]->m_iCharisma);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	// State change failures
	case Notify::StateChangeFailed:
	case Notify::SettingFailed:
	{
		hb::net::PacketNotifyEmpty pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::StateChangeSuccess:	// 2003-04-14     .. wtf korean junk
	{
		
		hb::net::PacketNotifyStateChangeSuccess pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;

		for(int i = 0; i < hb::shared::limits::MaxMagicType; i++) {
			pkt.magic_mastery[i] = static_cast<uint8_t>(m_pClientList[iToH]->m_cMagicMastery[i]);
		}

		for(int i = 0; i < hb::shared::limits::MaxSkillType; i++) {
			pkt.skill_mastery[i] = static_cast<uint8_t>(m_pClientList[iToH]->m_cSkillMastery[i]);
		}

		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::SettingSuccess:
	case Notify::LevelUp:
	{
		hb::net::PacketNotifyLevelUp pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.level = m_pClientList[iToH]->m_iLevel;
		pkt.str = m_pClientList[iToH]->m_iStr;
		pkt.vit = m_pClientList[iToH]->m_iVit;
		pkt.dex = m_pClientList[iToH]->m_iDex;
		pkt.intel = m_pClientList[iToH]->m_iInt;
		pkt.mag = m_pClientList[iToH]->m_iMag;
		pkt.chr = m_pClientList[iToH]->m_iCharisma;
		pkt.attack_delay = m_pClientList[iToH]->m_status.iAttackDelay;
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::QueryDismissGuildReqPermission:
	{
		hb::net::PacketNotifyQueryDismissGuildPermission pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		memcpy(pkt.name, m_pClientList[iFromH]->m_cCharName, sizeof(pkt.name));
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::QueryJoinGuildReqPermission:
	{
		hb::net::PacketNotifyQueryJoinGuildPermission pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		memcpy(pkt.name, m_pClientList[iFromH]->m_cCharName, sizeof(pkt.name));
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case Notify::CannotJoinMoreGuildsman:
	{
		hb::net::PacketNotifyCannotJoinMoreGuildsMan pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		memcpy(pkt.name, m_pClientList[iFromH]->m_cCharName, sizeof(pkt.name));
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		break;
	}

	case CommonType::JoinGuildApprove:
	{
		hb::net::PacketNotifyJoinGuildApprove pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		if (m_pClientList[iFromH] != 0) {
			memcpy(pkt.guild_name, m_pClientList[iFromH]->m_cGuildName, sizeof(pkt.guild_name));
		}
		else {
			memcpy(pkt.guild_name, "?", 1);
		}
		pkt.rank = m_iStartingGuildRank;
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case CommonType::JoinGuildReject:
	case CommonType::DismissGuildApprove:
	case CommonType::DismissGuildReject:
		if (wMsgType == CommonType::JoinGuildReject) {
			hb::net::PacketNotifyJoinGuildReject pkt{};
			pkt.header.msg_id = MsgId::Notify;
			pkt.header.msg_type = wMsgType;
			if (m_pClientList[iFromH] != 0) {
				memcpy(pkt.guild_name, m_pClientList[iFromH]->m_cGuildName, sizeof(pkt.guild_name));
			}
			else {
				memcpy(pkt.guild_name, "?", 1);
			}
			pkt.rank = m_iStartingGuildRank;
			memcpy(pkt.location, m_pClientList[iToH]->m_cLocation, sizeof(pkt.location));
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		}
		else if (wMsgType == CommonType::DismissGuildApprove) {
			hb::net::PacketNotifyDismissGuildApprove pkt{};
			pkt.header.msg_id = MsgId::Notify;
			pkt.header.msg_type = wMsgType;
			if (m_pClientList[iFromH] != 0) {
				memcpy(pkt.guild_name, m_pClientList[iFromH]->m_cGuildName, sizeof(pkt.guild_name));
			}
			else {
				memcpy(pkt.guild_name, "?", 1);
			}
			pkt.rank = m_iStartingGuildRank;
			memcpy(pkt.location, m_pClientList[iToH]->m_cLocation, sizeof(pkt.location));
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		}
		else {
			hb::net::PacketNotifyDismissGuildReject pkt{};
			pkt.header.msg_id = MsgId::Notify;
			pkt.header.msg_type = wMsgType;
			if (m_pClientList[iFromH] != 0) {
				memcpy(pkt.guild_name, m_pClientList[iFromH]->m_cGuildName, sizeof(pkt.guild_name));
			}
			else {
				memcpy(pkt.guild_name, "?", 1);
			}
			pkt.rank = m_iStartingGuildRank;
			memcpy(pkt.location, m_pClientList[iToH]->m_cLocation, sizeof(pkt.location));
			iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
		}
		break;

	case Notify::GuildDisbanded:
	{
		hb::net::PacketNotifyGuildDisbanded pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		if (pString != 0) {
			memcpy(pkt.guild_name, pString, sizeof(pkt.guild_name));
		}
		memcpy(pkt.location, m_pClientList[iToH]->m_cLocation, sizeof(pkt.location));
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::FightZoneReserve:
	{
		hb::net::PacketNotifyFightZoneReserve pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		pkt.result = static_cast<int32_t>(sV1);
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::NoGuildMasterLevel:
	{
		hb::net::PacketNotifyEmpty pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;

	case Notify::CannotBanGuildman:
	{
		hb::net::PacketNotifyEmpty pkt{};
		pkt.header.msg_id = MsgId::Notify;
		pkt.header.msg_type = wMsgType;
		iRet = m_pClientList[iToH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	break;
	}

	switch (iRet) {
	case sock::Event::QueueFull:
	case sock::Event::SocketError:
	case sock::Event::CriticalError:
	case sock::Event::SocketClosed:
		// . Time Out  .
		//DeleteClient(iToH, true, true);
		return;
	}
}







int CGame::GetNpcConfigIdByName(const char* pNpcName) const
{
	if (pNpcName == nullptr || pNpcName[0] == 0) return -1;
	for (int i = 0; i < MaxNpcTypes; i++) {
		if (m_pNpcConfigList[i] != 0) {
			if (memcmp(pNpcName, m_pNpcConfigList[i]->m_cNpcName, hb::shared::limits::NpcNameLen - 1) == 0) {
				return i;
			}
		}
	}
	return -1;
}

bool CGame::_bInitNpcAttr(class CNpc* pNpc, int iNpcConfigId, short sClass, char cSA)
{
	int iTemp;
	double dV1, dV2, dV3;

	if (iNpcConfigId < 0 || iNpcConfigId >= MaxNpcTypes || m_pNpcConfigList[iNpcConfigId] == 0) {
		return false;
	}

	{
		int iConfigIdx = iNpcConfigId;
				pNpc->m_iNpcConfigId = static_cast<short>(iNpcConfigId);

				std::memset(pNpc->m_cNpcName, 0, sizeof(pNpc->m_cNpcName));
				memcpy(pNpc->m_cNpcName, m_pNpcConfigList[iConfigIdx]->m_cNpcName, hb::shared::limits::NpcNameLen - 1);

				pNpc->m_sType = m_pNpcConfigList[iConfigIdx]->m_sType;

				// HitDice   .    .
				if (m_pNpcConfigList[iConfigIdx]->m_iHitDice <= 5)
					pNpc->m_iHP = (iDice(m_pNpcConfigList[iConfigIdx]->m_iHitDice, 4) + m_pNpcConfigList[iConfigIdx]->m_iHitDice);
				else pNpc->m_iHP = ((m_pNpcConfigList[iConfigIdx]->m_iHitDice * 4) + m_pNpcConfigList[iConfigIdx]->m_iHitDice + iDice(1, m_pNpcConfigList[iConfigIdx]->m_iHitDice));
				if (pNpc->m_iHP == 0) pNpc->m_iHP = 1;

				//50Cent - HP Bar
				pNpc->m_iMaxHP = pNpc->m_iHP;

				pNpc->m_iExpDiceMin = m_pNpcConfigList[iConfigIdx]->m_iExpDiceMin;
				pNpc->m_iExpDiceMax = m_pNpcConfigList[iConfigIdx]->m_iExpDiceMax;
				pNpc->m_iGoldDiceMin = m_pNpcConfigList[iConfigIdx]->m_iGoldDiceMin;
				pNpc->m_iGoldDiceMax = m_pNpcConfigList[iConfigIdx]->m_iGoldDiceMax;
				pNpc->m_iDropTableId = m_pNpcConfigList[iConfigIdx]->m_iDropTableId;
				pNpc->m_iExp = (iDice(1, (m_pNpcConfigList[iConfigIdx]->m_iExpDiceMax - m_pNpcConfigList[iConfigIdx]->m_iExpDiceMin)) + m_pNpcConfigList[iConfigIdx]->m_iExpDiceMin);

				pNpc->m_iHitDice = m_pNpcConfigList[iConfigIdx]->m_iHitDice;
				pNpc->m_iDefenseRatio = m_pNpcConfigList[iConfigIdx]->m_iDefenseRatio;
				pNpc->m_iHitRatio = m_pNpcConfigList[iConfigIdx]->m_iHitRatio;
				pNpc->m_iMinBravery = m_pNpcConfigList[iConfigIdx]->m_iMinBravery;
				pNpc->m_cAttackDiceThrow = m_pNpcConfigList[iConfigIdx]->m_cAttackDiceThrow;
				pNpc->m_cAttackDiceRange = m_pNpcConfigList[iConfigIdx]->m_cAttackDiceRange;
				pNpc->m_cSize = m_pNpcConfigList[iConfigIdx]->m_cSize;
				pNpc->m_cSide = m_pNpcConfigList[iConfigIdx]->m_cSide;
				pNpc->m_cActionLimit = m_pNpcConfigList[iConfigIdx]->m_cActionLimit;
				pNpc->m_dwActionTime = m_pNpcConfigList[iConfigIdx]->m_dwActionTime;
				pNpc->m_dwRegenTime = m_pNpcConfigList[iConfigIdx]->m_dwRegenTime;
				pNpc->m_cResistMagic = m_pNpcConfigList[iConfigIdx]->m_cResistMagic;
				pNpc->m_cMagicLevel = m_pNpcConfigList[iConfigIdx]->m_cMagicLevel;
				pNpc->m_iMaxMana = m_pNpcConfigList[iConfigIdx]->m_iMaxMana; // v1.4
				pNpc->m_iMana = m_pNpcConfigList[iConfigIdx]->m_iMaxMana;
				pNpc->m_cChatMsgPresence = m_pNpcConfigList[iConfigIdx]->m_cChatMsgPresence;
				pNpc->m_cDayOfWeekLimit = m_pNpcConfigList[iConfigIdx]->m_cDayOfWeekLimit;
				pNpc->m_cTargetSearchRange = m_pNpcConfigList[iConfigIdx]->m_cTargetSearchRange;

				switch (sClass) {
				case 43:
				case 44:
				case 45:
				case 46:
				case 47:
					pNpc->m_iAttackStrategy = AttackAI::Normal;
					break;

				default:
					pNpc->m_iAttackStrategy = iDice(1, 10);
					break;
				}

				pNpc->m_iAILevel = iDice(1, 3);
				pNpc->m_iAbsDamage = m_pNpcConfigList[iConfigIdx]->m_iAbsDamage;
				pNpc->m_iMagicHitRatio = m_pNpcConfigList[iConfigIdx]->m_iMagicHitRatio;
				pNpc->m_iAttackRange = m_pNpcConfigList[iConfigIdx]->m_iAttackRange;
				pNpc->m_cSpecialAbility = cSA;
				pNpc->m_iBuildCount = m_pNpcConfigList[iConfigIdx]->m_iMinBravery;

				switch (pNpc->m_cSpecialAbility) {
				case 1:
					dV2 = (double)pNpc->m_iExp;
					dV3 = 25.0f / 100.0f;
					dV1 = dV2 * dV3;
					pNpc->m_iExp += (uint32_t)dV1;
					break;

				case 2:
					dV2 = (double)pNpc->m_iExp;
					dV3 = 30.0f / 100.0f;
					dV1 = dV2 * dV3;
					pNpc->m_iExp += (uint32_t)dV1;
					break;

				case 3: // Absorbing Physical Damage
					if (pNpc->m_iAbsDamage > 0) {
						pNpc->m_cSpecialAbility = 0;
						cSA = 0;
					}
					else {
						iTemp = 20 + iDice(1, 60);
						pNpc->m_iAbsDamage -= iTemp;
						if (pNpc->m_iAbsDamage < -90) pNpc->m_iAbsDamage = -90;
					}

					dV2 = (double)pNpc->m_iExp;
					dV3 = (double)abs(pNpc->m_iAbsDamage) / 100.0f;
					dV1 = dV2 * dV3;
					pNpc->m_iExp += (uint32_t)dV1;
					break;

				case 4: // Absorbing Magical Damage
					if (pNpc->m_iAbsDamage < 0) {
						pNpc->m_cSpecialAbility = 0;
						cSA = 0;
					}
					else {
						iTemp = 20 + iDice(1, 60);
						pNpc->m_iAbsDamage += iTemp;
						if (pNpc->m_iAbsDamage > 90) pNpc->m_iAbsDamage = 90;
					}

					dV2 = (double)pNpc->m_iExp;
					dV3 = (double)(pNpc->m_iAbsDamage) / 100.0f;
					dV1 = dV2 * dV3;
					pNpc->m_iExp += (uint32_t)dV1;
					break;

				case 5:
					dV2 = (double)pNpc->m_iExp;
					dV3 = 15.0f / 100.0f;
					dV1 = dV2 * dV3;
					pNpc->m_iExp += (uint32_t)dV1;
					break;

				case 6:
				case 7:
					dV2 = (double)pNpc->m_iExp;
					dV3 = 20.0f / 100.0f;
					dV1 = dV2 * dV3;
					pNpc->m_iExp += (uint32_t)dV1;
					break;

				case 8:
					dV2 = (double)pNpc->m_iExp;
					dV3 = 25.0f / 100.0f;
					dV1 = dV2 * dV3;
					pNpc->m_iExp += (uint32_t)dV1;
					break;
				}

				pNpc->m_iNoDieRemainExp = (pNpc->m_iExp) - (pNpc->m_iExp / 3);

				pNpc->m_status.iAngelPercent = static_cast<uint8_t>(cSA);

				pNpc->m_status.iAttackDelay = static_cast<uint8_t>(sClass);

		return true;
	}
}

/*********************************************************************************************************************
**  int CGame::iDice(int iThrow, int iRange)																		**
**  description			:: produces a random number between the throw and range										**
**  last updated		:: November 20, 2004; 10:24 PM; Hypnotoad													**
**	return value		:: int																						**
**********************************************************************************************************************/
uint32_t CGame::iDice(uint32_t iThrow, uint32_t iRange)
{
	uint32_t iRet;

	if (iRange <= 0) return 0;
	iRet = 0;
	for (uint32_t i = 1; i <= iThrow; i++) {
		iRet += (rand() % iRange) + 1;
	}
	return iRet;
}


void CGame::TimeManaPointsUp(int iClientH)
{
	int iMaxMP, iTotal;
	double dV1, dV2, dV3;

	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_bIsKilled) return;
	if (m_pClientList[iClientH]->m_bIsInitComplete == false) return;
	if (m_pClientList[iClientH]->m_iHungerStatus <= 0) return;
	if (m_pClientList[iClientH]->m_bSkillUsingStatus[19]) return;

	iMaxMP = iGetMaxMP(iClientH); // v1.4
	if (m_pClientList[iClientH]->m_iMP < iMaxMP) {
		iTotal = iDice(1, (m_pClientList[iClientH]->m_iMag + m_pClientList[iClientH]->m_iAngelicMag));
		if (m_pClientList[iClientH]->m_iAddMP != 0) {
			dV2 = (double)iTotal;
			dV3 = (double)m_pClientList[iClientH]->m_iAddMP;
			dV1 = (dV3 / 100.0f) * dV2;
			iTotal += (int)dV1;
		}

		m_pClientList[iClientH]->m_iMP += iTotal;

		if (m_pClientList[iClientH]->m_iMP > iMaxMP)
			m_pClientList[iClientH]->m_iMP = iMaxMP;

		SendNotifyMsg(0, iClientH, Notify::Mp, 0, 0, 0, 0);
	}
}

// 05/29/2004 - Hypnotoad - fixed infinite sp bug
void CGame::TimeStaminarPointsUp(int iClientH)
{
	int iMaxSP, iTotal = 0;
	double dV1, dV2, dV3;

	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_bIsKilled) return;
	if (m_pClientList[iClientH]->m_bIsInitComplete == false) return;
	if (m_pClientList[iClientH]->m_iHungerStatus <= 0) return;
	if (m_pClientList[iClientH]->m_bSkillUsingStatus[19]) return;

	iMaxSP = iGetMaxSP(iClientH);
	if (m_pClientList[iClientH]->m_iSP < iMaxSP) {

		iTotal = iDice(1, (m_pClientList[iClientH]->m_iVit / 3)); // Staminar Point 10 1D(Vit/3) .
		if (m_pClientList[iClientH]->m_iAddSP != 0) {
			dV2 = (double)iTotal;
			dV3 = (double)m_pClientList[iClientH]->m_iAddSP;
			dV1 = (dV3 / 100.0f) * dV2;
			iTotal += (int)dV1;
		}

		if (m_pClientList[iClientH]->m_iLevel <= 20) {
			iTotal += 15;
		}
		else if (m_pClientList[iClientH]->m_iLevel <= 40) {
			iTotal += 10;
		}
		else if (m_pClientList[iClientH]->m_iLevel <= 60) {
			iTotal += 5;
		}

		m_pClientList[iClientH]->m_iSP += iTotal;
		if (m_pClientList[iClientH]->m_iSP > iMaxSP)
			m_pClientList[iClientH]->m_iSP = iMaxSP;

		SendNotifyMsg(0, iClientH, Notify::Sp, 0, 0, 0, 0);
	}
}


void CGame::ToggleCombatModeHandler(int iClientH)
{
	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_bIsInitComplete == false) return;
	if (m_pClientList[iClientH]->m_bIsKilled) return;
	if (m_pClientList[iClientH]->m_bSkillUsingStatus[19]) return;

	m_pClientList[iClientH]->m_bIsAttackModeChange = true; // v2.172

	if (!m_pClientList[iClientH]->m_appearance.bIsWalking) {
		m_pClientList[iClientH]->m_appearance.bIsWalking = true;
	}
	else {
		m_pClientList[iClientH]->m_appearance.bIsWalking = false;
	}

	SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventMotion, Type::NullAction, 0, 0, 0);

}


//  int CGame::iClientMotion_Magic_Handler(int iClientH, short sX, short sY, char cDir)
//  description			:: checks if player is casting magic
//  last updated		:: October 29, 2004; 6:51 PM; Hypnotoad
//	return value		:: int

/*********************************************************************************************************************
**  void CGame::PlayerMagicHandler(int iClientH, int dX, int dY, short sType, bool bItemEffect, int iV1)			**
**  description			:: handles all magic related items/spells													**
**  last updated		:: November 22, 2004; 5:45 PM; Hypnotoad													**
**	return value		:: void																						**
**  commentary			::	-	added 3.51 casting detection														**
**							-	updated it so civilians can only cast certain spells on players and vice versa		**
**							-	fixed bug causing spell to be cast when mana is below required amount				**
**********************************************************************************************************************/


void CGame::RequestTeleportHandler(int iClientH, char* pData, char* cMapName, int dX, int dY)
{
	char cTempMapName[21];
	char cDestMapName[11], cDir, cMapIndex, cQuestRemain;
	short sX, sY, sSummonPoints;
	int iRet, iSize, iDestX, iDestY, iExH, iMapSide;
	bool    bRet, bIsLockedMapNotify;
	SYSTEMTIME SysTime;

	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_bIsInitComplete == false) return;
	if (m_pClientList[iClientH]->m_bIsKilled) return;
	if (m_pClientList[iClientH]->m_bIsOnWaitingProcess) return;
	if ((m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_bIsRecallImpossible) &&
		(m_pClientList[iClientH]->m_bIsKilled == false) && (m_bIsApocalypseMode) && (m_pClientList[iClientH]->m_iHP > 0)) {
		SendNotifyMsg(0, iClientH, Notify::NoRecall, 0, 0, 0, 0);
		return;
	}
	if ((memcmp(m_pClientList[iClientH]->m_cLocation, "elvine", 6) == 0)
		&& (m_pClientList[iClientH]->m_iTimeLeft_ForceRecall > 0)
		&& (memcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cLocationName, "aresden", 7) == 0)
		&& ((pData[0] == '1') || (pData[0] == '3'))
		&& (m_bIsCrusadeMode == false)) return;

	if ((memcmp(m_pClientList[iClientH]->m_cLocation, "aresden", 7) == 0)
		&& (m_pClientList[iClientH]->m_iTimeLeft_ForceRecall > 0)
		&& (memcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cLocationName, "elvine", 6) == 0)
		&& ((pData[0] == '1') || (pData[0] == '3'))
		&& (m_bIsCrusadeMode == false)) return;

	bIsLockedMapNotify = false;

	if (m_pClientList[iClientH]->m_bIsExchangeMode) {
		iExH = m_pClientList[iClientH]->m_iExchangeH;
		m_pItemManager->_ClearExchangeStatus(iExH);
		m_pItemManager->_ClearExchangeStatus(iClientH);
	}

	if ((memcmp(m_pClientList[iClientH]->m_cLocation, "NONE", 4) == 0) && (pData[0] == '1'))
		return;

	m_pCombatManager->RemoveFromTarget(iClientH, hb::shared::owner_class::Player);

	// Delete all summoned NPCs belonging to this player
	for (int i = 0; i < MaxNpcs; i++)
		if (m_pNpcList[i] != 0) {
			if ((m_pNpcList[i]->m_bIsSummoned) &&
				(m_pNpcList[i]->m_iFollowOwnerIndex == iClientH) &&
				(m_pNpcList[i]->m_cFollowOwnerType == hb::shared::owner_class::Player)) {
				m_pEntityManager->DeleteEntity(i);
			}
		}

	m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->ClearOwner(13, iClientH, hb::shared::owner_class::Player,
		m_pClientList[iClientH]->m_sX,
		m_pClientList[iClientH]->m_sY);

	SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventLog, MsgType::Reject, 0, 0, 0);


	sX = m_pClientList[iClientH]->m_sX;
	sY = m_pClientList[iClientH]->m_sY;

	std::memset(cDestMapName, 0, sizeof(cDestMapName));
	bRet = m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->bSearchTeleportDest(sX, sY, cDestMapName, &iDestX, &iDestY, &cDir);

	// Crusade
	if ((strcmp(m_pClientList[iClientH]->m_cLockedMapName, "NONE") != 0) && (m_pClientList[iClientH]->m_iLockedMapTime > 0)) {
		iMapSide = iGetMapLocationSide(cDestMapName);
		if (iMapSide > 3) iMapSide -= 2; // New 18/05/2004
		if ((iMapSide != 0) && (m_pClientList[iClientH]->m_cSide == iMapSide)) {
		}
		else {
			iDestX = -1;
			iDestY = -1;
			bIsLockedMapNotify = true;
			std::memset(cDestMapName, 0, sizeof(cDestMapName));
			strcpy(cDestMapName, m_pClientList[iClientH]->m_cLockedMapName);
		}
	}

	if ((bRet) && (cMapName == 0)) {
		for(int i = 0; i < MaxMaps; i++)
			if (m_pMapList[i] != 0) {
				if (memcmp(m_pMapList[i]->m_cName, cDestMapName, 10) == 0) {
					m_pClientList[iClientH]->m_sX = iDestX;
					m_pClientList[iClientH]->m_sY = iDestY;
					m_pClientList[iClientH]->m_cDir = cDir;
					m_pClientList[iClientH]->m_cMapIndex = i;
					std::memset(m_pClientList[iClientH]->m_cMapName, 0, sizeof(m_pClientList[iClientH]->m_cMapName));
					memcpy(m_pClientList[iClientH]->m_cMapName, m_pMapList[i]->m_cName, 10);
					goto RTH_NEXTSTEP;
				}
			}

		m_pClientList[iClientH]->m_sX = iDestX;
		m_pClientList[iClientH]->m_sY = iDestY;
		m_pClientList[iClientH]->m_cDir = cDir;
		std::memset(m_pClientList[iClientH]->m_cMapName, 0, sizeof(m_pClientList[iClientH]->m_cMapName));
		memcpy(m_pClientList[iClientH]->m_cMapName, cDestMapName, 10);

		// New 18/05/2004
		SendNotifyMsg(0, iClientH, Notify::MagicEffectOff, hb::shared::magic::Confuse,
			m_pClientList[iClientH]->m_cMagicEffectStatus[hb::shared::magic::Confuse], 0, 0);
		m_pItemManager->SetSlateFlag(iClientH, SlateClearNotify, false);

		// bSendMsgToLS(ServerMsgId::RequestSavePlayerDataReply, iClientH, false);  // !   .
		m_pClientList[iClientH]->m_bIsOnServerChange = true;
		m_pClientList[iClientH]->m_bIsOnWaitingProcess = true;
		return;
	}
	else {
		switch (pData[0]) {
		case '0':
			// Forced Recall. 
			std::memset(cTempMapName, 0, sizeof(cTempMapName));
			if (memcmp(m_pClientList[iClientH]->m_cLocation, "NONE", 4) == 0) {
				strcpy(cTempMapName, "default");
			}
			else if (memcmp(m_pClientList[iClientH]->m_cLocation, "arehunter", 9) == 0) {
				strcpy(cTempMapName, "arefarm");
			}
			else if (memcmp(m_pClientList[iClientH]->m_cLocation, "elvhunter", 9) == 0) {
				strcpy(cTempMapName, "elvfarm");
			}
			else strcpy(cTempMapName, m_pClientList[iClientH]->m_cLocation);

			// Crusade
			if ((strcmp(m_pClientList[iClientH]->m_cLockedMapName, "NONE") != 0) && (m_pClientList[iClientH]->m_iLockedMapTime > 0)) {
				bIsLockedMapNotify = true;
				std::memset(cTempMapName, 0, sizeof(cTempMapName));
				strcpy(cTempMapName, m_pClientList[iClientH]->m_cLockedMapName);
			}

			for(int i = 0; i < MaxMaps; i++)
				if (m_pMapList[i] != 0) {
					if (memcmp(m_pMapList[i]->m_cName, cTempMapName, 10) == 0) {
						GetMapInitialPoint(i, &m_pClientList[iClientH]->m_sX, &m_pClientList[iClientH]->m_sY, m_pClientList[iClientH]->m_cLocation);

						m_pClientList[iClientH]->m_cMapIndex = i;
						std::memset(m_pClientList[iClientH]->m_cMapName, 0, sizeof(m_pClientList[iClientH]->m_cMapName));
						memcpy(m_pClientList[iClientH]->m_cMapName, cTempMapName, 10);
						goto RTH_NEXTSTEP;
					}
				}

			m_pClientList[iClientH]->m_sX = -1;
			m_pClientList[iClientH]->m_sY = -1;	  // -1 InitialPoint .

			std::memset(m_pClientList[iClientH]->m_cMapName, 0, sizeof(m_pClientList[iClientH]->m_cMapName));
			memcpy(m_pClientList[iClientH]->m_cMapName, cTempMapName, 10);

			// New 18/05/2004
			SendNotifyMsg(0, iClientH, Notify::MagicEffectOff, hb::shared::magic::Confuse,
				m_pClientList[iClientH]->m_cMagicEffectStatus[hb::shared::magic::Confuse], 0, 0);
			m_pItemManager->SetSlateFlag(iClientH, SlateClearNotify, false);

			// bSendMsgToLS(ServerMsgId::RequestSavePlayerDataReply, iClientH, false); // !   .

			m_pClientList[iClientH]->m_bIsOnServerChange = true;
			m_pClientList[iClientH]->m_bIsOnWaitingProcess = true;
			return;

		case '1':
			// Recall.     .
			// if (memcmp(m_pMapList[ m_pClientList[iClientH]->m_cMapIndex ]->m_cName, "resurr", 6) == 0) return;

			std::memset(cTempMapName, 0, sizeof(cTempMapName));
			if (memcmp(m_pClientList[iClientH]->m_cLocation, "NONE", 4) == 0) {
				strcpy(cTempMapName, "default");
			}
			else {
				if (m_pClientList[iClientH]->m_iLevel > 80)
					if (memcmp(m_pClientList[iClientH]->m_cLocation, "are", 3) == 0)
						strcpy(cTempMapName, "aresden");
					else strcpy(cTempMapName, "elvine");
				else {
					if (memcmp(m_pClientList[iClientH]->m_cLocation, "are", 3) == 0)
						strcpy(cTempMapName, "arefarm");
					else strcpy(cTempMapName, "elvfarm");
				}
			}
			// Crusade
			if ((strcmp(m_pClientList[iClientH]->m_cLockedMapName, "NONE") != 0) && (m_pClientList[iClientH]->m_iLockedMapTime > 0)) {
				bIsLockedMapNotify = true;
				std::memset(cTempMapName, 0, sizeof(cTempMapName));
				strcpy(cTempMapName, m_pClientList[iClientH]->m_cLockedMapName);
			}

			for(int i = 0; i < MaxMaps; i++)
				if (m_pMapList[i] != 0) {
					if (memcmp(m_pMapList[i]->m_cName, cTempMapName, 10) == 0) {

						GetMapInitialPoint(i, &m_pClientList[iClientH]->m_sX, &m_pClientList[iClientH]->m_sY, m_pClientList[iClientH]->m_cLocation);

						m_pClientList[iClientH]->m_cMapIndex = i;
						std::memset(m_pClientList[iClientH]->m_cMapName, 0, sizeof(m_pClientList[iClientH]->m_cMapName));
						memcpy(m_pClientList[iClientH]->m_cMapName, m_pMapList[i]->m_cName, 10);
						goto RTH_NEXTSTEP;
					}
				}

			m_pClientList[iClientH]->m_sX = -1;
			m_pClientList[iClientH]->m_sY = -1;	  // -1 InitialPoint .

			std::memset(m_pClientList[iClientH]->m_cMapName, 0, sizeof(m_pClientList[iClientH]->m_cMapName));
			memcpy(m_pClientList[iClientH]->m_cMapName, cTempMapName, 10);

			// New 18/05/2004
			SendNotifyMsg(0, iClientH, Notify::MagicEffectOff, hb::shared::magic::Confuse,
				m_pClientList[iClientH]->m_cMagicEffectStatus[hb::shared::magic::Confuse], 0, 0);
			m_pItemManager->SetSlateFlag(iClientH, SlateClearNotify, false);

			// bSendMsgToLS(ServerMsgId::RequestSavePlayerDataReply, iClientH, false); // !   .
			m_pClientList[iClientH]->m_bIsOnServerChange = true;
			m_pClientList[iClientH]->m_bIsOnWaitingProcess = true;
			return;

		case '2':

			// Crusade
			if ((strcmp(m_pClientList[iClientH]->m_cLockedMapName, "NONE") != 0) && (m_pClientList[iClientH]->m_iLockedMapTime > 0)) {
				dX = -1;
				dY = -1;
				bIsLockedMapNotify = true;
				std::memset(cTempMapName, 0, sizeof(cTempMapName));
				strcpy(cTempMapName, m_pClientList[iClientH]->m_cLockedMapName);
			}
			else {
				std::memset(cTempMapName, 0, sizeof(cTempMapName));
				strcpy(cTempMapName, cMapName);
			}

			cMapIndex = iGetMapIndex(cTempMapName);
			if (cMapIndex == -1) {
				m_pClientList[iClientH]->m_sX = dX; // -1;	  //   .
				m_pClientList[iClientH]->m_sY = dY; // -1;	  // -1 InitialPoint .

				std::memset(m_pClientList[iClientH]->m_cMapName, 0, sizeof(m_pClientList[iClientH]->m_cMapName));
				memcpy(m_pClientList[iClientH]->m_cMapName, cTempMapName, 10);

				// New 18/05/2004
				SendNotifyMsg(0, iClientH, Notify::MagicEffectOff, hb::shared::magic::Confuse,
					m_pClientList[iClientH]->m_cMagicEffectStatus[hb::shared::magic::Confuse], 0, 0);
				m_pItemManager->SetSlateFlag(iClientH, SlateClearNotify, false);

				// bSendMsgToLS(ServerMsgId::RequestSavePlayerDataReply, iClientH, false); // !   .
				m_pClientList[iClientH]->m_bIsOnServerChange = true;
				m_pClientList[iClientH]->m_bIsOnWaitingProcess = true;
				return;
			}

			m_pClientList[iClientH]->m_sX = dX;
			m_pClientList[iClientH]->m_sY = dY;
			m_pClientList[iClientH]->m_cMapIndex = cMapIndex;

			std::memset(m_pClientList[iClientH]->m_cMapName, 0, sizeof(m_pClientList[iClientH]->m_cMapName));
			memcpy(m_pClientList[iClientH]->m_cMapName, m_pMapList[cMapIndex]->m_cName, 10);
			break;
		}
	}

RTH_NEXTSTEP:

	// New 17/05/2004
	SetPlayingStatus(iClientH);
	// Set faction/identity status fields from player data
	m_pClientList[iClientH]->m_status.bPK = (m_pClientList[iClientH]->m_iPKCount != 0) ? 1 : 0;
	m_pClientList[iClientH]->m_status.bCitizen = (m_pClientList[iClientH]->m_cSide != 0) ? 1 : 0;
	m_pClientList[iClientH]->m_status.bAresden = (m_pClientList[iClientH]->m_cSide == 1) ? 1 : 0;
	m_pClientList[iClientH]->m_status.bHunter = m_pClientList[iClientH]->m_bIsPlayerCivil ? 1 : 0;

	// Crusade
	if (bIsLockedMapNotify) SendNotifyMsg(0, iClientH, Notify::LockedMap, m_pClientList[iClientH]->m_iLockedMapTime, 0, 0, m_pClientList[iClientH]->m_cLockedMapName);

	hb::net::PacketWriter writer;
	char initMapData[hb::shared::limits::MsgBufferSize + 1];

	writer.Reset();
	auto* init_header = writer.Append<hb::net::PacketResponseInitDataHeader>();
	init_header->header.msg_id = MsgId::ResponseInitData;
	init_header->header.msg_type = MsgType::Confirm;

	if (m_pClientList[iClientH]->m_bIsObserverMode == false)
		bGetEmptyPosition(&m_pClientList[iClientH]->m_sX, &m_pClientList[iClientH]->m_sY, m_pClientList[iClientH]->m_cMapIndex);
	else GetMapInitialPoint(m_pClientList[iClientH]->m_cMapIndex, &m_pClientList[iClientH]->m_sX, &m_pClientList[iClientH]->m_sY);

	init_header->player_object_id = static_cast<std::int16_t>(iClientH);
	init_header->pivot_x = static_cast<std::int16_t>(m_pClientList[iClientH]->m_sX - hb::shared::view::PlayerPivotOffsetX);
	init_header->pivot_y = static_cast<std::int16_t>(m_pClientList[iClientH]->m_sY - hb::shared::view::PlayerPivotOffsetY);
	init_header->player_type = m_pClientList[iClientH]->m_sType;
	init_header->appearance = m_pClientList[iClientH]->m_appearance;
	init_header->status = m_pClientList[iClientH]->m_status;
	std::memcpy(init_header->map_name, m_pClientList[iClientH]->m_cMapName, sizeof(init_header->map_name));
	std::memcpy(init_header->cur_location, m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cLocationName, sizeof(init_header->cur_location));

	if (m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_bIsFixedDayMode)
		init_header->sprite_alpha = 1;
	else init_header->sprite_alpha = m_cDayOrNight;

	if (m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_bIsFixedDayMode)
		init_header->weather_status = 0;
	else init_header->weather_status = m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cWhetherStatus;

	init_header->contribution = m_pClientList[iClientH]->m_iContribution;

	if (m_pClientList[iClientH]->m_bIsObserverMode == false) {
		m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->SetOwner(iClientH,
			hb::shared::owner_class::Player,
			m_pClientList[iClientH]->m_sX,
			m_pClientList[iClientH]->m_sY);
	}

	init_header->observer_mode = static_cast<std::uint8_t>(m_pClientList[iClientH]->m_bIsObserverMode);
	init_header->rating = m_pClientList[iClientH]->m_iRating;
	init_header->hp = m_pClientList[iClientH]->m_iHP;
	init_header->discount = 0;

	iSize = iComposeInitMapData(m_pClientList[iClientH]->m_sX - hb::shared::view::CenterX, m_pClientList[iClientH]->m_sY - hb::shared::view::CenterY, iClientH, initMapData);
	writer.AppendBytes(initMapData, static_cast<std::size_t>(iSize));

	iRet = m_pClientList[iClientH]->m_pXSock->iSendMsg(writer.Data(), static_cast<int>(writer.Size()));
	switch (iRet) {
	case sock::Event::QueueFull:
	case sock::Event::SocketError:
	case sock::Event::CriticalError:
	case sock::Event::SocketClosed:
		DeleteClient(iClientH, true, true);
		return;
	}

	SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventLog, MsgType::Confirm, 0, 0, 0);

	if ((memcmp(m_pClientList[iClientH]->m_cLocation, "are", 3) == 0) &&
		(memcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cLocationName, "elvine", 6) == 0)) {

		m_pClientList[iClientH]->m_dwWarBeginTime = GameClock::GetTimeMS();
		m_pClientList[iClientH]->m_bIsWarLocation = true;
		// New 17/05/2004
		CheckForceRecallTime(iClientH);
	}
	else if ((memcmp(m_pClientList[iClientH]->m_cLocation, "elv", 3) == 0) &&
		(memcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cLocationName, "aresden", 7) == 0)) {

		m_pClientList[iClientH]->m_dwWarBeginTime = GameClock::GetTimeMS();
		m_pClientList[iClientH]->m_bIsWarLocation = true;

		// New 17/05/2004
		CheckForceRecallTime(iClientH);
	}
	else if (m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_bIsFightZone) {
		m_pClientList[iClientH]->m_dwWarBeginTime = GameClock::GetTimeMS();
		m_pClientList[iClientH]->m_bIsWarLocation = true;
		SetForceRecallTime(iClientH);

		GetLocalTime(&SysTime);
		m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 2 * 20 * 60 - ((SysTime.wHour % 2) * 20 * 60 + SysTime.wMinute * 20) - 2 * 20;

	}
	else {
		m_pClientList[iClientH]->m_bIsWarLocation = false;
		m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 0;
		SetForceRecallTime(iClientH);
	}

	// No entering enemy shops
	int iMapside, iMapside2;

	iMapside = iGetMapLocationSide(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cName);
	if (iMapside > 3) iMapside2 = iMapside - 2;
	else iMapside2 = iMapside;
	m_pClientList[iClientH]->m_bIsInsideOwnTown = false;
	if ((m_pClientList[iClientH]->m_cSide != iMapside2) && (iMapside != 0)) {
		if ((iMapside <= 2)) {
			if (m_pClientList[iClientH]->m_cSide != 0) {
				m_pClientList[iClientH]->m_dwWarBeginTime = GameClock::GetTimeMS();
				m_pClientList[iClientH]->m_bIsWarLocation = true;
				m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 1;
				m_pClientList[iClientH]->m_bIsInsideOwnTown = true;
			}
		}
	}
	else {
		if (m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_bIsFightZone &&
			m_iFightzoneNoForceRecall == false) {
			m_pClientList[iClientH]->m_dwWarBeginTime = GameClock::GetTimeMS();
			m_pClientList[iClientH]->m_bIsWarLocation = true;
			GetLocalTime(&SysTime);
			m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 2 * 60 * 20 - ((SysTime.wHour % 2) * 20 * 60 + SysTime.wMinute * 20) - 2 * 20;
		}
		else {
			if (memcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cLocationName, "arejail", 7) == 0 ||
				memcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cLocationName, "elvjail", 7) == 0) {
				m_pClientList[iClientH]->m_bIsWarLocation = true;
				m_pClientList[iClientH]->m_dwWarBeginTime = GameClock::GetTimeMS();
				if (m_pClientList[iClientH]->m_iTimeLeft_ForceRecall == 0)
					m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 100;
				else if (m_pClientList[iClientH]->m_iTimeLeft_ForceRecall > 100)
					m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 100;
			}
		}
	}

	// . v1.1
	SendNotifyMsg(0, iClientH, Notify::SafeAttackMode, 0, 0, 0, 0);
	// v1.3
	SendNotifyMsg(0, iClientH, Notify::DownSkillIndexSet, m_pClientList[iClientH]->m_iDownSkillIndex, 0, 0, 0);
	// V1.3
	SendNotifyMsg(0, iClientH, Notify::ItemPosList, 0, 0, 0, 0);
	// v1.4 
	m_pQuestManager->_SendQuestContents(iClientH);
	m_pQuestManager->_CheckQuestEnvironment(iClientH);

	// v1.432
	if (m_pClientList[iClientH]->m_iSpecialAbilityTime == 0)
		SendNotifyMsg(0, iClientH, Notify::SpecialAbilityEnabled, 0, 0, 0, 0);

	if (m_bIsCrusadeMode) {
		if (m_pClientList[iClientH]->m_dwCrusadeGUID == 0) {
			m_pClientList[iClientH]->m_iCrusadeDuty = 0;
			m_pClientList[iClientH]->m_iConstructionPoint = 0;
			m_pClientList[iClientH]->m_dwCrusadeGUID = m_dwCrusadeGUID;
		}
		else if (m_pClientList[iClientH]->m_dwCrusadeGUID != m_dwCrusadeGUID) {
			m_pClientList[iClientH]->m_iCrusadeDuty = 0;
			m_pClientList[iClientH]->m_iConstructionPoint = 0;
			m_pClientList[iClientH]->m_iWarContribution = 0;
			m_pClientList[iClientH]->m_dwCrusadeGUID = m_dwCrusadeGUID;
			// ? GUID .  .
			SendNotifyMsg(0, iClientH, Notify::Crusade, (uint32_t)m_bIsCrusadeMode, 0, 0, 0, -1);
		}
		m_pClientList[iClientH]->m_cVar = 1;
		SendNotifyMsg(0, iClientH, Notify::Crusade, (uint32_t)m_bIsCrusadeMode, m_pClientList[iClientH]->m_iCrusadeDuty, 0, 0);
	}
	else if (m_bIsHeldenianMode) {
		sSummonPoints = m_pClientList[iClientH]->m_iCharisma * 300;
		if (sSummonPoints > m_iMaxSummonPoints) sSummonPoints = m_iMaxSummonPoints;
		if (m_pClientList[iClientH]->m_dwHeldenianGUID == 0) {
			m_pClientList[iClientH]->m_dwHeldenianGUID = m_dwHeldenianGUID;
			m_pClientList[iClientH]->m_iConstructionPoint = sSummonPoints;
		}
		else if (m_pClientList[iClientH]->m_dwHeldenianGUID != m_dwHeldenianGUID) {
			m_pClientList[iClientH]->m_iConstructionPoint = sSummonPoints;
			m_pClientList[iClientH]->m_iWarContribution = 0;
			m_pClientList[iClientH]->m_dwHeldenianGUID = m_dwHeldenianGUID;
		}
		m_pClientList[iClientH]->m_cVar = 2;
		if (m_bIsHeldenianMode) {
			SendNotifyMsg(0, iClientH, Notify::HeldenianTeleport, 0, 0, 0, 0);
		}
		if (m_bHeldenianInitiated) {
			SendNotifyMsg(0, iClientH, Notify::HeldenianStart, 0, 0, 0, 0);
		}
		SendNotifyMsg(0, iClientH, Notify::ConstructionPoint, m_pClientList[iClientH]->m_iConstructionPoint, m_pClientList[iClientH]->m_iWarContribution, 0, 0);
		m_pWarManager->UpdateHeldenianStatus();
	}
	else if ((m_pClientList[iClientH]->m_cVar == 1) && (m_pClientList[iClientH]->m_dwCrusadeGUID == m_dwCrusadeGUID)) {
		m_pClientList[iClientH]->m_iCrusadeDuty = 0;
		m_pClientList[iClientH]->m_iConstructionPoint = 0;
	}
	else {
		if (m_pClientList[iClientH]->m_dwCrusadeGUID == m_dwCrusadeGUID) {
			if (m_pClientList[iClientH]->m_cVar == 1) {
				SendNotifyMsg(0, iClientH, Notify::Crusade, (uint32_t)m_bIsCrusadeMode, 0, 0, 0, -1);
			}
		}
		else {
			m_pClientList[iClientH]->m_dwCrusadeGUID = 0;
			m_pClientList[iClientH]->m_iWarContribution = 0;
			m_pClientList[iClientH]->m_dwCrusadeGUID = 0;
		}
	}

	// v1.42
	// 2002-7-4
	if (memcmp(m_pClientList[iClientH]->m_cMapName, "fight", 5) == 0) {
		std::snprintf(G_cTxt, sizeof(G_cTxt), "Char(%s)-Enter(%s) Observer(%d)", m_pClientList[iClientH]->m_cCharName, m_pClientList[iClientH]->m_cMapName, m_pClientList[iClientH]->m_bIsObserverMode);
		PutLogEventFileList(G_cTxt);
	}

	// Crusade
	SendNotifyMsg(0, iClientH, Notify::ConstructionPoint, m_pClientList[iClientH]->m_iConstructionPoint, m_pClientList[iClientH]->m_iWarContribution, 1, 0);

	// v2.15
	SendNotifyMsg(0, iClientH, Notify::GizonItemUpgradeLeft, m_pClientList[iClientH]->m_iGizonItemUpgradeLeft, 0, 0, 0);

	if (m_bIsHeldenianMode) {
		SendNotifyMsg(0, iClientH, Notify::HeldenianTeleport, 0, 0, 0, 0);
		if (m_bHeldenianInitiated) {
			SendNotifyMsg(0, iClientH, Notify::HeldenianStart, 0, 0, 0, 0);
		}
		else {
			m_pWarManager->UpdateHeldenianStatus();
		}
	}

	if (m_pClientList[iClientH]->m_iQuest != 0) {
		cQuestRemain = (m_pQuestManager->m_pQuestConfigList[m_pClientList[iClientH]->m_iQuest]->m_iMaxCount - m_pClientList[iClientH]->m_iCurQuestCount);
		SendNotifyMsg(0, iClientH, Notify::QuestCounter, cQuestRemain, 0, 0, 0);
		m_pQuestManager->_bCheckIsQuestCompleted(iClientH);
	}

	SendNotifyMsg(0, iClientH, Notify::Hunger, m_pClientList[iClientH]->m_iHungerStatus, 0, 0, 0);
	SendNotifyMsg(0, iClientH, Notify::SuperAttackLeft, 0, 0, 0, 0);
}

void CGame::ReleaseFollowMode(short sOwnerH, char cOwnerType)
{
	

	for(int i = 0; i < MaxNpcs; i++)
		if ((i != sOwnerH) && (m_pNpcList[i] != 0)) {
			if ((m_pNpcList[i]->m_cMoveType == MoveType::Follow) &&
				(m_pNpcList[i]->m_iFollowOwnerIndex == sOwnerH) &&
				(m_pNpcList[i]->m_cFollowOwnerType == cOwnerType)) {

				m_pNpcList[i]->m_cMoveType = MoveType::RandomWaypoint;
			}
		}
}





void CGame::Quit()
{
	

	G_bIsThread = false;
	Sleep(300);

	if (_lsock != 0) delete _lsock;

	for(int i = 0; i < MaxClients; i++)
		if (m_pClientList[i] != 0) delete m_pClientList[i];

	for(int i = 0; i < MaxNpcs; i++)
		if (m_pNpcList[i] != 0) delete m_pNpcList[i];

	for(int i = 0; i < MaxMaps; i++)
		if (m_pMapList[i] != 0) delete m_pMapList[i];

	for(int i = 0; i < MaxItemTypes; i++)
		if (m_pItemConfigList[i] != 0) delete m_pItemConfigList[i];

	for(int i = 0; i < MaxNpcTypes; i++)
		if (m_pNpcConfigList[i] != 0) delete m_pNpcConfigList[i];

	for(int i = 0; i < hb::shared::limits::MaxMagicType; i++)
		if (m_pMagicConfigList[i] != 0) delete m_pMagicConfigList[i];

	for(int i = 0; i < hb::shared::limits::MaxSkillType; i++)
		if (m_pSkillConfigList[i] != 0) delete m_pSkillConfigList[i];




	for(int i = 0; i < MaxNotifyMsgs; i++)
		if (m_pNoticeMsgList[i] != 0) delete m_pNoticeMsgList[i];




	//	for(int i = 0; i < DEF_MAXTELEPORTTYPE; i++)
	//	if (m_pTeleportConfigList[i] != 0) delete m_pTeleportConfigList[i];

	for(int i = 0; i < hb::shared::limits::MaxBuildItems; i++)
		if (m_pBuildItemList[i] != 0) delete m_pBuildItemList[i];

	if (m_pNoticementData != 0) delete m_pNoticementData;

}

uint32_t CGame::iGetLevelExp(int iLevel)
{
	return hb::shared::calc::CalculateLevelExp(iLevel);
}


/*****************************************************************
**---------------------------FUNCTION---------------------------**
**             void Game::CheckLevelUp(int iClientH)            **
**-------------------------DESCRIPTION--------------------------**
** Level-Up                                                     **
**  - Level +1                                                  **
**  - +3 Level Up Points                                        **
**  - Reset Next Level EXP                                      **
**  - Civilian Level Limit                                      **
**      Player mode switches to Combatant                       **
**      when the limit is reached                               **
**  - Majestic Points +1                                        **
**  - Reset Next Level EXP                                      **
**------------------------CREATION DATE-------------------------**
**                January 30, 2007; 3:06 PM; Dax                **
*****************************************************************/
bool CGame::bCheckLevelUp(int iClientH)
{

	if (m_pClientList[iClientH] == 0) return false;


	while (m_pClientList[iClientH]->m_iExp >= m_pClientList[iClientH]->m_iNextLevelExp)
	{
		if (m_pClientList[iClientH]->m_iLevel < m_iMaxLevel)
		{
			m_pClientList[iClientH]->m_iLevel++;
			m_pClientList[iClientH]->m_iLU_Pool += 3;
//			if ( (m_pClientList[iClientH]->m_cLU_Str + m_pClientList[iClientH]->m_cLU_Vit + m_pClientList[iClientH]->m_cLU_Dex + 
//	  		      m_pClientList[iClientH]->m_cLU_Int + m_pClientList[iClientH]->m_cLU_Mag + m_pClientList[iClientH]->m_cLU_Char) <= TotalLevelUpPoint) {


			if (m_pClientList[iClientH]->m_iStr > CharPointLimit)      m_pClientList[iClientH]->m_iStr = CharPointLimit;
			if (m_pClientList[iClientH]->m_iDex > CharPointLimit)      m_pClientList[iClientH]->m_iDex = CharPointLimit;
			if (m_pClientList[iClientH]->m_iVit > CharPointLimit)      m_pClientList[iClientH]->m_iVit = CharPointLimit;
			if (m_pClientList[iClientH]->m_iInt > CharPointLimit)      m_pClientList[iClientH]->m_iInt = CharPointLimit;
			if (m_pClientList[iClientH]->m_iMag > CharPointLimit)      m_pClientList[iClientH]->m_iMag = CharPointLimit;
			if (m_pClientList[iClientH]->m_iCharisma > CharPointLimit) m_pClientList[iClientH]->m_iCharisma = CharPointLimit;

			// New 17/05/2004
			if (m_pClientList[iClientH]->m_iLevel > 100)
				if (m_pClientList[iClientH]->m_bIsPlayerCivil)
					ForceChangePlayMode(iClientH, true);


			SendNotifyMsg(0, iClientH, Notify::SuperAttackLeft, 0, 0, 0, 0);
			SendNotifyMsg(0, iClientH, Notify::LevelUp, 0, 0, 0, 0);

			m_pClientList[iClientH]->m_iNextLevelExp = m_iLevelExpTable[m_pClientList[iClientH]->m_iLevel + 1]; //iGetLevelExp(m_pClientList[iClientH]->m_iLevel + 1);

			m_pItemManager->CalcTotalItemEffect(iClientH, -1, false);

			//std::snprintf(G_cTxt, sizeof(G_cTxt), "(!) Level up: Player (%s) Level (%d) Experience(%d) Next Level Experience(%d)", m_pClientList[iClientH]->m_cCharName,m_pClientList[iClientH]->m_iLevel, m_pClientList[iClientH]->m_iExp, m_pClientList[iClientH]->m_iNextLevelExp);
			//PutLogFileList(G_cTxt);
		}
		else {
			m_pClientList[iClientH]->m_iGizonItemUpgradeLeft++;

			m_pClientList[iClientH]->m_iNextLevelExp = m_iLevelExpTable[m_iMaxLevel + 1];
			m_pClientList[iClientH]->m_iExp = m_iLevelExpTable[m_iMaxLevel];
			//addon
			SendNotifyMsg(0, iClientH, Notify::GizonItemUpgradeLeft, m_pClientList[iClientH]->m_iGizonItemUpgradeLeft, 1, 0, 0);
		}

		SendNotifyMsg(0, iClientH, Notify::Exp, 0, 0, 0, 0);
	}

	return false;
}
// 2003-04-14      ...
void CGame::StateChangeHandler(int iClientH, char* pData, size_t dwMsgSize)
{
	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_bIsInitComplete == false) return;
	if (m_pClientList[iClientH]->m_iGizonItemUpgradeLeft <= 0) return;

	const auto* pkt = hb::net::PacketCast<hb::net::PacketRequestStateChange>(
		pData, sizeof(hb::net::PacketRequestStateChange));
	if (!pkt) return;

	int16_t cStr = pkt->str;
	int16_t cVit = pkt->vit;
	int16_t cDex = pkt->dex;
	int16_t cInt = pkt->intel;
	int16_t cMag = pkt->mag;
	int16_t cChar = pkt->chr;

	// All reduction values must be >= 0
	if (cStr < 0 || cVit < 0 || cDex < 0 || cInt < 0 || cMag < 0 || cChar < 0)
	{
		SendNotifyMsg(0, iClientH, Notify::StateChangeFailed, 0, 0, 0, 0);
		return;
	}

	// Total must be a positive multiple of 3 (TotalLevelUpPoint per majestic point)
	int iTotalReduction = cStr + cVit + cDex + cInt + cMag + cChar;
	if (iTotalReduction <= 0 || (iTotalReduction % TotalLevelUpPoint) != 0)
	{
		SendNotifyMsg(0, iClientH, Notify::StateChangeFailed, 0, 0, 0, 0);
		return;
	}

	int iMajesticCost = iTotalReduction / TotalLevelUpPoint;
	if (iMajesticCost > m_pClientList[iClientH]->m_iGizonItemUpgradeLeft)
	{
		SendNotifyMsg(0, iClientH, Notify::StateChangeFailed, 0, 0, 0, 0);
		return;
	}

	// Stats must equal the max-level formula (all points fully allocated)
	int iOldStr = m_pClientList[iClientH]->m_iStr;
	int iOldVit = m_pClientList[iClientH]->m_iVit;
	int iOldDex = m_pClientList[iClientH]->m_iDex;
	int iOldInt = m_pClientList[iClientH]->m_iInt;
	int iOldMag = m_pClientList[iClientH]->m_iMag;
	int iOldChar = m_pClientList[iClientH]->m_iCharisma;

	if (iOldStr + iOldVit + iOldDex + iOldInt + iOldMag + iOldChar != ((m_iMaxLevel - 1) * 3 + 70))
		return;

	// Each stat must stay >= 10 and <= CharPointLimit after reduction
	if ((iOldStr - cStr < 10) || (iOldStr - cStr > CharPointLimit)) { SendNotifyMsg(0, iClientH, Notify::StateChangeFailed, 0, 0, 0, 0); return; }
	if ((iOldVit - cVit < 10) || (iOldVit - cVit > CharPointLimit)) { SendNotifyMsg(0, iClientH, Notify::StateChangeFailed, 0, 0, 0, 0); return; }
	if ((iOldDex - cDex < 10) || (iOldDex - cDex > CharPointLimit)) { SendNotifyMsg(0, iClientH, Notify::StateChangeFailed, 0, 0, 0, 0); return; }
	if ((iOldInt - cInt < 10) || (iOldInt - cInt > CharPointLimit)) { SendNotifyMsg(0, iClientH, Notify::StateChangeFailed, 0, 0, 0, 0); return; }
	if ((iOldMag - cMag < 10) || (iOldMag - cMag > CharPointLimit)) { SendNotifyMsg(0, iClientH, Notify::StateChangeFailed, 0, 0, 0, 0); return; }
	if ((iOldChar - cChar < 10) || (iOldChar - cChar > CharPointLimit)) { SendNotifyMsg(0, iClientH, Notify::StateChangeFailed, 0, 0, 0, 0); return; }

	// Guild masters cannot reduce CHR below 20
	if (m_pClientList[iClientH]->m_iGuildRank == 0)
	{
		if (iOldChar - cChar < 20)
		{
			SendNotifyMsg(0, iClientH, Notify::StateChangeFailed, 0, 0, 0, 0);
			return;
		}
	}

	std::snprintf(G_cTxt, sizeof(G_cTxt), "(*) Majestic: Char(%s) cost(%d) Str(%d) Vit(%d) Dex(%d) Int(%d) Mag(%d) Chr(%d)",
		m_pClientList[iClientH]->m_cCharName, iMajesticCost, cStr, cVit, cDex, cInt, cMag, cChar);
	PutLogList(G_cTxt);

	// Apply reductions
	m_pClientList[iClientH]->m_iGizonItemUpgradeLeft -= iMajesticCost;
	m_pClientList[iClientH]->m_iLU_Pool += iTotalReduction;

	m_pClientList[iClientH]->m_iStr -= cStr;
	m_pClientList[iClientH]->m_iVit -= cVit;
	m_pClientList[iClientH]->m_iDex -= cDex;
	m_pClientList[iClientH]->m_iInt -= cInt;
	if (cInt > 0) m_pMagicManager->bCheckMagicInt(iClientH);
	m_pClientList[iClientH]->m_iMag -= cMag;
	m_pClientList[iClientH]->m_iCharisma -= cChar;

	// Recalculate derived stats
	m_pClientList[iClientH]->m_iHP = iGetMaxHP(iClientH);
	m_pClientList[iClientH]->m_iMP = iGetMaxMP(iClientH);
	m_pClientList[iClientH]->m_iSP = iGetMaxSP(iClientH);

	SendNotifyMsg(0, iClientH, Notify::StateChangeSuccess, 0, 0, 0, 0);
}

// 2003-04-21     ...
//  bool CGame::bCheckMagicInt(int iClientH)  //another retarded korean function
// desc		 ::     ... ...
// return value ::  true   // ....dumbass koreans
//  date		 :: 2003-04-21

void CGame::LevelUpSettingsHandler(int iClientH, char* pData, size_t dwMsgSize)
{
	int iTotalSetting = 0;

	int16_t cStr, cVit, cDex, cInt, cMag, cChar;

	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_bIsInitComplete == false) return;
	if (m_pClientList[iClientH]->m_iLU_Pool <= 0)
	{
		SendNotifyMsg(0, iClientH, Notify::SettingFailed, 0, 0, 0, 0);
		return;
	}

	const auto* req = hb::net::PacketCast<hb::net::PacketRequestLevelUpSettings>(pData, sizeof(hb::net::PacketRequestLevelUpSettings));
	if (!req) return;

	cStr = req->str;
	cVit = req->vit;
	cDex = req->dex;
	cInt = req->intel;
	cMag = req->mag;
	cChar = req->chr;

	if ((cStr + cVit + cDex + cInt + cMag + cChar) > m_pClientList[iClientH]->m_iLU_Pool)
	{
		SendNotifyMsg(0, iClientH, Notify::SettingFailed, 0, 0, 0, 0);
		return;
	}

	// Check if adding points would exceed the stat limit or be negative
	if ((m_pClientList[iClientH]->m_iStr + cStr > CharPointLimit) || (cStr < 0))
		return;

	if ((m_pClientList[iClientH]->m_iDex + cDex > CharPointLimit) || (cDex < 0))
		return;

	if ((m_pClientList[iClientH]->m_iInt + cInt > CharPointLimit) || (cInt < 0))
		return;

	if ((m_pClientList[iClientH]->m_iVit + cVit > CharPointLimit) || (cVit < 0))
		return;

	if ((m_pClientList[iClientH]->m_iMag + cMag > CharPointLimit) || (cMag < 0))
		return;

	if ((m_pClientList[iClientH]->m_iCharisma + cChar > CharPointLimit) || (cChar < 0))
		return;

	iTotalSetting = m_pClientList[iClientH]->m_iStr + m_pClientList[iClientH]->m_iDex + m_pClientList[iClientH]->m_iVit +
		m_pClientList[iClientH]->m_iInt + m_pClientList[iClientH]->m_iMag + m_pClientList[iClientH]->m_iCharisma;

	// (  +   >   ) ..  ..      ..
	if (iTotalSetting + m_pClientList[iClientH]->m_iLU_Pool > ((m_pClientList[iClientH]->m_iLevel - 1) * 3 + 70))
	{
		m_pClientList[iClientH]->m_iLU_Pool = (m_pClientList[iClientH]->m_iLevel - 1) * 3 + 70 - iTotalSetting;

		if (m_pClientList[iClientH]->m_iLU_Pool < 0)
			m_pClientList[iClientH]->m_iLU_Pool = 0;
		SendNotifyMsg(0, iClientH, Notify::SettingFailed, 0, 0, 0, 0);
		return;
	}

	// (  +    D >   )  ..
	if (iTotalSetting + (cStr + cVit + cDex + cInt + cMag + cChar) > ((m_pClientList[iClientH]->m_iLevel - 1) * 3 + 70))
	{
		SendNotifyMsg(0, iClientH, Notify::SettingFailed, 0, 0, 0, 0);
		return;
	}

	m_pClientList[iClientH]->m_iLU_Pool = m_pClientList[iClientH]->m_iLU_Pool - (cStr + cVit + cDex + cInt + cMag + cChar);

	m_pClientList[iClientH]->m_iStr += cStr;
	m_pClientList[iClientH]->m_iVit += cVit;
	m_pClientList[iClientH]->m_iDex += cDex;
	m_pClientList[iClientH]->m_iInt += cInt;
	m_pClientList[iClientH]->m_iMag += cMag;
	m_pClientList[iClientH]->m_iCharisma += cChar;

	// Recalculate item effects and weapon swing speed after stat changes
	m_pItemManager->CalcTotalItemEffect(iClientH, -1, false);

	// Recalculate weapon swing speed (m_status.iAttackDelay)
	short sWeaponIndex = m_pClientList[iClientH]->m_sItemEquipmentStatus[ToInt(EquipPos::RightHand)];
	if (sWeaponIndex == -1)
		sWeaponIndex = m_pClientList[iClientH]->m_sItemEquipmentStatus[ToInt(EquipPos::TwoHand)];
	if (sWeaponIndex != -1 && m_pClientList[iClientH]->m_pItemList[sWeaponIndex] != nullptr)
	{
		short sSpeed = m_pClientList[iClientH]->m_pItemList[sWeaponIndex]->m_cSpeed;
		sSpeed -= ((m_pClientList[iClientH]->m_iStr + m_pClientList[iClientH]->m_iAngelicStr) / 13);
		if (sSpeed < 0) sSpeed = 0;
		m_pClientList[iClientH]->m_status.iAttackDelay = static_cast<uint8_t>(sSpeed);
	}

	SendNotifyMsg(0, iClientH, Notify::SettingSuccess, 0, 0, 0, 0);

}




bool CGame::bCheckLimitedUser(int iClientH)
{
	if (m_pClientList[iClientH] == 0) return false;

	if ((memcmp(m_pClientList[iClientH]->m_cLocation, "NONE", 4) == 0) &&
		(m_pClientList[iClientH]->m_iExp >= m_iLevelExp20)) {
		// 20   19 .

		m_pClientList[iClientH]->m_iExp = m_iLevelExp20 - 1;
		SendNotifyMsg(0, iClientH, Notify::TravelerLimitedLevel, 0, 0, 0, 0);
		return true;
	}


	return false;
}

void CGame::RequestCivilRightHandler(int iClientH, char* pData)
{
	uint16_t wResult;
	int  iRet;

	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_bIsInitComplete == false) return;

	if (memcmp(m_pClientList[iClientH]->m_cLocation, "NONE", 4) != 0) wResult = 0;
	else wResult = 1;

	if (m_pClientList[iClientH]->m_iLevel < 5) wResult = 0;

	if (wResult == 1) {
		std::memset(m_pClientList[iClientH]->m_cLocation, 0, sizeof(m_pClientList[iClientH]->m_cLocation));
		strcpy(m_pClientList[iClientH]->m_cLocation, m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cLocationName);
	}

	// Side
	if (memcmp(m_pClientList[iClientH]->m_cLocation, "are", 3) == 0)
		m_pClientList[iClientH]->m_cSide = 1;

	if (memcmp(m_pClientList[iClientH]->m_cLocation, "elv", 3) == 0)
		m_pClientList[iClientH]->m_cSide = 2;

	hb::net::PacketResponseCivilRight pkt{};
	pkt.header.msg_id = MsgId::ResponseCivilRight;
	pkt.header.msg_type = wResult;
	std::memset(pkt.location, 0, sizeof(pkt.location));
	memcpy(pkt.location, m_pClientList[iClientH]->m_cLocation, sizeof(pkt.location));

	iRet = m_pClientList[iClientH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	switch (iRet) {
	case sock::Event::QueueFull:
	case sock::Event::SocketError:
	case sock::Event::CriticalError:
	case sock::Event::SocketClosed:
		DeleteClient(iClientH, true, true);
		return;
	}
	SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventMotion, Type::NullAction, 0, 0, 0);
}



// 05/21/2004 - Hypnotoad - send player to jail

// 05/17/2004 - Hypnotoad - register pk log
// 05/22/2004 - Hypnotoad - register in pk log
// 05/29/2004 - Hypnotoad - Limits some items from not dropping

int CGame::_iCalcMaxLoad(int iClientH)
{
	if (m_pClientList[iClientH] == 0) return 0;

	return ((m_pClientList[iClientH]->m_iStr + m_pClientList[iClientH]->m_iAngelicStr) * 500 + m_pClientList[iClientH]->m_iLevel * 500);
}




void CGame::RequestFullObjectData(int iClientH, char* pData)
{
	uint16_t wObjectID;
	int iRet;
	uint32_t dwTime;

	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_bIsInitComplete == false) return;

	const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(pData, sizeof(hb::net::PacketHeader));
	if (!header) return;
	wObjectID = header->msg_type;
	dwTime = GameClock::GetTimeMS();

	if ((wObjectID != m_pClientList[iClientH]->m_dwLastFullObjectId) ||
		(dwTime - m_pClientList[iClientH]->m_dwLastFullObjectTime) > 1000) {
		m_pClientList[iClientH]->m_dwLastFullObjectId = wObjectID;
		m_pClientList[iClientH]->m_dwLastFullObjectTime = dwTime;
	}

	if (hb::shared::object_id::IsPlayerID(wObjectID)) {
		if ((wObjectID == 0) || (wObjectID >= MaxClients)) return;
		if (m_pClientList[wObjectID] == 0) return;

		hb::net::PacketEventMotionPlayer pkt{};
		pkt.header.msg_id = MsgId::EventMotion;
		pkt.header.msg_type = Type::Stop;
		pkt.object_id = wObjectID;
		pkt.x = m_pClientList[wObjectID]->m_sX;
		pkt.y = m_pClientList[wObjectID]->m_sY;
		pkt.type = m_pClientList[wObjectID]->m_sType;
		pkt.dir = static_cast<uint8_t>(m_pClientList[wObjectID]->m_cDir);
		memcpy(pkt.name, m_pClientList[wObjectID]->m_cCharName, sizeof(pkt.name));
		pkt.appearance = m_pClientList[wObjectID]->m_appearance;

		{
			auto pktStatus = m_pClientList[wObjectID]->m_status;
			pktStatus.bPK = (m_pClientList[wObjectID]->m_iPKCount != 0) ? 1 : 0;
			pktStatus.bCitizen = (m_pClientList[wObjectID]->m_cSide != 0) ? 1 : 0;
			pktStatus.bAresden = (m_pClientList[wObjectID]->m_cSide == 1) ? 1 : 0;
			pktStatus.bHunter = m_pClientList[wObjectID]->m_bIsPlayerCivil ? 1 : 0;
			pktStatus.iRelationship = m_pCombatManager->GetPlayerRelationship(wObjectID, iClientH);
			pkt.status = pktStatus;
		}
		pkt.loc = m_pClientList[wObjectID]->m_bIsKilled ? 1 : 0;

		iRet = m_pClientList[iClientH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
	else {
		uint16_t npcIdx = hb::shared::object_id::ToNpcIndex(wObjectID);
		if ((npcIdx == 0) || (npcIdx >= MaxNpcs)) return;
		if (m_pNpcList[npcIdx] == 0) return;

		const uint16_t objectId = wObjectID;
		wObjectID = npcIdx;

		hb::net::PacketEventMotionNpc pkt{};
		pkt.header.msg_id = MsgId::EventMotion;
		pkt.header.msg_type = Type::Stop;
		pkt.object_id = objectId;
		pkt.x = m_pNpcList[wObjectID]->m_sX;
		pkt.y = m_pNpcList[wObjectID]->m_sY;
		pkt.config_id = m_pNpcList[wObjectID]->m_iNpcConfigId;
		pkt.dir = static_cast<uint8_t>(m_pNpcList[wObjectID]->m_cDir);
		memcpy(pkt.name, m_pNpcList[wObjectID]->m_cName, sizeof(pkt.name));
		pkt.appearance = m_pNpcList[wObjectID]->m_appearance;

		pkt.status = m_pNpcList[wObjectID]->m_status;
		pkt.loc = m_pNpcList[wObjectID]->m_bIsKilled ? 1 : 0;

		iRet = m_pClientList[iClientH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}

	switch (iRet) {
	case sock::Event::QueueFull:
	case sock::Event::SocketError:
	case sock::Event::CriticalError:
	case sock::Event::SocketClosed:
		DeleteClient(iClientH, true, true);
		return;
	}
}











void CGame::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case VK_F1:
		m_bF1pressed = true;
		break;
	case VK_F4:
		m_bF4pressed = true;
		break;
	case VK_F5:
		m_bF5pressed = true;
		break;
	case VK_F12:
		m_bF12pressed = true;
		break;
	}
}

void CGame::OnKeyUp(WPARAM wParam, LPARAM lParam)
{
	

	switch (wParam) {
	case VK_F2:
		break;

	case VK_F1:
		m_bF1pressed = false;
		break;
	case VK_F4:
		m_bF4pressed = false;
		break;
	case VK_F5:
		m_bF5pressed = false;
		break;
	case VK_F12:
		m_bF12pressed = false;
		break;

	case VK_F6:
		if (m_bF1pressed) {
			PutLogList("(!) Send server shutdown announcement1...");
			for(int i = 1; i < MaxClients; i++)
				if ((m_pClientList[i] != 0) && (m_pClientList[i]->m_bIsInitComplete)) {
					SendNotifyMsg(0, i, Notify::ServerShutdown, 1, 0, 0, 0);
				}
		}
		break;

	case VK_F7:
		if (m_bF1pressed) {
			PutLogList("(!) Send server shutdown announcement2...");
			for(int i = 1; i < MaxClients; i++)
				if ((m_pClientList[i] != 0) && (m_pClientList[i]->m_bIsInitComplete)) {
					SendNotifyMsg(0, i, Notify::ServerShutdown, 2, 0, 0, 0);
				}
		}
		break;

	case VK_F9:
		if ((m_bF1pressed)) {
			PutLogList("(!!!) Resume Crusade Mode...");
			m_pWarManager->LocalStartCrusadeMode(0);
		}
		break;

	case VK_F11:
		if ((m_bF1pressed)) {
			PutLogList("(!!!) ManualEndCrusadeMode: side 0");
			m_pWarManager->ManualEndCrusadeMode(0);
		}
		break;

#define VK_1 0x31
	case VK_1:
		if ((m_bF1pressed)) {
			GlobalUpdateConfigs(1);
		}
		break;

#define VK_2 0x32
	case VK_2:
		if ((m_bF1pressed)) {
			GlobalUpdateConfigs(2);
		}
		break;

#define VK_3 0x33
	case VK_3:
		if ((m_bF1pressed)) {
			GlobalUpdateConfigs(3);
		}
		break;

#define VK_4 0x34
	case VK_4:
		if ((m_bF1pressed)) {
			GlobalUpdateConfigs(1);
		}
		break;

		/*#define VK_A 0x41 // a key
		case VK_A:
			if ((m_bF1pressed )) {
				GlobalStartApocalypseMode();
			}
			break;*/

			/*#define VK_H 0x49 // H key
			case VK_H:
				if ((m_bF1pressed )) {
					m_pWarManager->GlobalStartHeldenianMode();
				}
				break;*/

				//Crusade Testcode
	case VK_HOME:
		if ((m_bF1pressed)) {
			m_pWarManager->GlobalStartCrusadeMode();
		}
		break;

	case VK_INSERT:
		m_pWarManager->_GrandMagicLaunchMsgSend(1, 1);
		m_pWarManager->MeteorStrikeMsgHandler(1);
		break;

	case VK_DELETE:
		m_pWarManager->_GrandMagicLaunchMsgSend(1, 2);
		m_pWarManager->MeteorStrikeMsgHandler(2);
		break;
	}
}

int CGame::iGetFollowerNumber(short sOwnerH, char cOwnerType)
{
	int iTotal;

	iTotal = 0;

	for(int i = 1; i < MaxNpcs; i++)
		if ((m_pNpcList[i] != 0) && (m_pNpcList[i]->m_cMoveType == MoveType::Follow)) {

			if ((m_pNpcList[i]->m_iFollowOwnerIndex == sOwnerH) && (m_pNpcList[i]->m_cFollowOwnerType == cOwnerType))
				iTotal++;
		}

	return iTotal;
}

void CGame::SendObjectMotionRejectMsg(int iClientH)
{
	int     iRet;

	m_pClientList[iClientH]->m_bIsMoveBlocked = true; // v2.171

	// Send motion reject response.
	hb::net::PacketResponseMotionReject pkt{};
	pkt.header.msg_id = MsgId::ResponseMotion;
	pkt.header.msg_type = Confirm::MotionReject;
	pkt.x = m_pClientList[iClientH]->m_sX;
	pkt.y = m_pClientList[iClientH]->m_sY;
	iRet = m_pClientList[iClientH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	switch (iRet) {
	case sock::Event::QueueFull:
	case sock::Event::SocketError:
	case sock::Event::CriticalError:
	case sock::Event::SocketClosed:
		// Socket error while sending motion reject.
		DeleteClient(iClientH, true, true);
		return;
	}
	return;
}











int CGame::iCalcTotalWeight(int iClientH)
{
	int iWeight;
	short sItemIndex;

	if (m_pClientList[iClientH] == 0) return 0;

	m_pClientList[iClientH]->m_iAlterItemDropIndex = -1;
	for (sItemIndex = 0; sItemIndex < hb::shared::limits::MaxItems; sItemIndex++)
		if (m_pClientList[iClientH]->m_pItemList[sItemIndex] != 0) {
			switch (m_pClientList[iClientH]->m_pItemList[sItemIndex]->GetItemEffectType()) {
			case ItemEffectType::AlterItemDrop:
				if (m_pClientList[iClientH]->m_pItemList[sItemIndex]->m_wCurLifeSpan > 0) {
					m_pClientList[iClientH]->m_iAlterItemDropIndex = sItemIndex;
				}
				break;
			}
		}

	iWeight = 0;
	for(int i = 0; i < hb::shared::limits::MaxItems; i++)
		if (m_pClientList[iClientH]->m_pItemList[i] != 0) {

			iWeight += m_pItemManager->iGetItemWeight(m_pClientList[iClientH]->m_pItemList[i], m_pClientList[iClientH]->m_pItemList[i]->m_dwCount);
		}

	m_pClientList[iClientH]->m_iCurWeightLoad = iWeight;

	return iWeight;
}

void CGame::CheckAndNotifyPlayerConnection(int iClientH, char* pMsg, uint32_t dwSize)
{
	char   seps[] = "= \t\r\n";
	char* token, cName[hb::shared::limits::CharNameLen], cBuff[256], cPlayerLocation[120];
	

	if (m_pClientList[iClientH] == 0) return;
	if (dwSize <= 0) return;

	std::memset(cPlayerLocation, 0, sizeof(cPlayerLocation));
	std::memset(cName, 0, sizeof(cName));
	std::memset(cBuff, 0, sizeof(cBuff));
	memcpy(cBuff, pMsg, dwSize);

	token = strtok(cBuff, seps);
	token = strtok(NULL, seps);

	if (token == 0) {
		return;
	}

	if (strlen(token) > hb::shared::limits::CharNameLen - 1)
		memcpy(cName, token, hb::shared::limits::CharNameLen - 1);
	else memcpy(cName, token, strlen(token));

	// cName     .
	for(int i = 1; i < MaxClients; i++)
		if ((m_pClientList[i] != 0) && (_strnicmp(cName, m_pClientList[i]->m_cCharName, hb::shared::limits::CharNameLen - 1) == 0)) {
			SendNotifyMsg(0, iClientH, Notify::PlayerOnGame, 0, 0, 0, m_pClientList[i]->m_cCharName, 0, 0, 0, 0, 0, 0, cPlayerLocation);

			return;
		}

}


void CGame::SetPlayerProfile(int iClientH, char* pMsg, size_t dwMsgSize)
{
	char cTemp[256];
	


	if (m_pClientList[iClientH] == 0) return;
	if ((dwMsgSize - 7) <= 0) return;

	std::memset(cTemp, 0, sizeof(cTemp));
	memcpy(cTemp, (pMsg + 7), dwMsgSize - 7);

	for(int i = 0; i < 256; i++)
		if (cTemp[i] == ' ') cTemp[i] = '_';

	cTemp[255] = 0;

	std::memset(m_pClientList[iClientH]->m_cProfile, 0, sizeof(m_pClientList[iClientH]->m_cProfile));
	strcpy(m_pClientList[iClientH]->m_cProfile, cTemp);
}

void CGame::GetPlayerProfile(int iClientH, char* pMsg, size_t dwMsgSize)
{
	char   seps[] = "= \t\r\n";
	char* token, cName[hb::shared::limits::CharNameLen], cBuff[256], cBuff2[500];
	

	if (m_pClientList[iClientH] == 0) return;
	if ((dwMsgSize) <= 0) return;

	std::memset(cName, 0, sizeof(cName));
	std::memset(cBuff, 0, sizeof(cBuff));
	memcpy(cBuff, pMsg, dwMsgSize);

	token = strtok(cBuff, seps);
	token = strtok(NULL, seps);

	if (token != 0) {
		// token
		if (strlen(token) > hb::shared::limits::CharNameLen - 1)
			memcpy(cName, token, hb::shared::limits::CharNameLen - 1);
		else memcpy(cName, token, strlen(token));

		for(int i = 1; i < MaxClients; i++)
			if ((m_pClientList[i] != 0) && (_strnicmp(m_pClientList[i]->m_cCharName, cName, hb::shared::limits::CharNameLen - 1) == 0)) {

				std::memset(cBuff2, 0, sizeof(cBuff2));
				std::snprintf(cBuff2, sizeof(cBuff2), "%s Profile: %s", cName, m_pClientList[i]->m_cProfile);
				SendNotifyMsg(0, iClientH, Notify::PlayerProfile, 0, 0, 0, cBuff2);

				return;
			}
		SendNotifyMsg(0, iClientH, Notify::PlayerNotOnGame, 0, 0, 0, cName);
	}

	return;
}

void CGame::___RestorePlayerCharacteristics(int iClientH)
{
	int iStr, iDex, iInt, iVit, iMag, iCharisma;
	int iOriginalPoint, iCurPoint, iVerifyPoint, iToBeRestoredPoint;
	int iMax, iA, iB;
	bool bFlag;
	char cTxt[120];
	return;
	if (m_pClientList[iClientH] == 0) return;

	iStr = m_pClientList[iClientH]->m_iStr;
	iDex = m_pClientList[iClientH]->m_iDex;
	iInt = m_pClientList[iClientH]->m_iInt;
	iVit = m_pClientList[iClientH]->m_iVit;
	iMag = m_pClientList[iClientH]->m_iMag;
	iCharisma = m_pClientList[iClientH]->m_iCharisma;


	iCurPoint = m_pClientList[iClientH]->m_iStr + m_pClientList[iClientH]->m_iInt +
		m_pClientList[iClientH]->m_iVit + m_pClientList[iClientH]->m_iDex +
		m_pClientList[iClientH]->m_iMag + m_pClientList[iClientH]->m_iCharisma;

	iOriginalPoint = (m_pClientList[iClientH]->m_iLevel - 1) * 3 + 70;

	iToBeRestoredPoint = iOriginalPoint - iCurPoint;

	if (iToBeRestoredPoint == 0) return;

	if (iToBeRestoredPoint > 0) {
		// iToBeRestoredPoint   .
		while (1) {
			bFlag = false;

			if ((iToBeRestoredPoint > 0) && (m_pClientList[iClientH]->m_iStr < 10)) {
				m_pClientList[iClientH]->m_iStr++;
				iToBeRestoredPoint--;
				bFlag = true;
			}
			if ((iToBeRestoredPoint > 0) && (m_pClientList[iClientH]->m_iMag < 10)) {
				m_pClientList[iClientH]->m_iMag++;
				iToBeRestoredPoint--;
				bFlag = true;
			}
			if ((iToBeRestoredPoint > 0) && (m_pClientList[iClientH]->m_iInt < 10)) {
				m_pClientList[iClientH]->m_iInt++;
				iToBeRestoredPoint--;
				bFlag = true;
			}
			if ((iToBeRestoredPoint > 0) && (m_pClientList[iClientH]->m_iDex < 10)) {
				m_pClientList[iClientH]->m_iDex++;
				iToBeRestoredPoint--;
				bFlag = true;
			}
			if ((iToBeRestoredPoint > 0) && (m_pClientList[iClientH]->m_iVit < 10)) {
				m_pClientList[iClientH]->m_iVit++;
				iToBeRestoredPoint--;
				bFlag = true;
			}
			if ((iToBeRestoredPoint > 0) && (m_pClientList[iClientH]->m_iCharisma < 10)) {
				m_pClientList[iClientH]->m_iCharisma++;
				iToBeRestoredPoint--;
				bFlag = true;
			}

			if (bFlag == false)          break;
			if (iToBeRestoredPoint <= 0) break;
		}

		// iMax, Str iMax/2   .
		iMax = m_pClientList[iClientH]->m_cSkillMastery[5];

		if (m_pClientList[iClientH]->m_iStr < (iMax / 2)) {

			while (1) {
				if ((iToBeRestoredPoint > 0) && (m_pClientList[iClientH]->m_iStr < (iMax / 2))) {
					m_pClientList[iClientH]->m_iStr++;
					iToBeRestoredPoint--;
				}

				if (m_pClientList[iClientH]->m_iStr == (iMax / 2)) break;
				if (iToBeRestoredPoint <= 0) break;
			}
		}

		// iMax, Dex iMax/2   .
		iA = m_pClientList[iClientH]->m_cSkillMastery[7];
		iB = m_pClientList[iClientH]->m_cSkillMastery[8];
		if (iA > iB)
			iMax = iA;
		else iMax = iB;
		iA = m_pClientList[iClientH]->m_cSkillMastery[9];
		if (iA > iMax) iMax = iA;
		iA = m_pClientList[iClientH]->m_cSkillMastery[6];
		if (iA > iMax) iMax = iA;


		if (m_pClientList[iClientH]->m_iDex < (iMax / 2)) {

			while (1) {
				if ((iToBeRestoredPoint > 0) && (m_pClientList[iClientH]->m_iDex < (iMax / 2))) {
					m_pClientList[iClientH]->m_iDex++;
					iToBeRestoredPoint--;
				}

				if (m_pClientList[iClientH]->m_iDex == (iMax / 2)) break;
				if (iToBeRestoredPoint <= 0) break;
			}
		}

		// iMax, Int iMax/2   .
		iMax = m_pClientList[iClientH]->m_cSkillMastery[19];

		if (m_pClientList[iClientH]->m_iInt < (iMax / 2)) {

			while (1) {
				if ((iToBeRestoredPoint > 0) && (m_pClientList[iClientH]->m_iInt < (iMax / 2))) {
					m_pClientList[iClientH]->m_iInt++;
					iToBeRestoredPoint--;
				}

				if (m_pClientList[iClientH]->m_iInt == (iMax / 2)) break;
				if (iToBeRestoredPoint <= 0) break;
			}
		}

		// iMax, Mag iMax/2   .
		iA = m_pClientList[iClientH]->m_cSkillMastery[3];
		iB = m_pClientList[iClientH]->m_cSkillMastery[4];
		if (iA > iB)
			iMax = iA;
		else iMax = iB;

		if (m_pClientList[iClientH]->m_iMag < (iMax / 2)) {

			while (1) {
				if ((iToBeRestoredPoint > 0) && (m_pClientList[iClientH]->m_iMag < (iMax / 2))) {
					m_pClientList[iClientH]->m_iMag++;
					iToBeRestoredPoint--;
				}

				if (m_pClientList[iClientH]->m_iMag == (iMax / 2)) break;
				if (iToBeRestoredPoint <= 0) break;
			}
		}

		while (iToBeRestoredPoint != 0) {
			switch (iDice(1, 6)) {
			case 1:
				if (m_pClientList[iClientH]->m_iStr < CharPointLimit) {
					m_pClientList[iClientH]->m_iStr++;
					iToBeRestoredPoint--;
				}
				break;
			case 2:
				if (m_pClientList[iClientH]->m_iVit < CharPointLimit) {
					m_pClientList[iClientH]->m_iVit++;
					iToBeRestoredPoint--;
				}
				break;
			case 3:
				if (m_pClientList[iClientH]->m_iDex < CharPointLimit) {
					m_pClientList[iClientH]->m_iDex++;
					iToBeRestoredPoint--;
				}
				break;
			case 4:
				if (m_pClientList[iClientH]->m_iMag < CharPointLimit) {
					m_pClientList[iClientH]->m_iMag++;
					iToBeRestoredPoint--;
				}
				break;
			case 5:
				if (m_pClientList[iClientH]->m_iInt < CharPointLimit) {
					m_pClientList[iClientH]->m_iInt++;
					iToBeRestoredPoint--;
				}
				break;
			case 6:
				if (m_pClientList[iClientH]->m_iCharisma < CharPointLimit) {
					m_pClientList[iClientH]->m_iCharisma++;
					iToBeRestoredPoint--;
				}
				break;
			}
		}

		iVerifyPoint = m_pClientList[iClientH]->m_iStr + m_pClientList[iClientH]->m_iInt +
			m_pClientList[iClientH]->m_iVit + m_pClientList[iClientH]->m_iDex +
			m_pClientList[iClientH]->m_iMag + m_pClientList[iClientH]->m_iCharisma;

		if (iVerifyPoint != iOriginalPoint) {
			std::snprintf(cTxt, sizeof(cTxt), "(T_T) RestorePlayerCharacteristics(Minor) FAIL! Player(%s)-(%d/%d)", m_pClientList[iClientH]->m_cCharName, iVerifyPoint, iOriginalPoint);
			PutLogList(cTxt);

			m_pClientList[iClientH]->m_iStr = iStr;
			m_pClientList[iClientH]->m_iDex = iDex;
			m_pClientList[iClientH]->m_iInt = iInt;
			m_pClientList[iClientH]->m_iVit = iVit;
			m_pClientList[iClientH]->m_iMag = iMag;
			m_pClientList[iClientH]->m_iCharisma = iCharisma;
		}
		else {
			std::snprintf(cTxt, sizeof(cTxt), "(^o^) RestorePlayerCharacteristics(Minor) SUCCESS! Player(%s)-(%d/%d)", m_pClientList[iClientH]->m_cCharName, iVerifyPoint, iOriginalPoint);
			PutLogList(cTxt);
		}
	}
	else {
		// .   . iToBeRestoredPoint !

		while (1) {
			bFlag = false;
			if (m_pClientList[iClientH]->m_iStr > CharPointLimit) {
				bFlag = true;
				m_pClientList[iClientH]->m_iStr--;
				iToBeRestoredPoint++;
			}

			if (m_pClientList[iClientH]->m_iDex > CharPointLimit) {
				bFlag = true;
				m_pClientList[iClientH]->m_iDex--;
				iToBeRestoredPoint++;
			}

			if (m_pClientList[iClientH]->m_iVit > CharPointLimit) {
				bFlag = true;
				m_pClientList[iClientH]->m_iVit--;
				iToBeRestoredPoint++;
			}

			if (m_pClientList[iClientH]->m_iInt > CharPointLimit) {
				bFlag = true;
				m_pClientList[iClientH]->m_iInt--;
				iToBeRestoredPoint++;
			}

			if (m_pClientList[iClientH]->m_iMag > CharPointLimit) {
				bFlag = true;
				m_pClientList[iClientH]->m_iMag--;
				iToBeRestoredPoint++;
			}

			if (m_pClientList[iClientH]->m_iCharisma > CharPointLimit) {
				bFlag = true;
				m_pClientList[iClientH]->m_iCharisma--;
				iToBeRestoredPoint++;
			}

			if (bFlag == false)	break;
			if (iToBeRestoredPoint >= 0) break;
		}

		if (iToBeRestoredPoint < 0) {
			while (iToBeRestoredPoint != 0) {
				switch (iDice(1, 6)) {
				case 1:
					if (m_pClientList[iClientH]->m_iStr > 10) {
						m_pClientList[iClientH]->m_iStr--;
						iToBeRestoredPoint++;
					}
					break;
				case 2:
					if (m_pClientList[iClientH]->m_iVit > 10) {
						m_pClientList[iClientH]->m_iVit--;
						iToBeRestoredPoint++;
					}
					break;
				case 3:
					if (m_pClientList[iClientH]->m_iDex > 10) {
						m_pClientList[iClientH]->m_iDex--;
						iToBeRestoredPoint++;
					}
					break;
				case 4:
					if (m_pClientList[iClientH]->m_iMag > 10) {
						m_pClientList[iClientH]->m_iMag--;
						iToBeRestoredPoint++;
					}
					break;
				case 5:
					if (m_pClientList[iClientH]->m_iInt > 10) {
						m_pClientList[iClientH]->m_iInt--;
						iToBeRestoredPoint++;
					}
					break;
				case 6:
					if (m_pClientList[iClientH]->m_iCharisma > 10) {
						m_pClientList[iClientH]->m_iCharisma--;
						iToBeRestoredPoint++;
					}
					break;
				}
			}
		}
		else {
			while (iToBeRestoredPoint != 0) {
				switch (iDice(1, 6)) {
				case 1:
					if (m_pClientList[iClientH]->m_iStr < CharPointLimit) {
						m_pClientList[iClientH]->m_iStr++;
						iToBeRestoredPoint--;
					}
					break;
				case 2:
					if (m_pClientList[iClientH]->m_iVit < CharPointLimit) {
						m_pClientList[iClientH]->m_iVit++;
						iToBeRestoredPoint--;
					}
					break;
				case 3:
					if (m_pClientList[iClientH]->m_iDex < CharPointLimit) {
						m_pClientList[iClientH]->m_iDex++;
						iToBeRestoredPoint--;
					}
					break;
				case 4:
					if (m_pClientList[iClientH]->m_iMag < CharPointLimit) {
						m_pClientList[iClientH]->m_iMag++;
						iToBeRestoredPoint--;
					}
					break;
				case 5:
					if (m_pClientList[iClientH]->m_iInt < CharPointLimit) {
						m_pClientList[iClientH]->m_iInt++;
						iToBeRestoredPoint--;
					}
					break;
				case 6:
					if (m_pClientList[iClientH]->m_iCharisma < CharPointLimit) {
						m_pClientList[iClientH]->m_iCharisma++;
						iToBeRestoredPoint--;
					}
					break;
				}
			}
		}

		iVerifyPoint = m_pClientList[iClientH]->m_iStr + m_pClientList[iClientH]->m_iInt +
			m_pClientList[iClientH]->m_iVit + m_pClientList[iClientH]->m_iDex +
			m_pClientList[iClientH]->m_iMag + m_pClientList[iClientH]->m_iCharisma;

		if (iVerifyPoint != iOriginalPoint) {
			std::snprintf(cTxt, sizeof(cTxt), "(T_T) RestorePlayerCharacteristics(Over) FAIL! Player(%s)-(%d/%d)", m_pClientList[iClientH]->m_cCharName, iVerifyPoint, iOriginalPoint);
			PutLogList(cTxt);

		}
		else {
			std::snprintf(cTxt, sizeof(cTxt), "(^o^) RestorePlayerCharacteristics(Over) SUCCESS! Player(%s)-(%d/%d)", m_pClientList[iClientH]->m_cCharName, iVerifyPoint, iOriginalPoint);
			PutLogList(cTxt);
		}
	}
}


int CGame::_iGetPlayerNumberOnSpot(short dX, short dY, char cMapIndex, char cRange)
{
	int iSum = 0;
	short sOwnerH;
	char  cOwnerType;

	for(int ix = dX - cRange; ix <= dX + cRange; ix++)
		for(int iy = dY - cRange; iy <= dY + cRange; iy++) {
			m_pMapList[cMapIndex]->GetOwner(&sOwnerH, &cOwnerType, ix, iy);
			if ((sOwnerH != 0) && (cOwnerType == hb::shared::owner_class::Player))
				iSum++;
		}

	return iSum;
}



void CGame::CheckDayOrNightMode()
{
	SYSTEMTIME SysTime;
	char cPrevMode;

	// DEBUG: Force night mode for testing light effects
	// Set to 0 to use normal day/night cycle, 1 for forced day, 2 for forced night
	constexpr int DEBUG_FORCE_TIME = 0;

	cPrevMode = m_cDayOrNight;

	GetLocalTime(&SysTime);
	if (SysTime.wMinute >= m_iNighttimeDuration)
		m_cDayOrNight = 2;
	else m_cDayOrNight = 1;

	if (cPrevMode != m_cDayOrNight) {
		for(int i = 1; i < MaxClients; i++)
			if ((m_pClientList[i] != 0) && (m_pClientList[i]->m_bIsInitComplete)) {
				if ((m_pClientList[i]->m_cMapIndex >= 0) &&
					(m_pMapList[m_pClientList[i]->m_cMapIndex] != 0) &&
					(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_bIsFixedDayMode == false))
					SendNotifyMsg(0, i, Notify::TimeChange, m_cDayOrNight, 0, 0, 0);
			}
	}
}


void CGame::SetPlayerReputation(int iClientH, char* pMsg, char cValue, size_t dwMsgSize)
{
	char   seps[] = "= \t\r\n";
	char* token, cName[hb::shared::limits::CharNameLen], cBuff[256];
	

	if (m_pClientList[iClientH] == 0) return;
	if ((dwMsgSize) <= 0) return;
	if (m_pClientList[iClientH]->m_iLevel < 40) return;

	if ((m_pClientList[iClientH]->m_iTimeLeft_Rating != 0) || (m_pClientList[iClientH]->m_iPKCount != 0)) {
		SendNotifyMsg(0, iClientH, Notify::CannotRating, m_pClientList[iClientH]->m_iTimeLeft_Rating, 0, 0, 0);
		return;
	}
	else if (memcmp(m_pClientList[iClientH]->m_cLocation, "NONE", 4) == 0) {
		SendNotifyMsg(0, iClientH, Notify::CannotRating, 0, 0, 0, 0);
		return;
	}

	std::memset(cName, 0, sizeof(cName));
	std::memset(cBuff, 0, sizeof(cBuff));
	memcpy(cBuff, pMsg, dwMsgSize);

	token = strtok(cBuff, seps);
	token = strtok(NULL, seps);

	if (token != 0) {
		// token
		if (strlen(token) > hb::shared::limits::CharNameLen - 1)
			memcpy(cName, token, hb::shared::limits::CharNameLen - 1);
		else memcpy(cName, token, strlen(token));

		for(int i = 1; i < MaxClients; i++)
			if ((m_pClientList[i] != 0) && (_strnicmp(m_pClientList[i]->m_cCharName, cName, hb::shared::limits::CharNameLen - 1) == 0)) {

				if (i != iClientH) {
					if (cValue == 0)
						m_pClientList[i]->m_iRating--;
					else if (cValue == 1)
						m_pClientList[i]->m_iRating++;

					if (m_pClientList[i]->m_iRating > 500)  m_pClientList[i]->m_iRating = 500;
					if (m_pClientList[i]->m_iRating < -500) m_pClientList[i]->m_iRating = -500;
					m_pClientList[iClientH]->m_iTimeLeft_Rating = 20 * 60;

					SendNotifyMsg(0, i, Notify::RatingPlayer, cValue, 0, 0, cName);
					SendNotifyMsg(0, iClientH, Notify::RatingPlayer, cValue, 0, 0, cName);

					return;
				}
			}
		SendNotifyMsg(0, iClientH, Notify::PlayerNotOnGame, 0, 0, 0, cName);
	}

	return;
}

bool CGame::bReadNotifyMsgListFile(char* cFn)
{
	FILE* pFile;
	HANDLE hFile;
	uint32_t  dwFileSize;
	char* cp, * token, cReadMode;
	char seps[] = "=\t\n;";

	cReadMode = 0;
	m_iTotalNoticeMsg = 0;

	hFile = CreateFile(cFn, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
	dwFileSize = GetFileSize(hFile, 0);
	if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);

	pFile = fopen(cFn, "rt");
	if (pFile == 0) {
		PutLogList("(!) Notify Message list file not found!...");
		return false;
	}
	else {
		PutLogList("(!) Reading Notify Message list file...");
		cp = new char[dwFileSize + 2];
		std::memset(cp, 0, dwFileSize + 2);
		fread(cp, dwFileSize, 1, pFile);

		token = strtok(cp, seps);
		while (token != 0) {

			if (cReadMode != 0) {
				switch (cReadMode) {
				case 1:
					for(int i = 0; i < MaxNotifyMsgs; i++)
						if (m_pNoticeMsgList[i] == 0) {
							m_pNoticeMsgList[i] = new class CMsg;
							m_pNoticeMsgList[i]->bPut(0, token, strlen(token), 0, 0);
							m_iTotalNoticeMsg++;
							goto LNML_NEXTSTEP1;
						}
				LNML_NEXTSTEP1:
					cReadMode = 0;
					break;
				}
			}
			else {
				if (memcmp(token, "notify_msg", 10) == 0) cReadMode = 1;
			}

			token = strtok(NULL, seps);
		}

		delete cp;
	}
	if (pFile != 0) fclose(pFile);

	return true;
}
void CGame::NoticeHandler()
{
	char  cTemp, cBuffer[1000], cKey;
	size_t dwSize = 0;
	uint32_t dwTime = GameClock::GetTimeMS();
	int iMsgIndex, iTemp;

	if (m_iTotalNoticeMsg <= 1) return;

	if ((dwTime - m_dwNoticeTime) > NoticeTime) {
		m_dwNoticeTime = dwTime;
		do {
			iMsgIndex = iDice(1, m_iTotalNoticeMsg) - 1;
		} while (iMsgIndex == m_iPrevSendNoticeMsg);

		m_iPrevSendNoticeMsg = iMsgIndex;

		std::memset(cBuffer, 0, sizeof(cBuffer));
		if (m_pNoticeMsgList[iMsgIndex] != 0) {
			m_pNoticeMsgList[iMsgIndex]->Get(&cTemp, cBuffer, &dwSize, &iTemp, &cKey);
		}

		for(int i = 1; i < MaxClients; i++)
			if (m_pClientList[i] != 0) {
				SendNotifyMsg(0, i, Notify::NoticeMsg, 0, 0, 0, cBuffer);
			}
	}
}


void CGame::ResponseSavePlayerDataReplyHandler(char* pData, size_t dwMsgSize)
{
	char* cp, cCharName[hb::shared::limits::CharNameLen];


	std::memset(cCharName, 0, sizeof(cCharName));

	const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(pData, sizeof(hb::net::PacketHeader));
	if (!header) return;
	cp = (char*)(pData + sizeof(hb::net::PacketHeader));
	memcpy(cCharName, cp, hb::shared::limits::CharNameLen - 1);

	for(int i = 0; i < MaxClients; i++)
		if (m_pClientList[i] != 0) {
			if (_strnicmp(m_pClientList[i]->m_cCharName, cCharName, hb::shared::limits::CharNameLen - 1) == 0) {
				SendNotifyMsg(0, i, Notify::ServerChange, 0, 0, 0, 0);
			}
		}
}

void CGame::CalcExpStock(int iClientH)
{
	bool bIsLevelUp;
	CItem* pItem;

	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_bIsInitComplete == false) return;
	if (m_pClientList[iClientH]->m_iExpStock <= 0) return;
	//if ((m_pClientList[iClientH]->m_iLevel >= m_iMaxLevel) && (m_pClientList[iClientH]->m_iExp >= m_iLevelExpTable[m_iMaxLevel])) return;

	if (m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cType == smap::MapType::NoPenaltyNoReward) {
		m_pClientList[iClientH]->m_iExpStock = 0;
		return;
	}

	m_pClientList[iClientH]->m_iExp += m_pClientList[iClientH]->m_iExpStock;
	m_pClientList[iClientH]->m_iAutoExpAmount += m_pClientList[iClientH]->m_iExpStock;
	m_pClientList[iClientH]->m_iExpStock = 0;

	if (bCheckLimitedUser(iClientH) == false) {
		SendNotifyMsg(0, iClientH, Notify::Exp, 0, 0, 0, 0);
	}
	bIsLevelUp = bCheckLevelUp(iClientH);

	if ((bIsLevelUp) && (m_pClientList[iClientH]->m_iLevel <= 5)) {
		// Gold .  1~5 100 Gold .
		pItem = new CItem;
		if (m_pItemManager->_bInitItemAttr(pItem, hb::shared::item::ItemId::Gold) == false) {
			delete pItem;
			return;
		}
		else pItem->m_dwCount = (uint32_t)100000;
		m_pItemManager->bAddItem(iClientH, pItem, 0);
	}

	if ((bIsLevelUp) && (m_pClientList[iClientH]->m_iLevel > 5) && (m_pClientList[iClientH]->m_iLevel <= 20)) {
		// Gold .  5~20 300 Gold .
		pItem = new CItem;
		if (m_pItemManager->_bInitItemAttr(pItem, hb::shared::item::ItemId::Gold) == false) {
			delete pItem;
			return;
		}
		else pItem->m_dwCount = (uint32_t)100000;
		m_pItemManager->bAddItem(iClientH, pItem, 0);
	}
}

void CGame::___RestorePlayerRating(int iClientH)
{
	if (m_pClientList[iClientH] == 0) return;

	if (m_pClientList[iClientH]->m_iRating < -10000) m_pClientList[iClientH]->m_iRating = 0;
	if (m_pClientList[iClientH]->m_iRating > 10000) m_pClientList[iClientH]->m_iRating = 0;
}

int CGame::iGetExpLevel(uint32_t iExp)
{
	

	for(int i = 1; i < 1000; i++)
		if ((m_iLevelExpTable[i] <= iExp) && (m_iLevelExpTable[i + 1] > iExp)) return i;

	return 0;
}



int CGame::_iCalcPlayerNum(char cMapIndex, short dX, short dY, char cRadius)
{
	int iRet;
	class CTile* pTile;

	if ((cMapIndex < 0) || (cMapIndex > MaxMaps)) return 0;
	if (m_pMapList[cMapIndex] == 0) return 0;

	iRet = 0;
	for(int ix = dX - cRadius; ix <= dX + cRadius; ix++)
		for(int iy = dY - cRadius; iy <= dY + cRadius; iy++) {
			if ((ix < 0) || (ix >= m_pMapList[cMapIndex]->m_sSizeX) ||
				(iy < 0) || (iy >= m_pMapList[cMapIndex]->m_sSizeY)) {
			}
			else {
				pTile = (class CTile*)(m_pMapList[cMapIndex]->m_pTile + ix + iy * m_pMapList[cMapIndex]->m_sSizeY);
				if ((pTile->m_sOwner != 0) && (pTile->m_cOwnerClass == hb::shared::owner_class::Player))
					iRet++;
			}
		}

	return iRet;
}


void CGame::WhetherProcessor()
{
	char cPrevMode;
	int j;
	uint32_t dwTime;

	dwTime = GameClock::GetTimeMS();

	for(int i = 0; i < MaxMaps; i++) {
		if ((m_pMapList[i] != 0) && (m_pMapList[i]->m_bIsFixedDayMode == false)) {
			cPrevMode = m_pMapList[i]->m_cWhetherStatus;
			if (m_pMapList[i]->m_cWhetherStatus != 0) {
				if ((dwTime - m_pMapList[i]->m_dwWhetherStartTime) > m_pMapList[i]->m_dwWhetherLastTime)
					m_pMapList[i]->m_cWhetherStatus = 0;
			}
			else {
				if (iDice(1, 300) == 13) {
					m_pMapList[i]->m_cWhetherStatus = static_cast<char>(iDice(1, 3)); //This looks better or else we only get snow :(
					//m_pMapList[i]->m_cWhetherStatus = iDice(1,3)+3; <- This original code looks fucked
					m_pMapList[i]->m_dwWhetherStartTime = dwTime;
					m_pMapList[i]->m_dwWhetherLastTime = 60000 * 3 + 60000 * iDice(1, 7);
				}
			}

			if (m_pMapList[i]->m_bIsSnowEnabled) {
				m_pMapList[i]->m_cWhetherStatus = static_cast<char>(iDice(1, 3) + 3);
				m_pMapList[i]->m_dwWhetherStartTime = dwTime;
				m_pMapList[i]->m_dwWhetherLastTime = 60000 * 3 + 60000 * iDice(1, 7);
			}

			if (cPrevMode != m_pMapList[i]->m_cWhetherStatus) {
				for (j = 1; j < MaxClients; j++)
					if ((m_pClientList[j] != 0) && (m_pClientList[j]->m_bIsInitComplete) && (m_pClientList[j]->m_cMapIndex == i))
						SendNotifyMsg(0, j, Notify::WhetherChange, m_pMapList[i]->m_cWhetherStatus, 0, 0, 0);
			}
		}
	} //for Loop
}



/*********************************************************************************************************************
**  int CGame::iGetWhetherMagicBonusEffect(short sType, char cWheatherStatus)										**
**  description			:: checks for a weather bonus when magic is cast											**
**  last updated		:: November 20, 2004; 10:34 PM; Hypnotoad													**
**	return value		:: int																						**
*********************************************************************************************************************/


int CGame::iGetMapIndex(char* pMapName)
{
	int iMapIndex;
	char cTmpName[256];

	std::memset(cTmpName, 0, sizeof(cTmpName));
	strcpy(cTmpName, pMapName);

	iMapIndex = -1;
	for(int i = 0; i < MaxMaps; i++)
		if (m_pMapList[i] != 0) {
			if (memcmp(m_pMapList[i]->m_cName, pMapName, 10) == 0)
				iMapIndex = i;
		}

	return iMapIndex;
}


int CGame::_iForcePlayerDisconect(int iNum)
{
	int iCnt;

	iCnt = 0;
	for(int i = 1; i < MaxClients; i++)
		if (m_pClientList[i] != 0) {
			if (m_pClientList[i]->m_bIsInitComplete)
				DeleteClient(i, true, true);
			else DeleteClient(i, false, false);
			iCnt++;
			if (iCnt >= iNum) break;
		}

	return iCnt;
}

void CGame::SpecialEventHandler()
{
	uint32_t dwTime;

	dwTime = GameClock::GetTimeMS();

	if ((dwTime - m_dwSpecialEventTime) < SpecialEventTime) return; // SpecialEventTime
	m_dwSpecialEventTime = dwTime;
	m_bIsSpecialEventTime = true;

	switch (iDice(1, 180)) {
	case 98: m_cSpecialEventType = 2; break; // 30 1 1/30
	default: m_cSpecialEventType = 1; break;
	}
}

void CGame::ToggleSafeAttackModeHandler(int iClientH) //v1.1
{
	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_bIsInitComplete == false) return;
	if (m_pClientList[iClientH]->m_bIsKilled) return;

	if (m_pClientList[iClientH]->m_bIsSafeAttackMode)
		m_pClientList[iClientH]->m_bIsSafeAttackMode = false;
	else m_pClientList[iClientH]->m_bIsSafeAttackMode = true;

	SendNotifyMsg(0, iClientH, Notify::SafeAttackMode, 0, 0, 0, 0);
}


void CGame::ForceDisconnectAccount(char* pAccountName, uint16_t wCount)
{
	

	for(int i = 1; i < MaxClients; i++)
		if ((m_pClientList[i] != 0) && (_strnicmp(m_pClientList[i]->m_cAccountName, pAccountName, hb::shared::limits::AccountNameLen - 1) == 0)) {
			std::snprintf(G_cTxt, sizeof(G_cTxt), "<%d> Force disconnect account: CharName(%s) AccntName(%s) Count(%d)", i, m_pClientList[i]->m_cCharName, m_pClientList[i]->m_cAccountName, wCount);
			PutLogList(G_cTxt);

			//DeleteClient(i, true, true);

			//v1.4312
			SendNotifyMsg(0, i, Notify::ForceDisconn, wCount, 0, 0, 0);
		}
}

bool CGame::bOnClose()
{
	if (m_bIsServerShutdowned == false)
	{
#ifdef _WIN32
		if (MessageBox(0, "Player data not saved! Shutdown server now?", m_cRealmName, MB_ICONEXCLAMATION | MB_YESNO) == IDYES) return true;
		return false;
#else
		return false;
#endif
	}
	else return true;

	return false;
}

// 05/24/2004 - Hypnotoad - Hammer and Wand train to 100% fixed













//Hero Code by Zabuza











// New 14/05/2004




int CGame::iGetMaxHP(int iClientH)
{
	if (m_pClientList[iClientH] == 0) return 0;

	int iRet = hb::shared::calc::CalculateMaxHP(
		m_pClientList[iClientH]->m_iVit,
		m_pClientList[iClientH]->m_iLevel,
		m_pClientList[iClientH]->m_iStr,
		m_pClientList[iClientH]->m_iAngelicStr);

	// Apply side effect reduction if active
	if (m_pClientList[iClientH]->m_iSideEffect_MaxHPdown != 0)
		iRet = iRet - (iRet / m_pClientList[iClientH]->m_iSideEffect_MaxHPdown);

	return iRet;
}

int CGame::iGetMaxMP(int iClientH)
{
	if (m_pClientList[iClientH] == 0) return 0;

	return hb::shared::calc::CalculateMaxMP(
		m_pClientList[iClientH]->m_iMag,
		m_pClientList[iClientH]->m_iAngelicMag,
		m_pClientList[iClientH]->m_iLevel,
		m_pClientList[iClientH]->m_iInt,
		m_pClientList[iClientH]->m_iAngelicInt);
}

int CGame::iGetMaxSP(int iClientH)
{
	if (m_pClientList[iClientH] == 0) return 0;

	return hb::shared::calc::CalculateMaxSP(
		m_pClientList[iClientH]->m_iStr,
		m_pClientList[iClientH]->m_iAngelicStr,
		m_pClientList[iClientH]->m_iLevel);
}

void CGame::GetMapInitialPoint(int iMapIndex, short* pX, short* pY, char* pPlayerLocation)
{
	int iTotalPoint = 0;

	hb::shared::geometry::GamePoint pList[smap::MaxInitialPoint];

	if (m_pMapList[iMapIndex] == 0)
		return;
	for (int i = 0; i < smap::MaxInitialPoint; i++)
		if (m_pMapList[iMapIndex]->m_pInitialPoint[i].x != -1) {
			pList[iTotalPoint].x = m_pMapList[iMapIndex]->m_pInitialPoint[i].x;
			pList[iTotalPoint].y = m_pMapList[iMapIndex]->m_pInitialPoint[i].y;
			iTotalPoint++;
		}
	if (iTotalPoint == 0) return;
	int iIndex = 0;
	if ((pPlayerLocation != 0) && (memcmp(pPlayerLocation, "NONE", 4) != 0))
		iIndex = iDice(1, iTotalPoint) - 1;

	*pX = static_cast<short>(pList[iIndex].x);
	*pY = static_cast<short>(pList[iIndex].y);
}


// MODERNIZED: New function that polls login client socket instead of handling window messages
void CGame::OnLoginClientSocketEvent(int iLoginClientH)
{
	int iRet;

	if (iLoginClientH < 0 || iLoginClientH >= MaxClientLoginSock) return;

	auto p = _lclients[iLoginClientH];
	if (p == 0) return;

	iRet = p->_sock->Poll();

	switch (iRet) {
	case sock::Event::UnsentDataSendComplete:
		break;
	case sock::Event::ConnectionEstablish:
		break;

	case sock::Event::ReadComplete:
		OnClientLoginRead(iLoginClientH);
		break;

	case sock::Event::Block:
		break;

	case sock::Event::ConfirmCodeNotMatch:
		std::snprintf(G_cTxt, sizeof(G_cTxt), "<%d> Confirmcode Login notmatch!", iLoginClientH);
		PutLogList(G_cTxt);
		DeleteLoginClient(iLoginClientH);
		break;
	case sock::Event::MsgSizeTooLarge:
	case sock::Event::SocketError:
	case sock::Event::SocketClosed:
		DeleteLoginClient(iLoginClientH);
		break;
	}
}

void CGame::OnSubLogSocketEvent(UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT iTmp;
	int iLogSockH, iRet;

	iTmp = WM_USER_BOT_ACCEPT;
	iLogSockH = message - iTmp - 1;

	auto p = _lclients[iLogSockH];
	if (p == 0) return;

	iRet = p->_sock->Poll();

	switch (iRet) {
	case sock::Event::UnsentDataSendComplete:
		break;
	case sock::Event::ConnectionEstablish:
		break;

	case sock::Event::ReadComplete:
		OnClientLoginRead(iLogSockH);
		break;

	case sock::Event::Block:
		break;

	case sock::Event::ConfirmCodeNotMatch:
		std::snprintf(G_cTxt, sizeof(G_cTxt), "<%d> Confirmcode Login notmatch!", iLogSockH);
		PutLogList(G_cTxt);
		DeleteLoginClient(iLogSockH);
		break;
	case sock::Event::MsgSizeTooLarge:
	case sock::Event::SocketError:
	case sock::Event::SocketClosed:
		break;
	}
}

LoginClient::~LoginClient()
{
	if (_sock)
		delete _sock;
}

void CGame::OnClientLoginRead(int h)
{
	char* pData, cKey;
	size_t  dwMsgSize;

	if (_lclients[h] == 0) return;

	pData = _lclients[h]->_sock->pGetRcvDataPointer(&dwMsgSize, &cKey);

	if (bPutMsgQuene(Source::LogServer, pData, dwMsgSize, h, cKey) == false) {
		PutLogList("@@@@@@ CRITICAL ERROR in MsgQuene!!! @@@@@@");
	}
}

void CGame::DeleteLoginClient(int h)
{
	if (!_lclients[h])
		return;

	_lclients[h]->_timeout_tm = GameClock::GetTimeMS();
	_lclients_disconn.push_back(_lclients[h]);
	//delete _lclients[h];
	_lclients[h] = nullptr;
}



// 3.51 - 05/17/2004 - Hypnotoad/[KLKS] - Monster Special Abilities
char CGame::_cGetSpecialAbility(int iKindSA)
{
	char cSA;

	switch (iKindSA) {
	case 1:
		// Slime, Orc, Orge, WereWolf, YB-, Rabbit, Mountain-Giant, Stalker, Hellclaw, 
		// Wyvern, Fire-Wyvern, Barlog, Tentocle, Centaurus, Giant-Lizard, Minotaurus,
		// Abaddon, Claw-Turtle, Giant-Cray-Fish, Giant-Plant, MasterMage-Orc, Nizie,
		// Tigerworm
		switch (iDice(1, 2)) {
		case 1: cSA = 3; break; // Anti-Physical Damage
		case 2: cSA = 4; break; // Anti-Magic Damage
		}
		break;

	case 2:
		// Giant-Ant, Cat, Giant-Frog, 
		switch (iDice(1, 3)) {
		case 1: cSA = 3; break; // Anti-Physical Damage
		case 2: cSA = 4; break; // Anti-Magic Damage
		case 3: cSA = 5; break; // Poisonous
		}
		break;

	case 3:
		// Zombie, Scorpion, Amphis, Troll, Dark-Elf
		switch (iDice(1, 4)) {
		case 1: cSA = 3; break; // Anti-Physical Damage
		case 2: cSA = 4; break; // Anti-Magic Damage
		case 3: cSA = 5; break; // Poisonous
		case 4: cSA = 6; break; // Critical Poisonous
		}
		break;

	case 4:
		// no linked Npc
		switch (iDice(1, 3)) {
		case 1: cSA = 3; break; // Anti-Physical Damage
		case 2: cSA = 4; break; // Anti-Magic Damage
		case 3: cSA = 7; break; // Explosive
		}
		break;

	case 5:
		// Stone-Golem, Clay-Golem, Beholder, Cannibal-Plant, Rudolph, DireBoar
		switch (iDice(1, 4)) {
		case 1: cSA = 3; break; // Anti-Physical Damage
		case 2: cSA = 4; break; // Anti-Magic Damage
		case 3: cSA = 7; break; // Explosive
		case 4: cSA = 8; break; // Critical-Explosive
		}
		break;

	case 6:
		// no linked Npc
		switch (iDice(1, 3)) {
		case 1: cSA = 3; break; // Anti-Physical Damage
		case 2: cSA = 4; break; // Anti-Magic Damage
		case 3: cSA = 5; break; // Poisonous
		}
		break;

	case 7:
		// Orc-Mage, Unicorn
		switch (iDice(1, 3)) {
		case 1: cSA = 1; break; // Clairvoyant
		case 2: cSA = 2; break; // Distruction of Magic Protection
		case 3: cSA = 4; break; // Anti-Magic Damage
		}
		break;

	case 8:
		// Frost, Ice-Golem, Ettin, Gagoyle, Demon, Liche, Hellbound, Cyclops, 
		// Skeleton
		switch (iDice(1, 5)) {
		case 1: cSA = 1; break; // Clairvoyant
		case 2: cSA = 2; break; // Distruction of Magic Protection
		case 3: cSA = 4; break; // Anti-Magic Damage
		case 4: cSA = 3; break; // Anti-Physical Damage
		case 5: cSA = 8; break; // Critical-Explosive
		}
		break;

	case 9:
		// no linked Npc
		cSA = static_cast<char>(iDice(1, 8)); // All abilities available
		break;
	}

	return cSA;
}

void CGame::CheckSpecialEvent(int iClientH)
{
	CItem* pItem;
	char  cItemName[hb::shared::limits::ItemNameLen];
	int   iEraseReq;

	if (m_pClientList[iClientH] == 0) return;

	if (m_pClientList[iClientH]->m_iSpecialEventID == 200081) {

		if (m_pClientList[iClientH]->m_iLevel < 11) {
			m_pClientList[iClientH]->m_iSpecialEventID = 0;
			return;
		}

		std::memset(cItemName, 0, sizeof(cItemName));
		strcpy(cItemName, "MemorialRing");

		pItem = new CItem;
		if (m_pItemManager->_bInitItemAttr(pItem, cItemName) == false) {
			delete pItem;
		}
		else {
			if (m_pItemManager->_bAddClientItemList(iClientH, pItem, &iEraseReq)) {
				if (m_pClientList[iClientH]->m_iCurWeightLoad < 0) m_pClientList[iClientH]->m_iCurWeightLoad = 0;

				// testcode  .
				std::snprintf(G_cTxt, sizeof(G_cTxt), "(*) Get MemorialRing  : Char(%s)", m_pClientList[iClientH]->m_cCharName);
				PutLogFileList(G_cTxt);

				pItem->SetTouchEffectType(TouchEffectType::UniqueOwner);
				pItem->m_sTouchEffectValue1 = m_pClientList[iClientH]->m_sCharIDnum1;
				pItem->m_sTouchEffectValue2 = m_pClientList[iClientH]->m_sCharIDnum2;
				pItem->m_sTouchEffectValue3 = m_pClientList[iClientH]->m_sCharIDnum3;
				pItem->m_cItemColor = 9;

				m_pClientList[iClientH]->m_iSpecialEventID = 0;
			}
		}
	}
}





void CGame::RequestNoticementHandler(int iClientH, char* pData)
{
	char* cp;
	int iRet, iClientSize;

	if (m_pClientList[iClientH] == 0) return;
	if (m_dwNoticementDataSize < 10) return;

	const auto* pkt = hb::net::PacketCast<hb::net::PacketRequestNoticement>(
		pData, sizeof(hb::net::PacketRequestNoticement));
	if (!pkt) return;
	iClientSize = pkt->value;

	if (iClientSize != m_dwNoticementDataSize) {
		cp = new char[m_dwNoticementDataSize + 2 + sizeof(hb::net::PacketHeader)];
		std::memset(cp, 0, m_dwNoticementDataSize + 2 + sizeof(hb::net::PacketHeader));
		memcpy((cp + sizeof(hb::net::PacketHeader)), m_pNoticementData, m_dwNoticementDataSize);

		{
			auto* header = reinterpret_cast<hb::net::PacketResponseNoticementHeader*>(cp);
			header->header.msg_id = MsgId::ResponseNoticement;
			header->header.msg_type = MsgType::Reject;
		}

		iRet = m_pClientList[iClientH]->m_pXSock->iSendMsg(cp, m_dwNoticementDataSize + 2 + sizeof(hb::net::PacketHeader));

		delete cp;
	}
	else {
		hb::net::PacketResponseNoticementHeader pkt{};
		pkt.header.msg_id = MsgId::ResponseNoticement;
		pkt.header.msg_type = MsgType::Confirm;
		iRet = m_pClientList[iClientH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
	}
}

void CGame::RequestNoticementHandler(int iClientH)
{
	DWORD lpNumberOfBytesRead;

	if (m_pClientList[iClientH] == 0) return;

	HANDLE hFile = CreateFile("GameConfigs/Noticement.txt", GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
	uint32_t dwFileSize = GetFileSize(hFile, 0);
	if (dwFileSize == -1) {
		return;
	}

	std::memset(G_cData50000, 0, sizeof(G_cData50000));

	SetFilePointer(hFile, 0, 0, FILE_BEGIN);

	ReadFile(hFile, G_cData50000 + sizeof(hb::net::PacketHeader), dwFileSize, &lpNumberOfBytesRead, 0);
	CloseHandle(hFile);

	{
		auto* header = reinterpret_cast<hb::net::PacketResponseNoticementHeader*>(G_cData50000);
		header->header.msg_id = MsgId::ResponseNoticement;
		header->header.msg_type = MsgType::Confirm;
	}

	int iRet = m_pClientList[iClientH]->m_pXSock->iSendMsg(G_cData50000, dwFileSize + 2 + sizeof(hb::net::PacketHeader));

	switch (iRet) {
	case sock::Event::QueueFull:
	case sock::Event::SocketError:
	case sock::Event::CriticalError:
	case sock::Event::SocketClosed:
		return;
	}
}


void CGame::RequestCheckAccountPasswordHandler(char* pData, size_t dwMsgSize)
{
	int iLevel;
	char cAccountName[11], cAccountPassword[hb::shared::limits::AccountPassLen];

	const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(pData, sizeof(hb::net::PacketHeader));
	if (!header) return;

	const auto& payload = *reinterpret_cast<const hb::net::TestLogPayload*>(pData + sizeof(hb::net::PacketHeader));

	std::memset(cAccountName, 0, sizeof(cAccountName));
	std::memset(cAccountPassword, 0, sizeof(cAccountPassword));

	std::memcpy(cAccountName, payload.account_name, hb::shared::limits::AccountNameLen - 1);
	std::memcpy(cAccountPassword, payload.account_password, hb::shared::limits::AccountPassLen - 1);
	iLevel = payload.level;

	for(int i = 0; i < MaxClients; i++)
		if ((m_pClientList[i] != 0) && (_stricmp(m_pClientList[i]->m_cAccountName, cAccountName) == 0)) {
			if ((strcmp(m_pClientList[i]->m_cAccountPassword, cAccountPassword) != 0) || (m_pClientList[i]->m_iLevel != iLevel)) {
				std::snprintf(G_cTxt, sizeof(G_cTxt), "(TestLog) Error! Account(%s)-Level(%d) password(or level) mismatch! Disconnect.", cAccountName, iLevel);
				PutLogList(G_cTxt);
				DeleteClient(i, false, true);
				return;
			}
		}
}



int CGame::iRequestPanningMapDataRequest(int iClientH, char* pData)
{
	char cDir, mapData[3000];
	short dX, dY;
	int   iRet, iSize;

	if (m_pClientList[iClientH] == 0) return 0;
	if (m_pClientList[iClientH]->m_bIsObserverMode == false) return 0;
	if (m_pClientList[iClientH]->m_bIsKilled) return 0;
	if (m_pClientList[iClientH]->m_bIsInitComplete == false) return 0;

	dX = m_pClientList[iClientH]->m_sX;
	dY = m_pClientList[iClientH]->m_sY;

	const auto* req = hb::net::PacketCast<hb::net::PacketRequestPanning>(pData, sizeof(hb::net::PacketRequestPanning));
	if (!req) return 0;
	cDir = static_cast<char>(req->dir);
	if ((cDir <= 0) || (cDir > 8)) return 0;

	hb::shared::direction::ApplyOffset(cDir, dX, dY);

	m_pClientList[iClientH]->m_sX = dX;
	m_pClientList[iClientH]->m_sY = dY;
	m_pClientList[iClientH]->m_cDir = cDir;

	iSize = iComposeMoveMapData((short)(dX - hb::shared::view::CenterX), (short)(dY - hb::shared::view::CenterY), iClientH, cDir, mapData);

	hb::net::PacketWriter writer;
	writer.Reserve(sizeof(hb::net::PacketResponsePanningHeader) + iSize);

	auto* pkt = writer.Append<hb::net::PacketResponsePanningHeader>();
	pkt->header.msg_id = MsgId::ResponsePanning;
	pkt->header.msg_type = Confirm::MoveConfirm;
	pkt->x = static_cast<int16_t>(dX - hb::shared::view::CenterX);
	pkt->y = static_cast<int16_t>(dY - hb::shared::view::CenterY);
	pkt->dir = static_cast<uint8_t>(cDir);

	writer.AppendBytes(mapData, iSize);

	iRet = m_pClientList[iClientH]->m_pXSock->iSendMsg(writer.Data(), static_cast<int>(writer.Size()));
	switch (iRet) {
	case sock::Event::QueueFull:
	case sock::Event::SocketError:
	case sock::Event::CriticalError:
	case sock::Event::SocketClosed:
		DeleteClient(iClientH, true, true);
		return 0;
	}

	return 1;
}

void CGame::RequestRestartHandler(int iClientH)
{
	char  cTmpMap[32];

	if (m_pClientList[iClientH] == 0) return;

	if (m_pClientList[iClientH]->m_bIsKilled) {

		strcpy(cTmpMap, m_pClientList[iClientH]->m_cMapName);
		std::memset(m_pClientList[iClientH]->m_cMapName, 0, sizeof(m_pClientList[iClientH]->m_cMapName));

		if (strcmp(m_pClientList[iClientH]->m_cLocation, "NONE") == 0) {
			// default .
			strcpy(m_pClientList[iClientH]->m_cMapName, "default");
		}
		else {
			if ((strcmp(m_pClientList[iClientH]->m_cLocation, "aresden") == 0) || (strcmp(m_pClientList[iClientH]->m_cLocation, "arehunter") == 0)) {
				if (m_bIsCrusadeMode) {
					if (m_pClientList[iClientH]->m_iDeadPenaltyTime > 0) {
						std::memset(m_pClientList[iClientH]->m_cLockedMapName, 0, sizeof(m_pClientList[iClientH]->m_cLockedMapName));
						strcpy(m_pClientList[iClientH]->m_cLockedMapName, "aresden");
						m_pClientList[iClientH]->m_iLockedMapTime = 60 * 5;
						m_pClientList[iClientH]->m_iDeadPenaltyTime = 60 * 10; // v2.04
					}
					else {
						memcpy(m_pClientList[iClientH]->m_cMapName, "resurr1", 7);
						m_pClientList[iClientH]->m_iDeadPenaltyTime = 60 * 10;
					}
				}
				// v2.16 2002-5-31
				if (strcmp(cTmpMap, "elvine") == 0) {
					memcpy(m_pClientList[iClientH]->m_cMapName, "elvjail", 7);
					strcpy(m_pClientList[iClientH]->m_cLockedMapName, "elvjail");
					m_pClientList[iClientH]->m_iLockedMapTime = 60 * 3;
				}
				else if (m_pClientList[iClientH]->m_iLevel > 80)
					memcpy(m_pClientList[iClientH]->m_cMapName, "resurr1", 7);
				else memcpy(m_pClientList[iClientH]->m_cMapName, "arefarm", 7);
			}
			else {
				if (m_bIsCrusadeMode) {
					if (m_pClientList[iClientH]->m_iDeadPenaltyTime > 0) {
						std::memset(m_pClientList[iClientH]->m_cLockedMapName, 0, sizeof(m_pClientList[iClientH]->m_cLockedMapName));
						strcpy(m_pClientList[iClientH]->m_cLockedMapName, "elvine");
						m_pClientList[iClientH]->m_iLockedMapTime = 60 * 5;
						m_pClientList[iClientH]->m_iDeadPenaltyTime = 60 * 10; // v2.04
					}
					else {
						memcpy(m_pClientList[iClientH]->m_cMapName, "resurr2", 7);
						m_pClientList[iClientH]->m_iDeadPenaltyTime = 60 * 10;
					}
				}
				if (strcmp(cTmpMap, "aresden") == 0) {
					memcpy(m_pClientList[iClientH]->m_cMapName, "arejail", 7);
					strcpy(m_pClientList[iClientH]->m_cLockedMapName, "arejail");
					m_pClientList[iClientH]->m_iLockedMapTime = 60 * 3;

				}
				else if (m_pClientList[iClientH]->m_iLevel > 80)
					memcpy(m_pClientList[iClientH]->m_cMapName, "resurr2", 7);
				else memcpy(m_pClientList[iClientH]->m_cMapName, "elvfarm", 7);
			}
		}

		m_pClientList[iClientH]->m_bIsKilled = false;
		m_pClientList[iClientH]->m_iHP = iGetMaxHP(iClientH);
		m_pClientList[iClientH]->m_iHungerStatus = 100;

		std::memset(cTmpMap, 0, sizeof(cTmpMap));
		strcpy(cTmpMap, m_pClientList[iClientH]->m_cMapName);
		// !!! RequestTeleportHandler m_cMapName
		RequestTeleportHandler(iClientH, "2   ", cTmpMap, -1, -1);
	}
}


void CGame::RequestShopContentsHandler(int iClientH, char* pData)
{
	if (m_pClientList[iClientH] == 0) return;
	if (!m_bIsShopDataAvailable) {
		// No shop data configured
		return;
	}

	const auto* req = hb::net::PacketCast<hb::net::PacketShopRequest>(pData, sizeof(hb::net::PacketShopRequest));
	if (!req) return;

	int16_t npcType = req->npcType;

	// Look up shop ID for this NPC type
	auto mappingIt = m_NpcShopMappings.find(static_cast<int>(npcType));
	if (mappingIt == m_NpcShopMappings.end()) {
		// No shop configured for this NPC type
		char logMsg[128];
		std::snprintf(logMsg, sizeof(logMsg), "(!) Shop request for NPC type %d - no shop mapping found", npcType);
		PutLogList(logMsg);
		return;
	}

	int shopId = mappingIt->second;

	// Get shop data
	auto shopIt = m_ShopData.find(shopId);
	if (shopIt == m_ShopData.end() || shopIt->second.itemIds.empty()) {
		// Shop exists in mapping but has no items
		char logMsg[128];
		std::snprintf(logMsg, sizeof(logMsg), "(!) Shop request for NPC type %d, shop %d - no items found", npcType, shopId);
		PutLogList(logMsg);
		return;
	}

	const ShopData& shop = shopIt->second;
	uint16_t itemCount = static_cast<uint16_t>(shop.itemIds.size());
	if (itemCount > hb::net::MAX_SHOP_ITEMS) {
		itemCount = hb::net::MAX_SHOP_ITEMS;
	}

	// Build response packet
	// Header + array of int16_t item IDs
	size_t packetSize = sizeof(hb::net::PacketShopResponseHeader) + (itemCount * sizeof(int16_t));
	char* cData = new char[packetSize];
	std::memset(cData, 0, packetSize);

	auto* resp = reinterpret_cast<hb::net::PacketShopResponseHeader*>(cData);
	resp->header.msg_id = MSGID_RESPONSE_SHOP_CONTENTS;
	resp->header.msg_type = MsgType::Confirm;
	resp->npcType = npcType;
	resp->shopId = static_cast<int16_t>(shopId);
	resp->itemCount = itemCount;

	// Copy item IDs after header
	int16_t* itemIds = reinterpret_cast<int16_t*>(cData + sizeof(hb::net::PacketShopResponseHeader));
	for (uint16_t i = 0; i < itemCount; i++) {
		itemIds[i] = shop.itemIds[i];
	}

	// Send to client
	m_pClientList[iClientH]->m_pXSock->iSendMsg(cData, static_cast<uint32_t>(packetSize));

	char logMsg[128];
	std::snprintf(logMsg, sizeof(logMsg), "(!) Sent shop contents: NPC type %d, shop %d, %d items", npcType, shopId, itemCount);
	PutLogList(logMsg);

	delete[] cData;
}

void CGame::JoinPartyHandler(int iClientH, int iV1, const char* pMemberName)
{
	char cData[120]{};

	if (m_pClientList[iClientH] == 0) return;

	switch (iV1) {
	case 0:
		RequestDeletePartyHandler(iClientH);
		break;

	case 1:
		//testcode
		std::snprintf(G_cTxt, sizeof(G_cTxt), "Join Party Req: %s(%d) ID(%d) Stat(%d) ReqJoinH(%d) ReqJoinName(%s)", m_pClientList[iClientH]->m_cCharName, iClientH,
			m_pClientList[iClientH]->m_iPartyID, m_pClientList[iClientH]->m_iPartyStatus, m_pClientList[iClientH]->m_iReqJoinPartyClientH,
			m_pClientList[iClientH]->m_cReqJoinPartyName);
		PutLogList(G_cTxt);

		if ((m_pClientList[iClientH]->m_iPartyID != 0) || (m_pClientList[iClientH]->m_iPartyStatus != PartyStatus::Null)) {
			SendNotifyMsg(0, iClientH, Notify::Party, 7, 0, 0, 0);
			m_pClientList[iClientH]->m_iReqJoinPartyClientH = 0;
			std::memset(m_pClientList[iClientH]->m_cReqJoinPartyName, 0, sizeof(m_pClientList[iClientH]->m_cReqJoinPartyName));
			m_pClientList[iClientH]->m_iPartyStatus = PartyStatus::Null;
			//testcode
			PutLogList("Join Party Reject (1)");
			return;
		}

		for(int i = 1; i < MaxClients; i++)
			if ((m_pClientList[i] != 0) && (_stricmp(m_pClientList[i]->m_cCharName, pMemberName) == 0)) {
				if (m_pClientList[i]->m_appearance.bIsWalking) {
					SendNotifyMsg(0, iClientH, Notify::Party, 7, 0, 0, 0);
					//testcode
					PutLogList("Join Party Reject (2)");
				}
				else if (m_pClientList[i]->m_cSide != m_pClientList[iClientH]->m_cSide) {
					SendNotifyMsg(0, iClientH, Notify::Party, 7, 0, 0, 0);
					//testcode
					PutLogList("Join Party Reject (3)");
				}
				else if (m_pClientList[i]->m_iPartyStatus == PartyStatus::Processing) {
					SendNotifyMsg(0, iClientH, Notify::Party, 7, 0, 0, 0);
					//testcode
					PutLogList("Join Party Reject (4)");
					//testcode
					std::snprintf(G_cTxt, sizeof(G_cTxt), "Party join reject(2) ClientH:%d ID:%d JoinName:%s", i, m_pClientList[i]->m_iPartyID, m_pClientList[i]->m_cReqJoinPartyName);
					PutLogList(G_cTxt);

					m_pClientList[iClientH]->m_iReqJoinPartyClientH = 0;
					std::memset(m_pClientList[iClientH]->m_cReqJoinPartyName, 0, sizeof(m_pClientList[iClientH]->m_cReqJoinPartyName));
					m_pClientList[iClientH]->m_iPartyStatus = PartyStatus::Null;
				}
				else {
					m_pClientList[i]->m_iReqJoinPartyClientH = iClientH;
					std::memset(m_pClientList[i]->m_cReqJoinPartyName, 0, sizeof(m_pClientList[i]->m_cReqJoinPartyName));
					strcpy(m_pClientList[i]->m_cReqJoinPartyName, m_pClientList[iClientH]->m_cCharName);
					SendNotifyMsg(0, i, Notify::QueryJoinParty, 0, 0, 0, m_pClientList[iClientH]->m_cCharName);

					m_pClientList[iClientH]->m_iReqJoinPartyClientH = i;
					std::memset(m_pClientList[iClientH]->m_cReqJoinPartyName, 0, sizeof(m_pClientList[iClientH]->m_cReqJoinPartyName));
					strcpy(m_pClientList[iClientH]->m_cReqJoinPartyName, m_pClientList[i]->m_cCharName);
					m_pClientList[iClientH]->m_iPartyStatus = PartyStatus::Processing;
				}
				return;
			}
		break;

	case 2:
		if (m_pClientList[iClientH]->m_iPartyStatus == PartyStatus::Confirm) {
			std::memset(cData, 0, sizeof(cData));
			hb::net::PartyOpPayloadWithId partyOp{};
			partyOp.msg_id = ServerMsgId::PartyOperation;
			partyOp.op_type = 6;
			partyOp.client_h = static_cast<uint16_t>(iClientH);
			std::memcpy(partyOp.name, m_pClientList[iClientH]->m_cCharName, sizeof(partyOp.name));
			partyOp.party_id = static_cast<uint16_t>(m_pClientList[iClientH]->m_iPartyID);
			std::memcpy(cData, &partyOp, sizeof(partyOp));
			PartyOperation(cData);
		}
		break;
	}
}





void CGame::ActivateSpecialAbilityHandler(int iClientH)
{
	uint32_t dwTime = GameClock::GetTimeMS();

	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_iSpecialAbilityTime != 0) return;
	if (m_pClientList[iClientH]->m_iSpecialAbilityType == 0) return;
	if (m_pClientList[iClientH]->m_bIsSpecialAbilityEnabled) return;

	m_pClientList[iClientH]->m_bIsSpecialAbilityEnabled = true;
	m_pClientList[iClientH]->m_dwSpecialAbilityStartTime = dwTime;

	m_pClientList[iClientH]->m_iSpecialAbilityTime = SpecialAbilityTimeSec;

	switch (m_pClientList[iClientH]->m_iSpecialAbilityType) {
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		m_pClientList[iClientH]->m_appearance.iEffectType = 1;
		break;
	case 50:
	case 51:
	case 52:
	case 53:
	case 54:
		m_pClientList[iClientH]->m_appearance.iEffectType = 2;
		break;
	}

	SendNotifyMsg(0, iClientH, Notify::SpecialAbilityStatus, 1, m_pClientList[iClientH]->m_iSpecialAbilityType, m_pClientList[iClientH]->m_iSpecialAbilityLastSec, 0);
	SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventMotion, Type::NullAction, 0, 0, 0);
}


void CGame::UpdateMapSectorInfo()
{
	int iMaxNeutralActivity, iMaxAresdenActivity, iMaxElvineActivity, iMaxMonsterActivity, iMaxPlayerActivity;

	for(int i = 0; i < MaxMaps; i++)
		if (m_pMapList[i] != 0) {

			iMaxNeutralActivity = iMaxAresdenActivity = iMaxElvineActivity = iMaxMonsterActivity = iMaxPlayerActivity = 0;
			m_pMapList[i]->m_iMaxNx = m_pMapList[i]->m_iMaxNy = m_pMapList[i]->m_iMaxAx = m_pMapList[i]->m_iMaxAy = 0;
			m_pMapList[i]->m_iMaxEx = m_pMapList[i]->m_iMaxEy = m_pMapList[i]->m_iMaxMx = m_pMapList[i]->m_iMaxMy = 0;
			m_pMapList[i]->m_iMaxPx = m_pMapList[i]->m_iMaxPy = 0;

			// TempSectorInfo   SectorInfo   TempSectorInfo .
			for(int ix = 0; ix < smap::MaxSectors; ix++)
				for(int iy = 0; iy < smap::MaxSectors; iy++) {
					if (m_pMapList[i]->m_stTempSectorInfo[ix][iy].iNeutralActivity > iMaxNeutralActivity) {
						iMaxNeutralActivity = m_pMapList[i]->m_stTempSectorInfo[ix][iy].iNeutralActivity;
						m_pMapList[i]->m_iMaxNx = ix;
						m_pMapList[i]->m_iMaxNy = iy;
					}

					if (m_pMapList[i]->m_stTempSectorInfo[ix][iy].iAresdenActivity > iMaxAresdenActivity) {
						iMaxAresdenActivity = m_pMapList[i]->m_stTempSectorInfo[ix][iy].iAresdenActivity;
						m_pMapList[i]->m_iMaxAx = ix;
						m_pMapList[i]->m_iMaxAy = iy;
					}

					if (m_pMapList[i]->m_stTempSectorInfo[ix][iy].iElvineActivity > iMaxElvineActivity) {
						iMaxElvineActivity = m_pMapList[i]->m_stTempSectorInfo[ix][iy].iElvineActivity;
						m_pMapList[i]->m_iMaxEx = ix;
						m_pMapList[i]->m_iMaxEy = iy;
					}

					if (m_pMapList[i]->m_stTempSectorInfo[ix][iy].iMonsterActivity > iMaxMonsterActivity) {
						iMaxMonsterActivity = m_pMapList[i]->m_stTempSectorInfo[ix][iy].iMonsterActivity;
						m_pMapList[i]->m_iMaxMx = ix;
						m_pMapList[i]->m_iMaxMy = iy;
					}

					if (m_pMapList[i]->m_stTempSectorInfo[ix][iy].iPlayerActivity > iMaxPlayerActivity) {
						iMaxPlayerActivity = m_pMapList[i]->m_stTempSectorInfo[ix][iy].iPlayerActivity;
						m_pMapList[i]->m_iMaxPx = ix;
						m_pMapList[i]->m_iMaxPy = iy;
					}
				}

			// TempSectorInfo .
			m_pMapList[i]->ClearTempSectorInfo();

			// Sector Info
			if (m_pMapList[i]->m_iMaxNx > 0) m_pMapList[i]->m_stSectorInfo[m_pMapList[i]->m_iMaxNx][m_pMapList[i]->m_iMaxNy].iNeutralActivity++;
			if (m_pMapList[i]->m_iMaxAx > 0) m_pMapList[i]->m_stSectorInfo[m_pMapList[i]->m_iMaxAx][m_pMapList[i]->m_iMaxAy].iAresdenActivity++;
			if (m_pMapList[i]->m_iMaxEx > 0) m_pMapList[i]->m_stSectorInfo[m_pMapList[i]->m_iMaxEx][m_pMapList[i]->m_iMaxEy].iElvineActivity++;
			if (m_pMapList[i]->m_iMaxMx > 0) m_pMapList[i]->m_stSectorInfo[m_pMapList[i]->m_iMaxMx][m_pMapList[i]->m_iMaxMy].iMonsterActivity++;
			if (m_pMapList[i]->m_iMaxPx > 0) m_pMapList[i]->m_stSectorInfo[m_pMapList[i]->m_iMaxPx][m_pMapList[i]->m_iMaxPy].iPlayerActivity++;
		}
}


void CGame::AgingMapSectorInfo()
{
	for(int i = 0; i < MaxMaps; i++)
		if (m_pMapList[i] != 0) {
			for(int ix = 0; ix < smap::MaxSectors; ix++)
				for(int iy = 0; iy < smap::MaxSectors; iy++) {
					m_pMapList[i]->m_stSectorInfo[ix][iy].iNeutralActivity--;
					m_pMapList[i]->m_stSectorInfo[ix][iy].iAresdenActivity--;
					m_pMapList[i]->m_stSectorInfo[ix][iy].iElvineActivity--;
					m_pMapList[i]->m_stSectorInfo[ix][iy].iMonsterActivity--;
					m_pMapList[i]->m_stSectorInfo[ix][iy].iPlayerActivity--;

					if (m_pMapList[i]->m_stSectorInfo[ix][iy].iNeutralActivity < 0) m_pMapList[i]->m_stSectorInfo[ix][iy].iNeutralActivity = 0;
					if (m_pMapList[i]->m_stSectorInfo[ix][iy].iAresdenActivity < 0) m_pMapList[i]->m_stSectorInfo[ix][iy].iAresdenActivity = 0;
					if (m_pMapList[i]->m_stSectorInfo[ix][iy].iElvineActivity < 0) m_pMapList[i]->m_stSectorInfo[ix][iy].iElvineActivity = 0;
					if (m_pMapList[i]->m_stSectorInfo[ix][iy].iMonsterActivity < 0) m_pMapList[i]->m_stSectorInfo[ix][iy].iMonsterActivity = 0;
					if (m_pMapList[i]->m_stSectorInfo[ix][iy].iPlayerActivity < 0) m_pMapList[i]->m_stSectorInfo[ix][iy].iPlayerActivity = 0;
				}
		}
}








// New 14/05/2004 Changed

void CGame::CheckConnectionHandler(int iClientH, char* pData, bool bAlreadyResponded)
{
	uint32_t dwTimeRcv, dwTime, dwTimeGapClient, dwTimeGapServer;

	if (m_pClientList[iClientH] == 0) return;

	dwTime = GameClock::GetTimeMS();
	const auto* req = hb::net::PacketCast<hb::net::PacketCommandCheckConnection>(pData, sizeof(hb::net::PacketCommandCheckConnection));
	if (!req) return;
	dwTimeRcv = req->time_ms;

	if (m_pClientList[iClientH]->m_dwInitCCTimeRcv == 0) {
		m_pClientList[iClientH]->m_dwInitCCTimeRcv = dwTimeRcv;
		m_pClientList[iClientH]->m_dwInitCCTime = dwTime;
	}
	else {
		dwTimeGapClient = (dwTimeRcv - m_pClientList[iClientH]->m_dwInitCCTimeRcv);
		dwTimeGapServer = (dwTime - m_pClientList[iClientH]->m_dwInitCCTime);

		if (dwTimeGapClient < dwTimeGapServer) return;
		if ((dwTimeGapClient - dwTimeGapServer) >= (uint32_t)m_iClientTimeout) {
			DeleteClient(iClientH, true, true);
			return;
		}
	}

	if (!bAlreadyResponded) {
		hb::net::PacketCommandCheckConnection resp{};
		resp.header.msg_id = MsgId::CommandCheckConnection;
		resp.header.msg_type = MsgType::Confirm;
		resp.time_ms = dwTimeRcv;
		m_pClientList[iClientH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&resp), sizeof(resp));
	}
}






void CGame::RequestHelpHandler(int iClientH)
{
	

	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_iGuildRank == -1) return;
	if (m_pClientList[iClientH]->m_iCrusadeDuty != 1) return;

	for(int i = 1; i < MaxClients; i++)
		if ((m_pClientList[i] != 0) && (m_pClientList[i]->m_iGuildRank == 0) &&
			(m_pClientList[i]->m_iCrusadeDuty == 3) && (m_pClientList[i]->m_iGuildGUID == m_pClientList[iClientH]->m_iGuildGUID)) {
			SendNotifyMsg(0, i, Notify::Help, m_pClientList[iClientH]->m_sX, m_pClientList[iClientH]->m_sY, m_pClientList[iClientH]->m_iHP, m_pClientList[iClientH]->m_cCharName);
			return;
		}

	SendNotifyMsg(0, iClientH, Notify::HelpFailed, 0, 0, 0, 0);
}




bool CGame::bAddClientShortCut(int iClientH)
{
	


	for(int i = 0; i < MaxClients; i++)
		if (m_iClientShortCut[i] == iClientH) return false;

	for(int i = 0; i < MaxClients; i++)
		if (m_iClientShortCut[i] == 0) {
			m_iClientShortCut[i] = iClientH;
			return true;
		}

	return false;
}

void CGame::RemoveClientShortCut(int iClientH)
{
	

	for(int i = 0; i < MaxClients + 1; i++)
		if (m_iClientShortCut[i] == iClientH) {
			m_iClientShortCut[i] = 0;
			goto RCSC_LOOPBREAK;
		}

RCSC_LOOPBREAK:

	//m_iClientShortCut[i] = m_iClientShortCut[m_iTotalClients+1];
	//m_iClientShortCut[m_iTotalClients+1] = 0;
	for(int i = 0; i < MaxClients; i++)
		if ((m_iClientShortCut[i] == 0) && (m_iClientShortCut[i + 1] != 0)) {
			m_iClientShortCut[i] = m_iClientShortCut[i + 1];
			m_iClientShortCut[i + 1] = 0;
		}
}





int CGame::iGetMapLocationSide(char* pMapName)
{

	if (strcmp(pMapName, "aresden") == 0) return 3;
	if (strcmp(pMapName, "elvine") == 0) return 4;
	if (strcmp(pMapName, "arebrk11") == 0) return 3;
	if (strcmp(pMapName, "elvbrk11") == 0) return 4;

	if (strcmp(pMapName, "cityhall_1") == 0) return 1;
	if (strcmp(pMapName, "cityhall_2") == 0) return 2;
	if (strcmp(pMapName, "cath_1") == 0) return 1;
	if (strcmp(pMapName, "cath_2") == 0) return 2;
	if (strcmp(pMapName, "gshop_1") == 0) return 1;
	if (strcmp(pMapName, "gshop_2") == 0) return 2;
	if (strcmp(pMapName, "bsmith_1") == 0) return 1;
	if (strcmp(pMapName, "bsmith_2") == 0) return 2;
	if (strcmp(pMapName, "wrhus_1") == 0) return 1;
	if (strcmp(pMapName, "wrhus_2") == 0) return 2;
	if (strcmp(pMapName, "gldhall_1") == 0) return 1;
	if (strcmp(pMapName, "gldhall_2") == 0) return 2;
	if (strcmp(pMapName, "wzdtwr_1") == 0) return 1;
	if (strcmp(pMapName, "wzdtwr_2") == 0) return 2;
	if (strcmp(pMapName, "arefarm") == 0) return 1;
	if (strcmp(pMapName, "elvfarm") == 0) return 2;
	if (strcmp(pMapName, "arewrhus") == 0) return 1;
	if (strcmp(pMapName, "elvwrhus") == 0) return 2;
	if (strcmp(pMapName, "cmdhall_1") == 0) return 1;
	if (strcmp(pMapName, "Cmdhall_2") == 0) return 2;

	return 0;
}



//ArchAngel Function

void CGame::RequestChangePlayMode(int iClientH)
{

	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_iPKCount > 0) return;
	if (m_pClientList[iClientH]->m_bIsInitComplete == false) return;

	if (memcmp(m_pClientList[iClientH]->m_cMapName, "cityhall", 8) != 0) return;

	if (m_pClientList[iClientH]->m_iLevel < 100 ||
		m_pClientList[iClientH]->m_bIsPlayerCivil) {
		if (memcmp(m_pClientList[iClientH]->m_cLocation, "aresden", 7) == 0) strcpy(m_pClientList[iClientH]->m_cLocation, "arehunter");
		else if (memcmp(m_pClientList[iClientH]->m_cLocation, "elvine", 6) == 0) strcpy(m_pClientList[iClientH]->m_cLocation, "elvhunter");
		else if (memcmp(m_pClientList[iClientH]->m_cLocation, "arehunter", 9) == 0) strcpy(m_pClientList[iClientH]->m_cLocation, "aresden");
		else if (memcmp(m_pClientList[iClientH]->m_cLocation, "elvhunter", 9) == 0) strcpy(m_pClientList[iClientH]->m_cLocation, "elvine");

		if (m_pClientList[iClientH]->m_bIsPlayerCivil)
			m_pClientList[iClientH]->m_bIsPlayerCivil = false;
		else m_pClientList[iClientH]->m_bIsPlayerCivil = true;

		SendNotifyMsg(0, iClientH, Notify::ChangePlayMode, 0, 0, 0, m_pClientList[iClientH]->m_cLocation);
		SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventMotion, 100, 0, 0, 0);
	}

	g_login->LocalSavePlayerData(iClientH);
}

/*********************************************************************************************************************
**  void CGame::SetInvisibilityFlag(short sOwnerH, char cOwnerType, bool bStatus)									**
**  description			:: changes the status of the player to show invisibility aura								**
**  last updated		:: November 20, 2004; 9:30 PM; Hypnotoad													**
**	return value		:: void																						**
*********************************************************************************************************************/

/*********************************************************************************************************************
**  void CGame::SetInhibitionCastingFlag(short sOwnerH, char cOwnerType, bool bStatus)								**
**  description			:: changes the status of the player to show inhibit casting aura							**
**  last updated		:: November 20, 2004; 9:33 PM; Hypnotoad													**
**	return value		:: void																						**
*********************************************************************************************************************/

/*********************************************************************************************************************
**  void void CGame::SetBerserkFlag(short sOwnerH, char cOwnerType, bool bStatus)									**
**  description			:: changes the status of the player to show berserk aura									**
**  last updated		:: November 20, 2004; 9:35 PM; Hypnotoad													**
**	return value		:: void																						**
*********************************************************************************************************************/


/*********************************************************************************************************************
**  void void CGame::SetIceFlag(short sOwnerH, char cOwnerType, bool bStatus)										**
**  description			:: changes the status of the player to show frozen aura										**
**  last updated		:: November 20, 2004; 9:35 PM; Hypnotoad													**
**	return value		:: void																						**
*********************************************************************************************************************/

/*********************************************************************************************************************
**  void void CGame::SetPoisonFlag(short sOwnerH, char cOwnerType, bool bStatus)									**
**  description			:: changes the status of the player to show poison aura										**
**  last updated		:: November 20, 2004; 9:36 PM; Hypnotoad													**
**	return value		:: void																						**
*********************************************************************************************************************/

/*********************************************************************************************************************
**  void void CGame::SetIllusionFlag(short sOwnerH, char cOwnerType, bool bStatus)									**
**  description			:: changes the status of the player to show illusion aura									**
**  last updated		:: November 20, 2004; 9:36 PM; Hypnotoad													**
**	return value		:: void																						**
*********************************************************************************************************************/

/*********************************************************************************************************************
**  void void CGame::SetHeroFlag(short sOwnerH, char cOwnerType, bool bStatus)										**
**  description			:: changes the status of the player to show hero item aura									**
**  last updated		:: November 20, 2004; 9:37 PM; Hypnotoad													**
**	return value		:: void																						**
*********************************************************************************************************************/
int CGame::FindAdminByAccount(const char* accountName)
{
	if (accountName == nullptr) return -1;
	for (int i = 0; i < m_iAdminCount; i++) {
		if (_stricmp(m_stAdminList[i].m_cAccountName, accountName) == 0)
			return i;
	}
	return -1;
}

int CGame::FindAdminByCharName(const char* charName)
{
	if (charName == nullptr) return -1;
	for (int i = 0; i < m_iAdminCount; i++) {
		if (_stricmp(m_stAdminList[i].m_cCharName, charName) == 0)
			return i;
	}
	return -1;
}

bool CGame::IsClientAdmin(int iClientH)
{
	if (iClientH <= 0 || iClientH >= MaxClients) return false;
	if (m_pClientList[iClientH] == nullptr) return false;
	return m_pClientList[iClientH]->m_iAdminIndex != -1;
}

int CGame::GetCommandRequiredLevel(const char* cmdName) const
{
	if (cmdName == nullptr) return hb::shared::admin::Administrator;
	auto it = m_commandPermissions.find(cmdName);
	if (it != m_commandPermissions.end())
		return it->second.iAdminLevel;
	return hb::shared::admin::Administrator;
}

int CGame::FindClientByName(const char* pName) const
{
	if (pName == nullptr) return 0;
	for (int i = 1; i < MaxClients; i++)
	{
		if (m_pClientList[i] != nullptr && m_pClientList[i]->m_bIsInitComplete)
		{
			if (_strnicmp(m_pClientList[i]->m_cCharName, pName, hb::shared::limits::CharNameLen - 1) == 0)
				return i;
		}
	}
	return 0;
}

bool CGame::GMTeleportTo(int iClientH, const char* cDestMap, short sDestX, short sDestY)
{
	if (m_pClientList[iClientH] == nullptr) return false;
	if (cDestMap == nullptr) return false;

	// Find destination map index
	int iDestMapIndex = -1;
	for (int i = 0; i < MaxMaps; i++)
	{
		if (m_pMapList[i] != nullptr && memcmp(m_pMapList[i]->m_cName, cDestMap, 10) == 0)
		{
			iDestMapIndex = i;
			break;
		}
	}
	if (iDestMapIndex == -1) return false;

	// Remove from current location
	m_pCombatManager->RemoveFromTarget(iClientH, hb::shared::owner_class::Player);
	m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->ClearOwner(13, iClientH, hb::shared::owner_class::Player,
		m_pClientList[iClientH]->m_sX, m_pClientList[iClientH]->m_sY);
	SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventLog, MsgType::Reject, 0, 0, 0);

	// Update position and map
	m_pClientList[iClientH]->m_sX = sDestX;
	m_pClientList[iClientH]->m_sY = sDestY;
	m_pClientList[iClientH]->m_cMapIndex = static_cast<char>(iDestMapIndex);
	std::memset(m_pClientList[iClientH]->m_cMapName, 0, sizeof(m_pClientList[iClientH]->m_cMapName));
	memcpy(m_pClientList[iClientH]->m_cMapName, m_pMapList[iDestMapIndex]->m_cName, 10);

	// Always send full INITDATA — same pattern as RequestTeleportHandler RTH_NEXTSTEP.
	// Even same-map teleports need INITDATA so the client reinitializes its view.
	SetPlayingStatus(iClientH);
	// Set faction/identity status fields from player data
	m_pClientList[iClientH]->m_status.bPK = (m_pClientList[iClientH]->m_iPKCount != 0) ? 1 : 0;
	m_pClientList[iClientH]->m_status.bCitizen = (m_pClientList[iClientH]->m_cSide != 0) ? 1 : 0;
	m_pClientList[iClientH]->m_status.bAresden = (m_pClientList[iClientH]->m_cSide == 1) ? 1 : 0;
	m_pClientList[iClientH]->m_status.bHunter = m_pClientList[iClientH]->m_bIsPlayerCivil ? 1 : 0;

	hb::net::PacketWriter writer;
	char initMapData[hb::shared::limits::MsgBufferSize + 1];

	writer.Reset();
	auto* init_header = writer.Append<hb::net::PacketResponseInitDataHeader>();
	init_header->header.msg_id = MsgId::ResponseInitData;
	init_header->header.msg_type = MsgType::Confirm;

	bGetEmptyPosition(&m_pClientList[iClientH]->m_sX, &m_pClientList[iClientH]->m_sY, m_pClientList[iClientH]->m_cMapIndex);

	init_header->player_object_id = static_cast<std::int16_t>(iClientH);
	init_header->pivot_x = static_cast<std::int16_t>(m_pClientList[iClientH]->m_sX - hb::shared::view::PlayerPivotOffsetX);
	init_header->pivot_y = static_cast<std::int16_t>(m_pClientList[iClientH]->m_sY - hb::shared::view::PlayerPivotOffsetY);
	init_header->player_type = m_pClientList[iClientH]->m_sType;
	init_header->appearance = m_pClientList[iClientH]->m_appearance;
	init_header->status = m_pClientList[iClientH]->m_status;
	std::memcpy(init_header->map_name, m_pClientList[iClientH]->m_cMapName, sizeof(init_header->map_name));
	std::memcpy(init_header->cur_location, m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cLocationName, sizeof(init_header->cur_location));

	if (m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_bIsFixedDayMode)
		init_header->sprite_alpha = 1;
	else init_header->sprite_alpha = m_cDayOrNight;

	if (m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_bIsFixedDayMode)
		init_header->weather_status = 0;
	else init_header->weather_status = m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cWhetherStatus;

	init_header->contribution = m_pClientList[iClientH]->m_iContribution;

	m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->SetOwner(iClientH,
		hb::shared::owner_class::Player,
		m_pClientList[iClientH]->m_sX,
		m_pClientList[iClientH]->m_sY);

	init_header->observer_mode = static_cast<std::uint8_t>(m_pClientList[iClientH]->m_bIsObserverMode);
	init_header->rating = m_pClientList[iClientH]->m_iRating;
	init_header->hp = m_pClientList[iClientH]->m_iHP;
	init_header->discount = 0;

	int iSize = iComposeInitMapData(m_pClientList[iClientH]->m_sX - hb::shared::view::CenterX, m_pClientList[iClientH]->m_sY - hb::shared::view::CenterY, iClientH, initMapData);
	writer.AppendBytes(initMapData, static_cast<std::size_t>(iSize));

	int iRet = m_pClientList[iClientH]->m_pXSock->iSendMsg(writer.Data(), static_cast<int>(writer.Size()));
	if (iRet == sock::Event::QueueFull || iRet == sock::Event::SocketError ||
		iRet == sock::Event::CriticalError || iRet == sock::Event::SocketClosed)
	{
		DeleteClient(iClientH, true, true);
		return false;
	}

	SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventLog, MsgType::Confirm, 0, 0, 0);

	return true;
}


/*********************************************************************************************************************
**  void void CGame::SetDefenseShieldFlag(short sOwnerH, char cOwnerType, bool bStatus)								**
**  description			:: changes the status of the player to show defense aura									**
**  last updated		:: November 20, 2004; 9:37 PM; Hypnotoad													**
**	return value		:: void																						**
*********************************************************************************************************************/

/*********************************************************************************************************************
**  void void CGame::SetMagicProtectionFlag(short sOwnerH, char cOwnerType, bool bStatus)							**
**  description			:: changes the status of the player to show magic protect aura								**
**  last updated		:: November 20, 2004; 9:38 PM; Hypnotoad													**
**	return value		:: void																						**
*********************************************************************************************************************/

/*********************************************************************************************************************
**  void void CGame::SetProtectionFromArrowFlag(short sOwnerH, char cOwnerType, bool bStatus)						**
**  description			:: changes the status of the player to show arrow protect aura								**
**  last updated		:: November 20, 2004; 9:39 PM; Hypnotoad													**
**	return value		:: void																						**
*********************************************************************************************************************/

/*********************************************************************************************************************
**  void void CGame::SetIllusionMovementFlag(short sOwnerH, char cOwnerType, bool bStatus)							**
**  description			:: changes the status of the player to show illusion movement aura							**
**  last updated		:: November 20, 2004; 9:39 PM; Hypnotoad													**
**	return value		:: void																						**
*********************************************************************************************************************/

// New 07/05/2004
// Item Logging



void CGame::GetExp(int iClientH, uint32_t iExp, bool bIsAttackerOwn)
{
	double dV1, dV2, dV3;
	int iH;
	uint32_t dwTime = GameClock::GetTimeMS(), iUnitValue;
	int iTotalPartyMembers;

	if (m_pClientList[iClientH] == 0) return;
	if (iExp <= 0) return;

	if (m_pClientList[iClientH]->m_iLevel <= 80) {
		dV1 = (double)(80 - m_pClientList[iClientH]->m_iLevel);
		dV2 = dV1 * 0.025f;
		dV3 = (double)iExp;
		dV1 = (dV2 + 1.025f) * dV3;
		iExp = (uint32_t)dV1;
	}
	else { //Lower exp
		if ((m_pClientList[iClientH]->m_iLevel >= 80) && ((strcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cName, "aresdend1") == 0) || (strcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cName, "elvined1") == 0))) {
			iExp = (iExp / 10);
		}
		else if ((strcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cName, "aresdend1") == 0) || (strcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cName, "elvined1") == 0)) {
			iExp = (iExp * 1 / 4);
		}
	}

	//Check for party status, else give exp to player
	if ((m_pClientList[iClientH]->m_iPartyID != 0) && (m_pClientList[iClientH]->m_iPartyStatus == PartyStatus::Confirm)) {
		//Only divide exp if >= 1 person 
		if (m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iTotalMembers > 0) {

			//Calc total ppl in party
			iTotalPartyMembers = 0;
			for(int i = 0; i < m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iTotalMembers; i++) {
				iH = m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iIndex[i];
				if ((m_pClientList[iH] != 0) && (m_pClientList[iH]->m_iHP > 0)) {
					//Newly added, Only players on same map get exp :}
					if ((strlen(m_pMapList[m_pClientList[iH]->m_cMapIndex]->m_cName)) == (strlen(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cName))) {
						if (memcmp(m_pMapList[m_pClientList[iH]->m_cMapIndex]->m_cName, m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cName, strlen(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cName)) == 0) {
							iTotalPartyMembers++;
						}
					}
				}
			}

			//Check for party bug
			if (iTotalPartyMembers > hb::shared::limits::MaxPartyMembers) {
				std::snprintf(G_cTxt, sizeof(G_cTxt), "(X) Party Bug !! partyMember %d XXXXXXXXXX", iTotalPartyMembers);
				PutLogFileList(G_cTxt);
				iTotalPartyMembers = hb::shared::limits::MaxPartyMembers;
			}

			//Figure out how much exp a player gets
			dV1 = (double)iExp;
			dV2 = dV1;

			if (iTotalPartyMembers > 1)
			{
				dV2 = (dV1 + (dV1 * (double)(iTotalPartyMembers / hb::shared::limits::MaxPartyMembers))) / (double)iTotalPartyMembers;
			}

			dV3 = dV2 + 5.0e-1;
			iUnitValue = (uint32_t)dV3;

			//Divide exp among party members
			for(int i = 0; i < iTotalPartyMembers; i++) {

				iH = m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iIndex[i];
				if ((m_pClientList[iH] != 0) && (m_pClientList[iH]->m_bSkillUsingStatus[19] != 1) && (m_pClientList[iH]->m_iHP > 0)) { // Is player alive ??
					if (m_pClientList[iH]->m_status.bSlateExp)  iUnitValue *= 3;
					m_pClientList[iH]->m_iExpStock += iUnitValue;
				}
			}
		}
	}
	else {
		if ((m_pClientList[iClientH] != 0) && (m_pClientList[iClientH]->m_bSkillUsingStatus[19] != 1) && (m_pClientList[iClientH]->m_iHP > 0)) { // Is player alive ??
			if (m_pClientList[iClientH]->m_status.bSlateExp)  iExp *= 3;
			m_pClientList[iClientH]->m_iExpStock += iExp;
		}
	}
}



// New 12/05/2004



// New 16/05/2004


// New 18/05/2004
void CGame::SetPlayingStatus(int iClientH)
{
	char cMapName[11], cLocation[11];

	if (m_pClientList[iClientH] == 0) return;

	std::memset(cMapName, 0, sizeof(cMapName));
	std::memset(cLocation, 0, sizeof(cLocation));

	strcpy(cLocation, m_pClientList[iClientH]->m_cLocation);
	strcpy(cMapName, m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cName);

	m_pClientList[iClientH]->m_cSide = 0;
	m_pClientList[iClientH]->m_bIsOwnLocation = false;
	m_pClientList[iClientH]->m_bIsPlayerCivil = false;

	if (memcmp(cLocation, cMapName, 3) == 0) {
		m_pClientList[iClientH]->m_bIsOwnLocation = true;
	}

	if (memcmp(cLocation, "are", 3) == 0)
		m_pClientList[iClientH]->m_cSide = 1;
	else if (memcmp(cLocation, "elv", 3) == 0)
		m_pClientList[iClientH]->m_cSide = 2;
	else {
		if (strcmp(cMapName, "elvine") == 0 || strcmp(cMapName, "aresden") == 0) {
			m_pClientList[iClientH]->m_bIsOwnLocation = true;
		}
		m_pClientList[iClientH]->m_bIsNeutral = true;
	}

	if (memcmp(cLocation, "arehunter", 9) == 0 || memcmp(cLocation, "elvhunter", 9) == 0) {
		m_pClientList[iClientH]->m_bIsPlayerCivil = true;
	}

	if (memcmp(m_pClientList[iClientH]->m_cMapName, "bisle", 5) == 0) {
		m_pClientList[iClientH]->m_bIsPlayerCivil = false;
	}

	if (memcmp(m_pClientList[iClientH]->m_cMapName, "bsmith", 6) == 0 ||
		memcmp(m_pClientList[iClientH]->m_cMapName, "gldhall", 7) == 0 ||
		memcmp(m_pClientList[iClientH]->m_cMapName, "gshop", 5) == 0)
		m_pClientList[iClientH]->m_pIsProcessingAllowed = true;
	else
		m_pClientList[iClientH]->m_pIsProcessingAllowed = false;

	if (memcmp(m_pClientList[iClientH]->m_cMapName, "wrhus", 5) == 0 ||
		memcmp(m_pClientList[iClientH]->m_cMapName, "arewrhus", 8) == 0 ||
		memcmp(m_pClientList[iClientH]->m_cMapName, "elvwrhus", 8) == 0)
		m_pClientList[iClientH]->m_bIsInsideWarehouse = true;
	else
		m_pClientList[iClientH]->m_bIsInsideWarehouse = false;

	if (memcmp(m_pClientList[iClientH]->m_cMapName, "wzdtwr", 6) == 0)
		m_pClientList[iClientH]->m_bIsInsideWizardTower = true;
	else
		m_pClientList[iClientH]->m_bIsInsideWizardTower = false;
}

void CGame::ForceChangePlayMode(int iClientH, bool bNotify)
{
	if (m_pClientList[iClientH] == 0) return;

	if (memcmp(m_pClientList[iClientH]->m_cLocation, "arehunter", 9) == 0)
		strcpy(m_pClientList[iClientH]->m_cLocation, "aresden");
	else if (memcmp(m_pClientList[iClientH]->m_cLocation, "elvhunter", 9) == 0)
		strcpy(m_pClientList[iClientH]->m_cLocation, "elvine");

	if (m_pClientList[iClientH]->m_bIsPlayerCivil)
		m_pClientList[iClientH]->m_bIsPlayerCivil = false;

	if (bNotify) {
		SendNotifyMsg(0, iClientH, Notify::ChangePlayMode, 0, 0, 0, m_pClientList[iClientH]->m_cLocation);
		SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventMotion, Type::NullAction, 0, 0, 0);
	}
}

void CGame::ShowVersion(int iClientH)
{
	char cVerMessage[256];

	std::memset(cVerMessage, 0, sizeof(cVerMessage));
	std::snprintf(cVerMessage, sizeof(cVerMessage), "Helbreath Sources %s.%s - www.xtremehb.com", hb::server::version::Upper, hb::server::version::Lower);
	ShowClientMsg(iClientH, cVerMessage);

}

// v2.14 05/22/2004 - Hypnotoad - adds pk log
void CGame::RequestResurrectPlayer(int iClientH, bool bResurrect)
{
	short sX, sY;
	char buff[100];

	if (m_pClientList[iClientH] == 0) return;

	sX = m_pClientList[iClientH]->m_sX;
	sY = m_pClientList[iClientH]->m_sY;

	if (bResurrect == false) {
		m_pClientList[iClientH]->m_bIsBeingResurrected = false;
		return;
	}

	if (m_pClientList[iClientH]->m_bIsBeingResurrected == false) {
		try
		{
			std::snprintf(buff, sizeof(buff), "(!!!) Player(%s) Tried To Use Resurrection Hack", m_pClientList[iClientH]->m_cCharName);
			PutHackLogFileList(G_cTxt);
			DeleteClient(iClientH, true, true, true, true);
		}
		catch (...)
		{
		}
		return;
	}

	std::snprintf(buff, sizeof(buff), "(*) Resurrect Player! %s", m_pClientList[iClientH]->m_cCharName);
	PutLogList(buff);


	m_pClientList[iClientH]->m_bIsKilled = false;
	// Player's HP becomes half of the Max HP. 
	m_pClientList[iClientH]->m_iHP = iGetMaxHP(iClientH) / 2;
	// Player's MP
	m_pClientList[iClientH]->m_iMP = iGetMaxMP(iClientH);
	// Player's SP
	m_pClientList[iClientH]->m_iSP = iGetMaxSP(iClientH);
	// Player's Hunger
	m_pClientList[iClientH]->m_iHungerStatus = 100;

	m_pClientList[iClientH]->m_bIsBeingResurrected = false;

	// !!! RequestTeleportHandler m_cMapName
	RequestTeleportHandler(iClientH, "2   ", m_pClientList[iClientH]->m_cMapName, m_pClientList[iClientH]->m_sX, m_pClientList[iClientH]->m_sY);
}


bool CGame::bCheckClientMoveFrequency(int iClientH, uint32_t dwClientTime)
{
	uint32_t dwTimeGap;

	if (m_pClientList[iClientH] == 0) return false;


	if (m_pClientList[iClientH]->m_dwMoveFreqTime == 0)
		m_pClientList[iClientH]->m_dwMoveFreqTime = dwClientTime;
	else {
		if (m_pClientList[iClientH]->m_bIsMoveBlocked) {
			m_pClientList[iClientH]->m_dwMoveFreqTime = 0;
			m_pClientList[iClientH]->m_bIsMoveBlocked = false;
			return false;
		}

		if (m_pClientList[iClientH]->m_bIsAttackModeChange) {
			m_pClientList[iClientH]->m_dwMoveFreqTime = 0;
			m_pClientList[iClientH]->m_bIsAttackModeChange = false;
			return false;
		}

		dwTimeGap = dwClientTime - m_pClientList[iClientH]->m_dwMoveFreqTime;
		m_pClientList[iClientH]->m_dwMoveFreqTime = dwClientTime;

		if ((dwTimeGap < 200) && (dwTimeGap >= 0)) {
			try
			{
				std::snprintf(G_cTxt, sizeof(G_cTxt), "Speed Hack: (%s) Player: (%s) - running too fast.", m_pClientList[iClientH]->m_cIPaddress, m_pClientList[iClientH]->m_cCharName);
				PutHackLogFileList(G_cTxt);
				DeleteClient(iClientH, true, true);
			}
			catch (...)
			{
			}
			return false;
		}

		// testcode
		// std::snprintf(G_cTxt, sizeof(G_cTxt), "Move: %d", dwTimeGap);
		// PutLogList(G_cTxt);
	}

	return false;
}


void CGame::OnTimer(char cType)
{
	uint32_t dwTime;

	dwTime = GameClock::GetTimeMS();

	// MODERNIZED: GameProcess moved to EventLoop to prevent blocking socket polling
	// OnTimer now only handles periodic events (CheckClientResponseTime, weather, etc.)
	// GameProcess() is called directly in EventLoop every 300ms


	if ((dwTime - m_dwGameTime2) > 1000) {
		CheckClientResponseTime();
		CheckDayOrNightMode();
		InvalidateRect(G_hWnd, 0, true);
		m_dwGameTime2 = dwTime;
		// v1.41 

		// v1.41
		if (m_bIsGameStarted == false) {
			PutLogList("Sending start message...");
			SendMessage(m_hWnd, WM_USER_STARTGAMESIGNAL, 0, 0);
			m_bIsGameStarted = true;

			// Initialize EntityManager now that maps are loaded
			if (m_pEntityManager != NULL) {
				// EntityManager owns the entity array now, just set maps and game reference
				m_pEntityManager->SetMapList(m_pMapList, MaxMaps);
				m_pEntityManager->SetGame(this);

	// Initialize Gathering Managers
	m_pFishingManager = new FishingManager();
	m_pFishingManager->SetGame(this);
	m_pMiningManager = new MiningManager();
	m_pMiningManager->SetGame(this);
	m_pCraftingManager = new CraftingManager();
	m_pCraftingManager->SetGame(this);
	m_pQuestManager = new QuestManager();
	m_pQuestManager->SetGame(this);
	m_pGuildManager = new GuildManager();
	m_pGuildManager->SetGame(this);
	m_pDelayEventManager = new DelayEventManager();
	m_pDelayEventManager->SetGame(this);
	m_pDelayEventManager->InitArrays();
	m_pDynamicObjectManager = new DynamicObjectManager();
	m_pDynamicObjectManager->SetGame(this);
	m_pDynamicObjectManager->InitArrays();
	m_pLootManager = new LootManager();
	m_pLootManager->SetGame(this);
	m_pCombatManager = new CombatManager();
	m_pItemManager = new ItemManager();
	m_pMagicManager = new MagicManager();
	m_pSkillManager = new SkillManager();
	m_pWarManager = new WarManager();
	m_pStatusEffectManager = new StatusEffectManager();
	m_pCombatManager->SetGame(this);
	m_pItemManager->SetGame(this);
	m_pMagicManager->SetGame(this);
	m_pSkillManager->SetGame(this);
	m_pWarManager->SetGame(this);
	m_pStatusEffectManager->SetGame(this);
				PutLogList("EntityManager initialized");
			}

			// Initialize Gathering Managers
			if (m_pFishingManager != nullptr) {
				m_pFishingManager->SetGame(this);
			}
			if (m_pMiningManager != nullptr) {
				m_pMiningManager->SetGame(this);
			}
			if (m_pCraftingManager != nullptr) {
				m_pCraftingManager->SetGame(this);
			}
			if (m_pQuestManager != nullptr) {
				m_pQuestManager->SetGame(this);
			}
			if (m_pGuildManager != nullptr) {
				m_pGuildManager->SetGame(this);
			}
			if (m_pDelayEventManager != nullptr) {
				m_pDelayEventManager->SetGame(this);
				m_pDelayEventManager->CleanupArrays();
				m_pDelayEventManager->InitArrays();
			}
			if (m_pDynamicObjectManager != nullptr) {
				m_pDynamicObjectManager->SetGame(this);
				m_pDynamicObjectManager->CleanupArrays();
				m_pDynamicObjectManager->InitArrays();
			}
			if (m_pLootManager != nullptr) {
				m_pLootManager->SetGame(this);
			}
			if (m_pCombatManager != nullptr) {
				m_pCombatManager->SetGame(this);
	m_pItemManager->SetGame(this);
	m_pMagicManager->SetGame(this);
	m_pSkillManager->SetGame(this);
	m_pWarManager->SetGame(this);
	m_pStatusEffectManager->SetGame(this);
			}
		}
	}
	if ((dwTime - m_dwGameTime6) > 1000) {
		m_pDelayEventManager->DelayEventProcessor();
		m_dwGameTime6 = dwTime;

		// v2.05
		if (m_iFinalShutdownCount != 0) {
			m_iFinalShutdownCount--;
			std::snprintf(G_cTxt, sizeof(G_cTxt), "Final Shutdown...%d", m_iFinalShutdownCount);
			PutLogList(G_cTxt);
			if (m_iFinalShutdownCount <= 1) {
				// 2.14
				SendMessage(m_hWnd, WM_CLOSE, 0, 0);
				return;

			}
		}
	}

	if ((dwTime - m_dwGameTime3) > 1000) {
		m_pWarManager->SyncMiddlelandMapInfo();
		m_pDynamicObjectManager->CheckDynamicObjectList();
		m_pDynamicObjectManager->DynamicObjectEffectProcessor();
		NoticeHandler();
		SpecialEventHandler();
		m_pWarManager->EnergySphereProcessor();
		m_dwGameTime3 = dwTime;
	}

	if ((dwTime - m_dwGameTime4) > 600) {
		// Use EntityManager for spawn generation
		if (m_pEntityManager != NULL)
			m_pEntityManager->ProcessSpawns();

		m_dwGameTime4 = dwTime;
	}


	if ((dwTime - m_dwGameTime5) > 1000 * 60 * 3) {

		m_dwGameTime5 = dwTime;

		srand((unsigned)time(0));
	}

	if ((dwTime - m_pFishingManager->m_dwFishTime) > 5000) {
		m_pFishingManager->FishProcessor();
		m_pFishingManager->FishGenerator();
		m_pWarManager->SendCollectedMana();
		m_pWarManager->CrusadeWarStarter();
		//ApocalypseStarter();
		m_pWarManager->ApocalypseEnder();
		m_pFishingManager->m_dwFishTime = dwTime;
	}

	if ((dwTime - m_dwWhetherTime) > 1000 * 20) {
		WhetherProcessor();
		m_dwWhetherTime = dwTime;
	}

	if ((m_bHeldenianRunning) && (m_bIsHeldenianMode)) {
		m_pWarManager->SetHeldenianMode();
	}
	if ((dwTime - m_dwCanFightzoneReserveTime) > 7200000) {
		m_pWarManager->FightzoneReserveProcessor();
		m_dwCanFightzoneReserveTime = dwTime;
	}

	if ((m_bIsServerShutdowned == false) && (m_bOnExitProcess) && ((dwTime - m_dwExitProcessTime) > 1000 * 2)) {
		if (_iForcePlayerDisconect(15) == 0) {
			PutLogList("(!) GAME SERVER SHUTDOWN PROCESS COMPLETED! All players are disconnected.");
			m_bIsServerShutdowned = true;

			if ((m_cShutDownCode == 3) || (m_cShutDownCode == 4)) {
				PutLogFileList("(!!!) AUTO-SERVER-REBOOTING!");
				bInit();
				m_iAutoRebootingCount++;
			}
			else {
				if (m_iFinalShutdownCount == 0)	m_iFinalShutdownCount = 20;
			}
		}
		m_dwExitProcessTime = dwTime;
	}

	if ((dwTime - m_dwMapSectorInfoTime) > 1000 * 10) {
		m_dwMapSectorInfoTime = dwTime;
		UpdateMapSectorInfo();

		m_pMiningManager->MineralGenerator();

		m_iMapSectorInfoUpdateCount++;
		if (m_iMapSectorInfoUpdateCount >= 5) {
			AgingMapSectorInfo();
			m_iMapSectorInfoUpdateCount = 0;
		}
	}
}


void CGame::OnStartGameSignal()
{
	// Load map configurations from MapInfo.db
	sqlite3* mapInfoDb = nullptr;
	std::string mapInfoDbPath;
	bool mapInfoDbCreated = false;

	if (!EnsureMapInfoDatabase(&mapInfoDb, mapInfoDbPath, &mapInfoDbCreated)) {
		PutLogListLevel(LOG_LEVEL_NOTICE, "(!!!) CRITICAL ERROR: MapInfo.db not available!");
	}
	else {
		PutLogListLevel(LOG_LEVEL_NOTICE, "Loading map configurations from MapInfo.db...");
		int mapsLoaded = 0;
		for(int i = 0; i < MaxMaps; i++)
		{
			if (m_pMapList[i] != 0)
			{
				if (memcmp(m_pMapList[i]->m_cName, "fightzone", 9) == 0)
					m_pMapList[i]->m_bIsFightZone = true;
				if (memcmp(m_pMapList[i]->m_cName, "icebound", 8) == 0)
					m_pMapList[i]->m_bIsSnowEnabled = true;

				if (LoadMapConfig(mapInfoDb, m_pMapList[i]->m_cName, m_pMapList[i])) {
					mapsLoaded++;
					SpawnMapNpcsFromDatabase(mapInfoDb, i);
				}
				else {
					char cTxt[256];
					std::snprintf(cTxt, sizeof(cTxt), "(!) WARNING: Failed to load map config for: %s", m_pMapList[i]->m_cName);
					PutLogList(cTxt);
				}
			}
		}
		char cTxt[128];
		std::snprintf(cTxt, sizeof(cTxt), "Loaded %d map configurations from database.", mapsLoaded);
		PutLogListLevel(LOG_LEVEL_NOTICE, cTxt);
		CloseMapInfoDatabase(mapInfoDb);
	}

	bool loadedSchedules = false;
	sqlite3* configDb = nullptr;
	std::string configDbPath;
	bool configDbCreated = false;
	bool configDbReady = EnsureGameConfigDatabase(&configDb, configDbPath, &configDbCreated);
	if (configDbReady && !configDbCreated) {
		bool hasCrusade = HasGameConfigRows(configDb, "crusade_structures");
		bool hasSchedule = HasGameConfigRows(configDb, "event_schedule");
		if (hasCrusade && hasSchedule) {
			if (LoadCrusadeConfig(configDb, this) && LoadScheduleConfig(configDb, this)) {
				loadedSchedules = true;
			}
		}
	}

	if (!loadedSchedules) {
		PutLogList("(!!!) WARNING! Crusade/Schedule configs missing in GameConfigs.db.");
	}

	m_pWarManager->_LinkStrikePointMapIndex();

	if (configDb != nullptr) {
		CloseGameConfigDatabase(configDb);
	}

	m_pWarManager->bReadCrusadeGUIDFile("GameData/CrusadeGUID.txt");
	m_pWarManager->bReadApocalypseGUIDFile("GameData/ApocalypseGUID.txt");
	m_pWarManager->bReadHeldenianGUIDFile("GameData/HeldenianGUID.txt");

	PutLogList("(!) Game Server Activated.");

}




// New 12/05/2004 Changed



//HBest force recall start code

//HBest force recall code

void CGame::SetForceRecallTime(int iClientH)
{
	int iTL_ = 0;
	SYSTEMTIME SysTime;

	if (m_pClientList[iClientH] == 0) return;

	if (m_pClientList[iClientH]->m_iTimeLeft_ForceRecall == 0) {
		// iWarPeriod .


		if (m_sForceRecallTime > 0) {
			m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 20 * m_sForceRecallTime;
		}
		else {
			GetLocalTime(&SysTime);
			switch (SysTime.wDayOfWeek) {
			case 1:	m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 20 * m_sRaidTimeMonday; break;  // 3 2002-09-10 #1
			case 2:	m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 20 * m_sRaidTimeTuesday; break;
			case 3:	m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 20 * m_sRaidTimeWednesday; break;
			case 4:	m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 20 * m_sRaidTimeThursday; break;
			case 5:	m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 20 * m_sRaidTimeFriday; break;
			case 6:	m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 20 * m_sRaidTimeSaturday; break;
			case 0:	m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 20 * m_sRaidTimeSunday; break;
			}
		}
	}
	else { // if (m_pClientList[iClientH]->m_iTimeLeft_ForceRecall == 0) 
		if (m_sForceRecallTime > 0) {
			iTL_ = 20 * m_sForceRecallTime;
		}
		else {

			GetLocalTime(&SysTime);
			switch (SysTime.wDayOfWeek) {
			case 1:	iTL_ = 20 * m_sRaidTimeMonday; break;  // 3 2002-09-10 #1
			case 2:	iTL_ = 20 * m_sRaidTimeTuesday; break;
			case 3:	iTL_ = 20 * m_sRaidTimeWednesday; break;
			case 4:	iTL_ = 20 * m_sRaidTimeThursday; break;
			case 5:	iTL_ = 20 * m_sRaidTimeFriday; break;
			case 6:	iTL_ = 20 * m_sRaidTimeSaturday; break;
			case 0:	iTL_ = 20 * m_sRaidTimeSunday; break;
			}
		}

		if (m_pClientList[iClientH]->m_iTimeLeft_ForceRecall > iTL_)
			m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 1;

	}

	m_pClientList[iClientH]->m_bIsWarLocation = true;
	return;
}

void CGame::CheckForceRecallTime(int iClientH)
{
	SYSTEMTIME SysTime;
	int iTL_;

	if (m_pClientList[iClientH] == 0) return;


	if (m_pClientList[iClientH]->m_iTimeLeft_ForceRecall == 0) {
		// has admin set a recall time ??
		if (m_sForceRecallTime > 0) {
			m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = m_sForceRecallTime * 20;
		}
		// use standard recall time calculations
		else {
			GetLocalTime(&SysTime);
			switch (SysTime.wDayOfWeek) {
			case 1:	m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 20 * m_sRaidTimeMonday; break;  // 3 2002-09-10 #1
			case 2:	m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 20 * m_sRaidTimeTuesday; break;
			case 3:	m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 20 * m_sRaidTimeWednesday; break;
			case 4:	m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 20 * m_sRaidTimeThursday; break;
			case 5:	m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 20 * m_sRaidTimeFriday; break;
			case 6:	m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 20 * m_sRaidTimeSaturday; break;
			case 0:	m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = 20 * m_sRaidTimeSunday; break;
			}
		}
	}
	else {
		// has admin set a recall time ??
		if (m_sForceRecallTime > 0) {
			iTL_ = m_sForceRecallTime * 20;
		}
		// use standard recall time calculations
		else {
			GetLocalTime(&SysTime);
			switch (SysTime.wDayOfWeek) {
			case 1:	iTL_ = 20 * m_sRaidTimeMonday; break;  // 3 2002-09-10 #1
			case 2:	iTL_ = 20 * m_sRaidTimeTuesday; break;
			case 3:	iTL_ = 20 * m_sRaidTimeWednesday; break;
			case 4:	iTL_ = 20 * m_sRaidTimeThursday; break;
			case 5:	iTL_ = 20 * m_sRaidTimeFriday; break;
			case 6:	iTL_ = 20 * m_sRaidTimeSaturday; break;
			case 0:	iTL_ = 20 * m_sRaidTimeSunday; break;
			}
		}
		if (m_pClientList[iClientH]->m_iTimeLeft_ForceRecall > iTL_)
			m_pClientList[iClientH]->m_iTimeLeft_ForceRecall = iTL_;
	}

	m_pClientList[iClientH]->m_bIsWarLocation = true;
	return;

}

int ITEMSPREAD_FIEXD_COORD[25][2] =
{
	{ 0,  0},
	{ 1,  0},
	{ 1,  1},
	{ 0,  1},
	{-1,  1},
	{-1,  0},
	{-1, -1},
	{ 0, -1},
	{ 1, -1},
	{ 2, -1},
	{ 2,  0},
	{ 2,  1},
	{ 2,  2},
	{ 1,  2},
	{ 0,  2},
	{-1,  2},
	{-2,  2},
	{-2,  1},
	{-2,  0},
	{-2, -1},
	{-2, -2},
	{-1, -2},
	{ 0, -2},
	{ 1, -2},
	{ 2, -2},
};




//New Changed 11/05/2004


bool CGame::_bRegisterMap(char* pName)
{
	
	char cTmpName[11], cTxt[120];

	std::memset(cTmpName, 0, sizeof(cTmpName));
	strcpy(cTmpName, pName);
	for(int i = 0; i < MaxMaps; i++)
		if ((m_pMapList[i] != 0) && (memcmp(m_pMapList[i]->m_cName, cTmpName, 10) == 0)) {
			std::snprintf(cTxt, sizeof(cTxt), "Map already installed: %s", cTmpName);
			PutLogListLevel(LOG_LEVEL_ERROR, cTxt);
			return false;
		}

	for(int i = 0; i < MaxMaps; i++)
		if (m_pMapList[i] == 0) {
			m_pMapList[i] = new class CMap(this);
			if (m_pMapList[i]->bInit(pName) == false) {
				std::snprintf(cTxt, sizeof(cTxt), "Map data load failed: %s", pName);
				PutLogListLevel(LOG_LEVEL_ERROR, cTxt);
				return false;
			};

			if ((m_iMiddlelandMapIndex == -1) && (strcmp("middleland", pName) == 0))
				m_iMiddlelandMapIndex = i;

			if ((m_iAresdenMapIndex == -1) && (strcmp("aresden", pName) == 0))
				m_iAresdenMapIndex = i;

			if ((m_iElvineMapIndex == -1) && (strcmp("elvine", pName) == 0))
				m_iElvineMapIndex = i;

			if ((m_iBTFieldMapIndex == -1) && (strcmp("BtField", pName) == 0))
				m_iBTFieldMapIndex = i;

			if ((m_iGodHMapIndex == -1) && (strcmp("GodH", pName) == 0))
				m_iGodHMapIndex = i;

			m_iTotalMaps++;
			return true;
		}

	std::snprintf(cTxt, sizeof(cTxt), "Map cannot be added (no space): %s", pName);
	PutLogListLevel(LOG_LEVEL_ERROR, cTxt);
	return false;
}

//New Changed 11/05/2004


void CGame::ShowClientMsg(int iClientH, char* pMsg)
{
	char cTemp[256];
	std::memset(cTemp, 0, sizeof(cTemp));

	auto& chatPkt = *reinterpret_cast<hb::net::PacketChatMsg*>(cTemp);
	chatPkt.header.msg_id = MsgId::CommandChatMsg;
	chatPkt.header.msg_type = 0;
	chatPkt.reserved1 = 0;
	chatPkt.reserved2 = 0;
	std::memcpy(chatPkt.name, "HGServer", 8);
	chatPkt.chat_type = 10;

	size_t dwMsgSize = strlen(pMsg);
	if (dwMsgSize > 50) dwMsgSize = 50;
	memcpy(cTemp + sizeof(hb::net::PacketChatMsg), pMsg, dwMsgSize);

	m_pClientList[iClientH]->m_pXSock->iSendMsg(cTemp, dwMsgSize + sizeof(hb::net::PacketChatMsg));
}

void CGame::Command_YellowBall(int iClientH, char* pData, size_t dwMsgSize)
{
	char   seps[] = "= \t\r\n";
	char* token, cBuff[256], cPlayerName[11], cMapName[32];
	int iSoxH, iSoX;

	if (m_pClientList[iClientH] == 0) return;
	if ((dwMsgSize) <= 0) return;

	iSoX = 0;
	for(int i = 0; i < hb::shared::limits::MaxItems; i++)
		if (m_pClientList[iClientH]->m_pItemList[i] != 0) {
			switch (m_pClientList[iClientH]->m_pItemList[i]->m_sIDnum) {
			case 653: iSoX++; iSoxH = i; break;
			}
		}
	if (iSoX > 0) {

		std::memset(cPlayerName, 0, sizeof(cPlayerName));
		std::memset(cBuff, 0, sizeof(cBuff));
		memcpy(cBuff, pData, dwMsgSize);

		token = strtok(NULL, seps);

		token = strtok(NULL, seps);
		if (token == 0) {
			return;
		}

		if (strlen(token) > hb::shared::limits::CharNameLen - 1) {
			memcpy(cPlayerName, token, hb::shared::limits::CharNameLen - 1);
		}
		else {
			memcpy(cPlayerName, token, strlen(token));
		}

		for(int i = 1; i < MaxClients; i++) {
			if (m_pClientList[i] != 0) {
				if (_strnicmp(cPlayerName, m_pClientList[i]->m_cCharName, hb::shared::limits::CharNameLen - 1) == 0) {
					if ((m_pClientList[iClientH]->m_cLocation) != (m_pClientList[i]->m_cLocation)) return;
					std::memset(cMapName, 0, sizeof(cMapName));
					strcpy(cMapName, m_pClientList[i]->m_cMapName);
					ItemLog::Get().LogMisc("YellowBall", m_pClientList[i]->m_cCharName, m_pClientList[i]->m_cIPaddress,
						m_pClientList[i]->m_cMapName, m_pClientList[i]->m_sX, m_pClientList[i]->m_sY, nullptr);
					m_pItemManager->ItemDepleteHandler(iClientH, iSoxH, true);
					RequestTeleportHandler(iClientH, "2   ", cMapName, m_pClientList[i]->m_sX, m_pClientList[i]->m_sY);
					return;
				}
			}
		}
	}
}

void CGame::Command_RedBall(int iClientH, char* pData, size_t dwMsgSize)
{
	char seps[] = "= \t\r\n", cName[hb::shared::limits::NpcNameLen], cNpcName[hb::shared::limits::NpcNameLen], cNpcWaypoint[11];
	int iNamingValue, tX, tY, x, iNpcID;
	int iSoxH, iSoX;

	if (m_pClientList[iClientH] == 0) return;
	if ((memcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cName, "huntzone1", 9) != 0) &&
		(memcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cName, "huntzone2", 9) != 0) &&
		(memcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cName, "huntzone3", 9) != 0) &&
		(memcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cName, "huntzone4", 9) != 0)) return;

	iSoX = 0;
	for(int i = 0; i < hb::shared::limits::MaxItems; i++)
		if (m_pClientList[iClientH]->m_pItemList[i] != 0) {
			switch (m_pClientList[iClientH]->m_pItemList[i]->m_sIDnum) {
			case 652: iSoX++; iSoxH = i; break;
			}
		}
	if (iSoX > 0) {
		iNamingValue = m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->iGetEmptyNamingValue();
		if (iNamingValue == -1) {

		}
		else {

			std::memset(cNpcName, 0, sizeof(cNpcName));
			switch (iDice(1, 5)) {
			case 1: strcpy(cNpcName, "Wyvern"); iNpcID = 66; break;
			case 2: strcpy(cNpcName, "Hellclaw"); iNpcID = 49; break;
			case 3: strcpy(cNpcName, "Fire-Wyvern"); iNpcID = 73; break;
			case 4: strcpy(cNpcName, "Tigerworm"); iNpcID = 50; break;
			case 5: strcpy(cNpcName, "Gagoyle"); iNpcID = 52; break;
			}
			std::memset(cName, 0, sizeof(cName));
			std::snprintf(cName, sizeof(cName), "XX%d", iNamingValue);
			cName[0] = '_';
			cName[1] = m_pClientList[iClientH]->m_cMapIndex + 65;

			std::memset(cNpcWaypoint, 0, sizeof(cNpcWaypoint));

			tX = (int)m_pClientList[iClientH]->m_sX;
			tY = (int)m_pClientList[iClientH]->m_sY;
			int iNpcConfigId = GetNpcConfigIdByName(cNpcName);
			if (bCreateNewNpc(iNpcConfigId, cName, m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cName, 0, (rand() % 9),
				MoveType::Random, &tX, &tY, cNpcWaypoint, 0, 0, -1, false, false) == false) {
				m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->SetNamingValueEmpty(iNamingValue);
			}
			else {
				ItemLog::Get().LogMisc("RedBall", m_pClientList[iClientH]->m_cCharName, m_pClientList[iClientH]->m_cIPaddress,
					m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cName, tX, tY, nullptr);
			}
		}

		for (x = 1; x < MaxClients; x++)
			if ((m_pClientList[x] != 0) && (m_pClientList[x]->m_bIsInitComplete)) {
				SendNotifyMsg(0, x, Notify::SpawnEvent, tX, tY, iNpcID, 0, 0, 0);
			}
		m_pItemManager->ItemDepleteHandler(iClientH, iSoxH, true);
	}
}

void CGame::Command_BlueBall(int iClientH, char* pData, size_t dwMsgSize)

{
	char seps[] = "= \t\r\n";
	char   cName_Master[10], cName_Slave[10], cNpcName[hb::shared::limits::NpcNameLen], cWaypoint[11], cSA;
	int    pX, pY, j, iNum, iNamingValue, iNpcID;
	int x;
	bool   bMaster;
	int iSoxH, iSoX;

	if (m_pClientList[iClientH] == 0) return;
	if ((memcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cName, "huntzone1", 9) != 0) &&
		(memcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cName, "huntzone2", 9) != 0) &&
		(memcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cName, "huntzone3", 9) != 0) &&
		(memcmp(m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cName, "huntzone4", 9) != 0)) return;


	iSoX = 0;
	for(int i = 0; i < hb::shared::limits::MaxItems; i++)
		if (m_pClientList[iClientH]->m_pItemList[i] != 0) {
			switch (m_pClientList[iClientH]->m_pItemList[i]->m_sIDnum) {
			case 654: iSoX++; iSoxH = i; break;
			}
		}
	if (iSoX > 0) {
		iNamingValue = m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->iGetEmptyNamingValue();
		if (iNamingValue == -1) {

		}
		else {

			std::memset(cNpcName, 0, sizeof(cNpcName));

			switch (iDice(1, 38)) {
			case 1: strcpy(cNpcName, "Slime");			iNpcID = 10; break;
			case 2: strcpy(cNpcName, "Giant-Ant");		iNpcID = 15; break;
			case 3: strcpy(cNpcName, "Zombie");			iNpcID = 17; break;
			case 4: strcpy(cNpcName, "Scorpion");		iNpcID = 16; break;
			case 5: strcpy(cNpcName, "Skeleton");		iNpcID = 11; break;
			case 6: strcpy(cNpcName, "Orc-Mage");		iNpcID = 14; break;
			case 7: strcpy(cNpcName, "Clay-Golem");		iNpcID = 23; break;
			case 8: strcpy(cNpcName, "Stone-Golem");	iNpcID = 12; break;
			case 9: strcpy(cNpcName, "Hellbound");		iNpcID = 27; break;
			case 10: strcpy(cNpcName, "Giant-Frog");	iNpcID = 57; break;
			case 11: strcpy(cNpcName, "Troll");			iNpcID = 28; break;
			case 12: strcpy(cNpcName, "Cyclops");		iNpcID = 13; break;
			case 13: strcpy(cNpcName, "Ice-Golem");		iNpcID = 65; break;
			case 14: strcpy(cNpcName, "Beholder");		iNpcID = 53; break;
			case 15: strcpy(cNpcName, "Cannibal-Plant"); iNpcID = 60; break;
			case 16: strcpy(cNpcName, "Orge");			iNpcID = 29; break;
			case 17: strcpy(cNpcName, "Mountain-Giant"); iNpcID = 58; break;
			case 18: strcpy(cNpcName, "DireBoar");		iNpcID = 62; break;
			case 19: strcpy(cNpcName, "Liche");			iNpcID = 30; break;
			case 20: strcpy(cNpcName, "Stalker");		iNpcID = 48; break;
			case 21: strcpy(cNpcName, "WereWolf");		iNpcID = 33; break;
			case 22: strcpy(cNpcName, "Dark-Elf");		iNpcID = 54; break;
			case 23: strcpy(cNpcName, "Frost");			iNpcID = 63; break;
			case 24: strcpy(cNpcName, "Orc");			iNpcID = 14; break;
			case 25: strcpy(cNpcName, "Ettin");			iNpcID = 59; break;
			case 26: strcpy(cNpcName, "Tentocle");		iNpcID = 80; break;
			case 27: strcpy(cNpcName, "Giant-Crayfish"); iNpcID = 74; break;
			case 28: strcpy(cNpcName, "Giant-Plant");	iNpcID = 76; break;
			case 29: strcpy(cNpcName, "Rudolph");		iNpcID = 61; break;
			case 30: strcpy(cNpcName, "Claw-Turtle");	iNpcID = 72; break;
			case 31: strcpy(cNpcName, "Centaurus");		iNpcID = 71; break;
			case 32: strcpy(cNpcName, "Barlog");		iNpcID = 70; break;
			case 33: strcpy(cNpcName, "Giant-Lizard");	iNpcID = 75; break;
			case 34: strcpy(cNpcName, "MasterMage-Orc"); iNpcID = 77; break;
			case 35: strcpy(cNpcName, "Minotaurs");		iNpcID = 78; break;
			case 36: strcpy(cNpcName, "Unicorn");		iNpcID = 32; break;
			case 37: strcpy(cNpcName, "Nizie");			iNpcID = 79; break;
			}

			iNum = 10;
			cSA = 0;
			pX = m_pClientList[iClientH]->m_sX;
			pY = m_pClientList[iClientH]->m_sY;

			std::snprintf(G_cTxt, sizeof(G_cTxt), "(!) BlueBallEvent: SummonMob (%s)-(%d)", cNpcName, iNum);
			PutLogList(G_cTxt);

			int iNpcConfigId = GetNpcConfigIdByName(cNpcName);
			iNamingValue = m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->iGetEmptyNamingValue();
			if (iNamingValue != -1) {

				std::memset(cName_Master, 0, sizeof(cName_Master));
				std::snprintf(cName_Master, sizeof(cName_Master), "XX%d", iNamingValue);
				cName_Master[0] = '_';
				cName_Master[1] = m_pClientList[iClientH]->m_cMapIndex + 65;
				if ((bMaster = bCreateNewNpc(iNpcConfigId, cName_Master, m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cName, (rand() % 3), cSA, MoveType::Random, &pX, &pY, cWaypoint, 0, 0, -1, false, false, false, true)) == false) {

					m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->SetNamingValueEmpty(iNamingValue);
				}
			}

			for (j = 0; j < (iNum - 1); j++) {
				iNamingValue = m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->iGetEmptyNamingValue();
				if (iNamingValue != -1) {
					// Slave Mob
					std::memset(cName_Slave, 0, sizeof(cName_Slave));
					std::snprintf(cName_Slave, sizeof(cName_Slave), "XX%d", iNamingValue);
					cName_Slave[0] = '_';
					cName_Slave[1] = m_pClientList[iClientH]->m_cMapIndex + 65;


					if (bCreateNewNpc(iNpcConfigId, cName_Slave, m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_cName, (rand() % 3), cSA, MoveType::Random, &pX, &pY, cWaypoint, 0, 0, -1, false, false, false) == false) {

						m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->SetNamingValueEmpty(iNamingValue);
					}
					else {
						// Slave
						if (m_pEntityManager != 0) m_pEntityManager->bSetNpcFollowMode(cName_Slave, cName_Master, hb::shared::owner_class::Npc);
					}
				}
			}

		}
	}

	for (x = 1; x < MaxClients; x++)
		if ((m_pClientList[x] != 0) && (m_pClientList[x]->m_bIsInitComplete)) {
			SendNotifyMsg(0, x, Notify::SpawnEvent, pX, pY, iNpcID, 0, 0, 0);
		}
	m_pItemManager->ItemDepleteHandler(iClientH, iSoxH, true);
}


/*
at the end of client connection have a true switch
at the start of client move handler check if the switch is true
if it is not true add 1 warning, if the warning reaches 3
delete client and log him, if the true switch
*/
//and when a client walks into a map with dynamic portal
//[KLKS] - [Pretty Good Coders] says:
//u gotta inform it
//[KLKS] - [Pretty Good Coders] says:
//or else they wont see it

/*void CGame::OpenApocalypseGate(int iClientH)
{
	if (m_pClientList[iClientH] == 0) return;

	//m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->m_iTotalAliveObject;
	SendNotifyMsg(0, iClientH, Notify::ApocGateOpen, 95, 31, 0, m_pClientList[iClientH]->m_cMapName);
}*/


void CGame::GlobalUpdateConfigs(char cConfigType)
{
	LocalUpdateConfigs(cConfigType);
}

void CGame::LocalUpdateConfigs(char cConfigType)
{
	sqlite3* configDb = nullptr;
	std::string configDbPath;
	bool configDbCreated = false;
	if (!EnsureGameConfigDatabase(&configDb, configDbPath, &configDbCreated) || configDbCreated) {
		PutLogList("(!!!) GameConfigs.db unavailable; cannot reload configs.");
		return;
	}

	bool ok = false;
	if (cConfigType == 1) {
		ok = HasGameConfigRows(configDb, "settings") && LoadSettingsConfig(configDb, this);
		PutLogList(ok ? "(!!!) Settings updated successfully!" : "(!!!) Settings reload failed!");
	}
	if (cConfigType == 3) {
		ok = LoadBannedListConfig(configDb, this);
		PutLogList(ok ? "(!!!) BannedList updated successfully!" : "(!!!) BannedList reload failed!");
	}
	CloseGameConfigDatabase(configDb);
}




void CGame::ReloadNpcConfigs()
{
	sqlite3* configDb = nullptr;
	std::string configDbPath;
	bool configDbCreated = false;
	if (!EnsureGameConfigDatabase(&configDb, configDbPath, &configDbCreated) || configDbCreated)
	{
		PutLogList((char*)"(!) NPC config reload FAILED - GameConfigs.db unavailable");
		return;
	}

	for(int i = 0; i < MaxNpcTypes; i++)
	{
		if (m_pNpcConfigList[i] != 0)
		{
			delete m_pNpcConfigList[i];
			m_pNpcConfigList[i] = 0;
		}
	}

	if (!LoadNpcConfigs(configDb, this))
	{
		PutLogList((char*)"(!) NPC config reload FAILED");
		CloseGameConfigDatabase(configDb);
		return;
	}

	CloseGameConfigDatabase(configDb);
	ComputeConfigHashes();
	PutLogList((char*)"(*) NPC configs reloaded successfully (new spawns will use updated data)");
}

void CGame::SendConfigReloadNotification(bool bItems, bool bMagic, bool bSkills, bool bNpcs)
{
	hb::net::PacketNotifyConfigReload pkt{};
	pkt.header.msg_id = MsgId::NotifyConfigReload;
	pkt.header.msg_type = MsgType::Confirm;
	pkt.reloadItems = bItems ? 1 : 0;
	pkt.reloadMagic = bMagic ? 1 : 0;
	pkt.reloadSkills = bSkills ? 1 : 0;
	pkt.reloadNpcs = bNpcs ? 1 : 0;

	for(int i = 1; i < MaxClients; i++)
	{
		if (m_pClientList[i] != 0 && m_pClientList[i]->m_bIsInitComplete)
			m_pClientList[i]->m_pXSock->iSendMsg((char*)&pkt, sizeof(pkt));
	}
}

void CGame::PushConfigReloadToClients(bool bItems, bool bMagic, bool bSkills, bool bNpcs)
{
	int iCount = 0;
	for(int i = 1; i < MaxClients; i++)
	{
		if (m_pClientList[i] != 0 && m_pClientList[i]->m_bIsInitComplete)
		{
			if (bItems)  m_pItemManager->bSendClientItemConfigs(i);
			if (bMagic)  m_pMagicManager->bSendClientMagicConfigs(i);
			if (bSkills) m_pSkillManager->bSendClientSkillConfigs(i);
			if (bNpcs)   bSendClientNpcConfigs(i);
			iCount++;
		}
	}

	char buf[128];
	std::snprintf(buf, sizeof(buf), "(*) Config reload pushed to %d client(s)", iCount);
	PutLogList(buf);
}






/*void CGame::ApocalypseStarter()
{
 SYSTEMTIME SysTime;
 

	if (m_bIsApocalypseMode ) return;
	if (m_bIsApocalypseStarter == false) return;

	GetLocalTime(&SysTime);

	for(int i = 0; i < MaxApocalypse; i++)
	if	((m_stApocalypseScheduleStart[i].iDay == SysTime.wDayOfWeek) &&
		(m_stApocalypseScheduleStart[i].iHour == SysTime.wHour) &&
		(m_stApocalypseScheduleStart[i].iMinute == SysTime.wMinute)) {
			PutLogList("(!) Automated apocalypse is initiated!");
			GlobalStartApocalypseMode();
			return;
	}
}*/


// New 06/05/2004
// Party Code
void CGame::RequestCreatePartyHandler(int iClientH)
{
	char cData[120]{};

	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_bIsInitComplete == false) return;

	if (m_pClientList[iClientH]->m_iPartyStatus != PartyStatus::Null) {
		return;
	}

	m_pClientList[iClientH]->m_iPartyStatus = PartyStatus::Processing;

	hb::net::PartyOpCreateRequest req{};
	req.op_type = 1;
	req.client_h = static_cast<uint16_t>(iClientH);
	std::memcpy(req.name, m_pClientList[iClientH]->m_cCharName, sizeof(req.name));
	std::memcpy(cData, &req, sizeof(req));

	PartyOperation(cData);

	std::snprintf(G_cTxt, sizeof(G_cTxt), "Request Create Party: %d", iClientH);
	PutLogList(G_cTxt);
}

// Last Updated October 28, 2004 - 3.51 translation
void CGame::PartyOperationResultHandler(char* pData)
{
	char cName[12];
	int iClientH, iPartyID, iTotal;

	uint16_t opType = *reinterpret_cast<uint16_t*>(pData);

	switch (opType) {
	case 1: {
		const auto& res = *reinterpret_cast<const hb::net::PartyOpResultWithStatus*>(pData);
		iClientH = static_cast<int>(res.client_h);
		std::memset(cName, 0, sizeof(cName));
		std::memcpy(cName, res.name, sizeof(res.name));
		iPartyID = static_cast<int>(res.party_id);

		PartyOperationResult_Create(iClientH, cName, res.result, iPartyID);

		std::snprintf(G_cTxt, sizeof(G_cTxt), "party Operation Result: Create(ClientH:%d PartyID:%d)", iClientH, iPartyID);
		PutLogList(G_cTxt);
		break;
	}

	case 2: {
		const auto& res = *reinterpret_cast<const hb::net::PartyOpResultDelete*>(pData);
		iPartyID = static_cast<int>(res.party_id);

		PartyOperationResult_Delete(iPartyID);

		std::snprintf(G_cTxt, sizeof(G_cTxt), "party Operation Result: Delete(PartyID:%d)", iPartyID);
		PutLogList(G_cTxt);
		break;
	}

	case 3: {
		const auto& res = *reinterpret_cast<const hb::net::PartyOpCreateRequest*>(pData);
		iClientH = static_cast<int>(res.client_h);
		std::memset(cName, 0, sizeof(cName));
		std::memcpy(cName, res.name, sizeof(res.name));

		if ((iClientH < 0) && (iClientH > MaxClients)) return;
		if (m_pClientList[iClientH] == 0) return;
		if (_stricmp(m_pClientList[iClientH]->m_cCharName, cName) != 0) return;

		for (int i = 0; i < hb::shared::limits::MaxPartyMembers; i++)
			if (m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iIndex[i] == iClientH) {
				m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iIndex[i] = 0;
				m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iTotalMembers--;

				std::snprintf(G_cTxt, sizeof(G_cTxt), "PartyID:%d member:%d Out(Clear) Total:%d", m_pClientList[iClientH]->m_iPartyID, iClientH, m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iTotalMembers);
				PutLogList(G_cTxt);
				goto PORH_LOOPBREAK1;
			}
	PORH_LOOPBREAK1:

		for (int i = 0; i < hb::shared::limits::MaxPartyMembers - 1; i++)
			if ((m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iIndex[i] == 0) && (m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iIndex[i + 1] != 0)) {
				m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iIndex[i] = m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iIndex[i + 1];
				m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iIndex[i + 1] = 0;
			}

		m_pClientList[iClientH]->m_iPartyID = 0;
		m_pClientList[iClientH]->m_iPartyStatus = PartyStatus::Null;

		std::snprintf(G_cTxt, sizeof(G_cTxt), "Party Status 0: %s", m_pClientList[iClientH]->m_cCharName);
		PutLogList(G_cTxt);

		SendNotifyMsg(0, iClientH, Notify::Party, 8, 0, 0, 0);
		break;
	}

	case 4: {
		const auto& res = *reinterpret_cast<const hb::net::PartyOpResultWithStatus*>(pData);
		iClientH = static_cast<int>(res.client_h);
		std::memset(cName, 0, sizeof(cName));
		std::memcpy(cName, res.name, sizeof(res.name));
		iPartyID = static_cast<int>(res.party_id);

		PartyOperationResult_Join(iClientH, cName, res.result, iPartyID);

		std::snprintf(G_cTxt, sizeof(G_cTxt), "party Operation Result: Join(ClientH:%d PartyID:%d)", iClientH, iPartyID);
		PutLogList(G_cTxt);
		break;
	}

	case 5: {
		const auto& res = *reinterpret_cast<const hb::net::PartyOpResultInfoHeader*>(pData);
		iClientH = static_cast<int>(res.client_h);
		std::memset(cName, 0, sizeof(cName));
		std::memcpy(cName, res.name, sizeof(res.name));
		iTotal = static_cast<int>(res.total);

		char* memberList = pData + sizeof(hb::net::PartyOpResultInfoHeader);
		PartyOperationResult_Info(iClientH, cName, iTotal, memberList);

		std::snprintf(G_cTxt, sizeof(G_cTxt), "party Operation Result: Info(ClientH:%d Total:%d)", iClientH, iTotal);
		PutLogList(G_cTxt);
		break;
	}

	case 6: {
		const auto& res = *reinterpret_cast<const hb::net::PartyOpResultWithStatus*>(pData);
		iClientH = static_cast<int>(res.client_h);
		std::memset(cName, 0, sizeof(cName));
		std::memcpy(cName, res.name, sizeof(res.name));
		iPartyID = static_cast<int>(res.party_id);

		PartyOperationResult_Dismiss(iClientH, cName, res.result, iPartyID);

		std::snprintf(G_cTxt, sizeof(G_cTxt), "party Operation Result: Dismiss(ClientH:%d PartyID:%d)", iClientH, iPartyID);
		PutLogList(G_cTxt);
		break;
	}
	}
}


void CGame::PartyOperationResult_Create(int iClientH, char* pName, int iResult, int iPartyID)
{
	char cData[120];
	

	if (m_pClientList[iClientH] == 0) return;
	if (_stricmp(m_pClientList[iClientH]->m_cCharName, pName) != 0) return;

	switch (iResult) {
	case 0:
		if (m_pClientList[iClientH]->m_iPartyStatus != PartyStatus::Processing) return;
		if (_stricmp(m_pClientList[iClientH]->m_cCharName, pName) != 0) return;

		m_pClientList[iClientH]->m_iPartyID = 0;
		m_pClientList[iClientH]->m_iPartyStatus = PartyStatus::Null;
		m_pClientList[iClientH]->m_iReqJoinPartyClientH = 0;
		SendNotifyMsg(0, iClientH, Notify::Party, 1, 0, 0, 0);
		break;

	case 1:
		if (m_pClientList[iClientH]->m_iPartyStatus != PartyStatus::Processing) return;
		if (_stricmp(m_pClientList[iClientH]->m_cCharName, pName) != 0) return;

		m_pClientList[iClientH]->m_iPartyID = iPartyID;
		m_pClientList[iClientH]->m_iPartyStatus = PartyStatus::Confirm;
		SendNotifyMsg(0, iClientH, Notify::Party, 1, 1, 0, 0);

		for(int i = 0; i < hb::shared::limits::MaxPartyMembers; i++)
			if (m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iIndex[i] == 0) {
				m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iIndex[i] = iClientH;
				m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iTotalMembers++;
				//testcode
				std::snprintf(G_cTxt, sizeof(G_cTxt), "PartyID:%d member:%d New Total:%d", m_pClientList[iClientH]->m_iPartyID, iClientH, m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iTotalMembers);
				PutLogList(G_cTxt);
				goto PORC_LOOPBREAK1;
			}
	PORC_LOOPBREAK1:

		if ((m_pClientList[iClientH]->m_iReqJoinPartyClientH != 0) && (strlen(m_pClientList[iClientH]->m_cReqJoinPartyName) != 0)) {
			std::memset(cData, 0, sizeof(cData));
			hb::net::PartyOpPayload partyOp{};
			partyOp.op_type = 3;
			partyOp.client_h = static_cast<uint16_t>(m_pClientList[iClientH]->m_iReqJoinPartyClientH);
			std::memcpy(partyOp.name, m_pClientList[iClientH]->m_cReqJoinPartyName, sizeof(partyOp.name));
			partyOp.party_id = static_cast<uint16_t>(m_pClientList[iClientH]->m_iPartyID);
			std::memcpy(cData, &partyOp, sizeof(partyOp));
			PartyOperation(cData);
			m_pClientList[iClientH]->m_iReqJoinPartyClientH = 0;
			std::memset(m_pClientList[iClientH]->m_cReqJoinPartyName, 0, sizeof(m_pClientList[iClientH]->m_cReqJoinPartyName));
		}
		break;
	}
}

// Last Updated October 28, 2004 - 3.51 translation
void CGame::PartyOperationResult_Join(int iClientH, char* pName, int iResult, int iPartyID)
{
	

	if (m_pClientList[iClientH] == 0) return;

	switch (iResult) {
	case 0:
		if (m_pClientList[iClientH]->m_iPartyStatus != PartyStatus::Processing) return;
		if (_stricmp(m_pClientList[iClientH]->m_cCharName, pName) != 0) return;

		m_pClientList[iClientH]->m_iPartyID = 0;
		m_pClientList[iClientH]->m_iPartyStatus = PartyStatus::Null;
		SendNotifyMsg(0, iClientH, Notify::Party, 4, 0, 0, pName);

		m_pClientList[iClientH]->m_iReqJoinPartyClientH = 0;
		std::memset(m_pClientList[iClientH]->m_cReqJoinPartyName, 0, sizeof(m_pClientList[iClientH]->m_cReqJoinPartyName));
		break;

	case 1:
		if (m_pClientList[iClientH]->m_iPartyStatus != PartyStatus::Processing) return;
		if (_stricmp(m_pClientList[iClientH]->m_cCharName, pName) != 0) return;

		m_pClientList[iClientH]->m_iPartyID = iPartyID;
		m_pClientList[iClientH]->m_iPartyStatus = PartyStatus::Confirm;
		SendNotifyMsg(0, iClientH, Notify::Party, 4, 1, 0, pName);

		m_pClientList[iClientH]->m_iReqJoinPartyClientH = 0;
		std::memset(m_pClientList[iClientH]->m_cReqJoinPartyName, 0, sizeof(m_pClientList[iClientH]->m_cReqJoinPartyName));

		for(int i = 0; i < hb::shared::limits::MaxPartyMembers; i++)
			if (m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iIndex[i] == 0) {
				m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iIndex[i] = iClientH;
				m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iTotalMembers++;

				std::snprintf(G_cTxt, sizeof(G_cTxt), "PartyID:%d member:%d In(Join) Total:%d", m_pClientList[iClientH]->m_iPartyID, iClientH, m_stPartyInfo[m_pClientList[iClientH]->m_iPartyID].iTotalMembers);
				PutLogList(G_cTxt);
				goto PORC_LOOPBREAK1;
			}
	PORC_LOOPBREAK1:

		for(int i = 1; i < MaxClients; i++)
			if ((i != iClientH) && (m_pClientList[i] != 0) && (m_pClientList[i]->m_iPartyID != 0) && (m_pClientList[i]->m_iPartyID == iPartyID)) {
				SendNotifyMsg(0, i, Notify::Party, 4, 1, 0, pName);
			}
		break;
	}
}

void CGame::PartyOperationResult_Dismiss(int iClientH, char* pName, int iResult, int iPartyID)
{
	
	// iClientH     .

	switch (iResult) {
	case 0:
		break;

	case 1:
		if (iClientH == 0) {
			// iClientH  NULL        .
			for(int i = 1; i < MaxClients; i++)
				if ((m_pClientList[i] != 0) && (_stricmp(m_pClientList[i]->m_cCharName, pName) == 0)) {
					iClientH = i;
					goto PORD_LOOPBREAK;
				}
		PORD_LOOPBREAK:

			for(int i = 0; i < hb::shared::limits::MaxPartyMembers; i++)
				if (m_stPartyInfo[iPartyID].iIndex[i] == iClientH) {
					m_stPartyInfo[iPartyID].iIndex[i] = 0;
					m_stPartyInfo[iPartyID].iTotalMembers--;
					//testcode
					std::snprintf(G_cTxt, sizeof(G_cTxt), "PartyID:%d member:%d Out Total:%d", iPartyID, iClientH, m_stPartyInfo[iPartyID].iTotalMembers);
					PutLogList(G_cTxt);
					goto PORC_LOOPBREAK1;
				}
		PORC_LOOPBREAK1:
			for(int i = 0; i < hb::shared::limits::MaxPartyMembers - 1; i++)
				if ((m_stPartyInfo[iPartyID].iIndex[i] == 0) && (m_stPartyInfo[iPartyID].iIndex[i + 1] != 0)) {
					m_stPartyInfo[iPartyID].iIndex[i] = m_stPartyInfo[iPartyID].iIndex[i + 1];
					m_stPartyInfo[iPartyID].iIndex[i + 1] = 0;
				}

			if (m_pClientList[iClientH] != 0) {
				m_pClientList[iClientH]->m_iPartyID = 0;
				m_pClientList[iClientH]->m_iPartyStatus = PartyStatus::Null;
				m_pClientList[iClientH]->m_iReqJoinPartyClientH = 0;
			}

			for(int i = 1; i < MaxClients; i++)
				if ((m_pClientList[i] != 0) && (m_pClientList[i]->m_iPartyID != 0) && (m_pClientList[i]->m_iPartyID == iPartyID)) {
					SendNotifyMsg(0, i, Notify::Party, 6, 1, 0, pName);
				}
			return;
		}

		if ((m_pClientList[iClientH] != 0) && (m_pClientList[iClientH]->m_iPartyStatus != PartyStatus::Processing)) return;
		if ((m_pClientList[iClientH] != 0) && (_stricmp(m_pClientList[iClientH]->m_cCharName, pName) != 0)) return;

		for(int i = 1; i < MaxClients; i++)
			if ((m_pClientList[i] != 0) && (m_pClientList[i]->m_iPartyID != 0) && (m_pClientList[i]->m_iPartyID == iPartyID)) {
				SendNotifyMsg(0, i, Notify::Party, 6, 1, 0, pName);
			}

		for(int i = 0; i < hb::shared::limits::MaxPartyMembers; i++)
			if (m_stPartyInfo[iPartyID].iIndex[i] == iClientH) {
				m_stPartyInfo[iPartyID].iIndex[i] = 0;
				m_stPartyInfo[iPartyID].iTotalMembers--;
				//testcode
				std::snprintf(G_cTxt, sizeof(G_cTxt), "PartyID:%d member:%d Out Total:%d", iPartyID, iClientH, m_stPartyInfo[iPartyID].iTotalMembers);
				PutLogList(G_cTxt);
				goto PORC_LOOPBREAK2;
			}
	PORC_LOOPBREAK2:
		for(int i = 0; i < hb::shared::limits::MaxPartyMembers - 1; i++)
			if ((m_stPartyInfo[iPartyID].iIndex[i] == 0) && (m_stPartyInfo[iPartyID].iIndex[i + 1] != 0)) {
				m_stPartyInfo[iPartyID].iIndex[i] = m_stPartyInfo[iPartyID].iIndex[i + 1];
				m_stPartyInfo[iPartyID].iIndex[i + 1] = 0;
			}

		if (m_pClientList[iClientH] != 0) {
			m_pClientList[iClientH]->m_iPartyID = 0;
			m_pClientList[iClientH]->m_iPartyStatus = PartyStatus::Null;
			m_pClientList[iClientH]->m_iReqJoinPartyClientH = 0;
		}
		break;
	}
}

void CGame::PartyOperationResult_Delete(int iPartyID)
{
	

	for(int i = 0; i < hb::shared::limits::MaxPartyMembers; i++)
	{
		m_stPartyInfo[iPartyID].iIndex[i] = 0;
		m_stPartyInfo[iPartyID].iTotalMembers = 0;
	}

	for(int i = 1; i < MaxClients; i++)
		if ((m_pClientList[i] != 0) && (m_pClientList[i]->m_iPartyID == iPartyID)) {
			SendNotifyMsg(0, i, Notify::Party, 2, 0, 0, 0);
			m_pClientList[i]->m_iPartyID = 0;
			m_pClientList[i]->m_iPartyStatus = PartyStatus::Null;
			m_pClientList[i]->m_iReqJoinPartyClientH = 0;
			//testcode
			std::snprintf(G_cTxt, sizeof(G_cTxt), "Notify delete party: %d", i);
			PutLogList(G_cTxt);
		}
}


void CGame::RequestJoinPartyHandler(int iClientH, char* pData, size_t dwMsgSize)
{
	char   seps[] = "= \t\r\n";
	char* token, cBuff[256], cData[120], cName[12];

	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_iPartyStatus != PartyStatus::Null) return;
	if ((dwMsgSize) <= 0) return;

	std::memset(cBuff, 0, sizeof(cBuff));
	memcpy(cBuff, pData, dwMsgSize);

	token = strtok(cBuff, seps);

	token = strtok(NULL, seps);
	if (token != 0) {
		std::memset(cName, 0, sizeof(cName));
		strcpy(cName, token);
	}
	else return;

	for (int i = 1; i < MaxClients; i++)
		if ((m_pClientList[i] != 0) && (_stricmp(m_pClientList[i]->m_cCharName, cName) == 0)) {
			if ((m_pClientList[i]->m_iPartyID == 0) || (m_pClientList[i]->m_iPartyStatus != PartyStatus::Confirm)) {
				return;
			}

			std::memset(cData, 0, sizeof(cData));
			hb::net::PartyOpPayload partyOp{};
			partyOp.op_type = 3;
			partyOp.client_h = static_cast<uint16_t>(iClientH);
			std::memcpy(partyOp.name, m_pClientList[iClientH]->m_cCharName, sizeof(partyOp.name));
			partyOp.party_id = static_cast<uint16_t>(m_pClientList[i]->m_iPartyID);
			std::memcpy(cData, &partyOp, sizeof(partyOp));
			PartyOperation(cData);
			return;
		}

	SendNotifyMsg(0, iClientH, Notify::PlayerNotOnGame, 0, 0, 0, cName);
}


void CGame::RequestDismissPartyHandler(int iClientH)
{
	char cData[120]{};

	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_iPartyStatus != PartyStatus::Confirm) return;

	hb::net::PartyOpPayload partyOp{};
	partyOp.op_type = 4;
	partyOp.client_h = static_cast<uint16_t>(iClientH);
	std::memcpy(partyOp.name, m_pClientList[iClientH]->m_cCharName, sizeof(partyOp.name));
	partyOp.party_id = static_cast<uint16_t>(m_pClientList[iClientH]->m_iPartyID);
	std::memcpy(cData, &partyOp, sizeof(partyOp));
	PartyOperation(cData);

	m_pClientList[iClientH]->m_iPartyStatus = PartyStatus::Processing;
}


void CGame::GetPartyInfoHandler(int iClientH)
{
	char cData[120]{};

	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_iPartyStatus != PartyStatus::Confirm) return;

	hb::net::PartyOpPayload partyOp{};
	partyOp.op_type = 5;
	partyOp.client_h = static_cast<uint16_t>(iClientH);
	std::memcpy(partyOp.name, m_pClientList[iClientH]->m_cCharName, sizeof(partyOp.name));
	partyOp.party_id = static_cast<uint16_t>(m_pClientList[iClientH]->m_iPartyID);
	std::memcpy(cData, &partyOp, sizeof(partyOp));
	PartyOperation(cData);
}


void CGame::PartyOperationResult_Info(int iClientH, char* pName, int iTotal, char* pNameList)
{
	if (m_pClientList[iClientH] == 0) return;
	if (_stricmp(m_pClientList[iClientH]->m_cCharName, pName) != 0) return;
	if (m_pClientList[iClientH]->m_iPartyStatus != PartyStatus::Confirm) return;

	SendNotifyMsg(0, iClientH, Notify::Party, 5, 1, iTotal, pNameList);
}

void CGame::RequestDeletePartyHandler(int iClientH)
{
	char cData[120]{};

	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_iPartyID != 0) {
		hb::net::PartyOpPayload partyOp{};
		partyOp.op_type = 4;
		partyOp.client_h = static_cast<uint16_t>(iClientH);
		std::memcpy(partyOp.name, m_pClientList[iClientH]->m_cCharName, sizeof(partyOp.name));
		partyOp.party_id = static_cast<uint16_t>(m_pClientList[iClientH]->m_iPartyID);
		std::memcpy(cData, &partyOp, sizeof(partyOp));
		PartyOperation(cData);
		m_pClientList[iClientH]->m_iPartyStatus = PartyStatus::Processing;
	}
}

void CGame::RequestAcceptJoinPartyHandler(int iClientH, int iResult)
{
	char cData[120]{};
	int iH;

	if (m_pClientList[iClientH] == 0) return;

	switch (iResult) {
	case 0:
		iH = m_pClientList[iClientH]->m_iReqJoinPartyClientH;
		if (m_pClientList[iH] == 0) {
			return;
		}
		if (_stricmp(m_pClientList[iH]->m_cCharName, m_pClientList[iClientH]->m_cReqJoinPartyName) != 0) {
			return;
		}
		if (m_pClientList[iH]->m_iPartyStatus != PartyStatus::Processing) {
			return;
		}
		if ((m_pClientList[iH]->m_iReqJoinPartyClientH != iClientH) || (_stricmp(m_pClientList[iH]->m_cReqJoinPartyName, m_pClientList[iClientH]->m_cCharName) != 0)) {
			return;
		}

		SendNotifyMsg(0, iH, Notify::Party, 7, 0, 0, 0);
		//testcode
		std::snprintf(G_cTxt, sizeof(G_cTxt), "Party join reject(3) ClientH:%d ID:%d", iH, m_pClientList[iH]->m_iPartyID);
		PutLogList(G_cTxt);

		m_pClientList[iH]->m_iPartyID = 0;
		m_pClientList[iH]->m_iPartyStatus = PartyStatus::Null;
		m_pClientList[iH]->m_iReqJoinPartyClientH = 0;
		std::memset(m_pClientList[iH]->m_cReqJoinPartyName, 0, sizeof(m_pClientList[iH]->m_cReqJoinPartyName));

		m_pClientList[iClientH]->m_iReqJoinPartyClientH = 0;
		std::memset(m_pClientList[iClientH]->m_cReqJoinPartyName, 0, sizeof(m_pClientList[iClientH]->m_cReqJoinPartyName));
		break;

	case 1:
		if ((m_pClientList[iClientH]->m_iPartyStatus == PartyStatus::Confirm) && (m_pClientList[iClientH]->m_iPartyID != 0)) {
			iH = m_pClientList[iClientH]->m_iReqJoinPartyClientH;
			if (m_pClientList[iH] == 0) {
				return;
			}
			if (_stricmp(m_pClientList[iH]->m_cCharName, m_pClientList[iClientH]->m_cReqJoinPartyName) != 0) {
				return;
			}
			if (m_pClientList[iH]->m_iPartyStatus != PartyStatus::Processing) {
				return;
			}
			if ((m_pClientList[iH]->m_iReqJoinPartyClientH != iClientH) || (_stricmp(m_pClientList[iH]->m_cReqJoinPartyName, m_pClientList[iClientH]->m_cCharName) != 0)) {
				return;
			}

			std::memset(cData, 0, sizeof(cData));
			hb::net::PartyOpPayload partyOp{};
			partyOp.op_type = 3;
			partyOp.client_h = static_cast<uint16_t>(m_pClientList[iClientH]->m_iReqJoinPartyClientH);
			std::memcpy(partyOp.name, m_pClientList[iClientH]->m_cReqJoinPartyName, sizeof(partyOp.name));
			partyOp.party_id = static_cast<uint16_t>(m_pClientList[iClientH]->m_iPartyID);
			std::memcpy(cData, &partyOp, sizeof(partyOp));
			PartyOperation(cData);
		}
		else {
			iH = m_pClientList[iClientH]->m_iReqJoinPartyClientH;
			if (m_pClientList[iH] == 0) {
				return;
			}
			if (_stricmp(m_pClientList[iH]->m_cCharName, m_pClientList[iClientH]->m_cReqJoinPartyName) != 0) {
				return;
			}
			if (m_pClientList[iH]->m_iPartyStatus != PartyStatus::Processing) {
				return;
			}
			if ((m_pClientList[iH]->m_iReqJoinPartyClientH != iClientH) || (_stricmp(m_pClientList[iH]->m_cReqJoinPartyName, m_pClientList[iClientH]->m_cCharName) != 0)) {
				return;
			}

			if (m_pClientList[iClientH]->m_iPartyStatus == PartyStatus::Null) {
				RequestCreatePartyHandler(iClientH);
			}
			else {
			}
		}
		break;

	case 2:
		if ((m_pClientList[iClientH]->m_iPartyID != 0) && (m_pClientList[iClientH]->m_iPartyStatus == PartyStatus::Confirm)) {
			RequestDismissPartyHandler(iClientH);
		}
		else {
			iH = m_pClientList[iClientH]->m_iReqJoinPartyClientH;

			// NULL  .
			if ((m_pClientList[iH] != 0) && (m_pClientList[iH]->m_iReqJoinPartyClientH == iClientH) &&
				(_stricmp(m_pClientList[iH]->m_cReqJoinPartyName, m_pClientList[iClientH]->m_cCharName) == 0)) {
				m_pClientList[iH]->m_iReqJoinPartyClientH = 0;
				std::memset(m_pClientList[iH]->m_cReqJoinPartyName, 0, sizeof(m_pClientList[iH]->m_cReqJoinPartyName));
			}

			m_pClientList[iClientH]->m_iPartyID = 0;
			m_pClientList[iClientH]->m_iPartyStatus = PartyStatus::Null;
			m_pClientList[iClientH]->m_iReqJoinPartyClientH = 0;
			std::memset(m_pClientList[iClientH]->m_cReqJoinPartyName, 0, sizeof(m_pClientList[iClientH]->m_cReqJoinPartyName));
		}
		break;
	}
}

void CGame::PartyOperation(char* pData)
{
	char cName[12], cData[120]{};

	// Read the incoming payload as PartyOpPayload (superset of all request types)
	const auto& req = *reinterpret_cast<const hb::net::PartyOpPayload*>(pData);
	uint16_t wRequestType = req.op_type;
	int iGSCH = static_cast<int>(req.client_h);
	std::memset(cName, 0, sizeof(cName));
	std::memcpy(cName, req.name, sizeof(req.name));
	int iPartyID = static_cast<int>(req.party_id);

	std::snprintf(G_cTxt, sizeof(G_cTxt), "Party Operation Type: %d Name: %s PartyID:%d", wRequestType, cName, iPartyID);
	PutLogList(G_cTxt);

	switch (wRequestType) {
	case 1: {
		iPartyID = m_pPartyManager->iCreateNewParty(cName);

		hb::net::PartyOpResultWithStatus result{};
		result.op_type = 1;
		result.result = static_cast<uint8_t>(iPartyID != 0 ? 1 : 0);
		result.client_h = static_cast<uint16_t>(iGSCH);
		std::memcpy(result.name, cName, sizeof(result.name));
		result.party_id = static_cast<uint16_t>(iPartyID);
		std::memcpy(cData, &result, sizeof(result));

		PartyOperationResultHandler(cData);
		break;
	}

	case 2:
		break;

	case 3: {
		int bRet = m_pPartyManager->bAddMember(iPartyID, cName);

		hb::net::PartyOpResultWithStatus result{};
		result.op_type = 4;
		result.result = static_cast<uint8_t>(bRet);
		result.client_h = static_cast<uint16_t>(iGSCH);
		std::memcpy(result.name, cName, sizeof(result.name));
		result.party_id = static_cast<uint16_t>(iPartyID);
		std::memcpy(cData, &result, sizeof(result));

		PartyOperationResultHandler(cData);
		break;
	}

	case 4: {
		int bRet = m_pPartyManager->bRemoveMember(iPartyID, cName);

		hb::net::PartyOpResultWithStatus result{};
		result.op_type = 6;
		result.result = static_cast<uint8_t>(bRet);
		result.client_h = static_cast<uint16_t>(iGSCH);
		std::memcpy(result.name, cName, sizeof(result.name));
		result.party_id = static_cast<uint16_t>(iPartyID);
		std::memcpy(cData, &result, sizeof(result));

		PartyOperationResultHandler(cData);
		break;
	}

	case 5:
		m_pPartyManager->bCheckPartyMember(iGSCH, iPartyID, cName);
		break;

	case 6:
		m_pPartyManager->bGetPartyInfo(iGSCH, cName, iPartyID);
		break;

	case 7:
		m_pPartyManager->SetServerChangeStatus(cName, iPartyID);
		break;
	}
}


void CGame::TimeHitPointsUp(int iClientH)
{
	int iMaxHP, iTemp, iTotal;
	double dV1, dV2, dV3;

	if (m_pClientList[iClientH] == 0) return;

	if (m_pClientList[iClientH]->m_bIsInitComplete == false) return;

	if (m_pClientList[iClientH]->m_iHungerStatus <= 0) return;

	if (m_pClientList[iClientH]->m_bIsKilled) return;

	if (m_pClientList[iClientH]->m_bSkillUsingStatus[19]) return;

	iMaxHP = iGetMaxHP(iClientH);

	if (m_pClientList[iClientH]->m_iHP < iMaxHP) {

		iTemp = iDice(1, (m_pClientList[iClientH]->m_iVit));

		if (iTemp < (m_pClientList[iClientH]->m_iVit / 2)) iTemp = (m_pClientList[iClientH]->m_iVit / 2);

		if (m_pClientList[iClientH]->m_iSideEffect_MaxHPdown != 0)

			iTemp -= (iTemp / m_pClientList[iClientH]->m_iSideEffect_MaxHPdown);

		iTotal = iTemp + m_pClientList[iClientH]->m_iHPstock;

		if (m_pClientList[iClientH]->m_iAddHP != 0) {
			dV2 = (double)iTotal;
			dV3 = (double)m_pClientList[iClientH]->m_iAddHP;
			dV1 = (dV3 / 100.0f) * dV2;
			iTotal += (int)dV1;
		}

		m_pClientList[iClientH]->m_iHP += iTotal;

		if (m_pClientList[iClientH]->m_iHP > iMaxHP) m_pClientList[iClientH]->m_iHP = iMaxHP;

		if (m_pClientList[iClientH]->m_iHP <= 0) m_pClientList[iClientH]->m_iHP = 0;
		SendNotifyMsg(0, iClientH, Notify::Hp, 0, 0, 0, 0);
	}
	m_pClientList[iClientH]->m_iHPstock = 0;
}




/*void CGame::CalculateEnduranceDecrement(short sTargetH, short sAttackerH, char cTargetType, int iArmorType)
{
 short sItemIndex;
 int iDownValue = 1, iHammerChance = 100;

	if (m_pClientList[sTargetH] == 0) return;

	if ((cTargetType == hb::shared::owner_class::Player) && (m_pClientList[sAttackerH] != 0 )) {
		if ((cTargetType == hb::shared::owner_class::Player) && (m_pClientList[sTargetH]->m_cSide != m_pClientList[sAttackerH]->m_cSide)) {
			switch (m_pClientList[sAttackerH]->m_sUsingWeaponSkill) {
				case 14:
					if ((31 == m_pClientList[sAttackerH]->m_appearance.iWeaponType) || (32 == m_pClientList[sAttackerH]->m_appearance.iWeaponType)) {
						sItemIndex = m_pClientList[sAttackerH]->m_sItemEquipmentStatus[ToInt(EquipPos::TwoHand)];
						if ((sItemIndex != -1) && (m_pClientList[sAttackerH]->m_pItemList[sItemIndex] != 0)) {
							if (m_pClientList[sAttackerH]->m_pItemList[sItemIndex]->m_sIDnum == 761) { // BattleHammer
								iDownValue = 30;
								break;
							}
							if (m_pClientList[sAttackerH]->m_pItemList[sItemIndex]->m_sIDnum == 762) { // GiantBattleHammer
								iDownValue = 35;
								break;
							}
							if (m_pClientList[sAttackerH]->m_pItemList[sItemIndex]->m_sIDnum == 843) { // BarbarianHammer
								iDownValue = 30;
								break;
							}
							if (m_pClientList[sAttackerH]->m_pItemList[sItemIndex]->m_sIDnum == 745) { // BarbarianHammer
								iDownValue = 30;
								break;
							}
						}
					}
					else {
						iDownValue = 20; break;
					}
				case 10: iDownValue = 3; break;
				default: iDownValue = 1; break;
				}

				if (m_pClientList[sTargetH]->m_bIsSpecialAbilityEnabled ) {
					switch (m_pClientList[sTargetH]->m_iSpecialAbilityType)
						case 52: iDownValue = 0; iHammerChance = 0;
				}
			}
		}

		if ((m_pClientList[sTargetH]->m_cSide != 0) && (m_pClientList[sTargetH]->m_pItemList[iArmorType]->m_wCurLifeSpan > 0)) {
				m_pClientList[sTargetH]->m_pItemList[iArmorType]->m_wCurLifeSpan -= iDownValue;
		}

		if ((m_pClientList[sTargetH]->m_pItemList[iArmorType]->m_wCurLifeSpan <= 0) || (m_pClientList[sTargetH]->m_pItemList[iArmorType]->m_wCurLifeSpan > 64000)) {
			m_pClientList[sTargetH]->m_pItemList[iArmorType]->m_wCurLifeSpan = 0;
			SendNotifyMsg(0, sTargetH, Notify::ItemLifeSpanEnd, m_pClientList[sTargetH]->m_pItemList[iArmorType]->m_cEquipPos, iArmorType, 0, 0);
			m_pItemManager->ReleaseItemHandler(sTargetH, iArmorType, true);
			return;
		}

	/*try
	{
		if (m_pClientList[sAttackerH] != 0) {
			if (cTargetType == hb::shared::owner_class::Player) {
				sItemIndex = m_pClientList[sAttackerH]->m_sItemEquipmentStatus[ToInt(EquipPos::TwoHand)];
				if ((sItemIndex != -1) && (m_pClientList[sAttackerH]->m_pItemList[sItemIndex] != 0)) {
					if ((m_pClientList[sAttackerH]->m_pItemList[sItemIndex]->m_sIDnum == 617) || (m_pClientList[sAttackerH]->m_pItemList[sItemIndex]->m_sIDnum == 618) || (m_pClientList[sAttackerH]->m_pItemList[sItemIndex]->m_sIDnum == 619) || (m_pClientList[sAttackerH]->m_pItemList[sItemIndex]->m_sIDnum == 873) || (m_pClientList[sAttackerH]->m_pItemList[sItemIndex]->m_sIDnum == 874) || (m_pClientList[sAttackerH]->m_pItemList[sItemIndex]->m_sIDnum == 75) || (m_pClientList[sAttackerH]->m_pItemList[sItemIndex]->m_sIDnum == 76)) {
						m_pClientList[sAttackerH]->m_sUsingWeaponSkill = 6;
						return;
					}
				}
			}
		}*/

		/*if (m_pClientList[sAttackerH] != 0) {
		if (cTargetType == hb::shared::owner_class::Player) {
		if ((m_pClientList[sAttackerH]->m_sUsingWeaponSkill == 14) && (iHammerChance == 100)) {
			if (m_pClientList[sTargetH]->m_pItemList[iArmorType]->m_wMaxLifeSpan < 2000) {
				iHammerChance = iDice(6, (m_pClientList[sTargetH]->m_pItemList[iArmorType]->m_wMaxLifeSpan - m_pClientList[sTargetH]->m_pItemList[iArmorType]->m_wCurLifeSpan));
			}
			else {
				iHammerChance = iDice(4, (m_pClientList[sTargetH]->m_pItemList[iArmorType]->m_wMaxLifeSpan - m_pClientList[sTargetH]->m_pItemList[iArmorType]->m_wCurLifeSpan));
			}

			if ((31 == m_pClientList[sAttackerH]->m_appearance.iWeaponType) || (32 == m_pClientList[sAttackerH]->m_appearance.iWeaponType)) {
				sItemIndex = m_pClientList[sAttackerH]->m_sItemEquipmentStatus[ToInt(EquipPos::TwoHand)];
				if ((sItemIndex != -1) && (m_pClientList[sAttackerH]->m_pItemList[sItemIndex] != 0)) {
					if (m_pClientList[sAttackerH]->m_pItemList[sItemIndex]->m_sIDnum == 761) { // BattleHammer
						iHammerChance = iHammerChance/2;
					}
					if (m_pClientList[sAttackerH]->m_pItemList[sItemIndex]->m_sIDnum == 762) { // GiantBattleHammer
						iHammerChance = ((iHammerChance*10)/9);
					}
					if (m_pClientList[sAttackerH]->m_pItemList[sItemIndex]->m_sIDnum == 843) { // GiantBattleHammer
						iHammerChance = ((iHammerChance*10)/9);
					}
					if (m_pClientList[sAttackerH]->m_pItemList[sItemIndex]->m_sIDnum == 745) { // GiantBattleHammer
						iHammerChance = ((iHammerChance*10)/9);
					}
				}
			}
			if ((m_pClientList[sTargetH]->m_pItemList[iArmorType]->m_sIDnum == 622) || (m_pClientList[sTargetH]->m_pItemList[iArmorType]->m_sIDnum == 621)) {
				iHammerChance = 0;
			}
			if (m_pClientList[sTargetH]->m_pItemList[iArmorType]->m_wCurLifeSpan < iHammerChance) {
				std::snprintf(G_cTxt, sizeof(G_cTxt), "(iHammerChance (%d), target armor endurance (%d)!", iHammerChance, m_pClientList[sTargetH]->m_pItemList[iArmorType]->m_wCurLifeSpan);
				PutLogList(G_cTxt);
				m_pItemManager->ReleaseItemHandler(sTargetH, iArmorType, true);
				SendNotifyMsg(0, sTargetH, Notify::ItemReleased, m_pClientList[sTargetH]->m_pItemList[iArmorType]->m_cEquipPos, iArmorType, 0, 0);
				return;
			}
		}
		}
		}
	//catch(...)
	{

	}
}*/

// October 19, 2004 - 3.51 translated

// October 19, 2004 - 3.51 translated


// October 19, 2004 - 3.51 translated

// October 19, 2004 - 3.51 translated


// October 19, 2004 - 3.51 translated


// October 19,2004 - 3.51 translated


/*
void CGame::StormBringer(int iClientH, short dX, short dY)
{
	char cOwnerType;
	short sOwner, sAppr2, sAttackerWeapon;
	int  iDamage, iTemp, iV1, iV2, iV3;

	//ArchAngel Fix

	if (m_pClientList[iClientH]->m_sItemEquipmentStatus[ToInt(EquipPos::RightHand)] != -1) {
		m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->GetOwner(&sOwner, &cOwnerType, dX, dY);

		iTemp = m_pClientList[iClientH]->m_sItemEquipmentStatus[ToInt(EquipPos::RightHand)];
		sAppr2 = (short)((m_pClientList[iClientH]->m_appearance.bIsWalking) >> 12);

		if (m_pClientList[iClientH]->m_pItemList[iTemp]->m_sIDnum == hb::shared::item::ItemId::StormBringer){

			switch (cOwnerType) {
			case hb::shared::owner_class::Player:
				if (sAppr2 != 0) {
					iV1 = m_pClientList[iClientH]->m_cAttackDiceThrow_L;
					iV2 = m_pClientList[iClientH]->m_cAttackDiceRange_L;
					iV3 = m_pClientList[iClientH]->m_cAttackBonus_L;

					if (m_pClientList[iClientH]->m_cMagicEffectStatus[ hb::shared::magic::Berserk ] != 0){
						iDamage = iDice(iV1*2,iV2*2)+iV3;
					}
					else{
						iDamage = iDice(iV1,iV2)+iV3;
					}

					m_pClientList[sOwner]->m_iHP -= iDamage;
					if (m_pClientList[sOwner]->m_iHP <= 0){
						sAttackerWeapon = 1;
						m_pClientList[sOwner]->m_iHP = 0;

						m_pClientList[sOwner]->m_bIsKilled = true;
						m_pClientList[sOwner]->m_iLastDamage = iDamage;
						SendNotifyMsg(0, sOwner, Notify::Hp, 0, 0, 0, 0);
						SendEventToNearClient_TypeA(sOwner, hb::shared::owner_class::Player, MsgId::EventMotion, Type::Dying, iDamage, sAttackerWeapon, 0);
						m_pMapList[m_pClientList[sOwner]->m_cMapIndex]->ClearOwner(14, sOwner, hb::shared::owner_class::Player, m_pClientList[sOwner]->m_sX, m_pClientList[sOwner]->m_sY);
						m_pMapList[m_pClientList[sOwner]->m_cMapIndex]->SetDeadOwner(sOwner, hb::shared::owner_class::Player, m_pClientList[sOwner]->m_sX, m_pClientList[sOwner]->m_sY);
					}
					else{
						SendNotifyMsg(0, sOwner, Notify::Hp, 0, 0, 0, 0);
						SendEventToNearClient_TypeA(sOwner, hb::shared::owner_class::Player, MsgId::EventMotion, Type::Damage, iDamage, 0, 0);
					}
				}
				break;

			case hb::shared::owner_class::Npc:
				if (sAppr2 != 0) {
					if (m_pNpcList[sOwner]->m_cSize == 0){
						iV1 = m_pClientList[iClientH]->m_cAttackDiceThrow_SM;
						iV2 = m_pClientList[iClientH]->m_cAttackDiceRange_SM;
						iV3 = m_pClientList[iClientH]->m_cAttackBonus_SM;
					}
					else if (m_pNpcList[sOwner]->m_cSize == 1){
						iV1 = m_pClientList[iClientH]->m_cAttackDiceThrow_L;
						iV2 = m_pClientList[iClientH]->m_cAttackDiceRange_L;
						iV3 = m_pClientList[iClientH]->m_cAttackBonus_L;
					}

					if (m_pClientList[iClientH]->m_cMagicEffectStatus[ hb::shared::magic::Berserk ] != 0){
						iDamage = iDice(iV1*2,iV2*2)+iV3;
					}
					else{
						iDamage = iDice(iV1,iV2)+iV3;
					}

					m_pNpcList[sOwner]->m_iHP -= iDamage;
					if (m_pNpcList[sOwner]->m_iHP <= 0){
						sAttackerWeapon = 1;
						m_pNpcList[sOwner]->m_iHP = 0;

						m_pNpcList[sOwner]->m_sBehaviorTurnCount = 0;
						m_pNpcList[sOwner]->m_cBehavior = Behavior::Dead;
						m_pNpcList[sOwner]->m_dwDeadTime = GameClock::GetTimeMS();
						SendEventToNearClient_TypeA(sOwner, hb::shared::owner_class::Npc, MsgId::EventMotion, Type::Dying, iDamage, sAttackerWeapon, 0);
						m_pMapList[m_pNpcList[sOwner]->m_cMapIndex]->ClearOwner(10, sOwner, hb::shared::owner_class::Npc, m_pNpcList[sOwner]->m_sX, m_pNpcList[sOwner]->m_sY);
						m_pMapList[m_pNpcList[sOwner]->m_cMapIndex]->SetDeadOwner(sOwner, hb::shared::owner_class::Npc, m_pNpcList[sOwner]->m_sX, m_pNpcList[sOwner]->m_sY);
					}
					else{
						SendEventToNearClient_TypeA(sOwner, hb::shared::owner_class::Npc, MsgId::EventMotion, Type::Damage, iDamage, 0, 0);
					}
				}
				break;
			}
		}
	}
}*/

bool CGame::_bCheckCharacterData(int iClientH)
{
	

	if ((m_pClientList[iClientH]->m_iStr > CharPointLimit) || (m_pClientList[iClientH]->m_iVit > CharPointLimit) || (m_pClientList[iClientH]->m_iDex > CharPointLimit) ||
		(m_pClientList[iClientH]->m_iMag > CharPointLimit) || (m_pClientList[iClientH]->m_iInt > CharPointLimit) || (m_pClientList[iClientH]->m_iCharisma > CharPointLimit)) {
		try
		{
			std::snprintf(G_cTxt, sizeof(G_cTxt), "Packet Editing: (%s) Player: (%s) stat points are greater then server accepts.", m_pClientList[iClientH]->m_cIPaddress, m_pClientList[iClientH]->m_cCharName);
			PutHackLogFileList(G_cTxt);
			return false;
		}
		catch (...)
		{

		}
	}

	if ((m_pClientList[iClientH]->m_iLevel > m_iMaxLevel) ) {
		try
		{
			std::snprintf(G_cTxt, sizeof(G_cTxt), "Packet Editing: (%s) Player: (%s) level above max server level.", m_pClientList[iClientH]->m_cIPaddress, m_pClientList[iClientH]->m_cCharName);
			PutHackLogFileList(G_cTxt);
			return false;
		}
		catch (...)
		{

		}
	}

	if (m_pClientList[iClientH]->m_iExp < 0) {
		try
		{
			std::snprintf(G_cTxt, sizeof(G_cTxt), "Packet Editing: (%s) Player: (%s) experience is below 0 - (Exp:%d).", m_pClientList[iClientH]->m_cIPaddress, m_pClientList[iClientH]->m_cCharName, m_pClientList[iClientH]->m_iExp);
			PutHackLogFileList(G_cTxt);
			return false;
		}
		catch (...)
		{

		}
	}

	if ((m_pClientList[iClientH]->m_iHP > iGetMaxHP(iClientH)) ) {
		try
		{
			if (m_pClientList[iClientH]->m_pItemList[(m_pClientList[iClientH]->m_sItemEquipmentStatus[ToInt(EquipPos::RightHand)])] != 0) {
				if ((m_pClientList[iClientH]->m_pItemList[(m_pClientList[iClientH]->m_sItemEquipmentStatus[ToInt(EquipPos::RightHand)])]->m_sIDnum == 492) || (m_pClientList[iClientH]->m_pItemList[(m_pClientList[iClientH]->m_sItemEquipmentStatus[ToInt(EquipPos::RightHand)])]->m_sIDnum == 491)) {
					if (m_pClientList[iClientH]->m_iHP > (4 * (iGetMaxHP(iClientH) / 5))) {

					}
				}
			}
			else if (m_pClientList[iClientH]->m_pItemList[(m_pClientList[iClientH]->m_sItemEquipmentStatus[ToInt(EquipPos::TwoHand)])] != 0) {
				if ((m_pClientList[iClientH]->m_pItemList[(m_pClientList[iClientH]->m_sItemEquipmentStatus[ToInt(EquipPos::TwoHand)])]->m_sIDnum == 490)) {
					if (m_pClientList[iClientH]->m_iHP > (4 * (iGetMaxHP(iClientH) / 5))) {

					}
				}
			}
			else {
				std::snprintf(G_cTxt, sizeof(G_cTxt), "Packet Editing: (%s) Player: (%s) HP: current/maximum (%d/%d).", m_pClientList[iClientH]->m_cIPaddress, m_pClientList[iClientH]->m_cCharName, m_pClientList[iClientH]->m_iHP, iGetMaxHP(iClientH));
				PutHackLogFileList(G_cTxt);
				return false;
			}
		}
		catch (...)
		{

		}
	}

	if ((m_pClientList[iClientH]->m_iMP > iGetMaxMP(iClientH)) ) {
		try
		{
			std::snprintf(G_cTxt, sizeof(G_cTxt), "Packet Editing: (%s) Player: (%s) MP: current/maximum (%d/%d).", m_pClientList[iClientH]->m_cIPaddress, m_pClientList[iClientH]->m_cCharName, m_pClientList[iClientH]->m_iMP, iGetMaxMP(iClientH));
			PutHackLogFileList(G_cTxt);
			return false;
		}
		catch (...)
		{

		}
	}

	if ((m_pClientList[iClientH]->m_iSP > iGetMaxSP(iClientH)) ) {
		try
		{
			std::snprintf(G_cTxt, sizeof(G_cTxt), "Packet Editing: (%s) Player: (%s) SP: current/maximum (%d/%d).", m_pClientList[iClientH]->m_cIPaddress, m_pClientList[iClientH]->m_cCharName, m_pClientList[iClientH]->m_iSP, iGetMaxSP(iClientH));
			PutHackLogFileList(G_cTxt);
			return false;
		}
		catch (...)
		{

		}
	}

	try
	{
		for(int i = 0; i < MaxBanned; i++) {
			if (strlen(m_stBannedList[i].m_cBannedIPaddress) == 0) break; //No more GM's on list
			if ((strlen(m_stBannedList[i].m_cBannedIPaddress)) == (strlen(m_pClientList[iClientH]->m_cIPaddress))) {
				if (memcmp(m_stBannedList[i].m_cBannedIPaddress, m_pClientList[iClientH]->m_cIPaddress, strlen(m_pClientList[iClientH]->m_cIPaddress)) == 0) {
					std::snprintf(G_cTxt, sizeof(G_cTxt), "Client Rejected: Banned: (%s)", m_pClientList[iClientH]->m_cIPaddress);
					PutLogList(G_cTxt);
					return false;
				}
				else {

				}
			}
		}
	}
	catch (...)
	{

	}

	return true;
}




















void CGame::ForceRecallProcess() {
	
	int iMapSide = 0;

	uint32_t dwTime;

	dwTime = GameClock::GetTimeMS();

	for(int i = 1; i < MaxClients; i++) {
		if (m_pClientList[i] != 0) {
			if (m_pClientList[i]->m_bIsInitComplete) {
				//force recall in enemy buidlings at crusade
				iMapSide = iGetMapLocationSide(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cName);
				if ((memcmp(m_pClientList[i]->m_cLocation, "are", 3) == 0) && (iMapSide == 2) && (m_bIsCrusadeMode)) {
					RequestTeleportHandler(i, "2   ", "aresden", -1, -1);
				}
				if ((memcmp(m_pClientList[i]->m_cLocation, "elv", 3) == 0) && (iMapSide == 1) && (m_bIsCrusadeMode)) {
					RequestTeleportHandler(i, "2   ", "elvine", -1, -1);
				}

				//remove mim in building
				if ((memcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cName, "wrhus", 5) == 0)
					|| (strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cName, "gshop_1") == 0)
					|| (strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cName, "bsmith_1") == 0)
					|| (strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cName, "cath_1") == 0)
					|| (strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cName, "CmdHall_1") == 0)
					|| (strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cName, "cityhall_1") == 0)
					|| (strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cName, "gshop_2") == 0)
					|| (strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cName, "bsmith_2") == 0)
					|| (strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cName, "cath_2") == 0)
					|| (strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cName, "CmdHall_2") == 0)
					|| (strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cName, "cityhall_2") == 0)
					|| (memcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cName, "wzdtwr", 6) == 0)
					|| (memcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cName, "gldhall", 7) == 0))
				{
					//m_pStatusEffectManager->SetIllusionFlag(i, hb::shared::owner_class::Player, false);
					if (m_pClientList[i]->m_status.bIllusionMovement) {
						m_pStatusEffectManager->SetIllusionMovementFlag(i, hb::shared::owner_class::Player, false);
						m_pDelayEventManager->bRemoveFromDelayEventList(i, hb::shared::owner_class::Player, hb::shared::magic::Confuse);
						m_pDelayEventManager->bRegisterDelayEvent(sdelay::Type::MagicRelease, hb::shared::magic::Confuse, dwTime + 2, i, hb::shared::owner_class::Player, 0, 0, 0, 4, 0, 0);
					}
				}
			}
			//check gizon errors
			if (m_pClientList[i]->m_iLevel < 180) {
				if (m_pClientList[i]->m_iGizonItemUpgradeLeft > 0) {
					m_pClientList[i]->m_iGizonItemUpgradeLeft = 0;
				}
			}
		}
	}
}

//in stat change, check skillpoints

bool CGame::IsEnemyZone(int i) {
	if (memcmp(m_pClientList[i]->m_cLocation, "elv", 3) == 0) {
		if ((strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cLocationName, "aresden") == 0) || (strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cLocationName, "aresdend1") == 0) || (strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cLocationName, "areuni") == 0) || (strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cLocationName, "huntzone2") == 0) || (strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cLocationName, "huntzone4") == 0)) {
			return true;
		}
	}
	else if (memcmp(m_pClientList[i]->m_cLocation, "are", 3) == 0) {
		if ((strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cLocationName, "elvine") == 0) || (strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cLocationName, "elvined1") == 0) || (strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cLocationName, "elvuni") == 0) || (strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cLocationName, "huntzone1") == 0) || (strcmp(m_pMapList[m_pClientList[i]->m_cMapIndex]->m_cLocationName, "huntzone3") == 0)) {
			return true;
		}
	}
	return false;
}

void CGame::LoteryHandler(int iClientH)
{
	CItem* pItem;
	int     iItemID;
	if (m_pClientList[iClientH] == 0) return;
	switch (iDice(1, 22)) {
	case 1:iItemID = 656; break; // XelimaStone
	case 2:iItemID = 657; break; // MerienStone
	case 3:iItemID = 650; break; // ZemstoneOfSacrifice
	case 4:iItemID = 652; break; // RedBall
	case 5:iItemID = 654; break; // BlueBall
	case 6:iItemID = 881; break; // ArmorDye(Indigo)
	case 7:iItemID = 882; break; // ArmorDye(CrimsonRed)
	case 8:iItemID = 883; break; // ArmorDye(Gold)
	case 9:iItemID = 884; break; // ArmorDye(Aqua)
	case 10:iItemID = 885; break; // ArmorDye(Pink)
	case 11:iItemID = 886; break; // ArmorDye(Violet)
	case 12:iItemID = 887; break; // ArmorDye(Blue) 
	case 13:iItemID = 888; break; // ArmorDye(Khaki) 
	case 14:iItemID = 889; break; // ArmorDye(Yellow) 
	case 15:iItemID = 890; break; // ArmorDye(Red) 
	case 16:iItemID = 971; break; // ArmorDye(Green)
	case 17:iItemID = 972; break; // ArmorDye(Black) 
	case 18:iItemID = 973; break; // ArmorDye(Knight) 
	case 19:iItemID = 970; break; // CritCandy
	case 20:iItemID = 651; break; // GreenBall
	case 21:iItemID = 653; break; // YellowBall
	case 22:iItemID = 655; break; // PearlBall
	}

	//chance
	if (iDice(1, 120) <= 3) iItemID = 650;//ZemstoneOfSacrifice
	//chance

	pItem = new CItem;
	if (m_pItemManager->_bInitItemAttr(pItem, iItemID) == false) {
		delete pItem;
	}
	else {
		m_pMapList[m_pClientList[iClientH]->m_cMapIndex]->bSetItem(m_pClientList[iClientH]->m_sX,
			m_pClientList[iClientH]->m_sY, pItem);
		SendEventToNearClient_TypeB(MsgId::EventCommon, CommonType::ItemDrop, m_pClientList[iClientH]->m_cMapIndex,
			m_pClientList[iClientH]->m_sX, m_pClientList[iClientH]->m_sY,
			pItem->m_sIDnum, 0, pItem->m_cItemColor, pItem->m_dwAttribute);
	}


}


//Angel Code By SlammeR(I dont know if it works)
/*void CGame::GetAngelMantleHandler(int iClientH,int iItemID,char * pString)
{
 int   i, iNum, iRet, iEraseReq;
 char  * cp, cData[256], cItemName[hb::shared::limits::ItemNameLen];
 CItem * pItem;
 uint32_t * dwp;
 short * sp;
 uint16_t  * wp;

	if (m_pClientList[iClientH] == 0) return;
	if (m_pClientList[iClientH]->m_iGizonItemUpgradeLeft < 5) return;
	if (m_pClientList[iClientH]->m_cSide == 0) return;
	if (m_pItemManager->_iGetItemSpaceLeft(iClientH) == 0) {
		m_pItemManager->SendItemNotifyMsg(iClientH,	Notify::CannotCarryMoreItem, 0, 0);
		return;
	}

	//Prevents a crash if item dosent exist
	if (m_pItemConfigList[iItemID] == 0)  return;

	switch(iItemID) {
	//Angels
	case 908: //AngelicPandent(STR)
	case 909: //AngelicPandent(DEX)
		case 910: //AngelicPandent(INT)
		case 911: //AngelicPandent(MAG)
		if(m_pClientList[iClientH]->m_iGizonItemUpgradeLeft<5) return;
		m_pClientList[iClientH]->m_iGizonItemUpgradeLeft -= 5;
		break;


  default:
	 return;
	 break;
  }

  std::memset(cItemName, 0, sizeof(cItemName));
  memcpy(cItemName,m_pItemConfigList[iItemID]->m_cName, hb::shared::limits::ItemNameLen - 1);

  iNum = 1;
  for(int i = 1; i <= iNum; i++)
  {
	 pItem = new CItem;
	 if (m_pItemManager->_bInitItemAttr(pItem, cItemName) == false)
	 {
		delete pItem;
	 }
	 else {

		if (m_pItemManager->_bAddClientItemList(iClientH, pItem, &iEraseReq) ) {
		   if (m_pClientList[iClientH]->m_iCurWeightLoad < 0) m_pClientList[iClientH]->m_iCurWeightLoad = 0;

		   std::snprintf(G_cTxt, sizeof(G_cTxt), "(*) Get Angel : Char(%s) Player-Majestic-Points(%d) Angel Obtained(%s)", m_pClientList[iClientH]->m_cCharName, m_pClientList[iClientH]->m_iGizonItemUpgradeLeft, cItemName);
		   PutLogFileList(G_cTxt);

		   pItem->SetTouchEffectType(TouchEffectType::UniqueOwner);
		   pItem->m_sTouchEffectValue1 = m_pClientList[iClientH]->m_sCharIDnum1;
		   pItem->m_sTouchEffectValue2 = m_pClientList[iClientH]->m_sCharIDnum2;
		   pItem->m_sTouchEffectValue3 = m_pClientList[iClientH]->m_sCharIDnum3;

		   iRet = m_pItemManager->SendItemNotifyMsg(iClientH, Notify::ItemObtained, pItem, 0);

		   if (iEraseReq == 1) delete pItem;

		   iCalcTotalWeight(iClientH);

		   switch (iRet) {
		   case sock::Event::QueueFull:
		   case sock::Event::SocketError:
		   case sock::Event::CriticalError:
		   case sock::Event::SocketClosed:
			  DeleteClient(iClientH, true, true);
			  return;
		   }

		   SendNotifyMsg(0, iClientH, Notify::GizonItemUpgradeLeft, m_pClientList[iClientH]->m_iGizonItemUpgradeLeft, 0, 0, 0);
		}
		else
		{
		   delete pItem;

		   iCalcTotalWeight(iClientH);

	   iRet = m_pItemManager->SendItemNotifyMsg(iClientH, Notify::CannotCarryMoreItem, 0, 0);


		   switch (iRet) {
		   case sock::Event::QueueFull:
		   case sock::Event::SocketError:
		   case sock::Event::CriticalError:
		   case sock::Event::SocketClosed:

			  DeleteClient(iClientH, true, true);
			  return;
		   }
		}
	 }
   }
}*/

/*int CGame::iAngelEquip(int iClientH)
{
 int iTemp;
 CItem * cAngelTemp;
	iTemp = m_pClientList[iClientH]->m_sItemEquipmentStatus[ToInt(EquipPos::LeftFinger)];
	cAngelTemp = m_pClientList[iClientH]->m_pItemList[iTemp];
	if ((iTemp != -1) && (cAngelTemp != 0)) {
		if(cAngelTemp->m_sIDnum >= 908){ //AngelicPandent(STR)
				if(cAngelTemp->m_sIDnum >= 909){ //AngelicPandent(DEX)
				if(cAngelTemp->m_sIDnum >= 910){ //AngelicPandent(INT)
				if(cAngelTemp->m_sIDnum >= 911){ //AngelicPandent(MAG)

				return cAngelTemp->m_sIDnum;
			} else {
				return 0;
				}
				}
				}
		}
	}
}*/

/*void CGame::CheckAngelUnequip(int iClientH,int iAngelID)
{
 int iTemp;
 CItem * cAngelTemp;

	iTemp = m_pClientList[iClientH]->m_sItemEquipmentStatus[ToInt(EquipPos::LeftFinger)];
	cAngelTemp = m_pClientList[iClientH]->m_pItemList[iTemp];
	if ((iTemp != -1) && (cAngelTemp->m_sIDnum != iAngelID)) {
		cAngelTemp->m_sIDnum = iAngelID;
	}

}*/

/*********************************************************************************************************************
**  bool CGame::SetAngelFlag(short sOwnerH, char cOwnerType, int iStatus, iTemp)		Snoopy			**
** description	  :: Sets the staus to send or not Angels to every client							**
*********************************************************************************************************************/

/*********************************************************************************************************************
**  bool CGame::GetAngelHandler(int iClientH, char * pData, size_t dwMsgSize)										**
** description	  :: Reversed and coded by Snoopy																	**
*********************************************************************************************************************/
void CGame::GetAngelHandler(int iClientH, char* pData, size_t dwMsgSize)
{
	int   iAngel, iItemID;
	CItem* pItem;
	int   iRet, iEraseReq;
	if (m_pClientList[iClientH] == 0)					 return;
	if (m_pClientList[iClientH]->m_bIsInitComplete == false) return;
	if (m_pItemManager->_iGetItemSpaceLeft(iClientH) == 0)
	{
		m_pItemManager->SendItemNotifyMsg(iClientH, Notify::CannotCarryMoreItem, 0, 0);
		return;
	}
	const auto* req = hb::net::PacketCast<hb::net::PacketRequestAngel>(pData, sizeof(hb::net::PacketRequestAngel));
	if (!req) return;
	iAngel = req->angel_id;

	if (m_pClientList[iClientH]->m_iGizonItemUpgradeLeft < 5) return;

	switch (iAngel) {
	case 1: iItemID = hb::shared::item::ItemId::AngelicPandentSTR; break;
	case 2: iItemID = hb::shared::item::ItemId::AngelicPandentDEX; break;
	case 3: iItemID = hb::shared::item::ItemId::AngelicPandentINT; break;
	case 4: iItemID = hb::shared::item::ItemId::AngelicPandentMAG; break;
	default:
		PutLogList("Gail asked to create a wrong item!");
		return;
	}

	std::snprintf(G_cTxt, sizeof(G_cTxt), "PC(%s) requesting Angel (%d, ItemID:%d).   %s(%d %d)"
		, m_pClientList[iClientH]->m_cCharName
		, iAngel
		, iItemID
		, m_pClientList[iClientH]->m_cMapName
		, m_pClientList[iClientH]->m_sX
		, m_pClientList[iClientH]->m_sY);
	PutLogList(G_cTxt);

	pItem = new CItem;
	if ((m_pItemManager->_bInitItemAttr(pItem, iItemID)))
	{
		m_pClientList[iClientH]->m_iGizonItemUpgradeLeft -= 5;

		pItem->SetTouchEffectType(TouchEffectType::UniqueOwner);
		pItem->m_sTouchEffectValue1 = m_pClientList[iClientH]->m_sCharIDnum1;
		pItem->m_sTouchEffectValue2 = m_pClientList[iClientH]->m_sCharIDnum2;
		pItem->m_sTouchEffectValue3 = m_pClientList[iClientH]->m_sCharIDnum3;
		if (m_pItemManager->_bAddClientItemList(iClientH, pItem, &iEraseReq))
		{
			if (m_pClientList[iClientH]->m_iCurWeightLoad < 0) m_pClientList[iClientH]->m_iCurWeightLoad = 0;

			std::snprintf(G_cTxt, sizeof(G_cTxt), "(*) Get Angel : Char(%s) Player-Majestic-Points(%d) Angel Obtained(ID:%d)", m_pClientList[iClientH]->m_cCharName, m_pClientList[iClientH]->m_iGizonItemUpgradeLeft, iItemID);
			PutLogFileList(G_cTxt);

			iRet = m_pItemManager->SendItemNotifyMsg(iClientH, Notify::ItemObtained, pItem, 0);

			iCalcTotalWeight(iClientH);

			switch (iRet) {
			case sock::Event::QueueFull:
			case sock::Event::SocketError:
			case sock::Event::CriticalError:
			case sock::Event::SocketClosed:
				DeleteClient(iClientH, true, true);
				return;
			}

			SendNotifyMsg(0, iClientH, Notify::GizonItemUpgradeLeft, m_pClientList[iClientH]->m_iGizonItemUpgradeLeft, 0, 0, 0);
		}
		else
		{
			delete pItem;

			iCalcTotalWeight(iClientH);

			iRet = m_pItemManager->SendItemNotifyMsg(iClientH, Notify::CannotCarryMoreItem, 0, 0);

			switch (iRet) {
			case sock::Event::QueueFull:
			case sock::Event::SocketError:
			case sock::Event::CriticalError:
			case sock::Event::SocketClosed:
				DeleteClient(iClientH, true, true);
				return;
			}
		}
	}
	else
	{
		std::snprintf(G_cTxt, sizeof(G_cTxt), "(!) GetAngelHandler: _bInitItemAttr failed for ItemID %d. Item not found in config.", iItemID);
		PutLogList(G_cTxt);
		delete pItem;
	}
}

//50Cent - Repair All


