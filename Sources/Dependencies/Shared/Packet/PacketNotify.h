#pragma once

#include "PacketHeaders.h"

#include <cstddef>
#include <cstdint>

namespace hb {
namespace net {
	HB_PACK_BEGIN
	constexpr std::size_t kMaxItems = 50;
	constexpr std::size_t kMaxMagicType = 100;
	constexpr std::size_t kMaxSkillType = 60;
	struct HB_PACKED PacketNotifyCraftingFail {
		PacketHeader header;
		int32_t reason;
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifyAngelicStats {
		PacketHeader header;
		int32_t str;
		int32_t intel;
		int32_t dex;
		int32_t mag;
	};

	struct HB_PACKED PacketNotifyAngelFailed {
		PacketHeader header;
		int32_t reason;
	};

	struct HB_PACKED PacketNotifyApocGateOpen {
		PacketHeader header;
		int32_t gate_x;
		int32_t gate_y;
		char map_name[10];
	};

	struct HB_PACKED PacketNotifyQuestCounter {
		PacketHeader header;
		int32_t current_count;
		uint8_t padding[16];
	};

	struct HB_PACKED PacketNotifyMonsterCount {
		PacketHeader header;
		int16_t count;
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifyAbaddonKilled {
		PacketHeader header;
		char killer_name[10];
		uint8_t padding[10];
	};

	struct HB_PACKED PacketNotifyHeldenianVictory {
		PacketHeader header;
		int16_t side;
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifyHeldenianCount {
		PacketHeader header;
		int16_t aresden_tower_left;
		int16_t elvine_tower_left;
		int16_t aresden_flags;
		int16_t elvine_flags;
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifySpawnEvent {
		PacketHeader header;
		uint8_t monster_id;
		int16_t x;
		int16_t y;
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifyChangePlayMode {
		PacketHeader header;
		char location[10];
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifyCurLifeSpan {
		PacketHeader header;
		int32_t item_index;
		int32_t cur_lifespan;
		uint8_t padding[2];
	};

	// Hot reload: Update max lifespan of an item
	struct HB_PACKED PacketNotifyMaxLifeSpan {
		PacketHeader header;
		int32_t item_index;
		int32_t max_lifespan;
	};

	struct HB_PACKED PacketNotifyNpcHp {
		PacketHeader header;
		int32_t hp;
		int32_t max_hp;
	};

	struct HB_PACKED PacketNotifyForceRecallTime {
		PacketHeader header;
		uint16_t seconds_left;
	};

	struct HB_PACKED PacketNotifyReqGuildNameAnswer {
		PacketHeader header;
		int16_t guild_rank;
		int16_t index;
		char guild_name[20];
	};

	struct HB_PACKED PacketNotifyItemUpgradeFail {
		PacketHeader header;
		int16_t reason;
	};

	struct HB_PACKED PacketNotifyGizonItemUpgradeLeft {
		PacketHeader header;
		int16_t left;
		int32_t reason;
	};

	struct HB_PACKED PacketNotifyItemAttributeChange {
		PacketHeader header;
		int16_t item_index;
		uint32_t attribute;
		uint32_t spec_value1;
		uint32_t spec_value2;
	};

	struct HB_PACKED PacketNotifyGizonItemChange {
		PacketHeader header;
		int16_t item_index;
		uint8_t item_type;
		int16_t cur_lifespan;
		int16_t sprite;
		int16_t sprite_frame;
		uint8_t item_color;
		uint8_t spec_value2;
		uint32_t attribute;
		char item_name[20];
	};

	struct HB_PACKED PacketNotifyTCLoc {
		PacketHeader header;
		int16_t dest_x;
		int16_t dest_y;
		char teleport_map[10];
		int16_t construct_x;
		int16_t construct_y;
		char construct_map[10];
	};

	struct HB_PACKED PacketNotifyConstructionPoint {
		PacketHeader header;
		int16_t construction_point;
		int16_t war_contribution;
		int16_t notify_type;
	};

	struct HB_PACKED PacketNotifyMeteorStrikeComing {
		PacketHeader header;
		int16_t phase;
	};

	struct HB_PACKED PacketNotifyLockedMap {
		PacketHeader header;
		int16_t seconds_left;
		char map_name[10];
	};

	struct HB_PACKED PacketNotifyCannotConstruct {
		PacketHeader header;
		int16_t reason;
	};

	struct HB_PACKED PacketNotifySpecialAbilityStatus {
		PacketHeader header;
		int16_t status_type;
		int16_t ability_type;
		int16_t seconds_left;
	};

	struct HB_PACKED PacketNotifyDamageMove {
		PacketHeader header;
		uint8_t dir;
		int16_t amount;
		uint8_t weapon;
	};

	struct HB_PACKED PacketNotifyResponseCreateNewParty {
		PacketHeader header;
		int16_t result;
	};

	struct HB_PACKED PacketNotifyQueryJoinParty {
		PacketHeader header;
		char name[1];
	};

	struct HB_PACKED PacketNotifyEnergySphereCreated {
		PacketHeader header;
		int16_t x;
		int16_t y;
	};

	struct HB_PACKED PacketNotifyEnergySphereGoalIn {
		PacketHeader header;
		int16_t result;
		int16_t side;
		int16_t goal;
		char name[20];
	};

	struct HB_PACKED PacketNotifySuperAttackLeft {
		PacketHeader header;
		int16_t left;
	};

	struct HB_PACKED PacketNotifySafeAttackMode {
		PacketHeader header;
		uint8_t enabled;
	};

	struct HB_PACKED PacketNotifyObserverMode {
		PacketHeader header;
		int16_t enabled;
	};

	struct HB_PACKED PacketNotifyCrusade {
		PacketHeader header;
		int32_t crusade_mode;
		int32_t crusade_duty;
		int32_t v3;
		int32_t v4;
	};

	struct HB_PACKED PacketNotifyBuildItemResult {
		PacketHeader header;
		int16_t item_id;
		int16_t item_count;
	};

	struct HB_PACKED PacketNotifyItemPosList {
		PacketHeader header;
		int16_t positions[kMaxItems * 2];
	};

	struct HB_PACKED PacketNotifyEnemyKills {
		PacketHeader header;
		int32_t count;
	};

	struct HB_PACKED PacketNotifyIpAccountInfo {
		PacketHeader header;
		char text[1];
	};

	struct HB_PACKED PacketNotifyRewardGold {
		PacketHeader header;
		uint32_t gold;
	};

	struct HB_PACKED PacketNotifyServerShutdown {
		PacketHeader header;
		uint8_t mode;
	};

	struct HB_PACKED PacketNotifyFishCanceled {
		PacketHeader header;
		uint16_t reason;
	};

	struct HB_PACKED PacketNotifyNotEnoughGold {
		PacketHeader header;
		int8_t item_index;
	};

	struct HB_PACKED PacketNotifyFightZoneReserve {
		PacketHeader header;
		int32_t result;
	};

	struct HB_PACKED PacketNotifySpellSkill {
		PacketHeader header;
		uint8_t magic_mastery[kMaxMagicType];
		uint8_t skill_mastery[kMaxSkillType];
	};

	struct HB_PACKED PacketNotifyStateChangeSuccess {
		PacketHeader header;
		uint8_t magic_mastery[kMaxMagicType];
		uint8_t skill_mastery[kMaxSkillType];
	};

	struct HB_PACKED PacketNotifyPartyBasic {
		PacketHeader header;
		int16_t type;
		int16_t v2;
		int16_t v3;
		int16_t v4;
	};

	struct HB_PACKED PacketNotifyPartyName {
		PacketHeader header;
		int16_t type;
		int16_t v2;
		int16_t v3;
		char name[10];
	};

	struct HB_PACKED PacketNotifyPartyList {
		PacketHeader header;
		int16_t type;
		int16_t v2;
		int16_t count;
		char names[1];
	};

	struct HB_PACKED PacketNotifyGrandMagicResult {
		PacketHeader header;
		uint16_t crashed_structures;
		uint16_t structure_damage;
		uint16_t casualities;
		char map_name[10];
		uint16_t active_structure;
		uint16_t value_count;
		uint16_t values[1];
	};

	struct HB_PACKED PacketNotifyGrandMagicResultHeader {
		PacketHeader header;
		uint16_t crashed_structures;
		uint16_t structure_damage;
		uint16_t casualities;
		char map_name[10];
		uint16_t active_structure;
		uint16_t value_count;
	};

	struct HB_PACKED PacketNotifyItemObtained {
		PacketHeader header;
		uint8_t is_new;
		char name[20];
		uint32_t count;
		uint8_t item_type;
		uint8_t equip_pos;
		uint8_t is_equipped;
		int16_t level_limit;
		uint8_t gender_limit;
		uint16_t cur_lifespan;
		uint16_t weight;
		int16_t sprite;
		int16_t sprite_frame;
		uint8_t item_color;
		uint8_t spec_value2;
		uint32_t attribute;
		int16_t item_id;           // Item ID for config lookup
		uint16_t max_lifespan;     // Maximum durability
	};

	struct HB_PACKED PacketNotifyItemPurchased {
		PacketHeader header;
		uint8_t is_new;
		char name[20];
		uint32_t count;
		uint8_t item_type;
		uint8_t equip_pos;
		uint8_t is_equipped;
		int16_t level_limit;
		uint8_t gender_limit;
		uint16_t cur_lifespan;
		uint16_t weight;
		int16_t sprite;
		int16_t sprite_frame;
		uint8_t item_color;
		uint16_t cost;
		int16_t item_id;           // Item ID for config lookup
		uint16_t max_lifespan;     // Maximum durability
	};

	struct HB_PACKED PacketNotifyItemToBank {
		PacketHeader header;
		uint8_t bank_index;
		uint8_t is_new;
		char name[20];
		uint32_t count;
		uint8_t item_type;
		uint8_t equip_pos;
		uint8_t is_equipped;
		int16_t level_limit;
		uint8_t gender_limit;
		uint16_t cur_lifespan;
		uint16_t weight;
		int16_t sprite;
		int16_t sprite_frame;
		uint8_t item_color;
		int16_t item_effect_value2;
		uint32_t attribute;
		uint8_t spec_effect_value2;
		uint8_t padding;
		int16_t item_id;           // Item ID for config lookup
		uint16_t max_lifespan;     // Maximum durability
	};

	struct HB_PACKED PacketNotifyRatingPlayer {
		PacketHeader header;
		uint8_t result;
		char name[10];
		int32_t rating;
		uint8_t padding;
	};

	struct HB_PACKED PacketNotifyPlayerShutUp {
		PacketHeader header;
		uint16_t time;
		char name[10];
		uint8_t padding;
	};

	struct HB_PACKED PacketNotifyPlayerNotOnGame {
		PacketHeader header;
		char name[10];
		char filler[10];
		uint8_t padding;
	};

	struct HB_PACKED PacketNotifyPlayerOnGame {
		PacketHeader header;
		char name[10];
		char map_name[14];
		uint8_t padding;
	};

	struct HB_PACKED PacketNotifyPlayerStatus {
		PacketHeader header;
		char name[10];
		char map_name[10];
		uint16_t x;
		uint16_t y;
	};

	struct HB_PACKED PacketNotifyWhisperMode {
		PacketHeader header;
		char name[10];
	};

	struct HB_PACKED PacketNotifyServerChange {
		PacketHeader header;
		char map_name[10];
		char log_server_addr[15];
		int32_t log_server_port;
	};

	struct HB_PACKED PacketNotifySetItemCount {
		PacketHeader header;
		uint16_t item_index;
		uint32_t count;
		uint8_t notify;
	};

	struct HB_PACKED PacketNotifyShowMap {
		PacketHeader header;
		uint16_t map_id;
		uint16_t map_type;
	};

	struct HB_PACKED PacketNotifyMagicEffect {
		PacketHeader header;
		uint16_t magic_type;
		uint32_t effect;
		uint32_t owner;
	};

	struct HB_PACKED PacketNotifyMagicStudySuccess {
		PacketHeader header;
		uint8_t magic_id;
		char magic_name[30];
	};

	struct HB_PACKED PacketNotifyMagicStudyFail {
		PacketHeader header;
		uint8_t result;
		uint8_t magic_id;
		char magic_name[30];
		int32_t cost;
		int32_t req_int;
	};

	struct HB_PACKED PacketNotifyRepairItemPrice {
		PacketHeader header;
		uint32_t v1;
		uint32_t v2;
		uint32_t v3;
		uint32_t v4;
		char item_name[20];
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifySellItemPrice {
		PacketHeader header;
		uint32_t v1;
		uint32_t v2;
		uint32_t v3;
		uint32_t v4;
		char item_name[20];
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifyQuestContents {
		PacketHeader header;
		int16_t who;
		int16_t quest_type;
		int16_t contribution;
		int16_t target_type;
		int16_t target_count;
		int16_t x;
		int16_t y;
		int16_t range;
		int16_t is_completed;
		char target_name[20];
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifyPlayerProfile {
		PacketHeader header;
		char text[1];
	};

	struct HB_PACKED PacketNotifyNoticeMsg {
		PacketHeader header;
		char text[1];
	};

	struct HB_PACKED PacketNotifyQueryDismissGuildPermission {
		PacketHeader header;
		char name[10];
	};

	struct HB_PACKED PacketNotifyQueryJoinGuildPermission {
		PacketHeader header;
		char name[10];
	};

	struct HB_PACKED PacketNotifyTimeChange {
		PacketHeader header;
		uint8_t sprite_alpha;
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifyHP {
		PacketHeader header;
		uint32_t hp;
		uint32_t hunger;
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifyMP {
		PacketHeader header;
		uint32_t mp;
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifySP {
		PacketHeader header;
		uint32_t sp;
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifyExp {
		PacketHeader header;
		uint32_t exp;
		int32_t rating;
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifyCharisma {
		PacketHeader header;
		uint32_t charisma;
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifyHunger {
		PacketHeader header;
		uint8_t hunger;
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifyItemColorChange {
		PacketHeader header;
		int16_t item_index;
		int16_t item_color;
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifyItemDepletedEraseItem {
		PacketHeader header;
		uint16_t item_index;
		uint16_t use_result;
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifyItemLifeSpanEnd {
		PacketHeader header;
		int16_t equip_pos;
		int16_t item_index;
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifyItemReleased {
		PacketHeader header;
		int16_t equip_pos;
		int16_t item_index;
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifyItemRepaired {
		PacketHeader header;
		uint32_t item_id;
		uint32_t life;
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifySkill {
		PacketHeader header;
		uint16_t skill_index;
		uint16_t skill_value;
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifySkillTrainSuccess {
		PacketHeader header;
		uint8_t skill_num;
		uint8_t skill_level;
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifySkillUsingEnd {
		PacketHeader header;
		uint16_t result;
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifyLevelUp {
		PacketHeader header;
		int32_t level;
		int32_t str;
		int32_t vit;
		int32_t dex;
		int32_t intel;
		int32_t mag;
		int32_t chr;
	};

	struct HB_PACKED PacketNotifyQuestReward {
		PacketHeader header;
		int16_t who;
		int16_t flag;
		int32_t amount;
		char reward_name[20];
		int32_t contribution;
	};

	struct HB_PACKED PacketNotifyExchangeItem {
		PacketHeader header;
		int16_t dir;
		int16_t sprite;
		int16_t sprite_frame;
		int32_t amount;
		uint8_t color;
		int16_t cur_life;
		int16_t max_life;
		int16_t performance;
		char item_name[20];
		char char_name[10];
		uint32_t attribute;
	};

	struct HB_PACKED PacketNotifyRepairAllPricesHeader {
		PacketHeader header;
		int16_t total;
	};

	struct HB_PACKED PacketNotifyRepairAllPricesEntry {
		uint8_t index;
		int16_t price;
	};

	struct HB_PACKED PacketNotifyEnemyKillReward {
		PacketHeader header;
		uint32_t exp;
		uint32_t kill_count;
		char killer_name[10];
		char killer_guild[20];
		int16_t killer_rank;
		int16_t war_contribution;
	};

	struct HB_PACKED PacketNotifyPKcaptured {
		PacketHeader header;
		uint16_t pk_count;
		uint16_t victim_pk_count;
		char victim_name[10];
		uint32_t reward_gold;
		uint32_t exp;
	};

	struct HB_PACKED PacketNotifyPKpenalty {
		PacketHeader header;
		uint32_t exp;
		uint32_t str;
		uint32_t vit;
		uint32_t dex;
		uint32_t intel;
		uint32_t mag;
		uint32_t chr;
		uint32_t pk_count;
	};

	struct HB_PACKED PacketNotifyKilled {
		PacketHeader header;
		char attacker_name[20];
	};

	struct HB_PACKED PacketNotifyForceDisconn {
		PacketHeader header;
		uint16_t seconds;
	};

	struct HB_PACKED PacketNotifySimpleShort {
		PacketHeader header;
		int16_t value;
	};

	struct HB_PACKED PacketNotifySimpleInt {
		PacketHeader header;
		int32_t value;
	};

	struct HB_PACKED PacketNotifyEmpty {
		PacketHeader header;
	};

	struct HB_PACKED PacketNotifyNpcTalk {
		PacketHeader header;
		int16_t type;
		int16_t response;
		int16_t amount;
		int16_t contribution;
		int16_t target_type;
		int16_t target_count;
		int16_t x;
		int16_t y;
		int16_t range;
		char reward_name[20];
		char target_name[20];
	};

	struct HB_PACKED PacketNotifyMapStatusHeader {
		PacketHeader header;
		char map_name[10];
		int16_t index;
		uint8_t total;
	};

	struct HB_PACKED PacketNotifyMapStatusEntry {
		uint8_t type;
		int16_t x;
		int16_t y;
		uint8_t side;
	};

	struct HB_PACKED PacketNotifyMobKillCountHeader {
		PacketHeader header;
		int16_t total;
	};

	struct HB_PACKED PacketNotifyMobKillCountEntry {
		int16_t kill_count;
		int16_t next_count;
		int16_t level;
		char npc_name[20];
	};

	struct HB_PACKED PacketNotifyCannotGiveItem {
		PacketHeader header;
		uint16_t item_index;
		int32_t amount;
		char name[20];
	};

	struct HB_PACKED PacketNotifyGlobalAttackMode {
		PacketHeader header;
		uint8_t mode;
	};

	struct HB_PACKED PacketNotifyWhetherChange {
		PacketHeader header;
		uint8_t status;
	};

	struct HB_PACKED PacketNotifyFishChance {
		PacketHeader header;
		uint16_t chance;
	};

	struct HB_PACKED PacketNotifyEventFishMode {
		PacketHeader header;
		uint16_t price;
		uint16_t sprite;
		uint16_t sprite_frame;
		char name[20];
	};

	struct HB_PACKED PacketNotifyCannotRating {
		PacketHeader header;
		uint16_t time_left;
	};

	struct HB_PACKED PacketNotifyCannotRepairItem {
		PacketHeader header;
		uint16_t item_index;
		uint16_t reason;
		char name[20];
	};

	struct HB_PACKED PacketNotifyCannotSellItem {
		PacketHeader header;
		uint16_t item_index;
		uint16_t reason;
		char name[20];
	};

	struct HB_PACKED PacketNotifyDownSkillIndexSet {
		PacketHeader header;
		uint16_t skill_index;
	};

	struct HB_PACKED PacketNotifyAdminInfo {
		PacketHeader header;
		int32_t v1;
		int32_t v2;
		int32_t v3;
		int32_t v4;
		int32_t v5;
		int32_t v6;
		int32_t v7;
		int32_t v8;
	};

	struct HB_PACKED PacketNotifyCannotJoinMoreGuildsMan {
		PacketHeader header;
		char name[10];
	};

	struct HB_PACKED PacketNotifyNewGuildsMan {
		PacketHeader header;
		char name[11];
	};

	struct HB_PACKED PacketNotifyDismissGuildsMan {
		PacketHeader header;
		char name[11];
	};

	struct HB_PACKED PacketNotifyJoinGuildApprove {
		PacketHeader header;
		char guild_name[20];
		int16_t rank;
	};

	struct HB_PACKED PacketNotifyJoinGuildReject {
		PacketHeader header;
		char guild_name[20];
		int16_t rank;
		char location[10];
	};

	struct HB_PACKED PacketNotifyDismissGuildApprove {
		PacketHeader header;
		char guild_name[20];
		int16_t rank;
		char location[10];
	};

	struct HB_PACKED PacketNotifyDismissGuildReject {
		PacketHeader header;
		char guild_name[20];
		int16_t rank;
		char location[10];
	};

	struct HB_PACKED PacketNotifyGuildDisbanded {
		PacketHeader header;
		char guild_name[20];
		char location[10];
	};

	struct HB_PACKED PacketNotifyBanGuildMan {
		PacketHeader header;
		char guild_name[20];
		int16_t rank;
		char location[10];
	};

	struct HB_PACKED PacketNotifyTotalUsers {
		PacketHeader header;
		uint16_t total;
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifyDropItemFinCountChanged {
		PacketHeader header;
		uint16_t item_index;
		int32_t amount;
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifyDropItemFinEraseItem {
		PacketHeader header;
		uint16_t item_index;
		int32_t amount;
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifyGiveItemFinCountChanged {
		PacketHeader header;
		uint16_t item_index;
		int32_t amount;
		char name[20];
		uint8_t padding[2];
	};

	struct HB_PACKED PacketNotifyGiveItemFinEraseItem {
		PacketHeader header;
		uint16_t item_index;
		int32_t amount;
		char name[20];
		uint8_t padding[2];
	};
	HB_PACK_END
}
}
