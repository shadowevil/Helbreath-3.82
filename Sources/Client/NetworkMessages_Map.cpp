#include "Game.h"
#include "NetworkMessageManager.h"
#include "Packet/SharedPackets.h"
#include "lan_eng.h"
#include "DialogBoxIDs.h"
#include <cstdio>
#include <cstring>
#include "PlatformCompat.h"
#include <format>
#include <string>

namespace NetworkMessageHandlers {
	void HandleMapStatusNext(CGame* pGame, char* pData)
	{
		pGame->AddMapStatusInfo(pData, false);
	}

	void HandleMapStatusLast(CGame* pGame, char* pData)
	{
		pGame->AddMapStatusInfo(pData, true);
	}

	void HandleLockedMap(CGame* pGame, char* pData)
	{
		int sV1;
		char cTemp[120], cTxt[120]{};
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyLockedMap>(
			pData, sizeof(hb::net::PacketNotifyLockedMap));
		if (!pkt) return;
		sV1 = pkt->seconds_left;
		std::memset(cTemp, 0, sizeof(cTemp));
		memcpy(cTxt, pkt->map_name, sizeof(pkt->map_name));

		pGame->GetOfficialMapName(cTxt, cTemp);
		std::string msgBuf;
		msgBuf = std::format(NOTIFY_MSG_HANDLER3, sV1, cTemp);
		pGame->SetTopMsg(msgBuf.c_str(), 10);
		pGame->PlayGameSound('E', 25, 0, 0);
	}

	void HandleShowMap(CGame* pGame, char* pData)
	{
		WORD w1, w2;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyShowMap>(
			pData, sizeof(hb::net::PacketNotifyShowMap));
		if (!pkt) return;
		w1 = pkt->map_id;
		w2 = pkt->map_type;
		if (w2 == 0) pGame->AddEventList(NOTIFYMSG_SHOW_MAP1, 10);
		else pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::Map, 0, w1, w2 - 1);
	}
} // namespace NetworkMessageHandlers
