// BuildItem.h: interface for the CBuildItem class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include <string>

#include "PlatformCompat.h"
#include "CommonTypes.h"
#include "NetConstants.h"

class CBuildItem
{
public:
	inline CBuildItem()
	{
		int i;

		m_bBuildEnabled = false;
		m_iSkillLimit   = 0;
		m_iMaxSkill     = 0;

		m_iSprH     = 0;
		m_iSprFrame = 0;

		for (i = 0; i < 7; i++) {
			m_iElementCount[i] = 0;
			m_bElementFlag[i]  = 0;
		}
	}

	inline virtual ~CBuildItem()
	{

	}

	bool m_bBuildEnabled;
	std::string m_cName;
	int	 m_iSkillLimit;
	int  m_iMaxSkill;
	int  m_iSprH, m_iSprFrame;
	std::string m_cElementName1;
	std::string m_cElementName2;
	std::string m_cElementName3;
	std::string m_cElementName4;
	std::string m_cElementName5;
	std::string m_cElementName6;
	uint32_t m_iElementCount[7];
	bool  m_bElementFlag[7];

};
