#include "Platform.h"
#include "GameCmdUnblock.h"
#include "Game.h"
#include <cstring>
#include <algorithm>

using namespace hb::shared::net;
bool GameCmdUnblock::Execute(CGame* pGame, int iClientH, const char* pArgs)
{
	if (pGame->m_pClientList[iClientH] == nullptr)
		return true;

	if (pArgs == nullptr || pArgs[0] == '\0')
	{
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Usage: /unblock CharName");
		return true;
	}

	// Extract character name (max 10 chars, first word only)
	char cCharName[hb::shared::limits::CharNameLen] = {};
	size_t nameLen = std::strlen(pArgs);
	if (nameLen > 10) nameLen = 10;
	for (size_t i = 0; i < nameLen; i++)
	{
		if (pArgs[i] == ' ' || pArgs[i] == '\t')
			break;
		cCharName[i] = pArgs[i];
	}

	if (cCharName[0] == '\0')
		return true;

	// Find entry in block list by character name
	auto& blockList = pGame->m_pClientList[iClientH]->m_BlockedAccountsList;
	auto it = std::find_if(blockList.begin(), blockList.end(),
		[&](const std::pair<std::string, std::string>& entry) {
			return _stricmp(entry.second.c_str(), cCharName) == 0;
		});

	if (it == blockList.end())
	{
		char msg[64] = {};
		std::snprintf(msg, sizeof(msg), "'%s' is not in your block list.", cCharName);
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, msg);
		return true;
	}

	// Remove from set and list
	pGame->m_pClientList[iClientH]->m_BlockedAccounts.erase(it->first);
	blockList.erase(it);
	pGame->m_pClientList[iClientH]->m_bBlockListDirty = true;

	char msg[64] = {};
	std::snprintf(msg, sizeof(msg), "'%s' has been unblocked.", cCharName);
	pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, msg);

	return true;
}
