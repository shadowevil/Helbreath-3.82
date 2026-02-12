// Fish.h: interface for the CFish class.

#pragma once

// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)

#include "Platform.h"
#include "CommonTypes.h"
#include "Item.h"

class CFish  
{
public:
	inline CFish(char cMapIndex, short sX, short sY, short sType, CItem * pItem, int iDifficulty)
	{
		m_cMapIndex		= cMapIndex;
		m_sX			= sX;
		m_sY			= sY;
		m_sType			= sType;
		m_pItem			= pItem;

		m_sEngagingCount = 0;
		m_iDifficulty    = iDifficulty;

		if (m_iDifficulty <= 0)
			m_iDifficulty = 1;
	}

	inline virtual ~CFish()
	{
		if (m_pItem != 0) delete m_pItem;
	}

	char  m_cMapIndex;
	short m_sX, m_sY;

	short m_sType;
	CItem * m_pItem;

	short m_sDynamicObjectHandle;

	short m_sEngagingCount;
	int   m_iDifficulty;
};

