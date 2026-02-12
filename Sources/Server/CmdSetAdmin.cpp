#include "Platform.h"
#include "CmdSetAdmin.h"
#include "Game.h"
#include "GameConfigSqliteStore.h"
#include "AccountSqliteStore.h"
#include "AdminLevel.h"
#include "winmain.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>
using namespace hb::server::config;

void CmdSetAdmin::Execute(CGame* pGame, const char* pArgs)
{
	if (pArgs == nullptr || pArgs[0] == '\0')
	{
		PutLogList("Usage: setadmin <charname> [resetip]");
		return;
	}

	// Parse character name (first word)
	char cCharName[hb::shared::limits::CharNameLen] = {};
	const char* p = pArgs;
	int ci = 0;
	while (*p != '\0' && *p != ' ' && *p != '\t' && ci < 10)
	{
		cCharName[ci++] = *p++;
	}

	// Skip whitespace to check for subcommand
	while (*p == ' ' || *p == '\t')
		p++;

	// Check for "resetip" subcommand
	if (_stricmp(p, "resetip") == 0)
	{
		int idx = pGame->FindAdminByCharName(cCharName);
		if (idx == -1)
		{
			char buf[128];
			std::snprintf(buf, sizeof(buf), "(!) Admin entry not found for character: %s", cCharName);
			PutLogList(buf);
			return;
		}

		strcpy(pGame->m_stAdminList[idx].m_cApprovedIP, "0.0.0.0");

		sqlite3* configDb = nullptr;
		std::string dbPath;
		if (EnsureGameConfigDatabase(&configDb, dbPath, nullptr))
		{
			SaveAdminConfig(configDb, pGame);
			CloseGameConfigDatabase(configDb);
		}

		char buf[128];
		std::snprintf(buf, sizeof(buf), "(!) Admin IP reset for character: %s", cCharName);
		PutLogList(buf);
		return;
	}

	// Parse optional admin level (default: GameMaster = 1)
	int iAdminLevel = hb::shared::admin::GameMaster;
	if (*p != '\0')
	{
		iAdminLevel = std::atoi(p);
		if (iAdminLevel < 1)
		{
			PutLogList("(!) Admin level must be >= 1. Use level 0 is Player (non-admin).");
			return;
		}
	}

	// Adding a new admin - find the account name
	char cAccountName[11] = {};
	bool bFound = false;

	// First check online clients
	for (int i = 1; i < MaxClients; i++)
	{
		if (pGame->m_pClientList[i] != nullptr &&
			_stricmp(pGame->m_pClientList[i]->m_cCharName, cCharName) == 0)
		{
			strncpy(cAccountName, pGame->m_pClientList[i]->m_cAccountName, 10);
			cAccountName[10] = '\0';
			bFound = true;
			break;
		}
	}

	// If not found online, search account DB files
	if (!bFound)
	{
		if (ResolveCharacterToAccount(cCharName, cAccountName, sizeof(cAccountName)))
		{
			bFound = true;
		}
	}

	if (!bFound)
	{
		char buf[128];
		std::snprintf(buf, sizeof(buf), "(!) Player not found: %s", cCharName);
		PutLogList(buf);
		return;
	}

	// Check if already an admin - if so, update their level
	int existingAdminIdx = pGame->FindAdminByAccount(cAccountName);
	if (existingAdminIdx != -1)
	{
		pGame->m_stAdminList[existingAdminIdx].m_iAdminLevel = iAdminLevel;

		// Update online player's level
		for (int i = 1; i < MaxClients; i++)
		{
			if (pGame->m_pClientList[i] != nullptr &&
				_stricmp(pGame->m_pClientList[i]->m_cAccountName, cAccountName) == 0)
			{
				pGame->m_pClientList[i]->m_iAdminLevel = iAdminLevel;
				break;
			}
		}

		sqlite3* configDb = nullptr;
		std::string dbPath;
		if (EnsureGameConfigDatabase(&configDb, dbPath, nullptr))
		{
			SaveAdminConfig(configDb, pGame);
			CloseGameConfigDatabase(configDb);
		}

		char buf[128];
		std::snprintf(buf, sizeof(buf), "(!) Admin level updated: %s (account: %s) -> level %d",
			cCharName, cAccountName, iAdminLevel);
		PutLogList(buf);
		return;
	}

	// Check capacity
	if (pGame->m_iAdminCount >= MaxAdmins)
	{
		PutLogList("(!) Admin list is full!");
		return;
	}

	// Add new admin entry
	int idx = pGame->m_iAdminCount;
	std::memset(&pGame->m_stAdminList[idx], 0, sizeof(AdminEntry));
	strncpy(pGame->m_stAdminList[idx].m_cAccountName, cAccountName, 10);
	strncpy(pGame->m_stAdminList[idx].m_cCharName, cCharName, 10);
	strcpy(pGame->m_stAdminList[idx].m_cApprovedIP, "0.0.0.0");
	pGame->m_stAdminList[idx].m_iAdminLevel = iAdminLevel;
	pGame->m_iAdminCount++;

	// If the player is currently online, set their admin index and level
	for (int i = 1; i < MaxClients; i++)
	{
		if (pGame->m_pClientList[i] != nullptr &&
			_stricmp(pGame->m_pClientList[i]->m_cAccountName, cAccountName) == 0 &&
			_stricmp(pGame->m_pClientList[i]->m_cCharName, cCharName) == 0)
		{
			pGame->m_pClientList[i]->m_iAdminIndex = idx;
			pGame->m_pClientList[i]->m_iAdminLevel = iAdminLevel;
			break;
		}
	}

	// Save to DB
	sqlite3* configDb = nullptr;
	std::string dbPath;
	if (EnsureGameConfigDatabase(&configDb, dbPath, nullptr))
	{
		SaveAdminConfig(configDb, pGame);
		CloseGameConfigDatabase(configDb);
	}

	char buf[128];
	std::snprintf(buf, sizeof(buf), "(!) Admin added: %s (account: %s, level: %d)", cCharName, cAccountName, iAdminLevel);
	PutLogList(buf);
}
