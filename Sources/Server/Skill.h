// Skill.h: interface for the CSkill class.

#pragma once

// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)

#include "Platform.h"
#include "CommonTypes.h"



namespace hb::server::skill
{
namespace EffectType
{
	enum : int
	{
		Get     = 1,
		Pretend = 2,
		Taming  = 3,
	};
}
} // namespace hb::server::skill


class CSkill
{
public:
	inline CSkill()
	{
		std::memset(m_cName, 0, sizeof(m_cName));
		m_bIsUseable = false;
		m_cUseMethod = 0;
	}

	inline virtual ~CSkill()
	{
	}

	char m_cName[42];

	short m_sType;
	short m_sValue1, m_sValue2, m_sValue3, m_sValue4, m_sValue5, m_sValue6;

	// Client display fields (sent via PacketSkillConfig)
	bool  m_bIsUseable;    // Whether skill can be actively used
	char  m_cUseMethod;    // Use method (0=passive, 1=click, 2=target)
};
