// MapData.cpp: implementation of the CMapData class.
//
//////////////////////////////////////////////////////////////////////
#define _WINSOCKAPI_

#include "MapData.h"
#include "OwnerType.h"
#include "ObjectIDRange.h"
#include "DirectionHelpers.h"
#include "CommonTypes.h"
#include "Benchmark.h"
#include "EntityMotion.h"
#include <cstring>
#include <cstdio>
#include <string_view>
#include "WeatherManager.h"
#include <string>
#include <algorithm>
#include <charconv>



using namespace hb::shared::net;
namespace dynamic_object = hb::shared::dynamic_object;

using namespace hb::shared::action;
using namespace hb::client::config;

namespace
{
	const uint32_t DEF_FULLDATA_REQUEST_INTERVAL = 2000;
	uint32_t g_dwLastFullDataRequestTime[hb::shared::object_id::NpcMax];
	bool ShouldRequestFullData(uint16_t wObjectID, int sX, int sY)
	{
		if (hb::shared::object_id::IsNearbyOffset(wObjectID)) return false;
		if (sX != -1 || sY != -1) return true;

		uint32_t dwNow = GameClock::GetTimeMS();
		if (dwNow - g_dwLastFullDataRequestTime[wObjectID] < DEF_FULLDATA_REQUEST_INTERVAL) {
			return false;
		}
		g_dwLastFullDataRequestTime[wObjectID] = dwNow;
		return true;
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMapData::CMapData(class CGame* pGame)
{
	int i;
	m_pGame = pGame;
	std::fill(std::begin(m_iObjectIDcacheLocX), std::end(m_iObjectIDcacheLocX), 0);
	std::fill(std::begin(m_iObjectIDcacheLocY), std::end(m_iObjectIDcacheLocY), 0);
	m_dwDOframeTime = m_dwFrameTime = GameClock::GetTimeMS();

	for (i = 0; i < TotalCharacters; i++)
	{
		m_stFrame[i][Type::Move].m_sMaxFrame = 7;
	}
	for (i = 1; i <= 6; i++)
	{
		// Original Helbreath 3.82 timing values
		m_stFrame[i][Type::Stop].m_sMaxFrame = 14;
		m_stFrame[i][Type::Stop].m_sFrameTime = 60;
		m_stFrame[i][Type::Move].m_sMaxFrame = 7;
		m_stFrame[i][Type::Move].m_sFrameTime = 70;
		m_stFrame[i][Type::DamageMove].m_sMaxFrame = 3;
		m_stFrame[i][Type::DamageMove].m_sFrameTime = 50;
		m_stFrame[i][Type::Run].m_sMaxFrame = 7;
		m_stFrame[i][Type::Run].m_sFrameTime = 39;
		m_stFrame[i][Type::Attack].m_sMaxFrame = 7;
		m_stFrame[i][Type::Attack].m_sFrameTime = 78;
		m_stFrame[i][Type::AttackMove].m_sMaxFrame = 12;
		m_stFrame[i][Type::AttackMove].m_sFrameTime = 78;
		m_stFrame[i][Type::Magic].m_sMaxFrame = 15;
		m_stFrame[i][Type::Magic].m_sFrameTime = 88;
		m_stFrame[i][Type::GetItem].m_sMaxFrame = 3;
		m_stFrame[i][Type::GetItem].m_sFrameTime = 150;
		m_stFrame[i][Type::Damage].m_sMaxFrame = 7;
		m_stFrame[i][Type::Damage].m_sFrameTime = 70;
		m_stFrame[i][Type::Dying].m_sMaxFrame = 12;
		m_stFrame[i][Type::Dying].m_sFrameTime = 80;
	}

	m_stFrame[hb::shared::owner::Slime][Type::Stop].m_sFrameTime = 240;
	m_stFrame[hb::shared::owner::Slime][Type::Stop].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Slime][Type::Move].m_sFrameTime = 63;
	m_stFrame[hb::shared::owner::Slime][Type::Attack].m_sFrameTime = 90;
	m_stFrame[hb::shared::owner::Slime][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Slime][Type::Damage].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Slime][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Slime][Type::Dying].m_sFrameTime = 240;
	m_stFrame[hb::shared::owner::Slime][Type::Dying].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Skeleton][Type::Stop].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Skeleton][Type::Stop].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Skeleton][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::Skeleton][Type::Attack].m_sFrameTime = 90;
	m_stFrame[hb::shared::owner::Skeleton][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Skeleton][Type::Damage].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Skeleton][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Skeleton][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::Skeleton][Type::Dying].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::StoneGolem][Type::Stop].m_sFrameTime = 210;
	m_stFrame[hb::shared::owner::StoneGolem][Type::Stop].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::StoneGolem][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::StoneGolem][Type::Attack].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::StoneGolem][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::StoneGolem][Type::Damage].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::StoneGolem][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::StoneGolem][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::StoneGolem][Type::Dying].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Cyclops][Type::Stop].m_sFrameTime = 210;
	m_stFrame[hb::shared::owner::Cyclops][Type::Stop].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Cyclops][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Cyclops][Type::Attack].m_sFrameTime = 90;
	m_stFrame[hb::shared::owner::Cyclops][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Cyclops][Type::Damage].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Cyclops][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Cyclops][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::Cyclops][Type::Dying].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::OrcMage][Type::Stop].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::OrcMage][Type::Stop].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::OrcMage][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::OrcMage][Type::Attack].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::OrcMage][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::OrcMage][Type::Damage].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::OrcMage][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::OrcMage][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::OrcMage][Type::Dying].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::ShopKeeper][Type::Stop].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::ShopKeeper][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::ShopKeeper][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::ShopKeeper][Type::Attack].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::ShopKeeper][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::ShopKeeper][Type::Damage].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::ShopKeeper][Type::Damage].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::ShopKeeper][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::ShopKeeper][Type::Dying].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::GiantAnt][Type::Stop].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::GiantAnt][Type::Stop].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::GiantAnt][Type::Move].m_sFrameTime = 55;
	m_stFrame[hb::shared::owner::GiantAnt][Type::Attack].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::GiantAnt][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::GiantAnt][Type::Damage].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::GiantAnt][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::GiantAnt][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::GiantAnt][Type::Dying].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Scorpion][Type::Stop].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::Scorpion][Type::Stop].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Scorpion][Type::Move].m_sFrameTime = 40;
	m_stFrame[hb::shared::owner::Scorpion][Type::Attack].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::Scorpion][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Scorpion][Type::Damage].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Scorpion][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Scorpion][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::Scorpion][Type::Dying].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Zombie][Type::Stop].m_sFrameTime = 210;
	m_stFrame[hb::shared::owner::Zombie][Type::Stop].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Zombie][Type::Move].m_sFrameTime = 90;
	m_stFrame[hb::shared::owner::Zombie][Type::Attack].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Zombie][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Zombie][Type::Damage].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Zombie][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Zombie][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::Zombie][Type::Dying].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Gandalf][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Gandalf][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Gandalf][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Gandalf][Type::Attack].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Gandalf][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Gandalf][Type::Damage].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::Gandalf][Type::Damage].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Gandalf][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::Gandalf][Type::Dying].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Howard][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Howard][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Howard][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Howard][Type::Attack].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Howard][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Howard][Type::Damage].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::Howard][Type::Damage].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Howard][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::Howard][Type::Dying].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Guard][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Guard][Type::Stop].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Guard][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Guard][Type::Attack].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::Guard][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Guard][Type::Damage].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Guard][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Guard][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::Guard][Type::Dying].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Amphis][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Amphis][Type::Stop].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Amphis][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Amphis][Type::Attack].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::Amphis][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Amphis][Type::Damage].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Amphis][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Amphis][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::Amphis][Type::Dying].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::ClayGolem][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::ClayGolem][Type::Stop].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::ClayGolem][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::ClayGolem][Type::Attack].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::ClayGolem][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::ClayGolem][Type::Damage].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::ClayGolem][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::ClayGolem][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::ClayGolem][Type::Dying].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Tom][Type::Stop].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Tom][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::William][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::William][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Kennedy][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Kennedy][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Hellhound][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Hellhound][Type::Stop].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Hellhound][Type::Move].m_sFrameTime = 50;
	m_stFrame[hb::shared::owner::Hellhound][Type::Attack].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::Hellhound][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Hellhound][Type::Damage].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::Hellhound][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Hellhound][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::Hellhound][Type::Dying].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Troll][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Troll][Type::Stop].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Troll][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Troll][Type::Attack].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Troll][Type::Attack].m_sMaxFrame = 5;
	m_stFrame[hb::shared::owner::Troll][Type::Damage].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::Troll][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Troll][Type::Dying].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Troll][Type::Dying].m_sMaxFrame = 9;
	m_stFrame[hb::shared::owner::Ogre][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Ogre][Type::Stop].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Ogre][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Ogre][Type::Attack].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::Ogre][Type::Attack].m_sMaxFrame = 5;
	m_stFrame[hb::shared::owner::Ogre][Type::Damage].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::Ogre][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Ogre][Type::Dying].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Ogre][Type::Dying].m_sMaxFrame = 9;
	m_stFrame[hb::shared::owner::Liche][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Liche][Type::Stop].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Liche][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Liche][Type::Attack].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::Liche][Type::Attack].m_sMaxFrame = 5;
	m_stFrame[hb::shared::owner::Liche][Type::Damage].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::Liche][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Liche][Type::Dying].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Liche][Type::Dying].m_sMaxFrame = 9;
	m_stFrame[hb::shared::owner::Demon][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Demon][Type::Stop].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Demon][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Demon][Type::Attack].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::Demon][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Demon][Type::Damage].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::Demon][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Demon][Type::Dying].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Demon][Type::Dying].m_sMaxFrame = 9;
	m_stFrame[hb::shared::owner::Unicorn][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Unicorn][Type::Stop].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Unicorn][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Unicorn][Type::Attack].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::Unicorn][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Unicorn][Type::Damage].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::Unicorn][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Unicorn][Type::Dying].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Unicorn][Type::Dying].m_sMaxFrame = 11;
	m_stFrame[hb::shared::owner::WereWolf][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::WereWolf][Type::Stop].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::WereWolf][Type::Move].m_sFrameTime = 80;
	m_stFrame[hb::shared::owner::WereWolf][Type::Attack].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::WereWolf][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::WereWolf][Type::Damage].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::WereWolf][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::WereWolf][Type::Dying].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::WereWolf][Type::Dying].m_sMaxFrame = 11;
	m_stFrame[hb::shared::owner::Dummy][Type::Stop].m_sFrameTime = 240;
	m_stFrame[hb::shared::owner::Dummy][Type::Stop].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Dummy][Type::Move].m_sFrameTime = 80;
	m_stFrame[hb::shared::owner::Dummy][Type::Attack].m_sFrameTime = 90;
	m_stFrame[hb::shared::owner::Dummy][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Dummy][Type::Damage].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Dummy][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Dummy][Type::Dying].m_sFrameTime = 240;
	m_stFrame[hb::shared::owner::Dummy][Type::Dying].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::EnergySphere][Type::Stop].m_sFrameTime = 80;
	m_stFrame[hb::shared::owner::EnergySphere][Type::Stop].m_sMaxFrame = 9;
	m_stFrame[hb::shared::owner::EnergySphere][Type::Move].m_sFrameTime = 20;
	m_stFrame[hb::shared::owner::EnergySphere][Type::Move].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::EnergySphere][Type::Attack].m_sFrameTime = 80;
	m_stFrame[hb::shared::owner::EnergySphere][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::EnergySphere][Type::Damage].m_sFrameTime = 80;
	m_stFrame[hb::shared::owner::EnergySphere][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::EnergySphere][Type::Dying].m_sFrameTime = 80;
	m_stFrame[hb::shared::owner::EnergySphere][Type::Dying].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::ArrowGuardTower][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::ArrowGuardTower][Type::Stop].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::ArrowGuardTower][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::ArrowGuardTower][Type::Move].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::ArrowGuardTower][Type::Attack].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::ArrowGuardTower][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::ArrowGuardTower][Type::Damage].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::ArrowGuardTower][Type::Damage].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::ArrowGuardTower][Type::Dying].m_sFrameTime = 200;
	m_stFrame[hb::shared::owner::ArrowGuardTower][Type::Dying].m_sMaxFrame = 6;
	m_stFrame[hb::shared::owner::CannonGuardTower][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::CannonGuardTower][Type::Stop].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::CannonGuardTower][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::CannonGuardTower][Type::Move].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::CannonGuardTower][Type::Attack].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::CannonGuardTower][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::CannonGuardTower][Type::Damage].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::CannonGuardTower][Type::Damage].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::CannonGuardTower][Type::Dying].m_sFrameTime = 200;
	m_stFrame[hb::shared::owner::CannonGuardTower][Type::Dying].m_sMaxFrame = 6;
	m_stFrame[hb::shared::owner::ManaCollector][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::ManaCollector][Type::Stop].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::ManaCollector][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::ManaCollector][Type::Move].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::ManaCollector][Type::Attack].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::ManaCollector][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::ManaCollector][Type::Damage].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::ManaCollector][Type::Damage].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::ManaCollector][Type::Dying].m_sFrameTime = 200;
	m_stFrame[hb::shared::owner::ManaCollector][Type::Dying].m_sMaxFrame = 6;
	m_stFrame[hb::shared::owner::Detector][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Detector][Type::Stop].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::Detector][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Detector][Type::Move].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::Detector][Type::Attack].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::Detector][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Detector][Type::Damage].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Detector][Type::Damage].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::Detector][Type::Dying].m_sFrameTime = 200;
	m_stFrame[hb::shared::owner::Detector][Type::Dying].m_sMaxFrame = 6;
	m_stFrame[hb::shared::owner::EnergyShield][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::EnergyShield][Type::Stop].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::EnergyShield][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::EnergyShield][Type::Move].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::EnergyShield][Type::Attack].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::EnergyShield][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::EnergyShield][Type::Damage].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::EnergyShield][Type::Damage].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::EnergyShield][Type::Dying].m_sFrameTime = 200;
	m_stFrame[hb::shared::owner::EnergyShield][Type::Dying].m_sMaxFrame = 6;
	m_stFrame[hb::shared::owner::GrandMagicGenerator][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::GrandMagicGenerator][Type::Stop].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::GrandMagicGenerator][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::GrandMagicGenerator][Type::Move].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::GrandMagicGenerator][Type::Attack].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::GrandMagicGenerator][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::GrandMagicGenerator][Type::Damage].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::GrandMagicGenerator][Type::Damage].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::GrandMagicGenerator][Type::Dying].m_sFrameTime = 200;
	m_stFrame[hb::shared::owner::GrandMagicGenerator][Type::Dying].m_sMaxFrame = 6;
	m_stFrame[hb::shared::owner::ManaStone][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::ManaStone][Type::Stop].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::ManaStone][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::ManaStone][Type::Move].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::ManaStone][Type::Attack].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::ManaStone][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::ManaStone][Type::Damage].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::ManaStone][Type::Damage].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::ManaStone][Type::Dying].m_sFrameTime = 200;
	m_stFrame[hb::shared::owner::ManaStone][Type::Dying].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::LightWarBeetle][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::LightWarBeetle][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::LightWarBeetle][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::LightWarBeetle][Type::Attack].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::LightWarBeetle][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::LightWarBeetle][Type::Damage].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::LightWarBeetle][Type::Damage].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::LightWarBeetle][Type::Dying].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::LightWarBeetle][Type::Dying].m_sMaxFrame = 9;
	m_stFrame[hb::shared::owner::GodsHandKnight][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::GodsHandKnight][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::GodsHandKnight][Type::Move].m_sFrameTime = 55;
	m_stFrame[hb::shared::owner::GodsHandKnight][Type::Attack].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::GodsHandKnight][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::GodsHandKnight][Type::Damage].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::GodsHandKnight][Type::Damage].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::GodsHandKnight][Type::Dying].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::GodsHandKnight][Type::Dying].m_sMaxFrame = 9;
	m_stFrame[hb::shared::owner::GodsHandKnightCK][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::GodsHandKnightCK][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::GodsHandKnightCK][Type::Move].m_sFrameTime = 55;
	m_stFrame[hb::shared::owner::GodsHandKnightCK][Type::Attack].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::GodsHandKnightCK][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::GodsHandKnightCK][Type::Damage].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::GodsHandKnightCK][Type::Damage].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::GodsHandKnightCK][Type::Dying].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::GodsHandKnightCK][Type::Dying].m_sMaxFrame = 9;
	m_stFrame[hb::shared::owner::TempleKnight][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::TempleKnight][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::TempleKnight][Type::Move].m_sFrameTime = 55;
	m_stFrame[hb::shared::owner::TempleKnight][Type::Attack].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::TempleKnight][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::TempleKnight][Type::Damage].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::TempleKnight][Type::Damage].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::TempleKnight][Type::Dying].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::TempleKnight][Type::Dying].m_sMaxFrame = 9;
	m_stFrame[hb::shared::owner::BattleGolem][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::BattleGolem][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::BattleGolem][Type::Move].m_sFrameTime = 55;
	m_stFrame[hb::shared::owner::BattleGolem][Type::Attack].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::BattleGolem][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::BattleGolem][Type::Damage].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::BattleGolem][Type::Damage].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::BattleGolem][Type::Dying].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::BattleGolem][Type::Dying].m_sMaxFrame = 9;
	m_stFrame[hb::shared::owner::Stalker][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Stalker][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Stalker][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Stalker][Type::Attack].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Stalker][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Stalker][Type::Damage].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::Stalker][Type::Damage].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::Stalker][Type::Dying].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Stalker][Type::Dying].m_sMaxFrame = 9;
	m_stFrame[hb::shared::owner::HellClaw][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::HellClaw][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::HellClaw][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::HellClaw][Type::Attack].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::HellClaw][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::HellClaw][Type::Damage].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::HellClaw][Type::Damage].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::HellClaw][Type::Dying].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::HellClaw][Type::Dying].m_sMaxFrame = 9;
	m_stFrame[hb::shared::owner::TigerWorm][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::TigerWorm][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::TigerWorm][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::TigerWorm][Type::Attack].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::TigerWorm][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::TigerWorm][Type::Damage].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::TigerWorm][Type::Damage].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::TigerWorm][Type::Dying].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::TigerWorm][Type::Dying].m_sMaxFrame = 9;
	m_stFrame[hb::shared::owner::Catapult][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Catapult][Type::Stop].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::Catapult][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Catapult][Type::Attack].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Catapult][Type::Attack].m_sMaxFrame = 4;
	m_stFrame[hb::shared::owner::Catapult][Type::Damage].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::Catapult][Type::Damage].m_sMaxFrame = 0;
	m_stFrame[hb::shared::owner::Catapult][Type::Dying].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Catapult][Type::Dying].m_sMaxFrame = 6;
	m_stFrame[hb::shared::owner::Gargoyle][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Gargoyle][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Gargoyle][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Gargoyle][Type::Attack].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::Gargoyle][Type::Attack].m_sMaxFrame = 9;
	m_stFrame[hb::shared::owner::Gargoyle][Type::Damage].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::Gargoyle][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Gargoyle][Type::Dying].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Gargoyle][Type::Dying].m_sMaxFrame = 11 + 3;
	m_stFrame[hb::shared::owner::Beholder][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Beholder][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Beholder][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Beholder][Type::Attack].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Beholder][Type::Attack].m_sMaxFrame = 12;
	m_stFrame[hb::shared::owner::Beholder][Type::Damage].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::Beholder][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Beholder][Type::Dying].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::Beholder][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::DarkElf][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::DarkElf][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::DarkElf][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::DarkElf][Type::Attack].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::DarkElf][Type::Attack].m_sMaxFrame = 9;
	m_stFrame[hb::shared::owner::DarkElf][Type::Damage].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::DarkElf][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::DarkElf][Type::Dying].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::DarkElf][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::Bunny][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Bunny][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Bunny][Type::Move].m_sFrameTime = 30;
	m_stFrame[hb::shared::owner::Bunny][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Bunny][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Bunny][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Bunny][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Bunny][Type::Dying].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Bunny][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::Cat][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Cat][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Cat][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Cat][Type::Attack].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Cat][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Cat][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Cat][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Cat][Type::Dying].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Cat][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::GiantFrog][Type::Stop].m_sFrameTime = 300;
	m_stFrame[hb::shared::owner::GiantFrog][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::GiantFrog][Type::Move].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::GiantFrog][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::GiantFrog][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::GiantFrog][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::GiantFrog][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::GiantFrog][Type::Dying].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::GiantFrog][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::MountainGiant][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::MountainGiant][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::MountainGiant][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::MountainGiant][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::MountainGiant][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::MountainGiant][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::MountainGiant][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::MountainGiant][Type::Dying].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::MountainGiant][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::Ettin][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Ettin][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Ettin][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::Ettin][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Ettin][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Ettin][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Ettin][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Ettin][Type::Dying].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Ettin][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::CannibalPlant][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::CannibalPlant][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::CannibalPlant][Type::Move].m_sFrameTime = 80;
	m_stFrame[hb::shared::owner::CannibalPlant][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::CannibalPlant][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::CannibalPlant][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::CannibalPlant][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::CannibalPlant][Type::Dying].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::CannibalPlant][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::Rudolph][Type::Stop].m_sFrameTime = 200;
	m_stFrame[hb::shared::owner::Rudolph][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Rudolph][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::Rudolph][Type::Attack].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::Rudolph][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Rudolph][Type::Damage].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Rudolph][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Rudolph][Type::Dying].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Rudolph][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::DireBoar][Type::Stop].m_sFrameTime = 200;
	m_stFrame[hb::shared::owner::DireBoar][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::DireBoar][Type::Move].m_sFrameTime = 55;
	m_stFrame[hb::shared::owner::DireBoar][Type::Attack].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::DireBoar][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::DireBoar][Type::Damage].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::DireBoar][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::DireBoar][Type::Dying].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::DireBoar][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::Frost][Type::Stop].m_sFrameTime = 200;
	m_stFrame[hb::shared::owner::Frost][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Frost][Type::Move].m_sFrameTime = 55;
	m_stFrame[hb::shared::owner::Frost][Type::Attack].m_sFrameTime = 80;
	m_stFrame[hb::shared::owner::Frost][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Frost][Type::Damage].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Frost][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Frost][Type::Dying].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Frost][Type::Dying].m_sMaxFrame = 5 + 3;
	m_stFrame[hb::shared::owner::Crops][Type::Stop].m_sFrameTime = 200;
	m_stFrame[hb::shared::owner::Crops][Type::Stop].m_sMaxFrame = 40;
	m_stFrame[hb::shared::owner::Crops][Type::Move].m_sFrameTime = 160;
	m_stFrame[hb::shared::owner::Crops][Type::Attack].m_sFrameTime = 200;
	m_stFrame[hb::shared::owner::Crops][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Crops][Type::Damage].m_sFrameTime = 200;
	m_stFrame[hb::shared::owner::Crops][Type::Damage].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Crops][Type::Dying].m_sFrameTime = 200;
	m_stFrame[hb::shared::owner::Crops][Type::Dying].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::IceGolem][Type::Stop].m_sFrameTime = 200;
	m_stFrame[hb::shared::owner::IceGolem][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::IceGolem][Type::Move].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::IceGolem][Type::Attack].m_sFrameTime = 105;
	m_stFrame[hb::shared::owner::IceGolem][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::IceGolem][Type::Damage].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::IceGolem][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::IceGolem][Type::Dying].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::IceGolem][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::Wyvern][Type::Stop].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Wyvern][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Wyvern][Type::Move].m_sFrameTime = 90;
	m_stFrame[hb::shared::owner::Wyvern][Type::Attack].m_sFrameTime = 80;
	m_stFrame[hb::shared::owner::Wyvern][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Wyvern][Type::Damage].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Wyvern][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Wyvern][Type::Dying].m_sFrameTime = 65;
	m_stFrame[hb::shared::owner::Wyvern][Type::Dying].m_sMaxFrame = 15 + 3;
	m_stFrame[hb::shared::owner::McGaffin][Type::Stop].m_sFrameTime = 200;
	m_stFrame[hb::shared::owner::McGaffin][Type::Stop].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::McGaffin][Type::Move].m_sFrameTime = 80;
	m_stFrame[hb::shared::owner::Perry][Type::Move].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::McGaffin][Type::Attack].m_sFrameTime = 80;
	m_stFrame[hb::shared::owner::McGaffin][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::McGaffin][Type::Damage].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::McGaffin][Type::Damage].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::McGaffin][Type::Dying].m_sFrameTime = 65;
	m_stFrame[hb::shared::owner::McGaffin][Type::Dying].m_sMaxFrame = 3 + 3;
	m_stFrame[hb::shared::owner::Perry][Type::Stop].m_sFrameTime = 200;
	m_stFrame[hb::shared::owner::Perry][Type::Stop].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Perry][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::Perry][Type::Move].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Perry][Type::Attack].m_sFrameTime = 80;
	m_stFrame[hb::shared::owner::Perry][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Perry][Type::Damage].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Perry][Type::Damage].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Perry][Type::Dying].m_sFrameTime = 65;
	m_stFrame[hb::shared::owner::Perry][Type::Dying].m_sMaxFrame = 3 + 3;
	m_stFrame[hb::shared::owner::Devlin][Type::Stop].m_sFrameTime = 200;
	m_stFrame[hb::shared::owner::Devlin][Type::Stop].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Devlin][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::Perry][Type::Move].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Devlin][Type::Attack].m_sFrameTime = 80;
	m_stFrame[hb::shared::owner::Devlin][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Devlin][Type::Damage].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::Devlin][Type::Damage].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::Devlin][Type::Dying].m_sFrameTime = 65;
	m_stFrame[hb::shared::owner::Devlin][Type::Dying].m_sMaxFrame = 3 + 3;
	m_stFrame[hb::shared::owner::Dragon][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Dragon][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Dragon][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::Dragon][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Dragon][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Dragon][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Dragon][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Dragon][Type::Dying].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Dragon][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::Centaur][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Centaur][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Centaur][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::Centaur][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Centaur][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Centaur][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Centaur][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Centaur][Type::Dying].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Centaur][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::ClawTurtle][Type::Stop].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::ClawTurtle][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::ClawTurtle][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::ClawTurtle][Type::Attack].m_sFrameTime = 80;
	m_stFrame[hb::shared::owner::ClawTurtle][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::ClawTurtle][Type::Damage].m_sFrameTime = 60;
	m_stFrame[hb::shared::owner::ClawTurtle][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::ClawTurtle][Type::Dying].m_sFrameTime = 65;
	m_stFrame[hb::shared::owner::ClawTurtle][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::FireWyvern][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::FireWyvern][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::FireWyvern][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::FireWyvern][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::FireWyvern][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::FireWyvern][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::FireWyvern][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::FireWyvern][Type::Dying].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::FireWyvern][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::GiantCrayfish][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::GiantCrayfish][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::GiantCrayfish][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::GiantCrayfish][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::GiantCrayfish][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::GiantCrayfish][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::GiantCrayfish][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::GiantCrayfish][Type::Dying].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::GiantCrayfish][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::GiLizard][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::GiLizard][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::GiLizard][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::GiLizard][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::GiLizard][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::GiLizard][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::GiLizard][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::GiLizard][Type::Dying].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::GiLizard][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::GiTree][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::GiTree][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::GiTree][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::GiTree][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::GiTree][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::GiTree][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::GiTree][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::GiTree][Type::Dying].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::GiTree][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::MasterOrc][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::MasterOrc][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::MasterOrc][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::MasterOrc][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::MasterOrc][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::MasterOrc][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::MasterOrc][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::MasterOrc][Type::Dying].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::MasterOrc][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::Minaus][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Minaus][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Minaus][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::Minaus][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Minaus][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Minaus][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Minaus][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Minaus][Type::Dying].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Minaus][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::Nizie][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Nizie][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Nizie][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::Nizie][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Nizie][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Nizie][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Nizie][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Nizie][Type::Dying].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Nizie][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::Tentocle][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Tentocle][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Tentocle][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::Tentocle][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Tentocle][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Tentocle][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Tentocle][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Tentocle][Type::Dying].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::Tentocle][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::Abaddon][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Abaddon][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Abaddon][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::Abaddon][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Abaddon][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Abaddon][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Abaddon][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Abaddon][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::Abaddon][Type::Dying].m_sMaxFrame = 15 + 3;
	m_stFrame[hb::shared::owner::Sorceress][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Sorceress][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Sorceress][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::Sorceress][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Sorceress][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Sorceress][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Sorceress][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Sorceress][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::Sorceress][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::ATK][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::ATK][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::ATK][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::ATK][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::ATK][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::ATK][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::ATK][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::ATK][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::ATK][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::MasterElf][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::MasterElf][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::MasterElf][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::MasterElf][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::MasterElf][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::MasterElf][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::MasterElf][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::MasterElf][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::MasterElf][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::DSK][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::DSK][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::DSK][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::DSK][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::DSK][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::DSK][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::DSK][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::DSK][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::DSK][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::HBT][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::HBT][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::HBT][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::HBT][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::HBT][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::HBT][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::HBT][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::HBT][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::HBT][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::CT][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::CT][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::CT][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::CT][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::CT][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::CT][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::CT][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::CT][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::CT][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::Barbarian][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Barbarian][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Barbarian][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::Barbarian][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Barbarian][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Barbarian][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Barbarian][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Barbarian][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::Barbarian][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::AGC][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::AGC][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::AGC][Type::Move].m_sFrameTime = 70;
	m_stFrame[hb::shared::owner::AGC][Type::Attack].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::AGC][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::AGC][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::AGC][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::AGC][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::AGC][Type::Dying].m_sMaxFrame = 10;
	m_stFrame[hb::shared::owner::Gail][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Gail][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Gate][Type::Stop].m_sFrameTime = 250;
	m_stFrame[hb::shared::owner::Gate][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Gate][Type::Damage].m_sFrameTime = 100;
	m_stFrame[hb::shared::owner::Gate][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::Gate][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::Gate][Type::Dying].m_sMaxFrame = 10;

	m_stFrame[99][Type::Stop].m_sFrameTime = 250;
	m_stFrame[99][Type::Stop].m_sMaxFrame = 3;
	m_stFrame[99][Type::Move].m_sFrameTime = 80;
	m_stFrame[99][Type::Attack].m_sFrameTime = 120;
	m_stFrame[99][Type::Attack].m_sMaxFrame = 7;
	m_stFrame[99][Type::Damage].m_sFrameTime = 120;
	m_stFrame[99][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[99][Type::Dying].m_sFrameTime = 100;
	m_stFrame[99][Type::Dying].m_sMaxFrame = 9;

	m_stFrame[hb::shared::owner::AirElemental][Type::Stop].m_sFrameTime = 40;
	m_stFrame[hb::shared::owner::AirElemental][Type::Stop].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::AirElemental][Type::Move].m_sFrameTime = 80;
	m_stFrame[hb::shared::owner::AirElemental][Type::Attack].m_sFrameTime = 120;
	m_stFrame[hb::shared::owner::AirElemental][Type::Attack].m_sMaxFrame = 3;
	m_stFrame[hb::shared::owner::AirElemental][Type::Damage].m_sFrameTime = 150;
	m_stFrame[hb::shared::owner::AirElemental][Type::Damage].m_sMaxFrame = 7;
	m_stFrame[hb::shared::owner::AirElemental][Type::Dying].m_sFrameTime = 180;
	m_stFrame[hb::shared::owner::AirElemental][Type::Dying].m_sMaxFrame = 7;

}

void CMapData::Init()
{
	int x, y;
	m_dwFrameCheckTime = GameClock::GetTimeMS();
	m_dwFrameAdjustTime = 0;
	m_sPivotX = -1;
	m_sPivotY = -1;

	for (x = 0; x < MapDataSizeX; x++)
		for (y = 0; y < MapDataSizeY; y++)
			m_pData[x][y].Clear();

	for (x = 0; x < hb::shared::object_id::NpcMax; x++) {
		m_iObjectIDcacheLocX[x] = 0;
		m_iObjectIDcacheLocY[x] = 0;
	}
}

CMapData::~CMapData()
{
}

void CMapData::OpenMapDataFile(char* cFn)
{
	char cHeader[260];
	char* cp, * cpMapData;
	std::memset(cHeader, 0, sizeof(cHeader));
	FILE* f = fopen(cFn, "rb");
	if (!f) return;
	fread(cHeader, 1, 256, f);
	_bDecodeMapInfo(cHeader);
	cpMapData = new char[m_sMapSizeX * m_sMapSizeY * 10];
	fread(cpMapData, 1, m_sMapSizeX * m_sMapSizeY * 10, f);
	fclose(f);
	cp = cpMapData;
	for (int y = 0; y < m_sMapSizeY; y++)
	{
		for (int x = 0; x < m_sMapSizeX; x++)
		{
			std::memcpy(&m_tile[x][y].m_sTileSprite, cp, sizeof(short));
			cp += 2;
			std::memcpy(&m_tile[x][y].m_sTileSpriteFrame, cp, sizeof(short));
			cp += 2;
			std::memcpy(&m_tile[x][y].m_sObjectSprite, cp, sizeof(short));
			cp += 2;
			std::memcpy(&m_tile[x][y].m_sObjectSpriteFrame, cp, sizeof(short));
			cp += 2;
			m_tile[x][y].m_bIsMoveAllowed = ((*cp) & 0x80) == 0;
			m_tile[x][y].m_bIsTeleport = ((*cp) & 0x40) != 0;
			cp += 2;
		}
	}
	delete[] cpMapData;
}

void CMapData::_bDecodeMapInfo(char* pHeader)
{
	for (int i = 0; i < 256; i++)
		if (pHeader[i] == 0) pHeader[i] = ' ';

	constexpr std::string_view seps = "= ,\t\n";
	std::string_view input(pHeader, 256);
	char cReadMode = 0;

	size_t pos = input.find_first_not_of(seps);
	while (pos != std::string_view::npos)
	{
		size_t end = input.find_first_of(seps, pos);
		std::string_view token = input.substr(pos, end - pos);

		if (cReadMode != 0)
		{
			// token is null-terminated within pHeader, safe for atoi
			switch (cReadMode)
			{
			case 1:
				std::from_chars(token.data(), token.data() + token.size(), m_sMapSizeX);
				cReadMode = 0;
				break;
			case 2:
				std::from_chars(token.data(), token.data() + token.size(), m_sMapSizeY);
				cReadMode = 0;
				break;
			}
		}
		else
		{
			if (token == "MAPSIZEX") cReadMode = 1;
			if (token == "MAPSIZEY") cReadMode = 2;
		}
		pos = (end == std::string_view::npos) ? end : input.find_first_not_of(seps, end);
	}
}

void CMapData::ShiftMapData(char cDir)
{
	int ix, iy;
	for (iy = 0; iy < MapDataSizeY; iy++)
		for (ix = 0; ix < MapDataSizeX; ix++)
			m_pTmpData[ix][iy].Clear();

	switch (cDir) {
	case 1: // North
		for (ix = 0; ix < hb::shared::view::InitDataTilesX + 1; ix++)
			for (iy = 0; iy < hb::shared::view::InitDataTilesY; iy++)
				memcpy(&m_pTmpData[hb::shared::view::MapDataBufferX + ix][hb::shared::view::MapDataBufferY + 1 + iy], &m_pData[hb::shared::view::MapDataBufferX + ix][hb::shared::view::MapDataBufferY + iy], sizeof(class CTile));
		m_sPivotY--;
		break;
	case 2: // NE
		for (ix = 0; ix < hb::shared::view::InitDataTilesX; ix++)
			for (iy = 0; iy < hb::shared::view::InitDataTilesY; iy++)
				memcpy(&m_pTmpData[hb::shared::view::MapDataBufferX + ix][hb::shared::view::MapDataBufferY + 1 + iy], &m_pData[hb::shared::view::MapDataBufferY + ix][hb::shared::view::MapDataBufferY + iy], sizeof(class CTile));
		m_sPivotX++;
		m_sPivotY--;
		break;
	case 3: // East
		for (ix = 0; ix < hb::shared::view::InitDataTilesX; ix++)
			for (iy = 0; iy < hb::shared::view::InitDataTilesY + 1; iy++)
				memcpy(&m_pTmpData[hb::shared::view::MapDataBufferX + ix][hb::shared::view::MapDataBufferY + iy], &m_pData[hb::shared::view::MapDataBufferY + ix][hb::shared::view::MapDataBufferY + iy], sizeof(class CTile));
		m_sPivotX++;
		break;
	case 4: // SE
		for (ix = 0; ix < hb::shared::view::InitDataTilesX; ix++)
			for (iy = 0; iy < hb::shared::view::InitDataTilesY; iy++)
				memcpy(&m_pTmpData[hb::shared::view::MapDataBufferX + ix][hb::shared::view::MapDataBufferY + iy], &m_pData[hb::shared::view::MapDataBufferY + ix][hb::shared::view::MapDataBufferY + 1 + iy], sizeof(class CTile));
		m_sPivotX++;
		m_sPivotY++;
		break;
	case 5: // South
		for (ix = 0; ix < hb::shared::view::InitDataTilesX + 1; ix++)
			for (iy = 0; iy < hb::shared::view::InitDataTilesY; iy++)
				memcpy(&m_pTmpData[hb::shared::view::MapDataBufferX + ix][hb::shared::view::MapDataBufferY + iy], &m_pData[hb::shared::view::MapDataBufferX + ix][hb::shared::view::MapDataBufferY + 1 + iy], sizeof(class CTile));
		m_sPivotY++;
		break;
	case 6: // SW
		for (ix = 0; ix < hb::shared::view::InitDataTilesX; ix++)
			for (iy = 0; iy < hb::shared::view::InitDataTilesY; iy++)
				memcpy(&m_pTmpData[hb::shared::view::MapDataBufferY + ix][hb::shared::view::MapDataBufferY + iy], &m_pData[hb::shared::view::MapDataBufferX + ix][hb::shared::view::MapDataBufferY + 1 + iy], sizeof(class CTile));
		m_sPivotX--;
		m_sPivotY++;
		break;
	case 7: // West
		for (ix = 0; ix < hb::shared::view::InitDataTilesX; ix++)
			for (iy = 0; iy < hb::shared::view::InitDataTilesY + 1; iy++)
				memcpy(&m_pTmpData[hb::shared::view::MapDataBufferY + ix][hb::shared::view::MapDataBufferY + iy], &m_pData[hb::shared::view::MapDataBufferX + ix][hb::shared::view::MapDataBufferY + iy], sizeof(class CTile));
		m_sPivotX--;
		break;
	case 8: // NW
		for (ix = 0; ix < hb::shared::view::InitDataTilesX; ix++)
			for (iy = 0; iy < hb::shared::view::InitDataTilesY; iy++)
				memcpy(&m_pTmpData[hb::shared::view::MapDataBufferY + ix][hb::shared::view::MapDataBufferY + 1 + iy], &m_pData[hb::shared::view::MapDataBufferX + ix][hb::shared::view::MapDataBufferY + iy], sizeof(class CTile));
		m_sPivotX--;
		m_sPivotY--;
		break;
	}
	memcpy(&m_pData[0][0], &m_pTmpData[0][0], sizeof(m_pData));
}

bool CMapData::bGetIsLocateable(short sX, short sY)
{
	int dX, dY;
	if ((sX < m_sPivotX) || (sX > m_sPivotX + MapDataSizeX) ||
		(sY < m_sPivotY) || (sY > m_sPivotY + MapDataSizeY)) return false;
	dX = sX - m_sPivotX;
	dY = sY - m_sPivotY;
	//Helltrayn 28/05/09. A�adimos esto para corregir el bug MIM que cierra el cliente
	if (dX <= 0 || dY <= 0) return false;
	if (m_pData[dX][dY].m_sOwnerType != 0) return false;
	if (m_tile[sX][sY].m_bIsMoveAllowed == false) return false;
	if (m_pData[dX][dY].m_sDynamicObjectType == dynamic_object::Mineral1) return false; // 4
	if (m_pData[dX][dY].m_sDynamicObjectType == dynamic_object::Mineral2) return false; // 5

	if (m_pData[dX + 1][dY + 1].m_sOwnerType == hb::shared::owner::Wyvern) return false;
	if (m_pData[dX + 1][dY].m_sOwnerType == hb::shared::owner::Wyvern) return false;
	if ((dY > 0) && (m_pData[dX + 1][dY - 1].m_sOwnerType == hb::shared::owner::Wyvern)) return false;
	if (m_pData[dX][dY + 1].m_sOwnerType == hb::shared::owner::Wyvern) return false;
	if (m_pData[dX][dY].m_sOwnerType == hb::shared::owner::Wyvern) return false;
	if ((dY > 0) && (m_pData[dX][dY - 1].m_sOwnerType == hb::shared::owner::Wyvern)) return false;
	if ((dX > 0) && (m_pData[dX - 1][dY + 1].m_sOwnerType == hb::shared::owner::Wyvern)) return false;
	if ((dX > 0) && (m_pData[dX - 1][dY].m_sOwnerType == hb::shared::owner::Wyvern)) return false;
	if ((dX > 0) && (dY > 0) && (m_pData[dX - 1][dY - 1].m_sOwnerType == hb::shared::owner::Wyvern)) return false;
	if (m_pData[dX + 1][dY + 1].m_sOwnerType == hb::shared::owner::FireWyvern) return false;
	if (m_pData[dX + 1][dY].m_sOwnerType == hb::shared::owner::FireWyvern) return false;
	if ((dY > 0) && (m_pData[dX + 1][dY - 1].m_sOwnerType == hb::shared::owner::FireWyvern)) return false;
	if (m_pData[dX][dY + 1].m_sOwnerType == hb::shared::owner::FireWyvern) return false;
	if (m_pData[dX][dY].m_sOwnerType == hb::shared::owner::FireWyvern) return false;
	if ((dY > 0) && (m_pData[dX][dY - 1].m_sOwnerType == hb::shared::owner::FireWyvern)) return false;
	if ((dX > 0) && (m_pData[dX - 1][dY + 1].m_sOwnerType == hb::shared::owner::FireWyvern)) return false;
	if ((dX > 0) && (m_pData[dX - 1][dY].m_sOwnerType == hb::shared::owner::FireWyvern)) return false;
	if ((dX > 0) && (dY > 0) && (m_pData[dX - 1][dY - 1].m_sOwnerType == hb::shared::owner::FireWyvern)) return false;
	if (m_pData[dX + 1][dY + 1].m_sOwnerType == hb::shared::owner::Abaddon) return false;
	if (m_pData[dX + 1][dY].m_sOwnerType == hb::shared::owner::Abaddon) return false;
	if ((dY > 0) && (m_pData[dX + 1][dY - 1].m_sOwnerType == hb::shared::owner::Abaddon)) return false;
	if (m_pData[dX][dY + 1].m_sOwnerType == hb::shared::owner::Abaddon) return false;
	if (m_pData[dX][dY].m_sOwnerType == hb::shared::owner::Abaddon) return false;
	if ((dY > 0) && (m_pData[dX][dY - 1].m_sOwnerType == hb::shared::owner::Abaddon)) return false;
	if ((dX > 0) && (m_pData[dX - 1][dY + 1].m_sOwnerType == hb::shared::owner::Abaddon)) return false;
	if ((dX > 0) && (m_pData[dX - 1][dY].m_sOwnerType == hb::shared::owner::Abaddon)) return false;
	if ((dX > 0) && (dY > 0) && (m_pData[dX - 1][dY - 1].m_sOwnerType == hb::shared::owner::Abaddon)) return false;
	if (m_pData[dX + 1][dY + 1].m_sOwnerType == hb::shared::owner::Gate) return false;
	if (m_pData[dX + 1][dY].m_sOwnerType == hb::shared::owner::Gate) return false;
	if ((dY > 0) && (m_pData[dX + 1][dY - 1].m_sOwnerType == hb::shared::owner::Gate)) return false;
	if (m_pData[dX][dY + 1].m_sOwnerType == hb::shared::owner::Gate) return false;
	if (m_pData[dX][dY].m_sOwnerType == hb::shared::owner::Gate) return false;
	if ((dY > 0) && (m_pData[dX][dY - 1].m_sOwnerType == hb::shared::owner::Gate)) return false;
	if ((dX > 0) && (m_pData[dX - 1][dY + 1].m_sOwnerType == hb::shared::owner::Gate)) return false;
	if ((dX > 0) && (m_pData[dX - 1][dY].m_sOwnerType == hb::shared::owner::Gate)) return false;
	if ((dX > 0) && (dY > 0) && (m_pData[dX - 1][dY - 1].m_sOwnerType == hb::shared::owner::Gate)) return false;
	return true;
}

bool CMapData::bIsTeleportLoc(short sX, short sY)
{
	if ((sX < m_sPivotX) || (sX > m_sPivotX + MapDataSizeX) ||
		(sY < m_sPivotY) || (sY > m_sPivotY + MapDataSizeY)) return false;

	if (m_tile[sX][sY].m_bIsTeleport == false) return false;

	return true;
}

bool __fastcall CMapData::bSetOwner(uint16_t wObjectID, int sX, int sY, int sType, int cDir, const hb::shared::entity::PlayerAppearance& appearance, const hb::shared::entity::PlayerStatus& status, std::string& pName, short sAction, short sV1, short sV2, short sV3, int iPreLoc, int iFrame, short npcConfigId)
{
	int   iX, iY, dX, dY;
	int   iChatIndex, iAdd;
	std::string cTmpName;
	uint32_t dwTime;
	int   iEffectType, iEffectFrame, iEffectTotalFrame;
	bool  bUseAbsPos = false;
	uint16_t wOriginalObjectID = wObjectID;
	hb::shared::entity::PlayerStatus localStatus = status;
	hb::shared::entity::PlayerAppearance localAppearance = appearance;
	short localNpcConfigId = npcConfigId;
	// Track old motion offset for seamless tile transitions during continuous movement
	float fOldMotionOffsetX = 0.0f;
	float fOldMotionOffsetY = 0.0f;
	int8_t cOldMotionDir = 0;
	bool bHadOldMotion = false;

	if ((m_sPivotX == -1) || (m_sPivotY == -1)) return false;
	cTmpName.clear();
	cTmpName = pName;
	dwTime = m_dwFrameTime;
	iEffectType = iEffectFrame = iEffectTotalFrame = 0;
	if ((hb::shared::object_id::IsNearbyOffset(wObjectID)) &&
		((sAction == Type::Move) || (sAction == Type::Run) ||
			(sAction == Type::DamageMove) || (sAction == Type::Damage) ||
			(sAction == Type::Dying))) {
		if ((sX >= m_sPivotX) && (sX < m_sPivotX + MapDataSizeX) &&
			(sY >= m_sPivotY) && (sY < m_sPivotY + MapDataSizeY)) {
			bUseAbsPos = true;
		}
	}
	if ((!hb::shared::object_id::IsNearbyOffset(wObjectID))
		&& ((sX < m_sPivotX) || (sX >= m_sPivotX + MapDataSizeX)
			|| (sY < m_sPivotY) || (sY >= m_sPivotY + MapDataSizeY)))
	{
		if (m_iObjectIDcacheLocX[wObjectID] > 0)
		{
			iX = m_iObjectIDcacheLocX[wObjectID] - m_sPivotX;
			iY = m_iObjectIDcacheLocY[wObjectID] - m_sPivotY;
			if ((iX < 0) || (iX >= MapDataSizeX) || (iY < 0) || (iY >= MapDataSizeY))
			{
				m_iObjectIDcacheLocX[wObjectID] = 0;
				m_iObjectIDcacheLocY[wObjectID] = 0;
				return false;
			}

			if (m_pData[iX][iY].m_wObjectID == wObjectID)
			{
				m_pData[iX][iY].m_sOwnerType = 0;
				m_pData[iX][iY].m_sNpcConfigId = -1;
				m_pData[iX][iY].m_cOwnerName.clear();
				pName.clear();

				m_pGame->m_floatingText.Clear(m_pData[iX][iY].m_iChatMsg);
				m_pData[iX][iY].m_iChatMsg = 0;
				m_pData[iX][iY].m_iEffectType = 0;
				m_iObjectIDcacheLocX[wObjectID] = 0;
				m_iObjectIDcacheLocY[wObjectID] = 0;
				return false;
			}
		}
		else if (m_iObjectIDcacheLocX[wObjectID] < 0)
		{
			iX = abs(m_iObjectIDcacheLocX[wObjectID]) - m_sPivotX;
			iY = abs(m_iObjectIDcacheLocY[wObjectID]) - m_sPivotY;
			if ((iX < 0) || (iX >= MapDataSizeX) || (iY < 0) || (iY >= MapDataSizeY))
			{
				m_iObjectIDcacheLocX[wObjectID] = 0;
				m_iObjectIDcacheLocY[wObjectID] = 0;
				return false;
			}
			if ((m_pData[iX][iY].m_cDeadOwnerFrame == -1) && (m_pData[iX][iY].m_wDeadObjectID == wObjectID))
			{
				m_pData[iX][iY].m_cDeadOwnerFrame = 0;
				pName.clear();
				m_pGame->m_floatingText.Clear(m_pData[iX][iY].m_iDeadChatMsg);
				m_pData[iX][iY].m_iDeadChatMsg = 0;
				m_iObjectIDcacheLocX[wObjectID] = 0;
				m_iObjectIDcacheLocY[wObjectID] = 0;
				return false;
			}
		}

		for (iX = 0; iX < MapDataSizeX; iX++)
			for (iY = 0; iY < MapDataSizeY; iY++)
			{
				if (m_pData[iX][iY].m_wObjectID == wObjectID)
				{
					m_pData[iX][iY].m_sOwnerType = 0;
					m_pData[iX][iY].m_sNpcConfigId = -1;
					m_pData[iX][iY].m_cOwnerName.clear();
					pName.clear();
					m_pGame->m_floatingText.Clear(m_pData[iX][iY].m_iChatMsg);
					m_pData[iX][iY].m_iChatMsg = 0;
					m_iObjectIDcacheLocX[wObjectID] = 0;
					m_iObjectIDcacheLocY[wObjectID] = 0;
					m_pData[iX][iY].m_iEffectType = 0;
					return false;
				}

				if ((m_pData[iX][iY].m_cDeadOwnerFrame == -1) && (m_pData[iX][iY].m_wDeadObjectID == wObjectID))
				{
					m_pData[iX][iY].m_cDeadOwnerFrame = 0;
					pName.clear();
					m_pGame->m_floatingText.Clear(m_pData[iX][iY].m_iDeadChatMsg);
					m_pData[iX][iY].m_iDeadChatMsg = 0;
					m_iObjectIDcacheLocX[wObjectID] = 0;
					m_iObjectIDcacheLocY[wObjectID] = 0;
					return false;
				}
			}
		pName.clear();
		return false;
	}
	iChatIndex = 0;

	if ((!hb::shared::object_id::IsNearbyOffset(wObjectID)) && (sAction != Type::NullAction))
	{
		cTmpName.clear();
		cTmpName = pName;
		dX = sX - m_sPivotX;
		dY = sY - m_sPivotY;
		if (m_iObjectIDcacheLocX[wObjectID] > 0)
		{
			iX = m_iObjectIDcacheLocX[wObjectID] - m_sPivotX;
			iY = m_iObjectIDcacheLocY[wObjectID] - m_sPivotY;
			if ((iX < 0) || (iX >= MapDataSizeX) || (iY < 0) || (iY >= MapDataSizeY))
			{
				m_iObjectIDcacheLocX[wObjectID] = 0;
				m_iObjectIDcacheLocY[wObjectID] = 0;
				return false;
			}
			if (m_pData[iX][iY].m_wObjectID == wObjectID)
			{
				iChatIndex = m_pData[iX][iY].m_iChatMsg;
				iEffectType = m_pData[iX][iY].m_iEffectType;
				iEffectFrame = m_pData[iX][iY].m_iEffectFrame;
				iEffectTotalFrame = m_pData[iX][iY].m_iEffectTotalFrame;

				// Capture old motion offset and direction for seamless continuous movement
				if (m_pData[iX][iY].m_motion.bIsMoving) {
					fOldMotionOffsetX = m_pData[iX][iY].m_motion.fCurrentOffsetX;
					fOldMotionOffsetY = m_pData[iX][iY].m_motion.fCurrentOffsetY;
					cOldMotionDir = m_pData[iX][iY].m_motion.cDirection;
					bHadOldMotion = true;
				}

				m_pData[iX][iY].m_wObjectID = 0; //-1; v1.41
				m_pData[iX][iY].m_iChatMsg = 0; // v1.4
				m_pData[iX][iY].m_sOwnerType = 0;
				m_pData[iX][iY].m_sNpcConfigId = -1;
				m_pData[iX][iY].m_cOwnerName.clear();
				m_iObjectIDcacheLocX[wObjectID] = sX;
				m_iObjectIDcacheLocY[wObjectID] = sY;
				goto EXIT_SEARCH_LOOP;
			}
		}
		else if (m_iObjectIDcacheLocX[wObjectID] < 0)
		{
			iX = abs(m_iObjectIDcacheLocX[wObjectID]) - m_sPivotX;
			iY = abs(m_iObjectIDcacheLocY[wObjectID]) - m_sPivotY;
			if ((iX < 0) || (iX >= MapDataSizeX) || (iY < 0) || (iY >= MapDataSizeY))
			{
				m_iObjectIDcacheLocX[wObjectID] = 0;
				m_iObjectIDcacheLocY[wObjectID] = 0;
				return false;
			}
			if ((m_pData[iX][iY].m_cDeadOwnerFrame == -1) && (m_pData[iX][iY].m_wDeadObjectID == wObjectID))
			{
				iChatIndex = m_pData[iX][iY].m_iDeadChatMsg;
				iEffectType = m_pData[iX][iY].m_iEffectType;
				iEffectFrame = m_pData[iX][iY].m_iEffectFrame;
				iEffectTotalFrame = m_pData[iX][iY].m_iEffectTotalFrame;
				m_pData[iX][iY].m_wDeadObjectID = 0;
				m_pData[iX][iY].m_iDeadChatMsg = 0; // v1.4
				m_pData[iX][iY].m_sDeadOwnerType = 0;
				m_iObjectIDcacheLocX[wObjectID] = -1 * sX;
				m_iObjectIDcacheLocY[wObjectID] = -1 * sY;
				goto EXIT_SEARCH_LOOP;
			}
		}

		iAdd = 7;
		for (iX = sX - iAdd; iX <= sX + iAdd; iX++)
			for (iY = sY - iAdd; iY <= sY + iAdd; iY++)
			{
				if (iX < m_sPivotX) break;
				else if (iX >= m_sPivotX + MapDataSizeX) break;
				if (iY < m_sPivotY) break;
				else if (iY >= m_sPivotY + MapDataSizeY) break;
				if (m_pData[iX - m_sPivotX][iY - m_sPivotY].m_wObjectID == wObjectID)
				{
					iChatIndex = m_pData[iX - m_sPivotX][iY - m_sPivotY].m_iChatMsg;
					iEffectType = m_pData[iX - m_sPivotX][iY - m_sPivotY].m_iEffectType;
					iEffectFrame = m_pData[iX - m_sPivotX][iY - m_sPivotY].m_iEffectFrame;
					iEffectTotalFrame = m_pData[iX - m_sPivotX][iY - m_sPivotY].m_iEffectTotalFrame;
					m_pData[iX - m_sPivotX][iY - m_sPivotY].m_wObjectID = 0; //-1; v1.41
					m_pData[iX - m_sPivotX][iY - m_sPivotY].m_iChatMsg = 0;
					m_pData[iX - m_sPivotX][iY - m_sPivotY].m_sOwnerType = 0;
					m_pData[iX - m_sPivotX][iY - m_sPivotY].m_sNpcConfigId = -1;
					m_pData[iX - m_sPivotX][iY - m_sPivotY].m_iEffectType = 0;
					m_pData[iX - m_sPivotX][iY - m_sPivotY].m_cOwnerName.clear();
					m_iObjectIDcacheLocX[wObjectID] = sX;
					m_iObjectIDcacheLocY[wObjectID] = sY;
					goto EXIT_SEARCH_LOOP;
				}

				if (m_pData[iX - m_sPivotX][iY - m_sPivotY].m_wDeadObjectID == wObjectID)
				{
					iChatIndex = m_pData[iX - m_sPivotX][iY - m_sPivotY].m_iDeadChatMsg;
					iEffectType = m_pData[iX - m_sPivotX][iY - m_sPivotY].m_iEffectType;
					iEffectFrame = m_pData[iX - m_sPivotX][iY - m_sPivotY].m_iEffectFrame;
					iEffectTotalFrame = m_pData[iX - m_sPivotX][iY - m_sPivotY].m_iEffectTotalFrame;
					m_pData[iX - m_sPivotX][iY - m_sPivotY].m_wDeadObjectID = 0; //-1; v1.41
					m_pData[iX - m_sPivotX][iY - m_sPivotY].m_iDeadChatMsg = 0;
					m_pData[iX - m_sPivotX][iY - m_sPivotY].m_sDeadOwnerType = 0;
					m_pData[iX - m_sPivotX][iY - m_sPivotY].m_cDeadOwnerName.clear();
					m_iObjectIDcacheLocX[wObjectID] = -1 * sX;
					m_iObjectIDcacheLocY[wObjectID] = -1 * sY;
					goto EXIT_SEARCH_LOOP;
				}
			}
		m_iObjectIDcacheLocX[wObjectID] = sX;
		m_iObjectIDcacheLocY[wObjectID] = sY;
	}
	else
	{
		if (sAction != Type::NullAction)// ObjectID
			wObjectID = hb::shared::object_id::ToRealID(wObjectID);
		// v1.5 Crash
		if (hb::shared::object_id::IsNearbyOffset(wObjectID)) return false;
		if (m_iObjectIDcacheLocX[wObjectID] > 0)
		{
			iX = m_iObjectIDcacheLocX[wObjectID] - m_sPivotX;
			iY = m_iObjectIDcacheLocY[wObjectID] - m_sPivotY;
			if ((iX < 0) || (iX >= MapDataSizeX) || (iY < 0) || (iY >= MapDataSizeY))
			{
				m_iObjectIDcacheLocX[wObjectID] = 0;
				m_iObjectIDcacheLocY[wObjectID] = 0;
				return false;
			}
			if (m_pData[iX][iY].m_wObjectID == wObjectID)
			{
				dX = iX;
				dY = iY;
				if (bUseAbsPos) {
					dX = sX - m_sPivotX;
					dY = sY - m_sPivotY;
				}
				else {
					switch (sAction) {
					case Type::Run:
					case Type::Move:
					case Type::DamageMove:
					case Type::AttackMove:
						hb::shared::direction::ApplyOffset(cDir, dX, dY);
						break;
					default:
						break;
					}
				}
				if ((wObjectID != static_cast<WORD>(m_pGame->m_pPlayer->m_sPlayerObjectID))
					&& (m_pData[dX][dY].m_sOwnerType != 0) && (m_pData[dX][dY].m_wObjectID != wObjectID))
				{
					m_pGame->RequestFullObjectData(wObjectID);
					pName.clear();
					return false;
				}
				iChatIndex = m_pData[iX][iY].m_iChatMsg;
				if (sAction != Type::NullAction)
				{
					sType = m_pData[iX][iY].m_sOwnerType;
					localNpcConfigId = m_pData[iX][iY].m_sNpcConfigId;
					localAppearance = m_pData[iX][iY].m_appearance;
					localStatus = m_pData[iX][iY].m_status;
					iEffectType = m_pData[iX][iY].m_iEffectType;
					iEffectFrame = m_pData[iX][iY].m_iEffectFrame;
					iEffectTotalFrame = m_pData[iX][iY].m_iEffectTotalFrame;
				}
				cTmpName.clear();
				cTmpName = m_pData[iX][iY].m_cOwnerName;
				pName.clear();
				pName = m_pData[iX][iY].m_cOwnerName;
				m_pData[iX][iY].m_wObjectID = 0; //-1; v1.41
				m_pData[iX][iY].m_iChatMsg = 0;
				m_pData[iX][iY].m_sOwnerType = 0;
				m_pData[iX][iY].m_sNpcConfigId = -1;
				m_pData[iX][iY].m_iEffectType = 0;
				m_pData[iX][iY].m_cOwnerName.clear();
				m_iObjectIDcacheLocX[wObjectID] = dX + m_sPivotX;
				m_iObjectIDcacheLocY[wObjectID] = dY + m_sPivotY;
				goto EXIT_SEARCH_LOOP;
			}
		}
		else if (m_iObjectIDcacheLocX[wObjectID] < 0)
		{
			iX = abs(m_iObjectIDcacheLocX[wObjectID]) - m_sPivotX;
			iY = abs(m_iObjectIDcacheLocY[wObjectID]) - m_sPivotY;
			if ((iX < 0) || (iX >= MapDataSizeX) || (iY < 0) || (iY >= MapDataSizeY))
			{
				m_iObjectIDcacheLocX[wObjectID] = 0;
				m_iObjectIDcacheLocY[wObjectID] = 0;
				return false;
			}
			if ((m_pData[iX][iY].m_cDeadOwnerFrame == -1) && (m_pData[iX][iY].m_wDeadObjectID == wObjectID))
			{
				dX = iX;
				dY = iY;
				if (bUseAbsPos) {
					dX = sX - m_sPivotX;
					dY = sY - m_sPivotY;
				}
				else {
					switch (sAction) {
					case Type::Move:
					case Type::Run:
					case Type::DamageMove:
					case Type::AttackMove:
						hb::shared::direction::ApplyOffset(cDir, dX, dY);
						break;
					default:
						break;
					}
				}
				if ((wObjectID != static_cast<WORD>(m_pGame->m_pPlayer->m_sPlayerObjectID)) &&
					(m_pData[dX][dY].m_sOwnerType != 0) && (m_pData[dX][dY].m_wObjectID != wObjectID))
				{
					m_pGame->RequestFullObjectData(wObjectID);
					pName.clear();
					return false;
				}
				iChatIndex = m_pData[iX][iY].m_iDeadChatMsg;
				if (sAction != Type::NullAction) {
					sType = m_pData[iX][iY].m_sDeadOwnerType;
					localAppearance = m_pData[iX][iY].m_deadAppearance;
					localStatus = m_pData[iX][iY].m_deadStatus;
				}
				cTmpName.clear();
				cTmpName = m_pData[iX][iY].m_cDeadOwnerName;
				pName.clear();
				pName = m_pData[iX][iY].m_cDeadOwnerName;
				m_pData[iX][iY].m_wDeadObjectID = 0; // -1; v1.41
				m_pData[iX][iY].m_iDeadChatMsg = 0;
				m_pData[iX][iY].m_sDeadOwnerType = 0;
				m_pData[iX][iY].m_cDeadOwnerName.clear();
				m_iObjectIDcacheLocX[wObjectID] = -1 * (dX + m_sPivotX);
				m_iObjectIDcacheLocY[wObjectID] = -1 * (dY + m_sPivotY);
				goto EXIT_SEARCH_LOOP;
			}
		}

		for (iX = 0; iX < MapDataSizeX; iX++)
			for (iY = 0; iY < MapDataSizeY; iY++)
			{
				if (m_pData[iX][iY].m_wObjectID == wObjectID)
				{
					dX = iX;
					dY = iY;
					if (bUseAbsPos) {
						dX = sX - m_sPivotX;
						dY = sY - m_sPivotY;
					}
					else {
						switch (sAction) {
						case Type::Run:
						case Type::Move:
						case Type::DamageMove:
						case Type::AttackMove:
							hb::shared::direction::ApplyOffset(cDir, dX, dY);
							break;
						default:
							break;
						}
					}
					if ((wObjectID != static_cast<WORD>(m_pGame->m_pPlayer->m_sPlayerObjectID))
						&& (m_pData[dX][dY].m_sOwnerType != 0) && (m_pData[dX][dY].m_wObjectID != wObjectID))
					{
						m_pGame->RequestFullObjectData(wObjectID);
						pName.clear();
						return false;
					}
					iChatIndex = m_pData[iX][iY].m_iChatMsg;
					if (sAction != Type::NullAction) {
						sType = m_pData[iX][iY].m_sOwnerType;
						localNpcConfigId = m_pData[iX][iY].m_sNpcConfigId;
						localAppearance = m_pData[iX][iY].m_appearance;
						localStatus = m_pData[iX][iY].m_status;
						iEffectType = m_pData[iX][iY].m_iEffectType;
						iEffectFrame = m_pData[iX][iY].m_iEffectFrame;
						iEffectTotalFrame = m_pData[iX][iY].m_iEffectTotalFrame;
					}
					cTmpName.clear();
					cTmpName = m_pData[iX][iY].m_cOwnerName;
					pName.clear();
					pName = m_pData[iX][iY].m_cOwnerName;
					m_pData[iX][iY].m_wObjectID = 0; //-1; v1.41
					m_pData[iX][iY].m_iChatMsg = 0;
					m_pData[iX][iY].m_sOwnerType = 0;
					m_pData[iX][iY].m_sNpcConfigId = -1;
					m_pData[iX][iY].m_iEffectType = 0;
					m_pData[iX][iY].m_cOwnerName.clear();
					m_iObjectIDcacheLocX[wObjectID] = dX + m_sPivotX;
					m_iObjectIDcacheLocY[wObjectID] = dY + m_sPivotY;
					goto EXIT_SEARCH_LOOP;
				}
				if (m_pData[iX][iY].m_wDeadObjectID == wObjectID)
				{
					dX = iX;
					dY = iY;
					if (bUseAbsPos) {
						dX = sX - m_sPivotX;
						dY = sY - m_sPivotY;
					}
					else {
						switch (sAction) {
						case Type::Move:
						case Type::Run:
						case Type::DamageMove:
						case Type::AttackMove:
							hb::shared::direction::ApplyOffset(cDir, dX, dY);
							break;
						default:
							break;
						}
					}
					if ((wObjectID != static_cast<WORD>(m_pGame->m_pPlayer->m_sPlayerObjectID)) &&
						(m_pData[dX][dY].m_sOwnerType != 0) && (m_pData[dX][dY].m_wObjectID != wObjectID))
					{
						m_pGame->RequestFullObjectData(wObjectID);
						pName.clear();
						return false;
					}
					iChatIndex = m_pData[iX][iY].m_iDeadChatMsg;
					if (sAction != Type::NullAction) {
						sType = m_pData[iX][iY].m_sDeadOwnerType;
						localNpcConfigId = m_pData[iX][iY].m_sDeadNpcConfigId;
						localAppearance = m_pData[iX][iY].m_deadAppearance;
						localStatus = m_pData[iX][iY].m_deadStatus;
					}
					cTmpName.clear();
					cTmpName = m_pData[iX][iY].m_cDeadOwnerName;
					pName.clear();
					pName = m_pData[iX][iY].m_cDeadOwnerName;
					m_pData[iX][iY].m_wDeadObjectID = 0; //-1; v1.41
					m_pData[iX][iY].m_iDeadChatMsg = 0;
					m_pData[iX][iY].m_sDeadOwnerType = 0;
					m_pData[iX][iY].m_sDeadNpcConfigId = -1;
					m_pData[iX][iY].m_iEffectType = 0;
					m_pData[iX][iY].m_cDeadOwnerName.clear();
					m_iObjectIDcacheLocX[wObjectID] = -1 * (dX + m_sPivotX);
					m_iObjectIDcacheLocY[wObjectID] = -1 * (dY + m_sPivotY);
					goto EXIT_SEARCH_LOOP;
				}
			}
		if (ShouldRequestFullData(wObjectID, sX, sY)) {
			m_pGame->RequestFullObjectData(wObjectID);
		}
		pName.clear();
		return false;
	}

EXIT_SEARCH_LOOP:;

	if (iPreLoc == 0 && m_pData[dX][dY].m_sOwnerType != 0)
	{
		if (sAction == Type::Dying)
		{
			dX = sX - m_sPivotX;
			dY = sY - m_sPivotY;
		}
		if (m_pData[dX][dY].m_animation.cAction == Type::Dying)
		{
			m_pData[dX][dY].m_wDeadObjectID = m_pData[dX][dY].m_wObjectID;
			m_pData[dX][dY].m_sDeadOwnerType = m_pData[dX][dY].m_sOwnerType;
			m_pData[dX][dY].m_sDeadNpcConfigId = m_pData[dX][dY].m_sNpcConfigId;
			m_pData[dX][dY].m_cDeadDir = m_pData[dX][dY].m_animation.cDir;
			m_pData[dX][dY].m_deadAppearance = m_pData[dX][dY].m_appearance;
			m_pData[dX][dY].m_deadStatus = m_pData[dX][dY].m_status;
			m_pData[dX][dY].m_cDeadOwnerFrame = -1;
			m_pData[dX][dY].m_dwDeadOwnerTime = dwTime;
			m_pData[dX][dY].m_cDeadOwnerName = m_pData[dX][dY].m_cOwnerName;
			m_pData[dX][dY].m_iDeadChatMsg = m_pData[dX][dY].m_iChatMsg;
			m_pData[dX][dY].m_wObjectID = 0;
			m_pData[dX][dY].m_sOwnerType = 0;
			m_pData[dX][dY].m_sNpcConfigId = -1;
			m_pData[dX][dY].m_iChatMsg = 0;
			m_pData[dX][dY].m_cOwnerName.clear();
			m_iObjectIDcacheLocX[m_pData[dX][dY].m_wDeadObjectID] = -1 * m_iObjectIDcacheLocX[m_pData[dX][dY].m_wDeadObjectID];
			m_iObjectIDcacheLocY[m_pData[dX][dY].m_wDeadObjectID] = -1 * m_iObjectIDcacheLocY[m_pData[dX][dY].m_wDeadObjectID];

			if (m_pData[dX][dY].m_iEffectType != 0)
			{
				m_pData[dX][dY].m_iEffectType = 0;
				m_pData[dX][dY].m_iEffectFrame = 0;
				m_pData[dX][dY].m_iEffectTotalFrame = 0;
				m_pData[dX][dY].m_dwEffectTime = 0;
			}
		}
	}

	if (m_pData[dX][dY].m_sOwnerType != 0)
	{
		if ((wObjectID != static_cast<WORD>(m_pGame->m_pPlayer->m_sPlayerObjectID))
			&& (m_pData[dX][dY].m_wObjectID == static_cast<WORD>(m_pGame->m_pPlayer->m_sPlayerObjectID)))
		{
			m_pGame->RequestFullObjectData(wObjectID);
			return false;
		}
		else
		{
			return false;
		}
	}

	if (iPreLoc == 0)
	{
		m_pData[dX][dY].m_wObjectID = wObjectID;
		m_pData[dX][dY].m_sOwnerType = sType;
		m_pData[dX][dY].m_sNpcConfigId = localNpcConfigId;
		m_pData[dX][dY].m_animation.cDir = cDir;
		m_pData[dX][dY].m_appearance = localAppearance;
		m_pData[dX][dY].m_status = localStatus;
		m_pData[dX][dY].m_sV1 = sV1;
		m_pData[dX][dY].m_sV2 = sV2;
		m_pData[dX][dY].m_sV3 = sV3;
		m_pData[dX][dY].m_iEffectType = iEffectType;
		m_pData[dX][dY].m_iEffectFrame = iEffectFrame;
		m_pData[dX][dY].m_iEffectTotalFrame = iEffectTotalFrame;
		m_pData[dX][dY].m_cOwnerName.clear();
		m_pData[dX][dY].m_cOwnerName = cTmpName;
		if ((sAction != Type::NullAction) && (sAction != MsgType::Confirm) && (sAction != MsgType::Reject))
		{
			// Look up animation definition: players use PlayerAnim, NPCs use m_stFrame
			int16_t maxFrame, frameTime;
			bool loop;
			if (hb::shared::owner::IsPlayer(sType)) {
				const AnimDef& def = PlayerAnim::FromAction(static_cast<int8_t>(sAction));
				maxFrame = def.sMaxFrame;
				frameTime = def.sFrameTime;
				loop = def.bLoop;
			} else {
				maxFrame = m_stFrame[sType][sAction].m_sMaxFrame;
				frameTime = m_stFrame[sType][sAction].m_sFrameTime;
				loop = false; // All actions are one-shot; overflow triggers STOP transition + command unlock
			}
			m_pData[dX][dY].m_animation.SetAction(static_cast<int8_t>(sAction), cDir,
				maxFrame, frameTime, loop, static_cast<int8_t>(iFrame));

			// Initialize smooth movement interpolation for movement actions
			if (sAction == Type::Move || sAction == Type::Run ||
				sAction == Type::DamageMove || sAction == Type::AttackMove)
			{
				bool hasHaste = localStatus.bHaste;
				bool isFrozen = localStatus.bFrozen;
				uint32_t duration = EntityMotion::GetDurationForAction(sAction, hasHaste, isFrozen);

				if (m_pData[dX][dY].m_motion.IsMoving())
				{
					// Entity still interpolating previous tile — queue this move
					m_pData[dX][dY].m_motion.QueueMove(cDir, duration);
				}
				else if (bHadOldMotion)
				{
					// Seamless tile transition with direction-aware offset handling
					//
					// Key insight: Cardinal directions (N/S/E/W) only move ONE axis.
					// When changing from diagonal to cardinal, the "unused" axis has
					// a residual offset that would cause unwanted lateral movement.
					//
					// Solution: Only preserve offset in axes the NEW direction uses.
					// This may cause a small instant jump (1-3 pixels) but prevents
					// the "sliding" effect during cardinal movement.
					//
					int16_t stdStartX, stdStartY;
					EntityMotion::GetDirectionStartOffset(cDir, stdStartX, stdStartY);

					float newOffsetX, newOffsetY;

					// Determine which axes the new direction uses
					bool usesX = (stdStartX != 0);  // E/W/NE/SE/SW/NW use X
					bool usesY = (stdStartY != 0);  // N/S/NE/SE/SW/NW use Y

					if (cOldMotionDir == cDir)
					{
						// Same direction: full blending for smooth continuous movement
						newOffsetX = fOldMotionOffsetX + static_cast<float>(stdStartX);
						newOffsetY = fOldMotionOffsetY + static_cast<float>(stdStartY);
					}
					else
					{
						// Direction changed: only blend axes the new direction uses
						newOffsetX = usesX ? (fOldMotionOffsetX + static_cast<float>(stdStartX))
						                   : static_cast<float>(stdStartX);
						newOffsetY = usesY ? (fOldMotionOffsetY + static_cast<float>(stdStartY))
						                   : static_cast<float>(stdStartY);
					}

					m_pData[dX][dY].m_motion.StartMoveWithOffset(cDir, dwTime, duration, newOffsetX, newOffsetY);
				}
				else
				{
					// No previous motion - use standard start offset
					m_pData[dX][dY].m_motion.StartMove(cDir, dwTime, duration);
				}
			}
		}
		else
		{
			// NULLACTION/CONFIRM/REJECT: initialize with STOP animation if not already set
			if (m_pData[dX][dY].m_animation.sMaxFrame == 0)
			{
				int16_t maxFrame, frameTime;
				if (hb::shared::owner::IsPlayer(sType)) {
					maxFrame = PlayerAnim::Stop.sMaxFrame;
					frameTime = PlayerAnim::Stop.sFrameTime;
				} else {
					maxFrame = m_stFrame[sType][Type::Stop].m_sMaxFrame;
					frameTime = m_stFrame[sType][Type::Stop].m_sFrameTime;
				}
				m_pData[dX][dY].m_animation.SetAction(Type::Stop, cDir,
					maxFrame, frameTime, false);
			}
		}
		m_pData[dX][dY].m_iChatMsg = iChatIndex;
		if (localAppearance.iEffectType != 0)
		{
			m_pData[dX][dY].m_iEffectType = localAppearance.iEffectType;
			if (sAction == Type::NullAction)
			{
				m_pData[dX][dY].m_iEffectFrame = 0;
				m_pData[dX][dY].m_dwEffectTime = dwTime;
			}
			switch (m_pData[dX][dY].m_iEffectType) {
			case 1: m_pData[dX][dY].m_iEffectTotalFrame = 13; break;
			case 2: m_pData[dX][dY].m_iEffectTotalFrame = 11; break;
			}
		}
		else
		{
			m_pData[dX][dY].m_iEffectType = 0;
		}
	}
	else // iPreLoc == 1
	{
		m_pData[dX][dY].m_wDeadObjectID = wObjectID;
		m_pData[dX][dY].m_sDeadOwnerType = sType;
		m_pData[dX][dY].m_sDeadNpcConfigId = localNpcConfigId;
		m_pData[dX][dY].m_cDeadDir = cDir;
		m_pData[dX][dY].m_deadAppearance = localAppearance;
		m_pData[dX][dY].m_deadStatus = localStatus;
		m_pData[dX][dY].m_cDeadOwnerName.clear();
		m_pData[dX][dY].m_cDeadOwnerName = cTmpName;
		m_pData[dX][dY].m_dwDeadOwnerTime = dwTime;
		m_pData[dX][dY].m_iDeadChatMsg = iChatIndex;
		if (localAppearance.iEffectType != 0)
		{
			m_pData[dX][dY].m_iEffectType = localAppearance.iEffectType;
			if (sAction == Type::NullAction)
			{
				m_pData[dX][dY].m_iEffectFrame = 0;
				m_pData[dX][dY].m_dwEffectTime = dwTime;
			}
			switch (m_pData[dX][dY].m_iEffectType) {
			case 1: m_pData[dX][dY].m_iEffectTotalFrame = 13; break;
			case 2: m_pData[dX][dY].m_iEffectTotalFrame = 11; break;
			}
		}
		else
		{
			m_pData[dX][dY].m_iEffectType = 0;
		}
	}
	return true;
}






int CMapData::iObjectFrameCounter(const std::string& cPlayerName, short sViewPointX, short sViewPointY)
{
	int dX, dY, sVal;
	uint32_t dwTime, dwRealTime, dwFrameTime;
	int  iDelay;
	int  iRet, iSoundIndex;
	int  cDir, cTotalFrame, cFrameMoveDots;
	static DWORD S_dwUpdateTime = GameClock::GetTimeMS();
	int   sWeaponType, sCenterX, sCenterY, sDist;
	bool  bAutoUpdate = false, dynObjsNeedUpdate = false;
	short dx, dy;
	long  lPan;

	iRet = 0;
	dwTime = dwRealTime = GameClock::GetTimeMS();
	if ((dwTime - m_dwFrameTime) >= 1)
		m_dwFrameTime = dwTime;

	sVal = sViewPointX - (m_sPivotX * 32);
	sCenterX = (sVal / 32) + VIEW_CENTER_TILE_X();
	sVal = sViewPointY - (m_sPivotY * 32);
	sCenterY = (sVal / 32) + VIEW_CENTER_TILE_Y();
	m_sRectX = m_pGame->m_sVDL_X - m_sPivotX;
	m_sRectY = m_pGame->m_sVDL_Y - m_sPivotY;

	dynObjsNeedUpdate = (dwTime - m_dwDOframeTime) > 100;
	bAutoUpdate = (dwTime - S_dwUpdateTime) > 40;

	// PERFORMANCE OPTIMIZATION: Only process tiles near player's view
	// Screen is ~LOGICAL_WIDTHxLOGICAL_HEIGHT pixels = ~20x15 tiles, add buffer for effects
	// OLD: Processed all 60x55 = 3300 tiles every frame
	// NEW: Process only ~35x30 = 1050 tiles (68% reduction)
	int halfViewX = VIEW_TILE_WIDTH() / 2;
	int halfViewY = VIEW_TILE_HEIGHT() / 2;
	int bufferX = 5;
	int bufferY = 6;
	int startX = sCenterX - (halfViewX + bufferX);
	int endX = sCenterX + (halfViewX + bufferX + 1);
	int startY = sCenterY - (halfViewY + bufferY);
	int endY = sCenterY + (halfViewY + bufferY);
	if (startX < 0) startX = 0;
	if (startY < 0) startY = 0;
	if (endX > MapDataSizeX) endX = MapDataSizeX;
	if (endY > MapDataSizeY) endY = MapDataSizeY;

	for (dX = startX; dX < endX; dX++)
		for (dY = startY; dY < endY; dY++)
		{
			sDist = (abs(sCenterX - dX) + abs(sCenterY - dY)) / 2;
			lPan = halfViewX > 0 ? ((dX - sCenterX) * 100) / halfViewX : 0;

			// Dynamic Object
			if (dynObjsNeedUpdate)//00496B99  JBE 00496F43
			{
				m_pData[dX][dY].m_iEffectFrame++;
				switch (m_pData[dX][dY].m_iEffectType) {
				case 1:
					if (m_pData[dX][dY].m_iEffectTotalFrame < m_pData[dX][dY].m_iEffectFrame)
						m_pData[dX][dY].m_iEffectFrame = 4;
					break;
				case 2:
					if (m_pData[dX][dY].m_iEffectTotalFrame < m_pData[dX][dY].m_iEffectFrame)
						m_pData[dX][dY].m_iEffectFrame = 3;
					break;
				}
				if ((m_pData[dX][dY].m_sDynamicObjectType != 0))
				{
					m_pData[dX][dY].m_cDynamicObjectFrame++;
					switch (m_pData[dX][dY].m_sDynamicObjectType) {
					case dynamic_object::Spike:
						if (m_pData[dX][dY].m_cDynamicObjectFrame >= 13)
							m_pData[dX][dY].m_cDynamicObjectFrame = 0;
						break;

					case dynamic_object::IceStorm:
						if (m_pData[dX][dY].m_cDynamicObjectFrame >= 10)
							m_pData[dX][dY].m_cDynamicObjectFrame = 0;
						break;

					case dynamic_object::Fire:// Firewall
					case dynamic_object::Fire3: // by Snoopy(FireBow)
						if (m_pData[dX][dY].m_cDynamicObjectFrame >= 24)
							m_pData[dX][dY].m_cDynamicObjectFrame = 0;
						if (m_pData[dX][dY].m_cDynamicObjectFrame == 1)
						{
							m_pGame->PlayGameSound('E', 9, sDist);
						}
						break;

					case dynamic_object::Fire2:	//  // Crusade buildings burning.
						if (m_pData[dX][dY].m_cDynamicObjectFrame > 27)
							m_pData[dX][dY].m_cDynamicObjectFrame = 0;
						if (m_pData[dX][dY].m_cDynamicObjectFrame == 1)
						{
							m_pGame->PlayGameSound('E', 9, sDist);
						}
						if ((m_pData[dX][dY].m_cDynamicObjectFrame % 6) == 0)
						{
							m_pGame->m_pEffectManager->AddEffect(EffectType::MS_CRUSADE_CASTING, (m_sPivotX + dX) * 32 + (rand() % 10 - 5) + 5, (m_sPivotY + dY) * 32, 0, 0, 0, 0);
							m_pGame->m_pEffectManager->AddEffect(EffectType::MS_FIRE_SMOKE, (m_sPivotX + dX) * 32, (m_sPivotY + dY) * 32, 0, 0, 0, 0);
						}
						break;

					case dynamic_object::FishObject:
						if ((rand() % 12) == 1)
							m_pGame->m_pEffectManager->AddEffect(EffectType::BUBBLES_DRUNK, (m_sPivotX + dX) * 32 + m_pData[dX][dY].m_cDynamicObjectData1, (m_sPivotY + dY) * 32 + m_pData[dX][dY].m_cDynamicObjectData2, 0, 0, 0);
						break;

					case dynamic_object::Fish:
						if ((dwTime - m_pData[dX][dY].m_dwDynamicObjectTime) < 100) break;
						m_pData[dX][dY].m_dwDynamicObjectTime = dwTime;
						if (m_pData[dX][dY].m_cDynamicObjectFrame >= 15) m_pData[dX][dY].m_cDynamicObjectFrame = 0;
						if ((rand() % 15) == 1) m_pGame->m_pEffectManager->AddEffect(EffectType::BUBBLES_DRUNK, (m_sPivotX + dX) * 32 + m_pData[dX][dY].m_cDynamicObjectData1, (m_sPivotY + dY) * 32 + m_pData[dX][dY].m_cDynamicObjectData2, 0, 0, 0);
						cDir = CMisc::cGetNextMoveDir(m_pData[dX][dY].m_cDynamicObjectData1, m_pData[dX][dY].m_cDynamicObjectData2, 0, 0);
						switch (cDir) {
						case 1:
							m_pData[dX][dY].m_cDynamicObjectData4 -= 2;
							break;
						case 2:
							m_pData[dX][dY].m_cDynamicObjectData4 -= 2;
							m_pData[dX][dY].m_cDynamicObjectData3 += 2;
							break;
						case 3:
							m_pData[dX][dY].m_cDynamicObjectData3 += 2;
							break;
						case 4:
							m_pData[dX][dY].m_cDynamicObjectData3 += 2;
							m_pData[dX][dY].m_cDynamicObjectData4 += 2;
							break;
						case 5:
							m_pData[dX][dY].m_cDynamicObjectData4 += 2;
							break;
						case 6:
							m_pData[dX][dY].m_cDynamicObjectData3 -= 2;
							m_pData[dX][dY].m_cDynamicObjectData4 += 2;
							break;
						case 7:
							m_pData[dX][dY].m_cDynamicObjectData3 -= 2;
							break;
						case 8:
							m_pData[dX][dY].m_cDynamicObjectData3 -= 2;
							m_pData[dX][dY].m_cDynamicObjectData4 -= 2;
							break;
						}

						if (m_pData[dX][dY].m_cDynamicObjectData3 < -12) m_pData[dX][dY].m_cDynamicObjectData3 = -12;
						if (m_pData[dX][dY].m_cDynamicObjectData3 > 12) m_pData[dX][dY].m_cDynamicObjectData3 = 12;
						if (m_pData[dX][dY].m_cDynamicObjectData4 < -12) m_pData[dX][dY].m_cDynamicObjectData4 = -12;
						if (m_pData[dX][dY].m_cDynamicObjectData4 > 12) m_pData[dX][dY].m_cDynamicObjectData4 = 12;

						m_pData[dX][dY].m_cDynamicObjectData1 += m_pData[dX][dY].m_cDynamicObjectData3;
						m_pData[dX][dY].m_cDynamicObjectData2 += m_pData[dX][dY].m_cDynamicObjectData4;
						break;

					case dynamic_object::PCloudBegin:
						if (m_pData[dX][dY].m_cDynamicObjectFrame >= 8)
						{
							m_pData[dX][dY].m_sDynamicObjectType = dynamic_object::PCloudLoop;
							m_pData[dX][dY].m_cDynamicObjectFrame = rand() % 8;
						}
						break;

					case dynamic_object::PCloudLoop:
						if (m_pData[dX][dY].m_cDynamicObjectFrame >= 8)
						{
							m_pData[dX][dY].m_cDynamicObjectFrame = 0;
						}
						break;

					case dynamic_object::PCloudEnd:
						if (m_pData[dX][dY].m_cDynamicObjectFrame >= 8)
						{
							m_pData[dX][dY].m_sDynamicObjectType = 0;
						}
						break;

					case dynamic_object::AresdenFlag1:
						if (m_pData[dX][dY].m_cDynamicObjectFrame >= 4)
						{
							m_pData[dX][dY].m_cDynamicObjectFrame = 0;
						}
						break;

					case dynamic_object::ElvineFlag1:
						if (m_pData[dX][dY].m_cDynamicObjectFrame >= 8)
						{
							m_pData[dX][dY].m_cDynamicObjectFrame = 4;
						}
						break;
					}
				}
			}

			// Dead think 00496F43
			if (m_pData[dX][dY].m_sDeadOwnerType != 0) //00496F62  JE SHORT 00496FD8
				if ((m_pData[dX][dY].m_cDeadOwnerFrame >= 0) && ((dwTime - m_pData[dX][dY].m_dwDeadOwnerTime) > 150))
				{
					m_pData[dX][dY].m_dwDeadOwnerTime = dwTime;
					m_pData[dX][dY].m_cDeadOwnerFrame++;
					if (iRet == 0)
					{
						iRet = -1;
						S_dwUpdateTime = dwTime;
					}
					if (m_pData[dX][dY].m_cDeadOwnerFrame > 10)
					{
						m_pData[dX][dY].m_wDeadObjectID = 0;
						m_pData[dX][dY].m_sDeadOwnerType = 0;
						m_pData[dX][dY].m_cDeadOwnerName.clear();
					}
				}

			// Alive thing 00496FD8
			if (m_pData[dX][dY].m_sOwnerType != 0)
			{
				// Get base frame time from source (not the already-modified sFrameTime)
				int16_t baseFrameTime;
				short ownerType = m_pData[dX][dY].m_sOwnerType;
				int8_t ownerAction = m_pData[dX][dY].m_animation.cAction;
				if (hb::shared::owner::IsPlayer(ownerType)) {
					baseFrameTime = PlayerAnim::FromAction(ownerAction).sFrameTime;
				} else {
					baseFrameTime = m_stFrame[ownerType][ownerAction].m_sFrameTime;
				}

				// Compute effective frame time with status modifiers
				switch (ownerAction) {
				case Type::Attack: // 3
				case Type::AttackMove:	// 8
					iDelay = m_pData[dX][dY].m_status.iAttackDelay * 12;
					break;
				case Type::Magic: // 4
					if (m_pGame->m_pPlayer->m_iSkillMastery[4] == 100) iDelay = -17;
					else iDelay = 0;
					break;
				default:
					iDelay = 0;
					break;
				}
				// v1.42 Frozen
				if (m_pData[dX][dY].m_status.bFrozen)
					iDelay += baseFrameTime >> 2;

				if (m_pData[dX][dY].m_status.bHaste) { // haste
					int16_t runFrameTime = hb::shared::owner::IsPlayer(ownerType)
						? PlayerAnim::Run.sFrameTime
						: m_stFrame[ownerType][Type::Run].m_sFrameTime;
					iDelay -= static_cast<int>(runFrameTime / 2.3);
				}

				// Apply computed delay to animation state
				dwFrameTime = baseFrameTime + iDelay;
				m_pData[dX][dY].m_animation.sFrameTime = static_cast<int16_t>(dwFrameTime);

				if (m_pData[dX][dY].m_animation.Update(dwTime))
				{
					if (iRet == 0)
					{
						iRet = -1;
						S_dwUpdateTime = dwTime;
					}
					if (m_pData[dX][dY].m_cOwnerName == cPlayerName)
					{
						iRet = 1;
						S_dwUpdateTime = dwTime;
						if ((dwRealTime - m_dwFrameCheckTime) > dwFrameTime)
							m_dwFrameAdjustTime = ((dwRealTime - m_dwFrameCheckTime) - dwFrameTime);
						m_dwFrameCheckTime = dwRealTime;
					}
					if (m_pData[dX][dY].m_animation.IsFinished())
					{
						if ((m_sRectX <= dX) && ((m_sRectX + 25) >= dX)
							&& (m_sRectY <= dY) && ((m_sRectY + 19) >= dY))
							// (!) Ower -> DeadOwner 004971AB
						{
							if (m_pData[dX][dY].m_animation.cAction == Type::Dying) //10
							{
								m_pData[dX][dY].m_wDeadObjectID = m_pData[dX][dY].m_wObjectID;
								m_pData[dX][dY].m_sDeadOwnerType = m_pData[dX][dY].m_sOwnerType;
								m_pData[dX][dY].m_sDeadNpcConfigId = m_pData[dX][dY].m_sNpcConfigId;
								m_pData[dX][dY].m_cDeadDir = m_pData[dX][dY].m_animation.cDir;
								m_pData[dX][dY].m_deadAppearance = m_pData[dX][dY].m_appearance;
								m_pData[dX][dY].m_deadStatus = m_pData[dX][dY].m_status;
								m_pData[dX][dY].m_iDeadChatMsg = m_pData[dX][dY].m_iChatMsg; // v1.411
								m_pData[dX][dY].m_cDeadOwnerFrame = -1;
								m_pData[dX][dY].m_cDeadOwnerName = m_pData[dX][dY].m_cOwnerName;
								m_pData[dX][dY].m_wObjectID = 0;
								m_pData[dX][dY].m_sOwnerType = 0;
								m_pData[dX][dY].m_sNpcConfigId = -1;
								m_pData[dX][dY].m_cOwnerName.clear();
								m_iObjectIDcacheLocX[m_pData[dX][dY].m_wDeadObjectID] = -1 * m_iObjectIDcacheLocX[m_pData[dX][dY].m_wDeadObjectID];
								m_iObjectIDcacheLocY[m_pData[dX][dY].m_wDeadObjectID] = -1 * m_iObjectIDcacheLocY[m_pData[dX][dY].m_wDeadObjectID];
							}
							else
							{
								// Transition to STOP: use player or NPC anim defs
								int16_t stopMaxFrame, stopFrameTime;
								bool stopLoop;
								if (hb::shared::owner::IsPlayer(m_pData[dX][dY].m_sOwnerType)) {
									const AnimDef& def = PlayerAnim::FromAction(Type::Stop);
									stopMaxFrame = def.sMaxFrame;
									stopFrameTime = def.sFrameTime;
									stopLoop = def.bLoop;
								} else {
									stopMaxFrame = m_stFrame[m_pData[dX][dY].m_sOwnerType][Type::Stop].m_sMaxFrame;
									stopFrameTime = m_stFrame[m_pData[dX][dY].m_sOwnerType][Type::Stop].m_sFrameTime;
									stopLoop = false;
								}
								m_pData[dX][dY].m_animation.SetAction(Type::Stop,
									m_pData[dX][dY].m_animation.cDir,
									stopMaxFrame, stopFrameTime, stopLoop);
							}
							if (m_pData[dX][dY].m_cOwnerName == cPlayerName)
							{
								iRet = 2;
								S_dwUpdateTime = dwTime;
							}
						}
						else
						{
							m_pData[dX][dY].m_wObjectID = 0;
							m_pData[dX][dY].m_sOwnerType = 0;
							m_pData[dX][dY].m_sNpcConfigId = -1;
							m_pData[dX][dY].m_cOwnerName.clear();
							m_pGame->m_floatingText.Clear(m_pData[dX][dY].m_iChatMsg);
						}
					}
					if (m_pData[dX][dY].m_animation.cAction == Type::Stop) { // Type::Stop = 1 // 00497334
						switch (m_pData[dX][dY].m_sOwnerType) {
						case 1:
						case 2:
						case 3:
						case 4:
						case 5:
						case 6: // glowing armor/weapon
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1) || (m_pData[dX][dY].m_animation.cCurrentFrame == 5))
							{
								if ((((m_pData[dX][dY].m_appearance.iWeaponGlare | m_pData[dX][dY].m_appearance.iShieldGlare) != 0) || (m_pData[dX][dY].m_status.bGMMode)) && (!m_pData[dX][dY].m_status.bInvisibility))
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::STAR_TWINKLE, (m_sPivotX + dX) * 32 + (rand() % 20 - 10), (m_sPivotY + dY) * 32 - (rand() % 50) - 5, 0, 0, -(rand() % 8), 0);
								}
								// Snoopy: Angels
								if (((m_pData[dX][dY].m_status.iAngelPercent) > rand() % 6) // Angel stars
									&& (m_pData[dX][dY].m_status.HasAngelType())
									&& (!m_pData[dX][dY].m_status.bInvisibility))
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::STAR_TWINKLE, (m_sPivotX + dX) * 32 + (rand() % 15 + 10), (m_sPivotY + dY) * 32 - (rand() % 30) - 50, 0, 0, -(rand() % 8), 0);
								}
							}
							break;
						case hb::shared::owner::EnergyShield: // ESG
						case hb::shared::owner::GrandMagicGenerator: // GMG
						case hb::shared::owner::ManaStone: // ManaStone
							if ((rand() % 40) == 25)
							{
								m_pGame->m_pEffectManager->AddEffect(EffectType::STAR_TWINKLE, (m_sPivotX + dX) * 32 + (rand() % 60 - 30), (m_sPivotY + dY) * 32 - (rand() % 100) - 5, 0, 0, -(rand() % 12), 0);
								m_pGame->m_pEffectManager->AddEffect(EffectType::STAR_TWINKLE, (m_sPivotX + dX) * 32 + (rand() % 60 - 30), (m_sPivotY + dY) * 32 - (rand() % 100) - 5, 0, 0, -(rand() % 12), 0);
								m_pGame->m_pEffectManager->AddEffect(EffectType::STAR_TWINKLE, (m_sPivotX + dX) * 32 + (rand() % 60 - 30), (m_sPivotY + dY) * 32 - (rand() % 100) - 5, 0, 0, -(rand() % 12), 0);
								m_pGame->m_pEffectManager->AddEffect(EffectType::STAR_TWINKLE, (m_sPivotX + dX) * 32 + (rand() % 60 - 30), (m_sPivotY + dY) * 32 - (rand() % 100) - 5, 0, 0, -(rand() % 12), 0);
							}
							break;
						case hb::shared::owner::IceGolem: // IceGolem
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 3)
							{
								m_pGame->m_pEffectManager->AddEffect(EffectType::ICE_GOLEM_EFFECT_1, (m_sPivotX + dX) * 32 + (rand() % 40 - 20), (m_sPivotY + dY) * 32 + (rand() % 40 - 20), 0, 0, 0);
								m_pGame->m_pEffectManager->AddEffect(EffectType::ICE_GOLEM_EFFECT_1, (m_sPivotX + dX) * 32 + (rand() % 40 - 20), (m_sPivotY + dY) * 32 + (rand() % 40 - 20), 0, 0, 0);
							}
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
							{
								m_pGame->m_pEffectManager->AddEffect(EffectType::ICE_GOLEM_EFFECT_2, (m_sPivotX + dX) * 32 + (rand() % 40 - 20), (m_sPivotY + dY) * 32 + (rand() % 40 - 20), 0, 0, 0);
								m_pGame->m_pEffectManager->AddEffect(EffectType::ICE_GOLEM_EFFECT_2, (m_sPivotX + dX) * 32 + (rand() % 40 - 20), (m_sPivotY + dY) * 32 + (rand() % 40 - 20), 0, 0, 0);
							}
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
							{
								m_pGame->m_pEffectManager->AddEffect(EffectType::ICE_GOLEM_EFFECT_3, (m_sPivotX + dX) * 32 + (rand() % 40 - 20), (m_sPivotY + dY) * 32 + (rand() % 40 - 20), 0, 0, 0);
								m_pGame->m_pEffectManager->AddEffect(EffectType::ICE_GOLEM_EFFECT_3, (m_sPivotX + dX) * 32 + (rand() % 40 - 20), (m_sPivotY + dY) * 32 + (rand() % 40 - 20), 0, 0, 0);
							}
							break;
						}
					}

					if (m_pData[dX][dY].m_animation.cAction == Type::Move) { //2 //004977BF
						switch (m_pData[dX][dY].m_sOwnerType) {
						case 1:
						case 2:
						case 3:
						case 4:
						case 5:
						case 6:
						case hb::shared::owner::TempleKnight: // TK
						case hb::shared::owner::Beholder: // Beholder
						case hb::shared::owner::DarkElf: // Dark-Elf
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1) || (m_pData[dX][dY].m_animation.cCurrentFrame == 5))
							{
								m_pGame->PlayGameSound('C', 8, sDist, lPan);
								if ((((m_pData[dX][dY].m_appearance.iWeaponGlare | m_pData[dX][dY].m_appearance.iShieldGlare) != 0) || (m_pData[dX][dY].m_status.bGMMode)) && (!m_pData[dX][dY].m_status.bInvisibility))
								{
									cTotalFrame = 8;
									cFrameMoveDots = 32 / cTotalFrame;
									dx = dy = 0;
									switch (m_pData[dX][dY].m_animation.cDir) {
									case 1: dy = cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
									case 2: dy = cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); dx = -cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
									case 3: dx = -cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
									case 4: dx = -cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); dy = -cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
									case 5: dy = -cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
									case 6: dy = -cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); dx = cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
									case 7: dx = cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
									case 8: dx = cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); dy = cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
									}
									m_pGame->m_pEffectManager->AddEffect(EffectType::STAR_TWINKLE, (m_sPivotX + dX) * 32 + dx + (rand() % 20 - 10), (m_sPivotY + dY) * 32 + dy - (rand() % 50) - 5, 0, 0, -(rand() % 8), 0);
									m_pGame->m_pEffectManager->AddEffect(EffectType::STAR_TWINKLE, (m_sPivotX + dX) * 32 + dx + (rand() % 20 - 10), (m_sPivotY + dY) * 32 + dy - (rand() % 50) - 5, 0, 0, -(rand() % 8), 0);
								}
								// Snoopy: Angels
								if (((m_pData[dX][dY].m_status.iAngelPercent) > rand() % 6) // Angel stars
									&& (m_pData[dX][dY].m_status.HasAngelType())
									&& (!m_pData[dX][dY].m_status.bInvisibility))
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::STAR_TWINKLE, (m_sPivotX + dX) * 32 + (rand() % 15 + 10), (m_sPivotY + dY) * 32 - (rand() % 30) - 50, 0, 0, -(rand() % 8), 0);
								}
							}
							break;

						case hb::shared::owner::Sorceress: // Snoopy: Sorceress
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 149, sDist, lPan);
							break;

						case hb::shared::owner::ATK: // Snoopy: ATK
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 142, sDist, lPan);
							break;

						case hb::shared::owner::MasterElf: // Snoopy: MasterElf
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
							{
								if (true) m_pGame->PlayGameSound('C', 10, sDist, lPan);
							}
							break;

						case hb::shared::owner::DSK: // Snoopy: DSK
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 147, sDist, lPan);
							break;

						case hb::shared::owner::Slime: // Slime
						case hb::shared::owner::TigerWorm: // TW
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 1, sDist, lPan);
							break;

						case hb::shared::owner::Skeleton: // SKel
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 13, sDist, lPan);
							break;

						case hb::shared::owner::Cyclops: // Cyclops
						case hb::shared::owner::HellClaw: // HC
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 41, sDist, lPan);
							break;

						case hb::shared::owner::OrcMage: // Orc
						case hb::shared::owner::Stalker: // SK
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 9, sDist, lPan);
							break;

						case hb::shared::owner::GiantAnt: // Ant
						case hb::shared::owner::LightWarBeetle: // LWBeetle
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 29, sDist, lPan);
							break;

						case hb::shared::owner::Scorpion: // Scorpion
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 21, sDist, lPan);
							break;

						case hb::shared::owner::Zombie: // Zombie
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 17, sDist, lPan);
							break;

						case hb::shared::owner::Amphis: // Snake
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 25, sDist, lPan);
							break;

						case hb::shared::owner::ClayGolem: // Clay-Golem
						case hb::shared::owner::Gargoyle: // Gargoyle
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 37, sDist, lPan);
							break;

						case hb::shared::owner::Hellhound: // HH
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 5, sDist, lPan);
							break;

						case hb::shared::owner::Troll: // Troll
						case hb::shared::owner::Minaus: // Snoopy: Ajout Minaus
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 46, sDist, lPan);
							break;

						case hb::shared::owner::Ogre: // Ogre
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 51, sDist, lPan);
							break;

						case hb::shared::owner::Liche: // Liche
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 55, sDist, lPan);
							break;

						case hb::shared::owner::Demon: // DD
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 59, sDist, lPan);
							break;

						case hb::shared::owner::Unicorn: // Uni
						case hb::shared::owner::GodsHandKnightCK: // GHKABS
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 63, sDist, lPan);
							break;

						case hb::shared::owner::WereWolf: // WW
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 67, sDist, lPan);
							break;

						case hb::shared::owner::Bunny://Rabbit
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 71, sDist, lPan);
							break;

						case hb::shared::owner::Cat://Cat
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 72, sDist, lPan);
							break;

						case hb::shared::owner::GiantFrog://Giant-Frog
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 73, sDist, lPan);
							break;

						case hb::shared::owner::MountainGiant://Mountain Giant
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 87, sDist, lPan);
							break;

						case hb::shared::owner::Ettin://Ettin
						case hb::shared::owner::MasterOrc: // Snoopy: MasterMageOrc
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 91, sDist, lPan);
							break;

						case hb::shared::owner::CannibalPlant://Cannibal Plant
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 95, sDist, lPan);
							break;

						case hb::shared::owner::Rudolph://Rudolph
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('C', 11, sDist, lPan);
							break;

						case hb::shared::owner::DireBoar: // DireBoar
						case hb::shared::owner::GiantCrayfish: // Snoopy: GiantCrayFish
						case hb::shared::owner::Barbarian: // Snoopy: Barbarian
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 87, sDist, lPan);
							break;

						case hb::shared::owner::Frost://Frost
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 25, sDist, lPan);
							break;

						case hb::shared::owner::StoneGolem: // Stone-Golem
						case hb::shared::owner::BattleGolem: // BG
						case hb::shared::owner::IceGolem: // Snoopy: IceGolem
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 33, sDist, lPan);
							break;

						case hb::shared::owner::FireWyvern: // Snoopy: Fite-Wyvern
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 106, sDist, lPan);
							break;

						case hb::shared::owner::Tentocle: // Snoopy: Tentocle
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 110, sDist, lPan);
							break;

						case hb::shared::owner::ClawTurtle: // Snoopy: Claw Turtle
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 114, sDist, lPan);
							break;

						case hb::shared::owner::Centaur: // Snoopy: Centaurus
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 117, sDist, lPan);
							break;

						case hb::shared::owner::GiTree: // Snoopy: Giant Tree
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 122, sDist, lPan);
							break;

						case hb::shared::owner::GiLizard: // Snoopy: Giant Lizard
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 126, sDist, lPan);
							break;

						case hb::shared::owner::Dragon: // Snoopy: Dragon
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 130, sDist, lPan);
							break;

						case hb::shared::owner::Nizie: // Snoopy: Nizie
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 134, sDist, lPan);
							break;

						case hb::shared::owner::Abaddon: // void CGame::DrawDruncncity();Abaddon
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 136, sDist, lPan);
							break;

						default:
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1) || (m_pData[dX][dY].m_animation.cCurrentFrame == 3))
								m_pGame->PlayGameSound('C', 8, sDist, lPan);
							break;
						}
					} // Fin du Type::Move

					if (m_pData[dX][dY].m_animation.cAction == Type::Run)  // 2   //00497E34
					{
						switch (m_pData[dX][dY].m_sOwnerType) {
						case 1:
						case 2:
						case 3:
						case 4:
						case 5:
						case 6:
						case hb::shared::owner::GodsHandKnight: // GHK
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1) || (m_pData[dX][dY].m_animation.cCurrentFrame == 5))
							{
								cTotalFrame = 8;
								cFrameMoveDots = 32 / cTotalFrame;
								dx = dy = 0;
								switch (m_pData[dX][dY].m_animation.cDir) {
								case 1: dy = cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
								case 2: dy = cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); dx = -cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
								case 3: dx = -cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
								case 4: dx = -cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); dy = -cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
								case 5: dy = -cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
								case 6: dy = -cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); dx = cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
								case 7: dx = cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
								case 8: dx = cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); dy = cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
								}
								if (WeatherManager::Get().IsRaining())
									m_pGame->m_pEffectManager->AddEffect(EffectType::FOOTPRINT_RAIN, (m_sPivotX + dX) * 32 + dx, (m_sPivotY + dY) * 32 + dy, 0, 0, 0, 0);
								else m_pGame->m_pEffectManager->AddEffect(EffectType::FOOTPRINT, (m_sPivotX + dX) * 32 + dx, (m_sPivotY + dY) * 32 + dy, 0, 0, 0, 0);
								if ((((m_pData[dX][dY].m_appearance.iWeaponGlare | m_pData[dX][dY].m_appearance.iShieldGlare) != 0) || (m_pData[dX][dY].m_status.bGMMode)) && (!m_pData[dX][dY].m_status.bInvisibility))
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::STAR_TWINKLE, (m_sPivotX + dX) * 32 + dx + (rand() % 20 - 10), (m_sPivotY + dY) * 32 + dy - (rand() % 50) - 5, 0, 0, -(rand() % 8), 0);
									m_pGame->m_pEffectManager->AddEffect(EffectType::STAR_TWINKLE, (m_sPivotX + dX) * 32 + dx + (rand() % 20 - 10), (m_sPivotY + dY) * 32 + dy - (rand() % 50) - 5, 0, 0, -(rand() % 8), 0);
								}
								// Snoopy: Angels
								if (((m_pData[dX][dY].m_status.iAngelPercent) > rand() % 6) // Angel stars
									&& (m_pData[dX][dY].m_status.HasAngelType())
									&& (!m_pData[dX][dY].m_status.bInvisibility))
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::STAR_TWINKLE, (m_sPivotX + dX) * 32 + (rand() % 15 + 10), (m_sPivotY + dY) * 32 - (rand() % 30) - 50, 0, 0, -(rand() % 8), 0);
								}
								m_pGame->PlayGameSound('C', 10, sDist, lPan);
							}
							break;
						}
					}
					if (m_pData[dX][dY].m_animation.cAction == Type::AttackMove)  //8 //004980A5
					{
						switch (m_pData[dX][dY].m_sOwnerType) {
						case 1:
						case 2:
						case 3:
						case 4:
						case 5:
						case 6:
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 2) // vu comme case 2
							{
								if (true) m_pGame->PlayGameSound('C', 4, sDist); //bruit fleche
								cTotalFrame = 8;
								cFrameMoveDots = 32 / cTotalFrame;
								dx = dy = 0;
								switch (m_pData[dX][dY].m_animation.cDir) {
								case 1: dy = cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
								case 2: dy = cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); dx = -cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
								case 3: dx = -cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
								case 4: dx = -cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); dy = -cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
								case 5: dy = -cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
								case 6: dy = -cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); dx = cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
								case 7: dx = cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
								case 8: dx = cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); dy = cFrameMoveDots * (cTotalFrame - m_pData[dX][dY].m_animation.cCurrentFrame); break;
								}
								if ((((m_pData[dX][dY].m_appearance.iWeaponGlare | m_pData[dX][dY].m_appearance.iShieldGlare) != 0) || (m_pData[dX][dY].m_status.bGMMode)) && (!m_pData[dX][dY].m_status.bInvisibility))
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::STAR_TWINKLE, (m_sPivotX + dX) * 32 + dx + (rand() % 20 - 10), (m_sPivotY + dY) * 32 + dy - (rand() % 50) - 5, 0, 0, -(rand() % 8), 0);
									m_pGame->m_pEffectManager->AddEffect(EffectType::STAR_TWINKLE, (m_sPivotX + dX) * 32 + dx + (rand() % 20 - 10), (m_sPivotY + dY) * 32 + dy - (rand() % 50) - 5, 0, 0, -(rand() % 8), 0);
								}
								//Snoopy: Angels						
								if (((m_pData[dX][dY].m_status.iAngelPercent) > rand() % 6) // Angel stars
									&& (m_pData[dX][dY].m_status.HasAngelType())
									&& (!m_pData[dX][dY].m_status.bInvisibility))
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::STAR_TWINKLE, (m_sPivotX + dX) * 32 + (rand() % 15 + 10), (m_sPivotY + dY) * 32 - (rand() % 30) - 50, 0, 0, -(rand() % 8), 0);
								}
							}
							else if (m_pData[dX][dY].m_animation.cCurrentFrame == 4) // vu comme case 4
							{
								if (WeatherManager::Get().IsRaining())
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::FOOTPRINT_RAIN, (m_sPivotX + dX) * 32 + ((rand() % 20) - 10), (m_sPivotY + dY) * 32 + ((rand() % 20) - 10), 0, 0, 0, 0);
									m_pGame->m_pEffectManager->AddEffect(EffectType::FOOTPRINT_RAIN, (m_sPivotX + dX) * 32 + ((rand() % 20) - 10), (m_sPivotY + dY) * 32 + ((rand() % 20) - 10), 0, 0, 0, 0);
									m_pGame->m_pEffectManager->AddEffect(EffectType::FOOTPRINT_RAIN, (m_sPivotX + dX) * 32 + ((rand() % 20) - 10), (m_sPivotY + dY) * 32 + ((rand() % 20) - 10), 0, 0, 0, 0);
									m_pGame->m_pEffectManager->AddEffect(EffectType::FOOTPRINT_RAIN, (m_sPivotX + dX) * 32 + ((rand() % 20) - 10), (m_sPivotY + dY) * 32 + ((rand() % 20) - 10), 0, 0, 0, 0);
									m_pGame->m_pEffectManager->AddEffect(EffectType::FOOTPRINT_RAIN, (m_sPivotX + dX) * 32 + ((rand() % 20) - 10), (m_sPivotY + dY) * 32 + ((rand() % 20) - 10), 0, 0, 0, 0);
								}
								else
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::FOOTPRINT, (m_sPivotX + dX) * 32 + ((rand() % 20) - 10), (m_sPivotY + dY) * 32 + ((rand() % 20) - 10), 0, 0, 0, 0);
									m_pGame->m_pEffectManager->AddEffect(EffectType::FOOTPRINT, (m_sPivotX + dX) * 32 + ((rand() % 20) - 10), (m_sPivotY + dY) * 32 + ((rand() % 20) - 10), 0, 0, 0, 0);
									m_pGame->m_pEffectManager->AddEffect(EffectType::FOOTPRINT, (m_sPivotX + dX) * 32 + ((rand() % 20) - 10), (m_sPivotY + dY) * 32 + ((rand() % 20) - 10), 0, 0, 0, 0);
									m_pGame->m_pEffectManager->AddEffect(EffectType::FOOTPRINT, (m_sPivotX + dX) * 32 + ((rand() % 20) - 10), (m_sPivotY + dY) * 32 + ((rand() % 20) - 10), 0, 0, 0, 0);
									m_pGame->m_pEffectManager->AddEffect(EffectType::FOOTPRINT, (m_sPivotX + dX) * 32 + ((rand() % 20) - 10), (m_sPivotY + dY) * 32 + ((rand() % 20) - 10), 0, 0, 0, 0);
								}
								if (true) m_pGame->PlayGameSound('C', 11, sDist, lPan);
							}
							else if (m_pData[dX][dY].m_animation.cCurrentFrame == 5) // vu comme case 5
							{
								sWeaponType = m_pData[dX][dY].m_appearance.iWeaponType;
								if ((sWeaponType >= 1) && (sWeaponType <= 2))
								{
									m_pGame->PlayGameSound('C', 1, sDist, lPan);
								}
								else if ((sWeaponType >= 3) && (sWeaponType <= 19))
								{
									m_pGame->PlayGameSound('C', 2, sDist, lPan);
								}
								else if ((sWeaponType >= 20) && (sWeaponType <= 39))
								{
									m_pGame->PlayGameSound('C', 18, sDist, lPan);
								}
								else if ((sWeaponType >= 40) && (sWeaponType <= 59))
								{
									m_pGame->PlayGameSound('C', 3, sDist, lPan);
								}
							}
							break;
						}
					}

					if ((m_pData[dX][dY].m_animation.cAction == Type::Attack)) { //3 00498685
						switch (m_pData[dX][dY].m_sOwnerType) {
						case hb::shared::owner::IceGolem: // IceGolem
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
							{
								m_pGame->m_pEffectManager->AddEffect(EffectType::AURA_EFFECT_1, (m_sPivotX + dX) * 32, (m_sPivotY + dY) * 32, 0, 0, 0, 0);
							}
							break;
						case hb::shared::owner::CT: // void CGame::DrawDruncncity();Crossbow Turret (Heldenian)
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
							{
								m_pGame->m_pEffectManager->AddEffect(EffectType::GATE_ROUND, m_sPivotX + m_pData[dX][dY].m_sV1, m_sPivotY + m_pData[dX][dY].m_sV2
									, m_sPivotX + m_pData[dX][dY].m_sV1 + dX, m_sPivotY + m_pData[dX][dY].m_sV2 + dY, 0, 87);
								//m_pGame->PlayGameSound('E', 43, sDist, lPan); // Son "wouufffff"
							}
							break;
						case hb::shared::owner::AGC: // void CGame::DrawDruncncity();AGT (Heldenian)
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
							{
								m_pGame->m_pEffectManager->AddEffect(EffectType::ARROW_FLYING, m_sPivotX + m_pData[dX][dY].m_sV1, m_sPivotY + m_pData[dX][dY].m_sV2
									, m_sPivotX + m_pData[dX][dY].m_sV1 + dX, m_sPivotY + m_pData[dX][dY].m_sV2 + dY, 0, 89);
								//m_pGame->PlayGameSound('E', 43, sDist, lPan); // Son "wouufffff"
							}
							break;
						case 1:
						case 2:
						case 3:
						case 4:
						case 5:
						case 6: // Humans
							if ((m_pData[dX][dY].m_sV3 >= 20) && (m_pData[dX][dY].m_animation.cCurrentFrame == 2))
							{
								if (m_pGame->bHasHeroSet(m_pData[dX][dY].m_appearance, m_pData[dX][dY].m_sOwnerType) == 1) // Warr hero set
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::WAR_HERO_SET, m_sPivotX + dX, m_sPivotY + dY
										, m_sPivotX + dX, m_sPivotY + dY, 0, 1);
								}
								switch (m_pData[dX][dY].m_sOwnerType) {	// Son pour critiques
								case 1:
								case 2:
								case 3:
									if (true) m_pGame->PlayGameSound('C', 23, sDist, lPan); // Critical sound
									break;
								case 4:
								case 5:
								case 6:
									if (true) m_pGame->PlayGameSound('C', 24, sDist, lPan); // Critical sound
									break;
								}
							}
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
							{
								if (m_pData[dX][dY].m_appearance.bIsWalking) // not Peace mode
								{
									if (m_pData[dX][dY].m_sV3 != 1) // autre que corp � corp
									{
										m_pGame->m_pEffectManager->AddEffect(static_cast<EffectType>(m_pData[dX][dY].m_sV3), m_sPivotX + dX, m_sPivotY + dY
											, m_sPivotX + dX + m_pData[dX][dY].m_sV1, m_sPivotY + dY + m_pData[dX][dY].m_sV2
											, 0, m_pData[dX][dY].m_sOwnerType);
										if (m_pData[dX][dY].m_sV3 >= 20) m_pGame->PlayGameSound('E', 43, sDist, lPan); // Son "loup�"
									}
									if (m_pData[dX][dY].m_appearance.iWeaponType == 15) // StormBlade
									{
										m_pGame->m_pEffectManager->AddEffect(EffectType::STORM_BLADE, m_sPivotX + dX, m_sPivotY + dY
											, m_sPivotX + dX + m_pData[dX][dY].m_sV1, m_sPivotY + dY + m_pData[dX][dY].m_sV2
											, 0, m_pData[dX][dY].m_sOwnerType);
									}
									else
									{
										m_pGame->m_pEffectManager->AddEffect(EffectType::GATE_ROUND, m_sPivotX + dX, m_sPivotY + dY
											, m_sPivotX + dX + m_pData[dX][dY].m_sV1, m_sPivotY + dY + m_pData[dX][dY].m_sV2
											, 0, m_pData[dX][dY].m_sOwnerType);
									}
								}
								// Weapon Glare from appearance
								if ((((m_pData[dX][dY].m_appearance.iWeaponGlare | m_pData[dX][dY].m_appearance.iShieldGlare) != 0) || (m_pData[dX][dY].m_status.bGMMode)) && (!m_pData[dX][dY].m_status.bInvisibility))
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::STAR_TWINKLE, (m_sPivotX + dX) * 32 + (rand() % 20 - 10), (m_sPivotY + dY) * 32 - (rand() % 50) - 5, 0, 0, -(rand() % 8), 0);
									m_pGame->m_pEffectManager->AddEffect(EffectType::STAR_TWINKLE, (m_sPivotX + dX) * 32 + (rand() % 20 - 10), (m_sPivotY + dY) * 32 - (rand() % 50) - 5, 0, 0, -(rand() % 8), 0);
								}
								//Snoopy: Angels
								if (((m_pData[dX][dY].m_status.iAngelPercent) > rand() % 6) // Angel stars
									&& (m_pData[dX][dY].m_status.HasAngelType())
									&& (!m_pData[dX][dY].m_status.bInvisibility))
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::STAR_TWINKLE, (m_sPivotX + dX) * 32 + (rand() % 15 + 10), (m_sPivotY + dY) * 32 - (rand() % 30) - 50, 0, 0, -(rand() % 8), 0);
								}
							}
							break;

						default:
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
							{
								if (m_pData[dX][dY].m_sV3 == 2) // Arrow flying...
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::ARROW_FLYING, m_sPivotX + dX, m_sPivotY + dY
										, m_sPivotX + dX + m_pData[dX][dY].m_sV1
										, m_sPivotY + dY + m_pData[dX][dY].m_sV2
										, 0, m_pData[dX][dY].m_sOwnerType * 1000);
								}
							}
							break;
						}

						switch (m_pData[dX][dY].m_sOwnerType) {
						case 1:
						case 2:
						case 3:
						case 4:
						case 5:
						case 6:
							if (m_pData[dX][dY].m_appearance.bIsWalking)
							{
								sWeaponType = m_pData[dX][dY].m_appearance.iWeaponType;
								if ((sWeaponType >= 1) && (sWeaponType <= 2))
								{
									if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
									{
										m_pGame->PlayGameSound('C', 1, sDist, lPan);
									}
								}
								else if ((sWeaponType >= 3) && (sWeaponType <= 19))
								{
									if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
									{
										m_pGame->PlayGameSound('C', 2, sDist, lPan);
									}
								}
								else if ((sWeaponType >= 20) && (sWeaponType <= 39))
								{
									if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
									{
										m_pGame->PlayGameSound('C', 18, sDist, lPan);
									}
								}
								else if ((sWeaponType >= 40) && (sWeaponType <= 59))
								{
									if (m_pData[dX][dY].m_animation.cCurrentFrame == 3)
									{
										m_pGame->PlayGameSound('C', 3, sDist, lPan);
									}
								}
							}
							break;

						case hb::shared::owner::ATK: // Snoopy: ATK
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
								m_pGame->PlayGameSound('M', 140, sDist, lPan);
							break;

						case hb::shared::owner::MasterElf: // Snoopy: MasterElf
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
								m_pGame->PlayGameSound('C', 8, sDist, lPan);
							break;

						case hb::shared::owner::DSK: // Snoopy: DSK
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
								m_pGame->PlayGameSound('M', 145, sDist, lPan);
							break;

						case hb::shared::owner::Beholder: // Beholder
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
								m_pGame->PlayGameSound('E', 46, sDist, lPan);
							break;

						case hb::shared::owner::DarkElf: // DE
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
							{
								if (true) m_pGame->PlayGameSound('C', 3, sDist, lPan);
							}
							break;

						case hb::shared::owner::TigerWorm: // TW
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
							{
								if (true) m_pGame->PlayGameSound('C', 1, sDist, lPan);
							}
							break;

						case hb::shared::owner::Slime: // Slime
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
								m_pGame->PlayGameSound('M', 2, sDist, lPan);
							break;

						case hb::shared::owner::Skeleton: // Skell
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
								m_pGame->PlayGameSound('M', 14, sDist, lPan);
							break;

						case hb::shared::owner::StoneGolem: // Stone-Golem
						case hb::shared::owner::IceGolem: // ICeGolem
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
								m_pGame->PlayGameSound('M', 34, sDist, lPan);
							break;

						case hb::shared::owner::Cyclops: // Cyclops
						case hb::shared::owner::HellClaw: // HC
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
								m_pGame->PlayGameSound('M', 42, sDist, lPan);
							break;

						case hb::shared::owner::GodsHandKnight: // GHK
						case hb::shared::owner::GodsHandKnightCK: // GHKABS
						case hb::shared::owner::TempleKnight: // TK
						case hb::shared::owner::Gargoyle: // GG
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
							{
								if (true) m_pGame->PlayGameSound('C', 2, sDist, lPan);
							}
							break;

						case hb::shared::owner::OrcMage: // orc
						case hb::shared::owner::Stalker: // SK
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
								m_pGame->PlayGameSound('M', 10, sDist, lPan);
							break;

						case hb::shared::owner::GiantAnt: // Ant
						case hb::shared::owner::LightWarBeetle: // LWB
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
								m_pGame->PlayGameSound('M', 30, sDist, lPan);
							break;

						case hb::shared::owner::Scorpion: // Scorpion
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
								m_pGame->PlayGameSound('M', 22, sDist, lPan);
							break;

						case hb::shared::owner::Zombie: // Zombie
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
								m_pGame->PlayGameSound('M', 18, sDist, lPan);
							break;

						case hb::shared::owner::Amphis: // Snake
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
								m_pGame->PlayGameSound('M', 26, sDist, lPan);
							break;

						case hb::shared::owner::ClayGolem: // Clay-Golem
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
								m_pGame->PlayGameSound('M', 38, sDist, lPan);
							break;

						case hb::shared::owner::Hellhound: // HH
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
								m_pGame->PlayGameSound('M', 6, sDist, lPan);
							break;

						case hb::shared::owner::Troll: // Troll
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
								m_pGame->PlayGameSound('M', 47, sDist, lPan);
							break;

						case hb::shared::owner::Ogre: // Ogre
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
								m_pGame->PlayGameSound('M', 52, sDist, lPan);
							break;

						case hb::shared::owner::Liche: // Liche
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
								m_pGame->PlayGameSound('M', 56, sDist, lPan);
							break;

						case hb::shared::owner::Demon: // DD
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
								m_pGame->PlayGameSound('M', 60, sDist, lPan);
							break;

						case hb::shared::owner::Unicorn: // Uni
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
								m_pGame->PlayGameSound('M', 64, sDist, lPan);
							break;

						case hb::shared::owner::WereWolf: // WW
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
								m_pGame->PlayGameSound('M', 68, sDist, lPan);
							break;

						case hb::shared::owner::Bunny://Rabbit
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 75, sDist, lPan);
							break;

						case hb::shared::owner::Cat://Cat
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 76, sDist, lPan);
							break;

						case hb::shared::owner::GiantFrog://Giant-Frog
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 77, sDist, lPan);
							break;

						case hb::shared::owner::MountainGiant://Mountain Giant
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 88, sDist, lPan);
							break;

						case hb::shared::owner::Ettin://Ettin
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 92, sDist, lPan);
							break;

						case hb::shared::owner::CannibalPlant://Cannibal Plant
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 96, sDist, lPan);
							break;

						case hb::shared::owner::Rudolph://Rudolph
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
							{
								if (true) m_pGame->PlayGameSound('M', 38, sDist, lPan);
							}
							break;

						case hb::shared::owner::DireBoar://DireBoar
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 68, sDist, lPan);
							break;

						case hb::shared::owner::Frost://Frost
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
							{
								if (true) m_pGame->PlayGameSound('C', 4, sDist, lPan);
							}
							break;

						case hb::shared::owner::MasterOrc: // Snoopy: Master MageOrc
						case hb::shared::owner::Barbarian: // Snoopy: Barbarian
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 78, sDist, lPan);
							break;

						case hb::shared::owner::GiantCrayfish: // Snoopy: GiantCrayFish
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 100, sDist, lPan);
							break;

						case hb::shared::owner::FireWyvern: // Snoopy: Fire Wyvern
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 107, sDist, lPan);
							break;

						case hb::shared::owner::Tentocle: // Snoopy: Tentocle
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 111, sDist, lPan);
							break;

						case hb::shared::owner::Abaddon: // Snoopy: Abaddon
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 137, sDist, lPan);
							break;

						case hb::shared::owner::ClawTurtle: // Snoopy: Claw-Turtle
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 115, sDist, lPan);
							break;

						case hb::shared::owner::Centaur: // Snoopy: Centaurus
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 119, sDist, lPan);
							break;

						case hb::shared::owner::GiTree: // Snoopy: Giant-Tree
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 123, sDist, lPan);
							break;

						case hb::shared::owner::GiLizard: // Snoopy: GiantLizard
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 127, sDist, lPan);
							break;

						case hb::shared::owner::Dragon: // Snoopy: Dragon
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 131, sDist, lPan);
							break;

						case hb::shared::owner::Nizie: //Snoopy:  Nizie
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 135, sDist, lPan);
							break;

						case hb::shared::owner::Minaus: // Snoopy: Minaus
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 104, sDist, lPan);
							break;

						case hb::shared::owner::HBT: // Snoopy: Heavy BattleTank
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 151, sDist, lPan);
							break;

						case hb::shared::owner::CT: // Snoopy: Crosbow Turret
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 153, sDist, lPan);
							break;

						case hb::shared::owner::AGC: // Snoopy: Cannon Turret
							if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
								m_pGame->PlayGameSound('M', 155, sDist, lPan);
							break;

						case hb::shared::owner::Dummy: // Dummy
						case hb::shared::owner::EnergySphere: // Snoopy: EnergySphere
						default:
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 2) {
								if (true) m_pGame->PlayGameSound('C', 2, sDist, lPan);
							}
							break;
						}
					}

					if (m_pData[dX][dY].m_animation.cAction == Type::Damage)  // 6  00499159
					{
						switch (m_pData[dX][dY].m_sOwnerType) {
						case 1:
						case 2:
						case 3:  // Men
						case hb::shared::owner::GodsHandKnight: // GHK
						case hb::shared::owner::GodsHandKnightCK: // GHKABS
						case hb::shared::owner::TempleKnight: // TK
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 4)
							{
								if (m_pData[dX][dY].m_sV2 == -1)
									iSoundIndex = 5;
								else if (m_pData[dX][dY].m_sV2 == 0)
									iSoundIndex = 5;
								else if ((m_pData[dX][dY].m_sV2 >= 1) && (m_pData[dX][dY].m_sV2 <= 19))
									iSoundIndex = 6;
								else if ((m_pData[dX][dY].m_sV2 >= 20) && (m_pData[dX][dY].m_sV2 <= 39))
									iSoundIndex = 6;
								else if ((m_pData[dX][dY].m_sV2 >= 40) && (m_pData[dX][dY].m_sV2 <= 59))
									iSoundIndex = 7;
								else iSoundIndex = 5;
								if (true) m_pGame->PlayGameSound('C', iSoundIndex, sDist, lPan);
								m_pGame->m_pEffectManager->AddEffect(EffectType::NORMAL_HIT, m_sPivotX + dX, m_sPivotY + dY, 0, 0, 0, 4);
							}
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
							{
								if (true) m_pGame->PlayGameSound('C', 12, sDist, lPan);
							}
							break;
						case 4:
						case 5:
						case 6: // Women
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 4)
							{
								if (m_pData[dX][dY].m_sV2 == -1)
									iSoundIndex = 5;
								else if (m_pData[dX][dY].m_sV2 == 0)
									iSoundIndex = 5;
								else if ((m_pData[dX][dY].m_sV2 >= 1) && (m_pData[dX][dY].m_sV2 <= 19))
									iSoundIndex = 6;
								else if ((m_pData[dX][dY].m_sV2 >= 20) && (m_pData[dX][dY].m_sV2 <= 39))
									iSoundIndex = 6;
								else if ((m_pData[dX][dY].m_sV2 >= 40) && (m_pData[dX][dY].m_sV2 <= 59))
									iSoundIndex = 7;
								else iSoundIndex = 5;
								if (true) m_pGame->PlayGameSound('C', iSoundIndex, sDist, lPan);
								m_pGame->m_pEffectManager->AddEffect(EffectType::NORMAL_HIT, m_sPivotX + dX, m_sPivotY + dY, 0, 0, 0, 4);
							}
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
							{
								if (true) m_pGame->PlayGameSound('C', 13, sDist, lPan);
							}
							break;

						default:
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 4)
							{
								if (m_pData[dX][dY].m_sV2 == -1)
									iSoundIndex = 5;  // Hand Attack
								else if (m_pData[dX][dY].m_sV2 == 0)
									iSoundIndex = 5;  // Hand Attack
								else if ((m_pData[dX][dY].m_sV2 >= 1) && (m_pData[dX][dY].m_sV2 <= 19))
									iSoundIndex = 6;  // Blade hit
								else if ((m_pData[dX][dY].m_sV2 >= 20) && (m_pData[dX][dY].m_sV2 <= 39))
									iSoundIndex = 6;  // Blade hit
								else if ((m_pData[dX][dY].m_sV2 >= 40) && (m_pData[dX][dY].m_sV2 <= 59))
									iSoundIndex = 7; // Arrow hit
								else iSoundIndex = 5;

								if (true) m_pGame->PlayGameSound('C', iSoundIndex, sDist, lPan);
								if (iSoundIndex == 7) // Change the effect for Arrows hitting (no more at fixed heigh with arrow flying but on damage)
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::FOOTPRINT, (m_sPivotX + dX) * 32, (m_sPivotY + dY) * 32, 0, 0, 0, m_pData[dX][dY].m_sOwnerType);
								}
								else
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::NORMAL_HIT, m_sPivotX + dX, m_sPivotY + dY, 0, 0, 0, 4);
								}
							}

							switch (m_pData[dX][dY].m_sOwnerType) {
							case hb::shared::owner::Barbarian: // Snoopy: Barbarian
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 1 && true) m_pGame->PlayGameSound('M', 144, sDist, lPan);
								break;

							case hb::shared::owner::ATK: // Snoopy: ATK
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 1 && true) m_pGame->PlayGameSound('M', 143, sDist, lPan);
								break;

							case hb::shared::owner::MasterElf: // Snoopy: MasterElf
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 1) m_pGame->PlayGameSound('C', 7, sDist, lPan);
								break;

							case hb::shared::owner::DSK: // Snoopy: DSK
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 1) m_pGame->PlayGameSound('M', 148, sDist, lPan);
								break;

							case hb::shared::owner::DarkElf: // DE
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5 && true) m_pGame->PlayGameSound('C', 13, sDist, lPan);
								break;

							case hb::shared::owner::Slime: // Slime
							case hb::shared::owner::Beholder: // BB
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5) m_pGame->PlayGameSound('M', 3, sDist, lPan);
								break;

							case hb::shared::owner::Skeleton: // Skell
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5) m_pGame->PlayGameSound('M', 15, sDist, lPan);
								break;

							case hb::shared::owner::StoneGolem: // Stone-Golem
							case hb::shared::owner::IceGolem: // IceGolem
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5) m_pGame->PlayGameSound('M', 35, sDist, lPan);
								break;

							case hb::shared::owner::Cyclops: // Cyclops
							case hb::shared::owner::HellClaw: // HC
							case hb::shared::owner::Gargoyle: // GG
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5) m_pGame->PlayGameSound('M', 43, sDist, lPan);
								break;

							case hb::shared::owner::OrcMage: // Orc
							case hb::shared::owner::Stalker: // SK
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5) m_pGame->PlayGameSound('M', 11, sDist, lPan);
								break;

							case hb::shared::owner::GiantAnt: // Ant
							case hb::shared::owner::LightWarBeetle: // LWB
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5) m_pGame->PlayGameSound('M', 31, sDist, lPan);
								break;

							case hb::shared::owner::Scorpion: // Scorp
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5) m_pGame->PlayGameSound('M', 23, sDist, lPan);
								break;

							case hb::shared::owner::Zombie: // Zombie
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5) m_pGame->PlayGameSound('M', 19, sDist, lPan);
								break;

							case hb::shared::owner::Amphis: // Snake
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5) m_pGame->PlayGameSound('M', 27, sDist, lPan);
								break;

							case hb::shared::owner::ClayGolem: // Clay-Golem
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5) m_pGame->PlayGameSound('M', 39, sDist, lPan);
								break;

							case hb::shared::owner::Hellhound: // HH
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5) m_pGame->PlayGameSound('M', 7, sDist, lPan);
								break;

							case hb::shared::owner::Troll: // Troll
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5) m_pGame->PlayGameSound('M', 48, sDist, lPan);
								break;

							case hb::shared::owner::Ogre: // Ogre
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5) m_pGame->PlayGameSound('M', 53, sDist, lPan);
								break;

							case hb::shared::owner::Liche: // Liche
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5) m_pGame->PlayGameSound('M', 57, sDist, lPan);
								break;

							case hb::shared::owner::Demon: // DD
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5) m_pGame->PlayGameSound('M', 61, sDist, lPan);
								break;

							case hb::shared::owner::Unicorn: // Uni
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5) m_pGame->PlayGameSound('M', 65, sDist, lPan);
								break;

							case hb::shared::owner::WereWolf: // WW
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5) m_pGame->PlayGameSound('M', 69, sDist, lPan);
								break;

							case hb::shared::owner::Dummy: // dummy
							case hb::shared::owner::EnergySphere: // Snoopy: EnergyBall
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5) m_pGame->PlayGameSound('M', 2, sDist, lPan);
								break;

							case hb::shared::owner::Bunny://Rabbit
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1)) m_pGame->PlayGameSound('M', 79, sDist, lPan);
								break;

							case hb::shared::owner::Cat://Cat
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1)) m_pGame->PlayGameSound('M', 80, sDist, lPan);
								break;

							case hb::shared::owner::GiantFrog://Giant-Frog
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1)) m_pGame->PlayGameSound('M', 81, sDist, lPan);
								break;

							case hb::shared::owner::MountainGiant: // Mountain Giant
							case hb::shared::owner::MasterOrc: // Snoopy: MasterMageOrc
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1)) m_pGame->PlayGameSound('M', 89, sDist, lPan);
								break;

							case hb::shared::owner::Ettin://Ettin
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1)) m_pGame->PlayGameSound('M', 93, sDist, lPan);
								break;
							case hb::shared::owner::CannibalPlant://Cannabl Plant
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1)) m_pGame->PlayGameSound('M', 97, sDist, lPan);
								break;
							case hb::shared::owner::Rudolph://Rudolph
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1)) m_pGame->PlayGameSound('M', 69, sDist, lPan);
								break;
							case hb::shared::owner::DireBoar://DireBoar
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1)) m_pGame->PlayGameSound('M', 78, sDist, lPan);
								break;
							case hb::shared::owner::Frost://Frost
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 1) m_pGame->PlayGameSound('C', 13, sDist, lPan);
								break;

							case hb::shared::owner::GiantCrayfish: // Snoopy: Giant CrayFish
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1)) m_pGame->PlayGameSound('M', 101, sDist, lPan);
								break;

							case hb::shared::owner::Minaus: // Snoopy: Minaus
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1)) m_pGame->PlayGameSound('M', 102, sDist, lPan);
								break;

							case hb::shared::owner::Tentocle: // Snoopy: Tentocle
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1)) m_pGame->PlayGameSound('M', 108, sDist, lPan);
								break;

							case hb::shared::owner::Abaddon: // Snoopy: Abaddon
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1)) m_pGame->PlayGameSound('M', 138, sDist, lPan);
								break;

							case hb::shared::owner::ClawTurtle: // Snoopy: ClawTurtle
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1)) m_pGame->PlayGameSound('M', 112, sDist, lPan);
								break;

							case hb::shared::owner::Centaur: // Snoopy: Centaurus
							case hb::shared::owner::Sorceress: // Snoopy: Sorceress
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1)) m_pGame->PlayGameSound('M', 116, sDist, lPan);
								break;

							case hb::shared::owner::GiTree: // Snoopy: GiantTree
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1)) m_pGame->PlayGameSound('M', 120, sDist, lPan);
								break;

							case hb::shared::owner::GiLizard: // Snoopy: GiantLizard
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1)) m_pGame->PlayGameSound('M', 124, sDist, lPan);
								break;

							case hb::shared::owner::Dragon: // Snoopy: Dragon
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1)) m_pGame->PlayGameSound('M', 128, sDist, lPan);
								break;

							case hb::shared::owner::Nizie: // Snoopy: Nizie
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1)) m_pGame->PlayGameSound('M', 132, sDist, lPan);
								break;
							}
							break;
						}
					}

					if (m_pData[dX][dY].m_animation.cAction == Type::DamageMove) { // 7 004997BD
						switch (m_pData[dX][dY].m_sOwnerType) {
						case 1:
						case 2:
						case 3:
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
							{
								if (m_pData[dX][dY].m_sV2 == -1)
									iSoundIndex = 5;
								else if (m_pData[dX][dY].m_sV2 == 0)
									iSoundIndex = 5;
								else if ((m_pData[dX][dY].m_sV2 >= 1) && (m_pData[dX][dY].m_sV2 <= 19))
									iSoundIndex = 6;
								else if ((m_pData[dX][dY].m_sV2 >= 20) && (m_pData[dX][dY].m_sV2 <= 39))
									iSoundIndex = 6;
								else if ((m_pData[dX][dY].m_sV2 >= 40) && (m_pData[dX][dY].m_sV2 <= 59))
									iSoundIndex = 7;
								else iSoundIndex = 5;

								if (true) m_pGame->PlayGameSound('C', iSoundIndex, sDist, lPan);
								m_pGame->m_pEffectManager->AddEffect(EffectType::NORMAL_HIT, m_sPivotX + dX, m_sPivotY + dY, 0, 0, 0, 4);
							}
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
							{
								if (true) m_pGame->PlayGameSound('C', 12, sDist, lPan);
							}
							break;

						case 4:
						case 5:
						case 6:
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
							{
								if (m_pData[dX][dY].m_sV2 == -1)
									iSoundIndex = 5;
								else if (m_pData[dX][dY].m_sV2 == 0)
									iSoundIndex = 5;
								else if ((m_pData[dX][dY].m_sV2 >= 1) && (m_pData[dX][dY].m_sV2 <= 19))
									iSoundIndex = 6;
								else if ((m_pData[dX][dY].m_sV2 >= 20) && (m_pData[dX][dY].m_sV2 <= 39))
									iSoundIndex = 6;
								else if ((m_pData[dX][dY].m_sV2 >= 40) && (m_pData[dX][dY].m_sV2 <= 59))
									iSoundIndex = 7;
								else iSoundIndex = 5;
								if (true) m_pGame->PlayGameSound('C', iSoundIndex, sDist, lPan);
								m_pGame->m_pEffectManager->AddEffect(EffectType::NORMAL_HIT, m_sPivotX + dX, m_sPivotY + dY, 0, 0, 0, 4);
							}
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
							{
								if (true) m_pGame->PlayGameSound('C', 13, sDist, lPan);
							}
							break;

						default:
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
							{
								if (m_pData[dX][dY].m_sV2 == -1)
									iSoundIndex = 5;
								else if (m_pData[dX][dY].m_sV2 == 0)
									iSoundIndex = 5;
								else if ((m_pData[dX][dY].m_sV2 >= 1) && (m_pData[dX][dY].m_sV2 <= 19))
									iSoundIndex = 6;
								else if ((m_pData[dX][dY].m_sV2 >= 20) && (m_pData[dX][dY].m_sV2 <= 39))
									iSoundIndex = 6;
								else if ((m_pData[dX][dY].m_sV2 >= 40) && (m_pData[dX][dY].m_sV2 <= 59))
									iSoundIndex = 7;
								else iSoundIndex = 5;
								if (true) m_pGame->PlayGameSound('C', iSoundIndex, sDist, lPan);
								m_pGame->m_pEffectManager->AddEffect(EffectType::NORMAL_HIT, m_sPivotX + dX, m_sPivotY + dY, 0, 0, 0, 4);
							}

							switch (m_pData[dX][dY].m_sOwnerType) {
							case hb::shared::owner::ATK: //Snoopy:  ATK
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
									m_pGame->PlayGameSound('M', 143, sDist, lPan);
								break;
							case hb::shared::owner::MasterElf: // Snoopy: MasterElf
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
									m_pGame->PlayGameSound('C', 7, sDist, lPan);
								break;
							case hb::shared::owner::Barbarian: // Snoopy: Barbarian
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
									m_pGame->PlayGameSound('M', 144, sDist, lPan);
								break;
							case hb::shared::owner::DSK: // Snoopy: DSK
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
									m_pGame->PlayGameSound('M', 148, sDist, lPan);
								break;

							case hb::shared::owner::Slime: // Slime
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
									m_pGame->PlayGameSound('M', 3, sDist, lPan);
								break;

							case hb::shared::owner::Skeleton: // Skell
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
									m_pGame->PlayGameSound('M', 15, sDist, lPan);
								break;

							case hb::shared::owner::StoneGolem: // Stone Golem
							case hb::shared::owner::IceGolem: // IceGolem
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
									m_pGame->PlayGameSound('M', 35, sDist, lPan);
								break;

							case hb::shared::owner::Cyclops: // Cyclops
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
									m_pGame->PlayGameSound('M', 43, sDist, lPan);
								break;

							case hb::shared::owner::OrcMage: // Orc
							case hb::shared::owner::Stalker: // SK
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
									m_pGame->PlayGameSound('M', 11, sDist, lPan);
								break;

							case hb::shared::owner::GiantAnt: // Ant
							case hb::shared::owner::LightWarBeetle: // LWB
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
									m_pGame->PlayGameSound('M', 31, sDist, lPan);
								break;

							case hb::shared::owner::Scorpion: // Scorpion
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
									m_pGame->PlayGameSound('M', 23, sDist, lPan);
								break;

							case hb::shared::owner::Zombie: // Zombie
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
									m_pGame->PlayGameSound('M', 19, sDist, lPan);
								break;

							case hb::shared::owner::Amphis: // Snake
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
									m_pGame->PlayGameSound('M', 27, sDist, lPan);
								break;

							case hb::shared::owner::ClayGolem: // Clay-Golem
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
									m_pGame->PlayGameSound('M', 39, sDist, lPan);
								break;

							case hb::shared::owner::Hellhound: // HH
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
									m_pGame->PlayGameSound('M', 7, sDist, lPan);
								break;

							case hb::shared::owner::Troll: // Troll
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
									m_pGame->PlayGameSound('M', 48, sDist, lPan);
								break;

							case hb::shared::owner::Ogre: // Ogre
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
									m_pGame->PlayGameSound('M', 53, sDist, lPan);
								break;

							case hb::shared::owner::Liche: // Liche
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
									m_pGame->PlayGameSound('M', 57, sDist, lPan);
								break;

							case hb::shared::owner::Demon: // DD
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
									m_pGame->PlayGameSound('M', 61, sDist, lPan);
								break;

							case hb::shared::owner::Unicorn: // Uni
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
									m_pGame->PlayGameSound('M', 65, sDist, lPan);
								break;

							case hb::shared::owner::WereWolf: // WW
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 2)
									m_pGame->PlayGameSound('M', 69, sDist, lPan);
								break;
							case hb::shared::owner::Bunny://Rabbit
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 79, sDist, lPan);
								break;

							case hb::shared::owner::Cat://Cat
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 80, sDist, lPan);
								break;

							case hb::shared::owner::GiantFrog://Giant-Frog
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 81, sDist, lPan);
								break;

							case hb::shared::owner::MountainGiant://Mountain Giant
							case hb::shared::owner::MasterOrc: // Snoopy: MasterMageOrc
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 89, sDist, lPan);
								break;

							case hb::shared::owner::Ettin://Ettin
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 93, sDist, lPan);
								break;

							case hb::shared::owner::CannibalPlant://Cannibal Plant
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 97, sDist, lPan);
								break;

							case hb::shared::owner::Rudolph://Rudolph
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 69, sDist, lPan);
								break;
							case hb::shared::owner::DireBoar://DireBoar
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 78, sDist, lPan);
								break;

							case hb::shared::owner::GiantCrayfish: //Snoopy:  GiantCrayFish
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 101, sDist, lPan);
								break;

							case hb::shared::owner::Minaus: // Snoopy: Minos
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 101, sDist, lPan);
								break;

							case hb::shared::owner::Tentocle: // Snoopy: Tentocle
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 108, sDist, lPan);
								break;

							case hb::shared::owner::Abaddon: // Snoopy: Abaddon
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 138, sDist, lPan);
								break;

							case hb::shared::owner::ClawTurtle: // Snoopy: ClawTurtle
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 112, sDist, lPan);
								break;

							case hb::shared::owner::Centaur: // Snoopy: Centaurus
							case hb::shared::owner::Sorceress: // Snoopy: Sorceress
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 116, sDist, lPan);
								break;

							case hb::shared::owner::GiTree: // Snoopy: GiantTree
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 120, sDist, lPan);
								break;

							case hb::shared::owner::GiLizard: // Snoopy: GiantLizard
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 124, sDist, lPan);
								break;

							case hb::shared::owner::Dragon: // Snoopy: Dragon
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 128, sDist, lPan);
								break;

							case hb::shared::owner::Nizie: // Snoopy: Nizie
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 132, sDist, lPan);
								break;

							default:
								break;
							}
							break;
						}
					}

					if (m_pData[dX][dY].m_animation.cAction == Type::Magic)  // 4 00499D51
					{
						switch (m_pData[dX][dY].m_sOwnerType) {
						case 1:
						case 2:
						case 3:
						case 4:
						case 5:
						case 6:
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
							{
								if (true) m_pGame->PlayGameSound('C', 16, sDist, lPan);
								if ((((m_pData[dX][dY].m_appearance.iWeaponGlare | m_pData[dX][dY].m_appearance.iShieldGlare) != 0) || (m_pData[dX][dY].m_status.bGMMode)) && (!m_pData[dX][dY].m_status.bInvisibility))
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::STAR_TWINKLE, (m_sPivotX + dX) * 32 + (rand() % 20 - 10), (m_sPivotY + dY) * 32 - (rand() % 50) - 5, 0, 0, -(rand() % 8), 0);
									m_pGame->m_pEffectManager->AddEffect(EffectType::STAR_TWINKLE, (m_sPivotX + dX) * 32 + (rand() % 20 - 10), (m_sPivotY + dY) * 32 - (rand() % 50) - 5, 0, 0, -(rand() % 8), 0);
								}
								//Snoopy: Angels
								if (((m_pData[dX][dY].m_status.iAngelPercent) > rand() % 6) // Angel stars
									&& (m_pData[dX][dY].m_status.HasAngelType())
									&& (!m_pData[dX][dY].m_status.bInvisibility))
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::STAR_TWINKLE, (m_sPivotX + dX) * 32 + (rand() % 15 + 10), (m_sPivotY + dY) * 32 - (rand() % 30) - 50, 0, 0, -(rand() % 8), 0);
								}
								if (m_pGame->bHasHeroSet(m_pData[dX][dY].m_appearance, m_pData[dX][dY].m_sOwnerType) == 2) // Mage hero set
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::MAGE_HERO_SET, m_sPivotX + dX, m_sPivotY + dY
										, m_sPivotX + dX, m_sPivotY + dY, 0, 1);
								}
								if (m_pData[dX][dY].m_sV1 >= 70) // effet gros sorts autour du caster
									m_pGame->m_pEffectManager->AddEffect(EffectType::BUFF_EFFECT_LIGHT, (m_sPivotX + dX) * 32, (m_sPivotY + dY) * 32, 0, 0, 0, 0);
								if (m_pData[dX][dY].m_sV1 == 82) // lumi�re si MassMagicMissile autour du caster
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::MASS_MM_AURA_CASTER, (m_sPivotX + dX) * 32, (m_sPivotY + dY) * 32, 0, 0, 0, 0);
								}
							}
							break;
						}
					}

					if (m_pData[dX][dY].m_animation.cAction == Type::Dying)  // 10 // 00499F5D
					{
						switch (m_pData[dX][dY].m_sOwnerType) {
						case 1:
						case 2:
						case 3:
						case hb::shared::owner::GodsHandKnight: // GHK
						case hb::shared::owner::GodsHandKnightCK: // GHKABS
						case hb::shared::owner::TempleKnight: // TK
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 6)
							{
								if (m_pData[dX][dY].m_sV2 == -1)
									iSoundIndex = 5;
								else if (m_pData[dX][dY].m_sV2 == 0)
									iSoundIndex = 5;
								else if ((m_pData[dX][dY].m_sV2 >= 1) && (m_pData[dX][dY].m_sV2 <= 19))
									iSoundIndex = 6;
								else if ((m_pData[dX][dY].m_sV2 >= 20) && (m_pData[dX][dY].m_sV2 <= 39))
									iSoundIndex = 6;
								else if ((m_pData[dX][dY].m_sV2 >= 40) && (m_pData[dX][dY].m_sV2 <= 59))
									iSoundIndex = 7;
								else iSoundIndex = 5;
								if (true) m_pGame->PlayGameSound('C', iSoundIndex, sDist, lPan);
								m_pGame->m_pEffectManager->AddEffect(EffectType::NORMAL_HIT, m_sPivotX + dX, m_sPivotY + dY, 0, 0, 0, 12);
							}
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 7)
							{
								if (true) m_pGame->PlayGameSound('C', 14, sDist, lPan);
							}
							break;

						case 4:
						case 5:
						case 6:
						case hb::shared::owner::DarkElf: // DE
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 6)
							{
								if (m_pData[dX][dY].m_sV2 == -1)
									iSoundIndex = 5;
								else if (m_pData[dX][dY].m_sV2 == 0)
									iSoundIndex = 5;
								else if ((m_pData[dX][dY].m_sV2 >= 1) && (m_pData[dX][dY].m_sV2 <= 19))
									iSoundIndex = 6;
								else if ((m_pData[dX][dY].m_sV2 >= 20) && (m_pData[dX][dY].m_sV2 <= 39))
									iSoundIndex = 6;
								else if ((m_pData[dX][dY].m_sV2 >= 40) && (m_pData[dX][dY].m_sV2 <= 59))
									iSoundIndex = 7;
								else iSoundIndex = 5;
								if (true) m_pGame->PlayGameSound('C', iSoundIndex, sDist, lPan);
								m_pGame->m_pEffectManager->AddEffect(EffectType::NORMAL_HIT, m_sPivotX + dX, m_sPivotY + dY, 0, 0, 0, 12);
							}
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 7)
							{
								if (true) m_pGame->PlayGameSound('C', 15, sDist, lPan);
							}
							break;

						default:
							if (m_pData[dX][dY].m_animation.cCurrentFrame == 4)
							{
								if (m_pData[dX][dY].m_sV2 == -1)
									iSoundIndex = 5;
								else if (m_pData[dX][dY].m_sV2 == 0)
									iSoundIndex = 5;
								else if ((m_pData[dX][dY].m_sV2 >= 1) && (m_pData[dX][dY].m_sV2 <= 19))
									iSoundIndex = 6;
								else if ((m_pData[dX][dY].m_sV2 >= 20) && (m_pData[dX][dY].m_sV2 <= 39))
									iSoundIndex = 6;
								else if ((m_pData[dX][dY].m_sV2 >= 40) && (m_pData[dX][dY].m_sV2 <= 59))
									iSoundIndex = 7;
								else iSoundIndex = 5;
								if (true) m_pGame->PlayGameSound('C', iSoundIndex, sDist, lPan);
								m_pGame->m_pEffectManager->AddEffect(EffectType::NORMAL_HIT, m_sPivotX + dX, m_sPivotY + dY, 0, 0, 0, 12);
							}

							switch (m_pData[dX][dY].m_sOwnerType) {
							case hb::shared::owner::Beholder: // BB
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
									m_pGame->PlayGameSound('M', 39, sDist, lPan);
								break;

							case hb::shared::owner::Slime: // Slime
							case hb::shared::owner::Dummy: // Dummy
							case hb::shared::owner::EnergySphere: // Snoopy: EnergyBall
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
									m_pGame->PlayGameSound('M', 4, sDist, lPan);
								break;

							case hb::shared::owner::Skeleton: // Skell
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
									m_pGame->PlayGameSound('M', 16, sDist, lPan);
								break;

							case hb::shared::owner::StoneGolem: // Stone-Golem
							case hb::shared::owner::BattleGolem: // BG
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
									m_pGame->PlayGameSound('M', 36, sDist, lPan);
								break;

							case hb::shared::owner::IceGolem: // IceGolem
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5) {
									m_pGame->m_pEffectManager->AddEffect(EffectType::AURA_EFFECT_2, (m_sPivotX + dX) * 32, (m_sPivotY + dY) * 32, 0, 0, 0);
									m_pGame->PlayGameSound('M', 36, sDist, lPan);
								}
								break;

							case hb::shared::owner::Cyclops: // Cyclops
							case hb::shared::owner::HellClaw: // HC
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
									m_pGame->PlayGameSound('M', 44, sDist, lPan);
								break;

							case hb::shared::owner::OrcMage: // Orc
							case hb::shared::owner::Stalker: // SK
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
									m_pGame->PlayGameSound('M', 12, sDist, lPan);
								break;

							case hb::shared::owner::GiantAnt: // Ant
							case hb::shared::owner::LightWarBeetle: // LWB
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
									m_pGame->PlayGameSound('M', 32, sDist, lPan);
								break;

							case hb::shared::owner::Scorpion: // Scorp
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
									m_pGame->PlayGameSound('M', 24, sDist, lPan);
								break;

							case hb::shared::owner::Zombie: // Zombie
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
									m_pGame->PlayGameSound('M', 20, sDist, lPan);
								break;

							case hb::shared::owner::Amphis: // Snake
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
									m_pGame->PlayGameSound('M', 28, sDist, lPan);
								break;

							case hb::shared::owner::ClayGolem: // Clay-Golem
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
									m_pGame->PlayGameSound('M', 40, sDist, lPan);
								break;

							case hb::shared::owner::Hellhound: // HH
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
									m_pGame->PlayGameSound('M', 8, sDist, lPan);
								break;

							case hb::shared::owner::Troll: // Troll
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
									m_pGame->PlayGameSound('M', 49, sDist, lPan);
								break;

							case hb::shared::owner::Ogre: // Ogre
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
									m_pGame->PlayGameSound('M', 54, sDist, lPan);
								break;

							case hb::shared::owner::Liche: // Liche
							case hb::shared::owner::TigerWorm: // TW
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
									m_pGame->PlayGameSound('M', 58, sDist, lPan);
								break;

							case hb::shared::owner::Demon: // DD
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
									m_pGame->PlayGameSound('M', 62, sDist, lPan);
								break;

							case hb::shared::owner::Unicorn: // Uni
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
									m_pGame->PlayGameSound('M', 66, sDist, lPan);
								break;

							case hb::shared::owner::WereWolf: // WW
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
									m_pGame->PlayGameSound('M', 70, sDist, lPan);
								break;

							case hb::shared::owner::ArrowGuardTower: // AGT
							case hb::shared::owner::CannonGuardTower: // CGT
							case hb::shared::owner::ManaCollector: // MS
							case hb::shared::owner::Detector: // DT
							case hb::shared::owner::EnergyShield: // ESG
							case hb::shared::owner::GrandMagicGenerator: // GMG
							case hb::shared::owner::ManaStone: // ManaStone
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 3)
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::MS_CRUSADE_EXPLOSION, (m_sPivotX + dX) * 32, (m_sPivotY + dY) * 32, 0, 0, 0, 0);
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10), 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10), 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10), 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10), 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10), 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::MS_CRUSADE_CASTING, (m_sPivotX + dX) * 32 + 30 - (rand() % 60), (m_sPivotY + dY) * 32 + 30 - (rand() % 60), 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::MS_CRUSADE_CASTING, (m_sPivotX + dX) * 32 + 30 - (rand() % 60), (m_sPivotY + dY) * 32 + 30 - (rand() % 60), 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::MS_CRUSADE_CASTING, (m_sPivotX + dX) * 32 + 30 - (rand() % 60), (m_sPivotY + dY) * 32 + 30 - (rand() % 60), 0, 0, -1 * (rand() % 2));
								}
								break;

							case hb::shared::owner::CT: // Snoopy: CrossBowTurret
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 3)
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::MS_CRUSADE_EXPLOSION, (m_sPivotX + dX) * 32, (m_sPivotY + dY) * 32, 0, 0, 0, 0);
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10), 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10), 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10), 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10), 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10), 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::MS_CRUSADE_CASTING, (m_sPivotX + dX) * 32 + 30 - (rand() % 60), (m_sPivotY + dY) * 32 + 30 - (rand() % 60), 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::MS_CRUSADE_CASTING, (m_sPivotX + dX) * 32 + 30 - (rand() % 60), (m_sPivotY + dY) * 32 + 30 - (rand() % 60), 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::MS_CRUSADE_CASTING, (m_sPivotX + dX) * 32 + 30 - (rand() % 60), (m_sPivotY + dY) * 32 + 30 - (rand() % 60), 0, 0, -1 * (rand() % 2));
								}
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
									m_pGame->PlayGameSound('M', 154, sDist, lPan);
								break;

							case hb::shared::owner::AGC: // Snoopy: CannonTurret
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 3)
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::MS_CRUSADE_EXPLOSION, (m_sPivotX + dX) * 32, (m_sPivotY + dY) * 32, 0, 0, 0, 0);
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10), 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10), 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10), 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10), 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10), 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::MS_CRUSADE_CASTING, (m_sPivotX + dX) * 32 + 30 - (rand() % 60), (m_sPivotY + dY) * 32 + 30 - (rand() % 60), 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::MS_CRUSADE_CASTING, (m_sPivotX + dX) * 32 + 30 - (rand() % 60), (m_sPivotY + dY) * 32 + 30 - (rand() % 60), 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::MS_CRUSADE_CASTING, (m_sPivotX + dX) * 32 + 30 - (rand() % 60), (m_sPivotY + dY) * 32 + 30 - (rand() % 60), 0, 0, -1 * (rand() % 2));
								}
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
									m_pGame->PlayGameSound('M', 156, sDist, lPan);
								break;

							case hb::shared::owner::Catapult: // CP
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 1)
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::MS_CRUSADE_EXPLOSION, (m_sPivotX + dX) * 32, (m_sPivotY + dY) * 32 - 30, 0, 0, 0, 0);
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10) - 30, 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10) - 30, 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10) - 30, 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10) - 30, 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10) - 30, 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::MS_CRUSADE_CASTING, (m_sPivotX + dX) * 32 + 30 - (rand() % 60), (m_sPivotY + dY) * 32 + 30 - (rand() % 60) - 30, 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::MS_CRUSADE_CASTING, (m_sPivotX + dX) * 32 + 30 - (rand() % 60), (m_sPivotY + dY) * 32 + 30 - (rand() % 60) - 30, 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::MS_CRUSADE_CASTING, (m_sPivotX + dX) * 32 + 30 - (rand() % 60), (m_sPivotY + dY) * 32 + 30 - (rand() % 60) - 30, 0, 0, -1 * (rand() % 2));
								}
								break;

							case hb::shared::owner::Gargoyle: // GG
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
								{
									m_pGame->PlayGameSound('M', 44, sDist, lPan);
								}
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 11)
								{
									m_pGame->m_pEffectManager->AddEffect(EffectType::MS_CRUSADE_EXPLOSION, (m_sPivotX + dX) * 32, (m_sPivotY + dY) * 32 - 30, 0, 0, 0, 0);
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10) - 30, 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10) - 30, 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10) - 30, 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10) - 30, 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::BURST_LARGE, (m_sPivotX + dX) * 32 + 5 - (rand() % 10), (m_sPivotY + dY) * 32 + 5 - (rand() % 10) - 30, 0, 0, -1 * (rand() % 2));

									m_pGame->m_pEffectManager->AddEffect(EffectType::MS_CRUSADE_CASTING, (m_sPivotX + dX) * 32 + 30 - (rand() % 60), (m_sPivotY + dY) * 32 + 30 - (rand() % 60) - 30, 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::MS_CRUSADE_CASTING, (m_sPivotX + dX) * 32 + 30 - (rand() % 60), (m_sPivotY + dY) * 32 + 30 - (rand() % 60) - 30, 0, 0, -1 * (rand() % 2));
									m_pGame->m_pEffectManager->AddEffect(EffectType::MS_CRUSADE_CASTING, (m_sPivotX + dX) * 32 + 30 - (rand() % 60), (m_sPivotY + dY) * 32 + 30 - (rand() % 60) - 30, 0, 0, -1 * (rand() % 2));
								}
								break;

							case hb::shared::owner::Bunny:// Rabbit
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 83, sDist, lPan);
								break;

							case hb::shared::owner::Cat: // Cat
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 84, sDist, lPan);
								break;

							case hb::shared::owner::GiantFrog://Giant-Frog
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 85, sDist, lPan);
								break;

							case hb::shared::owner::MountainGiant://Mountain Giant
							case hb::shared::owner::MasterOrc: // Snoopy: MasterMageOrc
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 90, sDist, lPan);
								break;

							case hb::shared::owner::Ettin://Ettin
							case hb::shared::owner::Barbarian: // Snoopy: Barbarian
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 94, sDist, lPan);
								break;

							case hb::shared::owner::ATK: // Snoopy: ATK
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 141, sDist, lPan);
								break;

							case hb::shared::owner::DSK: // Snoopy: DSK
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 146, sDist, lPan);
								break;

							case hb::shared::owner::Rudolph://Rudolph
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 65, sDist, lPan);
								break;

							case hb::shared::owner::DireBoar://DireBoar
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 94, sDist, lPan);
								break;

							case hb::shared::owner::Wyvern: // Wyvern
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('E', 7, sDist, lPan);
								break;

							case hb::shared::owner::Dragon: // Snoopy: Dragon
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 129, sDist, lPan);
								break;

							case hb::shared::owner::Centaur: // Snoopy: Centaur
							case hb::shared::owner::Sorceress: // Snoopy: Sorceress
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 129, sDist, lPan);
								break;

							case hb::shared::owner::ClawTurtle: // Snoopy: ClawTurtle
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 113, sDist, lPan);
								break;

							case hb::shared::owner::FireWyvern: // Snoopy: FireWyvern
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 105, sDist, lPan);
								break;


							case hb::shared::owner::CannibalPlant: // Cannibal Plant
							case hb::shared::owner::GiantCrayfish: // Snoopy: GiantGrayFish
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 98, sDist, lPan);
								break;

							case hb::shared::owner::GiLizard: //Snoopy:
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 125, sDist, lPan);
								break;

							case hb::shared::owner::GiTree: // Snoopy:
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 121, sDist, lPan);
								break;

							case hb::shared::owner::Minaus: // Snoopy:
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 103, sDist, lPan);
								break;

							case hb::shared::owner::Nizie: // Snoopy:
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 133, sDist, lPan);
								break;

							case hb::shared::owner::Tentocle: //Snoopy: Tentocle
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 109, sDist, lPan);
								break;

							case hb::shared::owner::Abaddon: // Snoopy: Abaddon
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 139, sDist, lPan);
								break;

							case hb::shared::owner::MasterElf: // Snoopy: MasterElf
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 150, sDist, lPan);
								break;

							case hb::shared::owner::HBT: // Snoopy: HBT
								if ((m_pData[dX][dY].m_animation.cCurrentFrame == 1))
									m_pGame->PlayGameSound('M', 152, sDist, lPan);
								break;

							default:
								if (m_pData[dX][dY].m_animation.cCurrentFrame == 5)
									m_pGame->PlayGameSound('C', 15, sDist, lPan);
								break;

							case hb::shared::owner::Frost: // Frost
							case hb::shared::owner::Gate: // Snoopy: Gate
								break;
							}
							break;
						}
					}
				}
			}
		}
	if (bAutoUpdate)
	{
		S_dwUpdateTime = dwTime;
		if (iRet == 0)
			return -1;
	}
	if (dynObjsNeedUpdate) m_dwDOframeTime = dwTime; //v1.4
	return iRet;
}


bool CMapData::bSetItem(short sX, short sY, short sIDnum, char cItemColor, uint32_t dwItemAttr, bool bDropEffect)
{
	int dX, dY;
	int sAbsX, sAbsY, sDist;
	if ((sX < m_sPivotX) || (sX >= m_sPivotX + MapDataSizeX) ||
		(sY < m_sPivotY) || (sY >= m_sPivotY + MapDataSizeY))
	{
		return false;
	}

	dX = sX - m_sPivotX;
	dY = sY - m_sPivotY;

	m_pData[dX][dY].m_sItemID = sIDnum;
	m_pData[dX][dY].m_dwItemAttr = dwItemAttr;
	m_pData[dX][dY].m_cItemColor = cItemColor;

	sAbsX = abs(((m_pGame->m_Camera.GetX() / 32) + VIEW_CENTER_TILE_X()) - sX);
	sAbsY = abs(((m_pGame->m_Camera.GetY() / 32) + VIEW_CENTER_TILE_Y()) - sY);

	if (sAbsX > sAbsY) sDist = sAbsX;
	else sDist = sAbsY;

	if (sIDnum != 0)
	{
		if (bDropEffect == true)
		{
			m_pGame->PlayGameSound('E', 11, sDist);
			m_pGame->m_pEffectManager->AddEffect(EffectType::FOOTPRINT, (m_sPivotX + dX) * 32, (m_sPivotY + dY) * 32, 0, 0, 0, 0);
			m_pGame->m_pEffectManager->AddEffect(EffectType::FOOTPRINT, (m_sPivotX + dX) * 32 + (10 - (rand() % 20)), (m_sPivotY + dY) * 32 + (10 - (rand() % 20)), 0, 0, (rand() % 2), 0);
			m_pGame->m_pEffectManager->AddEffect(EffectType::FOOTPRINT, (m_sPivotX + dX) * 32 + (10 - (rand() % 20)), (m_sPivotY + dY) * 32 + (10 - (rand() % 20)), 0, 0, (rand() % 2), 0);
		}
	}

	return true;
}

bool __fastcall CMapData::bSetDeadOwner(uint16_t wObjectID, short sX, short sY, short sType, char cDir, const hb::shared::entity::PlayerAppearance& appearance, const hb::shared::entity::PlayerStatus& status, std::string& pName, short npcConfigId)
{
	int  dX, dY;
	std::string pTmpName;
	bool bEraseFlag = false;

	pTmpName = pName;
	if ((sX < m_sPivotX) || (sX >= m_sPivotX + MapDataSizeX) ||
		(sY < m_sPivotY) || (sY >= m_sPivotY + MapDataSizeY))
	{
		for (dX = 0; dX < MapDataSizeX; dX++)
			for (dY = 0; dY < MapDataSizeY; dY++)
			{
				if (m_pData[dX][dY].m_cDeadOwnerName == pTmpName)
				{
					m_pData[dX][dY].m_sDeadOwnerType = 0;
					m_pData[dX][dY].m_sDeadNpcConfigId = -1;
					m_pData[dX][dY].m_cDeadOwnerName.clear();
				}
			}
		return false;
	}

	for (dX = sX - 2; dX <= sX + 2; dX++)
		for (dY = sY - 2; dY <= sY + 2; dY++)
		{
			if (dX < m_sPivotX) break;
			else
				if (dX > m_sPivotX + MapDataSizeX) break;
			if (dY < m_sPivotY) break;
			else
				if (dY > m_sPivotY + MapDataSizeY) break;

			if (m_pData[dX - m_sPivotX][dY - m_sPivotY].m_cDeadOwnerName == pTmpName)
			{
				m_pData[dX - m_sPivotX][dY - m_sPivotY].m_sDeadOwnerType = 0;
				m_pData[dX - m_sPivotX][dY - m_sPivotY].m_sDeadNpcConfigId = -1;
				m_pData[dX - m_sPivotX][dY - m_sPivotY].m_cDeadOwnerName.clear();
				bEraseFlag = true;
			}
		}

	if (bEraseFlag != true) {
		for (dX = 0; dX < MapDataSizeX; dX++)
			for (dY = 0; dY < MapDataSizeY; dY++) {

				if (m_pData[dX][dY].m_cDeadOwnerName == pTmpName) {
					m_pData[dX][dY].m_sDeadOwnerType = 0;
					m_pData[dX][dY].m_sDeadNpcConfigId = -1;
					m_pData[dX][dY].m_cDeadOwnerName.clear();
				}

			}
	}

	dX = sX - m_sPivotX;
	dY = sY - m_sPivotY;

	m_pData[dX][dY].m_wDeadObjectID = wObjectID;
	m_pData[dX][dY].m_sDeadOwnerType = sType;
	m_pData[dX][dY].m_sDeadNpcConfigId = npcConfigId;
	m_pData[dX][dY].m_cDeadDir = cDir;
	m_pData[dX][dY].m_deadAppearance = appearance;
	m_pData[dX][dY].m_deadStatus = status;
	m_pData[dX][dY].m_cDeadOwnerFrame = -1;
	m_pData[dX][dY].m_cDeadOwnerName = pTmpName;

	m_iObjectIDcacheLocX[wObjectID] = -1 * sX;
	m_iObjectIDcacheLocY[wObjectID] = -1 * sY;



	return true;
}

bool __fastcall CMapData::bSetChatMsgOwner(uint16_t wObjectID, short sX, short sY, int iIndex)
{
	int dX, dY;

	if ((sX == -10) && (sY == -10)) goto SCMO_FULL_SEARCH;

	if ((sX < m_sPivotX) || (sX >= m_sPivotX + MapDataSizeX) ||
		(sY < m_sPivotY) || (sY >= m_sPivotY + MapDataSizeY))
	{
		return false;
	}
	for (dX = sX - 4; dX <= sX + 4; dX++)
		for (dY = sY - 4; dY <= sY + 4; dY++)
		{
			if (dX < m_sPivotX) break;
			else
				if (dX > m_sPivotX + MapDataSizeX) break;
			if (dY < m_sPivotY) break;
			else
				if (dY > m_sPivotY + MapDataSizeY) break;

			if (m_pData[dX - m_sPivotX][dY - m_sPivotY].m_wObjectID == wObjectID) {
				m_pData[dX - m_sPivotX][dY - m_sPivotY].m_iChatMsg = iIndex;
				return true;
			}
			if (m_pData[dX - m_sPivotX][dY - m_sPivotY].m_wDeadObjectID == wObjectID) {
				m_pData[dX - m_sPivotX][dY - m_sPivotY].m_iDeadChatMsg = iIndex;
				return true;
			}
		}

SCMO_FULL_SEARCH:;

	for (dX = 0; dX < MapDataSizeX; dX++)
		for (dY = 0; dY < MapDataSizeY; dY++) {

			if (m_pData[dX][dY].m_wObjectID == wObjectID) {
				m_pData[dX][dY].m_iChatMsg = iIndex;
				return true;
			}
			if (m_pData[dX][dY].m_wDeadObjectID == wObjectID) {
				m_pData[dX][dY].m_iDeadChatMsg = iIndex;
				return true;
			}
		}

	return false;
}

void CMapData::ClearChatMsg(short sX, short sY)
{
	m_pGame->m_floatingText.Clear(m_pData[sX - m_sPivotX][sY - m_sPivotY].m_iChatMsg);
	m_pData[sX - m_sPivotX][sY - m_sPivotY].m_iChatMsg = 0;
}

void CMapData::ClearDeadChatMsg(short sX, short sY)
{
	m_pData[sX - m_sPivotX][sY - m_sPivotY].m_iDeadChatMsg = 0;
}

bool __fastcall CMapData::bGetOwner(short sX, short sY, std::string& pName, short* pOwnerType, hb::shared::entity::PlayerStatus* pOwnerStatus, uint16_t* pObjectID)
{
	int dX, dY;

	if ((sX < m_sPivotX) || (sX > m_sPivotX + MapDataSizeX) ||
		(sY < m_sPivotY) || (sY > m_sPivotY + MapDataSizeY)) {
		pName.clear();
		return false;
	}

	dX = sX - m_sPivotX;
	dY = sY - m_sPivotY;

	*pOwnerType = m_pData[dX][dY].m_sOwnerType;
	pName = m_pData[dX][dY].m_cOwnerName;
	*pOwnerStatus = m_pData[dX][dY].m_status;
	*pObjectID = m_pData[dX][dY].m_wObjectID;

	return true;
}

bool CMapData::bSetDynamicObject(short sX, short sY, uint16_t wID, short sType, bool bIsEvent)
{
	int dX, dY, sPrevType;

	if ((sX < m_sPivotX) || (sX >= m_sPivotX + MapDataSizeX) ||
		(sY < m_sPivotY) || (sY >= m_sPivotY + MapDataSizeY))
	{
		return false;
	}

	dX = sX - m_sPivotX;
	dY = sY - m_sPivotY;

	sPrevType = m_pData[dX][dY].m_sDynamicObjectType;

	m_pData[dX][dY].m_sDynamicObjectType = sType;
	m_pData[dX][dY].m_cDynamicObjectFrame = rand() % 5;
	m_pData[dX][dY].m_dwDynamicObjectTime = GameClock::GetTimeMS();

	m_pData[dX][dY].m_cDynamicObjectData1 = 0;
	m_pData[dX][dY].m_cDynamicObjectData2 = 0;
	m_pData[dX][dY].m_cDynamicObjectData3 = 0;
	m_pData[dX][dY].m_cDynamicObjectData4 = 0;

	switch (sType) {
	case 0:
		if (sPrevType == dynamic_object::Fire)
		{
			m_pGame->m_pEffectManager->AddEffect(EffectType::RED_CLOUD_PARTICLES, (m_sPivotX + dX) * 32, (m_sPivotY + dY) * 32, 0, 0, 0, 0);
			m_pGame->m_pEffectManager->AddEffect(EffectType::RED_CLOUD_PARTICLES, (m_sPivotX + dX) * 32 + (10 - (rand() % 20)), (m_sPivotY + dY) * 32 + (20 - (rand() % 40)), 0, 0, 0, 0);
			m_pGame->m_pEffectManager->AddEffect(EffectType::RED_CLOUD_PARTICLES, (m_sPivotX + dX) * 32 + (10 - (rand() % 20)), (m_sPivotY + dY) * 32 + (20 - (rand() % 40)), 0, 0, 0, 0);
			m_pGame->m_pEffectManager->AddEffect(EffectType::RED_CLOUD_PARTICLES, (m_sPivotX + dX) * 32 + (10 - (rand() % 20)), (m_sPivotY + dY) * 32 + (20 - (rand() % 40)), 0, 0, 0, 0);
		}
		else if ((sPrevType == dynamic_object::PCloudBegin) || (sPrevType == dynamic_object::PCloudLoop))
		{
			m_pData[dX][dY].m_sDynamicObjectType = dynamic_object::PCloudEnd;
			m_pData[dX][dY].m_cDynamicObjectFrame = 0;
			m_pData[dX][dY].m_dwDynamicObjectTime = GameClock::GetTimeMS();
		}
		break;

	case dynamic_object::Fish:
		m_pData[dX][dY].m_cDynamicObjectData1 = (rand() % 40) - 20;
		m_pData[dX][dY].m_cDynamicObjectData2 = (rand() % 40) - 20;
		m_pData[dX][dY].m_cDynamicObjectData3 = (rand() % 10) - 5;
		m_pData[dX][dY].m_cDynamicObjectData4 = (rand() % 10) - 5;
		break;

	case dynamic_object::PCloudBegin:
		if (bIsEvent == false)
		{
			m_pData[dX][dY].m_sDynamicObjectType = dynamic_object::PCloudLoop;
			m_pData[dX][dY].m_cDynamicObjectFrame = rand() % 8;
		}
		else m_pData[dX][dY].m_cDynamicObjectFrame = -1 * (rand() % 8);
		break;

	case dynamic_object::AresdenFlag1:
		m_pData[dX][dY].m_cDynamicObjectFrame = (rand() % 4);
		break;

	case dynamic_object::ElvineFlag1:
		m_pData[dX][dY].m_cDynamicObjectFrame = 4 + (rand() % 4);
		break;
	}
	return true;
}

void CMapData::GetOwnerStatusByObjectID(uint16_t wObjectID, char* pOwnerType, char* pDir, hb::shared::entity::PlayerAppearance* pAppearance, hb::shared::entity::PlayerStatus* pStatus, std::string& pName)
{
	int iX, iY;
	for (iX = 0; iX < MapDataSizeX; iX++)
		for (iY = 0; iY < MapDataSizeY; iY++)
			if (m_pData[iX][iY].m_wObjectID == wObjectID)
			{
				*pOwnerType = (char)m_pData[iX][iY].m_sOwnerType;
				*pDir = m_pData[iX][iY].m_animation.cDir;
				*pAppearance = m_pData[iX][iY].m_appearance;
				*pStatus = m_pData[iX][iY].m_status;
				pName = m_pData[iX][iY].m_cOwnerName;
				return;
			}
}
