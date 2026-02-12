// MapData.h: interface for the CMapData class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdio>
#include <stdlib.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "CommonTypes.h"
#include "Tile.h"
#include "ActionID.h"
#include "Game.h"
#include "TileSpr.h"
#include "ActionID_Client.h"
#include "ObjectIDRange.h"
#include <string>

namespace hb::client::config
{
constexpr int MapDataSizeX = 60;
constexpr int MapDataSizeY = 55;
} // namespace hb::client::config

class CMapData
{
public:
	CMapData(class CGame * pGame);
	virtual ~CMapData();
	void Init();
	void OpenMapDataFile(char * cFn);
	void GetOwnerStatusByObjectID(uint16_t wObjectID, char * pOwnerType, char * pDir, hb::shared::entity::PlayerAppearance * pAppearance, hb::shared::entity::PlayerStatus * pStatus, std::string& pName);
	void ClearDeadChatMsg(short sX, short sY);
	void ClearChatMsg(short sX, short sY);
	void ShiftMapData(char cDir);
	void _bDecodeMapInfo(char * pHeader);
	bool __fastcall bSetChatMsgOwner(uint16_t wObjectID, short sX, short sY, int iIndex);
	bool __fastcall bSetDeadOwner(uint16_t wObjectID, short sX, short sY, short sType, char cDir, const hb::shared::entity::PlayerAppearance& appearance, const hb::shared::entity::PlayerStatus& status, std::string& pName, short npcConfigId = -1);
	bool __fastcall bSetOwner(uint16_t wObjectID, int sX, int sY, int sType, int cDir, const hb::shared::entity::PlayerAppearance& appearance, const hb::shared::entity::PlayerStatus& status, std::string& pName, short sAction, short sV1, short sV2, short sV3, int iPreLoc = 0, int iFrame = 0, short npcConfigId = -1);
	bool __fastcall bGetOwner(short sX, short sY, std::string& pName, short * pOwnerType, hb::shared::entity::PlayerStatus * pOwnerStatus, uint16_t * pObjectID);
	bool bSetDynamicObject(short sX, short sY, uint16_t wID, short sType, bool bIsEvent);
	bool bIsTeleportLoc(short sX, short sY);
	bool bGetIsLocateable(short sX, short sY);
	bool bSetItem(short sX, short sY, short sIDnum, char cItemColor, uint32_t dwItemAttr, bool bDropEffect = true);
	int  iObjectFrameCounter(const std::string& cPlayerName, short sViewPointX, short sViewPointY);

	class CTile m_pData[hb::client::config::MapDataSizeX][hb::client::config::MapDataSizeY];
	class CTile m_pTmpData[hb::client::config::MapDataSizeX][hb::client::config::MapDataSizeY];
	class CTileSpr m_tile[752][752];
	class CGame * m_pGame;

	struct {
		short m_sMaxFrame;
		short m_sFrameTime;
	} m_stFrame[hb::client::config::TotalCharacters][hb::client::config::TotalAction];
	uint32_t m_dwFrameTime;
	uint32_t m_dwDOframeTime;
	uint32_t m_dwFrameCheckTime;
	int m_iObjectIDcacheLocX[hb::shared::object_id::NpcMax];
	int m_iObjectIDcacheLocY[hb::shared::object_id::NpcMax];
	uint32_t m_dwFrameAdjustTime;
	short m_sMapSizeX, m_sMapSizeY;
	short m_sRectX, m_sRectY;
	short m_sPivotX, m_sPivotY;
};
