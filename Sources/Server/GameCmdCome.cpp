#include "Platform.h"
#include "GameCmdCome.h"
#include "Game.h"
#include "AccountSqliteStore.h"
#include <cstring>
#include <cstdio>

using namespace hb::shared::net;
bool GameCmdCome::Execute(CGame* pGame, int iClientH, const char* pArgs)
{
	if (pGame->m_pClientList[iClientH] == nullptr)
		return true;

	if (pArgs == nullptr || pArgs[0] == '\0')
	{
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Usage: /come <playername>");
		return true;
	}

	const char* gmMap = pGame->m_pClientList[iClientH]->m_cMapName;
	short gmX = pGame->m_pClientList[iClientH]->m_sX;
	short gmY = pGame->m_pClientList[iClientH]->m_sY;

	// Try online first
	int iTargetH = pGame->FindClientByName(pArgs);
	if (iTargetH != 0 && pGame->m_pClientList[iTargetH] != nullptr)
	{
		if (pGame->GMTeleportTo(iTargetH, gmMap, gmX, gmY))
		{
			pGame->SendNotifyMsg(0, iTargetH, Notify::NoticeMsg, 0, 0, 0, "You have been summoned by a GM.");

			char buf[64];
			std::snprintf(buf, sizeof(buf), "Summoned %s to your location.", pGame->m_pClientList[iTargetH]->m_cCharName);
			pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, buf);
		}
		else
		{
			pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Summon failed.");
		}
		return true;
	}

	// Offline path: update saved position in DB
	char cAccountName[12];
	std::memset(cAccountName, 0, sizeof(cAccountName));
	if (!ResolveCharacterToAccount(pArgs, cAccountName, sizeof(cAccountName)))
	{
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Player not found.");
		return true;
	}

	sqlite3* db = nullptr;
	std::string dbPath;
	if (!EnsureAccountDatabase(cAccountName, &db, dbPath))
	{
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Failed to open account database.");
		return true;
	}

	AccountDbCharacterState state{};
	bool bLoaded = LoadCharacterState(db, pArgs, state);
	if (!bLoaded)
	{
		CloseAccountDatabase(db);
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Failed to load character data.");
		return true;
	}

	// Update position to GM's location
	std::memset(state.mapName, 0, sizeof(state.mapName));
	std::memcpy(state.mapName, gmMap, 10);
	state.mapX = gmX;
	state.mapY = gmY;

	InsertCharacterState(db, state);
	CloseAccountDatabase(db);

	char buf[80];
	std::snprintf(buf, sizeof(buf), "Updated %s's saved location (takes effect on next login).", state.characterName);
	pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, buf);

	return true;
}
