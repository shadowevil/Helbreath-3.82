// guild_manager.cpp: Handles client-side guild network messages.
// Extracted from NetworkMessages_Guild.cpp (Phase B4).

#include "GuildManager.h"
#include "Game.h"
#include "Packet/SharedPackets.h"
#include "lan_eng.h"
#include <cstdio>
#include <cstring>
#include <string_view>
#include <format>
#include <string>
#include <algorithm>

using namespace hb::shared::net;

void guild_manager::update_location_flags(CGame* game, const char* location)
{
	if (memcmp(location, "aresden", 7) == 0)
	{
		game->m_player->m_aresden = true;
		game->m_player->m_citizen = true;
		game->m_player->m_hunter = false;
	}
	else if (memcmp(location, "arehunter", 9) == 0)
	{
		game->m_player->m_aresden = true;
		game->m_player->m_citizen = true;
		game->m_player->m_hunter = true;
	}
	else if (memcmp(location, "elvine", 6) == 0)
	{
		game->m_player->m_aresden = false;
		game->m_player->m_citizen = true;
		game->m_player->m_hunter = false;
	}
	else if (memcmp(location, "elvhunter", 9) == 0)
	{
		game->m_player->m_aresden = false;
		game->m_player->m_citizen = true;
		game->m_player->m_hunter = true;
	}
	else
	{
		game->m_player->m_aresden = true;
		game->m_player->m_citizen = false;
		game->m_player->m_hunter = true;
	}
}

void guild_manager::handle_create_new_guild_response(char* data)
{
	const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(
		data, sizeof(hb::net::PacketHeader));
	if (!header) return;
	switch (header->msg_type) {
	case MsgType::Confirm:
		m_game->m_player->m_guild_rank = 0;
		m_game->m_dialog_box_manager.Info(DialogBoxId::GuildMenu).m_mode = 3;
		break;
	case MsgType::Reject:
		m_game->m_player->m_guild_rank = -1;
		m_game->m_dialog_box_manager.Info(DialogBoxId::GuildMenu).m_mode = 4;
		break;
	}
}

void guild_manager::handle_disband_guild_response(char* data)
{
	const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(
		data, sizeof(hb::net::PacketHeader));
	if (!header) return;
	switch (header->msg_type) {
	case MsgType::Confirm:
		m_game->m_player->m_guild_rank = -1;
		m_game->m_player->m_guild_name.clear();
		m_game->m_dialog_box_manager.Info(DialogBoxId::GuildMenu).m_mode = 7;
		break;
	case MsgType::Reject:
		m_game->m_dialog_box_manager.Info(DialogBoxId::GuildMenu).m_mode = 8;
		break;
	}
}

void guild_manager::handle_guild_disbanded(char* data)
{
	char name[24]{}, location[12]{};
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyGuildDisbanded>(
		data, sizeof(hb::net::PacketNotifyGuildDisbanded));
	if (!pkt) return;
	memcpy(name, pkt->guild_name, sizeof(pkt->guild_name));
	memcpy(location, pkt->location, sizeof(pkt->location));
	CMisc::replace_string(name, '_', ' ');
	m_game->m_dialog_box_manager.enable_dialog_box(DialogBoxId::GuildOperation, 0, 0, 0);
	m_game->put_guild_operation_list(name, 7);
	m_game->m_player->m_guild_rank = -1;
	m_game->m_player->m_guild_name.clear();
	m_game->m_location.assign(location, strnlen(location, hb::shared::limits::MapNameLen));
	update_location_flags(m_game, m_game->m_location.c_str());
	m_game->clear_guild_name_list();
}

void guild_manager::handle_new_guilds_man(char* data)
{
	std::string txt;

	char name[12]{};
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyNewGuildsMan>(
		data, sizeof(hb::net::PacketNotifyNewGuildsMan));
	if (!pkt) return;
	memcpy(name, pkt->name, sizeof(pkt->name));
	txt = std::format(NOTIFYMSG_NEW_GUILDMAN1, name);
	m_game->add_event_list(txt.c_str(), 10);
	m_game->clear_guild_name_list();
}

void guild_manager::handle_dismiss_guilds_man(char* data)
{
	std::string txt;

	char name[12]{};

	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyDismissGuildsMan>(
		data, sizeof(hb::net::PacketNotifyDismissGuildsMan));
	if (!pkt) return;
	memcpy(name, pkt->name, sizeof(pkt->name));

	if (m_game->m_player->m_player_name != std::string_view(name, strnlen(name, hb::shared::limits::CharNameLen))) {
		txt = std::format(NOTIFYMSG_DISMISS_GUILDMAN1, name);
		m_game->add_event_list(txt.c_str(), 10);
	}
	m_game->clear_guild_name_list();
}

void guild_manager::handle_cannot_join_more_guilds_man(char* data)
{
	std::string txt;

	char name[12]{};

	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyCannotJoinMoreGuildsMan>(
		data, sizeof(hb::net::PacketNotifyCannotJoinMoreGuildsMan));
	if (!pkt) return;
	memcpy(name, pkt->name, sizeof(pkt->name));

	txt = std::format(NOTIFYMSG_CANNOT_JOIN_MOREGUILDMAN1, name);
	m_game->add_event_list(txt.c_str(), 10);
	m_game->add_event_list(NOTIFYMSG_CANNOT_JOIN_MOREGUILDMAN2, 10);
}

void guild_manager::handle_join_guild_approve(char* data)
{
	char name[21]{};
	short rank;
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyJoinGuildApprove>(
		data, sizeof(hb::net::PacketNotifyJoinGuildApprove));
	if (!pkt) return;
	memcpy(name, pkt->guild_name, sizeof(pkt->guild_name));
	rank = pkt->rank;
	m_game->m_player->m_guild_name = name;
	std::replace(m_game->m_player->m_guild_name.begin(), m_game->m_player->m_guild_name.end(), '_', ' ');
	m_game->m_player->m_guild_rank = rank;
	m_game->m_dialog_box_manager.enable_dialog_box(DialogBoxId::GuildOperation, 0, 0, 0);
	CMisc::replace_string(name, '_', ' ');
	m_game->put_guild_operation_list(name, 3);
}

void guild_manager::handle_join_guild_reject(char* data)
{
	char name[21]{};
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyJoinGuildReject>(
		data, sizeof(hb::net::PacketNotifyJoinGuildReject));
	if (!pkt) return;
	memcpy(name, pkt->guild_name, sizeof(pkt->guild_name));
	m_game->m_dialog_box_manager.enable_dialog_box(DialogBoxId::GuildOperation, 0, 0, 0);
	m_game->put_guild_operation_list(name, 4);
}

void guild_manager::handle_dismiss_guild_approve(char* data)
{
	char name[24]{}, location[12]{};
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyDismissGuildApprove>(
		data, sizeof(hb::net::PacketNotifyDismissGuildApprove));
	if (!pkt) return;
	memcpy(name, pkt->guild_name, sizeof(pkt->guild_name));
	memcpy(location, pkt->location, sizeof(pkt->location));
	CMisc::replace_string(name, '_', ' ');
	m_game->m_player->m_guild_rank = -1;
	m_game->m_player->m_guild_name.clear();
	m_game->m_location.assign(location, strnlen(location, hb::shared::limits::MapNameLen));
	update_location_flags(m_game, m_game->m_location.c_str());
	m_game->clear_guild_name_list();
	m_game->m_dialog_box_manager.enable_dialog_box(DialogBoxId::GuildOperation, 0, 0, 0);
	m_game->put_guild_operation_list(name, 5);
}

void guild_manager::handle_dismiss_guild_reject(char* data)
{
	char name[21]{};
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyDismissGuildReject>(
		data, sizeof(hb::net::PacketNotifyDismissGuildReject));
	if (!pkt) return;
	memcpy(name, pkt->guild_name, sizeof(pkt->guild_name));
	m_game->m_dialog_box_manager.enable_dialog_box(DialogBoxId::GuildOperation, 0, 0, 0);
	m_game->put_guild_operation_list(name, 6);
}

void guild_manager::handle_query_join_guild_permission(char* data)
{
	char name[12]{};
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyQueryJoinGuildPermission>(
		data, sizeof(hb::net::PacketNotifyQueryJoinGuildPermission));
	if (!pkt) return;
	memcpy(name, pkt->name, sizeof(pkt->name));
	m_game->m_dialog_box_manager.enable_dialog_box(DialogBoxId::GuildOperation, 0, 0, 0);
	m_game->put_guild_operation_list(name, 1);
}

void guild_manager::handle_query_dismiss_guild_permission(char* data)
{
	char name[12]{};
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyQueryDismissGuildPermission>(
		data, sizeof(hb::net::PacketNotifyQueryDismissGuildPermission));
	if (!pkt) return;
	memcpy(name, pkt->name, sizeof(pkt->name));
	m_game->m_dialog_box_manager.enable_dialog_box(DialogBoxId::GuildOperation, 0, 0, 0);
	m_game->put_guild_operation_list(name, 2);
}

void guild_manager::handle_req_guild_name_answer(char* data)
{
	short v1, v2;
	char temp[256]{};
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyReqGuildNameAnswer>(
		data, sizeof(hb::net::PacketNotifyReqGuildNameAnswer));
	if (!pkt) return;
	v1 = static_cast<short>(pkt->guild_rank);
	v2 = static_cast<short>(pkt->index);
	memcpy(temp, pkt->guild_name, sizeof(pkt->guild_name));

	m_game->m_guild_name_cache[v2].guild_name = temp;
	m_game->m_guild_name_cache[v2].guild_rank = v1;
	m_game->m_guild_name_cache[v2].ref_time = m_game->m_cur_time;
	std::replace(m_game->m_guild_name_cache[v2].guild_name.begin(), m_game->m_guild_name_cache[v2].guild_name.end(), '_', ' ');
}

void guild_manager::handle_no_guild_master_level(char* data)
{
	m_game->add_event_list(NOTIFY_MSG_HANDLER59, 10);
}

void guild_manager::handle_success_ban_guild_man(char* data)
{
	m_game->add_event_list(NOTIFY_MSG_HANDLER60, 10);
}

void guild_manager::handle_cannot_ban_guild_man(char* data)
{
	m_game->add_event_list(NOTIFY_MSG_HANDLER61, 10);
}
