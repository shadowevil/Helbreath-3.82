// StrategicPoint.h: interface for the CStrategicPoint class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)
#include "Platform.h"
#include "CommonTypes.h"


class CStrategicPoint
{
public:
	inline CStrategicPoint()
	{
		m_iX	= 0;
		m_iY	= 0;
		m_iSide = 0;
	}

	inline virtual ~CStrategicPoint()
	{
	}

	int		m_iSide;			// ������ �������� �Ҽ�: 0�̸� �߸�
	int     m_iValue;			// �߿䵵
	int		m_iX, m_iY;			// ��ġ
};
