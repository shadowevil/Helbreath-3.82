// DelayEvent.h: interface for the CDelayEvent class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)

#include "Platform.h"
#include "CommonTypes.h"

namespace hb::server::delay_event
{
namespace Type
{
	enum : int
	{
		DamageObject            = 1,
		MagicRelease            = 2,
		UseItemSkill            = 3,
		MeteorStrike            = 4,
		DoMeteorStrikeDamage    = 5,
		CalcMeteorStrikeEffect  = 6,
		AncientTablet           = 7,
	};
}
} // namespace hb::server::delay_event

class CDelayEvent
{
public:
	inline CDelayEvent()
	{
	}

	inline virtual ~CDelayEvent()
	{
	}

	int m_iDelayType;
	int m_iEffectType;

	char m_cMapIndex;
	int m_dX, m_dY;

	int  m_iTargetH;
	char m_cTargetType;
	int m_iV1, m_iV2, m_iV3;

	uint32_t m_dwTriggerTime;
};
