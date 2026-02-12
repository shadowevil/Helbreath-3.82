#include "Game.h"
#include "NetworkMessageManager.h"
#include "Packet/SharedPackets.h"
#include "lan_eng.h"
#include <cstdio>
#include <cstring>
#include "PlatformCompat.h"

namespace NetworkMessageHandlers {
	void HandleAgricultureNoArea(CGame* pGame, char* pData)
	{
		pGame->AddEventList(DEF_MSG_NOTIFY_AGRICULTURENOAREA, 10);
	}

	void HandleAgricultureSkillLimit(CGame* pGame, char* pData)
	{
		pGame->AddEventList(DEF_MSG_NOTIFY_AGRICULTURESKILLLIMIT, 10);
	}

	void HandleNoMoreAgriculture(CGame* pGame, char* pData)
	{
		pGame->AddEventList(DEF_MSG_NOTIFY_NOMOREAGRICULTURE, 10);
	}
} // namespace NetworkMessageHandlers
