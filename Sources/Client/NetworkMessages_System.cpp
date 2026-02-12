#include "Game.h"
#include "GameModeManager.h"
#include "AudioManager.h"
#include "WeatherManager.h"


#include "NetworkMessageManager.h"
#include "Packet/SharedPackets.h"
#include "lan_eng.h"
#include "PlatformCompat.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <format>
#include <string>

using namespace hb::shared::net;
namespace NetworkMessageHandlers {

void HandleWeatherChange(CGame* pGame, char* pData)
{
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyWhetherChange>(
		pData, sizeof(hb::net::PacketNotifyWhetherChange));
	if (!pkt) return;
	char weather_status = static_cast<char>(pkt->status);
	WeatherManager::Get().SetWeatherStatus(weather_status);

	if (weather_status != 0)
	{
		WeatherManager::Get().SetWeather(true, weather_status);
		if (weather_status >= 4 && weather_status <= 6 && AudioManager::Get().IsMusicEnabled())
			pGame->StartBGM();
	}
	else WeatherManager::Get().SetWeather(false, 0);
}

void HandleTimeChange(CGame* pGame, char* pData)
{
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyTimeChange>(
		pData, sizeof(hb::net::PacketNotifyTimeChange));
	if (!pkt) return;
	WeatherManager::Get().SetAmbientLight(static_cast<char>(pkt->sprite_alpha));
	switch (WeatherManager::Get().GetAmbientLight()) {
	case 1:	pGame->m_bIsXmas = false; pGame->PlayGameSound('E', 32, 0); break;
	case 2: pGame->m_bIsXmas = false; pGame->PlayGameSound('E', 31, 0); break;
	case 3: // Snoopy Special night with chrismas bulbs
		if (WeatherManager::Get().IsSnowing()) pGame->m_bIsXmas = true;
		else pGame->m_bIsXmas = false;
		WeatherManager::Get().SetXmas(pGame->m_bIsXmas);
		pGame->PlayGameSound('E', 31, 0);
		WeatherManager::Get().SetAmbientLight(2); break;
	}
}

void HandleNoticeMsg(CGame* pGame, char* pData)
{
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyNoticeMsg>(
		pData, sizeof(hb::net::PacketNotifyNoticeMsg));
	if (!pkt) return;
	pGame->AddEventList(pkt->text, 10);
}

void HandleStatusText(CGame* pGame, char* pData)
{
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyStatusText>(
		pData, sizeof(hb::net::PacketNotifyStatusText));
	if (!pkt) return;

	// Display floating text above the local player's head (like "* Failed! *")
	pGame->m_floatingText.AddDamageText(DamageTextType::Medium, pkt->text, pGame->m_dwCurTime,
		pGame->m_pPlayer->m_sPlayerObjectID, pGame->m_pMapData.get());
}

void HandleForceDisconn(CGame* pGame, char* pData)
{
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyForceDisconn>(
		pData, sizeof(hb::net::PacketNotifyForceDisconn));
	if (!pkt) return;
	const auto wpCount = pkt->seconds;
	pGame->m_bForceDisconn = true;
	if (pGame->m_cLogOutCount < 0 || pGame->m_cLogOutCount > 5) pGame->m_cLogOutCount = 5;
	pGame->AddEventList(NOTIFYMSG_FORCE_DISCONN1, 10);
}

void HandleSettingSuccess(CGame* pGame, char* pData)
{
	int iPrevLevel;
	std::string cTxt;
	iPrevLevel = pGame->m_pPlayer->m_iLevel;
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyLevelUp>(
		pData, sizeof(hb::net::PacketNotifyLevelUp));
	if (!pkt) return;
	pGame->m_pPlayer->m_iLevel = pkt->level;
	pGame->m_pPlayer->m_iStr = pkt->str;
	pGame->m_pPlayer->m_iVit = pkt->vit;
	pGame->m_pPlayer->m_iDex = pkt->dex;
	pGame->m_pPlayer->m_iInt = pkt->intel;
	pGame->m_pPlayer->m_iMag = pkt->mag;
	pGame->m_pPlayer->m_iCharisma = pkt->chr;
	pGame->m_pPlayer->m_playerStatus.iAttackDelay = pkt->attack_delay;
	cTxt = "Your stat has been changed.";
	pGame->AddEventList(cTxt.c_str(), 10);
	// CLEROTH - LU
	pGame->m_pPlayer->m_iLU_Point = (pGame->m_pPlayer->m_iLevel - 1) * 3 - ((pGame->m_pPlayer->m_iStr + pGame->m_pPlayer->m_iVit + pGame->m_pPlayer->m_iDex + pGame->m_pPlayer->m_iInt + pGame->m_pPlayer->m_iMag + pGame->m_pPlayer->m_iCharisma) - 70);
	pGame->m_pPlayer->m_wLU_Str = pGame->m_pPlayer->m_wLU_Vit = pGame->m_pPlayer->m_wLU_Dex = pGame->m_pPlayer->m_wLU_Int = pGame->m_pPlayer->m_wLU_Mag = pGame->m_pPlayer->m_wLU_Char = 0;
}

void HandleServerChange(CGame* pGame, char* pData)
{
	if (pGame->m_bIsServerChanging) return;

	char cWorldServerAddr[16]{};
	int iWorldServerPort;

	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyServerChange>(
		pData, sizeof(hb::net::PacketNotifyServerChange));
	if (!pkt) return;

	pGame->m_bIsServerChanging = true;


	pGame->m_cMapName.assign(pkt->map_name, strnlen(pkt->map_name, sizeof(pkt->map_name)));
	memcpy(cWorldServerAddr, pkt->log_server_addr, 15);
	iWorldServerPort = pkt->log_server_port;
	if (pGame->m_pGSock != 0)
	{
		pGame->m_pGSock.reset();
	}
	if (pGame->m_pLSock != 0)
	{
		pGame->m_pLSock.reset();
	}
	pGame->m_pLSock = std::make_unique<hb::shared::net::ASIOSocket>(pGame->m_pIOPool->GetContext(), game_limits::socket_block_limit);
	pGame->m_pLSock->bConnect(pGame->m_cLogServerAddr.c_str(), iWorldServerPort);
	pGame->m_pLSock->bInitBufferSize(hb::shared::limits::MsgBufferSize);

	pGame->m_pPlayer->m_bIsPoisoned = false;

	pGame->ChangeGameMode(GameMode::Connecting);
	pGame->m_dwConnectMode = MsgId::RequestEnterGame;
	//m_wEnterGameType = EnterGameMsg::New; //Gateway
	pGame->m_wEnterGameType = EnterGameMsg::NewToWlsButMls;
	std::snprintf(pGame->m_cMsg, sizeof(pGame->m_cMsg), "%s", "55");
}

void HandleTotalUsers(CGame* pGame, char* pData)
{
	int iTotal;
	std::string cTxt;
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyTotalUsers>(
		pData, sizeof(hb::net::PacketNotifyTotalUsers));
	if (!pkt) return;
	iTotal = pkt->total;
	cTxt = std::format(NOTIFYMSG_TOTAL_USER1, iTotal);
	pGame->AddEventList(cTxt.c_str(), 10);
}

void HandleChangePlayMode(CGame* pGame, char* pData)
{
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyChangePlayMode>(
		pData, sizeof(hb::net::PacketNotifyChangePlayMode));
	if (!pkt) return;
	pGame->m_cLocation.assign(pkt->location, strnlen(pkt->location, sizeof(pkt->location)));

	if (pGame->m_cLocation.starts_with("aresden"))
	{
		pGame->m_pPlayer->m_bAresden = true;
		pGame->m_pPlayer->m_bCitizen = true;
		pGame->m_pPlayer->m_bHunter = false;
	}
	else if (pGame->m_cLocation.starts_with("arehunter"))
	{
		pGame->m_pPlayer->m_bAresden = true;
		pGame->m_pPlayer->m_bCitizen = true;
		pGame->m_pPlayer->m_bHunter = true;
	}
	else if (pGame->m_cLocation.starts_with("elvine"))
	{
		pGame->m_pPlayer->m_bAresden = false;
		pGame->m_pPlayer->m_bCitizen = true;
		pGame->m_pPlayer->m_bHunter = false;
	}
	else if (pGame->m_cLocation.starts_with("elvhunter"))
	{
		pGame->m_pPlayer->m_bAresden = false;
		pGame->m_pPlayer->m_bCitizen = true;
		pGame->m_pPlayer->m_bHunter = true;
	}
	else
	{
		pGame->m_pPlayer->m_bAresden = true;
		pGame->m_pPlayer->m_bCitizen = false;
		pGame->m_pPlayer->m_bHunter = true;
	}
	pGame->AddEventList(DEF_MSG_GAMEMODE_CHANGED, 10);
}

void HandleForceRecallTime(CGame* pGame, char* pData)
{
	short sV1;
	std::string cTxt;
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyForceRecallTime>(
		pData, sizeof(hb::net::PacketNotifyForceRecallTime));
	if (!pkt) return;
	sV1 = static_cast<short>(pkt->seconds_left);
	if (static_cast<int>(sV1 / 20) > 0)
		cTxt = std::format(NOTIFY_MSG_FORCERECALLTIME1, static_cast<int>(sV1 / 20));
	else
		cTxt = NOTIFY_MSG_FORCERECALLTIME2;
	pGame->AddEventList(cTxt.c_str(), 10);
}

void HandleNoRecall(CGame* pGame, char* pData)
{
	pGame->AddEventList("You can not recall in this map.", 10);
}

void HandleFightZoneReserve(CGame* pGame, char* pData)
{
	std::string cTxt;
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyFightZoneReserve>(
		pData, sizeof(hb::net::PacketNotifyFightZoneReserve));
	if (!pkt) return;
	switch (pkt->result) {
	case -5:
		pGame->AddEventList(NOTIFY_MSG_HANDLER68, 10);
		break;
	case -4:
		pGame->AddEventList(NOTIFY_MSG_HANDLER69, 10);
		break;
	case -3:
		pGame->AddEventList(NOTIFY_MSG_HANDLER70, 10);
		break;
	case -2:
		pGame->m_iFightzoneNumber = 0;
		pGame->AddEventList(NOTIFY_MSG_HANDLER71, 10);
		break;
	case -1:
		pGame->m_iFightzoneNumber = pGame->m_iFightzoneNumber * -1;
		pGame->AddEventList(NOTIFY_MSG_HANDLER72, 10);
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		cTxt = std::format(NOTIFY_MSG_HANDLER73, pkt->result);
		pGame->AddEventList(cTxt.c_str(), 10);
		break;
	}
}

void HandleLoteryLost(CGame* pGame, char* pData)
{
	pGame->AddEventList(DEF_MSG_NOTIFY_LOTERY_LOST, 10);
}

void HandleNotFlagSpot(CGame* pGame, char* pData)
{
	pGame->AddEventList(NOTIFY_MSG_HANDLER45, 10);
}

void HandleNpcTalk(CGame* pGame, char* pData)
{
	pGame->NpcTalkHandler(pData);
}

void HandleTravelerLimitedLevel(CGame* pGame, char* pData)
{
	pGame->AddEventList(NOTIFY_MSG_HANDLER64, 10);
}

void HandleLimitedLevel(CGame* pGame, char* pData)
{
	pGame->AddEventList(NOTIFYMSG_LIMITED_LEVEL1, 10);
}

void HandleToBeRecalled(CGame* pGame, char* pData)
{
	pGame->AddEventList(NOTIFY_MSG_HANDLER62, 10);
}

} // namespace NetworkMessageHandlers