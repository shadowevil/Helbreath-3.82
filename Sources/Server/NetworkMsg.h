// Msg.h: interface for the CMsg class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)
#include "Platform.h"
#include "CommonTypes.h"

namespace hb::server::msg
{
namespace Source
{
	enum : int
	{
		Client    = 1,
		LogServer = 2,
		Bot       = 4,
	};
}
} // namespace hb::server::msg

class CMsg  								 
{
public:
	void Get(char * pFrom, char * pData, size_t* pSize, int * pIndex, char * pKey);
	bool bPut(char cFrom, char * pData, size_t dwSize, int iIndex, char cKey);
	CMsg();
	virtual ~CMsg();

	char   m_cFrom;

	char * m_pData;
	size_t  m_dwSize;

	int    m_iIndex;
	char   m_cKey;   // v1.4
};
