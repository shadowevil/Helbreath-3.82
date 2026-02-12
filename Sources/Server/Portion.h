// Portion.h: interface for the CPortion class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)
#include "Platform.h"
#include "CommonTypes.h"
#include "NetConstants.h"

class CPortion
{
public:
	inline CPortion()
	{
		

		std::memset(m_cName, 0, sizeof(m_cName));
		m_iSkillLimit = 0;
		m_iDifficulty = 0;

		for(int i = 0; i < 12; i++)
			m_sArray[i] = -1;
	}

	inline virtual ~CPortion()
	{

	}

	char  m_cName[hb::shared::limits::ItemNameLen];
	short m_sArray[12];

	int   m_iSkillLimit, m_iDifficulty;

};
