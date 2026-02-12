// Map.h: interface for the CMap class.

#pragma once

// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)

#include "Platform.h"
#include "CommonTypes.h"
#include "NetConstants.h"
#include "OccupyFlag.h"
#include "Tile.h"
#include "StrategicPoint.h"
#include "GameGeometry.h"


#include "OwnerClass.h"

namespace hb::server::map
{
constexpr int MaxTeleportLoc        = 200;
constexpr int MaxWaypointCfg        = 200;
constexpr int MaxMgar               = 50;
constexpr int MaxNmr                = 50;
constexpr int MaxSpotMobGenerator   = 100;
constexpr int MaxFishPoint          = 200;
constexpr int MaxItemEvents         = 200;
constexpr int MaxMineralPoint       = 200;
constexpr int MaxHeldenianDoor      = 200;
constexpr int MaxOccupyFlag         = 20001;
constexpr int MaxInitialPoint       = 20;
constexpr int MaxAgriculture        = 200;
constexpr int MaxDynamicGates       = 10;
constexpr int MaxHeldenianTower     = 200;
} // namespace hb::server::map

// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)

#include "Platform.h"
#include "CommonTypes.h"
#include "Game.h"
#include "TeleportLoc.h"
#include "GlobalDef.h"

namespace hb::server::map
{
namespace MapType
{
	enum : int
	{
		Normal              = 0,
		NoPenaltyNoReward   = 1,
	};
}
constexpr int MaxEnergySpheres      = 10;
constexpr int MaxStrategicPoints    = 200;
constexpr int MaxSectors            = 60;
constexpr int MaxStrikePoints       = 20;
} // namespace hb::server::map




class CMap  
{
public:
	
	bool bCheckFlySpaceAvailable(short sX, char sY, char cDir, short sOwner);
	bool bGetIsFarm(short tX, short tY);
	void RestoreStrikePoints();
	bool bRemoveCrusadeStructureInfo(short sX, short sY);
	bool bAddCrusadeStructureInfo(char cType, short sX, short sY, char cSide);
	int iGetAttribute(int dX, int dY, int iBitMask);
	void _SetupNoAttackArea();
	void ClearTempSectorInfo();
	void ClearSectorInfo();
	int iRegisterOccupyFlag(int dX, int dY, int iSide, int iEKNum, int iDOI);
	int  iCheckItem(short sX, short sY);
	void SetTempMoveAllowedFlag(int dX, int dY, bool bFlag);
	int iAnalyze(char cType, int *pX, int *pY, int * pV1, int *pV2, int * pV3);
	bool bGetIsWater(short dX, short dY);
	void GetDeadOwner(short * pOwner, char * pOwnerClass, short sX, short sY);
	bool bGetIsMoveAllowedTile(short dX, short dY);
	void SetNamingValueEmpty(int iValue);
	int iGetEmptyNamingValue();
	bool bGetDynamicObject(short sX, short sY, short * pType, uint32_t * pRegisterTime, int * pIndex = 0);
	void SetDynamicObject(uint16_t wID, short sType, short sX, short sY, uint32_t dwRegisterTime);
	bool bGetIsTeleport(short dX, short dY);
	bool bSearchTeleportDest(int sX, int sY, char * pMapName, int * pDx, int * pDy, char * pDir);
	bool bInit(char * pName);
	bool bIsValidLoc(short sX, short sY);
	CItem * pGetItem(short sX, short sY, short* pRemainItemID, char* pRemainItemColor, uint32_t* pRemainItemAttr);
	bool bSetItem(short sX, short sY, CItem * pItem);
	void ClearDeadOwner(short sX, short sY);
	void ClearOwner(int iDebugCode, short sOwnerH, char cOwnerType, short sX, short sY);
	bool bGetMoveable(short dX, short dY, short * pDOtype = 0, short * pTopItem = 0);
	void GetOwner(short * pOwner, char * pOwnerClass, short sX, short sY);
	void SetOwner(short sOwner, char cOwnerClass, short sX, short sY);
	void SetDeadOwner(short sOwner, char cOwnerClass, short sX, short sY);
	bool bRemoveCropsTotalSum();
	bool bAddCropsTotalSum();
	void SetBigOwner(short sOwner, char cOwnerClass, short sX, short sY, char cArea);

	CMap(class CGame * pGame);
	virtual ~CMap();

	class CTile * m_pTile;
	class CGame * m_pGame;
	char  m_cName[11];
	char  m_cLocationName[11];
	short m_sSizeX, m_sSizeY, m_sTileDataSize;
	class CTeleportLoc * m_pTeleportLoc[hb::server::map::MaxTeleportLoc];
	
	//short m_sInitialPointX, m_sInitialPointY;
	hb::shared::geometry::GamePoint m_pInitialPoint[hb::server::map::MaxInitialPoint];

	bool  m_bNamingValueUsingStatus[1000]; // 0~999
	bool  m_bRandomMobGenerator;
	char  m_cRandomMobGeneratorLevel;
	int   m_iTotalActiveObject;
	int   m_iTotalAliveObject;
	int   m_iMaximumObject;

	char  m_cType;

	bool  m_bIsFixedDayMode;

	struct {
		bool bDefined;
		char cType;				// 1:RANDOMAREA   2:RANDOMWAYPOINT

		char cWaypoint[10];     // RANDOMWAYPOINT
		hb::shared::geometry::GameRectangle rcRect;			// RANDOMAREA

		int  iTotalActiveMob;
		int  iNpcConfigId;
		int  iMaxMobs;
		int  iCurMobs;
		int  iProbSA;
		int  iKindSA;

	} m_stSpotMobGenerator[hb::server::map::MaxSpotMobGenerator];

	hb::shared::geometry::GamePoint m_WaypointList[hb::server::map::MaxWaypointCfg];
	hb::shared::geometry::GameRectangle  m_rcMobGenAvoidRect[hb::server::map::MaxMgar];
	hb::shared::geometry::GameRectangle  m_rcNoAttackRect[hb::server::map::MaxNmr];

	hb::shared::geometry::GamePoint m_FishPointList[hb::server::map::MaxFishPoint];
	int   m_iTotalFishPoint, m_iMaxFish, m_iCurFish;
	
	int	  m_iApocalypseMobGenType, m_iApocalypseBossMobNpcID;
	hb::shared::geometry::GameRectangle m_rcApocalypseBossMob;
	char  m_cDynamicGateType;
	hb::shared::geometry::GameRectangle m_rcDynamicGateCoord;
	char  m_cDynamicGateCoordDestMap[11];
	short m_sDynamicGateCoordTgtX, m_sDynamicGateCoordTgtY;
	bool  m_bIsCitizenLimit;
	short m_sHeldenianTowerType, m_sHeldenianTowerXPos, m_sHeldenianTowerYPos;
	char  m_cHeldenianTowerSide;
	char  m_cHeldenianModeMap;

	bool  m_bMineralGenerator;
	char  m_cMineralGeneratorLevel;
	hb::shared::geometry::GamePoint m_MineralPointList[hb::server::map::MaxMineralPoint];
	int   m_iTotalMineralPoint, m_iMaxMineral, m_iCurMineral;

	char  m_cWhetherStatus;		// . 0 . 1~3  4~6  7~9
	uint32_t m_dwWhetherLastTime, m_dwWhetherStartTime;

	int   m_iLevelLimit;
	int   m_iUpperLevelLimit;

	class COccupyFlag * m_pOccupyFlag[hb::server::map::MaxOccupyFlag];
	int   m_iTotalOccupyFlags;
	
	class CStrategicPoint * m_pStrategicPointList[hb::server::map::MaxStrategicPoints];
	bool  m_bIsAttackEnabled;

	bool  m_bIsFightZone;

	struct {
		char cType;
		int sX, sY;

	} m_stEnergySphereCreationList[hb::server::map::MaxEnergySpheres];

	int m_iTotalEnergySphereCreationPoint;
	
	struct {
		char cResult;
		int aresdenX, aresdenY, elvineX, elvineY;
	} m_stEnergySphereGoalList[hb::server::map::MaxEnergySpheres];

	int m_iTotalEnergySphereGoalPoint;

	bool m_bIsEnergySphereGoalEnabled;
	int m_iCurEnergySphereGoalPointIndex; 

	struct {
		bool m_bIsGateMap;
		char m_cDynamicGateMap[11];
		int m_iDynamicGateX;
		int m_iDynamicGateY;
	} m_stDynamicGateCoords[hb::server::map::MaxDynamicGates];

	struct {
		int iPlayerActivity;
		int iNeutralActivity;
		int iAresdenActivity;
		int iElvineActivity;
		int iMonsterActivity;

	} m_stSectorInfo[hb::server::map::MaxSectors][hb::server::map::MaxSectors], m_stTempSectorInfo[hb::server::map::MaxSectors][hb::server::map::MaxSectors];
	short sMobEventAmount;
	int m_iTotalItemEvents;
	struct {
		char cItemName[hb::shared::limits::ItemNameLen];
		int iAmount;
		int iTotal;
		int iMonth;
		int iDay;
		int iTotalNum;
	} m_stItemEventList[hb::server::map::MaxItemEvents]{};

	struct {
		char  cDir;
		short dX;
		short dY;
	} m_stHeldenianGateDoor[hb::server::map::MaxHeldenianDoor];

	struct {
		short sTypeID;
		short dX;
		short dY;
		char  cSide;
	} m_stHeldenianTower[hb::server::map::MaxHeldenianTower];

	int m_iMaxNx, m_iMaxNy, m_iMaxAx, m_iMaxAy, m_iMaxEx, m_iMaxEy, m_iMaxMx, m_iMaxMy, m_iMaxPx, m_iMaxPy;
	
	struct {
		char cRelatedMapName[11];
		int iMapIndex;
		int dX, dY;
		int iHP, iInitHP;

		int iEffectX[5];
		int iEffectY[5];
	
	} m_stStrikePoint[hb::server::map::MaxStrikePoints];
	int m_iTotalStrikePoints;

	bool m_bIsDisabled;
	int m_iTotalAgriculture;

	struct {
		char cType;			// NULL   .
		char cSide;
		short sX, sY;
	} m_stCrusadeStructureInfo[hb::shared::limits::MaxCrusadeStructures];
	int m_iTotalCrusadeStructures;
	bool m_bIsEnergySphereAutoCreation;
private:
	bool _bDecodeMapDataFileContents();
public:
	// Snow BOOLean for certain maps to snow instead of rain
	bool m_bIsSnowEnabled;
	bool m_bIsRecallImpossible;
	bool m_bIsApocalypseMap;
	bool m_bIsHeldenianMap;
};
