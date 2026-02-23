/*
 Navicat Premium Data Transfer

 Source Server         : GameConfigs
 Source Server Type    : SQLite
 Source Server Version : 3035005 (3.35.5)
 Source Schema         : main

 Target Server Type    : SQLite
 Target Server Version : 3035005 (3.35.5)
 File Encoding         : 65001

 Date: 16/02/2026 13:22:32
*/

PRAGMA foreign_keys = false;

-- ----------------------------
-- Table structure for active_maps
-- ----------------------------
DROP TABLE IF EXISTS "active_maps";
CREATE TABLE "active_maps" (
  "map_index" INTEGER,
  "map_name" TEXT NOT NULL,
  "active" INTEGER NOT NULL DEFAULT 0,
  PRIMARY KEY ("map_index")
);

-- ----------------------------
-- Records of active_maps
-- ----------------------------
INSERT INTO "active_maps" VALUES (0, 'default', 1);
INSERT INTO "active_maps" VALUES (1, 'inferniab', 1);
INSERT INTO "active_maps" VALUES (2, 'inferniaa', 1);
INSERT INTO "active_maps" VALUES (3, 'maze', 1);
INSERT INTO "active_maps" VALUES (4, 'druncncity', 1);
INSERT INTO "active_maps" VALUES (5, 'procella', 1);
INSERT INTO "active_maps" VALUES (6, 'abaddon', 1);
INSERT INTO "active_maps" VALUES (7, 'aresden', 1);
INSERT INTO "active_maps" VALUES (8, 'arefarm', 1);
INSERT INTO "active_maps" VALUES (9, 'aresdend1', 1);
INSERT INTO "active_maps" VALUES (10, 'arebrk11', 1);
INSERT INTO "active_maps" VALUES (11, 'arebrk12', 1);
INSERT INTO "active_maps" VALUES (12, 'arebrk21', 1);
INSERT INTO "active_maps" VALUES (13, 'arebrk22', 1);
INSERT INTO "active_maps" VALUES (14, 'wrhus_1', 1);
INSERT INTO "active_maps" VALUES (15, 'cityhall_1', 1);
INSERT INTO "active_maps" VALUES (16, 'arewrhus', 1);
INSERT INTO "active_maps" VALUES (17, 'resurr1', 1);
INSERT INTO "active_maps" VALUES (18, 'gshop_1', 1);
INSERT INTO "active_maps" VALUES (19, 'arejail', 1);
INSERT INTO "active_maps" VALUES (20, 'cath_1', 1);
INSERT INTO "active_maps" VALUES (21, 'wzdtwr_1', 1);
INSERT INTO "active_maps" VALUES (22, 'gshop_1f', 1);
INSERT INTO "active_maps" VALUES (23, 'wrhus_1f', 1);
INSERT INTO "active_maps" VALUES (24, 'bsmith_1f', 1);
INSERT INTO "active_maps" VALUES (25, 'bsmith_1', 1);
INSERT INTO "active_maps" VALUES (26, 'gldhall_1', 1);
INSERT INTO "active_maps" VALUES (27, 'cmdhall_1', 1);
INSERT INTO "active_maps" VALUES (28, 'dglv2', 1);
INSERT INTO "active_maps" VALUES (29, 'dglv3', 1);
INSERT INTO "active_maps" VALUES (30, 'dglv4', 1);
INSERT INTO "active_maps" VALUES (31, 'elvine', 1);
INSERT INTO "active_maps" VALUES (32, 'elvfarm', 1);
INSERT INTO "active_maps" VALUES (33, 'elvbrk11', 1);
INSERT INTO "active_maps" VALUES (34, 'elvbrk12', 1);
INSERT INTO "active_maps" VALUES (35, 'elvbrk21', 1);
INSERT INTO "active_maps" VALUES (36, 'elvbrk22', 1);
INSERT INTO "active_maps" VALUES (37, 'elvined1', 1);
INSERT INTO "active_maps" VALUES (38, 'elvjail', 1);
INSERT INTO "active_maps" VALUES (39, 'elvwrhus', 1);
INSERT INTO "active_maps" VALUES (40, 'gldhall_2', 1);
INSERT INTO "active_maps" VALUES (41, 'gshop_2', 1);
INSERT INTO "active_maps" VALUES (42, 'gshop_2f', 1);
INSERT INTO "active_maps" VALUES (43, 'resurr2', 1);
INSERT INTO "active_maps" VALUES (44, 'wrhus_2', 1);
INSERT INTO "active_maps" VALUES (45, 'wrhus_2f', 1);
INSERT INTO "active_maps" VALUES (46, 'wzdtwr_2', 1);
INSERT INTO "active_maps" VALUES (47, 'bsmith_2', 1);
INSERT INTO "active_maps" VALUES (48, 'bsmith_2f', 1);
INSERT INTO "active_maps" VALUES (49, 'cath_2', 1);
INSERT INTO "active_maps" VALUES (50, 'cityhall_2', 1);
INSERT INTO "active_maps" VALUES (51, 'cmdhall_2', 1);
INSERT INTO "active_maps" VALUES (52, 'btfield', 1);
INSERT INTO "active_maps" VALUES (53, 'godh', 1);
INSERT INTO "active_maps" VALUES (54, 'hrampart', 1);
INSERT INTO "active_maps" VALUES (55, 'areuni', 1);
INSERT INTO "active_maps" VALUES (56, 'elvuni', 1);
INSERT INTO "active_maps" VALUES (57, 'huntzone1', 1);
INSERT INTO "active_maps" VALUES (58, 'huntzone2', 1);
INSERT INTO "active_maps" VALUES (59, 'huntzone3', 1);
INSERT INTO "active_maps" VALUES (60, 'huntzone4', 1);
INSERT INTO "active_maps" VALUES (61, 'middleland', 1);
INSERT INTO "active_maps" VALUES (62, '2ndmiddle', 1);
INSERT INTO "active_maps" VALUES (63, 'middled1n', 1);
INSERT INTO "active_maps" VALUES (64, 'middled1x', 1);
INSERT INTO "active_maps" VALUES (65, 'bisle', 1);
INSERT INTO "active_maps" VALUES (66, 'fightzone1', 1);
INSERT INTO "active_maps" VALUES (67, 'fightzone2', 1);
INSERT INTO "active_maps" VALUES (68, 'fightzone3', 1);
INSERT INTO "active_maps" VALUES (69, 'fightzone4', 1);
INSERT INTO "active_maps" VALUES (70, 'fightzone5', 1);
INSERT INTO "active_maps" VALUES (71, 'fightzone6', 1);
INSERT INTO "active_maps" VALUES (72, 'fightzone7', 1);
INSERT INTO "active_maps" VALUES (73, 'fightzone8', 1);
INSERT INTO "active_maps" VALUES (74, 'fightzone9', 1);
INSERT INTO "active_maps" VALUES (75, 'toh1', 1);
INSERT INTO "active_maps" VALUES (76, 'toh2', 1);
INSERT INTO "active_maps" VALUES (77, 'toh3', 1);

-- ----------------------------
-- Table structure for admin_command_permissions
-- ----------------------------
DROP TABLE IF EXISTS "admin_command_permissions";
CREATE TABLE "admin_command_permissions" (
  "command" TEXT COLLATE NOCASE,
  "admin_level" INTEGER NOT NULL DEFAULT 1000,
  "description" TEXT NOT NULL DEFAULT '',
  PRIMARY KEY ("command")
);

-- ----------------------------
-- Records of admin_command_permissions
-- ----------------------------
INSERT INTO "admin_command_permissions" VALUES ('to', 0, 'Whisper for players');
INSERT INTO "admin_command_permissions" VALUES ('regen', 101, 'Regenerates HP/MP/SP/Hunger for self or desired player');
INSERT INTO "admin_command_permissions" VALUES ('block', 0, 'Blocking players');
INSERT INTO "admin_command_permissions" VALUES ('unblock', 0, 'Unblocking players');
INSERT INTO "admin_command_permissions" VALUES ('gm', 100, 'Enable/Disable GM Mode');
INSERT INTO "admin_command_permissions" VALUES ('createitem', 1000, 'Creates an item for the caller');
INSERT INTO "admin_command_permissions" VALUES ('giveitem', 1000, 'Gives an item to the designated player');
INSERT INTO "admin_command_permissions" VALUES ('spawn', 500, 'Spawns an entity by ID');
INSERT INTO "admin_command_permissions" VALUES ('goto', 100, 'Teleports caller to a player, online or offline');
INSERT INTO "admin_command_permissions" VALUES ('come', 101, 'Forces a player to teleport to you.');
INSERT INTO "admin_command_permissions" VALUES ('invis', 100, 'GM Invis, unable to be detected or seen');

-- ----------------------------
-- Table structure for admins
-- ----------------------------
DROP TABLE IF EXISTS "admins";
CREATE TABLE "admins" (
  "account_name" TEXT,
  "character_name" TEXT NOT NULL,
  "approved_ip" TEXT NOT NULL DEFAULT '0.0.0.0',
  "admin_level" INTEGER NOT NULL DEFAULT 1,
  PRIMARY KEY ("account_name")
);

-- ----------------------------
-- Records of admins
-- ----------------------------

-- ----------------------------
-- Table structure for banned_list
-- ----------------------------
DROP TABLE IF EXISTS "banned_list";
CREATE TABLE "banned_list" (
  "ip_address" TEXT,
  PRIMARY KEY ("ip_address")
);

-- ----------------------------
-- Records of banned_list
-- ----------------------------
INSERT INTO "banned_list" VALUES ('192.118.1.1');

-- ----------------------------
-- Table structure for builditem_configs
-- ----------------------------
DROP TABLE IF EXISTS "builditem_configs";
CREATE TABLE "builditem_configs" (
  "build_id" INTEGER,
  "name" TEXT NOT NULL,
  "skill_limit" INTEGER NOT NULL,
  "material_id1" INTEGER NOT NULL,
  "material_count1" INTEGER NOT NULL,
  "material_value1" INTEGER NOT NULL,
  "material_id2" INTEGER NOT NULL,
  "material_count2" INTEGER NOT NULL,
  "material_value2" INTEGER NOT NULL,
  "material_id3" INTEGER NOT NULL,
  "material_count3" INTEGER NOT NULL,
  "material_value3" INTEGER NOT NULL,
  "material_id4" INTEGER NOT NULL,
  "material_count4" INTEGER NOT NULL,
  "material_value4" INTEGER NOT NULL,
  "material_id5" INTEGER NOT NULL,
  "material_count5" INTEGER NOT NULL,
  "material_value5" INTEGER NOT NULL,
  "material_id6" INTEGER NOT NULL,
  "material_count6" INTEGER NOT NULL,
  "material_value6" INTEGER NOT NULL,
  "average_value" INTEGER NOT NULL,
  "max_skill" INTEGER NOT NULL,
  "attribute" INTEGER NOT NULL,
  "item_id" INTEGER NOT NULL,
  PRIMARY KEY ("build_id")
);

-- ----------------------------
-- Records of builditem_configs
-- ----------------------------
INSERT INTO "builditem_configs" VALUES (0, 'SuperCoal', 20, 355, 1, 0, 355, 1, 0, 355, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 30, 0, 501);
INSERT INTO "builditem_configs" VALUES (1, 'UltraCoal', 80, 501, 1, 0, 501, 1, 0, 357, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 90, 0, 502);
INSERT INTO "builditem_configs" VALUES (2, 'IronIngot', 20, 357, 1, 0, 357, 1, 0, 357, 1, 0, 501, 1, 0, 0, 0, 0, 0, 0, 0, 0, 30, 0, 500);
INSERT INTO "builditem_configs" VALUES (3, 'MithralIngot', 85, 508, 1, 0, 508, 1, 0, 502, 1, 0, 501, 1, 0, 0, 0, 0, 0, 0, 0, 0, 95, 0, 506);
INSERT INTO "builditem_configs" VALUES (4, 'GoldIngot', 70, 354, 1, 0, 354, 1, 0, 354, 1, 0, 501, 1, 0, 0, 0, 0, 0, 0, 0, 0, 95, 0, 503);
INSERT INTO "builditem_configs" VALUES (5, 'SilverIngot', 40, 356, 1, 0, 356, 1, 0, 356, 1, 0, 501, 1, 0, 0, 0, 0, 0, 0, 0, 0, 95, 0, 504);
INSERT INTO "builditem_configs" VALUES (6, 'BlondeIngot', 40, 507, 1, 0, 507, 1, 0, 507, 1, 0, 501, 1, 0, 0, 0, 0, 0, 0, 0, 0, 95, 0, 505);
INSERT INTO "builditem_configs" VALUES (7, 'Dagger', 30, 500, 1, 2, 501, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 26, 40, 0, 1);
INSERT INTO "builditem_configs" VALUES (8, 'ShortSword', 40, 500, 1, 2, 501, 1, 2, 357, 1, 1, 355, 1, 0, 0, 0, 0, 0, 0, 0, 33, 50, 0, 8);
INSERT INTO "builditem_configs" VALUES (9, 'MainGauche', 50, 500, 1, 2, 501, 1, 2, 357, 1, 1, 355, 1, 0, 0, 0, 0, 0, 0, 0, 41, 60, 0, 12);
INSERT INTO "builditem_configs" VALUES (10, 'Gradius', 60, 500, 1, 2, 501, 1, 2, 357, 1, 1, 355, 1, 0, 0, 0, 0, 0, 0, 0, 48, 70, 0, 15);
INSERT INTO "builditem_configs" VALUES (11, 'LongSword', 40, 500, 1, 2, 500, 1, 2, 501, 1, 1, 355, 1, 0, 0, 0, 0, 0, 0, 0, 33, 50, 0, 17);
INSERT INTO "builditem_configs" VALUES (12, 'Sabre', 50, 500, 1, 2, 500, 1, 2, 501, 1, 1, 355, 1, 0, 0, 0, 0, 0, 0, 0, 41, 60, 0, 23);
INSERT INTO "builditem_configs" VALUES (13, 'Scimitar', 60, 500, 1, 2, 500, 1, 2, 501, 1, 1, 355, 1, 0, 0, 0, 0, 0, 0, 0, 48, 70, 0, 25);
INSERT INTO "builditem_configs" VALUES (14, 'Falchion', 70, 500, 1, 2, 500, 1, 2, 501, 1, 1, 355, 1, 0, 0, 0, 0, 0, 0, 0, 56, 80, 0, 28);
INSERT INTO "builditem_configs" VALUES (15, 'Esterk', 40, 500, 1, 2, 500, 1, 2, 501, 1, 1, 357, 1, 0, 0, 0, 0, 0, 0, 0, 33, 50, 0, 31);
INSERT INTO "builditem_configs" VALUES (16, 'Rapier', 50, 500, 1, 2, 500, 1, 2, 501, 1, 1, 357, 1, 0, 0, 0, 0, 0, 0, 0, 41, 60, 0, 34);
INSERT INTO "builditem_configs" VALUES (17, 'BroadSword', 50, 500, 1, 2, 500, 1, 2, 501, 1, 2, 501, 1, 1, 0, 0, 0, 0, 0, 0, 41, 60, 0, 38);
INSERT INTO "builditem_configs" VALUES (18, 'BastadSword', 60, 500, 1, 2, 500, 1, 2, 501, 1, 2, 501, 1, 1, 355, 1, 1, 357, 1, 0, 48, 70, 0, 42);
INSERT INTO "builditem_configs" VALUES (19, 'Claymore', 70, 500, 1, 2, 500, 1, 2, 501, 1, 2, 501, 1, 1, 501, 1, 1, 0, 0, 0, 56, 80, 0, 46);
INSERT INTO "builditem_configs" VALUES (20, 'GreatSword', 80, 500, 1, 2, 500, 1, 2, 502, 1, 2, 501, 1, 1, 501, 1, 1, 0, 0, 0, 63, 90, 0, 50);
INSERT INTO "builditem_configs" VALUES (21, 'Flameberge', 90, 500, 1, 2, 500, 1, 2, 502, 1, 2, 502, 1, 2, 501, 1, 1, 501, 1, 1, 71, 100, 0, 54);
INSERT INTO "builditem_configs" VALUES (22, 'GiantSword', 95, 506, 1, 2, 503, 1, 2, 503, 1, 2, 508, 1, 1, 195, 1, -1, 500, 1, 1, 75, 100, 0, 615);
INSERT INTO "builditem_configs" VALUES (23, 'BlackShadowSword', 99, 506, 1, 2, 506, 1, 2, 505, 1, 2, 505, 1, 2, 508, 1, 1, 195, 1, -1, 75, 100, 0, 844);
INSERT INTO "builditem_configs" VALUES (24, 'LightAxe', 30, 500, 1, 2, 500, 1, 2, 501, 1, 1, 355, 1, 0, 0, 0, 0, 0, 0, 0, 26, 40, 0, 59);
INSERT INTO "builditem_configs" VALUES (25, 'Tomahoc', 40, 500, 1, 2, 500, 1, 2, 501, 1, 1, 355, 1, 0, 0, 0, 0, 0, 0, 0, 33, 50, 0, 62);
INSERT INTO "builditem_configs" VALUES (26, 'SexonAxe', 50, 500, 1, 2, 500, 1, 2, 501, 1, 1, 355, 1, 0, 0, 0, 0, 0, 0, 0, 41, 60, 0, 65);
INSERT INTO "builditem_configs" VALUES (27, 'DoubleAxe', 60, 500, 1, 2, 500, 1, 2, 501, 1, 1, 501, 1, 1, 0, 0, 0, 0, 0, 0, 48, 70, 0, 68);
INSERT INTO "builditem_configs" VALUES (28, 'WarAxe', 70, 500, 1, 2, 500, 1, 2, 501, 1, 2, 501, 1, 1, 501, 1, 1, 0, 0, 0, 56, 80, 0, 71);
INSERT INTO "builditem_configs" VALUES (29, 'BattleAxe', 90, 500, 1, 2, 500, 1, 2, 502, 1, 2, 502, 1, 2, 501, 1, 1, 501, 1, 1, 66, 100, 0, 560);
INSERT INTO "builditem_configs" VALUES (30, 'Hammer', 90, 503, 1, 2, 500, 1, 2, 500, 1, 2, 500, 1, 2, 502, 1, 1, 502, 1, 1, 71, 100, 0, 760);
INSERT INTO "builditem_configs" VALUES (31, 'BattleHammer', 95, 506, 1, 2, 503, 1, 2, 503, 1, 2, 508, 1, 1, 502, 1, 1, 500, 1, 1, 72, 100, 0, 761);
INSERT INTO "builditem_configs" VALUES (32, 'BarbarianHammer', 99, 506, 1, 2, 506, 1, 2, 503, 1, 2, 351, 1, -2, 508, 1, 1, 502, 1, 1, 73, 100, 0, 843);
INSERT INTO "builditem_configs" VALUES (33, 'Hauberk(M)', 40, 504, 1, 2, 500, 1, 2, 500, 1, 2, 207, 1, -1, 0, 0, 0, 0, 0, 0, 33, 50, 0, 454);
INSERT INTO "builditem_configs" VALUES (34, 'ChainHose(M)', 50, 504, 1, 2, 500, 1, 2, 500, 1, 2, 501, 1, 1, 207, 1, -1, 0, 0, 0, 41, 60, 0, 461);
INSERT INTO "builditem_configs" VALUES (35, 'ScaleMail(M)', 60, 504, 1, 2, 500, 1, 2, 500, 1, 2, 500, 1, 2, 501, 1, 1, 207, 1, -1, 48, 70, 0, 457);
INSERT INTO "builditem_configs" VALUES (36, 'ChainMail(M)', 70, 504, 1, 2, 500, 1, 2, 500, 1, 2, 500, 1, 2, 501, 1, 1, 207, 1, -1, 56, 80, 0, 456);
INSERT INTO "builditem_configs" VALUES (37, 'PlateLeggings(M)', 80, 504, 1, 2, 500, 1, 2, 500, 1, 2, 500, 1, 2, 502, 1, 1, 198, 1, -1, 63, 90, 0, 462);
INSERT INTO "builditem_configs" VALUES (38, 'PlateMail(M)', 90, 504, 1, 2, 504, 1, 2, 500, 1, 2, 500, 1, 2, 502, 1, 1, 198, 1, -1, 71, 100, 0, 458);
INSERT INTO "builditem_configs" VALUES (39, 'WizardHauberk(M)', 95, 506, 1, 2, 504, 1, 2, 508, 1, 2, 502, 1, 1, 553, 1, -1, 0, 0, 0, 71, 100, 0, 681);
INSERT INTO "builditem_configs" VALUES (40, 'KnightHauberk(M)', 95, 506, 1, 2, 504, 1, 2, 508, 1, 2, 502, 1, 1, 553, 1, -1, 0, 0, 0, 80, 100, 0, 687);
INSERT INTO "builditem_configs" VALUES (41, 'KnightPlateLeg(M)', 95, 506, 1, 2, 504, 1, 2, 508, 1, 2, 502, 1, 1, 553, 1, -1, 0, 0, 0, 80, 100, 0, 677);
INSERT INTO "builditem_configs" VALUES (42, 'KnightPlateMail(M)', 99, 506, 1, 2, 504, 1, 2, 504, 1, 2, 508, 1, 1, 502, 1, 1, 553, 1, -1, 71, 100, 0, 675);
INSERT INTO "builditem_configs" VALUES (43, 'Helm(M)', 75, 504, 1, 2, 504, 1, 2, 500, 1, 2, 500, 1, 1, 200, 1, -1, 0, 0, 0, 60, 85, 0, 600);
INSERT INTO "builditem_configs" VALUES (44, 'FullHelm(M)', 80, 504, 1, 2, 504, 1, 2, 500, 1, 2, 502, 1, 1, 501, 1, 1, 200, 1, -1, 65, 90, 0, 601);
INSERT INTO "builditem_configs" VALUES (45, 'KnightFullHelm(M)', 95, 506, 1, 2, 504, 1, 2, 508, 1, 2, 502, 1, 1, 553, 1, -1, 0, 0, 0, 75, 100, 0, 679);
INSERT INTO "builditem_configs" VALUES (46, 'Wings-Helm(M)', 95, 506, 1, 2, 504, 1, 2, 504, 1, 2, 508, 1, 1, 502, 1, 1, 200, 1, -1, 75, 100, 0, 751);
INSERT INTO "builditem_configs" VALUES (47, 'Horned-Helm(M)', 99, 506, 1, 2, 504, 1, 2, 504, 1, 2, 508, 1, 1, 502, 1, 1, 200, 1, -1, 80, 100, 0, 750);
INSERT INTO "builditem_configs" VALUES (48, 'Hauberk(W)', 40, 504, 1, 2, 500, 1, 2, 500, 1, 2, 207, 1, -1, 0, 0, 1, 0, 0, 0, 33, 50, 0, 472);
INSERT INTO "builditem_configs" VALUES (49, 'ChainHose(W)', 50, 504, 1, 2, 500, 1, 2, 500, 1, 2, 501, 1, 1, 207, 1, -1, 0, 0, 0, 41, 60, 0, 482);
INSERT INTO "builditem_configs" VALUES (50, 'ScaleMail(W)', 60, 504, 1, 2, 500, 1, 2, 500, 1, 2, 500, 1, 2, 501, 1, 1, 207, 1, -1, 48, 70, 0, 477);
INSERT INTO "builditem_configs" VALUES (51, 'ChainMail(W)', 70, 504, 1, 2, 500, 1, 2, 500, 1, 2, 500, 1, 2, 501, 1, 1, 207, 1, -1, 56, 80, 0, 476);
INSERT INTO "builditem_configs" VALUES (52, 'PlateLeggings(W)', 80, 504, 1, 2, 500, 1, 2, 500, 1, 2, 500, 1, 2, 502, 1, 1, 198, 1, -1, 63, 90, 0, 483);
INSERT INTO "builditem_configs" VALUES (53, 'PlateMail(W)', 90, 504, 1, 2, 504, 1, 2, 500, 1, 2, 500, 1, 2, 502, 1, 1, 198, 1, -1, 71, 100, 0, 478);
INSERT INTO "builditem_configs" VALUES (54, 'WizardHauberk(W)', 95, 506, 1, 2, 504, 1, 2, 502, 1, 1, 553, 1, -1, 0, 0, 0, 0, 0, 0, 71, 100, 0, 682);
INSERT INTO "builditem_configs" VALUES (55, 'KnightHauberk(W)', 95, 506, 1, 2, 506, 1, 2, 502, 1, 1, 553, 1, -1, 0, 0, 0, 0, 0, 0, 75, 100, 0, 688);
INSERT INTO "builditem_configs" VALUES (56, 'KnightPlateLeg(W)', 95, 506, 1, 2, 506, 1, 2, 502, 1, 1, 553, 1, -1, 0, 0, 0, 0, 0, 0, 75, 100, 0, 678);
INSERT INTO "builditem_configs" VALUES (57, 'KnightPlateMail(W)', 99, 506, 1, 2, 504, 1, 2, 504, 1, 2, 508, 1, 1, 502, 1, 1, 553, 1, -1, 71, 100, 0, 676);
INSERT INTO "builditem_configs" VALUES (58, 'Helm(W)', 75, 504, 1, 2, 504, 1, 2, 500, 1, 2, 500, 1, 1, 200, 1, -1, 0, 0, 0, 56, 85, 0, 602);
INSERT INTO "builditem_configs" VALUES (59, 'FullHelm(W)', 80, 504, 1, 2, 504, 1, 2, 500, 1, 2, 502, 1, 1, 501, 1, 1, 200, 1, -1, 63, 90, 0, 603);
INSERT INTO "builditem_configs" VALUES (60, 'KnightFullHelm(W)', 95, 506, 1, 2, 504, 1, 2, 508, 1, 2, 502, 1, 1, 553, 1, -1, 0, 0, 0, 71, 100, 0, 680);
INSERT INTO "builditem_configs" VALUES (61, 'Wings-Helm(W)', 95, 506, 1, 2, 504, 1, 2, 504, 1, 2, 508, 1, 1, 502, 1, 1, 200, 1, -1, 71, 100, 0, 755);
INSERT INTO "builditem_configs" VALUES (62, 'Horned-Helm(W)', 99, 506, 1, 2, 504, 1, 2, 504, 1, 2, 508, 1, 1, 502, 1, 1, 200, 1, -1, 80, 100, 0, 754);
INSERT INTO "builditem_configs" VALUES (63, 'LongSword+1', 45, 505, 1, 2, 500, 1, 2, 500, 1, 2, 501, 1, 1, 355, 1, 0, 355, 1, 0, 43, 80, 0, 18);
INSERT INTO "builditem_configs" VALUES (64, 'Sabre+1', 55, 505, 1, 2, 500, 1, 2, 500, 1, 2, 501, 1, 1, 355, 1, 0, 355, 1, 0, 51, 85, 0, 24);
INSERT INTO "builditem_configs" VALUES (65, 'Scimitar+1', 65, 505, 1, 2, 500, 1, 2, 500, 1, 2, 501, 1, 1, 355, 1, 0, 355, 1, 0, 58, 90, 0, 26);
INSERT INTO "builditem_configs" VALUES (66, 'Falchion+1', 75, 505, 1, 2, 500, 1, 2, 500, 1, 2, 501, 1, 1, 355, 1, 0, 355, 1, 0, 66, 95, 0, 29);
INSERT INTO "builditem_configs" VALUES (67, 'Esterk+1', 45, 505, 1, 2, 500, 1, 2, 500, 1, 2, 501, 1, 1, 357, 1, 0, 355, 1, 0, 43, 65, 0, 32);
INSERT INTO "builditem_configs" VALUES (68, 'Rapier+1', 55, 505, 1, 2, 500, 1, 2, 500, 1, 2, 501, 1, 1, 357, 1, 0, 355, 1, 0, 51, 75, 0, 35);
INSERT INTO "builditem_configs" VALUES (69, 'BroadSword+1', 55, 505, 1, 2, 500, 1, 2, 500, 1, 2, 501, 1, 1, 501, 1, 1, 0, 0, 0, 51, 90, 0, 39);
INSERT INTO "builditem_configs" VALUES (70, 'BastadSword+1', 65, 505, 1, 2, 500, 1, 2, 500, 1, 2, 501, 1, 1, 501, 1, 1, 357, 1, 1, 58, 95, 0, 43);
INSERT INTO "builditem_configs" VALUES (71, 'Claymore+1', 75, 505, 1, 2, 500, 1, 2, 500, 1, 2, 501, 1, 2, 501, 1, 1, 501, 1, 1, 66, 96, 0, 47);
INSERT INTO "builditem_configs" VALUES (72, 'GreatSword+1', 84, 505, 1, 2, 500, 1, 2, 500, 1, 2, 502, 1, 2, 502, 1, 1, 502, 1, 1, 73, 97, 0, 51);
INSERT INTO "builditem_configs" VALUES (73, 'Flameberge+1', 93, 505, 1, 2, 500, 1, 2, 500, 1, 2, 500, 1, 2, 502, 1, 1, 502, 1, 1, 81, 100, 0, 55);
INSERT INTO "builditem_configs" VALUES (74, 'LightAxe+1', 40, 505, 1, 2, 500, 1, 2, 500, 1, 1, 355, 1, 0, 0, 0, 0, 0, 0, 0, 36, 60, 0, 60);
INSERT INTO "builditem_configs" VALUES (75, 'Tomahoc+1', 45, 505, 1, 2, 500, 1, 2, 500, 1, 1, 355, 1, 0, 0, 0, 0, 0, 0, 0, 43, 70, 0, 63);
INSERT INTO "builditem_configs" VALUES (76, 'SexonAxe+1', 55, 505, 1, 2, 500, 1, 2, 500, 1, 1, 355, 1, 0, 0, 0, 0, 0, 0, 0, 51, 80, 0, 66);
INSERT INTO "builditem_configs" VALUES (77, 'DoubleAxe+1', 65, 505, 1, 2, 500, 1, 2, 500, 1, 2, 501, 1, 1, 501, 1, 1, 0, 0, 0, 58, 90, 0, 69);
INSERT INTO "builditem_configs" VALUES (78, 'WarAxe+1', 80, 505, 1, 2, 500, 1, 2, 500, 1, 2, 501, 1, 1, 501, 1, 1, 501, 1, 0, 66, 94, 0, 72);
INSERT INTO "builditem_configs" VALUES (79, 'BattleAxe+1', 95, 505, 1, 2, 500, 1, 2, 500, 1, 2, 500, 1, 1, 502, 1, 1, 502, 1, 1, 75, 100, 0, 580);
INSERT INTO "builditem_configs" VALUES (80, 'LongSword+2', 55, 506, 1, 2, 503, 1, 2, 195, 1, -2, 500, 1, 1, 0, 0, 0, 0, 0, 0, 53, 80, 0, 19);
INSERT INTO "builditem_configs" VALUES (81, 'Sabre+2', 65, 506, 1, 2, 503, 1, 2, 195, 1, -2, 500, 1, 1, 502, 1, 0, 501, 1, 0, 61, 85, 0, 582);
INSERT INTO "builditem_configs" VALUES (82, 'Scimitar+2', 75, 506, 1, 2, 503, 1, 2, 195, 1, -2, 500, 1, 1, 502, 1, 0, 501, 1, 0, 68, 90, 0, 27);
INSERT INTO "builditem_configs" VALUES (83, 'Falchion+2', 85, 506, 1, 2, 503, 1, 2, 195, 1, -2, 500, 1, 1, 502, 1, 0, 501, 1, 0, 76, 95, 0, 30);
INSERT INTO "builditem_configs" VALUES (84, 'Esterk+2', 55, 506, 1, 2, 503, 1, 2, 195, 1, -2, 500, 1, 1, 502, 1, 0, 501, 1, 0, 53, 65, 0, 33);
INSERT INTO "builditem_configs" VALUES (85, 'Rapier+2', 65, 506, 1, 2, 503, 1, 2, 195, 1, -2, 500, 1, 1, 502, 1, 0, 501, 1, 0, 61, 75, 0, 36);
INSERT INTO "builditem_configs" VALUES (86, 'BroadSword+2', 65, 506, 1, 2, 503, 1, 2, 195, 1, -2, 500, 1, 1, 500, 1, 1, 0, 0, 0, 61, 80, 0, 40);
INSERT INTO "builditem_configs" VALUES (87, 'BastadSword+2', 75, 506, 1, 2, 503, 1, 2, 195, 1, -2, 500, 1, 1, 500, 1, 1, 0, 0, 0, 68, 85, 0, 44);
INSERT INTO "builditem_configs" VALUES (88, 'Claymore+2', 85, 506, 1, 2, 503, 1, 2, 195, 1, -2, 500, 1, 2, 500, 1, 1, 500, 1, 1, 76, 90, 0, 48);
INSERT INTO "builditem_configs" VALUES (89, 'GreatSword+2', 94, 506, 1, 2, 503, 1, 2, 195, 1, -2, 500, 1, 2, 500, 1, 1, 502, 1, 1, 83, 95, 0, 52);
INSERT INTO "builditem_configs" VALUES (90, 'Flameberge+2', 97, 506, 1, 2, 506, 1, 2, 503, 1, 2, 195, 1, -2, 500, 1, 1, 500, 1, 1, 85, 100, 0, 56);
INSERT INTO "builditem_configs" VALUES (91, 'LightAxe+2', 50, 506, 1, 2, 503, 1, 2, 195, 1, -1, 502, 1, 0, 0, 0, 0, 0, 0, 0, 36, 75, 0, 61);
INSERT INTO "builditem_configs" VALUES (92, 'Tomahoc+2', 55, 506, 1, 2, 503, 1, 2, 195, 1, -1, 500, 1, 0, 0, 0, 0, 0, 0, 0, 53, 80, 0, 64);
INSERT INTO "builditem_configs" VALUES (93, 'SexonAxe+2', 65, 506, 1, 2, 503, 1, 2, 195, 1, -1, 500, 1, 0, 0, 0, 0, 0, 0, 0, 61, 85, 0, 67);
INSERT INTO "builditem_configs" VALUES (94, 'DoubleAxe+2', 75, 506, 1, 2, 503, 1, 2, 195, 1, -2, 500, 1, 1, 500, 1, 1, 0, 0, 0, 68, 90, 0, 70);
INSERT INTO "builditem_configs" VALUES (95, 'WarAxe+2', 90, 506, 1, 2, 503, 1, 2, 195, 1, -2, 500, 1, 1, 500, 1, 1, 500, 1, 0, 76, 95, 0, 73);
INSERT INTO "builditem_configs" VALUES (96, 'BattleAxe+2', 99, 506, 1, 2, 506, 1, 2, 503, 1, 2, 508, 1, 1, 195, 1, -1, 500, 1, 1, 88, 100, 0, 581);

-- ----------------------------
-- Table structure for crafting_configs
-- ----------------------------
DROP TABLE IF EXISTS "crafting_configs";
CREATE TABLE "crafting_configs" (
  "crafting_id" INTEGER,
  "name" TEXT NOT NULL,
  "array0" INTEGER NOT NULL,
  "array1" INTEGER NOT NULL,
  "array2" INTEGER NOT NULL,
  "array3" INTEGER NOT NULL,
  "array4" INTEGER NOT NULL,
  "array5" INTEGER NOT NULL,
  "array6" INTEGER NOT NULL,
  "array7" INTEGER NOT NULL,
  "array8" INTEGER NOT NULL,
  "array9" INTEGER NOT NULL,
  "array10" INTEGER NOT NULL,
  "array11" INTEGER NOT NULL,
  "skill_limit" INTEGER NOT NULL,
  "difficulty" INTEGER NOT NULL,
  PRIMARY KEY ("crafting_id")
);

-- ----------------------------
-- Records of crafting_configs
-- ----------------------------
INSERT INTO "crafting_configs" VALUES (1, 'MagicNecklace(DF+15)', 657, 1, 356, 1, 354, 1, 311, 2, -1, 0, -1, 0, 10, 70);
INSERT INTO "crafting_configs" VALUES (2, 'MagicNecklace(DF+20)', 1086, 1, 657, 1, 356, 1, 354, 1, 311, 1, -1, 0, 20, 70);
INSERT INTO "crafting_configs" VALUES (3, 'MagicNecklace(DF+25)', 1087, 1, 657, 1, 358, 1, 356, 1, 354, 1, 311, 1, 30, 70);
INSERT INTO "crafting_configs" VALUES (4, 'MagicNecklace(DF+30)', 1088, 1, 657, 1, 358, 1, 356, 1, 354, 1, 311, 1, 40, 70);
INSERT INTO "crafting_configs" VALUES (5, 'MagicNecklace(DM+2)', 657, 1, 356, 1, 354, 1, 305, 2, -1, 0, -1, 0, 10, 70);
INSERT INTO "crafting_configs" VALUES (6, 'MagicNecklace(DM+3)', 1090, 1, 657, 1, 356, 1, 354, 1, 311, 1, -1, 0, 20, 70);
INSERT INTO "crafting_configs" VALUES (7, 'MagicNecklace(DM+4)', 1091, 1, 657, 1, 358, 1, 356, 1, 354, 1, 305, 1, 30, 70);
INSERT INTO "crafting_configs" VALUES (8, 'MagicNecklace(DM+5)', 1092, 1, 657, 1, 358, 1, 356, 1, 354, 1, 305, 1, 40, 70);
INSERT INTO "crafting_configs" VALUES (9, 'MagicNecklace(MS12)', 657, 1, 356, 1, 354, 1, 308, 2, -1, 0, -1, 0, 10, 70);
INSERT INTO "crafting_configs" VALUES (10, 'MagicNecklace(MS13)', 1094, 1, 657, 1, 356, 1, 354, 1, 311, 1, -1, 0, 20, 70);
INSERT INTO "crafting_configs" VALUES (11, 'MagicNecklace(MS14)', 1095, 1, 657, 1, 358, 1, 356, 1, 354, 1, 308, 1, 30, 70);
INSERT INTO "crafting_configs" VALUES (12, 'MagicNecklace(MS15)', 1096, 1, 657, 1, 358, 1, 356, 1, 354, 1, 308, 1, 40, 70);
INSERT INTO "crafting_configs" VALUES (13, 'MagicNecklace(RM15)', 657, 1, 356, 1, 354, 1, 300, 2, -1, 0, -1, 0, 10, 70);
INSERT INTO "crafting_configs" VALUES (14, 'MagicNecklace(RM20)', 1098, 1, 657, 1, 356, 1, 354, 1, 311, 1, -1, 0, 20, 70);
INSERT INTO "crafting_configs" VALUES (15, 'MagicNecklace(RM25)', 1099, 1, 657, 1, 358, 1, 356, 1, 354, 1, 300, 1, 30, 70);
INSERT INTO "crafting_configs" VALUES (16, 'MagicNecklace(RM30)', 1100, 1, 657, 1, 358, 1, 356, 1, 354, 1, 300, 1, 40, 70);
INSERT INTO "crafting_configs" VALUES (17, 'DiamondWare', 358, 1, 350, 2, -1, 0, -1, 0, -1, 0, -1, 0, 0, 70);
INSERT INTO "crafting_configs" VALUES (18, 'RubyWare', 358, 1, 351, 2, -1, 0, -1, 0, -1, 0, -1, 0, 0, 70);
INSERT INTO "crafting_configs" VALUES (19, 'SapphireWare', 358, 1, 352, 2, -1, 0, -1, 0, -1, 0, -1, 0, 0, 70);
INSERT INTO "crafting_configs" VALUES (20, 'EmeraldWare', 358, 1, 353, 2, -1, 0, -1, 0, -1, 0, -1, 0, 0, 70);
INSERT INTO "crafting_configs" VALUES (21, 'CrystalWare', 358, 3, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, 0, 70);
INSERT INTO "crafting_configs" VALUES (22, 'MaginDiamond', 1106, 1, 1102, 1, 1102, 1, 657, 1, -1, 0, -1, 0, 30, 70);
INSERT INTO "crafting_configs" VALUES (23, 'MaginRuby', 1106, 1, 1103, 1, 1103, 1, 657, 1, -1, 0, -1, 0, 30, 70);
INSERT INTO "crafting_configs" VALUES (24, 'MagicEmerald', 1106, 1, 1105, 1, 1105, 1, 657, 1, -1, 0, -1, 0, 30, 70);
INSERT INTO "crafting_configs" VALUES (25, 'MagicSapphire', 1106, 1, 1104, 1, 1104, 1, 657, 1, -1, 0, -1, 0, 30, 70);

-- ----------------------------
-- Table structure for crusade_structures
-- ----------------------------
DROP TABLE IF EXISTS "crusade_structures";
CREATE TABLE "crusade_structures" (
  "structure_id" INTEGER,
  "map_name" TEXT NOT NULL,
  "structure_type" INTEGER NOT NULL,
  "pos_x" INTEGER NOT NULL,
  "pos_y" INTEGER NOT NULL,
  PRIMARY KEY ("structure_id")
);

-- ----------------------------
-- Records of crusade_structures
-- ----------------------------
INSERT INTO "crusade_structures" VALUES (1, 'aresden', 40, 145, 128);
INSERT INTO "crusade_structures" VALUES (2, 'aresden', 40, 152, 124);
INSERT INTO "crusade_structures" VALUES (3, 'aresden', 40, 99, 183);
INSERT INTO "crusade_structures" VALUES (4, 'aresden', 40, 109, 186);
INSERT INTO "crusade_structures" VALUES (5, 'aresden', 40, 124, 168);
INSERT INTO "crusade_structures" VALUES (6, 'aresden', 40, 134, 165);
INSERT INTO "crusade_structures" VALUES (7, 'aresden', 40, 171, 197);
INSERT INTO "crusade_structures" VALUES (8, 'aresden', 40, 156, 203);
INSERT INTO "crusade_structures" VALUES (9, 'elvine', 40, 144, 131);
INSERT INTO "crusade_structures" VALUES (10, 'elvine', 40, 152, 126);
INSERT INTO "crusade_structures" VALUES (11, 'elvine', 40, 196, 128);
INSERT INTO "crusade_structures" VALUES (12, 'elvine', 40, 205, 130);
INSERT INTO "crusade_structures" VALUES (13, 'elvine', 40, 224, 153);
INSERT INTO "crusade_structures" VALUES (14, 'elvine', 40, 234, 149);
INSERT INTO "crusade_structures" VALUES (15, 'elvine', 40, 241, 110);
INSERT INTO "crusade_structures" VALUES (16, 'elvine', 40, 230, 115);
INSERT INTO "crusade_structures" VALUES (17, 'middleland', 42, 418, 147);
INSERT INTO "crusade_structures" VALUES (18, 'middleland', 42, 112, 174);
INSERT INTO "crusade_structures" VALUES (19, 'middleland', 42, 263, 305);
INSERT INTO "crusade_structures" VALUES (20, 'aresden', 41, 165, 109);
INSERT INTO "crusade_structures" VALUES (21, 'elvine', 41, 170, 120);

-- ----------------------------
-- Table structure for drop_entries
-- ----------------------------
DROP TABLE IF EXISTS "drop_entries";
CREATE TABLE "drop_entries" (
  "drop_table_id" INTEGER NOT NULL,
  "tier" INTEGER NOT NULL,
  "item_id" INTEGER NOT NULL,
  "weight" INTEGER NOT NULL,
  "min_count" INTEGER NOT NULL,
  "max_count" INTEGER NOT NULL,
  PRIMARY KEY ("drop_table_id", "tier", "item_id")
);

-- ----------------------------
-- Records of drop_entries
-- ----------------------------
INSERT INTO "drop_entries" VALUES (20010, 2, 220, 12500, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 1, 12, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 1, 15, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 1, 23, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 1, 31, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 1, 62, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 1, 65, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 1, 79, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 1, 81, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 1, 17, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 1, 68, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 1, 81, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 1, 23, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 1, 31, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 31, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 34, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 71, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 84, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 455, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 475, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 1, 12, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 1, 15, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 1, 23, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 1, 31, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 1, 62, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 1, 65, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 1, 79, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 1, 81, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20016, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20016, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20016, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20016, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20016, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20016, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20016, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20016, 1, 1, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20016, 1, 8, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20016, 1, 59, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20016, 1, 79, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20016, 1, 81, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20016, 2, 192, 5115, 1, 1);
INSERT INTO "drop_entries" VALUES (20016, 2, 193, 4513, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 1, 12, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 1, 15, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 1, 23, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 1, 31, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 1, 62, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 1, 65, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 1, 79, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 1, 81, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 1, 12, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 1, 15, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 1, 23, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 1, 31, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 1, 62, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 1, 65, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 1, 79, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 1, 81, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 1, 1, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 1, 8, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 1, 59, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 1, 79, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 1, 81, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 1, 17, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 1, 68, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 1, 81, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 1, 23, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 1, 31, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 23, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 25, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 28, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 31, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 34, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 83, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 454, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 472, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 31, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 34, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 71, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 84, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 455, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 475, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 46, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 50, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 54, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 85, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 617, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 31, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 34, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 456, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 458, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 476, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 478, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 50, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 54, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 31, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 34, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 86, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 87, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 457, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 458, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 477, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 478, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 600, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 601, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 602, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 603, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 1, 50, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 1, 54, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 1, 55, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 1, 402, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 1, 560, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 1, 615, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 1, 50, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 1, 54, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 1, 55, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 1, 402, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 1, 560, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 1, 615, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 46, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 50, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 54, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 85, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 617, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 31, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 34, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 456, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 458, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 476, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 478, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 46, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 50, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 54, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 85, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 617, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 31, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 34, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 456, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 458, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 476, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 478, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 1, 50, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 1, 54, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 1, 55, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 1, 402, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 1, 451, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 1, 560, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 1, 615, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 1, 50, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 1, 54, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 1, 55, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 1, 402, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 1, 560, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 1, 615, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 1, 50, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 1, 54, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 1, 55, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 1, 402, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 1, 451, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 1, 560, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 1, 615, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 31, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 34, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 71, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 84, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 455, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 475, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 46, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 50, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 54, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 85, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 617, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 31, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 34, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 456, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 458, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 476, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 478, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 1, 23, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 1, 25, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 1, 28, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 1, 31, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 1, 34, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 1, 402, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 1, 451, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 1, 760, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 1, 31, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 1, 34, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 1, 46, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 1, 760, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 1, 761, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 1, 457, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 1, 477, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 1, 600, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 1, 602, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 50, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 54, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 31, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 34, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 86, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 87, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 457, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 458, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 477, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 478, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 600, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 601, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 602, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 603, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 50, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 54, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 31, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 34, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 86, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 87, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 457, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 458, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 477, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 478, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 600, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 601, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 602, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 603, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 1, 258, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 1, 258, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 1, 258, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 1, 258, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 1, 258, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 1, 258, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 257, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 257, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 257, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 257, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 257, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 257, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 257, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 257, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 256, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 256, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 1, 256, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 1, 256, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 1, 256, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 1, 256, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20010, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20010, 1, 91, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20010, 1, 95, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20010, 1, 1, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20010, 1, 12, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20010, 1, 62, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20010, 1, 17, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20010, 1, 79, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20010, 1, 8, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20010, 1, 258, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20071, 1, 1, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20071, 1, 8, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20071, 1, 12, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20071, 1, 17, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20071, 1, 62, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20071, 1, 79, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20071, 1, 91, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20071, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20071, 1, 95, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20071, 1, 258, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20072, 1, 1, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20072, 1, 8, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20072, 1, 12, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20072, 1, 17, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20072, 1, 62, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20072, 1, 79, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20072, 1, 91, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20072, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20072, 1, 95, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20072, 1, 258, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20075, 1, 12, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20075, 1, 15, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20075, 1, 23, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20075, 1, 31, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20075, 1, 62, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20075, 1, 65, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20075, 1, 79, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20075, 1, 81, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20075, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20075, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20075, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20075, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20075, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20075, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20075, 1, 258, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20075, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 1, 23, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 1, 25, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 1, 28, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 1, 31, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 1, 34, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 1, 83, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 1, 257, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 1, 454, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 1, 472, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 23, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 25, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 28, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 31, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 34, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 83, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 257, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 454, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 472, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20081, 1, 23, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20081, 1, 25, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20081, 1, 28, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20081, 1, 31, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20081, 1, 34, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20081, 1, 83, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20081, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20081, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20081, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20081, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20081, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20081, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20081, 1, 257, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20081, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20081, 1, 454, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20081, 1, 472, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 23, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 25, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 28, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 31, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 34, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 83, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 257, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 454, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 472, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 31, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 34, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 71, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 84, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 257, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 455, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 475, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 31, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 34, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 71, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 84, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 257, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 455, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 475, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20084, 1, 31, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20084, 1, 34, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20084, 1, 71, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20084, 1, 84, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20084, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20084, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20084, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20084, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20084, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20084, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20084, 1, 257, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20084, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20084, 1, 455, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20084, 1, 475, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20083, 1, 31, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20083, 1, 34, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20083, 1, 71, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20083, 1, 84, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20083, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20083, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20083, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20083, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20083, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20083, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20083, 1, 257, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20083, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20083, 1, 455, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20083, 1, 475, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 31, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 34, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 46, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 50, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 54, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 85, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 257, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 456, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 458, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 476, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 478, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 617, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 31, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 34, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 46, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 50, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 54, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 85, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 257, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 456, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 458, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 476, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 478, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 617, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 31, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 34, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 50, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 54, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 86, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 87, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 256, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 457, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 458, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 477, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 478, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 600, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 601, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 602, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 603, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 31, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 34, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 50, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 54, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 86, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 87, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 256, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 457, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 458, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 477, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 478, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 600, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 601, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 602, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 603, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 31, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 34, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 50, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 54, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 86, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 87, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 256, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 457, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 458, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 477, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 478, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 600, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 601, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 602, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 603, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 50, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 54, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 55, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 256, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 402, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 560, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 615, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 1, 50, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 1, 54, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 1, 55, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 1, 256, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 1, 402, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 1, 560, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 1, 615, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 1, 50, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 1, 54, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 1, 55, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 1, 256, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 1, 402, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 1, 560, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 1, 615, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 1, 50, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 1, 54, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 1, 55, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 1, 91, 1000, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 1, 92, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 1, 93, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 1, 94, 600, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 1, 95, 3000, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 1, 96, 1500, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 1, 256, 300, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 1, 390, 800, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 1, 402, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 1, 560, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 1, 615, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 2, 219, 2909, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 2, 221, 1766, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 2, 197, 1881, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 2, 196, 1410, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 2, 194, 940, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 2, 195, 752, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 2, 198, 752, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 2, 206, 3237, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 2, 207, 2312, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 2, 208, 1850, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 2, 216, 1901, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 2, 218, 1140, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 2, 215, 950, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 2, 217, 855, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 2, 188, 2308, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 2, 189, 1846, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 2, 190, 1846, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 2, 191, 1569, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 2, 205, 1958, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 2, 223, 1155, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 2, 224, 963, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 2, 225, 770, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 2, 222, 577, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 2, 209, 2278, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 2, 210, 1822, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 2, 211, 1367, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 2, 212, 1367, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 2, 213, 911, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 2, 214, 911, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 2, 542, 2923, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 2, 543, 1949, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 2, 540, 1461, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 2, 541, 974, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 2, 547, 2598, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 2, 546, 2079, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 2, 544, 1559, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 2, 541, 103, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 2, 550, 2200, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 2, 548, 1980, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 2, 552, 1980, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 2, 554, 1760, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 2, 551, 1540, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 2, 553, 1320, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 2, 549, 1100, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 2, 334, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 2, 336, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 2, 335, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 2, 337, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 2, 333, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 2, 634, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 2, 334, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 2, 336, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 2, 335, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 2, 337, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 2, 333, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 2, 634, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 2, 334, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 2, 336, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 2, 335, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 2, 337, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 2, 333, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 2, 634, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 2, 334, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 2, 336, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 2, 335, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 2, 337, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 2, 333, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 2, 634, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 2, 334, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 2, 336, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 2, 335, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 2, 337, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 2, 333, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 2, 634, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 2, 300, 8, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 2, 311, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 2, 308, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 2, 305, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 2, 337, 8, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 2, 637, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 2, 638, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 2, 852, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 2, 613, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 2, 639, 3, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 2, 641, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 2, 640, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 2, 613, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 2, 639, 3, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 2, 641, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 2, 640, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 2, 620, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 2, 621, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 2, 622, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 2, 644, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 2, 647, 6, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 2, 858, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 2, 620, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 2, 621, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 2, 622, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 2, 644, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 2, 858, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 2, 853, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 2, 300, 8, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 2, 311, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 2, 308, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 2, 305, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 2, 632, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 2, 637, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 2, 638, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 2, 300, 8, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 2, 311, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 2, 308, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 2, 305, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 2, 632, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 2, 637, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 2, 638, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 2, 259, 11, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 2, 291, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 2, 614, 11, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 2, 642, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 2, 643, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 2, 636, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 2, 734, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 2, 648, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 2, 852, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 2, 865, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 2, 866, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 2, 382, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 2, 491, 13, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 2, 490, 6, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 2, 492, 13, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 2, 381, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 2, 633, 13, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 2, 645, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 2, 616, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 2, 860, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 2, 861, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 2, 862, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 2, 865, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 2, 866, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 2, 620, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 2, 621, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 2, 622, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 2, 644, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 2, 848, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 2, 849, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 2, 850, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 2, 851, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 2, 859, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 2, 863, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 2, 864, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 2, 290, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 2, 292, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 2, 290, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 2, 292, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 2, 382, 11, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 2, 610, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 2, 611, 3, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 2, 612, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 2, 381, 11, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 2, 633, 11, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 2, 638, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 2, 645, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 2, 630, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 2, 631, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 2, 735, 3, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 2, 20, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 2, 847, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 2, 857, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 2, 618, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 2, 620, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 2, 621, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 2, 622, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 2, 644, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 2, 762, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 2, 337, 7, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 2, 858, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 2, 382, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 2, 491, 11, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 2, 490, 11, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 2, 492, 11, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 2, 381, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 2, 645, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 2, 762, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 2, 858, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 2, 853, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 2, 861, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 2, 862, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 2, 334, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 2, 336, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 2, 335, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 2, 337, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 2, 333, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 2, 634, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 2, 845, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 2, 636, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 2, 734, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 2, 642, 12, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 2, 643, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 2, 259, 60, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 2, 845, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 2, 636, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 2, 734, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 2, 642, 12, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 2, 643, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 2, 259, 60, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 2, 380, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 2, 847, 3, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 2, 852, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 2, 848, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 2, 861, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 2, 862, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 2, 861, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 2, 862, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 2, 852, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 2, 300, 250, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 2, 259, 250, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 2, 337, 150, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 2, 335, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 2, 308, 125, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 2, 311, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 2, 305, 13, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 2, 634, 35, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 2, 635, 7, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 2, 640, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 2, 637, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 2, 644, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 2, 620, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 2, 643, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 2, 614, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 2, 636, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 2, 311, 200, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 2, 305, 200, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 2, 614, 100, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 2, 290, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 2, 633, 25, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 2, 492, 13, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 2, 490, 6, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 2, 491, 3, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 2, 291, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 2, 630, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 2, 612, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 2, 610, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 2, 611, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 2, 20, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 2, 634, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 2, 636, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 2, 614, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 2, 380, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 2, 642, 10, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 2, 643, 3, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 2, 734, 4, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 2, 861, 10, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 2, 860, 1, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 2, 630, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 2, 861, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 2, 735, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 2, 20, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 2, 382, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 2, 381, 2, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 2, 645, 5, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 2, 638, 10, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 2, 636, 5, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 2, 734, 6, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 2, 634, 10, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 2, 290, 10, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 2, 490, 10, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 2, 491, 10, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 2, 492, 10, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 2, 20, 6, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 2, 647, 6, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 2, 860, 6, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 2, 631, 6, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 2, 490, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 2, 491, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 2, 492, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 2, 610, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 2, 611, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 2, 612, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 2, 645, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 2, 638, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 2, 382, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 2, 381, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 2, 259, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20010, 2, 0, 300000, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 2, 0, 32, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 2, 0, 18, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 2, 0, 18, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 2, 0, 32, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 2, 0, 53, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 2, 0, 97, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 2, 0, 26, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 2, 0, 95, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 2, 0, 92, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 2, 0, 96, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 2, 0, 459, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 2, 0, 459, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 2, 0, 153, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 2, 0, 110, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 2, 0, 128, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 2, 0, 450, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 2, 0, 264, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 2, 0, 165, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 2, 0, 531, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 2, 0, 228, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 2, 0, 86, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 2, 0, 3964, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 2, 0, 2416, 1, 1);
INSERT INTO "drop_entries" VALUES (20010, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20010, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20010, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20010, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20010, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20010, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20010, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20016, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20016, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20016, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20016, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20016, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20016, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20016, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20018, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20030, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20049, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20050, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20052, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20058, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20059, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20063, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20070, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20071, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20071, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20071, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20071, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20071, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20071, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20071, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20072, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20072, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20072, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20072, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20072, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20072, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20072, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20073, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20073, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20073, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20073, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20073, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20073, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20073, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20074, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20074, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20074, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20074, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20074, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20074, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20074, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20075, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20075, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20075, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20075, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20075, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20075, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20075, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20076, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20081, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20081, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20081, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20081, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20081, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20081, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20081, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20083, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20083, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20083, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20083, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20083, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20083, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20083, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20084, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20084, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20084, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20084, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20084, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20084, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20084, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20086, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20087, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20088, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20090, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20091, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20092, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20093, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20093, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20093, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20093, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20093, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20093, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20093, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20094, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20094, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20094, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20094, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20094, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20094, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20094, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20095, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20095, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20095, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20095, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20095, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20095, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20095, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20096, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20096, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20096, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20096, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20096, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20096, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20096, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20097, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20097, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20097, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20097, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20097, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20097, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20097, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20098, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20098, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20098, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20098, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20098, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20098, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20098, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20099, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20099, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20099, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20099, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20099, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20099, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20099, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20100, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20100, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20100, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20100, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20100, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20100, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20100, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20101, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20101, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20101, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20101, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20101, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20101, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20101, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20102, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20102, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20102, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20102, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20102, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20102, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20102, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20103, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20103, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20103, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20103, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20103, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20103, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20103, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20104, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20104, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20104, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20104, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20104, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20104, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20104, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20105, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20105, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20105, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20105, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20105, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20105, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20105, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20106, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20106, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20106, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20106, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20106, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20106, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20106, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20107, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20107, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20107, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20107, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20107, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20107, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20107, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20108, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20108, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20108, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20108, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20108, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20108, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20108, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20109, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20109, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20109, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20109, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20109, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20109, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20109, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20110, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20110, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20110, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20110, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20110, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20110, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20110, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20111, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20111, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20111, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20111, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20111, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20111, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20111, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20112, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20112, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20112, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20112, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20112, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20112, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20112, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20113, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20113, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20113, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20113, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20113, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20113, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20113, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20114, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20114, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20114, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20114, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20114, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20114, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20114, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20115, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20115, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20115, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20115, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20115, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20115, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20115, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20116, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20116, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20116, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20116, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20116, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20116, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20116, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20117, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20117, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20117, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20117, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20117, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20117, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20117, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20118, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20118, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20118, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20118, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20118, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20118, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20118, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20119, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20119, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20119, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20119, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20119, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20119, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20119, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20120, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20120, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20120, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20120, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20120, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20120, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20120, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20121, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20121, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20121, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20121, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20121, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20121, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20121, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20122, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20122, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20122, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20122, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20122, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20122, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20122, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20123, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20123, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20123, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20123, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20123, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20123, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20123, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20124, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20124, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20124, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20124, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20124, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20124, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20124, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20125, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20125, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20125, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20125, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20125, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20125, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20125, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20126, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20126, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20126, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20126, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20126, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20126, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20126, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20127, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20127, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20127, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20127, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20127, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20127, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20127, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20128, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20128, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20128, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20128, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20128, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20128, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20128, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20129, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20129, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20129, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20129, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20129, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20129, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20129, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20130, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20130, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20130, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20130, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20130, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20130, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20130, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20131, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20131, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20131, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20131, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20131, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20131, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20131, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20132, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20132, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20132, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20132, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20132, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20132, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20132, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20133, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20133, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20133, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20133, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20133, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20133, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20133, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20134, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20134, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20134, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20134, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20134, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20134, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20134, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20135, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20135, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20135, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20135, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20135, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20135, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20135, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20136, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20136, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20136, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20136, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20136, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20136, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20136, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20137, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20137, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20137, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20137, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20137, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20137, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20137, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20138, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20138, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20138, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20138, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20138, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20138, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20138, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20139, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20139, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20139, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20139, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20139, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20139, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20139, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20140, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20140, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20140, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20140, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20140, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20140, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20140, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20141, 1, 650, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20141, 1, 656, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20141, 1, 657, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20141, 1, 868, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20141, 1, 869, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20141, 1, 870, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20141, 1, 871, 30, 1, 1);
INSERT INTO "drop_entries" VALUES (20031, 2, 0, 13677, 1, 1);
INSERT INTO "drop_entries" VALUES (20032, 2, 0, 9531, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 2, 0, 13009, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 2, 0, 8640, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 2, 0, 5218, 1, 1);
INSERT INTO "drop_entries" VALUES (20017, 2, 0, 9025, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 2, 0, 17826, 1, 1);
INSERT INTO "drop_entries" VALUES (20014, 2, 0, 13767, 1, 1);
INSERT INTO "drop_entries" VALUES (20022, 2, 0, 14071, 1, 1);
INSERT INTO "drop_entries" VALUES (20023, 2, 0, 2947, 1, 1);
INSERT INTO "drop_entries" VALUES (20012, 2, 0, 2668, 1, 1);
INSERT INTO "drop_entries" VALUES (20011, 2, 0, 5428, 1, 1);
INSERT INTO "drop_entries" VALUES (20016, 2, 0, 14442, 1, 1);
INSERT INTO "drop_entries" VALUES (20010, 1, 81, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 461, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20027, 1, 482, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 461, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20077, 1, 482, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 461, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20082, 1, 482, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 454, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 472, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 461, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20013, 1, 482, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 454, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 472, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 461, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20028, 1, 482, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 454, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 472, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 461, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20053, 1, 482, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 454, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 472, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 461, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20079, 1, 482, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 454, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 472, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 461, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20080, 1, 482, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 454, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 472, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 461, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 482, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 750, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 751, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 752, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 753, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 754, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 755, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 756, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20029, 1, 757, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 454, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 472, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 461, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 482, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 750, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 751, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 752, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 753, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 754, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 755, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 756, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20033, 1, 757, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 454, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 472, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 461, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 482, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 750, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 751, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 752, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 753, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 754, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 755, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 756, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20048, 1, 757, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 454, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 472, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 461, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 482, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 750, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 751, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 752, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 753, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 754, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 755, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 756, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20054, 1, 757, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 454, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 472, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 461, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 482, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 750, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 751, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 752, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 753, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 754, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 755, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 756, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20078, 1, 757, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 457, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 477, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 458, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 478, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 86, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 87, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 600, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 601, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 602, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20089, 1, 603, 50, 1, 1);
INSERT INTO "drop_entries" VALUES (20085, 1, 402, 50, 1, 1);

-- ----------------------------
-- Table structure for drop_tables
-- ----------------------------
DROP TABLE IF EXISTS "drop_tables";
CREATE TABLE "drop_tables" (
  "drop_table_id" INTEGER,
  "name" TEXT NOT NULL,
  "description" TEXT NOT NULL,
  PRIMARY KEY ("drop_table_id")
);

-- ----------------------------
-- Records of drop_tables
-- ----------------------------
INSERT INTO "drop_tables" VALUES (20010, 'drops_Slime', 'Level 1');
INSERT INTO "drop_tables" VALUES (20011, 'drops_Skeleton', 'Level 2');
INSERT INTO "drop_tables" VALUES (20012, 'drops_Stone-Golem', 'Level 3');
INSERT INTO "drop_tables" VALUES (20013, 'drops_Cyclops', 'Level 5');
INSERT INTO "drop_tables" VALUES (20014, 'drops_Orc', 'Level 2');
INSERT INTO "drop_tables" VALUES (20016, 'drops_Giant-Ant', 'Level 1');
INSERT INTO "drop_tables" VALUES (20017, 'drops_Scorpion', 'Level 2');
INSERT INTO "drop_tables" VALUES (20018, 'drops_Zombie', 'Level 2');
INSERT INTO "drop_tables" VALUES (20022, 'drops_Amphis', 'Level 1');
INSERT INTO "drop_tables" VALUES (20023, 'drops_Clay-Golem', 'Level 3');
INSERT INTO "drop_tables" VALUES (20027, 'drops_Hellbound', 'Level 4');
INSERT INTO "drop_tables" VALUES (20028, 'drops_Troll', 'Level 5');
INSERT INTO "drop_tables" VALUES (20029, 'drops_Ogre', 'Level 6');
INSERT INTO "drop_tables" VALUES (20030, 'drops_Liche', 'Level 7');
INSERT INTO "drop_tables" VALUES (20031, 'drops_Demon', 'Level 8');
INSERT INTO "drop_tables" VALUES (20032, 'drops_Unicorn', 'Level 8');
INSERT INTO "drop_tables" VALUES (20033, 'drops_WereWolf', 'Level 6');
INSERT INTO "drop_tables" VALUES (20048, 'drops_Stalker', 'Level 6');
INSERT INTO "drop_tables" VALUES (20049, 'drops_Hellclaw', 'Level 8');
INSERT INTO "drop_tables" VALUES (20050, 'drops_Tigerworm', 'Level 8');
INSERT INTO "drop_tables" VALUES (20052, 'drops_Gagoyle', 'Level 8');
INSERT INTO "drop_tables" VALUES (20053, 'drops_Beholder', 'Level 5');
INSERT INTO "drop_tables" VALUES (20054, 'drops_Dark-Elf', 'Level 6');
INSERT INTO "drop_tables" VALUES (20058, 'drops_Mountain-Giant', 'Level 9');
INSERT INTO "drop_tables" VALUES (20059, 'drops_Ettin', 'Level 10');
INSERT INTO "drop_tables" VALUES (20063, 'drops_Frost', 'Level 7');
INSERT INTO "drop_tables" VALUES (20070, 'drops_Barlog', 'Level 7');
INSERT INTO "drop_tables" VALUES (20071, 'drops_Rabbit', 'Drops for Rabbit');
INSERT INTO "drop_tables" VALUES (20072, 'drops_Cat', 'Drops for Cat');
INSERT INTO "drop_tables" VALUES (20073, 'drops_Dummy', 'Drops for Dummy');
INSERT INTO "drop_tables" VALUES (20074, 'drops_Attack-Dummy', 'Drops for Attack-Dummy');
INSERT INTO "drop_tables" VALUES (20075, 'drops_Orc-Mage', 'Drops for Orc-Mage');
INSERT INTO "drop_tables" VALUES (20076, 'drops_Giant-Frog', 'Drops for Giant-Frog');
INSERT INTO "drop_tables" VALUES (20077, 'drops_Rudolph', 'Drops for Rudolph');
INSERT INTO "drop_tables" VALUES (20078, 'drops_Ice-Golem', 'Drops for Ice-Golem');
INSERT INTO "drop_tables" VALUES (20079, 'drops_Cannibal-Plant', 'Drops for Cannibal-Plant');
INSERT INTO "drop_tables" VALUES (20080, 'drops_DireBoar', 'Drops for DireBoar');
INSERT INTO "drop_tables" VALUES (20081, 'drops_Tentocle', 'Drops for Tentocle');
INSERT INTO "drop_tables" VALUES (20082, 'drops_Giant-Crayfish', 'Drops for Giant-Crayfish');
INSERT INTO "drop_tables" VALUES (20083, 'drops_Giant-Plant', 'Drops for Giant-Plant');
INSERT INTO "drop_tables" VALUES (20084, 'drops_Claw-Turtle', 'Drops for Claw-Turtle');
INSERT INTO "drop_tables" VALUES (20085, 'drops_Centaurus', 'Drops for Centaurus');
INSERT INTO "drop_tables" VALUES (20086, 'drops_Giant-Lizard', 'Drops for Giant-Lizard');
INSERT INTO "drop_tables" VALUES (20087, 'drops_MasterMage-Orc', 'Drops for MasterMage-Orc');
INSERT INTO "drop_tables" VALUES (20088, 'drops_Minotaurs', 'Drops for Minotaurs');
INSERT INTO "drop_tables" VALUES (20089, 'drops_Nizie', 'Drops for Nizie');
INSERT INTO "drop_tables" VALUES (20090, 'drops_Wyvern', 'Drops for Wyvern');
INSERT INTO "drop_tables" VALUES (20091, 'drops_Fire-Wyvern', 'Drops for Fire-Wyvern');
INSERT INTO "drop_tables" VALUES (20092, 'drops_Abaddon', 'Drops for Abaddon');
INSERT INTO "drop_tables" VALUES (20093, 'drops_AGT-Aresden', 'Drops for AGT-Aresden');
INSERT INTO "drop_tables" VALUES (20094, 'drops_AGT-Elvine', 'Drops for AGT-Elvine');
INSERT INTO "drop_tables" VALUES (20095, 'drops_CGT-Aresden', 'Drops for CGT-Aresden');
INSERT INTO "drop_tables" VALUES (20096, 'drops_CGT-Elvine', 'Drops for CGT-Elvine');
INSERT INTO "drop_tables" VALUES (20097, 'drops_MS-Aresden', 'Drops for MS-Aresden');
INSERT INTO "drop_tables" VALUES (20098, 'drops_MS-Elvine', 'Drops for MS-Elvine');
INSERT INTO "drop_tables" VALUES (20099, 'drops_DT-Aresden', 'Drops for DT-Aresden');
INSERT INTO "drop_tables" VALUES (20100, 'drops_DT-Elvine', 'Drops for DT-Elvine');
INSERT INTO "drop_tables" VALUES (20101, 'drops_ESG-Aresden', 'Drops for ESG-Aresden');
INSERT INTO "drop_tables" VALUES (20102, 'drops_ESG-Elvine', 'Drops for ESG-Elvine');
INSERT INTO "drop_tables" VALUES (20103, 'drops_GMG-Aresden', 'Drops for GMG-Aresden');
INSERT INTO "drop_tables" VALUES (20104, 'drops_GMG-Elvine', 'Drops for GMG-Elvine');
INSERT INTO "drop_tables" VALUES (20105, 'drops_ManaStone', 'Drops for ManaStone');
INSERT INTO "drop_tables" VALUES (20106, 'drops_LWB-Aresden', 'Drops for LWB-Aresden');
INSERT INTO "drop_tables" VALUES (20107, 'drops_LWB-Elvine', 'Drops for LWB-Elvine');
INSERT INTO "drop_tables" VALUES (20108, 'drops_GHK', 'Drops for GHK');
INSERT INTO "drop_tables" VALUES (20109, 'drops_GHKABS', 'Drops for GHKABS');
INSERT INTO "drop_tables" VALUES (20110, 'drops_TK', 'Drops for TK');
INSERT INTO "drop_tables" VALUES (20111, 'drops_BG', 'Drops for BG');
INSERT INTO "drop_tables" VALUES (20112, 'drops_XB-Aresden', 'Drops for XB-Aresden');
INSERT INTO "drop_tables" VALUES (20113, 'drops_XB-Elvine', 'Drops for XB-Elvine');
INSERT INTO "drop_tables" VALUES (20114, 'drops_XW-Aresden', 'Drops for XW-Aresden');
INSERT INTO "drop_tables" VALUES (20115, 'drops_XW-Elvine', 'Drops for XW-Elvine');
INSERT INTO "drop_tables" VALUES (20116, 'drops_XY-Aresden', 'Drops for XY-Aresden');
INSERT INTO "drop_tables" VALUES (20117, 'drops_XY-Elvine', 'Drops for XY-Elvine');
INSERT INTO "drop_tables" VALUES (20118, 'drops_YB-Aresden', 'Drops for YB-Aresden');
INSERT INTO "drop_tables" VALUES (20119, 'drops_YB-Elvine', 'Drops for YB-Elvine');
INSERT INTO "drop_tables" VALUES (20120, 'drops_YW-Aresden', 'Drops for YW-Aresden');
INSERT INTO "drop_tables" VALUES (20121, 'drops_YW-Elvine', 'Drops for YW-Elvine');
INSERT INTO "drop_tables" VALUES (20122, 'drops_YY-Aresden', 'Drops for YY-Aresden');
INSERT INTO "drop_tables" VALUES (20123, 'drops_YY-Elvine', 'Drops for YY-Elvine');
INSERT INTO "drop_tables" VALUES (20124, 'drops_CP-Aresden', 'Drops for CP-Aresden');
INSERT INTO "drop_tables" VALUES (20125, 'drops_CP-Elvine', 'Drops for CP-Elvine');
INSERT INTO "drop_tables" VALUES (20126, 'drops_Sor-Aresden', 'Drops for Sor-Aresden');
INSERT INTO "drop_tables" VALUES (20127, 'drops_Sor-Elvine', 'Drops for Sor-Elvine');
INSERT INTO "drop_tables" VALUES (20128, 'drops_ATK-Aresden', 'Drops for ATK-Aresden');
INSERT INTO "drop_tables" VALUES (20129, 'drops_ATK-Evline', 'Drops for ATK-Evline');
INSERT INTO "drop_tables" VALUES (20130, 'drops_Elf-Aresden', 'Drops for Elf-Aresden');
INSERT INTO "drop_tables" VALUES (20131, 'drops_Elf-Elvine', 'Drops for Elf-Elvine');
INSERT INTO "drop_tables" VALUES (20132, 'drops_DSK-Aresden', 'Drops for DSK-Aresden');
INSERT INTO "drop_tables" VALUES (20133, 'drops_DSK-Elvine', 'Drops for DSK-Elvine');
INSERT INTO "drop_tables" VALUES (20134, 'drops_HBT-Aresden', 'Drops for HBT-Aresden');
INSERT INTO "drop_tables" VALUES (20135, 'drops_HBT-Elvine', 'Drops for HBT-Elvine');
INSERT INTO "drop_tables" VALUES (20136, 'drops_CT-Aresden', 'Drops for CT-Aresden');
INSERT INTO "drop_tables" VALUES (20137, 'drops_CT-Elvine', 'Drops for CT-Elvine');
INSERT INTO "drop_tables" VALUES (20138, 'drops_Bar-Aresden', 'Drops for Bar-Aresden');
INSERT INTO "drop_tables" VALUES (20139, 'drops_Bar-Elvine', 'Drops for Bar-Elvine');
INSERT INTO "drop_tables" VALUES (20140, 'drops_AGC-Aresden', 'Drops for AGC-Aresden');
INSERT INTO "drop_tables" VALUES (20141, 'drops_AGC-Elvine', 'Drops for AGC-Elvine');

-- ----------------------------
-- Table structure for event_schedule
-- ----------------------------
DROP TABLE IF EXISTS "event_schedule";
CREATE TABLE "event_schedule" (
  "id" INTEGER PRIMARY KEY AUTOINCREMENT,
  "event_type" TEXT NOT NULL,
  "schedule_index" INTEGER NOT NULL,
  "day" INTEGER NOT NULL,
  "start_hour" INTEGER NOT NULL,
  "start_minute" INTEGER NOT NULL,
  "end_hour" INTEGER,
  "end_minute" INTEGER,
  "is_active" INTEGER NOT NULL DEFAULT 0,
  UNIQUE ("event_type" ASC, "schedule_index" ASC)
);

-- ----------------------------
-- Records of event_schedule
-- ----------------------------
INSERT INTO "event_schedule" VALUES (1, 'crusade', 0, 1, 1, 1, NULL, NULL, 0);
INSERT INTO "event_schedule" VALUES (4, 'apocalypse', 0, 6, 1, 1, 23, 1, 0);

-- ----------------------------
-- Table structure for items
-- ----------------------------
DROP TABLE IF EXISTS "items";
CREATE TABLE "items" (
  "item_id" INTEGER,
  "name" TEXT NOT NULL,
  "item_type" INTEGER NOT NULL,
  "equip_pos" INTEGER NOT NULL,
  "item_effect_type" INTEGER NOT NULL,
  "item_effect_value1" INTEGER NOT NULL,
  "item_effect_value2" INTEGER NOT NULL,
  "item_effect_value3" INTEGER NOT NULL,
  "item_effect_value4" INTEGER NOT NULL,
  "item_effect_value5" INTEGER NOT NULL,
  "item_effect_value6" INTEGER NOT NULL,
  "max_lifespan" INTEGER NOT NULL,
  "special_effect" INTEGER NOT NULL,
  "sprite" INTEGER NOT NULL,
  "sprite_frame" INTEGER NOT NULL,
  "price" INTEGER NOT NULL,
  "is_for_sale" INTEGER NOT NULL,
  "weight" INTEGER NOT NULL,
  "appr_value" INTEGER NOT NULL,
  "speed" INTEGER NOT NULL,
  "level_limit" INTEGER NOT NULL,
  "gender_limit" INTEGER NOT NULL,
  "special_effect_value1" INTEGER NOT NULL,
  "special_effect_value2" INTEGER NOT NULL,
  "related_skill" INTEGER NOT NULL,
  "category" INTEGER NOT NULL,
  "item_color" INTEGER NOT NULL,
  PRIMARY KEY ("item_id")
);

-- ----------------------------
-- Records of items
-- ----------------------------
INSERT INTO "items" VALUES (1, 'Dagger', 1, 8, 1, 1, 5, 0, 1, 4, 0, 300, 0, 1, 0, 25, 1, 200, 1, 0, 0, 0, 0, -10, 7, 1, 0);
INSERT INTO "items" VALUES (2, 'Dagger (S.C)', 1, 8, 1, 1, 4, 0, 1, 3, 0, 800, 0, 1, 0, 55, 0, 200, 1, 0, 0, 0, 0, -10, 7, 1, 0);
INSERT INTO "items" VALUES (3, 'Dagger (Sword Breaker)', 1, 8, 1, 1, 4, 0, 1, 3, 0, 1, 0, 1, 0, 55, 0, 200, 1, 0, 0, 0, 0, -10, 7, 1, 0);
INSERT INTO "items" VALUES (4, 'Dagger +1', 1, 8, 1, 1, 5, 1, 1, 4, 1, 800, 0, 1, 0, 100, 1, 200, 1, 0, 10, 0, 0, -10, 7, 1, 0);
INSERT INTO "items" VALUES (6, 'Kight Dagger', 1, 8, 1, 1, 4, 0, 1, 3, 0, 800, 0, 1, 0, 35, 0, 200, 1, 0, 0, 0, 0, -10, 7, 1, 0);
INSERT INTO "items" VALUES (7, 'Dirk', 1, 8, 1, 1, 4, 0, 1, 3, 0, 800, 0, 1, 0, 35, 0, 200, 1, 0, 0, 0, 0, -10, 7, 1, 0);
INSERT INTO "items" VALUES (8, 'Short Sword', 1, 8, 1, 1, 6, 0, 1, 5, 0, 500, 0, 1, 1, 50, 1, 800, 2, 2, 0, 0, 0, -5, 7, 1, 0);
INSERT INTO "items" VALUES (9, 'Short Sword +1', 1, 8, 1, 1, 6, 1, 1, 5, 1, 800, 0, 1, 1, 200, 1, 800, 2, 2, 10, 0, 0, -5, 7, 1, 0);
INSERT INTO "items" VALUES (11, 'Short Sword (S.C)', 1, 8, 1, 1, 6, 0, 1, 5, 0, 800, 0, 1, 1, 100, 0, 800, 2, 2, 0, 0, 0, -5, 7, 1, 0);
INSERT INTO "items" VALUES (12, 'Main Gauche', 1, 8, 1, 1, 7, 0, 1, 6, 0, 500, 0, 1, 1, 50, 1, 800, 2, 2, 0, 0, 0, -2, 7, 1, 0);
INSERT INTO "items" VALUES (13, 'Main Gauche +1', 1, 8, 1, 1, 7, 1, 1, 6, 1, 800, 0, 1, 1, 200, 1, 800, 2, 2, 15, 0, 0, -2, 7, 1, 0);
INSERT INTO "items" VALUES (14, 'Main Gauche (S.C)', 1, 8, 1, 1, 6, 0, 1, 4, 0, 800, 0, 1, 1, 100, 0, 800, 2, 2, 0, 0, 0, -2, 7, 1, 0);
INSERT INTO "items" VALUES (15, 'Gradius', 1, 8, 1, 1, 8, 0, 1, 7, 0, 500, 0, 1, 1, 90, 1, 800, 2, 2, 0, 0, 0, -2, 7, 1, 0);
INSERT INTO "items" VALUES (16, 'Gradius +1', 1, 8, 1, 1, 8, 1, 1, 7, 1, 800, 0, 1, 1, 350, 1, 800, 2, 2, 20, 0, 0, -2, 7, 1, 0);
INSERT INTO "items" VALUES (17, 'Long Sword', 1, 8, 1, 1, 9, 0, 1, 10, 0, 800, 0, 1, 2, 180, 1, 1400, 3, 6, 0, 0, -1, 0, 8, 1, 0);
INSERT INTO "items" VALUES (18, 'Long Sword +1', 1, 8, 1, 1, 9, 1, 1, 10, 1, 1400, 0, 1, 2, 650, 1, 1400, 3, 6, 25, 0, -1, 0, 8, 1, 0);
INSERT INTO "items" VALUES (19, 'Long Sword +2', 1, 8, 1, 1, 9, 2, 1, 10, 2, 1400, 0, 1, 2, 2600, 0, 1600, 3, 6, 50, 0, -1, 0, 8, 1, 0);
INSERT INTO "items" VALUES (20, 'Excalibur', 1, 8, 1, 2, 10, 4, 2, 12, 4, 8000, 0, 1, 19, 31000, 0, 4000, 5, 0, 0, 0, -1, 0, 8, 1, 0);
INSERT INTO "items" VALUES (22, 'Long Sword (S.C)', 1, 8, 1, 1, 8, 0, 1, 12, 0, 1400, 0, 1, 2, 250, 0, 1400, 3, 6, 0, 0, -1, 0, 8, 1, 0);
INSERT INTO "items" VALUES (23, 'Sabre', 1, 8, 1, 1, 8, 0, 1, 9, 0, 800, 0, 1, 3, 150, 1, 1200, 4, 5, 0, 0, -1, 0, 8, 1, 0);
INSERT INTO "items" VALUES (24, 'Sabre +1', 1, 8, 1, 1, 8, 1, 1, 9, 1, 1000, 0, 1, 3, 600, 1, 1200, 4, 5, 20, 0, -1, 0, 8, 1, 0);
INSERT INTO "items" VALUES (25, 'Scimitar', 1, 8, 1, 1, 10, 0, 1, 12, 0, 800, 0, 1, 4, 200, 1, 1800, 6, 6, 0, 0, -1, 3, 8, 1, 0);
INSERT INTO "items" VALUES (26, 'Scimitar +1', 1, 8, 1, 1, 10, 1, 1, 12, 1, 1000, 0, 1, 4, 800, 1, 1800, 6, 6, 30, 0, -1, 3, 8, 1, 0);
INSERT INTO "items" VALUES (27, 'Scimitar +2', 1, 8, 1, 1, 10, 2, 1, 12, 2, 1000, 0, 1, 4, 3200, 0, 2000, 6, 6, 70, 0, -1, 3, 8, 1, 0);
INSERT INTO "items" VALUES (28, 'Falchion', 1, 8, 1, 1, 12, 0, 2, 7, 0, 800, 0, 1, 5, 250, 1, 2200, 6, 6, 0, 0, -2, 2, 8, 1, 0);
INSERT INTO "items" VALUES (29, 'Falchion +1', 1, 8, 1, 1, 12, 1, 2, 7, 1, 1400, 0, 1, 5, 1000, 1, 2200, 6, 6, 35, 0, -2, 2, 8, 1, 0);
INSERT INTO "items" VALUES (30, 'Falchion +2', 1, 8, 1, 1, 12, 2, 2, 7, 2, 1400, 0, 1, 5, 4000, 0, 2200, 6, 6, 70, 0, -2, 2, 8, 1, 0);
INSERT INTO "items" VALUES (31, 'Esterk', 1, 8, 1, 1, 8, 0, 1, 12, 0, 800, 0, 1, 6, 400, 1, 1200, 7, 4, 0, 0, 0, 0, 9, 1, 0);
INSERT INTO "items" VALUES (32, 'Esterk +1', 1, 8, 1, 1, 8, 1, 1, 12, 1, 1400, 0, 1, 6, 800, 1, 1200, 7, 4, 20, 0, 0, 0, 9, 1, 0);
INSERT INTO "items" VALUES (33, 'Esterk +2', 1, 8, 1, 1, 8, 2, 1, 12, 2, 1400, 0, 1, 6, 3200, 0, 1200, 7, 4, 70, 0, 0, 0, 9, 1, 0);
INSERT INTO "items" VALUES (34, 'Rapier', 1, 8, 1, 1, 7, 0, 1, 10, 0, 800, 0, 1, 6, 300, 1, 1100, 7, 3, 0, 0, 0, 0, 9, 1, 0);
INSERT INTO "items" VALUES (35, 'Rapier +1', 1, 8, 1, 1, 7, 1, 1, 10, 1, 1000, 0, 1, 6, 1300, 1, 1100, 7, 3, 30, 0, 0, 0, 9, 1, 0);
INSERT INTO "items" VALUES (36, 'Rapier +2', 1, 8, 1, 1, 7, 2, 1, 10, 2, 1000, 0, 1, 6, 5200, 0, 1100, 7, 3, 70, 0, 0, 0, 9, 1, 0);
INSERT INTO "items" VALUES (37, 'Templar Sword', 1, 8, 1, 1, 8, 0, 1, 12, 0, 1000, 0, 1, 6, 1000, 0, 1200, 7, 5, 0, 0, -1, 1, 8, 1, 0);
INSERT INTO "items" VALUES (38, 'Broad Sword', 1, 9, 1, 2, 7, 0, 2, 8, 0, 800, 0, 1, 7, 250, 1, 2800, 8, 6, 0, 0, -3, 2, 8, 1, 0);
INSERT INTO "items" VALUES (39, 'Broad Sword +1', 1, 9, 1, 2, 7, 1, 2, 8, 1, 1400, 0, 1, 7, 1100, 1, 2800, 8, 6, 30, 0, -3, 2, 8, 1, 0);
INSERT INTO "items" VALUES (40, 'Broad Sword +2', 1, 9, 1, 2, 7, 2, 2, 8, 2, 1400, 0, 1, 7, 4400, 0, 2800, 8, 6, 70, 0, -3, 2, 8, 1, 0);
INSERT INTO "items" VALUES (41, 'Broad Sword (S.C)', 1, 9, 1, 2, 7, 0, 2, 8, 0, 1400, 0, 1, 7, 400, 0, 2800, 8, 6, 0, 0, -3, 2, 8, 1, 0);
INSERT INTO "items" VALUES (42, 'Bastad Sword', 1, 9, 1, 2, 8, 0, 2, 9, 0, 800, 0, 1, 8, 300, 1, 3300, 9, 6, 0, 0, -4, 3, 8, 1, 0);
INSERT INTO "items" VALUES (43, 'Bastard Sword +1', 1, 9, 1, 2, 8, 1, 2, 9, 1, 1400, 0, 1, 8, 1200, 1, 3300, 9, 6, 35, 0, -4, 3, 8, 1, 0);
INSERT INTO "items" VALUES (44, 'Bastard Sword +2', 1, 9, 1, 2, 8, 2, 2, 9, 2, 1400, 0, 1, 8, 4800, 0, 3300, 9, 6, 70, 0, -4, 3, 8, 1, 0);
INSERT INTO "items" VALUES (45, 'Bastard Sword (S.C)', 1, 9, 1, 2, 5, 0, 2, 9, 0, 1400, 0, 1, 8, 450, 0, 3300, 9, 6, 0, 0, -4, 3, 8, 1, 0);
INSERT INTO "items" VALUES (46, 'Claymore', 1, 9, 1, 2, 9, 0, 2, 10, 0, 1000, 0, 1, 9, 400, 1, 4000, 10, 7, 0, 0, -5, 4, 8, 1, 0);
INSERT INTO "items" VALUES (47, 'Claymore +1', 1, 9, 1, 2, 9, 1, 2, 10, 1, 1400, 0, 1, 9, 1800, 1, 4000, 10, 7, 40, 0, -5, 4, 8, 1, 0);
INSERT INTO "items" VALUES (48, 'Claymore +2', 1, 9, 1, 2, 9, 2, 2, 10, 2, 1400, 0, 1, 9, 7200, 0, 4000, 10, 7, 70, 0, -5, 4, 8, 1, 0);
INSERT INTO "items" VALUES (49, 'Claymore (S.C)', 1, 9, 1, 2, 9, 0, 2, 10, 0, 1400, 0, 1, 9, 600, 0, 4000, 10, 7, 0, 0, -5, 4, 8, 1, 0);
INSERT INTO "items" VALUES (50, 'Great Sword', 1, 9, 1, 2, 10, 0, 2, 11, 0, 1000, 0, 1, 10, 500, 1, 5200, 11, 8, 0, 0, -8, 5, 8, 1, 0);
INSERT INTO "items" VALUES (51, 'Great Sword +1', 1, 9, 1, 2, 10, 1, 2, 11, 1, 1400, 0, 1, 10, 2300, 1, 5200, 11, 8, 50, 0, -8, 5, 8, 1, 0);
INSERT INTO "items" VALUES (52, 'Great Sword +2', 1, 9, 1, 2, 10, 2, 2, 11, 2, 1400, 0, 1, 10, 9200, 0, 5200, 11, 8, 70, 0, -8, 5, 8, 1, 0);
INSERT INTO "items" VALUES (53, 'Great Sword (S.C)', 1, 9, 1, 2, 10, 0, 2, 11, 0, 1400, 0, 1, 10, 700, 0, 5200, 11, 8, 0, 0, -8, 5, 8, 1, 0);
INSERT INTO "items" VALUES (54, 'Flameberge', 1, 9, 1, 2, 11, 0, 2, 12, 0, 1000, 0, 1, 11, 700, 1, 6000, 12, 10, 0, 0, -15, 10, 8, 1, 0);
INSERT INTO "items" VALUES (55, 'Flameberge +1', 1, 9, 1, 2, 11, 1, 2, 12, 1, 1400, 0, 1, 11, 3300, 1, 6000, 12, 10, 60, 0, -15, 10, 8, 1, 0);
INSERT INTO "items" VALUES (56, 'Flameberge +2', 1, 9, 1, 2, 11, 2, 2, 12, 2, 1400, 0, 1, 11, 13200, 0, 6000, 12, 10, 70, 0, -15, 10, 8, 1, 0);
INSERT INTO "items" VALUES (57, 'Flameberge (S.C)', 1, 9, 1, 2, 11, 0, 2, 12, 0, 1400, 0, 1, 11, 1000, 0, 6000, 12, 10, 0, 0, -15, 10, 8, 1, 0);
INSERT INTO "items" VALUES (59, 'Light Axe', 1, 8, 1, 1, 6, 0, 1, 7, 0, 500, 0, 15, 2, 100, 1, 1400, 22, 3, 0, 0, 0, 0, 10, 1, 0);
INSERT INTO "items" VALUES (60, 'Light Axe +1', 1, 8, 1, 1, 6, 1, 1, 7, 1, 500, 0, 15, 2, 350, 1, 1400, 22, 3, 10, 0, 0, 0, 10, 1, 0);
INSERT INTO "items" VALUES (61, 'Light Axe +2', 1, 8, 1, 1, 6, 2, 1, 7, 2, 500, 0, 15, 2, 1400, 0, 1400, 22, 3, 70, 0, 0, 0, 10, 1, 0);
INSERT INTO "items" VALUES (62, 'Tomahoc', 1, 8, 1, 2, 3, 0, 2, 4, 0, 500, 0, 15, 2, 180, 1, 1700, 22, 4, 0, 0, 0, 0, 10, 1, 0);
INSERT INTO "items" VALUES (63, 'Tomahoc +1', 1, 8, 1, 2, 3, 1, 2, 4, 1, 500, 0, 15, 2, 700, 1, 1700, 22, 4, 20, 0, 0, 0, 10, 1, 0);
INSERT INTO "items" VALUES (64, 'Tomahoc +2', 1, 8, 1, 2, 3, 2, 2, 4, 2, 500, 0, 15, 2, 2800, 0, 1700, 22, 4, 70, 0, 0, 0, 10, 1, 0);
INSERT INTO "items" VALUES (65, 'Saxon Axe', 1, 8, 1, 2, 4, 0, 2, 5, 0, 700, 0, 15, 3, 200, 1, 2000, 23, 4, 0, 0, 0, 0, 10, 1, 0);
INSERT INTO "items" VALUES (66, 'Saxon Axe +1', 1, 8, 1, 2, 4, 1, 2, 5, 1, 700, 0, 15, 3, 800, 1, 2000, 23, 4, 30, 0, 0, 0, 10, 1, 0);
INSERT INTO "items" VALUES (67, 'Saxon Axe +2', 1, 8, 1, 2, 4, 2, 2, 5, 2, 700, 0, 15, 3, 3200, 0, 2000, 23, 4, 70, 0, 0, 0, 10, 1, 0);
INSERT INTO "items" VALUES (68, 'Double Axe', 1, 8, 1, 3, 3, 0, 3, 3, 0, 800, 0, 15, 0, 560, 1, 3300, 20, 5, 0, 0, 0, 0, 10, 1, 0);
INSERT INTO "items" VALUES (69, 'Double Axe +1', 1, 8, 1, 3, 3, 1, 3, 3, 1, 800, 0, 15, 0, 1200, 1, 3300, 20, 5, 40, 0, 0, 0, 10, 1, 0);
INSERT INTO "items" VALUES (70, 'Double Axe +2', 1, 8, 1, 3, 3, 2, 3, 3, 2, 800, 0, 15, 0, 4800, 0, 3300, 20, 5, 70, 0, 0, 0, 10, 1, 0);
INSERT INTO "items" VALUES (71, 'War Axe', 1, 8, 1, 3, 5, 0, 3, 6, 0, 1000, 0, 15, 1, 700, 1, 4400, 21, 8, 0, 0, 0, 0, 10, 1, 0);
INSERT INTO "items" VALUES (72, 'War Axe +1', 1, 8, 1, 3, 5, 1, 3, 6, 1, 1000, 0, 15, 1, 2000, 1, 4400, 21, 8, 60, 0, 0, 0, 10, 1, 0);
INSERT INTO "items" VALUES (73, 'War Axe +2', 1, 8, 1, 3, 5, 2, 3, 6, 2, 1000, 0, 15, 1, 8000, 0, 4400, 21, 8, 70, 0, 0, 0, 10, 1, 0);
INSERT INTO "items" VALUES (74, 'Golden Axe', 1, 8, 1, 3, 6, 0, 3, 6, 0, 2000, 0, 15, 4, 52000, 0, 2000, 24, 6, 60, 0, 0, 0, 10, 1, 0);
INSERT INTO "items" VALUES (75, 'Short Bow', 1, 9, 3, 1, 6, 0, 1, 6, 0, 500, 0, 2, 0, 100, 1, 800, 40, 4, 0, 0, 5, 0, 6, 3, 0);
INSERT INTO "items" VALUES (76, 'Long Bow', 1, 9, 3, 1, 7, 0, 1, 7, 0, 800, 0, 2, 1, 200, 1, 1200, 41, 5, 0, 0, 0, 5, 6, 3, 0);
INSERT INTO "items" VALUES (77, 'Arrow', 6, 0, 0, 1, 4, 0, 1, 4, 0, 1, 0, 6, 10, 1, 1, 10, 1, 3, 0, 0, 0, 0, 0, 4, 0);
INSERT INTO "items" VALUES (78, 'Poison Arrow', 6, 0, 0, 1, 8, 0, 1, 8, 0, 1, 0, 6, 10, 5, 0, 10, 1, 5, 0, 0, 0, 0, 0, 4, 0);
INSERT INTO "items" VALUES (79, 'Wood Shield', 1, 7, 2, 8, 1, 1, 1, 1, 1, 300, 0, 3, 0, 100, 1, 800, 1, 0, 0, 0, 0, 0, 11, 5, 0);
INSERT INTO "items" VALUES (80, 'Leather Shield', 1, 7, 2, 10, 1, 1, 1, 1, 1, 300, 0, 3, 1, 150, 1, 1000, 2, 0, 0, 0, 0, 0, 11, 5, 0);
INSERT INTO "items" VALUES (81, 'Targe Shield', 1, 7, 2, 13, 1, 1, 1, 1, 1, 500, 0, 3, 2, 250, 1, 1800, 3, 0, 0, 0, 0, 0, 11, 5, 0);
INSERT INTO "items" VALUES (82, 'Scrutum Shield', 1, 7, 2, 16, 1, 1, 1, 1, 1, 500, 0, 3, 3, 300, 1, 2000, 4, 0, 0, 0, 0, 0, 11, 5, 0);
INSERT INTO "items" VALUES (83, 'Blonde Shield', 1, 7, 2, 18, 1, 1, 1, 1, 1, 500, 0, 3, 4, 450, 1, 2000, 5, 0, 0, 0, 0, 0, 11, 5, 0);
INSERT INTO "items" VALUES (84, 'Iron Shield', 1, 7, 2, 22, 1, 1, 1, 1, 1, 500, 0, 3, 5, 700, 1, 2500, 6, 0, 0, 0, -2, 0, 11, 5, 0);
INSERT INTO "items" VALUES (85, 'Lagi Shield', 1, 7, 2, 26, 1, 1, 1, 1, 1, 800, 0, 3, 6, 1300, 1, 3000, 7, 0, 0, 0, -4, 0, 11, 5, 0);
INSERT INTO "items" VALUES (86, 'Kite Shield', 1, 7, 2, 30, 1, 1, 1, 1, 1, 800, 0, 3, 7, 1500, 1, 3200, 8, 0, 0, 0, -8, -5, 11, 5, 0);
INSERT INTO "items" VALUES (87, 'Tower Shield', 1, 7, 2, 35, 1, 1, 1, 1, 1, 800, 0, 3, 8, 1800, 1, 4000, 9, 0, 0, 0, -10, -10, 11, 5, 0);
INSERT INTO "items" VALUES (88, 'Guild Admission Ticket', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 9, 5, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (89, 'Guild Secession Ticket', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 9, 5, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (90, 'Gold', 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 1, 1, 0, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (91, 'Health Potion', 7, 0, 4, 2, 12, 10, 0, 0, 0, 1, 0, 6, 1, 10, 1, 30, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (92, 'Big Health Potion', 7, 0, 4, 3, 8, 40, 0, 0, 0, 1, 0, 6, 2, 65, 1, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (93, 'Mana Potion', 7, 0, 5, 2, 12, 10, 0, 0, 0, 1, 0, 6, 3, 10, 1, 30, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (94, 'Big Mana Potion', 7, 0, 5, 4, 8, 50, 0, 0, 0, 1, 0, 6, 4, 65, 1, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (95, 'Revitalizing Potion', 7, 0, 6, 2, 12, 10, 0, 0, 0, 1, 0, 6, 5, 10, 1, 30, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (96, 'Big Revitalizing Potion', 7, 0, 6, 4, 8, 50, 0, 0, 0, 1, 0, 6, 6, 65, 1, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (97, 'Dilution Potion', 3, 0, 9, 23, 20, 0, 0, 0, 0, 1, 0, 6, 5, 200, 1, 30, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (98, 'Baguette', 7, 0, 7, 2, 8, 10, 0, 0, 0, 1, 0, 6, 8, 5, 1, 17, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (99, 'Meat', 7, 0, 7, 4, 8, 10, 0, 0, 0, 1, 0, 6, 7, 10, 1, 52, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (100, 'Fish', 7, 0, 7, 4, 8, 10, 0, 0, 0, 1, 0, 6, 11, 30, 1, 30, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (101, 'Red Fish', 7, 0, 7, 8, 4, 60, 0, 0, 0, 1, 0, 6, 14, 400, 0, 100, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (102, 'Green Fish', 7, 0, 7, 8, 4, 40, 0, 0, 0, 1, 0, 6, 15, 200, 0, 100, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (103, 'Yellow Fish', 7, 0, 7, 8, 4, 30, 0, 0, 0, 1, 0, 6, 16, 100, 0, 100, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (104, 'Map', 9, 0, 10, 1, 0, 0, 0, 0, 0, 35, 0, 6, 9, 30, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (105, 'Fishing Rod', 8, 0, 0, 1, 8, 0, 0, 0, 0, 200, 0, 14, 0, 100, 1, 300, -1, 0, 0, 0, 0, 0, 1, 43, 0);
INSERT INTO "items" VALUES (106, 'Pretend Corpse Manual', 3, 0, 9, 19, 20, 0, 0, 0, 0, 0, 0, 6, 92, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (107, 'Bow & Arrow Manual', 3, 0, 9, 6, 20, 0, 0, 0, 0, 0, 0, 6, 92, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (108, 'Shield Manual', 3, 0, 9, 11, 20, 0, 0, 0, 0, 0, 0, 6, 92, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (109, 'Long Sword Manual', 3, 0, 9, 8, 20, 0, 0, 0, 0, 0, 0, 6, 92, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (110, 'Fencing Manual', 3, 0, 9, 9, 20, 0, 0, 0, 0, 0, 0, 6, 92, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (111, 'Fishing Manual', 3, 0, 9, 1, 20, 0, 0, 0, 0, 0, 0, 6, 92, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (112, 'Axe Manual', 3, 0, 9, 10, 20, 0, 0, 0, 0, 0, 0, 6, 92, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (113, 'Magic Resist Manual', 3, 0, 9, 3, 20, 0, 0, 0, 0, 0, 0, 6, 92, 500, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (114, 'Recall Scroll', 3, 0, 11, 1, 0, 0, 0, 0, 0, 1, 0, 6, 9, 120, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (115, 'Invisibility Scroll', 3, 0, 11, 2, 0, 0, 0, 0, 0, 1, 0, 6, 9, 560, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (116, 'Detect Invisibility Scroll', 3, 0, 11, 3, 0, 0, 0, 0, 0, 1, 0, 6, 9, 330, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (117, 'Bleeding Island Ticket', 3, 0, 11, 4, 1, 0, 0, 0, 0, 1, 0, 6, 9, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (188, 'Snake Meat', 7, 0, 7, 4, 8, 20, 0, 0, 0, 1, 0, 6, 17, 57, 0, 45, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (189, 'Snake Skin', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 18, 175, 0, 20, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (190, 'Snake Teeth', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 19, 55, 0, 5, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (191, 'Snake Tongue', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 20, 50, 0, 3, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (192, 'Ant Leg', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 21, 32, 0, 17, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (193, 'Ant Antenna', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 22, 34, 0, 10, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (194, 'Cyclops Eye', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 23, 170, 0, 330, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (195, 'Cyclops Hand Edge', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 24, 270, 0, 4000, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (196, 'Cyclops Heart', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 25, 160, 0, 1230, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (197, 'Cyclops Meat', 7, 0, 7, 4, 8, 30, 0, 0, 0, 1, 0, 6, 26, 90, 0, 500, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (198, 'Cyclops Leather', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 27, 600, 0, 350, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (199, 'Helhound Heart', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 28, 90, 0, 820, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (200, 'Helhound Leather', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 29, 320, 0, 200, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (201, 'Helhound Tail', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 30, 70, 0, 250, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (202, 'Helhound Teeth', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 31, 70, 0, 130, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (203, 'Helhound Claw', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 32, 70, 0, 280, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (204, 'Helhound Tongue', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 33, 75, 0, 150, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (205, 'Lump of Clay', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 34, 95, 0, 230, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (206, 'Orc Meat', 7, 0, 7, 2, 4, 5, 0, 0, 0, 1, 0, 6, 35, 50, 0, 100, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (207, 'Orc Leather', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 36, 193, 0, 100, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (208, 'Orc Teeth', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 37, 56, 0, 50, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (209, 'Ogre Hair', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 38, 230, 0, 250, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (210, 'Ogre Heart', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 39, 340, 0, 1580, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (211, 'Ogre Meat', 7, 0, 7, 4, 8, 50, 0, 0, 0, 1, 0, 6, 40, 200, 0, 710, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (212, 'Ogre Leather', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 41, 840, 0, 650, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (213, 'Ogre Teeth', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 42, 215, 0, 230, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (214, 'Ogre Claw', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 43, 215, 0, 370, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (215, 'Scorpion Pincers', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 44, 50, 0, 1200, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (216, 'Scorpion Meat', 7, 0, 7, 4, 8, 25, 0, 0, 0, 1, 0, 6, 45, 55, 0, 450, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (217, 'Scorpion Sting', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 46, 55, 0, 1000, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (218, 'Scorpion Skin', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 47, 90, 0, 380, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (219, 'Skeleton Bones', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 48, 50, 0, 1300, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (220, 'Slime Jelly', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 49, 10, 0, 100, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (221, 'Stone Golem Piece', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 50, 50, 0, 500, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (222, 'Troll Heart', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 51, 145, 0, 1050, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (223, 'Troll Meat', 7, 0, 7, 8, 4, 50, 0, 0, 0, 1, 0, 6, 52, 180, 0, 500, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (224, 'Troll Leather', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 53, 335, 0, 450, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (225, 'Troll Claw', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 54, 70, 0, 290, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (226, 'Alchemy Manual', 3, 0, 9, 12, 20, 0, 0, 0, 0, 0, 0, 6, 92, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (227, 'Alchemy Bowl', 10, 0, 0, 1, 0, 0, 0, 0, 0, 300, 0, 6, 55, 1000, 1, 700, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (230, 'Mining Manual', 3, 0, 9, 0, 20, 0, 0, 0, 0, 0, 0, 6, 92, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (231, 'Pick Axe', 1, 8, 1, 2, 4, 0, 2, 4, 0, 1000, 0, 15, 5, 500, 1, 1000, 25, 10, 0, 0, 0, 0, 0, 1, 0);
INSERT INTO "items" VALUES (232, 'Hoe', 1, 8, 1, 2, 4, 0, 2, 4, 0, 800, 0, 15, 9, 300, 1, 1000, 27, 5, 0, 0, 0, 0, -1, 1, 0);
INSERT INTO "items" VALUES (235, 'Manufacturing Manual', 3, 0, 9, 13, 20, 0, 0, 0, 0, 0, 0, 6, 92, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (236, 'Smiths Anvil', 10, 0, 0, 1, 0, 0, 0, 0, 0, 300, 0, 6, 113, 1500, 1, 2000, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (237, 'Hammer Attack Manual', 3, 0, 9, 14, 20, 0, 0, 0, 0, 0, 0, 6, 92, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (245, 'Aresden Master Flag', 11, 0, 16, 1, 1, 0, 0, 0, 0, 1, 0, 6, 56, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (246, 'Elvine Master Flag', 11, 0, 16, 2, 1, 0, 0, 0, 0, 1, 0, 6, 57, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (247, 'Aresden Flag', 11, 0, 16, 1, 0, 0, 0, 0, 0, 1, 0, 6, 56, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (248, 'Elvine Flag', 11, 0, 16, 2, 0, 0, 0, 0, 0, 1, 0, 6, 57, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (250, 'Staff Attack Manual', 3, 0, 9, 21, 20, 0, 0, 0, 0, 0, 0, 6, 92, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (256, 'Magic Wand (MS20)', 1, 8, 13, 2, 4, 0, 20, 0, 0, 3600, 0, 17, 1, 5000, 1, 1000, 36, 3, 50, 0, 0, 0, 21, 8, 0);
INSERT INTO "items" VALUES (257, 'Magic Wand (MS10)', 1, 8, 13, 2, 4, 0, 10, 0, 0, 2400, 0, 17, 1, 2500, 1, 1000, 36, 3, 40, 0, 0, 0, 21, 8, 0);
INSERT INTO "items" VALUES (258, 'Magic Wand (MS0)', 1, 8, 13, 2, 4, 0, 0, 0, 0, 1200, 0, 17, 1, 1000, 1, 1000, 36, 3, 30, 0, 0, 0, 21, 8, 0);
INSERT INTO "items" VALUES (259, 'Magic Wand (M.Shield)', 1, 8, 20, 2, 4, 0, 0, 0, 0, 5000, 30, 17, 1, 8200, 0, 1000, 36, 3, 0, 0, 0, 0, 21, 8, 0);
INSERT INTO "items" VALUES (270, 'Hair Color Potion', 7, 0, 12, 1, 0, 0, 0, 0, 0, 1, 0, 6, 5, 300, 0, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (271, 'Hair Style Potion', 7, 0, 12, 2, 0, 0, 0, 0, 0, 1, 0, 6, 5, 400, 0, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (272, 'Skin Color Potion', 7, 0, 12, 3, 0, 0, 0, 0, 0, 1, 0, 6, 5, 500, 0, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (273, 'Invisibility Potion', 7, 0, 11, 2, 0, 0, 0, 0, 0, 1, 0, 6, 5, 700, 0, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (274, 'Sex Change Potion', 7, 0, 12, 4, 0, 0, 0, 0, 0, 1, 0, 6, 5, 4000, 0, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (275, 'Ogre Potion', 7, 0, 11, 5, 8, 0, 0, 0, 0, 1, 0, 6, 5, 5000, 0, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (276, 'Underwear Potion', 7, 0, 12, 5, 0, 0, 0, 0, 0, 1, 0, 6, 5, 4000, 0, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (290, 'Flameberge +3 (LLF)', 1, 9, 1, 2, 11, 3, 2, 12, 3, 8000, 0, 1, 11, 21000, 0, 6000, 12, 10, 0, 0, -15, 10, 8, 1, 0);
INSERT INTO "items" VALUES (291, 'Magic Wand (MS30-LLF)', 1, 8, 13, 2, 4, 0, 30, 0, 0, 4800, 0, 17, 0, 10000, 0, 1000, 35, 3, 0, 0, 0, 0, 21, 8, 0);
INSERT INTO "items" VALUES (292, 'Golden Axe (LLF)', 1, 8, 1, 2, 8, 0, 2, 8, 0, 2000, 0, 15, 4, 32000, 0, 2000, 24, 6, 0, 0, 0, 0, 10, 1, 0);
INSERT INTO "items" VALUES (300, 'Magic Necklace (RM10)', 1, 6, 14, 1, 10, 0, 0, 0, 0, 300, 0, 16, 4, 2250, 0, 400, -1, 0, 30, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (305, 'Magic Necklace (DM+1)', 1, 6, 14, 3, 1, 0, 0, 0, 0, 300, 0, 16, 5, 39800, 0, 400, -1, 0, 65, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (308, 'Magic Necklace (MS10)', 1, 6, 14, 2, 10, 0, 0, 0, 0, 300, 0, 16, 7, 7000, 0, 400, -1, 0, 50, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (311, 'Magic Necklace (DF+10)', 1, 6, 14, 4, 10, 0, 0, 0, 0, 300, 0, 16, 6, 19000, 0, 400, -1, 0, 50, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (315, 'Gold Necklace', 1, 6, 0, 0, 0, 0, 0, 0, 0, 300, 0, 16, 8, 1000, 1, 300, -1, 0, 0, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (316, 'Silver Necklace', 1, 6, 0, 0, 0, 0, 0, 0, 0, 300, 0, 16, 9, 500, 1, 300, -1, 0, 0, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (331, 'Gold Ring', 1, 10, 0, 0, 0, 0, 0, 0, 0, 300, 0, 16, 14, 700, 1, 200, -1, 0, 0, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (332, 'Silver Ring', 1, 10, 0, 0, 0, 0, 0, 0, 0, 300, 0, 16, 15, 350, 1, 200, -1, 0, 0, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (333, 'Platinum Ring', 1, 10, 0, 0, 0, 0, 0, 0, 0, 300, 0, 16, 15, 750, 0, 200, -1, 0, 0, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (334, 'Lucky Gold Ring', 1, 10, 14, 5, 0, 0, 0, 0, 0, 300, 0, 16, 13, 2750, 0, 200, -1, 0, 0, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (335, 'Emerald Ring', 1, 10, 15, 0, 0, 0, 0, 0, 0, 9000, 0, 16, 10, 2500, 0, 200, -1, 0, 0, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (336, 'Sapphire Ring', 1, 10, 0, 0, 0, 0, 0, 0, 0, 5000, 0, 16, 11, 2450, 0, 200, -1, 0, 0, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (337, 'Ruby Ring', 1, 10, 15, 0, 0, 0, 0, 0, 0, 5000, 0, 16, 12, 1800, 0, 200, -1, 0, 0, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (338, 'Memorial Ring', 1, 10, 0, 0, 0, 0, 0, 0, 0, 300, 0, 16, 12, 1000, 0, 200, -1, 0, 0, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (350, 'Diamond', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 16, 37, 3000, 0, 200, 1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (351, 'Ruby', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 16, 38, 2000, 0, 200, 1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (352, 'Sapphire', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 16, 39, 2000, 0, 200, 1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (353, 'Emerald', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 16, 40, 2000, 0, 200, 1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (354, 'Gold Nugget', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 16, 16, 300, 0, 300, 1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (355, 'Coal', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 16, 17, 50, 0, 180, 1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (356, 'Silver Nugget', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 16, 18, 200, 0, 220, 1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (357, 'Iron Ore', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 16, 19, 100, 0, 250, 1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (358, 'Crystal', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 16, 41, 300, 0, 200, 1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (360, 'Dye (Indigo)', 11, 0, 17, 1, 0, 0, 0, 0, 0, 1, 0, 6, 58, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (361, 'Dye (Crimson-Red)', 11, 0, 17, 2, 0, 0, 0, 0, 0, 1, 0, 6, 58, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (362, 'Dye (Brown)', 11, 0, 17, 3, 0, 0, 0, 0, 0, 1, 0, 6, 58, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (363, 'Dye (Gold)', 11, 0, 17, 4, 0, 0, 0, 0, 0, 1, 0, 6, 58, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (364, 'Dye (Green)', 11, 0, 17, 5, 0, 0, 0, 0, 0, 1, 0, 6, 62, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (365, 'Dye (Gray)', 11, 0, 17, 6, 0, 0, 0, 0, 0, 1, 0, 6, 63, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (366, 'Dye (Aqua)', 11, 0, 17, 7, 0, 0, 0, 0, 0, 1, 0, 6, 58, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (367, 'Dye (Pink)', 11, 0, 17, 8, 0, 0, 0, 0, 0, 1, 0, 6, 58, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (368, 'Dye (Violet)', 11, 0, 17, 9, 0, 0, 0, 0, 0, 1, 0, 6, 66, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (369, 'Dye (Blue)', 11, 0, 17, 10, 0, 0, 0, 0, 0, 1, 0, 6, 67, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (370, 'Dye (Tan)', 11, 0, 17, 11, 0, 0, 0, 0, 0, 1, 0, 6, 68, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (371, 'Dye (Khaki)', 11, 0, 17, 12, 0, 0, 0, 0, 0, 1, 0, 6, 69, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (372, 'Dye (Yellow)', 11, 0, 17, 13, 0, 0, 0, 0, 0, 1, 0, 6, 70, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (373, 'Dye (Red)', 11, 0, 17, 14, 0, 0, 0, 0, 0, 1, 0, 6, 71, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (374, 'Dye (Black)', 11, 0, 17, 15, 0, 0, 0, 0, 0, 1, 0, 6, 58, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (375, 'Dye Removal', 11, 0, 17, 0, 0, 0, 0, 0, 0, 1, 0, 6, 58, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (380, 'Ice Storm Manual', 3, 0, 18, 55, 0, 0, 0, 0, 0, 0, 0, 6, 91, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 1);
INSERT INTO "items" VALUES (381, 'Mass Fire Strike Manual', 3, 0, 18, 61, 0, 0, 0, 0, 0, 0, 0, 6, 91, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 2);
INSERT INTO "items" VALUES (382, 'Bloody Shock Wave Manual', 3, 0, 18, 70, 0, 0, 0, 0, 0, 0, 0, 6, 91, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 2);
INSERT INTO "items" VALUES (385, 'Hand Attack Manual', 3, 0, 9, 5, 20, 0, 0, 0, 0, 0, 0, 6, 92, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (386, 'Short Sword Manual', 3, 0, 9, 7, 20, 0, 0, 0, 0, 0, 0, 6, 92, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (390, 'Power Green Potion', 7, 0, 22, 300, 0, 0, 0, 0, 0, 1, 0, 6, 5, 600, 0, 30, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (391, 'SuperGreenPotion', 7, 0, 22, 600, 0, 0, 0, 0, 0, 1, 0, 6, 5, 1200, 0, 30, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (400, 'Aresden-Hero''s Cape', 1, 12, 2, 2, 1, 1, 1, 1, 1, 300, 0, 20, 0, 2000, 0, 200, 1, 0, 0, 0, 0, 0, -1, 13, 0);
INSERT INTO "items" VALUES (401, 'Elvine-Hero''s Cape', 1, 12, 2, 2, 1, 1, 1, 1, 1, 300, 0, 20, 1, 2000, 0, 200, 2, 0, 0, 0, 0, 0, -1, 13, 0);
INSERT INTO "items" VALUES (402, 'Cape', 1, 12, 2, 2, 1, 1, 1, 1, 1, 300, 0, 20, 2, 1000, 1, 200, 3, 0, 0, 0, 0, 0, -1, 12, 0);
INSERT INTO "items" VALUES (403, 'Aresden Hero Helm (M)', 1, 1, 2, 18, 24, 0, 0, 0, 0, 2500, 0, 21, 7, 16000, 0, 10000, 10, 0, 0, 1, -7, -7, -1, 6, 0);
INSERT INTO "items" VALUES (404, 'Aresden Hero Helm (W)', 1, 1, 2, 18, 24, 0, 0, 0, 0, 2500, 0, 21, 7, 16000, 0, 10000, 10, 0, 0, 2, -7, -7, -1, 6, 0);
INSERT INTO "items" VALUES (405, 'Elvine Hero Helm (M)', 1, 1, 2, 18, 24, 0, 0, 0, 0, 2500, 0, 21, 6, 16000, 0, 10000, 9, 0, 0, 1, -7, -7, -1, 6, 0);
INSERT INTO "items" VALUES (406, 'Elvine Hero Helm (W)', 1, 1, 2, 18, 24, 0, 0, 0, 0, 2500, 0, 21, 6, 16000, 0, 10000, 9, 0, 0, 2, -7, -7, -1, 6, 0);
INSERT INTO "items" VALUES (407, 'Aresden Hero Cap (M)', 1, 1, 2, 12, 15, 0, 0, 0, 0, 1500, 0, 21, 9, 12000, 0, 1500, 12, 0, 0, 1, -7, -7, -1, 6, 0);
INSERT INTO "items" VALUES (408, 'Aresden Hero Cap (W)', 1, 1, 2, 12, 15, 0, 0, 0, 0, 1500, 0, 21, 9, 12000, 0, 1500, 12, 0, 0, 2, -7, -7, -1, 6, 0);
INSERT INTO "items" VALUES (409, 'Elvine Hero Cap (M)', 1, 1, 2, 12, 15, 0, 0, 0, 0, 1500, 0, 21, 8, 12000, 0, 1500, 11, 0, 0, 1, -7, -7, -1, 6, 0);
INSERT INTO "items" VALUES (410, 'Elvine Hero Cap (W)', 1, 1, 2, 12, 15, 0, 0, 0, 0, 1500, 0, 21, 8, 12000, 0, 1500, 11, 0, 0, 2, -7, -7, -1, 6, 0);
INSERT INTO "items" VALUES (411, 'Aresden Hero Armor (M)', 1, 2, 2, 45, 48, 0, 0, 0, 0, 5000, 0, 9, 8, 40000, 0, 10000, 9, 0, 0, 1, -5, -5, 0, 6, 0);
INSERT INTO "items" VALUES (412, 'Aresden Hero Armor (W)', 1, 2, 2, 45, 48, 0, 0, 0, 0, 5000, 0, 13, 9, 40000, 0, 10000, 10, 0, 0, 2, -5, -5, 0, 6, 0);
INSERT INTO "items" VALUES (413, 'Elvine Hero Armor (M)', 1, 2, 2, 45, 48, 0, 0, 0, 0, 5000, 0, 9, 7, 40000, 0, 10000, 8, 0, 0, 1, -5, -5, 0, 6, 0);
INSERT INTO "items" VALUES (414, 'Elvine Hero Armor (W)', 1, 2, 2, 45, 48, 0, 0, 0, 0, 5000, 0, 13, 8, 40000, 0, 10000, 9, 0, 0, 2, -5, -5, 0, 6, 0);
INSERT INTO "items" VALUES (415, 'Aresden Hero Robe (M)', 1, 2, 2, 20, 20, 0, 0, 0, 0, 4000, 0, 9, 10, 30000, 0, 1500, 11, 0, 0, 1, 0, 0, 0, 13, 0);
INSERT INTO "items" VALUES (416, 'Aresden Hero Robe (M)', 1, 2, 2, 20, 20, 0, 0, 0, 0, 4000, 0, 13, 11, 30000, 0, 1500, 12, 0, 0, 2, 0, 0, 0, 13, 0);
INSERT INTO "items" VALUES (417, 'Elvine Hero Robe (M)', 1, 2, 2, 20, 20, 0, 0, 0, 0, 4000, 0, 9, 9, 30000, 0, 1500, 10, 0, 0, 1, 0, 0, 0, 13, 0);
INSERT INTO "items" VALUES (418, 'Elvine Hero Robe (W)', 1, 2, 2, 20, 20, 0, 0, 0, 0, 4000, 0, 13, 10, 30000, 0, 1500, 11, 0, 0, 2, 0, 0, 0, 13, 0);
INSERT INTO "items" VALUES (419, 'Aresden Hero Hauberk (M)', 1, 3, 2, 15, 15, 0, 0, 0, 0, 2000, 0, 7, 3, 14000, 0, 1500, 4, 0, 0, 1, -3, -3, 0, 6, 0);
INSERT INTO "items" VALUES (420, 'Aresden Hero Hauberk (W)', 1, 3, 2, 15, 15, 0, 0, 0, 0, 2000, 0, 11, 4, 14000, 0, 1500, 5, 0, 0, 2, -3, -3, 0, 6, 0);
INSERT INTO "items" VALUES (421, 'Elvine Hero Hauberk (M)', 1, 3, 2, 15, 15, 0, 0, 0, 0, 2000, 0, 7, 2, 14000, 0, 1500, 3, 0, 0, 1, -3, -3, 0, 6, 0);
INSERT INTO "items" VALUES (422, 'Elvine Hero Hauberk (W)', 1, 3, 2, 15, 15, 0, 0, 0, 0, 2000, 0, 11, 3, 14000, 0, 1500, 4, 0, 0, 2, -3, -3, 0, 6, 0);
INSERT INTO "items" VALUES (423, 'Aresden Hero Leggings (M)', 1, 4, 2, 18, 25, 1, 1, 1, 1, 3000, 0, 8, 5, 22000, 0, 1500, 6, 0, 0, 1, -6, -6, 0, 6, 0);
INSERT INTO "items" VALUES (424, 'Aresden Hero Leggings (W)', 1, 4, 2, 18, 25, 1, 1, 1, 1, 3000, 0, 12, 6, 22000, 0, 1500, 7, 0, 0, 2, -6, -6, 0, 6, 0);
INSERT INTO "items" VALUES (425, 'Elvine Hero Leggings (M)', 1, 4, 2, 18, 25, 1, 1, 1, 1, 3000, 0, 8, 4, 22000, 0, 1500, 5, 0, 0, 1, -6, -6, 0, 6, 0);
INSERT INTO "items" VALUES (426, 'Elvine Hero Leggings (W)', 1, 4, 2, 18, 25, 1, 1, 1, 1, 3000, 0, 12, 5, 22000, 0, 1500, 6, 0, 0, 2, -6, -6, 0, 6, 0);
INSERT INTO "items" VALUES (427, 'Aresden-Hero''s Cape +1', 1, 12, 2, 2, 1, 1, 1, 1, 1, 300, 0, 20, 3, 2000, 0, 200, 4, 0, 0, 0, 0, 0, -1, 13, 0);
INSERT INTO "items" VALUES (428, 'Elvine-Hero''s Cape +1', 1, 12, 2, 2, 1, 1, 1, 1, 1, 300, 0, 20, 4, 2000, 0, 200, 5, 0, 0, 0, 0, 0, -1, 13, 0);
INSERT INTO "items" VALUES (429, 'Combatant Cape', 1, 12, 2, 2, 1, 1, 1, 1, 1, 300, 0, 20, 5, 1000, 1, 200, 6, 0, 0, 0, 0, 0, -1, 12, 0);
INSERT INTO "items" VALUES (450, 'Shoes', 1, 5, 2, 1, 1, 1, 1, 1, 1, 300, 0, 5, 0, 20, 1, 200, 1, 0, 0, 0, 0, 0, 0, 12, 0);
INSERT INTO "items" VALUES (451, 'Long Boots', 1, 5, 2, 1, 1, 1, 1, 1, 1, 300, 0, 5, 1, 100, 1, 500, 2, 0, 0, 0, 0, 0, 0, 12, 0);
INSERT INTO "items" VALUES (453, 'Shirt (M)', 1, 3, 2, 1, 1, 1, 1, 1, 1, 300, 0, 7, 0, 20, 1, 100, 1, 0, 0, 1, 0, 0, 0, 11, 0);
INSERT INTO "items" VALUES (454, 'Hauberk (M)', 1, 3, 2, 8, 10, 0, 0, 0, 0, 300, 0, 7, 1, 400, 1, 1200, 2, 0, 0, 1, -3, -3, 0, 6, 0);
INSERT INTO "items" VALUES (455, 'Leather (M)', 1, 2, 2, 10, 10, 0, 0, 0, 0, 400, 0, 9, 0, 500, 1, 1500, 1, 0, 0, 1, -5, -5, 0, 6, 0);
INSERT INTO "items" VALUES (456, 'Chain Mail (M)', 1, 2, 2, 30, 32, 0, 0, 0, 0, 1000, 0, 9, 1, 1200, 1, 3000, 2, 0, 0, 1, -12, -12, 0, 6, 0);
INSERT INTO "items" VALUES (457, 'Scale Mail (M)', 1, 2, 2, 20, 20, 0, 0, 0, 0, 600, 0, 9, 2, 900, 1, 2000, 3, 0, 0, 1, -8, -8, 0, 6, 0);
INSERT INTO "items" VALUES (458, 'Plate Mail (M)', 1, 2, 2, 37, 40, 0, 0, 0, 0, 3000, 0, 9, 3, 4500, 1, 10000, 4, 0, 0, 1, -10, -10, 0, 6, 0);
INSERT INTO "items" VALUES (459, 'Trousers (M)', 1, 4, 2, 1, 1, 1, 1, 1, 1, 300, 0, 8, 0, 80, 1, 100, 1, 0, 0, 1, 0, 0, 0, 11, 0);
INSERT INTO "items" VALUES (460, 'Knee Trousers (M)', 1, 4, 2, 1, 1, 1, 1, 1, 1, 300, 0, 8, 1, 20, 1, 100, 2, 0, 0, 1, 0, 0, 0, 11, 0);
INSERT INTO "items" VALUES (461, 'Chain Hose (M)', 1, 4, 2, 6, 10, 1, 1, 1, 1, 500, 0, 8, 2, 400, 1, 1000, 3, 0, 0, 1, -3, -3, 0, 6, 0);
INSERT INTO "items" VALUES (462, 'Plate Leggings (M)', 1, 4, 2, 12, 20, 1, 1, 1, 1, 1000, 0, 8, 3, 1000, 1, 2000, 4, 0, 0, 1, -6, -6, 0, 6, 0);
INSERT INTO "items" VALUES (470, 'Chemise (W)', 1, 3, 2, 1, 1, 1, 1, 1, 1, 300, 0, 11, 0, 200, 1, 100, 1, 0, 0, 2, 0, 0, 0, 11, 0);
INSERT INTO "items" VALUES (471, 'Shirt (W)', 1, 3, 2, 1, 1, 1, 1, 1, 1, 300, 0, 11, 1, 20, 1, 100, 2, 0, 0, 2, 0, 0, 0, 11, 0);
INSERT INTO "items" VALUES (472, 'Hauberk (W)', 1, 3, 2, 8, 10, 0, 0, 0, 0, 300, 0, 11, 2, 400, 1, 1200, 3, 0, 0, 2, -3, -3, 0, 6, 0);
INSERT INTO "items" VALUES (473, 'Bodice (W)', 1, 2, 2, 1, 1, 1, 1, 1, 1, 400, 0, 13, 0, 150, 1, 200, 1, 0, 0, 2, -1, -1, 0, 11, 0);
INSERT INTO "items" VALUES (474, 'Long Bodice (W)', 1, 2, 2, 1, 1, 1, 1, 1, 1, 400, 0, 13, 1, 180, 1, 200, 2, 0, 0, 2, -1, -1, 0, 11, 0);
INSERT INTO "items" VALUES (475, 'Leather Armor (W)', 1, 2, 2, 10, 10, 0, 0, 0, 0, 400, 0, 13, 2, 500, 1, 1500, 3, 0, 0, 2, -5, -5, 0, 6, 0);
INSERT INTO "items" VALUES (476, 'Chain Mail (W)', 1, 2, 2, 30, 32, 0, 0, 0, 0, 1000, 0, 13, 3, 1200, 1, 3000, 4, 0, 0, 2, -12, -12, 0, 6, 0);
INSERT INTO "items" VALUES (477, 'Scale Mail (W)', 1, 2, 2, 20, 20, 0, 0, 0, 0, 600, 0, 13, 4, 900, 1, 2000, 5, 0, 0, 2, -8, -8, 0, 6, 0);
INSERT INTO "items" VALUES (478, 'Plate Mail (W)', 1, 2, 2, 37, 40, 0, 0, 0, 0, 3000, 0, 13, 5, 4500, 1, 10000, 6, 0, 0, 2, -10, -10, 0, 6, 0);
INSERT INTO "items" VALUES (479, 'Skirt (W)', 1, 4, 2, 1, 1, 1, 1, 1, 1, 300, 0, 12, 0, 200, 1, 100, 1, 0, 0, 2, 0, 0, 0, 11, 0);
INSERT INTO "items" VALUES (480, 'Trousers (W)', 1, 4, 2, 1, 1, 1, 1, 1, 1, 300, 0, 12, 1, 80, 1, 100, 2, 0, 0, 2, 0, 0, 0, 11, 0);
INSERT INTO "items" VALUES (481, 'Knee Trousers (W)', 1, 4, 2, 1, 1, 1, 1, 1, 1, 300, 0, 12, 2, 20, 1, 100, 3, 0, 0, 2, 0, 0, 0, 11, 0);
INSERT INTO "items" VALUES (482, 'Chain Hose (W)', 1, 4, 2, 6, 10, 1, 1, 1, 1, 500, 0, 12, 3, 400, 1, 1000, 4, 0, 0, 2, -3, -3, 0, 6, 0);
INSERT INTO "items" VALUES (483, 'Plate Leggings (W)', 1, 4, 2, 12, 20, 1, 1, 1, 1, 1000, 0, 12, 4, 1000, 1, 2000, 5, 0, 0, 2, -6, -6, 0, 6, 0);
INSERT INTO "items" VALUES (484, 'Tunic (M)', 1, 2, 2, 3, 1, 1, 1, 1, 1, 300, 0, 9, 4, 350, 1, 500, 5, 0, 0, 1, 0, 0, 0, 11, 0);
INSERT INTO "items" VALUES (490, 'Blood Sword', 1, 9, 19, 4, 7, 1, 4, 7, 1, 8000, 5, 1, 7, 31000, 0, 13100, 8, 11, 0, 0, 0, 0, 8, 1, 9);
INSERT INTO "items" VALUES (491, 'Blood Axe', 1, 8, 19, 3, 6, 2, 3, 6, 2, 5000, 5, 15, 1, 25000, 0, 6100, 21, 8, 0, 0, 0, 0, 10, 1, 9);
INSERT INTO "items" VALUES (492, 'Blood Rapier', 1, 8, 19, 2, 5, 2, 2, 5, 2, 5000, 5, 1, 6, 25000, 0, 1100, 7, 3, 0, 0, 0, 0, 9, 1, 9);
INSERT INTO "items" VALUES (500, 'Iron Bar', 12, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 76, 500, 0, 500, 1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (501, 'Super Coal', 12, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 88, 200, 0, 500, 1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (502, 'Ultra Coal', 12, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 88, 500, 0, 500, 1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (503, 'Gold Bar', 12, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 74, 3000, 0, 500, 1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (504, 'Silver Bar', 12, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 75, 2000, 0, 500, 1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (505, 'Blonde Bar', 12, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 77, 1000, 0, 500, 1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (506, 'Mithral Bar', 12, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 78, 6000, 0, 800, 1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (507, 'Blonde Bar', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 79, 160, 0, 180, 1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (508, 'Mithral', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 80, 50, 0, 180, 1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (511, 'Arena Ticket', 3, 0, 11, 4, 11, 0, 0, 0, 0, 1, 0, 6, 89, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (513, 'Arena Ticket (2)', 3, 0, 11, 4, 12, 0, 0, 0, 0, 1, 0, 6, 89, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (515, 'Arena Ticket (3)', 3, 0, 11, 4, 13, 0, 0, 0, 0, 1, 0, 6, 89, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (517, 'Arena Ticket (4)', 3, 0, 11, 4, 14, 0, 0, 0, 0, 1, 0, 6, 89, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (520, 'Bouquette', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 81, 1000, 1, 180, 1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (521, 'Flower Basket', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 82, 1500, 1, 180, 1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (522, 'Flowerpot', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 83, 500, 1, 180, 1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (530, 'Arena Ticket (5)', 3, 0, 11, 4, 15, 0, 0, 0, 0, 1, 0, 6, 89, 10, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (531, 'Arena Ticket (6)', 3, 0, 11, 4, 16, 0, 0, 0, 0, 1, 0, 6, 89, 10, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (532, 'Arena Ticket (7)', 3, 0, 11, 4, 17, 0, 0, 0, 0, 1, 0, 6, 89, 10, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (533, 'Arena Ticket (8)', 3, 0, 11, 4, 18, 0, 0, 0, 0, 1, 0, 6, 89, 10, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (534, 'Arena Ticket (9)', 3, 0, 11, 4, 19, 0, 0, 0, 0, 1, 0, 6, 89, 10, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (540, 'Demon Eye', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 93, 3000, 0, 530, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (541, 'Demon Heart', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 94, 5000, 0, 1500, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (542, 'Demon Meat', 7, 0, 7, 4, 8, 50, 0, 0, 0, 1, 0, 6, 95, 2000, 0, 1200, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (543, 'Demon Leather', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 96, 6000, 0, 900, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (544, 'Unicorn Heart', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 97, 5000, 0, 230, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (545, 'Unicorn Antenna', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 98, 50000, 0, 250, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (546, 'Unicorn Meat', 7, 0, 7, 4, 8, 50, 0, 0, 0, 1, 0, 6, 99, 2000, 0, 500, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (547, 'Unicorn Leather', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 100, 6000, 0, 650, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (548, 'Werewolf Heart', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 101, 210, 0, 350, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (549, 'Werewolf Nail', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 102, 140, 0, 290, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (550, 'Werewolf Meat', 7, 0, 7, 8, 4, 50, 0, 0, 0, 1, 0, 6, 103, 250, 0, 500, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (551, 'Werewolf Tail', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 104, 300, 0, 550, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (552, 'Werewolf Teeth', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 105, 140, 0, 290, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (553, 'Werewolf Leather', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 106, 820, 0, 450, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (554, 'Werewolf Claw', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 107, 140, 0, 290, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (560, 'Battle Axe', 1, 9, 1, 3, 8, 0, 3, 9, 0, 1500, 0, 15, 6, 3500, 1, 9000, 26, 13, 0, 0, 0, 0, 10, 1, 0);
INSERT INTO "items" VALUES (570, 'Red Carp', 7, 0, 7, 8, 4, 30, 0, 0, 0, 1, 0, 6, 14, 1200, 0, 100, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (571, 'Green Carp', 7, 0, 7, 8, 4, 40, 0, 0, 0, 1, 0, 6, 15, 1500, 0, 100, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (572, 'Gold Carp', 7, 0, 7, 8, 4, 60, 0, 0, 0, 1, 0, 6, 16, 3000, 0, 100, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (573, 'Crucian Carp', 7, 0, 7, 8, 4, 0, 0, 0, 0, 1, 0, 6, 12, 200, 0, 30, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (574, 'Blue Sea Bream', 7, 0, 7, 8, 4, 50, 0, 0, 0, 1, 0, 6, 84, 2000, 0, 80, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (575, 'Salmon', 7, 0, 7, 8, 4, 10, 0, 0, 0, 1, 0, 6, 85, 800, 0, 50, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (576, 'Red Sea Bream', 7, 0, 7, 8, 4, 50, 0, 0, 0, 1, 0, 6, 86, 2000, 0, 80, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (577, 'Gray Mullet', 7, 0, 7, 8, 4, 5, 0, 0, 0, 1, 0, 6, 87, 500, 0, 50, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (580, 'Battle Axe +1', 1, 9, 1, 3, 8, 1, 3, 9, 1, 2000, 0, 15, 6, 6000, 1, 9000, 26, 13, 110, 0, 0, 0, 10, 1, 0);
INSERT INTO "items" VALUES (581, 'Battle Axe +2', 1, 9, 1, 3, 8, 2, 3, 9, 2, 4000, 0, 15, 6, 48000, 0, 9000, 26, 13, 0, 0, 0, 0, 10, 1, 0);
INSERT INTO "items" VALUES (582, 'Sabre +2', 1, 8, 1, 1, 8, 2, 1, 8, 2, 1000, 0, 1, 3, 2400, 0, 1200, 4, 5, 20, 0, -1, 0, 8, 1, 0);
INSERT INTO "items" VALUES (590, 'Robe (M)', 1, 2, 2, 12, 1, 0, 14, 100, 0, 3000, 0, 9, 5, 2000, 1, 1000, 6, 0, 0, 1, 0, 0, 0, 11, 0);
INSERT INTO "items" VALUES (591, 'Robe (W)', 1, 2, 2, 12, 1, 0, 14, 100, 0, 3000, 0, 13, 6, 2000, 1, 1000, 7, 0, 0, 2, 0, 0, 0, 11, 0);
INSERT INTO "items" VALUES (600, 'Helm (M)', 1, 1, 2, 5, 10, 0, 0, 0, 0, 1500, 0, 21, 0, 800, 1, 5200, 4, 0, 0, 1, -4, -4, -1, 6, 0);
INSERT INTO "items" VALUES (601, 'Full-Helm (M)', 1, 1, 2, 10, 20, 0, 0, 0, 0, 2000, 0, 21, 1, 1500, 1, 8500, 1, 0, 0, 1, -7, -7, -1, 6, 0);
INSERT INTO "items" VALUES (602, 'Helm (W)', 1, 1, 2, 5, 10, 0, 0, 0, 0, 1500, 0, 21, 0, 800, 1, 5200, 4, 0, 0, 2, -4, -4, -1, 6, 0);
INSERT INTO "items" VALUES (603, 'Full-Helm (W)', 1, 1, 2, 10, 20, 0, 0, 0, 0, 2000, 0, 21, 1, 1500, 1, 8500, 1, 0, 0, 2, -7, -7, -1, 6, 0);
INSERT INTO "items" VALUES (610, 'Xelima''s Blade', 1, 9, 24, 2, 11, 2, 2, 12, 2, 8000, 1, 1, 13, 31000, 0, 12000, 11, 11, 0, 0, 60, 0, 8, 1, 0);
INSERT INTO "items" VALUES (611, 'Xelima''s Axe', 1, 9, 24, 3, 8, 1, 3, 9, 1, 5000, 1, 1, 12, 31000, 0, 12000, 26, 13, 0, 0, 60, 0, 10, 1, 0);
INSERT INTO "items" VALUES (612, 'Xelima''s Rapier', 1, 8, 24, 2, 6, 1, 2, 6, 1, 5000, 1, 1, 14, 25000, 0, 1100, 7, 3, 0, 0, 60, 0, 9, 1, 0);
INSERT INTO "items" VALUES (613, 'Sword of Medusa', 1, 9, 24, 2, 11, 2, 2, 12, 2, 7000, 3, 1, 16, 31000, 0, 10000, 11, 10, 0, 0, 60, 0, 8, 1, 0);
INSERT INTO "items" VALUES (614, 'Sword of Ice Elemental', 1, 9, 24, 2, 11, 2, 2, 12, 2, 7000, 2, 1, 15, 31000, 0, 10000, 11, 10, 0, 0, 60, 0, 8, 1, 0);
INSERT INTO "items" VALUES (615, 'Giant Sword', 1, 9, 1, 2, 11, 3, 2, 12, 3, 1500, 0, 1, 18, 8000, 1, 13000, 13, 12, 100, 0, 0, 0, 8, 1, 0);
INSERT INTO "items" VALUES (616, 'Demon Slayer', 1, 9, 1, 4, 7, 2, 4, 8, 2, 5000, 0, 1, 17, 25000, 0, 12000, 13, 12, 0, 0, 0, 0, 8, 1, 9);
INSERT INTO "items" VALUES (617, 'Composite Bow', 1, 9, 3, 1, 6, 0, 1, 6, 0, 1200, 0, 2, 2, 3000, 1, 4000, 41, 5, 70, 0, 0, 0, 6, 3, 0);
INSERT INTO "items" VALUES (618, 'Dark Elf''s Bow', 1, 9, 3, 2, 7, 0, 2, 7, 0, 2500, 0, 2, 3, 8000, 0, 8000, 40, 4, 0, 0, 0, 0, 6, 3, 0);
INSERT INTO "items" VALUES (620, 'Merien''s Shield', 1, 7, 25, 50, 1, 1, 1, 1, 1, 1600, 52, 3, 9, 45000, 0, 4000, 9, 0, 0, 0, 60, 0, 11, 5, 0);
INSERT INTO "items" VALUES (621, 'Merien''s Plate Mail (M)', 1, 2, 25, 60, 45, 0, 0, 0, 0, 5000, 50, 9, 3, 45000, 0, 10000, 4, 0, 0, 1, 60, 0, 0, 6, 5);
INSERT INTO "items" VALUES (622, 'Merien''s Plate Mail (W)', 1, 2, 25, 60, 45, 0, 0, 0, 0, 5000, 50, 13, 5, 45000, 0, 10000, 6, 0, 0, 2, 60, 0, 0, 6, 5);
INSERT INTO "items" VALUES (623, 'GM Shield', 1, 7, 25, 50, 1, 1, 1, 1, 1, 1600, 53, 3, 7, 1000, 0, 1000, 8, 0, 0, 0, 60, 0, 11, 5, 0);
INSERT INTO "items" VALUES (630, 'Ring of the Xelima', 1, 10, 14, 3, 7, 0, 0, 0, 0, 300, 0, 16, 20, 10000, 0, 500, -1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (631, 'Ring of the Abaddon', 1, 10, 14, 3, 10, 0, 0, 0, 0, 300, 0, 16, 21, 19000, 0, 500, -1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (632, 'Ring of Ogre power', 1, 10, 14, 3, 2, 0, 0, 0, 0, 300, 0, 16, 24, 10000, 0, 500, -1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (633, 'Ring of Demon power', 1, 10, 14, 3, 4, 0, 0, 0, 0, 300, 0, 16, 23, 10000, 0, 500, -1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (634, 'Ring of Wizard', 1, 10, 14, 6, 1, 0, 0, 0, 0, 300, 0, 16, 25, 10000, 0, 500, -1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (635, 'Ring of Mage', 1, 10, 14, 6, 2, 0, 0, 0, 0, 300, 0, 16, 26, 10000, 0, 500, -1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (636, 'Ring of Grand Mage', 1, 10, 14, 6, 3, 0, 0, 0, 0, 300, 0, 16, 27, 10000, 0, 500, -1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (637, 'Necklace of Light Protection', 1, 6, 14, 7, 25, 0, 0, 0, 0, 300, 0, 16, 28, 20000, 0, 1000, -1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (638, 'Necklace of Fire Protection', 1, 6, 14, 9, 25, 0, 0, 0, 0, 300, 0, 16, 29, 20000, 0, 1000, -1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (639, 'Necklace of Poison Protection', 1, 6, 14, 11, 50, 0, 0, 0, 0, 300, 0, 16, 30, 20000, 0, 1000, -1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (640, 'Necklace of Sufferent', 1, 6, 14, 11, 110, 0, 0, 0, 0, 300, 0, 16, 31, 20000, 0, 1000, -1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (641, 'Necklace of Medusa', 1, 6, 14, 11, 500, 0, 0, 0, 0, 300, 0, 16, 31, 20000, 0, 1000, -1, 0, 0, 0, 0, 0, -1, 46, 5);
INSERT INTO "items" VALUES (642, 'Necklace of Ice Protection', 1, 6, 14, 10, 25, 0, 0, 0, 0, 300, 0, 16, 32, 20000, 0, 1000, -1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (643, 'Necklace of Ice Elemental', 1, 6, 14, 10, 50, 0, 0, 0, 0, 300, 0, 16, 33, 20000, 0, 1000, -1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (644, 'Necklace of Air Elemental', 1, 6, 14, 7, 50, 0, 0, 0, 0, 300, 0, 16, 35, 20000, 0, 1000, -1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (645, 'Necklace of Efreet', 1, 6, 14, 9, 50, 0, 0, 0, 0, 300, 0, 16, 36, 20000, 0, 1000, -1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (646, 'Necklace of Beholder', 1, 6, 14, 9, 50, 0, 0, 0, 0, 300, 0, 16, 42, 20000, 0, 1000, -1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (647, 'Necklace of Stone Golem', 1, 6, 14, 4, 25, 0, 0, 0, 0, 300, 0, 16, 43, 20000, 0, 1000, -1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (648, 'NecklaceOfLiche', 1, 6, 14, 2, 15, 0, 0, 0, 0, 300, 0, 16, 46, 10000, 0, 600, -1, 0, 0, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (650, 'Zemstone of Sacrifice', 0, 0, 26, 0, 0, 0, 0, 0, 0, 3, 0, 16, 34, 5000, 0, 5000, -1, 0, 0, 0, 0, 0, -1, 0, 0);
INSERT INTO "items" VALUES (651, 'Green Ball', 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 6, 123, 5000, 0, 4000, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (652, 'Red Ball', 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 6, 124, 5000, 0, 4000, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (653, 'Yellow Ball', 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 6, 125, 5000, 0, 4000, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (654, 'Blue Ball', 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 6, 126, 5000, 0, 4000, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (655, 'Pearl Ball', 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 6, 127, 5000, 0, 4000, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (656, 'Stone of Xelima', 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 6, 128, 10000, 0, 4000, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (657, 'Stone of Merien', 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 6, 129, 10000, 0, 4000, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (658, 'Aresden Mine Potion', 7, 0, 4, 2, 12, 10, 0, 0, 0, 1, 0, 6, 1, 10, 1, 30, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (659, 'Elvine Mine Potion', 7, 0, 4, 2, 12, 10, 0, 0, 0, 1, 0, 6, 1, 10, 1, 30, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (660, 'Unfreeze Potion', 7, 0, 28, 2, 12, 10, 0, 0, 0, 1, 0, 6, 130, 200, 0, 50, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (671, 'Knight Rapier', 1, 8, 1, 1, 7, 2, 1, 7, 2, 1100, 0, 1, 6, 3200, 1, 1200, 7, 3, 95, 0, 0, 0, 9, 1, 11);
INSERT INTO "items" VALUES (672, 'Knight Great Sword', 1, 9, 1, 2, 8, 2, 2, 10, 2, 1200, 0, 1, 10, 3500, 1, 5200, 11, 8, 85, 0, -8, 5, 8, 1, 11);
INSERT INTO "items" VALUES (673, 'Knight Flameberge', 1, 9, 1, 2, 10, 2, 2, 12, 2, 1200, 0, 1, 11, 3700, 1, 6000, 12, 10, 95, 0, -15, 10, 8, 1, 11);
INSERT INTO "items" VALUES (674, 'Knight War Axe', 1, 8, 1, 2, 6, 2, 2, 8, 2, 1200, 0, 15, 1, 3000, 1, 4400, 21, 8, 85, 0, 0, 0, 10, 1, 11);
INSERT INTO "items" VALUES (675, 'Knight Plate Mail (M)', 1, 2, 2, 40, 42, 0, 0, 0, 0, 3500, 0, 9, 3, 6000, 1, 10000, 4, 0, 85, 1, -10, -10, 0, 6, 11);
INSERT INTO "items" VALUES (676, 'Knight Plate Mail (W)', 1, 2, 2, 40, 42, 0, 0, 0, 0, 3500, 0, 13, 5, 6000, 1, 10000, 6, 0, 85, 2, -10, -10, 0, 6, 11);
INSERT INTO "items" VALUES (677, 'Knight Plate Leggings (M)', 1, 4, 2, 15, 22, 1, 1, 1, 1, 1100, 0, 8, 3, 3000, 1, 2000, 4, 0, 80, 1, -6, -6, 0, 6, 11);
INSERT INTO "items" VALUES (678, 'Knight Plate Leggings (W)', 1, 4, 2, 15, 22, 1, 1, 1, 1, 1100, 0, 12, 4, 3000, 1, 2000, 5, 0, 80, 2, -6, -6, 0, 6, 11);
INSERT INTO "items" VALUES (679, 'Knight Full-Helm (M)', 1, 1, 2, 12, 22, 0, 0, 0, 0, 2000, 0, 21, 1, 3500, 1, 8500, 1, 0, 80, 1, -7, -7, -1, 6, 11);
INSERT INTO "items" VALUES (680, 'Knight Full-Helm (W)', 1, 1, 2, 12, 22, 0, 0, 0, 0, 2000, 0, 21, 1, 3500, 1, 8500, 1, 0, 80, 2, -7, -7, -1, 6, 11);
INSERT INTO "items" VALUES (681, 'Wizard Hauberk (M)', 1, 3, 2, 12, 12, 0, 14, 100, 0, 1000, 0, 7, 1, 2400, 1, 1200, 2, 0, 90, 1, -3, -3, 0, 6, 11);
INSERT INTO "items" VALUES (682, 'Wizard Hauberk (W)', 1, 3, 2, 12, 12, 0, 14, 100, 0, 1000, 0, 11, 2, 2400, 1, 1200, 3, 0, 90, 2, -3, -3, 0, 6, 11);
INSERT INTO "items" VALUES (683, 'Wizard Magic Staff (MS20)', 1, 8, 13, 1, 6, 0, 20, 0, 0, 3600, 10, 17, 1, 6000, 1, 1000, 36, 3, 95, 0, 0, 0, 21, 8, 4);
INSERT INTO "items" VALUES (684, 'Wizard Magic Staff (MS10)', 1, 8, 13, 1, 6, 0, 10, 0, 0, 3600, 10, 17, 1, 5500, 1, 1000, 36, 3, 90, 0, 0, 0, 21, 8, 4);
INSERT INTO "items" VALUES (685, 'Wizard Robe (M)', 1, 2, 2, 12, 1, 0, 14, 100, 0, 3000, 0, 9, 5, 3000, 1, 1000, 6, 0, 80, 1, 0, 0, 0, 13, 11);
INSERT INTO "items" VALUES (686, 'Wizard Robe (W)', 1, 2, 2, 12, 1, 0, 14, 100, 0, 3000, 0, 13, 6, 3000, 1, 1000, 7, 0, 80, 2, 0, 0, 0, 13, 11);
INSERT INTO "items" VALUES (687, 'Knight Hauberk (M)', 1, 3, 2, 12, 12, 0, 11, 100, 0, 1000, 0, 7, 1, 2400, 1, 1200, 2, 0, 70, 1, -3, -3, 0, 6, 11);
INSERT INTO "items" VALUES (688, 'Knight Hauberk (W)', 1, 3, 2, 12, 12, 0, 11, 100, 0, 1000, 0, 11, 2, 2400, 1, 1200, 3, 0, 70, 2, -3, -3, 0, 6, 11);
INSERT INTO "items" VALUES (706, 'Dark Knight Hauberk (M)', 1, 3, 2, 18, 18, 0, 0, 0, 0, 30000, 0, 7, 1, 2400, 0, 1200, 2, 0, 0, 1, -3, -3, 0, 6, 6);
INSERT INTO "items" VALUES (707, 'Dark Knight Full Helm (M)', 1, 1, 2, 17, 27, 0, 0, 0, 0, 30000, 0, 21, 1, 3500, 0, 8500, 1, 0, 0, 1, -7, -7, -1, 6, 6);
INSERT INTO "items" VALUES (708, 'Dark Knight Leggings (M)', 1, 4, 2, 21, 28, 1, 1, 1, 1, 30000, 0, 8, 3, 3000, 0, 2000, 4, 0, 0, 1, -6, -6, 0, 6, 6);
INSERT INTO "items" VALUES (709, 'Dark Knight Flameberge', 1, 9, 1, 4, 7, 0, 4, 7, 0, 30000, 0, 1, 11, 3700, 0, 6000, 12, 10, 0, 0, -15, 10, 8, 1, 6);
INSERT INTO "items" VALUES (710, 'Dark Knight Plate Mail (M)', 1, 2, 2, 53, 55, 0, 0, 0, 0, 30000, 0, 9, 3, 6000, 0, 10000, 4, 0, 0, 1, -10, -10, 0, 6, 6);
INSERT INTO "items" VALUES (711, 'Dark Mage Hauberk (M)', 1, 3, 2, 18, 18, 0, 0, 0, 0, 30000, 0, 7, 1, 2400, 0, 1200, 2, 0, 0, 1, -3, -3, 0, 6, 6);
INSERT INTO "items" VALUES (712, 'Dark Mage Chain Mail (M)', 1, 2, 2, 36, 38, 0, 0, 0, 0, 30000, 0, 9, 1, 1200, 0, 3000, 2, 0, 0, 1, -12, -12, 0, 6, 6);
INSERT INTO "items" VALUES (713, 'Dark Mage Leggings (M)', 1, 4, 2, 21, 28, 1, 1, 1, 1, 30000, 0, 8, 3, 3000, 0, 2000, 4, 0, 0, 1, -6, -6, 0, 6, 6);
INSERT INTO "items" VALUES (714, 'Dark Mage Magic Staff', 1, 8, 13, 1, 6, 0, 25, 0, 0, 30000, 10, 17, 1, 6000, 0, 1000, 36, 3, 0, 0, 0, 0, 21, 8, 6);
INSERT INTO "items" VALUES (715, 'Dark Mage Robe (M)', 1, 2, 2, 22, 20, 0, 14, 100, 0, 30000, 0, 9, 5, 2000, 0, 1000, 6, 0, 0, 1, 0, 0, 0, 11, 6);
INSERT INTO "items" VALUES (716, 'Dark Mage Leather Armor (M)', 1, 2, 2, 24, 22, 0, 0, 0, 0, 30000, 0, 9, 0, 2000, 0, 1500, 1, 0, 0, 1, -5, -5, 0, 6, 6);
INSERT INTO "items" VALUES (717, 'Dark Knight Rapier', 1, 8, 1, 1, 7, 0, 1, 7, 0, 30000, 0, 1, 6, 3000, 0, 1100, 7, 3, 0, 0, 0, 0, 9, 1, 6);
INSERT INTO "items" VALUES (718, 'Dark Knight Great Sword', 1, 9, 1, 2, 8, 0, 2, 10, 0, 30000, 0, 1, 10, 3000, 0, 5200, 11, 8, 0, 0, -8, 5, 8, 1, 6);
INSERT INTO "items" VALUES (719, 'Dark Mage Scale Mail (M)', 1, 2, 2, 32, 30, 0, 0, 0, 0, 30000, 0, 9, 2, 1000, 0, 2000, 3, 0, 0, 1, -8, -8, 0, 6, 6);
INSERT INTO "items" VALUES (720, 'Pine Cake', 7, 0, 22, 900, 0, 0, 0, 0, 0, 1, 0, 6, 115, 2400, 0, 20, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (721, 'Ginseng', 7, 0, 28, 2, 12, 10, 0, 0, 0, 1, 0, 6, 114, 600, 0, 50, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (722, 'Beef Ribs', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 116, 50000, 0, 5000, -1, 0, 0, 0, 0, 0, -1, 0, 0);
INSERT INTO "items" VALUES (723, 'Wine', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 117, 25000, 0, 250, -1, 0, 0, 0, 0, 0, -1, 0, 0);
INSERT INTO "items" VALUES (724, 'Dark Knight Hauberk (W)', 1, 3, 2, 18, 18, 0, 0, 0, 0, 30000, 0, 11, 2, 2400, 0, 1200, 3, 0, 0, 2, -3, -3, 0, 6, 6);
INSERT INTO "items" VALUES (725, 'Dark Knight Full-Helm (W)', 1, 1, 2, 17, 27, 0, 0, 0, 0, 30000, 0, 21, 1, 3500, 0, 8500, 1, 0, 0, 2, -7, -7, -1, 6, 6);
INSERT INTO "items" VALUES (726, 'Dark Knight Leggings (W)', 1, 4, 2, 21, 28, 1, 1, 1, 1, 30000, 0, 12, 4, 3000, 0, 2000, 5, 0, 0, 2, -6, -6, 0, 6, 6);
INSERT INTO "items" VALUES (727, 'Dark Knight Flameberge', 1, 9, 1, 4, 7, 0, 4, 7, 0, 30000, 0, 1, 11, 3700, 0, 6000, 12, 10, 0, 0, -15, 10, 8, 1, 6);
INSERT INTO "items" VALUES (728, 'Dark Knight Plate Mail (W)', 1, 2, 2, 53, 55, 0, 0, 0, 0, 30000, 0, 13, 5, 6000, 0, 10000, 6, 0, 0, 2, -10, -10, 0, 6, 6);
INSERT INTO "items" VALUES (729, 'Dark Mage Hauberk (W)', 1, 3, 2, 18, 18, 0, 0, 0, 0, 30000, 0, 11, 2, 2400, 0, 1200, 3, 0, 0, 2, -3, -3, 0, 6, 6);
INSERT INTO "items" VALUES (730, 'Dark Mage Chain Mail (W)', 1, 2, 2, 36, 38, 0, 0, 0, 0, 30000, 0, 13, 3, 1200, 0, 3000, 4, 0, 0, 2, -12, -12, 0, 6, 6);
INSERT INTO "items" VALUES (731, 'Dark Mage Leggings (W)', 1, 4, 2, 21, 28, 1, 1, 1, 1, 30000, 0, 12, 4, 3000, 0, 2000, 5, 0, 0, 2, -6, -6, 0, 6, 6);
INSERT INTO "items" VALUES (732, 'Dark Mage Magic Staff (W)', 1, 8, 13, 1, 6, 0, 25, 0, 0, 30000, 10, 17, 1, 6000, 0, 1000, 36, 3, 0, 0, 0, 0, 21, 8, 6);
INSERT INTO "items" VALUES (733, 'Dark Mage Robe (W)', 1, 2, 2, 22, 20, 0, 14, 100, 0, 30000, 0, 13, 6, 2000, 0, 1000, 7, 0, 0, 2, 0, 0, 0, 11, 6);
INSERT INTO "items" VALUES (734, 'Ring of Arch Mage', 1, 10, 14, 6, 4, 0, 0, 0, 0, 300, 0, 16, 44, 10000, 0, 500, -1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (735, 'Ring of Dragon Power', 1, 10, 14, 3, 5, 0, 0, 0, 0, 300, 0, 16, 45, 10000, 0, 500, -1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (737, 'Dark Knight Giant Sword', 1, 9, 1, 2, 10, 2, 2, 12, 2, 30000, 0, 1, 18, 3700, 0, 6000, 13, 10, 120, 0, -15, 10, 8, 1, 6);
INSERT INTO "items" VALUES (738, 'Dark Mage Magic Wand', 1, 8, 13, 1, 6, 0, 28, 0, 0, 30000, 10, 17, 0, 6000, 0, 1000, 35, 3, 120, 0, 0, 0, 21, 8, 6);
INSERT INTO "items" VALUES (740, '5000 Gold Sack', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 118, 10000, 0, 500, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (741, '10000 Gold Sack', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 119, 20000, 0, 1000, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (742, '50000 Gold Sack', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 120, 100000, 0, 2000, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (743, '100000 Gold Sack', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 121, 200000, 0, 3000, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (744, '1000000 Gold Sack', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 122, 2000000, 0, 4000, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (745, 'Dark Knight Templar', 1, 9, 1, 2, 10, 2, 2, 12, 2, 30000, 0, 1, 20, 3700, 0, 6000, 14, 10, 120, 0, -15, 10, 8, 1, 6);
INSERT INTO "items" VALUES (746, 'Dark Mage Templar', 1, 8, 13, 1, 6, 0, 28, 0, 0, 30000, 10, 17, 2, 6000, 0, 1000, 37, 3, 120, 0, 0, 0, 21, 8, 6);
INSERT INTO "items" VALUES (750, 'Horned Helm (M)', 1, 1, 2, 17, 24, 0, 0, 0, 0, 3500, 0, 21, 2, 4000, 1, 16000, 5, 0, 120, 1, -7, -7, -1, 6, 0);
INSERT INTO "items" VALUES (751, 'Wings Helm (M)', 1, 1, 2, 15, 22, 0, 0, 0, 0, 2500, 0, 21, 3, 5000, 1, 13000, 6, 0, 100, 1, -7, -7, -1, 6, 0);
INSERT INTO "items" VALUES (752, 'Wizard Cap (M)', 1, 1, 2, 5, 10, 0, 14, 110, 0, 500, 0, 21, 4, 1500, 1, 1500, 7, 0, 90, 1, -7, -7, -1, 6, 0);
INSERT INTO "items" VALUES (753, 'Wizard Hat (M)', 1, 1, 2, 10, 15, 0, 14, 130, 0, 800, 0, 21, 5, 3000, 1, 1500, 8, 0, 120, 1, -7, -7, -1, 6, 0);
INSERT INTO "items" VALUES (754, 'Horned Helm (W)', 1, 1, 2, 17, 24, 0, 0, 0, 0, 3500, 0, 21, 2, 4000, 1, 16000, 5, 0, 120, 2, -7, -7, -1, 6, 0);
INSERT INTO "items" VALUES (755, 'Wings Helm (W)', 1, 1, 2, 15, 22, 0, 0, 0, 0, 2500, 0, 21, 3, 5000, 1, 13000, 6, 0, 100, 2, -7, -7, -1, 6, 0);
INSERT INTO "items" VALUES (756, 'Wizard Cap (W)', 1, 1, 2, 5, 10, 0, 14, 110, 0, 500, 0, 21, 4, 1500, 1, 1500, 7, 0, 90, 2, -7, -7, -1, 6, 0);
INSERT INTO "items" VALUES (757, 'Wizard Hat (W)', 1, 1, 2, 10, 15, 0, 14, 130, 0, 800, 0, 21, 5, 3000, 1, 1500, 8, 0, 120, 2, -7, -7, -1, 6, 0);
INSERT INTO "items" VALUES (760, 'Hammer', 1, 9, 1, 3, 7, 0, 3, 9, 0, 2000, 0, 15, 7, 6000, 1, 9000, 30, 12, 0, 0, 0, 0, 14, 1, 0);
INSERT INTO "items" VALUES (761, 'Battle Hammer', 1, 9, 1, 3, 7, 2, 3, 9, 2, 4000, 0, 15, 8, 9000, 1, 12000, 31, 14, 110, 0, 0, 0, 14, 1, 0);
INSERT INTO "items" VALUES (762, 'Giant Battle Hammer', 1, 9, 1, 3, 9, 3, 3, 10, 3, 6000, 0, 15, 8, 15000, 0, 14000, 31, 14, 110, 0, 0, 0, 14, 1, 9);
INSERT INTO "items" VALUES (765, 'Third Memorial Ring', 1, 10, 4, 0, 0, 0, 10, 0, 0, 300, 0, 16, 11, 1000, 0, 200, -1, 0, 0, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (770, 'Santa Costume (M)', 1, 13, 14, 10, 25, 0, 0, 0, 0, 1500, 0, 9, 6, 18000, 0, 1000, 7, 0, 0, 1, 0, 0, 0, 15, 0);
INSERT INTO "items" VALUES (771, 'Santa Costume (W)', 1, 13, 14, 10, 25, 0, 0, 0, 0, 1500, 0, 13, 7, 18000, 0, 1000, 8, 0, 0, 2, 0, 0, 0, 15, 0);
INSERT INTO "items" VALUES (780, 'Red Candy', 7, 0, 4, 3, 8, 200, 0, 0, 0, 1, 0, 6, 131, 2000, 0, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (781, 'BlueCandy', 7, 0, 5, 4, 8, 200, 0, 0, 0, 1, 0, 6, 132, 2000, 0, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (782, 'Green Candy', 7, 0, 6, 4, 8, 200, 0, 0, 0, 1, 0, 6, 133, 2000, 0, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (800, 'Farming Manual', 3, 0, 9, 2, 20, 0, 0, 0, 0, 0, 0, 6, 92, 100, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (801, 'Seed Bag (Watermelon)', 11, 0, 30, 1, 20, 0, 0, 0, 0, 1, 0, 6, 137, 100, 1, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (802, 'Seed Bag (Pumpkin)', 11, 0, 30, 2, 20, 0, 0, 0, 0, 1, 0, 6, 137, 100, 1, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (803, 'Seed Bag (Garlic)', 11, 0, 30, 3, 30, 0, 0, 0, 0, 1, 0, 6, 137, 150, 1, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (804, 'Seed Bag (Barley)', 11, 0, 30, 4, 30, 0, 0, 0, 0, 1, 0, 6, 137, 150, 1, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (805, 'Seed Bag (Carrot)', 11, 0, 30, 5, 40, 0, 0, 0, 0, 1, 0, 6, 137, 200, 1, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (806, 'Seed Bag (Radish)', 11, 0, 30, 6, 40, 0, 0, 0, 0, 1, 0, 6, 137, 200, 1, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (807, 'Seed Bag (Corn)', 11, 0, 30, 7, 50, 0, 0, 0, 0, 1, 0, 6, 137, 250, 1, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (808, 'Seed Bag (Chinese Bellflower)', 11, 0, 30, 8, 50, 0, 0, 0, 0, 1, 0, 6, 137, 250, 1, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (809, 'Seed Bag (Melon)', 11, 0, 30, 9, 60, 0, 0, 0, 0, 1, 0, 6, 137, 300, 1, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (810, 'Seed Bag (Tomato)', 11, 0, 30, 10, 60, 0, 0, 0, 0, 1, 0, 6, 137, 300, 1, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (811, 'Seed Bag (Grapes)', 11, 0, 30, 11, 70, 0, 0, 0, 0, 1, 0, 6, 137, 350, 1, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (812, 'Seed Bag (BlueGrapes)', 11, 0, 30, 12, 70, 0, 0, 0, 0, 1, 0, 6, 137, 350, 1, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (813, 'Seed Bag (Mushroom)', 11, 0, 30, 13, 80, 0, 0, 0, 0, 1, 0, 6, 137, 400, 1, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (814, 'Seed Bag (Ginseng)', 11, 0, 30, 14, 90, 0, 0, 0, 0, 1, 0, 6, 137, 450, 1, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (820, 'Watermelon', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 138, 120, 0, 100, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (821, 'Pumpkin', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 139, 120, 0, 100, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (822, 'Garlic', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 140, 180, 0, 100, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (823, 'Barley', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 141, 180, 0, 100, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (824, 'Carrot', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 142, 200, 0, 100, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (825, 'Radish', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 143, 200, 0, 100, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (826, 'Corn', 7, 0, 7, 4, 8, 10, 0, 0, 0, 1, 0, 6, 144, 240, 0, 100, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (827, 'Chinese Bellflower', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 145, 240, 0, 100, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (828, 'Melon', 7, 0, 7, 5, 7, 12, 0, 0, 0, 1, 0, 6, 146, 300, 0, 100, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (829, 'Tomato', 7, 0, 7, 5, 7, 10, 0, 0, 0, 1, 0, 6, 147, 300, 0, 100, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (830, 'Grapes', 7, 0, 7, 5, 8, 10, 0, 0, 0, 1, 0, 6, 148, 360, 0, 100, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (831, 'Blue Grapes', 7, 0, 7, 5, 8, 10, 0, 0, 0, 1, 0, 6, 149, 360, 0, 100, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (832, 'Mushroom', 5, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 150, 400, 0, 100, 1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (840, 'Super Health Potion', 7, 0, 4, 3, 8, 130, 0, 0, 0, 1, 0, 6, 134, 2000, 0, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (841, 'Super Mana Potion', 7, 0, 5, 3, 8, 130, 0, 0, 0, 1, 0, 6, 135, 2000, 0, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (842, 'Super Revitalizing Potion', 7, 0, 6, 4, 8, 130, 0, 0, 0, 1, 0, 6, 136, 2000, 0, 100, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (843, 'Barbarian Hammer', 1, 9, 1, 3, 9, 2, 3, 10, 2, 4500, 0, 15, 10, 15000, 1, 20000, 32, 15, 110, 0, 0, 0, 14, 1, 0);
INSERT INTO "items" VALUES (844, 'Black Shadow Sword', 1, 9, 1, 2, 12, 3, 2, 12, 3, 2800, 0, 1, 25, 10000, 1, 15000, 33, 14, 120, 0, 0, 0, 8, 1, 0);
INSERT INTO "items" VALUES (845, 'Storm Bringer', 1, 9, 1, 3, 10, 0, 3, 11, 0, 3000, 0, 1, 26, 12000, 0, 11000, 15, 11, 0, 0, 0, 0, 8, 1, 0);
INSERT INTO "items" VALUES (846, 'The Devastator', 1, 9, 1, 4, 8, 1, 4, 9, 1, 3000, 0, 1, 21, 18000, 0, 20000, 19, 15, 0, 0, 0, 0, 8, 1, 0);
INSERT INTO "items" VALUES (847, 'Dark Executor', 1, 9, 1, 2, 12, 3, 2, 13, 3, 4000, 0, 1, 24, 14000, 0, 14000, 16, 13, 0, 0, 0, 0, 8, 1, 0);
INSERT INTO "items" VALUES (848, 'Lighting Blade', 1, 9, 1, 2, 12, 3, 2, 13, 3, 4000, 0, 1, 22, 14000, 0, 14000, 29, 13, 0, 0, 0, 0, 8, 1, 0);
INSERT INTO "items" VALUES (849, 'Kloness Blade', 1, 9, 1, 2, 12, 2, 2, 13, 2, 8000, 0, 1, 23, 12000, 0, 12000, 17, 12, 0, 0, 0, 0, 8, 1, 0);
INSERT INTO "items" VALUES (850, 'Kloness Axe', 1, 9, 1, 3, 9, 1, 3, 10, 1, 5000, 0, 15, 11, 8000, 0, 15000, 28, 14, 0, 0, 0, 0, 8, 1, 0);
INSERT INTO "items" VALUES (851, 'Kloness Esterk', 1, 8, 1, 2, 6, 2, 2, 7, 2, 5000, 0, 1, 27, 6000, 0, 5000, 18, 5, 0, 0, 0, 0, 8, 1, 0);
INSERT INTO "items" VALUES (852, 'Cancellation Manual', 3, 0, 18, 76, 0, 0, 0, 0, 0, 0, 0, 6, 91, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 1);
INSERT INTO "items" VALUES (853, 'Earth shock Wave Manual', 3, 0, 18, 96, 0, 0, 0, 0, 0, 0, 0, 6, 91, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 1);
INSERT INTO "items" VALUES (857, 'Inhibition Magic Casting Manual', 3, 0, 18, 83, 0, 0, 0, 0, 0, 0, 0, 6, 91, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 1);
INSERT INTO "items" VALUES (858, 'Necklace of Merien', 1, 6, 14, 4, 50, 0, 0, 0, 0, 300, 0, 16, 49, 20000, 0, 1000, -1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (859, 'Necklace of Kloness', 1, 6, 14, 4, 0, 0, 0, 0, 0, 300, 0, 16, 47, 20000, 0, 1000, -1, 0, 0, 0, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (860, 'Necklace of Xelima', 1, 6, 14, 12, 100, 0, 0, 0, 0, 300, 0, 16, 48, 20000, 0, 1000, -1, 0, 0, 0, 0, 0, -1, 46, 5);
INSERT INTO "items" VALUES (861, 'Berserk Magic Wand (MS.20)', 1, 8, 13, 1, 6, 0, 20, 0, 0, 5000, 0, 17, 4, 12000, 0, 1000, 34, 3, 100, 0, 0, 0, 21, 8, 0);
INSERT INTO "items" VALUES (862, 'Berserk Magic Wand (MS.10)', 1, 8, 13, 1, 6, 0, 10, 0, 0, 5000, 0, 17, 4, 12000, 0, 1000, 34, 3, 100, 0, 0, 0, 21, 8, 0);
INSERT INTO "items" VALUES (863, 'Kloness Magic Wand (MS.20)', 1, 8, 13, 1, 6, 0, 20, 0, 0, 5000, 0, 17, 5, 14000, 0, 1000, 39, 3, 120, 0, 0, 0, 21, 8, 0);
INSERT INTO "items" VALUES (864, 'Kloness Magic Wand (MS.10)', 1, 8, 13, 1, 6, 0, 10, 0, 0, 5000, 0, 17, 5, 14000, 0, 1000, 39, 3, 120, 0, 0, 0, 21, 8, 0);
INSERT INTO "items" VALUES (865, 'Resurrection Magic Wand (MS.20)', 1, 8, 13, 1, 6, 0, 20, 0, 0, 5000, 0, 17, 3, 10000, 0, 1000, 38, 3, 100, 0, 0, 0, 21, 8, 0);
INSERT INTO "items" VALUES (866, 'Resurrection Magic Wand (MS.10)', 1, 8, 13, 1, 6, 0, 10, 0, 0, 5000, 0, 17, 3, 10000, 0, 1000, 38, 3, 100, 0, 0, 0, 21, 8, 0);
INSERT INTO "items" VALUES (867, 'Ancient Tablet', 3, 0, 31, 7, 0, 0, 0, 0, 0, 1, 0, 6, 155, 1, 0, 50, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (868, 'An Ancient Piece of Stone (UL)', 10, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 151, 1, 0, 50, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (869, 'An Ancient Piece of Stone (LL)', 10, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 152, 1, 0, 50, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (870, 'An Ancient Piece of Stone (UR)', 10, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 153, 1, 0, 50, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (871, 'An Ancient Piece of Stone (LR)', 10, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 154, 1, 0, 50, -1, 0, 0, 0, 0, 0, -1, 31, 0);
INSERT INTO "items" VALUES (873, 'Fire-Bow', 1, 9, 3, 2, 5, 0, 2, 5, 0, 1000, 0, 2, 5, 6000, 1, 7000, 43, 6, 70, 0, 0, 0, 6, 3, 0);
INSERT INTO "items" VALUES (874, 'Direction-Bow', 1, 9, 3, 2, 6, 0, 2, 6, 0, 1200, 0, 2, 4, 7000, 1, 10000, 42, 8, 70, 0, 0, 0, 6, 3, 0);
INSERT INTO "items" VALUES (875, 'Sorceress Summon Scroll', 3, 0, 11, 5, 9, 0, 0, 0, 0, 1, 9, 6, 9, 300, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (876, 'A.T. Knight Summon Scroll', 3, 0, 11, 5, 10, 0, 0, 0, 0, 1, 9, 6, 9, 300, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (877, 'Elf Master Summon Scroll', 3, 0, 11, 5, 11, 0, 0, 0, 0, 1, 9, 6, 9, 300, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (878, 'D.S. Knight Summon Scroll', 3, 0, 11, 5, 12, 0, 0, 0, 0, 1, 9, 6, 9, 300, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (879, 'Heavy Battle Summon Scroll', 3, 0, 11, 5, 13, 0, 0, 0, 0, 1, 9, 6, 9, 300, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (880, 'Barbarian Summon Scroll', 3, 0, 11, 5, 14, 0, 0, 0, 0, 1, 9, 6, 9, 300, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (881, 'Armor Dye (Indigo)', 11, 0, 32, 1, 0, 0, 0, 0, 0, 1, 0, 6, 58, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (882, 'Armor Dye (Crimson Red)', 11, 0, 32, 4, 0, 0, 0, 0, 0, 1, 0, 6, 61, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (883, 'Armor Dye (Gold)', 11, 0, 32, 3, 0, 0, 0, 0, 0, 1, 0, 6, 60, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (884, 'Armor Dye (Aqua)', 11, 0, 32, 7, 0, 0, 0, 0, 0, 1, 0, 6, 58, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (885, 'Armor Dye (Pink)', 11, 0, 32, 8, 0, 0, 0, 0, 0, 1, 0, 6, 65, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (886, 'Armor Dye (Violet)', 11, 0, 32, 9, 0, 0, 0, 0, 0, 1, 0, 6, 66, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (887, 'Armor Dye (Blue)', 11, 0, 32, 10, 0, 0, 0, 0, 0, 1, 0, 6, 67, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (888, 'Armor Dye (Khaki)', 11, 0, 32, 12, 0, 0, 0, 0, 0, 1, 0, 6, 69, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (889, 'Armor Dye (Yellow)', 11, 0, 32, 13, 0, 0, 0, 0, 0, 1, 0, 6, 70, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (890, 'Armor Dye (Red)', 11, 0, 32, 14, 0, 0, 0, 0, 0, 1, 0, 6, 71, 100, 0, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (1081, 'Magic Diamond', 1, 11, 14, 14, 0, 0, 0, 0, 0, 300, 0, 22, 6, 8000, 0, 200, -1, 0, 0, 0, 0, -10, -1, 46, 9);
INSERT INTO "items" VALUES (1082, 'Magic Ruby', 1, 11, 14, 13, 0, 0, 0, 0, 0, 300, 0, 22, 8, 5000, 0, 200, -1, 0, 0, 0, 0, -10, -1, 46, 9);
INSERT INTO "items" VALUES (1083, 'Magic Emerald', 1, 11, 14, 15, 0, 0, 0, 0, 0, 300, 0, 22, 7, 5000, 0, 200, -1, 0, 0, 0, 0, -10, -1, 46, 9);
INSERT INTO "items" VALUES (1084, 'Magic Sapphire', 1, 11, 14, 30, 0, 0, 0, 0, 0, 300, 0, 22, 9, 5000, 0, 200, -1, 0, 0, 0, 0, -10, -1, 46, 9);
INSERT INTO "items" VALUES (1085, 'Lucky Prize Ticket', 3, 0, 23, 1, 0, 0, 0, 0, 0, 1, 9, 6, 9, 50000, 1, 1, -1, 0, 0, 0, 0, 0, -1, 42, 0);
INSERT INTO "items" VALUES (1086, 'Magic Necklace (DF+15)', 1, 6, 14, 4, 15, 0, 0, 0, 0, 300, 0, 16, 6, 19000, 0, 400, -1, 0, 50, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (1087, 'Magic Necklace (DF+20)', 1, 6, 14, 4, 20, 0, 0, 0, 0, 300, 0, 16, 6, 19000, 0, 400, -1, 0, 50, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (1088, 'Magic Necklace (DF+25)', 1, 6, 14, 4, 25, 0, 0, 0, 0, 300, 0, 16, 6, 19000, 0, 400, -1, 0, 50, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (1089, 'Magic Necklace (DF+30)', 1, 6, 14, 4, 30, 0, 0, 0, 0, 300, 0, 16, 6, 19000, 0, 400, -1, 0, 50, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (1090, 'Magic Necklace (DM+2)', 1, 6, 14, 3, 2, 0, 0, 0, 0, 300, 0, 16, 5, 39800, 0, 400, -1, 0, 65, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (1091, 'Magic Necklace (DM+3)', 1, 6, 14, 3, 3, 0, 0, 0, 0, 300, 0, 16, 5, 39800, 0, 400, -1, 0, 65, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (1092, 'Magic Necklace (DM+4)', 1, 6, 14, 3, 4, 0, 0, 0, 0, 300, 0, 16, 5, 39800, 0, 400, -1, 0, 65, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (1093, 'Magic Necklace (DM+5)', 1, 6, 14, 3, 5, 0, 0, 0, 0, 300, 0, 16, 5, 39800, 0, 400, -1, 0, 65, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (1094, 'Magic Necklace (MS12)', 1, 6, 14, 2, 12, 0, 0, 0, 0, 300, 0, 16, 7, 7000, 0, 400, -1, 0, 50, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (1095, 'Magic Necklace (MS14)', 1, 6, 14, 2, 14, 0, 0, 0, 0, 300, 0, 16, 7, 7000, 0, 400, -1, 0, 50, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (1096, 'Magic Necklace (MS16)', 1, 6, 14, 2, 16, 0, 0, 0, 0, 300, 0, 16, 7, 7000, 0, 400, -1, 0, 50, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (1097, 'Magic Necklace (MS18)', 1, 6, 14, 2, 18, 0, 0, 0, 0, 300, 0, 16, 7, 7000, 0, 400, -1, 0, 50, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (1098, 'Magic Necklace (RM15)', 1, 6, 14, 1, 15, 0, 0, 0, 0, 300, 0, 16, 4, 2250, 0, 400, -1, 0, 30, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (1099, 'Magic Necklace (RM20)', 1, 6, 14, 1, 20, 0, 0, 0, 0, 300, 0, 16, 4, 2250, 0, 400, -1, 0, 30, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (1100, 'Magic Necklace (RM25)', 1, 6, 14, 1, 25, 0, 0, 0, 0, 300, 0, 16, 4, 2250, 0, 400, -1, 0, 30, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (1101, 'Magic Necklace (RM30)', 1, 6, 14, 1, 30, 0, 0, 0, 0, 300, 0, 16, 4, 2250, 0, 400, -1, 0, 30, 0, 0, -10, -1, 46, 0);
INSERT INTO "items" VALUES (1102, 'Diamond Ware', 12, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 22, 1, 5000, 0, 200, 1, 0, 0, 30, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (1103, 'Ruby Ware', 12, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 22, 3, 3000, 0, 200, 1, 0, 0, 30, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (1104, 'Sapphire Ware', 12, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 22, 4, 3000, 0, 200, 1, 0, 0, 30, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (1105, 'Emerald Ware', 12, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 22, 2, 3000, 0, 200, 1, 0, 0, 30, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (1106, 'Crystal Ware', 12, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 22, 5, 500, 0, 200, 1, 0, 0, 30, 0, 0, -1, 46, 0);
INSERT INTO "items" VALUES (1107, 'Crafting Vessel', 10, 0, 0, 1, 0, 0, 0, 0, 0, 300, 0, 22, 0, 1000, 0, 700, -1, 0, 0, 0, 0, 0, -1, 21, 0);
INSERT INTO "items" VALUES (1108, 'Angelic Pendent (STR)', 1, 11, 14, 16, 0, 0, 0, 0, 0, 300, 0, 22, 11, 3000, 0, 200, -1, 0, 0, 0, 60, -10, -1, 46, 6);
INSERT INTO "items" VALUES (1109, 'Angelic Pendent (DEX)', 1, 11, 14, 17, 0, 0, 0, 0, 0, 300, 0, 22, 10, 3000, 0, 200, -1, 0, 0, 0, 60, -10, -1, 46, 6);
INSERT INTO "items" VALUES (1110, 'Angelic Pendent (INT)', 1, 11, 14, 18, 0, 0, 0, 0, 0, 300, 0, 22, 12, 3000, 0, 200, -1, 0, 0, 0, 60, -10, -1, 46, 6);
INSERT INTO "items" VALUES (1111, 'Angelic Pendent (MAG)', 1, 11, 14, 19, 0, 0, 0, 0, 0, 300, 0, 22, 13, 3000, 0, 200, -1, 0, 0, 0, 60, -10, -1, 46, 6);

-- ----------------------------
-- Table structure for magic_configs
-- ----------------------------
DROP TABLE IF EXISTS "magic_configs";
CREATE TABLE "magic_configs" (
  "magic_id" INTEGER,
  "name" TEXT NOT NULL,
  "magic_type" INTEGER NOT NULL,
  "delay_time" INTEGER NOT NULL,
  "last_time" INTEGER NOT NULL,
  "value1" INTEGER NOT NULL,
  "value2" INTEGER NOT NULL,
  "value3" INTEGER NOT NULL,
  "value4" INTEGER NOT NULL,
  "value5" INTEGER NOT NULL,
  "value6" INTEGER NOT NULL,
  "value7" INTEGER NOT NULL,
  "value8" INTEGER NOT NULL,
  "value9" INTEGER NOT NULL,
  "value10" INTEGER NOT NULL,
  "value11" INTEGER NOT NULL,
  "value12" INTEGER NOT NULL,
  "int_limit" INTEGER NOT NULL,
  "gold_cost" INTEGER NOT NULL,
  "category" INTEGER NOT NULL,
  "attribute" INTEGER NOT NULL,
  PRIMARY KEY ("magic_id")
);

-- ----------------------------
-- Records of magic_configs
-- ----------------------------
INSERT INTO "magic_configs" VALUES (0, 'Magic-Missile', 1, 0, 0, 8, 1, 1, 1, 8, 0, 0, 0, 0, 0, 0, 0, 18, 100, 1, 2);
INSERT INTO "magic_configs" VALUES (1, 'Heal', 2, 0, 0, 15, 1, 1, 2, 6, 10, 0, 0, 0, 0, 0, 0, 20, 100, 0, 4);
INSERT INTO "magic_configs" VALUES (2, 'Create-Food', 10, 0, 0, 18, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 18, 100, 0, 0);
INSERT INTO "magic_configs" VALUES (10, 'Energy-Bolt', 3, 0, 0, 15, 2, 2, 2, 4, 1, 2, 4, 1, 0, 0, 0, 24, 200, 1, 2);
INSERT INTO "magic_configs" VALUES (11, 'Staminar-Drain', 5, 0, 0, 14, 3, 3, 4, 6, 10, 2, 6, 5, 0, 0, 0, 22, 200, 1, 0);
INSERT INTO "magic_configs" VALUES (12, 'Recall', 8, 0, 0, 15, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 20, 120, 0, 0);
INSERT INTO "magic_configs" VALUES (13, 'Defense-Shield', 11, 0, 60, 19, 1, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 26, 200, 2, 0);
INSERT INTO "magic_configs" VALUES (14, 'Celebrating-Light', 5, 0, 0, 20, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 25, 400, 0, 0);
INSERT INTO "magic_configs" VALUES (20, 'Fire-Ball', 3, 0, 0, 27, 2, 2, 2, 6, 2, 2, 6, 2, 0, 0, 0, 26, 500, 1, 3);
INSERT INTO "magic_configs" VALUES (21, 'Great-Heal', 2, 0, 0, 28, 1, 1, 4, 10, 20, 0, 0, 0, 0, 0, 0, 28, 500, 0, 0);
INSERT INTO "magic_configs" VALUES (23, 'Staminar-Recovery', 7, 0, 0, 20, 3, 3, 4, 8, 8, 2, 6, 5, 0, 0, 0, 20, 300, 0, 0);
INSERT INTO "magic_configs" VALUES (24, 'Protection-From-Arrow', 11, 0, 60, 22, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 20, 300, 2, 0);
INSERT INTO "magic_configs" VALUES (25, 'Hold-Person', 12, 0, 30, 24, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 26, 500, 1, 0);
INSERT INTO "magic_configs" VALUES (26, 'Possession', 15, 0, 0, 25, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 26, 500, 1, 0);
INSERT INTO "magic_configs" VALUES (27, 'Poison', 17, 0, 0, 28, 1, 1, 1, 15, 0, 0, 0, 0, 0, 0, 0, 29, 700, 1, 1);
INSERT INTO "magic_configs" VALUES (28, 'Great-Staminar-Recov.', 7, 0, 0, 45, 3, 3, 4, 16, 16, 2, 12, 10, 0, 0, 0, 30, 800, 0, 0);
INSERT INTO "magic_configs" VALUES (30, 'Fire-Strike', 3, 0, 0, 36, 2, 2, 2, 8, 3, 2, 8, 3, 0, 0, 0, 34, 1000, 1, 3);
INSERT INTO "magic_configs" VALUES (31, 'Summon-Creature', 9, 0, 0, 35, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 38, 1000, 1, 1);
INSERT INTO "magic_configs" VALUES (32, 'Invisibility', 13, 0, 60, 31, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 30, 800, 1, 0);
INSERT INTO "magic_configs" VALUES (33, 'Protection-From-Magic', 11, 0, 60, 35, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 32, 850, 2, 0);
INSERT INTO "magic_configs" VALUES (34, 'Detect-Invisibility', 13, 0, 0, 33, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 30, 700, 0, 0);
INSERT INTO "magic_configs" VALUES (35, 'Paralyze', 12, 0, 50, 35, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 36, 1000, 1, 0);
INSERT INTO "magic_configs" VALUES (36, 'Cure', 17, 0, 0, 32, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 35, 700, 0, 1);
INSERT INTO "magic_configs" VALUES (37, 'Lightning-Arrow', 1, 0, 0, 32, 1, 1, 4, 5, 10, 0, 0, 0, 0, 0, 0, 38, 1100, 1, 2);
INSERT INTO "magic_configs" VALUES (38, 'Tremor', 22, 0, 0, 34, 2, 2, 3, 4, 3, 3, 4, 3, 0, 0, 0, 33, 1000, 1, 1);
INSERT INTO "magic_configs" VALUES (40, 'Fire-Wall', 14, 0, 30, 42, 1, 1, 2, 8, 0, 2, 8, 0, 1, 1, 2, 45, 1200, 1, 3);
INSERT INTO "magic_configs" VALUES (41, 'Fire-Field', 14, 0, 30, 48, 1, 1, 2, 8, 0, 2, 8, 0, 1, 2, 1, 48, 1400, 1, 3);
INSERT INTO "magic_configs" VALUES (42, 'Confuse-Language', 16, 0, 120, 40, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 42, 1300, 1, 0);
INSERT INTO "magic_configs" VALUES (43, 'Lightning', 1, 0, 0, 44, 1, 1, 4, 7, 12, 0, 0, 0, 0, 0, 0, 47, 1700, 1, 2);
INSERT INTO "magic_configs" VALUES (44, 'Great-Defense-Shield', 11, 0, 40, 45, 1, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 46, 1500, 2, 0);
INSERT INTO "magic_configs" VALUES (45, 'Chill-Wind', 23, 0, 0, 48, 2, 2, 2, 8, 3, 2, 8, 3, 5, 0, 0, 50, 2000, 1, 4);
INSERT INTO "magic_configs" VALUES (46, 'Poison-Cloud', 14, 0, 30, 48, 1, 1, 0, 15, 0, 0, 0, 0, 10, 2, 1, 49, 1800, 1, 1);
INSERT INTO "magic_configs" VALUES (47, 'Triple-Energy-Bolt', 3, 0, 0, 40, 2, 2, 2, 4, 2, 3, 8, 3, 0, 0, 0, 45, 1700, 1, 2);
INSERT INTO "magic_configs" VALUES (50, 'Berserk', 18, 0, 40, 57, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 59, 2200, 1, 0);
INSERT INTO "magic_configs" VALUES (51, 'Lightning-Bolt', 19, 0, 0, 58, 2, 2, 4, 5, 18, 4, 2, 6, 0, 0, 0, 58, 2500, 1, 2);
INSERT INTO "magic_configs" VALUES (53, 'Mass-Poison', 17, 0, 0, 54, 1, 1, 1, 40, 0, 0, 0, 0, 0, 0, 0, 52, 2100, 1, 1);
INSERT INTO "magic_configs" VALUES (54, 'Spike-Field', 14, 0, 30, 56, 1, 1, 2, 8, 0, 2, 8, 0, 9, 2, 2, 56, 2300, 1, 1);
INSERT INTO "magic_configs" VALUES (55, 'Ice-Storm', 14, 0, 30, 58, 1, 1, 4, 0, 0, 0, 0, 0, 8, 1, 0, 59, -1, 1, 4);
INSERT INTO "magic_configs" VALUES (56, 'Mass-Lightning-Arrow', 1, 0, 0, 55, 1, 1, 5, 6, 20, 0, 0, 0, 0, 0, 0, 53, 3000, 1, 2);
INSERT INTO "magic_configs" VALUES (57, 'Ice-Strike', 23, 0, 0, 59, 2, 2, 5, 6, 12, 0, 0, 0, 4, 0, 0, 60, 4200, 1, 4);
INSERT INTO "magic_configs" VALUES (60, 'Energy-Strike', 21, 0, 0, 65, 2, 2, 0, 0, 0, 7, 6, 17, 0, 0, 0, 67, 5000, 1, 2);
INSERT INTO "magic_configs" VALUES (61, 'Mass-Fire-Strike', 3, 0, 0, 80, 2, 3, 5, 6, 12, 7, 10, 18, 0, 0, 0, 85, -1, 1, 3);
INSERT INTO "magic_configs" VALUES (62, 'Confusion', 16, 0, 20, 78, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 75, 7500, 1, 0);
INSERT INTO "magic_configs" VALUES (63, 'Mass-Chill-Wind', 23, 0, 0, 90, 2, 2, 3, 8, 6, 3, 8, 6, 10, 0, 0, 93, 9800, 1, 4);
INSERT INTO "magic_configs" VALUES (64, 'Earthworm-Strike', 25, 0, 0, 80, 2, 2, 7, 6, 17, 7, 6, 17, 0, 0, 0, 97, 12000, 1, 1);
INSERT INTO "magic_configs" VALUES (65, 'Absolute-Magic-Protect.', 11, 0, 60, 90, 1, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 112, 13500, 2, 0);
INSERT INTO "magic_configs" VALUES (66, 'Armor-Break', 28, 0, 0, 90, 1, 1, 7, 6, 17, 7, 6, 17, 15, 0, 0, 97, 20000, 1, 1);
INSERT INTO "magic_configs" VALUES (67, 'Scan', 33, 0, 0, 50, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 70, 7500, 1, 1);
INSERT INTO "magic_configs" VALUES (70, 'Bloody-Shock-Wave', 19, 0, 0, 120, 2, 2, 5, 8, 20, 5, 4, 9, 0, 0, 0, 105, -1, 1, 2);
INSERT INTO "magic_configs" VALUES (71, 'Mass-Confusion', 16, 0, 60, 125, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 130, 15000, 1, 0);
INSERT INTO "magic_configs" VALUES (72, 'Mass-Ice-Strike', 23, 0, 0, 120, 2, 2, 7, 8, 25, 0, 0, 0, 7, 0, 0, 133, 21000, 1, 4);
INSERT INTO "magic_configs" VALUES (73, 'Cloud-Kill', 14, 0, 30, 130, 1, 1, 0, 40, 0, 0, 0, 0, 10, 2, 2, 120, 20000, 1, 1);
INSERT INTO "magic_configs" VALUES (74, 'Lightning-Strike', 21, 0, 0, 90, 2, 2, 0, 0, 0, 7, 7, 20, 0, 0, 0, 123, 35000, 1, 2);
INSERT INTO "magic_configs" VALUES (76, 'Cancellation', 29, 0, 30, 120, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 135, -200, 1, 0);
INSERT INTO "magic_configs" VALUES (77, 'Illusion-Movement', 21, 0, 0, 130, 2, 2, 0, 0, 0, 7, 7, 20, 0, 0, 0, 160, 27000, 1, 2);
INSERT INTO "magic_configs" VALUES (78, 'Haste', 45, 0, 5, 60, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 150, -1, 1, 0);
INSERT INTO "magic_configs" VALUES (80, 'Illusion', 16, 0, 20, 143, 2, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 150, 27000, 1, 0);
INSERT INTO "magic_configs" VALUES (81, 'Meteor-Strike', 21, 0, 0, 120, 2, 2, 6, 8, 12, 7, 12, 18, 0, 0, 0, 169, 40000, 1, 3);
INSERT INTO "magic_configs" VALUES (82, 'Mass-Magic-Missile', 21, 0, 0, 160, 3, 3, 0, 0, 0, 7, 7, 30, 0, 0, 0, 185, 45000, 1, 2);
INSERT INTO "magic_configs" VALUES (83, 'Inhibition-Casting', 31, 0, 0, 180, 1, 1, 8, 20, 30, 4, 10, 30, 0, 0, 0, 180, -1, 0, 0);
INSERT INTO "magic_configs" VALUES (90, 'Mass-Illusion', 16, 0, 60, 200, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 180, 35000, 1, 0);
INSERT INTO "magic_configs" VALUES (91, 'Blizzard', 26, 0, 20, 170, 3, 3, 5, 1, 20, 7, 8, 16, 2, 0, 0, 195, 43000, 1, 4);
INSERT INTO "magic_configs" VALUES (94, 'Resurrection', 32, 600, 0, 200, 2, 2, 3, 0, 0, 7, 7, 20, 0, 0, 0, 0, -1, 1, 2);
INSERT INTO "magic_configs" VALUES (95, 'Mass-Illusion-Movement', 21, 0, 0, 200, 2, 2, 0, 0, 0, 7, 7, 20, 0, 0, 0, 200, 27000, 1, 2);
INSERT INTO "magic_configs" VALUES (96, 'Earth-Shock-Wave', 30, 0, 0, 180, 2, 2, 5, 1, 20, 7, 8, 16, 0, 0, 0, 200, -1, 1, 1);

-- ----------------------------
-- Table structure for meta
-- ----------------------------
DROP TABLE IF EXISTS "meta";
CREATE TABLE "meta" (
  "key" TEXT,
  "value" TEXT NOT NULL,
  PRIMARY KEY ("key")
);

-- ----------------------------
-- Records of meta
-- ----------------------------
INSERT INTO "meta" VALUES ('schema_version', '3');

-- ----------------------------
-- Table structure for npc_configs
-- ----------------------------
DROP TABLE IF EXISTS "npc_configs";
CREATE TABLE "npc_configs" (
  "npc_id" INT,
  "name" TEXT,
  "npc_type" INT,
  "hit_dice" INT,
  "defense_ratio" INT,
  "hit_ratio" INT,
  "min_bravery" INT,
  "exp_min" INT,
  "exp_max" INT,
  "gold_min" INT,
  "gold_max" INT,
  "attack_dice_throw" INT,
  "attack_dice_range" INT,
  "npc_size" INT,
  "side" INT,
  "action_limit" INT,
  "action_time" INT,
  "resist_magic" INT,
  "magic_level" INT,
  "day_of_week_limit" INT,
  "chat_msg_presence" INT,
  "target_search_range" INT,
  "regen_time" INT,
  "attribute" INT,
  "abs_damage" INT,
  "max_mana" INT,
  "magic_hit_ratio" INT,
  "attack_range" INT,
  "drop_table_id" INT
);

-- ----------------------------
-- Records of npc_configs
-- ----------------------------
INSERT INTO "npc_configs" VALUES (0, 'Slime', 10, 2, 10, 30, 1, 120, 220, 11, 32, 1, 4, 0, 10, 0, 2100, 5, 0, 10, 0, 2, 5000, 1, 0, 0, 0, 1, 20010);
INSERT INTO "npc_configs" VALUES (1, 'Rabbit', 55, 4, 10, 35, 2, 150, 260, 12, 33, 1, 5, 0, 0, 0, 1500, 5, 0, 10, 0, 2, 5000, 1, 0, 0, 0, 1, 20071);
INSERT INTO "npc_configs" VALUES (2, 'Cat', 56, 4, 20, 45, 3, 200, 260, 12, 33, 2, 4, 0, 0, 0, 1500, 20, 0, 10, 0, 5, 5000, 1, 0, 0, 0, 1, 20072);
INSERT INTO "npc_configs" VALUES (3, 'Giant Ant', 16, 3, 15, 40, 2, 300, 350, 13, 34, 2, 3, 0, 10, 0, 1200, 10, 0, 10, 0, 2, 5000, 1, 0, 0, 0, 1, 20016);
INSERT INTO "npc_configs" VALUES (4, 'Snake', 22, 4, 30, 50, 1, 300, 400, 13, 35, 2, 4, 0, 10, 0, 1000, 20, 0, 10, 0, 3, 5000, 1, 0, 0, 0, 1, 20022);
INSERT INTO "npc_configs" VALUES (5, 'Orc', 14, 4, 35, 70, 2, 560, 700, 16, 39, 3, 3, 0, 10, 0, 1200, 25, 0, 10, 0, 5, 5000, 1, 0, 0, 0, 1, 20014);
INSERT INTO "npc_configs" VALUES (6, 'Dummy', 34, 10, 15, 300, 100, 0, 0, 0, 0, 0, 0, 1, 10, 3, 2100, 5, 0, 10, 0, 7, 3000, 2, 0, 0, 0, 1, 20073);
INSERT INTO "npc_configs" VALUES (7, 'Attack Dummy', 34, 10, 15, 300, 100, 0, 0, 0, 0, 0, 0, 1, 10, 0, 2100, 5, 0, 10, 0, 7, 3000, 2, 0, 0, 0, 1, 20074);
INSERT INTO "npc_configs" VALUES (8, 'Zombie', 18, 10, 40, 90, 100, 1200, 1500, 23, 50, 4, 4, 0, 10, 0, 1500, 30, 0, 10, 0, 6, 5000, 1, 0, 0, 0, 1, 20018);
INSERT INTO "npc_configs" VALUES (9, 'Giant Scorpion', 17, 6, 35, 80, 3, 1300, 1600, 24, 51, 5, 3, 0, 10, 0, 1200, 30, 0, 10, 0, 4, 5000, 1, 0, 0, 0, 1, 20017);
INSERT INTO "npc_configs" VALUES (10, 'Skeleton', 11, 8, 40, 100, 100, 1500, 1600, 25, 53, 5, 4, 0, 10, 0, 800, 40, 0, 10, 0, 5, 5000, 1, 0, 0, 0, 2, 20011);
INSERT INTO "npc_configs" VALUES (11, 'Orc Mage', 14, 12, 35, 100, 3, 1200, 1260, 22, 48, 4, 3, 0, 10, 0, 1500, 45, 3, 10, 0, 6, 5000, 1, 0, 150, 10, 1, 20075);
INSERT INTO "npc_configs" VALUES (12, 'Clay Golem', 23, 30, 100, 150, 100, 3900, 5000, 54, 96, 7, 4, 1, 10, 0, 1200, 50, 0, 10, 0, 5, 10000, 1, 0, 0, 0, 1, 20023);
INSERT INTO "npc_configs" VALUES (13, 'Stone Golem', 12, 25, 110, 150, 100, 2500, 5000, 47, 86, 7, 4, 1, 10, 0, 1200, 50, 0, 10, 0, 5, 10000, 1, 0, 0, 0, 1, 20012);
INSERT INTO "npc_configs" VALUES (14, 'Helhound', 27, 35, 90, 170, 4, 5000, 5500, 62, 108, 7, 5, 0, 10, 0, 900, 60, 5, 10, 0, 7, 10000, 3, 0, 250, 20, 1, 20027);
INSERT INTO "npc_configs" VALUES (15, 'Frog', 57, 35, 90, 170, 4, 4516, 6555, 65, 113, 7, 5, 0, 10, 0, 900, 60, 0, 10, 0, 7, 10000, 1, 0, 0, 20, 2, 20076);
INSERT INTO "npc_configs" VALUES (16, 'Rudolph', 61, 40, 100, 180, 10, 4526, 4912, 57, 100, 7, 5, 0, 10, 0, 1000, 60, 5, 10, 0, 5, 10000, 3, 0, 0, 0, 1, 20077);
INSERT INTO "npc_configs" VALUES (17, 'Troll', 28, 55, 85, 200, 100, 3220, 3500, 43, 80, 8, 5, 1, 10, 0, 700, 60, 0, 10, 0, 7, 10000, 1, 0, 0, 0, 1, 20028);
INSERT INTO "npc_configs" VALUES (18, 'Cyclops', 13, 60, 100, 180, 805, 6511, 8710, 86, 144, 8, 6, 1, 10, 0, 1200, 65, 5, 10, 0, 7, 10000, 3, 0, 450, 50, 3, 20013);
INSERT INTO "npc_configs" VALUES (19, 'Ice Golem', 65, 60, 100, 180, 100, 6244, 7560, 79, 133, 8, 6, 1, 10, 0, 1200, 65, 11, 10, 0, 7, 10000, 4, 30, 0, 200, 1, 20078);
INSERT INTO "npc_configs" VALUES (20, 'Beholder', 53, 100, 100, 450, 100, 9050, 9800, 104, 171, 8, 8, 1, 10, 0, 800, 70, 0, 10, 0, 7, 10000, 2, 30, 0, 200, 4, 20053);
INSERT INTO "npc_configs" VALUES (21, 'Plant', 60, 105, 80, 230, 100, 6522, 7543, 80, 135, 8, 8, 1, 10, 0, 800, 70, 5, 10, 0, 7, 10000, 1, 0, 550, 65, 2, 20079);
INSERT INTO "npc_configs" VALUES (22, 'Ogre', 29, 115, 150, 230, 100, 7212, 8312, 87, 146, 8, 7, 1, 10, 0, 700, 70, 0, 10, 0, 7, 10000, 2, 0, 0, 0, 3, 20029);
INSERT INTO "npc_configs" VALUES (23, 'Mountain Giant', 58, 100, 250, 230, 100, 19872, 19883, 208, 328, 8, 7, 1, 10, 0, 700, 75, 0, 10, 0, 7, 10000, 1, 20, 0, 0, 2, 20058);
INSERT INTO "npc_configs" VALUES (24, 'Direboar', 62, 130, 80, 230, 100, 6441, 7212, 78, 132, 8, 8, 0, 10, 0, 800, 70, 0, 10, 0, 7, 10000, 1, 0, 0, 0, 1, 20080);
INSERT INTO "npc_configs" VALUES (25, 'Tentocle', 80, 80, 150, 180, 100, 6005, 6200, 71, 121, 7, 6, 2, 10, 0, 400, 65, 6, 10, 0, 5, 10000, 1, 0, 500, 80, 3, 20081);
INSERT INTO "npc_configs" VALUES (26, 'Giant Crayfish', 74, 70, 150, 200, 100, 9655, 10651, 111, 182, 7, 7, 1, 10, 0, 400, 60, 0, 10, 0, 5, 10000, 4, 0, 0, 0, 2, 20082);
INSERT INTO "npc_configs" VALUES (27, 'Giant Tree', 76, 100, 120, 200, 100, 8321, 9321, 98, 162, 8, 6, 1, 10, 0, 700, 70, 0, 10, 0, 5, 10000, 1, 0, 0, 0, 2, 20083);
INSERT INTO "npc_configs" VALUES (28, 'Liche', 30, 130, 300, 230, 100, 8982, 9543, 102, 168, 8, 6, 1, 10, 0, 700, 75, 6, 10, 0, 7, 10000, 4, 10, 1000, 80, 5, 20030);
INSERT INTO "npc_configs" VALUES (29, 'Stalker', 48, 130, 200, 300, 100, 25321, 25902, 266, 414, 8, 9, 1, 10, 0, 600, 80, 0, 10, 0, 7, 10000, 1, 20, 0, 0, 4, 20048);
INSERT INTO "npc_configs" VALUES (30, 'Werewolf', 33, 140, 180, 300, 100, 15221, 19326, 182, 289, 8, 8, 1, 10, 0, 700, 80, 0, 10, 0, 7, 10000, 1, 20, 0, 0, 1, 20033);
INSERT INTO "npc_configs" VALUES (31, 'Darkelf', 54, 140, 200, 450, 100, 4521, 4834, 56, 100, 8, 4, 1, 10, 0, 700, 60, 0, 10, 0, 8, 10000, 1, 50, 0, 0, 8, 20054);
INSERT INTO "npc_configs" VALUES (32, 'Frost', 63, 130, 300, 230, 100, 4211, 6542, 63, 110, 8, 6, 1, 10, 0, 700, 75, 10, 10, 0, 7, 10000, 4, 10, 700, 80, 3, 20063);
INSERT INTO "npc_configs" VALUES (33, 'Claw Turtle', 72, 120, 200, 280, 100, 56212, 56899, 575, 878, 8, 9, 1, 10, 0, 500, 80, 0, 10, 0, 5, 10000, 4, 0, 0, 0, 2, 20084);
INSERT INTO "npc_configs" VALUES (34, 'Dragon', 70, 500, 400, 500, 100, 26541, 27981, 282, 438, 9, 10, 1, 10, 0, 500, 90, 7, 10, 0, 5, 10000, 3, 10, 2000, 80, 3, 20070);
INSERT INTO "npc_configs" VALUES (35, 'Ettin', 59, 180, 350, 350, 100, 35561, 39564, 385, 593, 8, 9, 1, 10, 0, 700, 90, 0, 10, 0, 7, 10000, 2, 65, 0, 0, 3, 20059);
INSERT INTO "npc_configs" VALUES (36, 'Demon', 31, 340, 450, 500, 100, 122000, 161200, 1426, 2154, 10, 10, 1, 10, 0, 700, 100, 7, 10, 0, 7, 60000, 3, 75, 2000, 250, 4, 20031);
INSERT INTO "npc_configs" VALUES (37, 'Unicorn', 32, 340, 450, 500, 100, 25611, 25666, 266, 414, 10, 10, 1, 0, 0, 700, 100, 8, 10, 0, 7, 60000, 2, 75, 2000, 250, 4, 20032);
INSERT INTO "npc_configs" VALUES (38, 'Gargoyle', 52, 400, 450, 500, 100, 162130, 166540, 1653, 2495, 13, 10, 1, 10, 0, 700, 70, 7, 10, 0, 7, 60000, 3, 80, 2000, 250, 5, 20052);
INSERT INTO "npc_configs" VALUES (39, 'Centaur', 71, 350, 450, 500, 100, 452321, 351321, 4028, 6057, 12, 11, 1, 10, 0, 500, 100, 8, 10, 0, 5, 10000, 1, 0, 2000, 80, 3, 20085);
INSERT INTO "npc_configs" VALUES (40, 'Giant Lizard', 75, 450, 400, 400, 100, 56894, 65411, 621, 947, 10, 10, 1, 10, 0, 600, 90, 8, 10, 0, 5, 10000, 4, 0, 700, 80, 2, 20086);
INSERT INTO "npc_configs" VALUES (41, 'Master Mage Orc', 77, 250, 300, 300, 100, 56431, 65412, 619, 943, 8, 7, 1, 10, 0, 500, 100, 7, 10, 0, 5, 10000, 3, 0, 1500, 80, 3, 20087);
INSERT INTO "npc_configs" VALUES (42, 'Minotaur', 78, 340, 500, 500, 100, 60212, 65421, 638, 972, 13, 11, 1, 10, 0, 500, 60, 0, 10, 0, 5, 10000, 1, 0, 0, 0, 2, 20088);
INSERT INTO "npc_configs" VALUES (43, 'Nizie', 79, 280, 300, 300, 100, 65132, 65421, 662, 1009, 10, 10, 1, 10, 0, 500, 90, 10, 10, 0, 5, 10000, 4, 0, 2000, 80, 3, 20089);
INSERT INTO "npc_configs" VALUES (44, 'Helclaw', 49, 1000, 450, 1000, 100, 190050, 198121, 1950, 2941, 15, 14, 1, 10, 0, 600, 100, 7, 10, 0, 7, 60000, 3, 50, 0, 0, 5, 20049);
INSERT INTO "npc_configs" VALUES (45, 'Tiger Worm', 50, 2000, 550, 1200, 100, 152121, 192132, 1731, 2611, 18, 17, 1, 10, 0, 600, 100, 9, 10, 0, 7, 60000, 3, 70, 16000, 250, 6, 20050);
INSERT INTO "npc_configs" VALUES (46, 'Wyvern', 66, 2500, 1000, 1300, 100, 198945, 199995, 2004, 3022, 20, 18, 1, 10, 0, 600, 100, 12, 10, 0, 8, 60000, 4, 80, 16000, 250, 7, 20090);
INSERT INTO "npc_configs" VALUES (47, 'Fire Wyvern', 73, 3000, 1000, 1300, 100, 251210, 263211, 2582, 3888, -10, 18, 1, 10, 0, 600, 100, 7, 10, 0, 8, 60000, 3, 80, 16000, 250, 7, 20091);
INSERT INTO "npc_configs" VALUES (48, 'Abaddon', 81, 15000, 1500, 1600, 100, 8494654, 9845165, 91709, 137578, 20, 20, 1, 10, 0, 500, 100, 13, 10, 0, 8, 60000, 2, 90, 20000, 300, 7, 20092);
INSERT INTO "npc_configs" VALUES (49, 'Crop', 64, 2000, 10, 10, 30, 0, 0, 0, 0, 0, 0, 0, 10, 5, 60000, 5, 0, 10, 0, 7, 3000, 2, 50, 0, 200, 1, 0);
INSERT INTO "npc_configs" VALUES (50, 'Energy Sphere', 35, 10, 10, 10, 100, 0, 0, 0, 0, 0, 0, 1, 10, 4, 60000, 5, 0, 10, 0, 7, 3000, 2, 0, 0, 0, 1, 0);
INSERT INTO "npc_configs" VALUES (51, 'Gate', 91, 2000, 800, 100, 100, 0, 0, 0, 0, 0, 0, 1, 1, 8, 1600, 100, 0, 10, 0, 1, 3000, 2, 0, 0, 0, 1, 0);
INSERT INTO "npc_configs" VALUES (52, 'Gate', 91, 2000, 800, 100, 100, 0, 0, 0, 0, 0, 0, 1, 2, 8, 1600, 100, 0, 10, 0, 1, 3000, 2, 0, 0, 0, 1, 0);
INSERT INTO "npc_configs" VALUES (53, 'Mcgaffin', 67, 3000, 1000, 2000, 5, 0, 0, 0, 0, 1, 1, 0, 0, 6, 15000, 0, 0, 10, 1, 0, 10000, 0, 0, 0, 0, 4, 0);
INSERT INTO "npc_configs" VALUES (54, 'Perry', 68, 3000, 1000, 2000, 5, 0, 0, 0, 0, 1, 1, 0, 0, 6, 5000, 0, 0, 10, 1, 0, 10000, 0, 0, 0, 0, 4, 0);
INSERT INTO "npc_configs" VALUES (55, 'Devlin', 69, 3000, 1000, 2000, 5, 0, 0, 0, 0, 1, 1, 0, 0, 6, 30000, 0, 0, 10, 1, 0, 10000, 0, 0, 0, 0, 4, 0);
INSERT INTO "npc_configs" VALUES (56, 'Shop Keeper', 15, 10, 10, 20, 5, 0, 0, 0, 0, 1, 1, 0, 0, 2, 15000, 0, 0, 10, 1, 0, 10000, 0, 0, 0, 0, 1, 0);
INSERT INTO "npc_configs" VALUES (57, 'Sorcerer', 19, 10, 10, 20, 5, 0, 0, 0, 0, 1, 1, 0, 0, 2, 5000, 0, 0, 10, 1, 0, 10000, 0, 0, 0, 0, 1, 0);
INSERT INTO "npc_configs" VALUES (58, 'Warehouse Keeper', 20, 10, 10, 20, 5, 0, 0, 0, 0, 1, 1, 0, 0, 2, 30000, 0, 0, 10, 1, 0, 10000, 0, 0, 0, 0, 1, 0);
INSERT INTO "npc_configs" VALUES (59, 'Guard', 21, 115, 150, 230, 3, 0, 0, 0, 0, 3, 8, 0, 1, 0, 1000, 100, -10, 10, 1, 8, 10000, 2, 0, 1000, 130, 5, 0);
INSERT INTO "npc_configs" VALUES (60, 'Guard', 21, 115, 150, 230, 3, 0, 0, 0, 0, 3, 8, 0, 2, 0, 1000, 100, -10, 10, 1, 8, 10000, 2, 0, 1000, 130, 5, 0);
INSERT INTO "npc_configs" VALUES (61, 'Guard', 21, 115, 150, 230, 3, 0, 0, 0, 0, 3, 8, 0, 0, 0, 1000, 100, -10, 10, 1, 8, 10000, 2, 0, 1000, 130, 5, 0);
INSERT INTO "npc_configs" VALUES (62, 'BlackSmith Keeper', 24, 10, 10, 20, 1, 0, 0, 0, 0, 1, 1, 0, 0, 2, 5000, 0, 0, 10, 1, 0, 10000, 0, 0, 0, 0, 1, 0);
INSERT INTO "npc_configs" VALUES (63, 'Cityhall Officer', 25, 10, 10, 20, 1, 0, 0, 0, 0, 1, 1, 0, 0, 2, 5000, 0, 0, 10, 1, 0, 10000, 0, 0, 0, 0, 1, 0);
INSERT INTO "npc_configs" VALUES (64, 'Guildhall Officer', 26, 10, 10, 20, 1, 0, 0, 0, 0, 1, 1, 0, 0, 2, 5000, 0, 0, 10, 1, 0, 10000, 0, 0, 0, 0, 1, 0);
INSERT INTO "npc_configs" VALUES (65, 'Gail', 90, 10, 10, 20, 1, 0, 0, 0, 0, 1, 1, 0, 0, 2, 5000, 0, 0, 10, 1, 0, 10000, 0, 0, 0, 0, 1, 0);
INSERT INTO "npc_configs" VALUES (66, 'Arrow Guard Tower', 36, 500, 50, 500, 80, 2222, 2223, 500, 1500, 5, 8, 1, 1, 5, 600, 30, 0, 10, 0, 8, 3000, 2, 0, 0, 0, 8, 20093);
INSERT INTO "npc_configs" VALUES (67, 'Arrow Guard Tower', 36, 500, 50, 500, 80, 2222, 2223, 500, 1500, 5, 8, 1, 2, 5, 600, 30, 0, 10, 0, 8, 3000, 2, 0, 0, 0, 8, 20094);
INSERT INTO "npc_configs" VALUES (68, 'Cannon Guard Tower', 37, 500, 50, 500, 90, 2222, 2223, 750, 1250, 10, 14, 1, 1, 5, 1800, 30, 0, 10, 0, 8, 3000, 2, 0, 0, 0, 8, 20095);
INSERT INTO "npc_configs" VALUES (69, 'Cannon Guard Tower', 37, 500, 50, 500, 90, 2222, 2223, 750, 1250, 10, 14, 1, 2, 5, 1800, 30, 0, 10, 0, 8, 3000, 2, 0, 0, 0, 8, 20096);
INSERT INTO "npc_configs" VALUES (70, 'Mana Collector', 38, 500, 50, 500, 40, 2222, 2223, 250, 1000, 0, 0, 1, 1, 5, 800, 30, 0, 10, 0, 8, 3000, 2, 0, 0, 0, 8, 20097);
INSERT INTO "npc_configs" VALUES (71, 'Mana Collector', 38, 500, 50, 500, 40, 2222, 2223, 250, 1000, 0, 0, 1, 2, 5, 800, 30, 0, 10, 0, 8, 3000, 2, 0, 0, 0, 8, 20098);
INSERT INTO "npc_configs" VALUES (72, 'Detector', 39, 500, 50, 500, 50, 2222, 2226, 200, 800, 0, 0, 1, 1, 5, 800, 30, 0, 10, 0, 8, 3000, 2, 0, 0, 0, 8, 20099);
INSERT INTO "npc_configs" VALUES (73, 'Detector', 39, 500, 50, 500, 50, 2222, 2222, 200, 800, 0, 0, 1, 2, 5, 800, 30, 0, 10, 0, 8, 3000, 2, 0, 0, 0, 8, 20100);
INSERT INTO "npc_configs" VALUES (74, 'Energy Shield Generator', 40, 500, 50, 500, 60, 2222, 2222, 500, 1000, 0, 0, 1, 1, 5, 800, 30, 0, 10, 0, 8, 3000, 2, 0, 0, 0, 8, 20101);
INSERT INTO "npc_configs" VALUES (75, 'Energy Shield Generator', 40, 500, 50, 500, 60, 2222, 2222, 500, 1000, 0, 0, 1, 2, 5, 800, 30, 0, 10, 0, 8, 3000, 2, 0, 0, 0, 8, 20102);
INSERT INTO "npc_configs" VALUES (76, 'Grand Magic Generator', 41, 50000, 5000, 500, 500, 2222, 2222, 500, 3000, 0, 0, 1, 1, 5, 800, 30, 0, 10, 0, 8, 3000, 2, 0, 1, 0, 8, 20103);
INSERT INTO "npc_configs" VALUES (77, 'Grand Magic Generator', 41, 50000, 5000, 500, 500, 2222, 2222, 500, 3000, 0, 0, 1, 2, 5, 800, 30, 0, 10, 0, 8, 3000, 2, 0, 1, 0, 8, 20104);
INSERT INTO "npc_configs" VALUES (78, 'Mana Stone', 42, 50000, 5000, 500, 500, 2222, 2222, 0, 0, 0, 0, 1, 0, 5, 500, 30, 0, 10, 0, 8, 3000, 2, 0, 0, 0, 8, 20105);
INSERT INTO "npc_configs" VALUES (79, 'Light War Beetle', 43, 250, 450, 500, 20, 2222, 2222, 200, 1000, 10, 5, 1, 1, 0, 700, 100, 7, 10, 0, 7, 60000, 3, 50, 0, 0, 8, 20106);
INSERT INTO "npc_configs" VALUES (80, 'Light War Beetle', 43, 250, 450, 500, 20, 2222, 2222, 200, 1000, 10, 5, 1, 2, 0, 700, 100, 7, 10, 0, 7, 60000, 3, 50, 0, 0, 8, 20107);
INSERT INTO "npc_configs" VALUES (81, 'God''s Hand Knight', 44, 350, 450, 500, 20, 2222, 2322, 300, 1500, 10, 9, 1, 2, 0, 700, 100, 7, 10, 0, 7, 60000, 3, 50, 0, 0, 4, 20108);
INSERT INTO "npc_configs" VALUES (82, 'God''s Hand Knight', 45, 800, 450, 1000, 100, 2222, 2222, 500, 2000, 13, 12, 1, 2, 0, 600, 100, 7, 10, 0, 7, 60000, 3, 50, 0, 0, 5, 20109);
INSERT INTO "npc_configs" VALUES (83, 'Temple Knight', 46, 350, 450, 500, 20, 2222, 2222, 300, 1500, 10, 9, 1, 1, 0, 700, 100, 7, 10, 0, 7, 60000, 3, 50, 0, 0, 4, 20110);
INSERT INTO "npc_configs" VALUES (84, 'Battle Golem', 47, 800, 450, 1000, 100, 2222, 2222, 500, 2000, 13, 12, 1, 1, 0, 600, 100, 7, 10, 0, 7, 60000, 3, 50, 0, 0, 5, 20111);
INSERT INTO "npc_configs" VALUES (85, 'Catapult', 1, 85, 150, 200, 4, 2222, 2222, 100, 300, 8, 5, 1, 1, 0, 600, 70, 0, 10, 0, 7, 10000, 2, 0, 0, 0, 3, 20112);
INSERT INTO "npc_configs" VALUES (86, 'Catapult', 1, 85, 150, 200, 4, 2222, 2222, 100, 300, 8, 5, 1, 2, 0, 600, 70, 0, 10, 0, 7, 10000, 2, 0, 0, 0, 3, 20113);
INSERT INTO "npc_configs" VALUES (87, 'Catapult', 2, 85, 150, 200, 4, 2222, 2222, 100, 300, 8, 5, 1, 1, 0, 600, 70, 0, 10, 0, 7, 10000, 2, 0, 0, 0, 3, 20114);
INSERT INTO "npc_configs" VALUES (88, 'Catapult', 2, 85, 150, 200, 4, 2222, 2222, 100, 300, 8, 5, 1, 2, 0, 600, 70, 0, 10, 0, 7, 10000, 2, 0, 0, 0, 3, 20115);
INSERT INTO "npc_configs" VALUES (89, 'Catapult', 3, 85, 150, 200, 4, 2222, 2222, 100, 300, 8, 5, 1, 1, 0, 600, 70, 0, 10, 0, 7, 10000, 2, 0, 0, 0, 3, 20116);
INSERT INTO "npc_configs" VALUES (90, 'Catapult', 3, 85, 150, 200, 4, 2222, 2222, 100, 300, 8, 5, 1, 2, 0, 600, 70, 0, 10, 0, 7, 10000, 2, 0, 0, 0, 3, 20117);
INSERT INTO "npc_configs" VALUES (91, 'Catapult', 4, 85, 150, 200, 4, 2222, 2222, 100, 300, 8, 5, 1, 1, 0, 600, 70, 0, 10, 0, 7, 10000, 2, 0, 0, 0, 3, 20118);
INSERT INTO "npc_configs" VALUES (92, 'Catapult', 4, 85, 150, 200, 4, 2222, 2222, 100, 300, 8, 5, 1, 2, 0, 600, 70, 0, 10, 0, 7, 10000, 2, 0, 0, 0, 3, 20119);
INSERT INTO "npc_configs" VALUES (93, 'Catapult', 5, 85, 150, 200, 4, 2222, 2222, 100, 300, 8, 5, 1, 1, 0, 600, 70, 0, 10, 0, 7, 10000, 2, 0, 0, 0, 3, 20120);
INSERT INTO "npc_configs" VALUES (94, 'Catapult', 5, 85, 150, 200, 4, 2222, 2222, 100, 300, 8, 5, 1, 2, 0, 600, 70, 0, 10, 0, 7, 10000, 2, 0, 0, 0, 3, 20121);
INSERT INTO "npc_configs" VALUES (95, 'Catapult', 6, 85, 150, 200, 4, 2222, 2222, 100, 300, 8, 5, 1, 1, 0, 600, 70, 0, 10, 0, 7, 10000, 2, 0, 0, 0, 3, 20122);
INSERT INTO "npc_configs" VALUES (96, 'Catapult', 6, 85, 150, 200, 4, 2222, 2222, 100, 300, 8, 5, 1, 2, 0, 600, 70, 0, 10, 0, 7, 10000, 2, 0, 0, 0, 3, 20123);
INSERT INTO "npc_configs" VALUES (97, 'Catapult', 51, 500, 50, 500, 100, 2222, 2222, 500, 1000, 10, 14, 1, 1, 0, 1600, 100, 0, 10, 0, 9, 10000, 1, 0, 0, 0, 9, 20124);
INSERT INTO "npc_configs" VALUES (98, 'Catapult', 51, 500, 50, 500, 100, 2222, 2222, 500, 1000, 10, 14, 1, 2, 0, 1600, 50, 0, 10, 0, 9, 10000, 1, 0, 0, 0, 9, 20125);
INSERT INTO "npc_configs" VALUES (99, 'Sorceress', 82, 300, 450, 1000, 100, 2222, 2222, 750, 1250, 11, 10, 1, 1, 0, 700, 100, 7, 10, 0, 7, 60000, 3, 50, 19000, 9, 6, 20126);
INSERT INTO "npc_configs" VALUES (100, 'Sorceress', 82, 300, 450, 1000, 100, 2222, 2222, 750, 1250, 11, 10, 1, 2, 0, 700, 100, 7, 10, 0, 7, 60000, 3, 50, 19000, 9, 6, 20127);
INSERT INTO "npc_configs" VALUES (101, 'Ancient Temple Knight', 83, 800, 500, 1000, 100, 2222, 2222, 1000, 1500, 13, 12, 1, 1, 0, 1000, 100, 0, 10, 0, 7, 60000, 3, 50, 0, 0, 3, 20128);
INSERT INTO "npc_configs" VALUES (102, 'Ancient Temple Knight', 83, 800, 500, 1000, 100, 2222, 2222, 1000, 1500, 13, 12, 1, 2, 0, 1000, 100, 0, 10, 0, 7, 60000, 3, 50, 0, 0, 3, 20129);
INSERT INTO "npc_configs" VALUES (103, 'Elf Master', 84, 250, 450, 500, 100, 2222, 2222, 500, 1000, 10, 6, 1, 1, 0, 700, 100, 0, 10, 0, 8, 60000, 3, 50, 0, 0, 8, 20130);
INSERT INTO "npc_configs" VALUES (104, 'Elf Master', 84, 250, 450, 500, 100, 2222, 2222, 500, 1000, 10, 6, 1, 2, 0, 700, 100, 0, 10, 0, 8, 60000, 3, 50, 0, 0, 8, 20131);
INSERT INTO "npc_configs" VALUES (105, 'Dark Shadow Knight', 85, 500, 400, 600, 100, 2222, 2222, 700, 1250, 10, 9, 1, 1, 0, 500, 100, 0, 10, 0, 6, 60000, 3, 50, 0, 0, 4, 20132);
INSERT INTO "npc_configs" VALUES (106, 'Dark Shadow Knight', 85, 500, 400, 600, 100, 2222, 2222, 700, 1250, 10, 9, 1, 2, 0, 500, 100, 0, 10, 0, 6, 60000, 3, 50, 0, 0, 4, 20133);
INSERT INTO "npc_configs" VALUES (107, 'Heavy Battle Tank', 86, 500, 150, 500, 100, 2222, 2222, 550, 1100, 10, 14, 1, 1, 0, 1600, 100, 0, 10, 0, 9, 60000, 3, 50, 0, 0, 9, 20134);
INSERT INTO "npc_configs" VALUES (108, 'Heavy Battle Tank', 86, 500, 150, 500, 100, 2222, 2222, 550, 1100, 10, 14, 1, 2, 0, 1600, 100, 0, 10, 0, 9, 60000, 3, 50, 0, 0, 9, 20135);
INSERT INTO "npc_configs" VALUES (109, 'Crossbow Turret', 87, 800, 200, 500, 100, 2222, 2222, 550, 1200, 8, 8, 1, 1, 5, 700, 30, 0, 10, 0, 8, 3000, 2, 0, 0, 0, 9, 20136);
INSERT INTO "npc_configs" VALUES (110, 'Crossbow Turret', 87, 800, 200, 500, 100, 2222, 2222, 550, 1200, 8, 8, 1, 2, 5, 700, 30, 0, 10, 0, 8, 3000, 2, 0, 0, 0, 9, 20137);
INSERT INTO "npc_configs" VALUES (111, 'Barbarian', 88, 350, 500, 1000, 100, 2222, 2222, 800, 1700, 15, 12, 1, 1, 0, 1400, 100, 0, 10, 0, 7, 60000, 3, 50, 0, 0, 2, 20138);
INSERT INTO "npc_configs" VALUES (112, 'Barbarian', 88, 350, 500, 1000, 100, 2222, 2222, 800, 1700, 15, 12, 1, 2, 0, 1400, 100, 0, 10, 0, 7, 60000, 3, 50, 0, 0, 2, 20139);
INSERT INTO "npc_configs" VALUES (113, 'Ancient Giant Cannon', 89, 1000, 200, 500, 100, 2222, 2222, 550, 1100, 14, 10, 1, 1, 5, 1600, 30, 0, 10, 0, 8, 3000, 2, 0, 0, 0, 9, 20140);
INSERT INTO "npc_configs" VALUES (114, 'Ancient Giant Cannon', 89, 1000, 200, 500, 100, 2222, 2222, 550, 1100, 14, 10, 1, 2, 5, 1600, 30, 0, 10, 0, 8, 3000, 2, 0, 0, 0, 9, 20141);
INSERT INTO "npc_configs" VALUES (115, 'Training Dummy', 34, 999999, 15, 300, 100, 0, 0, 0, 0, 0, 0, 1, 10, 3, 2100, 5, 0, 10, 0, 7, 3000, 2, 0, 0, 0, 1, 20073);

-- ----------------------------
-- Table structure for npc_shop_mapping
-- ----------------------------
DROP TABLE IF EXISTS "npc_shop_mapping";
CREATE TABLE "npc_shop_mapping" (
  "npc_config_id" INTEGER,
  "shop_id" INTEGER NOT NULL,
  "description" TEXT,
  PRIMARY KEY ("npc_config_id")
);

-- ----------------------------
-- Records of npc_shop_mapping
-- ----------------------------
INSERT INTO "npc_shop_mapping" VALUES (56, 1, 'General Shop (Shopkeeper)');
INSERT INTO "npc_shop_mapping" VALUES (62, 2, 'Blacksmith Shop');

-- ----------------------------
-- Table structure for potion_configs
-- ----------------------------
DROP TABLE IF EXISTS "potion_configs";
CREATE TABLE "potion_configs" (
  "potion_id" INTEGER,
  "name" TEXT NOT NULL,
  "array0" INTEGER NOT NULL,
  "array1" INTEGER NOT NULL,
  "array2" INTEGER NOT NULL,
  "array3" INTEGER NOT NULL,
  "array4" INTEGER NOT NULL,
  "array5" INTEGER NOT NULL,
  "array6" INTEGER NOT NULL,
  "array7" INTEGER NOT NULL,
  "array8" INTEGER NOT NULL,
  "array9" INTEGER NOT NULL,
  "array10" INTEGER NOT NULL,
  "array11" INTEGER NOT NULL,
  "skill_limit" INTEGER NOT NULL,
  "difficulty" INTEGER NOT NULL,
  PRIMARY KEY ("potion_id")
);

-- ----------------------------
-- Records of potion_configs
-- ----------------------------
INSERT INTO "potion_configs" VALUES (1, 'RedPotion', 220, 1, 192, 1, 191, 1, -1, 0, -1, 0, -1, 0, 20, 10);
INSERT INTO "potion_configs" VALUES (2, 'BluePotion', 220, 1, 193, 1, 189, 1, -1, 0, -1, 0, -1, 0, 20, 10);
INSERT INTO "potion_configs" VALUES (3, 'GreenPotion', 220, 1, 215, 1, 188, 1, -1, 0, -1, 0, -1, 0, 20, 10);
INSERT INTO "potion_configs" VALUES (4, 'DilutionPotion', 220, 1, 217, 1, 208, 1, 190, 2, -1, 0, -1, 0, 30, 20);
INSERT INTO "potion_configs" VALUES (5, 'BigRedPotion', 223, 1, 220, 3, 192, 1, 191, 1, -1, 0, -1, 0, 40, 20);
INSERT INTO "potion_configs" VALUES (6, 'BigBluePotion', 220, 3, 197, 1, 193, 1, 189, 1, -1, 0, -1, 0, 40, 20);
INSERT INTO "potion_configs" VALUES (7, 'BigGreenPotion', 220, 2, 215, 1, 189, 1, 188, 1, 188, 1, -1, 0, 40, 20);
INSERT INTO "potion_configs" VALUES (8, 'InvisibilityPotion', 220, 1, 219, 1, 208, 1, 206, 1, -1, 0, -1, 0, 50, 30);
INSERT INTO "potion_configs" VALUES (9, 'HairColorPotion', 220, 1, 216, 1, 205, 1, 188, 1, -1, 0, -1, 0, 60, 40);
INSERT INTO "potion_configs" VALUES (10, 'HairStylePotion', 221, 1, 220, 1, 204, 1, 188, 1, -1, 0, -1, 0, 70, 40);
INSERT INTO "potion_configs" VALUES (11, 'SkinColorPotion', 220, 1, 205, 1, 201, 1, 190, 1, -1, 0, -1, 0, 90, 60);
INSERT INTO "potion_configs" VALUES (12, 'SexChangePotion', 225, 1, 220, 1, 219, 1, 210, 1, 205, 1, 194, 1, 95, 70);
INSERT INTO "potion_configs" VALUES (13, 'OgrePotion', 222, 1, 214, 1, 212, 1, 211, 1, 210, 1, 209, 1, 100, 80);
INSERT INTO "potion_configs" VALUES (14, 'PowerGreenPotion', 554, 1, 222, 1, 220, 1, 199, 1, 196, 1, -1, 0, 95, 80);
INSERT INTO "potion_configs" VALUES (15, 'UnderWearPotion', 575, 1, 220, 1, 218, 1, 217, 1, 215, 1, 203, 1, 60, 50);
INSERT INTO "potion_configs" VALUES (16, 'UnfreezePotion', 548, 1, 220, 1, 217, 1, 207, 1, 198, 1, -1, 0, 90, 60);
INSERT INTO "potion_configs" VALUES (17, 'SuperRedPotion', 823, 1, 222, 1, 220, 1, 193, 1, -1, 0, -1, 0, 75, 50);
INSERT INTO "potion_configs" VALUES (18, 'SuperBluePotion', 820, 1, 220, 1, 206, 1, 196, 1, -1, 0, -1, 0, 75, 50);
INSERT INTO "potion_configs" VALUES (19, 'SuperGreenPotion', 821, 1, 220, 1, 199, 1, 192, 1, -1, 0, -1, 0, 70, 40);

-- ----------------------------
-- Table structure for quest_configs
-- ----------------------------
DROP TABLE IF EXISTS "quest_configs";
CREATE TABLE "quest_configs" (
  "quest_index" INTEGER,
  "side" INTEGER NOT NULL,
  "quest_type" INTEGER NOT NULL,
  "target_config_id" INTEGER NOT NULL,
  "max_count" INTEGER NOT NULL,
  "quest_from" INTEGER NOT NULL,
  "min_level" INTEGER NOT NULL,
  "max_level" INTEGER NOT NULL,
  "required_skill_num" INTEGER NOT NULL,
  "required_skill_level" INTEGER NOT NULL,
  "time_limit" INTEGER NOT NULL,
  "assign_type" INTEGER NOT NULL,
  "reward_type1" INTEGER NOT NULL,
  "reward_amount1" INTEGER NOT NULL,
  "reward_type2" INTEGER NOT NULL,
  "reward_amount2" INTEGER NOT NULL,
  "reward_type3" INTEGER NOT NULL,
  "reward_amount3" INTEGER NOT NULL,
  "contribution" INTEGER NOT NULL,
  "contribution_limit" INTEGER NOT NULL,
  "response_mode" INTEGER NOT NULL,
  "target_name" TEXT NOT NULL,
  "target_x" INTEGER NOT NULL,
  "target_y" INTEGER NOT NULL,
  "target_range" INTEGER NOT NULL,
  "quest_id" INTEGER NOT NULL,
  "req_contribution" INTEGER NOT NULL,
  PRIMARY KEY ("quest_index")
);

-- ----------------------------
-- Records of quest_configs
-- ----------------------------
INSERT INTO "quest_configs" VALUES (1, 1, 1, 2, 20, 4, 11, 20, -1, -1, -1, -1, -1, 100, 90, 200, 90, 100, 2, 20, 1, 'arefarm', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (2, 2, 1, 2, 20, 4, 11, 20, -1, -1, -1, -1, -1, 100, 90, 200, 90, 100, 2, 20, 1, 'elvfarm', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (3, 1, 1, 0, 30, 4, 11, 20, -1, -1, -1, -1, -1, 100, 90, 200, 90, 100, 2, 20, 1, 'arefarm', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (4, 2, 1, 0, 30, 4, 11, 20, -1, -1, -1, -1, -1, 100, 90, 200, 90, 100, 2, 20, 1, 'elvfarm', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (5, 1, 1, 3, 15, 4, 11, 20, -1, -1, -1, -1, -1, 100, 90, 200, 90, 100, 2, 20, 1, 'arefarm', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (6, 2, 1, 3, 15, 4, 11, 20, -1, -1, -1, -1, -1, 100, 90, 200, 90, 100, 2, 20, 1, 'elvfarm', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (7, 1, 1, 9, 25, 4, 20, 40, -1, -1, -1, -1, -1, 500, 90, 500, 90, 400, 3, 40, 1, 'arefarm', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (8, 2, 1, 9, 25, 4, 20, 40, -1, -1, -1, -1, -1, 500, 90, 500, 90, 400, 3, 40, 1, 'elvfarm', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (9, 1, 1, 4, 30, 4, 20, 40, -1, -1, -1, -1, -1, 500, 90, 500, 90, 400, 3, 40, 1, 'arefarm', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (10, 2, 1, 4, 30, 4, 20, 40, -1, -1, -1, -1, -1, 500, 90, 500, 90, 400, 3, 40, 1, 'elvfarm', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (11, 1, 1, 5, 30, 4, 20, 40, -1, -1, -1, -1, -1, 500, 90, 500, 90, 400, 3, 40, 1, 'arefarm', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (12, 2, 1, 5, 30, 4, 20, 40, -1, -1, -1, -1, -1, 500, 90, 500, 90, 400, 3, 40, 1, 'elvfarm', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (13, 1, 1, 5, 35, 4, 40, 50, -1, -1, -1, -1, -1, 1000, 90, 1000, 90, 800, 4, 80, 1, 'aresden', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (14, 2, 1, 5, 35, 4, 40, 50, -1, -1, -1, -1, -1, 1000, 90, 1000, 90, 800, 4, 80, 1, 'elvine', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (15, 1, 1, 9, 30, 4, 40, 50, -1, -1, -1, -1, -1, 1000, 90, 1000, 90, 800, 4, 80, 1, 'aresden', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (16, 2, 1, 9, 30, 4, 40, 50, -1, -1, -1, -1, -1, 1000, 90, 1000, 90, 800, 4, 80, 1, 'elvine', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (17, 1, 1, 10, 25, 4, 40, 50, -1, -1, -1, -1, -1, 1000, 90, 1000, 90, 800, 4, 80, 1, 'huntzone2', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (18, 2, 1, 10, 25, 4, 40, 50, -1, -1, -1, -1, -1, 1000, 90, 1000, 90, 800, 4, 80, 1, 'huntzone1', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (19, 1, 1, 10, 30, 4, 50, 70, -1, -1, -1, -1, -1, 2000, 90, 3000, 90, 2500, 5, 120, 1, 'middled1n', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (20, 2, 1, 10, 30, 4, 50, 70, -1, -1, -1, -1, -1, 2000, 90, 3000, 90, 2500, 5, 121, 1, 'middled1n', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (21, 1, 1, 13, 20, 4, 50, 70, -1, -1, -1, -1, -1, 2000, 90, 3000, 90, 2500, 5, 122, 1, 'huntzone2', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (22, 2, 1, 13, 20, 4, 50, 70, -1, -1, -1, -1, -1, 2000, 90, 3000, 90, 2500, 5, 123, 1, 'huntzone1', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (23, 1, 1, 16, 25, 4, 70, 80, -1, -1, -1, -1, -1, 3000, 90, 4000, 90, 3000, 6, 124, 1, 'huntzone2', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (24, 2, 1, 16, 25, 4, 70, 80, -1, -1, -1, -1, -1, 3000, 90, 4000, 90, 3000, 6, 125, 1, 'huntzone1', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (25, 1, 1, 17, 15, 4, 80, 90, -1, -1, -1, -1, -1, 4000, 90, 5000, 90, 4000, 7, 160, 1, 'areuni', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (26, 2, 1, 17, 15, 4, 80, 90, -1, -1, -1, -1, -1, 4000, 90, 5000, 90, 4000, 7, 160, 1, 'elvuni', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (27, 1, 1, 12, 30, 4, 80, 90, -1, -1, -1, -1, -1, 4000, 90, 5000, 90, 4000, 7, 160, 1, 'middled1n', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (28, 2, 1, 12, 30, 4, 80, 90, -1, -1, -1, -1, -1, 4000, 90, 5000, 90, 4000, 7, 160, 1, 'middled1n', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (29, 1, 7, 0, 0, 4, 50, 300, -1, -1, -1, -1, -2, 1, -2, 1, -2, 1, 1, 500, 1, 'elvine', 166, 136, 3, 723, 70);
INSERT INTO "quest_configs" VALUES (30, 2, 7, 0, 0, 4, 50, 300, -1, -1, -1, -1, -2, 1, -2, 1, -2, 1, 1, 500, 1, 'aresden', 244, 136, 3, 723, 70);
INSERT INTO "quest_configs" VALUES (31, 1, 7, 0, 0, 4, 50, 300, -1, -1, -1, -1, -2, 1, -2, 1, -2, 1, 1, 500, 1, 'elvine', 200, 193, 3, 723, 80);
INSERT INTO "quest_configs" VALUES (32, 2, 7, 0, 0, 4, 50, 300, -1, -1, -1, -1, -2, 1, -2, 1, -2, 1, 1, 500, 1, 'aresden', 218, 180, 3, 723, 80);
INSERT INTO "quest_configs" VALUES (33, 1, 7, 0, 0, 4, 50, 300, -1, -1, -1, -1, -2, 1, -2, 1, -2, 1, 1, 500, 1, 'elvine', 202, 108, 3, 723, 90);
INSERT INTO "quest_configs" VALUES (34, 2, 7, 0, 0, 4, 50, 300, -1, -1, -1, -1, -2, 1, -2, 1, -2, 1, 1, 500, 1, 'aresden', 179, 97, 3, 723, 90);
INSERT INTO "quest_configs" VALUES (41, 1, 10, 0, 0, 4, 31, 120, -1, -1, -1, 1, 0, 0, 0, 0, 0, 0, 1, 1000, 1, 'huntzone1', -1, -1, 3, 614, 0);
INSERT INTO "quest_configs" VALUES (42, 2, 10, 0, 0, 4, 31, 120, -1, -1, -1, 1, 0, 0, 0, 0, 0, 0, 1, 1000, 1, 'huntzone2', -1, -1, 3, 614, 0);
INSERT INTO "quest_configs" VALUES (43, 1, 1, 15, 20, 4, 90, 100, -1, -1, -1, -1, -1, 5000, 90, 7000, 90, 5500, 7, 210, 1, 'middled1n', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (44, 2, 1, 15, 20, 4, 90, 100, -1, -1, -1, -1, -1, 5000, 90, 7000, 90, 5500, 7, 210, 1, 'middled1n', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (45, 1, 1, 17, 20, 4, 90, 100, -1, -1, -1, -1, -1, 5000, 90, 7000, 90, 5500, 7, 210, 1, 'areuni', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (46, 2, 1, 17, 20, 4, 90, 100, -1, -1, -1, -1, -1, 5000, 90, 7000, 90, 5500, 7, 210, 1, 'elvuni', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (47, 1, 1, 18, 20, 4, 100, 110, -1, -1, -1, -1, -1, 5500, 90, 8000, 90, 6000, 8, 260, 1, '2ndmiddle', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (48, 2, 1, 18, 20, 4, 100, 110, -1, -1, -1, -1, -1, 5500, 90, 8000, 90, 6000, 8, 260, 1, '2ndmiddle', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (49, 1, 1, 17, 25, 4, 100, 110, -1, -1, -1, -1, -1, 5500, 90, 8000, 90, 6000, 8, 260, 1, 'areuni', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (50, 2, 1, 17, 25, 4, 100, 110, -1, -1, -1, -1, -1, 5500, 90, 8000, 90, 6000, 8, 260, 1, 'elvuni', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (51, 1, 1, 18, 25, 4, 110, 120, -1, -1, -1, -1, -1, 6000, 90, 10000, 90, 8500, 8, 300, 1, '2ndmiddle', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (52, 2, 1, 18, 25, 4, 110, 120, -1, -1, -1, -1, -1, 6000, 90, 10000, 90, 8500, 8, 300, 1, '2ndmiddle', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (53, 1, 1, 21, 15, 4, 110, 120, -1, -1, -1, -1, -1, 6000, 90, 10000, 90, 8500, 8, 300, 1, 'areuni', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (54, 2, 1, 21, 15, 4, 110, 120, -1, -1, -1, -1, -1, 6000, 90, 10000, 90, 8500, 8, 300, 1, 'elvuni', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (55, 1, 1, 22, 15, 4, 120, 130, -1, -1, -1, -1, -1, 6500, 90, 12000, 90, 9000, 9, 350, 1, 'huntzone4', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (56, 2, 1, 22, 15, 4, 120, 130, -1, -1, -1, -1, -1, 6500, 90, 12000, 90, 9000, 9, 350, 1, 'huntzone3', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (57, 1, 1, 17, 30, 4, 120, 130, -1, -1, -1, -1, -1, 6500, 90, 12000, 90, 9000, 9, 350, 1, 'huntzone4', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (58, 2, 1, 17, 30, 4, 120, 130, -1, -1, -1, -1, -1, 6500, 90, 12000, 90, 9000, 9, 350, 1, 'huntzone3', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (59, 1, 1, 23, 20, 4, 130, 140, -1, -1, -1, -1, -1, 7000, 90, 13000, 90, 10000, 10, 400, 1, 'huntzone4', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (60, 2, 1, 23, 20, 4, 130, 140, -1, -1, -1, -1, -1, 7000, 90, 13000, 90, 10000, 10, 400, 1, 'huntzone3', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (61, 1, 1, 22, 15, 4, 130, 140, -1, -1, -1, -1, -1, 7000, 90, 13000, 90, 10000, 10, 400, 1, 'dglv3', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (62, 2, 1, 22, 15, 4, 130, 140, -1, -1, -1, -1, -1, 7000, 90, 13000, 90, 10000, 10, 400, 1, 'dglv3', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (63, 1, 1, 22, 20, 4, 140, 150, -1, -1, -1, -1, -1, 8000, 90, 15000, 90, 12000, 11, 450, 1, 'toh1', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (64, 2, 1, 22, 20, 4, 140, 150, -1, -1, -1, -1, -1, 8000, 90, 15000, 90, 12000, 11, 450, 1, 'toh1', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (65, 1, 1, 28, 15, 4, 140, 150, -1, -1, -1, -1, -1, 8000, 90, 15000, 90, 12000, 12, 450, 1, 'dglv3', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (66, 2, 1, 28, 15, 4, 140, 150, -1, -1, -1, -1, -1, 8000, 90, 15000, 90, 12000, 12, 450, 1, 'dglv3', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (67, 1, 1, 30, 18, 4, 140, 150, -1, -1, -1, -1, -1, 8000, 90, 15000, 90, 12000, 11, 450, 1, 'huntzone4', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (68, 2, 1, 30, 18, 4, 140, 150, -1, -1, -1, -1, -1, 8000, 90, 15000, 90, 12000, 11, 450, 1, 'huntzone3', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (69, 1, 1, 29, 20, 4, 140, 150, -1, -1, -1, -1, -1, 8000, 90, 15000, 90, 12000, 10, 450, 1, 'middleland', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (70, 2, 1, 29, 20, 4, 140, 150, -1, -1, -1, -1, -1, 8000, 90, 15000, 90, 12000, 10, 450, 1, 'middleland', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (71, 1, 1, 24, 20, 4, 150, 160, -1, -1, -1, -1, -1, 9000, 90, 16000, 90, 13000, 13, 520, 1, 'icebound', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (72, 2, 1, 24, 20, 4, 150, 160, -1, -1, -1, -1, -1, 9000, 90, 16000, 90, 13000, 13, 520, 1, 'icebound', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (73, 1, 1, 31, 30, 4, 150, 160, -1, -1, -1, -1, -1, 9000, 90, 16000, 90, 13000, 10, 520, 1, 'toh1', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (74, 2, 1, 31, 30, 4, 150, 160, -1, -1, -1, -1, -1, 9000, 90, 16000, 90, 13000, 10, 520, 1, 'toh1', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (75, 1, 1, 36, 10, 4, 160, 170, -1, -1, -1, -1, -1, 10000, 90, 17000, 90, 13500, 11, 600, 1, 'dglv4', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (76, 2, 1, 36, 10, 4, 160, 170, -1, -1, -1, -1, -1, 10000, 90, 17000, 90, 13500, 11, 600, 1, 'dglv4', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (77, 1, 1, 35, 15, 4, 160, 170, -1, -1, -1, -1, -1, 10000, 90, 17000, 90, 13500, 12, 600, 1, 'huntzone4', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (78, 2, 1, 35, 15, 4, 160, 170, -1, -1, -1, -1, -1, 10000, 90, 17000, 90, 13500, 12, 600, 1, 'huntzone3', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (79, 1, 1, 32, 18, 4, 160, 170, -1, -1, -1, -1, -1, 10000, 90, 17000, 90, 13500, 11, 600, 1, 'icebound', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (80, 2, 1, 32, 18, 4, 160, 170, -1, -1, -1, -1, -1, 10000, 90, 17000, 90, 13500, 11, 600, 1, 'icebound', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (81, 1, 1, 28, 22, 4, 170, 180, -1, -1, -1, -1, -1, 12000, 90, 18000, 90, 14000, 12, 700, 1, 'dglv4', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (82, 2, 1, 28, 22, 4, 170, 180, -1, -1, -1, -1, -1, 12000, 90, 18000, 90, 14000, 12, 700, 1, 'dglv4', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (83, 1, 1, 36, 15, 4, 170, 180, -1, -1, -1, -1, -1, 12000, 90, 18000, 90, 14000, 14, 700, 1, 'dglv4', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (84, 2, 1, 36, 15, 4, 170, 180, -1, -1, -1, -1, -1, 12000, 90, 18000, 90, 14000, 14, 700, 1, 'dglv4', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (85, 1, 1, 38, 13, 4, 170, 180, -1, -1, -1, -1, -1, 12000, 90, 18000, 90, 14000, 15, 700, 1, 'toh3', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (86, 2, 1, 38, 13, 4, 170, 180, -1, -1, -1, -1, -1, 12000, 90, 18000, 90, 14000, 15, 700, 1, 'toh3', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (87, 1, 1, 35, 20, 4, 170, 180, -1, -1, -1, -1, -1, 12000, 90, 18000, 90, 14000, 12, 700, 1, 'huntzone4', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (88, 2, 1, 35, 20, 4, 170, 180, -1, -1, -1, -1, -1, 12000, 90, 18000, 90, 14000, 12, 700, 1, 'huntzone3', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (89, 1, 1, 32, 22, 4, 170, 180, -1, -1, -1, -1, -1, 12000, 90, 18000, 90, 14000, 12, 700, 1, 'icebound', 0, 0, 0, 354, 0);
INSERT INTO "quest_configs" VALUES (90, 2, 1, 32, 22, 4, 170, 180, -1, -1, -1, -1, -1, 12000, 90, 18000, 90, 14000, 12, 700, 1, 'icebound', 0, 0, 0, 354, 0);

-- ----------------------------
-- Table structure for realmlist
-- ----------------------------
DROP TABLE IF EXISTS "realmlist";
CREATE TABLE "realmlist" (
  "id" INTEGER,
  "realm_name" TEXT NOT NULL,
  "login_listen_ip" TEXT NOT NULL DEFAULT '0.0.0.0',
  "login_listen_port" INTEGER NOT NULL DEFAULT 2848,
  "game_server_listen_ip" TEXT NOT NULL DEFAULT '0.0.0.0',
  "game_server_listen_port" INTEGER NOT NULL DEFAULT 2858,
  "game_server_connection_ip" TEXT DEFAULT NULL,
  "game_server_connection_port" INTEGER DEFAULT NULL,
  PRIMARY KEY ("id"),
  UNIQUE ("realm_name" ASC)
);

-- ----------------------------
-- Records of realmlist
-- ----------------------------
INSERT INTO "realmlist" VALUES (1, 'Apocalypse', '0.0.0.0', 2500, '0.0.0.0', 9907, NULL, NULL);

-- ----------------------------
-- Table structure for settings
-- ----------------------------
DROP TABLE IF EXISTS "settings";
CREATE TABLE "settings" (
  "key" TEXT,
  "value" TEXT NOT NULL,
  "description" TEXT,
  PRIMARY KEY ("key")
);

-- ----------------------------
-- Records of settings
-- ----------------------------
INSERT INTO "settings" VALUES ('primary-drop-rate', '1.0', 'Primary item drop rate drop chance multiplier');
INSERT INTO "settings" VALUES ('secondary-drop-rate', '1.0', 'Secondary item drop chance multiplier');
INSERT INTO "settings" VALUES ('enemy-kill-mode', 'deathmatch', 'PvP mode: deathmatch (anywhere) or classic (enemy territory)');
INSERT INTO "settings" VALUES ('enemy-kill-adjust', '1', 'Multiplier for enemy kill rewards');
INSERT INTO "settings" VALUES ('monday-raid-time', '3', 'Raid event start time on Mondays (minutes past hour)');
INSERT INTO "settings" VALUES ('tuesday-raid-time', '3', 'Raid event start time on Tuesdays (minutes past hour)');
INSERT INTO "settings" VALUES ('wednesday-raid-time', '3', 'Raid event start time on Wednesdays (minutes past hour)');
INSERT INTO "settings" VALUES ('thursday-raid-time', '3', 'Raid event start time on Thursdays (minutes past hour)');
INSERT INTO "settings" VALUES ('friday-raid-time', '30', 'Raid event start time on Fridays (minutes past hour)');
INSERT INTO "settings" VALUES ('saturday-raid-time', '45', 'Raid event start time on Saturdays (minutes past hour)');
INSERT INTO "settings" VALUES ('sunday-raid-time', '60', 'Raid event start time on Sundays (minutes past hour)');
INSERT INTO "settings" VALUES ('log-chat-settings', 'all', 'Chat logging level: player, gm, all, or none');
INSERT INTO "settings" VALUES ('slate-success-rate', '25', 'Success rate for slate crafting (%)');
INSERT INTO "settings" VALUES ('rep-drop-modifier', '5', 'Reputation drop modifier on death');
INSERT INTO "settings" VALUES ('client-timeout-ms', '30000', 'Milliseconds before disconnecting inactive clients');
INSERT INTO "settings" VALUES ('stamina-regen-interval', '10000', 'Milliseconds between stamina regeneration ticks');
INSERT INTO "settings" VALUES ('poison-damage-interval', '12000', 'Milliseconds between poison damage ticks');
INSERT INTO "settings" VALUES ('health-regen-interval', '15000', 'Milliseconds between health regeneration ticks');
INSERT INTO "settings" VALUES ('mana-regen-interval', '20000', 'Milliseconds between mana regeneration ticks');
INSERT INTO "settings" VALUES ('hunger-consume-interval', '60000', 'Milliseconds between hunger consumption ticks');
INSERT INTO "settings" VALUES ('summon-creature-duration', '300000', 'Milliseconds summoned creatures remain active');
INSERT INTO "settings" VALUES ('autosave-interval', '600000', 'Milliseconds between automatic player data saves');
INSERT INTO "settings" VALUES ('lag-protection-interval', '7000', 'Milliseconds of inactivity before damage immunity');
INSERT INTO "settings" VALUES ('base-stat-value', '10', 'Starting value for all character stats');
INSERT INTO "settings" VALUES ('creation-stat-bonus', '4', 'Maximum bonus points per stat at character creation');
INSERT INTO "settings" VALUES ('levelup-stat-gain', '3', 'Stat points gained per level up');
INSERT INTO "settings" VALUES ('minimum-hit-ratio', '15', 'Minimum hit chance percentage');
INSERT INTO "settings" VALUES ('maximum-hit-ratio', '99', 'Maximum hit chance percentage');
INSERT INTO "settings" VALUES ('nighttime-duration', '30', 'Minutes past hour when night begins');
INSERT INTO "settings" VALUES ('starting-guild-rank', '12', 'Initial rank for new guild members');
INSERT INTO "settings" VALUES ('grand-magic-mana-consumption', '15', 'Mana consumed per Grand Magic Generator tick');
INSERT INTO "settings" VALUES ('maximum-construction-points', '30000', 'Maximum construction points during Crusade');
INSERT INTO "settings" VALUES ('maximum-summon-points', '30000', 'Maximum summon points during war events');
INSERT INTO "settings" VALUES ('maximum-war-contribution', '200000', 'Maximum war contribution points per player');
INSERT INTO "settings" VALUES ('max-level', '180', 'Maximum character level');
INSERT INTO "settings" VALUES ('gold-drop-rate', '1.0', 'Default gold rate drop chance multiplier');
INSERT INTO "settings" VALUES ('max-bank-items', '200', 'Default max bank items');

-- ----------------------------
-- Table structure for shop_items
-- ----------------------------
DROP TABLE IF EXISTS "shop_items";
CREATE TABLE "shop_items" (
  "shop_id" INTEGER NOT NULL,
  "item_id" INTEGER NOT NULL,
  "sort_order" INTEGER NOT NULL DEFAULT 0,
  PRIMARY KEY ("shop_id", "item_id")
);

-- ----------------------------
-- Records of shop_items
-- ----------------------------
INSERT INTO "shop_items" VALUES (1, 104, 9);
INSERT INTO "shop_items" VALUES (1, 91, 0);
INSERT INTO "shop_items" VALUES (1, 92, 1);
INSERT INTO "shop_items" VALUES (1, 93, 2);
INSERT INTO "shop_items" VALUES (1, 94, 3);
INSERT INTO "shop_items" VALUES (1, 95, 4);
INSERT INTO "shop_items" VALUES (1, 96, 5);
INSERT INTO "shop_items" VALUES (1, 97, 6);
INSERT INTO "shop_items" VALUES (1, 98, 7);
INSERT INTO "shop_items" VALUES (1, 99, 8);
INSERT INTO "shop_items" VALUES (1, 111, 11);
INSERT INTO "shop_items" VALUES (1, 105, 10);
INSERT INTO "shop_items" VALUES (1, 114, 13);
INSERT INTO "shop_items" VALUES (1, 115, 14);
INSERT INTO "shop_items" VALUES (1, 113, 12);
INSERT INTO "shop_items" VALUES (1, 1085, 73);
INSERT INTO "shop_items" VALUES (1, 1107, 74);
INSERT INTO "shop_items" VALUES (1, 226, 15);
INSERT INTO "shop_items" VALUES (1, 227, 16);
INSERT INTO "shop_items" VALUES (1, 800, 59);
INSERT INTO "shop_items" VALUES (1, 802, 60);
INSERT INTO "shop_items" VALUES (1, 803, 61);
INSERT INTO "shop_items" VALUES (1, 804, 62);
INSERT INTO "shop_items" VALUES (1, 805, 63);
INSERT INTO "shop_items" VALUES (1, 806, 64);
INSERT INTO "shop_items" VALUES (1, 807, 65);
INSERT INTO "shop_items" VALUES (1, 808, 66);
INSERT INTO "shop_items" VALUES (1, 809, 67);
INSERT INTO "shop_items" VALUES (1, 810, 68);
INSERT INTO "shop_items" VALUES (1, 811, 69);
INSERT INTO "shop_items" VALUES (1, 812, 70);
INSERT INTO "shop_items" VALUES (1, 813, 71);
INSERT INTO "shop_items" VALUES (1, 814, 72);
INSERT INTO "shop_items" VALUES (1, 375, 31);
INSERT INTO "shop_items" VALUES (1, 360, 21);
INSERT INTO "shop_items" VALUES (1, 369, 26);
INSERT INTO "shop_items" VALUES (1, 364, 22);
INSERT INTO "shop_items" VALUES (1, 365, 23);
INSERT INTO "shop_items" VALUES (1, 366, 24);
INSERT INTO "shop_items" VALUES (1, 368, 25);
INSERT INTO "shop_items" VALUES (1, 370, 27);
INSERT INTO "shop_items" VALUES (1, 371, 28);
INSERT INTO "shop_items" VALUES (1, 372, 29);
INSERT INTO "shop_items" VALUES (1, 373, 30);
INSERT INTO "shop_items" VALUES (1, 450, 34);
INSERT INTO "shop_items" VALUES (1, 451, 35);
INSERT INTO "shop_items" VALUES (1, 453, 36);
INSERT INTO "shop_items" VALUES (1, 484, 46);
INSERT INTO "shop_items" VALUES (1, 460, 38);
INSERT INTO "shop_items" VALUES (1, 459, 37);
INSERT INTO "shop_items" VALUES (1, 471, 40);
INSERT INTO "shop_items" VALUES (1, 470, 39);
INSERT INTO "shop_items" VALUES (1, 473, 41);
INSERT INTO "shop_items" VALUES (1, 474, 42);
INSERT INTO "shop_items" VALUES (1, 479, 43);
INSERT INTO "shop_items" VALUES (1, 480, 44);
INSERT INTO "shop_items" VALUES (1, 481, 45);
INSERT INTO "shop_items" VALUES (1, 402, 32);
INSERT INTO "shop_items" VALUES (1, 590, 49);
INSERT INTO "shop_items" VALUES (1, 685, 51);
INSERT INTO "shop_items" VALUES (1, 752, 53);
INSERT INTO "shop_items" VALUES (1, 753, 54);
INSERT INTO "shop_items" VALUES (1, 770, 57);
INSERT INTO "shop_items" VALUES (1, 771, 58);
INSERT INTO "shop_items" VALUES (1, 591, 50);
INSERT INTO "shop_items" VALUES (1, 686, 52);
INSERT INTO "shop_items" VALUES (1, 756, 55);
INSERT INTO "shop_items" VALUES (1, 757, 56);
INSERT INTO "shop_items" VALUES (1, 316, 18);
INSERT INTO "shop_items" VALUES (1, 315, 17);
INSERT INTO "shop_items" VALUES (1, 332, 20);
INSERT INTO "shop_items" VALUES (1, 331, 19);
INSERT INTO "shop_items" VALUES (1, 522, 48);
INSERT INTO "shop_items" VALUES (1, 520, 47);
INSERT INTO "shop_items" VALUES (2, 385, 0);
INSERT INTO "shop_items" VALUES (2, 386, 1);
INSERT INTO "shop_items" VALUES (2, 117, 60);
INSERT INTO "shop_items" VALUES (2, 235, 10);
INSERT INTO "shop_items" VALUES (2, 1, 11);
INSERT INTO "shop_items" VALUES (2, 4, 12);
INSERT INTO "shop_items" VALUES (2, 8, 13);
INSERT INTO "shop_items" VALUES (2, 9, 14);
INSERT INTO "shop_items" VALUES (2, 12, 15);
INSERT INTO "shop_items" VALUES (2, 13, 16);
INSERT INTO "shop_items" VALUES (2, 15, 17);
INSERT INTO "shop_items" VALUES (2, 16, 18);
INSERT INTO "shop_items" VALUES (2, 109, 2);
INSERT INTO "shop_items" VALUES (2, 23, 21);
INSERT INTO "shop_items" VALUES (2, 24, 22);
INSERT INTO "shop_items" VALUES (2, 17, 19);
INSERT INTO "shop_items" VALUES (2, 18, 20);
INSERT INTO "shop_items" VALUES (2, 25, 23);
INSERT INTO "shop_items" VALUES (2, 26, 24);
INSERT INTO "shop_items" VALUES (2, 28, 25);
INSERT INTO "shop_items" VALUES (2, 29, 26);
INSERT INTO "shop_items" VALUES (2, 38, 31);
INSERT INTO "shop_items" VALUES (2, 39, 32);
INSERT INTO "shop_items" VALUES (2, 42, 33);
INSERT INTO "shop_items" VALUES (2, 43, 34);
INSERT INTO "shop_items" VALUES (2, 46, 35);
INSERT INTO "shop_items" VALUES (2, 47, 36);
INSERT INTO "shop_items" VALUES (2, 50, 37);
INSERT INTO "shop_items" VALUES (2, 51, 38);
INSERT INTO "shop_items" VALUES (2, 672, 88);
INSERT INTO "shop_items" VALUES (2, 54, 39);
INSERT INTO "shop_items" VALUES (2, 55, 40);
INSERT INTO "shop_items" VALUES (2, 673, 89);
INSERT INTO "shop_items" VALUES (2, 615, 85);
INSERT INTO "shop_items" VALUES (2, 844, 106);
INSERT INTO "shop_items" VALUES (2, 110, 3);
INSERT INTO "shop_items" VALUES (2, 31, 27);
INSERT INTO "shop_items" VALUES (2, 32, 28);
INSERT INTO "shop_items" VALUES (2, 34, 29);
INSERT INTO "shop_items" VALUES (2, 35, 30);
INSERT INTO "shop_items" VALUES (2, 671, 87);
INSERT INTO "shop_items" VALUES (2, 112, 4);
INSERT INTO "shop_items" VALUES (2, 59, 41);
INSERT INTO "shop_items" VALUES (2, 60, 42);
INSERT INTO "shop_items" VALUES (2, 62, 43);
INSERT INTO "shop_items" VALUES (2, 63, 44);
INSERT INTO "shop_items" VALUES (2, 68, 45);
INSERT INTO "shop_items" VALUES (2, 69, 46);
INSERT INTO "shop_items" VALUES (2, 71, 47);
INSERT INTO "shop_items" VALUES (2, 72, 48);
INSERT INTO "shop_items" VALUES (2, 674, 90);
INSERT INTO "shop_items" VALUES (2, 560, 79);
INSERT INTO "shop_items" VALUES (2, 580, 80);
INSERT INTO "shop_items" VALUES (2, 237, 5);
INSERT INTO "shop_items" VALUES (2, 760, 103);
INSERT INTO "shop_items" VALUES (2, 761, 104);
INSERT INTO "shop_items" VALUES (2, 843, 105);
INSERT INTO "shop_items" VALUES (2, 75, 49);
INSERT INTO "shop_items" VALUES (2, 76, 50);
INSERT INTO "shop_items" VALUES (2, 617, 86);
INSERT INTO "shop_items" VALUES (2, 873, 107);
INSERT INTO "shop_items" VALUES (2, 874, 108);
INSERT INTO "shop_items" VALUES (2, 108, 7);
INSERT INTO "shop_items" VALUES (2, 79, 52);
INSERT INTO "shop_items" VALUES (2, 80, 53);
INSERT INTO "shop_items" VALUES (2, 81, 54);
INSERT INTO "shop_items" VALUES (2, 82, 55);
INSERT INTO "shop_items" VALUES (2, 83, 56);
INSERT INTO "shop_items" VALUES (2, 84, 57);
INSERT INTO "shop_items" VALUES (2, 85, 58);
INSERT INTO "shop_items" VALUES (2, 258, 65);
INSERT INTO "shop_items" VALUES (2, 257, 64);
INSERT INTO "shop_items" VALUES (2, 256, 63);
INSERT INTO "shop_items" VALUES (2, 230, 9);
INSERT INTO "shop_items" VALUES (2, 231, 61);
INSERT INTO "shop_items" VALUES (2, 232, 62);
INSERT INTO "shop_items" VALUES (2, 454, 66);
INSERT INTO "shop_items" VALUES (2, 681, 95);
INSERT INTO "shop_items" VALUES (2, 687, 97);
INSERT INTO "shop_items" VALUES (2, 457, 68);
INSERT INTO "shop_items" VALUES (2, 456, 67);
INSERT INTO "shop_items" VALUES (2, 458, 69);
INSERT INTO "shop_items" VALUES (2, 675, 91);
INSERT INTO "shop_items" VALUES (2, 461, 70);
INSERT INTO "shop_items" VALUES (2, 462, 71);
INSERT INTO "shop_items" VALUES (2, 677, 93);
INSERT INTO "shop_items" VALUES (2, 472, 72);
INSERT INTO "shop_items" VALUES (2, 682, 96);
INSERT INTO "shop_items" VALUES (2, 688, 98);
INSERT INTO "shop_items" VALUES (2, 475, 73);
INSERT INTO "shop_items" VALUES (2, 477, 75);
INSERT INTO "shop_items" VALUES (2, 476, 74);
INSERT INTO "shop_items" VALUES (2, 478, 76);
INSERT INTO "shop_items" VALUES (2, 676, 92);
INSERT INTO "shop_items" VALUES (2, 482, 77);
INSERT INTO "shop_items" VALUES (2, 483, 78);
INSERT INTO "shop_items" VALUES (2, 678, 94);
INSERT INTO "shop_items" VALUES (2, 600, 81);
INSERT INTO "shop_items" VALUES (2, 750, 99);
INSERT INTO "shop_items" VALUES (2, 751, 100);
INSERT INTO "shop_items" VALUES (2, 602, 83);
INSERT INTO "shop_items" VALUES (2, 754, 101);
INSERT INTO "shop_items" VALUES (2, 755, 102);
INSERT INTO "shop_items" VALUES (2, 77, 51);
INSERT INTO "shop_items" VALUES (2, 107, 6);
INSERT INTO "shop_items" VALUES (2, 601, 82);
INSERT INTO "shop_items" VALUES (2, 603, 84);
INSERT INTO "shop_items" VALUES (1, 429, 33);
INSERT INTO "shop_items" VALUES (2, 250, 8);
INSERT INTO "shop_items" VALUES (2, 87, 59);

-- ----------------------------
-- Table structure for skill_configs
-- ----------------------------
DROP TABLE IF EXISTS "skill_configs";
CREATE TABLE "skill_configs" (
  "skill_id" INTEGER,
  "name" TEXT NOT NULL,
  "skill_type" INTEGER NOT NULL,
  "value1" INTEGER NOT NULL,
  "value2" INTEGER NOT NULL,
  "value3" INTEGER NOT NULL,
  "value4" INTEGER NOT NULL,
  "value5" INTEGER NOT NULL,
  "value6" INTEGER NOT NULL,
  "is_useable" INTEGER NOT NULL DEFAULT 0,
  "use_method" INTEGER NOT NULL DEFAULT 0,
  PRIMARY KEY ("skill_id")
);

-- ----------------------------
-- Records of skill_configs
-- ----------------------------
INSERT INTO "skill_configs" VALUES (0, 'Mining', 1, 1, 6, 0, 0, 0, 0, 0, 0);
INSERT INTO "skill_configs" VALUES (1, 'Fishing', 1, 2, 6, 0, 0, 0, 0, 0, 0);
INSERT INTO "skill_configs" VALUES (2, 'Farming', 1, 3, 6, 0, 0, 0, 0, 0, 0);
INSERT INTO "skill_configs" VALUES (3, 'Magic-Resistance', 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO "skill_configs" VALUES (4, 'Magic', 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO "skill_configs" VALUES (5, 'Hand-Attack', 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO "skill_configs" VALUES (6, 'Archery', 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO "skill_configs" VALUES (7, 'Short-Sword', 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO "skill_configs" VALUES (8, 'Long-Sword', 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO "skill_configs" VALUES (9, 'Fencing', 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO "skill_configs" VALUES (10, 'Axe-Attack', 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO "skill_configs" VALUES (11, 'Shield', 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO "skill_configs" VALUES (12, 'Alchemy', 1, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO "skill_configs" VALUES (13, 'Manufacturing', 1, 2, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO "skill_configs" VALUES (14, 'Hammer', 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO "skill_configs" VALUES (15, '????', 1, 1, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO "skill_configs" VALUES (16, '????', 1, 1, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO "skill_configs" VALUES (17, '????', 1, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO "skill_configs" VALUES (18, '????', 1, 1, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO "skill_configs" VALUES (19, 'Pretend-Corpse', 2, 1, 0, 0, 0, 0, 0, 1, 1);
INSERT INTO "skill_configs" VALUES (20, '????', 1, 1, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO "skill_configs" VALUES (21, 'Staff-Attack', 0, 0, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO "skill_configs" VALUES (22, '????', 1, 2, 0, 0, 0, 0, 0, 0, 0);
INSERT INTO "skill_configs" VALUES (23, 'Poison-Resistance', 1, 0, 0, 0, 0, 0, 0, 0, 0);

-- ----------------------------
-- Table structure for sqlite_sequence
-- ----------------------------
DROP TABLE IF EXISTS "sqlite_sequence";
CREATE TABLE "sqlite_sequence" (
  "name",
  "seq"
);

-- ----------------------------
-- Records of sqlite_sequence
-- ----------------------------
INSERT INTO "sqlite_sequence" VALUES ('event_schedule', 4);

-- ----------------------------
-- Auto increment value for event_schedule
-- ----------------------------
UPDATE "sqlite_sequence" SET seq = 4 WHERE name = 'event_schedule';

PRAGMA foreign_keys = true;
