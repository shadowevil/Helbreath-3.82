#include "Game.h"
#include "BuildItemManager.h"
#include "InventoryManager.h"
#include "ItemNameFormatter.h"
#include "NetworkMessageManager.h"
#include "Packet/SharedPackets.h"
#include "lan_eng.h"
#include "DialogBoxIDs.h"
#include "PlatformCompat.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <format>
#include <string>

using namespace hb::shared::net;
using namespace hb::shared::item;

namespace NetworkMessageHandlers {
	void HandleItemPurchased(CGame* pGame, char* pData)
	{
		int i, j;
		uint32_t dwCount;
		char  cName[hb::shared::limits::ItemNameLen]{}, cEquipPos, cGenderLimit;
		ItemType cItemType;
		bool  bIsEquipped;
		short sSprite, sSpriteFrame, sLevelLimit;
		uint16_t wCost, wWeight, wCurLifeSpan, wMaxLifeSpan;
		std::string cTxt;

		char cItemColor;

		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyItemPurchased>(
			pData, sizeof(hb::net::PacketNotifyItemPurchased));
		if (!pkt) return;

		memcpy(cName, pkt->name, sizeof(pkt->name));
		dwCount = pkt->count;
		cItemType = static_cast<ItemType>(pkt->item_type);
		cEquipPos = static_cast<char>(pkt->equip_pos);
		bIsEquipped = (pkt->is_equipped != 0);
		sLevelLimit = static_cast<short>(pkt->level_limit);
		cGenderLimit = static_cast<char>(pkt->gender_limit);
		wCurLifeSpan = pkt->cur_lifespan;
		wMaxLifeSpan = pkt->max_lifespan;
		wWeight = pkt->weight;
		sSprite = static_cast<short>(pkt->sprite);
		sSpriteFrame = static_cast<short>(pkt->sprite_frame);
		cItemColor = static_cast<char>(pkt->item_color);
		wCost = pkt->cost;
		cTxt = std::format(NOTIFYMSG_ITEMPURCHASED, cName, wCost);
		pGame->AddEventList(cTxt.c_str(), 10);

		short sItemID = pkt->item_id;

		if ((cItemType == ItemType::Consume) || (cItemType == ItemType::Arrow))
		{
			for (i = 0; i < hb::shared::limits::MaxItems; i++)
				if ((pGame->m_pItemList[i] != 0) && (pGame->m_pItemList[i]->m_sIDnum == sItemID))
				{
					pGame->m_pItemList[i]->m_dwCount += dwCount;
					return;
				}
		}

		short nX, nY;
		for (i = 0; i < hb::shared::limits::MaxItems; i++)
		{
			if ((pGame->m_pItemList[i] != 0) && (pGame->m_pItemList[i]->m_sIDnum == sItemID))
			{
				nX = pGame->m_pItemList[i]->m_sX;
				nY = pGame->m_pItemList[i]->m_sY;
				break;
			}
			else
			{
				nX = 40;
				nY = 30;
			}
		}

		for (i = 0; i < hb::shared::limits::MaxItems; i++)
			if (pGame->m_pItemList[i] == 0)
			{
				pGame->m_pItemList[i] = std::make_unique<CItem>();
				pGame->m_pItemList[i]->m_sIDnum = sItemID;
				pGame->m_pItemList[i]->m_dwCount = dwCount;
				pGame->m_pItemList[i]->m_sX = nX;
				pGame->m_pItemList[i]->m_sY = nY;
				pGame->bSendCommand(MsgId::RequestSetItemPos, 0, i, nX, nY, 0, 0);
				pGame->m_bIsItemDisabled[i] = false;
				pGame->m_bIsItemEquipped[i] = false;
				pGame->m_pItemList[i]->m_wCurLifeSpan = wCurLifeSpan;
				pGame->m_pItemList[i]->m_cItemColor = cItemColor;
				pGame->m_pItemList[i]->m_dwAttribute = 0;

				for (j = 0; j < hb::shared::limits::MaxItems; j++)
					if (pGame->m_cItemOrder[j] == -1) {
						pGame->m_cItemOrder[j] = i;
						return;
					}

				return;
			}
	}

	void HandleItemObtained(CGame* pGame, char* pData)
	{
		int i, j;
		uint32_t dwCount, dwAttribute;
		char  cName[hb::shared::limits::ItemNameLen]{}, cEquipPos;
		ItemType cItemType;
		bool  bIsEquipped;
		short sSprite, sSpriteFrame, sLevelLimit, sSpecialEV2;
		std::string cTxt;

		char cGenderLimit, cItemColor;
		uint16_t wWeight, wCurLifeSpan, wMaxLifeSpan;

		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyItemObtained>(
			pData, sizeof(hb::net::PacketNotifyItemObtained));
		if (!pkt) return;

		memcpy(cName, pkt->name, sizeof(pkt->name));
		dwCount = pkt->count;
		cItemType = static_cast<ItemType>(pkt->item_type);
		cEquipPos = static_cast<char>(pkt->equip_pos);
		bIsEquipped = (pkt->is_equipped != 0);
		sLevelLimit = static_cast<short>(pkt->level_limit);
		cGenderLimit = static_cast<char>(pkt->gender_limit);
		wCurLifeSpan = pkt->cur_lifespan;
		wMaxLifeSpan = pkt->max_lifespan;
		wWeight = pkt->weight;
		sSprite = static_cast<short>(pkt->sprite);
		sSpriteFrame = static_cast<short>(pkt->sprite_frame);
		cItemColor = static_cast<char>(pkt->item_color);
		sSpecialEV2 = static_cast<short>(pkt->spec_value2);
		dwAttribute = pkt->attribute;

		if (dwCount == 1) cTxt = std::format(NOTIFYMSG_ITEMOBTAINED2, cName);
		else cTxt = std::format(NOTIFYMSG_ITEMOBTAINED1, dwCount, cName);

		pGame->AddEventList(cTxt.c_str(), 10);
		pGame->PlayGameSound('E', 20, 0);

		pGame->m_pMapData->bSetItem(pGame->m_pPlayer->m_sPlayerX, pGame->m_pPlayer->m_sPlayerY, 0, 0, 0, false);

		short sItemID = pkt->item_id;

		if ((cItemType == ItemType::Consume) || (cItemType == ItemType::Arrow))
		{
			for (i = 0; i < hb::shared::limits::MaxItems; i++)
				if ((pGame->m_pItemList[i] != 0) && (pGame->m_pItemList[i]->m_sIDnum == sItemID))
				{
					pGame->m_pItemList[i]->m_dwCount += dwCount;
					pGame->m_bIsItemDisabled[i] = false;
					return;
				}
		}

		short nX, nY;
		for (i = 0; i < hb::shared::limits::MaxItems; i++)
		{
			if ((pGame->m_pItemList[i] != 0) && (pGame->m_pItemList[i]->m_sIDnum == sItemID))
			{
				nX = pGame->m_pItemList[i]->m_sX;
				nY = pGame->m_pItemList[i]->m_sY;
				break;
			}
			else
			{
				nX = 40;
				nY = 30;
			}
		}

		for (i = 0; i < hb::shared::limits::MaxItems; i++)
			if (pGame->m_pItemList[i] == 0)
			{
				pGame->m_pItemList[i] = std::make_unique<CItem>();
				pGame->m_pItemList[i]->m_sIDnum = sItemID;
				pGame->m_pItemList[i]->m_dwCount = dwCount;
				pGame->m_pItemList[i]->m_sX = nX;
				pGame->m_pItemList[i]->m_sY = nY;
				pGame->bSendCommand(MsgId::RequestSetItemPos, 0, i, nX, nY, 0, 0);
				pGame->m_bIsItemDisabled[i] = false;
				pGame->m_bIsItemEquipped[i] = false;
				pGame->m_pItemList[i]->m_wCurLifeSpan = wCurLifeSpan;
				pGame->m_pItemList[i]->m_cItemColor = cItemColor;
				pGame->m_pItemList[i]->m_sItemSpecEffectValue2 = sSpecialEV2;
				pGame->m_pItemList[i]->m_dwAttribute = dwAttribute;

				BuildItemManager::Get().UpdateAvailableRecipes();

				for (j = 0; j < hb::shared::limits::MaxItems; j++)
					if (pGame->m_cItemOrder[j] == -1) {
						pGame->m_cItemOrder[j] = i;
						return;
					}
				return;
			}
	}

	void HandleItemObtainedBulk(CGame* pGame, char* pData)
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyItemObtained>(
			pData, sizeof(hb::net::PacketNotifyItemObtained));
		if (!pkt) return;

		char cName[hb::shared::limits::ItemNameLen]{};
		memcpy(cName, pkt->name, sizeof(pkt->name));

		int iTotalCount = pkt->count;
		short sItemID = pkt->item_id;
		uint16_t wCurLifeSpan = pkt->cur_lifespan;
		uint16_t wMaxLifeSpan = pkt->max_lifespan;
		uint16_t wWeight = pkt->weight;
		short sSprite = static_cast<short>(pkt->sprite);
		short sSpriteFrame = static_cast<short>(pkt->sprite_frame);
		char cItemColor = static_cast<char>(pkt->item_color);
		short sSpecialEV2 = static_cast<short>(pkt->spec_value2);
		uint32_t dwAttribute = pkt->attribute;

		// One chat message for the entire batch
		std::string cTxt;
		if (iTotalCount == 1) cTxt = std::format(NOTIFYMSG_ITEMOBTAINED2, cName);
		else cTxt = std::format(NOTIFYMSG_ITEMOBTAINED1, iTotalCount, cName);
		pGame->AddEventList(cTxt.c_str(), 10);
		pGame->PlayGameSound('E', 20, 0);

		// Create individual items in separate slots (no Consume/Arrow merge)
		short nX = 40, nY = 30;
		for (int i = 0; i < hb::shared::limits::MaxItems; i++)
		{
			if ((pGame->m_pItemList[i] != 0) && (pGame->m_pItemList[i]->m_sIDnum == sItemID))
			{
				nX = pGame->m_pItemList[i]->m_sX;
				nY = pGame->m_pItemList[i]->m_sY;
				break;
			}
		}

		int iCreated = 0;
		for (int n = 0; n < iTotalCount; n++)
		{
			for (int i = 0; i < hb::shared::limits::MaxItems; i++)
			{
				if (pGame->m_pItemList[i] == 0)
				{
					pGame->m_pItemList[i] = std::make_unique<CItem>();
					pGame->m_pItemList[i]->m_sIDnum = sItemID;
					pGame->m_pItemList[i]->m_dwCount = 1;
					pGame->m_pItemList[i]->m_sX = nX;
					pGame->m_pItemList[i]->m_sY = nY;
					pGame->bSendCommand(MsgId::RequestSetItemPos, 0, i, nX, nY, 0, 0);
					pGame->m_bIsItemDisabled[i] = false;
					pGame->m_bIsItemEquipped[i] = false;
					pGame->m_pItemList[i]->m_wCurLifeSpan = wCurLifeSpan;
					pGame->m_pItemList[i]->m_wMaxLifeSpan = wMaxLifeSpan;
					pGame->m_pItemList[i]->m_wWeight = wWeight;
					pGame->m_pItemList[i]->m_cItemColor = cItemColor;
					pGame->m_pItemList[i]->m_sItemSpecEffectValue2 = sSpecialEV2;
					pGame->m_pItemList[i]->m_dwAttribute = dwAttribute;

					for (int j = 0; j < hb::shared::limits::MaxItems; j++)
					{
						if (pGame->m_cItemOrder[j] == -1)
						{
							pGame->m_cItemOrder[j] = i;
							break;
						}
					}
					iCreated++;
					break;
				}
			}
		}

		if (iCreated > 0)
			BuildItemManager::Get().UpdateAvailableRecipes();
	}

	void HandleItemLifeSpanEnd(CGame* pGame, char* pData)
	{
		short sEquipPos, sItemIndex;
		std::string cTxt;

		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyItemLifeSpanEnd>(
			pData, sizeof(hb::net::PacketNotifyItemLifeSpanEnd));
		if (!pkt) return;
		sEquipPos = static_cast<short>(pkt->equip_pos);
		sItemIndex = static_cast<short>(pkt->item_index);

		auto itemInfo = ItemNameFormatter::Get().Format(pGame->m_pItemList[sItemIndex].get());
		cTxt = std::format(NOTIFYMSG_ITEMLIFE_SPANEND1, itemInfo.name.c_str());
		pGame->AddEventList(cTxt.c_str(), 10);
		pGame->m_sItemEquipmentStatus[pGame->m_pItemList[sItemIndex]->m_cEquipPos] = -1;
		pGame->m_bIsItemEquipped[sItemIndex] = false;
		pGame->m_pItemList[sItemIndex]->m_wCurLifeSpan = 0;

		pGame->PlayGameSound('E', 10, 0);
	}

	void HandleItemReleased(CGame* pGame, char* pData)
	{
		short sEquipPos, sItemIndex;
		std::string cTxt;

		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyItemReleased>(
			pData, sizeof(hb::net::PacketNotifyItemReleased));
		if (!pkt) return;
		sEquipPos = static_cast<short>(pkt->equip_pos);
		sItemIndex = static_cast<short>(pkt->item_index);

		auto itemInfo2 = ItemNameFormatter::Get().Format(pGame->m_pItemList[sItemIndex].get());
		cTxt = std::format(ITEM_EQUIPMENT_RELEASED, itemInfo2.name.c_str());
		pGame->AddEventList(cTxt.c_str(), 10);
		pGame->m_bIsItemEquipped[sItemIndex] = false;
		pGame->m_sItemEquipmentStatus[pGame->m_pItemList[sItemIndex]->m_cEquipPos] = -1;

		{
			short sID = pGame->m_pItemList[sItemIndex]->m_sIDnum;
			if (sID == hb::shared::item::ItemId::AngelicPandentSTR || sID == hb::shared::item::ItemId::AngelicPandentDEX ||
				sID == hb::shared::item::ItemId::AngelicPandentINT || sID == hb::shared::item::ItemId::AngelicPandentMAG)
				pGame->PlayGameSound('E', 53, 0);
			else
				pGame->PlayGameSound('E', 29, 0);
		}
	}

	void HandleSetItemCount(CGame* pGame, char* pData)
	{
		short  sItemIndex;
		uint32_t dwCount;
		bool   bIsItemUseResponse;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifySetItemCount>(
			pData, sizeof(hb::net::PacketNotifySetItemCount));
		if (!pkt) return;
		sItemIndex = static_cast<short>(pkt->item_index);
		dwCount = pkt->count;
		bIsItemUseResponse = (pkt->notify != 0);
		if (pGame->m_pItemList[sItemIndex] != 0)
		{
			pGame->m_pItemList[sItemIndex]->m_dwCount = dwCount;
			if (bIsItemUseResponse == true) pGame->m_bIsItemDisabled[sItemIndex] = false;
		}
	}

	void HandleItemDepleted_EraseItem(CGame* pGame, char* pData)
	{
		short  sItemIndex;
		bool   bIsUseItemResult;
		std::string cTxt;

		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyItemDepletedEraseItem>(
			pData, sizeof(hb::net::PacketNotifyItemDepletedEraseItem));
		if (!pkt) return;
		sItemIndex = static_cast<short>(pkt->item_index);
		bIsUseItemResult = (pkt->use_result != 0);


		auto itemInfo3 = ItemNameFormatter::Get().Format(pGame->m_pItemList[sItemIndex].get());

		CItem* pCfg = pGame->GetItemConfig(pGame->m_pItemList[sItemIndex]->m_sIDnum);

		if (pGame->m_bIsItemEquipped[sItemIndex] == true) {
			cTxt = std::format(ITEM_EQUIPMENT_RELEASED, itemInfo3.name.c_str());
			pGame->AddEventList(cTxt.c_str(), 10);

			if (pCfg) pGame->m_sItemEquipmentStatus[pCfg->m_cEquipPos] = -1;
			pGame->m_bIsItemEquipped[sItemIndex] = false;
		}

		if (pCfg && ((pCfg->GetItemType() == ItemType::Consume) ||
			(pCfg->GetItemType() == ItemType::Arrow))) {
			cTxt = std::format(NOTIFYMSG_ITEMDEPlETED_ERASEITEM2, itemInfo3.name.c_str());
		}
		else if (pCfg) {
			if (pCfg->GetItemType() == ItemType::UseDeplete) {
				if (bIsUseItemResult == true) {
					cTxt = std::format(NOTIFYMSG_ITEMDEPlETED_ERASEITEM3, itemInfo3.name.c_str());
				}
			}
			else if (pCfg->GetItemType() == ItemType::Eat) {
				if (bIsUseItemResult == true) {
					cTxt = std::format(NOTIFYMSG_ITEMDEPlETED_ERASEITEM4, itemInfo3.name.c_str());
					if ((pGame->m_pPlayer->m_sPlayerType >= 1) && (pGame->m_pPlayer->m_sPlayerType <= 3))
						pGame->PlayGameSound('C', 19, 0);
					if ((pGame->m_pPlayer->m_sPlayerType >= 4) && (pGame->m_pPlayer->m_sPlayerType <= 6))
						pGame->PlayGameSound('C', 20, 0);
				}
			}
			else if (pCfg->GetItemType() == ItemType::UseDepleteDest) {
				if (bIsUseItemResult == true) {
					cTxt = std::format(NOTIFYMSG_ITEMDEPlETED_ERASEITEM3, itemInfo3.name.c_str());
				}
			}
			else {
				if (bIsUseItemResult == true) {
					cTxt = std::format(NOTIFYMSG_ITEMDEPlETED_ERASEITEM6, itemInfo3.name.c_str());
					pGame->PlayGameSound('E', 10, 0);
				}
			}
		}
		pGame->AddEventList(cTxt.c_str(), 10);

		if (bIsUseItemResult == true) pGame->m_bItemUsingStatus = false;
		InventoryManager::Get().EraseItem(static_cast<char>(sItemIndex));
		BuildItemManager::Get().UpdateAvailableRecipes();
	}

	void HandleDropItemFin_EraseItem(CGame* pGame, char* pData)
	{
		int iAmount;
		short  sItemIndex;
		std::string cTxt;

		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyDropItemFinEraseItem>(
			pData, sizeof(hb::net::PacketNotifyDropItemFinEraseItem));
		if (!pkt) return;
		sItemIndex = static_cast<short>(pkt->item_index);
		iAmount = static_cast<int>(pkt->amount);

		auto itemInfo4 = ItemNameFormatter::Get().Format(pGame->m_pItemList[sItemIndex].get());

		if (pGame->m_bIsItemEquipped[sItemIndex] == true)
		{
			cTxt = std::format(ITEM_EQUIPMENT_RELEASED, itemInfo4.name.c_str());
			pGame->AddEventList(cTxt.c_str(), 10);
			pGame->m_sItemEquipmentStatus[pGame->m_pItemList[sItemIndex]->m_cEquipPos] = -1;
			pGame->m_bIsItemEquipped[sItemIndex] = false;
		}
		if (pGame->m_pPlayer->m_iHP > 0)
		{
			cTxt = std::format(NOTIFYMSG_THROW_ITEM2, itemInfo4.name.c_str());
		}
		else
		{
			if (iAmount < 2)
				cTxt = std::format(NOTIFYMSG_DROPITEMFIN_ERASEITEM3, itemInfo4.name.c_str());
			else
			{
				cTxt = std::format(NOTIFYMSG_DROPITEMFIN_ERASEITEM5, itemInfo4.name.c_str());
			}
		}
		pGame->AddEventList(cTxt.c_str(), 10);
		InventoryManager::Get().EraseItem(static_cast<char>(sItemIndex));
		BuildItemManager::Get().UpdateAvailableRecipes();
	}

	void HandleGiveItemFin_EraseItem(CGame* pGame, char* pData)
	{
		int iAmount;
		short  sItemIndex;
		std::string cTxt;

		char cName[hb::shared::limits::ItemNameLen]{};

		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyGiveItemFinEraseItem>(
			pData, sizeof(hb::net::PacketNotifyGiveItemFinEraseItem));
		if (!pkt) return;
		sItemIndex = static_cast<short>(pkt->item_index);
		iAmount = static_cast<int>(pkt->amount);

		memcpy(cName, pkt->name, sizeof(pkt->name));

		char cStr1[64], cStr2[64], cStr3[64];
		CItem* pCfg = pGame->GetItemConfig(pGame->m_pItemList[sItemIndex]->m_sIDnum);
		cStr2[0] = 0;
		cStr3[0] = 0;

		if (pGame->m_bIsItemEquipped[sItemIndex] == true) {
			cTxt = std::format(ITEM_EQUIPMENT_RELEASED, pCfg ? pCfg->m_cName : "Unknown");
			pGame->AddEventList(cTxt.c_str(), 10);

			if (pCfg) pGame->m_sItemEquipmentStatus[pCfg->m_cEquipPos] = -1;
			pGame->m_bIsItemEquipped[sItemIndex] = false;
		}
		if (cName[0] == 0) cTxt = std::format(NOTIFYMSG_GIVEITEMFIN_ERASEITEM2, iAmount, cStr1);
		else {
			if (strcmp(cName, "Howard") == 0)
				cTxt = std::format(NOTIFYMSG_GIVEITEMFIN_ERASEITEM3, iAmount, cStr1);
			else if (strcmp(cName, "William") == 0)
				cTxt = std::format(NOTIFYMSG_GIVEITEMFIN_ERASEITEM4, iAmount, cStr1);
			else if (strcmp(cName, "Kennedy") == 0)
				cTxt = std::format(NOTIFYMSG_GIVEITEMFIN_ERASEITEM5, iAmount, cStr1);
			else if (strcmp(cName, "Tom") == 0)
				cTxt = std::format(NOTIFYMSG_GIVEITEMFIN_ERASEITEM7, iAmount, cStr1);
			else cTxt = std::format(NOTIFYMSG_GIVEITEMFIN_ERASEITEM8, iAmount, cStr1, cName);
		}
		pGame->AddEventList(cTxt.c_str(), 10);
		InventoryManager::Get().EraseItem(static_cast<char>(sItemIndex));
		BuildItemManager::Get().UpdateAvailableRecipes();
	}

	void HandleItemRepaired(CGame* pGame, char* pData)
	{
		std::string cTxt;
		DWORD dwItemID, dwLife;

		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyItemRepaired>(
			pData, sizeof(hb::net::PacketNotifyItemRepaired));
		if (!pkt) return;
		dwItemID = pkt->item_id;
		dwLife = pkt->life;

		pGame->m_pItemList[dwItemID]->m_wCurLifeSpan = static_cast<WORD>(dwLife);
		pGame->m_bIsItemDisabled[dwItemID] = false;
		auto itemInfo5 = ItemNameFormatter::Get().Format(pGame->m_pItemList[dwItemID].get());

		cTxt = std::format(NOTIFYMSG_ITEMREPAIRED1, itemInfo5.name.c_str());

		pGame->AddEventList(cTxt.c_str(), 10);
	}

	void HandleRepairItemPrice(CGame* pGame, char* pData)
	{
		char cName[hb::shared::limits::ItemNameLen]{};
		DWORD wV1, wV2, wV3, wV4;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyRepairItemPrice>(
			pData, sizeof(hb::net::PacketNotifyRepairItemPrice));
		if (!pkt) return;
		wV1 = pkt->v1;
		wV2 = pkt->v2;
		wV3 = pkt->v3;
		wV4 = pkt->v4;
		memcpy(cName, pkt->item_name, sizeof(pkt->item_name));
		pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::SellOrRepair, 2, wV1, wV2);
		pGame->m_dialogBoxManager.Info(DialogBoxId::SellOrRepair).sV3 = wV3;
	}

	void HandleRepairAllPrices(CGame* pGame, char* pData)
	{
		int i;

		pGame->totalPrice = 0;
		const auto* header = hb::net::PacketCast<hb::net::PacketNotifyRepairAllPricesHeader>(
			pData, sizeof(hb::net::PacketNotifyRepairAllPricesHeader));
		if (!header) return;
		const auto* entries = reinterpret_cast<const hb::net::PacketNotifyRepairAllPricesEntry*>(
			pData + sizeof(hb::net::PacketNotifyRepairAllPricesHeader));
		pGame->totalItemRepair = header->total;

		for (i = 0; i < pGame->totalItemRepair; i++)
		{
			pGame->m_stRepairAll[i].index = entries[i].index;
			pGame->m_stRepairAll[i].price = entries[i].price;

			pGame->totalPrice += pGame->m_stRepairAll[i].price;
		}
		if (pGame->totalItemRepair == 0)
			pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::RepairAll, 1, 0, 0);
		else
			pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::RepairAll, 0, 0, 0);
	}

	void HandleSellItemPrice(CGame* pGame, char* pData)
	{
		char cName[hb::shared::limits::ItemNameLen]{};
		DWORD wV1, wV2, wV3, wV4;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifySellItemPrice>(
			pData, sizeof(hb::net::PacketNotifySellItemPrice));
		if (!pkt) return;
		wV1 = pkt->v1;
		wV2 = pkt->v2;
		wV3 = pkt->v3;
		wV4 = pkt->v4;
		memcpy(cName, pkt->item_name, sizeof(pkt->item_name));
		pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::SellOrRepair, 1, wV1, wV2);
		pGame->m_dialogBoxManager.Info(DialogBoxId::SellOrRepair).sV3 = wV3;
		pGame->m_dialogBoxManager.Info(DialogBoxId::SellOrRepair).sV4 = wV4;
	}

	void HandleCannotRepairItem(CGame* pGame, char* pData)
	{
		std::string cTxt;


		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyCannotRepairItem>(
			pData, sizeof(hb::net::PacketNotifyCannotRepairItem));
		if (!pkt) return;
		const auto wV1 = pkt->item_index;
		const auto wV2 = pkt->reason;
		auto itemInfo6 = ItemNameFormatter::Get().Format(pGame->m_pItemList[wV1].get());

		switch (wV2) {
		case 1:
			cTxt = std::format(NOTIFYMSG_CANNOT_REPAIR_ITEM1, itemInfo6.name.c_str());
			pGame->AddEventList(cTxt.c_str(), 10);
			break;
		case 2:
			cTxt = std::format(NOTIFYMSG_CANNOT_REPAIR_ITEM2, itemInfo6.name.c_str());
			pGame->AddEventList(cTxt.c_str(), 10);
			break;
		}
		pGame->m_bIsItemDisabled[wV1] = false;
	}

	void HandleCannotSellItem(CGame* pGame, char* pData)
	{
		std::string cTxt;


		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyCannotSellItem>(
			pData, sizeof(hb::net::PacketNotifyCannotSellItem));
		if (!pkt) return;
		const auto wV1 = pkt->item_index;
		const auto wV2 = pkt->reason;

		auto itemInfo7 = ItemNameFormatter::Get().Format(pGame->m_pItemList[wV1].get());

		switch (wV2) {
		case 1:
			cTxt = std::format(NOTIFYMSG_CANNOT_SELL_ITEM1, itemInfo7.name.c_str());
			pGame->AddEventList(cTxt.c_str(), 10);
			break;

		case 2:
			cTxt = std::format(NOTIFYMSG_CANNOT_SELL_ITEM2, itemInfo7.name.c_str());
			pGame->AddEventList(cTxt.c_str(), 10);
			break;

		case 3:
			cTxt = std::format(NOTIFYMSG_CANNOT_SELL_ITEM3, itemInfo7.name.c_str());
			pGame->AddEventList(cTxt.c_str(), 10);
			pGame->AddEventList(NOTIFYMSG_CANNOT_SELL_ITEM4, 10);
			break;

		case 4:
			pGame->AddEventList(NOTIFYMSG_CANNOT_SELL_ITEM5, 10);
			pGame->AddEventList(NOTIFYMSG_CANNOT_SELL_ITEM6, 10);
			break;
		}
		pGame->m_bIsItemDisabled[wV1] = false;
	}

	void HandleCannotGiveItem(CGame* pGame, char* pData)
	{
		std::string cTxt;

		char cName[hb::shared::limits::ItemNameLen]{};

		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyCannotGiveItem>(
			pData, sizeof(hb::net::PacketNotifyCannotGiveItem));
		if (!pkt) return;
		const auto wItemIndex = pkt->item_index;
		const auto iAmount = static_cast<int>(pkt->amount);
		memcpy(cName, pkt->name, sizeof(pkt->name));

		auto itemInfo8 = ItemNameFormatter::Get().Format(pGame->m_pItemList[wItemIndex].get());
		if (iAmount == 1) cTxt = std::format(NOTIFYMSG_CANNOT_GIVE_ITEM2, itemInfo8.name.c_str(), cName);
		else cTxt = std::format(NOTIFYMSG_CANNOT_GIVE_ITEM1, iAmount, itemInfo8.name.c_str(), cName);

		pGame->AddEventList(cTxt.c_str(), 10);
	}

	void HandleItemColorChange(CGame* pGame, char* pData)
	{
		short sItemIndex, sItemColor;
		std::string cTxt;

		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyItemColorChange>(
			pData, sizeof(hb::net::PacketNotifyItemColorChange));
		if (!pkt) return;
		sItemIndex = static_cast<short>(pkt->item_index);
		sItemColor = static_cast<short>(pkt->item_color);

		if (pGame->m_pItemList[sItemIndex] != 0) {
			auto itemInfo9 = ItemNameFormatter::Get().Format(pGame->m_pItemList[sItemIndex].get());
			if (sItemColor != -1) {
				pGame->m_pItemList[sItemIndex]->m_cItemColor = static_cast<char>(sItemColor);
				cTxt = std::format(NOTIFYMSG_ITEMCOLOR_CHANGE1, itemInfo9.name.c_str());
				pGame->AddEventList(cTxt.c_str(), 10);
			}
			else {
				cTxt = std::format(NOTIFYMSG_ITEMCOLOR_CHANGE2, itemInfo9.name.c_str());
				pGame->AddEventList(cTxt.c_str(), 10);
			}
		}
	}

	void HandleDropItemFin_CountChanged(CGame* pGame, char* pData)
	{
		std::string cTxt;

		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyDropItemFinCountChanged>(
			pData, sizeof(hb::net::PacketNotifyDropItemFinCountChanged));
		if (!pkt) return;
		const auto wItemIndex = pkt->item_index;
		const auto iAmount = static_cast<int>(pkt->amount);

		CItem* pCfg = pGame->GetItemConfig(pGame->m_pItemList[wItemIndex]->m_sIDnum);
		cTxt = std::format(NOTIFYMSG_THROW_ITEM1, iAmount, pCfg ? pCfg->m_cName : "Unknown");

		pGame->AddEventList(cTxt.c_str(), 10);
	}

	void HandleGiveItemFin_CountChanged(CGame* pGame, char* pData)
	{
		std::string cTxt;

		char cName[hb::shared::limits::ItemNameLen]{};
		WORD wItemIndex;
		int iAmount;

		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyGiveItemFinCountChanged>(
			pData, sizeof(hb::net::PacketNotifyGiveItemFinCountChanged));
		if (!pkt) return;
		wItemIndex = pkt->item_index;
		iAmount = static_cast<int>(pkt->amount);

		memcpy(cName, pkt->name, sizeof(pkt->name));

		CItem* pCfg = pGame->GetItemConfig(pGame->m_pItemList[wItemIndex]->m_sIDnum);
		if (iAmount == 1) cTxt = std::format(NOTIFYMSG_GIVEITEMFIN_COUNTCHANGED1, pCfg ? pCfg->m_cName : "Unknown", cName);
		cTxt = std::format(NOTIFYMSG_GIVEITEMFIN_COUNTCHANGED2, iAmount, pCfg ? pCfg->m_cName : "Unknown", cName);
		pGame->AddEventList(cTxt.c_str(), 10);
	}

	void HandleSetExchangeItem(CGame* pGame, char* pData)
	{
		short sDir, sSprite, sSpriteFrame, sCurLife, sMaxLife, sPerformance, sItemID;
		int iAmount, i;
		char cColor, cItemName[hb::shared::limits::ItemNameLen], cCharName[12];
		DWORD dwAttribute;
		std::memset(cItemName, 0, sizeof(cItemName));

		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyExchangeItem>(
			pData, sizeof(hb::net::PacketNotifyExchangeItem));
		if (!pkt) return;
		sDir = static_cast<short>(pkt->dir);
		sSprite = pkt->sprite;
		sSpriteFrame = pkt->sprite_frame;
		iAmount = pkt->amount;
		cColor = static_cast<char>(pkt->color);
		sCurLife = pkt->cur_life;
		sMaxLife = pkt->max_life;
		sPerformance = pkt->performance;
		memcpy(cItemName, pkt->item_name, sizeof(pkt->item_name));
		memcpy(cCharName, pkt->char_name, sizeof(pkt->char_name));
		dwAttribute = pkt->attribute;
		sItemID = pkt->item_id;

		if (sDir >= 1000)  // Set the item I want to exchange
		{
			i = 0;
			while (pGame->m_stDialogBoxExchangeInfo[i].sV1 != -1)
			{
				i++;
				if (i >= 4) return; // Error situation
			}
		}
		else // Set the item he proposes me.
		{
			i = 4;
			while (pGame->m_stDialogBoxExchangeInfo[i].sV1 != -1)
			{
				i++;
				if (i >= 8) return; // Error situation
			}
		}
		pGame->m_stDialogBoxExchangeInfo[i].sV1 = sSprite;
		pGame->m_stDialogBoxExchangeInfo[i].sV2 = sSpriteFrame;
		pGame->m_stDialogBoxExchangeInfo[i].sV3 = iAmount;
		pGame->m_stDialogBoxExchangeInfo[i].sV4 = cColor;
		pGame->m_stDialogBoxExchangeInfo[i].sV5 = static_cast<int>(sCurLife);
		pGame->m_stDialogBoxExchangeInfo[i].sV6 = static_cast<int>(sMaxLife);
		pGame->m_stDialogBoxExchangeInfo[i].sV7 = static_cast<int>(sPerformance);
		pGame->m_stDialogBoxExchangeInfo[i].cStr1.assign(cItemName, strnlen(cItemName, hb::shared::limits::ItemNameLen));
		pGame->m_stDialogBoxExchangeInfo[i].cStr2.assign(cCharName, strnlen(cCharName, hb::shared::limits::CharNameLen));
		pGame->m_stDialogBoxExchangeInfo[i].dwV1 = dwAttribute;
		pGame->m_stDialogBoxExchangeInfo[i].sItemID = sItemID;
	}

	void HandleOpenExchangeWindow(CGame* pGame, char* pData)
	{
		short sDir, sSprite, sSpriteFrame, sCurLife, sMaxLife, sPerformance, sItemID;
		int iAmount;
		char cColor, cItemName[hb::shared::limits::ItemNameLen], cCharName[12];
		DWORD dwAttribute;
		std::memset(cItemName, 0, sizeof(cItemName));

		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyExchangeItem>(
			pData, sizeof(hb::net::PacketNotifyExchangeItem));
		if (!pkt) return;
		sDir = static_cast<short>(pkt->dir);
		sSprite = pkt->sprite;
		sSpriteFrame = pkt->sprite_frame;
		iAmount = pkt->amount;
		cColor = static_cast<char>(pkt->color);
		sCurLife = pkt->cur_life;
		sMaxLife = pkt->max_life;
		sPerformance = pkt->performance;
		memcpy(cItemName, pkt->item_name, sizeof(pkt->item_name));
		memcpy(cCharName, pkt->char_name, sizeof(pkt->char_name));
		dwAttribute = pkt->attribute;
		sItemID = pkt->item_id;

		pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::Exchange, 1, 0, 0, 0);
		// Initialize all exchange slots
		for (int j = 0; j < 8; j++)
		{
			pGame->m_stDialogBoxExchangeInfo[j].sV1 = -1;
			pGame->m_stDialogBoxExchangeInfo[j].sV2 = -1;
			pGame->m_stDialogBoxExchangeInfo[j].sV3 = -1;
			pGame->m_stDialogBoxExchangeInfo[j].sV4 = -1;
			pGame->m_stDialogBoxExchangeInfo[j].sV5 = -1;
			pGame->m_stDialogBoxExchangeInfo[j].sV6 = -1;
			pGame->m_stDialogBoxExchangeInfo[j].sV7 = -1;
			pGame->m_stDialogBoxExchangeInfo[j].sItemID = -1;
			pGame->m_stDialogBoxExchangeInfo[j].dwV1 = 0;
		}
		int i;
		if (sDir >= 1000)  // Set the item I want to exchange
		{
			i = 0;
			while (pGame->m_stDialogBoxExchangeInfo[i].sV1 != -1)
			{
				i++;
				if (i >= 4) return; // Error situation
			}
			if ((sDir > 1000) && (i == 0))
			{
				pGame->m_bIsItemDisabled[sDir - 1000] = true;
			}
		}
		else // Set the item he proposes me.
		{
			i = 4;
			while (pGame->m_stDialogBoxExchangeInfo[i].sV1 != -1)
			{
				i++;
				if (i >= 8) return; // Error situation
			}
		}
		pGame->m_stDialogBoxExchangeInfo[i].sV1 = sSprite;
		pGame->m_stDialogBoxExchangeInfo[i].sV2 = sSpriteFrame;
		pGame->m_stDialogBoxExchangeInfo[i].sV3 = iAmount;
		pGame->m_stDialogBoxExchangeInfo[i].sV4 = cColor;
		pGame->m_stDialogBoxExchangeInfo[i].sV5 = static_cast<int>(sCurLife);
		pGame->m_stDialogBoxExchangeInfo[i].sV6 = static_cast<int>(sMaxLife);
		pGame->m_stDialogBoxExchangeInfo[i].sV7 = static_cast<int>(sPerformance);
		pGame->m_stDialogBoxExchangeInfo[i].cStr1.assign(cItemName, strnlen(cItemName, hb::shared::limits::ItemNameLen));
		pGame->m_stDialogBoxExchangeInfo[i].cStr2.assign(cCharName, strnlen(cCharName, hb::shared::limits::CharNameLen));
		pGame->m_stDialogBoxExchangeInfo[i].dwV1 = dwAttribute;
		pGame->m_stDialogBoxExchangeInfo[i].sItemID = sItemID;
	}

	void HandleCurLifeSpan(CGame* pGame, char* pData)
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyCurLifeSpan>(
			pData, sizeof(hb::net::PacketNotifyCurLifeSpan));

		if (!pkt)
			return;

		int iItemIndex = pkt->item_index;

		if (pGame->m_pItemList[iItemIndex] == nullptr)
			return;

		pGame->m_pItemList[iItemIndex]->m_wCurLifeSpan = static_cast<WORD>(pkt->cur_lifespan);
	}

	void HandleNotEnoughGold(CGame* pGame, char* pData)
	{
		pGame->m_dialogBoxManager.DisableDialogBox(DialogBoxId::SellOrRepair);
		pGame->AddEventList(NOTIFY_MSG_HANDLER67, 10);
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyNotEnoughGold>(
			pData, sizeof(hb::net::PacketNotifyNotEnoughGold));
		if (!pkt) return;
		if (pkt->item_index >= 0) {
			pGame->m_bIsItemDisabled[pkt->item_index] = false;
		}
	}

	void HandleCannotCarryMoreItem(CGame* pGame, char* pData)
	{
		pGame->AddEventList(NOTIFY_MSG_HANDLER65, 10);
		pGame->AddEventList(NOTIFY_MSG_HANDLER66, 10);
		// Bank dialog Box
		pGame->m_dialogBoxManager.Info(DialogBoxId::Bank).cMode = 0;
	}

	void HandleItemAttributeChange(CGame* pGame, char* pData)
	{
		short sV1;
		DWORD dwTemp;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyItemAttributeChange>(
			pData, sizeof(hb::net::PacketNotifyItemAttributeChange));
		if (!pkt) return;
		sV1 = static_cast<short>(pkt->item_index);
		dwTemp = pGame->m_pItemList[sV1]->m_dwAttribute;
		pGame->m_pItemList[sV1]->m_dwAttribute = pkt->attribute;
		if (pkt->spec_value1 != 0)
			pGame->m_pItemList[sV1]->m_sItemSpecEffectValue1 = static_cast<short>(pkt->spec_value1);
		if (pkt->spec_value2 != 0)
			pGame->m_pItemList[sV1]->m_sItemSpecEffectValue2 = static_cast<short>(pkt->spec_value2);
		
		if (dwTemp == pGame->m_pItemList[sV1]->m_dwAttribute)
		{
			if (pGame->m_dialogBoxManager.IsEnabled(DialogBoxId::ItemUpgrade) == true)
			{
				pGame->m_dialogBoxManager.Info(DialogBoxId::ItemUpgrade).cMode = 4;// Failed
			}
			pGame->PlayGameSound('E', 24, 5);
		}
		else
		{
			if (pGame->m_dialogBoxManager.IsEnabled(DialogBoxId::ItemUpgrade) == true)
			{
				pGame->m_dialogBoxManager.Info(DialogBoxId::ItemUpgrade).cMode = 3; // Success
			}
			pGame->PlayGameSound('E', 23, 5);
			switch (pGame->m_pPlayer->m_sPlayerType) {
			case 1:
			case 2:
			case 3:
				pGame->PlayGameSound('C', 21, 0);
				break;
			case 4:
			case 5:
			case 6:
				pGame->PlayGameSound('C', 22, 0);
				break;
			}
		}
	}

	void HandleItemUpgradeFail(CGame* pGame, char* pData)
	{
		short sV1;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyItemUpgradeFail>(
			pData, sizeof(hb::net::PacketNotifyItemUpgradeFail));
		if (!pkt) return;
		sV1 = static_cast<short>(pkt->reason);
		if (pGame->m_dialogBoxManager.IsEnabled(DialogBoxId::ItemUpgrade) == false) return;
		pGame->PlayGameSound('E', 24, 5);
		switch (sV1) {
		case 1:
			pGame->m_dialogBoxManager.Info(DialogBoxId::ItemUpgrade).cMode = 8; // Failed
			break;
		case 2:
			pGame->m_dialogBoxManager.Info(DialogBoxId::ItemUpgrade).cMode = 9; // Failed
			break;
		case 3:
			pGame->m_dialogBoxManager.Info(DialogBoxId::ItemUpgrade).cMode = 10; // Failed
			break;
		}
	}

	void HandleGizonItemUpgradeLeft(CGame* pGame, char* pData)
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyGizonItemUpgradeLeft>(
			pData, sizeof(hb::net::PacketNotifyGizonItemUpgradeLeft));
		if (!pkt) return;
		pGame->m_iGizonItemUpgradeLeft = pkt->left;
		switch (pkt->reason) {
		case 1:
			pGame->AddEventList(NOTIFY_MSG_HANDLER_GIZONITEMUPGRADELEFT1, 10);
			break;
		}
	}

	void HandleGizonItemChange(CGame* pGame, char* pData)
	{
		short sV1;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyGizonItemChange>(
			pData, sizeof(hb::net::PacketNotifyGizonItemChange));
		if (!pkt) return;
		sV1 = static_cast<short>(pkt->item_index);
		if (pkt->item_id > 0) pGame->m_pItemList[sV1]->m_sIDnum = pkt->item_id;
		pGame->m_pItemList[sV1]->m_wCurLifeSpan = pkt->cur_lifespan;
		pGame->m_pItemList[sV1]->m_cItemColor = pkt->item_color;
		pGame->m_pItemList[sV1]->m_sItemSpecEffectValue2 = pkt->spec_value2;
		pGame->m_pItemList[sV1]->m_dwAttribute = pkt->attribute;
		
		if (pGame->m_dialogBoxManager.IsEnabled(DialogBoxId::ItemUpgrade) == true)
		{
			pGame->m_dialogBoxManager.Info(DialogBoxId::ItemUpgrade).cMode = 3; // success
		}
		pGame->PlayGameSound('E', 23, 5);
		switch (pGame->m_pPlayer->m_sPlayerType) {
		case 1:
		case 2:
		case 3:
			pGame->PlayGameSound('C', 21, 0);
			break;
		case 4:
		case 5:
		case 6:
			pGame->PlayGameSound('C', 22, 0);
			break;
		}
	}

	void HandleItemPosList(CGame* pGame, char* pData)
	{
		int i;
		short sX, sY;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyItemPosList>(
			pData, sizeof(hb::net::PacketNotifyItemPosList));
		if (!pkt) return;
		for (i = 0; i < hb::shared::limits::MaxItems; i++) {
			sX = pkt->positions[i * 2];
			sY = pkt->positions[i * 2 + 1];
			if (pGame->m_pItemList[i] != 0) {
				if (sY < -10) sY = -10;
				if (sX < 0)   sX = 0;
				if (sX > 170) sX = 170;
				if (sY > 95)  sY = 95;

				pGame->m_pItemList[i]->m_sX = sX;
				pGame->m_pItemList[i]->m_sY = sY;
			}
		}
	}

	void HandleItemSold(CGame* pGame, char* pData)
	{
		pGame->m_dialogBoxManager.DisableDialogBox(DialogBoxId::SellOrRepair);
	}
}

