#include "WarManager.h"
#include "Game.h"
#include "StatusEffectManager.h"
#include "Item.h"
#include "CombatManager.h"
#include "EntityManager.h"
#include "DynamicObjectManager.h"
#include "DelayEventManager.h"
#include "ItemManager.h"
#include "MagicManager.h"
#include "SkillManager.h"
#include "GuildManager.h"
#include "QuestManager.h"
#include "LootManager.h"
#include "Packet/SharedPackets.h"
#include "ObjectIDRange.h"
#include "Skill.h"
#include "GameConfigSqliteStore.h"
#include "TeleportLoc.h"

using namespace hb::shared::net;
using namespace hb::shared::action;
using namespace hb::server::net;
using namespace hb::server::config;
using namespace hb::shared::item;
namespace sock = hb::shared::net::socket;
namespace dynamic_object = hb::shared::dynamic_object;
namespace smap = hb::server::map;
namespace sdelay = hb::server::delay_event;
using namespace hb::server::npc;
using namespace hb::server::skill;

extern char G_cTxt[512];
extern char G_cData50000[50000];
extern void PutLogList(char* cStr);
extern void PutLogFileList(char* cStr);
extern void PutHackLogFileList(char* cStr);

void WarManager::CrusadeWarStarter()
{
	SYSTEMTIME SysTime;
	

	if (m_pGame->m_bIsCrusadeMode) return;
	if (m_pGame->m_bIsCrusadeWarStarter == false) return;

	GetLocalTime(&SysTime);

	for(int i = 0; i < MaxSchedule; i++)
		if ((m_pGame->m_stCrusadeWarSchedule[i].iDay == SysTime.wDayOfWeek) &&
			(m_pGame->m_stCrusadeWarSchedule[i].iHour == SysTime.wHour) &&
			(m_pGame->m_stCrusadeWarSchedule[i].iMinute == SysTime.wMinute)) {
			PutLogList("(!) Automated crusade is being initiated!");
			GlobalStartCrusadeMode();
			return;
		}
}

void WarManager::GlobalStartCrusadeMode()
{
	uint32_t dwCrusadeGUID;
	SYSTEMTIME SysTime;

	GetLocalTime(&SysTime);
	if (m_pGame->m_iLatestCrusadeDayOfWeek != -1) {
		if (m_pGame->m_iLatestCrusadeDayOfWeek == SysTime.wDayOfWeek) return;
	}
	else m_pGame->m_iLatestCrusadeDayOfWeek = SysTime.wDayOfWeek;

	dwCrusadeGUID = GameClock::GetTimeMS();

	LocalStartCrusadeMode(dwCrusadeGUID);
}

void WarManager::LocalStartCrusadeMode(uint32_t dwCrusadeGUID)
{
	

	if (m_pGame->m_bIsCrusadeMode) return;
	m_pGame->m_bIsCrusadeMode = true;
	m_pGame->m_iCrusadeWinnerSide = 0;

	if (dwCrusadeGUID != 0) {
		// GUID  .
		_CreateCrusadeGUID(dwCrusadeGUID, 0);
		m_pGame->m_dwCrusadeGUID = dwCrusadeGUID;
	}

	for(int i = 1; i < MaxClients; i++)
		if ((m_pGame->m_pClientList[i] != 0) && (m_pGame->m_pClientList[i]->m_bIsInitComplete)) {
			m_pGame->m_pClientList[i]->m_iCrusadeDuty = 0;
			m_pGame->m_pClientList[i]->m_iConstructionPoint = 0;
			m_pGame->m_pClientList[i]->m_dwCrusadeGUID = m_pGame->m_dwCrusadeGUID;
			m_pGame->SendNotifyMsg(0, i, Notify::Crusade, (uint32_t)m_pGame->m_bIsCrusadeMode, m_pGame->m_pClientList[i]->m_iCrusadeDuty, 0, 0);
		}

	for(int i = 0; i < MaxMaps; i++)
		if (m_pGame->m_pMapList[i] != 0) m_pGame->m_pMapList[i]->RestoreStrikePoints();

	CreateCrusadeStructures();

	PutLogList("(!)Crusade Mode ON.");
}

void WarManager::LocalEndCrusadeMode(int iWinnerSide)
{
	

	//testcode
	std::snprintf(G_cTxt, sizeof(G_cTxt), "LocalEndCrusadeMode(%d)", iWinnerSide);
	PutLogList(G_cTxt);

	if (m_pGame->m_bIsCrusadeMode == false) return;
	m_pGame->m_bIsCrusadeMode = false;

	PutLogList("(!)Crusade Mode OFF.");

	RemoveCrusadeStructures();

	RemoveCrusadeNpcs();

	_CreateCrusadeGUID(m_pGame->m_dwCrusadeGUID, iWinnerSide);
	m_pGame->m_iCrusadeWinnerSide = iWinnerSide;
	m_pGame->m_iLastCrusadeWinner = iWinnerSide;

	for(int i = 1; i < MaxClients; i++)
		if ((m_pGame->m_pClientList[i] != 0) && (m_pGame->m_pClientList[i]->m_bIsInitComplete)) {
			m_pGame->m_pClientList[i]->m_iCrusadeDuty = 0;
			m_pGame->m_pClientList[i]->m_iConstructionPoint = 0;
			m_pGame->SendNotifyMsg(0, i, Notify::Crusade, (uint32_t)m_pGame->m_bIsCrusadeMode, 0, 0, 0, m_pGame->m_iCrusadeWinnerSide);
		}
	RemoveCrusadeRecallTime();

	if (m_pGame->m_iMiddlelandMapIndex != -1) {
		//bSendMsgToLS(0x3D00123C, 0, true, 0);
	}
}

void WarManager::ManualEndCrusadeMode(int iWinnerSide)
{


	if (m_pGame->m_bIsCrusadeMode == false) return;

	LocalEndCrusadeMode(iWinnerSide);
}

void WarManager::CreateCrusadeStructures()
{
	int z, tX, tY, iNamingValue;
	char cName[6], cNpcName[hb::shared::limits::NpcNameLen], cNpcWayPoint[11];

	std::memset(cName, 0, sizeof(cName));
	std::memset(cNpcName, 0, sizeof(cNpcName));
	std::memset(cNpcWayPoint, 0, sizeof(cNpcWayPoint));

	for(int i = 0; i < hb::shared::limits::MaxCrusadeStructures; i++)
		if (m_pGame->m_stCrusadeStructures[i].cType != 0) {
			for (z = 0; z < MaxMaps; z++)
				if ((m_pGame->m_pMapList[z] != 0) && (strcmp(m_pGame->m_pMapList[z]->m_cName, m_pGame->m_stCrusadeStructures[i].cMapName) == 0)) {
					iNamingValue = m_pGame->m_pMapList[z]->iGetEmptyNamingValue();
					if (iNamingValue == -1) {
						// NPC  .     .
					}
					else {
						// NPC .
						std::snprintf(cName, sizeof(cName), "XX%d", iNamingValue);
						cName[0] = '_';
						cName[1] = z + 65;

						switch (m_pGame->m_stCrusadeStructures[i].cType) {
						case 36:
							if (strcmp(m_pGame->m_pMapList[z]->m_cName, "aresden") == 0)
								strcpy(cNpcName, "AGT-Aresden");
							else if (strcmp(m_pGame->m_pMapList[z]->m_cName, "elvine") == 0)
								strcpy(cNpcName, "AGT-Elvine");
							break;

						case 37:
							if (strcmp(m_pGame->m_pMapList[z]->m_cName, "aresden") == 0)
								strcpy(cNpcName, "CGT-Aresden");
							else if (strcmp(m_pGame->m_pMapList[z]->m_cName, "elvine") == 0)
								strcpy(cNpcName, "CGT-Elvine");
							break;

						case 40:
							if (strcmp(m_pGame->m_pMapList[z]->m_cName, "aresden") == 0)
								strcpy(cNpcName, "ESG-Aresden");
							else if (strcmp(m_pGame->m_pMapList[z]->m_cName, "elvine") == 0)
								strcpy(cNpcName, "ESG-Elvine");
							break;

						case 41:
							if (strcmp(m_pGame->m_pMapList[z]->m_cName, "aresden") == 0)
								strcpy(cNpcName, "GMG-Aresden");
							else if (strcmp(m_pGame->m_pMapList[z]->m_cName, "elvine") == 0)
								strcpy(cNpcName, "GMG-Elvine");
							break;

						case 42:
							strcpy(cNpcName, "ManaStone");
							break;

						default:
							strcpy(cNpcName, "Unknown");
							break;
						}

						tX = (int)m_pGame->m_stCrusadeStructures[i].dX;
						tY = (int)m_pGame->m_stCrusadeStructures[i].dY;
						int iNpcConfigId = m_pGame->GetNpcConfigIdByName(cNpcName);
						if (m_pGame->bCreateNewNpc(iNpcConfigId, cName, m_pGame->m_pMapList[z]->m_cName, 0, 0, MoveType::Random,
							&tX, &tY, cNpcWayPoint, 0, 0, -1, false) == false) {
							// NameValue .
							m_pGame->m_pMapList[z]->SetNamingValueEmpty(iNamingValue);
						}
						else {
							std::snprintf(G_cTxt, sizeof(G_cTxt), "(!) Creating Crusade Structure(%s) at %s(%d, %d)", cNpcName, m_pGame->m_stCrusadeStructures[i].cMapName, tX, tY);
							PutLogList(G_cTxt);
						}
					}
				}
		}
}

void WarManager::RemoveCrusadeStructures()
{
	

	for(int i = 0; i < MaxNpcs; i++)
		if (m_pGame->m_pNpcList[i] != 0) {
			switch (m_pGame->m_pNpcList[i]->m_sType) {
			case 36:
			case 37:
			case 38:
			case 39:
			case 40:
			case 41:
			case 42:
				// Use EntityManager for NPC deletion
				if (m_pGame->m_pEntityManager != NULL)
					m_pGame->m_pEntityManager->DeleteEntity(i);
				break;
			}
		}
}

void WarManager::RemoveCrusadeNpcs(void)
{
	for(int i = 0; i < MaxNpcs; i++) {
		if (m_pGame->m_pNpcList[i] != 0) {
			if ((m_pGame->m_pNpcList[i]->m_sType >= 43 && m_pGame->m_pNpcList[i]->m_sType <= 47) || m_pGame->m_pNpcList[i]->m_sType == 51) {
				m_pGame->m_pEntityManager->OnEntityKilled(i, 0, 0, 0);
			}
		}
	}
}

void WarManager::RemoveCrusadeRecallTime(void)
{
	for(int i = 1; i < MaxClients; i++) {
		if (m_pGame->m_pClientList[i] != 0) {
			if (m_pGame->m_pClientList[i]->m_bIsWarLocation &&
				m_pGame->m_pClientList[i]->m_bIsPlayerCivil &&
				m_pGame->m_pClientList[i]->m_bIsInitComplete) {
				m_pGame->m_pClientList[i]->m_iTimeLeft_ForceRecall = 0;
			}
		}
	}
}

void WarManager::SyncMiddlelandMapInfo()
{
	

	if (m_pGame->m_iMiddlelandMapIndex != -1) {
		for(int i = 0; i < hb::shared::limits::MaxCrusadeStructures; i++) {
			m_pGame->m_stMiddleCrusadeStructureInfo[i].cType = 0;
			m_pGame->m_stMiddleCrusadeStructureInfo[i].cSide = 0;
			m_pGame->m_stMiddleCrusadeStructureInfo[i].sX = 0;
			m_pGame->m_stMiddleCrusadeStructureInfo[i].sY = 0;
		}
		m_pGame->m_iTotalMiddleCrusadeStructures = m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_iTotalCrusadeStructures;
		for(int i = 0; i < m_pGame->m_iTotalMiddleCrusadeStructures; i++) {
			m_pGame->m_stMiddleCrusadeStructureInfo[i].cType = m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_stCrusadeStructureInfo[i].cType;
			m_pGame->m_stMiddleCrusadeStructureInfo[i].cSide = m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_stCrusadeStructureInfo[i].cSide;
			m_pGame->m_stMiddleCrusadeStructureInfo[i].sX = m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_stCrusadeStructureInfo[i].sX;
			m_pGame->m_stMiddleCrusadeStructureInfo[i].sY = m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_stCrusadeStructureInfo[i].sY;

			/**cp = m_pGame->m_stMiddleCrusadeStructureInfo[i].cType;
			cp++;
			*cp = m_pGame->m_stMiddleCrusadeStructureInfo[i].cSide;
			cp++;
			sp = (short *)cp;
			*sp = (short)m_pGame->m_stMiddleCrusadeStructureInfo[i].sX;
			cp += 2;
			sp = (short *)cp;
			*sp = (short)m_pGame->m_stMiddleCrusadeStructureInfo[i].sY;
			cp += 2;*/
		}

		if (m_pGame->m_iTotalMiddleCrusadeStructures != 0) {
			//testcode
			//std::snprintf(G_cTxt, sizeof(G_cTxt), "m_pGame->m_iTotalMiddleCrusadeStructures: %d", m_pGame->m_iTotalMiddleCrusadeStructures);
			//PutLogList(G_cTxt);
		}
	}
}

void WarManager::SelectCrusadeDutyHandler(int iClientH, int iDuty)
{

	if (m_pGame->m_pClientList[iClientH] == 0) return;
	if ((m_pGame->m_pClientList[iClientH]->m_iGuildRank != 0) && (iDuty == 3)) return;

	if (m_pGame->m_iLastCrusadeWinner == m_pGame->m_pClientList[iClientH]->m_cSide &&
		m_pGame->m_pClientList[iClientH]->m_dwCrusadeGUID == 0 && iDuty == 3) {
		m_pGame->m_pClientList[iClientH]->m_iConstructionPoint = 3000;
	}
	m_pGame->m_pClientList[iClientH]->m_iCrusadeDuty = iDuty;

	m_pGame->SendNotifyMsg(0, iClientH, Notify::Crusade, (uint32_t)m_pGame->m_bIsCrusadeMode, m_pGame->m_pClientList[iClientH]->m_iCrusadeDuty, 0, 0);
}

void WarManager::CheckCrusadeResultCalculation(int iClientH)
{
	if (m_pGame->m_pClientList[iClientH] == 0) return;
	if (m_pGame->m_pClientList[iClientH]->m_cVar == 1) return;

	if ((m_pGame->m_bIsCrusadeMode == false) && (m_pGame->m_pClientList[iClientH]->m_dwCrusadeGUID != 0)) {
		if (m_pGame->m_pClientList[iClientH]->m_iWarContribution > m_pGame->m_iMaxWarContribution) m_pGame->m_pClientList[iClientH]->m_iWarContribution = m_pGame->m_iMaxWarContribution;
		if (m_pGame->m_pClientList[iClientH]->m_dwCrusadeGUID == m_pGame->m_dwCrusadeGUID) {
			if (m_pGame->m_iCrusadeWinnerSide == 0) {
				m_pGame->m_pClientList[iClientH]->m_iExpStock += (m_pGame->m_pClientList[iClientH]->m_iWarContribution / 6);
				m_pGame->SendNotifyMsg(0, iClientH, Notify::Crusade, (uint32_t)m_pGame->m_bIsCrusadeMode, 0, m_pGame->m_pClientList[iClientH]->m_iWarContribution, 0);
			}
			else {
				if (m_pGame->m_iCrusadeWinnerSide == m_pGame->m_pClientList[iClientH]->m_cSide) {
					if (m_pGame->m_pClientList[iClientH]->m_iLevel <= 80) {
						m_pGame->m_pClientList[iClientH]->m_iWarContribution += m_pGame->m_pClientList[iClientH]->m_iLevel * 100;
					}
					else if (m_pGame->m_pClientList[iClientH]->m_iLevel <= 100) {
						m_pGame->m_pClientList[iClientH]->m_iWarContribution += m_pGame->m_pClientList[iClientH]->m_iLevel * 40;
					}
					else m_pGame->m_pClientList[iClientH]->m_iWarContribution += m_pGame->m_pClientList[iClientH]->m_iLevel;
					m_pGame->m_pClientList[iClientH]->m_iExpStock += m_pGame->m_pClientList[iClientH]->m_iWarContribution;
					m_pGame->SendNotifyMsg(0, iClientH, Notify::Crusade, (uint32_t)m_pGame->m_bIsCrusadeMode, 0, m_pGame->m_pClientList[iClientH]->m_iWarContribution, 0);
				}
				else if (m_pGame->m_iCrusadeWinnerSide != m_pGame->m_pClientList[iClientH]->m_cSide) {
					if (m_pGame->m_pClientList[iClientH]->m_iLevel <= 80) {
						m_pGame->m_pClientList[iClientH]->m_iWarContribution += m_pGame->m_pClientList[iClientH]->m_iLevel * 100;
					}
					else if (m_pGame->m_pClientList[iClientH]->m_iLevel <= 100) {
						m_pGame->m_pClientList[iClientH]->m_iWarContribution += m_pGame->m_pClientList[iClientH]->m_iLevel * 40;
					}
					else m_pGame->m_pClientList[iClientH]->m_iWarContribution += m_pGame->m_pClientList[iClientH]->m_iLevel;
					m_pGame->m_pClientList[iClientH]->m_iExpStock += m_pGame->m_pClientList[iClientH]->m_iWarContribution / 10;
					m_pGame->SendNotifyMsg(0, iClientH, Notify::Crusade, (uint32_t)m_pGame->m_bIsCrusadeMode, 0, -1 * m_pGame->m_pClientList[iClientH]->m_iWarContribution, 0);
				}
			}
		}
		else {
			m_pGame->SendNotifyMsg(0, iClientH, Notify::Crusade, (uint32_t)m_pGame->m_bIsCrusadeMode, 0, 0, 0, -1);
		}
		m_pGame->m_pClientList[iClientH]->m_iCrusadeDuty = 0;
		m_pGame->m_pClientList[iClientH]->m_iWarContribution = 0;
		m_pGame->m_pClientList[iClientH]->m_dwCrusadeGUID = 0;
		m_pGame->m_pClientList[iClientH]->m_dwSpeedHackCheckTime = GameClock::GetTimeMS();
		m_pGame->m_pClientList[iClientH]->m_iSpeedHackCheckExp = m_pGame->m_pClientList[iClientH]->m_iExp;
	}
}

bool WarManager::bReadCrusadeGUIDFile(char* cFn)
{
	FILE* pFile;
	HANDLE hFile;
	uint32_t  dwFileSize;
	char* cp, * token, cReadMode;
	char seps[] = "= \t\r\n";

	cReadMode = 0;

	hFile = CreateFile(cFn, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
	dwFileSize = GetFileSize(hFile, 0);
	if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);

	pFile = fopen(cFn, "rt");
	if (pFile == 0) {
		return false;
	}
	else {
		cp = new char[dwFileSize + 2];
		std::memset(cp, 0, dwFileSize + 2);
		fread(cp, dwFileSize, 1, pFile);

		token = strtok(cp, seps);

		while (token != 0) {

			if (cReadMode != 0) {
				switch (cReadMode) {
				case 1:
					m_pGame->m_dwCrusadeGUID = atoi(token);
					std::snprintf(G_cTxt, sizeof(G_cTxt), "CrusadeGUID = %d", m_pGame->m_dwCrusadeGUID);
					PutLogList(G_cTxt);
					cReadMode = 0;
					break;

				case 2:
					// New 13/05/2004 Changed
					m_pGame->m_iLastCrusadeWinner = atoi(token);
					std::snprintf(G_cTxt, sizeof(G_cTxt), "CrusadeWinnerSide = %d", m_pGame->m_iLastCrusadeWinner);
					PutLogList(G_cTxt);
					cReadMode = 0;
					break;
				}
			}
			else {
				if (memcmp(token, "CrusadeGUID", 11) == 0) cReadMode = 1;
				if (memcmp(token, "winner-side", 11) == 0) cReadMode = 2;
			}

			token = strtok(NULL, seps);
		}

		delete cp;
	}
	if (pFile != 0) fclose(pFile);

	return true;
}

void WarManager::_CreateCrusadeGUID(uint32_t dwCrusadeGUID, int iWinnerSide)
{
	char* cp, cTxt[256], cFn[256], cTemp[1024];
	FILE* pFile;

	_mkdir("GameData");
	std::memset(cFn, 0, sizeof(cFn));

	strcat(cFn, "GameData/");
	strcat(cFn, "CrusadeGUID.txt");

	pFile = fopen(cFn, "wt");
	if (pFile == 0) {
		std::snprintf(cTxt, sizeof(cTxt), "(!) Cannot create CrusadeGUID(%d) file", dwCrusadeGUID);
		PutLogList(cTxt);
	}
	else {
		std::memset(cTemp, 0, sizeof(cTemp));

		std::memset(cTxt, 0, sizeof(cTxt));
		std::snprintf(cTxt, sizeof(cTxt), "CrusadeGUID = %d\n", dwCrusadeGUID);
		strcat(cTemp, cTxt);

		std::memset(cTxt, 0, sizeof(cTxt));
		std::snprintf(cTxt, sizeof(cTxt), "winner-side = %d\n", iWinnerSide);
		strcat(cTemp, cTxt);

		cp = (char*)cTemp;
		fwrite(cp, strlen(cp), 1, pFile);

		std::snprintf(cTxt, sizeof(cTxt), "(O) CrusadeGUID(%d) file created", dwCrusadeGUID);
		PutLogList(cTxt);
	}
	if (pFile != 0) fclose(pFile);
}

void WarManager::CheckCommanderConstructionPoint(int iClientH)
{
	

	if (m_pGame->m_pClientList[iClientH] == 0) return;
	if (m_pGame->m_bIsCrusadeMode == false) return;
	if (m_pGame->m_pClientList[iClientH]->m_iConstructionPoint <= 0) return;

	switch (m_pGame->m_pClientList[iClientH]->m_iCrusadeDuty) {
	case 1:
	case 2:
		for(int i = 0; i < MaxClients; i++)
			if ((m_pGame->m_pClientList[i] != 0) && (m_pGame->m_pClientList[i]->m_iCrusadeDuty == 3) &&
				(m_pGame->m_pClientList[i]->m_iGuildGUID == m_pGame->m_pClientList[iClientH]->m_iGuildGUID)) {
				m_pGame->m_pClientList[i]->m_iConstructionPoint += m_pGame->m_pClientList[iClientH]->m_iConstructionPoint;
				m_pGame->m_pClientList[i]->m_iWarContribution += (m_pGame->m_pClientList[iClientH]->m_iConstructionPoint / 10);

				if (m_pGame->m_pClientList[i]->m_iConstructionPoint > m_pGame->m_iMaxConstructionPoints)
					m_pGame->m_pClientList[i]->m_iConstructionPoint = m_pGame->m_iMaxConstructionPoints;

				if (m_pGame->m_pClientList[i]->m_iWarContribution > m_pGame->m_iMaxWarContribution)
					m_pGame->m_pClientList[i]->m_iWarContribution = m_pGame->m_iMaxWarContribution;

				m_pGame->SendNotifyMsg(0, i, Notify::ConstructionPoint, m_pGame->m_pClientList[i]->m_iConstructionPoint, m_pGame->m_pClientList[i]->m_iWarContribution, 0, 0);
				m_pGame->m_pClientList[iClientH]->m_iConstructionPoint = 0;
				return;
			}

		m_pGame->m_pClientList[iClientH]->m_iConstructionPoint = 0;
		break;

	case 3:

		break;
	}
}

bool WarManager::__bSetConstructionKit(int iMapIndex, int dX, int dY, int iType, int iTimeCost, int iClientH)
{
	int iNamingValue, tX, tY;
	char cNpcName[hb::shared::limits::NpcNameLen], cName[hb::shared::limits::NpcNameLen], cNpcWaypoint[11], cOwnerType;
	short sOwnerH;

	if ((m_pGame->m_bIsCrusadeMode == false) || (m_pGame->m_pClientList[iClientH]->m_iCrusadeDuty != 2)) return false;
	if (m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->m_iTotalCrusadeStructures >= hb::shared::limits::MaxCrusadeStructures) {
		m_pGame->SendNotifyMsg(0, iClientH, Notify::NoMoreCrusadeStructure, 0, 0, 0, 0);
		return false;
	}

	// NPC .
	iNamingValue = m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->iGetEmptyNamingValue();
	if (iNamingValue == -1) {
		// NPC  .     .
	}
	else {

		for(int ix = dX - 3; ix <= dX + 5; ix++)
			for(int iy = dY - 3; iy <= dX + 5; iy++) {
				m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->GetOwner(&sOwnerH, &cOwnerType, ix, iy);
				if ((sOwnerH != 0) && (cOwnerType == hb::shared::owner_class::Npc) && (m_pGame->m_pNpcList[sOwnerH]->m_cActionLimit == 5)) return false;
			}

		// NPC .
		std::memset(cNpcName, 0, sizeof(cNpcName));
		if (m_pGame->m_pClientList[iClientH]->m_cSide == 1) {
			switch (iType) {
			case 1: strcpy(cNpcName, "AGT-Aresden"); break;
			case 2: strcpy(cNpcName, "CGT-Aresen"); break;
			case 3: strcpy(cNpcName, "MS-Aresden"); break;
			case 4: strcpy(cNpcName, "DT-Aresden"); break;
			}
		}
		else if (m_pGame->m_pClientList[iClientH]->m_cSide == 2) {
			switch (iType) {
			case 1: strcpy(cNpcName, "AGT-Elvine"); break;
			case 2: strcpy(cNpcName, "CGT-Elvine"); break;
			case 3: strcpy(cNpcName, "MS-Elvine"); break;
			case 4: strcpy(cNpcName, "DT-Elvine"); break;
			}
		}
		else return false;

		std::memset(cName, 0, sizeof(cName));
		std::snprintf(cName, sizeof(cName), "XX%d", iNamingValue);
		cName[0] = '_';
		cName[1] = m_pGame->m_pClientList[iClientH]->m_cMapIndex + 65;

		std::memset(cNpcWaypoint, 0, sizeof(cNpcWaypoint));

		tX = (int)dX;
		tY = (int)dY;
		int iNpcConfigId = m_pGame->GetNpcConfigIdByName(cNpcName);
		if (m_pGame->bCreateNewNpc(iNpcConfigId, cName, m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->m_cName, 0, (rand() % 9),
			MoveType::Random, &tX, &tY, cNpcWaypoint, 0, 0, -1, false, false) == false) {
			// NameValue .
			m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->SetNamingValueEmpty(iNamingValue);
		}
		else {
			std::snprintf(G_cTxt, sizeof(G_cTxt), "Structure(%s) construction begin(%d,%d)!", cNpcName, tX, tY);
			PutLogList(G_cTxt);
			return true;
		}
	}

	return false;
}

void WarManager::MeteorStrikeHandler(int iMapIndex)
{
	int dX, dY, iIndex, iTargetIndex, iTotalESG, iEffect;
	int iTargetArray[smap::MaxStrikePoints];
	short sOwnerH;
	char  cOwnerType;
	uint32_t dwTime = GameClock::GetTimeMS();

	PutLogList("(!) Beginning Meteor Strike Procedure...");

	if (iMapIndex == -1) {
		PutLogList("(X) MeteorStrikeHandler Error! MapIndex -1!");
		return;
	}

	if (m_pGame->m_pMapList[iMapIndex] == 0) {
		PutLogList("(X) MeteorStrikeHandler Error! 0 Map!");
		return;
	}

	if (m_pGame->m_pMapList[iMapIndex]->m_iTotalStrikePoints == 0) {
		PutLogList("(X) MeteorStrikeHandler Error! No Strike Points!");
		return;
	}

	for(int i = 0; i < smap::MaxStrikePoints; i++) iTargetArray[i] = -1;

	iIndex = 0;
	for(int i = 1; i <= m_pGame->m_pMapList[iMapIndex]->m_iTotalStrikePoints; i++) {
		if (m_pGame->m_pMapList[iMapIndex]->m_stStrikePoint[i].iHP > 0) {
			iTargetArray[iIndex] = i;
			iIndex++;
		}
	}

	//testcode
	std::snprintf(G_cTxt, sizeof(G_cTxt), "(!) Map(%s) has %d available strike points", m_pGame->m_pMapList[iMapIndex]->m_cName, iIndex);
	PutLogList(G_cTxt);

	m_pGame->m_stMeteorStrikeResult.iCasualties = 0;
	m_pGame->m_stMeteorStrikeResult.iCrashedStructureNum = 0;
	m_pGame->m_stMeteorStrikeResult.iStructureDamageAmount = 0;

	if (iIndex == 0) {
		PutLogList("(!) No strike points!");
		m_pGame->m_pDelayEventManager->bRegisterDelayEvent(sdelay::Type::CalcMeteorStrikeEffect, 0, dwTime + 6000, 0, 0, iMapIndex, 0, 0, 0, 0, 0);
	}
	else {

		for(int i = 1; i < MaxClients; i++)
			if ((m_pGame->m_pClientList[i] != 0) && (m_pGame->m_pClientList[i]->m_bIsInitComplete) && (m_pGame->m_pClientList[i]->m_cMapIndex == iMapIndex)) {
				m_pGame->SendNotifyMsg(0, i, Notify::MeteorStrikeHit, 0, 0, 0, 0);
			}

		for(int i = 0; i < iIndex; i++) {
			iTargetIndex = iTargetArray[i];

			if (iTargetIndex == -1) {
				PutLogList("(X) Strike Point MapIndex: -1!");
				goto MSH_SKIP_STRIKE;
			}

			dX = m_pGame->m_pMapList[iMapIndex]->m_stStrikePoint[iTargetIndex].dX;
			dY = m_pGame->m_pMapList[iMapIndex]->m_stStrikePoint[iTargetIndex].dY;

			// dX, dY    2  Energy Shield Generator    .  1   HP .
			// NPC       .
			iTotalESG = 0;
			for(int ix = dX - 10; ix <= dX + 10; ix++)
				for(int iy = dY - 10; iy <= dY + 10; iy++) {
					m_pGame->m_pMapList[iMapIndex]->GetOwner(&sOwnerH, &cOwnerType, ix, iy);
					if ((cOwnerType == hb::shared::owner_class::Npc) && (m_pGame->m_pNpcList[sOwnerH] != 0) && (m_pGame->m_pNpcList[sOwnerH]->m_sType == 40)) {
						iTotalESG++;
					}
				}

			// testcode
			std::snprintf(G_cTxt, sizeof(G_cTxt), "(!) Meteor Strike Target(%d, %d) ESG(%d)", dX, dY, iTotalESG);
			PutLogList(G_cTxt);

			if (iTotalESG < 2) {

				m_pGame->m_pMapList[iMapIndex]->m_stStrikePoint[iTargetIndex].iHP -= (2 - iTotalESG);
				if (m_pGame->m_pMapList[iMapIndex]->m_stStrikePoint[iTargetIndex].iHP <= 0) {
					m_pGame->m_pMapList[iMapIndex]->m_stStrikePoint[iTargetIndex].iHP = 0;
					m_pGame->m_pMapList[m_pGame->m_pMapList[iMapIndex]->m_stStrikePoint[iTargetIndex].iMapIndex]->m_bIsDisabled = true;
					m_pGame->m_stMeteorStrikeResult.iCrashedStructureNum++;
				}
				else {
					m_pGame->m_stMeteorStrikeResult.iStructureDamageAmount += (2 - iTotalESG);
					iEffect = m_pGame->iDice(1, 5) - 1;
					m_pGame->m_pDynamicObjectManager->iAddDynamicObjectList(0, hb::shared::owner_class::PlayerIndirect, dynamic_object::Fire2, iMapIndex,
						static_cast<short>(m_pGame->m_pMapList[iMapIndex]->m_stStrikePoint[iTargetIndex].iEffectX[iEffect] + (m_pGame->iDice(1, 3) - 2)),
						static_cast<short>(m_pGame->m_pMapList[iMapIndex]->m_stStrikePoint[iTargetIndex].iEffectY[iEffect] + (m_pGame->iDice(1, 3) - 2)),
						60 * 1000 * 50);
				}
			}
		MSH_SKIP_STRIKE:;
		}

		m_pGame->m_pDelayEventManager->bRegisterDelayEvent(sdelay::Type::DoMeteorStrikeDamage, 0, dwTime + 1000, 0, 0, iMapIndex, 0, 0, 0, 0, 0);
		m_pGame->m_pDelayEventManager->bRegisterDelayEvent(sdelay::Type::DoMeteorStrikeDamage, 0, dwTime + 4000, 0, 0, iMapIndex, 0, 0, 0, 0, 0);
		m_pGame->m_pDelayEventManager->bRegisterDelayEvent(sdelay::Type::CalcMeteorStrikeEffect, 0, dwTime + 6000, 0, 0, iMapIndex, 0, 0, 0, 0, 0);
	}
}

void WarManager::MeteorStrikeMsgHandler(char cAttackerSide)
{
	
	uint32_t dwTime = GameClock::GetTimeMS();

	switch (cAttackerSide) {
	case 1:
		if (m_pGame->m_iElvineMapIndex != -1) {
			for(int i = 1; i < MaxClients; i++)
				if ((m_pGame->m_pClientList[i] != 0) && (m_pGame->m_pClientList[i]->m_bIsInitComplete)) {
					if (strcmp(m_pGame->m_pMapList[m_pGame->m_pClientList[i]->m_cMapIndex]->m_cLocationName, "elvine") == 0) {
						m_pGame->SendNotifyMsg(0, i, Notify::MeteorStrikeComing, 1, 0, 0, 0);
					}
					else {
						m_pGame->SendNotifyMsg(0, i, Notify::MeteorStrikeComing, 2, 0, 0, 0);
					}
				}
			m_pGame->m_pDelayEventManager->bRegisterDelayEvent(sdelay::Type::MeteorStrike, 0, dwTime + 5000, 0, 0, m_pGame->m_iElvineMapIndex, 0, 0, 0, 0, 0);
		}
		else {
			for(int i = 1; i < MaxClients; i++)
				if (m_pGame->m_pClientList[i] != 0) {
					m_pGame->SendNotifyMsg(0, i, Notify::MeteorStrikeComing, 2, 0, 0, 0);
				}
		}
		break;

	case 2:
		if (m_pGame->m_iAresdenMapIndex != -1) {
			for(int i = 1; i < MaxClients; i++)
				if ((m_pGame->m_pClientList[i] != 0) && (m_pGame->m_pClientList[i]->m_bIsInitComplete)) {
					if (strcmp(m_pGame->m_pMapList[m_pGame->m_pClientList[i]->m_cMapIndex]->m_cLocationName, "aresden") == 0) {
						m_pGame->SendNotifyMsg(0, i, Notify::MeteorStrikeComing, 3, 0, 0, 0);
					}
					else {
						m_pGame->SendNotifyMsg(0, i, Notify::MeteorStrikeComing, 4, 0, 0, 0);
					}
				}
			m_pGame->m_pDelayEventManager->bRegisterDelayEvent(sdelay::Type::MeteorStrike, 0, dwTime + 1000 * 5, 0, 0, m_pGame->m_iAresdenMapIndex, 0, 0, 0, 0, 0);
		}
		else {
			for(int i = 1; i < MaxClients; i++)
				if (m_pGame->m_pClientList[i] != 0) {
					m_pGame->SendNotifyMsg(0, i, Notify::MeteorStrikeComing, 4, 0, 0, 0);
				}
		}
		break;
	}
}

void WarManager::CalcMeteorStrikeEffectHandler(int iMapIndex)
{
	int iActiveStructure, iStructureHP[smap::MaxStrikePoints];
	char cWinnerSide, cTempData[120];

	if (m_pGame->m_bIsCrusadeMode == false) return;

	for(int i = 0; i < smap::MaxStrikePoints; i++)
		iStructureHP[i] = 0;

	iActiveStructure = 0;
	for(int i = 1; i <= m_pGame->m_pMapList[iMapIndex]->m_iTotalStrikePoints; i++) {
		if (m_pGame->m_pMapList[iMapIndex]->m_stStrikePoint[i].iHP > 0) {
			iActiveStructure++;
			iStructureHP[i] = m_pGame->m_pMapList[iMapIndex]->m_stStrikePoint[i].iHP;
		}
	}

	//testcode
	std::snprintf(G_cTxt, sizeof(G_cTxt), "ActiveStructure:%d  MapIndex:%d AresdenMap:%d ElvineMap:%d", iActiveStructure, iMapIndex, m_pGame->m_iAresdenMapIndex, m_pGame->m_iElvineMapIndex);
	PutLogList(G_cTxt);

	if (iActiveStructure == 0) {
		if (iMapIndex == m_pGame->m_iAresdenMapIndex) {
			cWinnerSide = 2;
			LocalEndCrusadeMode(2);
		}
		else if (iMapIndex == m_pGame->m_iElvineMapIndex) {
			cWinnerSide = 1;
			LocalEndCrusadeMode(1);
		}
		else {
			cWinnerSide = 0;
			LocalEndCrusadeMode(0);
		}

	}
	else {
		std::memset(cTempData, 0, sizeof(cTempData));
		auto& meteorHdr = *reinterpret_cast<hb::net::MeteorStrikeHeader*>(cTempData);
		meteorHdr.total_points = static_cast<uint16_t>(m_pGame->m_pMapList[iMapIndex]->m_iTotalStrikePoints);

		auto* hpEntries = reinterpret_cast<uint16_t*>(cTempData + sizeof(hb::net::MeteorStrikeHeader));
		for (int i = 1; i <= m_pGame->m_pMapList[iMapIndex]->m_iTotalStrikePoints; i++) {
			hpEntries[i - 1] = static_cast<uint16_t>(iStructureHP[i]);
		}

		GrandMagicResultHandler(m_pGame->m_pMapList[iMapIndex]->m_cName, m_pGame->m_stMeteorStrikeResult.iCrashedStructureNum, m_pGame->m_stMeteorStrikeResult.iStructureDamageAmount, m_pGame->m_stMeteorStrikeResult.iCasualties, iActiveStructure, m_pGame->m_pMapList[iMapIndex]->m_iTotalStrikePoints, cTempData);
	}

	m_pGame->m_stMeteorStrikeResult.iCasualties = 0;
	m_pGame->m_stMeteorStrikeResult.iCrashedStructureNum = 0;
	m_pGame->m_stMeteorStrikeResult.iStructureDamageAmount = 0;
}

void WarManager::DoMeteorStrikeDamageHandler(int iMapIndex)
{
	int iDamage;

	for(int i = 1; i < MaxClients; i++)
		if ((m_pGame->m_pClientList[i] != 0) && (m_pGame->m_pClientList[i]->m_cSide != 0) && (m_pGame->m_pClientList[i]->m_cMapIndex == iMapIndex)) {
			if (m_pGame->m_pClientList[i]->m_iLevel < 80)
				iDamage = m_pGame->m_pClientList[i]->m_iLevel + m_pGame->iDice(1, 10);
			else iDamage = m_pGame->m_pClientList[i]->m_iLevel * 2 + m_pGame->iDice(1, 10);
			iDamage = m_pGame->iDice(1, m_pGame->m_pClientList[i]->m_iLevel) + m_pGame->m_pClientList[i]->m_iLevel;
			// 255   .
			if (iDamage > 255) iDamage = 255;

			if (m_pGame->m_pClientList[i]->m_cMagicEffectStatus[hb::shared::magic::Protect] == 2) { //magic cut in half
				iDamage = (iDamage / 2) - 2;
			}

			if (m_pGame->m_pClientList[i]->m_cMagicEffectStatus[hb::shared::magic::Protect] == 5) {
				iDamage = 0;
			}

			m_pGame->m_pClientList[i]->m_iHP -= iDamage;
			if (m_pGame->m_pClientList[i]->m_iHP <= 0) {
				m_pGame->m_pCombatManager->ClientKilledHandler(i, 0, 0, iDamage);
				m_pGame->m_stMeteorStrikeResult.iCasualties++;
			}
			else {
				if (iDamage > 0) {
					m_pGame->SendNotifyMsg(0, i, Notify::Hp, 0, 0, 0, 0);
					m_pGame->SendEventToNearClient_TypeA(i, hb::shared::owner_class::Player, MsgId::EventMotion, Type::Damage, iDamage, 0, 0);

					if (m_pGame->m_pClientList[i]->m_bSkillUsingStatus[19] != true) {
						m_pGame->m_pMapList[m_pGame->m_pClientList[i]->m_cMapIndex]->ClearOwner(0, i, hb::shared::owner_class::Player, m_pGame->m_pClientList[i]->m_sX, m_pGame->m_pClientList[i]->m_sY);
						m_pGame->m_pMapList[m_pGame->m_pClientList[i]->m_cMapIndex]->SetOwner(i, hb::shared::owner_class::Player, m_pGame->m_pClientList[i]->m_sX, m_pGame->m_pClientList[i]->m_sY);
					}

					if (m_pGame->m_pClientList[i]->m_cMagicEffectStatus[hb::shared::magic::HoldObject] != 0) {
						// Hold-Person    .     .
						// 1: Hold-Person 
						// 2: Paralize
						m_pGame->SendNotifyMsg(0, i, Notify::MagicEffectOff, hb::shared::magic::HoldObject, m_pGame->m_pClientList[i]->m_cMagicEffectStatus[hb::shared::magic::HoldObject], 0, 0);

						m_pGame->m_pClientList[i]->m_cMagicEffectStatus[hb::shared::magic::HoldObject] = 0;
						m_pGame->m_pDelayEventManager->bRemoveFromDelayEventList(i, hb::shared::owner_class::Player, hb::shared::magic::HoldObject);
					}
				}
			}
		}
}

void WarManager::_LinkStrikePointMapIndex()
{
	int z, x;

	for(int i = 0; i < MaxMaps; i++)
		if ((m_pGame->m_pMapList[i] != 0) && (m_pGame->m_pMapList[i]->m_iTotalStrikePoints != 0)) {
			for (z = 0; z < smap::MaxStrikePoints; z++)
				if (strlen(m_pGame->m_pMapList[i]->m_stStrikePoint[z].cRelatedMapName) != 0) {
					for (x = 0; x < MaxMaps; x++)
						if ((m_pGame->m_pMapList[x] != 0) && (strcmp(m_pGame->m_pMapList[x]->m_cName, m_pGame->m_pMapList[i]->m_stStrikePoint[z].cRelatedMapName) == 0)) {
							m_pGame->m_pMapList[i]->m_stStrikePoint[z].iMapIndex = x;
							//testcode
							PutLogList(G_cTxt);

							goto LSPMI_LOOPBREAK;
						}
				LSPMI_LOOPBREAK:;
				}
		}
}

void WarManager::_GrandMagicLaunchMsgSend(int iType, char cAttackerSide)
{}

void WarManager::GrandMagicResultHandler(char* cMapName, int iCrashedStructureNum, int iStructureDamageAmount, int iCasualities, int iActiveStructure, int iTotalStrikePoints, char* cData)
{
	

	for(int i = 1; i < MaxClients; i++)
		if (m_pGame->m_pClientList[i] != 0) {
			m_pGame->SendNotifyMsg(0, i, Notify::GrandMagicResult, iCrashedStructureNum, iStructureDamageAmount, iCasualities, cMapName, iActiveStructure, 0, 0, 0, 0, iTotalStrikePoints, cData);
		}
}

void WarManager::CollectedManaHandler(uint16_t wAresdenMana, uint16_t wElvineMana)
{
	if (m_pGame->m_iAresdenMapIndex != -1) {
		m_pGame->m_iAresdenMana += wAresdenMana;
		//testcode
		if (wAresdenMana > 0) {
			std::snprintf(G_cTxt, sizeof(G_cTxt), "Aresden Mana: %d Total:%d", wAresdenMana, m_pGame->m_iAresdenMana);
			PutLogList(G_cTxt);
		}
	}

	if (m_pGame->m_iElvineMapIndex != -1) {
		m_pGame->m_iElvineMana += wElvineMana;
		//testcode
		if (wElvineMana > 0) {
			std::snprintf(G_cTxt, sizeof(G_cTxt), "Elvine Mana: %d Total:%d", wElvineMana, m_pGame->m_iElvineMana);
			PutLogList(G_cTxt);
		}
	}
}

void WarManager::SendCollectedMana()
{

	if ((m_pGame->m_iCollectedMana[1] == 0) && (m_pGame->m_iCollectedMana[2] == 0)) return;

	//testcode
	std::snprintf(G_cTxt, sizeof(G_cTxt), "Sending Collected Mana: %d %d", m_pGame->m_iCollectedMana[1], m_pGame->m_iCollectedMana[2]);
	PutLogList(G_cTxt);

	CollectedManaHandler(m_pGame->m_iCollectedMana[1], m_pGame->m_iCollectedMana[2]);

	m_pGame->m_iCollectedMana[0] = 0;
	m_pGame->m_iCollectedMana[1] = 0;
	m_pGame->m_iCollectedMana[2] = 0;
}

void WarManager::_SendMapStatus(int iClientH)
{
	char cData[hb::shared::limits::MaxCrusadeStructures * sizeof(hb::net::CrusadeStructureEntry) + sizeof(hb::net::CrusadeMapStatusHeader)];
	std::memset(cData, 0, sizeof(cData));

	auto& hdr = *reinterpret_cast<hb::net::CrusadeMapStatusHeader*>(cData);
	std::memcpy(hdr.map_name, m_pGame->m_pClientList[iClientH]->m_cSendingMapName, sizeof(hdr.map_name));
	hdr.send_point = static_cast<int16_t>(m_pGame->m_pClientList[iClientH]->m_iCSIsendPoint);

	if (m_pGame->m_pClientList[iClientH]->m_iCSIsendPoint == 0)
		m_pGame->m_pClientList[iClientH]->m_bIsSendingMapStatus = true;

	auto* entries = reinterpret_cast<hb::net::CrusadeStructureEntry*>(cData + sizeof(hb::net::CrusadeMapStatusHeader));
	int entryCount = 0;

	for (int i = 0; i < 100; i++) {
		if (m_pGame->m_pClientList[iClientH]->m_iCSIsendPoint >= hb::shared::limits::MaxCrusadeStructures) goto SMS_ENDOFDATA;
		if (m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[m_pGame->m_pClientList[iClientH]->m_iCSIsendPoint].cType == 0) goto SMS_ENDOFDATA;

		entries[entryCount].type = m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[m_pGame->m_pClientList[iClientH]->m_iCSIsendPoint].cType;
		entries[entryCount].x = m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[m_pGame->m_pClientList[iClientH]->m_iCSIsendPoint].sX;
		entries[entryCount].y = m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[m_pGame->m_pClientList[iClientH]->m_iCSIsendPoint].sY;
		entries[entryCount].side = m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[m_pGame->m_pClientList[iClientH]->m_iCSIsendPoint].cSide;

		entryCount++;
		m_pGame->m_pClientList[iClientH]->m_iCSIsendPoint++;
	}

	hdr.count = static_cast<uint8_t>(entryCount);
	m_pGame->SendNotifyMsg(0, iClientH, Notify::MapStatusNext, static_cast<int>(sizeof(hb::net::CrusadeMapStatusHeader) + entryCount * sizeof(hb::net::CrusadeStructureEntry)), 0, 0, cData);
	return;

SMS_ENDOFDATA:

	hdr.count = static_cast<uint8_t>(entryCount);
	m_pGame->SendNotifyMsg(0, iClientH, Notify::MapStatusLast, static_cast<int>(sizeof(hb::net::CrusadeMapStatusHeader) + entryCount * sizeof(hb::net::CrusadeStructureEntry)), 0, 0, cData);
	m_pGame->m_pClientList[iClientH]->m_bIsSendingMapStatus = false;

	return;
}

void WarManager::MapStatusHandler(int iClientH, int iMode, const char* pMapName)
{
	

	if (m_pGame->m_pClientList[iClientH] == 0) return;

	switch (iMode) {
	case 1:
		if (m_pGame->m_pClientList[iClientH]->m_iCrusadeDuty == 0) return;

		for(int i = 0; i < MaxGuilds; i++)
			if ((m_pGame->m_pGuildTeleportLoc[i].m_iV1 != 0) && (m_pGame->m_pGuildTeleportLoc[i].m_iV1 == m_pGame->m_pClientList[iClientH]->m_iGuildGUID)) {
				m_pGame->SendNotifyMsg(0, iClientH, Notify::TcLoc, m_pGame->m_pGuildTeleportLoc[i].m_sDestX, m_pGame->m_pGuildTeleportLoc[i].m_sDestY,
					0, m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName, m_pGame->m_pGuildTeleportLoc[i].m_sDestX2, m_pGame->m_pGuildTeleportLoc[i].m_sDestY2,
					0, 0, 0, 0, m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName2);
				std::memset(m_pGame->m_pClientList[iClientH]->m_cConstructMapName, 0, sizeof(m_pGame->m_pClientList[iClientH]->m_cConstructMapName));
				memcpy(m_pGame->m_pClientList[iClientH]->m_cConstructMapName, m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName2, 10);
				m_pGame->m_pClientList[iClientH]->m_iConstructLocX = m_pGame->m_pGuildTeleportLoc[i].m_sDestX2;
				m_pGame->m_pClientList[iClientH]->m_iConstructLocY = m_pGame->m_pGuildTeleportLoc[i].m_sDestY2;
				return;
			}

		break;

	case 3:
		//if (m_pGame->m_pClientList[iClientH]->m_iCrusadeDuty != 3) return;
		for(int i = 0; i < hb::shared::limits::MaxCrusadeStructures; i++) {
			m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[i].cType = 0;
			m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[i].cSide = 0;
			m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[i].sX = 0;
			m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[i].sY = 0;
		}
		m_pGame->m_pClientList[iClientH]->m_iCSIsendPoint = 0;
		std::memset(m_pGame->m_pClientList[iClientH]->m_cSendingMapName, 0, sizeof(m_pGame->m_pClientList[iClientH]->m_cSendingMapName));

		if (strcmp(pMapName, m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->m_cName) == 0) {
			for(int i = 0; i < m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->m_iTotalCrusadeStructures; i++) {
				if (m_pGame->m_pClientList[iClientH]->m_iCrusadeDuty == 3)
				{
					m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[i].cType = m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->m_stCrusadeStructureInfo[i].cType;
					m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[i].cSide = m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->m_stCrusadeStructureInfo[i].cSide;
					m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[i].sX = m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->m_stCrusadeStructureInfo[i].sX;
					m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[i].sY = m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->m_stCrusadeStructureInfo[i].sY;
				}
				else if (m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->m_stCrusadeStructureInfo[i].cType == 42)
				{
					m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[i].cType = m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->m_stCrusadeStructureInfo[i].cType;
					m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[i].cSide = m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->m_stCrusadeStructureInfo[i].cSide;
					m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[i].sX = m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->m_stCrusadeStructureInfo[i].sX;
					m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[i].sY = m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->m_stCrusadeStructureInfo[i].sY;
				}
			}
			memcpy(m_pGame->m_pClientList[iClientH]->m_cSendingMapName, m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->m_cName, 10);
		}
		else {
			if (strcmp(pMapName, "middleland") == 0) {
				for(int i = 0; i < m_pGame->m_iTotalMiddleCrusadeStructures; i++) {
					if (m_pGame->m_pClientList[iClientH]->m_iCrusadeDuty == 3)
					{
						m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[i].cType = m_pGame->m_stMiddleCrusadeStructureInfo[i].cType;
						m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[i].cSide = m_pGame->m_stMiddleCrusadeStructureInfo[i].cSide;
						m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[i].sX = m_pGame->m_stMiddleCrusadeStructureInfo[i].sX;
						m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[i].sY = m_pGame->m_stMiddleCrusadeStructureInfo[i].sY;
					}
					else if (m_pGame->m_stMiddleCrusadeStructureInfo[i].cType == 42)
					{
						m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[i].cType = m_pGame->m_stMiddleCrusadeStructureInfo[i].cType;
						m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[i].cSide = m_pGame->m_stMiddleCrusadeStructureInfo[i].cSide;
						m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[i].sX = m_pGame->m_stMiddleCrusadeStructureInfo[i].sX;
						m_pGame->m_pClientList[iClientH]->m_stCrusadeStructureInfo[i].sY = m_pGame->m_stMiddleCrusadeStructureInfo[i].sY;
					}
				}
				strcpy(m_pGame->m_pClientList[iClientH]->m_cSendingMapName, "middleland");
			}
			else {
			}
		}

		_SendMapStatus(iClientH);
		break;
	}
}

void WarManager::RequestSummonWarUnitHandler(int iClientH, int dX, int dY, char cType, char cNum, char cMode)
{
	char cName[6], cNpcName[hb::shared::limits::NpcNameLen], cMapName[11], cNpcWayPoint[11], cOwnerType;
	int x;
	int iNamingValue, tX, tY;
	int bRet;
	short sOwnerH;
	uint32_t dwTime = GameClock::GetTimeMS();

	if (m_pGame->m_pClientList[iClientH] == 0) return;
	if (m_pGame->m_pClientList[iClientH]->m_bIsInitComplete == false) return;
	//hbest - crusade units summon mapcheck
	if (((strcmp(m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->m_cLocationName, "toh3") == 0) || (strcmp(m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->m_cLocationName, "icebound") == 0))) {
		return;
	}

	std::memset(cNpcWayPoint, 0, sizeof(cNpcWayPoint));
	std::memset(cNpcName, 0, sizeof(cNpcName));
	std::memset(cMapName, 0, sizeof(cMapName));

	if (cType < 0) return;
	if (cType >= MaxNpcTypes) return;
	if (cNum > 10) return;

	if (m_pGame->m_pClientList[iClientH]->m_iConstructionPoint < m_pGame->m_iNpcConstructionPoint[cType]) return;
	if ((m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex] != 0) && (m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->m_bIsFixedDayMode)) return;

	cNum = 1;

	// ConstructionPoint     .
	for (x = 1; x <= cNum; x++) {
		iNamingValue = m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->iGetEmptyNamingValue();
		if (iNamingValue == -1) {
			// NPC  .     .
		}
		else {
			// NPC .
			std::memset(cName, 0, sizeof(cName));
			std::snprintf(cName, sizeof(cName), "XX%d", iNamingValue);
			cName[0] = '_';
			cName[1] = m_pGame->m_pClientList[iClientH]->m_cMapIndex + 65;


			switch (cType) {
			case 43: // Light War Beetle
				switch (m_pGame->m_pClientList[iClientH]->m_cSide) {
				case 1: strcpy(cNpcName, "LWB-Aresden"); break;
				case 2: strcpy(cNpcName, "LWB-Elvine"); break;
				}
				break;

			case 36: // Arrow Guard Tower
				switch (m_pGame->m_pClientList[iClientH]->m_cSide) {
				case 1: strcpy(cNpcName, "AGT-Aresden"); break;
				case 2: strcpy(cNpcName, "AGT-Elvine"); break;
				}
				break;

			case 37: // Cannon Guard Tower
				switch (m_pGame->m_pClientList[iClientH]->m_cSide) {
				case 1: strcpy(cNpcName, "CGT-Aresden"); break;
				case 2: strcpy(cNpcName, "CGT-Elvine"); break;
				}
				break;

			case 38: // Mana Collector
				switch (m_pGame->m_pClientList[iClientH]->m_cSide) {
				case 1: strcpy(cNpcName, "MS-Aresden"); break;
				case 2: strcpy(cNpcName, "MS-Elvine"); break;
				}
				break;

			case 39: // Detector
				switch (m_pGame->m_pClientList[iClientH]->m_cSide) {
				case 1: strcpy(cNpcName, "DT-Aresden"); break;
				case 2: strcpy(cNpcName, "DT-Elvine"); break;
				}
				break;

			case 51: // Catapult
				switch (m_pGame->m_pClientList[iClientH]->m_cSide) {
				case 1: strcpy(cNpcName, "CP-Aresden"); break;
				case 2: strcpy(cNpcName, "CP-Elvine"); break;
				}
				break;

			case 44:
				strcpy(cNpcName, "GHK");
				break;

			case 45:
				strcpy(cNpcName, "GHKABS");
				break;

			case 46:
				strcpy(cNpcName, "TK");
				break;

			case 47:
				strcpy(cNpcName, "BG");
				break;

			case 82:
				switch (m_pGame->m_pClientList[iClientH]->m_cSide) {
				case 1: strcpy(cNpcName, "Sor-Aresden"); break;
				case 2: strcpy(cNpcName, "Sor-Elvine"); break;
				}
				break;

			case 83:
				switch (m_pGame->m_pClientList[iClientH]->m_cSide) {
				case 1: strcpy(cNpcName, "ATK-Aresden"); break;
				case 2: strcpy(cNpcName, "ATK-Elvine"); break;
				}
				break;

			case 84:
				switch (m_pGame->m_pClientList[iClientH]->m_cSide) {
				case 1: strcpy(cNpcName, "Elf-Aresden"); break;
				case 2: strcpy(cNpcName, "Elf-Elvine"); break;
				}
				break;

			case 85:
				switch (m_pGame->m_pClientList[iClientH]->m_cSide) {
				case 1: strcpy(cNpcName, "DSK-Aresden"); break;
				case 2: strcpy(cNpcName, "DSK-Elvine"); break;
				}
				break;

			case 86:
				switch (m_pGame->m_pClientList[iClientH]->m_cSide) {
				case 1: strcpy(cNpcName, "HBT-Aresden"); break;
				case 2: strcpy(cNpcName, "HBT-Elvine"); break;
				}
				break;

			case 87:
				switch (m_pGame->m_pClientList[iClientH]->m_cSide) {
				case 1: strcpy(cNpcName, "CT-Aresden"); break;
				case 2: strcpy(cNpcName, "CT-Elvine"); break;
				}
				break;

			case 88:
				switch (m_pGame->m_pClientList[iClientH]->m_cSide) {
				case 1: strcpy(cNpcName, "Bar-Aresden"); break;
				case 2: strcpy(cNpcName, "Bar-Elvine"); break;
				}
				break;

			case 89:
				switch (m_pGame->m_pClientList[iClientH]->m_cSide) {
				case 1: strcpy(cNpcName, "AGC-Aresden"); break;
				case 2: strcpy(cNpcName, "AGC-Elvine"); break;
				}
				break;
			}

			//testcode
			std::snprintf(G_cTxt, sizeof(G_cTxt), "(!) Request Summon War Unit (%d) (%s)", cType, cNpcName);
			PutLogList(G_cTxt);

			tX = (int)dX;
			tY = (int)dY;

			bRet = false;
			switch (cType) {
			case 36:
			case 37:
			case 38:
			case 39:
				if (strcmp(m_pGame->m_pClientList[iClientH]->m_cConstructMapName, m_pGame->m_pClientList[iClientH]->m_cMapName) != 0) bRet = true;
				if (abs(m_pGame->m_pClientList[iClientH]->m_sX - m_pGame->m_pClientList[iClientH]->m_iConstructLocX) > 10) bRet = true;
				if (abs(m_pGame->m_pClientList[iClientH]->m_sY - m_pGame->m_pClientList[iClientH]->m_iConstructLocY) > 10) bRet = true;

				if (bRet) {
					m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->SetNamingValueEmpty(iNamingValue);
					m_pGame->SendNotifyMsg(0, iClientH, Notify::CannotConstruct, 2, 0, 0, 0);
					return;
				}

				for(int i = 0; i < MaxGuilds; i++)
					if (m_pGame->m_pGuildTeleportLoc[i].m_iV1 == m_pGame->m_pClientList[iClientH]->m_iGuildGUID) {
						m_pGame->m_pGuildTeleportLoc[i].m_dwTime = dwTime;
						if (m_pGame->m_pGuildTeleportLoc[i].m_iV2 >= MaxConstructNum) {
							m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->SetNamingValueEmpty(iNamingValue);
							m_pGame->SendNotifyMsg(0, iClientH, Notify::CannotConstruct, 3, 0, 0, 0);
							return;
						}
						else {
							m_pGame->m_pGuildTeleportLoc[i].m_iV2++;
							goto RSWU_LOOPBREAK;
						}
					}

				m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->SetNamingValueEmpty(iNamingValue);
				m_pGame->SendNotifyMsg(0, iClientH, Notify::CannotConstruct, 3, 0, 0, 0);
				return;
				break;
			case 43:
			case 44:
			case 45:
			case 46:
			case 47:
			case 51:
				break;

			case 40:
			case 41:
			case 42:
			case 48:
			case 49:
			case 50:
				break;
			}

		RSWU_LOOPBREAK:

			bRet = false;
			switch (cType) {
			case 36:
			case 37:
				for(int ix = tX - 2; ix <= tX + 2; ix++)
					for(int iy = tY - 2; iy <= tY + 2; iy++) {
						m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->GetOwner(&sOwnerH, &cOwnerType, ix, iy);
						if ((sOwnerH != 0) && (cOwnerType == hb::shared::owner_class::Npc)) {
							switch (m_pGame->m_pNpcList[sOwnerH]->m_sType) {
							case 36:
							case 37:
								bRet = true;
								break;
							}
						}
					}

				if ((dY <= 32) || (dY >= 783)) bRet = true;
				break;
			}

			if (bRet) {
				m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->SetNamingValueEmpty(iNamingValue);
				m_pGame->SendNotifyMsg(0, iClientH, Notify::CannotConstruct, 1, 0, 0, 0);
				return;
			}

			int iNpcConfigId = m_pGame->GetNpcConfigIdByName(cNpcName);
			if (cMode == 0) {
				bRet = m_pGame->bCreateNewNpc(iNpcConfigId, cName, m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->m_cName, 0, 0, MoveType::Follow, &tX, &tY, cNpcWayPoint, 0, 0, -1, false, false, false, false, m_pGame->m_pClientList[iClientH]->m_iGuildGUID);
				if (m_pGame->m_pEntityManager != 0) m_pGame->m_pEntityManager->bSetNpcFollowMode(cName, m_pGame->m_pClientList[iClientH]->m_cCharName, hb::shared::owner_class::Player);
			}
			else bRet = m_pGame->bCreateNewNpc(iNpcConfigId, cName, m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->m_cName, 0, 0, MoveType::Guard, &tX, &tY, cNpcWayPoint, 0, 0, -1, false, false, false, false, m_pGame->m_pClientList[iClientH]->m_iGuildGUID);

			if (bRet == false) {
				// NameValue .
				m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->SetNamingValueEmpty(iNamingValue);
			}
			else {
				m_pGame->m_pClientList[iClientH]->m_iConstructionPoint -= m_pGame->m_iNpcConstructionPoint[cType];
				if (m_pGame->m_pClientList[iClientH]->m_iConstructionPoint < 0) m_pGame->m_pClientList[iClientH]->m_iConstructionPoint = 0;
				m_pGame->SendNotifyMsg(0, iClientH, Notify::ConstructionPoint, m_pGame->m_pClientList[iClientH]->m_iConstructionPoint, m_pGame->m_pClientList[iClientH]->m_iWarContribution, 0, 0);
			}
		}
	}
}

void WarManager::RequestGuildTeleportHandler(int iClientH)
{
	
	char cMapName[11];

	if (m_pGame->m_pClientList[iClientH] == 0) return;
	if (m_pGame->m_pClientList[iClientH]->m_iLockedMapTime != 0) {
		m_pGame->SendNotifyMsg(0, iClientH, Notify::LockedMap, m_pGame->m_pClientList[iClientH]->m_iLockedMapTime, 0, 0, m_pGame->m_pClientList[iClientH]->m_cLockedMapName);
		return;
	}

	// if a guild teleport is set when its not a crusade, log the hacker
	if (!m_pGame->m_bIsCrusadeMode) {
		try
		{
			std::snprintf(G_cTxt, sizeof(G_cTxt), "Accessing crusade teleport: (%s) Player: (%s) - setting teleport location when crusade is disabled.",
				m_pGame->m_pClientList[iClientH]->m_cIPaddress, m_pGame->m_pClientList[iClientH]->m_cCharName);
			PutHackLogFileList(G_cTxt);
			m_pGame->DeleteClient(iClientH, true, true);
		}
		catch (...)
		{
		}
		return;
	}

	// if a player is using guild teleport and he is not in a guild, log the hacker
	if (m_pGame->m_pClientList[iClientH]->m_iCrusadeDuty == 0) {
		try
		{
			std::snprintf(G_cTxt, sizeof(G_cTxt), "Accessing crusade teleport: (%s) Player: (%s) - teleporting when not in a guild",
				m_pGame->m_pClientList[iClientH]->m_cIPaddress, m_pGame->m_pClientList[iClientH]->m_cCharName);
			PutHackLogFileList(G_cTxt);
			m_pGame->DeleteClient(iClientH, true, true);
		}
		catch (...)
		{
		}
		return;
	}

	if ((m_pGame->m_pClientList[iClientH]->m_cMapIndex == m_pGame->m_iMiddlelandMapIndex) &&
		m_pGame->m_iMiddlelandMapIndex != -1)
		return;

	for(int i = 0; i < MaxGuilds; i++)
		if (m_pGame->m_pGuildTeleportLoc[i].m_iV1 == m_pGame->m_pClientList[iClientH]->m_iGuildGUID) {
			std::memset(cMapName, 0, sizeof(cMapName));
			strcpy(cMapName, m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName);

			//testcode
			std::snprintf(G_cTxt, sizeof(G_cTxt), "ReqGuildTeleport: %d %d %d %s", m_pGame->m_pClientList[iClientH]->m_iGuildGUID, m_pGame->m_pGuildTeleportLoc[i].m_sDestX, m_pGame->m_pGuildTeleportLoc[i].m_sDestY, cMapName);
			PutLogList(G_cTxt);

			// !!! RequestTeleportHandler m_cMapName
			m_pGame->RequestTeleportHandler(iClientH, "2   ", cMapName, m_pGame->m_pGuildTeleportLoc[i].m_sDestX, m_pGame->m_pGuildTeleportLoc[i].m_sDestY);
			return;
		}

	switch (m_pGame->m_pClientList[iClientH]->m_cSide) {
	case 1:
		break;
	case 2:
		break;
	}
}

void WarManager::RequestSetGuildTeleportLocHandler(int iClientH, int dX, int dY, int iGuildGUID, char* pMapName)
{
	
	int iIndex;
	uint32_t dwTemp, dwTime;

	if (m_pGame->m_pClientList[iClientH] == 0) return;
	if (m_pGame->m_pClientList[iClientH]->m_bIsOnServerChange) return;

	// if a player is teleporting and its not a crusade, log the hacker
	if (!m_pGame->m_bIsCrusadeMode) {
		try
		{
			std::snprintf(G_cTxt, sizeof(G_cTxt), "Accessing Crusade Set Teleport:(%s) Player: (%s) - setting point when not a crusade.",
				m_pGame->m_pClientList[iClientH]->m_cIPaddress, m_pGame->m_pClientList[iClientH]->m_cCharName);
			PutHackLogFileList(G_cTxt);
			m_pGame->DeleteClient(iClientH, true, true);
		}
		catch (...)
		{

		}
		return;
	}

	// if a player is teleporting and its not a crusade, log the hacker
	if (m_pGame->m_pClientList[iClientH]->m_iCrusadeDuty != 3) {
		try
		{
			std::snprintf(G_cTxt, sizeof(G_cTxt), "Accessing Crusade Set Teleport: (%s) Player: (%s) - setting point when not a guildmaster.",
				m_pGame->m_pClientList[iClientH]->m_cIPaddress, m_pGame->m_pClientList[iClientH]->m_cCharName);
			PutHackLogFileList(G_cTxt);
			m_pGame->DeleteClient(iClientH, true, true);
		}
		catch (...)
		{

		}
		return;
	}

	if (dY < 100) dY = 100;
	if (dY > 600) dY = 600;

	dwTime = GameClock::GetTimeMS();

	//testcode
	std::snprintf(G_cTxt, sizeof(G_cTxt), "SetGuildTeleportLoc: %d %s %d %d", iGuildGUID, pMapName, dX, dY);
	PutLogList(G_cTxt);

	// GUID       .
	for(int i = 0; i < MaxGuilds; i++)
		if (m_pGame->m_pGuildTeleportLoc[i].m_iV1 == iGuildGUID) {
			if ((m_pGame->m_pGuildTeleportLoc[i].m_sDestX == dX) && (m_pGame->m_pGuildTeleportLoc[i].m_sDestY == dY) && (strcmp(m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName, pMapName) == 0)) {
				m_pGame->m_pGuildTeleportLoc[i].m_dwTime = dwTime;
				return;
			}
			else {
				m_pGame->m_pGuildTeleportLoc[i].m_sDestX = dX;
				m_pGame->m_pGuildTeleportLoc[i].m_sDestY = dY;
				std::memset(m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName, 0, sizeof(m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName));
				strcpy(m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName, pMapName);
				m_pGame->m_pGuildTeleportLoc[i].m_dwTime = dwTime;
				return;
			}
		}

	dwTemp = 0;
	iIndex = -1;
	for(int i = 0; i < MaxGuilds; i++) {
		if (m_pGame->m_pGuildTeleportLoc[i].m_iV1 == 0) {

			m_pGame->m_pGuildTeleportLoc[i].m_iV1 = iGuildGUID;
			m_pGame->m_pGuildTeleportLoc[i].m_sDestX = dX;
			m_pGame->m_pGuildTeleportLoc[i].m_sDestY = dY;
			std::memset(m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName, 0, sizeof(m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName));
			strcpy(m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName, pMapName);
			m_pGame->m_pGuildTeleportLoc[i].m_dwTime = dwTime;
			return;
		}
		else {
			if (dwTemp < (dwTime - m_pGame->m_pGuildTeleportLoc[i].m_dwTime)) {
				dwTemp = (dwTime - m_pGame->m_pGuildTeleportLoc[i].m_dwTime);
				iIndex = i;
			}
		}
	}

	// .         (iIndex)   .
	if (iIndex == -1) return;

	////testcode
	//PutLogList("(X) No more GuildTeleportLoc Space! Replaced.");

	//m_pGame->m_pGuildTeleportLoc[i].m_iV1 = iGuildGUID;
	//m_pGame->m_pGuildTeleportLoc[i].m_sDestX = dX;
	//m_pGame->m_pGuildTeleportLoc[i].m_sDestY = dY;
	//std::memset(m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName, 0, sizeof(m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName));
	//strcpy(m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName, pMapName);
	//m_pGame->m_pGuildTeleportLoc[i].m_dwTime = dwTime;
}

void WarManager::RequestSetGuildConstructLocHandler(int iClientH, int dX, int dY, int iGuildGUID, const char* pMapName)
{
	int iIndex;
	uint32_t dwTemp, dwTime;

	if (m_pGame->m_pClientList[iClientH] == 0) return;
	if (m_pGame->m_pClientList[iClientH]->m_bIsOnServerChange) return;

	dwTime = GameClock::GetTimeMS();

	//testcode
	std::snprintf(G_cTxt, sizeof(G_cTxt), "SetGuildConstructLoc: %d %s %d %d", iGuildGUID, pMapName, dX, dY);
	PutLogList(G_cTxt);

	// GUID       .
	for(int i = 0; i < MaxGuilds; i++)
	{
		if (m_pGame->m_pGuildTeleportLoc[i].m_iV1 == iGuildGUID) {
			if ((m_pGame->m_pGuildTeleportLoc[i].m_sDestX2 == dX) && (m_pGame->m_pGuildTeleportLoc[i].m_sDestY2 == dY) && (strcmp(m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName2, pMapName) == 0)) {
				m_pGame->m_pGuildTeleportLoc[i].m_dwTime2 = dwTime;
				return;
			}
			else {
				m_pGame->m_pGuildTeleportLoc[i].m_sDestX2 = dX;
				m_pGame->m_pGuildTeleportLoc[i].m_sDestY2 = dY;
				std::memset(m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName2, 0, sizeof(m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName2));
				strcpy(m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName2, pMapName);
				m_pGame->m_pGuildTeleportLoc[i].m_dwTime2 = dwTime;
				return;
			}
		}
	}

	dwTemp = 0;
	iIndex = -1;
	for(int i = 0; i < MaxGuilds; i++) {
		{
			if (m_pGame->m_pGuildTeleportLoc[i].m_iV1 == 0) {

				m_pGame->m_pGuildTeleportLoc[i].m_iV1 = iGuildGUID;
				m_pGame->m_pGuildTeleportLoc[i].m_sDestX2 = dX;
				m_pGame->m_pGuildTeleportLoc[i].m_sDestY2 = dY;
				std::memset(m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName2, 0, sizeof(m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName2));
				strcpy(m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName2, pMapName);
				m_pGame->m_pGuildTeleportLoc[i].m_dwTime2 = dwTime;
				return;
			}
			else {
				if (dwTemp < (dwTime - m_pGame->m_pGuildTeleportLoc[i].m_dwTime2)) {
					dwTemp = (dwTime - m_pGame->m_pGuildTeleportLoc[i].m_dwTime2);
					iIndex = i;
				}
			}
		}
	}

	// .         (iIndex)   .
	//if (iIndex == -1) return;

	////testcode
	//PutLogList("(X) No more GuildConstructLoc Space! Replaced.");

	//m_pGame->m_pGuildTeleportLoc[i].m_iV1 = iGuildGUID;
	//m_pGame->m_pGuildTeleportLoc[i].m_sDestX2 = dX;
	//m_pGame->m_pGuildTeleportLoc[i].m_sDestY2 = dY;
	//std::memset(m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName2, 0, sizeof(m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName2));
	//strcpy(m_pGame->m_pGuildTeleportLoc[i].m_cDestMapName, pMapName);
	//m_pGame->m_pGuildTeleportLoc[i].m_dwTime2 = dwTime;
}

void WarManager::SetHeldenianMode()
{
	SYSTEMTIME SysTime;

	GetLocalTime(&SysTime);
	m_pGame->m_dwHeldenianStartHour = SysTime.wHour;
	m_pGame->m_dwHeldenianStartMinute = SysTime.wMinute;

	if (m_pGame->m_cHeldenianModeType != 2) {
		m_pGame->m_cHeldenianVictoryType = m_pGame->m_sLastHeldenianWinner;
	}
}

void WarManager::GlobalStartHeldenianMode()
{
	uint32_t dwTime = GameClock::GetTimeMS();
	LocalStartHeldenianMode(m_pGame->m_cHeldenianModeType, m_pGame->m_sLastHeldenianWinner, dwTime);

}

void WarManager::LocalStartHeldenianMode(short sV1, short sV2, uint32_t dwHeldenianGUID)
{
	int x, z, iNamingValue;
	char cName[hb::shared::limits::CharNameLen], cNpcWaypointIndex[10], cSide, cOwnerType;
	short sOwnerH;
	int bRet;
	int dX, dY;

	if (m_pGame->m_bIsHeldenianMode) return;

	if ((m_pGame->m_cHeldenianModeType == -1) || (m_pGame->m_cHeldenianModeType != sV1)) {
		m_pGame->m_cHeldenianModeType = static_cast<char>(sV1);
	}
	if ((m_pGame->m_sLastHeldenianWinner != -1) && (m_pGame->m_sLastHeldenianWinner == sV2)) {
		std::snprintf(G_cTxt, sizeof(G_cTxt), "Heldenian Mode : %d , Heldenian Last Winner : %d", m_pGame->m_cHeldenianModeType, m_pGame->m_sLastHeldenianWinner);
		PutLogFileList(G_cTxt);
	}

	if (dwHeldenianGUID != 0) {
		_CreateHeldenianGUID(dwHeldenianGUID, 0);
		m_pGame->m_dwHeldenianGUID = dwHeldenianGUID;
	}
	m_pGame->m_iHeldenianAresdenLeftTower = 0;
	m_pGame->m_iHeldenianElvineLeftTower = 0;
	m_pGame->m_iHeldenianAresdenDead = 0;
	m_pGame->m_iHeldenianElvineDead = 0;

	for(int i = 0; i < MaxClients; i++) {
		if (m_pGame->m_pClientList[i] != 0) {
			if (m_pGame->m_pClientList[i]->m_bIsInitComplete != true) break;
			m_pGame->m_pClientList[i]->m_cVar = 2;
			m_pGame->SendNotifyMsg(0, i, Notify::HeldenianTeleport, 0, 0, 0, 0);
			m_pGame->m_pClientList[i]->m_iWarContribution = 0;
			m_pGame->m_pClientList[i]->m_iConstructionPoint = (m_pGame->m_pClientList[i]->m_iCharisma * 300);
			if (m_pGame->m_pClientList[i]->m_iConstructionPoint > 12000) m_pGame->m_pClientList[i]->m_iConstructionPoint = 12000;
			m_pGame->SendNotifyMsg(0, i, Notify::ConstructionPoint, m_pGame->m_pClientList[i]->m_iConstructionPoint, m_pGame->m_pClientList[i]->m_iWarContribution, 1, 0);
		}
	}

	for (x = 0; x < MaxMaps; x++) {
		if (m_pGame->m_pMapList[x] == 0) break;
		if (m_pGame->m_pMapList[x]->m_bIsHeldenianMap) {
			for(int i = 0; i < MaxClients; i++) {
				if (m_pGame->m_pClientList[i] == 0) break;
				if (m_pGame->m_pClientList[i]->m_bIsInitComplete != true) break;
				if (m_pGame->m_pClientList[i]->m_cMapIndex != x) break;
				m_pGame->SendNotifyMsg(0, i, Notify::Unknown0BE8, 0, 0, 0, 0);
				m_pGame->RequestTeleportHandler(i, "1   ", 0, -1, -1);
			}
			for(int i = 0; i < MaxNpcs; i++) {
				if (m_pGame->m_pNpcList[i] == 0) break;
				if (m_pGame->m_pNpcList[i]->m_bIsKilled) break;
				if (m_pGame->m_pNpcList[i]->m_cMapIndex != x) break;
				m_pGame->m_pNpcList[i]->m_bIsSummoned = true;
				RemoveHeldenianNpc(i);
			}

			if (m_pGame->m_cHeldenianModeType == 1) {
				if (strcmp(m_pGame->m_pMapList[x]->m_cName, "BtField") == 0) {
					for(int i = 0; i < smap::MaxHeldenianTower; i++) {
						iNamingValue = m_pGame->m_pMapList[x]->iGetEmptyNamingValue();
						if (m_pGame->m_pMapList[x]->m_stHeldenianTower[i].sTypeID < 1)  break;
						if (m_pGame->m_pMapList[x]->m_stHeldenianTower[i].sTypeID > MaxNpcTypes) break;
						if (iNamingValue != -1) {
							dX = m_pGame->m_pMapList[x]->m_stHeldenianTower[i].dX;
							dY = m_pGame->m_pMapList[x]->m_stHeldenianTower[i].dY;
							cSide = m_pGame->m_pMapList[x]->m_stHeldenianTower[i].cSide;
							int iNpcConfigId = -1;
							for (z = 0; z < MaxNpcTypes; z++) {
								if (m_pGame->m_pNpcConfigList[z] == 0) break;
								if (m_pGame->m_pNpcConfigList[z]->m_sType == m_pGame->m_pMapList[x]->m_stHeldenianTower[i].sTypeID) {
									iNpcConfigId = z;
								}
							}
							std::memset(cName, 0, sizeof(cName));
							std::snprintf(cName, sizeof(cName), "XX%d", iNamingValue);
							cName[0] = 95;
							cName[1] = static_cast<char>(i + 65);
							bRet = m_pGame->bCreateNewNpc(iNpcConfigId, cName, m_pGame->m_pMapList[x]->m_cName, (rand() % 3), 0, MoveType::Random, &dX, &dY, cNpcWaypointIndex, 0, 0, cSide, false, false, false, true, false);
							if (bRet == 0) {
								m_pGame->m_pMapList[x]->SetNamingValueEmpty(iNamingValue);
							}
							else {
								m_pGame->m_pMapList[x]->GetOwner(&sOwnerH, &cOwnerType, dX, dY);
								if ((m_pGame->m_pNpcList[sOwnerH] != 0) && (sOwnerH > 0) && (sOwnerH < MaxNpcs)) {
									m_pGame->m_pNpcList[sOwnerH]->m_iBuildCount = 0;
								}
								if (cSide == 1)	m_pGame->m_iHeldenianAresdenLeftTower += 1;
								if (cSide == 2) m_pGame->m_iHeldenianElvineLeftTower += 1;
							}
						}
					}
					std::snprintf(G_cTxt, sizeof(G_cTxt), "HeldenianAresdenLeftTower : %d , HeldenianElvineLeftTower : %d", m_pGame->m_iHeldenianAresdenLeftTower, m_pGame->m_iHeldenianElvineLeftTower);
					PutLogFileList(G_cTxt);
					UpdateHeldenianStatus();
				}
			}
			else if (m_pGame->m_cHeldenianModeType == 2) {
				if (strcmp(m_pGame->m_pMapList[x]->m_cName, "HRampart") == 0) {
					for(int i = 0; i < smap::MaxHeldenianDoor; i++) {
						iNamingValue = m_pGame->m_pMapList[x]->iGetEmptyNamingValue();
						if (iNamingValue != -1) {
							dX = m_pGame->m_pMapList[x]->m_stHeldenianGateDoor[i].dX;
							dY = m_pGame->m_pMapList[x]->m_stHeldenianGateDoor[i].dY;
							cSide = m_pGame->m_sLastHeldenianWinner;
							int iNpcConfigId = -1;
							for (z = 0; z < MaxNpcTypes; z++) {
								if (m_pGame->m_pNpcConfigList[z] == 0) break;
								if (m_pGame->m_pNpcConfigList[z]->m_sType == 91) {
									iNpcConfigId = z;
								}
							}
							std::memset(cName, 0, sizeof(cName));
							std::snprintf(cName, sizeof(cName), "XX%d", iNamingValue);
							cName[0] = 95;
							cName[1] = static_cast<char>(i + 65);
							bRet = m_pGame->bCreateNewNpc(iNpcConfigId, cName, m_pGame->m_pMapList[x]->m_cName, (rand() % 3), 0, MoveType::Random, &dX, &dY, cNpcWaypointIndex, 0, 0, cSide, false, false, false, true, false);
							if (bRet == 0) {
								m_pGame->m_pMapList[x]->SetNamingValueEmpty(iNamingValue);
							}
							else {
								//m_pGame->m_pMapList[x]->GetOwner(&sOwnerH, &cOwnerType, dX, dY);
								if ((m_pGame->m_pNpcList[bRet] != 0) && (bRet > 0) && (bRet < MaxNpcs)) {
									m_pGame->m_pNpcList[bRet]->m_iBuildCount = 0;
									m_pGame->m_pNpcList[bRet]->m_cDir = m_pGame->m_pMapList[x]->m_stHeldenianGateDoor[i].cDir;
								}
							}
						}
					}
				}
			}
		}
	}
	m_pGame->m_bHeldenianInitiated = true;
	m_pGame->m_bIsHeldenianMode = true;
	std::snprintf(G_cTxt, sizeof(G_cTxt), "(!) HELDENIAN Start.");
	PutLogFileList(G_cTxt);
	m_pGame->m_dwHeldenianStartTime = static_cast<uint32_t>(time(0));
}

void WarManager::GlobalEndHeldenianMode()
{
	//char * cp, cData[32];

	if (m_pGame->m_bIsHeldenianMode == false) return;

	LocalEndHeldenianMode();

}

void WarManager::LocalEndHeldenianMode()
{
	if (m_pGame->m_bIsHeldenianMode == false) return;
	m_pGame->m_bIsHeldenianMode = false;
	m_pGame->m_bHeldenianInitiated = true;

	m_pGame->m_dwHeldenianFinishTime = static_cast<uint32_t>(time(0));
	if (m_pGame->var_88C == 1) {
		if (m_pGame->m_cHeldenianModeType == 1) {
			if (m_pGame->m_iHeldenianAresdenLeftTower > m_pGame->m_iHeldenianElvineLeftTower) {
				m_pGame->m_cHeldenianVictoryType = 1;
			}
			else if (m_pGame->m_iHeldenianAresdenLeftTower < m_pGame->m_iHeldenianElvineLeftTower) {
				m_pGame->m_cHeldenianVictoryType = 2;
			}
			else if (m_pGame->m_iHeldenianAresdenDead < m_pGame->m_iHeldenianElvineDead) {
				m_pGame->m_cHeldenianVictoryType = 1;
			}
			else if (m_pGame->m_iHeldenianAresdenDead > m_pGame->m_iHeldenianElvineDead) {
				m_pGame->m_cHeldenianVictoryType = 2;
			}
			else {
				m_pGame->m_sLastHeldenianWinner = m_pGame->m_cHeldenianVictoryType;
			}
		}
		else if (m_pGame->m_cHeldenianModeType == 2) {
			m_pGame->m_sLastHeldenianWinner = m_pGame->m_cHeldenianVictoryType;
		}
		m_pGame->m_sLastHeldenianWinner = m_pGame->m_cHeldenianVictoryType;
		if (bNotifyHeldenianWinner() == false) {
			std::snprintf(G_cTxt, sizeof(G_cTxt), "(!) HELDENIAN End. Result Report Failed");
			PutLogList(G_cTxt);
		}
	}
	std::snprintf(G_cTxt, sizeof(G_cTxt), "(!) HELDENIAN End. %d", m_pGame->m_sLastHeldenianWinner);
	PutLogList(G_cTxt);

	for(int i = 0; i < MaxMaps; i++)
	{
		if (m_pGame->m_pMapList[i] != 0)
		{
			for (int x = 0; x < MaxClients; x++)
				if ((m_pGame->m_pClientList[x] != 0) && (m_pGame->m_pClientList[x]->m_bIsInitComplete)) {
					m_pGame->SendNotifyMsg(0, x, Notify::HeldenianEnd, 0, 0, 0, 0);
					if (m_pGame->m_pMapList[m_pGame->m_pClientList[x]->m_cMapIndex]->m_bIsHeldenianMap) {
						for (int n = 0; n < MaxNpcs; n++)
							if ((m_pGame->m_pNpcList[n] != 0) && (m_pGame->m_pMapList[m_pGame->m_pNpcList[n]->m_cMapIndex] != 0) && (m_pGame->m_pNpcList[n]->m_bIsSummoned)) {
								RemoveHeldenianNpc(n);
							}
						RemoveOccupyFlags(x);
					}
				}
		}
	}
	_CreateHeldenianGUID(m_pGame->m_dwHeldenianGUID, m_pGame->m_cHeldenianVictoryType);
}

bool WarManager::UpdateHeldenianStatus()
{
	
	bool bFlag;
	int iShortCutIndex, iClientH;

	if (m_pGame->m_bIsHeldenianMode != true) return false;
	for(int i = 0; i < MaxMaps; i++)
		if (m_pGame->m_pMapList[i] != 0) {
			if (m_pGame->m_pMapList[i]->m_bIsHeldenianMap) {
				bFlag = true;
				iShortCutIndex = 0;
			}
			if (bFlag) {
				iClientH = m_pGame->m_iClientShortCut[iShortCutIndex];
				iShortCutIndex++;
				if (iClientH == 0) {
					bFlag = 0;
				}
				else {
					if ((m_pGame->m_pClientList[iClientH] != 0) && (m_pGame->m_pClientList[iClientH]->m_bIsInitComplete) && (strcmp(m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->m_cName, "BtField") == 0)) {
						m_pGame->SendNotifyMsg(0, iClientH, Notify::HeldenianCount, m_pGame->m_iHeldenianAresdenLeftTower, m_pGame->m_iHeldenianElvineLeftTower, m_pGame->m_iHeldenianAresdenDead, 0, m_pGame->m_iHeldenianElvineDead, 0);
					}
				}
			}
		}
	return true;
}

void WarManager::_CreateHeldenianGUID(uint32_t dwHeldenianGUID, int iWinnerSide)
{
	char* cp, cTxt[256], cFn[256], cTemp[1024];
	FILE* pFile;

	_mkdir("GameData");
	std::memset(cFn, 0, sizeof(cFn));

	strcat(cFn, "GameData/");
	strcat(cFn, "HeldenianGUID.txt");

	pFile = fopen(cFn, "wt");
	if (pFile == 0) {
		std::snprintf(cTxt, sizeof(cTxt), "(!) Cannot create HeldenianGUID(%d) file", dwHeldenianGUID);
		PutLogList(cTxt);
	}
	else {
		std::memset(cTemp, 0, sizeof(cTemp));

		std::memset(cTxt, 0, sizeof(cTxt));
		std::snprintf(cTxt, sizeof(cTxt), "HeldenianGUID = %d", dwHeldenianGUID);
		strcat(cTemp, cTxt);

		std::memset(cTxt, 0, sizeof(cTxt));
		std::snprintf(cTxt, sizeof(cTxt), "winner-side = %d\n", iWinnerSide);
		strcat(cTemp, cTxt);

		cp = (char*)cTemp;
		fwrite(cp, strlen(cp), 1, pFile);

		std::snprintf(cTxt, sizeof(cTxt), "(O) HeldenianGUID(%d) file created", dwHeldenianGUID);
		PutLogList(cTxt);
	}
	if (pFile != 0) fclose(pFile);
}

void WarManager::ManualStartHeldenianMode(int iClientH, char* pData, size_t dwMsgSize)
{
	char cHeldenianType, cBuff[256], * token, seps[] = "= \t\r\n";
	SYSTEMTIME SysTime;
	int iV1;

	if (m_pGame->m_bIsHeldenianMode) return;
	if (m_pGame->m_bIsApocalypseMode) return;
	if (m_pGame->m_bIsCrusadeMode) return;
	if ((dwMsgSize != 0) && (pData != 0)) {
		m_pGame->m_bHeldenianRunning = true;
		GetLocalTime(&SysTime);

		std::memset(cBuff, 0, sizeof(cBuff));
		memcpy(cBuff, pData, dwMsgSize);
		token = strtok(NULL, seps);
		token = strtok(NULL, seps);
		if (token != 0) {
			iV1 = atoi(token);
			iV1 += (SysTime.wHour * 24 + SysTime.wMinute * 60);
			m_pGame->m_dwHeldenianStartHour = (iV1 / 24);
			m_pGame->m_dwHeldenianStartMinute = (iV1 / 60);
		}
		token = strtok(NULL, seps);
		if (token != 0) {
			cHeldenianType = atoi(token);
			if ((cHeldenianType == 1) || (cHeldenianType == 2)) {
				m_pGame->m_cHeldenianModeType = cHeldenianType;
			}
		}
	}
	GlobalStartHeldenianMode();
	std::snprintf(G_cTxt, sizeof(G_cTxt), "GM Order(%s): begin Heldenian", m_pGame->m_pClientList[iClientH]->m_cCharName);
	PutLogFileList(G_cTxt);
}

void WarManager::ManualEndHeldenianMode(int iClientH, char* pData, size_t dwMsgSize)
{
	if (m_pGame->m_bIsHeldenianMode) {
		GlobalEndHeldenianMode();
		m_pGame->m_bHeldenianRunning = false;
		std::snprintf(G_cTxt, sizeof(G_cTxt), "GM Order(%s): end Heldenian", m_pGame->m_pClientList[iClientH]->m_cCharName);
		PutLogFileList(G_cTxt);
	}
}

bool WarManager::bNotifyHeldenianWinner()
{
	if (m_pGame->var_88C == 0) {
		return true;
	}
	else {
		return false;
	}

}

void WarManager::RemoveHeldenianNpc(int iNpcH)
{
	if (m_pGame->m_pNpcList[iNpcH] == 0) return;
	if (m_pGame->m_pNpcList[iNpcH]->m_bIsKilled) return;

	m_pGame->m_pNpcList[iNpcH]->m_bIsKilled = true;
	m_pGame->m_pNpcList[iNpcH]->m_iHP = 0;
	m_pGame->m_pNpcList[iNpcH]->m_iLastDamage = 0;
	m_pGame->m_pNpcList[iNpcH]->m_dwRegenTime = 0;
	m_pGame->m_pMapList[m_pGame->m_pNpcList[iNpcH]->m_cMapIndex]->m_iTotalAliveObject--;

	m_pGame->ReleaseFollowMode(iNpcH, hb::shared::owner_class::Npc);
	m_pGame->m_pNpcList[iNpcH]->m_iTargetIndex = 0;
	m_pGame->m_pNpcList[iNpcH]->m_cTargetType = 0;

	m_pGame->SendEventToNearClient_TypeA(iNpcH, hb::shared::owner_class::Npc, MsgId::EventMotion, Type::Dying, 0, 1, 0);
	m_pGame->m_pMapList[m_pGame->m_pNpcList[iNpcH]->m_cMapIndex]->ClearOwner(10, iNpcH, hb::shared::owner_class::Npc, m_pGame->m_pNpcList[iNpcH]->m_sX, m_pGame->m_pNpcList[iNpcH]->m_sY);
	m_pGame->m_pMapList[m_pGame->m_pNpcList[iNpcH]->m_cMapIndex]->SetDeadOwner(iNpcH, hb::shared::owner_class::Npc, m_pGame->m_pNpcList[iNpcH]->m_sX, m_pGame->m_pNpcList[iNpcH]->m_sY);
	m_pGame->m_pNpcList[iNpcH]->m_cBehavior = 4;
	m_pGame->m_pNpcList[iNpcH]->m_sBehaviorTurnCount = 0;
	m_pGame->m_pNpcList[iNpcH]->m_dwDeadTime = GameClock::GetTimeMS();

}

void WarManager::RequestHeldenianTeleport(int iClientH, char* pData, size_t dwMsgSize)
{
	char cTmpName[hb::shared::limits::NpcNameLen], cTxt[512], cMapName[11]{};
	short tX = 0, tY = 0, cLoc = 0;
	int iRet, iWhyReturn = 0;

	if (m_pGame->m_pClientList[iClientH] == 0) return;

	const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(pData, sizeof(hb::net::PacketHeader));
	if (!header) return;
	char* cp = (char*)(pData + sizeof(hb::net::PacketHeader));
	std::memset(cTmpName, 0, sizeof(cTmpName));
	strcpy(cTmpName, cp);
	if (strcmp(cTmpName, "Gail") == 0) {
		std::memset(cTxt, 0, sizeof(cTxt));
		if ((m_pGame->m_bIsHeldenianMode == 1) && (m_pGame->m_pClientList[iClientH]->m_bIsPlayerCivil != true) && (m_pGame->m_pClientList[iClientH]->m_cSide == 2 || m_pGame->m_pClientList[iClientH]->m_cSide == 1)) {
			if (m_pGame->m_cHeldenianType == 1) {
				std::memcpy(cMapName, "BtField", 7);
				if (m_pGame->m_pClientList[iClientH]->m_cSide == 1) {
					tX = 68;
					tY = 225;
					cLoc = 1;
				}
				else if (m_pGame->m_pClientList[iClientH]->m_cSide == 2) {
					tX = 202;
					tY = 70;
					cLoc = 2;
				}
			}
			else if (m_pGame->m_cHeldenianType == 2) {
				std::memcpy(cMapName, "HRampart", 8);
				if (m_pGame->m_pClientList[iClientH]->m_cSide == m_pGame->m_sLastHeldenianWinner) {
					tX = 81;
					tY = 42;
					cLoc = 3;
				}
				else {
					tX = 156;
					tY = 153;
					cLoc = 4;
				}
			}
			iWhyReturn = 0;
		}
	}

	// Build response into cTxt buffer
	std::memset(cTxt, 0, sizeof(cTxt));
	auto& resp = *reinterpret_cast<hb::net::HeldenianTeleportResponse*>(cTxt + sizeof(hb::net::PacketHeader));
	resp.count = 4;
	resp.location = cLoc;
	std::memcpy(resp.map_name, cMapName, sizeof(resp.map_name));
	resp.x = tX;
	resp.y = tY;
	resp.why_return = iWhyReturn;

	iRet = m_pGame->m_pClientList[iClientH]->m_pXSock->iSendMsg(cTxt, static_cast<int>(sizeof(hb::net::PacketHeader) + sizeof(hb::net::HeldenianTeleportResponse)));
	switch (iRet) {
	case sock::Event::QueueFull:
	case sock::Event::SocketError:
	case sock::Event::CriticalError:
	case sock::Event::SocketClosed:
		m_pGame->DeleteClient(iClientH, true, true);
		break;
	}
}

bool WarManager::bCheckHeldenianMap(int sAttackerH, int iMapIndex, char cType)
{
	short tX, tY;
	int iRet;
	class CTile* pTile;

	iRet = 0;
	if (m_pGame->m_pClientList[sAttackerH] == 0) return 0;
	if ((m_pGame->m_bIsHeldenianMode == 1) || (m_pGame->m_cHeldenianType == 1)) {
		if (cType == hb::shared::owner_class::Player) {
			if ((m_pGame->m_pMapList[m_pGame->m_pClientList[sAttackerH]->m_cMapIndex] != 0) && (m_pGame->m_pClientList[sAttackerH]->m_cSide > 0)) {
				tX = m_pGame->m_pClientList[sAttackerH]->m_sX;
				tY = m_pGame->m_pClientList[sAttackerH]->m_sY;
				if ((tX < 0) || (tX >= m_pGame->m_pMapList[m_pGame->m_pClientList[sAttackerH]->m_cMapIndex]->m_sSizeX) ||
					(tY < 0) || (tY >= m_pGame->m_pMapList[m_pGame->m_pClientList[sAttackerH]->m_cMapIndex]->m_sSizeY)) return 0;
				pTile = (class CTile*)(m_pGame->m_pMapList[m_pGame->m_pClientList[sAttackerH]->m_cMapIndex]->m_pTile + tX + tY * m_pGame->m_pMapList[m_pGame->m_pClientList[sAttackerH]->m_cMapIndex]->m_sSizeY);
				if (pTile == 0) return 0;
				if (pTile->m_iOccupyStatus != 0) {
					if (pTile->m_iOccupyStatus < 0) {
						if (m_pGame->m_pClientList[sAttackerH]->m_cSide == 1) {
							iRet = 1;
						}
					}
					else if (pTile->m_iOccupyStatus > 0) {
						if (m_pGame->m_pClientList[sAttackerH]->m_cSide == 2) {
							iRet = 1;
						}
					}
				}
			}
		}
		else if (cType == hb::shared::owner_class::Npc) {
			if ((m_pGame->m_pMapList[m_pGame->m_pNpcList[sAttackerH]->m_cMapIndex] != 0) && (iMapIndex != -1) && (m_pGame->m_pNpcList[sAttackerH]->m_cSide > 0)) {
				tX = m_pGame->m_pNpcList[sAttackerH]->m_sX;
				tY = m_pGame->m_pNpcList[sAttackerH]->m_sY;
				pTile = (class CTile*)(m_pGame->m_pMapList[m_pGame->m_pNpcList[sAttackerH]->m_cMapIndex]->m_pTile + tX + tY * m_pGame->m_pMapList[m_pGame->m_pNpcList[sAttackerH]->m_cMapIndex]->m_sSizeY);
				if (pTile == 0) return 0;
				if (pTile->m_iOccupyStatus != 0) {
					if (pTile->m_iOccupyStatus < 0) {
						if (m_pGame->m_pNpcList[sAttackerH]->m_cSide == 1) {
							iRet = 1;
						}
					}
					else if (pTile->m_iOccupyStatus > 0) {
						if (m_pGame->m_pNpcList[sAttackerH]->m_cSide == 2) {
							iRet = 1;
						}
					}
				}
			}
		}
	}
	return iRet;
}

void WarManager::CheckHeldenianResultCalculation(int iClientH)
{
	double dV1, dV2, dV3;

	if (m_pGame->m_pClientList[iClientH] == 0) return;
	if (m_pGame->m_pClientList[iClientH]->m_cVar != 2) return;
	if ((m_pGame->m_cHeldenianType == 0) || (m_pGame->m_pClientList[iClientH]->m_dwHeldenianGUID == 0)) return;
	if (m_pGame->m_pClientList[iClientH]->m_dwHeldenianGUID == m_pGame->m_dwHeldenianGUID) {
		if (m_pGame->m_pClientList[iClientH]->m_cSide == m_pGame->m_sLastHeldenianWinner) {
			if (m_pGame->m_pClientList[iClientH]->m_iLevel <= 80) {
				m_pGame->m_pClientList[iClientH]->m_iWarContribution += (m_pGame->m_pClientList[iClientH]->m_iLevel) * 200;
			}
			else if (m_pGame->m_pClientList[iClientH]->m_iLevel > 80 && m_pGame->m_pClientList[iClientH]->m_iLevel <= 100) {
				m_pGame->m_pClientList[iClientH]->m_iWarContribution += (m_pGame->m_pClientList[iClientH]->m_iLevel) * 100;
			}
			else if (m_pGame->m_pClientList[iClientH]->m_iLevel > 100) {
				m_pGame->m_pClientList[iClientH]->m_iWarContribution += (m_pGame->m_pClientList[iClientH]->m_iLevel) * 30;
			}
			dV2 = (double)m_pGame->m_pClientList[iClientH]->m_iExp;
			dV3 = (double)m_pGame->m_pClientList[iClientH]->m_iWarContribution * 1.2f;
			dV1 = dV2 + dV3;
			m_pGame->GetExp(iClientH, (uint32_t)dV1);
		}
		else {
			m_pGame->GetExp(iClientH, (m_pGame->m_pClientList[iClientH]->m_iWarContribution / 5));
		}
		m_pGame->m_pClientList[iClientH]->m_iWarContribution = 0;
		m_pGame->m_pClientList[iClientH]->m_dwHeldenianGUID = 0;
		m_pGame->m_pClientList[iClientH]->m_dwSpeedHackCheckTime = GameClock::GetTimeMS();
		m_pGame->m_pClientList[iClientH]->m_iSpeedHackCheckExp = m_pGame->m_pClientList[iClientH]->m_iExp;
	}
}

void WarManager::RemoveOccupyFlags(int iMapIndex)
{
	uint32_t dwTime = GameClock::GetTimeMS();
	
	short dX, dY;
	int iDynamicObjectIndex;
	class COccupyFlag* iOccupyFlagIndex;
	class CTile* pTile;

	if (m_pGame->m_pMapList[iMapIndex] == 0) return;
	for(int i = 1; i < smap::MaxOccupyFlag; i++)
		//if (m_pGame->m_pMapList[iMapIndex]->m_pOccupyFlag[i]) return; // centu : wtf ?
		if (m_pGame->m_pMapList[iMapIndex]->m_pOccupyFlag[i]) {
			dX = m_pGame->m_pMapList[iMapIndex]->m_pOccupyFlag[i]->m_sX;
			dY = m_pGame->m_pMapList[iMapIndex]->m_pOccupyFlag[i]->m_sY;
			pTile = (class CTile*)(m_pGame->m_pMapList[iMapIndex]->m_pTile + dX + dY * m_pGame->m_pMapList[iMapIndex]->m_sSizeY);
			m_pGame->m_pMapList[iMapIndex]->m_iTotalOccupyFlags--;
			iDynamicObjectIndex = m_pGame->m_pMapList[iMapIndex]->m_pOccupyFlag[i]->m_iDynamicObjectIndex;
			if (m_pGame->m_pDynamicObjectManager->m_pDynamicObjectList[iDynamicObjectIndex] == 0) return;

			m_pGame->SendEventToNearClient_TypeB(MsgId::DynamicObject, MsgType::Reject, m_pGame->m_pDynamicObjectManager->m_pDynamicObjectList[iDynamicObjectIndex]->m_cMapIndex,
				m_pGame->m_pDynamicObjectManager->m_pDynamicObjectList[iDynamicObjectIndex]->m_sX, m_pGame->m_pDynamicObjectManager->m_pDynamicObjectList[iDynamicObjectIndex]->m_sY,
				m_pGame->m_pDynamicObjectManager->m_pDynamicObjectList[iDynamicObjectIndex]->m_sType, iDynamicObjectIndex, 0, (short)0);

			m_pGame->m_pMapList[m_pGame->m_pDynamicObjectManager->m_pDynamicObjectList[iDynamicObjectIndex]->m_cMapIndex]->SetDynamicObject(0, 0, m_pGame->m_pDynamicObjectManager->m_pDynamicObjectList[iDynamicObjectIndex]->m_sX, m_pGame->m_pDynamicObjectManager->m_pDynamicObjectList[iDynamicObjectIndex]->m_sY, dwTime);

			iOccupyFlagIndex = m_pGame->m_pMapList[iMapIndex]->m_pOccupyFlag[i];

			if (m_pGame->m_pDynamicObjectManager->m_pDynamicObjectList[iDynamicObjectIndex] == 0) {
				for(int ix = dX - 2; ix <= dX + 2; ix++)
					for(int iy = dY - 2; iy <= dY + 2; iy++) {
						pTile = (class CTile*)(m_pGame->m_pMapList[iMapIndex]->m_pTile + ix + iy * m_pGame->m_pMapList[iMapIndex]->m_sSizeY);
						pTile->m_sOwner = 0;
					}
			}
		}
}

void WarManager::ApocalypseEnder()
{
	SYSTEMTIME SysTime;
	

	if (m_pGame->m_bIsApocalypseMode == false) return;
	if (m_pGame->m_bIsApocalypseStarter == false) return;

	GetLocalTime(&SysTime);

	for(int i = 0; i < MaxApocalypse; i++)
		if ((m_pGame->m_stApocalypseScheduleEnd[i].iDay == SysTime.wDayOfWeek) &&
			(m_pGame->m_stApocalypseScheduleEnd[i].iHour == SysTime.wHour) &&
			(m_pGame->m_stApocalypseScheduleEnd[i].iMinute == SysTime.wMinute)) {
			PutLogList("(!) Automated apocalypse is concluded!");
			GlobalEndApocalypseMode();
			return;
		}
}

void WarManager::GlobalEndApocalypseMode()
{
	if (m_pGame->m_bIsApocalypseMode == false) return;

	LocalEndApocalypse();
}

void WarManager::LocalEndApocalypse()
{
	

	m_pGame->m_bIsApocalypseMode = false;

	for(int i = 1; i < MaxClients; i++) {
		if (m_pGame->m_pClientList[i] != 0) {
			m_pGame->SendNotifyMsg(0, i, Notify::ApocGateEndMsg, 0, 0, 0, 0);
		}
	}
	std::snprintf(G_cTxt, sizeof(G_cTxt), "(!)Apocalypse Mode OFF.");
	PutLogList(G_cTxt);
}

void WarManager::LocalStartApocalypse(uint32_t dwApocalypseGUID)
{
	
	//uint32_t dwApocalypse;

	m_pGame->m_bIsApocalypseMode = true;

	if (dwApocalypseGUID != 0) {
		_CreateApocalypseGUID(dwApocalypseGUID);
		//m_pGame->m_dwApocalypseGUID = dwApocalypse;
	}

	for(int i = 1; i < MaxClients; i++) {
		if (m_pGame->m_pClientList[i] != 0) {
			m_pGame->SendNotifyMsg(0, i, Notify::ApocGateStartMsg, 0, 0, 0, 0);
			//m_pGame->RequestTeleportHandler(i, "0   ");
			//m_pGame->SendNotifyMsg(0, i, Notify::ApocForceRecallPlayers, 0, 0, 0, 0);
		}
	}
	std::snprintf(G_cTxt, sizeof(G_cTxt), "(!)Apocalypse Mode ON.");
	PutLogList(G_cTxt);
}

bool WarManager::bReadApocalypseGUIDFile(char* cFn)
{
	FILE* pFile;
	HANDLE hFile;
	uint32_t  dwFileSize;
	char* cp, * token, cReadMode;
	char seps[] = "= \t\r\n";

	cReadMode = 0;

	hFile = CreateFile(cFn, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
	dwFileSize = GetFileSize(hFile, 0);
	if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);

	pFile = fopen(cFn, "rt");
	if (pFile == 0) {
		return false;
	}
	else {
		cp = new char[dwFileSize + 2];
		std::memset(cp, 0, dwFileSize + 2);
		fread(cp, dwFileSize, 1, pFile);

		token = strtok(cp, seps);

		while (token != 0) {

			if (cReadMode != 0) {
				switch (cReadMode) {
				case 1:
					m_pGame->m_dwApocalypseGUID = atoi(token);
					std::snprintf(G_cTxt, sizeof(G_cTxt), "ApocalypseGUID = %d", m_pGame->m_dwApocalypseGUID);
					PutLogList(G_cTxt);
					cReadMode = 0;
					break;
				}
			}
			else {
				if (memcmp(token, "ApocalypseGUID", 14) == 0) cReadMode = 1;
			}

			token = strtok(NULL, seps);
		}

		delete cp;
	}
	if (pFile != 0) fclose(pFile);

	return true;
}

bool WarManager::bReadHeldenianGUIDFile(char* cFn)
{
	FILE* pFile;
	HANDLE hFile;
	uint32_t  dwFileSize;
	char* cp, * token, cReadMode;
	char seps[] = "= \t\r\n";

	cReadMode = 0;

	hFile = CreateFile(cFn, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
	dwFileSize = GetFileSize(hFile, 0);
	if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);

	pFile = fopen(cFn, "rt");
	if (pFile == 0) {
		return false;
	}
	else {
		cp = new char[dwFileSize + 2];
		std::memset(cp, 0, dwFileSize + 2);
		fread(cp, dwFileSize, 1, pFile);

		token = strtok(cp, seps);

		while (token != 0) {

			if (cReadMode != 0) {
				switch (cReadMode) {
				case 1:
					m_pGame->m_dwHeldenianGUID = atoi(token);
					std::snprintf(G_cTxt, sizeof(G_cTxt), "HeldenianGUID = %d", m_pGame->m_dwHeldenianGUID);
					PutLogList(G_cTxt);
					cReadMode = 0;
					break;
				case 2:
					m_pGame->m_sLastHeldenianWinner = atoi(token);
					std::snprintf(G_cTxt, sizeof(G_cTxt), "HeldenianWinnerSide = %d", m_pGame->m_sLastHeldenianWinner);
					PutLogList(G_cTxt);
					cReadMode = 0;
					break;
				}
			}
			else {
				if (memcmp(token, "HeldenianGUID", 13) == 0) cReadMode = 1;
				if (memcmp(token, "winner-side", 11) == 0) cReadMode = 2;
			}

			token = strtok(NULL, seps);
		}

		delete cp;
	}
	if (pFile != 0) fclose(pFile);

	return true;
}

void WarManager::_CreateApocalypseGUID(uint32_t dwApocalypseGUID)
{
	char* cp, cTxt[256], cFn[256], cTemp[1024];
	FILE* pFile;

	_mkdir("GameData");
	std::memset(cFn, 0, sizeof(cFn));

	strcat(cFn, "GameData/");
	strcat(cFn, "ApocalypseGUID.txt");

	pFile = fopen(cFn, "wt");
	if (pFile == 0) {
		std::snprintf(cTxt, sizeof(cTxt), "(!) Cannot create ApocalypseGUID(%d) file", dwApocalypseGUID);
		PutLogList(cTxt);
	}
	else {
		std::memset(cTemp, 0, sizeof(cTemp));

		std::memset(cTxt, 0, sizeof(cTxt));
		std::snprintf(cTxt, sizeof(cTxt), "ApocalypseGUID = %d\n", dwApocalypseGUID);
		strcat(cTemp, cTxt);

		cp = (char*)cTemp;
		fwrite(cp, strlen(cp), 1, pFile);

		std::snprintf(cTxt, sizeof(cTxt), "(O) ApocalypseGUID(%d) file created", dwApocalypseGUID);
		PutLogList(cTxt);
	}
	if (pFile != 0) fclose(pFile);
}

void WarManager::EnergySphereProcessor()
{
	int iNamingValue, iCIndex, iTemp, pX, pY;
	char cSA, cName_Internal[31], cWaypoint[31];

	if (m_pGame->m_iMiddlelandMapIndex < 0) return;
	if (m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex] == 0) return;
	if (m_pGame->iDice(1, 2000) != 123) return;
	if (m_pGame->m_iTotalGameServerClients < 500) return;

	if (m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_iCurEnergySphereGoalPointIndex >= 0) return;

	iCIndex = m_pGame->iDice(1, m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_iTotalEnergySphereCreationPoint);

	if (m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_stEnergySphereCreationList[iCIndex].cType == 0) return;

	cSA = 0;
	pX = m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_stEnergySphereCreationList[iCIndex].sX;
	pY = m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_stEnergySphereCreationList[iCIndex].sY;
	std::memset(cWaypoint, 0, sizeof(cWaypoint));

	iNamingValue = m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->iGetEmptyNamingValue();
	if (iNamingValue != -1) {
		std::memset(cName_Internal, 0, sizeof(cName_Internal));
		std::snprintf(cName_Internal, sizeof(cName_Internal), "XX%d", iNamingValue);
		cName_Internal[0] = '_';
		cName_Internal[1] = m_pGame->m_iMiddlelandMapIndex + 65;

		int iNpcConfigId = m_pGame->GetNpcConfigIdByName("Energy-Sphere");
		if ((m_pGame->bCreateNewNpc(iNpcConfigId, cName_Internal, m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_cName, (rand() % 5), cSA, MoveType::Random, &pX, &pY, cWaypoint, 0, 0, -1, false, false, false)) == false) {
			m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->SetNamingValueEmpty(iNamingValue);
			return;
		}
	}

	iTemp = m_pGame->iDice(1, m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_iTotalEnergySphereGoalPoint);
	if (m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_stEnergySphereGoalList[iTemp].cResult == 0) return;

	m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_iCurEnergySphereGoalPointIndex = iTemp;

	for(int i = 1; i < MaxClients; i++)
		if ((m_pGame->m_pClientList[i] != 0) && (m_pGame->m_pClientList[i]->m_bIsInitComplete)) {
			m_pGame->SendNotifyMsg(0, i, Notify::EnergySphereCreated, pX, pY, 0, 0);
		}

	std::snprintf(G_cTxt, sizeof(G_cTxt), "(!) Energy Sphere Created! (%d, %d)", pX, pY);
	PutLogList(G_cTxt);
	PutLogFileList(G_cTxt);
}

bool WarManager::bCheckEnergySphereDestination(int iNpcH, short sAttackerH, char cAttackerType)
{
	int sX, sY, dX, dY, iGoalMapIndex;
	char cResult;

	if (m_pGame->m_pNpcList[iNpcH] == 0) return false;
	if (m_pGame->m_pMapList[m_pGame->m_pNpcList[iNpcH]->m_cMapIndex]->m_iCurEnergySphereGoalPointIndex == -1) return false;

	if (m_pGame->m_pNpcList[iNpcH]->m_cMapIndex != m_pGame->m_iMiddlelandMapIndex) {
		iGoalMapIndex = m_pGame->m_pNpcList[iNpcH]->m_cMapIndex;

		sX = m_pGame->m_pNpcList[iNpcH]->m_sX;
		sY = m_pGame->m_pNpcList[iNpcH]->m_sY;

		cResult = m_pGame->m_pMapList[iGoalMapIndex]->m_stEnergySphereGoalList[m_pGame->m_pMapList[iGoalMapIndex]->m_iCurEnergySphereGoalPointIndex].cResult;
		dX = m_pGame->m_pMapList[iGoalMapIndex]->m_stEnergySphereGoalList[m_pGame->m_pMapList[iGoalMapIndex]->m_iCurEnergySphereGoalPointIndex].aresdenX;
		dY = m_pGame->m_pMapList[iGoalMapIndex]->m_stEnergySphereGoalList[m_pGame->m_pMapList[iGoalMapIndex]->m_iCurEnergySphereGoalPointIndex].aresdenY;
		if ((sX >= dX - 2) && (sX <= dX + 2) && (sY >= dY - 2) && (sY <= dY + 2)) {
			m_pGame->m_pMapList[iGoalMapIndex]->m_iCurEnergySphereGoalPointIndex = -1;

			if ((cAttackerType == hb::shared::owner_class::Player) && (m_pGame->m_pClientList[sAttackerH] != 0)) {
				if (m_pGame->m_pClientList[sAttackerH]->m_cSide == 1) { // Aresden (Side:1)
					m_pGame->m_pClientList[sAttackerH]->m_iContribution += 5;
					std::snprintf(G_cTxt, sizeof(G_cTxt), "(!) EnergySphere Hit By Aresden Player (%s)", m_pGame->m_pClientList[sAttackerH]->m_cCharName);
					PutLogFileList(G_cTxt);
				}
				else {
					m_pGame->m_pClientList[sAttackerH]->m_iContribution -= 10;
				}

				for(int i = 1; i < MaxClients; i++)
					if ((m_pGame->m_pClientList[i] != 0) && (m_pGame->m_pClientList[i]->m_bIsInitComplete)) {
						m_pGame->SendNotifyMsg(0, i, Notify::EnergySphereGoalIn, cResult, m_pGame->m_pClientList[sAttackerH]->m_cSide, 2, m_pGame->m_pClientList[sAttackerH]->m_cCharName);
					}
			}
			return true;
		}

		dX = m_pGame->m_pMapList[iGoalMapIndex]->m_stEnergySphereGoalList[m_pGame->m_pMapList[iGoalMapIndex]->m_iCurEnergySphereGoalPointIndex].elvineX;
		dY = m_pGame->m_pMapList[iGoalMapIndex]->m_stEnergySphereGoalList[m_pGame->m_pMapList[iGoalMapIndex]->m_iCurEnergySphereGoalPointIndex].elvineY;
		if ((sX >= dX - 2) && (sX <= dX + 2) && (sY >= dY - 2) && (sY <= dY + 2)) {
			m_pGame->m_pMapList[iGoalMapIndex]->m_iCurEnergySphereGoalPointIndex = -1;

			if ((cAttackerType == hb::shared::owner_class::Player) && (m_pGame->m_pClientList[sAttackerH] != 0)) {
				if (m_pGame->m_pClientList[sAttackerH]->m_cSide == 2) { // Elvine (Side:2)
					m_pGame->m_pClientList[sAttackerH]->m_iContribution += 5;
					std::snprintf(G_cTxt, sizeof(G_cTxt), "(!) EnergySphere Hit By Elvine Player (%s)", m_pGame->m_pClientList[sAttackerH]->m_cCharName);
					PutLogFileList(G_cTxt);
				}
				else {
					m_pGame->m_pClientList[sAttackerH]->m_iContribution -= 10;
				}

				for(int i = 1; i < MaxClients; i++)
					if ((m_pGame->m_pClientList[i] != 0) && (m_pGame->m_pClientList[i]->m_bIsInitComplete)) {
						m_pGame->SendNotifyMsg(0, i, Notify::EnergySphereGoalIn, cResult, m_pGame->m_pClientList[sAttackerH]->m_cSide, 1, m_pGame->m_pClientList[sAttackerH]->m_cCharName);
					}
			}
		}
		return false;
	}
	else {

		sX = m_pGame->m_pNpcList[iNpcH]->m_sX;
		sY = m_pGame->m_pNpcList[iNpcH]->m_sY;

		cResult = m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_stEnergySphereGoalList[m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_iCurEnergySphereGoalPointIndex].cResult;
		dX = m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_stEnergySphereGoalList[m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_iCurEnergySphereGoalPointIndex].aresdenX;
		dY = m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_stEnergySphereGoalList[m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_iCurEnergySphereGoalPointIndex].aresdenY;
		if ((sX >= dX - 4) && (sX <= dX + 4) && (sY >= dY - 4) && (sY <= dY + 4)) {
			m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_iCurEnergySphereGoalPointIndex = -1;

			if ((cAttackerType == hb::shared::owner_class::Player) && (m_pGame->m_pClientList[sAttackerH] != 0)) {
				if (m_pGame->m_pClientList[sAttackerH]->m_cSide == 1) { // Aresden (Side:1)
					m_pGame->m_pClientList[sAttackerH]->m_iContribution += 5;
					std::snprintf(G_cTxt, sizeof(G_cTxt), "(!) EnergySphere Hit By Aresden Player (%s)", m_pGame->m_pClientList[sAttackerH]->m_cCharName);
					PutLogFileList(G_cTxt);
				}
				else {
					m_pGame->m_pClientList[sAttackerH]->m_iContribution -= 10;
				}

				for(int i = 1; i < MaxClients; i++)
					if ((m_pGame->m_pClientList[i] != 0) && (m_pGame->m_pClientList[i]->m_bIsInitComplete)) {
						m_pGame->SendNotifyMsg(0, i, Notify::EnergySphereGoalIn, cResult, m_pGame->m_pClientList[sAttackerH]->m_cSide, 2, m_pGame->m_pClientList[sAttackerH]->m_cCharName);
					}
			}
			return true;
		}

		dX = m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_stEnergySphereGoalList[m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_iCurEnergySphereGoalPointIndex].elvineX;
		dY = m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_stEnergySphereGoalList[m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_iCurEnergySphereGoalPointIndex].elvineY;
		if ((sX >= dX - 4) && (sX <= dX + 4) && (sY >= dY - 4) && (sY <= dY + 4)) {
			m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_iCurEnergySphereGoalPointIndex = -1;

			if ((cAttackerType == hb::shared::owner_class::Player) && (m_pGame->m_pClientList[sAttackerH] != 0)) {
				if (m_pGame->m_pClientList[sAttackerH]->m_cSide == 2) { // Elvine (Side:2)
					m_pGame->m_pClientList[sAttackerH]->m_iContribution += 5;
					std::snprintf(G_cTxt, sizeof(G_cTxt), "(!) EnergySphere Hit By Aresden Player (%s)", m_pGame->m_pClientList[sAttackerH]->m_cCharName);
					PutLogFileList(G_cTxt);
				}
				else {
					m_pGame->m_pClientList[sAttackerH]->m_iContribution -= 10;
				}

				for(int i = 1; i < MaxClients; i++)
					if ((m_pGame->m_pClientList[i] != 0) && (m_pGame->m_pClientList[i]->m_bIsInitComplete)) {
						m_pGame->SendNotifyMsg(0, i, Notify::EnergySphereGoalIn, cResult, m_pGame->m_pClientList[sAttackerH]->m_cSide, 1, m_pGame->m_pClientList[sAttackerH]->m_cCharName);
					}
			}
			return true;
		}
		return false;
	}
}

void WarManager::GetOccupyFlagHandler(int iClientH)
{
	int   iNum, iRet, iEraseReq, iEKNum;
	char cItemName[hb::shared::limits::ItemNameLen];
	CItem* pItem;

	if (m_pGame->m_pClientList[iClientH] == 0) return;
	if (m_pGame->m_pClientList[iClientH]->m_iEnemyKillCount < 3) return;
	if (m_pGame->m_pClientList[iClientH]->m_cSide == 0) return;

	std::memset(cItemName, 0, sizeof(cItemName));
	switch (m_pGame->m_pClientList[iClientH]->m_cSide) {
	case 1: strcpy(cItemName, "¾Æ·¹½ºµ§±ê¹ß"); break;
	case 2: strcpy(cItemName, "¿¤¹ÙÀÎ±ê¹ß");   break;
	}

	// ReqPurchaseItemHandler   .
	iNum = 1;
	for(int i = 1; i <= iNum; i++) {

		pItem = new CItem;
		if (m_pGame->m_pItemManager->_bInitItemAttr(pItem, cItemName) == false) {
			delete pItem;
		}
		else {

			if (m_pGame->m_pItemManager->_bAddClientItemList(iClientH, pItem, &iEraseReq)) {
				if (m_pGame->m_pClientList[iClientH]->m_iCurWeightLoad < 0) m_pGame->m_pClientList[iClientH]->m_iCurWeightLoad = 0;

				if (m_pGame->m_pClientList[iClientH]->m_iEnemyKillCount > 12) {
					iEKNum = 12;
					m_pGame->m_pClientList[iClientH]->m_iEnemyKillCount -= 12;
				}
				else {
					iEKNum = m_pGame->m_pClientList[iClientH]->m_iEnemyKillCount;
					m_pGame->m_pClientList[iClientH]->m_iEnemyKillCount = 0;
				}

				// EKNum .
				pItem->m_sItemSpecEffectValue1 = iEKNum;

				// testcode  .
				std::snprintf(G_cTxt, sizeof(G_cTxt), "(*) Get Flag : Char(%s) Flag-EK(%d) Player-EK(%d)", m_pGame->m_pClientList[iClientH]->m_cCharName, iEKNum, m_pGame->m_pClientList[iClientH]->m_iEnemyKillCount);
				PutLogFileList(G_cTxt);

				iRet = m_pGame->m_pItemManager->SendItemNotifyMsg(iClientH, Notify::ItemObtained, pItem, 0);

				m_pGame->iCalcTotalWeight(iClientH);

				switch (iRet) {
				case sock::Event::QueueFull:
				case sock::Event::SocketError:
				case sock::Event::CriticalError:
				case sock::Event::SocketClosed:
					m_pGame->DeleteClient(iClientH, true, true);
					return;
				}

				m_pGame->SendNotifyMsg(0, iClientH, Notify::EnemyKills, m_pGame->m_pClientList[iClientH]->m_iEnemyKillCount, 0, 0, 0);
			}
			else
			{
				delete pItem;

				m_pGame->iCalcTotalWeight(iClientH);

				iRet = m_pGame->m_pItemManager->SendItemNotifyMsg(iClientH, Notify::CannotCarryMoreItem, 0, 0);


				switch (iRet) {
				case sock::Event::QueueFull:
				case sock::Event::SocketError:
				case sock::Event::CriticalError:
				case sock::Event::SocketClosed:
					m_pGame->DeleteClient(iClientH, true, true);
					return;
				}
			}
		}
	}
}

size_t WarManager::_iComposeFlagStatusContents(char* pData)
{
	SYSTEMTIME SysTime;
	char cTxt[120];
	

	if (m_pGame->m_iMiddlelandMapIndex < 0) return 0;

	GetLocalTime(&SysTime);
	strcat(pData, "[FILE-DATE]\n\n");

	std::snprintf(cTxt, sizeof(cTxt), "file-saved-date: %d %d %d %d %d\n", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute);
	strcat(pData, cTxt);
	strcat(pData, "\n\n");

	for(int i = 1; i < smap::MaxOccupyFlag; i++)
		if (m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_pOccupyFlag[i] != 0) {

			std::snprintf(cTxt, sizeof(cTxt), "flag = %d %d %d %d", m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_pOccupyFlag[i]->m_cSide,
				m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_pOccupyFlag[i]->m_sX,
				m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_pOccupyFlag[i]->m_sY,
				m_pGame->m_pMapList[m_pGame->m_iMiddlelandMapIndex]->m_pOccupyFlag[i]->m_iEKCount);
			strcat(pData, cTxt);
			strcat(pData, "\n");
		}

	strcat(pData, "\n\n");

	return strlen(pData);
}

void WarManager::SetSummonMobAction(int iClientH, int iMode, size_t dwMsgSize, char* pData)
{
	int iTargetIndex;
	char   seps[] = "= \t\r\n";
	char* token, cTargetName[11], cBuff[256];

	if (m_pGame->m_pClientList[iClientH] == 0) return;
	if (m_pGame->m_pClientList[iClientH]->m_cSide == 0) return;

	switch (iMode) {
	case 0: // Free
	case 1: // Hold
		// iClientH   .
		for(int i = 0; i < MaxNpcs; i++)
			if (m_pGame->m_pNpcList[i] != 0) {
				if ((m_pGame->m_pNpcList[i]->m_bIsSummoned) &&
					(m_pGame->m_pNpcList[i]->m_iFollowOwnerIndex == iClientH) &&
					(m_pGame->m_pNpcList[i]->m_cFollowOwnerType == hb::shared::owner_class::Player)) {

					m_pGame->m_pNpcList[i]->m_iSummonControlMode = iMode;
					m_pGame->m_pNpcList[i]->m_bIsPermAttackMode = false;
					m_pGame->m_pNpcList[i]->m_cBehavior = Behavior::Move;
					m_pGame->m_pNpcList[i]->m_sBehaviorTurnCount = 0;
					m_pGame->m_pNpcList[i]->m_iTargetIndex = 0;
				}
			}
		break;

	case 2:
		if ((dwMsgSize) <= 0) return;
		memcpy(cBuff, pData, dwMsgSize);

		token = strtok(NULL, seps);
		token = strtok(NULL, seps);

		iTargetIndex = 0;
		if (token != 0) {
			// token
			if (strlen(token) > hb::shared::limits::CharNameLen - 1)
				memcpy(cTargetName, token, hb::shared::limits::CharNameLen - 1);
			else memcpy(cTargetName, token, strlen(token));

			// 2002.8.17
			for(int i = 1; i < MaxClients; i++)
			{
				// if ((m_pGame->m_pClientList[i] != 0) && (memcmp(m_pGame->m_pClientList[i]->m_cCharName, cTargetName, 10) == 0)) { // original
				if ((m_pGame->m_pClientList[i] != 0) &&
					(_strnicmp(m_pGame->m_pClientList[i]->m_cCharName, cTargetName, hb::shared::limits::CharNameLen - 1) == 0) &&
					(strcmp(m_pGame->m_pClientList[iClientH]->m_cMapName, m_pGame->m_pClientList[i]->m_cMapName) == 0)) // adamas(map  .)
				{
					iTargetIndex = i;
					goto SSMA_SKIPSEARCH;
				}
			}
		}

	SSMA_SKIPSEARCH:

		if ((iTargetIndex != 0) && (m_pGame->m_pClientList[iTargetIndex]->m_cSide != 0) &&
			(m_pGame->m_pClientList[iTargetIndex]->m_cSide != m_pGame->m_pClientList[iClientH]->m_cSide)) {
			for(int i = 0; i < MaxNpcs; i++)
				if (m_pGame->m_pNpcList[i] != 0) {
					if ((m_pGame->m_pNpcList[i]->m_bIsSummoned) &&
						(m_pGame->m_pNpcList[i]->m_iFollowOwnerIndex == iClientH) &&
						(m_pGame->m_pNpcList[i]->m_cFollowOwnerType == hb::shared::owner_class::Player)) {

						m_pGame->m_pNpcList[i]->m_iSummonControlMode = iMode;
						m_pGame->m_pNpcList[i]->m_cBehavior = Behavior::Attack;
						m_pGame->m_pNpcList[i]->m_sBehaviorTurnCount = 0;
						m_pGame->m_pNpcList[i]->m_iTargetIndex = iTargetIndex;
						m_pGame->m_pNpcList[i]->m_cTargetType = hb::shared::owner_class::Player;
						m_pGame->m_pNpcList[i]->m_bIsPermAttackMode = true;
					}
				}
		}
		break;
	}
}

bool WarManager::__bSetOccupyFlag(char cMapIndex, int dX, int dY, int iSide, int iEKNum, int iClientH)
{
	int   iDynamicObjectIndex, iIndex;
	class CTile* pTile;
	uint32_t dwTime;

	dwTime = GameClock::GetTimeMS();

	if (m_pGame->m_pMapList[cMapIndex] == 0) return false;
	if (((m_pGame->m_bIsHeldenianMode == false) || (static_cast<char>(m_pGame->m_bIsHeldenianMode) != m_pGame->m_cHeldenianType)) &&
		(m_pGame->m_bHeldenianInitiated == 1)) return false;
	if ((m_pGame->m_cHeldenianType == 1) && (m_pGame->m_iBTFieldMapIndex == -1)) return false;
	if ((m_pGame->m_cHeldenianType == 2) && (m_pGame->m_iGodHMapIndex == -1)) return false;
	if ((m_pGame->m_pClientList[iClientH]->m_iGuildRank == 0)) return false;

	pTile = (class CTile*)(m_pGame->m_pMapList[cMapIndex]->m_pTile + dX + dY * m_pGame->m_pMapList[cMapIndex]->m_sSizeY);
	if (pTile->m_iAttribute != 0) return false;
	iSide = m_pGame->m_sLastHeldenianWinner;
	if ((dX < 25) || (dX >= m_pGame->m_pMapList[cMapIndex]->m_sSizeX - 25) ||
		(dY < 25) || (dY >= m_pGame->m_pMapList[cMapIndex]->m_sSizeY - 25)) return false;

	if ((iClientH > 0) && (m_pGame->m_pClientList[iClientH] != 0)) {
		if (m_pGame->m_pClientList[iClientH]->m_cSide != iSide) return false;
	}

	pTile = (class CTile*)(m_pGame->m_pMapList[cMapIndex]->m_pTile + dX + dY * m_pGame->m_pMapList[cMapIndex]->m_sSizeY);
	if (pTile->m_iOccupyFlagIndex != 0) return false;
	if (pTile->m_bIsMoveAllowed == false)  return false;

	for(int ix = dX - 3; ix <= dX + 3; ix++)
		for(int iy = dY - 3; iy <= dY + 3; iy++) {
			if ((ix == dX) && (iy == dY)) {

			}
			else {
				pTile = (class CTile*)(m_pGame->m_pMapList[cMapIndex]->m_pTile + ix + iy * m_pGame->m_pMapList[cMapIndex]->m_sSizeY);
				if ((pTile->m_iOccupyFlagIndex != 0) && (pTile->m_iOccupyFlagIndex > 0) &&
					(pTile->m_iOccupyFlagIndex < smap::MaxOccupyFlag) && (m_pGame->m_pMapList[cMapIndex]->m_pOccupyFlag[pTile->m_iOccupyFlagIndex] != 0)) {
					if (m_pGame->m_pMapList[cMapIndex]->m_pOccupyFlag[pTile->m_iOccupyFlagIndex]->m_cSide == iSide) return false;
				}
			}
		}

	if (m_pGame->m_pMapList[cMapIndex]->m_iTotalOccupyFlags >= smap::MaxOccupyFlag) {
		return false;
	}

	switch (iSide) {
	case 1:	iDynamicObjectIndex = m_pGame->m_pDynamicObjectManager->iAddDynamicObjectList(0, 0, dynamic_object::AresdenFlag1, cMapIndex, dX, dY, 0, 0);	break;
	case 2:	iDynamicObjectIndex = m_pGame->m_pDynamicObjectManager->iAddDynamicObjectList(0, 0, dynamic_object::ElvineFlag1, cMapIndex, dX, dY, 0, 0);	break;
	default: iDynamicObjectIndex = 0;
	}

	iEKNum = 1;
	iIndex = m_pGame->m_pMapList[cMapIndex]->iRegisterOccupyFlag(dX, dY, iSide, iEKNum, iDynamicObjectIndex);
	if (iIndex < 0) {
		if (iDynamicObjectIndex > MaxGuilds)
			return true;
	}

	pTile = (class CTile*)(m_pGame->m_pMapList[cMapIndex]->m_pTile + dX + dY * m_pGame->m_pMapList[cMapIndex]->m_sSizeY);
	pTile->m_iOccupyFlagIndex = iIndex;

	m_pGame->m_pMapList[cMapIndex]->m_iTotalOccupyFlags++;

	if (m_pGame->m_cHeldenianType == 1) {
		for(int ix = dX - 3; ix <= dX + 3; ix++)
			for(int iy = dY - 3; iy <= dY + 3; iy++) {
				if ((ix < 0) || (ix >= m_pGame->m_pMapList[cMapIndex]->m_sSizeX) ||
					(iy < 0) || (iy >= m_pGame->m_pMapList[cMapIndex]->m_sSizeY)) {
				}
				else {
					pTile = (class CTile*)(m_pGame->m_pMapList[cMapIndex]->m_pTile + ix + iy * m_pGame->m_pMapList[cMapIndex]->m_sSizeY);
					switch (iSide) {
					case 1:
						pTile->m_iOccupyStatus -= iEKNum;
						break;
					case 2:
						pTile->m_iOccupyStatus += iEKNum;
						break;
					}
				}
			}
	}

	if (m_pGame->m_cHeldenianType == 2) {
		if (iSide == m_pGame->m_sLastHeldenianWinner) {
			m_pGame->m_cHeldenianVictoryType = iSide;
			//sub_4AB9D0
		}
	}
	return true;
}

void WarManager::FightzoneReserveHandler(int iClientH, char* pData, size_t dwMsgSize)
{
	int iFightzoneNum, iEnableReserveTime;
	uint32_t dwGoldCount;
	uint16_t wResult;
	int     iRet, iResult = 1, iCannotReserveDay;
	SYSTEMTIME SysTime;

	if (m_pGame->m_pClientList[iClientH] == 0) return;
	if (m_pGame->m_pClientList[iClientH]->m_bIsInitComplete == false) return;

	GetLocalTime(&SysTime);

	iEnableReserveTime = 2 * 20 * 60 - ((SysTime.wHour % 2) * 20 * 60 + SysTime.wMinute * 20) - 5 * 20;

	dwGoldCount = m_pGame->m_pItemManager->dwGetItemCountByID(iClientH, hb::shared::item::ItemId::Gold);

	const auto* pkt = hb::net::PacketCast<hb::net::PacketRequestFightzoneReserve>(
		pData, sizeof(hb::net::PacketRequestFightzoneReserve));
	if (!pkt) return;
	iFightzoneNum = pkt->fightzone;

	// fightzone  .
	if ((iFightzoneNum < 1) || (iFightzoneNum > MaxFightZone)) return;

	// 2 4 6 8  1 3 5 7
	// ex) 1 => {1 + 1 () + 1 (  )} %2 == 1

	iCannotReserveDay = (SysTime.wDay + m_pGame->m_pClientList[iClientH]->m_cSide + iFightzoneNum) % 2;
	if (iEnableReserveTime <= 0) {
		wResult = MsgType::Reject;
		iResult = 0;
	}
	else if (m_pGame->m_iFightZoneReserve[iFightzoneNum - 1] != 0) {
		wResult = MsgType::Reject;
		iResult = -1;
	}
	else if (dwGoldCount < 1500) {
		// Gold    .
		wResult = MsgType::Reject;
		iResult = -2;
	}
	else if (iCannotReserveDay) {
		wResult = MsgType::Reject;
		iResult = -3;
	}
	else if (m_pGame->m_pClientList[iClientH]->m_iFightzoneNumber != 0) {
		wResult = MsgType::Reject;
		iResult = -4;
	}
	else {

		wResult = MsgType::Confirm;

		m_pGame->m_pItemManager->SetItemCountByID(iClientH, hb::shared::item::ItemId::Gold, dwGoldCount - 1500);
		m_pGame->iCalcTotalWeight(iClientH);

		m_pGame->m_iFightZoneReserve[iFightzoneNum - 1] = iClientH;

		m_pGame->m_pClientList[iClientH]->m_iFightzoneNumber = iFightzoneNum;
		m_pGame->m_pClientList[iClientH]->m_iReserveTime = SysTime.wMonth * 10000 + SysTime.wDay * 100 + SysTime.wHour;

		if (SysTime.wHour % 2)	m_pGame->m_pClientList[iClientH]->m_iReserveTime += 1;
		else					m_pGame->m_pClientList[iClientH]->m_iReserveTime += 2;
		std::snprintf(G_cTxt, sizeof(G_cTxt), "(*) Reserve FIGHTZONETICKET : Char(%s) TICKENUMBER (%d)", m_pGame->m_pClientList[iClientH]->m_cCharName, m_pGame->m_pClientList[iClientH]->m_iReserveTime);
		PutLogFileList(G_cTxt);
		PutLogList(G_cTxt);

		m_pGame->m_pClientList[iClientH]->m_iFightZoneTicketNumber = 50;
		iResult = 1;
	}

	hb::net::PacketResponseFightzoneReserve resp{};
	resp.header.msg_id = MsgId::ResponseFightZoneReserve;
	resp.header.msg_type = wResult;
	resp.result = iResult;


	iRet = m_pGame->m_pClientList[iClientH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&resp), sizeof(resp));

	switch (iRet) {
	case sock::Event::QueueFull:
	case sock::Event::SocketError:
	case sock::Event::CriticalError:
	case sock::Event::SocketClosed:
		m_pGame->DeleteClient(iClientH, true, true);
		return;
	}
}

void WarManager::FightzoneReserveProcessor()
{
}

void WarManager::GetFightzoneTicketHandler(int iClientH)
{
	int   iRet, iEraseReq, iMonth, iDay, iHour;
	char cItemName[hb::shared::limits::ItemNameLen];
	CItem* pItem;

	if (m_pGame->m_pClientList[iClientH] == 0) return;

	if (m_pGame->m_pClientList[iClientH]->m_iFightZoneTicketNumber <= 0) {
		m_pGame->m_pClientList[iClientH]->m_iFightzoneNumber *= -1;
		m_pGame->SendNotifyMsg(0, iClientH, Notify::FightZoneReserve, -1, 0, 0, 0);
		return;
	}

	std::memset(cItemName, 0, sizeof(cItemName));

	if (m_pGame->m_pClientList[iClientH]->m_iFightzoneNumber == 1)
		strcpy(cItemName, "ArenaTicket");
	else  std::snprintf(cItemName, sizeof(cItemName), "ArenaTicket(%d)", m_pGame->m_pClientList[iClientH]->m_iFightzoneNumber);

	pItem = new CItem;
	if (m_pGame->m_pItemManager->_bInitItemAttr(pItem, cItemName) == false) {
		delete pItem;
		return;
	}

	if (m_pGame->m_pItemManager->_bAddClientItemList(iClientH, pItem, &iEraseReq)) {
		if (m_pGame->m_pClientList[iClientH]->m_iCurWeightLoad < 0) m_pGame->m_pClientList[iClientH]->m_iCurWeightLoad = 0;

		m_pGame->m_pClientList[iClientH]->m_iFightZoneTicketNumber = m_pGame->m_pClientList[iClientH]->m_iFightZoneTicketNumber - 1;

		pItem->SetTouchEffectType(TouchEffectType::Date);

		iMonth = m_pGame->m_pClientList[iClientH]->m_iReserveTime / 10000;
		iDay = (m_pGame->m_pClientList[iClientH]->m_iReserveTime - iMonth * 10000) / 100;
		iHour = m_pGame->m_pClientList[iClientH]->m_iReserveTime - iMonth * 10000 - iDay * 100;

		pItem->m_sTouchEffectValue1 = iMonth;
		pItem->m_sTouchEffectValue2 = iDay;
		pItem->m_sTouchEffectValue3 = iHour;


		std::snprintf(G_cTxt, sizeof(G_cTxt), "(*) Get FIGHTZONETICKET : Char(%s) TICKENUMBER (%d)(%d)(%d)", m_pGame->m_pClientList[iClientH]->m_cCharName, pItem->m_sTouchEffectValue1, pItem->m_sTouchEffectValue2, pItem->m_sTouchEffectValue3);
		PutLogFileList(G_cTxt);
		PutLogList(G_cTxt);


		iRet = m_pGame->m_pItemManager->SendItemNotifyMsg(iClientH, Notify::ItemObtained, pItem, 0);

		m_pGame->iCalcTotalWeight(iClientH);

		switch (iRet) {
		case sock::Event::QueueFull:
		case sock::Event::SocketError:
		case sock::Event::CriticalError:
		case sock::Event::SocketClosed:
			m_pGame->DeleteClient(iClientH, true, true);
			return;
		}
	}
	else {
		delete pItem;

		m_pGame->iCalcTotalWeight(iClientH);

		iRet = m_pGame->m_pItemManager->SendItemNotifyMsg(iClientH, Notify::CannotCarryMoreItem, 0, 0);


		switch (iRet) {
		case sock::Event::QueueFull:
		case sock::Event::SocketError:
		case sock::Event::CriticalError:
		case sock::Event::SocketClosed:
			m_pGame->DeleteClient(iClientH, true, true);
			return;
		}
	}
}
