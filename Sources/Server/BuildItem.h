// BuildItem.h: interface for the CBuildItem class.

#pragma once

// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)

#include "Platform.h"
#include "CommonTypes.h"
#include "NetConstants.h"

class CBuildItem  
{
public:
	inline CBuildItem()
	{
		

		std::memset(m_cName, 0, sizeof(m_cName));
		m_sItemID = -1;

		m_iSkillLimit = 0;

		for(int i = 0; i < 6; i++) {
			m_iMaterialItemID[i]    = 0;
			m_iMaterialItemCount[i] = 0;
			m_iMaterialItemValue[i] = 0;
			m_iIndex[i]            = -1;
		}

		m_iMaxValue     = 0;
		m_iAverageValue = 0;
		m_iMaxSkill     = 0;
		m_wAttribute    = 0;
	}

	inline virtual ~CBuildItem()
	{

	}

	char  m_cName[hb::shared::limits::ItemNameLen];
	short m_sItemID;

	int  m_iSkillLimit;
	
	int  m_iMaterialItemID[6];
	int  m_iMaterialItemCount[6];
	int  m_iMaterialItemValue[6];
	int  m_iIndex[6];

	int	 m_iMaxValue;
	int  m_iAverageValue;	
	int   m_iMaxSkill;
	uint16_t  m_wAttribute;
};

