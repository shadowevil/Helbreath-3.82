#include "Platform.h"
#include "GameCmdGoto.h"
#include "Game.h"
#include "AccountSqliteStore.h"
#include <cstring>
#include <cstdio>

using namespace hb::shared::net;
bool GameCmdGoto::Execute(CGame* pGame, int iClientH, const char* pArgs)
{
	if (pGame->m_pClientList[iClientH] == nullptr)
		return true;

	if (pArgs == nullptr || pArgs[0] == '\0')
	{
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Usage: /goto <playername>");
		return true;
	}

	// Try online first
	int iTargetH = pGame->FindClientByName(pArgs);
	if (iTargetH != 0 && pGame->m_pClientList[iTargetH] != nullptr)
	{
		if (pGame->GMTeleportTo(iClientH, pGame->m_pClientList[iTargetH]->m_cMapName,
			pGame->m_pClientList[iTargetH]->m_sX, pGame->m_pClientList[iTargetH]->m_sY))
		{
			char buf[64];
			std::snprintf(buf, sizeof(buf), "Teleported to %s.", pGame->m_pClientList[iTargetH]->m_cCharName);
			pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, buf);
		}
		else
		{
			pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Teleport failed (invalid map).");
		}
		return true;
	}

	// Offline path: look up last-logout position from DB
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
	CloseAccountDatabase(db);

	if (!bLoaded)
	{
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Failed to load character data.");
		return true;
	}

	if (pGame->GMTeleportTo(iClientH, state.mapName, static_cast<short>(state.mapX), static_cast<short>(state.mapY)))
	{
		char buf[64];
		std::snprintf(buf, sizeof(buf), "Teleported to %s's last position (offline).", state.characterName);
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, buf);
	}
	else
	{
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Teleport failed (invalid map).");
	}

	return true;
}
