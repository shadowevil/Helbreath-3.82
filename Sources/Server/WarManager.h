#pragma once
#include <cstdint>
#include <cstddef>

class CGame;

class WarManager
{
public:
	WarManager() = default;
	~WarManager() = default;
	void SetGame(CGame* pGame) { m_pGame = pGame; }

	// ========================================================================
	// Crusade System
	// ========================================================================
	void CrusadeWarStarter();
	void GlobalStartCrusadeMode();
	void LocalStartCrusadeMode(uint32_t dwGuildGUID);
	void LocalEndCrusadeMode(int iWinnerSide);
	void ManualEndCrusadeMode(int iWinnerSide);
	void CreateCrusadeStructures();
	void RemoveCrusadeStructures();
	void RemoveCrusadeNpcs(void);
	void RemoveCrusadeRecallTime(void);
	void SyncMiddlelandMapInfo();
	void SelectCrusadeDutyHandler(int iClientH, int iDuty);
	void CheckCrusadeResultCalculation(int iClientH);
	bool bReadCrusadeGUIDFile(char* cFn);
	void _CreateCrusadeGUID(uint32_t dwCrusadeGUID, int iWinnerSide);
	void CheckCommanderConstructionPoint(int iClientH);
	bool __bSetConstructionKit(int iMapIndex, int dX, int dY, int iType, int iTimeCost, int iClientH);

	// ========================================================================
	// Grand Magic / Meteor Strike
	// ========================================================================
	void MeteorStrikeHandler(int iMapIndex);
	void MeteorStrikeMsgHandler(char cAttackerSide);
	void CalcMeteorStrikeEffectHandler(int iMapIndex);
	void DoMeteorStrikeDamageHandler(int iMapIndex);
	void _LinkStrikePointMapIndex();
	void _GrandMagicLaunchMsgSend(int iType, char cAttackerSide);
	void GrandMagicResultHandler(char* cMapName, int iCrashedStructureNum, int iStructureDamageAmount, int iCasualities, int iActiveStructure, int iTotalStrikePoints, char* cData);
	void CollectedManaHandler(uint16_t wAresdenMana, uint16_t wElvineMana);
	void SendCollectedMana();

	// ========================================================================
	// Map Status & Guild War Operations
	// ========================================================================
	void _SendMapStatus(int iClientH);
	void MapStatusHandler(int iClientH, int iMode, const char* pMapName);
	void RequestSummonWarUnitHandler(int iClientH, int dX, int dY, char cType, char cNum, char cMode);
	void RequestGuildTeleportHandler(int iClientH);
	void RequestSetGuildTeleportLocHandler(int iClientH, int dX, int dY, int iGuildGUID, char* pMapName);
	void RequestSetGuildConstructLocHandler(int iClientH, int dX, int dY, int iGuildGUID, const char* pMapName);

	// ========================================================================
	// Heldenian Battle System
	// ========================================================================
	void SetHeldenianMode();
	void GlobalStartHeldenianMode();
	void LocalStartHeldenianMode(short sV1, short sV2, uint32_t dwHeldenianGUID);
	void GlobalEndHeldenianMode();
	void LocalEndHeldenianMode();
	bool UpdateHeldenianStatus();
	void _CreateHeldenianGUID(uint32_t dwHeldenianGUID, int iWinnerSide);
	void ManualStartHeldenianMode(int iClientH, char* pData, size_t dwMsgSize);
	void ManualEndHeldenianMode(int iClientH, char* pData, size_t dwMsgSize);
	bool bNotifyHeldenianWinner();
	void RemoveHeldenianNpc(int iNpcH);
	void RequestHeldenianTeleport(int iClientH, char* pData, size_t dwMsgSize);
	bool bCheckHeldenianMap(int sAttackerH, int iMapIndex, char cType);
	void CheckHeldenianResultCalculation(int iClientH);
	void RemoveOccupyFlags(int iMapIndex);

	// ========================================================================
	// Apocalypse System
	// ========================================================================
	void ApocalypseEnder();
	void GlobalEndApocalypseMode();
	void LocalEndApocalypse();
	void LocalStartApocalypse(uint32_t dwApocalypseGUID);
	bool bReadApocalypseGUIDFile(char* cFn);
	bool bReadHeldenianGUIDFile(char* cFn);
	void _CreateApocalypseGUID(uint32_t dwApocalypseGUID);

	// ========================================================================
	// Energy Sphere & Occupy Territory
	// ========================================================================
	void EnergySphereProcessor();
	bool bCheckEnergySphereDestination(int iNpcH, short sAttackerH, char cAttackerType);
	void GetOccupyFlagHandler(int iClientH);
	size_t _iComposeFlagStatusContents(char* pData);
	void SetSummonMobAction(int iClientH, int iMode, size_t dwMsgSize, char* pData = 0);
	bool __bSetOccupyFlag(char cMapIndex, int dX, int dY, int iSide, int iEKNum, int iClientH);

	// ========================================================================
	// FightZone System
	// ========================================================================
	void FightzoneReserveHandler(int iClientH, char* pData, size_t dwMsgSize);
	void FightzoneReserveProcessor();
	void GetFightzoneTicketHandler(int iClientH);

private:
	CGame* m_pGame = nullptr;
};
