// GuildManager.cpp: Handles client-side guild network messages.
// Extracted from NetworkMessages_Guild.cpp (Phase B4).

#include "GuildManager.h"
#include "Game.h"
#include "Packet/SharedPackets.h"
#include "lan_eng.h"
#include "PlatformCompat.h"
#include <cstdio>
#include <cstring>
#include <string_view>
#include <format>
#include <string>
#include <algorithm>

using namespace hb::shared::net;

void GuildManager::UpdateLocationFlags(CGame* pGame, const char* cLocation)
{
	if (memcmp(cLocation, "aresden", 7) == 0)
	{
		pGame->m_pPlayer->m_bAresden = true;
		pGame->m_pPlayer->m_bCitizen = true;
		pGame->m_pPlayer->m_bHunter = false;
	}
	else if (memcmp(cLocation, "arehunter", 9) == 0)
	{
		pGame->m_pPlayer->m_bAresden = true;
		pGame->m_pPlayer->m_bCitizen = true;
		pGame->m_pPlayer->m_bHunter = true;
	}
	else if (memcmp(cLocation, "elvine", 6) == 0)
	{
		pGame->m_pPlayer->m_bAresden = false;
		pGame->m_pPlayer->m_bCitizen = true;
		pGame->m_pPlayer->m_bHunter = false;
	}
	else if (memcmp(cLocation, "elvhunter", 9) == 0)
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
}

void GuildManager::HandleCreateNewGuildResponse(char* pData)
{
	const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(
		pData, sizeof(hb::net::PacketHeader));
	if (!header) return;
	switch (header->msg_type) {
	case MsgType::Confirm:
		m_pGame->m_pPlayer->m_iGuildRank = 0;
		m_pGame->m_dialogBoxManager.Info(DialogBoxId::GuildMenu).cMode = 3;
		break;
	case MsgType::Reject:
		m_pGame->m_pPlayer->m_iGuildRank = -1;
		m_pGame->m_dialogBoxManager.Info(DialogBoxId::GuildMenu).cMode = 4;
		break;
	}
}

void GuildManager::HandleDisbandGuildResponse(char* pData)
{
	const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(
		pData, sizeof(hb::net::PacketHeader));
	if (!header) return;
	switch (header->msg_type) {
	case MsgType::Confirm:
		m_pGame->m_pPlayer->m_iGuildRank = -1;
		m_pGame->m_dialogBoxManager.Info(DialogBoxId::GuildMenu).cMode = 7;
		break;
	case MsgType::Reject:
		m_pGame->m_dialogBoxManager.Info(DialogBoxId::GuildMenu).cMode = 8;
		break;
	}
}

void GuildManager::HandleGuildDisbanded(char* pData)
{
	char cName[24]{}, cLocation[12]{};
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyGuildDisbanded>(
		pData, sizeof(hb::net::PacketNotifyGuildDisbanded));
	if (!pkt) return;
	memcpy(cName, pkt->guild_name, sizeof(pkt->guild_name));
	memcpy(cLocation, pkt->location, sizeof(pkt->location));
	CMisc::ReplaceString(cName, '_', ' ');
	m_pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::GuildOperation, 0, 0, 0);
	m_pGame->_PutGuildOperationList(cName, 7);
	m_pGame->m_pPlayer->m_iGuildRank = -1;
	m_pGame->m_cLocation.assign(cLocation, strnlen(cLocation, hb::shared::limits::MapNameLen));
	UpdateLocationFlags(m_pGame, m_pGame->m_cLocation.c_str());
}

void GuildManager::HandleNewGuildsMan(char* pData)
{
	std::string cTxt;

	char cName[12]{};
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyNewGuildsMan>(
		pData, sizeof(hb::net::PacketNotifyNewGuildsMan));
	if (!pkt) return;
	memcpy(cName, pkt->name, sizeof(pkt->name));
	cTxt = std::format(NOTIFYMSG_NEW_GUILDMAN1, cName);
	m_pGame->AddEventList(cTxt.c_str(), 10);
	m_pGame->ClearGuildNameList();
}

void GuildManager::HandleDismissGuildsMan(char* pData)
{
	std::string cTxt;

	char cName[12]{};

	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyDismissGuildsMan>(
		pData, sizeof(hb::net::PacketNotifyDismissGuildsMan));
	if (!pkt) return;
	memcpy(cName, pkt->name, sizeof(pkt->name));

	if (m_pGame->m_pPlayer->m_cPlayerName != std::string_view(cName, strnlen(cName, hb::shared::limits::CharNameLen))) {
		cTxt = std::format(NOTIFYMSG_DISMISS_GUILDMAN1, cName);
		m_pGame->AddEventList(cTxt.c_str(), 10);
	}
	m_pGame->ClearGuildNameList();
}

void GuildManager::HandleCannotJoinMoreGuildsMan(char* pData)
{
	std::string cTxt;

	char cName[12]{};

	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyCannotJoinMoreGuildsMan>(
		pData, sizeof(hb::net::PacketNotifyCannotJoinMoreGuildsMan));
	if (!pkt) return;
	memcpy(cName, pkt->name, sizeof(pkt->name));

	cTxt = std::format(NOTIFYMSG_CANNOT_JOIN_MOREGUILDMAN1, cName);
	m_pGame->AddEventList(cTxt.c_str(), 10);
	m_pGame->AddEventList(NOTIFYMSG_CANNOT_JOIN_MOREGUILDMAN2, 10);
}

void GuildManager::HandleJoinGuildApprove(char* pData)
{
	char cName[21]{};
	short sRank;
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyJoinGuildApprove>(
		pData, sizeof(hb::net::PacketNotifyJoinGuildApprove));
	if (!pkt) return;
	memcpy(cName, pkt->guild_name, sizeof(pkt->guild_name));
	sRank = pkt->rank;
	m_pGame->m_pPlayer->m_cGuildName = cName;
	m_pGame->m_pPlayer->m_iGuildRank = sRank;
	m_pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::GuildOperation, 0, 0, 0);
	m_pGame->_PutGuildOperationList(cName, 3);
}

void GuildManager::HandleJoinGuildReject(char* pData)
{
	char cName[21]{};
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyJoinGuildReject>(
		pData, sizeof(hb::net::PacketNotifyJoinGuildReject));
	if (!pkt) return;
	memcpy(cName, pkt->guild_name, sizeof(pkt->guild_name));
	m_pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::GuildOperation, 0, 0, 0);
	m_pGame->_PutGuildOperationList(cName, 4);
}

void GuildManager::HandleDismissGuildApprove(char* pData)
{
	char cName[24]{}, cLocation[12]{};
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyDismissGuildApprove>(
		pData, sizeof(hb::net::PacketNotifyDismissGuildApprove));
	if (!pkt) return;
	memcpy(cName, pkt->guild_name, sizeof(pkt->guild_name));
	memcpy(cLocation, pkt->location, sizeof(pkt->location));
	m_pGame->m_pPlayer->m_iGuildRank = -1;
	m_pGame->m_cLocation.assign(cLocation, strnlen(cLocation, hb::shared::limits::MapNameLen));
	UpdateLocationFlags(m_pGame, m_pGame->m_cLocation.c_str());
	m_pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::GuildOperation, 0, 0, 0);
	m_pGame->_PutGuildOperationList(cName, 5);
}

void GuildManager::HandleDismissGuildReject(char* pData)
{
	char cName[21]{};
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyDismissGuildReject>(
		pData, sizeof(hb::net::PacketNotifyDismissGuildReject));
	if (!pkt) return;
	memcpy(cName, pkt->guild_name, sizeof(pkt->guild_name));
	m_pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::GuildOperation, 0, 0, 0);
	m_pGame->_PutGuildOperationList(cName, 6);
}

void GuildManager::HandleQueryJoinGuildPermission(char* pData)
{
	char cName[12]{};
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyQueryJoinGuildPermission>(
		pData, sizeof(hb::net::PacketNotifyQueryJoinGuildPermission));
	if (!pkt) return;
	memcpy(cName, pkt->name, sizeof(pkt->name));
	m_pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::GuildOperation, 0, 0, 0);
	m_pGame->_PutGuildOperationList(cName, 1);
}

void GuildManager::HandleQueryDismissGuildPermission(char* pData)
{
	char cName[12]{};
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyQueryDismissGuildPermission>(
		pData, sizeof(hb::net::PacketNotifyQueryDismissGuildPermission));
	if (!pkt) return;
	memcpy(cName, pkt->name, sizeof(pkt->name));
	m_pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::GuildOperation, 0, 0, 0);
	m_pGame->_PutGuildOperationList(cName, 2);
}

void GuildManager::HandleReqGuildNameAnswer(char* pData)
{
	short sV1, sV2;
	char cTemp[256]{};
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyReqGuildNameAnswer>(
		pData, sizeof(hb::net::PacketNotifyReqGuildNameAnswer));
	if (!pkt) return;
	sV1 = static_cast<short>(pkt->guild_rank);
	sV2 = static_cast<short>(pkt->index);
	memcpy(cTemp, pkt->guild_name, sizeof(pkt->guild_name));

	m_pGame->m_stGuildName[sV2].cGuildName = cTemp;
	m_pGame->m_stGuildName[sV2].iGuildRank = sV1;
	std::replace(m_pGame->m_stGuildName[sV2].cGuildName.begin(), m_pGame->m_stGuildName[sV2].cGuildName.end(), '_', ' ');
}

void GuildManager::HandleNoGuildMasterLevel(char* pData)
{
	m_pGame->AddEventList(NOTIFY_MSG_HANDLER59, 10);
}

void GuildManager::HandleSuccessBanGuildMan(char* pData)
{
	m_pGame->AddEventList(NOTIFY_MSG_HANDLER60, 10);
}

void GuildManager::HandleCannotBanGuildMan(char* pData)
{
	m_pGame->AddEventList(NOTIFY_MSG_HANDLER61, 10);
}
