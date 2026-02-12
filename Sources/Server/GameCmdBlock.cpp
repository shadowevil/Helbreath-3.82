#include "Platform.h"
#include "GameCmdBlock.h"
#include "Game.h"
#include "AccountSqliteStore.h"
#include <cstring>

using namespace hb::shared::net;
bool GameCmdBlock::Execute(CGame* pGame, int iClientH, const char* pArgs)
{
	if (pGame->m_pClientList[iClientH] == nullptr)
		return true;

	if (pArgs == nullptr || pArgs[0] == '\0')
	{
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Usage: /block CharName");
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

	// Can't block yourself
	if (_stricmp(cCharName, pGame->m_pClientList[iClientH]->m_cCharName) == 0)
	{
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "You cannot block yourself.");
		return true;
	}

	// Resolve character name to account name
	char cAccountName[11] = {};
	if (!ResolveCharacterToAccount(cCharName, cAccountName, sizeof(cAccountName)))
	{
		char msg[64] = {};
		std::snprintf(msg, sizeof(msg), "Character '%s' not found.", cCharName);
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, msg);
		return true;
	}

	// Check if already blocked
	if (pGame->m_pClientList[iClientH]->m_BlockedAccounts.count(cAccountName) > 0)
	{
		char msg[64] = {};
		std::snprintf(msg, sizeof(msg), "'%s' is already blocked.", cCharName);
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, msg);
		return true;
	}

	// Add to block set and list
	pGame->m_pClientList[iClientH]->m_BlockedAccounts.insert(cAccountName);
	pGame->m_pClientList[iClientH]->m_BlockedAccountsList.push_back(
		std::make_pair(std::string(cAccountName), std::string(cCharName)));
	pGame->m_pClientList[iClientH]->m_bBlockListDirty = true;

	char msg[64] = {};
	std::snprintf(msg, sizeof(msg), "'%s' has been blocked.", cCharName);
	pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, msg);

	return true;
}
