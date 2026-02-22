#include "DialogBox_SellOrRepair.h"
#include "Game.h"
#include "ItemNameFormatter.h"
#include "ItemSpriteMetadata.h"
#include "GlobalDef.h"
#include "SpriteID.h"
#include "lan_eng.h"
#include "NetMessages.h"
#include <format>
#include <string>

using namespace hb::shared::net;
using namespace hb::client::sprite_id;
using hb::shared::item::EquipPos;
using hb::shared::item::to_int;
DialogBox_SellOrRepair::DialogBox_SellOrRepair(CGame* game)
	: IDialogBox(DialogBoxId::SellOrRepair, game)
{
	set_default_rect(497 , 57 , 258, 339);
}

void DialogBox_SellOrRepair::on_draw(short mouse_x, short mouse_y, short z, char lb)
{
	if (!m_game->ensure_item_configs_loaded()) return;
	short sX, sY;
	uint32_t time = m_game->m_cur_time;
	std::string txt;

	int item_id;
	char item_color;

	sX = Info().m_x;
	sY = Info().m_y;

	switch (Info().m_mode) {
	case 1:
	{
		draw_new_dialog_box(InterfaceNdGame2, sX, sY, 2);
		draw_new_dialog_box(InterfaceNdText, sX, sY, 11);

		item_id = Info().m_v1;

		item_color = m_game->m_item_list[item_id]->m_item_color;
		{
			CItem* sell_cfg = m_game->get_item_config(m_game->m_item_list[item_id]->m_id_num);
			auto sell_draw = m_game->get_item_draw(sell_cfg ? sell_cfg->m_display_id : 0, item_atlas::pack, sell_cfg ? sell_cfg->sprite_is_female() : false);
			if (item_color == 0)
				sell_draw.sprite->draw(sX + 62 + 15, sY + 84 + 30, sell_draw.frame);
			else
			{
				bool sell_is_weapon = sell_cfg && (sell_cfg->m_equip_pos == to_int(EquipPos::LeftHand) ||
					sell_cfg->m_equip_pos == to_int(EquipPos::RightHand) || sell_cfg->m_equip_pos == to_int(EquipPos::TwoHand));
				const auto& sell_tint = sell_is_weapon ? GameColors::Weapons[item_color] : GameColors::Items[item_color];
				sell_draw.sprite->draw(sX + 62 + 15, sY + 84 + 30, sell_draw.frame, hb::shared::sprite::DrawParams::tint(sell_tint.r, sell_tint.g, sell_tint.b));
			}
		}

		auto itemInfo = item_name_formatter::get().format(m_game->m_item_list[item_id].get());
		txt = itemInfo.name;

		if (itemInfo.is_special)
		{
			put_aligned_string(sX + 25, sX + 240, sY + 60, txt.c_str(), GameColors::UIItemName_Special);
			put_aligned_string(sX + 25 + 1, sX + 240 + 1, sY + 60, txt.c_str(), GameColors::UIItemName_Special);
		}
		else
		{
			put_aligned_string(sX + 25, sX + 240, sY + 60, txt.c_str(), GameColors::UILabel);
			put_aligned_string(sX + 25 + 1, sX + 240 + 1, sY + 60, txt.c_str(), GameColors::UILabel);
		}

		txt = std::format(DRAW_DIALOGBOX_SELLOR_REPAIR_ITEM2, Info().m_v2);
		put_string(sX + 95 + 15, sY + 53 + 60, txt.c_str(), GameColors::UILabel);
		txt = std::format(DRAW_DIALOGBOX_SELLOR_REPAIR_ITEM3, Info().m_v3);
		put_string(sX + 95 + 15, sY + 53 + 75, txt.c_str(), GameColors::UILabel);
		put_string(sX + 55, sY + 190, DRAW_DIALOGBOX_SELLOR_REPAIR_ITEM4, GameColors::UILabel);

		if ((mouse_x >= sX + ui_layout::left_btn_x) && (mouse_x <= sX + ui_layout::left_btn_x + ui_layout::btn_size_x) && (mouse_y >= sY + ui_layout::btn_y) && (mouse_y <= sY + ui_layout::btn_y + ui_layout::btn_size_y))
			draw_new_dialog_box(InterfaceNdButton, sX + ui_layout::left_btn_x, sY + ui_layout::btn_y, 39);
		else draw_new_dialog_box(InterfaceNdButton, sX + ui_layout::left_btn_x, sY + ui_layout::btn_y, 38);

		if ((mouse_x >= sX + ui_layout::right_btn_x) && (mouse_x <= sX + ui_layout::right_btn_x + ui_layout::btn_size_x) && (mouse_y >= sY + ui_layout::btn_y) && (mouse_y <= sY + ui_layout::btn_y + ui_layout::btn_size_y))
			draw_new_dialog_box(InterfaceNdButton, sX + ui_layout::right_btn_x, sY + ui_layout::btn_y, 17);
		else draw_new_dialog_box(InterfaceNdButton, sX + ui_layout::right_btn_x, sY + ui_layout::btn_y, 16);
		break;
	}

	case 2:
	{
		draw_new_dialog_box(InterfaceNdGame2, sX, sY, 2);
		draw_new_dialog_box(InterfaceNdText, sX, sY, 10);
		item_id = Info().m_v1;
		item_color = m_game->m_item_list[item_id]->m_item_color;
		{
			CItem* rep_cfg = m_game->get_item_config(m_game->m_item_list[item_id]->m_id_num);
			auto rep_draw = m_game->get_item_draw(rep_cfg ? rep_cfg->m_display_id : 0, item_atlas::pack, rep_cfg ? rep_cfg->sprite_is_female() : false);
			if (item_color == 0)
				rep_draw.sprite->draw(sX + 62 + 15, sY + 84 + 30, rep_draw.frame);
			else
			{
				bool rep_is_weapon = rep_cfg && (rep_cfg->m_equip_pos == to_int(EquipPos::LeftHand) ||
					rep_cfg->m_equip_pos == to_int(EquipPos::RightHand) || rep_cfg->m_equip_pos == to_int(EquipPos::TwoHand));
				const auto& rep_tint = rep_is_weapon ? GameColors::Weapons[item_color] : GameColors::Items[item_color];
				rep_draw.sprite->draw(sX + 62 + 15, sY + 84 + 30, rep_draw.frame, hb::shared::sprite::DrawParams::tint(rep_tint.r, rep_tint.g, rep_tint.b));
			}
		}
		auto itemInfo2 = item_name_formatter::get().format(m_game->m_item_list[item_id].get());
		txt = itemInfo2.name.c_str();
		if (itemInfo2.is_special)
		{
			put_aligned_string(sX + 25, sX + 240, sY + 60, txt.c_str(), GameColors::UIItemName_Special);
			put_aligned_string(sX + 25 + 1, sX + 240 + 1, sY + 60, txt.c_str(), GameColors::UIItemName_Special);
		}
		else
		{
			put_aligned_string(sX + 25, sX + 240, sY + 60, txt.c_str(), GameColors::UILabel);
			put_aligned_string(sX + 25 + 1, sX + 240 + 1, sY + 60, txt.c_str(), GameColors::UILabel);
		}
		txt = std::format(DRAW_DIALOGBOX_SELLOR_REPAIR_ITEM2, Info().m_v2);
		put_string(sX + 95 + 15, sY + 53 + 60, txt.c_str(), GameColors::UILabel);
		txt = std::format(DRAW_DIALOGBOX_SELLOR_REPAIR_ITEM6, Info().m_v3);
		put_string(sX + 95 + 15, sY + 53 + 75, txt.c_str(), GameColors::UILabel);
		put_string(sX + 55, sY + 190, DRAW_DIALOGBOX_SELLOR_REPAIR_ITEM7, GameColors::UILabel);

		if ((mouse_x >= sX + ui_layout::left_btn_x) && (mouse_x <= sX + ui_layout::left_btn_x + ui_layout::btn_size_x) && (mouse_y >= sY + ui_layout::btn_y) && (mouse_y <= sY + ui_layout::btn_y + ui_layout::btn_size_y))
			draw_new_dialog_box(InterfaceNdButton, sX + ui_layout::left_btn_x, sY + ui_layout::btn_y, 43);
		else draw_new_dialog_box(InterfaceNdButton, sX + ui_layout::left_btn_x, sY + ui_layout::btn_y, 42);

		if ((mouse_x >= sX + ui_layout::right_btn_x) && (mouse_x <= sX + ui_layout::right_btn_x + ui_layout::btn_size_x) && (mouse_y >= sY + ui_layout::btn_y) && (mouse_y <= sY + ui_layout::btn_y + ui_layout::btn_size_y))
			draw_new_dialog_box(InterfaceNdButton, sX + ui_layout::right_btn_x, sY + ui_layout::btn_y, 17);
		else draw_new_dialog_box(InterfaceNdButton, sX + ui_layout::right_btn_x, sY + ui_layout::btn_y, 16);
		break;
	}

	case 3:
		draw_new_dialog_box(InterfaceNdGame2, sX, sY, 2);
		draw_new_dialog_box(InterfaceNdText, sX, sY, 11);

		put_string(sX + 55, sY + 100, DRAW_DIALOGBOX_SELLOR_REPAIR_ITEM8, GameColors::UILabel);
		put_string(sX + 55, sY + 120, DRAW_DIALOGBOX_SELLOR_REPAIR_ITEM9, GameColors::UILabel);
		put_string(sX + 55, sY + 135, DRAW_DIALOGBOX_SELLOR_REPAIR_ITEM10, GameColors::UILabel);
		break;

	case 4:
		draw_new_dialog_box(InterfaceNdGame2, sX, sY, 2);
		draw_new_dialog_box(InterfaceNdText, sX, sY, 10);

		put_string(sX + 55, sY + 100, DRAW_DIALOGBOX_SELLOR_REPAIR_ITEM11, GameColors::UILabel);
		put_string(sX + 55, sY + 120, DRAW_DIALOGBOX_SELLOR_REPAIR_ITEM9, GameColors::UILabel);
		put_string(sX + 55, sY + 135, DRAW_DIALOGBOX_SELLOR_REPAIR_ITEM10, GameColors::UILabel);
		break;
	}
}

bool DialogBox_SellOrRepair::on_click(short mouse_x, short mouse_y)
{
	short sX, sY;

	sX = Info().m_x;
	sY = Info().m_y;

	switch (Info().m_mode) {
	case 1:
	{
		CItem* cfg = m_game->get_item_config(m_game->m_item_list[Info().m_v1]->m_id_num);
		if ((mouse_x >= sX + 30) && (mouse_x <= sX + 30 + ui_layout::btn_size_x) && (mouse_y >= sY + ui_layout::btn_y) && (mouse_y <= sY + ui_layout::btn_y + ui_layout::btn_size_y)) {
			// Sell
			if (cfg) m_game->send_command(MsgId::CommandCommon, CommonType::ReqSellItemConfirm, 0, Info().m_v1, Info().m_v4, Info().m_v3, cfg->m_name);
			Info().m_mode = 3;
			return true;
		}
		if ((mouse_x >= sX + 154) && (mouse_x <= sX + 154 + ui_layout::btn_size_x) && (mouse_y >= sY + ui_layout::btn_y) && (mouse_y <= sY + ui_layout::btn_y + ui_layout::btn_size_y)) {
			// Cancel
			m_game->m_is_item_disabled[Info().m_v1] = false;
			m_game->m_dialog_box_manager.disable_dialog_box(DialogBoxId::SellOrRepair);
			return true;
		}
		break;
	}

	case 2:
	{
		CItem* cfg = m_game->get_item_config(m_game->m_item_list[Info().m_v1]->m_id_num);
		if ((mouse_x >= sX + 30) && (mouse_x <= sX + 30 + ui_layout::btn_size_x) && (mouse_y >= sY + ui_layout::btn_y) && (mouse_y <= sY + ui_layout::btn_y + ui_layout::btn_size_y)) {
			// Repair
			if (cfg) m_game->send_command(MsgId::CommandCommon, CommonType::ReqRepairItemConfirm, 0, Info().m_v1, 0, 0, cfg->m_name);
			Info().m_mode = 4;
			return true;
		}
		if ((mouse_x >= sX + 154) && (mouse_x <= sX + 154 + ui_layout::btn_size_x) && (mouse_y >= sY + ui_layout::btn_y) && (mouse_y <= sY + ui_layout::btn_y + ui_layout::btn_size_y)) {
			// Cancel
			m_game->m_is_item_disabled[Info().m_v1] = false;
			m_game->m_dialog_box_manager.disable_dialog_box(DialogBoxId::SellOrRepair);
			return true;
		}
		break;
	}
	}

	return false;
}
