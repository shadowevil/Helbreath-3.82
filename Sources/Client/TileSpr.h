// TileSpr.h: interface for the CTileSpr class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "PlatformCompat.h"

class CTileSpr
{
public:
	inline CTileSpr()
	{
		m_sTileSprite = 0;
		m_sTileSpriteFrame = 0;
		m_sObjectSprite = 0;
		m_sObjectSpriteFrame = 0;
		m_bIsMoveAllowed = true;
		m_bIsTeleport = false;
	}

	inline virtual ~CTileSpr()
	{
	}

	short m_sTileSprite;
	short m_sTileSpriteFrame;
	short m_sObjectSprite;
	short m_sObjectSpriteFrame;
	bool  m_bIsMoveAllowed;
	bool  m_bIsTeleport;
};
