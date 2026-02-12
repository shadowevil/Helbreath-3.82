#include "Game.h"
#include "ChatManager.h"
#include "NetworkMessageManager.h"
#include "Packet/SharedPackets.h"
#include "lan_eng.h"
#include "PlatformCompat.h"
#include <cstdio>
#include <cstring>
#include <string_view>
#include <cmath>
#include <format>
#include <string>

namespace NetworkMessageHandlers {
	void HandleCharisma(CGame* pGame, char* pData)
	{
		int  iPrevChar;
		std::string cTxt;

		iPrevChar = pGame->m_pPlayer->m_iCharisma;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyCharisma>(
			pData, sizeof(hb::net::PacketNotifyCharisma));
		if (!pkt) return;
		pGame->m_pPlayer->m_iCharisma = static_cast<int>(pkt->charisma);

		if (pGame->m_pPlayer->m_iCharisma > iPrevChar)
		{
			cTxt = std::format(NOTIFYMSG_CHARISMA_UP, pGame->m_pPlayer->m_iCharisma - iPrevChar);
			pGame->AddEventList(cTxt.c_str(), 10);
			pGame->PlayGameSound('E', 21, 0);
		}
		else
		{
			cTxt = std::format(NOTIFYMSG_CHARISMA_DOWN, iPrevChar - pGame->m_pPlayer->m_iCharisma);
			pGame->AddEventList(cTxt.c_str(), 10);
		}
	}

	void HandleHunger(CGame* pGame, char* pData)
	{
		char cHLv;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyHunger>(
			pData, sizeof(hb::net::PacketNotifyHunger));
		if (!pkt) return;
		pGame->m_pPlayer->m_iHungerStatus = pkt->hunger;

		cHLv = pGame->m_pPlayer->m_iHungerStatus;
		if ((cHLv <= 40) && (cHLv > 30)) pGame->AddEventList(NOTIFYMSG_HUNGER1, 10);
		if ((cHLv <= 25) && (cHLv > 20)) pGame->AddEventList(NOTIFYMSG_HUNGER2, 10);
		if ((cHLv <= 20) && (cHLv > 15)) pGame->AddEventList(NOTIFYMSG_HUNGER3, 10);
		if ((cHLv <= 15) && (cHLv > 10)) pGame->AddEventList(NOTIFYMSG_HUNGER4, 10);
		if ((cHLv <= 10) && (cHLv >= 0)) pGame->AddEventList(NOTIFYMSG_HUNGER5, 10);
	}

	void HandlePlayerProfile(CGame* pGame, char* pData)
	{
		std::string cTemp;
		int i;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyPlayerProfile>(
			pData, sizeof(hb::net::PacketNotifyPlayerProfile));
		if (!pkt) return;
		cTemp = pkt->text;
		for (i = 0; i < 500; i++)
			if (cTemp[i] == '_') cTemp[i] = ' ';
		pGame->AddEventList(cTemp.c_str(), 10);
	}

	void HandlePlayerStatus(CGame* pGame, bool bOnGame, char* pData)
	{
		char cName[12]{}, cMapName[12]{};
		std::string cTxt;
		uint16_t dx = 1, dy = 1;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyPlayerStatus>(
			pData, sizeof(hb::net::PacketNotifyPlayerStatus));
		if (!pkt) return;
		memcpy(cName, pkt->name, sizeof(pkt->name));
		memcpy(cMapName, pkt->map_name, sizeof(pkt->map_name));
		dx = pkt->x;
		dy = pkt->y;
		if (bOnGame == true) {
			if (cMapName[0] == 0)
				cTxt = std::format(NOTIFYMSG_PLAYER_STATUS1, cName);
			else cTxt = std::format(NOTIFYMSG_PLAYER_STATUS2, cName, cMapName, dx, dy);
		}
		else cTxt = std::format(NOTIFYMSG_PLAYER_STATUS3, cName);
		pGame->AddEventList(cTxt.c_str(), 10);
	}

	void HandleWhisperMode(CGame* pGame, bool bActive, char* pData)
	{
		char cName[12]{};
		std::string cTxt;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyWhisperMode>(
			pData, sizeof(hb::net::PacketNotifyWhisperMode));
		if (!pkt) return;
		memcpy(cName, pkt->name, sizeof(pkt->name));
		if (bActive == true)
		{
			cTxt = std::format(NOTIFYMSG_WHISPERMODE1, cName);
			ChatManager::Get().AddWhisperTarget(cName);
		}
		else cTxt = NOTIFYMSG_WHISPERMODE2;

		pGame->AddEventList(cTxt.c_str(), 10);
	}

	void HandlePlayerShutUp(CGame* pGame, char* pData)
	{
		char cName[12]{};
		std::string cTxt;
		WORD wTime;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyPlayerShutUp>(
			pData, sizeof(hb::net::PacketNotifyPlayerShutUp));
		if (!pkt) return;
		wTime = pkt->time;
		memcpy(cName, pkt->name, sizeof(pkt->name));
		if (pGame->m_pPlayer->m_cPlayerName == std::string_view(cName, strnlen(cName, hb::shared::limits::CharNameLen)))
			cTxt = std::format(NOTIFYMSG_PLAYER_SHUTUP1, wTime);
		else cTxt = std::format(NOTIFYMSG_PLAYER_SHUTUP2, cName, wTime);

		pGame->AddEventList(cTxt.c_str(), 10);
	}

	void HandleRatingPlayer(CGame* pGame, char* pData)
	{
		char cName[12]{};
		std::string cTxt;
		uint16_t cValue;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyRatingPlayer>(
			pData, sizeof(hb::net::PacketNotifyRatingPlayer));
		if (!pkt) return;
		cValue = pkt->result;
		memcpy(cName, pkt->name, sizeof(pkt->name));
		if (pGame->m_pPlayer->m_cPlayerName == std::string_view(cName, strnlen(cName, hb::shared::limits::CharNameLen)))
		{
			if (cValue == 1)
			{
				cTxt = NOTIFYMSG_RATING_PLAYER1;
				pGame->PlayGameSound('E', 23, 0);
			}
		}
		else
		{
			if (cValue == 1)
				cTxt = std::format(NOTIFYMSG_RATING_PLAYER2, cName);
			else cTxt = std::format(NOTIFYMSG_RATING_PLAYER3, cName);
		}
		pGame->AddEventList(cTxt.c_str(), 10);
	}

	void HandleCannotRating(CGame* pGame, char* pData)
	{
		std::string cTxt;

		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyCannotRating>(
			pData, sizeof(hb::net::PacketNotifyCannotRating));
		if (!pkt) return;
		const auto wTime = pkt->time_left;

		if (wTime == 0) cTxt = NOTIFYMSG_CANNOT_RATING1;
		else cTxt = std::format(NOTIFYMSG_CANNOT_RATING2, wTime * 3);
		pGame->AddEventList(cTxt.c_str(), 10);
	}
}
