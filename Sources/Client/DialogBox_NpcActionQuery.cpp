#include "DialogBox_NpcActionQuery.h"
#include "Game.h"
#include "InventoryManager.h"
#include "ItemNameFormatter.h"
#include "GlobalDef.h"
#include "lan_eng.h"
#include <format>
#include <string>

using namespace hb::shared::net;
using namespace hb::shared::item;
using namespace hb::client::sprite_id;

DialogBox_NpcActionQuery::DialogBox_NpcActionQuery(CGame* game)
	: IDialogBox(DialogBoxId::NpcActionQuery, game)
{
	set_default_rect(237 , 57 , 252, 87);
}

void DialogBox_NpcActionQuery::draw_highlighted_text(short sX, short sY, const char* text, short mouse_x, short mouse_y, short hitX1, short hitX2, short hitY1, short hitY2)
{
	if ((mouse_x > hitX1) && (mouse_x < hitX2) && (mouse_y > hitY1) && (mouse_y < hitY2)) {
		put_string(sX, sY, (char*)text, GameColors::UIWhite);
		put_string(sX + 1, sY, (char*)text, GameColors::UIWhite);
	}
	else {
		put_string(sX, sY, (char*)text, GameColors::UIMagicBlue);
		put_string(sX + 1, sY, (char*)text, GameColors::UIMagicBlue);
	}
}

void DialogBox_NpcActionQuery::DrawMode0_NpcMenu(short sX, short sY, short mouse_x, short mouse_y)
{
	draw_new_dialog_box(InterfaceNdGame2, sX, sY, 5);

	if (Info().m_v3 == 90) {
		put_string(sX + 33, sY + 23, "Heldenian staff officer", GameColors::UILabel);
		put_string(sX + 33 - 1, sY + 23 - 1, "Heldenian staff officer", GameColors::UIWhite);
	}
	else {
		put_string(sX + 33, sY + 23, Info().m_str, GameColors::UILabel);
		put_string(sX + 33 - 1, sY + 23 - 1, Info().m_str, GameColors::UIWhite);
	}

	if (Info().m_v3 == 25) {
		// OFFER
		draw_highlighted_text(sX + 28, sY + 55, DRAW_DIALOGBOX_NPCACTION_QUERY13, mouse_x, mouse_y, sX + 25, sX + 100, sY + 55, sY + 70);
	}
	else if (Info().m_v3 == 20) {
		// WITHDRAW
		draw_highlighted_text(sX + 28, sY + 55, DRAW_DIALOGBOX_NPCACTION_QUERY17, mouse_x, mouse_y, sX + 25, sX + 100, sY + 55, sY + 70);
	}
	else if (Info().m_v3 == 19) {
		// LEARN
		draw_highlighted_text(sX + 28, sY + 55, DRAW_DIALOGBOX_NPCACTION_QUERY19, mouse_x, mouse_y, sX + 25, sX + 100, sY + 55, sY + 70);
	}
	else {
		// TRADE
		draw_highlighted_text(sX + 28, sY + 55, DRAW_DIALOGBOX_NPCACTION_QUERY21, mouse_x, mouse_y, sX + 25, sX + 100, sY + 55, sY + 70);
	}

	if (m_game->m_dialog_box_manager.is_enabled(DialogBoxId::NpcTalk) == false) {
		draw_highlighted_text(sX + 125, sY + 55, DRAW_DIALOGBOX_NPCACTION_QUERY25, mouse_x, mouse_y, sX + 125, sX + 180, sY + 55, sY + 70);
	}
}

void DialogBox_NpcActionQuery::DrawMode1_GiveToPlayer(short sX, short sY, short mouse_x, short mouse_y)
{
	std::string txt, txt2;


	draw_new_dialog_box(InterfaceNdGame2, sX, sY, 6);
	auto itemInfo = item_name_formatter::get().format(m_game->m_item_list[Info().m_v1].get());
	txt = std::format("{} to", itemInfo.name);
	txt2 = std::format(DRAW_DIALOGBOX_NPCACTION_QUERY29_1, Info().m_str);

	put_string(sX + 24, sY + 25, txt.c_str(), GameColors::UILabel);
	put_string(sX + 24, sY + 40, txt2.c_str(), GameColors::UILabel);

	draw_highlighted_text(sX + 28, sY + 55, DRAW_DIALOGBOX_NPCACTION_QUERY30, mouse_x, mouse_y, sX + 25, sX + 100, sY + 55, sY + 70);
	draw_highlighted_text(sX + 155, sY + 55, DRAW_DIALOGBOX_NPCACTION_QUERY34, mouse_x, mouse_y, sX + 155, sX + 210, sY + 55, sY + 70);
}

void DialogBox_NpcActionQuery::DrawMode2_SellToShop(short sX, short sY, short mouse_x, short mouse_y)
{
	std::string txt, txt2;


	draw_new_dialog_box(InterfaceNdGame2, sX, sY, 5);
	auto itemInfo2 = item_name_formatter::get().format(m_game->m_item_list[Info().m_v1].get());

	txt = std::format("{} to", itemInfo2.name);
	txt2 = std::format(DRAW_DIALOGBOX_NPCACTION_QUERY29_1, Info().m_str);

	put_string(sX + 24, sY + 20, txt.c_str(), GameColors::UILabel);
	put_string(sX + 24, sY + 35, txt2.c_str(), GameColors::UILabel);

	draw_highlighted_text(sX + 28, sY + 55, DRAW_DIALOGBOX_NPCACTION_QUERY39, mouse_x, mouse_y, sX + 25, sX + 100, sY + 55, sY + 70);

	CItem* cfg = m_game->get_item_config(m_game->m_item_list[Info().m_v1]->m_id_num);
	if (cfg && (cfg->get_item_type() != ItemType::Consume) &&
		(cfg->get_item_type() != ItemType::Arrow) &&
		Info().m_v2 == hb::shared::owner::Tom)
	{
		draw_highlighted_text(sX + 125, sY + 55, DRAW_DIALOGBOX_NPCACTION_QUERY43, mouse_x, mouse_y, sX + 125, sX + 180, sY + 55, sY + 70);
	}
}

void DialogBox_NpcActionQuery::DrawMode3_DepositToWarehouse(short sX, short sY, short mouse_x, short mouse_y)
{
	std::string txt, txt2;


	draw_new_dialog_box(InterfaceNdGame2, sX, sY, 6);
	auto itemInfo3 = item_name_formatter::get().format(m_game->m_item_list[Info().m_v1].get());

	txt = std::format("{} to", itemInfo3.name);
	txt2 = std::format(DRAW_DIALOGBOX_NPCACTION_QUERY29_1, Info().m_str);

	put_aligned_string(sX, sX + 240, sY + 20, txt.c_str(), GameColors::UILabel);
	put_aligned_string(sX, sX + 240, sY + 35, txt2.c_str(), GameColors::UILabel);

	draw_highlighted_text(sX + 28, sY + 55, DRAW_DIALOGBOX_NPCACTION_QUERY48, mouse_x, mouse_y, sX + 25, sX + 100, sY + 55, sY + 70);
}

void DialogBox_NpcActionQuery::DrawMode4_TalkToNpcOrUnicorn(short sX, short sY, short mouse_x, short mouse_y)
{
	draw_new_dialog_box(InterfaceNdGame2, sX, sY, 5);

	put_string(sX + 35, sY + 25, Info().m_str, GameColors::UILabel);
	put_string(sX + 35 - 1, sY + 25 - 1, Info().m_str, GameColors::UIWhite);

	if (m_game->m_dialog_box_manager.is_enabled(DialogBoxId::NpcTalk) == false) {
		draw_highlighted_text(sX + 125, sY + 55, DRAW_DIALOGBOX_NPCACTION_QUERY25, mouse_x, mouse_y, sX + 125, sX + 180, sY + 55, sY + 70);
	}
}

void DialogBox_NpcActionQuery::DrawMode5_ShopWithSell(short sX, short sY, short mouse_x, short mouse_y)
{
	draw_new_dialog_box(InterfaceNdGame2, sX, sY, 6);

	put_string(sX + 33, sY + 23, Info().m_str, GameColors::UILabel);
	put_string(sX + 33 - 1, sY + 23 - 1, Info().m_str, GameColors::UIWhite);

	if (Info().m_v3 == 24) {
		// Repair All button (Blacksmith only)
		draw_highlighted_text(sX + 155, sY + 22, DRAW_DIALOGBOX_NPCACTION_QUERY49, mouse_x, mouse_y, sX + 155, sX + 210, sY + 22, sY + 37);
	}

	// Trade
	draw_highlighted_text(sX + 28, sY + 55, DRAW_DIALOGBOX_NPCACTION_QUERY21, mouse_x, mouse_y, sX + 25, sX + 100, sY + 55, sY + 70);

	// Sell
	draw_highlighted_text(sX + 28 + 75, sY + 55, DRAW_DIALOGBOX_NPCACTION_QUERY39, mouse_x, mouse_y, sX + 25 + 79, sX + 80 + 75, sY + 55, sY + 70);

	if (m_game->m_dialog_box_manager.is_enabled(DialogBoxId::NpcTalk) == false) {
		draw_highlighted_text(sX + 155, sY + 55, DRAW_DIALOGBOX_NPCACTION_QUERY25, mouse_x, mouse_y, sX + 155, sX + 210, sY + 55, sY + 70);
	}
}

void DialogBox_NpcActionQuery::DrawMode6_Gail(short sX, short sY, short mouse_x, short mouse_y)
{
	draw_new_dialog_box(InterfaceNdGame2, sX, sY, 5);

	draw_highlighted_text(sX + 28, sY + 55, DRAW_DIALOGBOX_NPCACTION_QUERY21, mouse_x, mouse_y, sX + 25, sX + 100, sY + 55, sY + 70);

	put_string(sX + 33, sY + 23, "Heldenian staff officer", GameColors::UILabel);
	put_string(sX + 33 - 1, sY + 23 - 1, "Heldenian staff officer", GameColors::UIWhite);
}

void DialogBox_NpcActionQuery::on_draw(short mouse_x, short mouse_y, short z, char lb)
{
	if (!m_game->ensure_item_configs_loaded()) return;
	short sX = Info().m_x;
	short sY = Info().m_y;

	switch (Info().m_mode) {
	case 0:
		DrawMode0_NpcMenu(sX, sY, mouse_x, mouse_y);
		break;
	case 1:
		DrawMode1_GiveToPlayer(sX, sY, mouse_x, mouse_y);
		break;
	case 2:
		DrawMode2_SellToShop(sX, sY, mouse_x, mouse_y);
		break;
	case 3:
		DrawMode3_DepositToWarehouse(sX, sY, mouse_x, mouse_y);
		break;
	case 4:
		DrawMode4_TalkToNpcOrUnicorn(sX, sY, mouse_x, mouse_y);
		break;
	case 5:
		DrawMode5_ShopWithSell(sX, sY, mouse_x, mouse_y);
		break;
	case 6:
		DrawMode6_Gail(sX, sY, mouse_x, mouse_y);
		break;
	}
}

bool DialogBox_NpcActionQuery::on_click(short mouse_x, short mouse_y)
{
	short sX = Info().m_x;
	short sY = Info().m_y;
	int absX, absY;

	if (m_game->m_dialog_box_manager.is_enabled(DialogBoxId::Exchange) == true) {
		add_event_list(BITEMDROP_SKILLDIALOG1, 10);
		return true;
	}

	switch (Info().m_mode) {
	case 0: // Talk to npc
		if ((mouse_x > sX + 25) && (mouse_x < sX + 100) && (mouse_y > sY + 55) && (mouse_y < sY + 70)) {
			enable_dialog_box((DialogBoxId::Type)Info().m_v1, Info().m_v2, 0, 0);
			disable_this_dialog();
			return true;
		}
		if ((m_game->m_dialog_box_manager.is_enabled(DialogBoxId::NpcTalk) == false) && (mouse_x > sX + 125) && (mouse_x < sX + 180) && (mouse_y > sY + 55) && (mouse_y < sY + 70)) {
			switch (Info().m_v1) {
			case 7:
				send_command(MsgId::CommandCommon, CommonType::TalkToNpc, 0, 1, 0, 0, 0);
				add_event_list(TALKING_TO_GUILDHALL_OFFICER, 10);
				break;
			case 11:
				switch (Info().m_v2) {
				case 1:
					send_command(MsgId::CommandCommon, CommonType::TalkToNpc, 0, 2, 0, 0, 0);
					add_event_list(TALKING_TO_SHOP_KEEPER, 10);
					break;
				case 2:
					send_command(MsgId::CommandCommon, CommonType::TalkToNpc, 0, 3, 0, 0, 0);
					add_event_list(TALKING_TO_BLACKSMITH_KEEPER, 10);
					break;
				}
				break;
			case 13:
				send_command(MsgId::CommandCommon, CommonType::TalkToNpc, 0, 4, 0, 0, 0);
				add_event_list(TALKING_TO_CITYHALL_OFFICER, 10);
				break;
			case 14:
				send_command(MsgId::CommandCommon, CommonType::TalkToNpc, 0, 5, 0, 0, 0);
				add_event_list(TALKING_TO_WAREHOUSE_KEEPER, 10);
				break;
			case 16:
				send_command(MsgId::CommandCommon, CommonType::TalkToNpc, 0, 6, 0, 0, 0);
				add_event_list(TALKING_TO_MAGICIAN, 10);
				break;
			}
			disable_this_dialog();
			return true;
		}
		break;

	case 1: // On other player
	{
		CItem* cfg = m_game->get_item_config(m_game->m_item_list[Info().m_v1]->m_id_num);
		if ((mouse_x > sX + 25) && (mouse_x < sX + 100) && (mouse_y > sY + 55) && (mouse_y < sY + 70)) {
			absX = abs(Info().m_v5 - m_game->m_player->m_player_x);
			absY = abs(Info().m_v6 - m_game->m_player->m_player_y);
			if ((absX <= 4) && (absY <= 4) && cfg)
				send_command(MsgId::CommandCommon, CommonType::GiveItemToChar, Info().m_v1, Info().m_v3, Info().m_v5, Info().m_v6, cfg->m_name, Info().m_v4);
			else add_event_list(DLGBOX_CLICK_NPCACTION_QUERY7, 10);
			disable_this_dialog();
			return true;
		}
		else if ((mouse_x > sX + 155) && (mouse_x < sX + 210) && (mouse_y > sY + 55) && (mouse_y < sY + 70)) {
			absX = abs(Info().m_v5 - m_game->m_player->m_player_x);
			absY = abs(Info().m_v6 - m_game->m_player->m_player_y);
			if ((absX <= 4) && (absY <= 4) && cfg)
				send_command(MsgId::CommandCommon, CommonType::ExchangeItemToChar, Info().m_v1, Info().m_v3, Info().m_v5, Info().m_v6, cfg->m_name, Info().m_v4);
			else add_event_list(DLGBOX_CLICK_NPCACTION_QUERY8, 10);
			disable_this_dialog();
			return true;
		}
		break;
	}

	case 2: // Item on Shop/BS
	{
		CItem* cfg = m_game->get_item_config(m_game->m_item_list[Info().m_v1]->m_id_num);
		if ((mouse_x > sX + 25) && (mouse_x < sX + 100) && (mouse_y > sY + 55) && (mouse_y < sY + 70)) {
			// Can't sell gold
			if (m_game->m_item_list[Info().m_v1]->m_id_num == ItemId::Gold)
			{
				add_event_list(BITEMDROP_SELLLIST2, 10);
				disable_this_dialog();
				return true;
			}
			if (cfg) send_command(MsgId::CommandCommon, CommonType::ReqSellItem, 0, Info().m_v1, Info().m_v2, Info().m_v3, cfg->m_name, Info().m_v4);
			disable_this_dialog();
			return true;
		}
		else if ((mouse_x > sX + 125) && (mouse_x < sX + 180) && (mouse_y > sY + 55) && (mouse_y < sY + 70)) {
			if (Info().m_v3 == 1) {
				if (cfg) send_command(MsgId::CommandCommon, CommonType::ReqRepairItem, 0, Info().m_v1, Info().m_v2, 0, cfg->m_name, Info().m_v4);
				disable_this_dialog();
				return true;
			}
		}
		break;
	}

	case 3: // Put item in the WH
	{
		CItem* cfg = m_game->get_item_config(m_game->m_item_list[Info().m_v1]->m_id_num);
		if ((mouse_x > sX + 25) && (mouse_x < sX + 105) && (mouse_y > sY + 55) && (mouse_y < sY + 70)) {
			absX = abs(Info().m_v5 - m_game->m_player->m_player_x);
			absY = abs(Info().m_v6 - m_game->m_player->m_player_y);
			if ((absX <= 8) && (absY <= 8)) {
				if (inventory_manager::get().get_bank_item_count() >= (m_game->m_max_bank_items - 1)) {
					add_event_list(DLGBOX_CLICK_NPCACTION_QUERY9, 10);
				}
				else if (cfg) send_command(MsgId::CommandCommon, CommonType::GiveItemToChar, Info().m_v1, Info().m_v3, Info().m_v5, Info().m_v6, cfg->m_name, Info().m_v4);
			}
			else add_event_list(DLGBOX_CLICK_NPCACTION_QUERY7, 10);
			disable_this_dialog();
			return true;
		}
		break;
	}

	case 4: // talk to npc or Unicorn
		if ((m_game->m_dialog_box_manager.is_enabled(DialogBoxId::NpcTalk) == false) && (mouse_x > sX + 125) && (mouse_x < sX + 180) && (mouse_y > sY + 55) && (mouse_y < sY + 70)) {
			switch (Info().m_v3) {
			case 21:
				send_command(MsgId::CommandCommon, CommonType::TalkToNpc, 0, 21, 0, 0, 0);
				add_event_list(TALKING_TO_GUARD, 10);
				break;
			case 32:
				send_command(MsgId::CommandCommon, CommonType::TalkToNpc, 0, 32, 0, 0, 0);
				add_event_list(TALKING_TO_UNICORN, 10);
				break;
			case 67:
				send_command(MsgId::CommandCommon, CommonType::TalkToNpc, 0, 67, 0, 0, 0);
				add_event_list(TALKING_TO_MCGAFFIN, 10);
				break;
			case 68:
				send_command(MsgId::CommandCommon, CommonType::TalkToNpc, 0, 68, 0, 0, 0);
				add_event_list(TALKING_TO_PERRY, 10);
				break;
			case 69:
				send_command(MsgId::CommandCommon, CommonType::TalkToNpc, 0, 69, 0, 0, 0);
				add_event_list(TALKING_TO_DEVLIN, 10);
				break;
			}
		}
		disable_this_dialog();
		return true;

	case 5: // Talk
		if ((mouse_x > sX + 25) && (mouse_x < sX + 100) && (mouse_y > sY + 55) && (mouse_y < sY + 70)) {
			enable_dialog_box((DialogBoxId::Type)Info().m_v1, Info().m_v2, 0, 0);
			disable_this_dialog();
			return true;
		}
		if ((mouse_x > sX + 25 + 75) && (mouse_x < sX + 80 + 75) && (mouse_y > sY + 55) && (mouse_y < sY + 70)) {
			enable_dialog_box(DialogBoxId::SellList, 0, 0, 0);
			disable_this_dialog();
			return true;
		}
		if ((m_game->m_dialog_box_manager.is_enabled(DialogBoxId::NpcTalk) == false) && (mouse_x > sX + 155) && (mouse_x < sX + 210) && (mouse_y > sY + 55) && (mouse_y < sY + 70)) {
			switch (Info().m_v1) {
			case 7:
				send_command(MsgId::CommandCommon, CommonType::TalkToNpc, 0, 1, 0, 0, 0);
				add_event_list(TALKING_TO_GUILDHALL_OFFICER, 10);
				break;
			case 11:
				switch (Info().m_v2) {
				case 1:
					send_command(MsgId::CommandCommon, CommonType::TalkToNpc, 0, 2, 0, 0, 0);
					add_event_list(TALKING_TO_SHOP_KEEPER, 10);
					break;
				case 2:
					send_command(MsgId::CommandCommon, CommonType::TalkToNpc, 0, 3, 0, 0, 0);
					add_event_list(TALKING_TO_BLACKSMITH_KEEPER, 10);
					break;
				}
				break;
			case 13:
				send_command(MsgId::CommandCommon, CommonType::TalkToNpc, 0, 4, 0, 0, 0);
				add_event_list(TALKING_TO_CITYHALL_OFFICER, 10);
				break;
			case 14:
				send_command(MsgId::CommandCommon, CommonType::TalkToNpc, 0, 5, 0, 0, 0);
				add_event_list(TALKING_TO_WAREHOUSE_KEEPER, 10);
				break;
			case 16:
				send_command(MsgId::CommandCommon, CommonType::TalkToNpc, 0, 6, 0, 0, 0);
				add_event_list(TALKING_TO_MAGICIAN, 10);
				break;
			}
			disable_this_dialog();
			return true;
		}
		// Repair All
		if ((mouse_x > sX + 155) && (mouse_x < sX + 210) && (mouse_y > sY + 22) && (mouse_y < sY + 37)) {
			if (Info().m_v3 == 24) {
				send_command(MsgId::CommandCommon, CommonType::ReqRepairAll, 0, 0, 0, 0, 0, 0);
				disable_this_dialog();
				return true;
			}
		}
		break;

	case 6: // Gail
		if ((mouse_x > sX + 25) && (mouse_x < sX + 100) && (mouse_y > sY + 55) && (mouse_y < sY + 70)) {
			enable_dialog_box(DialogBoxId::GuildHallMenu, 0, 0, 0);
			disable_this_dialog();
			return true;
		}
		break;
	}

	return false;
}
