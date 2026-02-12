// FishingManager.cpp: Client-side fishing network message handlers.
// Extracted from NetworkMessages_Fish.cpp (Phase B1).

#include "FishingManager.h"
#include "Game.h"
#include "Packet/SharedPackets.h"
#include "lan_eng.h"
#include "DialogBoxIDs.h"
#include <cstdio>
#include <cstring>
#include "PlatformCompat.h"

void FishingManager::HandleFishChance(char* pData)
{
	int iFishChance;
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyFishChance>(
		pData, sizeof(hb::net::PacketNotifyFishChance));
	if (!pkt) return;
	iFishChance = pkt->chance;
	m_pGame->m_dialogBoxManager.Info(DialogBoxId::Fishing).sV1 = iFishChance;
}

void FishingManager::HandleEventFishMode(char* pData)
{
	short sSprite, sSpriteFrame;
	char cName[hb::shared::limits::ItemNameLen]{};
	WORD wPrice;

	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyEventFishMode>(
		pData, sizeof(hb::net::PacketNotifyEventFishMode));
	if (!pkt) return;

	wPrice = pkt->price;
	sSprite = static_cast<short>(pkt->sprite);
	sSpriteFrame = static_cast<short>(pkt->sprite_frame);

	memcpy(cName, pkt->name, sizeof(pkt->name));

	m_pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::Fishing, 0, 0, wPrice, cName);
	m_pGame->m_dialogBoxManager.Info(DialogBoxId::Fishing).sV3 = sSprite;
	m_pGame->m_dialogBoxManager.Info(DialogBoxId::Fishing).sV4 = sSpriteFrame;

	m_pGame->AddEventList(NOTIFYMSG_EVENTFISHMODE1, 10);
}

void FishingManager::HandleFishCanceled(char* pData)
{
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyFishCanceled>(
		pData, sizeof(hb::net::PacketNotifyFishCanceled));
	if (!pkt) return;
	switch (pkt->reason) {
	case 0:
		m_pGame->AddEventList(NOTIFY_MSG_HANDLER52, 10);
		m_pGame->m_dialogBoxManager.DisableDialogBox(DialogBoxId::Fishing);
		break;
	case 1:
		m_pGame->AddEventList(NOTIFY_MSG_HANDLER53, 10);
		m_pGame->m_dialogBoxManager.DisableDialogBox(DialogBoxId::Fishing);
		break;
	case 2:
		m_pGame->AddEventList(NOTIFY_MSG_HANDLER54, 10);
		m_pGame->m_dialogBoxManager.DisableDialogBox(DialogBoxId::Fishing);
		break;
	}
}

void FishingManager::HandleFishSuccess(char* pData)
{
	m_pGame->AddEventList(NOTIFY_MSG_HANDLER55, 10);
	m_pGame->PlayGameSound('E', 23, 5);
	m_pGame->PlayGameSound('E', 17, 5);
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

void FishingManager::HandleFishFail(char* pData)
{
	m_pGame->AddEventList(NOTIFY_MSG_HANDLER56, 10);
	m_pGame->PlayGameSound('E', 24, 5);
}
