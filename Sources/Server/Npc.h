// Npc.h: interface for the CNpc class.

#pragma once

// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)

#include "Platform.h"
#include "CommonTypes.h"
#include "Magic.h"
#include "GameGeometry.h"
#include "Appearance.h"
#include "PlayerStatusData.h"

namespace hb::server::npc
{

constexpr int MaxWaypoints = 10;

namespace MoveType
{
	enum : int
	{
		Stop            = 0,
		SeqWaypoint     = 1,
		RandomWaypoint  = 2,
		Follow          = 3,
		RandomArea      = 4,
		Random          = 5,
		Guard           = 6,
	};
}

namespace Behavior
{
	enum : int
	{
		Stop    = 0,
		Move    = 1,
		Attack  = 2,
		Flee    = 3,
		Dead    = 4,
	};
}

} // namespace hb::server::npc

class CNpc  
{
public:
	CNpc(char * pName5);
	virtual ~CNpc();

	// Auras
	char m_pMagicConfigList[100];

	char  m_cNpcName[hb::shared::limits::NpcNameLen];

	char  m_cName[6];
	char  m_cMapIndex;
	short m_sX, m_sY;
	short m_sPrevX, m_sPrevY; // OPTIMIZATION FIX #3: Track previous position for delta detection
	short m_dX, m_dY;
	short m_vX, m_vY;
	int   m_tmp_iError;
	hb::shared::geometry::GameRectangle  m_rcRandomArea;	// MOVETYPE_RANDOMAREA

	char  m_cDir;
	char  m_cAction;
	char  m_cTurn;

	short m_sType;
	short m_sOriginalType;
	short m_iNpcConfigId;
	hb::shared::entity::EntityAppearance m_appearance;
	hb::shared::entity::EntityStatus m_status;

	uint32_t m_dwTime;
	uint32_t m_dwActionTime;
	uint32_t m_dwHPupTime, m_dwMPupTime;
	uint32_t m_dwDeadTime, m_dwRegenTime;

	int  m_iHP, m_iMaxHP;						// Hit Point 
	uint32_t  m_iExp;                    // ? ? . ExpDice  .

	int  m_iHitDice;				// Hit Dice.   HP .
	int  m_iDefenseRatio;			// Defense Ratio
	int  m_iHitRatio;				// HitRatio
	int  m_iMagicHitRatio;
	int  m_iMinBravery;
	uint32_t  m_iExpDiceMin;
	uint32_t	 m_iExpDiceMax;
	uint32_t  m_iGoldDiceMin;
	uint32_t  m_iGoldDiceMax;
	int   m_iDropTableId;

	char m_cSide;
	char m_cActionLimit;            // 1 Move   .    2    . 3 Dummy.  ,

	char m_cSize;					// 0: Small-Medium 1: Large
	char m_cAttackDiceThrow;
	char m_cAttackDiceRange;
	char m_cAttackBonus;
	char m_cBravery;
	char m_cResistMagic;
	char m_cMagicLevel;
	char m_cDayOfWeekLimit;
	char m_cChatMsgPresence;		// ? Chat Msg
	int  m_iMana;                   // MagicLevel*30
	int  m_iMaxMana;
																    
	char  m_cMoveType;
	char  m_cBehavior;
	short m_sBehaviorTurnCount;
	char  m_cTargetSearchRange;

	int   m_iFollowOwnerIndex;
	char  m_cFollowOwnerType;		// (NPC or Player)
	bool  m_bIsSummoned;            // NPC? HP  .
	bool  m_bBypassMobLimit;        // GM-spawned: don't count toward map entity limit
	uint32_t m_dwSummonedTime;

	int   m_iTargetIndex;
	char  m_cTargetType;			// (NPC or Player)
	char  m_cCurWaypoint;
	char  m_cTotalWaypoint;

	int   m_iSpotMobIndex;			// spot-mob-generator ?
	int   m_iWayPointIndex[hb::server::npc::MaxWaypoints+1];
	char  m_cMagicEffectStatus[hb::server::config::MaxMagicEffects];

	bool  m_bIsPermAttackMode;
   	uint32_t   m_iNoDieRemainExp;
	int   m_iAttackStrategy;
	int   m_iAILevel;

	int   m_iAttackRange;
	/*
		AI-Level 
			1: ���� �ൿ 
			2: �������� ���� ���� ��ǥ���� ���� 
			3: ���� ��ȣ���� ��ǥ�� ���� ���ݴ�󿡼�? ���� 
	*/
	int   m_iAttackCount;
	bool  m_bIsKilled;
	bool  m_bIsUnsummoned;

	int   m_iLastDamage;
	int   m_iSummonControlMode;		// ?: 0 Free, 1 Hold 2 Tgt
	char  m_cAttribute;				// :   1  2  3  4
	int   m_iAbsDamage;

	int   m_iItemRatio;
	int   m_iAssignedItem;

	char  m_cSpecialAbility;

									/*
case 0: break;
case 1:  "Penetrating Invisibility"
case 2:  "Breaking Magic Protection"
case 3:  "Absorbing Physical Damage"
case 4:  "Absorbing Magical Damage"
case 5:  "Poisonous"
case 6:  "Extremely Poisonous"
case 7:  "Explosive"
case 8:  "Hi-Explosive" 

 ���� �� ���� 60���� ũ�� NPC�� ȿ���ʹ� �����ϹǷ� �����Ѵ�.
									*/

	int	  m_iBuildCount;			// ?      .  m_iMinBravery.
	int   m_iManaStock;
	bool  m_bIsMaster;
	int   m_iGuildGUID;
	
	char m_cCropType;
	char m_cCropSkill;

	int   m_iV1;
	char m_cArea;

	int m_iNpcItemType;
	int m_iNpcItemMax;

};
