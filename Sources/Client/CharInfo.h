// CharInfo.h: interface for the CCharInfo class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include <string>

#include "PlatformCompat.h"
#include "Appearance.h"

class CCharInfo
{
public:
	inline CCharInfo()
	{
		m_iYear   = 0;
		m_iMonth  = 0;
		m_iDay    = 0;
		m_iHour   = 0;
		m_iMinute = 0;
	}

	inline virtual ~CCharInfo()
	{
	}

	std::string m_cName;
	std::string m_cMapName;
	short m_sSkinCol, m_sSex;
	hb::shared::entity::PlayerAppearance m_appearance;
	short	m_sStr, m_sVit, m_sDex, m_sInt, m_sMag, m_sChr;
	int	m_sLevel;
	DWORD   m_iExp;
	int   m_iYear, m_iMonth, m_iDay, m_iHour, m_iMinute;
};
