// Quest.h: interface for the CQuest class.

#pragma once

// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)
#include "Platform.h"
#include "CommonTypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>


namespace hb::server::quest
{
namespace Type
{
	enum : int
	{
		MonsterHunt             = 1,
		MonsterHuntTimeLimit    = 2,
		Assassination           = 3,
		Delivery                = 4,
		Escort                  = 5,
		Guard                   = 6,
		GoPlace                 = 7,
		BuildStructure          = 8,
		SupplyBuildStructure    = 9,
		StrategicStrike         = 10,
		SendToBattle            = 11,
		SetOccupyFlag           = 12,
	};
}
} // namespace hb::server::quest

class CQuest  
{
public:
	
	char m_cSide;
	
	int m_iType;				// Quest
	int m_iTargetType;			// Quest  .  Type    .
	int m_iMaxCount;

	int m_iFrom;				// Quest  NPC
	
	int m_iMinLevel;			// Quest    .
	int m_iMaxLevel;			// Quest

	int m_iRequiredSkillNum;
	int m_iRequiredSkillLevel;

	int m_iTimeLimit;
	int m_iAssignType;			// . -1 . 1 Crusade .

								// . 3  1  . 0   .
	int m_iRewardType[4]; 
	int m_iRewardAmount[4];

	int m_iContribution;
	int m_iContributionLimit;

	int m_iResponseMode;		// : 0(ok) 1(Accept/Decline) 2(Next)

	char m_cTargetName[hb::shared::limits::NpcNameLen];
	int  m_sX, m_sY, m_iRange;

	int  m_iQuestID;

	int  m_iReqContribution;


	//CQuest();
	//virtual ~CQuest();

};
