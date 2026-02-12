#include "Game.h"
#include "NetworkMessageManager.h"
#include "Packet/SharedPackets.h"
#include "lan_eng.h"
#include <cstdio>
#include <cstring>
#include "PlatformCompat.h"

namespace NetworkMessageHandlers {
	void HandleAngelFailed(CGame* pGame, char* pData)
	{
		int iV1;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyAngelFailed>(
			pData, sizeof(hb::net::PacketNotifyAngelFailed));
		if (!pkt) return;
		iV1 = pkt->reason;
		switch (iV1) {
		case 1:
			pGame->AddEventList(DEF_MSG_NOTIFY_ANGEL_FAILED, 10);
			break;
		case 2:
			pGame->AddEventList(DEF_MSG_NOTIFY_ANGEL_MAJESTIC, 10);
			break;
		case 3:
			pGame->AddEventList(DEF_MSG_NOTIFY_ANGEL_LOW_LVL, 10);
			break;
		}
	}

	void HandleAngelReceived(CGame* pGame, char* pData)
	{
		pGame->AddEventList(DEF_MSG_NOTIFY_ANGEL_RECEIVED, 10);
	}

	void HandleAngelicStats(CGame* pGame, char* pData)
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyAngelicStats>(
			pData, sizeof(hb::net::PacketNotifyAngelicStats));
		if (!pkt) return;
		pGame->m_pPlayer->m_iAngelicStr = pkt->str;
		pGame->m_pPlayer->m_iAngelicInt = pkt->intel;
		pGame->m_pPlayer->m_iAngelicDex = pkt->dex;
		pGame->m_pPlayer->m_iAngelicMag = pkt->mag;
	}
} // namespace NetworkMessageHandlers
