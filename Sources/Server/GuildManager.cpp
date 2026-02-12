// GuildManager.cpp: Handles server-side guild operations.
// Extracted from Game.cpp (Phase B4).

#include "GuildManager.h"
#include "Game.h"
#include "Packet/SharedPackets.h"
#include "Platform.h"
#include <cstdio>
#include <cstring>

using namespace hb::shared::net;
using namespace hb::shared::action;
namespace sock = hb::shared::net::socket;

extern char G_cTxt[512];
extern void PutLogList(char* cStr);

void GuildManager::ResponseCreateNewGuildHandler(char* pData, int iType)
{

	uint16_t wResult;
	char cCharName[hb::shared::limits::CharNameLen], cTxt[120];
	int iRet;

	std::memset(cCharName, 0, sizeof(cCharName));
	memcpy(cCharName, pData, hb::shared::limits::CharNameLen - 1);

	for(int i = 1; i < hb::server::config::MaxClients; i++)
		if ((m_pGame->m_pClientList[i] != 0) && (_strnicmp(m_pGame->m_pClientList[i]->m_cCharName, cCharName, hb::shared::limits::CharNameLen - 1) == 0) &&
			(m_pGame->m_pClientList[i]->m_iLevel >= 20) && (m_pGame->m_pClientList[i]->m_iCharisma >= 20)) {

			switch (iType) {
			case 1: // LogResMsg::Confirm
				wResult = MsgType::Confirm;
				m_pGame->m_pClientList[i]->m_iGuildRank = 0;
				std::snprintf(cTxt, sizeof(cTxt), "(!) New guild(%s) creation success! : character(%s)", m_pGame->m_pClientList[i]->m_cGuildName, m_pGame->m_pClientList[i]->m_cCharName);
				PutLogList(cTxt);
				break;

			case 0: // LogResMsg::Reject
				wResult = MsgType::Reject;
				std::memset(m_pGame->m_pClientList[i]->m_cGuildName, 0, sizeof(m_pGame->m_pClientList[i]->m_cGuildName));
				memcpy(m_pGame->m_pClientList[i]->m_cGuildName, "NONE", 4);
				m_pGame->m_pClientList[i]->m_iGuildRank = -1;
				m_pGame->m_pClientList[i]->m_iGuildGUID = -1;
				std::snprintf(cTxt, sizeof(cTxt), "(!) New guild(%s) creation Fail! : character(%s)", m_pGame->m_pClientList[i]->m_cGuildName, m_pGame->m_pClientList[i]->m_cCharName);
				PutLogList(cTxt);
				break;
			}

			hb::net::PacketHeader pkt{};
			pkt.msg_id = MsgId::ResponseCreateNewGuild;
			pkt.msg_type = wResult;

			iRet = m_pGame->m_pClientList[i]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
			switch (iRet) {
			case sock::Event::QueueFull:
			case sock::Event::SocketError:
			case sock::Event::CriticalError:
			case sock::Event::SocketClosed:
				m_pGame->DeleteClient(i, true, true);
				return;
			}

			return;
		}

	std::snprintf(cTxt, sizeof(cTxt), "(!)Non-existing player data received from Log server(2): CharName(%s)", cCharName);
	PutLogList(cTxt);
}

void GuildManager::RequestCreateNewGuildHandler(int iClientH, char* pData, size_t dwMsgSize)
{
	char cGuildName[21], cTxt[120], cData[100];
	int     iRet;
	SYSTEMTIME SysTime;

	if (m_pGame->m_pClientList[iClientH] == 0) return;
	if (m_pGame->m_pClientList[iClientH]->m_bIsInitComplete == false) return;
	if (m_pGame->m_bIsCrusadeMode) return;

	const auto* pkt = hb::net::PacketCast<hb::net::PacketRequestGuildAction>(
		pData, sizeof(hb::net::PacketRequestGuildAction));
	if (!pkt) return;
	std::memset(cGuildName, 0, sizeof(cGuildName));
	memcpy(cGuildName, pkt->guild, sizeof(pkt->guild));

	if (m_pGame->m_pClientList[iClientH]->m_iGuildRank != -1) {
		std::snprintf(cTxt, sizeof(cTxt), "(!)Cannot create guild! Already guild member.: CharName(%s)", m_pGame->m_pClientList[iClientH]->m_cCharName);
		PutLogList(cTxt);
	}
	else {
		if ((m_pGame->m_pClientList[iClientH]->m_iLevel < 20) || (m_pGame->m_pClientList[iClientH]->m_iCharisma < 20) ||
			(memcmp(m_pGame->m_pClientList[iClientH]->m_cLocation, "NONE", 4) == 0) ||
			(memcmp(m_pGame->m_pClientList[iClientH]->m_cLocation, m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->m_cLocationName, 10) != 0)) { // v1.4
			std::memset(cData, 0, sizeof(cData));

			hb::net::PacketHeader pkt{};
			pkt.msg_id = MsgId::ResponseCreateNewGuild;
			pkt.msg_type = MsgType::Reject;

			iRet = m_pGame->m_pClientList[iClientH]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
			switch (iRet) {
			case sock::Event::QueueFull:
			case sock::Event::SocketError:
			case sock::Event::CriticalError:
			case sock::Event::SocketClosed:
				m_pGame->DeleteClient(iClientH, true, true);
				return;
			}
		}
		else {
			std::memset(m_pGame->m_pClientList[iClientH]->m_cGuildName, 0, sizeof(m_pGame->m_pClientList[iClientH]->m_cGuildName));
			strcpy(m_pGame->m_pClientList[iClientH]->m_cGuildName, cGuildName);
			std::memset(m_pGame->m_pClientList[iClientH]->m_cLocation, 0, sizeof(m_pGame->m_pClientList[iClientH]->m_cLocation));
			strcpy(m_pGame->m_pClientList[iClientH]->m_cLocation, m_pGame->m_pMapList[m_pGame->m_pClientList[iClientH]->m_cMapIndex]->m_cLocationName);
			GetLocalTime(&SysTime);
			m_pGame->m_pClientList[iClientH]->m_iGuildGUID = (int)(SysTime.wYear + SysTime.wMonth + SysTime.wDay + SysTime.wHour + SysTime.wMinute + GameClock::GetTimeMS());

			hb::net::GuildCreatePayload guildData{};
			std::memcpy(guildData.char_name, m_pGame->m_pClientList[iClientH]->m_cCharName, sizeof(guildData.char_name));
			std::memcpy(guildData.guild_name, m_pGame->m_pClientList[iClientH]->m_cGuildName, sizeof(guildData.guild_name));
			std::memcpy(guildData.location, m_pGame->m_pClientList[iClientH]->m_cLocation, sizeof(guildData.location));
			guildData.guild_guid = static_cast<uint32_t>(m_pGame->m_pClientList[iClientH]->m_iGuildGUID);
			RequestCreateNewGuild(iClientH, reinterpret_cast<char*>(&guildData));
		}
	}
}

void GuildManager::RequestDisbandGuildHandler(int iClientH, char* pData, size_t dwMsgSize)
{
	char cGuildName[21], cTxt[120];

	if (m_pGame->m_bIsCrusadeMode) return;

	const auto* pkt = hb::net::PacketCast<hb::net::PacketRequestGuildAction>(
		pData, sizeof(hb::net::PacketRequestGuildAction));
	if (!pkt) return;
	std::memset(cGuildName, 0, sizeof(cGuildName));

	memcpy(cGuildName, pkt->guild, sizeof(pkt->guild));

	if ((m_pGame->m_pClientList[iClientH]->m_iGuildRank != 0) || (memcmp(m_pGame->m_pClientList[iClientH]->m_cGuildName, cGuildName, 20) != 0)) {
		std::snprintf(cTxt, sizeof(cTxt), "(!)Cannot Disband guild! Not guildmaster.: CharName(%s)", m_pGame->m_pClientList[iClientH]->m_cCharName);
		PutLogList(cTxt);
	}
	else {
		hb::net::GuildDisbandPayload disbandData{};
		std::memcpy(disbandData.char_name, m_pGame->m_pClientList[iClientH]->m_cCharName, sizeof(disbandData.char_name));
		std::memcpy(disbandData.guild_name, m_pGame->m_pClientList[iClientH]->m_cGuildName, sizeof(disbandData.guild_name));
		RequestDisbandGuild(iClientH, reinterpret_cast<char*>(&disbandData));
	}
}

void GuildManager::ResponseDisbandGuildHandler(char* pData, int iType)
{

	uint16_t wResult;
	char cCharName[hb::shared::limits::CharNameLen], cTxt[120];
	int iRet;

	std::memset(cCharName, 0, sizeof(cCharName));
	memcpy(cCharName, pData, hb::shared::limits::CharNameLen - 1);

	for(int i = 1; i < hb::server::config::MaxClients; i++)
		if ((m_pGame->m_pClientList[i] != 0) && (_strnicmp(m_pGame->m_pClientList[i]->m_cCharName, cCharName, hb::shared::limits::CharNameLen - 1) == 0)) {

			switch (iType) {
			case 1: // LogResMsg::Confirm
				wResult = MsgType::Confirm;
				std::snprintf(cTxt, sizeof(cTxt), "(!) Disband guild(%s) success! : character(%s)", m_pGame->m_pClientList[i]->m_cGuildName, m_pGame->m_pClientList[i]->m_cCharName);
				PutLogList(cTxt);

				SendGuildMsg(i, Notify::GuildDisbanded, 0, 0, 0);

				std::memset(m_pGame->m_pClientList[i]->m_cGuildName, 0, sizeof(m_pGame->m_pClientList[i]->m_cGuildName));
				memcpy(m_pGame->m_pClientList[i]->m_cGuildName, "NONE", 4);
				m_pGame->m_pClientList[i]->m_iGuildRank = -1;
				m_pGame->m_pClientList[i]->m_iGuildGUID = -1;
				break;

			case 0: // LogResMsg::Reject
				wResult = MsgType::Reject;
				std::snprintf(cTxt, sizeof(cTxt), "(!) Disband guild(%s) Fail! : character(%s)", m_pGame->m_pClientList[i]->m_cGuildName, m_pGame->m_pClientList[i]->m_cCharName);
				PutLogList(cTxt);
				break;
			}

			hb::net::PacketHeader pkt{};
			pkt.msg_id = MsgId::ResponseDisbandGuild;
			pkt.msg_type = wResult;

			iRet = m_pGame->m_pClientList[i]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
			switch (iRet) {
			case sock::Event::QueueFull:
			case sock::Event::SocketError:
			case sock::Event::CriticalError:
			case sock::Event::SocketClosed:
				m_pGame->DeleteClient(i, true, true);
				return;
			}
			return;
		}

	std::snprintf(cTxt, sizeof(cTxt), "(!)Non-existing player data received from Log server(2): CharName(%s)", cCharName);
	PutLogList(cTxt);
}

void GuildManager::JoinGuildApproveHandler(int iClientH, const char* pName)
{

	bool bIsExist = false;

	if (m_pGame->m_pClientList[iClientH] == 0) return;
	if (m_pGame->m_pClientList[iClientH]->m_bIsInitComplete == false) return;

	for(int i = 1; i < hb::server::config::MaxClients; i++)
		if ((m_pGame->m_pClientList[i] != 0) && (_strnicmp(m_pGame->m_pClientList[i]->m_cCharName, pName, hb::shared::limits::CharNameLen - 1) == 0)) {
			if (memcmp(m_pGame->m_pClientList[i]->m_cLocation, m_pGame->m_pClientList[iClientH]->m_cLocation, 10) != 0) return;

			std::memset(m_pGame->m_pClientList[i]->m_cGuildName, 0, sizeof(m_pGame->m_pClientList[i]->m_cGuildName));
			strcpy(m_pGame->m_pClientList[i]->m_cGuildName, m_pGame->m_pClientList[iClientH]->m_cGuildName);

			// GUID.
			m_pGame->m_pClientList[i]->m_iGuildGUID = m_pGame->m_pClientList[iClientH]->m_iGuildGUID;

			std::memset(m_pGame->m_pClientList[i]->m_cLocation, 0, sizeof(m_pGame->m_pClientList[i]->m_cLocation));
			strcpy(m_pGame->m_pClientList[i]->m_cLocation, m_pGame->m_pClientList[iClientH]->m_cLocation);

			m_pGame->m_pClientList[i]->m_iGuildRank = m_pGame->m_iStartingGuildRank;

			m_pGame->SendNotifyMsg(iClientH, i, CommonType::JoinGuildApprove, 0, 0, 0, 0);

			m_pGame->SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventMotion, Type::NullAction, 0, 0, 0);

			SendGuildMsg(i, Notify::NewGuildsman, 0, 0, 0);

			return;
		}

}

void GuildManager::JoinGuildRejectHandler(int iClientH, const char* pName)
{


	if (m_pGame->m_pClientList[iClientH] == 0) return;
	if (m_pGame->m_pClientList[iClientH]->m_bIsInitComplete == false) return;

	for(int i = 1; i < hb::server::config::MaxClients; i++)
		if ((m_pGame->m_pClientList[i] != 0) && (_strnicmp(m_pGame->m_pClientList[i]->m_cCharName, pName, hb::shared::limits::CharNameLen - 1) == 0)) {

			m_pGame->SendNotifyMsg(iClientH, i, CommonType::JoinGuildReject, 0, 0, 0, 0);
			return;
		}

}

void GuildManager::DismissGuildApproveHandler(int iClientH, const char* pName)
{


	if (m_pGame->m_pClientList[iClientH] == 0) return;
	if (m_pGame->m_pClientList[iClientH]->m_bIsInitComplete == false) return;
	for(int i = 1; i < hb::server::config::MaxClients; i++)
		if ((m_pGame->m_pClientList[i] != 0) && (_strnicmp(m_pGame->m_pClientList[i]->m_cCharName, pName, hb::shared::limits::CharNameLen - 1) == 0)) {

			SendGuildMsg(i, Notify::DismissGuildsman, 0, 0, 0);

			std::memset(m_pGame->m_pClientList[i]->m_cGuildName, 0, sizeof(m_pGame->m_pClientList[i]->m_cGuildName));
			strcpy(m_pGame->m_pClientList[i]->m_cGuildName, "NONE");
			m_pGame->m_pClientList[i]->m_iGuildRank = -1;
			m_pGame->m_pClientList[i]->m_iGuildGUID = -1;

			m_pGame->SendNotifyMsg(iClientH, i, CommonType::DismissGuildApprove, 0, 0, 0, 0);

			m_pGame->SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventMotion, Type::NullAction, 0, 0, 0);
			return;
		}

}

void GuildManager::DismissGuildRejectHandler(int iClientH, const char* pName)
{


	if (m_pGame->m_pClientList[iClientH] == 0) return;
	if (m_pGame->m_pClientList[iClientH]->m_bIsInitComplete == false) return;

	for(int i = 1; i < hb::server::config::MaxClients; i++)
		if ((m_pGame->m_pClientList[i] != 0) && (_strnicmp(m_pGame->m_pClientList[i]->m_cCharName, pName, hb::shared::limits::CharNameLen - 1) == 0)) {

			m_pGame->SendNotifyMsg(iClientH, i, CommonType::DismissGuildReject, 0, 0, 0, 0);
			return;
		}

}

void GuildManager::SendGuildMsg(int iClientH, uint16_t wNotifyMsgType, short sV1, short sV2, char* pString)
{
	int iRet;

	if (m_pGame->m_pClientList[iClientH] == 0) return;
	if (m_pGame->m_pClientList[iClientH]->m_bIsInitComplete == false) return;

	for(int i = 0; i < hb::server::config::MaxClients; i++)
		if ((m_pGame->m_pClientList[i] != 0) &&
			(memcmp(m_pGame->m_pClientList[i]->m_cGuildName, m_pGame->m_pClientList[iClientH]->m_cGuildName, 20) == 0)) {

			switch (wNotifyMsgType) {
			case Notify::GuildDisbanded:
				if (i == iClientH) break;
				{
					hb::net::PacketNotifyGuildDisbanded pkt{};
					pkt.header.msg_id = MsgId::Notify;
					pkt.header.msg_type = wNotifyMsgType;
					memcpy(pkt.guild_name, m_pGame->m_pClientList[iClientH]->m_cGuildName, sizeof(pkt.guild_name));
					iRet = m_pGame->m_pClientList[i]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
				}
				std::memset(m_pGame->m_pClientList[i]->m_cGuildName, 0, sizeof(m_pGame->m_pClientList[i]->m_cGuildName));
				strcpy(m_pGame->m_pClientList[i]->m_cGuildName, "NONE");
				m_pGame->m_pClientList[i]->m_iGuildRank = -1;
				m_pGame->m_pClientList[i]->m_iGuildGUID = -1;
				break;

			case Notify::EventMsgString:
			{
				hb::net::PacketWriter writer;
				writer.Reserve(sizeof(hb::net::PacketHeader) + 256);

				auto* header = writer.Append<hb::net::PacketHeader>();
				header->msg_id = MsgId::Notify;
				header->msg_type = wNotifyMsgType;

				if (pString != 0) {
					const std::size_t len = std::strlen(pString);
					writer.AppendBytes(pString, len);
				}
				writer.AppendBytes("", 1);

				iRet = m_pGame->m_pClientList[i]->m_pXSock->iSendMsg(writer.Data(), static_cast<int>(writer.Size()));
			}
			break;

			case Notify::NewGuildsman:
			{
				hb::net::PacketNotifyNewGuildsMan pkt{};
				pkt.header.msg_id = MsgId::Notify;
				pkt.header.msg_type = wNotifyMsgType;
				memcpy(pkt.name, m_pGame->m_pClientList[iClientH]->m_cCharName, sizeof(pkt.name));
				iRet = m_pGame->m_pClientList[i]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
			}
			break;

			case Notify::DismissGuildsman:
			{
				hb::net::PacketNotifyDismissGuildsMan pkt{};
				pkt.header.msg_id = MsgId::Notify;
				pkt.header.msg_type = wNotifyMsgType;
				memcpy(pkt.name, m_pGame->m_pClientList[iClientH]->m_cCharName, sizeof(pkt.name));
				iRet = m_pGame->m_pClientList[i]->m_pXSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
			}
			break;
			}

			switch (iRet) {
			case sock::Event::QueueFull:
			case sock::Event::SocketError:
			case sock::Event::CriticalError:
			case sock::Event::SocketClosed:
				m_pGame->DeleteClient(i, true, true);
				return;
			}
		}

}

void GuildManager::GuildNotifyHandler(char* pData, size_t dwMsgSize)
{
	char* cp, cCharName[hb::shared::limits::CharNameLen], cGuildName[21];

	std::memset(cCharName, 0, sizeof(cCharName));
	std::memset(cGuildName, 0, sizeof(cGuildName));

	const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(pData, sizeof(hb::net::PacketHeader));
	if (!header) return;
	cp = (char*)(pData + sizeof(hb::net::PacketHeader));

	memcpy(cCharName, cp, hb::shared::limits::CharNameLen);
	cp += hb::shared::limits::CharNameLen;

	memcpy(cGuildName, cp, hb::shared::limits::GuildNameLen);
	cp += hb::shared::limits::GuildNameLen;

}

void GuildManager::UserCommand_BanGuildsman(int iClientH, char* pData, size_t dwMsgSize)
{
	char   seps[] = "= \t\r\n";
	char* token, cTargetName[11], cBuff[256];


	if (m_pGame->m_pClientList[iClientH] == 0) return;
	if ((dwMsgSize) <= 0) return;

	if (m_pGame->m_pClientList[iClientH]->m_iGuildRank != 0) {
		m_pGame->SendNotifyMsg(0, iClientH, Notify::NoGuildMasterLevel, 0, 0, 0, 0);
		return;
	}

	std::memset(cTargetName, 0, sizeof(cTargetName));
	std::memset(cBuff, 0, sizeof(cBuff));
	memcpy(cBuff, pData, dwMsgSize);

	token = strtok(cBuff, seps);
	token = strtok(NULL, seps);

	if (token != 0) {
		if (strlen(token) > hb::shared::limits::CharNameLen - 1)
			memcpy(cTargetName, token, hb::shared::limits::CharNameLen - 1);
		else memcpy(cTargetName, token, strlen(token));

		for(int i = 1; i < hb::server::config::MaxClients; i++)
			if ((m_pGame->m_pClientList[i] != 0) && (_strnicmp(m_pGame->m_pClientList[i]->m_cCharName, cTargetName, hb::shared::limits::CharNameLen - 1) == 0)) {

				if (memcmp(m_pGame->m_pClientList[iClientH]->m_cGuildName, m_pGame->m_pClientList[i]->m_cGuildName, 20) != 0) {

					m_pGame->SendNotifyMsg(0, iClientH, Notify::CannotBanGuildman, 0, 0, 0, 0);
					return;
				}

				SendGuildMsg(i, Notify::DismissGuildsman, 0, 0, 0);

				std::memset(m_pGame->m_pClientList[i]->m_cGuildName, 0, sizeof(m_pGame->m_pClientList[i]->m_cGuildName));
				strcpy(m_pGame->m_pClientList[i]->m_cGuildName, "NONE");
				m_pGame->m_pClientList[i]->m_iGuildRank = -1;
				m_pGame->m_pClientList[i]->m_iGuildGUID = -1;

				m_pGame->SendNotifyMsg(0, iClientH, Notify::SuccessBanGuildman, 0, 0, 0, 0);

				m_pGame->SendNotifyMsg(iClientH, i, CommonType::BanGuild, 0, 0, 0, 0);

				m_pGame->SendEventToNearClient_TypeA(i, hb::shared::owner_class::Player, MsgId::EventMotion, Type::NullAction, 0, 0, 0);

				return;
			}
		m_pGame->SendNotifyMsg(0, iClientH, Notify::PlayerNotOnGame, 0, 0, 0, cTargetName);
	}

	return;
}

void GuildManager::UserCommand_DissmissGuild(int iClientH, char* pData, size_t dwMsgSize)
{

}

void GuildManager::RequestCreateNewGuild(int iClientH, char* pData)
{
	char cFileName[255];
	char cTxt[500];
	char cTxt2[100];
	char cGuildMasterName[hb::shared::limits::CharNameLen], cGuildLocation[11], cDir[255], cGuildName[21];
	uint32_t dwGuildGUID;
	SYSTEMTIME SysTime;
	FILE* pFile;

	if (m_pGame->m_pClientList[iClientH] == 0) return;
	std::memset(cFileName, 0, sizeof(cFileName));
	std::memset(cTxt, 0, sizeof(cTxt));
	std::memset(cTxt2, 0, sizeof(cTxt2));
	std::memset(cDir, 0, sizeof(cDir));
	std::memset(cGuildMasterName, 0, sizeof(cGuildMasterName));
	std::memset(cGuildName, 0, sizeof(cGuildName));
	std::memset(cGuildLocation, 0, sizeof(cGuildLocation));

	const auto& guildData = *reinterpret_cast<const hb::net::GuildCreatePayload*>(pData);
	std::memcpy(cGuildMasterName, guildData.char_name, sizeof(guildData.char_name));
	std::memcpy(cGuildName, guildData.guild_name, sizeof(guildData.guild_name));
	std::memcpy(cGuildLocation, guildData.location, sizeof(guildData.location));
	dwGuildGUID = guildData.guild_guid;

	strcat(cFileName, "Guilds/");
	std::snprintf(cTxt2, sizeof(cTxt2), "AscII%d", *cGuildName);
	strcat(cFileName, cTxt2);
	strcat(cDir, cFileName);
	strcat(cFileName, "/");
	strcat(cFileName, cGuildName);
	strcat(cFileName, ".txt");

	_mkdir("Guilds");
	_mkdir(cDir);

	pFile = fopen(cFileName, "rt");
	if (pFile != 0) {
		std::snprintf(cTxt2, sizeof(cTxt2), "(X) Cannot create new guild - Already existing guild name: Name(%s)", cFileName);
		PutLogList(cTxt2);

		ResponseCreateNewGuildHandler(cGuildMasterName, 0);
		fclose(pFile);
	}
	else {
		pFile = fopen(cFileName, "wt");
		if (pFile == 0) {
			std::snprintf(cTxt2, sizeof(cTxt2), "(X) Cannot create new guild - cannot create file : Name(%s)", cFileName);
			PutLogList(cTxt2);

			ResponseCreateNewGuildHandler(cGuildMasterName, 0);
		}
		else {
			std::snprintf(cTxt2, sizeof(cTxt2), "(O) New guild created : Name(%s)", cFileName);
			PutLogList(cTxt2);

			std::memset(cTxt2, 0, sizeof(cTxt2));
			std::memset(cTxt, 0, sizeof(cTxt));
			GetLocalTime(&SysTime);

			std::snprintf(cTxt, sizeof(cTxt), ";Guild file - Updated %4d/%2d/%2d/%2d/%2d", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute);
			strcat(cTxt, "\n");
			strcat(cTxt, ";Just created\n\n");

			strcat(cTxt, "[GUILD-INFO]\n\n");

			strcat(cTxt, "guildmaster-name     = ");
			strcat(cTxt, cGuildMasterName);
			strcat(cTxt, "\n");

			strcat(cTxt, "guild-GUID           = ");
			std::snprintf(cTxt2, sizeof(cTxt2), "%d", dwGuildGUID);
			strcat(cTxt, cTxt2);
			strcat(cTxt, "\n");

			strcat(cTxt, "guild-location       = ");
			strcat(cTxt, cGuildLocation);
			strcat(cTxt, "\n\n");

			strcat(cTxt, "[GUILDSMAN]\n\n");

			fwrite(cTxt, 1, strlen(cTxt), pFile);

			ResponseCreateNewGuildHandler(cGuildMasterName, 1);
			fclose(pFile);
		}
	}
}

void GuildManager::RequestDisbandGuild(int iClientH, char* pData)
{
	char cTemp[500];
	char cFileName[255], cTxt[100], cDir[100];
	char cGuildMasterName[hb::shared::limits::CharNameLen], cGuildName[21];
	FILE* pFile;

	if (m_pGame->m_pClientList[iClientH] == 0) return;
	std::memset(cFileName, 0, sizeof(cFileName));
	std::memset(cTxt, 0, sizeof(cTxt));
	std::memset(cDir, 0, sizeof(cDir));
	std::memset(cTemp, 0, sizeof(cTemp));
	std::memset(cGuildMasterName, 0, sizeof(cGuildMasterName));
	std::memset(cGuildName, 0, sizeof(cGuildName));

	const auto& disbandData = *reinterpret_cast<const hb::net::GuildDisbandPayload*>(pData);
	std::memcpy(cGuildMasterName, disbandData.char_name, sizeof(disbandData.char_name));
	std::memcpy(cGuildName, disbandData.guild_name, sizeof(disbandData.guild_name));

	strcat(cFileName, "Guilds/");
	std::snprintf(cTxt, sizeof(cTxt), "AscII%d", *cGuildName);
	strcat(cFileName, cTxt);
	strcat(cDir, cFileName);
	strcat(cFileName, "/");
	strcat(cFileName, cGuildName);
	strcat(cFileName, ".txt");

	pFile = fopen(cFileName, "rt");
	if (pFile != 0) {
		fclose(pFile);
		std::snprintf(G_cTxt, sizeof(G_cTxt), "(O) Disband Guild - Deleting guild file... : Name(%s)", cFileName);
		PutLogList(G_cTxt);
		if (DeleteFile(cFileName) != 0) {
			ResponseDisbandGuildHandler(cGuildMasterName, 1);
		}
		else {

			ResponseDisbandGuildHandler(cGuildMasterName, 0);
		}
	}
	else {

		ResponseDisbandGuildHandler(cGuildMasterName, 0);
	}
}

void GuildManager::RequestGuildNameHandler(int iClientH, int iObjectID, int iIndex)
{
	if (m_pGame->m_pClientList[iClientH] == 0) return;
	if ((iObjectID <= 0) || (iObjectID >= hb::server::config::MaxClients)) return;

	if (m_pGame->m_pClientList[iObjectID] == 0) {
		// Object .

	}
	else {
		m_pGame->SendNotifyMsg(0, iClientH, Notify::ReqGuildNameAnswer, m_pGame->m_pClientList[iObjectID]->m_iGuildRank, iIndex, 0, m_pGame->m_pClientList[iObjectID]->m_cGuildName);
	}
}
