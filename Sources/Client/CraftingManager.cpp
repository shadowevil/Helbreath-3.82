// CraftingManager.cpp: Handles client-side crafting/portion network messages.
// Extracted from NetworkMessages_Crafting.cpp (Phase B2).

#include "CraftingManager.h"
#include "Game.h"
#include "ObjectIDRange.h"
#include "Packet/SharedPackets.h"
#include "lan_eng.h"
#include "DialogBoxIDs.h"
#include "PlatformCompat.h"
#include <cstdio>
#include <cstring>

void CraftingManager::HandleCraftingSuccess(char* pData)
{
	m_pGame->m_pPlayer->m_iContribution -= m_pGame->m_iContributionPrice;
	m_pGame->m_iContributionPrice = 0;
	m_pGame->m_dialogBoxManager.DisableDialogBox(DialogBoxId::Noticement);
	m_pGame->AddEventList(NOTIFY_MSG_HANDLER42, 10);		// "Item manufacture success!"
	m_pGame->PlayGameSound('E', 23, 5);
	switch (m_pGame->m_pPlayer->m_sPlayerType) {
	case 1:
	case 2:
	case 3:
		m_pGame->PlayGameSound('C', 21, 0);
		break;
	case 4:
	case 5:
	case 6:
		m_pGame->PlayGameSound('C', 22, 0);
		break;
	}
}

void CraftingManager::HandleCraftingFail(char* pData)
{
	int iV1;
	m_pGame->m_iContributionPrice = 0;
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyCraftingFail>(
			pData, sizeof(hb::net::PacketNotifyCraftingFail));
		if (!pkt) return;
		iV1 = pkt->reason; // Error reason
	}
	switch (iV1) {
	case 1:
		m_pGame->AddEventList(DEF_MSG_NOTIFY_CRAFTING_NO_PART, 10);		// "There is not enough material"
		m_pGame->PlayGameSound('E', 24, 5);
		break;
	case 2:
		m_pGame->AddEventList(DEF_MSG_NOTIFY_CRAFTING_NO_CONTRIB, 10);	// "There is not enough Contribution Point"
		m_pGame->PlayGameSound('E', 24, 5);
		break;
	default:
	case 3:
		m_pGame->AddEventList(DEF_MSG_NOTIFY_CRAFTING_FAILED, 10);		// "Crafting failed"
		m_pGame->PlayGameSound('E', 24, 5);
		break;
	}
}

void CraftingManager::HandleBuildItemSuccess(char* pData)
{
	short sV1, sV2;
	m_pGame->m_dialogBoxManager.DisableDialogBox(DialogBoxId::Manufacture);
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyBuildItemResult>(
			pData, sizeof(hb::net::PacketNotifyBuildItemResult));
		if (!pkt) return;
		sV1 = pkt->item_id;
		sV2 = pkt->item_count;
	}
	if (hb::shared::object_id::IsPlayerID(sV1))
	{
		m_pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::Manufacture, 6, 1, sV1, 0);
		m_pGame->m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV1 = sV2;
	}
	else
	{
		m_pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::Manufacture, 6, 1, -1 * (sV1 - 10000), 0);
		m_pGame->m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV1 = sV2;
	}
	m_pGame->AddEventList(NOTIFY_MSG_HANDLER42, 10);
	m_pGame->PlayGameSound('E', 23, 5);
	switch (m_pGame->m_pPlayer->m_sPlayerType) {
	case 1:
	case 2:
	case 3:
		m_pGame->PlayGameSound('C', 21, 0);
		break;

	case 4:
	case 5:
	case 6:
		m_pGame->PlayGameSound('C', 22, 0);
		break;
	}
}

void CraftingManager::HandleBuildItemFail(char* pData)
{
	m_pGame->m_dialogBoxManager.DisableDialogBox(DialogBoxId::Manufacture);
	m_pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::Manufacture, 6, 0, 0);
	m_pGame->AddEventList(NOTIFY_MSG_HANDLER43, 10);
	m_pGame->PlayGameSound('E', 24, 5);
}

void CraftingManager::HandlePortionSuccess(char* pData)
{
	m_pGame->AddEventList(NOTIFY_MSG_HANDLER46, 10);
}

void CraftingManager::HandlePortionFail(char* pData)
{
	m_pGame->AddEventList(NOTIFY_MSG_HANDLER47, 10);
}

void CraftingManager::HandleLowPortionSkill(char* pData)
{
	m_pGame->AddEventList(NOTIFY_MSG_HANDLER48, 10);
}

void CraftingManager::HandleNoMatchingPortion(char* pData)
{
	m_pGame->AddEventList(NOTIFY_MSG_HANDLER49, 10);
}
