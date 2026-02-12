#include "Game.h"
#include "NetworkMessageManager.h"
#include "Packet/SharedPackets.h"
#include "lan_eng.h"
#include "PlatformCompat.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <format>
#include <string>


namespace NetworkMessageHandlers {
	void HandleHP(CGame* pGame, char* pData)
	{
		int iPrevHP;
		std::string cTxt;

		iPrevHP = pGame->m_pPlayer->m_iHP;
	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyHP>(
		pData, sizeof(hb::net::PacketNotifyHP));
	if (!pkt) return;
	pGame->m_pPlayer->m_iHP = static_cast<int>(pkt->hp);
	pGame->m_pPlayer->m_iHungerStatus = static_cast<int>(pkt->hunger);

	if (pGame->m_pPlayer->m_iHP > iPrevHP)
	{
		if ((pGame->m_pPlayer->m_iHP - iPrevHP) < 10) return;
		cTxt = std::format(NOTIFYMSG_HP_UP, pGame->m_pPlayer->m_iHP - iPrevHP);
		pGame->AddEventList(cTxt.c_str(), 10);
		pGame->PlayGameSound('E', 21, 0);
	}
	else
	{
		if ((pGame->m_cLogOutCount > 0) && (pGame->m_bForceDisconn == false))
		{
			pGame->m_cLogOutCount = -1;
			pGame->AddEventList(NOTIFYMSG_HP2, 10);
		}
		pGame->m_dwDamagedTime = GameClock::GetTimeMS();
		if (pGame->m_pPlayer->m_iHP < 20) pGame->AddEventList(NOTIFYMSG_HP3, 10);
		if ((iPrevHP - pGame->m_pPlayer->m_iHP) < 10) return;
		cTxt = std::format(NOTIFYMSG_HP_DOWN, iPrevHP - pGame->m_pPlayer->m_iHP);
		pGame->AddEventList(cTxt.c_str(), 10);
	}
	}

	void HandleMP(CGame* pGame, char* pData)
	{
		int iPrevMP;
		std::string cTxt;
		iPrevMP = pGame->m_pPlayer->m_iMP;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyMP>(
			pData, sizeof(hb::net::PacketNotifyMP));
		if (!pkt) return;
		pGame->m_pPlayer->m_iMP = static_cast<int>(pkt->mp);
		if (abs(pGame->m_pPlayer->m_iMP - iPrevMP) < 10) return;
		if (pGame->m_pPlayer->m_iMP > iPrevMP)
		{
			cTxt = std::format(NOTIFYMSG_MP_UP, pGame->m_pPlayer->m_iMP - iPrevMP);
			pGame->AddEventList(cTxt.c_str(), 10);
			pGame->PlayGameSound('E', 21, 0);
		}
		else
		{
			cTxt = std::format(NOTIFYMSG_MP_DOWN, iPrevMP - pGame->m_pPlayer->m_iMP);
			pGame->AddEventList(cTxt.c_str(), 10);
		}
	}

	void HandleSP(CGame* pGame, char* pData)
	{
		int iPrevSP;
		std::string cTxt;
		iPrevSP = pGame->m_pPlayer->m_iSP;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifySP>(
			pData, sizeof(hb::net::PacketNotifySP));
		if (!pkt) return;
		pGame->m_pPlayer->m_iSP = static_cast<int>(pkt->sp);
		if (abs(pGame->m_pPlayer->m_iSP - iPrevSP) < 10) return;
		if (pGame->m_pPlayer->m_iSP > iPrevSP)
		{
			cTxt = std::format(NOTIFYMSG_SP_UP, pGame->m_pPlayer->m_iSP - iPrevSP);
			pGame->AddEventList(cTxt.c_str(), 10);
			pGame->PlayGameSound('E', 21, 0);
		}
		else
		{
			cTxt = std::format(NOTIFYMSG_SP_DOWN, iPrevSP - pGame->m_pPlayer->m_iSP);
			pGame->AddEventList(cTxt.c_str(), 10);
		}
	}

	void HandleExp(CGame* pGame, char* pData)
	{
		DWORD iPrevExp;
		std::string cTxt;

		iPrevExp = pGame->m_pPlayer->m_iExp;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyExp>(
			pData, sizeof(hb::net::PacketNotifyExp));
		if (!pkt) return;
		pGame->m_pPlayer->m_iExp = static_cast<int>(pkt->exp);

		if (pGame->m_pPlayer->m_iExp > iPrevExp)
		{
			cTxt = std::format(EXP_INCREASED, pGame->m_pPlayer->m_iExp - iPrevExp);
			pGame->AddEventList(cTxt.c_str(), 10);
		}
		else
		{
			cTxt = std::format(EXP_DECREASED, iPrevExp - pGame->m_pPlayer->m_iExp);
			pGame->AddEventList(cTxt.c_str(), 10);
		}
	}

	void HandleLevelUp(CGame* pGame, char* pData)
	{
		int iPrevLevel;
		std::string cTxt;

		iPrevLevel = pGame->m_pPlayer->m_iLevel;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyLevelUp>(
			pData, sizeof(hb::net::PacketNotifyLevelUp));
		if (!pkt) return;

		pGame->m_pPlayer->m_iLevel = pkt->level;
		pGame->m_pPlayer->m_iStr = pkt->str;
		pGame->m_pPlayer->m_iVit = pkt->vit;
		pGame->m_pPlayer->m_iDex = pkt->dex;
		pGame->m_pPlayer->m_iInt = pkt->intel;
		pGame->m_pPlayer->m_iMag = pkt->mag;
		pGame->m_pPlayer->m_iCharisma = pkt->chr;

		pGame->m_pPlayer->m_iLU_Point = (pGame->m_pPlayer->m_iLevel - 1) * 3 - ((pGame->m_pPlayer->m_iStr + pGame->m_pPlayer->m_iVit + pGame->m_pPlayer->m_iDex + pGame->m_pPlayer->m_iInt + pGame->m_pPlayer->m_iMag + pGame->m_pPlayer->m_iCharisma) - 70);
		pGame->m_pPlayer->m_wLU_Str = pGame->m_pPlayer->m_wLU_Vit = pGame->m_pPlayer->m_wLU_Dex = pGame->m_pPlayer->m_wLU_Int = pGame->m_pPlayer->m_wLU_Mag = pGame->m_pPlayer->m_wLU_Char = 0;

		cTxt = std::format(NOTIFYMSG_LEVELUP1, pGame->m_pPlayer->m_iLevel);
		pGame->AddEventList(cTxt.c_str(), 10);

		switch (pGame->m_pPlayer->m_sPlayerType) {
		case 1:
		case 2:
		case 3:
			pGame->PlayGameSound('C', 21, 0);
			break;
		case 4:
		case 5:
		case 6:
			pGame->PlayGameSound('C', 22, 0);
			break;
		}

		pGame->m_floatingText.RemoveByObjectID(pGame->m_pPlayer->m_sPlayerObjectID);
		pGame->m_floatingText.AddNotifyText(NotifyTextType::LevelUp, "Level up!", pGame->m_dwCurTime,
			pGame->m_pPlayer->m_sPlayerObjectID, pGame->m_pMapData.get());
		return;
	}
} // namespace NetworkMessageHandlers
