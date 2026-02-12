// Tile.h: interface for the CTile class.

#pragma once

// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)
#include "Platform.h"
#include "CommonTypes.h"
#include "Item.h"

namespace hb::server::map { constexpr int TilePerItems = 12; }

class CTile  
{												  
public:
	CTile();
	virtual ~CTile();

	char  m_cOwnerClass;		// DEF_OT_PLAYER / DEF_OT_NPC
	short m_sOwner;

	char  m_cDeadOwnerClass;	// DEF_OT_PLAYER / DEF_OT_NPC
	short m_sDeadOwner;

	CItem * m_pItem[hb::server::map::TilePerItems];
	char  m_cTotalItem;

	uint16_t  m_wDynamicObjectID;
	short m_sDynamicObjectType;
	uint32_t m_dwDynamicObjectRegisterTime;

	bool  m_bIsMoveAllowed, m_bIsTeleport, m_bIsWater, m_bIsFarm, m_bIsTempMoveAllowed;

	int   m_iOccupyStatus;    // Aresden -, Elvine + .
	int   m_iOccupyFlagIndex;

	// Crusade
	int	  m_iAttribute;		  // :  ( )  (  )  ()
	
	
};

