// GuildManager.cpp: Handles server-side guild operations.
// Extracted from Game.cpp (Phase B4).

#include "GuildManager.h"
#include "Game.h"
#include "Packet/SharedPackets.h"
#include "StringCompat.h"
#include "TimeUtils.h"
#include <filesystem>
#include <cstdio>
#include <cstring>
#include "Log.h"

using namespace hb::shared::net;
using namespace hb::shared::action;
namespace sock = hb::shared::net::socket;

extern char G_cTxt[512];

void GuildManager::response_create_new_guild_handler(char* data, int type)
{

	uint16_t result;
	char char_name[hb::shared::limits::CharNameLen];
	int ret;

	std::memset(char_name, 0, sizeof(char_name));
	memcpy(char_name, data, hb::shared::limits::CharNameLen - 1);

	for(int i = 1; i < hb::server::config::MaxClients; i++)
		if ((m_game->m_client_list[i] != 0) && (hb_strnicmp(m_game->m_client_list[i]->m_char_name, char_name, hb::shared::limits::CharNameLen - 1) == 0) &&
			(m_game->m_client_list[i]->m_level >= 20) && (m_game->m_client_list[i]->m_charisma >= 20)) {

			switch (type) {
			case 1: // LogResMsg::Confirm
				result = MsgType::Confirm;
				m_game->m_client_list[i]->m_guild_rank = 0;
				hb::logger::log("Guild '{}' created by {}", m_game->m_client_list[i]->m_guild_name, m_game->m_client_list[i]->m_char_name);
				break;

			case 0: // LogResMsg::Reject
				result = MsgType::Reject;
				std::memset(m_game->m_client_list[i]->m_guild_name, 0, sizeof(m_game->m_client_list[i]->m_guild_name));
				memcpy(m_game->m_client_list[i]->m_guild_name, "NONE", 4);
				m_game->m_client_list[i]->m_guild_rank = -1;
				m_game->m_client_list[i]->m_guild_guid = -1;
				hb::logger::log("Guild '{}' creation failed for {}", m_game->m_client_list[i]->m_guild_name, m_game->m_client_list[i]->m_char_name);
				break;
			}

			hb::net::PacketHeader pkt{};
			pkt.msg_id = MsgId::ResponseCreateNewGuild;
			pkt.msg_type = result;

			ret = m_game->m_client_list[i]->m_socket->send_msg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
			switch (ret) {
			case sock::Event::QueueFull:
			case sock::Event::SocketError:
			case sock::Event::CriticalError:
			case sock::Event::SocketClosed:
				m_game->delete_client(i, true, true);
				return;
			}

			return;
		}

	hb::logger::log("Non-existent player data from login server: {}", char_name);
}

void GuildManager::request_create_new_guild_handler(int client_h, char* data, size_t msg_size)
{
	char guild_name[21], msg_data[100];
	int     ret;
	hb::time::local_time SysTime{};

	if (m_game->m_client_list[client_h] == 0) return;
	if (m_game->m_client_list[client_h]->m_is_init_complete == false) return;
	if (m_game->m_is_crusade_mode) return;

	const auto* pkt = hb::net::PacketCast<hb::net::PacketRequestGuildAction>(
		data, sizeof(hb::net::PacketRequestGuildAction));
	if (!pkt) return;
	std::memset(guild_name, 0, sizeof(guild_name));
	memcpy(guild_name, pkt->guild, sizeof(pkt->guild));

	if (m_game->m_client_list[client_h]->m_guild_rank != -1) {
		hb::logger::log("Cannot create guild: already a guild member ({})", m_game->m_client_list[client_h]->m_char_name);
	}
	else {
		if ((m_game->m_client_list[client_h]->m_level < 20) || (m_game->m_client_list[client_h]->m_charisma < 20) ||
			(memcmp(m_game->m_client_list[client_h]->m_location, "NONE", 4) == 0) ||
			(memcmp(m_game->m_client_list[client_h]->m_location, m_game->m_map_list[m_game->m_client_list[client_h]->m_map_index]->m_location_name, 10) != 0)) { // v1.4
			std::memset(msg_data, 0, sizeof(msg_data));

			hb::net::PacketHeader pkt{};
			pkt.msg_id = MsgId::ResponseCreateNewGuild;
			pkt.msg_type = MsgType::Reject;

			ret = m_game->m_client_list[client_h]->m_socket->send_msg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
			switch (ret) {
			case sock::Event::QueueFull:
			case sock::Event::SocketError:
			case sock::Event::CriticalError:
			case sock::Event::SocketClosed:
				m_game->delete_client(client_h, true, true);
				return;
			}
		}
		else {
			std::memset(m_game->m_client_list[client_h]->m_guild_name, 0, sizeof(m_game->m_client_list[client_h]->m_guild_name));
			strcpy(m_game->m_client_list[client_h]->m_guild_name, guild_name);
			std::memset(m_game->m_client_list[client_h]->m_location, 0, sizeof(m_game->m_client_list[client_h]->m_location));
			strcpy(m_game->m_client_list[client_h]->m_location, m_game->m_map_list[m_game->m_client_list[client_h]->m_map_index]->m_location_name);
			SysTime = hb::time::local_time::now();
			m_game->m_client_list[client_h]->m_guild_guid = (int)(SysTime.year + SysTime.month + SysTime.day + SysTime.hour + SysTime.minute + GameClock::GetTimeMS());

			hb::net::GuildCreatePayload guildData{};
			std::memcpy(guildData.char_name, m_game->m_client_list[client_h]->m_char_name, sizeof(guildData.char_name));
			std::memcpy(guildData.guild_name, m_game->m_client_list[client_h]->m_guild_name, sizeof(guildData.guild_name));
			std::memcpy(guildData.location, m_game->m_client_list[client_h]->m_location, sizeof(guildData.location));
			guildData.guild_guid = static_cast<uint32_t>(m_game->m_client_list[client_h]->m_guild_guid);
			request_create_new_guild(client_h, reinterpret_cast<char*>(&guildData));
		}
	}
}

void GuildManager::request_disband_guild_handler(int client_h, char* data, size_t msg_size)
{
	char guild_name[21];

	if (m_game->m_is_crusade_mode) return;

	const auto* pkt = hb::net::PacketCast<hb::net::PacketRequestGuildAction>(
		data, sizeof(hb::net::PacketRequestGuildAction));
	if (!pkt) return;
	std::memset(guild_name, 0, sizeof(guild_name));

	memcpy(guild_name, pkt->guild, sizeof(pkt->guild));

	if ((m_game->m_client_list[client_h]->m_guild_rank != 0) || (memcmp(m_game->m_client_list[client_h]->m_guild_name, guild_name, 20) != 0)) {
		hb::logger::log("Cannot disband guild: not guildmaster ({})", m_game->m_client_list[client_h]->m_char_name);
	}
	else {
		hb::net::GuildDisbandPayload disbandData{};
		std::memcpy(disbandData.char_name, m_game->m_client_list[client_h]->m_char_name, sizeof(disbandData.char_name));
		std::memcpy(disbandData.guild_name, m_game->m_client_list[client_h]->m_guild_name, sizeof(disbandData.guild_name));
		request_disband_guild(client_h, reinterpret_cast<char*>(&disbandData));
	}
}

void GuildManager::response_disband_guild_handler(char* data, int type)
{

	uint16_t result;
	char char_name[hb::shared::limits::CharNameLen];
	int ret;

	std::memset(char_name, 0, sizeof(char_name));
	memcpy(char_name, data, hb::shared::limits::CharNameLen - 1);

	for(int i = 1; i < hb::server::config::MaxClients; i++)
		if ((m_game->m_client_list[i] != 0) && (hb_strnicmp(m_game->m_client_list[i]->m_char_name, char_name, hb::shared::limits::CharNameLen - 1) == 0)) {

			switch (type) {
			case 1: // LogResMsg::Confirm
				result = MsgType::Confirm;
				hb::logger::log("Guild '{}' disbanded by {}", m_game->m_client_list[i]->m_guild_name, m_game->m_client_list[i]->m_char_name);

				send_guild_msg(i, Notify::GuildDisbanded, 0, 0, 0);

				std::memset(m_game->m_client_list[i]->m_guild_name, 0, sizeof(m_game->m_client_list[i]->m_guild_name));
				memcpy(m_game->m_client_list[i]->m_guild_name, "NONE", 4);
				m_game->m_client_list[i]->m_guild_rank = -1;
				m_game->m_client_list[i]->m_guild_guid = -1;
				m_game->m_client_list[i]->m_fightzone_number = 0;
				m_game->m_client_list[i]->m_reserve_time = 0;
				m_game->m_client_list[i]->m_fightzone_ticket_number = 0;
				break;

			case 0: // LogResMsg::Reject
				result = MsgType::Reject;
				hb::logger::log("Guild '{}' disband failed for {}", m_game->m_client_list[i]->m_guild_name, m_game->m_client_list[i]->m_char_name);
				break;
			}

			hb::net::PacketHeader pkt{};
			pkt.msg_id = MsgId::ResponseDisbandGuild;
			pkt.msg_type = result;

			ret = m_game->m_client_list[i]->m_socket->send_msg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
			switch (ret) {
			case sock::Event::QueueFull:
			case sock::Event::SocketError:
			case sock::Event::CriticalError:
			case sock::Event::SocketClosed:
				m_game->delete_client(i, true, true);
				return;
			}
			return;
		}

	hb::logger::log("Non-existent player data from login server: {}", char_name);
}

void GuildManager::join_guild_approve_handler(int client_h, const char* name)
{

	bool is_exist = false;

	if (m_game->m_client_list[client_h] == 0) return;
	if (m_game->m_client_list[client_h]->m_is_init_complete == false) return;

	for(int i = 1; i < hb::server::config::MaxClients; i++)
		if ((m_game->m_client_list[i] != 0) && (hb_strnicmp(m_game->m_client_list[i]->m_char_name, name, hb::shared::limits::CharNameLen - 1) == 0)) {
			if (memcmp(m_game->m_client_list[i]->m_location, m_game->m_client_list[client_h]->m_location, 10) != 0) return;

			std::memset(m_game->m_client_list[i]->m_guild_name, 0, sizeof(m_game->m_client_list[i]->m_guild_name));
			strcpy(m_game->m_client_list[i]->m_guild_name, m_game->m_client_list[client_h]->m_guild_name);

			// GUID.
			m_game->m_client_list[i]->m_guild_guid = m_game->m_client_list[client_h]->m_guild_guid;

			std::memset(m_game->m_client_list[i]->m_location, 0, sizeof(m_game->m_client_list[i]->m_location));
			strcpy(m_game->m_client_list[i]->m_location, m_game->m_client_list[client_h]->m_location);

			m_game->m_client_list[i]->m_guild_rank = m_game->m_starting_guild_rank;

			m_game->send_notify_msg(client_h, i, CommonType::JoinGuildApprove, 0, 0, 0, 0);

			m_game->send_event_to_near_client_type_a(client_h, hb::shared::owner_class::Player, MsgId::EventMotion, Type::NullAction, 0, 0, 0);
			m_game->send_event_to_near_client_type_a(i, hb::shared::owner_class::Player, MsgId::EventMotion, Type::NullAction, 0, 0, 0);

			send_guild_msg(i, Notify::NewGuildsman, 0, 0, 0);

			return;
		}

}

void GuildManager::join_guild_reject_handler(int client_h, const char* name)
{

	if (m_game->m_client_list[client_h] == 0) return;
	if (m_game->m_client_list[client_h]->m_is_init_complete == false) return;

	for(int i = 1; i < hb::server::config::MaxClients; i++)
		if ((m_game->m_client_list[i] != 0) && (hb_strnicmp(m_game->m_client_list[i]->m_char_name, name, hb::shared::limits::CharNameLen - 1) == 0)) {

			m_game->send_notify_msg(client_h, i, CommonType::JoinGuildReject, 0, 0, 0, 0);
			return;
		}

}

void GuildManager::dismiss_guild_approve_handler(int client_h, const char* name)
{

	if (m_game->m_client_list[client_h] == 0) return;
	if (m_game->m_client_list[client_h]->m_is_init_complete == false) return;
	for(int i = 1; i < hb::server::config::MaxClients; i++)
		if ((m_game->m_client_list[i] != 0) && (hb_strnicmp(m_game->m_client_list[i]->m_char_name, name, hb::shared::limits::CharNameLen - 1) == 0)) {

			send_guild_msg(i, Notify::DismissGuildsman, 0, 0, 0);

			std::memset(m_game->m_client_list[i]->m_guild_name, 0, sizeof(m_game->m_client_list[i]->m_guild_name));
			strcpy(m_game->m_client_list[i]->m_guild_name, "NONE");
			m_game->m_client_list[i]->m_guild_rank = -1;
			m_game->m_client_list[i]->m_guild_guid = -1;

			m_game->send_notify_msg(client_h, i, CommonType::DismissGuildApprove, 0, 0, 0, 0);

			m_game->send_event_to_near_client_type_a(client_h, hb::shared::owner_class::Player, MsgId::EventMotion, Type::NullAction, 0, 0, 0);
			return;
		}

}

void GuildManager::dismiss_guild_reject_handler(int client_h, const char* name)
{

	if (m_game->m_client_list[client_h] == 0) return;
	if (m_game->m_client_list[client_h]->m_is_init_complete == false) return;

	for(int i = 1; i < hb::server::config::MaxClients; i++)
		if ((m_game->m_client_list[i] != 0) && (hb_strnicmp(m_game->m_client_list[i]->m_char_name, name, hb::shared::limits::CharNameLen - 1) == 0)) {

			m_game->send_notify_msg(client_h, i, CommonType::DismissGuildReject, 0, 0, 0, 0);
			return;
		}

}

void GuildManager::send_guild_msg(int client_h, uint16_t notify_msg_type, short v1, short v2, char* string)
{
	int ret;

	if (m_game->m_client_list[client_h] == 0) return;
	if (m_game->m_client_list[client_h]->m_is_init_complete == false) return;

	for(int i = 0; i < hb::server::config::MaxClients; i++)
		if ((m_game->m_client_list[i] != 0) &&
			(memcmp(m_game->m_client_list[i]->m_guild_name, m_game->m_client_list[client_h]->m_guild_name, 20) == 0)) {

			switch (notify_msg_type) {
			case Notify::GuildDisbanded:
				if (i == client_h) break;
				{
					hb::net::PacketNotifyGuildDisbanded pkt{};
					pkt.header.msg_id = MsgId::Notify;
					pkt.header.msg_type = notify_msg_type;
					memcpy(pkt.guild_name, m_game->m_client_list[client_h]->m_guild_name, sizeof(pkt.guild_name));
					memcpy(pkt.location, m_game->m_client_list[i]->m_location, sizeof(pkt.location));
					ret = m_game->m_client_list[i]->m_socket->send_msg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
				}
				std::memset(m_game->m_client_list[i]->m_guild_name, 0, sizeof(m_game->m_client_list[i]->m_guild_name));
				strcpy(m_game->m_client_list[i]->m_guild_name, "NONE");
				m_game->m_client_list[i]->m_guild_rank = -1;
				m_game->m_client_list[i]->m_guild_guid = -1;
				m_game->m_client_list[i]->m_fightzone_number = 0;
				m_game->m_client_list[i]->m_reserve_time = 0;
				m_game->m_client_list[i]->m_fightzone_ticket_number = 0;
				break;

			case Notify::EventMsgString:
			{
				hb::net::PacketWriter writer;
				writer.Reserve(sizeof(hb::net::PacketHeader) + 256);

				auto* header = writer.Append<hb::net::PacketHeader>();
				header->msg_id = MsgId::Notify;
				header->msg_type = notify_msg_type;

				if (string != 0) {
					const std::size_t len = std::strlen(string);
					writer.AppendBytes(string, len);
				}
				writer.AppendBytes("", 1);

				ret = m_game->m_client_list[i]->m_socket->send_msg(writer.Data(), static_cast<int>(writer.size()));
			}
			break;

			case Notify::NewGuildsman:
			{
				hb::net::PacketNotifyNewGuildsMan pkt{};
				pkt.header.msg_id = MsgId::Notify;
				pkt.header.msg_type = notify_msg_type;
				memcpy(pkt.name, m_game->m_client_list[client_h]->m_char_name, sizeof(pkt.name));
				ret = m_game->m_client_list[i]->m_socket->send_msg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
			}
			break;

			case Notify::DismissGuildsman:
			{
				hb::net::PacketNotifyDismissGuildsMan pkt{};
				pkt.header.msg_id = MsgId::Notify;
				pkt.header.msg_type = notify_msg_type;
				memcpy(pkt.name, m_game->m_client_list[client_h]->m_char_name, sizeof(pkt.name));
				ret = m_game->m_client_list[i]->m_socket->send_msg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
			}
			break;
			}

			switch (ret) {
			case sock::Event::QueueFull:
			case sock::Event::SocketError:
			case sock::Event::CriticalError:
			case sock::Event::SocketClosed:
				m_game->delete_client(i, true, true);
				return;
			}
		}

}

void GuildManager::guild_notify_handler(char* data, size_t msg_size)
{
	char* cp, char_name[hb::shared::limits::CharNameLen], guild_name[21];

	std::memset(char_name, 0, sizeof(char_name));
	std::memset(guild_name, 0, sizeof(guild_name));

	const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(data, sizeof(hb::net::PacketHeader));
	if (!header) return;
	cp = (char*)(data + sizeof(hb::net::PacketHeader));

	memcpy(char_name, cp, hb::shared::limits::CharNameLen);
	cp += hb::shared::limits::CharNameLen;

	memcpy(guild_name, cp, hb::shared::limits::GuildNameLen);
	cp += hb::shared::limits::GuildNameLen;

}

void GuildManager::user_command_ban_guildsman(int client_h, char* data, size_t msg_size)
{
	char   seps[] = "= \t\r\n";
	char* token, target_name[11], buff[256];

	if (m_game->m_client_list[client_h] == 0) return;
	if ((msg_size) <= 0) return;

	if (m_game->m_client_list[client_h]->m_guild_rank != 0) {
		m_game->send_notify_msg(0, client_h, Notify::NoGuildMasterLevel, 0, 0, 0, 0);
		return;
	}

	std::memset(target_name, 0, sizeof(target_name));
	std::memset(buff, 0, sizeof(buff));
	memcpy(buff, data, msg_size);

	token = strtok(buff, seps);
	token = strtok(NULL, seps);

	if (token != 0) {
		if (strlen(token) > hb::shared::limits::CharNameLen - 1)
			memcpy(target_name, token, hb::shared::limits::CharNameLen - 1);
		else memcpy(target_name, token, strlen(token));

		for(int i = 1; i < hb::server::config::MaxClients; i++)
			if ((m_game->m_client_list[i] != 0) && (hb_strnicmp(m_game->m_client_list[i]->m_char_name, target_name, hb::shared::limits::CharNameLen - 1) == 0)) {

				if (memcmp(m_game->m_client_list[client_h]->m_guild_name, m_game->m_client_list[i]->m_guild_name, 20) != 0) {

					m_game->send_notify_msg(0, client_h, Notify::CannotBanGuildman, 0, 0, 0, 0);
					return;
				}

				send_guild_msg(i, Notify::DismissGuildsman, 0, 0, 0);

				std::memset(m_game->m_client_list[i]->m_guild_name, 0, sizeof(m_game->m_client_list[i]->m_guild_name));
				strcpy(m_game->m_client_list[i]->m_guild_name, "NONE");
				m_game->m_client_list[i]->m_guild_rank = -1;
				m_game->m_client_list[i]->m_guild_guid = -1;

				m_game->send_notify_msg(0, client_h, Notify::SuccessBanGuildman, 0, 0, 0, 0);

				m_game->send_notify_msg(client_h, i, CommonType::BanGuild, 0, 0, 0, 0);

				m_game->send_event_to_near_client_type_a(i, hb::shared::owner_class::Player, MsgId::EventMotion, Type::NullAction, 0, 0, 0);

				return;
			}
		m_game->send_notify_msg(0, client_h, Notify::PlayerNotOnGame, 0, 0, 0, target_name);
	}

	return;
}

void GuildManager::user_command_dismiss_guild(int client_h, char* data, size_t msg_size)
{

}

void GuildManager::request_create_new_guild(int client_h, char* data)
{
	char file_name[255];
	char txt[500];
	char txt2[100];
	char guild_master_name[hb::shared::limits::CharNameLen], guild_location[11], dir[255], guild_name[21];
	uint32_t guild_guid;
	hb::time::local_time SysTime{};
	FILE* file;

	if (m_game->m_client_list[client_h] == 0) return;
	std::memset(file_name, 0, sizeof(file_name));
	std::memset(txt, 0, sizeof(txt));
	std::memset(txt2, 0, sizeof(txt2));
	std::memset(dir, 0, sizeof(dir));
	std::memset(guild_master_name, 0, sizeof(guild_master_name));
	std::memset(guild_name, 0, sizeof(guild_name));
	std::memset(guild_location, 0, sizeof(guild_location));

	const auto& guildData = *reinterpret_cast<const hb::net::GuildCreatePayload*>(data);
	std::memcpy(guild_master_name, guildData.char_name, sizeof(guildData.char_name));
	std::memcpy(guild_name, guildData.guild_name, sizeof(guildData.guild_name));
	std::memcpy(guild_location, guildData.location, sizeof(guildData.location));
	guild_guid = guildData.guild_guid;

	strcat(file_name, "Guilds");
	strcat(file_name, "/");
	std::snprintf(txt2, sizeof(txt2), "AscII%d", *guild_name);
	strcat(file_name, txt2);
	strcat(dir, file_name);
	strcat(file_name, "/");
	strcat(file_name, "/");
	strcat(file_name, guild_name);
	strcat(file_name, ".txt");

	std::filesystem::create_directories(dir);

	file = fopen(file_name, "rt");
	if (file != 0) {
		hb::logger::error("Cannot create guild: name already exists ({})", file_name);

		response_create_new_guild_handler(guild_master_name, 0);
		fclose(file);
	}
	else {
		file = fopen(file_name, "wt");
		if (file == 0) {
			hb::logger::error("Cannot create guild: file creation failed ({})", file_name);

			response_create_new_guild_handler(guild_master_name, 0);
		}
		else {
			hb::logger::log("Guild created: {}", file_name);

			std::memset(txt2, 0, sizeof(txt2));
			std::memset(txt, 0, sizeof(txt));
			SysTime = hb::time::local_time::now();

			std::snprintf(txt, sizeof(txt), ";Guild file - Updated %4d/%2d/%2d/%2d/%2d", SysTime.year, SysTime.month, SysTime.day, SysTime.hour, SysTime.minute);
			strcat(txt, "\n");
			strcat(txt, ";Just created\n\n");

			strcat(txt, "[GUILD-INFO]\n\n");

			strcat(txt, "guildmaster-name     = ");
			strcat(txt, guild_master_name);
			strcat(txt, "\n");

			strcat(txt, "guild-GUID           = ");
			std::snprintf(txt2, sizeof(txt2), "%d", guild_guid);
			strcat(txt, txt2);
			strcat(txt, "\n");

			strcat(txt, "guild-location       = ");
			strcat(txt, guild_location);
			strcat(txt, "\n\n");

			strcat(txt, "[GUILDSMAN]\n\n");

			fwrite(txt, 1, strlen(txt), file);

			response_create_new_guild_handler(guild_master_name, 1);
			fclose(file);
		}
	}
}

void GuildManager::request_disband_guild(int client_h, char* data)
{
	char temp[500];
	char file_name[255], txt[100], dir[100];
	char guild_master_name[hb::shared::limits::CharNameLen], guild_name[21];
	FILE* file;

	if (m_game->m_client_list[client_h] == 0) return;
	std::memset(file_name, 0, sizeof(file_name));
	std::memset(txt, 0, sizeof(txt));
	std::memset(dir, 0, sizeof(dir));
	std::memset(temp, 0, sizeof(temp));
	std::memset(guild_master_name, 0, sizeof(guild_master_name));
	std::memset(guild_name, 0, sizeof(guild_name));

	const auto& disbandData = *reinterpret_cast<const hb::net::GuildDisbandPayload*>(data);
	std::memcpy(guild_master_name, disbandData.char_name, sizeof(disbandData.char_name));
	std::memcpy(guild_name, disbandData.guild_name, sizeof(disbandData.guild_name));

	strcat(file_name, "Guilds");
	strcat(file_name, "/");
	strcat(file_name, "/");
	std::snprintf(txt, sizeof(txt), "AscII%d", *guild_name);
	strcat(file_name, txt);
	strcat(dir, file_name);
	strcat(file_name, "/");
	strcat(file_name, "/");
	strcat(file_name, guild_name);
	strcat(file_name, ".txt");

	file = fopen(file_name, "rt");
	if (file != 0) {
		fclose(file);
		hb::logger::log("Guild disbanded, deleting file: {}", file_name);
		if (std::remove(file_name) == 0) {
			response_disband_guild_handler(guild_master_name, 1);
		}
		else {

			response_disband_guild_handler(guild_master_name, 0);
		}
	}
	else {

		response_disband_guild_handler(guild_master_name, 0);
	}
}

void GuildManager::request_guild_name_handler(int client_h, int object_id, int index)
{
	if (m_game->m_client_list[client_h] == 0) return;
	if ((object_id <= 0) || (object_id >= hb::server::config::MaxClients)) return;

	if (m_game->m_client_list[object_id] == 0) {
		// Object .

	}
	else {
		m_game->send_notify_msg(0, client_h, Notify::ReqGuildNameAnswer, m_game->m_client_list[object_id]->m_guild_rank, index, 0, m_game->m_client_list[object_id]->m_guild_name);
	}
}
