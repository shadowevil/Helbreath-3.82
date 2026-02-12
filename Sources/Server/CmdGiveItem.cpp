#include "Platform.h"
#include "CmdGiveItem.h"
#include "Game.h"
#include "ItemManager.h"
#include "Item.h"
#include "Item/ItemEnums.h"
#include "winmain.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
using namespace hb::server::config;

void CmdGiveItem::Execute(CGame* pGame, const char* pArgs)
{
	if (pArgs == nullptr || pArgs[0] == '\0')
	{
		PutLogList((char*)"Usage: giveitem <playername> <item_id> <amount>");
		return;
	}

	// Parse player name
	char cPlayerName[32];
	std::memset(cPlayerName, 0, sizeof(cPlayerName));
	const char* p = pArgs;
	int i = 0;
	while (*p != '\0' && *p != ' ' && *p != '\t' && i < (int)(sizeof(cPlayerName) - 1))
	{
		cPlayerName[i++] = *p++;
	}
	cPlayerName[i] = '\0';

	// Skip whitespace
	while (*p == ' ' || *p == '\t')
		p++;

	if (*p == '\0')
	{
		PutLogList((char*)"Usage: giveitem <playername> <item_id> <amount>");
		return;
	}

	// Parse item ID
	int iItemID = std::atoi(p);
	while (*p != '\0' && *p != ' ' && *p != '\t')
		p++;

	// Skip whitespace
	while (*p == ' ' || *p == '\t')
		p++;

	if (*p == '\0')
	{
		PutLogList((char*)"Usage: giveitem <playername> <item_id> <amount>");
		return;
	}

	// Parse amount
	int iAmount = std::atoi(p);

	// Find player by name
	int iClientH = 0;
	for(int j = 1; j < MaxClients; j++)
	{
		if (pGame->m_pClientList[j] != nullptr &&
			_stricmp(pGame->m_pClientList[j]->m_cCharName, cPlayerName) == 0)
		{
			iClientH = j;
			break;
		}
	}

	if (iClientH == 0)
	{
		char buf[256];
		std::snprintf(buf, sizeof(buf), "Player '%s' not found.", cPlayerName);
		PutLogList(buf);
		return;
	}

	// Validate item ID
	if (iItemID < 0 || iItemID >= MaxItemTypes || pGame->m_pItemConfigList[iItemID] == nullptr)
	{
		char buf[256];
		std::snprintf(buf, sizeof(buf), "Invalid item ID: %d.", iItemID);
		PutLogList(buf);
		return;
	}

	if (iAmount < 1) iAmount = 1;
	if (iAmount > 1000) iAmount = 1000;

	const char* pItemName = pGame->m_pItemConfigList[iItemID]->m_cName;
	auto itemType = pGame->m_pItemConfigList[iItemID]->GetItemType();
	bool bTrueStack = hb::shared::item::IsTrueStackType(itemType) || (iItemID == hb::shared::item::ItemId::Gold);

	int iCreated = 0;

	if (bTrueStack)
	{
		// True stacks: single item with count = amount (arrows, materials, gold)
		CItem* pItem = new CItem;
		if (pGame->m_pItemManager->_bInitItemAttr(pItem, iItemID) == false)
		{
			delete pItem;
			char buf[256];
			std::snprintf(buf, sizeof(buf), "Failed to initialize item ID: %d.", iItemID);
			PutLogList(buf);
			return;
		}
		pItem->m_dwCount = iAmount;

		if (pGame->m_pItemManager->bAddItem(iClientH, pItem, 0) == false)
		{
			PutLogList((char*)"Failed to give item: player inventory full.");
			return;
		}
		iCreated = iAmount;
	}
	else
	{
		// Soft-linked items: individual items, one bulk notification
		iCreated = pGame->m_pItemManager->_bAddClientBulkItemList(iClientH, pItemName, iAmount);
	}

	// Success
	char buf[256];
	std::snprintf(buf, sizeof(buf), "Gave %dx %s (ID: %d) to %s.", iCreated, pItemName, iItemID, cPlayerName);
	PutLogList(buf);
}
