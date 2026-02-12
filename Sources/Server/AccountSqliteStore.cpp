#include "AccountSqliteStore.h"

#include "Platform.h"
#include <cstdio>
#include <cctype>
#include <cstring>
#include <map>
#include <string>

#include "Client.h"
#include "sqlite3.h"

extern void PutLogList(char* cMsg);

namespace
{
    static void LowercaseInPlace(char* buf, size_t len)
    {
        for (size_t i = 0; i < len && buf[i] != '\0'; i++)
            buf[i] = static_cast<char>(::tolower(static_cast<unsigned char>(buf[i])));
    }

    void FormatTimestamp(const SYSTEMTIME& sysTime, char* outBuffer, size_t outBufferSize)
    {
        std::snprintf(outBuffer, outBufferSize, "%04d-%02d-%02d %02d:%02d:%02d",
            sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
    }

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

    bool PrepareAndBindText(sqlite3_stmt** stmt, int idx, const char* value)
    {
        return sqlite3_bind_text(*stmt, idx, value, -1, SQLITE_TRANSIENT) == SQLITE_OK;
    }

    bool PrepareAndBindText(sqlite3_stmt* stmt, int idx, const char* value)
    {
        return sqlite3_bind_text(stmt, idx, value, -1, SQLITE_TRANSIENT) == SQLITE_OK;
    }

    bool ColumnExists(sqlite3* db, const char* tableName, const char* columnName)
    {
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

    bool AddColumnIfMissing(sqlite3* db, const char* tableName, const char* columnName, const char* columnDef)
    {
        if (ColumnExists(db, tableName, columnName)) {
            return true;
        }

        char sql[512] = {};
        std::snprintf(sql, sizeof(sql), "ALTER TABLE %s ADD COLUMN %s %s;", tableName, columnName, columnDef);
        return ExecSql(db, sql);
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

    // Load item name to ID mapping from GameConfigs.db
    bool LoadItemNameMapping(std::map<std::string, int>& mapping)
{
    sqlite3* configDb = nullptr;
    if (sqlite3_open("GameConfigs.db", &configDb) != SQLITE_OK) {
        sqlite3_close(configDb);
        return false;
    }

    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT item_id, name FROM items";
    if (sqlite3_prepare_v2(configDb, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(configDb);
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int itemId = sqlite3_column_int(stmt, 0);
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (name) {
            mapping[name] = itemId;
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(configDb);
    return !mapping.empty();
}

// Migrate character_items and character_bank_items from item_name to item_id
static bool MigrateItemNamesToIds(sqlite3* db)
{
    // Check if migration is needed (old schema has item_name, new has item_id)
    bool hasItemName = ColumnExists(db, "character_items", "item_name");
    bool hasItemId = ColumnExists(db, "character_items", "item_id");

    if (!hasItemName || hasItemId) {
        // Already migrated or fresh database
        return true;
    }

    char logMsg[256];
    std::snprintf(logMsg, sizeof(logMsg), "(SQLITE) Migrating item storage from names to IDs...");
    PutLogList(logMsg);

    // Load item mapping
    std::map<std::string, int> itemMapping;
    if (!LoadItemNameMapping(itemMapping)) {
        std::snprintf(logMsg, sizeof(logMsg), "(SQLITE) Failed to load item mapping from GameConfigs.db");
        PutLogList(logMsg);
        return false;
    }

    // Begin transaction
    if (!ExecSql(db, "BEGIN TRANSACTION;")) {
        return false;
    }

    // Create new character_items table
    const char* createItemsSql =
        "CREATE TABLE character_items_new ("
        " character_name TEXT NOT NULL,"
        " slot INTEGER NOT NULL,"
        " item_id INTEGER NOT NULL,"
        " count INTEGER NOT NULL,"
        " touch_effect_type INTEGER NOT NULL,"
        " touch_effect_value1 INTEGER NOT NULL,"
        " touch_effect_value2 INTEGER NOT NULL,"
        " touch_effect_value3 INTEGER NOT NULL,"
        " item_color INTEGER NOT NULL,"
        " spec_effect_value1 INTEGER NOT NULL,"
        " spec_effect_value2 INTEGER NOT NULL,"
        " spec_effect_value3 INTEGER NOT NULL,"
        " cur_lifespan INTEGER NOT NULL,"
        " attribute INTEGER NOT NULL,"
        " pos_x INTEGER NOT NULL,"
        " pos_y INTEGER NOT NULL,"
        " is_equipped INTEGER NOT NULL,"
        " PRIMARY KEY(character_name, slot),"
        " FOREIGN KEY(character_name) REFERENCES characters(character_name) ON DELETE CASCADE"
        ");";

    if (!ExecSql(db, createItemsSql)) {
        ExecSql(db, "ROLLBACK;");
        return false;
    }

    // Copy and convert character_items
    sqlite3_stmt* readStmt = nullptr;
    const char* readSql =
        "SELECT character_name, slot, item_name, count, touch_effect_type,"
        " touch_effect_value1, touch_effect_value2, touch_effect_value3,"
        " item_color, spec_effect_value1, spec_effect_value2, spec_effect_value3,"
        " cur_lifespan, attribute, pos_x, pos_y, is_equipped FROM character_items";

    if (sqlite3_prepare_v2(db, readSql, -1, &readStmt, nullptr) != SQLITE_OK) {
        ExecSql(db, "ROLLBACK;");
        return false;
    }

    sqlite3_stmt* writeStmt = nullptr;
    const char* writeSql =
        "INSERT INTO character_items_new VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";

    if (sqlite3_prepare_v2(db, writeSql, -1, &writeStmt, nullptr) != SQLITE_OK) {
        sqlite3_finalize(readStmt);
        ExecSql(db, "ROLLBACK;");
        return false;
    }

    int migratedItems = 0;
    int skippedItems = 0;

    while (sqlite3_step(readStmt) == SQLITE_ROW) {
        const char* charName = reinterpret_cast<const char*>(sqlite3_column_text(readStmt, 0));
        int slot = sqlite3_column_int(readStmt, 1);
        const char* itemName = reinterpret_cast<const char*>(sqlite3_column_text(readStmt, 2));

        if (!itemName || itemMapping.find(itemName) == itemMapping.end()) {
            skippedItems++;
            continue;
        }

        int itemId = itemMapping[itemName];

        sqlite3_reset(writeStmt);
        sqlite3_clear_bindings(writeStmt);

        int col = 1;
        sqlite3_bind_text(writeStmt, col++, charName, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(writeStmt, col++, slot);
        sqlite3_bind_int(writeStmt, col++, itemId);
        for(int i = 3; i <= 16; i++) {
            sqlite3_bind_int(writeStmt, col++, sqlite3_column_int(readStmt, i));
        }

        if (sqlite3_step(writeStmt) != SQLITE_DONE) {
            skippedItems++;
        } else {
            migratedItems++;
        }
    }

    sqlite3_finalize(readStmt);
    sqlite3_finalize(writeStmt);

    // Create new character_bank_items table
    const char* createBankSql =
        "CREATE TABLE character_bank_items_new ("
        " character_name TEXT NOT NULL,"
        " slot INTEGER NOT NULL,"
        " item_id INTEGER NOT NULL,"
        " count INTEGER NOT NULL,"
        " touch_effect_type INTEGER NOT NULL,"
        " touch_effect_value1 INTEGER NOT NULL,"
        " touch_effect_value2 INTEGER NOT NULL,"
        " touch_effect_value3 INTEGER NOT NULL,"
        " item_color INTEGER NOT NULL,"
        " spec_effect_value1 INTEGER NOT NULL,"
        " spec_effect_value2 INTEGER NOT NULL,"
        " spec_effect_value3 INTEGER NOT NULL,"
        " cur_lifespan INTEGER NOT NULL,"
        " attribute INTEGER NOT NULL,"
        " PRIMARY KEY(character_name, slot),"
        " FOREIGN KEY(character_name) REFERENCES characters(character_name) ON DELETE CASCADE"
        ");";

    if (!ExecSql(db, createBankSql)) {
        ExecSql(db, "ROLLBACK;");
        return false;
    }

    // Copy and convert character_bank_items
    readSql =
        "SELECT character_name, slot, item_name, count, touch_effect_type,"
        " touch_effect_value1, touch_effect_value2, touch_effect_value3,"
        " item_color, spec_effect_value1, spec_effect_value2, spec_effect_value3,"
        " cur_lifespan, attribute FROM character_bank_items";

    if (sqlite3_prepare_v2(db, readSql, -1, &readStmt, nullptr) != SQLITE_OK) {
        ExecSql(db, "ROLLBACK;");
        return false;
    }

    writeSql = "INSERT INTO character_bank_items_new VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?)";

    if (sqlite3_prepare_v2(db, writeSql, -1, &writeStmt, nullptr) != SQLITE_OK) {
        sqlite3_finalize(readStmt);
        ExecSql(db, "ROLLBACK;");
        return false;
    }

    int migratedBankItems = 0;
    int skippedBankItems = 0;

    while (sqlite3_step(readStmt) == SQLITE_ROW) {
        const char* charName = reinterpret_cast<const char*>(sqlite3_column_text(readStmt, 0));
        int slot = sqlite3_column_int(readStmt, 1);
        const char* itemName = reinterpret_cast<const char*>(sqlite3_column_text(readStmt, 2));

        if (!itemName || itemMapping.find(itemName) == itemMapping.end()) {
            skippedBankItems++;
            continue;
        }

        int itemId = itemMapping[itemName];

        sqlite3_reset(writeStmt);
        sqlite3_clear_bindings(writeStmt);

        int col = 1;
        sqlite3_bind_text(writeStmt, col++, charName, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(writeStmt, col++, slot);
        sqlite3_bind_int(writeStmt, col++, itemId);
        for(int i = 3; i <= 13; i++) {
            sqlite3_bind_int(writeStmt, col++, sqlite3_column_int(readStmt, i));
        }

        if (sqlite3_step(writeStmt) != SQLITE_DONE) {
            skippedBankItems++;
        } else {
            migratedBankItems++;
        }
    }

    sqlite3_finalize(readStmt);
    sqlite3_finalize(writeStmt);

    // Drop old tables and rename new ones
    if (!ExecSql(db, "DROP TABLE character_items;") ||
        !ExecSql(db, "DROP TABLE character_bank_items;") ||
        !ExecSql(db, "ALTER TABLE character_items_new RENAME TO character_items;") ||
        !ExecSql(db, "ALTER TABLE character_bank_items_new RENAME TO character_bank_items;")) {
        ExecSql(db, "ROLLBACK;");
        return false;
    }

    // Commit transaction
    if (!ExecSql(db, "COMMIT;")) {
        return false;
    }

    std::snprintf(logMsg, sizeof(logMsg),
        "(SQLITE) Migration complete: %d items, %d bank items migrated (%d/%d skipped)",
        migratedItems, migratedBankItems, skippedItems, skippedBankItems);
    PutLogList(logMsg);

    return true;
}

} // end anonymous namespace

bool EnsureAccountDatabase(const char* accountName, sqlite3** outDb, std::string& outPath)
{
    if (outDb == nullptr || accountName == nullptr || accountName[0] == 0) {
        return false;
    }

    _mkdir("Accounts");

    char lowerName[64] = {};
    std::strncpy(lowerName, accountName, sizeof(lowerName) - 1);
    LowercaseInPlace(lowerName, sizeof(lowerName));
    char dbPath[MAX_PATH] = {};
    std::snprintf(dbPath, sizeof(dbPath), "Accounts/%s.db", lowerName);
    outPath = dbPath;

    sqlite3* db = nullptr;
    int rc = sqlite3_open(dbPath, &db);
    if (rc != SQLITE_OK) {
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
        "INSERT OR REPLACE INTO meta(key, value) VALUES('schema_version','6');"
        "CREATE TABLE IF NOT EXISTS accounts ("
        " account_name TEXT PRIMARY KEY,"
        " password_hash TEXT NOT NULL,"
        " password_salt TEXT NOT NULL,"
        " email TEXT NOT NULL,"
        " created_at TEXT NOT NULL,"
        " password_changed_at TEXT NOT NULL,"
        " last_ip TEXT NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS characters ("
        " character_name TEXT PRIMARY KEY,"
        " account_name TEXT NOT NULL,"
        " created_at TEXT NOT NULL,"
        " underwear_type INTEGER NOT NULL DEFAULT 0,"
        " hair_color INTEGER NOT NULL DEFAULT 0,"
        " hair_style INTEGER NOT NULL DEFAULT 0,"
        " skin_color INTEGER NOT NULL DEFAULT 0,"
        " level INTEGER NOT NULL,"
        " exp INTEGER NOT NULL,"
        " map_name TEXT NOT NULL,"
        " map_x INTEGER NOT NULL,"
        " map_y INTEGER NOT NULL,"
        " hp INTEGER NOT NULL,"
        " mp INTEGER NOT NULL,"
        " sp INTEGER NOT NULL,"
        " str INTEGER NOT NULL,"
        " vit INTEGER NOT NULL,"
        " dex INTEGER NOT NULL,"
        " intl INTEGER NOT NULL,"
        " mag INTEGER NOT NULL,"
        " chr INTEGER NOT NULL,"
        " gender INTEGER NOT NULL,"
        " skin INTEGER NOT NULL,"
        " hairstyle INTEGER NOT NULL,"
        " haircolor INTEGER NOT NULL,"
        " underwear INTEGER NOT NULL,"
        " FOREIGN KEY(account_name) REFERENCES accounts(account_name) ON DELETE CASCADE"
        ");"
        "CREATE TABLE IF NOT EXISTS character_items ("
        " character_name TEXT NOT NULL,"
        " slot INTEGER NOT NULL,"
        " item_id INTEGER NOT NULL,"
        " count INTEGER NOT NULL,"
        " touch_effect_type INTEGER NOT NULL,"
        " touch_effect_value1 INTEGER NOT NULL,"
        " touch_effect_value2 INTEGER NOT NULL,"
        " touch_effect_value3 INTEGER NOT NULL,"
        " item_color INTEGER NOT NULL,"
        " spec_effect_value1 INTEGER NOT NULL,"
        " spec_effect_value2 INTEGER NOT NULL,"
        " spec_effect_value3 INTEGER NOT NULL,"
        " cur_lifespan INTEGER NOT NULL,"
        " attribute INTEGER NOT NULL,"
        " pos_x INTEGER NOT NULL,"
        " pos_y INTEGER NOT NULL,"
        " is_equipped INTEGER NOT NULL,"
        " PRIMARY KEY(character_name, slot),"
        " FOREIGN KEY(character_name) REFERENCES characters(character_name) ON DELETE CASCADE"
        ");"
        "CREATE TABLE IF NOT EXISTS character_bank_items ("
        " character_name TEXT NOT NULL,"
        " slot INTEGER NOT NULL,"
        " item_id INTEGER NOT NULL,"
        " count INTEGER NOT NULL,"
        " touch_effect_type INTEGER NOT NULL,"
        " touch_effect_value1 INTEGER NOT NULL,"
        " touch_effect_value2 INTEGER NOT NULL,"
        " touch_effect_value3 INTEGER NOT NULL,"
        " item_color INTEGER NOT NULL,"
        " spec_effect_value1 INTEGER NOT NULL,"
        " spec_effect_value2 INTEGER NOT NULL,"
        " spec_effect_value3 INTEGER NOT NULL,"
        " cur_lifespan INTEGER NOT NULL,"
        " attribute INTEGER NOT NULL,"
        " PRIMARY KEY(character_name, slot),"
        " FOREIGN KEY(character_name) REFERENCES characters(character_name) ON DELETE CASCADE"
        ");"
        "CREATE TABLE IF NOT EXISTS character_item_positions ("
        " character_name TEXT NOT NULL,"
        " slot INTEGER NOT NULL,"
        " pos_x INTEGER NOT NULL,"
        " pos_y INTEGER NOT NULL,"
        " PRIMARY KEY(character_name, slot),"
        " FOREIGN KEY(character_name) REFERENCES characters(character_name) ON DELETE CASCADE"
        ");"
        "CREATE TABLE IF NOT EXISTS character_item_equips ("
        " character_name TEXT NOT NULL,"
        " slot INTEGER NOT NULL,"
        " is_equipped INTEGER NOT NULL,"
        " PRIMARY KEY(character_name, slot),"
        " FOREIGN KEY(character_name) REFERENCES characters(character_name) ON DELETE CASCADE"
        ");"
        "CREATE TABLE IF NOT EXISTS character_magic_mastery ("
        " character_name TEXT NOT NULL,"
        " magic_index INTEGER NOT NULL,"
        " mastery_value INTEGER NOT NULL,"
        " PRIMARY KEY(character_name, magic_index),"
        " FOREIGN KEY(character_name) REFERENCES characters(character_name) ON DELETE CASCADE"
        ");"
        "CREATE TABLE IF NOT EXISTS character_skill_mastery ("
        " character_name TEXT NOT NULL,"
        " skill_index INTEGER NOT NULL,"
        " mastery_value INTEGER NOT NULL,"
        " PRIMARY KEY(character_name, skill_index),"
        " FOREIGN KEY(character_name) REFERENCES characters(character_name) ON DELETE CASCADE"
        ");"
        "CREATE TABLE IF NOT EXISTS character_skill_ssn ("
        " character_name TEXT NOT NULL,"
        " skill_index INTEGER NOT NULL,"
        " ssn_value INTEGER NOT NULL,"
        " PRIMARY KEY(character_name, skill_index),"
        " FOREIGN KEY(character_name) REFERENCES characters(character_name) ON DELETE CASCADE"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_characters_account ON characters(account_name);"
        "CREATE TABLE IF NOT EXISTS block_list ("
        " blocked_account_name TEXT NOT NULL,"
        " blocked_character_name TEXT NOT NULL,"
        " PRIMARY KEY(blocked_account_name)"
        ");"
        "COMMIT;";

    if (!ExecSql(db, schemaSql)) {
        sqlite3_close(db);
        return false;
    }

    if (!AddColumnIfMissing(db, "characters", "profile", "TEXT NOT NULL DEFAULT ''") ||
        !AddColumnIfMissing(db, "characters", "location", "TEXT NOT NULL DEFAULT ''") ||
        !AddColumnIfMissing(db, "characters", "guild_name", "TEXT NOT NULL DEFAULT ''") ||
        !AddColumnIfMissing(db, "characters", "guild_guid", "INTEGER NOT NULL DEFAULT -1") ||
        !AddColumnIfMissing(db, "characters", "guild_rank", "INTEGER NOT NULL DEFAULT -1") ||
        !AddColumnIfMissing(db, "characters", "rating", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "luck", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "lu_pool", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "enemy_kill_count", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "pk_count", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "reward_gold", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "downskill_index", "INTEGER NOT NULL DEFAULT -1") ||
        !AddColumnIfMissing(db, "characters", "idnum1", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "idnum2", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "idnum3", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "hunger_status", "INTEGER NOT NULL DEFAULT 100") ||
        !AddColumnIfMissing(db, "characters", "timeleft_rating", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "timeleft_force_recall", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "timeleft_firm_staminar", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "penalty_block_year", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "penalty_block_month", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "penalty_block_day", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "quest_number", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "quest_id", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "current_quest_count", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "quest_reward_type", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "quest_reward_amount", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "contribution", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "war_contribution", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "quest_completed", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "special_event_id", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "super_attack_left", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "fightzone_number", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "reserve_time", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "fightzone_ticket_number", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "special_ability_time", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "locked_map_name", "TEXT NOT NULL DEFAULT ''") ||
        !AddColumnIfMissing(db, "characters", "locked_map_time", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "crusade_job", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "crusade_guid", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "construct_point", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "dead_penalty_time", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "party_id", "INTEGER NOT NULL DEFAULT 0") ||
        !AddColumnIfMissing(db, "characters", "gizon_item_upgrade_left", "INTEGER NOT NULL DEFAULT 0")) {
        sqlite3_close(db);
        return false;
    }

    // Migrate old item_name schema to item_id if needed
    if (!MigrateItemNamesToIds(db)) {
        sqlite3_close(db);
        return false;
    }

    // Migrate account_name to lowercase (accounts created before lowercase enforcement)
    // Must disable FK checks temporarily since accounts is referenced by characters
    ExecSql(db, "PRAGMA foreign_keys = OFF;");
    ExecSql(db, "UPDATE accounts SET account_name = LOWER(account_name) WHERE account_name != LOWER(account_name);");
    ExecSql(db, "UPDATE characters SET account_name = LOWER(account_name) WHERE account_name != LOWER(account_name);");
    ExecSql(db, "PRAGMA foreign_keys = ON;");

    *outDb = db;
    return true;
}

bool LoadAccountRecord(sqlite3* db, const char* accountName, AccountDbAccountData& outData)
{
    if (db == nullptr || accountName == nullptr) {
        return false;
    }

    const char* sql =
        "SELECT account_name, password_hash, password_salt, email, created_at, password_changed_at, last_ip "
        "FROM accounts WHERE account_name = ? COLLATE NOCASE;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    PrepareAndBindText(&stmt, 1, accountName);
    bool ok = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::memset(&outData, 0, sizeof(outData));
        CopyColumnText(stmt, 0, outData.name, sizeof(outData.name));
        CopyColumnText(stmt, 1, outData.password_hash, sizeof(outData.password_hash));
        CopyColumnText(stmt, 2, outData.password_salt, sizeof(outData.password_salt));
        CopyColumnText(stmt, 3, outData.email, sizeof(outData.email));
        CopyColumnText(stmt, 4, outData.createdAt, sizeof(outData.createdAt));
        CopyColumnText(stmt, 5, outData.passwordChangedAt, sizeof(outData.passwordChangedAt));
        CopyColumnText(stmt, 6, outData.lastIp, sizeof(outData.lastIp));
        ok = true;
    }

    sqlite3_finalize(stmt);
    return ok;
}

bool UpdateAccountPassword(sqlite3* db, const char* accountName, const char* passwordHash, const char* passwordSalt)
{
    if (db == nullptr || accountName == nullptr || passwordHash == nullptr || passwordSalt == nullptr) {
        return false;
    }

    SYSTEMTIME sysTime;
    GetLocalTime(&sysTime);
    char timestamp[32] = {};
    FormatTimestamp(sysTime, timestamp, sizeof(timestamp));

    const char* sql =
        "UPDATE accounts SET password_hash = ?, password_salt = ?, password_changed_at = ? WHERE account_name = ? COLLATE NOCASE;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    bool ok = true;
    ok &= PrepareAndBindText(&stmt, 1, passwordHash);
    ok &= PrepareAndBindText(&stmt, 2, passwordSalt);
    ok &= PrepareAndBindText(&stmt, 3, timestamp);
    ok &= PrepareAndBindText(&stmt, 4, accountName);

    if (ok) {
        ok = sqlite3_step(stmt) == SQLITE_DONE;
    }

    sqlite3_finalize(stmt);
    return ok;
}

bool ListCharacterSummaries(sqlite3* db, const char* accountName, std::vector<AccountDbCharacterSummary>& outChars)
{
    if (db == nullptr || accountName == nullptr) {
        return false;
    }

    const char* sql =
        "SELECT character_name, underwear_type, hair_color, hair_style, skin_color, gender, skin, level, exp, map_name "
        "FROM characters WHERE account_name = ? COLLATE NOCASE ORDER BY character_name;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    PrepareAndBindText(&stmt, 1, accountName);
    outChars.clear();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        AccountDbCharacterSummary row = {};
        CopyColumnText(stmt, 0, row.characterName, sizeof(row.characterName));
        row.appearance.iUnderwearType = static_cast<uint8_t>(sqlite3_column_int(stmt, 1));
        row.appearance.iHairColor = static_cast<uint8_t>(sqlite3_column_int(stmt, 2));
        row.appearance.iHairStyle = static_cast<uint8_t>(sqlite3_column_int(stmt, 3));
        row.appearance.iSkinColor = static_cast<uint8_t>(sqlite3_column_int(stmt, 4));
        row.sex = static_cast<uint16_t>(sqlite3_column_int(stmt, 5));
        row.skin = static_cast<uint16_t>(sqlite3_column_int(stmt, 6));
        row.level = static_cast<uint16_t>(sqlite3_column_int(stmt, 7));
        row.exp = static_cast<uint32_t>(sqlite3_column_int(stmt, 8));
        CopyColumnText(stmt, 9, row.mapName, sizeof(row.mapName));
        outChars.push_back(row);
    }

    sqlite3_finalize(stmt);
    return true;
}

bool LoadCharacterState(sqlite3* db, const char* characterName, AccountDbCharacterState& outState)
{
    if (db == nullptr || characterName == nullptr) {
        return false;
    }

    const char* sql =
        "SELECT account_name, character_name, profile, location, guild_name, guild_guid, guild_rank, "
        "map_name, map_x, map_y, hp, mp, sp, level, rating, str, intl, vit, dex, mag, chr, luck, exp, "
        "lu_pool, enemy_kill_count, pk_count, reward_gold, downskill_index, idnum1, idnum2, idnum3, "
        "gender, skin, hairstyle, haircolor, underwear, hunger_status, timeleft_rating, "
        "timeleft_force_recall, timeleft_firm_staminar, penalty_block_year, "
        "penalty_block_month, penalty_block_day, quest_number, quest_id, current_quest_count, "
        "quest_reward_type, quest_reward_amount, contribution, war_contribution, quest_completed, "
        "special_event_id, super_attack_left, fightzone_number, reserve_time, fightzone_ticket_number, "
        "special_ability_time, locked_map_name, locked_map_time, crusade_job, crusade_guid, "
        "construct_point, dead_penalty_time, party_id, gizon_item_upgrade_left, "
        "underwear_type, hair_color, hair_style, skin_color "
        "FROM characters WHERE character_name = ? COLLATE NOCASE;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    PrepareAndBindText(&stmt, 1, characterName);
    bool ok = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::memset(&outState, 0, sizeof(outState));
        int col = 0;
        CopyColumnText(stmt, col++, outState.accountName, sizeof(outState.accountName));
        CopyColumnText(stmt, col++, outState.characterName, sizeof(outState.characterName));
        CopyColumnText(stmt, col++, outState.profile, sizeof(outState.profile));
        CopyColumnText(stmt, col++, outState.location, sizeof(outState.location));
        CopyColumnText(stmt, col++, outState.guildName, sizeof(outState.guildName));
        outState.guildGuid = sqlite3_column_int(stmt, col++);
        outState.guildRank = sqlite3_column_int(stmt, col++);
        CopyColumnText(stmt, col++, outState.mapName, sizeof(outState.mapName));
        outState.mapX = sqlite3_column_int(stmt, col++);
        outState.mapY = sqlite3_column_int(stmt, col++);
        outState.hp = sqlite3_column_int(stmt, col++);
        outState.mp = sqlite3_column_int(stmt, col++);
        outState.sp = sqlite3_column_int(stmt, col++);
        outState.level = sqlite3_column_int(stmt, col++);
        outState.rating = sqlite3_column_int(stmt, col++);
        outState.str = sqlite3_column_int(stmt, col++);
        outState.intl = sqlite3_column_int(stmt, col++);
        outState.vit = sqlite3_column_int(stmt, col++);
        outState.dex = sqlite3_column_int(stmt, col++);
        outState.mag = sqlite3_column_int(stmt, col++);
        outState.chr = sqlite3_column_int(stmt, col++);
        outState.luck = sqlite3_column_int(stmt, col++);
        outState.exp = static_cast<uint32_t>(sqlite3_column_int(stmt, col++));
        outState.luPool = sqlite3_column_int(stmt, col++);
        outState.enemyKillCount = sqlite3_column_int(stmt, col++);
        outState.pkCount = sqlite3_column_int(stmt, col++);
        outState.rewardGold = static_cast<uint32_t>(sqlite3_column_int(stmt, col++));
        outState.downSkillIndex = sqlite3_column_int(stmt, col++);
        outState.idnum1 = sqlite3_column_int(stmt, col++);
        outState.idnum2 = sqlite3_column_int(stmt, col++);
        outState.idnum3 = sqlite3_column_int(stmt, col++);
        outState.sex = sqlite3_column_int(stmt, col++);
        outState.skin = sqlite3_column_int(stmt, col++);
        outState.hairStyle = sqlite3_column_int(stmt, col++);
        outState.hairColor = sqlite3_column_int(stmt, col++);
        outState.underwear = sqlite3_column_int(stmt, col++);
        outState.hungerStatus = sqlite3_column_int(stmt, col++);
        outState.timeleftRating = sqlite3_column_int(stmt, col++);
        outState.timeleftForceRecall = sqlite3_column_int(stmt, col++);
        outState.timeleftFirmStaminar = sqlite3_column_int(stmt, col++);
        outState.penaltyBlockYear = sqlite3_column_int(stmt, col++);
        outState.penaltyBlockMonth = sqlite3_column_int(stmt, col++);
        outState.penaltyBlockDay = sqlite3_column_int(stmt, col++);
        outState.questNumber = sqlite3_column_int(stmt, col++);
        outState.questId = sqlite3_column_int(stmt, col++);
        outState.currentQuestCount = sqlite3_column_int(stmt, col++);
        outState.questRewardType = sqlite3_column_int(stmt, col++);
        outState.questRewardAmount = sqlite3_column_int(stmt, col++);
        outState.contribution = sqlite3_column_int(stmt, col++);
        outState.warContribution = sqlite3_column_int(stmt, col++);
        outState.questCompleted = sqlite3_column_int(stmt, col++);
        outState.specialEventId = sqlite3_column_int(stmt, col++);
        outState.superAttackLeft = sqlite3_column_int(stmt, col++);
        outState.fightzoneNumber = sqlite3_column_int(stmt, col++);
        outState.reserveTime = sqlite3_column_int(stmt, col++);
        outState.fightzoneTicketNumber = sqlite3_column_int(stmt, col++);
        outState.specialAbilityTime = sqlite3_column_int(stmt, col++);
        CopyColumnText(stmt, col++, outState.lockedMapName, sizeof(outState.lockedMapName));
        outState.lockedMapTime = sqlite3_column_int(stmt, col++);
        outState.crusadeJob = sqlite3_column_int(stmt, col++);
        outState.crusadeGuid = static_cast<uint32_t>(sqlite3_column_int(stmt, col++));
        outState.constructPoint = sqlite3_column_int(stmt, col++);
        outState.deadPenaltyTime = sqlite3_column_int(stmt, col++);
        outState.partyId = sqlite3_column_int(stmt, col++);
        outState.gizonItemUpgradeLeft = sqlite3_column_int(stmt, col++);
        outState.appearance.iUnderwearType = static_cast<uint8_t>(sqlite3_column_int(stmt, col++));
        outState.appearance.iHairColor = static_cast<uint8_t>(sqlite3_column_int(stmt, col++));
        outState.appearance.iHairStyle = static_cast<uint8_t>(sqlite3_column_int(stmt, col++));
        outState.appearance.iSkinColor = static_cast<uint8_t>(sqlite3_column_int(stmt, col++));
        ok = true;
    }

    sqlite3_finalize(stmt);
    return ok;
}

bool LoadCharacterItems(sqlite3* db, const char* characterName, std::vector<AccountDbItemRow>& outItems)
{
    if (db == nullptr || characterName == nullptr) {
        return false;
    }

    const char* sql =
        "SELECT slot, item_id, count, touch_effect_type, touch_effect_value1, touch_effect_value2, "
        "touch_effect_value3, item_color, spec_effect_value1, spec_effect_value2, spec_effect_value3, "
        "cur_lifespan, attribute, pos_x, pos_y, is_equipped "
        "FROM character_items WHERE character_name = ? COLLATE NOCASE ORDER BY slot;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    PrepareAndBindText(&stmt, 1, characterName);
    outItems.clear();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        AccountDbItemRow row = {};
        int col = 0;
        row.slot = sqlite3_column_int(stmt, col++);
        row.itemId = sqlite3_column_int(stmt, col++);
        row.count = sqlite3_column_int(stmt, col++);
        row.touchEffectType = sqlite3_column_int(stmt, col++);
        row.touchEffectValue1 = sqlite3_column_int(stmt, col++);
        row.touchEffectValue2 = sqlite3_column_int(stmt, col++);
        row.touchEffectValue3 = sqlite3_column_int(stmt, col++);
        row.itemColor = sqlite3_column_int(stmt, col++);
        row.specEffectValue1 = sqlite3_column_int(stmt, col++);
        row.specEffectValue2 = sqlite3_column_int(stmt, col++);
        row.specEffectValue3 = sqlite3_column_int(stmt, col++);
        row.curLifeSpan = sqlite3_column_int(stmt, col++);
        row.attribute = static_cast<uint32_t>(sqlite3_column_int(stmt, col++));
        row.posX = sqlite3_column_int(stmt, col++);
        row.posY = sqlite3_column_int(stmt, col++);
        row.isEquipped = sqlite3_column_int(stmt, col++);
        outItems.push_back(row);
    }

    sqlite3_finalize(stmt);
    return true;
}

bool LoadCharacterBankItems(sqlite3* db, const char* characterName, std::vector<AccountDbBankItemRow>& outItems)
{
    if (db == nullptr || characterName == nullptr) {
        return false;
    }

    const char* sql =
        "SELECT slot, item_id, count, touch_effect_type, touch_effect_value1, touch_effect_value2, "
        "touch_effect_value3, item_color, spec_effect_value1, spec_effect_value2, spec_effect_value3, "
        "cur_lifespan, attribute "
        "FROM character_bank_items WHERE character_name = ? COLLATE NOCASE ORDER BY slot;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    PrepareAndBindText(&stmt, 1, characterName);
    outItems.clear();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        AccountDbBankItemRow row = {};
        int col = 0;
        row.slot = sqlite3_column_int(stmt, col++);
        row.itemId = sqlite3_column_int(stmt, col++);
        row.count = sqlite3_column_int(stmt, col++);
        row.touchEffectType = sqlite3_column_int(stmt, col++);
        row.touchEffectValue1 = sqlite3_column_int(stmt, col++);
        row.touchEffectValue2 = sqlite3_column_int(stmt, col++);
        row.touchEffectValue3 = sqlite3_column_int(stmt, col++);
        row.itemColor = sqlite3_column_int(stmt, col++);
        row.specEffectValue1 = sqlite3_column_int(stmt, col++);
        row.specEffectValue2 = sqlite3_column_int(stmt, col++);
        row.specEffectValue3 = sqlite3_column_int(stmt, col++);
        row.curLifeSpan = sqlite3_column_int(stmt, col++);
        row.attribute = static_cast<uint32_t>(sqlite3_column_int(stmt, col++));
        outItems.push_back(row);
    }

    sqlite3_finalize(stmt);
    return true;
}

bool LoadCharacterItemPositions(sqlite3* db, const char* characterName, std::vector<AccountDbIndexedValue>& outPositionsX, std::vector<AccountDbIndexedValue>& outPositionsY)
{
    if (db == nullptr || characterName == nullptr) {
        return false;
    }

    const char* sql =
        "SELECT slot, pos_x, pos_y FROM character_item_positions WHERE character_name = ? COLLATE NOCASE ORDER BY slot;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    PrepareAndBindText(&stmt, 1, characterName);
    outPositionsX.clear();
    outPositionsY.clear();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        AccountDbIndexedValue posX = {};
        AccountDbIndexedValue posY = {};
        posX.index = sqlite3_column_int(stmt, 0);
        posX.value = sqlite3_column_int(stmt, 1);
        posY.index = posX.index;
        posY.value = sqlite3_column_int(stmt, 2);
        outPositionsX.push_back(posX);
        outPositionsY.push_back(posY);
    }

    sqlite3_finalize(stmt);
    return true;
}

bool LoadCharacterItemEquips(sqlite3* db, const char* characterName, std::vector<AccountDbIndexedValue>& outEquips)
{
    if (db == nullptr || characterName == nullptr) {
        return false;
    }

    const char* sql =
        "SELECT slot, is_equipped FROM character_item_equips WHERE character_name = ? COLLATE NOCASE ORDER BY slot;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    PrepareAndBindText(&stmt, 1, characterName);
    outEquips.clear();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        AccountDbIndexedValue equip = {};
        equip.index = sqlite3_column_int(stmt, 0);
        equip.value = sqlite3_column_int(stmt, 1);
        outEquips.push_back(equip);
    }

    sqlite3_finalize(stmt);
    return true;
}

bool LoadCharacterMagicMastery(sqlite3* db, const char* characterName, std::vector<AccountDbIndexedValue>& outMastery)
{
    if (db == nullptr || characterName == nullptr) {
        return false;
    }

    const char* sql =
        "SELECT magic_index, mastery_value FROM character_magic_mastery WHERE character_name = ? COLLATE NOCASE ORDER BY magic_index;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    PrepareAndBindText(&stmt, 1, characterName);
    outMastery.clear();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        AccountDbIndexedValue row = {};
        row.index = sqlite3_column_int(stmt, 0);
        row.value = sqlite3_column_int(stmt, 1);
        outMastery.push_back(row);
    }

    sqlite3_finalize(stmt);
    return true;
}

bool LoadCharacterSkillMastery(sqlite3* db, const char* characterName, std::vector<AccountDbIndexedValue>& outMastery)
{
    if (db == nullptr || characterName == nullptr) {
        return false;
    }

    const char* sql =
        "SELECT skill_index, mastery_value FROM character_skill_mastery WHERE character_name = ? COLLATE NOCASE ORDER BY skill_index;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    PrepareAndBindText(&stmt, 1, characterName);
    outMastery.clear();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        AccountDbIndexedValue row = {};
        row.index = sqlite3_column_int(stmt, 0);
        row.value = sqlite3_column_int(stmt, 1);
        outMastery.push_back(row);
    }

    sqlite3_finalize(stmt);
    return true;
}

bool LoadCharacterSkillSSN(sqlite3* db, const char* characterName, std::vector<AccountDbIndexedValue>& outValues)
{
    if (db == nullptr || characterName == nullptr) {
        return false;
    }

    const char* sql =
        "SELECT skill_index, ssn_value FROM character_skill_ssn WHERE character_name = ? COLLATE NOCASE ORDER BY skill_index;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    PrepareAndBindText(&stmt, 1, characterName);
    outValues.clear();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        AccountDbIndexedValue row = {};
        row.index = sqlite3_column_int(stmt, 0);
        row.value = sqlite3_column_int(stmt, 1);
        outValues.push_back(row);
    }

    sqlite3_finalize(stmt);
    return true;
}

bool LoadEquippedItemAppearances(sqlite3* db, const char* characterName, std::vector<AccountDbEquippedItem>& outItems)
{
    if (db == nullptr || characterName == nullptr) {
        return false;
    }

    const char* sql = "SELECT item_id, item_color FROM character_items WHERE character_name = ? COLLATE NOCASE AND is_equipped = 1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    PrepareAndBindText(&stmt, 1, characterName);
    outItems.clear();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        AccountDbEquippedItem item = {};
        item.itemId = sqlite3_column_int(stmt, 0);
        item.itemColor = sqlite3_column_int(stmt, 1);
        outItems.push_back(item);
    }

    sqlite3_finalize(stmt);
    return true;
}

bool InsertCharacterState(sqlite3* db, const AccountDbCharacterState& state)
{
    if (db == nullptr) {
        return false;
    }

    SYSTEMTIME sysTime;
    GetLocalTime(&sysTime);
    char timestamp[32] = {};
    FormatTimestamp(sysTime, timestamp, sizeof(timestamp));

    const char* sql =
        "INSERT INTO characters("
        " account_name, character_name, created_at, profile, location, guild_name, guild_guid, guild_rank, "
        " map_name, map_x, map_y, hp, mp, sp, level, rating, str, intl, vit, dex, mag, chr, luck, exp, "
        " lu_pool, enemy_kill_count, pk_count, reward_gold, downskill_index, idnum1, idnum2, idnum3, "
        " gender, skin, hairstyle, haircolor, underwear, hunger_status, timeleft_rating, "
        " timeleft_force_recall, timeleft_firm_staminar, penalty_block_year, "
        " penalty_block_month, penalty_block_day, quest_number, quest_id, current_quest_count, "
        " quest_reward_type, quest_reward_amount, contribution, war_contribution, quest_completed, "
        " special_event_id, super_attack_left, fightzone_number, reserve_time, fightzone_ticket_number, "
        " special_ability_time, locked_map_name, locked_map_time, crusade_job, crusade_guid, "
        " construct_point, dead_penalty_time, party_id, gizon_item_upgrade_left,"
        " underwear_type, hair_color, hair_style, skin_color"
        ") VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    int col = 1;
    bool ok = true;
    ok &= PrepareAndBindText(stmt, col++, state.accountName);
    ok &= PrepareAndBindText(stmt, col++, state.characterName);
    ok &= PrepareAndBindText(stmt, col++, timestamp);
    ok &= PrepareAndBindText(stmt, col++, state.profile);
    ok &= PrepareAndBindText(stmt, col++, state.location);
    ok &= PrepareAndBindText(stmt, col++, state.guildName);
    ok &= (sqlite3_bind_int(stmt, col++, state.guildGuid) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.guildRank) == SQLITE_OK);
    ok &= PrepareAndBindText(stmt, col++, state.mapName);
    ok &= (sqlite3_bind_int(stmt, col++, state.mapX) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.mapY) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.hp) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.mp) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.sp) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.level) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.rating) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.str) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.intl) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.vit) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.dex) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.mag) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.chr) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.luck) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, static_cast<int>(state.exp)) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.luPool) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.enemyKillCount) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.pkCount) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, static_cast<int>(state.rewardGold)) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.downSkillIndex) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.idnum1) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.idnum2) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.idnum3) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.sex) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.skin) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.hairStyle) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.hairColor) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.underwear) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.hungerStatus) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.timeleftRating) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.timeleftForceRecall) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.timeleftFirmStaminar) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.penaltyBlockYear) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.penaltyBlockMonth) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.penaltyBlockDay) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.questNumber) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.questId) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.currentQuestCount) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.questRewardType) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.questRewardAmount) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.contribution) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.warContribution) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.questCompleted) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.specialEventId) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.superAttackLeft) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.fightzoneNumber) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.reserveTime) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.fightzoneTicketNumber) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.specialAbilityTime) == SQLITE_OK);
    ok &= PrepareAndBindText(stmt, col++, state.lockedMapName);
    ok &= (sqlite3_bind_int(stmt, col++, state.lockedMapTime) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.crusadeJob) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, static_cast<int>(state.crusadeGuid)) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.constructPoint) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.deadPenaltyTime) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.partyId) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.gizonItemUpgradeLeft) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.appearance.iUnderwearType) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.appearance.iHairColor) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.appearance.iHairStyle) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, col++, state.appearance.iSkinColor) == SQLITE_OK);

    if (ok) {
        ok = sqlite3_step(stmt) == SQLITE_DONE;
    }

    sqlite3_finalize(stmt);
    return ok;
}

bool InsertCharacterItems(sqlite3* db, const char* characterName, const std::vector<AccountDbItemRow>& items)
{
    if (db == nullptr || characterName == nullptr) {
        return false;
    }

    const char* sql =
        "INSERT INTO character_items("
        " character_name, slot, item_id, count, touch_effect_type, touch_effect_value1, touch_effect_value2,"
        " touch_effect_value3, item_color, spec_effect_value1, spec_effect_value2, spec_effect_value3,"
        " cur_lifespan, attribute, pos_x, pos_y, is_equipped"
        ") VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    for (const auto& item : items) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        int col = 1;
        bool ok = true;
        ok &= PrepareAndBindText(stmt, col++, characterName);
        ok &= (sqlite3_bind_int(stmt, col++, item.slot) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.itemId) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.count) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.touchEffectType) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.touchEffectValue1) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.touchEffectValue2) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.touchEffectValue3) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.itemColor) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.specEffectValue1) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.specEffectValue2) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.specEffectValue3) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.curLifeSpan) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, static_cast<int>(item.attribute)) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.posX) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.posY) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.isEquipped) == SQLITE_OK);
        if (!ok || sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return false;
        }
    }

    sqlite3_finalize(stmt);
    return true;
}

bool InsertCharacterBankItems(sqlite3* db, const char* characterName, const std::vector<AccountDbBankItemRow>& items)
{
    if (db == nullptr || characterName == nullptr) {
        return false;
    }

    const char* sql =
        "INSERT INTO character_bank_items("
        " character_name, slot, item_id, count, touch_effect_type, touch_effect_value1, touch_effect_value2,"
        " touch_effect_value3, item_color, spec_effect_value1, spec_effect_value2, spec_effect_value3,"
        " cur_lifespan, attribute"
        ") VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    for (const auto& item : items) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        int col = 1;
        bool ok = true;
        ok &= PrepareAndBindText(stmt, col++, characterName);
        ok &= (sqlite3_bind_int(stmt, col++, item.slot) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.itemId) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.count) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.touchEffectType) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.touchEffectValue1) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.touchEffectValue2) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.touchEffectValue3) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.itemColor) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.specEffectValue1) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.specEffectValue2) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.specEffectValue3) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, item.curLifeSpan) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, static_cast<int>(item.attribute)) == SQLITE_OK);
        if (!ok || sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return false;
        }
    }

    sqlite3_finalize(stmt);
    return true;
}

bool InsertCharacterItemPositions(sqlite3* db, const char* characterName, const std::vector<AccountDbIndexedValue>& positionsX, const std::vector<AccountDbIndexedValue>& positionsY)
{
    if (db == nullptr || characterName == nullptr || positionsX.size() != positionsY.size()) {
        return false;
    }

    const char* sql =
        "INSERT INTO character_item_positions(character_name, slot, pos_x, pos_y) VALUES(?,?,?,?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    for (size_t i = 0; i < positionsX.size(); i++) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        bool ok = true;
        ok &= PrepareAndBindText(stmt, 1, characterName);
        ok &= (sqlite3_bind_int(stmt, 2, positionsX[i].index) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, 3, positionsX[i].value) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, 4, positionsY[i].value) == SQLITE_OK);
        if (!ok || sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return false;
        }
    }

    sqlite3_finalize(stmt);
    return true;
}

bool InsertCharacterItemEquips(sqlite3* db, const char* characterName, const std::vector<AccountDbIndexedValue>& equips)
{
    if (db == nullptr || characterName == nullptr) {
        return false;
    }

    const char* sql =
        "INSERT INTO character_item_equips(character_name, slot, is_equipped) VALUES(?,?,?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    for (const auto& equip : equips) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        bool ok = true;
        ok &= PrepareAndBindText(stmt, 1, characterName);
        ok &= (sqlite3_bind_int(stmt, 2, equip.index) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, 3, equip.value) == SQLITE_OK);
        if (!ok || sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return false;
        }
    }

    sqlite3_finalize(stmt);
    return true;
}

bool InsertCharacterMagicMastery(sqlite3* db, const char* characterName, const std::vector<AccountDbIndexedValue>& mastery)
{
    if (db == nullptr || characterName == nullptr) {
        return false;
    }

    const char* sql =
        "INSERT INTO character_magic_mastery(character_name, magic_index, mastery_value) VALUES(?,?,?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    for (const auto& entry : mastery) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        bool ok = true;
        ok &= PrepareAndBindText(stmt, 1, characterName);
        ok &= (sqlite3_bind_int(stmt, 2, entry.index) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, 3, entry.value) == SQLITE_OK);
        if (!ok || sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return false;
        }
    }

    sqlite3_finalize(stmt);
    return true;
}

bool InsertCharacterSkillMastery(sqlite3* db, const char* characterName, const std::vector<AccountDbIndexedValue>& mastery)
{
    if (db == nullptr || characterName == nullptr) {
        return false;
    }

    const char* sql =
        "INSERT INTO character_skill_mastery(character_name, skill_index, mastery_value) VALUES(?,?,?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    for (const auto& entry : mastery) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        bool ok = true;
        ok &= PrepareAndBindText(stmt, 1, characterName);
        ok &= (sqlite3_bind_int(stmt, 2, entry.index) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, 3, entry.value) == SQLITE_OK);
        if (!ok || sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return false;
        }
    }

    sqlite3_finalize(stmt);
    return true;
}

bool InsertCharacterSkillSSN(sqlite3* db, const char* characterName, const std::vector<AccountDbIndexedValue>& values)
{
    if (db == nullptr || characterName == nullptr) {
        return false;
    }

    const char* sql =
        "INSERT INTO character_skill_ssn(character_name, skill_index, ssn_value) VALUES(?,?,?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    for (const auto& entry : values) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        bool ok = true;
        ok &= PrepareAndBindText(stmt, 1, characterName);
        ok &= (sqlite3_bind_int(stmt, 2, entry.index) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, 3, entry.value) == SQLITE_OK);
        if (!ok || sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return false;
        }
    }

    sqlite3_finalize(stmt);
    return true;
}

bool InsertAccountRecord(sqlite3* db, const AccountDbAccountData& data)
{
    if (db == nullptr) {
        return false;
    }

    const char* sql =
        "INSERT INTO accounts("
        " account_name, password_hash, password_salt, email, created_at, password_changed_at, last_ip"
        ") VALUES(?,?,?,?,?,?,?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        char logMsg[512] = {};
        std::snprintf(logMsg, sizeof(logMsg), "(SQLITE) Prepare account insert failed: %s", sqlite3_errmsg(db));
        PutLogList(logMsg);
        return false;
    }

    bool ok =
        PrepareAndBindText(&stmt, 1, data.name) &&
        PrepareAndBindText(&stmt, 2, data.password_hash) &&
        PrepareAndBindText(&stmt, 3, data.password_salt) &&
        PrepareAndBindText(&stmt, 4, data.email) &&
        PrepareAndBindText(&stmt, 5, data.createdAt) &&
        PrepareAndBindText(&stmt, 6, data.passwordChangedAt) &&
        PrepareAndBindText(&stmt, 7, data.lastIp);

    if (ok) {
        ok = sqlite3_step(stmt) == SQLITE_DONE;
    }

    if (!ok) {
        char logMsg[512] = {};
        std::snprintf(logMsg, sizeof(logMsg), "(SQLITE) Insert account failed: %s", sqlite3_errmsg(db));
        PutLogList(logMsg);
    }

    sqlite3_finalize(stmt);
    return ok;
}

bool InsertCharacterRecord(sqlite3* db, const AccountDbCharacterData& data)
{
    if (db == nullptr) {
        return false;
    }

    const char* sql =
        "INSERT INTO characters("
        " character_name, account_name, created_at,"
        " underwear_type, hair_color, hair_style, skin_color,"
        " level, exp, map_name, map_x, map_y, hp, mp, sp, str, vit, dex, intl, mag, chr,"
        " gender, skin, hairstyle, haircolor, underwear"
        ") VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        char logMsg[512] = {};
        std::snprintf(logMsg, sizeof(logMsg), "(SQLITE) Prepare character insert failed: %s", sqlite3_errmsg(db));
        PutLogList(logMsg);
        return false;
    }

    int idx = 1;
    bool ok = true;
    ok &= PrepareAndBindText(&stmt, idx++, data.characterName);
    ok &= PrepareAndBindText(&stmt, idx++, data.accountName);
    ok &= PrepareAndBindText(&stmt, idx++, data.createdAt);
    ok &= (sqlite3_bind_int(stmt, idx++, data.appearance.iUnderwearType) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, data.appearance.iHairColor) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, data.appearance.iHairStyle) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, data.appearance.iSkinColor) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, data.level) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, static_cast<int>(data.exp)) == SQLITE_OK);
    ok &= PrepareAndBindText(&stmt, idx++, data.mapName);
    ok &= (sqlite3_bind_int(stmt, idx++, data.mapX) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, data.mapY) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, data.hp) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, data.mp) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, data.sp) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, data.str) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, data.vit) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, data.dex) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, data.intl) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, data.mag) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, data.chr) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, data.gender) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, data.skin) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, data.hairStyle) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, data.hairColor) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, data.underwear) == SQLITE_OK);

    if (ok) {
        ok = sqlite3_step(stmt) == SQLITE_DONE;
    }

    if (!ok) {
        char logMsg[512] = {};
        std::snprintf(logMsg, sizeof(logMsg), "(SQLITE) Insert character failed: %s", sqlite3_errmsg(db));
        PutLogList(logMsg);
    }

    sqlite3_finalize(stmt);
    return ok;
}

bool DeleteCharacterData(sqlite3* db, const char* characterName)
{
    if (db == nullptr || characterName == nullptr) {
        return false;
    }

    const char* sql = "DELETE FROM characters WHERE character_name = ? COLLATE NOCASE;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    PrepareAndBindText(&stmt, 1, characterName);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

void CloseAccountDatabase(sqlite3* db)
{
    if (db != nullptr) {
        sqlite3_close(db);
    }
}

bool SaveCharacterSnapshot(sqlite3* db, const CClient* client)
{
    if (db == nullptr || client == nullptr) {
        PutLogList("(SQLITE) SaveCharacterSnapshot: null db or client");
        return false;
    }

    // Helper: capture the real SQLite error before ROLLBACK clears it
    auto FailAndRollback = [&](const char* stage) {
        char logMsg[512] = {};
        std::snprintf(logMsg, sizeof(logMsg),
            "(SQLITE) SaveCharacterSnapshot failed at [%s]: %s",
            stage, sqlite3_errmsg(db));
        PutLogList(logMsg);
        ExecSql(db, "ROLLBACK;");
    };

    if (!ExecSql(db, "BEGIN;")) {
        PutLogList("(SQLITE) SaveCharacterSnapshot: BEGIN failed");
        return false;
    }

    SYSTEMTIME sysTime;
    GetLocalTime(&sysTime);
    char timestamp[32] = {};
    FormatTimestamp(sysTime, timestamp, sizeof(timestamp));

    const char* upsertSql =
        "INSERT OR REPLACE INTO characters("
        " account_name, character_name, created_at, profile, location, guild_name, guild_guid, guild_rank,"
        " map_name, map_x, map_y, hp, mp, sp, level, rating, str, intl, vit, dex, mag, chr, luck, exp,"
        " lu_pool, enemy_kill_count, pk_count, reward_gold, downskill_index, idnum1, idnum2, idnum3,"
        " gender, skin, hairstyle, haircolor, underwear, hunger_status, timeleft_rating,"
        " timeleft_force_recall, timeleft_firm_staminar, penalty_block_year,"
        " penalty_block_month, penalty_block_day, quest_number, quest_id, current_quest_count,"
        " quest_reward_type, quest_reward_amount, contribution, war_contribution, quest_completed,"
        " special_event_id, super_attack_left, fightzone_number, reserve_time, fightzone_ticket_number,"
        " special_ability_time, locked_map_name, locked_map_time, crusade_job, crusade_guid,"
        " construct_point, dead_penalty_time, party_id, gizon_item_upgrade_left,"
        " underwear_type, hair_color, hair_style, skin_color"
        ") VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, upsertSql, -1, &stmt, nullptr) != SQLITE_OK) {
        char logMsg[512] = {};
        std::snprintf(logMsg, sizeof(logMsg), "(SQLITE) SaveCharacterSnapshot: upsert prepare failed: %s", sqlite3_errmsg(db));
        PutLogList(logMsg);
        ExecSql(db, "ROLLBACK;");
        return false;
    }

    int idx = 1;
    bool ok = true;
    ok &= PrepareAndBindText(stmt, idx++, client->m_cAccountName);
    ok &= PrepareAndBindText(stmt, idx++, client->m_cCharName);
    ok &= PrepareAndBindText(stmt, idx++, timestamp);
    ok &= PrepareAndBindText(stmt, idx++, client->m_cProfile);
    ok &= PrepareAndBindText(stmt, idx++, client->m_cLocation);
    ok &= PrepareAndBindText(stmt, idx++, client->m_cGuildName);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iGuildGUID) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iGuildRank) == SQLITE_OK);
    ok &= PrepareAndBindText(stmt, idx++, client->m_cMapName);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_sX) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_sY) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iHP) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iMP) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iSP) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iLevel) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iRating) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iStr) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iInt) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iVit) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iDex) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iMag) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iCharisma) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iLuck) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, static_cast<int>(client->m_iExp)) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iLU_Pool) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iEnemyKillCount) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iPKCount) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, static_cast<int>(client->m_iRewardGold)) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iDownSkillIndex) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_sCharIDnum1) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_sCharIDnum2) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_sCharIDnum3) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_cSex) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_cSkin) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_cHairStyle) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_cHairColor) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_cUnderwear) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iHungerStatus) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iTimeLeft_Rating) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iTimeLeft_ForceRecall) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iTimeLeft_FirmStaminar) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iPenaltyBlockYear) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iPenaltyBlockMonth) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iPenaltyBlockDay) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iQuest) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iQuestID) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iCurQuestCount) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iQuestRewardType) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iQuestRewardAmount) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iContribution) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iWarContribution) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_bIsQuestCompleted ? 1 : 0) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iSpecialEventID) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iSuperAttackLeft) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iFightzoneNumber) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iReserveTime) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iFightZoneTicketNumber) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iSpecialAbilityTime) == SQLITE_OK);
    ok &= PrepareAndBindText(stmt, idx++, client->m_cLockedMapName);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iLockedMapTime) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iCrusadeDuty) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, static_cast<int>(client->m_dwCrusadeGUID)) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iConstructionPoint) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iDeadPenaltyTime) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iPartyID) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_iGizonItemUpgradeLeft) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_appearance.iUnderwearType) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_appearance.iHairColor) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_appearance.iHairStyle) == SQLITE_OK);
    ok &= (sqlite3_bind_int(stmt, idx++, client->m_appearance.iSkinColor) == SQLITE_OK);

    if (!ok) {
        char logMsg[512] = {};
        std::snprintf(logMsg, sizeof(logMsg), "(SQLITE) SaveCharacterSnapshot: upsert bind failed at idx %d: %s", idx - 1, sqlite3_errmsg(db));
        PutLogList(logMsg);
        sqlite3_finalize(stmt);
        ExecSql(db, "ROLLBACK;");
        return false;
    }

    int stepRc = sqlite3_step(stmt);
    if (stepRc != SQLITE_DONE) {
        char logMsg[512] = {};
        std::snprintf(logMsg, sizeof(logMsg), "(SQLITE) SaveCharacterSnapshot: upsert step failed (rc=%d): %s", stepRc, sqlite3_errmsg(db));
        PutLogList(logMsg);
        sqlite3_finalize(stmt);
        ExecSql(db, "ROLLBACK;");
        return false;
    }
    sqlite3_finalize(stmt);

    const char* deleteItemsSql = "DELETE FROM character_items WHERE character_name = ? COLLATE NOCASE;";
    const char* deleteBankSql = "DELETE FROM character_bank_items WHERE character_name = ? COLLATE NOCASE;";
    const char* deletePosSql = "DELETE FROM character_item_positions WHERE character_name = ? COLLATE NOCASE;";
    const char* deleteEquipSql = "DELETE FROM character_item_equips WHERE character_name = ? COLLATE NOCASE;";
    const char* deleteMagicSql = "DELETE FROM character_magic_mastery WHERE character_name = ? COLLATE NOCASE;";
    const char* deleteSkillSql = "DELETE FROM character_skill_mastery WHERE character_name = ? COLLATE NOCASE;";
    const char* deleteSsnSql = "DELETE FROM character_skill_ssn WHERE character_name = ? COLLATE NOCASE;";

    const char* deleteStatements[] = {
        deleteItemsSql, deleteBankSql, deletePosSql, deleteEquipSql, deleteMagicSql, deleteSkillSql, deleteSsnSql
    };

    for (const char* deleteSql : deleteStatements) {
        if (sqlite3_prepare_v2(db, deleteSql, -1, &stmt, nullptr) != SQLITE_OK) {
            char logMsg[512] = {};
            std::snprintf(logMsg, sizeof(logMsg), "(SQLITE) SaveCharacterSnapshot: delete prepare failed: %s | SQL: %.100s", sqlite3_errmsg(db), deleteSql);
            PutLogList(logMsg);
            ExecSql(db, "ROLLBACK;");
            return false;
        }
        PrepareAndBindText(stmt, 1, client->m_cCharName);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (rc != SQLITE_DONE) {
            char logMsg[512] = {};
            std::snprintf(logMsg, sizeof(logMsg), "(SQLITE) SaveCharacterSnapshot: delete step failed (rc=%d): %s | SQL: %.100s", rc, sqlite3_errmsg(db), deleteSql);
            PutLogList(logMsg);
            ExecSql(db, "ROLLBACK;");
            return false;
        }
    }

    const char* insertItemSql =
        "INSERT INTO character_items("
        " character_name, slot, item_id, count, touch_effect_type, touch_effect_value1, touch_effect_value2,"
        " touch_effect_value3, item_color, spec_effect_value1, spec_effect_value2, spec_effect_value3,"
        " cur_lifespan, attribute, pos_x, pos_y, is_equipped"
        ") VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    if (sqlite3_prepare_v2(db, insertItemSql, -1, &stmt, nullptr) != SQLITE_OK) {
        char logMsg[512] = {};
        std::snprintf(logMsg, sizeof(logMsg), "(SQLITE) SaveCharacterSnapshot: insert items prepare failed: %s", sqlite3_errmsg(db));
        PutLogList(logMsg);
        ExecSql(db, "ROLLBACK;");
        return false;
    }

    for(int i = 0; i < hb::shared::limits::MaxItems; i++) {
        if (client->m_pItemList[i] == nullptr) {
            continue;
        }
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        int col = 1;
        ok = true;
        ok &= PrepareAndBindText(stmt, col++, client->m_cCharName);
        ok &= (sqlite3_bind_int(stmt, col++, i) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, client->m_pItemList[i]->m_sIDnum) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, static_cast<int>(client->m_pItemList[i]->m_dwCount)) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, client->m_pItemList[i]->m_sTouchEffectType) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, client->m_pItemList[i]->m_sTouchEffectValue1) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, client->m_pItemList[i]->m_sTouchEffectValue2) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, client->m_pItemList[i]->m_sTouchEffectValue3) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, client->m_pItemList[i]->m_cItemColor) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, client->m_pItemList[i]->m_sItemSpecEffectValue1) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, client->m_pItemList[i]->m_sItemSpecEffectValue2) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, client->m_pItemList[i]->m_sItemSpecEffectValue3) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, client->m_pItemList[i]->m_wCurLifeSpan) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, static_cast<int>(client->m_pItemList[i]->m_dwAttribute)) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, client->m_ItemPosList[i].x) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, client->m_ItemPosList[i].y) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, client->m_bIsItemEquipped[i] ? 1 : 0) == SQLITE_OK);

        if (ok) {
            int rc = sqlite3_step(stmt);
            ok = rc == SQLITE_DONE;
            if (!ok) {
                char logMsg[512] = {};
                std::snprintf(logMsg, sizeof(logMsg), "(SQLITE) SaveCharacterSnapshot: insert item[%d] step failed (rc=%d): %s", i, rc, sqlite3_errmsg(db));
                PutLogList(logMsg);
            }
        } else {
            char logMsg[256] = {};
            std::snprintf(logMsg, sizeof(logMsg), "(SQLITE) SaveCharacterSnapshot: insert item[%d] bind failed", i);
            PutLogList(logMsg);
        }
        if (!ok) {
            sqlite3_finalize(stmt);
            ExecSql(db, "ROLLBACK;");
            return false;
        }
    }
    sqlite3_finalize(stmt);

    const char* insertBankSql =
        "INSERT INTO character_bank_items("
        " character_name, slot, item_id, count, touch_effect_type, touch_effect_value1, touch_effect_value2,"
        " touch_effect_value3, item_color, spec_effect_value1, spec_effect_value2, spec_effect_value3,"
        " cur_lifespan, attribute"
        ") VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    if (sqlite3_prepare_v2(db, insertBankSql, -1, &stmt, nullptr) != SQLITE_OK) {
        FailAndRollback("bank_items prepare");
        return false;
    }

    for(int i = 0; i < hb::shared::limits::MaxBankItems; i++) {
        if (client->m_pItemInBankList[i] == nullptr) {
            continue;
        }
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        int col = 1;
        ok = true;
        ok &= PrepareAndBindText(stmt, col++, client->m_cCharName);
        ok &= (sqlite3_bind_int(stmt, col++, i) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, client->m_pItemInBankList[i]->m_sIDnum) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, static_cast<int>(client->m_pItemInBankList[i]->m_dwCount)) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, client->m_pItemInBankList[i]->m_sTouchEffectType) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, client->m_pItemInBankList[i]->m_sTouchEffectValue1) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, client->m_pItemInBankList[i]->m_sTouchEffectValue2) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, client->m_pItemInBankList[i]->m_sTouchEffectValue3) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, client->m_pItemInBankList[i]->m_cItemColor) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, client->m_pItemInBankList[i]->m_sItemSpecEffectValue1) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, client->m_pItemInBankList[i]->m_sItemSpecEffectValue2) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, client->m_pItemInBankList[i]->m_sItemSpecEffectValue3) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, client->m_pItemInBankList[i]->m_wCurLifeSpan) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, col++, static_cast<int>(client->m_pItemInBankList[i]->m_dwAttribute)) == SQLITE_OK);

        if (ok) {
            ok = sqlite3_step(stmt) == SQLITE_DONE;
        }
        if (!ok) {
            sqlite3_finalize(stmt);
            FailAndRollback("bank_items step/bind");
            return false;
        }
    }
    sqlite3_finalize(stmt);

    const char* insertPosSql =
        "INSERT INTO character_item_positions(character_name, slot, pos_x, pos_y)"
        " VALUES(?,?,?,?);";
    if (sqlite3_prepare_v2(db, insertPosSql, -1, &stmt, nullptr) != SQLITE_OK) {
        FailAndRollback("item_positions prepare");
        return false;
    }
    for(int i = 0; i < hb::shared::limits::MaxItems; i++) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        ok = true;
        ok &= PrepareAndBindText(stmt, 1, client->m_cCharName);
        ok &= (sqlite3_bind_int(stmt, 2, i) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, 3, client->m_ItemPosList[i].x) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, 4, client->m_ItemPosList[i].y) == SQLITE_OK);
        if (ok) {
            ok = sqlite3_step(stmt) == SQLITE_DONE;
        }
        if (!ok) {
            sqlite3_finalize(stmt);
            FailAndRollback("item_positions step/bind");
            return false;
        }
    }
    sqlite3_finalize(stmt);

    const char* insertEquipSql =
        "INSERT INTO character_item_equips(character_name, slot, is_equipped)"
        " VALUES(?,?,?);";
    if (sqlite3_prepare_v2(db, insertEquipSql, -1, &stmt, nullptr) != SQLITE_OK) {
        FailAndRollback("item_equips prepare");
        return false;
    }
    for(int i = 0; i < hb::shared::limits::MaxItems; i++) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        ok = true;
        ok &= PrepareAndBindText(stmt, 1, client->m_cCharName);
        ok &= (sqlite3_bind_int(stmt, 2, i) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, 3, client->m_bIsItemEquipped[i] ? 1 : 0) == SQLITE_OK);
        if (ok) {
            ok = sqlite3_step(stmt) == SQLITE_DONE;
        }
        if (!ok) {
            sqlite3_finalize(stmt);
            FailAndRollback("item_equips step/bind");
            return false;
        }
    }
    sqlite3_finalize(stmt);

    const char* insertMagicSql =
        "INSERT INTO character_magic_mastery(character_name, magic_index, mastery_value)"
        " VALUES(?,?,?);";
    if (sqlite3_prepare_v2(db, insertMagicSql, -1, &stmt, nullptr) != SQLITE_OK) {
        FailAndRollback("magic_mastery prepare");
        return false;
    }
    for(int i = 0; i < hb::shared::limits::MaxMagicType; i++) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        ok = true;
        ok &= PrepareAndBindText(stmt, 1, client->m_cCharName);
        ok &= (sqlite3_bind_int(stmt, 2, i) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, 3, client->m_cMagicMastery[i]) == SQLITE_OK);
        if (ok) {
            ok = sqlite3_step(stmt) == SQLITE_DONE;
        }
        if (!ok) {
            sqlite3_finalize(stmt);
            FailAndRollback("magic_mastery step/bind");
            return false;
        }
    }
    sqlite3_finalize(stmt);

    const char* insertSkillSql =
        "INSERT INTO character_skill_mastery(character_name, skill_index, mastery_value)"
        " VALUES(?,?,?);";
    if (sqlite3_prepare_v2(db, insertSkillSql, -1, &stmt, nullptr) != SQLITE_OK) {
        FailAndRollback("skill_mastery prepare");
        return false;
    }
    for(int i = 0; i < hb::shared::limits::MaxSkillType; i++) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        ok = true;
        ok &= PrepareAndBindText(stmt, 1, client->m_cCharName);
        ok &= (sqlite3_bind_int(stmt, 2, i) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, 3, client->m_cSkillMastery[i]) == SQLITE_OK);
        if (ok) {
            ok = sqlite3_step(stmt) == SQLITE_DONE;
        }
        if (!ok) {
            sqlite3_finalize(stmt);
            FailAndRollback("skill_mastery step/bind");
            return false;
        }
    }
    sqlite3_finalize(stmt);

    const char* insertSsnSql =
        "INSERT INTO character_skill_ssn(character_name, skill_index, ssn_value)"
        " VALUES(?,?,?);";
    if (sqlite3_prepare_v2(db, insertSsnSql, -1, &stmt, nullptr) != SQLITE_OK) {
        FailAndRollback("skill_ssn prepare");
        return false;
    }
    for(int i = 0; i < hb::shared::limits::MaxSkillType; i++) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        ok = true;
        ok &= PrepareAndBindText(stmt, 1, client->m_cCharName);
        ok &= (sqlite3_bind_int(stmt, 2, i) == SQLITE_OK);
        ok &= (sqlite3_bind_int(stmt, 3, client->m_iSkillSSN[i]) == SQLITE_OK);
        if (ok) {
            ok = sqlite3_step(stmt) == SQLITE_DONE;
        }
        if (!ok) {
            sqlite3_finalize(stmt);
            FailAndRollback("skill_ssn step/bind");
            return false;
        }
    }
    sqlite3_finalize(stmt);

    if (!ExecSql(db, "COMMIT;")) {
        FailAndRollback("COMMIT");
        return false;
    }
    return true;
}

bool CharacterNameExistsGlobally(const char* characterName)
{
    if (characterName == nullptr || characterName[0] == '\0') {
        return false;
    }

    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile("Accounts/*.db", &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        // No account databases found
        return false;
    }

    bool found = false;

    do {
        // Skip directories
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }

        // Build full path to database
        char dbPath[MAX_PATH] = {};
        std::snprintf(dbPath, sizeof(dbPath), "Accounts/%s", findData.cFileName);

        // Open the database
        sqlite3* db = nullptr;
        if (sqlite3_open(dbPath, &db) != SQLITE_OK) {
            if (db) sqlite3_close(db);
            continue;
        }

        // Check if character name exists in this database
        const char* sql = "SELECT 1 FROM characters WHERE character_name = ? COLLATE NOCASE LIMIT 1";
        sqlite3_stmt* stmt = nullptr;

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_bind_text(stmt, 1, characterName, -1, SQLITE_TRANSIENT) == SQLITE_OK) {
                if (sqlite3_step(stmt) == SQLITE_ROW) {
                    // Character name found in this database
                    found = true;
                }
            }
            sqlite3_finalize(stmt);
        }

        sqlite3_close(db);

        if (found) {
            break;  // No need to check remaining databases
        }

    } while (FindNextFile(hFind, &findData) != 0);

    FindClose(hFind);

    return found;
}

bool AccountNameExists(const char* accountName)
{
    if (accountName == nullptr || accountName[0] == '\0') {
        return false;
    }

    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile("Accounts/*.db", &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        // No account databases found
        return false;
    }

    bool found = false;

    do {
        // Skip directories
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }

        // Build full path to database
        char dbPath[MAX_PATH] = {};
        std::snprintf(dbPath, sizeof(dbPath), "Accounts/%s", findData.cFileName);

        // Open the database
        sqlite3* db = nullptr;
        if (sqlite3_open(dbPath, &db) != SQLITE_OK) {
            if (db) sqlite3_close(db);
            continue;
        }

        // Check if account name exists in this database
        const char* sql = "SELECT 1 FROM accounts WHERE account_name = ? COLLATE NOCASE LIMIT 1";
        sqlite3_stmt* stmt = nullptr;

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_bind_text(stmt, 1, accountName, -1, SQLITE_TRANSIENT) == SQLITE_OK) {
                if (sqlite3_step(stmt) == SQLITE_ROW) {
                    // Account name found in this database
                    found = true;
                }
            }
            sqlite3_finalize(stmt);
        }

        sqlite3_close(db);

        if (found) {
            break;  // No need to check remaining databases
        }

    } while (FindNextFile(hFind, &findData) != 0);

    FindClose(hFind);

    return found;
}

bool LoadBlockList(sqlite3* db, std::vector<std::pair<std::string, std::string>>& outBlocks)
{
    outBlocks.clear();
    const char* sql = "SELECT blocked_account_name, blocked_character_name FROM block_list";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char* accountName = (const char*)sqlite3_column_text(stmt, 0);
        const char* charName = (const char*)sqlite3_column_text(stmt, 1);
        if (accountName && charName)
            outBlocks.push_back(std::make_pair(std::string(accountName), std::string(charName)));
    }

    sqlite3_finalize(stmt);
    return true;
}

bool SaveBlockList(sqlite3* db, const std::vector<std::pair<std::string, std::string>>& blocks)
{
    if (!ExecSql(db, "DELETE FROM block_list"))
        return false;

    const char* sql = "INSERT INTO block_list (blocked_account_name, blocked_character_name) VALUES (?, ?)";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    for (const auto& entry : blocks)
    {
        sqlite3_reset(stmt);
        sqlite3_bind_text(stmt, 1, entry.first.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, entry.second.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            sqlite3_finalize(stmt);
            return false;
        }
    }

    sqlite3_finalize(stmt);
    return true;
}

bool ResolveCharacterToAccount(const char* characterName, char* outAccountName, size_t accountNameSize)
{
    if (characterName == nullptr || outAccountName == nullptr || accountNameSize == 0)
        return false;

    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile("Accounts/*.db", &findData);

    if (hFind == INVALID_HANDLE_VALUE)
        return false;

    bool found = false;

    do {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;

        char dbPath[MAX_PATH] = {};
        std::snprintf(dbPath, sizeof(dbPath), "Accounts/%s", findData.cFileName);

        sqlite3* db = nullptr;
        if (sqlite3_open(dbPath, &db) != SQLITE_OK)
        {
            if (db) sqlite3_close(db);
            continue;
        }

        const char* sql = "SELECT account_name FROM characters WHERE character_name = ? COLLATE NOCASE LIMIT 1";
        sqlite3_stmt* stmt = nullptr;

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK)
        {
            if (sqlite3_bind_text(stmt, 1, characterName, -1, SQLITE_TRANSIENT) == SQLITE_OK)
            {
                if (sqlite3_step(stmt) == SQLITE_ROW)
                {
                    const char* acctName = (const char*)sqlite3_column_text(stmt, 0);
                    if (acctName)
                    {
                        std::strncpy(outAccountName, acctName, accountNameSize - 1);
                        outAccountName[accountNameSize - 1] = '\0';
                        found = true;
                    }
                }
            }
            sqlite3_finalize(stmt);
        }

        sqlite3_close(db);

        if (found)
            break;

    } while (FindNextFile(hFind, &findData) != 0);

    FindClose(hFind);

    return found;
}
