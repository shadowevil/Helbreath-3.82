#include "GameConfigSqliteStore.h"

#include "Platform.h"
#include <cstdio>
#include <cstring>

#include "Item.h"
#include "BuildItem.h"
#include "Game.h"
#include "ItemManager.h"
#include "CraftingManager.h"
#include "QuestManager.h"
#include "Magic.h"
#include "Npc.h"
#include "Portion.h"
#include "Quest.h"
#include "Skill.h"
#include "sqlite3.h"
using namespace hb::server::config;

extern void PutLogList(char* cMsg);
extern void PutLogListLevel(int level, const char* cMsg);

namespace
{
    bool ExecSql(sqlite3* db, const char* sql)
    {
        char* err = nullptr;
        int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err);
        if (rc != SQLITE_OK) {
            char logMsg[512] = {};
            std::snprintf(logMsg, sizeof(logMsg), "(SQLITE) Exec failed: %s", err ? err : "unknown");
            PutLogList(logMsg);
            sqlite3_free(err);
            return false;
        }
        return true;
    }

    bool BeginTransaction(sqlite3* db)
    {
        return ExecSql(db, "BEGIN;");
    }

    bool CommitTransaction(sqlite3* db)
    {
        return ExecSql(db, "COMMIT;");
    }

    bool RollbackTransaction(sqlite3* db)
    {
        return ExecSql(db, "ROLLBACK;");
    }

    bool ClearTable(sqlite3* db, const char* tableName)
    {
        char sql[256] = {};
        std::snprintf(sql, sizeof(sql), "DELETE FROM %s;", tableName);
        return ExecSql(db, sql);
    }

    bool PrepareAndBindText(sqlite3_stmt* stmt, int idx, const char* value)
    {
        return sqlite3_bind_text(stmt, idx, value, -1, SQLITE_TRANSIENT) == SQLITE_OK;
    }

    bool HasColumn(sqlite3* db, const char* tableName, const char* columnName)
    {
        if (db == nullptr || tableName == nullptr || columnName == nullptr) {
            return false;
        }
        char sql[256] = {};
        std::snprintf(sql, sizeof(sql), "PRAGMA table_info(%s);", tableName);
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }
        bool found = false;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* name = sqlite3_column_text(stmt, 1);
            if (name != nullptr && std::strcmp(reinterpret_cast<const char*>(name), columnName) == 0) {
                found = true;
                break;
            }
        }
        sqlite3_finalize(stmt);
        return found;
    }

    void CopyColumnText(sqlite3_stmt* stmt, int col, char* dest, size_t destSize)
    {
        const unsigned char* text = sqlite3_column_text(stmt, col);
        if (text == nullptr) {
            if (destSize > 0) {
                dest[0] = 0;
            }
            return;
        }
        std::snprintf(dest, destSize, "%s", reinterpret_cast<const char*>(text));
    }

    bool InsertKeyValue(sqlite3_stmt* stmt, const char* key, const char* value)
    {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        bool ok = true;
        ok &= PrepareAndBindText(stmt, 1, key);
        ok &= PrepareAndBindText(stmt, 2, value);
        if (!ok) {
            return false;
        }
        return sqlite3_step(stmt) == SQLITE_DONE;
    }

    bool InsertKeyValueInt(sqlite3_stmt* stmt, const char* key, int value)
    {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        bool ok = true;
        ok &= PrepareAndBindText(stmt, 1, key);
        ok &= (sqlite3_bind_int(stmt, 2, value) == SQLITE_OK);
        if (!ok) {
            return false;
        }
        return sqlite3_step(stmt) == SQLITE_DONE;
    }

    bool InsertKeyValueFloat(sqlite3_stmt* stmt, const char* key, float value)
    {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        bool ok = true;
        ok &= PrepareAndBindText(stmt, 1, key);
        ok &= (sqlite3_bind_double(stmt, 2, static_cast<double>(value)) == SQLITE_OK);
        if (!ok) {
            return false;
        }
        return sqlite3_step(stmt) == SQLITE_DONE;
    }
}

bool EnsureGameConfigDatabase(sqlite3** outDb, std::string& outPath, bool* outCreated)
{
    if (outDb == nullptr) {
        return false;
    }

    std::string dbPath = "GameConfigs.db";
    DWORD attrs = GetFileAttributes(dbPath.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        char modulePath[MAX_PATH] = {};
        DWORD len = GetModuleFileNameA(nullptr, modulePath, MAX_PATH);
        if (len > 0 && len < MAX_PATH) {
            char* lastSlash = strrchr(modulePath, '/');
            if (!lastSlash) lastSlash = strrchr(modulePath, '\\');
            if (lastSlash != nullptr) {
                *(lastSlash + 1) = '\0';
                dbPath.assign(modulePath);
                dbPath.append("GameConfigs.db");
            }
        }
    }
    outPath = dbPath;

    bool created = false;
    attrs = GetFileAttributes(dbPath.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        created = true;
    }

    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        char logMsg[512] = {};
        std::snprintf(logMsg, sizeof(logMsg), "(SQLITE) Open failed: %s", sqlite3_errmsg(db));
        PutLogList(logMsg);
        sqlite3_close(db);
        return false;
    }

    sqlite3_busy_timeout(db, 1000);
    if (!ExecSql(db, "PRAGMA foreign_keys = ON;")) {
        sqlite3_close(db);
        return false;
    }

    const char* schemaSql =
        "BEGIN;"
        "CREATE TABLE IF NOT EXISTS meta ("
        " key TEXT PRIMARY KEY,"
        " value TEXT NOT NULL"
        ");"
        "INSERT OR REPLACE INTO meta(key, value) VALUES('schema_version','3');"
        "CREATE TABLE IF NOT EXISTS items ("
        " item_id INTEGER PRIMARY KEY,"
        " name TEXT NOT NULL,"
        " item_type INTEGER NOT NULL,"
        " equip_pos INTEGER NOT NULL,"
        " item_effect_type INTEGER NOT NULL,"
        " item_effect_value1 INTEGER NOT NULL,"
        " item_effect_value2 INTEGER NOT NULL,"
        " item_effect_value3 INTEGER NOT NULL,"
        " item_effect_value4 INTEGER NOT NULL,"
        " item_effect_value5 INTEGER NOT NULL,"
        " item_effect_value6 INTEGER NOT NULL,"
        " max_lifespan INTEGER NOT NULL,"
        " special_effect INTEGER NOT NULL,"
        " sprite INTEGER NOT NULL,"
        " sprite_frame INTEGER NOT NULL,"
        " price INTEGER NOT NULL,"
        " is_for_sale INTEGER NOT NULL,"
        " weight INTEGER NOT NULL,"
        " appr_value INTEGER NOT NULL,"
        " speed INTEGER NOT NULL,"
        " level_limit INTEGER NOT NULL,"
        " gender_limit INTEGER NOT NULL,"
        " special_effect_value1 INTEGER NOT NULL,"
        " special_effect_value2 INTEGER NOT NULL,"
        " related_skill INTEGER NOT NULL,"
        " category INTEGER NOT NULL,"
        " item_color INTEGER NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS realmlist ("
        " id INTEGER PRIMARY KEY,"
        " realm_name TEXT UNIQUE NOT NULL,"
        " login_listen_ip TEXT NOT NULL DEFAULT '0.0.0.0',"
        " login_listen_port INTEGER NOT NULL DEFAULT 2848,"
        " game_server_listen_ip TEXT NOT NULL DEFAULT '0.0.0.0',"
        " game_server_listen_port INTEGER NOT NULL DEFAULT 2858,"
        " game_server_connection_ip TEXT DEFAULT NULL,"
        " game_server_connection_port INTEGER DEFAULT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS active_maps ("
        " map_index INTEGER PRIMARY KEY,"
        " map_name TEXT NOT NULL,"
        " active INTEGER NOT NULL DEFAULT 0"
        ");"
        "CREATE TABLE IF NOT EXISTS settings ("
        " key TEXT PRIMARY KEY,"
        " value TEXT NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS banned_list ("
        " ip_address TEXT PRIMARY KEY"
        ");"
        "CREATE TABLE IF NOT EXISTS admins ("
        " account_name TEXT PRIMARY KEY,"
        " character_name TEXT NOT NULL,"
        " approved_ip TEXT NOT NULL DEFAULT '0.0.0.0',"
        " admin_level INTEGER NOT NULL DEFAULT 1"
        ");"
        "CREATE TABLE IF NOT EXISTS admin_command_permissions ("
        " command TEXT PRIMARY KEY COLLATE NOCASE,"
        " admin_level INTEGER NOT NULL DEFAULT 1000,"
        " description TEXT NOT NULL DEFAULT ''"
        ");"
        "CREATE TABLE IF NOT EXISTS npc_configs ("
        " npc_id INTEGER PRIMARY KEY,"
        " name TEXT NOT NULL,"
        " npc_type INTEGER NOT NULL,"
        " hit_dice INTEGER NOT NULL,"
        " defense_ratio INTEGER NOT NULL,"
        " hit_ratio INTEGER NOT NULL,"
        " min_bravery INTEGER NOT NULL,"
        " exp_min INTEGER NOT NULL,"
        " exp_max INTEGER NOT NULL,"
        " gold_min INTEGER NOT NULL,"
        " gold_max INTEGER NOT NULL,"
        " attack_dice_throw INTEGER NOT NULL,"
        " attack_dice_range INTEGER NOT NULL,"
        " npc_size INTEGER NOT NULL,"
        " side INTEGER NOT NULL,"
        " action_limit INTEGER NOT NULL,"
        " action_time INTEGER NOT NULL,"
        " resist_magic INTEGER NOT NULL,"
        " magic_level INTEGER NOT NULL,"
        " day_of_week_limit INTEGER NOT NULL,"
        " chat_msg_presence INTEGER NOT NULL,"
        " target_search_range INTEGER NOT NULL,"
        " regen_time INTEGER NOT NULL,"
        " attribute INTEGER NOT NULL,"
        " abs_damage INTEGER NOT NULL,"
        " max_mana INTEGER NOT NULL,"
        " magic_hit_ratio INTEGER NOT NULL,"
        " attack_range INTEGER NOT NULL,"
        " drop_table_id INTEGER NOT NULL DEFAULT 0"
        ");"
        "CREATE TABLE IF NOT EXISTS drop_tables ("
        " drop_table_id INTEGER PRIMARY KEY,"
        " name TEXT NOT NULL,"
        " description TEXT NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS drop_entries ("
        " drop_table_id INTEGER NOT NULL,"
        " tier INTEGER NOT NULL,"
        " item_id INTEGER NOT NULL,"
        " weight INTEGER NOT NULL,"
        " min_count INTEGER NOT NULL,"
        " max_count INTEGER NOT NULL,"
        " PRIMARY KEY (drop_table_id, tier, item_id)"
        ");"
        "CREATE TABLE IF NOT EXISTS magic_configs ("
        " magic_id INTEGER PRIMARY KEY,"
        " name TEXT NOT NULL,"
        " magic_type INTEGER NOT NULL,"
        " delay_time INTEGER NOT NULL,"
        " last_time INTEGER NOT NULL,"
        " value1 INTEGER NOT NULL,"
        " value2 INTEGER NOT NULL,"
        " value3 INTEGER NOT NULL,"
        " value4 INTEGER NOT NULL,"
        " value5 INTEGER NOT NULL,"
        " value6 INTEGER NOT NULL,"
        " value7 INTEGER NOT NULL,"
        " value8 INTEGER NOT NULL,"
        " value9 INTEGER NOT NULL,"
        " value10 INTEGER NOT NULL,"
        " value11 INTEGER NOT NULL,"
        " value12 INTEGER NOT NULL,"
        " int_limit INTEGER NOT NULL,"
        " gold_cost INTEGER NOT NULL,"
        " category INTEGER NOT NULL,"
        " attribute INTEGER NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS skill_configs ("
        " skill_id INTEGER PRIMARY KEY,"
        " name TEXT NOT NULL,"
        " skill_type INTEGER NOT NULL,"
        " value1 INTEGER NOT NULL,"
        " value2 INTEGER NOT NULL,"
        " value3 INTEGER NOT NULL,"
        " value4 INTEGER NOT NULL,"
        " value5 INTEGER NOT NULL,"
        " value6 INTEGER NOT NULL,"
        " is_useable INTEGER NOT NULL DEFAULT 0,"
        " use_method INTEGER NOT NULL DEFAULT 0"
        ");"
        "CREATE TABLE IF NOT EXISTS quest_configs ("
        " quest_index INTEGER PRIMARY KEY,"
        " side INTEGER NOT NULL,"
        " quest_type INTEGER NOT NULL,"
        " target_type INTEGER NOT NULL,"
        " max_count INTEGER NOT NULL,"
        " quest_from INTEGER NOT NULL,"
        " min_level INTEGER NOT NULL,"
        " max_level INTEGER NOT NULL,"
        " required_skill_num INTEGER NOT NULL,"
        " required_skill_level INTEGER NOT NULL,"
        " time_limit INTEGER NOT NULL,"
        " assign_type INTEGER NOT NULL,"
        " reward_type1 INTEGER NOT NULL,"
        " reward_amount1 INTEGER NOT NULL,"
        " reward_type2 INTEGER NOT NULL,"
        " reward_amount2 INTEGER NOT NULL,"
        " reward_type3 INTEGER NOT NULL,"
        " reward_amount3 INTEGER NOT NULL,"
        " contribution INTEGER NOT NULL,"
        " contribution_limit INTEGER NOT NULL,"
        " response_mode INTEGER NOT NULL,"
        " target_name TEXT NOT NULL,"
        " target_x INTEGER NOT NULL,"
        " target_y INTEGER NOT NULL,"
        " target_range INTEGER NOT NULL,"
        " quest_id INTEGER NOT NULL,"
        " req_contribution INTEGER NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS potion_configs ("
        " potion_id INTEGER PRIMARY KEY,"
        " name TEXT NOT NULL,"
        " array0 INTEGER NOT NULL,"
        " array1 INTEGER NOT NULL,"
        " array2 INTEGER NOT NULL,"
        " array3 INTEGER NOT NULL,"
        " array4 INTEGER NOT NULL,"
        " array5 INTEGER NOT NULL,"
        " array6 INTEGER NOT NULL,"
        " array7 INTEGER NOT NULL,"
        " array8 INTEGER NOT NULL,"
        " array9 INTEGER NOT NULL,"
        " array10 INTEGER NOT NULL,"
        " array11 INTEGER NOT NULL,"
        " skill_limit INTEGER NOT NULL,"
        " difficulty INTEGER NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS crafting_configs ("
        " crafting_id INTEGER PRIMARY KEY,"
        " name TEXT NOT NULL,"
        " array0 INTEGER NOT NULL,"
        " array1 INTEGER NOT NULL,"
        " array2 INTEGER NOT NULL,"
        " array3 INTEGER NOT NULL,"
        " array4 INTEGER NOT NULL,"
        " array5 INTEGER NOT NULL,"
        " array6 INTEGER NOT NULL,"
        " array7 INTEGER NOT NULL,"
        " array8 INTEGER NOT NULL,"
        " array9 INTEGER NOT NULL,"
        " array10 INTEGER NOT NULL,"
        " array11 INTEGER NOT NULL,"
        " skill_limit INTEGER NOT NULL,"
        " difficulty INTEGER NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS builditem_configs ("
        " build_id INTEGER PRIMARY KEY,"
        " name TEXT NOT NULL,"
        " skill_limit INTEGER NOT NULL,"
        " material_id1 INTEGER NOT NULL,"
        " material_count1 INTEGER NOT NULL,"
        " material_value1 INTEGER NOT NULL,"
        " material_id2 INTEGER NOT NULL,"
        " material_count2 INTEGER NOT NULL,"
        " material_value2 INTEGER NOT NULL,"
        " material_id3 INTEGER NOT NULL,"
        " material_count3 INTEGER NOT NULL,"
        " material_value3 INTEGER NOT NULL,"
        " material_id4 INTEGER NOT NULL,"
        " material_count4 INTEGER NOT NULL,"
        " material_value4 INTEGER NOT NULL,"
        " material_id5 INTEGER NOT NULL,"
        " material_count5 INTEGER NOT NULL,"
        " material_value5 INTEGER NOT NULL,"
        " material_id6 INTEGER NOT NULL,"
        " material_count6 INTEGER NOT NULL,"
        " material_value6 INTEGER NOT NULL,"
        " average_value INTEGER NOT NULL,"
        " max_skill INTEGER NOT NULL,"
        " attribute INTEGER NOT NULL,"
        " item_id INTEGER NOT NULL"
        ");"
                "CREATE TABLE IF NOT EXISTS crusade_structures ("
        " structure_id INTEGER PRIMARY KEY,"
        " map_name TEXT NOT NULL,"
        " structure_type INTEGER NOT NULL,"
        " pos_x INTEGER NOT NULL,"
        " pos_y INTEGER NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS event_schedule ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " event_type TEXT NOT NULL,"
        " schedule_index INTEGER NOT NULL,"
        " day INTEGER NOT NULL,"
        " start_hour INTEGER NOT NULL,"
        " start_minute INTEGER NOT NULL,"
        " end_hour INTEGER,"
        " end_minute INTEGER,"
        " is_active INTEGER NOT NULL DEFAULT 0,"
        " UNIQUE(event_type, schedule_index)"
        ");"
        "CREATE TABLE IF NOT EXISTS npc_shop_mapping ("
        " npc_type INTEGER PRIMARY KEY,"
        " shop_id INTEGER NOT NULL,"
        " description TEXT"
        ");"
        "CREATE TABLE IF NOT EXISTS shop_items ("
        " shop_id INTEGER NOT NULL,"
        " item_id INTEGER NOT NULL,"
        " PRIMARY KEY (shop_id, item_id)"
        ");"
        "COMMIT;";

    if (!ExecSql(db, schemaSql)) {
        sqlite3_close(db);
        return false;
    }

    if (!HasColumn(db, "npc_configs", "drop_table_id")) {
        ExecSql(db, "ALTER TABLE npc_configs ADD COLUMN drop_table_id INTEGER NOT NULL DEFAULT 0;");
    }

    if (!HasColumn(db, "admins", "admin_level")) {
        ExecSql(db, "ALTER TABLE admins ADD COLUMN admin_level INTEGER NOT NULL DEFAULT 1;");
    }

    *outDb = db;
    if (outCreated != nullptr) {
        *outCreated = created;
    }
    return true;
}

bool SaveItemConfigs(sqlite3* db, CItem* const* itemList, int maxItems)
{
    if (db == nullptr || itemList == nullptr || maxItems <= 0) {
        return false;
    }

    if (!ExecSql(db, "BEGIN;")) {
        return false;
    }
    if (!ExecSql(db, "DELETE FROM items;")) {
        ExecSql(db, "ROLLBACK;");
        return false;
    }

    const char* sql =
        "INSERT INTO items("
        " item_id, name, item_type, equip_pos, item_effect_type, item_effect_value1,"
        " item_effect_value2, item_effect_value3, item_effect_value4, item_effect_value5,"
        " item_effect_value6, max_lifespan, special_effect, sprite, sprite_frame, price,"
        " is_for_sale, weight, appr_value, speed, level_limit, gender_limit,"
        " special_effect_value1, special_effect_value2, related_skill, category, item_color"
        ") VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        ExecSql(db, "ROLLBACK;");
        return false;
    }

    for(int i = 0; i < maxItems; i++) {
        if (itemList[i] == nullptr) {
            continue;
        }

        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        int col = 1;
        bool ok = true;
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_sIDnum) == SQLITE_OK);
        ok &= PrepareAndBindText(stmt, col++, itemList[i]->m_cName);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_cItemType) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_cEquipPos) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_sItemEffectType) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_sItemEffectValue1) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_sItemEffectValue2) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_sItemEffectValue3) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_sItemEffectValue4) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_sItemEffectValue5) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_sItemEffectValue6) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_wMaxLifeSpan) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_sSpecialEffect) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_sSprite) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_sSpriteFrame) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, static_cast<int>(itemList[i]->m_wPrice)) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_bIsForSale ? 1 : 0) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_wWeight) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_cApprValue) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_cSpeed) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_sLevelLimit) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_cGenderLimit) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_sSpecialEffectValue1) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_sSpecialEffectValue2) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_sRelatedSkill) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_cCategory) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, itemList[i]->m_cItemColor) == SQLITE_OK);

        if (!ok || sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            ExecSql(db, "ROLLBACK;");
            return false;
        }
    }

    sqlite3_finalize(stmt);
    if (!ExecSql(db, "COMMIT;")) {
        ExecSql(db, "ROLLBACK;");
        return false;
    }
    return true;
}

bool LoadItemConfigs(sqlite3* db, CItem** itemList, int maxItems)
{
    if (db == nullptr || itemList == nullptr || maxItems <= 0) {
        return false;
    }

    const char* sql =
        "SELECT item_id, name, item_type, equip_pos, item_effect_type, item_effect_value1,"
        " item_effect_value2, item_effect_value3, item_effect_value4, item_effect_value5,"
        " item_effect_value6, max_lifespan, special_effect, sprite, sprite_frame, price,"
        " is_for_sale, weight, appr_value, speed, level_limit, gender_limit,"
        " special_effect_value1, special_effect_value2, related_skill, category, item_color"
        " FROM items ORDER BY item_id;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = 0;
        int itemId = sqlite3_column_int(stmt, col++);
        if (itemId < 0 || itemId >= maxItems) {
            continue;
        }

        if (itemList[itemId] != nullptr) {
            delete itemList[itemId];
            itemList[itemId] = nullptr;
        }

        CItem* item = new CItem();
        item->m_sIDnum = (short)itemId;
        CopyColumnText(stmt, col++, item->m_cName, sizeof(item->m_cName));
        item->m_cItemType = (char)sqlite3_column_int(stmt, col++);
        item->m_cEquipPos = (char)sqlite3_column_int(stmt, col++);
        item->m_sItemEffectType = (short)sqlite3_column_int(stmt, col++);
        item->m_sItemEffectValue1 = (short)sqlite3_column_int(stmt, col++);
        item->m_sItemEffectValue2 = (short)sqlite3_column_int(stmt, col++);
        item->m_sItemEffectValue3 = (short)sqlite3_column_int(stmt, col++);
        item->m_sItemEffectValue4 = (short)sqlite3_column_int(stmt, col++);
        item->m_sItemEffectValue5 = (short)sqlite3_column_int(stmt, col++);
        item->m_sItemEffectValue6 = (short)sqlite3_column_int(stmt, col++);
        item->m_wMaxLifeSpan = (uint16_t)sqlite3_column_int(stmt, col++);
        item->m_sSpecialEffect = (short)sqlite3_column_int(stmt, col++);
        item->m_sSprite = (short)sqlite3_column_int(stmt, col++);
        item->m_sSpriteFrame = (short)sqlite3_column_int(stmt, col++);
        item->m_wPrice = (uint32_t)sqlite3_column_int(stmt, col++);
        item->m_bIsForSale = (sqlite3_column_int(stmt, col++) != 0);
        item->m_wWeight = (uint16_t)sqlite3_column_int(stmt, col++);
        item->m_cApprValue = (char)sqlite3_column_int(stmt, col++);
        item->m_cSpeed = (char)sqlite3_column_int(stmt, col++);
        item->m_sLevelLimit = (short)sqlite3_column_int(stmt, col++);
        item->m_cGenderLimit = (char)sqlite3_column_int(stmt, col++);
        item->m_sSpecialEffectValue1 = (short)sqlite3_column_int(stmt, col++);
        item->m_sSpecialEffectValue2 = (short)sqlite3_column_int(stmt, col++);
        item->m_sRelatedSkill = (short)sqlite3_column_int(stmt, col++);
        item->m_cCategory = (char)sqlite3_column_int(stmt, col++);
        item->m_cItemColor = (char)sqlite3_column_int(stmt, col++);

        itemList[itemId] = item;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool SaveRealmConfig(sqlite3* db, const CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    if (!BeginTransaction(db)) {
        return false;
    }

    if (!ClearTable(db, "realmlist") || !ClearTable(db, "active_maps")) {
        RollbackTransaction(db);
        return false;
    }

    // Insert realm configuration
    sqlite3_stmt* stmtRealm = nullptr;
    const char* realmSql =
        "INSERT INTO realmlist(id, realm_name, login_listen_ip, login_listen_port, "
        "game_server_listen_ip, game_server_listen_port, game_server_connection_ip, game_server_connection_port) "
        "VALUES(1, ?, ?, ?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, realmSql, -1, &stmtRealm, nullptr) != SQLITE_OK) {
        RollbackTransaction(db);
        return false;
    }

    bool ok = true;
    ok &= PrepareAndBindText(stmtRealm, 1, game->m_cRealmName);
    ok &= PrepareAndBindText(stmtRealm, 2, game->m_cLoginListenIP);
    ok &= (sqlite3_bind_int(stmtRealm, 3, game->m_iLoginListenPort) == SQLITE_OK);
    ok &= PrepareAndBindText(stmtRealm, 4, game->m_cGameListenIP);
    ok &= (sqlite3_bind_int(stmtRealm, 5, game->m_iGameListenPort) == SQLITE_OK);

    // Optional connection IP/port (bind NULL if empty)
    if (game->m_cGameConnectionIP[0] != '\0') {
        ok &= PrepareAndBindText(stmtRealm, 6, game->m_cGameConnectionIP);
        ok &= (sqlite3_bind_int(stmtRealm, 7, game->m_iGameConnectionPort) == SQLITE_OK);
    } else {
        ok &= (sqlite3_bind_null(stmtRealm, 6) == SQLITE_OK);
        ok &= (sqlite3_bind_null(stmtRealm, 7) == SQLITE_OK);
    }

    if (!ok || sqlite3_step(stmtRealm) != SQLITE_DONE) {
        sqlite3_finalize(stmtRealm);
        RollbackTransaction(db);
        return false;
    }
    sqlite3_finalize(stmtRealm);

    // Insert maps (active = 1 since we're saving currently loaded maps)
    sqlite3_stmt* stmtMap = nullptr;
    if (sqlite3_prepare_v2(db, "INSERT INTO active_maps(map_index, map_name, active) VALUES(?, ?, 1);", -1, &stmtMap, nullptr) != SQLITE_OK) {
        RollbackTransaction(db);
        return false;
    }

    int mapIndex = 0;
    for(int i = 0; i < MaxMaps; i++) {
        if (game->m_pMapList[i] == nullptr) {
            continue;
        }
        sqlite3_reset(stmtMap);
        sqlite3_clear_bindings(stmtMap);
        ok = (sqlite3_bind_int(stmtMap, 1, mapIndex++) == SQLITE_OK);
        ok &= PrepareAndBindText(stmtMap, 2, game->m_pMapList[i]->m_cName);
        if (!ok || sqlite3_step(stmtMap) != SQLITE_DONE) {
            sqlite3_finalize(stmtMap);
            RollbackTransaction(db);
            return false;
        }
    }

    sqlite3_finalize(stmtMap);

    if (!CommitTransaction(db)) {
        RollbackTransaction(db);
        return false;
    }
    return true;
}

bool LoadRealmConfig(sqlite3* db, CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    // Load realm configuration
    sqlite3_stmt* stmt = nullptr;
    const char* realmSql =
        "SELECT realm_name, login_listen_ip, login_listen_port, "
        "game_server_listen_ip, game_server_listen_port, "
        "game_server_connection_ip, game_server_connection_port "
        "FROM realmlist WHERE id = 1;";

    if (sqlite3_prepare_v2(db, realmSql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    bool realmLoaded = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* realmName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* loginIP = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        int loginPort = sqlite3_column_int(stmt, 2);
        const char* gameIP = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        int gamePort = sqlite3_column_int(stmt, 4);
        const char* connIP = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        int connPort = sqlite3_column_int(stmt, 6);

        if (realmName) {
            std::snprintf(game->m_cRealmName, sizeof(game->m_cRealmName), "%s", realmName);
        }
        if (loginIP) {
            std::snprintf(game->m_cLoginListenIP, sizeof(game->m_cLoginListenIP), "%s", loginIP);
        }
        game->m_iLoginListenPort = loginPort;

        if (gameIP) {
            std::snprintf(game->m_cGameListenIP, sizeof(game->m_cGameListenIP), "%s", gameIP);
        }
        game->m_iGameListenPort = gamePort;

        // Optional connection IP/port
        if (connIP && connIP[0] != '\0') {
            std::snprintf(game->m_cGameConnectionIP, sizeof(game->m_cGameConnectionIP), "%s", connIP);
            game->m_iGameConnectionPort = connPort;
        } else {
            game->m_cGameConnectionIP[0] = '\0';
            game->m_iGameConnectionPort = 0;
        }

        realmLoaded = true;

        char logMsg[256] = {};
        std::snprintf(logMsg, sizeof(logMsg), "Loaded realm: %s (Login: %s:%d, Game: %s:%d)",
            game->m_cRealmName, game->m_cLoginListenIP, game->m_iLoginListenPort,
            game->m_cGameListenIP, game->m_iGameListenPort);
        PutLogListLevel(LOG_LEVEL_NOTICE, logMsg);
    }

    sqlite3_finalize(stmt);

    if (!realmLoaded) {
        PutLogListLevel(LOG_LEVEL_ERROR, "No realm configuration found in realmlist table (id=1)");
        return false;
    }

    // Load active maps only
    if (sqlite3_prepare_v2(db, "SELECT map_name FROM active_maps WHERE active = 1 ORDER BY map_index;", -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    int mapsLoaded = 0;
    PutLogListLevel(LOG_LEVEL_NOTICE, "Loading active maps from GameConfigs.db...");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* nameText = sqlite3_column_text(stmt, 0);
        if (nameText == nullptr) {
            continue;
        }
        const char* name = reinterpret_cast<const char*>(nameText);
        if (!game->_bRegisterMap(const_cast<char*>(name))) {
            char logMsg[256] = {};
            std::snprintf(logMsg, sizeof(logMsg), "Map load failed: %s", name);
            PutLogListLevel(LOG_LEVEL_ERROR, logMsg);
            sqlite3_finalize(stmt);
            return false;
        }
        mapsLoaded++;
    }

    sqlite3_finalize(stmt);
    {
        char logMsg[128] = {};
        std::snprintf(logMsg, sizeof(logMsg), "Loaded %d maps.", mapsLoaded);
        PutLogListLevel(LOG_LEVEL_NOTICE, logMsg);
    }
    return true;
}

bool SaveSettingsConfig(sqlite3* db, const CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    if (!BeginTransaction(db)) {
        return false;
    }

    if (!ClearTable(db, "settings")) {
        RollbackTransaction(db);
        return false;
    }

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "INSERT INTO settings(key, value) VALUES(?, ?);", -1, &stmt, nullptr) != SQLITE_OK) {
        RollbackTransaction(db);
        return false;
    }

    bool ok = true;
    ok &= InsertKeyValueFloat(stmt, "primary-drop-rate", game->m_fPrimaryDropRate);
    ok &= InsertKeyValueFloat(stmt, "gold-drop-rate", game->m_fGoldDropRate);
    ok &= InsertKeyValueFloat(stmt, "secondary-drop-rate", game->m_fSecondaryDropRate);
    ok &= InsertKeyValue(stmt, "enemy-kill-mode", game->m_bEnemyKillMode ? "deathmatch" : "classic");
    ok &= InsertKeyValueInt(stmt, "enemy-kill-adjust", game->m_iEnemyKillAdjust);
    ok &= InsertKeyValueInt(stmt, "monday-raid-time", game->m_sRaidTimeMonday);
    ok &= InsertKeyValueInt(stmt, "tuesday-raid-time", game->m_sRaidTimeTuesday);
    ok &= InsertKeyValueInt(stmt, "wednesday-raid-time", game->m_sRaidTimeWednesday);
    ok &= InsertKeyValueInt(stmt, "thursday-raid-time", game->m_sRaidTimeThursday);
    ok &= InsertKeyValueInt(stmt, "friday-raid-time", game->m_sRaidTimeFriday);
    ok &= InsertKeyValueInt(stmt, "saturday-raid-time", game->m_sRaidTimeSaturday);
    ok &= InsertKeyValueInt(stmt, "sunday-raid-time", game->m_sRaidTimeSunday);
    ok &= InsertKeyValueInt(stmt, "slate-success-rate", game->m_sSlateSuccessRate);
    ok &= InsertKeyValueInt(stmt, "rep-drop-modifier", game->m_cRepDropModifier);

    // Timing Settings
    ok &= InsertKeyValueInt(stmt, "client-timeout-ms", game->m_iClientTimeout);
    ok &= InsertKeyValueInt(stmt, "stamina-regen-interval", game->m_iStaminaRegenInterval);
    ok &= InsertKeyValueInt(stmt, "poison-damage-interval", game->m_iPoisonDamageInterval);
    ok &= InsertKeyValueInt(stmt, "health-regen-interval", game->m_iHealthRegenInterval);
    ok &= InsertKeyValueInt(stmt, "mana-regen-interval", game->m_iManaRegenInterval);
    ok &= InsertKeyValueInt(stmt, "hunger-consume-interval", game->m_iHungerConsumeInterval);
    ok &= InsertKeyValueInt(stmt, "summon-creature-duration", game->m_iSummonCreatureDuration);
    ok &= InsertKeyValueInt(stmt, "autosave-interval", game->m_iAutosaveInterval);
    ok &= InsertKeyValueInt(stmt, "lag-protection-interval", game->m_iLagProtectionInterval);

    // Character/Leveling Settings
    ok &= InsertKeyValueInt(stmt, "base-stat-value", game->m_iBaseStatValue);
    ok &= InsertKeyValueInt(stmt, "creation-stat-bonus", game->m_iCreationStatBonus);
    ok &= InsertKeyValueInt(stmt, "levelup-stat-gain", game->m_iLevelupStatGain);
    ok &= InsertKeyValueInt(stmt, "max-level", game->m_iMaxLevel);

    // Combat Settings
    ok &= InsertKeyValueInt(stmt, "minimum-hit-ratio", game->m_iMinimumHitRatio);
    ok &= InsertKeyValueInt(stmt, "maximum-hit-ratio", game->m_iMaximumHitRatio);

    // Gameplay Settings
    ok &= InsertKeyValueInt(stmt, "nighttime-duration", game->m_iNighttimeDuration);
    ok &= InsertKeyValueInt(stmt, "starting-guild-rank", game->m_iStartingGuildRank);
    ok &= InsertKeyValueInt(stmt, "grand-magic-mana-consumption", game->m_iGrandMagicManaConsumption);
    ok &= InsertKeyValueInt(stmt, "maximum-construction-points", game->m_iMaxConstructionPoints);
    ok &= InsertKeyValueInt(stmt, "maximum-summon-points", game->m_iMaxSummonPoints);
    ok &= InsertKeyValueInt(stmt, "maximum-war-contribution", game->m_iMaxWarContribution);
    ok &= InsertKeyValueInt(stmt, "max-bank-items", game->m_iMaxBankItems);

    sqlite3_finalize(stmt);
    if (!ok) {
        RollbackTransaction(db);
        return false;
    }

    if (!CommitTransaction(db)) {
        RollbackTransaction(db);
        return false;
    }
    return true;
}

bool LoadSettingsConfig(sqlite3* db, CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT key, value FROM settings;", -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* keyText = sqlite3_column_text(stmt, 0);
        const unsigned char* valueText = sqlite3_column_text(stmt, 1);
        if (keyText == nullptr || valueText == nullptr) {
            continue;
        }
        const char* key = reinterpret_cast<const char*>(keyText);
        const char* value = reinterpret_cast<const char*>(valueText);

        if (std::strcmp(key, "primary-drop-rate") == 0) {
            game->m_fPrimaryDropRate = static_cast<float>(std::atof(value));
        } else if (std::strcmp(key, "gold-drop-rate") == 0) {
            game->m_fGoldDropRate = static_cast<float>(std::atof(value));
        } else if (std::strcmp(key, "secondary-drop-rate") == 0) {
            game->m_fSecondaryDropRate = static_cast<float>(std::atof(value));
        } else if (std::strcmp(key, "enemy-kill-mode") == 0) {
            if (_stricmp(value, "deathmatch") == 0) {
                game->m_bEnemyKillMode = true;
            } else if (_stricmp(value, "classic") == 0) {
                game->m_bEnemyKillMode = false;
            } else {
                game->m_bEnemyKillMode = (std::atoi(value) != 0);
            }
        } else if (std::strcmp(key, "enemy-kill-adjust") == 0) {
            game->m_iEnemyKillAdjust = std::atoi(value);
        } else if (std::strcmp(key, "monday-raid-time") == 0) {
            game->m_sRaidTimeMonday = (short)std::atoi(value);
        } else if (std::strcmp(key, "tuesday-raid-time") == 0) {
            game->m_sRaidTimeTuesday = (short)std::atoi(value);
        } else if (std::strcmp(key, "wednesday-raid-time") == 0) {
            game->m_sRaidTimeWednesday = (short)std::atoi(value);
        } else if (std::strcmp(key, "thursday-raid-time") == 0) {
            game->m_sRaidTimeThursday = (short)std::atoi(value);
        } else if (std::strcmp(key, "friday-raid-time") == 0) {
            game->m_sRaidTimeFriday = (short)std::atoi(value);
        } else if (std::strcmp(key, "saturday-raid-time") == 0) {
            game->m_sRaidTimeSaturday = (short)std::atoi(value);
        } else if (std::strcmp(key, "sunday-raid-time") == 0) {
            game->m_sRaidTimeSunday = (short)std::atoi(value);
        } else if (std::strcmp(key, "slate-success-rate") == 0) {
            game->m_sSlateSuccessRate = (short)std::atoi(value);
        } else if (std::strcmp(key, "rep-drop-modifier") == 0) {
            game->m_cRepDropModifier = (char)std::atoi(value);
        }
        // Timing Settings
        else if (std::strcmp(key, "client-timeout-ms") == 0) {
            game->m_iClientTimeout = std::atoi(value);
        } else if (std::strcmp(key, "stamina-regen-interval") == 0) {
            game->m_iStaminaRegenInterval = std::atoi(value);
        } else if (std::strcmp(key, "poison-damage-interval") == 0) {
            game->m_iPoisonDamageInterval = std::atoi(value);
        } else if (std::strcmp(key, "health-regen-interval") == 0) {
            game->m_iHealthRegenInterval = std::atoi(value);
        } else if (std::strcmp(key, "mana-regen-interval") == 0) {
            game->m_iManaRegenInterval = std::atoi(value);
        } else if (std::strcmp(key, "hunger-consume-interval") == 0) {
            game->m_iHungerConsumeInterval = std::atoi(value);
        } else if (std::strcmp(key, "summon-creature-duration") == 0) {
            game->m_iSummonCreatureDuration = std::atoi(value);
        } else if (std::strcmp(key, "autosave-interval") == 0) {
            game->m_iAutosaveInterval = std::atoi(value);
        } else if (std::strcmp(key, "lag-protection-interval") == 0) {
            game->m_iLagProtectionInterval = std::atoi(value);
        }
        // Character/Leveling Settings
        else if (std::strcmp(key, "base-stat-value") == 0) {
            game->m_iBaseStatValue = std::atoi(value);
        } else if (std::strcmp(key, "creation-stat-bonus") == 0) {
            game->m_iCreationStatBonus = std::atoi(value);
        } else if (std::strcmp(key, "levelup-stat-gain") == 0) {
            game->m_iLevelupStatGain = std::atoi(value);
        } else if (std::strcmp(key, "max-level") == 0 || std::strcmp(key, "max-player-level") == 0) {
            game->m_iMaxLevel = std::atoi(value);
        }
        // Combat Settings
        else if (std::strcmp(key, "minimum-hit-ratio") == 0) {
            game->m_iMinimumHitRatio = std::atoi(value);
        } else if (std::strcmp(key, "maximum-hit-ratio") == 0) {
            game->m_iMaximumHitRatio = std::atoi(value);
        }
        // Gameplay Settings
        else if (std::strcmp(key, "nighttime-duration") == 0) {
            game->m_iNighttimeDuration = std::atoi(value);
        } else if (std::strcmp(key, "starting-guild-rank") == 0) {
            game->m_iStartingGuildRank = std::atoi(value);
        } else if (std::strcmp(key, "grand-magic-mana-consumption") == 0) {
            game->m_iGrandMagicManaConsumption = std::atoi(value);
        } else if (std::strcmp(key, "maximum-construction-points") == 0) {
            game->m_iMaxConstructionPoints = std::atoi(value);
        } else if (std::strcmp(key, "maximum-summon-points") == 0) {
            game->m_iMaxSummonPoints = std::atoi(value);
        } else if (std::strcmp(key, "maximum-war-contribution") == 0) {
            game->m_iMaxWarContribution = std::atoi(value);
        } else if (std::strcmp(key, "max-bank-items") == 0) {
            game->m_iMaxBankItems = std::atoi(value);
        }
    }

    sqlite3_finalize(stmt);
    return true;
}


bool SaveBannedListConfig(sqlite3* db, const CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    if (!BeginTransaction(db)) {
        return false;
    }

    if (!ClearTable(db, "banned_list")) {
        RollbackTransaction(db);
        return false;
    }

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "INSERT INTO banned_list(ip_address) VALUES(?);", -1, &stmt, nullptr) != SQLITE_OK) {
        RollbackTransaction(db);
        return false;
    }

    for(int i = 0; i < MaxBanned; i++) {
        if (game->m_stBannedList[i].m_cBannedIPaddress[0] == 0) {
            continue;
        }
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        if (!PrepareAndBindText(stmt, 1, game->m_stBannedList[i].m_cBannedIPaddress) ||
            sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            RollbackTransaction(db);
            return false;
        }
    }

    sqlite3_finalize(stmt);
    if (!CommitTransaction(db)) {
        RollbackTransaction(db);
        return false;
    }
    return true;
}

bool LoadBannedListConfig(sqlite3* db, CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    for(int i = 0; i < MaxBanned; i++) {
        std::memset(game->m_stBannedList[i].m_cBannedIPaddress, 0, sizeof(game->m_stBannedList[i].m_cBannedIPaddress));
    }

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT ip_address FROM banned_list ORDER BY ip_address;", -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    int index = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (index >= MaxBanned) {
            break;
        }
        CopyColumnText(stmt, 0, game->m_stBannedList[index].m_cBannedIPaddress, sizeof(game->m_stBannedList[index].m_cBannedIPaddress));
        index++;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool SaveAdminConfig(sqlite3* db, const CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    if (!BeginTransaction(db)) {
        return false;
    }

    if (!ClearTable(db, "admins")) {
        RollbackTransaction(db);
        return false;
    }

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "INSERT INTO admins(account_name, character_name, approved_ip, admin_level) VALUES(?,?,?,?);", -1, &stmt, nullptr) != SQLITE_OK) {
        RollbackTransaction(db);
        return false;
    }

    for (int i = 0; i < game->m_iAdminCount; i++) {
        if (game->m_stAdminList[i].m_cAccountName[0] == 0) {
            continue;
        }
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        if (!PrepareAndBindText(stmt, 1, game->m_stAdminList[i].m_cAccountName) ||
            !PrepareAndBindText(stmt, 2, game->m_stAdminList[i].m_cCharName) ||
            !PrepareAndBindText(stmt, 3, game->m_stAdminList[i].m_cApprovedIP) ||
            sqlite3_bind_int(stmt, 4, game->m_stAdminList[i].m_iAdminLevel) != SQLITE_OK ||
            sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            RollbackTransaction(db);
            return false;
        }
    }

    sqlite3_finalize(stmt);
    if (!CommitTransaction(db)) {
        RollbackTransaction(db);
        return false;
    }
    return true;
}

bool LoadAdminConfig(sqlite3* db, CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    for (int i = 0; i < MaxAdmins; i++) {
        std::memset(&game->m_stAdminList[i], 0, sizeof(AdminEntry));
    }
    game->m_iAdminCount = 0;

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT account_name, character_name, approved_ip, admin_level FROM admins ORDER BY account_name;", -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    int index = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (index >= MaxAdmins) {
            break;
        }
        CopyColumnText(stmt, 0, game->m_stAdminList[index].m_cAccountName, sizeof(game->m_stAdminList[index].m_cAccountName));
        CopyColumnText(stmt, 1, game->m_stAdminList[index].m_cCharName, sizeof(game->m_stAdminList[index].m_cCharName));
        CopyColumnText(stmt, 2, game->m_stAdminList[index].m_cApprovedIP, sizeof(game->m_stAdminList[index].m_cApprovedIP));
        game->m_stAdminList[index].m_iAdminLevel = sqlite3_column_int(stmt, 3);
        if (game->m_stAdminList[index].m_iAdminLevel < 1)
            game->m_stAdminList[index].m_iAdminLevel = 1;
        index++;
    }

    sqlite3_finalize(stmt);
    game->m_iAdminCount = index;
    return true;
}

bool SaveCommandPermissions(sqlite3* db, const CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    if (!BeginTransaction(db)) {
        return false;
    }

    if (!ClearTable(db, "admin_command_permissions")) {
        RollbackTransaction(db);
        return false;
    }

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "INSERT INTO admin_command_permissions(command, admin_level, description) VALUES(?,?,?);", -1, &stmt, nullptr) != SQLITE_OK) {
        RollbackTransaction(db);
        return false;
    }

    for (const auto& pair : game->m_commandPermissions) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        sqlite3_bind_text(stmt, 1, pair.first.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, pair.second.iAdminLevel);
        sqlite3_bind_text(stmt, 3, pair.second.sDescription.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            RollbackTransaction(db);
            return false;
        }
    }

    sqlite3_finalize(stmt);
    if (!CommitTransaction(db)) {
        RollbackTransaction(db);
        return false;
    }
    return true;
}

bool LoadCommandPermissions(sqlite3* db, CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    game->m_commandPermissions.clear();

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT command, admin_level, description FROM admin_command_permissions;", -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        int level = sqlite3_column_int(stmt, 1);
        const char* desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        if (name != nullptr) {
            CommandPermission perm;
            perm.iAdminLevel = level;
            perm.sDescription = (desc != nullptr) ? desc : "";
            game->m_commandPermissions[name] = perm;
        }
    }

    sqlite3_finalize(stmt);
    return true;
}

bool SaveNpcConfigs(sqlite3* db, const CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    if (!BeginTransaction(db)) {
        return false;
    }

    if (!ClearTable(db, "npc_configs")) {
        RollbackTransaction(db);
        return false;
    }

    const char* sql =
        "INSERT INTO npc_configs("
        " npc_id, name, npc_type, hit_dice, defense_ratio, hit_ratio, min_bravery,"
        " exp_min, exp_max, gold_min, gold_max, attack_dice_throw, attack_dice_range,"
        " npc_size, side, action_limit, action_time, resist_magic, magic_level,"
        " day_of_week_limit, chat_msg_presence, target_search_range, regen_time,"
        " attribute, abs_damage, max_mana, magic_hit_ratio, attack_range, drop_table_id"
        ") VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        RollbackTransaction(db);
        return false;
    }

    for(int i = 0; i < MaxNpcTypes; i++) {
        if (game->m_pNpcConfigList[i] == nullptr) {
            continue;
        }

        const CNpc* npc = game->m_pNpcConfigList[i];
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        int col = 1;
        bool ok = true;
        ok &= (sqlite3_bind_int(stmt, col++, i) == SQLITE_OK);
        ok &= PrepareAndBindText(stmt, col++, npc->m_cNpcName);
        ok &= (sqlite3_bind_int(stmt, col++, npc->m_sType) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, npc->m_iHitDice) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, npc->m_iDefenseRatio) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, npc->m_iHitRatio) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, npc->m_iMinBravery) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, static_cast<int>(npc->m_iExpDiceMin)) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, static_cast<int>(npc->m_iExpDiceMax)) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, static_cast<int>(npc->m_iGoldDiceMin)) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, static_cast<int>(npc->m_iGoldDiceMax)) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, npc->m_cAttackDiceThrow) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, npc->m_cAttackDiceRange) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, npc->m_cSize) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, npc->m_cSide) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, npc->m_cActionLimit) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, static_cast<int>(npc->m_dwActionTime)) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, npc->m_cResistMagic) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, npc->m_cMagicLevel) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, npc->m_cDayOfWeekLimit) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, npc->m_cChatMsgPresence) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, npc->m_cTargetSearchRange) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, static_cast<int>(npc->m_dwRegenTime)) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, npc->m_cAttribute) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, npc->m_iAbsDamage) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, npc->m_iMaxMana) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, npc->m_iMagicHitRatio) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, npc->m_iAttackRange) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, npc->m_iDropTableId) == SQLITE_OK);

        if (!ok || sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            RollbackTransaction(db);
            return false;
        }
    }

    sqlite3_finalize(stmt);
    if (!CommitTransaction(db)) {
        RollbackTransaction(db);
        return false;
    }
    return true;
}

bool LoadNpcConfigs(sqlite3* db, CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    for(int i = 0; i < MaxNpcTypes; i++) {
        delete game->m_pNpcConfigList[i];
        game->m_pNpcConfigList[i] = nullptr;
    }

    const char* sql =
        "SELECT npc_id, name, npc_type, hit_dice, defense_ratio, hit_ratio, min_bravery,"
        " exp_min, exp_max, gold_min, gold_max, attack_dice_throw, attack_dice_range,"
        " npc_size, side, action_limit, action_time, resist_magic, magic_level,"
        " day_of_week_limit, chat_msg_presence, target_search_range, regen_time,"
        " attribute, abs_damage, max_mana, magic_hit_ratio, attack_range, drop_table_id"
        " FROM npc_configs ORDER BY npc_id;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = 0;
        int npcId = sqlite3_column_int(stmt, col++);
        if (npcId < 0 || npcId >= MaxNpcTypes) {
            continue;
        }

        CNpc* npc = new CNpc(" ");
        std::memset(npc->m_cNpcName, 0, sizeof(npc->m_cNpcName));
        CopyColumnText(stmt, col++, npc->m_cNpcName, sizeof(npc->m_cNpcName));
        npc->m_sType = (short)sqlite3_column_int(stmt, col++);
        npc->m_iHitDice = sqlite3_column_int(stmt, col++);
        npc->m_iDefenseRatio = sqlite3_column_int(stmt, col++);
        npc->m_iHitRatio = sqlite3_column_int(stmt, col++);
        npc->m_iMinBravery = sqlite3_column_int(stmt, col++);
        npc->m_iExpDiceMin = sqlite3_column_int(stmt, col++);
        npc->m_iExpDiceMax = sqlite3_column_int(stmt, col++);
        npc->m_iGoldDiceMin = sqlite3_column_int(stmt, col++);
        npc->m_iGoldDiceMax = sqlite3_column_int(stmt, col++);
        npc->m_cAttackDiceThrow = (char)sqlite3_column_int(stmt, col++);
        npc->m_cAttackDiceRange = (char)sqlite3_column_int(stmt, col++);
        npc->m_cSize = (char)sqlite3_column_int(stmt, col++);
        npc->m_cSide = (char)sqlite3_column_int(stmt, col++);
        npc->m_cActionLimit = (char)sqlite3_column_int(stmt, col++);
        npc->m_dwActionTime = (uint32_t)sqlite3_column_int(stmt, col++);
        npc->m_cResistMagic = (char)sqlite3_column_int(stmt, col++);
        npc->m_cMagicLevel = (char)sqlite3_column_int(stmt, col++);
        npc->m_cDayOfWeekLimit = (char)sqlite3_column_int(stmt, col++);
        npc->m_cChatMsgPresence = (char)sqlite3_column_int(stmt, col++);
        npc->m_cTargetSearchRange = (char)sqlite3_column_int(stmt, col++);
        npc->m_dwRegenTime = (uint32_t)sqlite3_column_int(stmt, col++);
        npc->m_cAttribute = (char)sqlite3_column_int(stmt, col++);
        npc->m_iAbsDamage = sqlite3_column_int(stmt, col++);
        npc->m_iMaxMana = sqlite3_column_int(stmt, col++);
        npc->m_iMagicHitRatio = sqlite3_column_int(stmt, col++);
        npc->m_iAttackRange = sqlite3_column_int(stmt, col++);
        npc->m_iDropTableId = sqlite3_column_int(stmt, col++);

        game->m_pNpcConfigList[npcId] = npc;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool LoadDropTables(sqlite3* db, CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    game->m_DropTables.clear();

    const char* tableSql = "SELECT drop_table_id, name, description FROM drop_tables ORDER BY drop_table_id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, tableSql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        DropTable table = {};
        table.id = sqlite3_column_int(stmt, 0);
        std::memset(table.name, 0, sizeof(table.name));
        std::memset(table.description, 0, sizeof(table.description));
        CopyColumnText(stmt, 1, table.name, sizeof(table.name));
        CopyColumnText(stmt, 2, table.description, sizeof(table.description));
        std::memset(table.totalWeight, 0, sizeof(table.totalWeight));
        game->m_DropTables[table.id] = table;
    }
    sqlite3_finalize(stmt);

    const char* entrySql =
        "SELECT drop_table_id, tier, item_id, weight, min_count, max_count"
        " FROM drop_entries ORDER BY drop_table_id, tier;";
    if (sqlite3_prepare_v2(db, entrySql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int tableId = sqlite3_column_int(stmt, 0);
        int tier = sqlite3_column_int(stmt, 1);
        int itemId = sqlite3_column_int(stmt, 2);
        int weight = sqlite3_column_int(stmt, 3);
        int minCount = sqlite3_column_int(stmt, 4);
        int maxCount = sqlite3_column_int(stmt, 5);

        auto it = game->m_DropTables.find(tableId);
        if (it == game->m_DropTables.end()) {
            continue;
        }
        if (tier < 1 || tier > 2) {
            continue;
        }
        if (weight <= 0) {
            continue;
        }

        DropEntry entry = {};
        entry.itemId = itemId;
        entry.weight = weight;
        entry.minCount = minCount;
        entry.maxCount = maxCount;
        it->second.tierEntries[tier].push_back(entry);
        it->second.totalWeight[tier] += weight;
    }
    sqlite3_finalize(stmt);

    return !game->m_DropTables.empty();
}

bool LoadShopConfigs(sqlite3* db, CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    game->m_NpcShopMappings.clear();
    game->m_ShopData.clear();
    game->m_bIsShopDataAvailable = false;

    // Load NPC -> shop mappings
    const char* mappingSql = "SELECT npc_type, shop_id, description FROM npc_shop_mapping ORDER BY npc_type;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, mappingSql, -1, &stmt, nullptr) != SQLITE_OK) {
        char logMsg[256] = {};
        std::snprintf(logMsg, sizeof(logMsg), "(!) LoadShopConfigs: Failed to prepare npc_shop_mapping query");
        PutLogList(logMsg);
        return false;
    }

    int mappingCount = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int npcType = sqlite3_column_int(stmt, 0);
        int shopId = sqlite3_column_int(stmt, 1);
        game->m_NpcShopMappings[npcType] = shopId;
        mappingCount++;
    }
    sqlite3_finalize(stmt);

    // Load shop items
    const char* itemsSql = "SELECT shop_id, item_id FROM shop_items ORDER BY shop_id, item_id;";
    if (sqlite3_prepare_v2(db, itemsSql, -1, &stmt, nullptr) != SQLITE_OK) {
        char logMsg[256] = {};
        std::snprintf(logMsg, sizeof(logMsg), "(!) LoadShopConfigs: Failed to prepare shop_items query");
        PutLogList(logMsg);
        return false;
    }

    int itemCount = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int shopId = sqlite3_column_int(stmt, 0);
        int16_t itemId = static_cast<int16_t>(sqlite3_column_int(stmt, 1));

        // Create shop if it doesn't exist
        if (game->m_ShopData.find(shopId) == game->m_ShopData.end()) {
            ShopData shop = {};
            shop.shopId = shopId;
            game->m_ShopData[shopId] = shop;
        }

        game->m_ShopData[shopId].itemIds.push_back(itemId);
        itemCount++;
    }
    sqlite3_finalize(stmt);

    if (mappingCount > 0 || itemCount > 0) {
        game->m_bIsShopDataAvailable = true;
        char logMsg[256] = {};
        std::snprintf(logMsg, sizeof(logMsg), "(!) Shop configs loaded: %d NPC mappings, %d shops, %d total items",
            mappingCount, static_cast<int>(game->m_ShopData.size()), itemCount);
        PutLogList(logMsg);
    }

    return true;
}

bool SaveMagicConfigs(sqlite3* db, const CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    if (!BeginTransaction(db)) {
        return false;
    }

    if (!ClearTable(db, "magic_configs")) {
        RollbackTransaction(db);
        return false;
    }

    const char* sql =
        "INSERT INTO magic_configs("
        " magic_id, name, magic_type, delay_time, last_time, value1, value2, value3,"
        " value4, value5, value6, value7, value8, value9, value10, value11, value12,"
        " int_limit, gold_cost, category, attribute"
        ") VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        RollbackTransaction(db);
        return false;
    }

    for(int i = 0; i < hb::shared::limits::MaxMagicType; i++) {
        if (game->m_pMagicConfigList[i] == nullptr) {
            continue;
        }

        const CMagic* magic = game->m_pMagicConfigList[i];
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        int col = 1;
        bool ok = true;
        ok &= (sqlite3_bind_int(stmt, col++, i) == SQLITE_OK);
        ok &= PrepareAndBindText(stmt, col++, magic->m_cName);
        ok &= (sqlite3_bind_int(stmt, col++, magic->m_sType) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, static_cast<int>(magic->m_dwDelayTime)) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, static_cast<int>(magic->m_dwLastTime)) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, magic->m_sValue1) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, magic->m_sValue2) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, magic->m_sValue3) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, magic->m_sValue4) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, magic->m_sValue5) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, magic->m_sValue6) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, magic->m_sValue7) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, magic->m_sValue8) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, magic->m_sValue9) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, magic->m_sValue10) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, magic->m_sValue11) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, magic->m_sValue12) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, magic->m_sIntLimit) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, magic->m_iGoldCost) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, magic->m_cCategory) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, magic->m_iAttribute) == SQLITE_OK);

        if (!ok || sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            RollbackTransaction(db);
            return false;
        }
    }

    sqlite3_finalize(stmt);
    if (!CommitTransaction(db)) {
        RollbackTransaction(db);
        return false;
    }
    return true;
}

bool LoadMagicConfigs(sqlite3* db, CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    for(int i = 0; i < hb::shared::limits::MaxMagicType; i++) {
        delete game->m_pMagicConfigList[i];
        game->m_pMagicConfigList[i] = nullptr;
    }

    const char* sql =
        "SELECT magic_id, name, magic_type, delay_time, last_time, value1, value2, value3,"
        " value4, value5, value6, value7, value8, value9, value10, value11, value12,"
        " int_limit, gold_cost, category, attribute"
        " FROM magic_configs ORDER BY magic_id;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = 0;
        int magicId = sqlite3_column_int(stmt, col++);
        if (magicId < 0 || magicId >= hb::shared::limits::MaxMagicType) {
            continue;
        }

        CMagic* magic = new CMagic();
        CopyColumnText(stmt, col++, magic->m_cName, sizeof(magic->m_cName));
        magic->m_sType = (short)sqlite3_column_int(stmt, col++);
        magic->m_dwDelayTime = (uint32_t)sqlite3_column_int(stmt, col++);
        magic->m_dwLastTime = (uint32_t)sqlite3_column_int(stmt, col++);
        magic->m_sValue1 = (short)sqlite3_column_int(stmt, col++);
        magic->m_sValue2 = (short)sqlite3_column_int(stmt, col++);
        magic->m_sValue3 = (short)sqlite3_column_int(stmt, col++);
        magic->m_sValue4 = (short)sqlite3_column_int(stmt, col++);
        magic->m_sValue5 = (short)sqlite3_column_int(stmt, col++);
        magic->m_sValue6 = (short)sqlite3_column_int(stmt, col++);
        magic->m_sValue7 = (short)sqlite3_column_int(stmt, col++);
        magic->m_sValue8 = (short)sqlite3_column_int(stmt, col++);
        magic->m_sValue9 = (short)sqlite3_column_int(stmt, col++);
        magic->m_sValue10 = (short)sqlite3_column_int(stmt, col++);
        magic->m_sValue11 = (short)sqlite3_column_int(stmt, col++);
        magic->m_sValue12 = (short)sqlite3_column_int(stmt, col++);
        magic->m_sIntLimit = (short)sqlite3_column_int(stmt, col++);
        magic->m_iGoldCost = sqlite3_column_int(stmt, col++);
        magic->m_cCategory = (char)sqlite3_column_int(stmt, col++);
        magic->m_iAttribute = sqlite3_column_int(stmt, col++);

        game->m_pMagicConfigList[magicId] = magic;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool SaveSkillConfigs(sqlite3* db, const CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    if (!BeginTransaction(db)) {
        return false;
    }

    if (!ClearTable(db, "skill_configs")) {
        RollbackTransaction(db);
        return false;
    }

    const char* sql =
        "INSERT INTO skill_configs("
        " skill_id, name, skill_type, value1, value2, value3, value4, value5, value6,"
        " is_useable, use_method"
        ") VALUES(?,?,?,?,?,?,?,?,?,?,?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        RollbackTransaction(db);
        return false;
    }

    for(int i = 0; i < hb::shared::limits::MaxSkillType; i++) {
        if (game->m_pSkillConfigList[i] == nullptr) {
            continue;
        }

        const CSkill* skill = game->m_pSkillConfigList[i];
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        int col = 1;
        bool ok = true;
        ok &= (sqlite3_bind_int(stmt, col++, i) == SQLITE_OK);
        ok &= PrepareAndBindText(stmt, col++, skill->m_cName);
        ok &= (sqlite3_bind_int(stmt, col++, skill->m_sType) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, skill->m_sValue1) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, skill->m_sValue2) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, skill->m_sValue3) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, skill->m_sValue4) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, skill->m_sValue5) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, skill->m_sValue6) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, skill->m_bIsUseable ? 1 : 0) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, skill->m_cUseMethod) == SQLITE_OK);

        if (!ok || sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            RollbackTransaction(db);
            return false;
        }
    }

    sqlite3_finalize(stmt);
    if (!CommitTransaction(db)) {
        RollbackTransaction(db);
        return false;
    }
    return true;
}

bool LoadSkillConfigs(sqlite3* db, CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    for(int i = 0; i < hb::shared::limits::MaxSkillType; i++) {
        delete game->m_pSkillConfigList[i];
        game->m_pSkillConfigList[i] = nullptr;
    }

    const char* sql =
        "SELECT skill_id, name, skill_type, value1, value2, value3, value4, value5, value6,"
        " is_useable, use_method"
        " FROM skill_configs ORDER BY skill_id;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = 0;
        int skillId = sqlite3_column_int(stmt, col++);
        if (skillId < 0 || skillId >= hb::shared::limits::MaxSkillType) {
            continue;
        }

        CSkill* skill = new CSkill();
        CopyColumnText(stmt, col++, skill->m_cName, sizeof(skill->m_cName));
        skill->m_sType = (short)sqlite3_column_int(stmt, col++);
        skill->m_sValue1 = (short)sqlite3_column_int(stmt, col++);
        skill->m_sValue2 = (short)sqlite3_column_int(stmt, col++);
        skill->m_sValue3 = (short)sqlite3_column_int(stmt, col++);
        skill->m_sValue4 = (short)sqlite3_column_int(stmt, col++);
        skill->m_sValue5 = (short)sqlite3_column_int(stmt, col++);
        skill->m_sValue6 = (short)sqlite3_column_int(stmt, col++);
        skill->m_bIsUseable = (sqlite3_column_int(stmt, col++) != 0);
        skill->m_cUseMethod = (char)sqlite3_column_int(stmt, col++);

        game->m_pSkillConfigList[skillId] = skill;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool SaveQuestConfigs(sqlite3* db, const CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    if (!BeginTransaction(db)) {
        return false;
    }

    if (!ClearTable(db, "quest_configs")) {
        RollbackTransaction(db);
        return false;
    }

    const char* sql =
        "INSERT INTO quest_configs("
        " quest_index, side, quest_type, target_type, max_count, quest_from, min_level, max_level,"
        " required_skill_num, required_skill_level, time_limit, assign_type,"
        " reward_type1, reward_amount1, reward_type2, reward_amount2, reward_type3, reward_amount3,"
        " contribution, contribution_limit, response_mode, target_name, target_x, target_y, target_range,"
        " quest_id, req_contribution"
        ") VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        RollbackTransaction(db);
        return false;
    }

    for(int i = 0; i < hb::server::config::MaxQuestType; i++) {
        if (game->m_pQuestManager->m_pQuestConfigList[i] == nullptr) {
            continue;
        }

        const CQuest* quest = game->m_pQuestManager->m_pQuestConfigList[i];
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        int col = 1;
        bool ok = true;
        ok &= (sqlite3_bind_int(stmt, col++, i) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_cSide) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_iType) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_iTargetType) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_iMaxCount) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_iFrom) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_iMinLevel) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_iMaxLevel) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_iRequiredSkillNum) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_iRequiredSkillLevel) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_iTimeLimit) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_iAssignType) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_iRewardType[1]) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_iRewardAmount[1]) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_iRewardType[2]) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_iRewardAmount[2]) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_iRewardType[3]) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_iRewardAmount[3]) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_iContribution) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_iContributionLimit) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_iResponseMode) == SQLITE_OK);
        ok &= PrepareAndBindText(stmt, col++, quest->m_cTargetName);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_sX) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_sY) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_iRange) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_iQuestID) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, quest->m_iReqContribution) == SQLITE_OK);

        if (!ok || sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            RollbackTransaction(db);
            return false;
        }
    }

    sqlite3_finalize(stmt);
    if (!CommitTransaction(db)) {
        RollbackTransaction(db);
        return false;
    }
    return true;
}

bool LoadQuestConfigs(sqlite3* db, CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    for(int i = 0; i < hb::server::config::MaxQuestType; i++) {
        delete game->m_pQuestManager->m_pQuestConfigList[i];
        game->m_pQuestManager->m_pQuestConfigList[i] = nullptr;
    }

    const char* sql =
        "SELECT quest_index, side, quest_type, target_type, max_count, quest_from, min_level, max_level,"
        " required_skill_num, required_skill_level, time_limit, assign_type,"
        " reward_type1, reward_amount1, reward_type2, reward_amount2, reward_type3, reward_amount3,"
        " contribution, contribution_limit, response_mode, target_name, target_x, target_y, target_range,"
        " quest_id, req_contribution"
        " FROM quest_configs ORDER BY quest_index;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = 0;
        int questIndex = sqlite3_column_int(stmt, col++);
        if (questIndex < 0 || questIndex >= hb::server::config::MaxQuestType) {
            continue;
        }

        CQuest* quest = new CQuest();
        quest->m_cSide = (char)sqlite3_column_int(stmt, col++);
        quest->m_iType = sqlite3_column_int(stmt, col++);
        quest->m_iTargetType = sqlite3_column_int(stmt, col++);
        quest->m_iMaxCount = sqlite3_column_int(stmt, col++);
        quest->m_iFrom = sqlite3_column_int(stmt, col++);
        quest->m_iMinLevel = sqlite3_column_int(stmt, col++);
        quest->m_iMaxLevel = sqlite3_column_int(stmt, col++);
        quest->m_iRequiredSkillNum = sqlite3_column_int(stmt, col++);
        quest->m_iRequiredSkillLevel = sqlite3_column_int(stmt, col++);
        quest->m_iTimeLimit = sqlite3_column_int(stmt, col++);
        quest->m_iAssignType = sqlite3_column_int(stmt, col++);
        quest->m_iRewardType[1] = sqlite3_column_int(stmt, col++);
        quest->m_iRewardAmount[1] = sqlite3_column_int(stmt, col++);
        quest->m_iRewardType[2] = sqlite3_column_int(stmt, col++);
        quest->m_iRewardAmount[2] = sqlite3_column_int(stmt, col++);
        quest->m_iRewardType[3] = sqlite3_column_int(stmt, col++);
        quest->m_iRewardAmount[3] = sqlite3_column_int(stmt, col++);
        quest->m_iContribution = sqlite3_column_int(stmt, col++);
        quest->m_iContributionLimit = sqlite3_column_int(stmt, col++);
        quest->m_iResponseMode = sqlite3_column_int(stmt, col++);
        CopyColumnText(stmt, col++, quest->m_cTargetName, sizeof(quest->m_cTargetName));
        quest->m_sX = sqlite3_column_int(stmt, col++);
        quest->m_sY = sqlite3_column_int(stmt, col++);
        quest->m_iRange = sqlite3_column_int(stmt, col++);
        quest->m_iQuestID = sqlite3_column_int(stmt, col++);
        quest->m_iReqContribution = sqlite3_column_int(stmt, col++);

        game->m_pQuestManager->m_pQuestConfigList[questIndex] = quest;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool SavePortionConfigs(sqlite3* db, const CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    if (!BeginTransaction(db)) {
        return false;
    }

    if (!ClearTable(db, "potion_configs") || !ClearTable(db, "crafting_configs")) {
        RollbackTransaction(db);
        return false;
    }

    const char* potionSql =
        "INSERT INTO potion_configs("
        " potion_id, name, array0, array1, array2, array3, array4, array5, array6,"
        " array7, array8, array9, array10, array11, skill_limit, difficulty"
        ") VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";
    const char* craftingSql =
        "INSERT INTO crafting_configs("
        " crafting_id, name, array0, array1, array2, array3, array4, array5, array6,"
        " array7, array8, array9, array10, array11, skill_limit, difficulty"
        ") VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlite3_stmt* potionStmt = nullptr;
    sqlite3_stmt* craftingStmt = nullptr;
    if (sqlite3_prepare_v2(db, potionSql, -1, &potionStmt, nullptr) != SQLITE_OK ||
        sqlite3_prepare_v2(db, craftingSql, -1, &craftingStmt, nullptr) != SQLITE_OK) {
        if (potionStmt) sqlite3_finalize(potionStmt);
        if (craftingStmt) sqlite3_finalize(craftingStmt);
        RollbackTransaction(db);
        return false;
    }

    for(int i = 0; i < hb::server::config::MaxPortionTypes; i++) {
        if (game->m_pCraftingManager->m_pPortionConfigList[i] != nullptr) {
            const CPortion* potion = game->m_pCraftingManager->m_pPortionConfigList[i];
            sqlite3_reset(potionStmt);
            sqlite3_clear_bindings(potionStmt);
            int col = 1;
            bool ok = true;
            ok &= (sqlite3_bind_int(potionStmt, col++, i) == SQLITE_OK);
            ok &= PrepareAndBindText(potionStmt, col++, potion->m_cName);
            for(int a = 0; a < 12; a++) {
                ok &= (sqlite3_bind_int(potionStmt, col++, potion->m_sArray[a]) == SQLITE_OK);
            }
            ok &= (sqlite3_bind_int(potionStmt, col++, potion->m_iSkillLimit) == SQLITE_OK);
            ok &= (sqlite3_bind_int(potionStmt, col++, potion->m_iDifficulty) == SQLITE_OK);
            if (!ok || sqlite3_step(potionStmt) != SQLITE_DONE) {
                sqlite3_finalize(potionStmt);
                sqlite3_finalize(craftingStmt);
                RollbackTransaction(db);
                return false;
            }
        }

        if (game->m_pCraftingManager->m_pCraftingConfigList[i] != nullptr) {
            const CPortion* crafting = game->m_pCraftingManager->m_pCraftingConfigList[i];
            sqlite3_reset(craftingStmt);
            sqlite3_clear_bindings(craftingStmt);
            int col = 1;
            bool ok = true;
            ok &= (sqlite3_bind_int(craftingStmt, col++, i) == SQLITE_OK);
            ok &= PrepareAndBindText(craftingStmt, col++, crafting->m_cName);
            for(int a = 0; a < 12; a++) {
                ok &= (sqlite3_bind_int(craftingStmt, col++, crafting->m_sArray[a]) == SQLITE_OK);
            }
            ok &= (sqlite3_bind_int(craftingStmt, col++, crafting->m_iSkillLimit) == SQLITE_OK);
            ok &= (sqlite3_bind_int(craftingStmt, col++, crafting->m_iDifficulty) == SQLITE_OK);
            if (!ok || sqlite3_step(craftingStmt) != SQLITE_DONE) {
                sqlite3_finalize(potionStmt);
                sqlite3_finalize(craftingStmt);
                RollbackTransaction(db);
                return false;
            }
        }
    }

    sqlite3_finalize(potionStmt);
    sqlite3_finalize(craftingStmt);
    if (!CommitTransaction(db)) {
        RollbackTransaction(db);
        return false;
    }
    return true;
}

bool LoadPortionConfigs(sqlite3* db, CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    for(int i = 0; i < hb::server::config::MaxPortionTypes; i++) {
        delete game->m_pCraftingManager->m_pPortionConfigList[i];
        game->m_pCraftingManager->m_pPortionConfigList[i] = nullptr;
        delete game->m_pCraftingManager->m_pCraftingConfigList[i];
        game->m_pCraftingManager->m_pCraftingConfigList[i] = nullptr;
    }

    const char* potionSql =
        "SELECT potion_id, name, array0, array1, array2, array3, array4, array5, array6,"
        " array7, array8, array9, array10, array11, skill_limit, difficulty"
        " FROM potion_configs ORDER BY potion_id;";
    const char* craftingSql =
        "SELECT crafting_id, name, array0, array1, array2, array3, array4, array5, array6,"
        " array7, array8, array9, array10, array11, skill_limit, difficulty"
        " FROM crafting_configs ORDER BY crafting_id;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, potionSql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = 0;
        int potionId = sqlite3_column_int(stmt, col++);
        if (potionId < 0 || potionId >= hb::server::config::MaxPortionTypes) {
            continue;
        }
        CPortion* potion = new CPortion();
        CopyColumnText(stmt, col++, potion->m_cName, sizeof(potion->m_cName));
        for(int a = 0; a < 12; a++) {
            potion->m_sArray[a] = (short)sqlite3_column_int(stmt, col++);
        }
        potion->m_iSkillLimit = sqlite3_column_int(stmt, col++);
        potion->m_iDifficulty = sqlite3_column_int(stmt, col++);
        game->m_pCraftingManager->m_pPortionConfigList[potionId] = potion;
    }

    sqlite3_finalize(stmt);
    if (sqlite3_prepare_v2(db, craftingSql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = 0;
        int craftingId = sqlite3_column_int(stmt, col++);
        if (craftingId < 0 || craftingId >= hb::server::config::MaxPortionTypes) {
            continue;
        }
        CPortion* crafting = new CPortion();
        CopyColumnText(stmt, col++, crafting->m_cName, sizeof(crafting->m_cName));
        for(int a = 0; a < 12; a++) {
            crafting->m_sArray[a] = (short)sqlite3_column_int(stmt, col++);
        }
        crafting->m_iSkillLimit = sqlite3_column_int(stmt, col++);
        crafting->m_iDifficulty = sqlite3_column_int(stmt, col++);
        game->m_pCraftingManager->m_pCraftingConfigList[craftingId] = crafting;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool SaveBuildItemConfigs(sqlite3* db, const CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    if (!BeginTransaction(db)) {
        return false;
    }

    if (!ClearTable(db, "builditem_configs")) {
        RollbackTransaction(db);
        return false;
    }

    const char* sql =
        "INSERT INTO builditem_configs("
        " build_id, name, skill_limit, material_id1, material_count1, material_value1,"
        " material_id2, material_count2, material_value2, material_id3, material_count3, material_value3,"
        " material_id4, material_count4, material_value4, material_id5, material_count5, material_value5,"
        " material_id6, material_count6, material_value6, average_value, max_skill, attribute, item_id"
        ") VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        RollbackTransaction(db);
        return false;
    }

    for(int i = 0; i < hb::shared::limits::MaxBuildItems; i++) {
        if (game->m_pBuildItemList[i] == nullptr) {
            continue;
        }

        const CBuildItem* build = game->m_pBuildItemList[i];
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        int col = 1;
        bool ok = true;
        ok &= (sqlite3_bind_int(stmt, col++, i) == SQLITE_OK);
        ok &= PrepareAndBindText(stmt, col++, build->m_cName);
        ok &= (sqlite3_bind_int(stmt, col++, build->m_iSkillLimit) == SQLITE_OK);
        for(int a = 0; a < 6; a++) {
            ok &= (sqlite3_bind_int(stmt, col++, build->m_iMaterialItemID[a]) == SQLITE_OK);
            ok &= (sqlite3_bind_int(stmt, col++, build->m_iMaterialItemCount[a]) == SQLITE_OK);
            ok &= (sqlite3_bind_int(stmt, col++, build->m_iMaterialItemValue[a]) == SQLITE_OK);
        }
        ok &= (sqlite3_bind_int(stmt, col++, build->m_iAverageValue) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, build->m_iMaxSkill) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, build->m_wAttribute) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, build->m_sItemID) == SQLITE_OK);

        if (!ok || sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            RollbackTransaction(db);
            return false;
        }
    }

    sqlite3_finalize(stmt);
    if (!CommitTransaction(db)) {
        RollbackTransaction(db);
        return false;
    }
    return true;
}

bool LoadBuildItemConfigs(sqlite3* db, CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    for(int i = 0; i < hb::shared::limits::MaxBuildItems; i++) {
        delete game->m_pBuildItemList[i];
        game->m_pBuildItemList[i] = nullptr;
    }

    const char* sql =
        "SELECT build_id, name, skill_limit, material_id1, material_count1, material_value1,"
        " material_id2, material_count2, material_value2, material_id3, material_count3, material_value3,"
        " material_id4, material_count4, material_value4, material_id5, material_count5, material_value5,"
        " material_id6, material_count6, material_value6, average_value, max_skill, attribute, item_id"
        " FROM builditem_configs ORDER BY build_id;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = 0;
        int buildId = sqlite3_column_int(stmt, col++);
        if (buildId < 0 || buildId >= hb::shared::limits::MaxBuildItems) {
            continue;
        }

        CBuildItem* build = new CBuildItem();
        CopyColumnText(stmt, col++, build->m_cName, sizeof(build->m_cName));
        build->m_iSkillLimit = sqlite3_column_int(stmt, col++);
        for(int a = 0; a < 6; a++) {
            build->m_iMaterialItemID[a] = sqlite3_column_int(stmt, col++);
            build->m_iMaterialItemCount[a] = sqlite3_column_int(stmt, col++);
            build->m_iMaterialItemValue[a] = sqlite3_column_int(stmt, col++);
        }
        build->m_iAverageValue = sqlite3_column_int(stmt, col++);
        build->m_iMaxSkill = sqlite3_column_int(stmt, col++);
        build->m_wAttribute = (uint16_t)sqlite3_column_int(stmt, col++);
        int storedItemId = sqlite3_column_int(stmt, col++);

        // Use the stored item_id directly instead of looking up by name
        // (names in items table are now display names, not internal names)
        if (storedItemId > 0 && storedItemId < MaxItemTypes && game->m_pItemConfigList[storedItemId] != nullptr) {
            build->m_sItemID = static_cast<short>(storedItemId);
        } else {
            // Fallback to name lookup for backwards compatibility
            CItem tempItem;
            if (game->m_pItemManager->_bInitItemAttr(&tempItem, build->m_cName)) {
                build->m_sItemID = tempItem.m_sIDnum;
            } else {
                char logMsg[256] = {};
                std::snprintf(logMsg, sizeof(logMsg), "(!) Warning: BuildItem '%s' has invalid item_id %d", build->m_cName, storedItemId);
                PutLogList(logMsg);
                delete build;
                continue; // Skip this build item instead of failing completely
            }
        }

        build->m_iMaxValue = 0;
        for(int a = 0; a < 6; a++) {
            build->m_iMaxValue += (build->m_iMaterialItemValue[a] * 100);
        }

        game->m_pBuildItemList[buildId] = build;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool SaveCrusadeConfig(sqlite3* db, const CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    if (!BeginTransaction(db)) {
        return false;
    }

    if (!ClearTable(db, "crusade_structures")) {
        RollbackTransaction(db);
        return false;
    }

    const char* sql =
        "INSERT INTO crusade_structures("
        " structure_id, map_name, structure_type, pos_x, pos_y"
        ") VALUES(?,?,?,?,?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        RollbackTransaction(db);
        return false;
    }

    for(int i = 0; i < hb::shared::limits::MaxCrusadeStructures; i++) {
        const auto& entry = game->m_stCrusadeStructures[i];
        if (entry.cType == 0 || entry.cMapName[0] == 0) {
            continue;
        }
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        bool ok = true;
        ok &= (sqlite3_bind_int(stmt, 1, i) == SQLITE_OK);
        ok &= PrepareAndBindText(stmt, 2, entry.cMapName);
        ok &= (sqlite3_bind_int(stmt, 3, entry.cType) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, 4, entry.dX) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, 5, entry.dY) == SQLITE_OK);
        if (!ok || sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            RollbackTransaction(db);
            return false;
        }
    }

    sqlite3_finalize(stmt);
    if (!CommitTransaction(db)) {
        RollbackTransaction(db);
        return false;
    }
    return true;
}

bool LoadCrusadeConfig(sqlite3* db, CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    for(int i = 0; i < hb::shared::limits::MaxCrusadeStructures; i++) {
        std::memset(game->m_stCrusadeStructures[i].cMapName, 0, sizeof(game->m_stCrusadeStructures[i].cMapName));
        game->m_stCrusadeStructures[i].cType = 0;
        game->m_stCrusadeStructures[i].dX = 0;
        game->m_stCrusadeStructures[i].dY = 0;
    }

    const char* sql =
        "SELECT structure_id, map_name, structure_type, pos_x, pos_y"
        " FROM crusade_structures ORDER BY structure_id;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = 0;
        int structureId = sqlite3_column_int(stmt, col++);
        if (structureId < 0 || structureId >= hb::shared::limits::MaxCrusadeStructures) {
            continue;
        }
        CopyColumnText(stmt, col++, game->m_stCrusadeStructures[structureId].cMapName,
            sizeof(game->m_stCrusadeStructures[structureId].cMapName));
        game->m_stCrusadeStructures[structureId].cType = (char)sqlite3_column_int(stmt, col++);
        game->m_stCrusadeStructures[structureId].dX = sqlite3_column_int(stmt, col++);
        game->m_stCrusadeStructures[structureId].dY = sqlite3_column_int(stmt, col++);
    }

    sqlite3_finalize(stmt);
    return true;
}

bool SaveScheduleConfig(sqlite3* db, const CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    if (!BeginTransaction(db)) {
        return false;
    }

    if (!ClearTable(db, "event_schedule")) {
        RollbackTransaction(db);
        return false;
    }

    sqlite3_stmt* stmt = nullptr;
    const char* sql = "INSERT INTO event_schedule(event_type, schedule_index, day, start_hour, start_minute, end_hour, end_minute, is_active) "
                      "VALUES(?, ?, ?, ?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        RollbackTransaction(db);
        return false;
    }

    bool success = true;

    // Save crusade schedules
    for(int i = 0; i < hb::server::config::MaxSchedule && success; i++) {
        if (game->m_stCrusadeWarSchedule[i].iDay >= 0) {
            sqlite3_reset(stmt);
            sqlite3_clear_bindings(stmt);
            bool ok = true;
            ok &= PrepareAndBindText(stmt, 1, "crusade");
            ok &= (sqlite3_bind_int(stmt, 2, i) == SQLITE_OK);
            ok &= (sqlite3_bind_int(stmt, 3, game->m_stCrusadeWarSchedule[i].iDay) == SQLITE_OK);
            ok &= (sqlite3_bind_int(stmt, 4, game->m_stCrusadeWarSchedule[i].iHour) == SQLITE_OK);
            ok &= (sqlite3_bind_int(stmt, 5, game->m_stCrusadeWarSchedule[i].iMinute) == SQLITE_OK);
            ok &= (sqlite3_bind_null(stmt, 6) == SQLITE_OK);  // end_hour
            ok &= (sqlite3_bind_null(stmt, 7) == SQLITE_OK);  // end_minute
            ok &= (sqlite3_bind_int(stmt, 8, 0) == SQLITE_OK);  // is_active
            success = ok && (sqlite3_step(stmt) == SQLITE_DONE);
        }
    }

    // Save apocalypse schedules (start and end times in one row)
    for(int i = 0; i < hb::server::config::MaxApocalypse && success; i++) {
        if (game->m_stApocalypseScheduleStart[i].iDay >= 0) {
            sqlite3_reset(stmt);
            sqlite3_clear_bindings(stmt);
            bool ok = true;
            ok &= PrepareAndBindText(stmt, 1, "apocalypse");
            ok &= (sqlite3_bind_int(stmt, 2, i) == SQLITE_OK);
            ok &= (sqlite3_bind_int(stmt, 3, game->m_stApocalypseScheduleStart[i].iDay) == SQLITE_OK);
            ok &= (sqlite3_bind_int(stmt, 4, game->m_stApocalypseScheduleStart[i].iHour) == SQLITE_OK);
            ok &= (sqlite3_bind_int(stmt, 5, game->m_stApocalypseScheduleStart[i].iMinute) == SQLITE_OK);
            // End time from the corresponding end schedule
            if (game->m_stApocalypseScheduleEnd[i].iDay >= 0) {
                ok &= (sqlite3_bind_int(stmt, 6, game->m_stApocalypseScheduleEnd[i].iHour) == SQLITE_OK);
                ok &= (sqlite3_bind_int(stmt, 7, game->m_stApocalypseScheduleEnd[i].iMinute) == SQLITE_OK);
            } else {
                ok &= (sqlite3_bind_null(stmt, 6) == SQLITE_OK);
                ok &= (sqlite3_bind_null(stmt, 7) == SQLITE_OK);
            }
            ok &= (sqlite3_bind_int(stmt, 8, 0) == SQLITE_OK);  // is_active
            success = ok && (sqlite3_step(stmt) == SQLITE_DONE);
        }
    }

    // Save heldenian schedules (these have end times)
    for(int i = 0; i < hb::server::config::MaxHeldenian && success; i++) {
        if (game->m_stHeldenianSchedule[i].iDay >= 0) {
            sqlite3_reset(stmt);
            sqlite3_clear_bindings(stmt);
            bool ok = true;
            ok &= PrepareAndBindText(stmt, 1, "heldenian");
            ok &= (sqlite3_bind_int(stmt, 2, i) == SQLITE_OK);
            ok &= (sqlite3_bind_int(stmt, 3, game->m_stHeldenianSchedule[i].iDay) == SQLITE_OK);
            ok &= (sqlite3_bind_int(stmt, 4, game->m_stHeldenianSchedule[i].StartiHour) == SQLITE_OK);
            ok &= (sqlite3_bind_int(stmt, 5, game->m_stHeldenianSchedule[i].StartiMinute) == SQLITE_OK);
            ok &= (sqlite3_bind_int(stmt, 6, game->m_stHeldenianSchedule[i].EndiHour) == SQLITE_OK);
            ok &= (sqlite3_bind_int(stmt, 7, game->m_stHeldenianSchedule[i].EndiMinute) == SQLITE_OK);
            ok &= (sqlite3_bind_int(stmt, 8, 0) == SQLITE_OK);  // is_active
            success = ok && (sqlite3_step(stmt) == SQLITE_DONE);
        }
    }

    sqlite3_finalize(stmt);

    if (!success) {
        RollbackTransaction(db);
        return false;
    }

    if (!CommitTransaction(db)) {
        RollbackTransaction(db);
        return false;
    }
    return true;
}

bool LoadScheduleConfig(sqlite3* db, CGame* game)
{
    if (db == nullptr || game == nullptr) {
        return false;
    }

    // Initialize all schedules to -1
    for(int i = 0; i < hb::server::config::MaxSchedule; i++) {
        game->m_stCrusadeWarSchedule[i].iDay = -1;
        game->m_stCrusadeWarSchedule[i].iHour = -1;
        game->m_stCrusadeWarSchedule[i].iMinute = -1;
    }
    for(int i = 0; i < hb::server::config::MaxApocalypse; i++) {
        game->m_stApocalypseScheduleStart[i].iDay = -1;
        game->m_stApocalypseScheduleStart[i].iHour = -1;
        game->m_stApocalypseScheduleStart[i].iMinute = -1;
        game->m_stApocalypseScheduleEnd[i].iDay = -1;
        game->m_stApocalypseScheduleEnd[i].iHour = -1;
        game->m_stApocalypseScheduleEnd[i].iMinute = -1;
    }
    for(int i = 0; i < hb::server::config::MaxHeldenian; i++) {
        game->m_stHeldenianSchedule[i].iDay = -1;
        game->m_stHeldenianSchedule[i].StartiHour = -1;
        game->m_stHeldenianSchedule[i].StartiMinute = -1;
        game->m_stHeldenianSchedule[i].EndiHour = -1;
        game->m_stHeldenianSchedule[i].EndiMinute = -1;
    }

    // Load all events from unified table
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT event_type, schedule_index, day, start_hour, start_minute, end_hour, end_minute, is_active "
                      "FROM event_schedule ORDER BY event_type, schedule_index;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* eventType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (eventType == nullptr) continue;

        int idx = sqlite3_column_int(stmt, 1);
        int day = sqlite3_column_int(stmt, 2);
        int startHour = sqlite3_column_int(stmt, 3);
        int startMinute = sqlite3_column_int(stmt, 4);
        int endHour = sqlite3_column_type(stmt, 5) != SQLITE_NULL ? sqlite3_column_int(stmt, 5) : -1;
        int endMinute = sqlite3_column_type(stmt, 6) != SQLITE_NULL ? sqlite3_column_int(stmt, 6) : -1;
        // is_active at column 7 - can be used for state tracking

        if (strcmp(eventType, "crusade") == 0) {
            if (idx >= 0 && idx < hb::server::config::MaxSchedule) {
                game->m_stCrusadeWarSchedule[idx].iDay = day;
                game->m_stCrusadeWarSchedule[idx].iHour = startHour;
                game->m_stCrusadeWarSchedule[idx].iMinute = startMinute;
            }
        }
        else if (strcmp(eventType, "apocalypse") == 0) {
            if (idx >= 0 && idx < hb::server::config::MaxApocalypse) {
                // Start time
                game->m_stApocalypseScheduleStart[idx].iDay = day;
                game->m_stApocalypseScheduleStart[idx].iHour = startHour;
                game->m_stApocalypseScheduleStart[idx].iMinute = startMinute;
                // End time (same day as start)
                game->m_stApocalypseScheduleEnd[idx].iDay = day;
                game->m_stApocalypseScheduleEnd[idx].iHour = endHour;
                game->m_stApocalypseScheduleEnd[idx].iMinute = endMinute;
            }
        }
        else if (strcmp(eventType, "heldenian") == 0) {
            if (idx >= 0 && idx < hb::server::config::MaxHeldenian) {
                game->m_stHeldenianSchedule[idx].iDay = day;
                game->m_stHeldenianSchedule[idx].StartiHour = startHour;
                game->m_stHeldenianSchedule[idx].StartiMinute = startMinute;
                game->m_stHeldenianSchedule[idx].EndiHour = endHour;
                game->m_stHeldenianSchedule[idx].EndiMinute = endMinute;
            }
        }
    }
    sqlite3_finalize(stmt);

    return true;
}

bool HasGameConfigRows(sqlite3* db, const char* tableName)
{
    if (db == nullptr || tableName == nullptr || tableName[0] == 0) {
        return false;
    }

    char sql[256] = {};
    std::snprintf(sql, sizeof(sql), "SELECT 1 FROM %s LIMIT 1;", tableName);
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    bool hasRow = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return hasRow;
}

void CloseGameConfigDatabase(sqlite3* db)
{
    if (db != nullptr) {
        sqlite3_close(db);
    }
}
