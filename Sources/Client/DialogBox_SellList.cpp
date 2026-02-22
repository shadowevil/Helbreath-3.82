#include "DialogBox_SellList.h"
#include "CursorTarget.h"
#include "Game.h"
#include "ItemNameFormatter.h"
#include "GlobalDef.h"
#include "lan_eng.h"
#include <format>
#include <string>

using namespace hb::shared::net;
using namespace hb::shared::item;
using namespace hb::client::sprite_id;

DialogBox_SellList::DialogBox_SellList(CGame* game)
	: IDialogBox(DialogBoxId::SellList, game)
{
	set_default_rect(170 , 70 , 258, 339);
}

void DialogBox_SellList::on_draw(short mouse_x, short mouse_y, short z, char lb)
{
	if (!m_game->ensure_item_configs_loaded()) return;
	short sX = Info().m_x;
	short sY = Info().m_y;
	short size_x = Info().m_size_x;

	draw_new_dialog_box(InterfaceNdGame2, sX, sY, 2);
	draw_new_dialog_box(InterfaceNdText, sX, sY, 11);

	int empty_count = 0;
	draw_item_list(sX, sY, size_x, mouse_x, mouse_y, empty_count);

	if (empty_count == game_limits::max_sell_list) {
		draw_empty_list_message(sX, sY, size_x);
	}

	bool has_items = (empty_count < game_limits::max_sell_list);
	draw_buttons(sX, sY, mouse_x, mouse_y, has_items);
}

void DialogBox_SellList::draw_item_list(short sX, short sY, short size_x, short mouse_x, short mouse_y, int& empty_count)
{
	std::string txt;
	int row = 0;

	for (int i = 0; i < game_limits::max_sell_list; i++)
	{
		if (m_game->m_sell_item_list[i].index != -1)
		{
			int item_index = m_game->m_sell_item_list[i].index;
			auto itemInfo = item_name_formatter::get().format(m_game->m_item_list[item_index].get());
			auto effect = itemInfo.effect_text();
			auto extra = itemInfo.extra_text();

			bool hover = (mouse_x > sX + 25) && (mouse_x < sX + 250) && (mouse_y >= sY + 55 + row * 15) && (mouse_y <= sY + 55 + 14 + row * 15);

			if (m_game->m_sell_item_list[i].amount > 1)
			{
				// Multiple items — itemInfo.name already includes the count prefix
				if (hover)
					put_aligned_string(sX, sX + size_x, sY + 55 + row * 15, itemInfo.name.c_str(), GameColors::UIWhite);
				else if (itemInfo.is_special)
					put_aligned_string(sX, sX + size_x, sY + 55 + row * 15, itemInfo.name.c_str(), GameColors::UIItemName_Special);
				else
					put_aligned_string(sX, sX + size_x, sY + 55 + row * 15, itemInfo.name.c_str(), GameColors::UILabel);
			}
			else
			{
				// Single item
				if (hover)
				{
					if (effect.empty() && extra.empty())
					{
						put_aligned_string(sX, sX + size_x, sY + 55 + row * 15, itemInfo.name.c_str(), GameColors::UIWhite);
					}
					else
					{
						if ((itemInfo.name.size() + effect.size() + extra.size()) < 36)
						{
							if (!effect.empty() && !extra.empty())
								txt = std::format("{}({}, {})", itemInfo.name.c_str(), effect.c_str(), extra.c_str());
							else
								txt = std::format("{}({}{})", itemInfo.name.c_str(), effect.c_str(), extra.c_str());
							put_aligned_string(sX, sX + size_x, sY + 55 + row * 15, txt.c_str(), GameColors::UIWhite);
						}
						else
						{
							if (!effect.empty() && !extra.empty())
								txt = std::format("({}, {})", effect.c_str(), extra.c_str());
							else
								txt = std::format("({}{})", effect.c_str(), extra.c_str());
							put_aligned_string(sX, sX + size_x, sY + 55 + row * 15, itemInfo.name.c_str(), GameColors::UIWhite);
							put_aligned_string(sX, sX + size_x, sY + 55 + row * 15 + 15, txt.c_str(), GameColors::UIDisabled);
							row++;
						}
					}
				}
				else
				{
					if (effect.empty() && extra.empty())
					{
						if (itemInfo.is_special)
							put_aligned_string(sX, sX + size_x, sY + 55 + row * 15, itemInfo.name.c_str(), GameColors::UIItemName_Special);
						else
							put_aligned_string(sX, sX + size_x, sY + 55 + row * 15, itemInfo.name.c_str(), GameColors::UILabel);
					}
					else
					{
						if ((itemInfo.name.size() + effect.size() + extra.size()) < 36)
						{
							if (!effect.empty() && !extra.empty())
								txt = std::format("{}({}, {})", itemInfo.name.c_str(), effect.c_str(), extra.c_str());
							else
								txt = std::format("{}({}{})", itemInfo.name.c_str(), effect.c_str(), extra.c_str());

							if (itemInfo.is_special)
								put_aligned_string(sX, sX + size_x, sY + 55 + row * 15, txt.c_str(), GameColors::UIItemName_Special);
							else
								put_aligned_string(sX, sX + size_x, sY + 55 + row * 15, txt.c_str(), GameColors::UILabel);
						}
						else
						{
							if (itemInfo.is_special)
								put_aligned_string(sX, sX + size_x, sY + 55 + row * 15, itemInfo.name.c_str(), GameColors::UIItemName_Special);
							else
								put_aligned_string(sX, sX + size_x, sY + 55 + row * 15, itemInfo.name.c_str(), GameColors::UILabel);
						}
					}
				}
			}
		}
		else
		{
			empty_count++;
		}
		row++;
	}
}

void DialogBox_SellList::draw_empty_list_message(short sX, short sY, short size_x)
{
	put_aligned_string(sX, sX + size_x, sY + 55 + 30 + 282 - 117 - 170, DRAW_DIALOGBOX_SELL_LIST2);
	put_aligned_string(sX, sX + size_x, sY + 55 + 45 + 282 - 117 - 170, DRAW_DIALOGBOX_SELL_LIST3);
	put_aligned_string(sX, sX + size_x, sY + 55 + 60 + 282 - 117 - 170, DRAW_DIALOGBOX_SELL_LIST4);
	put_aligned_string(sX, sX + size_x, sY + 55 + 75 + 282 - 117 - 170, DRAW_DIALOGBOX_SELL_LIST5);
	put_aligned_string(sX, sX + size_x, sY + 55 + 95 + 282 - 117 - 170, DRAW_DIALOGBOX_SELL_LIST6);
	put_aligned_string(sX, sX + size_x, sY + 55 + 110 + 282 - 117 - 170, DRAW_DIALOGBOX_SELL_LIST7);
	put_aligned_string(sX, sX + size_x, sY + 55 + 125 + 282 - 117 - 170, DRAW_DIALOGBOX_SELL_LIST8);
	put_aligned_string(sX, sX + size_x, sY + 55 + 155 + 282 - 117 - 170, DRAW_DIALOGBOX_SELL_LIST9);
}

void DialogBox_SellList::draw_buttons(short sX, short sY, short mouse_x, short mouse_y, bool has_items)
{
	// Sell button (only enabled when there are items)
	if ((mouse_x >= sX + ui_layout::left_btn_x) && (mouse_x <= sX + ui_layout::left_btn_x + ui_layout::btn_size_x) &&
		(mouse_y >= sY + ui_layout::btn_y) && (mouse_y <= sY + ui_layout::btn_y + ui_layout::btn_size_y) && has_items)
		draw_new_dialog_box(InterfaceNdButton, sX + ui_layout::left_btn_x, sY + ui_layout::btn_y, 39);
	else
		draw_new_dialog_box(InterfaceNdButton, sX + ui_layout::left_btn_x, sY + ui_layout::btn_y, 38);

	// Cancel button
	if ((mouse_x >= sX + ui_layout::right_btn_x) && (mouse_x <= sX + ui_layout::right_btn_x + ui_layout::btn_size_x) &&
		(mouse_y >= sY + ui_layout::btn_y) && (mouse_y <= sY + ui_layout::btn_y + ui_layout::btn_size_y))
		draw_new_dialog_box(InterfaceNdButton, sX + ui_layout::right_btn_x, sY + ui_layout::btn_y, 17);
	else
		draw_new_dialog_box(InterfaceNdButton, sX + ui_layout::right_btn_x, sY + ui_layout::btn_y, 16);
}

bool DialogBox_SellList::on_click(short mouse_x, short mouse_y)
{
	short sX = Info().m_x;
	short sY = Info().m_y;

	// Check if clicking on an item in the list to remove it
	for (int i = 0; i < game_limits::max_sell_list; i++)
	{
		if ((mouse_x > sX + 25) && (mouse_x < sX + 250) && (mouse_y >= sY + 55 + i * 15) && (mouse_y <= sY + 55 + 14 + i * 15))
		{
			if (m_game->m_item_list[m_game->m_sell_item_list[i].index] != 0)
			{
				// Re-enable the item
				m_game->m_is_item_disabled[m_game->m_sell_item_list[i].index] = false;
				m_game->m_sell_item_list[i].index = -1;

				m_game->play_game_sound('E', 14, 5);

				// Compact the list
				for (int x = 0; x < game_limits::max_sell_list - 1; x++)
				{
					if (m_game->m_sell_item_list[x].index == -1)
					{
						m_game->m_sell_item_list[x].index = m_game->m_sell_item_list[x + 1].index;
						m_game->m_sell_item_list[x].amount = m_game->m_sell_item_list[x + 1].amount;

						m_game->m_sell_item_list[x + 1].index = -1;
						m_game->m_sell_item_list[x + 1].amount = 0;
					}
				}
			}
			return true;
		}
	}

	// Sell button
	if ((mouse_x >= sX + 30) && (mouse_x <= sX + 30 + ui_layout::btn_size_x) &&
		(mouse_y >= sY + ui_layout::btn_y) && (mouse_y <= sY + ui_layout::btn_y + ui_layout::btn_size_y))
	{
		m_game->send_command(MsgId::RequestSellItemList, 0, 0, 0, 0, 0, 0);
		m_game->play_game_sound('E', 14, 5);
		disable_this_dialog();
		return true;
	}

	// Cancel button
	if ((mouse_x >= sX + 154) && (mouse_x <= sX + 154 + ui_layout::btn_size_x) &&
		(mouse_y >= sY + ui_layout::btn_y) && (mouse_y <= sY + ui_layout::btn_y + ui_layout::btn_size_y))
	{
		m_game->play_game_sound('E', 14, 5);
		disable_this_dialog();
		return true;
	}

	return false;
}

bool DialogBox_SellList::on_item_drop(short mouse_x, short mouse_y)
{
	int item_id = CursorTarget::get_selected_id();
	if (item_id < 0 || item_id >= hb::shared::limits::MaxItems) return false;
	if (m_game->m_item_list[item_id] == nullptr) return false;
	if (m_game->m_is_item_disabled[item_id]) return false;
	if (m_game->m_player->m_Controller.get_command() < 0) return false;

	// Check if item is already in sell list
	for (int i = 0; i < game_limits::max_sell_list; i++)
	{
		if (m_game->m_sell_item_list[i].index == item_id)
		{
			add_event_list(BITEMDROP_SELLLIST1, 10);
			return false;
		}
	}

	// Can't sell gold or arrows
	if (m_game->m_item_list[item_id]->m_id_num == hb::shared::item::ItemId::Gold ||
		m_game->m_item_list[item_id]->m_id_num == hb::shared::item::ItemId::Arrow)
	{
		auto msg = std::format(NOTIFYMSG_CANNOT_SELL_ITEM3, m_game->m_item_list[item_id]->m_name);
		add_event_list(msg.c_str(), 10);
		return false;
	}

	// Can't sell broken items
	if (m_game->m_item_list[item_id]->m_cur_life_span == 0)
	{
		std::string G_cTxt;
		auto itemInfo2 = item_name_formatter::get().format(m_game->m_item_list[item_id].get());
		G_cTxt = std::format(NOTIFYMSG_CANNOT_SELL_ITEM2, itemInfo2.name.c_str());
		add_event_list(G_cTxt.c_str(), 10);
		return false;
	}

	// Stackable items - open quantity dialog
	CItem* cfg = m_game->get_item_config(m_game->m_item_list[item_id]->m_id_num);
	if (cfg && ((cfg->get_item_type() == ItemType::Consume) ||
		(cfg->get_item_type() == ItemType::Arrow)) &&
		(m_game->m_item_list[item_id]->m_count > 1))
	{
		auto& dropInfo = m_game->m_dialog_box_manager.Info(DialogBoxId::ItemDropExternal);
		dropInfo.m_x = mouse_x - 140;
		dropInfo.m_y = mouse_y - 70;
		if (dropInfo.m_y < 0) dropInfo.m_y = 0;
		dropInfo.m_v1 = m_game->m_player->m_player_x + 1;
		dropInfo.m_v2 = m_game->m_player->m_player_y + 1;
		dropInfo.m_v3 = 1001;
		dropInfo.m_v4 = item_id;
		std::memset(dropInfo.m_str, 0, sizeof(dropInfo.m_str));
		m_game->m_dialog_box_manager.enable_dialog_box(DialogBoxId::ItemDropExternal, item_id, static_cast<int64_t>(m_game->m_item_list[item_id]->m_count), 0);
		m_game->m_is_item_disabled[item_id] = true;
	}
	else
	{
		// Add single item to sell list
		for (int i = 0; i < game_limits::max_sell_list; i++)
		{
			if (m_game->m_sell_item_list[i].index == -1)
			{
				m_game->m_sell_item_list[i].index = item_id;
				m_game->m_sell_item_list[i].amount = 1;
				m_game->m_is_item_disabled[item_id] = true;
				return true;
			}
		}
		add_event_list(BITEMDROP_SELLLIST3, 10);
	}

	return true;
}
