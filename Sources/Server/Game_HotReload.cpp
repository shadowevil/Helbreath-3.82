// Game_HotReload.cpp: Hot Reload System Implementation
//
// This file contains all hot reload functionality for CGame.
// Allows reloading configurations from GameConfigs.db without server restart.
//
//////////////////////////////////////////////////////////////////////

#include "Game.h"
#include "GameConfigSqliteStore.h"
#include "Item.h"
#include "Magic.h"
#include "Map.h"
#include "Skill.h"
#include "Tile.h"
#include "sqlite3.h"

extern void PutLogList(char *cMsg);
extern char G_cTxt[512];

//==============================================================================
// ITEM HOT RELOAD
//==============================================================================

//------------------------------------------------------------------------------
// Update an existing item instance from config
// Preserves instance data: m_dwCount, m_wCurLifeSpan, m_dwAttribute, m_sX, m_sY
//------------------------------------------------------------------------------
void CGame::UpdateExistingItemFromConfig(CItem *pItem)
{
  if (pItem == nullptr)
    return;

  int iItemID = pItem->m_sIDnum;
  if (iItemID < 0 || iItemID >= DEF_MAXITEMTYPES)
    return;
  if (m_pItemConfigList[iItemID] == nullptr)
    return;

  CItem *pConfig = m_pItemConfigList[iItemID];

  // Update base properties from config (preserve instance data)
  pItem->m_cItemType = pConfig->m_cItemType;
  pItem->m_cEquipPos = pConfig->m_cEquipPos;
  pItem->m_sItemEffectType = pConfig->m_sItemEffectType;
  pItem->m_sItemEffectValue1 = pConfig->m_sItemEffectValue1;
  pItem->m_sItemEffectValue2 = pConfig->m_sItemEffectValue2;
  pItem->m_sItemEffectValue3 = pConfig->m_sItemEffectValue3;
  pItem->m_sItemEffectValue4 = pConfig->m_sItemEffectValue4;
  pItem->m_sItemEffectValue5 = pConfig->m_sItemEffectValue5;
  pItem->m_sItemEffectValue6 = pConfig->m_sItemEffectValue6;
  pItem->m_wMaxLifeSpan = pConfig->m_wMaxLifeSpan;
  pItem->m_sSpecialEffect = pConfig->m_sSpecialEffect;
  pItem->m_sSpecialEffectValue1 = pConfig->m_sSpecialEffectValue1;
  pItem->m_sSpecialEffectValue2 = pConfig->m_sSpecialEffectValue2;
  pItem->m_sSprite = pConfig->m_sSprite;
  pItem->m_sSpriteFrame = pConfig->m_sSpriteFrame;
  pItem->m_wPrice = pConfig->m_wPrice;
  pItem->m_wWeight = pConfig->m_wWeight;
  pItem->m_cApprValue = pConfig->m_cApprValue;
  pItem->m_cSpeed = pConfig->m_cSpeed;
  pItem->m_sLevelLimit = pConfig->m_sLevelLimit;
  pItem->m_cGenderLimit = pConfig->m_cGenderLimit;
  pItem->m_sRelatedSkill = pConfig->m_sRelatedSkill;
  pItem->m_cCategory = pConfig->m_cCategory;
  pItem->m_bIsForSale = pConfig->m_bIsForSale;

  // Note: Preserved instance data (NOT updated):
  // - m_dwCount (stack count)
  // - m_wCurLifeSpan (current durability)
  // - m_dwAttribute (item enchantments/upgrades)
  // - m_sX, m_sY (inventory position)
  // - m_cItemColor (may be instance-specific)
}

//------------------------------------------------------------------------------
// Update all existing items in the game world
//------------------------------------------------------------------------------
void CGame::UpdateAllExistingItems()
{
  int iUpdatedCount = 0;
  int iNotifiedCount = 0;
  int iRecalcCount = 0;

  // 1. Update items in player inventories and banks
  for (int iClientH = 1; iClientH < DEF_MAXCLIENTS; iClientH++)
  {
    if (m_pClientList[iClientH] == nullptr)
      continue;

    bool bClientConnected = m_pClientList[iClientH]->m_bIsInitComplete;

    // Update inventory items
    for (int i = 0; i < DEF_MAXITEMS; i++)
    {
      if (m_pClientList[iClientH]->m_pItemList[i] != nullptr)
      {
        UpdateExistingItemFromConfig(m_pClientList[iClientH]->m_pItemList[i]);
        iUpdatedCount++;

        // Notify client about the updated max_lifespan
        if (bClientConnected)
        {
          SendNotifyMsg(0, iClientH, DEF_NOTIFY_MAXLIFESPAN, i,
                        m_pClientList[iClientH]->m_pItemList[i]->m_wMaxLifeSpan, 0, 0);
          iNotifiedCount++;
        }
      }
    }

    // Update bank items (no notification needed for bank items as they're not displayed)
    for (int i = 0; i < DEF_MAXBANKITEMS; i++)
    {
      if (m_pClientList[iClientH]->m_pItemInBankList[i] != nullptr)
      {
        UpdateExistingItemFromConfig(m_pClientList[iClientH]->m_pItemInBankList[i]);
        iUpdatedCount++;
      }
    }

    // Recalculate player stats based on equipped items (for damage, defense, etc.)
    if (bClientConnected)
    {
      CalcTotalItemEffect(iClientH, -1, true);
      iRecalcCount++;
    }
  }

  // 2. Update items on the ground (in map tiles)
  for (int iMapIndex = 0; iMapIndex < DEF_MAXMAPS; iMapIndex++)
  {
    if (m_pMapList[iMapIndex] == nullptr)
      continue;

    CMap *pMap = m_pMapList[iMapIndex];
    if (pMap->m_pTile == nullptr)
      continue;

    int iTotalTiles = pMap->m_sSizeX * pMap->m_sSizeY;
    for (int iTile = 0; iTile < iTotalTiles; iTile++)
    {
      CTile *pTile = &pMap->m_pTile[iTile];
      for (int iItem = 0; iItem < DEF_TILE_PER_ITEMS; iItem++)
      {
        if (pTile->m_pItem[iItem] != nullptr)
        {
          UpdateExistingItemFromConfig(pTile->m_pItem[iItem]);
          iUpdatedCount++;
        }
      }
    }
  }

  std::snprintf(G_cTxt, sizeof(G_cTxt), "(!) Hot Reload: Updated %d items, sent %d notifications, recalculated stats for %d players",
                iUpdatedCount, iNotifiedCount, iRecalcCount);
  PutLogList(G_cTxt);
}

//------------------------------------------------------------------------------
// Broadcast new item configs to all connected clients
//------------------------------------------------------------------------------
void CGame::BroadcastItemConfigsToAllClients()
{
  int iClientCount = 0;

  for (int iClientH = 1; iClientH < DEF_MAXCLIENTS; iClientH++)
  {
    if (m_pClientList[iClientH] != nullptr &&
        m_pClientList[iClientH]->m_bIsInitComplete)
    {
      bSendClientItemConfigs(iClientH);
      iClientCount++;
    }
  }

  std::snprintf(G_cTxt, sizeof(G_cTxt), "(!) Hot Reload: Sent item configs to %d clients", iClientCount);
  PutLogList(G_cTxt);
}

//------------------------------------------------------------------------------
// Main item reload function
//------------------------------------------------------------------------------
bool CGame::ReloadItemConfigs()
{
  PutLogList("(!) Hot Reload: Reloading item configurations...");

  // Open the configuration database
  sqlite3 *configDb = nullptr;
  std::string configDbPath;
  bool configDbCreated = false;

  if (!EnsureGameConfigDatabase(&configDb, configDbPath, &configDbCreated) ||
      configDbCreated)
  {
    PutLogList("(!!!) Hot Reload FAILED: GameConfigs.db unavailable");
    return false;
  }

  if (!HasGameConfigRows(configDb, "items"))
  {
    PutLogList("(!!!) Hot Reload FAILED: No items table in GameConfigs.db");
    CloseGameConfigDatabase(configDb);
    return false;
  }

  // Create temporary array for new configs
  CItem *pNewConfigList[DEF_MAXITEMTYPES];
  for (int i = 0; i < DEF_MAXITEMTYPES; i++)
  {
    pNewConfigList[i] = nullptr;
  }

  // Load new configurations
  if (!LoadItemConfigs(configDb, pNewConfigList, DEF_MAXITEMTYPES))
  {
    PutLogList("(!!!) Hot Reload FAILED: Could not load items from database");
    // Clean up any partially loaded items
    for (int i = 0; i < DEF_MAXITEMTYPES; i++)
    {
      if (pNewConfigList[i] != nullptr)
      {
        delete pNewConfigList[i];
      }
    }
    CloseGameConfigDatabase(configDb);
    return false;
  }

  CloseGameConfigDatabase(configDb);

  // Replace old configs with new ones
  for (int i = 0; i < DEF_MAXITEMTYPES; i++)
  {
    if (m_pItemConfigList[i] != nullptr)
    {
      delete m_pItemConfigList[i];
    }
    m_pItemConfigList[i] = pNewConfigList[i];
  }

  // Update all existing items in the world
  UpdateAllExistingItems();

  // Broadcast new configs to all connected clients
  BroadcastItemConfigsToAllClients();

  PutLogList("(!) Hot Reload: Item configurations reloaded successfully!");
  return true;
}

//==============================================================================
// MAGIC HOT RELOAD
//==============================================================================

bool CGame::ReloadMagicConfigs()
{
  PutLogList("(!) Hot Reload: Reloading magic configurations...");

  sqlite3 *configDb = nullptr;
  std::string configDbPath;
  bool configDbCreated = false;

  if (!EnsureGameConfigDatabase(&configDb, configDbPath, &configDbCreated) ||
      configDbCreated)
  {
    PutLogList("(!!!) Hot Reload FAILED: GameConfigs.db unavailable");
    return false;
  }

  if (!HasGameConfigRows(configDb, "magic_configs"))
  {
    PutLogList("(!!!) Hot Reload FAILED: No magic_configs table in GameConfigs.db");
    CloseGameConfigDatabase(configDb);
    return false;
  }

  // Clear existing magic configs
  for (int i = 0; i < DEF_MAXMAGICTYPE; i++)
  {
    if (m_pMagicConfigList[i] != nullptr)
    {
      delete m_pMagicConfigList[i];
      m_pMagicConfigList[i] = nullptr;
    }
  }

  // Reload magic configs
  if (!LoadMagicConfigs(configDb, this))
  {
    PutLogList("(!!!) Hot Reload FAILED: Could not load magic from database");
    CloseGameConfigDatabase(configDb);
    return false;
  }

  CloseGameConfigDatabase(configDb);

  PutLogList("(!) Hot Reload: Magic configurations reloaded successfully!");
  return true;
}

//==============================================================================
// SKILL HOT RELOAD
//==============================================================================

bool CGame::ReloadSkillConfigs()
{
  PutLogList("(!) Hot Reload: Reloading skill configurations...");

  sqlite3 *configDb = nullptr;
  std::string configDbPath;
  bool configDbCreated = false;

  if (!EnsureGameConfigDatabase(&configDb, configDbPath, &configDbCreated) ||
      configDbCreated)
  {
    PutLogList("(!!!) Hot Reload FAILED: GameConfigs.db unavailable");
    return false;
  }

  if (!HasGameConfigRows(configDb, "skill_configs"))
  {
    PutLogList("(!!!) Hot Reload FAILED: No skill_configs table in GameConfigs.db");
    CloseGameConfigDatabase(configDb);
    return false;
  }

  // Clear existing skill configs
  for (int i = 0; i < DEF_MAXSKILLTYPE; i++)
  {
    if (m_pSkillConfigList[i] != nullptr)
    {
      delete m_pSkillConfigList[i];
      m_pSkillConfigList[i] = nullptr;
    }
  }

  // Reload skill configs
  if (!LoadSkillConfigs(configDb, this))
  {
    PutLogList("(!!!) Hot Reload FAILED: Could not load skills from database");
    CloseGameConfigDatabase(configDb);
    return false;
  }

  CloseGameConfigDatabase(configDb);

  PutLogList("(!) Hot Reload: Skill configurations reloaded successfully!");
  return true;
}

//==============================================================================
// SETTINGS HOT RELOAD
//==============================================================================

bool CGame::ReloadSettingsConfigs()
{
  PutLogList("(!) Hot Reload: Reloading settings configurations...");

  sqlite3 *configDb = nullptr;
  std::string configDbPath;
  bool configDbCreated = false;

  if (!EnsureGameConfigDatabase(&configDb, configDbPath, &configDbCreated) ||
      configDbCreated)
  {
    PutLogList("(!!!) Hot Reload FAILED: GameConfigs.db unavailable");
    return false;
  }

  if (!HasGameConfigRows(configDb, "settings"))
  {
    PutLogList("(!!!) Hot Reload FAILED: No settings table in GameConfigs.db");
    CloseGameConfigDatabase(configDb);
    return false;
  }

  // Reload settings
  if (!LoadSettingsConfig(configDb, this))
  {
    PutLogList("(!!!) Hot Reload FAILED: Could not load settings from database");
    CloseGameConfigDatabase(configDb);
    return false;
  }

  CloseGameConfigDatabase(configDb);

  std::snprintf(G_cTxt, sizeof(G_cTxt),
                "(!) Hot Reload: Settings reloaded - DropRate: %d/%d, MaxLevel: %d",
                m_iPrimaryDropRate, m_iSecondaryDropRate, m_iMaxLevel);
  PutLogList(G_cTxt);

  return true;
}

//==============================================================================
// DROP TABLES HOT RELOAD
//==============================================================================

bool CGame::ReloadDropTables()
{
  PutLogList("(!) Hot Reload: Reloading drop tables...");

  sqlite3 *configDb = nullptr;
  std::string configDbPath;
  bool configDbCreated = false;

  if (!EnsureGameConfigDatabase(&configDb, configDbPath, &configDbCreated) ||
      configDbCreated)
  {
    PutLogList("(!!!) Hot Reload FAILED: GameConfigs.db unavailable");
    return false;
  }

  if (!HasGameConfigRows(configDb, "drop_tables") ||
      !HasGameConfigRows(configDb, "drop_entries"))
  {
    PutLogList("(!!!) Hot Reload FAILED: No drop tables in GameConfigs.db");
    CloseGameConfigDatabase(configDb);
    return false;
  }

  // Clear existing drop tables
  m_DropTables.clear();

  // Reload drop tables
  if (!LoadDropTables(configDb, this))
  {
    PutLogList("(!!!) Hot Reload FAILED: Could not load drop tables from database");
    CloseGameConfigDatabase(configDb);
    return false;
  }

  CloseGameConfigDatabase(configDb);

  std::snprintf(G_cTxt, sizeof(G_cTxt),
                "(!) Hot Reload: Drop tables reloaded - %zu tables loaded",
                m_DropTables.size());
  PutLogList(G_cTxt);

  return true;
}

//==============================================================================
// RELOAD ALL CONFIGURATIONS
//==============================================================================

bool CGame::ReloadAllConfigs()
{
  PutLogList("(!) Hot Reload: Reloading ALL configurations...");

  bool bSuccess = true;
  int iSuccessCount = 0;
  int iTotalCount = 5;

  if (ReloadSettingsConfigs())
    iSuccessCount++;
  else
    bSuccess = false;

  if (ReloadItemConfigs())
    iSuccessCount++;
  else
    bSuccess = false;

  if (ReloadMagicConfigs())
    iSuccessCount++;
  else
    bSuccess = false;

  if (ReloadSkillConfigs())
    iSuccessCount++;
  else
    bSuccess = false;

  if (ReloadDropTables())
    iSuccessCount++;
  else
    bSuccess = false;

  std::snprintf(G_cTxt, sizeof(G_cTxt),
                "(!) Hot Reload: Completed %d/%d configurations successfully",
                iSuccessCount, iTotalCount);
  PutLogList(G_cTxt);

  return bSuccess;
}
