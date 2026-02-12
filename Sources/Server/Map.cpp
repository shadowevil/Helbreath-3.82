// Map.cpp: implementation of the CMap class.

#include "CommonTypes.h"
#include "Map.h"
using namespace hb::server::map;
using namespace hb::server::config;

extern void PutLogFileList(char* cStr);

// Construction/Destruction

CMap::CMap(class CGame* pGame)
	: m_bIsSnowEnabled(false)
{
	for(int i = 0; i < MaxTeleportLoc; i++)
		m_pTeleportLoc[i] = 0;

	for(int i = 0; i < MaxWaypointCfg; i++) {
		m_WaypointList[i].x = -1;
		m_WaypointList[i].y = -1;
	}

	for(int i = 0; i < MaxMgar; i++) {
		m_rcMobGenAvoidRect[i].y = -1;
		m_rcMobGenAvoidRect[i].x = -1;
	}

	for(int i = 0; i < MaxNmr; i++) {
		m_rcNoAttackRect[i].y = -1;
		m_rcNoAttackRect[i].x = -1;
	}

	for(int i = 0; i < MaxSpotMobGenerator; i++) {
		m_stSpotMobGenerator[i].bDefined = false;
		m_stSpotMobGenerator[i].iTotalActiveMob = 0;
	}

	for(int i = 0; i < MaxFishPoint; i++) {
		m_FishPointList[i].x = -1;
		m_FishPointList[i].y = -1;
	}

	for(int i = 0; i < MaxMineralPoint; i++) {
		m_MineralPointList[i].x = -1;
		m_MineralPointList[i].y = -1;
	}

	for(int i = 0; i < MaxInitialPoint; i++) {
		m_pInitialPoint[i].x = -1;
		m_pInitialPoint[i].y = -1;
	}

	for(int i = 0; i < 1000; i++)
		m_bNamingValueUsingStatus[i] = false;

	for(int i = 0; i < MaxOccupyFlag; i++)
		m_pOccupyFlag[i] = 0;

	for(int i = 0; i < MaxStrategicPoints; i++)
		m_pStrategicPointList[i] = 0;

	for(int i = 0; i < MaxEnergySpheres; i++) {
		m_stEnergySphereCreationList[i].cType = 0;
		m_stEnergySphereGoalList[i].cResult = 0;
	}

	m_bIsHeldenianMap = false;
	m_iTotalActiveObject = 0;
	m_iTotalAliveObject = 0;
	m_iMaximumObject = 1000;  // Default max objects per map (can be overridden by config)
	m_iTotalItemEvents = 0;
	sMobEventAmount = 15;
	//m_sInitialPointX = 0;
	//m_sInitialPointY = 0;

	m_bIsFixedDayMode = false;

	m_iTotalFishPoint = 0;
	m_iMaxFish = 0;
	m_iCurFish = 0;

	m_iTotalMineralPoint = 0;
	m_iMaxMineral = 0;
	m_iCurMineral = 0;

	m_pTile = 0;

	m_cWhetherStatus = 0;
	m_cType = MapType::Normal;

	m_pGame = pGame;

	m_iLevelLimit = 0;
	m_iUpperLevelLimit = 0; // v1.4
	m_bMineralGenerator = false;

	m_iTotalOccupyFlags = 0;

	m_bIsAttackEnabled = true;
	m_cRandomMobGeneratorLevel = 0;

	m_bIsFightZone = false;

	m_iTotalEnergySphereCreationPoint = 0;
	m_iTotalEnergySphereGoalPoint = 0;

	m_bIsEnergySphereGoalEnabled = false;
	m_iCurEnergySphereGoalPointIndex = -1;

	for(int ix = 0; ix < MaxSectors; ix++)
		for(int iy = 0; iy < MaxSectors; iy++) {
			m_stSectorInfo[ix][iy].iNeutralActivity = 0;
			m_stSectorInfo[ix][iy].iAresdenActivity = 0;
			m_stSectorInfo[ix][iy].iElvineActivity = 0;
			m_stSectorInfo[ix][iy].iMonsterActivity = 0;
			m_stSectorInfo[ix][iy].iPlayerActivity = 0;

			m_stTempSectorInfo[ix][iy].iNeutralActivity = 0;
			m_stTempSectorInfo[ix][iy].iAresdenActivity = 0;
			m_stTempSectorInfo[ix][iy].iElvineActivity = 0;
			m_stTempSectorInfo[ix][iy].iMonsterActivity = 0;
			m_stTempSectorInfo[ix][iy].iPlayerActivity = 0;
		}

	m_iMaxNx = m_iMaxNy = m_iMaxAx = m_iMaxAy = m_iMaxEx = m_iMaxEy = m_iMaxMx = m_iMaxMy = m_iMaxPx = m_iMaxPy = 0;

	for(int i = 0; i < MaxHeldenianDoor; i++) {
		m_stHeldenianGateDoor[i].cDir = 0;
		m_stHeldenianGateDoor[i].dX = 0;
		m_stHeldenianGateDoor[i].dY = 0;
	}

	for(int i = 0; i < MaxHeldenianTower; i++) {
		m_stHeldenianTower[i].sTypeID = 0;
		m_stHeldenianTower[i].dX = 0;
		m_stHeldenianTower[i].dY = 0;
		m_stHeldenianTower[i].cSide = 0;
	}

	for(int i = 0; i < MaxStrikePoints; i++) {
		m_stStrikePoint[i].dX = 0;
		m_stStrikePoint[i].dY = 0;
		m_stStrikePoint[i].iHP = 0;
		m_stStrikePoint[i].iMapIndex = -1;
		std::memset(m_stStrikePoint[i].cRelatedMapName, 0, sizeof(m_stStrikePoint[i].cRelatedMapName));
	}
	m_iTotalStrikePoints = 0;
	m_bIsDisabled = false;

	for(int i = 0; i < hb::shared::limits::MaxCrusadeStructures; i++) {
		m_stCrusadeStructureInfo[i].cType = 0;
		m_stCrusadeStructureInfo[i].cSide = 0;
		m_stCrusadeStructureInfo[i].sX = 0;
		m_stCrusadeStructureInfo[i].sY = 0;
	}
	m_iTotalCrusadeStructures = 0;
	m_iTotalAgriculture = 0;

	// Initialize boolean members that may not be set in map config files
	m_bRandomMobGenerator = false;
	m_bIsCitizenLimit = false;
	m_bIsEnergySphereAutoCreation = false;
	m_bIsRecallImpossible = false;
	m_bIsApocalypseMap = false;

	for(int i = 0; i < MaxDynamicGates; i++) {
		m_stDynamicGateCoords[i].m_bIsGateMap = false;
		m_stDynamicGateCoords[i].m_iDynamicGateX = 0;
		m_stDynamicGateCoords[i].m_iDynamicGateY = 0;
		std::memset(m_stDynamicGateCoords[i].m_cDynamicGateMap, 0, sizeof(m_stDynamicGateCoords[i].m_cDynamicGateMap));
	}
}

CMap::~CMap()
{

	

	if (m_pTile != 0)
		delete[]m_pTile;

	for(int i = 0; i < MaxTeleportLoc; i++)
		if (m_pTeleportLoc[i] != 0) delete m_pTeleportLoc[i];

	for(int i = 0; i < MaxOccupyFlag; i++)
		if (m_pOccupyFlag[i] != 0) delete m_pOccupyFlag[i];

	for(int i = 0; i < MaxStrategicPoints; i++)
		if (m_pStrategicPointList[i] != 0) delete m_pStrategicPointList[i];
}

void CMap::SetOwner(short sOwner, char cOwnerClass, short sX, short sY)
{
	class CTile* pTile;

	if ((sX < 0) || (sX >= m_sSizeX) || (sY < 0) || (sY >= m_sSizeY)) return;

	pTile = (class CTile*)(m_pTile + sX + sY * m_sSizeX);
	pTile->m_sOwner = sOwner;
	pTile->m_cOwnerClass = cOwnerClass;
}

char _tmp_cMoveDirX[9] = { 0,0,1,1,1,0,-1,-1,-1 };
char _tmp_cMoveDirY[9] = { 0,-1,-1,0,1,1,1,0,-1 };
bool CMap::bCheckFlySpaceAvailable(short sX, char sY, char cDir, short sOwner)
{
	class CTile* pTile;
	short dX, dY;

	if ((cDir <= 0) || (cDir > 8)) return 0;
	dX = _tmp_cMoveDirX[cDir] + sX;
	dY = _tmp_cMoveDirY[cDir] + sY;
	if ((dX < 20) || (dX >= m_sSizeX - 20) || (dY < 20) || (dY >= m_sSizeY - 20)) return 0;
	pTile = (class CTile*)(m_pTile + sX + sY * m_sSizeX);
	if (pTile->m_sOwner != 0) return 0;
	pTile->m_sOwner = sOwner;
	return 1;
}

void CMap::SetDeadOwner(short sOwner, char cOwnerClass, short sX, short sY)
{
	class CTile* pTile;

	if ((sX < 0) || (sX >= m_sSizeX) || (sY < 0) || (sY >= m_sSizeY)) return;

	pTile = (class CTile*)(m_pTile + sX + sY * m_sSizeX);
	pTile->m_sDeadOwner = sOwner;
	pTile->m_cDeadOwnerClass = cOwnerClass;
}


/*********************************************************************************************************************
**  void CMap::GetOwner(short * pOwner, char * pOwnerClass, short sX, short sY)										**
**  description			:: check if the tile contains a player														**
**  last updated		:: November 17, 2004; 10:48 PM; Hypnotoad													**
**	return value		:: void																						**
**  commentary			::	-	added check to see if owner is class 1 or if is greater than max clients 			**
**********************************************************************************************************************/
void CMap::GetOwner(short* pOwner, char* pOwnerClass, short sX, short sY)
{
	class CTile* pTile;

	if ((sX < 0) || (sX >= m_sSizeX) || (sY < 0) || (sY >= m_sSizeY)) {
		*pOwner = 0;
		*pOwnerClass = 0;
		return;
	}

	pTile = (class CTile*)(m_pTile + sX + sY * m_sSizeX);
	*pOwner = pTile->m_sOwner;
	*pOwnerClass = pTile->m_cOwnerClass;

	if ((*pOwnerClass == 1) && (*pOwner > MaxClients)) {
		*pOwner = 0;
		*pOwnerClass = 0;
		return;
	}

	if (pTile->m_sOwner == 0) *pOwnerClass = 0;
}

/*********************************************************************************************************************
**  void CMap::GetDeadOwner(short * pOwner, char * pOwnerClass, short sX, short sY)									**
**  description			:: check if the tile contains a dead player													**
**  last updated		:: November 20, 2004; 9:13 PM; Hypnotoad													**
**	return value		:: void																						**
**********************************************************************************************************************/
void CMap::GetDeadOwner(short* pOwner, char* pOwnerClass, short sX, short sY)
{
	class CTile* pTile;

	if ((sX < 0) || (sX >= m_sSizeX) || (sY < 0) || (sY >= m_sSizeY)) {
		*pOwner = 0;
		*pOwnerClass = 0;
		return;
	}

	pTile = (class CTile*)(m_pTile + sX + sY * m_sSizeX);
	*pOwner = pTile->m_sDeadOwner;
	*pOwnerClass = pTile->m_cDeadOwnerClass;
}


bool CMap::bGetMoveable(short dX, short dY, short* pDOtype, short* pTopItem)
{
	class CTile* pTile;

	if ((dX < 20) || (dX >= m_sSizeX - 20) || (dY < 20) || (dY >= m_sSizeY - 20)) return false;
	pTile = (class CTile*)(m_pTile + dX + dY * m_sSizeX);

	if (pDOtype != 0) *pDOtype = pTile->m_sDynamicObjectType;
	if (pTopItem != 0) *pTopItem = pTile->m_cTotalItem;

	if (pTile->m_sOwner != 0) return false;
	if (pTile->m_bIsMoveAllowed == false) return false;
	if (pTile->m_bIsTempMoveAllowed == false) return false;

	return true;
}

bool CMap::bGetIsMoveAllowedTile(short dX, short dY)
{
	class CTile* pTile;

	if ((dX < 20) || (dX >= m_sSizeX - 20) || (dY < 20) || (dY >= m_sSizeY - 20)) return false;

	pTile = (class CTile*)(m_pTile + dX + dY * m_sSizeX);

	if (pTile->m_bIsMoveAllowed == false) return false;
	if (pTile->m_bIsTempMoveAllowed == false) return false;

	return true;
}

/*bool CMap::sub_4C0F20(short dX, short dY)
{
 class CTile * pTile;

	3CA18h = 0;

	if ((dX < 14) || (dX >= m_sSizeX - 16) || (dY < 12) || (dY >= m_sSizeY - 14)) return false;

	pTile = (class CTile *)(m_pTile + dX + dY*m_sSizeY);

}*/

bool CMap::bGetIsTeleport(short dX, short dY)
{
	class CTile* pTile;

	if ((dX < 14) || (dX >= m_sSizeX - 16) || (dY < 12) || (dY >= m_sSizeY - 14)) return false;

	pTile = (class CTile*)(m_pTile + dX + dY * m_sSizeX);

	if (pTile->m_bIsTeleport == false) return false;

	return true;
}

void CMap::ClearOwner(int iDebugCode, short sOwnerH, char cOwnerType, short sX, short sY)
{
	class CTile* pTile;

	if ((sX < 0) || (sX >= m_sSizeX) || (sY < 0) || (sY >= m_sSizeY)) return;

	pTile = (class CTile*)(m_pTile + sX + sY * m_sSizeX);

	if ((pTile->m_sOwner == sOwnerH) && (pTile->m_cOwnerClass == cOwnerType)) {
		pTile->m_sOwner = 0;
		pTile->m_cOwnerClass = 0;
	}

	if ((pTile->m_sDeadOwner == sOwnerH) && (pTile->m_cDeadOwnerClass == cOwnerType)) {
		pTile->m_sDeadOwner = 0;
		pTile->m_cDeadOwnerClass = 0;
	}
}

void CMap::ClearDeadOwner(short sX, short sY)
{
	class CTile* pTile;

	if ((sX < 0) || (sX >= m_sSizeX) || (sY < 0) || (sY >= m_sSizeY)) return;

	pTile = (class CTile*)(m_pTile + sX + sY * m_sSizeX);
	pTile->m_sDeadOwner = 0;
	pTile->m_cDeadOwnerClass = 0;
}

bool CMap::bSetItem(short sX, short sY, CItem* pItem)
{
	class CTile* pTile;
	

	if ((sX < 0) || (sX >= m_sSizeX) || (sY < 0) || (sY >= m_sSizeY)) return 0;

	pTile = (class CTile*)(m_pTile + sX + sY * m_sSizeX);

	if (pTile->m_pItem[TilePerItems - 1] != 0)
		delete pTile->m_pItem[TilePerItems - 1];
	else pTile->m_cTotalItem++;

	for(int i = TilePerItems - 2; i >= 0; i--)
		pTile->m_pItem[i + 1] = pTile->m_pItem[i];

	pTile->m_pItem[0] = pItem;
	//pTile->m_cTotalItem++;
	return true;
}


CItem* CMap::pGetItem(short sX, short sY, short* pRemainItemID, char* pRemainItemColor, uint32_t* pRemainItemAttr) //v1.4 color
{
	class CTile* pTile;
	CItem* pItem;
	

	if ((sX < 0) || (sX >= m_sSizeX) || (sY < 0) || (sY >= m_sSizeY)) return 0;

	pTile = (class CTile*)(m_pTile + sX + sY * m_sSizeX);
	pItem = pTile->m_pItem[0];
	if (pTile->m_cTotalItem == 0) return 0;

	for(int i = 0; i <= TilePerItems - 2; i++)
		pTile->m_pItem[i] = pTile->m_pItem[i + 1];
	pTile->m_cTotalItem--;
	pTile->m_pItem[pTile->m_cTotalItem] = 0;

	if (pTile->m_pItem[0] == 0) {
		*pRemainItemID = 0;
		*pRemainItemColor = 0;
		*pRemainItemAttr = 0;
	}
	else
	{
		*pRemainItemID = pTile->m_pItem[0]->m_sIDnum;
		*pRemainItemColor = pTile->m_pItem[0]->m_cItemColor;
		*pRemainItemAttr = pTile->m_pItem[0]->m_dwAttribute;
	}

	return pItem;
}


int CMap::iCheckItem(short sX, short sY)
{
	class CTile* pTile;
	CItem* pItem;

	if ((sX < 0) || (sX >= m_sSizeX) || (sY < 0) || (sY >= m_sSizeY)) return 0;

	pTile = (class CTile*)(m_pTile + sX + sY * m_sSizeX);
	pItem = pTile->m_pItem[0];
	if (pTile->m_cTotalItem == 0) return 0;

	return pItem->m_sIDnum;
}


bool CMap::bIsValidLoc(short sX, short sY)
{
	if ((sX < 0) || (sX >= m_sSizeX) || (sY < 0) || (sY >= m_sSizeY)) return false;
	return true;
}

bool CMap::bInit(char* pName)
{
	
	std::memset(m_cName, 0, sizeof(m_cName));
	strcpy(m_cName, pName);

	std::memset(m_cLocationName, 0, sizeof(m_cLocationName));

	if (_bDecodeMapDataFileContents() == false)
		return false;

	for(int i = 0; i < MaxTeleportLoc; i++)
		m_pTeleportLoc[i] = 0;

	return true;
}

bool CMap::_bDecodeMapDataFileContents()
{
	HANDLE hFile;
	char  cMapFileName[256], cHeader[260], cTemp[100];
	DWORD dwFileSize, nRead;
	char* token, * context, cReadMode;
	char seps[] = "= \t\r\n";
	class CTile* pTile;

	std::memset(cMapFileName, 0, sizeof(cMapFileName));
	strcat(cMapFileName, "mapdata/");
	strcat(cMapFileName, m_cName);
	strcat(cMapFileName, ".amd");

	hFile = CreateFile(cMapFileName, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
	if (hFile == INVALID_HANDLE_VALUE) return false;
	dwFileSize = GetFileSize(hFile, 0);

	std::memset(cHeader, 0, sizeof(cHeader));
	ReadFile(hFile, (char*)cHeader, 256, &nRead, 0);


	for(int i = 0; i < 256; i++)
		if (cHeader[i] == 0) cHeader[i] = ' ';

	cReadMode = 0;

	token = strtok_s(cHeader, seps, &context);
	while (token != 0) {

		if (cReadMode != 0) {
			switch (cReadMode) {
			case 1:
				m_sSizeX = atoi(token);
				cReadMode = 0;
				break;
			case 2:
				m_sSizeY = atoi(token);
				cReadMode = 0;
				break;
			case 3:
				m_sTileDataSize = atoi(token);
				cReadMode = 0;
				break;
			}
		}
		else {
			if (memcmp(token, "MAPSIZEX", 8) == 0) cReadMode = 1;
			if (memcmp(token, "MAPSIZEY", 8) == 0) cReadMode = 2;
			if (memcmp(token, "TILESIZE", 8) == 0) cReadMode = 3;
		}

		token = strtok_s(NULL, seps, &context);
	}

	m_pTile = (class CTile*)new class CTile[m_sSizeX * m_sSizeY];

	for(int iy = 0; iy < m_sSizeY; iy++)
		for(int ix = 0; ix < m_sSizeX; ix++) {
			ReadFile(hFile, (char*)cTemp, m_sTileDataSize, &nRead, 0);
			pTile = (class CTile*)(m_pTile + ix + iy * m_sSizeX);
			if ((cTemp[8] & 0x80) != 0) {
				pTile->m_bIsMoveAllowed = false;
			}
			else pTile->m_bIsMoveAllowed = true;

			if ((cTemp[8] & 0x40) != 0) {
				pTile->m_bIsTeleport = true;
			}
			else pTile->m_bIsTeleport = false;

			if ((cTemp[8] & 0x20) != 0) {
				pTile->m_bIsFarm = true;
			}
			else pTile->m_bIsFarm = false;

			short sTileId;
			std::memcpy(&sTileId, &cTemp[0], sizeof(short));
			if (sTileId == 19) {
				pTile->m_bIsWater = true;
			}
			else pTile->m_bIsWater = false;


		}

	CloseHandle(hFile);

	return true;
}


bool CMap::bSearchTeleportDest(int sX, int sY, char* pMapName, int* pDx, int* pDy, char* pDir)
{
	// Collect all matching teleport entries for this source tile
	int matches[MaxTeleportLoc];
	int matchCount = 0;

	for (int i = 0; i < MaxTeleportLoc; i++) {
		if ((m_pTeleportLoc[i] != 0) && (m_pTeleportLoc[i]->m_sSrcX == sX) && (m_pTeleportLoc[i]->m_sSrcY == sY)) {
			matches[matchCount++] = i;
		}
	}

	if (matchCount == 0) return false;

	// Randomly select among matching entries
	int pick = matches[rand() % matchCount];
	memcpy(pMapName, m_pTeleportLoc[pick]->m_cDestMapName, 10);
	*pDx = m_pTeleportLoc[pick]->m_sDestX;
	*pDy = m_pTeleportLoc[pick]->m_sDestY;
	*pDir = m_pTeleportLoc[pick]->m_cDir;
	return true;
}

void CMap::SetDynamicObject(uint16_t wID, short sType, short sX, short sY, uint32_t dwRegisterTime)
{
	class CTile* pTile;


	if ((sX < 0) || (sX >= m_sSizeX) || (sY < 0) || (sY >= m_sSizeY)) return;

	pTile = (class CTile*)(m_pTile + sX + sY * m_sSizeX);

	pTile->m_wDynamicObjectID = wID;
	pTile->m_sDynamicObjectType = sType;
	pTile->m_dwDynamicObjectRegisterTime = dwRegisterTime;
}

bool CMap::bGetDynamicObject(short sX, short sY, short* pType, uint32_t* pRegisterTime, int* pIndex)
{
	class CTile* pTile;


	if ((sX < 0) || (sX >= m_sSizeX) || (sY < 0) || (sY >= m_sSizeY)) return false;

	pTile = (class CTile*)(m_pTile + sX + sY * m_sSizeX);

	*pType = pTile->m_sDynamicObjectType;
	*pRegisterTime = pTile->m_dwDynamicObjectRegisterTime;
	if (pIndex != 0) *pIndex = pTile->m_wDynamicObjectID;

	return true;
}

int CMap::iGetEmptyNamingValue()
{
	

	for(int i = 0; i < 1000; i++)
		if (m_bNamingValueUsingStatus[i] == false) {

			m_bNamingValueUsingStatus[i] = true;
			return i;
		}

	return -1;
}

void CMap::SetNamingValueEmpty(int iValue)
{
	m_bNamingValueUsingStatus[iValue] = false;
}

bool CMap::bGetIsWater(short dX, short dY)
{
	class CTile* pTile;

	if ((dX < 14) || (dX >= m_sSizeX - 16) || (dY < 12) || (dY >= m_sSizeY - 14)) return false;

	pTile = (class CTile*)(m_pTile + dX + dY * m_sSizeX);

	if (pTile->m_bIsWater == false) return false;

	return true;
}

bool CMap::bRemoveCropsTotalSum()
{
	if (m_iTotalAgriculture < MaxAgriculture)
	{
		m_iTotalAgriculture--;
		if (m_iTotalAgriculture < 0)
		{
			m_iTotalAgriculture = 0;
		}
		return true;
	}
	return false;
}

bool CMap::bAddCropsTotalSum()
{
	if (m_iTotalAgriculture < MaxAgriculture)
	{
		m_iTotalAgriculture++;
		return true;
	}
	return false;
}

bool CMap::bGetIsFarm(short tX, short tY)
{
	class CTile* pTile;

	if ((tX < 14) || (tX >= m_sSizeX - 16) || (tY < 12) || (tY >= m_sSizeY - 14)) return false;

	pTile = (class CTile*)(m_pTile + tX + tY * m_sSizeX);

	if (pTile->m_bIsFarm == false) return false;

	return true;
}

int CMap::iAnalyze(char cType, int* pX, int* pY, int* pV1, int* pV2, int* pV3)
{


	switch (cType) {
	case 1:

		break;


	}

	return 0;
}

void CMap::SetTempMoveAllowedFlag(int dX, int dY, bool bFlag)
{
	class CTile* pTile;

	if ((dX < 20) || (dX >= m_sSizeX - 20) || (dY < 20) || (dY >= m_sSizeY - 20)) return;

	pTile = (class CTile*)(m_pTile + dX + dY * m_sSizeX);
	pTile->m_bIsTempMoveAllowed = bFlag;
}

int CMap::iRegisterOccupyFlag(int dX, int dY, int iSide, int iEKNum, int iDOI)
{
	

	if ((dX < 20) || (dX >= m_sSizeX - 20) || (dY < 20) || (dY >= m_sSizeY - 20)) return -1;

	for(int i = 1; i < MaxOccupyFlag; i++)
		if (m_pOccupyFlag[i] == 0) {
			m_pOccupyFlag[i] = new class COccupyFlag(dX, dY, iSide, iEKNum, iDOI);
			if (m_pOccupyFlag == 0) return -1;
			else return i;
		}

	return -1;
}



void CMap::ClearSectorInfo()
{
	for(int ix = 0; ix < MaxSectors; ix++)
		for(int iy = 0; iy < MaxSectors; iy++) {
			m_stSectorInfo[ix][iy].iNeutralActivity = 0;
			m_stSectorInfo[ix][iy].iAresdenActivity = 0;
			m_stSectorInfo[ix][iy].iElvineActivity = 0;
			m_stSectorInfo[ix][iy].iMonsterActivity = 0;
			m_stSectorInfo[ix][iy].iPlayerActivity = 0;
		}
}

void CMap::ClearTempSectorInfo()
{
	for(int ix = 0; ix < MaxSectors; ix++)
		for(int iy = 0; iy < MaxSectors; iy++) {
			m_stTempSectorInfo[ix][iy].iNeutralActivity = 0;
			m_stTempSectorInfo[ix][iy].iAresdenActivity = 0;
			m_stTempSectorInfo[ix][iy].iElvineActivity = 0;
			m_stTempSectorInfo[ix][iy].iMonsterActivity = 0;
			m_stTempSectorInfo[ix][iy].iPlayerActivity = 0;
		}
}

void CMap::_SetupNoAttackArea()
{
	class CTile* pTile;

	for(int i = 0; i < MaxNmr; i++) {
		if ((m_rcNoAttackRect[i].y > 0)) {
			for(int ix = m_rcNoAttackRect[i].Left(); ix <= m_rcNoAttackRect[i].Right(); ix++)
				for(int iy = m_rcNoAttackRect[i].Top(); iy <= m_rcNoAttackRect[i].Bottom(); iy++) {
					pTile = (class CTile*)(m_pTile + ix + iy * m_sSizeX);
					pTile->m_iAttribute = pTile->m_iAttribute | 0x00000004;
				}
		}
		else if (m_rcNoAttackRect[i].y == -10) {
			for(int ix = 0; ix < m_sSizeX; ix++)
				for(int iy = 0; iy < m_sSizeY; iy++) {
					pTile = (class CTile*)(m_pTile + ix + iy * m_sSizeX);
					pTile->m_iAttribute = pTile->m_iAttribute | 0x00000004;
				}
		}
	}
}

/*********************************************************************************************************************
**  int CMap::iGetAttribute(int dX, int dY, int iBitMask)															**
**  description			:: check if the tile contains a dead player													**
**  last updated		:: November 20, 2004; 9:55 PM; Hypnotoad													**
**	return value		:: int																						**
**********************************************************************************************************************/
int CMap::iGetAttribute(int dX, int dY, int iBitMask)
{
	class CTile* pTile;

	if ((dX < 20) || (dX >= m_sSizeX - 20) || (dY < 20) || (dY >= m_sSizeY - 20)) return -1;

	pTile = (class CTile*)(m_pTile + dX + dY * m_sSizeX);
	return (pTile->m_iAttribute & iBitMask);
}

bool CMap::bAddCrusadeStructureInfo(char cType, short sX, short sY, char cSide)
{
	

	for(int i = 0; i < hb::shared::limits::MaxCrusadeStructures; i++)
		if (m_stCrusadeStructureInfo[i].cType == 0) {
			m_stCrusadeStructureInfo[i].cType = cType;
			m_stCrusadeStructureInfo[i].cSide = cSide;
			m_stCrusadeStructureInfo[i].sX = sX;
			m_stCrusadeStructureInfo[i].sY = sY;

			m_iTotalCrusadeStructures++;
			return true;
		}

	return false;
}

/*bool CMap::bAddHeldenianTowerInfo(char cType, short sX, short sY, char cSide)
{
 

	for(int i = 0; i < MaxHeldenianTower; i++)
	if (m_stHeldenianTower[i].cType == 0) {
	if (m_stHeldenianTower[i].cSide == 1) {
		m_stHeldenianTower[i].sTypeID = sTypeID;
		m_stHeldenianTower[i].cSide = cSide;
		m_stHeldenianTower[i].sX = sX;
		m_stHeldenianTower[i].sY = sY;
		m_iHeldenianAresdenLeftTower++;
		return true;
	}
	else if (m_stHeldenianTower[i].cSide == 2) {
		m_stHeldenianTower[i].sTypeID = sTypeID;
		m_stHeldenianTower[i].cSide = cSide;
		m_stHeldenianTower[i].sX = sX;
		m_stHeldenianTower[i].sY = sY;
		m_iHeldenianElvineLeftTower++;
		return true;
	}

	return false;
}*/

bool CMap::bRemoveCrusadeStructureInfo(short sX, short sY)
{
	

	for(int i = 0; i < hb::shared::limits::MaxCrusadeStructures; i++)
		if ((m_stCrusadeStructureInfo[i].sX == sX) && (m_stCrusadeStructureInfo[i].sY == sY)) {
			m_stCrusadeStructureInfo[i].cType = 0;
			m_stCrusadeStructureInfo[i].cSide = 0;
			m_stCrusadeStructureInfo[i].sX = 0;
			m_stCrusadeStructureInfo[i].sY = 0;
			goto RCSI_REARRANGE;
		}

	return false;

RCSI_REARRANGE:;

	for(int i = 0; i < hb::shared::limits::MaxCrusadeStructures - 1; i++)
		if ((m_stCrusadeStructureInfo[i].cType == 0) && (m_stCrusadeStructureInfo[i + 1].cType != 0)) {
			m_stCrusadeStructureInfo[i].cType = m_stCrusadeStructureInfo[i + 1].cType;
			m_stCrusadeStructureInfo[i].cSide = m_stCrusadeStructureInfo[i + 1].cSide;
			m_stCrusadeStructureInfo[i].sX = m_stCrusadeStructureInfo[i + 1].sX;
			m_stCrusadeStructureInfo[i].sY = m_stCrusadeStructureInfo[i + 1].sY;

			m_stCrusadeStructureInfo[i + 1].cType = 0;
			m_stCrusadeStructureInfo[i + 1].cSide = 0;
			m_stCrusadeStructureInfo[i + 1].sX = 0;
			m_stCrusadeStructureInfo[i + 1].sY = 0;
		}

	m_iTotalCrusadeStructures--;
	return true;
}



void CMap::RestoreStrikePoints()
{
	

	for(int i = 0; i < MaxStrikePoints; i++) {
		m_stStrikePoint[i].iInitHP = m_stStrikePoint[i].iHP;
	}
}

