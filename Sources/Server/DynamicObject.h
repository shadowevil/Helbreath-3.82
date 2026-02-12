// DynamicObject.h: interface for the CDynamicObject class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)

#include "Platform.h"
#include "CommonTypes.h"

class CDynamicObject
{
public:
	inline CDynamicObject(short sOwner, char cOwnerType, short sType, char cMapIndex, short sX, short sY, uint32_t dwRegisterTime, uint32_t dwLastTime, int iV1)
	{
		m_sOwner         = sOwner;
		m_cOwnerType     = cOwnerType;

		m_sType          = sType;

		m_cMapIndex      = cMapIndex;
		m_sX             = sX;
		m_sY             = sY;

		m_dwRegisterTime = dwRegisterTime;
		m_dwLastTime     = dwLastTime;

		m_iCount         = 0;
		m_iV1            = iV1;
	}

	inline virtual ~CDynamicObject()
	{
	}

	short m_sOwner;
	char  m_cOwnerType;

	short m_sType;
	char  m_cMapIndex;
	short m_sX, m_sY; 
	uint32_t m_dwRegisterTime;
	uint32_t m_dwLastTime;

	int   m_iCount;			// Ư�� ������Ʈ�� ��� ����ϴ� ī���� ���� 
	int   m_iV1;			// �߰� ������ ������ ����Ѵ�.
};

