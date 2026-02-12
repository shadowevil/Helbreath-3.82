#include "Platform.h"
#include "GameCmdCreateItem.h"
#include "Game.h"
#include "ItemManager.h"
#include "Item.h"
#include "Item/ItemEnums.h"
#include <cstring>
#include <cstdio>

using namespace hb::shared::net;
using namespace hb::server::config;
bool GameCmdCreateItem::Execute(CGame* pGame, int iClientH, const char* pArgs)
{
	if (pGame->m_pClientList[iClientH] == nullptr)
		return true;

	int iItemID = 0, iAmount = 1;
	if (pArgs == nullptr || pArgs[0] == '\0' || sscanf(pArgs, "%d %d", &iItemID, &iAmount) < 1)
	{
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Usage: /createitem <item_id> [amount]");
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
			if (pGame->m_pItemManager->_bAddClientItemList(iClientH, pItem, &iEraseReq))
			{
				pGame->m_pItemManager->SendItemNotifyMsg(iClientH, Notify::ItemObtained, pItem, 0);
				iCreated = iAmount;
			}
			else
			{
				delete pItem;
				pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Inventory full.");
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
		iCreated = pGame->m_pItemManager->_bAddClientBulkItemList(iClientH, pItemName, iAmount);
	}

	char buf[128];
	std::snprintf(buf, sizeof(buf), "Created %d x %s (ID: %d).", iCreated, pItemName, iItemID);
	pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, buf);

	return true;
}
