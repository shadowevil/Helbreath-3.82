// Magic.h: interface for the CMagic class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)

#include "Platform.h"
#include "CommonTypes.h"
#include "MagicTypes.h"

namespace hb::server::config { constexpr int MaxMagicEffects = 100; }

class CMagic
{
public:
	inline CMagic()
	{
		std::memset(m_cName, 0, sizeof(m_cName));
		m_iAttribute = 0;
	}

	inline virtual ~CMagic()
	{
	}

	char m_cName[31];

	short m_sType;
	uint32_t m_dwDelayTime, m_dwLastTime;
	short m_sValue1, m_sValue2, m_sValue3, m_sValue4, m_sValue5, m_sValue6;
	short m_sValue7, m_sValue8, m_sValue9, m_sValue10, m_sValue11, m_sValue12;
	short m_sIntLimit;
	int   m_iGoldCost;
	
	char  m_cCategory; // ��a� �l�u: RDa� ��a� 0, �r�ݸ�a� 1, acl� ��a� 2 
	int   m_iAttribute; // ��a� L�Ls:  �A 1 �D�A 2 sN 3 a� 4
};

