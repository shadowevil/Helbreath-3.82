// Game.h: interface for the CGame class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)
#define _WINSOCKAPI_

#include <windows.h>
#include "CommonTypes.h"
#include <map>
#include <memory.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>

#include "ActionID_Server.h"
#include "BuildItem.h"
#include "Client.h"
#include "DelayEvent.h"
#include "DynamicObject.h"
#include "DynamicObjectID.h"
#include "Fish.h"
#include "GlobalDef.h"
#include "Magic.h"
#include "Map.h"
#include "MessageIndex.h"
#include "Mineral.h"
#include "Misc.h"
#include "NetMessages.h"
#include "NetworkMsg.h"
#include "Npc.h"
#include "PartyManager.h"
#include "Portion.h"
#include "Quest.h"
#include "ServerMessages.h"
#include "Skill.h"
#include "TeleportLoc.h"
#include "TempNpcItem.h"
#include "UserMessages.h"
#include "Version.h"
#include "Xsocket.h"
#include "winmain.h"

#define DEF_MAXADMINS 50
#define DEF_MAXMAPS 100
#define DEF_MAXAGRICULTURE 200
#define DEF_MAXNPCTYPES 200
#define DEF_MAXBUILDITEMS 300
#define DEF_SERVERSOCKETBLOCKLIMIT 300
#define DEF_MAXBANNED 500
#define DEF_MAXNPCITEMS 1000
#define DEF_MAXCLIENTS 2000
#define DEF_MAXCLIENTLOGINSOCK 2000
#define DEF_MAXNPCS 15000
#define DEF_MAXITEMTYPES 5000
#define DEF_CLIENTTIMEOUT \
  30000 // MODERNIZED: Increased from 10s to 30s for heavy entity rendering
#define DEF_SPUPTIME 10000
#define DEF_POISONTIME 12000
#define DEF_HPUPTIME 15000
#define DEF_MPUPTIME 20000
#define DEF_HUNGERTIME 60000
#define DEF_NOTICETIME 80000
#define DEF_SUMMONTIME 300000
#define DEF_AUTOSAVETIME 600000
#define MAX_HELDENIANTOWER 200
#define DEF_VIEWTILES_X 20
#define DEF_VIEWTILES_Y 15
#define DEF_VIEWRANGE_X (DEF_VIEWTILES_X / 2)
#define DEF_VIEWRANGE_Y (DEF_VIEWTILES_Y / 2)

#define DEF_EXPSTOCKTIME 1000 * 10 // ExpStock�� ����ϴ�? �ð� ����
#define DEF_MSGQUENESIZE \
  100000 // �޽��� ť ������ 10����
#define DEF_AUTOEXPTIME \
  1000 * 60 * 6                 // �ڵ����� ����ġ�� �ö󰡴�
                                // �ð�����
#define DEF_TOTALLEVELUPPOINT 3 // �������� �Ҵ��ϴ� �� ����Ʈ ��

#define DEF_MAXDYNAMICOBJECTS 60000
#define DEF_MAXDELAYEVENTS 60000
#define DEF_GUILDSTARTRANK 12

#define DEF_SSN_LIMIT_MULTIPLY_VALUE 2 // SSN-limit ���ϴ� ��

#define DEF_MAXNOTIFYMSGS \
  300                          // �ִ� �������� �޽���
#define DEF_MAXSKILLPOINTS 700 // ��ų ����Ʈ�� ����
#define DEF_NIGHTTIME 30

#define DEF_CHARPOINTLIMIT 1000 // ������ Ư��ġ�� �ִ밪
#define DEF_RAGPROTECTIONTIME \
  7000                             // �� �� �̻� ������ ������ ���� ��ȣ��
                                   // �޴���
#define DEF_MAXREWARDGOLD 99999999 // �����? �ִ�ġ

#define DEF_ATTACKAI_NORMAL 1         // ������ ����
#define DEF_ATTACKAI_EXCHANGEATTACK 2 // ��ȯ ���� - ����
#define DEF_ATTACKAI_TWOBYONEATTACK 3 // 2-1 ����, ����

#define DEF_MAXFISHS 200
#define DEF_MAXMINERALS 200
#define DEF_MAXCROPS 200
#define DEF_MAXENGAGINGFISH \
  30                            // �� �����⿡ ���ø� �õ��� �� �ִ� �ִ�
                                // �ο�
#define DEF_MAXPORTIONTYPES 500 // �ִ� ���� ���� ����

#define DEF_SPECIALEVENTTIME 300000 // 600000 // 10��
#define DEF_MAXQUESTTYPE 200
#define DEF_DEF_MAXHELDENIANDOOR 10

#define DEF_MAXSUBLOGSOCK 10

#define DEF_ITEMLOG_GIVE 1
#define DEF_ITEMLOG_DROP 2
#define DEF_ITEMLOG_GET 3
#define DEF_ITEMLOG_DEPLETE 4
#define DEF_ITEMLOG_NEWGENDROP 5

// New 07/05/2004
#define DEF_ITEMLOG_BUY 7
#define DEF_ITEMLOG_SELL 8
#define DEF_ITEMLOG_RETRIEVE 9
#define DEF_ITEMLOG_DEPOSIT 10
#define DEF_ITEMLOG_EXCHANGE 11
#define DEF_ITEMLOG_MAKE 13
#define DEF_ITEMLOG_SUMMONMONSTER 14
#define DEF_ITEMLOG_POISONED 15
#define DEF_ITEMLOG_REPAIR 17
#define DEF_ITEMLOG_SKILLLEARN 12
#define DEF_ITEMLOG_MAGICLEARN 16
#define DEF_ITEMLOG_USE 32

#define DEF_MAXGUILDS 1000 // ���ÿ� ������ �� �ִ� ����

#define DEF_MAXGATESERVERSTOCKMSGSIZE 10000

#define DEF_MAXCONSTRUCTNUM 10
#define DEF_MAXSCHEDULE 10
#define DEF_MAXAPOCALYPSE 7
#define DEF_MAXHELDENIAN 10

// v1.4311-3  �������� �ִ� ����
#define DEF_MAXFIGHTZONE 10

//============================
#define DEF_MINIMUMHITRATIO 15 // ���� ���� Ȯ��
//============================

//============================
#define DEF_MAXIMUMHITRATIO 99 // �ִ� ���� Ȯ��
//============================

//============================
#define DEF_PLAYERMAXLEVEL \
  180 // �ִ� ����: Npc.cfg ���Ͽ� �����Ǿ� ���� ���� ���? m_iPlayerMaxLevel��
      // �Էµȴ�.
//============================

//============================
// New Changed 12/05/2004
#define DEF_GMGMANACONSUMEUNIT 15 // Grand Magic Generator ���� ���� ����.
//============================

#define DEF_MAXCONSTRUCTIONPOINT 30000 // �ִ� ��ȯ ����Ʈ
#define DEF_MAXSUMMONPOINTS 30000
#define DEF_MAXWARCONTRIBUTION 200000

// MOG Definitions - 3.51
// Level up MSG
#define MSGID_LEVELUPSETTINGS 0x11A01000
// 2003-04-14 ���� ����Ʈ�� ���� ������ ���� �ִ�...
// Stat Point Change MSG
#define MSGID_STATECHANGEPOINT 0x11A01001

// #define DEF_NOTIFY_STATECHANGE_FAILED 0x11A01002
// #define DEF_NOTIFY_SETTING_FAILED 0x11A01003
// #define DEF_NOTIFY_STATECHANGE_SUCCESS 0x11A01004
// #define DEF_NOTIFY_SETTING_SUCCESS 0x11A01005

// Mine
// #define DEF_NOTIFY_SETTING_FAILED 0x11A01003
// #define DEF_NOTIFY_SETTING_SUCCESS 0x11A01005
// 2.24
// #define DEF_NOTIFY_SETTING_FAILED 0xBB4
// #define DEF_NOTIFY_SETTING_SUCCESS 0xBB3

#define DEF_STR 0x01
#define DEF_DEX 0x02
#define DEF_INT 0x03
#define DEF_VIT 0x04
#define DEF_MAG 0x05
#define DEF_CHR 0x06

#define DEF_TEST 0xFFFF0000
// #define DEF_TESTSERVER

#define NO_MSGSPEEDCHECK

using namespace std;
typedef unsigned long long u64;
typedef signed long long i64;
typedef unsigned long u32;
typedef signed long i32;
typedef unsigned short u16;
typedef signed short i16;
typedef unsigned char u8;
typedef signed char i8;

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
  int npcType;          // NPC type (15=ShopKeeper, 24=Blacksmith)
  int shopId;           // Which shop inventory to use
  char description[64]; // For documentation
};

struct ShopData
{
  int shopId;
  std::vector<int16_t> itemIds; // List of item IDs available in this shop
};

template <typename T>
static bool In(const T &value, std::initializer_list<T> values)
{
  return std::any_of(values.begin(), values.end(),
                     [&value](const T &x)
                     { return x == value; });
}

template <typename T>
static bool NotIn(const T &value, std::initializer_list<T> values)
{
  return !In(value, values);
}

template <typename T, class = typename enable_if<!is_pointer<T>::value>::type>
static void Push(char *&cp, T value)
{
  auto p = (T *)cp;
  *p = (T)value;
  cp += sizeof(T);
}

template <typename T, class = typename enable_if<!is_pointer<T>::value>::type>
static void Pop(char *&cp, T &v)
{
  T *p = (T *)cp;
  v = *p;
  cp += sizeof(T);
}

static void Push(char *&dest, const char *src, u32 len)
{
  memcpy(dest, src, len);
  dest += len;
}

static void Push(char *&dest, const char *src)
{

  strcpy(dest, src);
  dest += strlen(src) + 1;
}

static void Push(char *&dest, const string &str)
{
  strcpy(dest, str.c_str());
  dest += str.length() + 1;
}

static void Pop(char *&src, char *dest, u32 len)
{
  memcpy(dest, src, len);
  src += len;
}
static void Pop(char *&src, char *dest)
{

  u32 len = strlen(src) + 1;
  memcpy(dest, src, len);
  src += len;
}

static void Pop(char *&src, string &str)
{
  str = src;
  src += str.length() + 1;
}

struct LoginClient
{
  // MODERNIZED: Removed HWND parameter
  LoginClient(HWND hWnd)
  {
    _sock = 0;
    _sock = new class XSocket(DEF_CLIENTSOCKETBLOCKLIMIT);
    _sock->bInitBufferSize(DEF_MSGBUFFERSIZE);
    _timeout_tm = 0;
  }

  u32 _timeout_tm;
  ~LoginClient();
  XSocket *_sock;
  char _ip[21];
};

class CGame
{
public:
  void RequestNoticementHandler(int iClientH);
  bool bSendClientItemConfigs(int iClientH);
  const DropTable *GetDropTable(int id) const;

  LoginClient *_lclients[DEF_MAXCLIENTLOGINSOCK];

  bool bAcceptLogin(XSocket *sock);

  void PartyOperation(char *pData);

  void SetHeldenianMode();
  void LocalStartHeldenianMode(short sV1, short sV2, uint32_t dwHeldenianGUID);
  void GlobalStartHeldenianMode();
  bool UpdateHeldenianStatus();
  void _CreateHeldenianGUID(uint32_t dwHeldenianGUID, int iWinnerSide);
  void ManualStartHeldenianMode(int iClientH, char *pData, uint32_t dwMsgSize);
  void ManualEndHeldenianMode(int iClientH, char *pData, uint32_t dwMsgSize);

  bool _bCheckCharacterData(int iClientH);
  // bool _bDecodeNpcItemConfigFileContents(char * pData, uint32_t dwMsgSize);
  void GlobalUpdateConfigs(char cConfigType);
  void LocalUpdateConfigs(char cConfigType);
  // void UpdateHeldenianStatus();
  void GlobalEndHeldenianMode();
  void LocalEndHeldenianMode();
  bool bNotifyHeldenianWinner();
  void RemoveHeldenianNpc(int iNpcH);
  void RemoveOccupyFlags(int iMapIndex);
  void RequestHeldenianTeleport(int iClientH, char *pData, uint32_t dwMsgSize);
  bool bCheckHeldenianMap(int sAttackerH, int iMapIndex, char cType);
  void SetHeroFlag(short sOwnerH, char cOwnerType, bool bStatus);
  void SetInhibitionCastingFlag(short sOwnerH, char cOwnerType, bool bStatus);
  // void CalculateEnduranceDecrement(short sTargetH, short sAttackerH, char
  // cTargetType, int iArmorType);
  bool bCalculateEnduranceDecrement(short sTargetH, short sAttackerH,
                                    char cTargetType, int iArmorType);
  char _cCheckHeroItemEquipped(int iClientH);
  bool bPlantSeedBag(int iMapIndex, int dX, int dY, int iItemEffectValue1,
                     int iItemEffectValue2, int iClientH);
  void _CheckFarmingAction(short sAttackerH, short sTargetH, bool bType);

  void ApocalypseEnder();

  bool bReadHeldenianGUIDFile(char *cFn);
  bool bReadApocalypseGUIDFile(char *cFn);

  void _CreateApocalypseGUID(uint32_t dwApocalypseGUID);
  void LocalEndApocalypse();
  void LocalStartApocalypse(uint32_t dwApocalypseGUID);
  void GlobalEndApocalypseMode();

  // Lists

  void Command_RedBall(int iClientH, char *pData, uint32_t dwMsgSize);
  void Command_BlueBall(int iClientH, char *pData, uint32_t dwMsgSize);
  void Command_YellowBall(int iClientH, char *pData, uint32_t dwMsgSize);

  // Crusade
  void ManualEndCrusadeMode(int iWinnerSide); // 2.17 (x) 2.14 ( )
  void CrusadeWarStarter();
  bool bReadCrusadeGUIDFile(char *cFn);
  void _CreateCrusadeGUID(uint32_t dwCrusadeGUID, int iWinnerSide);
  void GlobalStartCrusadeMode();
  void GSM_SetGuildTeleportLoc(int iGuildGUID, int dX, int dY, char *pMapName);
  void SyncMiddlelandMapInfo();
  void RemoveCrusadeStructures();
  void _SendMapStatus(int iClientH);
  void MapStatusHandler(int iClientH, int iMode, const char *pMapName);
  void SelectCrusadeDutyHandler(int iClientH, int iDuty);
  void RequestSummonWarUnitHandler(int iClientH, int dX, int dY, char cType,
                                   char cNum, char cMode);
  void RequestGuildTeleportHandler(int iClientH);
  void RequestSetGuildTeleportLocHandler(int iClientH, int dX, int dY,
                                         int iGuildGUID, char *pMapName);
  void MeteorStrikeHandler(int iMapIndex);
  void _LinkStrikePointMapIndex();
  void MeteorStrikeMsgHandler(char cAttackerSide);
  void CollectedManaHandler(uint16_t wAresdenMana, uint16_t wElvineMana);
  void SendCollectedMana();
  void CreateCrusadeStructures();
  void _GrandMagicLaunchMsgSend(int iType, char cAttackerSide);
  void GrandMagicResultHandler(char *cMapName, int iCrashedStructureNum,
                               int iStructureDamageAmount, int iCasualities,
                               int iActiveStructure, int iTotalStrikePoints,
                               char *cData);
  void CalcMeteorStrikeEffectHandler(int iMapIndex);
  void DoMeteorStrikeDamageHandler(int iMapIndex);
  void RequestSetGuildConstructLocHandler(int iClientH, int dX, int dY,
                                          int iGuildGUID, const char *pMapName);
  void GSM_SetGuildConstructLoc(int iGuildGUID, int dX, int dY, char *pMapName);
  void GSM_ConstructionPoint(int iGuildGUID, int iPoint);
  void CheckCommanderConstructionPoint(int iClientH);
  void LocalEndCrusadeMode(int iWinnerSide);
  void LocalStartCrusadeMode(uint32_t dwGuildGUID);
  void CheckCrusadeResultCalculation(int iClientH);
  void CheckHeldenianResultCalculation(int iClientH);
  bool __bSetConstructionKit(int iMapIndex, int dX, int dY, int iType,
                             int iTimeCost, int iClientH);

  // Acidx commands

  void SetForceRecallTime(int iClientH);
  void ApplyCombatKilledPenalty(int iClientH, int cPenaltyLevel,
                                bool bIsSAattacked);

  //  bool bReadTeleportConfigFile(char * cFn);
  //	void RequestTeleportD2Handler(int iClientH, char * pData);

  // Daryl - AdminSettings.cfg

  // Hack Checks
  bool bCheckClientMoveFrequency(int iClientH, uint32_t dwClientTime);
  bool bCheckClientMagicFrequency(int iClientH, uint32_t dwClientTime);
  bool bCheckClientAttackFrequency(int iClientH, uint32_t dwClientTime);

  // bool bCheckClientInvisibility(short iClientH);

  // Hypnotoad functions
  void SetDefenseShieldFlag(short sOwnerH, char cOwnerType, bool bStatus);
  void SetMagicProtectionFlag(short sOwnerH, char cOwnerType, bool bStatus);
  void SetProtectionFromArrowFlag(short sOwnerH, char cOwnerType, bool bStatus);
  void SetIllusionMovementFlag(short sOwnerH, char cOwnerType, bool bStatus);
  void SetIllusionFlag(short sOwnerH, char cOwnerType, bool bStatus);

  void RequestChangePlayMode(int iClientH);
  void GetHeroMantleHandler(int iClientH, int iItemID, const char *pString);

  bool bCheckMagicInt(int iClientH);
  bool bChangeState(char cStateChange, char *cStr, char *cVit, char *cDex,
                    char *cInt, char *cMag, char *cChar);
  void StateChangeHandler(int iClientH, char *pData, uint32_t dwMsgSize);

  void SetPoisonFlag(short sOwnerH, char cOwnerType, bool bStatus);

  bool bCopyItemContents(class CItem *pOriginal, class CItem *pCopy);
  int iGetMapLocationSide(char *pMapName);
  void ChatMsgHandlerGSM(int iMsgType, int iV1, char *pName, char *pData,
                         uint32_t dwMsgSize);
  void RemoveClientShortCut(int iClientH);
  bool bAddClientShortCut(int iClientH);

  void GSM_RequestFindCharacter(uint16_t wReqServerID, uint16_t wReqClientH,
                                char *pName,
                                char *pFinder); // New 16/05/2001 Changed
  void ServerStockMsgHandler(char *pData);
  void SendStockMsgToGateServer();
  bool bStockMsgToGateServer(char *pData, uint32_t dwSize);
  void RequestHelpHandler(int iClientH);

  void CheckConnectionHandler(int iClientH, char *pData);

  void AgingMapSectorInfo();
  void UpdateMapSectorInfo();
  int iGetItemWeight(class CItem *pItem, int iCount);
  void CancelQuestHandler(int iClientH);
  void ActivateSpecialAbilityHandler(int iClientH);
  void EnergySphereProcessor(bool bIsAdminCreate = false, int iClientH = 0);
  bool bCheckEnergySphereDestination(int iNpcH, short sAttackerH,
                                     char cAttackerType);
  void JoinPartyHandler(int iClientH, int iV1, const char *pMemberName);
  void RequestSellItemListHandler(int iClientH, char *pData);
  void RequestShopContentsHandler(int iClientH, char *pData);
  void RequestRestartHandler(int iClientH);
  int iRequestPanningMapDataRequest(int iClientH, char *pData);
  void GetMagicAbilityHandler(int iClientH);
  void Effect_Damage_Spot_DamageMove(short sAttackerH, char cAttackerType,
                                     short sTargetH, char cTargetType,
                                     short sAtkX, short sAtkY, short sV1,
                                     short sV2, short sV3, bool bExp,
                                     int iAttr);
  void _TamingHandler(int iClientH, int iSkillNum, char cMapIndex, int dX,
                      int dY);
  void RequestCheckAccountPasswordHandler(char *pData, uint32_t dwMsgSize);
  int _iTalkToNpcResult_Guard(int iClientH, int *pQuestType, int *pMode,
                              int *pRewardType, int *pRewardAmount,
                              int *pContribution, char *pTargetName,
                              int *pTargetType, int *pTargetCount, int *pX,
                              int *pY, int *pRange);
  void SetIceFlag(short sOwnerH, char cOwnerType, bool bStatus);
  void RequestNoticementHandler(int iClientH, char *pData);
  void _AdjustRareItemValue(class CItem *pItem);
  int iGetPlayerABSStatus(int iWhatH, int iRecvH);
  void CheckSpecialEvent(int iClientH);
  char _cGetSpecialAbility(int iKindSA);
  void BuildItemHandler(int iClientH, char *pData);
  bool _bCheckSubLogSocketIndex();
  void _CheckGateSockConnection();
  void OnSubLogRead(int iIndex);
  void OnSubLogSocketEvent(UINT message, WPARAM wParam, LPARAM lParam);
  void GetMapInitialPoint(int iMapIndex, short *pX, short *pY,
                          char *pPlayerLocation = 0);
  int iGetMaxHP(int iClientH);
  int iGetMaxMP(int iClientH);
  int iGetMaxSP(int iClientH);
  void _ClearQuestStatus(int iClientH);
  bool _bCheckItemReceiveCondition(int iClientH, class CItem *pItem);
  int SendItemNotifyMsg(int iClientH, uint16_t wMsgType, class CItem *pItem,
                        int iV1);

  bool _bCheckIsQuestCompleted(int iClientH);
  void _CheckQuestEnvironment(int iClientH);
  void _SendQuestContents(int iClientH);
  void QuestAcceptedHandler(int iClientH);

  void CancelExchangeItem(int iClientH);
  bool bAddItem(int iClientH, class CItem *pItem, char cMode);
  void ConfirmExchangeItem(int iClientH);
  void SetExchangeItem(int iClientH, int iItemIndex, int iAmount);
  void ExchangeItemHandler(int iClientH, short sItemIndex, int iAmount,
                           short dX, short dY, uint16_t wObjectID,
                           const char *pItemName);

  void _BWM_Command_Shutup(char *pData);
  void _BWM_Init(int iClientH, char *pData);
  void CheckUniqueItemEquipment(int iClientH);
  void _SetItemPos(int iClientH, char *pData);

  void GetOccupyFlagHandler(int iClientH);
  int _iComposeFlagStatusContents(char *pData);
  void SetSummonMobAction(int iClientH, int iMode, uint32_t dwMsgSize,
                          char *pData = 0);
  bool __bSetOccupyFlag(char cMapIndex, int dX, int dY, int iSide, int iEKNum,
                        int iClientH, bool bAdminFlag);
  bool _bDepleteDestTypeItemUseEffect(int iClientH, int dX, int dY,
                                      short sItemIndex, short sDestItemID);
  void SetDownSkillIndexHandler(int iClientH, int iSkillIndex);
  int iGetComboAttackBonus(int iSkill, int iComboCount);
  int _iGetWeaponSkillType(int iClientH);
  void CheckFireBluring(char cMapIndex, int sX, int sY);
  void NpcTalkHandler(int iClientH, int iWho);
  bool bDeleteMineral(int iIndex);
  void _CheckMiningAction(int iClientH, int dX, int dY);
  int iCreateMineral(char cMapIndex, int tX, int tY, char cLevel);
  void MineralGenerator();
  void LocalSavePlayerData(int iClientH);
  void ReqCreatePortionHandler(int iClientH, char *pData);
  void ReqCreateCraftingHandler(int iClientH, char *pData);
  void _CheckAttackType(int iClientH, short *spType);
  bool bOnClose();
  void ForceDisconnectAccount(char *pAccountName, uint16_t wCount);
  void ToggleSafeAttackModeHandler(int iClientH);
  void SetBerserkFlag(short sOwnerH, char cOwnerType, bool bStatus);
  void SetHasteFlag(short sOwnerH, char cOwnerType, bool bStatus);
  void SpecialEventHandler();

  int _iForcePlayerDisconect(int iNum);
  int iGetMapIndex(char *pMapName);
  int iGetPlayerRelationship(int iClientH, int iOpponentH);
  int iGetWhetherMagicBonusEffect(short sType, char cWheatherStatus);
  void WhetherProcessor();
  int _iCalcPlayerNum(char cMapIndex, short dX, short dY, char cRadius);
  void FishGenerator();
  void ReqGetFishThisTimeHandler(int iClientH);
  void FishProcessor();
  int iCheckFish(int iClientH, char cMapIndex, short dX, short dY);
  bool bDeleteFish(int iHandle, int iDelMode);
  int iCreateFish(char cMapIndex, short sX, short sY, short sDifficulty,
                  class CItem *pItem, int iDifficulty, uint32_t dwLastTime);
  void UserCommand_DissmissGuild(int iClientH, char *pData, uint32_t dwMsgSize);
  void UserCommand_BanGuildsman(int iClientH, char *pData, uint32_t dwMsgSize);
  int iGetExpLevel(uint32_t iExp);
  void ___RestorePlayerRating(int iClientH);
  void CalcExpStock(int iClientH);
  void ResponseSavePlayerDataReplyHandler(char *pData, uint32_t dwMsgSize);
  void NoticeHandler();
  bool bReadNotifyMsgListFile(char *cFn);
  void SetPlayerReputation(int iClientH, char *pMsg, char cValue,
                           uint32_t dwMsgSize);
  void ShutUpPlayer(int iClientH, char *pMsg, uint32_t dwMsgSize);
  void CheckDayOrNightMode();
  bool bCheckBadWord(char *pString);
  bool bCheckResistingPoisonSuccess(short sOwnerH, char cOwnerType);
  void PoisonEffect(int iClientH, int iV1);
  bool _bGetIsPlayerHostile(int iClientH, int sOwnerH);
  bool bAnalyzeCriminalAction(int iClientH, short dX, short dY,
                              bool bIsCheck = false);
  void RequestAdminUserMode(int iClientH, char *pData);
  int _iGetPlayerNumberOnSpot(short dX, short dY, char cMapIndex, char cRange);
  void CalcTotalItemEffect(int iClientH, int iEquipItemID, bool bNotify = true);
  void ___RestorePlayerCharacteristics(int iClientH);
  void GetPlayerProfile(int iClientH, char *pMsg, uint32_t dwMsgSize);
  void SetPlayerProfile(int iClientH, char *pMsg, uint32_t dwMsgSize);
  void ToggleWhisperPlayer(int iClientH, char *pMsg, uint32_t dwMsgSize);
  void CheckAndNotifyPlayerConnection(int iClientH, char *pMsg,
                                      uint32_t dwSize);
  int iCalcTotalWeight(int iClientH);
  void ReqRepairItemCofirmHandler(int iClientH, char cItemID,
                                  const char *pString);
  void ReqRepairItemHandler(int iClientH, char cItemID, char cRepairWhom,
                            const char *pString);
  void ReqSellItemConfirmHandler(int iClientH, char cItemID, int iNum,
                                 const char *pString);
  void ReqSellItemHandler(int iClientH, char cItemID, char cSellToWhom,
                          int iNum, const char *pItemName);
  void UseSkillHandler(int iClientH, int iV1, int iV2, int iV3);
  int iCalculateUseSkillItemEffect(int iOwnerH, char cOwnerType,
                                   char cOwnerSkill, int iSkillNum,
                                   char cMapIndex, int dX, int dY);
  void ClearSkillUsingStatus(int iClientH);
  void DynamicObjectEffectProcessor();
  void SendObjectMotionRejectMsg(int iClientH);
  void SetInvisibilityFlag(short sOwnerH, char cOwnerType, bool bStatus);
  bool bRemoveFromDelayEventList(int iH, char cType, int iEffectType);
  void DelayEventProcessor();
  bool bRegisterDelayEvent(int iDelayType, int iEffectType, uint32_t dwLastTime,
                           int iTargetH, char cTargetType, char cMapIndex,
                           int dX, int dY, int iV1, int iV2, int iV3);
  int iGetFollowerNumber(short sOwnerH, char cOwnerType);
  int _iCalcSkillSSNpoint(int iLevel);
  void OnKeyUp(WPARAM wParam, LPARAM lParam);
  void OnKeyDown(WPARAM wParam, LPARAM lParam);
  bool bCheckTotalSkillMasteryPoints(int iClientH, int iSkill);
  bool bSetItemToBankItem(int iClientH, class CItem *pItem);
  bool bCheckResistingIceSuccess(char cAttackerDir, short sTargetH,
                                 char cTargetType, int iHitRatio);
  bool bCheckResistingMagicSuccess(char cAttackerDir, short sTargetH,
                                   char cTargetType, int iHitRatio);
  void Effect_SpUp_Spot(short sAttackerH, char cAttackerType, short sTargetH,
                        char cTargetType, short sV1, short sV2, short sV3);
  void Effect_SpDown_Spot(short sAttackerH, char cAttackerType, short sTargetH,
                          char cTargetType, short sV1, short sV2, short sV3);
  void Effect_HpUp_Spot(short sAttackerH, char cAttackerType, short sTargetH,
                        char cTargetType, short sV1, short sV2, short sV3);
  void Effect_Damage_Spot(short sAttackerH, char cAttackerType, short sTargetH,
                          char cTargetType, short sV1, short sV2, short sV3,
                          bool bExp, int iAttr = 0);
  void UseItemHandler(int iClientH, short sItemIndex, short dX, short dY,
                      short sDestItemID);
  void ItemDepleteHandler(int iClientH, short sItemIndex,
                          bool bIsUseItemResult);
  int _iGetArrowItemIndex(int iClientH);
  void RequestFullObjectData(int iClientH, char *pData);
  void CalculateSSN_SkillIndex(int iClientH, short sSkillIndex, int iValue);
  void CalculateSSN_ItemIndex(int iClientH, short sWeaponIndex, int iValue);
  void CheckDynamicObjectList();
  int iAddDynamicObjectList(short sOwner, char cOwnerType, short sType,
                            char cMapIndex, short sX, short sY,
                            uint32_t dwLastTime, int iV1 = 0);
  int _iCalcMaxLoad(int iClientH);
  void GetRewardMoneyHandler(int iClientH);
  void _PenaltyItemDrop(int iClientH, int iTotal, bool bIsSAattacked = false);
  // void ApplyCombatKilledPenalty(int iClientH, char cPenaltyLevel, bool
  // bIsSAattacked = false);
  void EnemyKillRewardHandler(int iAttackerH, int iClientH);
  void PK_KillRewardHandler(short sAttackerH, short sVictumH);
  void ApplyPKpenalty(short sAttackerH, short sVictumH);
  bool bSetItemToBankItem(int iClientH, short sItemIndex);
  void RequestRetrieveItemHandler(int iClientH, char *pData);
  void RequestCivilRightHandler(int iClientH, char *pData);
  bool bCheckLimitedUser(int iClientH);
  void LevelUpSettingsHandler(int iClientH, char *pData, uint32_t dwMsgSize);
  // v1.4311-3 ���� �Լ�  ������ ���� �Լ� ���� FightzoneReserveHandler
  void FightzoneReserveHandler(int iClientH, char *pData, uint32_t dwMsgSize);
  bool bCheckLevelUp(int iClientH);
  uint32_t iGetLevelExp(int iLevel);
  void TimeManaPointsUp(int iClientH);
  void TimeStaminarPointsUp(int iClientH);
  void Quit();
  bool __bReadMapInfo(int iMapIndex);
  void TrainSkillResponse(bool bSuccess, int iClientH, int iSkillNum,
                          int iSkillLevel);
  int _iGetMagicNumber(char *pMagicName, int *pReqInt, int *pCost);
  void RequestStudyMagicHandler(int iClientH, const char *pName,
                                bool bIsPurchase = true);
  void ReleaseFollowMode(short sOwnerH, char cOwnerType);
  void RequestTeleportHandler(int iClientH, char *pData, char *cMapName = 0,
                              int dX = -1, int dY = -1);
  void PlayerMagicHandler(int iClientH, int dX, int dY, short sType,
                          bool bItemEffect = false, int iV1 = 0);
  int iClientMotion_Magic_Handler(int iClientH, short sX, short sY, char cDir);
  void SendMsgToGateServer(uint32_t dwMsg, int iClientH, char *pData = 0);
  void OnGateRead();
  void OnGateSocketEvent(UINT message, WPARAM wParam, LPARAM lParam);
  void ToggleCombatModeHandler(int iClientH);
  void GuildNotifyHandler(char *pData, uint32_t dwMsgSize);
  void SendGuildMsg(int iClientH, uint16_t wNotifyMsgType, short sV1, short sV2,
                    char *pString);
  void DelayEventProcess();
  void TimeHitPointsUp(int iClientH);
  void OnStartGameSignal();
  uint32_t iDice(uint32_t iThrow, uint32_t iRange);
  bool _bInitNpcAttr(class CNpc *pNpc, char *pNpcName, short sClass, char cSA);
  void ReleaseItemHandler(int iClientH, short sItemIndex, bool bNotice);
  void ClientKilledHandler(int iClientH, int iAttackerH, char cAttackerType,
                           short sDamage);
  int SetItemCount(int iClientH, char *pItemName, uint32_t dwCount);
  int SetItemCount(int iClientH, int iItemIndex, uint32_t dwCount);
  int SetItemCountByID(int iClientH, short sItemID, uint32_t dwCount);
  uint32_t dwGetItemCount(int iClientH, char *pName);
  uint32_t dwGetItemCountByID(int iClientH, short sItemID);
  void DismissGuildRejectHandler(int iClientH, const char *pName);
  void DismissGuildApproveHandler(int iClientH, const char *pName);
  void JoinGuildRejectHandler(int iClientH, const char *pName);
  void JoinGuildApproveHandler(int iClientH, const char *pName);
  void SendNotifyMsg(int iFromH, int iToH, uint16_t wMsgType, uint32_t sV1,
                     uint32_t sV2, uint32_t sV3, char *pString,
                     uint32_t sV4 = 0, uint32_t sV5 = 0, uint32_t sV6 = 0,
                     uint32_t sV7 = 0, uint32_t sV8 = 0, uint32_t sV9 = 0,
                     char *pString2 = 0);
  void GiveItemHandler(int iClientH, short sItemIndex, int iAmount, short dX,
                       short dY, uint16_t wObjectID, const char *pItemName);
  void RequestPurchaseItemHandler(int iClientH, const char *pItemName, int iNum,
                                  int iItemId = 0);
  void ResponseDisbandGuildHandler(char *pData, int iType);
  void RequestDisbandGuildHandler(int iClientH, char *pData,
                                  uint32_t dwMsgSize);
  void RequestCreateNewGuildHandler(int iClientH, char *pData,
                                    uint32_t dwMsgSize);
  void ResponseCreateNewGuildHandler(char *pData, int iType);
  int iClientMotion_Stop_Handler(int iClientH, short sX, short sY, char cDir);

  void RequestCreateNewGuild(int iClientH, char *pData);
  void RequestDisbandGuild(int iClientH, char *pData);

  bool bEquipItemHandler(int iClientH, short sItemIndex, bool bNotify = true);
  bool _bAddClientItemList(int iClientH, class CItem *pItem, int *pDelReq);
  int iClientMotion_GetItem_Handler(int iClientH, short sX, short sY,
                                    char cDir);
  void DropItemHandler(int iClientH, short sItemIndex, int iAmount,
                       const char *pItemName, bool bByPlayer = true);
  void ClientCommonHandler(int iClientH, char *pData);
  bool __fastcall bGetMsgQuene(char *pFrom, char *pData, uint32_t *pMsgSize,
                               int *pIndex, char *pKey);
  void MsgProcess();
  bool __fastcall bPutMsgQuene(char cFrom, char *pData, uint32_t dwMsgSize,
                               int iIndex, char cKey);
  int iGetDangerValue(int iNpcH, short dX, short dY);
  // int  iCalculateAttackEffect(short sTargetH, char cTargetType, short
  // sAttackerH, char cAttackerType, int tdX, int tdY, int iAttackMode, bool
  // bNearAttack = false);
  uint32_t iCalculateAttackEffect(short sTargetH, char cTargetType,
                                  short sAttackerH, char cAttackerType, int tdX,
                                  int tdY, int iAttackMode,
                                  bool bNearAttack = false,
                                  bool bIsDash = false, bool bArrowUse = false);
  void RemoveFromTarget(short sTargetH, char cTargetType, int iCode = 0);
  bool bGetEmptyPosition(short *pX, short *pY, char cMapIndex);
  char cGetNextMoveDir(short sX, short sY, short dstX, short dstY,
                       char cMapIndex, char cTurn, int *pError);
  int iClientMotion_Attack_Handler(int iClientH, short sX, short sY, short dX,
                                   short dY, short wType, char cDir,
                                   uint16_t wTargetObjectID,
                                   bool bResponse = true, bool bIsDash = false);
  void ChatMsgHandler(int iClientH, char *pData, uint32_t dwMsgSize);
  void NpcProcess();
  int bCreateNewNpc(char *pNpcName, char *pName, char *pMapName, short sClass,
                    char cSA, char cMoveType, int *poX, int *poY,
                    char *pWaypointList, RECT *pArea, int iSpotMobIndex,
                    char cChangeSide, bool bHideGenMode,
                    bool bIsSummoned = false, bool bFirmBerserk = false,
                    bool bIsMaster = false, int iGuildGUID = 0);
  // bool bCreateNewNpc(char * pNpcName, char * pName, char * pMapName, short
  // sX, short sY);
  int SpawnMapNpcsFromDatabase(struct sqlite3 *db, int iMapIndex);
  bool _bReadMapInfoFiles(int iMapIndex);

  bool _bGetIsStringIsNumber(char *pStr);
  bool _bInitItemAttr(class CItem *pItem, const char *pItemName);
  void GameProcess();
  void InitPlayerData(int iClientH, char *pData, uint32_t dwSize);
  void ResponsePlayerDataHandler(char *pData, uint32_t dwSize);
  bool bSendMsgToLS(uint32_t dwMsg, int iClientH, bool bFlag = true,
                    char *pData = 0);
  void OnMainLogRead();
  void OnMainLogSocketEvent(UINT message, WPARAM wParam, LPARAM lParam);
  void CheckClientResponseTime();
  void OnTimer(char cType);
  int iComposeMoveMapData(short sX, short sY, int iClientH, char cDir,
                          char *pData);
  void SendEventToNearClient_TypeB(uint32_t dwMsgID, uint16_t wMsgType,
                                   char cMapIndex, short sX, short sY,
                                   short sV1, short sV2, short sV3,
                                   short sV4 = 0);
  void SendEventToNearClient_TypeB(uint32_t dwMsgID, uint16_t wMsgType,
                                   char cMapIndex, short sX, short sY,
                                   short sV1, short sV2, short sV3,
                                   uint32_t dwV4 = 0);
  void SendEventToNearClient_TypeA(short sOwnerH, char cOwnerType,
                                   uint32_t dwMsgID, uint16_t wMsgType,
                                   short sV1, short sV2, short sV3);
  void DeleteClient(int iClientH, bool bSave, bool bNotify,
                    bool bCountLogout = true, bool bForceCloseConn = false);
  int iComposeInitMapData(short sX, short sY, int iClientH, char *pData);
  void RequestInitDataHandler(int iClientH, char *pData, char cKey);
  void RequestInitPlayerHandler(int iClientH, char *pData, char cKey);
  int iClientMotion_Move_Handler(int iClientH, short sX, short sY, char cDir,
                                 char cMoveType);
  void ClientMotionHandler(int iClientH, char *pData);
  void OnClientRead(int iClientH);
  bool bInit();
  void OnClientSocketEvent(int iClientH); // MODERNIZED: Polls socket instead of
                                          // handling window messages
  bool bAccept(class XSocket *pXSock);
  void GetFightzoneTicketHandler(int iClientH);
  void FightzoneReserveProcessor();

  // New 06/05/2004
  // Upgrades
  bool bCheckIsItemUpgradeSuccess(int iClientH, int iItemIndex, int iSomH,
                                  bool bBonus = false);
  void RequestItemUpgradeHandler(int iClientH, int iItemIndex);

  // Party Codes
  void RequestCreatePartyHandler(int iClientH);
  void PartyOperationResultHandler(char *pData);
  void PartyOperationResult_Create(int iClientH, char *pName, int iResult,
                                   int iPartyID);
  void PartyOperationResult_Join(int iClientH, char *pName, int iResult,
                                 int iPartyID);
  void PartyOperationResult_Dismiss(int iClientH, char *pName, int iResult,
                                    int iPartyID);
  void PartyOperationResult_Delete(int iPartyID);
  void RequestJoinPartyHandler(int iClientH, char *pData, uint32_t dwMsgSize);
  void RequestDismissPartyHandler(int iClientH);
  void GetPartyInfoHandler(int iClientH);
  void PartyOperationResult_Info(int iClientH, char *pName, int iTotal,
                                 char *pNameList);
  void RequestDeletePartyHandler(int iClientH);
  void RequestAcceptJoinPartyHandler(int iClientH, int iResult);
  void GetExp(int iClientH, uint32_t iExp, bool bIsAttackerOwn = false);

  // New 07/05/2004
  // Guild Codes
  void RequestGuildNameHandler(int iClientH, int iObjectID, int iIndex);

  // Item Logs
  bool _bItemLog(int iAction, int iClientH, char *cName, class CItem *pItem);
  bool _bItemLog(int iAction, int iGiveH, int iRecvH, class CItem *pItem,
                 bool bForceItemLog = false);
  bool _bCheckGoodItem(class CItem *pItem);

  bool bCheckAndConvertPlusWeaponItem(int iClientH, int iItemIndex);
  void ArmorLifeDecrement(int iAttackerH, int iTargetH, char cOwnerType,
                          int iValue);

  // Player shutup
  void GSM_RequestShutupPlayer(char *pGMName, uint16_t wReqServerID,
                               uint16_t wReqClientH, uint16_t wTime,
                               char *pPlayer);

  // PK Logs
  bool _bPKLog(int iAction, int iAttackerH, int iVictumH, char *pNPC);

  // HBest code
  void ForceRecallProcess();
  void SkillCheck(int sTargetH);
  bool IsEnemyZone(int i);

  CGame(HWND hWnd);
  virtual ~CGame();

  // Realm configuration (from realmlist table)
  char m_cRealmName[32];
  char m_cLoginListenIP[16];
  int m_iLoginListenPort;
  char m_cGameListenIP[16];
  int m_iGameListenPort;
  char m_cGameConnectionIP[16]; // Optional - for future login->game server
                                // connection
  int m_iGameConnectionPort;    // Optional - for future login->game server
                                // connection

  uint32_t m_iLevelExp20;

  // private:
  bool LoadPlayerDataFromDb(int iClientH);
  void _ClearItemConfigList();
  bool _bRegisterMap(char *pName);

  // Hot Reload Items
  bool ReloadItemConfigs();
  void UpdateExistingItemFromConfig(class CItem *pItem);
  void UpdateAllExistingItems();
  void BroadcastItemConfigsToAllClients();

  class CClient *m_pClientList[DEF_MAXCLIENTS];
  class CNpc **m_pNpcList; // Pointer to EntityManager's entity array (for
                           // backward compatibility)
  class CMap *m_pMapList[DEF_MAXMAPS];
  class CNpcItem *m_pTempNpcItem[DEF_MAXNPCITEMS];
  class CDynamicObject *m_pDynamicObjectList[DEF_MAXDYNAMICOBJECTS];
  class CDelayEvent *m_pDelayEventList[DEF_MAXDELAYEVENTS];

  class CEntityManager *m_pEntityManager; // Entity spawn/despawn manager

  class CMsg *m_pMsgQuene[DEF_MSGQUENESIZE];
  int m_iQueneHead, m_iQueneTail;
  int m_iTotalMaps;
  // class XSocket * m_pMainLogSock, * m_pGateSock;
  // int				m_iGateSockConnRetryTimes;
  bool m_bIsGameStarted;
  // bool            m_bIsLogSockAvailable, m_bIsGateSockAvailable;
  bool m_bIsItemAvailable, m_bIsBuildItemAvailable, m_bIsNpcAvailable,
      m_bIsMagicAvailable;
  bool m_bIsSkillAvailable, m_bIsPortionAvailable, m_bIsQuestAvailable,
      m_bIsTeleportAvailable;
  bool m_bIsDropTableAvailable;
  std::map<int, DropTable> m_DropTables;

  // Shop system - server sends shop contents to client by item IDs
  bool m_bIsShopDataAvailable;
  std::map<int, int> m_NpcShopMappings; // npc_type → shop_id
  std::map<int, ShopData> m_ShopData;   // shop_id → ShopData
  class CItem *m_pItemConfigList[DEF_MAXITEMTYPES];
  class CNpc *m_pNpcConfigList[DEF_MAXNPCTYPES];
  class CMagic *m_pMagicConfigList[DEF_MAXMAGICTYPE];
  class CSkill *m_pSkillConfigList[DEF_MAXSKILLTYPE];
  class CQuest *m_pQuestConfigList[DEF_MAXQUESTTYPE];
  // class CTeleport * m_pTeleportConfigList[DEF_MAXTELEPORTTYPE];

  class XSocket *_lsock;

  class PartyManager *m_pPartyManager;

  void OnClientLoginRead(int h);
  void OnLoginClientSocketEvent(
      int iLoginClientH); // MODERNIZED: Polls login client socket instead of
                          // handling window messages
  void DeleteLoginClient(int h);

  std::vector<LoginClient *> _lclients_disconn;

  char m_pMsgBuffer[DEF_MSGBUFFERSIZE + 1];

  HWND m_hWnd;
  int m_iTotalClients, m_iMaxClients, m_iTotalGameServerClients,
      m_iTotalGameServerMaxClients;
  int m_iTotalBots, m_iMaxBots, m_iTotalGameServerBots,
      m_iTotalGameServerMaxBots;
  SYSTEMTIME m_MaxUserSysTime;

  bool m_bF1pressed, m_bF4pressed, m_bF12pressed, m_bF5pressed;
  bool m_bOnExitProcess;
  uint32_t m_dwExitProcessTime;

  uint32_t m_dwWhetherTime, m_dwGameTime1, m_dwGameTime2, m_dwGameTime3,
      m_dwGameTime4, m_dwGameTime5, m_dwGameTime6, m_dwFishTime;

  // Crusade Schedule
  bool m_bIsCrusadeWarStarter;
  bool m_bIsApocalypseStarter;
  int m_iLatestCrusadeDayOfWeek;

  char m_cDayOrNight;
  int m_iSkillSSNpoint[102];

  class CMsg *m_pNoticeMsgList[DEF_MAXNOTIFYMSGS];
  int m_iTotalNoticeMsg, m_iPrevSendNoticeMsg;
  uint32_t m_dwNoticeTime, m_dwSpecialEventTime;
  bool m_bIsSpecialEventTime;
  char m_cSpecialEventType;

  uint32_t m_iLevelExpTable[1000]; // New 22/10/04

  class CFish *m_pFish[DEF_MAXFISHS];
  class CPortion *m_pPortionConfigList[DEF_MAXPORTIONTYPES];
  class CPortion *m_pCraftingConfigList[DEF_MAXPORTIONTYPES];

  bool m_bIsServerShutdowned;
  char m_cShutDownCode;
  class CMineral *m_pMineral[DEF_MAXMINERALS];

  int m_iMiddlelandMapIndex;
  int m_iAresdenMapIndex; // �Ʒ����� �� �ε���
  int m_iElvineMapIndex;  // ������ �� �ε���
  int m_iBTFieldMapIndex;
  int m_iGodHMapIndex;
  int m_iAresdenOccupyTiles;
  int m_iElvineOccupyTiles;
  int m_iCurMsgs, m_iMaxMsgs;

  uint32_t m_dwCanFightzoneReserveTime;

  int m_iFightZoneReserve[DEF_MAXFIGHTZONE];
  int m_iFightzoneNoForceRecall;

  struct
  {
    __int64 iFunds;
    __int64 iCrimes;
    __int64 iWins;

  } m_stCityStatus[3];

  int m_iStrategicStatus;

  class XSocket *m_pSubLogSock[DEF_MAXSUBLOGSOCK];
  int m_iSubLogSockInitIndex;
  bool m_bIsSubLogSockAvailable[DEF_MAXSUBLOGSOCK];
  int m_iCurSubLogSockIndex;
  int m_iSubLogSockFailCount;
  int m_iSubLogSockActiveCount;
  int m_iAutoRebootingCount;

  class CBuildItem *m_pBuildItemList[DEF_MAXBUILDITEMS];

  char *m_pNoticementData;
  uint32_t m_dwNoticementDataSize;

  uint32_t m_dwMapSectorInfoTime;
  int m_iMapSectorInfoUpdateCount;

  // Crusade ó����
  int m_iCrusadeCount;
  bool m_bIsCrusadeMode;
  bool m_bIsApocalypseMode;
  // Daryl - Chat logging option
  int m_bLogChatOption;

  struct
  {
    char cMapName[11];
    char cType;
    int dX, dY;

  } m_stCrusadeStructures[DEF_MAXCRUSADESTRUCTURES];

  int m_iCollectedMana[3];
  int m_iAresdenMana, m_iElvineMana;

  class CTeleportLoc m_pGuildTeleportLoc[DEF_MAXGUILDS];
  //

  uint16_t m_wServerID_GSS;
  char m_cGateServerStockMsg[DEF_MAXGATESERVERSTOCKMSGSIZE];
  int m_iIndexGSS;

  int m_iLastCrusadeWinner; // New 13/05/2004
  struct
  {
    int iCrashedStructureNum;
    int iStructureDamageAmount;
    int iCasualties;
  } m_stMeteorStrikeResult;

  struct
  {
    char cType;
    char cSide;
    short sX, sY;
  } m_stMiddleCrusadeStructureInfo[DEF_MAXCRUSADESTRUCTURES];

  struct
  {
    char m_cBannedIPaddress[21];
  } m_stBannedList[DEF_MAXBANNED];

  struct
  {
    char m_cGMName[11];
  } m_stAdminList[DEF_MAXADMINS];

  // Crusade Scheduler
  struct
  {
    int iDay;
    int iHour;
    int iMinute;
  } m_stCrusadeWarSchedule[DEF_MAXSCHEDULE];

  struct
  {
    int iDay;
    int iHour;
    int iMinute;
  } m_stApocalypseScheduleStart[DEF_MAXAPOCALYPSE];

  struct
  {
    int iDay;
    int StartiHour;
    int StartiMinute;
    int EndiHour;
    int EndiMinute;
  } m_stHeldenianSchedule[DEF_MAXHELDENIAN];

  struct
  {
    int iDay;
    int iHour;
    int iMinute;
  } m_stApocalypseScheduleEnd[DEF_MAXAPOCALYPSE];

  int m_iTotalMiddleCrusadeStructures;

  int m_iClientShortCut[DEF_MAXCLIENTS + 1];

  int m_iNpcConstructionPoint[DEF_MAXNPCTYPES];
  uint32_t m_dwCrusadeGUID;
  short m_sLastCrusadeDate;
  int m_iCrusadeWinnerSide;

  struct
  {
    int iTotalMembers;
    int iIndex[9];
  } m_stPartyInfo[DEF_MAXCLIENTS];

  // 09/26/2004
  short m_sSlateSuccessRate;

  // 17/05/2004
  short m_sForceRecallTime;

  // 22/05/2004
  int m_iPrimaryDropRate, m_iSecondaryDropRate;

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
  bool m_bIsApocalyseMode;
  bool m_bIsHeldenianMode;
  bool m_bIsHeldenianTeleport;
  char m_cHeldenianType;

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
  int m_iClientTimeout;          // client-timeout-ms
  int m_iStaminaRegenInterval;   // stamina-regen-interval
  int m_iPoisonDamageInterval;   // poison-damage-interval
  int m_iHealthRegenInterval;    // health-regen-interval
  int m_iManaRegenInterval;      // mana-regen-interval
  int m_iHungerConsumeInterval;  // hunger-consume-interval
  int m_iSummonCreatureDuration; // summon-creature-duration
  int m_iAutosaveInterval;       // autosave-interval
  int m_iLagProtectionInterval;  // lag-protection-interval

  // Character/Leveling Settings
  int m_iBaseStatValue;     // base-stat-value
  int m_iCreationStatBonus; // creation-stat-bonus
  int m_iLevelupStatGain;   // levelup-stat-gain
  int m_iMaxLevel;          // max-level (renamed from max-player-level)
  int m_iMaxStatValue;      // calculated: base + creation + (levelup * max_level) +
                            // 16

  // Combat Settings
  int m_iMinimumHitRatio; // minimum-hit-ratio
  int m_iMaximumHitRatio; // maximum-hit-ratio

  // Gameplay Settings
  int m_iNighttimeDuration;         // nighttime-duration
  int m_iStartingGuildRank;         // starting-guild-rank
  int m_iGrandMagicManaConsumption; // grand-magic-mana-consumption
  int m_iMaxConstructionPoints;     // maximum-construction-points
  int m_iMaxSummonPoints;           // maximum-summon-points
  int m_iMaxWarContribution;        // maximum-war-contribution
  int m_iMaxBankItems;              // max-bank-items

  // ============================================================================

  bool var_89C, var_8A0;
  char m_cHeldenianVictoryType, m_sLastHeldenianWinner, m_cHeldenianModeType;
  int m_iHeldenianAresdenDead, m_iHeldenianElvineDead, var_A38, var_88C;
  int m_iHeldenianAresdenLeftTower, m_iHeldenianElvineLeftTower;
  uint32_t m_dwHeldenianGUID, m_dwHeldenianStartHour, m_dwHeldenianStartMinute,
      m_dwHeldenianStartTime, m_dwHeldenianFinishTime;
  bool m_bReceivedItemList;
  bool m_bHeldenianInitiated;
  bool m_bHeldenianRunning;

private:
  int __iSearchForQuest(int iClientH, int iWho, int *pQuestType, int *pMode,
                        int *pRewardType, int *pRewardAmount,
                        int *pContribution, char *pTargetName, int *pTargetType,
                        int *pTargetCount, int *pX, int *pY, int *pRange);
  int _iTalkToNpcResult_Cityhall(int iClientH, int *pQuestType, int *pMode,
                                 int *pRewardType, int *pRewardAmount,
                                 int *pContribution, char *pTargetName,
                                 int *pTargetType, int *pTargetCount, int *pX,
                                 int *pY, int *pRange);
  void _ClearExchangeStatus(int iToH);
  int _iGetItemSpaceLeft(int iClientH);

public:
  void RemoveCrusadeNpcs(void);
  void RemoveCrusadeRecallTime(void);
  bool _bCrusadeLog(int iAction, int iClientH, int iData, char *cName);
  int iGetPlayerABSStatus(int iClientH);
  bool _bInitItemAttr(class CItem *pItem, int iItemID);
  void ReqCreateSlateHandler(int iClientH, char *pData);
  void SetSlateFlag(int iClientH, short sType, bool bFlag);
  void CheckForceRecallTime(int iClientH);
  void SetPlayingStatus(int iClientH);
  void ForceChangePlayMode(int iClientH, bool bNotify);
  void ShowVersion(int iClientH);
  void ShowClientMsg(int iClientH, char *pMsg);
  void RequestResurrectPlayer(int iClientH, bool bResurrect);
  void LoteryHandler(int iClientH);
  void SetSkillAll(int iClientH, char *pData, uint32_t dwMsgSize);

  /*void GetAngelMantleHandler(int iClientH,int iItemID,char * pString);
  void CheckAngelUnequip(int iClientH, int iAngelID);
  int iAngelEquip(int iClientH);*/

  void SetAngelFlag(short sOwnerH, char cOwnerType, int iStatus, int iTemp);
  void GetAngelHandler(int iClientH, char *pData, uint32_t dwMsgSize);

  void RequestEnchantUpgradeHandler(int client, uint32_t type, uint32_t lvl,
                                    int iType);
  int GetRequiredLevelForUpgrade(uint32_t value);
  void RequestItemEnchantHandler(int iClientH, int sDestItemID, int iType);
  void RequestItemDisenchantHandler(int iClientH, int iItemIndex);
  char *GetShardDesc(uint32_t dwType);
  char *GetFragmentDesc(uint32_t dwType);
  char *GetShardName(uint32_t dwType);
  char *GetFragmentName(uint32_t dwType);

  // 50Cent - Repair All
  void RequestRepairAllItemsHandler(int iClientH);
  void RequestRepairAllItemsDeleteHandler(int iClientH, int index);
  void RequestRepairAllItemsConfirmHandler(int iClientH);
};
