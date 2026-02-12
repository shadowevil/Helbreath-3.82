#include "Platform.h"
#include "GameCmdGiveItem.h"
#include "Game.h"
#include "ItemManager.h"
#include "Item.h"
#include "Item/ItemEnums.h"
#include <cstring>
#include <cstdio>

using namespace hb::shared::net;
using namespace hb::server::config;
bool GameCmdGiveItem::Execute(CGame* pGame, int iClientH, const char* pArgs)
{
	if (pGame->m_pClientList[iClientH] == nullptr)
		return true;

	if (pArgs == nullptr || pArgs[0] == '\0')
	{
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Usage: /giveitem <name> <item_id> [amount]");
		return true;
	}

	// Parse name (first word), then item_id and amount
	char cName[12];
	std::memset(cName, 0, sizeof(cName));
	int iItemID = 0, iAmount = 1;

	const char* pCur = pArgs;
	int iNameLen = 0;
	while (*pCur != '\0' && *pCur != ' ' && *pCur != '\t' && iNameLen < 10)
	{
		cName[iNameLen++] = *pCur++;
	}
	while (*pCur == ' ' || *pCur == '\t') pCur++;

	if (sscanf(pCur, "%d %d", &iItemID, &iAmount) < 1)
	{
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Usage: /giveitem <name> <item_id> [amount]");
		return true;
	}

	int iTargetH = pGame->FindClientByName(cName);
	if (iTargetH == 0)
	{
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Player not found or offline.");
		return true;
	}

	if (iItemID < 0 || iItemID >= MaxItemTypes || pGame->m_pItemConfigList[iItemID] == nullptr)
	{
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Invalid item ID.");
		return true;
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
		CItem* pItem = new CItem();
		if (pGame->m_pItemManager->_bInitItemAttr(pItem, pItemName))
		{
			pItem->m_dwCount = iAmount;
			int iEraseReq = 0;
			if (pGame->m_pItemManager->_bAddClientItemList(iTargetH, pItem, &iEraseReq))
			{
				pGame->m_pItemManager->SendItemNotifyMsg(iTargetH, Notify::ItemObtained, pItem, 0);
				iCreated = iAmount;
			}
			else
			{
				delete pItem;
				pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Target's inventory is full.");
				return true;
			}
		}
		else
		{
			delete pItem;
			pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Failed to create item.");
			return true;
		}
	}
	else
	{
		// Soft-linked items: individual items, one bulk notification
		iCreated = pGame->m_pItemManager->_bAddClientBulkItemList(iTargetH, pItemName, iAmount);
	}

	char buf[128];
	std::snprintf(buf, sizeof(buf), "Gave %d x %s (ID: %d) to %s.", iCreated, pItemName, iItemID, pGame->m_pClientList[iTargetH]->m_cCharName);
	pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, buf);

	if (iCreated > 0)
	{
		char buf2[128];
		std::snprintf(buf2, sizeof(buf2), "You received %d x %s from a GM.", iCreated, pItemName);
		pGame->SendNotifyMsg(0, iTargetH, Notify::NoticeMsg, 0, 0, 0, buf2);
	}

	return true;
}
