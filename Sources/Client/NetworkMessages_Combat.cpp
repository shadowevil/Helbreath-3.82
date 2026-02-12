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


using namespace hb::shared::action;

namespace NetworkMessageHandlers {
	void HandleKilled(CGame* pGame, char* pData)
	{
		char cAttackerName[21]{};
		pGame->m_pPlayer->m_Controller.SetCommandAvailable(false);
		pGame->m_pPlayer->m_Controller.SetCommand(Type::Stop);
		pGame->m_pPlayer->m_iHP = 0;
		pGame->m_pPlayer->m_Controller.SetCommand(-1);
		// Restart
		pGame->m_bItemUsingStatus = false;
		pGame->ClearSkillUsingStatus();
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyKilled>(
			pData, sizeof(hb::net::PacketNotifyKilled));
		if (!pkt) return;
		memcpy(cAttackerName, pkt->attacker_name, sizeof(pkt->attacker_name));
		pGame->AddEventList(NOTIFYMSG_KILLED1, 10);
		pGame->AddEventList(NOTIFYMSG_KILLED3, 10);
	}

	void HandlePKcaptured(CGame* pGame, char* pData)
	{
		DWORD iExp, iRewardGold;
		int     iPKcount, iLevel;
		std::string cTxt;

		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyPKcaptured>(
			pData, sizeof(hb::net::PacketNotifyPKcaptured));
		if (!pkt) return;
		iPKcount = pkt->pk_count;
		iLevel = pkt->victim_pk_count;
		std::string cName(pkt->victim_name, strnlen(pkt->victim_name, sizeof(pkt->victim_name)));
		iRewardGold = pkt->reward_gold;
		iExp = pkt->exp;
		cTxt = std::format(NOTIFYMSG_PK_CAPTURED1, iLevel, cName, iPKcount);
		pGame->AddEventList(cTxt.c_str(), 10);
		cTxt = std::format(EXP_INCREASED, iExp - pGame->m_pPlayer->m_iExp);
		pGame->AddEventList(cTxt.c_str(), 10);
		cTxt = std::format(NOTIFYMSG_PK_CAPTURED3, iExp - pGame->m_pPlayer->m_iExp);
		pGame->AddEventList(cTxt.c_str(), 10);
	}

	void HandlePKpenalty(CGame* pGame, char* pData)
	{
		DWORD iExp;
		int     iPKcount, iStr, iVit, iDex, iInt, iMag, iChr;
		std::string cTxt;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyPKpenalty>(
			pData, sizeof(hb::net::PacketNotifyPKpenalty));
		if (!pkt) return;
		iExp = pkt->exp;
		iStr = pkt->str;
		iVit = pkt->vit;
		iDex = pkt->dex;
		iInt = pkt->intel;
		iMag = pkt->mag;
		iChr = pkt->chr;
		iPKcount = pkt->pk_count;
		cTxt = std::format(NOTIFYMSG_PK_PENALTY1, iPKcount);
		pGame->AddEventList(cTxt.c_str(), 10);
		if (pGame->m_pPlayer->m_iExp > iExp)
		{
			cTxt = std::format(NOTIFYMSG_PK_PENALTY2, pGame->m_pPlayer->m_iExp - iExp);
			pGame->AddEventList(cTxt.c_str(), 10);
		}
		pGame->m_pPlayer->m_iExp = iExp;
		pGame->m_pPlayer->m_iStr = iStr;
		pGame->m_pPlayer->m_iVit = iVit;
		pGame->m_pPlayer->m_iDex = iDex;
		pGame->m_pPlayer->m_iInt = iInt;
		pGame->m_pPlayer->m_iMag = iMag;
		pGame->m_pPlayer->m_iCharisma = iChr;
		pGame->m_pPlayer->m_iPKCount = iPKcount;
	}

	void HandleEnemyKills(CGame* pGame, char* pData)
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyEnemyKills>(
			pData, sizeof(hb::net::PacketNotifyEnemyKills));
		if (!pkt) return;
		pGame->m_pPlayer->m_iEnemyKillCount = pkt->count;
	}

	void HandleEnemyKillReward(CGame* pGame, char* pData)
	{
		DWORD iExp;
		short sGuildRank;
		std::string cTxt;

		int   iEnemyKillCount, iWarContribution;

		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyEnemyKillReward>(
			pData, sizeof(hb::net::PacketNotifyEnemyKillReward));
		if (!pkt) return;

		iExp = pkt->exp;
		iEnemyKillCount = static_cast<int>(pkt->kill_count);
		std::string cName(pkt->killer_name, strnlen(pkt->killer_name, sizeof(pkt->killer_name)));
		std::string cGuildName(pkt->killer_guild, strnlen(pkt->killer_guild, sizeof(pkt->killer_guild)));
		sGuildRank = pkt->killer_rank;
		iWarContribution = pkt->war_contribution;

		if (iWarContribution > pGame->m_pPlayer->m_iWarContribution)
		{
			std::string warBuf;
			warBuf = std::format("{} +{}!", pGame->m_pGameMsgList[21]->m_pMsg, iWarContribution - pGame->m_pPlayer->m_iWarContribution);
			pGame->SetTopMsg(warBuf.c_str(), 5);
		}
		else if (iWarContribution < pGame->m_pPlayer->m_iWarContribution)
		{
		}
		pGame->m_pPlayer->m_iWarContribution = iWarContribution;

		if (sGuildRank == -1)
		{
			cTxt = std::format(NOTIFYMSG_ENEMYKILL_REWARD1, cName);
			pGame->AddEventList(cTxt.c_str(), 10);
		}
		else
		{
			cTxt = std::format(NOTIFYMSG_ENEMYKILL_REWARD2, cName, cGuildName);
			pGame->AddEventList(cTxt.c_str(), 10);
		}

		if (pGame->m_pPlayer->m_iEnemyKillCount != iEnemyKillCount)
		{
			if (pGame->m_pPlayer->m_iEnemyKillCount > iEnemyKillCount)
			{
				cTxt = std::format(NOTIFYMSG_ENEMYKILL_REWARD5, pGame->m_pPlayer->m_iEnemyKillCount - iEnemyKillCount);
				pGame->AddEventList(cTxt.c_str(), 10);
			}
			else
			{
				cTxt = std::format(NOTIFYMSG_ENEMYKILL_REWARD6, iEnemyKillCount - pGame->m_pPlayer->m_iEnemyKillCount);
				pGame->AddEventList(cTxt.c_str(), 10);
			}
		}

		if (iExp >= 0) pGame->m_pPlayer->m_iExp = iExp;
		if (iEnemyKillCount >= 0) pGame->m_pPlayer->m_iEnemyKillCount = iEnemyKillCount;
		pGame->PlayGameSound('E', 23, 0);

		pGame->m_floatingText.RemoveByObjectID(pGame->m_pPlayer->m_sPlayerObjectID);
		pGame->m_floatingText.AddNotifyText(NotifyTextType::EnemyKill, "Enemy Kill!", pGame->m_dwCurTime,
			pGame->m_pPlayer->m_sPlayerObjectID, pGame->m_pMapData.get());
		pGame->CreateScreenShot();
	}

	void HandleGlobalAttackMode(CGame* pGame, char* pData)
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyGlobalAttackMode>(
			pData, sizeof(hb::net::PacketNotifyGlobalAttackMode));
		if (!pkt) return;
		switch (pkt->mode) {
		case 0:
			pGame->AddEventList(NOTIFYMSG_GLOBAL_ATTACK_MODE1, 10);
			pGame->AddEventList(NOTIFYMSG_GLOBAL_ATTACK_MODE2, 10);
			break;

		case 1:
			pGame->AddEventList(NOTIFYMSG_GLOBAL_ATTACK_MODE3, 10);
			break;
		}
	}

	void HandleDamageMove(CGame* pGame, char* pData)
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyDamageMove>(
			pData, sizeof(hb::net::PacketNotifyDamageMove));
		if (!pkt) return;
		pGame->m_pPlayer->m_sDamageMove = pkt->dir;
		pGame->m_pPlayer->m_sDamageMoveAmount = pkt->amount;
	}

	void HandleObserverMode(CGame* pGame, char* pData)
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyObserverMode>(
			pData, sizeof(hb::net::PacketNotifyObserverMode));
		if (!pkt) return;
		if (pkt->enabled == 1)
		{
			pGame->AddEventList(NOTIFY_MSG_HANDLER40); // "Observer Mode On. Press 'SHIFT + ESC' to Log Out..."
			pGame->m_bIsObserverMode = true;
			pGame->m_dwObserverCamTime = GameClock::GetTimeMS();
			std::string cName = pGame->m_pPlayer->m_cPlayerName;
			pGame->m_pMapData->bSetOwner(pGame->m_pPlayer->m_sPlayerObjectID, -1, -1, 0, 0, hb::shared::entity::PlayerAppearance{}, hb::shared::entity::PlayerStatus{}, cName, 0, 0, 0, 0);
		}
		else
		{
			pGame->AddEventList(NOTIFY_MSG_HANDLER41); // "Observer Mode Off"
			pGame->m_bIsObserverMode = false;
			pGame->m_pMapData->bSetOwner(pGame->m_pPlayer->m_sPlayerObjectID, pGame->m_pPlayer->m_sPlayerX, pGame->m_pPlayer->m_sPlayerY, pGame->m_pPlayer->m_sPlayerType, pGame->m_pPlayer->m_iPlayerDir, pGame->m_pPlayer->m_playerAppearance, pGame->m_pPlayer->m_playerStatus, pGame->m_pPlayer->m_cPlayerName, Type::Stop, 0, 0, 0);
		}
	}

	void HandleSuperAttackLeft(CGame* pGame, char* pData)
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifySuperAttackLeft>(
			pData, sizeof(hb::net::PacketNotifySuperAttackLeft));
		if (!pkt) return;
		pGame->m_pPlayer->m_iSuperAttackLeft = pkt->left;
	}

	void HandleSafeAttackMode(CGame* pGame, char* pData)
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifySafeAttackMode>(
			pData, sizeof(hb::net::PacketNotifySafeAttackMode));
		if (!pkt) return;
		switch (pkt->enabled) {
		case 1:
			if (!pGame->m_pPlayer->m_bIsSafeAttackMode) pGame->AddEventList(NOTIFY_MSG_HANDLER50, 10);
			pGame->m_pPlayer->m_bIsSafeAttackMode = true;
			break;
		case 0:
			if (pGame->m_pPlayer->m_bIsSafeAttackMode) pGame->AddEventList(NOTIFY_MSG_HANDLER51, 10);
			pGame->m_pPlayer->m_bIsSafeAttackMode = false;
			break;
		}
	}

	void HandleSpellInterrupted(CGame* pGame, char* pData)
	{
		if (pGame->m_pPlayer->m_Controller.GetCommand() == Type::Magic)
			pGame->m_pPlayer->m_Controller.SetCommand(Type::Stop);
		pGame->m_bIsGetPointingMode = false;
		pGame->m_iPointCommandType = -1;
	}
}

