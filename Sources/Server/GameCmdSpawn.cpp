#include "Platform.h"
#include "GameCmdSpawn.h"
#include "Game.h"
#include "Npc.h"
#include <cstring>
#include <cstdio>

using namespace hb::shared::net;
using namespace hb::server::config;
bool GameCmdSpawn::Execute(CGame* pGame, int iClientH, const char* pArgs)
{
	if (pGame->m_pClientList[iClientH] == nullptr)
		return true;

	int iNpcID = 0, iAmount = 1;
	if (pArgs == nullptr || pArgs[0] == '\0' || sscanf(pArgs, "%d %d", &iNpcID, &iAmount) < 1)
	{
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Usage: /spawn <npc_id> [amount]");
		return true;
	}

	if (iNpcID < 0 || iNpcID >= MaxNpcTypes || pGame->m_pNpcConfigList[iNpcID] == nullptr)
	{
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Invalid NPC ID.");
		return true;
	}

	if (iAmount < 1) iAmount = 1;
	if (iAmount > 50) iAmount = 50;

	char* pMapName = pGame->m_pMapList[pGame->m_pClientList[iClientH]->m_cMapIndex]->m_cName;
	int iSpawned = 0;

	for (int i = 0; i < iAmount; i++)
	{
		char cUniqueName[21];
		std::snprintf(cUniqueName, sizeof(cUniqueName), "GM-spawn%d", i);

		int tX = pGame->m_pClientList[iClientH]->m_sX;
		int tY = pGame->m_pClientList[iClientH]->m_sY;

		// bIsSummoned=false so NPCs give EXP/drops, bBypassMobLimit=true so they don't count toward map limit
		if (pGame->bCreateNewNpc(iNpcID, cUniqueName, pMapName, 0, 0, hb::server::npc::MoveType::Random,
			&tX, &tY, nullptr, nullptr, 0, -1, false, false, false, false, 0, true))
		{
			iSpawned++;
		}
	}

	char buf[128];
	std::snprintf(buf, sizeof(buf), "Spawned %d x %s (ID: %d).", iSpawned, pGame->m_pNpcConfigList[iNpcID]->m_cNpcName, iNpcID);
	pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, buf);

	return true;
}
