#include "Game.h"
#include "TeleportManager.h"
#include "NetworkMessageManager.h"
#include "Packet/SharedPackets.h"
#include "lan_eng.h"
#include "DialogBoxIDs.h"
#include <cstdio>
#include <cstring>
#include "PlatformCompat.h"
#include <cmath>
#include <format>

namespace NetworkMessageHandlers {
	std::string G_cTxt;
	void HandleCrusade(CGame* pGame, char* pData)
	{
		int iV1, iV2, iV3, iV4;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyCrusade>(
			pData, sizeof(hb::net::PacketNotifyCrusade));
		if (!pkt) return;
		iV1 = pkt->crusade_mode;
		iV2 = pkt->crusade_duty;
		iV3 = pkt->v3;
		iV4 = pkt->v4;

		if (pGame->m_bIsCrusadeMode == false)
		{
			if (iV1 != 0) // begin crusade
			{
				pGame->m_bIsCrusadeMode = true;
				pGame->m_pPlayer->m_iCrusadeDuty = iV2;
				if ((pGame->m_pPlayer->m_iCrusadeDuty != 3) && (pGame->m_pPlayer->m_bCitizen == true))
					pGame->_RequestMapStatus("middleland", 3);
				if (pGame->m_pPlayer->m_iCrusadeDuty != 0)
					pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::CrusadeJob, 2, iV2, 0);
				else pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::CrusadeJob, 1, 0, 0);
				
				if (pGame->m_pPlayer->m_bCitizen == false) pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::Text, LOGICAL_WIDTH(), 0, 0);
				else if (pGame->m_pPlayer->m_bAresden == true) pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::Text, 801, 0, 0);
				else if (pGame->m_pPlayer->m_bAresden == false) pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::Text, 802, 0, 0);
				
				if (pGame->m_pPlayer->m_bCitizen == false) pGame->SetTopMsg(NOTIFY_MSG_CRUSADESTART_NONE, 10);
				else pGame->SetTopMsg(pGame->m_pGameMsgList[9]->m_pMsg, 10);
				pGame->PlayGameSound('E', 25, 0, 0);
			}
			if (iV3 != 0) // Crusade finished, show XP result screen
			{
				pGame->CrusadeContributionResult(iV3);
			}
			if (iV4 == -1) // The crusade you played in was finished.
			{
				pGame->CrusadeContributionResult(0); 
			}
		}
		else
		{
			if (iV1 == 0) // crusade finished show result (1st result: winner)
			{
				pGame->m_bIsCrusadeMode = false;
				pGame->m_pPlayer->m_iCrusadeDuty = 0;
				pGame->CrusadeWarResult(iV4);
				pGame->SetTopMsg(pGame->m_pGameMsgList[57]->m_pMsg, 8);
			}
			else
			{
				if (pGame->m_pPlayer->m_iCrusadeDuty != iV2)
				{
					pGame->m_pPlayer->m_iCrusadeDuty = iV2;
					pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::CrusadeJob, 2, iV2, 0);
					pGame->PlayGameSound('E', 25, 0, 0);
				}
			}
			if (iV4 == -1)
			{
				pGame->CrusadeContributionResult(0); 
			}
		}
	}

	void HandleGrandMagicResult(CGame* pGame, char* pData)
	{
		char cTxt[120]{};
		int sV1, sV2, sV3, sV4, sV5, sV6, sV7, sV8, sV9;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyGrandMagicResult>(
			pData, sizeof(hb::net::PacketNotifyGrandMagicResult));
		if (!pkt) return;
		sV1 = pkt->crashed_structures;
		sV2 = pkt->structure_damage;
		sV3 = pkt->casualities;
		memcpy(cTxt, pkt->map_name, sizeof(pkt->map_name));
		sV4 = pkt->active_structure;
		sV5 = pkt->value_count;
		sV6 = sV7 = sV8 = sV9 = 0;
		if (sV5 > 0) sV6 = pkt->values[0];
		if (sV5 > 1) sV7 = pkt->values[1];
		if (sV5 > 2) sV8 = pkt->values[2];
		if (sV5 > 3) sV9 = pkt->values[3];

		pGame->GrandMagicResult(cTxt, sV1, sV2, sV3, sV4, sV6, sV7, sV8, sV9);
	}

	void HandleMeteorStrikeComing(CGame* pGame, char* pData)
	{
		int sV1;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyMeteorStrikeComing>(
			pData, sizeof(hb::net::PacketNotifyMeteorStrikeComing));
		if (!pkt) return;
		sV1 = pkt->phase;
		pGame->MeteorStrikeComing(sV1);
		pGame->PlayGameSound('E', 25, 0, 0);
	}

	void HandleMeteorStrikeHit(CGame* pGame, char* pData)
	{
		int i;
		pGame->SetTopMsg(pGame->m_pGameMsgList[17]->m_pMsg, 5);
		for (i = 0; i < 36; i++)
			pGame->m_pEffectManager->AddEffect(EffectType::METEOR_FLYING, pGame->m_Camera.GetX() + (rand() % LOGICAL_MAX_X()), pGame->m_Camera.GetY() + (rand() % LOGICAL_MAX_Y()), 0, 0, -(rand() % 80));
	}

	void HandleCannotConstruct(CGame* pGame, char* pData)
	{
		short sV1;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyCannotConstruct>(
			pData, sizeof(hb::net::PacketNotifyCannotConstruct));
		if (!pkt) return;
		sV1 = static_cast<short>(pkt->reason);
		pGame->CannotConstruct(sV1);
		pGame->PlayGameSound('E', 25, 0, 0);
	}

	void HandleTCLoc(CGame* pGame, char* pData)
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyTCLoc>(
			pData, sizeof(hb::net::PacketNotifyTCLoc));
		if (!pkt) return;
		TeleportManager::Get().SetLocation(pkt->dest_x, pkt->dest_y);
		TeleportManager::Get().SetMapName(pkt->teleport_map, sizeof(pkt->teleport_map));
		pGame->m_pPlayer->m_iConstructLocX = pkt->construct_x;
		pGame->m_pPlayer->m_iConstructLocY = pkt->construct_y;
		pGame->m_cConstructMapName.assign(pkt->construct_map, strnlen(pkt->construct_map, sizeof(pkt->construct_map)));
	}

	void HandleConstructionPoint(CGame* pGame, char* pData)
	{
		short sV1, sV2, sV3;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyConstructionPoint>(
			pData, sizeof(hb::net::PacketNotifyConstructionPoint));
		if (!pkt) return;
		sV1 = static_cast<short>(pkt->construction_point);
		sV2 = static_cast<short>(pkt->war_contribution);
		sV3 = static_cast<short>(pkt->notify_type);

		if (sV3 == 0) {
			if ((sV1 > pGame->m_pPlayer->m_iConstructionPoint) && (sV2 > pGame->m_pPlayer->m_iWarContribution)) {
				G_cTxt = std::format("{} +{}, {} +{}", pGame->m_pGameMsgList[13]->m_pMsg, (sV1 - pGame->m_pPlayer->m_iConstructionPoint), pGame->m_pGameMsgList[21]->m_pMsg, (sV2 - pGame->m_pPlayer->m_iWarContribution));
				pGame->SetTopMsg(G_cTxt.c_str(), 5);
				pGame->PlayGameSound('E', 23, 0, 0);
			}

			if ((sV1 > pGame->m_pPlayer->m_iConstructionPoint) && (sV2 == pGame->m_pPlayer->m_iWarContribution)) {
				if (pGame->m_pPlayer->m_iCrusadeDuty == 3) {
					G_cTxt = std::format("{} +{}", pGame->m_pGameMsgList[13]->m_pMsg, sV1 - pGame->m_pPlayer->m_iConstructionPoint);
					pGame->SetTopMsg(G_cTxt.c_str(), 5);
					pGame->PlayGameSound('E', 23, 0, 0);
				}
			}

			if ((sV1 == pGame->m_pPlayer->m_iConstructionPoint) && (sV2 > pGame->m_pPlayer->m_iWarContribution)) {
				G_cTxt = std::format("{} +{}", pGame->m_pGameMsgList[21]->m_pMsg, sV2 - pGame->m_pPlayer->m_iWarContribution);
				pGame->SetTopMsg(G_cTxt.c_str(), 5);
				pGame->PlayGameSound('E', 23, 0, 0);
			}

			if (sV1 < pGame->m_pPlayer->m_iConstructionPoint) {
				if (pGame->m_pPlayer->m_iCrusadeDuty == 3) {
					G_cTxt = std::format("{} -{}", pGame->m_pGameMsgList[13]->m_pMsg, pGame->m_pPlayer->m_iConstructionPoint - sV1);
					pGame->SetTopMsg(G_cTxt.c_str(), 5);
					pGame->PlayGameSound('E', 25, 0, 0);
				}
			}

			if (sV2 < pGame->m_pPlayer->m_iWarContribution) {
				G_cTxt = std::format("{} -{}", pGame->m_pGameMsgList[21]->m_pMsg, pGame->m_pPlayer->m_iWarContribution - sV2);
				pGame->SetTopMsg(G_cTxt.c_str(), 5);
				pGame->PlayGameSound('E', 24, 0, 0);
			}
		}

		pGame->m_pPlayer->m_iConstructionPoint = sV1;
		pGame->m_pPlayer->m_iWarContribution = sV2;
	}

	void HandleNoMoreCrusadeStructure(CGame* pGame, char* pData)
	{
		pGame->SetTopMsg(pGame->m_pGameMsgList[12]->m_pMsg, 5);
		pGame->PlayGameSound('E', 25, 0, 0);
	}

	void HandleEnergySphereGoalIn(CGame* pGame, char* pData)
	{
		int sV1, sV2, sV3;
		char cTxt[120]{};
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyEnergySphereGoalIn>(
			pData, sizeof(hb::net::PacketNotifyEnergySphereGoalIn));
		if (!pkt) return;
		sV1 = pkt->result;
		sV2 = pkt->side;
		sV3 = pkt->goal;
		memcpy(cTxt, pkt->name, sizeof(pkt->name));

		if (sV2 == sV3)
		{
			pGame->PlayGameSound('E', 24, 0);
			if (strcmp(cTxt, pGame->m_pPlayer->m_cPlayerName.c_str()) == 0)
			{
				pGame->AddEventList(NOTIFY_MSG_HANDLER33, 10); // "You pushed energy sphere to enemy's energy portal! Contribution point will be decreased by 10 points."
				pGame->m_pPlayer->m_iContribution += sV1; // fixed, server must match...
				pGame->m_iContributionPrice = 0;
				if (pGame->m_pPlayer->m_iContribution < 0) pGame->m_pPlayer->m_iContribution = 0;
			}
			else {
				if (pGame->m_pPlayer->m_bAresden == true) G_cTxt = std::format(NOTIFY_MSG_HANDLER34, cTxt); // "%s(Aresden) pushed energy sphere to enemy's portal!!..."
				else if (pGame->m_pPlayer->m_bAresden == false) G_cTxt = std::format(NOTIFY_MSG_HANDLER34_ELV, cTxt); // "%s(Elvine) pushed energy sphere to enemy's portal!!..."
				pGame->AddEventList(G_cTxt.c_str(), 10);
			}
		}
		else
		{
			pGame->PlayGameSound('E', 23, 0);
			if (strcmp(cTxt, pGame->m_pPlayer->m_cPlayerName.c_str()) == 0)
			{
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
				pGame->AddEventList(NOTIFY_MSG_HANDLER35, 10); // "Congulaturations! You brought energy sphere to energy portal and earned experience and prize gold!"
				pGame->m_pPlayer->m_iContribution += 5;
				if (pGame->m_pPlayer->m_iContribution < 0) pGame->m_pPlayer->m_iContribution = 0;
			}
			else
			{
				if (sV3 == 1)
				{
					G_cTxt = std::format(NOTIFY_MSG_HANDLER36, cTxt); // "Elvine %s : Goal in!"
					pGame->AddEventList(G_cTxt.c_str(), 10);
				}
				else if (sV3 == 2)
				{
					G_cTxt = std::format(NOTIFY_MSG_HANDLER37, cTxt); // "Aresden %s : Goal in!"
					pGame->AddEventList(G_cTxt.c_str(), 10);
				}
			}
		}
	}

	void HandleEnergySphereCreated(CGame* pGame, char* pData)
	{
		int sV1, sV2;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyEnergySphereCreated>(
			pData, sizeof(hb::net::PacketNotifyEnergySphereCreated));
		if (!pkt) return;
		sV1 = pkt->x;
		sV2 = pkt->y;
		G_cTxt = std::format(NOTIFY_MSG_HANDLER38, sV1, sV2); // "Energy sphere was dropped in (%d, %d) of middleland!"
		pGame->AddEventList(G_cTxt.c_str(), 10);
		pGame->AddEventList(NOTIFY_MSG_HANDLER39, 10); // "A player who pushed energy sphere to the energy portal of his city will earn many Exp and Contribution."
	}
}
