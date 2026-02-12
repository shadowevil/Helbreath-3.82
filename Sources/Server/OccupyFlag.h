// OccupyFlag.h: interface for the COccupyFlag class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)
#include "Platform.h"
#include "CommonTypes.h"

class COccupyFlag
{
public:
	inline COccupyFlag(int dX, int dY, char cSide, int iEKNum, int iDOI)
	{
		m_sX = dX;
		m_sY = dY;

		m_cSide = cSide;
		m_iEKCount = iEKNum;

		m_iDynamicObjectIndex = iDOI;
	}

	inline virtual ~COccupyFlag()
	{
	}

	char m_cSide;
	int  m_iEKCount;
	int  m_sX, m_sY;

	int  m_iDynamicObjectIndex;
};
