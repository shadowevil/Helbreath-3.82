// TeleportLoc.h: interface for the CTeleportLoc class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)
#include "Platform.h"
#include "CommonTypes.h"

class CTeleportLoc  
{
public:
	inline CTeleportLoc()
	{

		std::memset(m_cDestMapName, 0, sizeof(m_cDestMapName));
		//std::memset(m_cDestMapName2, 0, sizeof(m_cDestMapName2));
		m_sSrcX   = -1;
		m_sSrcY	  = -1;
		m_sDestX  = -1;
		m_sDestY  = -1;
		m_sDestX2 = -1;
		m_sDestY2 = -1;

		m_iV1     = 0;
		m_iV2     = 0;
		m_dwTime  = 0;
		m_dwTime2 = 0;

	}

	inline virtual ~CTeleportLoc()
	{

	}
												  
	short m_sSrcX, m_sSrcY;

	char  m_cDestMapName[11],  m_cDir;
	char  m_cDestMapName2[11];
	short m_sDestX,  m_sDestY;
	short m_sDestX2, m_sDestY2;

	int   m_iV1, m_iV2;
	uint32_t m_dwTime, m_dwTime2;

};
