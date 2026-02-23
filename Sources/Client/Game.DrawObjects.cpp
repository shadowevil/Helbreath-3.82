// Game.draw_objects.cpp: CGame partial implementation — draw_objects coordinator + dispatchers
//
//////////////////////////////////////////////////////////////////////

#include "Game.h"
#include "WeatherManager.h"
#include "ItemNameFormatter.h"
#include "ItemTooltip.h"
#include "ItemSpriteMetadata.h"
#include "RenderHelpers.h"
#include "EntityRenderState.h"
#include "ConfigManager.h"
#include "GameFonts.h"
#include "TextLibExt.h"
#include "lan_eng.h"



namespace dynamic_object = hb::shared::dynamic_object;

using namespace hb::shared::action;
using namespace hb::shared::direction;

using namespace hb::shared::item;
using namespace hb::client::config;
using namespace hb::client::sprite_id;


// Equipment sprite indices for character rendering (menu only)
struct MenuCharEquipment {
	int body, undies, hair, bodyArmor, armArmor, pants, boots, weapon, shield, mantle, helm;
	int weaponColor, shieldColor, armorColor, mantleColor, armColor, pantsColor, bootsColor, helmColor;
	bool skirtDraw;
};

// Calculate equipment indices for human characters (male/female) in menu
static void CalcHumanEquipment(const CEntityRenderState& state, bool female, MenuCharEquipment& eq)
{
	const auto& appr = state.m_appearance;

	// Walking uses pose 3, standing uses pose 2
	bool walking = appr.is_walking;
	int pose = walking ? 3 : 2;

	// Body index (still from m_sprite)
	eq.body = 500 + (state.m_owner_type - 1) * 8 * 15 + (pose * 8);

	// Cosmetics — still from m_sprite with old base IDs
	int UNDIES = female ? UndiesW : UndiesM;
	int HAIR   = female ? HairW  : HairM;
	eq.undies = UNDIES + appr.underwear_type * 15 + pose;
	eq.hair   = HAIR + appr.hair_style * 15 + pose;

	// Equipment — from m_equip_sprites via equip_sprite::index()
	eq.bodyArmor = (!appr.hide_armor && appr.armor_item_id > 0)  ? equip_sprite::index(female, appr.armor_display_id, pose)   : -1;
	eq.armArmor  = (appr.arm_item_id > 0)    ? equip_sprite::index(female, appr.arm_display_id, pose)    : -1;
	eq.pants     = (appr.pants_item_id > 0)   ? equip_sprite::index(female, appr.pants_display_id, pose)  : -1;
	eq.boots     = (appr.boots_item_id > 0)   ? equip_sprite::index(female, appr.boots_display_id, pose)  : -1;
	eq.mantle    = (appr.mantle_item_id > 0)  ? equip_sprite::index(female, appr.mantle_display_id, pose) : -1;
	eq.helm      = (appr.helm_item_id > 0)    ? equip_sprite::index(female, appr.helm_display_id, pose)   : -1;
	eq.weapon    = (appr.weapon_item_id > 0)  ? equip_sprite::index(female, appr.weapon_display_id, pose) : -1;
	eq.shield    = (appr.shield_item_id > 0)  ? equip_sprite::index(female, appr.shield_display_id, pose) : -1;

	// Female skirt check — from is_skirt flag computed at broadcast time
	eq.skirtDraw = female && appr.is_skirt;
}

hb::shared::sprite::BoundRect CGame::draw_object_on_move_for_menu(int indexX, int indexY, int sX, int sY, bool trans, uint32_t time, bool draw_shadow)
{
	if (m_entity_state.m_dir < 1 || m_entity_state.m_dir > 8) return {};

	// Extract equipment colors from packed appearance color
	MenuCharEquipment eq = {};
	eq.weaponColor = m_entity_state.m_appearance.weapon_color;
	eq.shieldColor = m_entity_state.m_appearance.shield_color;
	eq.armorColor  = m_entity_state.m_appearance.armor_color;
	eq.mantleColor = m_entity_state.m_appearance.mantle_color;
	eq.armColor    = m_entity_state.m_appearance.arm_color;
	eq.pantsColor  = m_entity_state.m_appearance.pants_color;
	eq.bootsColor  = m_entity_state.m_appearance.boots_color;
	eq.helmColor   = m_entity_state.m_appearance.helm_color;

	// Calculate equipment indices based on character type
	bool mob = false;
	switch (m_entity_state.m_owner_type) {
	case 1: case 2: case 3:  // Male
		CalcHumanEquipment(m_entity_state, false, eq);
		break;
	case 4: case 5: case 6:  // Female
		CalcHumanEquipment(m_entity_state, true, eq);
		break;
	default:  // Mob/NPC
		if (m_entity_state.m_owner_type < 10) return {};
		eq.body = Mob + (m_entity_state.m_owner_type - 10) * 8 * 7 + (1 * 8);
		eq.undies = eq.hair = eq.bodyArmor = eq.armArmor = -1;
		eq.boots = eq.pants = eq.weapon = eq.shield = eq.helm = eq.mantle = -1;
		mob = true;
		break;
	}
	// Helper lambdas for drawing with optional color tint
	int dirFrame = (m_entity_state.m_dir - 1) * 8 + m_entity_state.m_frame;
	int hairColor = m_entity_state.m_appearance.hair_color;

	// Equipment draws from m_equip_sprites, cosmetics from m_sprite
	auto drawCosmeticLayer = [&](int idx, int color) {
		if (idx == -1) return;
		if (color == 0)
			m_sprite[idx]->draw(sX, sY, dirFrame);
		else
			m_sprite[idx]->draw(sX, sY, dirFrame, hb::shared::sprite::DrawParams::tint(GameColors::Items[color].r, GameColors::Items[color].g, GameColors::Items[color].b));
	};

	auto drawEquipLayer = [&](int idx, int color) {
		if (idx == -1) return;
		if (color == 0)
			m_equip_sprites[idx]->draw(sX, sY, dirFrame);
		else
			m_equip_sprites[idx]->draw(sX, sY, dirFrame, hb::shared::sprite::DrawParams::tint(GameColors::Items[color].r, GameColors::Items[color].g, GameColors::Items[color].b));
	};

	auto drawWeapon = [&]() {
		if (eq.weapon == -1) return;
		if (eq.weaponColor == 0)
			m_equip_sprites[eq.weapon]->draw(sX, sY, dirFrame);
		else
			m_equip_sprites[eq.weapon]->draw(sX, sY, dirFrame, hb::shared::sprite::DrawParams::tint(GameColors::Weapons[eq.weaponColor].r, GameColors::Weapons[eq.weaponColor].g, GameColors::Weapons[eq.weaponColor].b));
	};

	auto drawMantle = [&](int order) {
		if (eq.mantle != -1 && mantle_draw_order[m_entity_state.m_dir] == order)
			drawEquipLayer(eq.mantle, eq.mantleColor);
	};

	// Check if mob type should skip shadow
	auto shouldSkipShadow = [&]() {
		switch (m_entity_state.m_owner_type) {
		case hb::shared::owner::Slime: case hb::shared::owner::EnergySphere: case hb::shared::owner::TigerWorm: case hb::shared::owner::Catapult: case hb::shared::owner::CannibalPlant: case hb::shared::owner::IceGolem: case hb::shared::owner::Abaddon: case hb::shared::owner::Gate:
			return true;
		default:
			return false;
		}
	};

	// draw body shadow
	if (draw_shadow && !shouldSkipShadow() && config_manager::get().get_detail_level() != 0 && !mob)
		m_sprite[eq.body + (m_entity_state.m_dir - 1)]->draw(sX, sY, m_entity_state.m_frame, hb::shared::sprite::DrawParams::shadow());

	// draw weapon first if drawing order is 1
	if (weapon_draw_order[m_entity_state.m_dir] == 1)
		drawWeapon();

	// draw body
	if (mob)
		m_sprite[eq.body + (m_entity_state.m_dir - 1)]->draw(sX, sY, m_entity_state.m_frame, hb::shared::sprite::DrawParams::alpha_blend(0.5f));
	else
		m_sprite[eq.body + (m_entity_state.m_dir - 1)]->draw(sX, sY, m_entity_state.m_frame);

	// draw equipment layers (back-to-front order)
	drawMantle(0);  // Mantle behind body
	drawCosmeticLayer(eq.undies, 0);  // Undies from m_sprite

	// Hair (cosmetic, from m_sprite, only if no helm)
	if (eq.hair != -1 && eq.helm == -1)
	{
		const auto& hc = GameColors::Hair[hairColor];
		m_sprite[eq.hair]->draw(sX, sY, dirFrame, hb::shared::sprite::DrawParams::tint(hc.r, hc.g, hc.b));
	}

	// Boots before pants if wearing skirt
	if (eq.skirtDraw)
		drawEquipLayer(eq.boots, eq.bootsColor);

	drawEquipLayer(eq.pants, eq.pantsColor);
	drawEquipLayer(eq.armArmor, eq.armColor);

	// Boots after pants if not wearing skirt
	if (!eq.skirtDraw)
		drawEquipLayer(eq.boots, eq.bootsColor);

	drawEquipLayer(eq.bodyArmor, eq.armorColor);
	drawEquipLayer(eq.helm, eq.helmColor);
	drawMantle(2);  // Mantle over armor
	drawEquipLayer(eq.shield, eq.shieldColor);
	drawMantle(1);  // Mantle in front

	// draw weapon last if drawing order is not 1
	if (weapon_draw_order[m_entity_state.m_dir] != 1)
		drawWeapon();

	// Chat message
	if (m_entity_state.m_chat_index != 0)
	{
		if (m_floating_text.is_occupied(m_entity_state.m_chat_index))
			m_floating_text.draw_single(m_entity_state.m_chat_index, sX, sY, m_cur_time, m_Renderer);
		else
			m_map_data->clear_chat_msg(indexX, indexY);
	}

	m_entity_state.m_move_offset_x = 0;
	m_entity_state.m_move_offset_y = 0;
	return m_sprite[eq.body + (m_entity_state.m_dir - 1)]->GetBoundRect();
}

// --- DrawObject dispatcher functions ---

hb::shared::sprite::BoundRect CGame::draw_object_on_stop(int indexX, int indexY, int sX, int sY, bool trans, uint32_t time)
{
	if (m_entity_state.is_player())
		return m_player_renderer.draw_stop(indexX, indexY, sX, sY, trans, time);
	else
		return m_npc_renderer.draw_stop(indexX, indexY, sX, sY, trans, time);
}

hb::shared::sprite::BoundRect CGame::draw_object_on_move(int indexX, int indexY, int sX, int sY, bool trans, uint32_t time)
{
	if (m_entity_state.is_player())
		return m_player_renderer.draw_move(indexX, indexY, sX, sY, trans, time);
	else
		return m_npc_renderer.draw_move(indexX, indexY, sX, sY, trans, time);
}

hb::shared::sprite::BoundRect CGame::draw_object_on_run(int indexX, int indexY, int sX, int sY, bool trans, uint32_t time)
{
	if (m_entity_state.is_player())
		return m_player_renderer.draw_run(indexX, indexY, sX, sY, trans, time);
	else
		return m_npc_renderer.draw_run(indexX, indexY, sX, sY, trans, time);
}

hb::shared::sprite::BoundRect CGame::draw_object_on_attack(int indexX, int indexY, int sX, int sY, bool trans, uint32_t time)
{
	if (m_entity_state.is_player())
		return m_player_renderer.draw_attack(indexX, indexY, sX, sY, trans, time);
	else
		return m_npc_renderer.draw_attack(indexX, indexY, sX, sY, trans, time);
}

hb::shared::sprite::BoundRect CGame::draw_object_on_attack_move(int indexX, int indexY, int sX, int sY, bool trans, uint32_t time)
{
	if (m_entity_state.is_player())
		return m_player_renderer.draw_attack_move(indexX, indexY, sX, sY, trans, time);
	else
		return m_npc_renderer.draw_attack_move(indexX, indexY, sX, sY, trans, time);
}

hb::shared::sprite::BoundRect CGame::draw_object_on_magic(int indexX, int indexY, int sX, int sY, bool trans, uint32_t time)
{
	if (m_entity_state.is_player())
		return m_player_renderer.draw_magic(indexX, indexY, sX, sY, trans, time);
	else
		return m_npc_renderer.draw_magic(indexX, indexY, sX, sY, trans, time);
}

hb::shared::sprite::BoundRect CGame::draw_object_on_get_item(int indexX, int indexY, int sX, int sY, bool trans, uint32_t time)
{
	if (m_entity_state.is_player())
		return m_player_renderer.draw_get_item(indexX, indexY, sX, sY, trans, time);
	else
		return m_npc_renderer.draw_get_item(indexX, indexY, sX, sY, trans, time);
}

hb::shared::sprite::BoundRect CGame::draw_object_on_damage(int indexX, int indexY, int sX, int sY, bool trans, uint32_t time)
{
	if (m_entity_state.is_player())
		return m_player_renderer.draw_damage(indexX, indexY, sX, sY, trans, time);
	else
		return m_npc_renderer.draw_damage(indexX, indexY, sX, sY, trans, time);
}

hb::shared::sprite::BoundRect CGame::draw_object_on_damage_move(int indexX, int indexY, int sX, int sY, bool trans, uint32_t time)
{
	if (m_entity_state.is_player())
		return m_player_renderer.draw_damage_move(indexX, indexY, sX, sY, trans, time);
	else
		return m_npc_renderer.draw_damage_move(indexX, indexY, sX, sY, trans, time);
}

hb::shared::sprite::BoundRect CGame::draw_object_on_dying(int indexX, int indexY, int sX, int sY, bool trans, uint32_t time)
{
	if (m_entity_state.is_player())
		return m_player_renderer.draw_dying(indexX, indexY, sX, sY, trans, time);
	else
		return m_npc_renderer.draw_dying(indexX, indexY, sX, sY, trans, time);
}

hb::shared::sprite::BoundRect CGame::draw_object_on_dead(int indexX, int indexY, int sX, int sY, bool trans, uint32_t time)
{
	if (m_entity_state.is_player())
		return m_player_renderer.draw_dead(indexX, indexY, sX, sY, trans, time);
	else
		return m_npc_renderer.draw_dead(indexX, indexY, sX, sY, trans, time);
}

// --- draw_objects coordinator ---

void CGame::draw_objects(short pivot_x, short pivot_y, short div_x, short div_y, short mod_x, short mod_y, short mouse_x, short mouse_y)
{
	int ix, iy, indexX, indexY, dX, dY, dvalue;
	char item_color;
	bool is_player_drawed = false;
	bool ret = false;
	short obj_spr, obj_spr_frame, dynamic_object, dynamic_object_frame;
	char dynamic_object_data1, dynamic_object_data2, dynamic_object_data3, dynamic_object_data4;
	// Xmas tree bulb positions — static arrays shared across all trees (same bulb pattern each frame).
	// 100 entries matches the loop count at line ~725; indexed by j in [0..99].
	static int ix1[100];
	static int iy2[100];
	static int xmas_tree_bulb_delay = 76;
	int idelay = 75;

	// Item's desc on floor
	uint32_t item_attr, item_selected_attr;
	int item_selectedx, item_selectedy;
	short item_id, item_selected_id = -1;

	int res_x = LOGICAL_MAX_X();
	int res_y = LOGICAL_MAX_Y();
	int res_msy = LOGICAL_HEIGHT() - 49;

	if (div_y < 0 || div_x < 0) return;

	// initialize Picking system for this frame
	CursorTarget::begin_frame();

	uint32_t time = m_cur_time;

	// Pre-calculate map data bounds for efficient boundary checking
	const short mapMinX = m_map_data->m_pivot_x;
	const short mapMaxX = m_map_data->m_pivot_x + MapDataSizeX;
	const short mapMinY = m_map_data->m_pivot_y;
	const short mapMaxY = m_map_data->m_pivot_y + MapDataSizeY;

	// Tile-based loop bounds (much cleaner than pixel-based)
	// Buffer: 7 tiles around visible area for smooth object sliding
	// Extra 2 tiles on bottom for layering/depth sorting of tall objects
	constexpr int TILE_SIZE = 32;
	constexpr int BUFFER_TILES = 7;
	constexpr int EXTRA_BOTTOM_TILES = 2;  // For depth sorting of tall objects

	const int visibleTilesX = (res_x / TILE_SIZE) + 1;  // ~20 tiles
	const int visibleTilesY = (res_y / TILE_SIZE) + 1;  // ~15 tiles

	const int startTileX = -BUFFER_TILES;
	const int endTileX = visibleTilesX + BUFFER_TILES;
	const int startTileY = -BUFFER_TILES;
	const int endTileY = visibleTilesY + BUFFER_TILES + EXTRA_BOTTOM_TILES;

	// Visibility bounds in pixels (for culling non-visible tiles from detailed processing)
	const int visMinX = -mod_x;
	const int visMaxX = res_x + 16;
	const int visMinY = -mod_y;
	const int visMaxY = res_y + 32 + 16;

	// Loop over tiles, calculate pixel positions when needed
	for (int tileY = startTileY; tileY <= endTileY; tileY++)
	{
		indexY = div_y + pivot_y + tileY;
		iy = tileY * TILE_SIZE - mod_y;

		for (int tileX = startTileX; tileX <= endTileX; tileX++)
		{
			indexX = div_x + pivot_x + tileX;
			ix = tileX * TILE_SIZE - mod_x;

			dynamic_object = 0;
			ret = false;
			if ((ix >= visMinX) && (ix <= visMaxX) && (iy >= visMinY) && (iy <= visMaxY))
			{
				m_entity_state.m_object_id = m_entity_state.m_owner_type = 0; m_entity_state.m_status.clear();
				m_entity_state.m_appearance.clear();
				m_entity_state.m_dir = direction{}; m_entity_state.m_frame = 0;
				m_entity_state.m_effect_type = m_entity_state.m_effect_frame = m_entity_state.m_chat_index = 0;
				m_entity_state.m_name.fill('\0');
				if ((indexX < mapMinX) || (indexX > mapMaxX) ||
					(indexY < mapMinY) || (indexY > mapMaxY))
				{
					item_id = 0;
					ret = false;
					item_color = 0;
					item_attr = 0;
				}
				else
				{
					m_entity_state.m_data_x = dX = indexX - mapMinX;
					m_entity_state.m_data_y = dY = indexY - mapMinY;
					m_entity_state.m_object_id = m_map_data->m_data[dX][dY].m_dead_object_id;
					m_entity_state.m_owner_type = m_map_data->m_data[dX][dY].m_dead_owner_type;
					m_entity_state.m_npc_config_id = m_map_data->m_data[dX][dY].m_dead_npc_config_id;
					m_entity_state.m_dir = m_map_data->m_data[dX][dY].m_dead_dir;
					m_entity_state.m_appearance = m_map_data->m_data[dX][dY].m_dead_appearance;
					m_entity_state.m_frame = m_map_data->m_data[dX][dY].m_dead_owner_frame;
					m_entity_state.m_chat_index = m_map_data->m_data[dX][dY].m_dead_chat_msg;
					m_entity_state.m_status = m_map_data->m_data[dX][dY].m_deadStatus;
					std::snprintf(m_entity_state.m_name.data(), m_entity_state.m_name.size(), "%s", m_map_data->m_data[dX][dY].m_dead_owner_name.c_str());
					item_id = m_map_data->m_data[dX][dY].m_item_id;
					item_attr = m_map_data->m_data[dX][dY].m_item_attr;
					item_color = m_map_data->m_data[dX][dY].m_item_color & 0x0F;
					dynamic_object = m_map_data->m_data[dX][dY].m_dynamic_object_type;
					dynamic_object_frame = static_cast<short>(m_map_data->m_data[dX][dY].m_dynamic_object_frame);
					dynamic_object_data1 = m_map_data->m_data[dX][dY].m_dynamic_object_data_1;
					dynamic_object_data2 = m_map_data->m_data[dX][dY].m_dynamic_object_data_2;
					dynamic_object_data3 = m_map_data->m_data[dX][dY].m_dynamic_object_data_3;
					dynamic_object_data4 = m_map_data->m_data[dX][dY].m_dynamic_object_data_4;

					ret = true;
				}

				if ((ret == true) && (item_id != 0) && m_item_config_list[item_id] != 0)
				{
					auto ground_draw = get_item_draw(m_item_config_list[item_id]->m_display_id, item_atlas::ground, m_item_config_list[item_id]->sprite_is_female());

					// Center ground item sprite on tile, offset by half-tile to align with tile center
					auto rect = ground_draw.sprite->GetFrameRect(ground_draw.frame);
					int cx = ix + (TILE_SIZE - rect.width) / 2 - (TILE_SIZE / 2);
					int cy = iy + (TILE_SIZE - rect.height) / 2 - (TILE_SIZE / 2);

					if (item_color == 0)
					{
						hb::shared::sprite::DrawParams params;
						params.m_ignore_pivot = true;
						ground_draw.sprite->draw(cx, cy, ground_draw.frame, params);
					}
					else
					{
						int eq = m_item_config_list[item_id]->m_equip_pos;
						bool is_weapon = (eq == hb::shared::item::to_int(hb::shared::item::EquipPos::LeftHand) ||
							eq == hb::shared::item::to_int(hb::shared::item::EquipPos::RightHand) ||
							eq == hb::shared::item::to_int(hb::shared::item::EquipPos::TwoHand));
						const auto& tint = is_weapon ? GameColors::Weapons[item_color] : GameColors::Items[item_color];
						auto params = hb::shared::sprite::DrawParams::tint(tint.r, tint.g, tint.b);
						params.m_ignore_pivot = true;
						ground_draw.sprite->draw(cx, cy, ground_draw.frame, params);
					}

					if (hb::shared::input::is_shift_down() && mouse_x >= ix - 16 && mouse_y >= iy - 16 && mouse_x <= ix + 16 && mouse_y <= iy + 16) {
						item_selected_id = item_id;
						item_selected_attr = item_attr;
						item_selectedx = ix;
						item_selectedy = iy;
					}

					// Test ground item with Picking system
					CursorTarget::test_ground_item(ix, iy, res_msy);
				}

				if ((ret == true) && (m_entity_state.m_object_id != 0))
				{
					auto dead_bounds = draw_object_on_dead(indexX, indexY, ix, iy, false, time);

					// Enable hover name for player corpses
					if (m_entity_state.is_player())
					{
						TargetObjectInfo info = {};
						info.m_object_id = m_entity_state.m_object_id;
						info.m_map_x = indexX;
						info.m_map_y = indexY;
						info.m_screen_x = ix;
						info.m_screen_y = iy;
						info.m_data_x = m_entity_state.m_data_x;
						info.m_data_y = m_entity_state.m_data_y;
						info.m_owner_type = m_entity_state.m_owner_type;
						info.m_npc_config_id = m_entity_state.m_npc_config_id;
						info.m_action = ObjectDead;
						info.m_direction = m_entity_state.m_dir;
						info.m_frame = m_entity_state.m_frame;
						info.m_name = m_entity_state.m_name.data();
						info.m_appearance = m_entity_state.m_appearance;
						info.m_status = m_entity_state.m_status;
						info.m_type = FocusedObjectType::DeadBody;
						CursorTarget::test_object(dead_bounds, info, iy, res_msy);
					}
				}

				m_entity_state.m_object_id = m_entity_state.m_owner_type = 0; m_entity_state.m_status.clear();
				m_entity_state.m_appearance.clear();
				m_entity_state.m_frame = 0; m_entity_state.m_dir = direction{};
				m_entity_state.m_effect_type = m_entity_state.m_effect_frame = m_entity_state.m_chat_index = 0;
				m_entity_state.m_name.fill('\0');

				if ((indexX < mapMinX) || (indexX > mapMaxX) ||
					(indexY < mapMinY) || (indexY > mapMaxY))
				{
					item_id = 0;
					ret = false;
				}
				else
				{
					m_entity_state.m_data_x = dX = indexX - mapMinX;
					m_entity_state.m_data_y = dY = indexY - mapMinY;
					m_entity_state.m_object_id = m_map_data->m_data[dX][dY].m_object_id;
					m_entity_state.m_owner_type = m_map_data->m_data[dX][dY].m_owner_type;
					m_entity_state.m_npc_config_id = m_map_data->m_data[dX][dY].m_npc_config_id;
					m_entity_state.m_action = m_map_data->m_data[dX][dY].m_animation.m_action;
					m_entity_state.m_status = m_map_data->m_data[dX][dY].m_status;
					m_entity_state.m_dir = m_map_data->m_data[dX][dY].m_animation.m_dir;
					m_entity_state.m_appearance = m_map_data->m_data[dX][dY].m_appearance;
					m_entity_state.m_frame = m_map_data->m_data[dX][dY].m_animation.m_current_frame;
					m_entity_state.m_chat_index = m_map_data->m_data[dX][dY].m_chat_msg;
					m_entity_state.m_effect_type = m_map_data->m_data[dX][dY].m_effect_type;
					m_entity_state.m_effect_frame = m_map_data->m_data[dX][dY].m_effect_frame;

					std::snprintf(m_entity_state.m_name.data(), m_entity_state.m_name.size(), "%s", m_map_data->m_data[dX][dY].m_owner_name.c_str());
					ret = true;

					if (m_ilusion_owner_h != 0)
					{
						if ((strcmp(m_entity_state.m_name.data(), m_player->m_player_name.c_str()) != 0) && (!hb::shared::owner::is_npc(m_entity_state.m_owner_type)))
						{
							m_entity_state.m_owner_type = m_ilusion_owner_type;
							m_entity_state.m_status = m_player->m_illusionStatus;
							m_entity_state.m_appearance = m_player->m_illusionAppearance;
						}
					}
				}

				if ((ret == true) && (m_entity_state.m_name[0] != '\0'))
				{
					m_entity_state.m_move_offset_x = 0;
					m_entity_state.m_move_offset_y = 0;
					hb::shared::sprite::BoundRect bounds = {0, -1, 0, 0};
					switch (m_entity_state.m_action) {
					case Type::stop:
						bounds = draw_object_on_stop(indexX, indexY, ix, iy, false, time);
						break;

					case Type::Move:
						bounds = draw_object_on_move(indexX, indexY, ix, iy, false, time);
						break;

					case Type::DamageMove:
						bounds = draw_object_on_damage_move(indexX, indexY, ix, iy, false, time);
						break;

					case Type::Run:
						bounds = draw_object_on_run(indexX, indexY, ix, iy, false, time);
						break;

					case Type::Attack:
						bounds = draw_object_on_attack(indexX, indexY, ix, iy, false, time);
						break;

					case Type::AttackMove:
						bounds = draw_object_on_attack_move(indexX, indexY, ix, iy, false, time);
						break;

					case Type::Magic:
						bounds = draw_object_on_magic(indexX, indexY, ix, iy, false, time);
						break;

					case Type::GetItem:
						bounds = draw_object_on_get_item(indexX, indexY, ix, iy, false, time);
						break;

					case Type::Damage:
						bounds = draw_object_on_damage(indexX, indexY, ix, iy, false, time);
						break;

					case Type::Dying:
						bounds = draw_object_on_dying(indexX, indexY, ix, iy, false, time);
						break;
					}

					// Build picking info for living object
					TargetObjectInfo info = {};
					info.m_object_id = m_entity_state.m_object_id;
					info.m_map_x = indexX;
					info.m_map_y = indexY;
					info.m_screen_x = ix;
					info.m_screen_y = iy;
					info.m_data_x = m_entity_state.m_data_x;
					info.m_data_y = m_entity_state.m_data_y;
					info.m_owner_type = m_entity_state.m_owner_type;
					info.m_npc_config_id = m_entity_state.m_npc_config_id;
					info.m_action = m_entity_state.m_action;
					info.m_direction = m_entity_state.m_dir;
					info.m_frame = m_entity_state.m_frame;
					info.m_name = m_entity_state.m_name.data();
					info.m_appearance = m_entity_state.m_appearance;
					info.m_status = m_entity_state.m_status;
					// Determine type based on owner type
					info.m_type = (m_entity_state.is_player()) ?
						FocusedObjectType::Player : FocusedObjectType::NPC;
					CursorTarget::test_object(bounds, info, iy, res_msy);

					if (m_entity_state.m_object_id == m_player->m_player_object_id)
					{
						// Camera is now updated in on_render() before drawing, so we don't need to update it here
						// This ensures viewport and entity position use the same motion offset
						m_player_rect = m_body_rect;
						is_player_drawed = true;
					}
				}
			}

			// CLEROTH - Object sprites on tiles
			// Bounds check for tile array access (752x752)
			if (indexX >= 0 && indexX < 752 && indexY >= 0 && indexY < 752)
			{
				obj_spr = m_map_data->m_tile[indexX][indexY].m_sObjectSprite;
				obj_spr_frame = m_map_data->m_tile[indexX][indexY].m_sObjectSpriteFrame;
			}
			else
			{
				obj_spr = 0;
				obj_spr_frame = 0;
			}

			if (obj_spr != 0)
			{
				if ((obj_spr < 100) || (obj_spr >= 200))
				{
					switch (obj_spr) {
					case 200:
					case 223:
						m_tile_spr[obj_spr]->draw(ix - 16, iy - 16, obj_spr_frame, hb::shared::sprite::DrawParams::shadow());
						break;

					case 224:
						switch (obj_spr_frame) {
						case hb::shared::owner::Tom:
						case hb::shared::owner::Dummy:
						case hb::shared::owner::EnergySphere:
						case hb::shared::owner::ArrowGuardTower:
						case hb::shared::owner::CannonGuardTower:
						case hb::shared::owner::ManaCollector:
							break;
						default:
							m_tile_spr[obj_spr]->draw(ix - 16, iy - 16, obj_spr_frame, hb::shared::sprite::DrawParams::shadow());
							break;
						}
					}
					if (config_manager::get().get_detail_level() == 0) // Special Grass & Flowers
					{
						if ((obj_spr != 6) && (obj_spr != 9))
							m_tile_spr[obj_spr]->draw(ix - 16, iy - 16, obj_spr_frame);
					}
					else
					{
						m_tile_spr[obj_spr]->draw(ix - 16, iy - 16, obj_spr_frame);
					}

					switch (obj_spr) {
					case 223:
						if (obj_spr_frame == 4)
						{
							if (weather_manager::get().is_night()) //nuit
							{
								// Lamp fixture lights (the actual light sources on the lamp)
								m_effect_sprites[0]->draw(ix + 2, iy - 147, 1, hb::shared::sprite::DrawParams::additive_colored(255, 230, 180, 0.8f));
								m_effect_sprites[0]->draw(ix + 16, iy - 94, 1, hb::shared::sprite::DrawParams::additive_colored(255, 230, 180, 0.8f));
								m_effect_sprites[0]->draw(ix - 19, iy - 126, 1, hb::shared::sprite::DrawParams::additive_colored(255, 230, 180, 0.8f));
							}
						}
						break;

					case 370: // nuit
						if (((time - m_env_effect_time) > 400) && (obj_spr_frame == 9) && (weather_manager::get().is_night())) m_effect_manager->add_effect(EffectType::MS_CRUSADE_CASTING, m_Camera.get_x() + ix - 16 + 30, m_Camera.get_y() + iy - 16 - 334, 0, 0, 0, 0);
						if (((time - m_env_effect_time) > 400) && (obj_spr_frame == 11) && (weather_manager::get().is_night())) m_effect_manager->add_effect(EffectType::MS_CRUSADE_CASTING, m_Camera.get_x() + ix - 16 + 17, m_Camera.get_y() + iy - 16 - 300, 0, 0, 0, 0);
						break;

					case 374: // nuit
						if (((time - m_env_effect_time) > 400) && (obj_spr_frame == 2) && (weather_manager::get().is_night())) m_effect_manager->add_effect(EffectType::MS_CRUSADE_CASTING, m_Camera.get_x() + ix - 7, m_Camera.get_y() + iy - 122, 0, 0, 0, 0);
						if (((time - m_env_effect_time) > 400) && (obj_spr_frame == 6) && (weather_manager::get().is_night())) m_effect_manager->add_effect(EffectType::MS_CRUSADE_CASTING, m_Camera.get_x() + ix - 14, m_Camera.get_y() + iy - 321, 0, 0, 0, 0);
						if (((time - m_env_effect_time) > 400) && (obj_spr_frame == 7) && (weather_manager::get().is_night())) m_effect_manager->add_effect(EffectType::MS_CRUSADE_CASTING, m_Camera.get_x() + ix + 7, m_Camera.get_y() + iy - 356, 0, 0, 0, 0);
						break;

					case 376: // nuit
						if (((time - m_env_effect_time) > 400) && (obj_spr_frame == 12) && (weather_manager::get().is_night())) {
							m_effect_manager->add_effect(EffectType::MS_CRUSADE_CASTING, m_Camera.get_x() + ix - 16, m_Camera.get_y() + iy - 346, 0, 0, 0, 0);
							m_effect_manager->add_effect(EffectType::MS_CRUSADE_CASTING, m_Camera.get_x() + ix + 11, m_Camera.get_y() + iy - 308, 0, 0, 0, 0);
						}
						break;

					case 378: // nuit
						if (((time - m_env_effect_time) > 400) && (obj_spr_frame == 11) && (weather_manager::get().is_night())) m_effect_manager->add_effect(EffectType::MS_CRUSADE_CASTING, m_Camera.get_x() + ix, m_Camera.get_y() + iy - 91, 0, 0, 0, 0);
						break;

					case 382: // nuit
						if (((time - m_env_effect_time) > 400) && (obj_spr_frame == 9) && (weather_manager::get().is_night())) {
							m_effect_manager->add_effect(EffectType::MS_CRUSADE_CASTING, m_Camera.get_x() + ix + 73, m_Camera.get_y() + iy - 264, 0, 0, 0, 0);
							m_effect_manager->add_effect(EffectType::MS_CRUSADE_CASTING, m_Camera.get_x() + ix + 23, m_Camera.get_y() + iy - 228, 0, 0, 0, 0);
						}
						break;

					case 429:
						if (((time - m_env_effect_time) > 400) && (obj_spr_frame == 2)) m_effect_manager->add_effect(EffectType::MS_CRUSADE_CASTING, m_Camera.get_x() + ix - 15, m_Camera.get_y() + iy - 224, 0, 0, 0, 0);
						break;
					}
				}
				else // sprites 100..199: Trees and tree shadows
				{
					m_tile_spr[obj_spr]->CalculateBounds(ix - 16, iy - 16, obj_spr_frame);
					if (config_manager::get().get_detail_level() == 0)
					{
						if (obj_spr < 100 + 11) m_tile_spr[100 + 4]->draw(ix - 16, iy - 16, obj_spr_frame);
						else if (obj_spr < 100 + 23) m_tile_spr[100 + 9]->draw(ix - 16, iy - 16, obj_spr_frame);
						else if (obj_spr < 100 + 32) m_tile_spr[100 + 23]->draw(ix - 16, iy - 16, obj_spr_frame);
						else m_tile_spr[100 + 32]->draw(ix - 16, iy - 16, obj_spr_frame);
					}
					else
					{
						// obj_spr is [100..199] here; obj_spr+50 is the tree shadow sprite [150..249].
						// SpriteCollection returns NullSprite for missing entries, so no crash if shadow is absent.
						if ((is_player_drawed == true) && (m_tile_spr[obj_spr]->GetBoundRect().top <= m_player_rect.Top()) && (m_tile_spr[obj_spr]->GetBoundRect().bottom >= m_player_rect.Bottom()) &&
							(config_manager::get().get_detail_level() >= 2) && (m_tile_spr[obj_spr]->GetBoundRect().left <= m_player_rect.Left()) && (m_tile_spr[obj_spr]->GetBoundRect().right >= m_player_rect.Right()))
						{
							m_tile_spr[obj_spr + 50]->draw(ix, iy, obj_spr_frame, hb::shared::sprite::DrawParams::fade());
							m_tile_spr[obj_spr]->draw(ix - 16, iy - 16, obj_spr_frame, hb::shared::sprite::DrawParams::average());
						}
						else
						{
							// Normal rendering - draw shadow and tree opaque (matches original PutSpriteFast)
							m_tile_spr[obj_spr + 50]->draw(ix, iy, obj_spr_frame);
							m_tile_spr[obj_spr]->draw(ix - 16, iy - 16, obj_spr_frame);
						}
						if (m_is_xmas == true)
						{
							if (weather_manager::get().is_night()) // nuit
							{
								if (xmas_tree_bulb_delay < 0 || xmas_tree_bulb_delay > idelay + 1) xmas_tree_bulb_delay = 0;
								if (xmas_tree_bulb_delay > idelay)
								{
									for (int i = 0; i < 100; i++) {
										ix1[i] = 1 * (rand() % 400) - 200;
										iy2[i] = -1 * (rand() % 300);
									}
									xmas_tree_bulb_delay = 0;
								}
								else xmas_tree_bulb_delay++;

								for (int j = 0; j < 100; j++)
								{
									if (m_tile_spr[obj_spr]->CheckCollision(ix - 16, iy - 16, obj_spr_frame, ix + ix1[j], iy + iy2[j]))
									{
										m_effect_sprites[66 + (j % 6)]->draw(ix + ix1[j], iy + iy2[j], (xmas_tree_bulb_delay >> 2), hb::shared::sprite::DrawParams::alpha_blend(0.5f));
									}
								}
							}
						}
					}
				}
			}

			// Dynamic Object
			if ((ret == true) && (dynamic_object != 0))
			{
				switch (dynamic_object) {
				case dynamic_object::PCloudBegin:	// 10
					if (dynamic_object_frame >= 0)
						m_effect_sprites[23]->draw(ix + (rand() % 2), iy + (rand() % 2), dynamic_object_frame, hb::shared::sprite::DrawParams{0.5f, 0, 0, 0, false});
					break;

				case dynamic_object::PCloudLoop:		// 11
					m_effect_sprites[23]->draw(ix + (rand() % 2), iy + (rand() % 2), dynamic_object_frame + 8, hb::shared::sprite::DrawParams{0.5f, 0, 0, 0, false});
					break;

				case dynamic_object::PCloudEnd:		// 12
					m_effect_sprites[23]->draw(ix + (rand() % 2), iy + (rand() % 2), dynamic_object_frame + 16, hb::shared::sprite::DrawParams{0.5f, 0, 0, 0, false});
					break;

				case dynamic_object::IceStorm:		// 8
					dvalue = (rand() % 5) * (-1);
					m_effect_sprites[0]->draw(ix, iy, 1, hb::shared::sprite::DrawParams::tinted_alpha(192 + 2 * dvalue, 192 + 2 * dvalue, 192 + 2 * dvalue, 0.7f));
					m_effect_sprites[13]->draw(ix, iy, dynamic_object_frame, hb::shared::sprite::DrawParams{0.7f, 0, 0, 0, false});
					break;

				case dynamic_object::Fire:			// 1
				case dynamic_object::Fire3:			// 14
					switch (rand() % 3) {
					case 0: m_effect_sprites[0]->draw(ix, iy, 1, hb::shared::sprite::DrawParams{0.25f, 0, 0, 0, false}); break;
					case 1: m_effect_sprites[0]->draw(ix, iy, 1, hb::shared::sprite::DrawParams{0.5f, 0, 0, 0, false}); break;
					case 2: m_effect_sprites[0]->draw(ix, iy, 1, hb::shared::sprite::DrawParams{0.7f, 0, 0, 0, false}); break;
					}
					m_effect_sprites[9]->draw(ix, iy, dynamic_object_frame / 3, hb::shared::sprite::DrawParams{0.7f, 0, 0, 0, false});
					break;

				case dynamic_object::Fire2:			// 13
					switch (rand() % 3) {
					case 0: m_effect_sprites[0]->draw(ix, iy, 1, hb::shared::sprite::DrawParams{0.25f, 0, 0, 0, false}); break;
					case 1: m_effect_sprites[0]->draw(ix, iy, 1, hb::shared::sprite::DrawParams{0.5f, 0, 0, 0, false}); break;
					case 2: m_effect_sprites[0]->draw(ix, iy, 1, hb::shared::sprite::DrawParams{0.7f, 0, 0, 0, false}); break;
					}
					break;

				case dynamic_object::Fish:			// 2
				{
					direction tmp_d_odir;
					char tmp_d_oframe;
					tmp_d_odir = CMisc::calc_direction(dynamic_object_data1, dynamic_object_data2, dynamic_object_data1 + dynamic_object_data3, dynamic_object_data2 + dynamic_object_data4);
					tmp_d_oframe = ((tmp_d_odir - 1) * 4) + (rand() % 4);
					m_sprite[ItemDynamicPivotPoint + 0]->draw(ix + dynamic_object_data1, iy + dynamic_object_data2, tmp_d_oframe, hb::shared::sprite::DrawParams::alpha_blend(0.25f));
				}
				break;

				case dynamic_object::Mineral1:		// 4
					if (config_manager::get().get_detail_level() != 0) m_sprite[ItemDynamicPivotPoint + 1]->draw(ix, iy, 0, hb::shared::sprite::DrawParams::shadow());
					m_sprite[ItemDynamicPivotPoint + 1]->draw(ix, iy, 0);
					CursorTarget::test_dynamic_object(m_sprite[ItemDynamicPivotPoint + 1]->GetBoundRect(), indexX, indexY, res_msy);
					break;

				case dynamic_object::Mineral2:		// 5
					if (config_manager::get().get_detail_level() != 0) m_sprite[ItemDynamicPivotPoint + 1]->draw(ix, iy, 1, hb::shared::sprite::DrawParams::shadow());
					m_sprite[ItemDynamicPivotPoint + 1]->draw(ix, iy, 1);
					CursorTarget::test_dynamic_object(m_sprite[ItemDynamicPivotPoint + 1]->GetBoundRect(), indexX, indexY, res_msy);
					break;

				case dynamic_object::Spike:			// 9
					m_effect_sprites[17]->draw(ix, iy, dynamic_object_frame, hb::shared::sprite::DrawParams{0.7f, 0, 0, 0, false});
					break;

				case dynamic_object::AresdenFlag1:  // 6
					m_sprite[ItemDynamicPivotPoint + 2]->draw(ix, iy, dynamic_object_frame);
					break;

				case dynamic_object::ElvineFlag1: // 7
					m_sprite[ItemDynamicPivotPoint + 2]->draw(ix, iy, dynamic_object_frame);
					break;
				}
			}
		}
	}

	if ((time - m_env_effect_time) > 400) m_env_effect_time = time;

	// Finalize Picking system - determines cursor type
	EntityRelationship focusRelationship = CursorTarget::has_focused_object() ? CursorTarget::GetFocusStatus().relationship : EntityRelationship::Neutral;
	CursorTarget::end_frame(focusRelationship, m_point_command_type, m_player->m_Controller.is_command_available(), m_is_get_pointing_mode);

	// update legacy compatibility variables from Picking system
	m_mcx = CursorTarget::get_focused_map_x();
	m_mcy = CursorTarget::get_focused_map_y();
	m_mc_name = CursorTarget::get_focused_name();

	// draw focused object with highlight (transparency)
	if (CursorTarget::has_focused_object())
	{
		short focusSX, focusSY;
		uint16_t focusObjID;
		short focusOwnerType, focusNpcConfigId;
		char focusAction, focusFrame;
		direction focusDir;
		hb::shared::entity::PlayerAppearance focusAppearance;
		hb::shared::entity::PlayerStatus focusStatus;
		short focusDataX, focusDataY;

		if (CursorTarget::get_focus_highlight_data(focusSX, focusSY, focusObjID, focusOwnerType,
			focusNpcConfigId, focusAction, focusDir, focusFrame, focusAppearance,
			focusStatus, focusDataX, focusDataY))
		{
			// Set up temporary vars for drawing
			m_entity_state.m_object_id = focusObjID;
			m_entity_state.m_owner_type = focusOwnerType;
			m_entity_state.m_npc_config_id = focusNpcConfigId;
			m_entity_state.m_action = focusAction;
			m_entity_state.m_frame = focusFrame;
			m_entity_state.m_dir = focusDir;
			m_entity_state.m_appearance = focusAppearance;
			m_entity_state.m_status = focusStatus;
			m_entity_state.m_data_x = focusDataX;
			m_entity_state.m_data_y = focusDataY;
			m_entity_state.m_name.fill('\0');
			std::snprintf(m_entity_state.m_name.data(), m_entity_state.m_name.size(), "%s", CursorTarget::get_focused_name());

			if ((focusAction != ObjectDead) && (focusFrame < 0)) {
				// Skip drawing invalid frame
			} else {
				switch (focusAction) {
				case Type::stop:
					draw_object_on_stop(m_mcx, m_mcy, focusSX, focusSY, true, time);
					break;
				case Type::Move:
					switch (focusOwnerType) {
					case 1:
					case 2:
					case 3: // Human M
					case 4:
					case 5:
					case 6: // Human F

					case hb::shared::owner::Troll: // Troll.
					case hb::shared::owner::Ogre: // Ogre
					case hb::shared::owner::Liche: // Liche
					case hb::shared::owner::Demon: // DD
					case hb::shared::owner::Unicorn: // Uni
					case hb::shared::owner::WereWolf: // WW
					case hb::shared::owner::LightWarBeetle: // LWB
					case hb::shared::owner::GodsHandKnight: // GHK
					case hb::shared::owner::GodsHandKnightCK: // GHKABS
					case hb::shared::owner::TempleKnight: // TK
					case hb::shared::owner::BattleGolem: // BG
					case hb::shared::owner::Stalker: // SK
					case hb::shared::owner::HellClaw: // HC
					case hb::shared::owner::TigerWorm: // TW
					case hb::shared::owner::Catapult: // CP
					case hb::shared::owner::Gargoyle: // GG
					case hb::shared::owner::Beholder: // BB
					case hb::shared::owner::DarkElf: // DE
					case hb::shared::owner::Bunny: // Rabbit
					case hb::shared::owner::Cat: // Cat
					case hb::shared::owner::GiantFrog: // Frog
					case hb::shared::owner::MountainGiant: // MG
					case hb::shared::owner::Ettin: // Ettin
					case hb::shared::owner::CannibalPlant: // Plant
					case hb::shared::owner::Rudolph: // Rudolph
					case hb::shared::owner::DireBoar: // DireBoar
					case hb::shared::owner::Frost: // Frost
					case hb::shared::owner::IceGolem: // Ice-Golem
					case hb::shared::owner::Wyvern: // Wyvern
					case hb::shared::owner::Dragon: // Dragon..........Ajouts par Snoopy
					case hb::shared::owner::Centaur: // Centaur
					case hb::shared::owner::ClawTurtle: // ClawTurtle
					case hb::shared::owner::FireWyvern: // FireWyvern
					case hb::shared::owner::GiantCrayfish: // GiantCrayfish
					case hb::shared::owner::GiLizard: // Gi Lizard
					case hb::shared::owner::GiTree: // Gi Tree
					case hb::shared::owner::MasterOrc: // Master Orc
					case hb::shared::owner::Minaus: // Minaus
					case hb::shared::owner::Nizie: // Nizie
					case hb::shared::owner::Tentocle: // Tentocle
					case hb::shared::owner::Abaddon: // Abaddon
					case hb::shared::owner::Sorceress: // Sorceress
					case hb::shared::owner::ATK: // ATK
					case hb::shared::owner::MasterElf: // MasterElf
					case hb::shared::owner::DSK: // DSK
					case hb::shared::owner::HBT: // HBT
					case hb::shared::owner::CT: // CT
					case hb::shared::owner::Barbarian: // Barbarian
					case hb::shared::owner::AGC: // AGC
					case hb::shared::owner::Gate: // Gate
						break;

					default: // 10..27
						m_entity_state.m_frame = m_entity_state.m_frame * 2;
						break;
					}

					draw_object_on_move(m_mcx, m_mcy, focusSX, focusSY, true, time);
					break;

				case Type::DamageMove:
					draw_object_on_damage_move(m_mcx, m_mcy, focusSX, focusSY, true, time);
					break;

				case Type::Run:
					draw_object_on_run(m_mcx, m_mcy, focusSX, focusSY, true, time);
					break;

				case Type::Attack:
					draw_object_on_attack(m_mcx, m_mcy, focusSX, focusSY, true, time);
					break;

				case Type::AttackMove:
					draw_object_on_attack_move(m_mcx, m_mcy, focusSX, focusSY, true, time);
					break;

				case Type::Magic:
					draw_object_on_magic(m_mcx, m_mcy, focusSX, focusSY, true, time);
					break;

				case Type::Damage:
					draw_object_on_damage(m_mcx, m_mcy, focusSX, focusSY, true, time);
					break;

				case Type::Dying:
					draw_object_on_dying(m_mcx, m_mcy, focusSX, focusSY, true, time);
					break;

				case ObjectDead:
					draw_object_on_dead(m_mcx, m_mcy, focusSX, focusSY, true, time);
					break;
				}
			}
		}
	}

	if (item_selected_id != -1) {
		auto itemInfo = item_name_formatter::get().format(static_cast<short>(item_selected_id), item_selected_attr);

		item_tooltip tooltip;
		auto name_color = itemInfo.is_special ? GameColors::UIItemName_Special : GameColors::UIWhite;
		tooltip.add_line(itemInfo.name, name_color);
		for (const auto& eff : itemInfo.effects)
		{
			if (eff.label.empty() && eff.value.empty()) continue;
			if (eff.value.empty())
				tooltip.add_line(eff.label, GameColors::InfoGrayLight);
			else
				tooltip.add_dual_line(eff.label, GameColors::InfoGrayLight, eff.value, GameColors::UIItemName_Special);
		}
		tooltip.draw(mouse_x, mouse_y + 25, m_Renderer);
	}
}
