#include "MapInfoSqliteStore.h"

#include "Platform.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "Map.h"
#include "TeleportLoc.h"
#include "StrategicPoint.h"
#include "sqlite3.h"

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
			std::snprintf(logMsg, sizeof(logMsg), "(MAPINFO-SQLITE) Exec failed: %s", err ? err : "unknown");
			PutLogList(logMsg);
			sqlite3_free(err);
			return false;
		}
		return true;
	}

	bool PrepareAndBindText(sqlite3_stmt* stmt, int idx, const char* value)
	{
		return sqlite3_bind_text(stmt, idx, value, -1, SQLITE_TRANSIENT) == SQLITE_OK;
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

	// Parse comma-separated waypoint indices into a char array
	void ParseWaypointList(const char* waypointStr, char* waypointArray, int maxWaypoints)
	{
		std::memset(waypointArray, -1, maxWaypoints);
		if (waypointStr == nullptr || waypointStr[0] == '\0') {
			return;
		}

		char buffer[256];
		std::strncpy(buffer, waypointStr, sizeof(buffer) - 1);
		buffer[sizeof(buffer) - 1] = '\0';

		int idx = 0;
		char* token = std::strtok(buffer, ",");
		while (token != nullptr && idx < maxWaypoints) {
			waypointArray[idx++] = static_cast<char>(std::atoi(token));
			token = std::strtok(nullptr, ",");
		}
	}
}

bool EnsureMapInfoDatabase(sqlite3** outDb, std::string& outPath, bool* outCreated)
{
	if (outDb == nullptr) {
		return false;
	}

	std::string dbPath = "MapInfo.db";
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
				dbPath.append("MapInfo.db");
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
		std::snprintf(logMsg, sizeof(logMsg), "(MAPINFO-SQLITE) Open failed: %s", sqlite3_errmsg(db));
		PutLogList(logMsg);
		sqlite3_close(db);
		return false;
	}

	sqlite3_busy_timeout(db, 1000);
	if (!ExecSql(db, "PRAGMA foreign_keys = ON;")) {
		sqlite3_close(db);
		return false;
	}

	// Create schema
	const char* schemaSql =
		"BEGIN;"
		"CREATE TABLE IF NOT EXISTS meta ("
		" key TEXT PRIMARY KEY,"
		" value TEXT NOT NULL"
		");"
		"INSERT OR REPLACE INTO meta(key, value) VALUES('schema_version','2');"

		// Core map settings
		"CREATE TABLE IF NOT EXISTS maps ("
		" map_name TEXT PRIMARY KEY CHECK(length(map_name) <= 10),"
		" location_name TEXT NOT NULL DEFAULT '' CHECK(length(location_name) <= 10),"
		" maximum_object INTEGER NOT NULL DEFAULT 1000,"
		" level_limit INTEGER NOT NULL DEFAULT 0,"
		" upper_level_limit INTEGER NOT NULL DEFAULT 0,"
		" map_type INTEGER NOT NULL DEFAULT 0,"
		" random_mob_generator_enabled INTEGER NOT NULL DEFAULT 0,"
		" random_mob_generator_level INTEGER NOT NULL DEFAULT 0,"
		" mineral_generator_enabled INTEGER NOT NULL DEFAULT 0,"
		" mineral_generator_level INTEGER NOT NULL DEFAULT 0,"
		" max_fish INTEGER NOT NULL DEFAULT 0,"
		" max_mineral INTEGER NOT NULL DEFAULT 0,"
		" fixed_day_mode INTEGER NOT NULL DEFAULT 0,"
		" recall_impossible INTEGER NOT NULL DEFAULT 0,"
		" apocalypse_map INTEGER NOT NULL DEFAULT 0,"
		" apocalypse_mob_gen_type INTEGER NOT NULL DEFAULT 0,"
		" citizen_limit INTEGER NOT NULL DEFAULT 0,"
		" is_fight_zone INTEGER NOT NULL DEFAULT 0,"
		" heldenian_map INTEGER NOT NULL DEFAULT 0,"
		" heldenian_mode_map INTEGER NOT NULL DEFAULT 0,"
		" mob_event_amount INTEGER NOT NULL DEFAULT 15,"
		" energy_sphere_auto_creation INTEGER NOT NULL DEFAULT 0,"
		" pk_mode INTEGER NOT NULL DEFAULT 0,"
		" attack_enabled INTEGER NOT NULL DEFAULT 1"
		");"

		// Teleport locations
		"CREATE TABLE IF NOT EXISTS map_teleport_locations ("
		" map_name TEXT NOT NULL,"
		" teleport_index INTEGER NOT NULL,"
		" src_x INTEGER NOT NULL,"
		" src_y INTEGER NOT NULL,"
		" dest_map_name TEXT NOT NULL CHECK(length(dest_map_name) <= 10),"
		" dest_x INTEGER NOT NULL,"
		" dest_y INTEGER NOT NULL,"
		" direction INTEGER NOT NULL CHECK(direction >= 0 AND direction <= 8),"
		" PRIMARY KEY (map_name, teleport_index),"
		" FOREIGN KEY (map_name) REFERENCES maps(map_name) ON DELETE CASCADE"
		");"

		// Initial spawn points
		"CREATE TABLE IF NOT EXISTS map_initial_points ("
		" map_name TEXT NOT NULL,"
		" point_index INTEGER NOT NULL CHECK(point_index >= 0 AND point_index < 20),"
		" x INTEGER NOT NULL,"
		" y INTEGER NOT NULL,"
		" PRIMARY KEY (map_name, point_index),"
		" FOREIGN KEY (map_name) REFERENCES maps(map_name) ON DELETE CASCADE"
		");"

		// Waypoints
		"CREATE TABLE IF NOT EXISTS map_waypoints ("
		" map_name TEXT NOT NULL,"
		" waypoint_index INTEGER NOT NULL CHECK(waypoint_index >= 0 AND waypoint_index < 200),"
		" x INTEGER NOT NULL,"
		" y INTEGER NOT NULL,"
		" PRIMARY KEY (map_name, waypoint_index),"
		" FOREIGN KEY (map_name) REFERENCES maps(map_name) ON DELETE CASCADE"
		");"

		// No-attack areas
		"CREATE TABLE IF NOT EXISTS map_no_attack_areas ("
		" map_name TEXT NOT NULL,"
		" area_index INTEGER NOT NULL CHECK(area_index >= 0 AND area_index < 50),"
		" tile_x INTEGER NOT NULL,"
		" tile_y INTEGER NOT NULL,"
		" tile_w INTEGER NOT NULL,"
		" tile_h INTEGER NOT NULL,"
		" PRIMARY KEY (map_name, area_index),"
		" FOREIGN KEY (map_name) REFERENCES maps(map_name) ON DELETE CASCADE"
		");"

		// NPC avoid rects
		"CREATE TABLE IF NOT EXISTS map_npc_avoid_rects ("
		" map_name TEXT NOT NULL,"
		" rect_index INTEGER NOT NULL CHECK(rect_index >= 0 AND rect_index < 50),"
		" tile_x INTEGER NOT NULL,"
		" tile_y INTEGER NOT NULL,"
		" tile_w INTEGER NOT NULL,"
		" tile_h INTEGER NOT NULL,"
		" PRIMARY KEY (map_name, rect_index),"
		" FOREIGN KEY (map_name) REFERENCES maps(map_name) ON DELETE CASCADE"
		");"

		// Spot mob generators
		"CREATE TABLE IF NOT EXISTS map_spot_mob_generators ("
		" map_name TEXT NOT NULL,"
		" generator_index INTEGER NOT NULL CHECK(generator_index >= 0 AND generator_index < 100),"
		" generator_type INTEGER NOT NULL CHECK(generator_type IN (1, 2)),"
		" tile_x INTEGER NOT NULL DEFAULT 0,"
		" tile_y INTEGER NOT NULL DEFAULT 0,"
		" tile_w INTEGER NOT NULL DEFAULT 0,"
		" tile_h INTEGER NOT NULL DEFAULT 0,"
		" waypoints TEXT NOT NULL DEFAULT '',"
		" npc_config_id INTEGER NOT NULL,"
		" max_mobs INTEGER NOT NULL,"
		" prob_sa INTEGER NOT NULL DEFAULT 15,"
		" kind_sa INTEGER NOT NULL DEFAULT 1,"
		" PRIMARY KEY (map_name, generator_index),"
		" FOREIGN KEY (map_name) REFERENCES maps(map_name) ON DELETE CASCADE"
		");"

		// Fish points
		"CREATE TABLE IF NOT EXISTS map_fish_points ("
		" map_name TEXT NOT NULL,"
		" point_index INTEGER NOT NULL CHECK(point_index >= 0 AND point_index < 200),"
		" x INTEGER NOT NULL,"
		" y INTEGER NOT NULL,"
		" PRIMARY KEY (map_name, point_index),"
		" FOREIGN KEY (map_name) REFERENCES maps(map_name) ON DELETE CASCADE"
		");"

		// Mineral points
		"CREATE TABLE IF NOT EXISTS map_mineral_points ("
		" map_name TEXT NOT NULL,"
		" point_index INTEGER NOT NULL CHECK(point_index >= 0 AND point_index < 200),"
		" x INTEGER NOT NULL,"
		" y INTEGER NOT NULL,"
		" PRIMARY KEY (map_name, point_index),"
		" FOREIGN KEY (map_name) REFERENCES maps(map_name) ON DELETE CASCADE"
		");"

		// Strategic points
		"CREATE TABLE IF NOT EXISTS map_strategic_points ("
		" map_name TEXT NOT NULL,"
		" point_index INTEGER NOT NULL CHECK(point_index >= 0 AND point_index < 200),"
		" side INTEGER NOT NULL,"
		" point_value INTEGER NOT NULL,"
		" x INTEGER NOT NULL,"
		" y INTEGER NOT NULL,"
		" PRIMARY KEY (map_name, point_index),"
		" FOREIGN KEY (map_name) REFERENCES maps(map_name) ON DELETE CASCADE"
		");"

		// Energy sphere creation points
		"CREATE TABLE IF NOT EXISTS map_energy_sphere_creation ("
		" map_name TEXT NOT NULL,"
		" point_index INTEGER NOT NULL CHECK(point_index >= 0 AND point_index < 10),"
		" sphere_type INTEGER NOT NULL,"
		" x INTEGER NOT NULL,"
		" y INTEGER NOT NULL,"
		" PRIMARY KEY (map_name, point_index),"
		" FOREIGN KEY (map_name) REFERENCES maps(map_name) ON DELETE CASCADE"
		");"

		// Energy sphere goal points
		"CREATE TABLE IF NOT EXISTS map_energy_sphere_goal ("
		" map_name TEXT NOT NULL,"
		" point_index INTEGER NOT NULL CHECK(point_index >= 0 AND point_index < 10),"
		" result INTEGER NOT NULL,"
		" aresden_x INTEGER NOT NULL,"
		" aresden_y INTEGER NOT NULL,"
		" elvine_x INTEGER NOT NULL,"
		" elvine_y INTEGER NOT NULL,"
		" PRIMARY KEY (map_name, point_index),"
		" FOREIGN KEY (map_name) REFERENCES maps(map_name) ON DELETE CASCADE"
		");"

		// Strike points
		"CREATE TABLE IF NOT EXISTS map_strike_points ("
		" map_name TEXT NOT NULL,"
		" point_index INTEGER NOT NULL CHECK(point_index >= 0 AND point_index < 20),"
		" x INTEGER NOT NULL,"
		" y INTEGER NOT NULL,"
		" hp INTEGER NOT NULL,"
		" effect_x1 INTEGER NOT NULL,"
		" effect_y1 INTEGER NOT NULL,"
		" effect_x2 INTEGER NOT NULL,"
		" effect_y2 INTEGER NOT NULL,"
		" effect_x3 INTEGER NOT NULL,"
		" effect_y3 INTEGER NOT NULL,"
		" effect_x4 INTEGER NOT NULL,"
		" effect_y4 INTEGER NOT NULL,"
		" effect_x5 INTEGER NOT NULL,"
		" effect_y5 INTEGER NOT NULL,"
		" related_map_name TEXT NOT NULL CHECK(length(related_map_name) <= 10),"
		" PRIMARY KEY (map_name, point_index),"
		" FOREIGN KEY (map_name) REFERENCES maps(map_name) ON DELETE CASCADE"
		");"

		// Item events
		"CREATE TABLE IF NOT EXISTS map_item_events ("
		" map_name TEXT NOT NULL,"
		" event_index INTEGER NOT NULL CHECK(event_index >= 0 AND event_index < 200),"
		" item_name TEXT NOT NULL CHECK(length(item_name) <= 41),"
		" amount INTEGER NOT NULL,"
		" total_num INTEGER NOT NULL,"
		" event_month INTEGER NOT NULL,"
		" event_day INTEGER NOT NULL,"
		" event_type INTEGER NOT NULL DEFAULT 0,"
		" mob_list TEXT NOT NULL DEFAULT '',"
		" PRIMARY KEY (map_name, event_index),"
		" FOREIGN KEY (map_name) REFERENCES maps(map_name) ON DELETE CASCADE"
		");"

		// Heldenian towers
		"CREATE TABLE IF NOT EXISTS map_heldenian_towers ("
		" map_name TEXT NOT NULL,"
		" tower_index INTEGER NOT NULL CHECK(tower_index >= 0 AND tower_index < 200),"
		" type_id INTEGER NOT NULL,"
		" side INTEGER NOT NULL,"
		" x INTEGER NOT NULL,"
		" y INTEGER NOT NULL,"
		" PRIMARY KEY (map_name, tower_index),"
		" FOREIGN KEY (map_name) REFERENCES maps(map_name) ON DELETE CASCADE"
		");"

		// Heldenian gate doors
		"CREATE TABLE IF NOT EXISTS map_heldenian_gate_doors ("
		" map_name TEXT NOT NULL,"
		" door_index INTEGER NOT NULL CHECK(door_index >= 0 AND door_index < 200),"
		" direction INTEGER NOT NULL,"
		" x INTEGER NOT NULL,"
		" y INTEGER NOT NULL,"
		" PRIMARY KEY (map_name, door_index),"
		" FOREIGN KEY (map_name) REFERENCES maps(map_name) ON DELETE CASCADE"
		");"

		// Static NPCs
		"CREATE TABLE IF NOT EXISTS map_npcs ("
		" id INTEGER PRIMARY KEY AUTOINCREMENT,"
		" map_name TEXT NOT NULL,"
		" npc_config_id INTEGER NOT NULL,"
		" move_type INTEGER NOT NULL,"
		" waypoint_list TEXT NOT NULL DEFAULT '',"
		" name_prefix TEXT NOT NULL DEFAULT '' CHECK(length(name_prefix) <= 1),"
		" FOREIGN KEY (map_name) REFERENCES maps(map_name) ON DELETE CASCADE"
		");"

		// Apocalypse boss config
		"CREATE TABLE IF NOT EXISTS map_apocalypse_boss ("
		" map_name TEXT PRIMARY KEY,"
		" npc_id INTEGER NOT NULL,"
		" tile_x INTEGER NOT NULL,"
		" tile_y INTEGER NOT NULL,"
		" tile_w INTEGER NOT NULL,"
		" tile_h INTEGER NOT NULL,"
		" FOREIGN KEY (map_name) REFERENCES maps(map_name) ON DELETE CASCADE"
		");"

		// Dynamic gate config
		"CREATE TABLE IF NOT EXISTS map_dynamic_gate ("
		" map_name TEXT PRIMARY KEY,"
		" gate_type INTEGER NOT NULL,"
		" tile_x INTEGER NOT NULL,"
		" tile_y INTEGER NOT NULL,"
		" tile_w INTEGER NOT NULL,"
		" tile_h INTEGER NOT NULL,"
		" dest_map TEXT NOT NULL CHECK(length(dest_map) <= 10),"
		" dest_x INTEGER NOT NULL,"
		" dest_y INTEGER NOT NULL,"
		" FOREIGN KEY (map_name) REFERENCES maps(map_name) ON DELETE CASCADE"
		");"

		// Indexes for performance
		"CREATE INDEX IF NOT EXISTS idx_teleport_map ON map_teleport_locations(map_name);"
		"CREATE INDEX IF NOT EXISTS idx_initial_points_map ON map_initial_points(map_name);"
		"CREATE INDEX IF NOT EXISTS idx_waypoints_map ON map_waypoints(map_name);"
		"CREATE INDEX IF NOT EXISTS idx_no_attack_map ON map_no_attack_areas(map_name);"
		"CREATE INDEX IF NOT EXISTS idx_npc_avoid_map ON map_npc_avoid_rects(map_name);"
		"CREATE INDEX IF NOT EXISTS idx_spot_mob_map ON map_spot_mob_generators(map_name);"
		"CREATE INDEX IF NOT EXISTS idx_fish_map ON map_fish_points(map_name);"
		"CREATE INDEX IF NOT EXISTS idx_mineral_map ON map_mineral_points(map_name);"
		"CREATE INDEX IF NOT EXISTS idx_strategic_map ON map_strategic_points(map_name);"
		"CREATE INDEX IF NOT EXISTS idx_sphere_creation_map ON map_energy_sphere_creation(map_name);"
		"CREATE INDEX IF NOT EXISTS idx_sphere_goal_map ON map_energy_sphere_goal(map_name);"
		"CREATE INDEX IF NOT EXISTS idx_strike_map ON map_strike_points(map_name);"
		"CREATE INDEX IF NOT EXISTS idx_item_events_map ON map_item_events(map_name);"
		"CREATE INDEX IF NOT EXISTS idx_heldenian_towers_map ON map_heldenian_towers(map_name);"
		"CREATE INDEX IF NOT EXISTS idx_heldenian_doors_map ON map_heldenian_gate_doors(map_name);"
		"CREATE INDEX IF NOT EXISTS idx_npcs_map ON map_npcs(map_name);"

		"COMMIT;";

	if (!ExecSql(db, schemaSql)) {
		sqlite3_close(db);
		return false;
	}

	*outDb = db;
	if (outCreated != nullptr) {
		*outCreated = created;
	}
	return true;
}

void CloseMapInfoDatabase(sqlite3* db)
{
	if (db != nullptr) {
		sqlite3_close(db);
	}
}

int GetMapNamesFromDatabase(sqlite3* db, char mapNames[][11], int maxMaps)
{
	if (db == nullptr || mapNames == nullptr) {
		return 0;
	}

	const char* sql = "SELECT map_name FROM maps ORDER BY map_name;";
	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		char logMsg[512] = {};
		std::snprintf(logMsg, sizeof(logMsg), "(MAPINFO-SQLITE) GetMapNames prepare failed: %s", sqlite3_errmsg(db));
		PutLogList(logMsg);
		return 0;
	}

	int count = 0;
	while (sqlite3_step(stmt) == SQLITE_ROW && count < maxMaps) {
		CopyColumnText(stmt, 0, mapNames[count], 11);
		count++;
	}

	sqlite3_finalize(stmt);
	return count;
}

bool LoadMapBaseSettings(sqlite3* db, const char* mapName, CMap* pMap)
{
	if (db == nullptr || mapName == nullptr || pMap == nullptr) {
		return false;
	}

	const char* sql =
		"SELECT location_name, maximum_object, level_limit, upper_level_limit, map_type,"
		" random_mob_generator_enabled, random_mob_generator_level,"
		" mineral_generator_enabled, mineral_generator_level,"
		" max_fish, max_mineral, fixed_day_mode, recall_impossible,"
		" apocalypse_map, apocalypse_mob_gen_type, citizen_limit, is_fight_zone,"
		" heldenian_map, heldenian_mode_map, mob_event_amount, energy_sphere_auto_creation, pk_mode,"
		" attack_enabled"
		" FROM maps WHERE map_name = ? COLLATE NOCASE;";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return false;
	}

	PrepareAndBindText(stmt, 1, mapName);

	bool ok = false;
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		int col = 0;
		CopyColumnText(stmt, col++, pMap->m_cLocationName, sizeof(pMap->m_cLocationName));
		pMap->m_iMaximumObject = sqlite3_column_int(stmt, col++);
		pMap->m_iLevelLimit = sqlite3_column_int(stmt, col++);
		pMap->m_iUpperLevelLimit = sqlite3_column_int(stmt, col++);
		pMap->m_cType = static_cast<char>(sqlite3_column_int(stmt, col++));
		pMap->m_bRandomMobGenerator = sqlite3_column_int(stmt, col++) != 0;
		pMap->m_cRandomMobGeneratorLevel = static_cast<char>(sqlite3_column_int(stmt, col++));
		pMap->m_bMineralGenerator = sqlite3_column_int(stmt, col++) != 0;
		pMap->m_cMineralGeneratorLevel = static_cast<char>(sqlite3_column_int(stmt, col++));
		pMap->m_iMaxFish = sqlite3_column_int(stmt, col++);
		pMap->m_iMaxMineral = sqlite3_column_int(stmt, col++);
		pMap->m_bIsFixedDayMode = sqlite3_column_int(stmt, col++) != 0;
		pMap->m_bIsRecallImpossible = sqlite3_column_int(stmt, col++) != 0;
		pMap->m_bIsApocalypseMap = sqlite3_column_int(stmt, col++) != 0;
		pMap->m_iApocalypseMobGenType = sqlite3_column_int(stmt, col++);
		pMap->m_bIsCitizenLimit = sqlite3_column_int(stmt, col++) != 0;
		pMap->m_bIsFightZone = sqlite3_column_int(stmt, col++) != 0;
		pMap->m_bIsHeldenianMap = sqlite3_column_int(stmt, col++) != 0;
		pMap->m_cHeldenianModeMap = static_cast<char>(sqlite3_column_int(stmt, col++));
		pMap->sMobEventAmount = static_cast<short>(sqlite3_column_int(stmt, col++));
		pMap->m_bIsEnergySphereAutoCreation = sqlite3_column_int(stmt, col++) != 0;
		col++; // pk_mode - read but no direct member
		pMap->m_bIsAttackEnabled = sqlite3_column_int(stmt, col++) != 0;
		ok = true;
	}

	sqlite3_finalize(stmt);
	return ok;
}

bool LoadMapTeleportLocations(sqlite3* db, const char* mapName, CMap* pMap)
{
	if (db == nullptr || mapName == nullptr || pMap == nullptr) {
		return false;
	}

	const char* sql =
		"SELECT teleport_index, src_x, src_y, dest_map_name, dest_x, dest_y, direction"
		" FROM map_teleport_locations WHERE map_name = ? COLLATE NOCASE ORDER BY teleport_index;";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return false;
	}

	PrepareAndBindText(stmt, 1, mapName);

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int idx = sqlite3_column_int(stmt, 0);
		if (idx < 0 || idx >= hb::server::map::MaxTeleportLoc) continue;

		if (pMap->m_pTeleportLoc[idx] == nullptr) {
			pMap->m_pTeleportLoc[idx] = new CTeleportLoc;
		}

		CTeleportLoc* pTele = pMap->m_pTeleportLoc[idx];
		pTele->m_sSrcX = static_cast<short>(sqlite3_column_int(stmt, 1));
		pTele->m_sSrcY = static_cast<short>(sqlite3_column_int(stmt, 2));
		CopyColumnText(stmt, 3, pTele->m_cDestMapName, sizeof(pTele->m_cDestMapName));
		pTele->m_sDestX = static_cast<short>(sqlite3_column_int(stmt, 4));
		pTele->m_sDestY = static_cast<short>(sqlite3_column_int(stmt, 5));
		pTele->m_cDir = static_cast<char>(sqlite3_column_int(stmt, 6));
	}

	sqlite3_finalize(stmt);
	return true;
}

bool LoadMapInitialPoints(sqlite3* db, const char* mapName, CMap* pMap)
{
	if (db == nullptr || mapName == nullptr || pMap == nullptr) {
		return false;
	}

	const char* sql =
		"SELECT point_index, x, y FROM map_initial_points WHERE map_name = ? COLLATE NOCASE ORDER BY point_index;";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return false;
	}

	PrepareAndBindText(stmt, 1, mapName);

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int idx = sqlite3_column_int(stmt, 0);
		if (idx < 0 || idx >= hb::server::map::MaxInitialPoint) continue;

		pMap->m_pInitialPoint[idx].x = sqlite3_column_int(stmt, 1);
		pMap->m_pInitialPoint[idx].y = sqlite3_column_int(stmt, 2);
	}

	sqlite3_finalize(stmt);
	return true;
}

bool LoadMapWaypoints(sqlite3* db, const char* mapName, CMap* pMap)
{
	if (db == nullptr || mapName == nullptr || pMap == nullptr) {
		return false;
	}

	const char* sql =
		"SELECT waypoint_index, x, y FROM map_waypoints WHERE map_name = ? COLLATE NOCASE ORDER BY waypoint_index;";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return false;
	}

	PrepareAndBindText(stmt, 1, mapName);

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int idx = sqlite3_column_int(stmt, 0);
		if (idx < 0 || idx >= hb::server::map::MaxWaypointCfg) continue;

		pMap->m_WaypointList[idx].x = sqlite3_column_int(stmt, 1);
		pMap->m_WaypointList[idx].y = sqlite3_column_int(stmt, 2);
	}

	sqlite3_finalize(stmt);
	return true;
}

bool LoadMapNoAttackAreas(sqlite3* db, const char* mapName, CMap* pMap)
{
	if (db == nullptr || mapName == nullptr || pMap == nullptr) {
		return false;
	}

	const char* sql =
		"SELECT area_index, tile_x, tile_y, tile_w, tile_h"
		" FROM map_no_attack_areas WHERE map_name = ? COLLATE NOCASE ORDER BY area_index;";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return false;
	}

	PrepareAndBindText(stmt, 1, mapName);

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int idx = sqlite3_column_int(stmt, 0);
		if (idx < 0 || idx >= hb::server::map::MaxNmr) continue;

		pMap->m_rcNoAttackRect[idx] = hb::shared::geometry::GameRectangle(
			sqlite3_column_int(stmt, 1),
			sqlite3_column_int(stmt, 2),
			sqlite3_column_int(stmt, 3),
			sqlite3_column_int(stmt, 4));
	}

	sqlite3_finalize(stmt);
	return true;
}

bool LoadMapNpcAvoidRects(sqlite3* db, const char* mapName, CMap* pMap)
{
	if (db == nullptr || mapName == nullptr || pMap == nullptr) {
		return false;
	}

	const char* sql =
		"SELECT rect_index, tile_x, tile_y, tile_w, tile_h"
		" FROM map_npc_avoid_rects WHERE map_name = ? COLLATE NOCASE ORDER BY rect_index;";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return false;
	}

	PrepareAndBindText(stmt, 1, mapName);

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int idx = sqlite3_column_int(stmt, 0);
		if (idx < 0 || idx >= hb::server::map::MaxMgar) continue;

		pMap->m_rcMobGenAvoidRect[idx] = hb::shared::geometry::GameRectangle(
			sqlite3_column_int(stmt, 1),
			sqlite3_column_int(stmt, 2),
			sqlite3_column_int(stmt, 3),
			sqlite3_column_int(stmt, 4));
	}

	sqlite3_finalize(stmt);
	return true;
}

bool LoadMapSpotMobGenerators(sqlite3* db, const char* mapName, CMap* pMap)
{
	if (db == nullptr || mapName == nullptr || pMap == nullptr) {
		return false;
	}

	const char* sql =
		"SELECT generator_index, generator_type, tile_x, tile_y, tile_w, tile_h,"
		" waypoints, npc_config_id, max_mobs, prob_sa, kind_sa"
		" FROM map_spot_mob_generators WHERE map_name = ? COLLATE NOCASE ORDER BY generator_index;";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return false;
	}

	PrepareAndBindText(stmt, 1, mapName);

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int idx = sqlite3_column_int(stmt, 0);
		if (idx < 0 || idx >= hb::server::map::MaxSpotMobGenerator) continue;

		pMap->m_stSpotMobGenerator[idx].bDefined = true;
		pMap->m_stSpotMobGenerator[idx].cType = static_cast<char>(sqlite3_column_int(stmt, 1));
		pMap->m_stSpotMobGenerator[idx].rcRect = hb::shared::geometry::GameRectangle(
			sqlite3_column_int(stmt, 2),
			sqlite3_column_int(stmt, 3),
			sqlite3_column_int(stmt, 4),
			sqlite3_column_int(stmt, 5));

		// Parse waypoints for type 2
		char waypointStr[256] = {};
		CopyColumnText(stmt, 6, waypointStr, sizeof(waypointStr));
		ParseWaypointList(waypointStr, pMap->m_stSpotMobGenerator[idx].cWaypoint, 10);

		pMap->m_stSpotMobGenerator[idx].iNpcConfigId = sqlite3_column_int(stmt, 7);
		pMap->m_stSpotMobGenerator[idx].iMaxMobs = sqlite3_column_int(stmt, 8);
		pMap->m_stSpotMobGenerator[idx].iProbSA = sqlite3_column_int(stmt, 9);
		pMap->m_stSpotMobGenerator[idx].iKindSA = sqlite3_column_int(stmt, 10);
		pMap->m_stSpotMobGenerator[idx].iCurMobs = 0;
		pMap->m_stSpotMobGenerator[idx].iTotalActiveMob = 0;
	}

	sqlite3_finalize(stmt);
	return true;
}

bool LoadMapFishPoints(sqlite3* db, const char* mapName, CMap* pMap)
{
	if (db == nullptr || mapName == nullptr || pMap == nullptr) {
		return false;
	}

	const char* sql =
		"SELECT point_index, x, y FROM map_fish_points WHERE map_name = ? COLLATE NOCASE ORDER BY point_index;";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return false;
	}

	PrepareAndBindText(stmt, 1, mapName);

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int idx = sqlite3_column_int(stmt, 0);
		if (idx < 0 || idx >= hb::server::map::MaxFishPoint) continue;

		pMap->m_FishPointList[idx].x = sqlite3_column_int(stmt, 1);
		pMap->m_FishPointList[idx].y = sqlite3_column_int(stmt, 2);
		pMap->m_iTotalFishPoint++;
	}

	sqlite3_finalize(stmt);
	return true;
}

bool LoadMapMineralPoints(sqlite3* db, const char* mapName, CMap* pMap)
{
	if (db == nullptr || mapName == nullptr || pMap == nullptr) {
		return false;
	}

	const char* sql =
		"SELECT point_index, x, y FROM map_mineral_points WHERE map_name = ? COLLATE NOCASE ORDER BY point_index;";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return false;
	}

	PrepareAndBindText(stmt, 1, mapName);

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int idx = sqlite3_column_int(stmt, 0);
		if (idx < 0 || idx >= hb::server::map::MaxMineralPoint) continue;

		pMap->m_MineralPointList[idx].x = sqlite3_column_int(stmt, 1);
		pMap->m_MineralPointList[idx].y = sqlite3_column_int(stmt, 2);
		pMap->m_iTotalMineralPoint++;
	}

	sqlite3_finalize(stmt);
	return true;
}

bool LoadMapStrategicPoints(sqlite3* db, const char* mapName, CMap* pMap)
{
	if (db == nullptr || mapName == nullptr || pMap == nullptr) {
		return false;
	}

	const char* sql =
		"SELECT point_index, side, point_value, x, y"
		" FROM map_strategic_points WHERE map_name = ? COLLATE NOCASE ORDER BY point_index;";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return false;
	}

	PrepareAndBindText(stmt, 1, mapName);

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int idx = sqlite3_column_int(stmt, 0);
		if (idx < 0 || idx >= hb::server::map::MaxStrategicPoints) continue;

		if (pMap->m_pStrategicPointList[idx] == nullptr) {
			pMap->m_pStrategicPointList[idx] = new CStrategicPoint;
		}

		pMap->m_pStrategicPointList[idx]->m_iSide = sqlite3_column_int(stmt, 1);
		pMap->m_pStrategicPointList[idx]->m_iValue = sqlite3_column_int(stmt, 2);
		pMap->m_pStrategicPointList[idx]->m_iX = sqlite3_column_int(stmt, 3);
		pMap->m_pStrategicPointList[idx]->m_iY = sqlite3_column_int(stmt, 4);
	}

	sqlite3_finalize(stmt);
	return true;
}

bool LoadMapEnergySphereCreationPoints(sqlite3* db, const char* mapName, CMap* pMap)
{
	if (db == nullptr || mapName == nullptr || pMap == nullptr) {
		return false;
	}

	const char* sql =
		"SELECT point_index, sphere_type, x, y"
		" FROM map_energy_sphere_creation WHERE map_name = ? COLLATE NOCASE ORDER BY point_index;";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return false;
	}

	PrepareAndBindText(stmt, 1, mapName);

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int idx = sqlite3_column_int(stmt, 0);
		if (idx < 0 || idx >= hb::server::map::MaxEnergySpheres) continue;

		pMap->m_stEnergySphereCreationList[idx].cType = static_cast<char>(sqlite3_column_int(stmt, 1));
		pMap->m_stEnergySphereCreationList[idx].sX = sqlite3_column_int(stmt, 2);
		pMap->m_stEnergySphereCreationList[idx].sY = sqlite3_column_int(stmt, 3);
		pMap->m_iTotalEnergySphereCreationPoint++;
	}

	sqlite3_finalize(stmt);
	return true;
}

bool LoadMapEnergySphereGoalPoints(sqlite3* db, const char* mapName, CMap* pMap)
{
	if (db == nullptr || mapName == nullptr || pMap == nullptr) {
		return false;
	}

	const char* sql =
		"SELECT point_index, result, aresden_x, aresden_y, elvine_x, elvine_y"
		" FROM map_energy_sphere_goal WHERE map_name = ? COLLATE NOCASE ORDER BY point_index;";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return false;
	}

	PrepareAndBindText(stmt, 1, mapName);

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int idx = sqlite3_column_int(stmt, 0);
		if (idx < 0 || idx >= hb::server::map::MaxEnergySpheres) continue;

		pMap->m_stEnergySphereGoalList[idx].cResult = static_cast<char>(sqlite3_column_int(stmt, 1));
		pMap->m_stEnergySphereGoalList[idx].aresdenX = sqlite3_column_int(stmt, 2);
		pMap->m_stEnergySphereGoalList[idx].aresdenY = sqlite3_column_int(stmt, 3);
		pMap->m_stEnergySphereGoalList[idx].elvineX = sqlite3_column_int(stmt, 4);
		pMap->m_stEnergySphereGoalList[idx].elvineY = sqlite3_column_int(stmt, 5);
		pMap->m_iTotalEnergySphereGoalPoint++;
	}

	sqlite3_finalize(stmt);
	return true;
}

bool LoadMapStrikePoints(sqlite3* db, const char* mapName, CMap* pMap)
{
	if (db == nullptr || mapName == nullptr || pMap == nullptr) {
		return false;
	}

	const char* sql =
		"SELECT point_index, x, y, hp, effect_x1, effect_y1, effect_x2, effect_y2,"
		" effect_x3, effect_y3, effect_x4, effect_y4, effect_x5, effect_y5, related_map_name"
		" FROM map_strike_points WHERE map_name = ? COLLATE NOCASE ORDER BY point_index;";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return false;
	}

	PrepareAndBindText(stmt, 1, mapName);

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int idx = sqlite3_column_int(stmt, 0);
		if (idx < 0 || idx >= hb::server::map::MaxStrikePoints) continue;

		pMap->m_stStrikePoint[idx].dX = sqlite3_column_int(stmt, 1);
		pMap->m_stStrikePoint[idx].dY = sqlite3_column_int(stmt, 2);
		pMap->m_stStrikePoint[idx].iHP = sqlite3_column_int(stmt, 3);
		pMap->m_stStrikePoint[idx].iInitHP = pMap->m_stStrikePoint[idx].iHP;
		pMap->m_stStrikePoint[idx].iEffectX[0] = sqlite3_column_int(stmt, 4);
		pMap->m_stStrikePoint[idx].iEffectY[0] = sqlite3_column_int(stmt, 5);
		pMap->m_stStrikePoint[idx].iEffectX[1] = sqlite3_column_int(stmt, 6);
		pMap->m_stStrikePoint[idx].iEffectY[1] = sqlite3_column_int(stmt, 7);
		pMap->m_stStrikePoint[idx].iEffectX[2] = sqlite3_column_int(stmt, 8);
		pMap->m_stStrikePoint[idx].iEffectY[2] = sqlite3_column_int(stmt, 9);
		pMap->m_stStrikePoint[idx].iEffectX[3] = sqlite3_column_int(stmt, 10);
		pMap->m_stStrikePoint[idx].iEffectY[3] = sqlite3_column_int(stmt, 11);
		pMap->m_stStrikePoint[idx].iEffectX[4] = sqlite3_column_int(stmt, 12);
		pMap->m_stStrikePoint[idx].iEffectY[4] = sqlite3_column_int(stmt, 13);
		CopyColumnText(stmt, 14, pMap->m_stStrikePoint[idx].cRelatedMapName,
			sizeof(pMap->m_stStrikePoint[idx].cRelatedMapName));
	}

	sqlite3_finalize(stmt);
	return true;
}

bool LoadMapItemEvents(sqlite3* db, const char* mapName, CMap* pMap)
{
	if (db == nullptr || mapName == nullptr || pMap == nullptr) {
		return false;
	}

	const char* sql =
		"SELECT event_index, item_name, amount, total_num, event_month, event_day, event_type, mob_list"
		" FROM map_item_events WHERE map_name = ? COLLATE NOCASE ORDER BY event_index;";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return false;
	}

	PrepareAndBindText(stmt, 1, mapName);

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int idx = sqlite3_column_int(stmt, 0);
		if (idx < 0 || idx >= hb::server::map::MaxItemEvents) continue;

		CopyColumnText(stmt, 1, pMap->m_stItemEventList[idx].cItemName,
			sizeof(pMap->m_stItemEventList[idx].cItemName));
		pMap->m_stItemEventList[idx].iAmount = sqlite3_column_int(stmt, 2);
		pMap->m_stItemEventList[idx].iTotalNum = sqlite3_column_int(stmt, 3);
		pMap->m_stItemEventList[idx].iMonth = sqlite3_column_int(stmt, 4);
		pMap->m_stItemEventList[idx].iDay = sqlite3_column_int(stmt, 5);
		// event_type (column 6) is not stored in CMap struct
		// mob_list (column 7) could be parsed into separate mob fields if needed
		pMap->m_iTotalItemEvents++;
	}

	sqlite3_finalize(stmt);
	return true;
}

bool LoadMapHeldenianTowers(sqlite3* db, const char* mapName, CMap* pMap)
{
	if (db == nullptr || mapName == nullptr || pMap == nullptr) {
		return false;
	}

	const char* sql =
		"SELECT tower_index, type_id, side, x, y"
		" FROM map_heldenian_towers WHERE map_name = ? COLLATE NOCASE ORDER BY tower_index;";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return false;
	}

	PrepareAndBindText(stmt, 1, mapName);

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int idx = sqlite3_column_int(stmt, 0);
		if (idx < 0 || idx >= hb::server::map::MaxHeldenianTower) continue;

		pMap->m_stHeldenianTower[idx].sTypeID = static_cast<short>(sqlite3_column_int(stmt, 1));
		pMap->m_stHeldenianTower[idx].cSide = static_cast<char>(sqlite3_column_int(stmt, 2));
		pMap->m_stHeldenianTower[idx].dX = static_cast<short>(sqlite3_column_int(stmt, 3));
		pMap->m_stHeldenianTower[idx].dY = static_cast<short>(sqlite3_column_int(stmt, 4));
	}

	sqlite3_finalize(stmt);
	return true;
}

bool LoadMapHeldenianGateDoors(sqlite3* db, const char* mapName, CMap* pMap)
{
	if (db == nullptr || mapName == nullptr || pMap == nullptr) {
		return false;
	}

	const char* sql =
		"SELECT door_index, direction, x, y"
		" FROM map_heldenian_gate_doors WHERE map_name = ? COLLATE NOCASE ORDER BY door_index;";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return false;
	}

	PrepareAndBindText(stmt, 1, mapName);

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int idx = sqlite3_column_int(stmt, 0);
		if (idx < 0 || idx >= hb::server::map::MaxHeldenianDoor) continue;

		pMap->m_stHeldenianGateDoor[idx].cDir = static_cast<char>(sqlite3_column_int(stmt, 1));
		pMap->m_stHeldenianGateDoor[idx].dX = static_cast<short>(sqlite3_column_int(stmt, 2));
		pMap->m_stHeldenianGateDoor[idx].dY = static_cast<short>(sqlite3_column_int(stmt, 3));
	}

	sqlite3_finalize(stmt);
	return true;
}

bool LoadMapNpcs(sqlite3* db, const char* mapName, CMap* pMap)
{
	// NPCs are handled differently - they are spawned by CGame, not stored in CMap directly
	// This function is a placeholder for future NPC spawning from database
	return true;
}

bool LoadMapApocalypseBoss(sqlite3* db, const char* mapName, CMap* pMap)
{
	if (db == nullptr || mapName == nullptr || pMap == nullptr) {
		return false;
	}

	const char* sql =
		"SELECT npc_id, tile_x, tile_y, tile_w, tile_h"
		" FROM map_apocalypse_boss WHERE map_name = ? COLLATE NOCASE;";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return false;
	}

	PrepareAndBindText(stmt, 1, mapName);

	if (sqlite3_step(stmt) == SQLITE_ROW) {
		pMap->m_iApocalypseBossMobNpcID = sqlite3_column_int(stmt, 0);
		pMap->m_rcApocalypseBossMob = hb::shared::geometry::GameRectangle(
			sqlite3_column_int(stmt, 1),
			sqlite3_column_int(stmt, 2),
			sqlite3_column_int(stmt, 3),
			sqlite3_column_int(stmt, 4));
	}

	sqlite3_finalize(stmt);
	return true;
}

bool LoadMapDynamicGate(sqlite3* db, const char* mapName, CMap* pMap)
{
	if (db == nullptr || mapName == nullptr || pMap == nullptr) {
		return false;
	}

	const char* sql =
		"SELECT gate_type, tile_x, tile_y, tile_w, tile_h, dest_map, dest_x, dest_y"
		" FROM map_dynamic_gate WHERE map_name = ? COLLATE NOCASE;";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return false;
	}

	PrepareAndBindText(stmt, 1, mapName);

	if (sqlite3_step(stmt) == SQLITE_ROW) {
		pMap->m_cDynamicGateType = static_cast<char>(sqlite3_column_int(stmt, 0));
		pMap->m_rcDynamicGateCoord = hb::shared::geometry::GameRectangle(
			sqlite3_column_int(stmt, 1),
			sqlite3_column_int(stmt, 2),
			sqlite3_column_int(stmt, 3),
			sqlite3_column_int(stmt, 4));
		CopyColumnText(stmt, 5, pMap->m_cDynamicGateCoordDestMap,
			sizeof(pMap->m_cDynamicGateCoordDestMap));
		pMap->m_sDynamicGateCoordTgtX = static_cast<short>(sqlite3_column_int(stmt, 6));
		pMap->m_sDynamicGateCoordTgtY = static_cast<short>(sqlite3_column_int(stmt, 7));
	}

	sqlite3_finalize(stmt);
	return true;
}

bool LoadMapConfig(sqlite3* db, const char* mapName, CMap* pMap)
{
	if (db == nullptr || mapName == nullptr || pMap == nullptr) {
		return false;
	}

	// Load all components
	if (!LoadMapBaseSettings(db, mapName, pMap)) {
		char logMsg[256];
		std::snprintf(logMsg, sizeof(logMsg), "(MAPINFO-SQLITE) Failed to load base settings for map: %s", mapName);
		PutLogList(logMsg);
		return false;
	}

	// Load child tables - these are non-critical, log but don't fail
	LoadMapTeleportLocations(db, mapName, pMap);
	LoadMapInitialPoints(db, mapName, pMap);
	LoadMapWaypoints(db, mapName, pMap);
	LoadMapNoAttackAreas(db, mapName, pMap);
	LoadMapNpcAvoidRects(db, mapName, pMap);
	LoadMapSpotMobGenerators(db, mapName, pMap);
	LoadMapFishPoints(db, mapName, pMap);
	LoadMapMineralPoints(db, mapName, pMap);
	LoadMapStrategicPoints(db, mapName, pMap);
	LoadMapEnergySphereCreationPoints(db, mapName, pMap);
	LoadMapEnergySphereGoalPoints(db, mapName, pMap);
	LoadMapStrikePoints(db, mapName, pMap);
	LoadMapItemEvents(db, mapName, pMap);
	LoadMapHeldenianTowers(db, mapName, pMap);
	LoadMapHeldenianGateDoors(db, mapName, pMap);
	LoadMapNpcs(db, mapName, pMap);
	LoadMapApocalypseBoss(db, mapName, pMap);
	LoadMapDynamicGate(db, mapName, pMap);

	return true;
}

bool SaveMapConfig(sqlite3* db, const CMap* pMap)
{
	// TODO: Implement save functionality for admin tools
	return false;
}

bool SaveAllMapConfigs(sqlite3* db, CMap** pMapList, int mapCount)
{
	// TODO: Implement save functionality for admin tools
	return false;
}

bool HasMapInfoRows(sqlite3* db, const char* tableName)
{
	if (db == nullptr || tableName == nullptr) {
		return false;
	}

	char sql[256] = {};
	std::snprintf(sql, sizeof(sql), "SELECT COUNT(*) FROM %s;", tableName);

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return false;
	}

	bool hasRows = false;
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		hasRows = sqlite3_column_int(stmt, 0) > 0;
	}

	sqlite3_finalize(stmt);
	return hasRows;
}
