// GameMonitor.h: interface for the CGameMonitor class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "PlatformCompat.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "ChatMsg.h"

namespace hb::client::config { constexpr int MaxBadWord = 500; }

class CGameMonitor  
{
public:
	bool bCheckBadWord(char * pWord);
	int iReadBadWordFileList(char * pFn);
	CGameMonitor();
	virtual ~CGameMonitor();

	class CMsg * m_pWordList[hb::client::config::MaxBadWord];

};
