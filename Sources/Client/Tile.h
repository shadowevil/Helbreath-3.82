// Tile.h: interface for the CTile class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "PlatformCompat.h"
#include "EntityMotion.h"
#include "AnimationState.h"
#include <string>
#include "Appearance.h"
#include "PlayerStatusData.h"

class CTile
{
public:
	inline void Clear()
	{
		m_wObjectID     = 0;
		m_wDeadObjectID = 0;

		m_sOwnerType = 0;
		m_sNpcConfigId = -1;
		m_cOwnerName.clear();

		m_sDeadOwnerType = 0;
		m_sDeadNpcConfigId = -1;
		m_cDeadOwnerName.clear();

		m_cDeadOwnerFrame = -1;
		m_dwDeadOwnerTime = 0;

		m_sItemID = 0;
		m_dwItemAttr = 0;
		m_cItemColor       = 0;

		m_sDynamicObjectType  = 0;
		m_cDynamicObjectFrame = 0;

		m_iChatMsg     = 0;
		m_iDeadChatMsg = 0;

		m_status.Clear();
		m_deadStatus.Clear();

		m_sV1 = 0;
		m_sV2 = 0;
		m_sV3 = 0;

		m_iEffectType  = 0;
		m_iEffectFrame = 0;
		m_iEffectTotalFrame = 0;
		m_dwEffectTime = 0;

		m_appearance.Clear();
		m_deadAppearance.Clear();

		m_animation.Reset();
		m_motion.Reset();
	}

	inline CTile()
	{
		m_sOwnerType = 0;
		m_sNpcConfigId = -1;
		m_sDeadOwnerType = 0;
		m_sDeadNpcConfigId = -1;
		m_cDeadOwnerFrame     = -1;

		m_sDynamicObjectType  = 0;
		m_cDynamicObjectFrame = 0;

		m_iChatMsg       = 0;
		m_iDeadChatMsg   = 0;

		m_wObjectID = 0;

		m_iEffectType  = 0;
		m_iEffectFrame = 0;
		m_iEffectTotalFrame = 0;
		m_dwEffectTime = 0;
	}

	inline virtual ~CTile()
	{
	}
	DWORD m_dwEffectTime;
	DWORD m_dwDeadOwnerTime;
	DWORD m_dwDynamicObjectTime;

	int   m_iChatMsg;
	int   m_cItemColor; // v1.4
	int   m_iEffectType;
	int   m_iEffectFrame, m_iEffectTotalFrame;
	int   m_iDeadChatMsg;

	WORD  m_wDeadObjectID;
	WORD  m_wObjectID;

	short m_sOwnerType;							// +B2C
	short m_sNpcConfigId;						// NPC config index (for name lookup, -1 if player)
	hb::shared::entity::PlayerStatus m_status;

	short m_sDeadOwnerType;						// +B3C
	short m_sDeadNpcConfigId;					// Dead NPC config index

	hb::shared::entity::PlayerStatus m_deadStatus;
	short m_sV1;
	short m_sV2;
	short m_sV3;								// +B50
	short m_sDynamicObjectType;

	short m_sItemID;
	DWORD m_dwItemAttr;

	char  m_cDeadOwnerFrame;
	char  m_cDeadDir;

	char  m_cDynamicObjectFrame;
	char  m_cDynamicObjectData1, m_cDynamicObjectData2, m_cDynamicObjectData3, m_cDynamicObjectData4;
	std::string m_cOwnerName;
	std::string m_cDeadOwnerName;

	// Unpacked appearance data (extracted from m_sAppr1-4 at reception)
	hb::shared::entity::PlayerAppearance m_appearance;
	hb::shared::entity::PlayerAppearance m_deadAppearance;

	// Animation state (replaces m_cOwnerAction, m_cOwnerFrame, m_cDir, m_dwOwnerTime)
	AnimationState m_animation;

	// Smooth movement interpolation
	EntityMotion m_motion;
};
