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
	void HandleDownSkillIndexSet(CGame* pGame, char* pData)
	{
		short sSkillIndex;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyDownSkillIndexSet>(
			pData, sizeof(hb::net::PacketNotifyDownSkillIndexSet));
		if (!pkt) return;
		sSkillIndex = static_cast<short>(pkt->skill_index);
		pGame->m_iDownSkillIndex = sSkillIndex;
		pGame->m_dialogBoxManager.Info(DialogBoxId::Skill).bFlag = false;
	}

	void HandleMagicStudyFail(CGame* pGame, char* pData)
	{
		char cMagicNum, cName[31]{}, cFailCode;
		std::string cTxt;
		int iCost, iReqInt;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyMagicStudyFail>(
			pData, sizeof(hb::net::PacketNotifyMagicStudyFail));
		if (!pkt) return;
		cFailCode = static_cast<char>(pkt->result);
		cMagicNum = static_cast<char>(pkt->magic_id);
		memcpy(cName, pkt->magic_name, 30);
		iCost = pkt->cost;
		iReqInt = pkt->req_int;

		if (iCost > 0)
		{
			cTxt = std::format(NOTIFYMSG_MAGICSTUDY_FAIL1, cName);
			pGame->AddEventList(cTxt.c_str(), 10);
		}
		else
		{
			cTxt = std::format(NOTIFYMSG_MAGICSTUDY_FAIL2, cName);
			pGame->AddEventList(cTxt.c_str(), 10);
			cTxt = std::format(NOTIFYMSG_MAGICSTUDY_FAIL3, iReqInt);
			pGame->AddEventList(cTxt.c_str(), 10);
		}
	}

	void HandleMagicStudySuccess(CGame* pGame, char* pData)
	{
		char cMagicNum, cName[31]{};
		std::string cTxt;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyMagicStudySuccess>(
			pData, sizeof(hb::net::PacketNotifyMagicStudySuccess));
		if (!pkt) return;
		cMagicNum = static_cast<char>(pkt->magic_id);
		pGame->m_pPlayer->m_iMagicMastery[cMagicNum] = 1;
	  // Magic learned - affects magic list
		memcpy(cName, pkt->magic_name, 30);
		cTxt = std::format(NOTIFYMSG_MAGICSTUDY_SUCCESS1, cName);
		pGame->AddEventList(cTxt.c_str(), 10);
		pGame->PlayGameSound('E', 23, 0);
	}

	void HandleSkillTrainSuccess(CGame* pGame, char* pData)
	{
		char cSkillNum, cSkillLevel;
		std::string cTemp;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifySkillTrainSuccess>(
			pData, sizeof(hb::net::PacketNotifySkillTrainSuccess));
		if (!pkt) return;
		cSkillNum = static_cast<char>(pkt->skill_num);
		cSkillLevel = static_cast<char>(pkt->skill_level);
		cTemp = std::format(NOTIFYMSG_SKILL_TRAIN_SUCCESS1, pGame->m_pSkillCfgList[cSkillNum]->m_cName, cSkillLevel);
		pGame->AddEventList(cTemp.c_str(), 10);
		pGame->m_pSkillCfgList[cSkillNum]->m_iLevel = cSkillLevel;
		pGame->m_pPlayer->m_iSkillMastery[cSkillNum] = static_cast<unsigned char>(cSkillLevel);
		pGame->PlayGameSound('E', 23, 0);
	}

	void HandleSkill(CGame* pGame, char* pData)
	{
		short sSkillIndex, sValue;
		std::string cTxt;

		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifySkill>(
			pData, sizeof(hb::net::PacketNotifySkill));
		if (!pkt) return;
		sSkillIndex = static_cast<short>(pkt->skill_index);
		sValue = static_cast<short>(pkt->skill_value);
		pGame->m_floatingText.RemoveByObjectID(pGame->m_pPlayer->m_sPlayerObjectID);
		if (pGame->m_pSkillCfgList[sSkillIndex]->m_iLevel < sValue)
		{
			cTxt = std::format(NOTIFYMSG_SKILL1, pGame->m_pSkillCfgList[sSkillIndex]->m_cName, sValue - pGame->m_pSkillCfgList[sSkillIndex]->m_iLevel);
			pGame->AddEventList(cTxt.c_str(), 10);
			pGame->PlayGameSound('E', 23, 0);
			cTxt = std::format("{} +{}%", pGame->m_pSkillCfgList[sSkillIndex]->m_cName, sValue - pGame->m_pSkillCfgList[sSkillIndex]->m_iLevel);
			pGame->m_floatingText.AddNotifyText(NotifyTextType::SkillChange, cTxt.c_str(), pGame->m_dwCurTime,
				pGame->m_pPlayer->m_sPlayerObjectID, pGame->m_pMapData.get());
		}
		else if (pGame->m_pSkillCfgList[sSkillIndex]->m_iLevel > sValue) {
			cTxt = std::format(NOTIFYMSG_SKILL2, pGame->m_pSkillCfgList[sSkillIndex]->m_cName, pGame->m_pSkillCfgList[sSkillIndex]->m_iLevel - sValue);
			pGame->AddEventList(cTxt.c_str(), 10);
			pGame->PlayGameSound('E', 24, 0);
			cTxt = std::format("{} -{}%", pGame->m_pSkillCfgList[sSkillIndex]->m_cName, sValue - pGame->m_pSkillCfgList[sSkillIndex]->m_iLevel);
			pGame->m_floatingText.AddNotifyText(NotifyTextType::SkillChange, cTxt.c_str(), pGame->m_dwCurTime,
				pGame->m_pPlayer->m_sPlayerObjectID, pGame->m_pMapData.get());
		}
		pGame->m_pSkillCfgList[sSkillIndex]->m_iLevel = sValue;
		pGame->m_pPlayer->m_iSkillMastery[sSkillIndex] = static_cast<unsigned char>(sValue);
	}

	void HandleSkillUsingEnd(CGame* pGame, char* pData)
	{
		WORD wResult;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifySkillUsingEnd>(
			pData, sizeof(hb::net::PacketNotifySkillUsingEnd));
		if (!pkt) return;
		wResult = pkt->result;
		switch (wResult) {
		case 0:
			pGame->AddEventList(NOTIFYMSG_SKILL_USINGEND1, 10);
			break;
		case 1:
			pGame->AddEventList(NOTIFYMSG_SKILL_USINGEND2, 10);
			break;
		}
		pGame->m_bSkillUsingStatus = false;
	}

	void HandleMagicEffectOn(CGame* pGame, char* pData)
	{
		short  sMagicType, sMagicEffect, sOwnerH;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyMagicEffect>(
			pData, sizeof(hb::net::PacketNotifyMagicEffect));
		if (!pkt) return;
		sMagicType = static_cast<short>(pkt->magic_type);
		sMagicEffect = static_cast<short>(pkt->effect);
		sOwnerH = static_cast<short>(pkt->owner);
		switch (sMagicType) {
		case hb::shared::magic::Protect:
			switch (sMagicEffect) {
			case 1: // "You are completely protected from arrows!"
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_ON1, 10);
				break;
			case 2: // "You are protected from magic!"
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_ON2, 10);
				break;
			case 3: // "Defense ratio increased by a magic shield!"
			case 4: // "Defense ratio increased by a magic shield!"
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_ON3, 10);
				break;
			case 5: // "You are completely protected from magic!"
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_ON14, 10);
				break;
			}
			break;

		case hb::shared::magic::HoldObject:
			switch (sMagicEffect) {
			case 1: // "You were bounded by a Hold Person spell! Unable to move!"
				pGame->m_pPlayer->m_bParalyze = true;
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_ON4, 10);
				break;
			case 2: // "You were bounded by a Paralysis spell! Unable to move!"
				pGame->m_pPlayer->m_bParalyze = true;
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_ON5, 10);
				break;
			}
			break;

		case hb::shared::magic::Invisibility:
			switch (sMagicEffect) {
			case 1: // "You are now invisible, no one can see you!"
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_ON6, 10);
				break;
			}
			break;

		case hb::shared::magic::Confuse:
			switch (sMagicEffect) {
			case 1:	// Confuse Language "No one understands you because of language confusion magic!"
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_ON7, 10);
				break;

			case 2: // Confusion "Confusion magic casted, impossible to determine player allegience."
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_ON8, 10);
				pGame->m_pPlayer->m_bIsConfusion = true;
				break;

			case 3:	// Illusion "Illusion magic casted, impossible to tell who is who!"
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_ON9, 10);
				pGame->_SetIlusionEffect(sOwnerH);
				break;

			case 4:	// IllusionMouvement "You are thrown into confusion, and you are flustered yourself." // snoopy
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_ON15, 10);
				pGame->m_bIllusionMVT = true;
				break;
			}
			break;

		case hb::shared::magic::Poison:
			pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_ON10, 10);
			pGame->m_pPlayer->m_bIsPoisoned = true;
			break;

		case hb::shared::magic::Berserk:
			switch (sMagicEffect) {
			case 1:
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_ON11, 10);
				break;
			}
			break;

		case hb::shared::magic::Polymorph:
			switch (sMagicEffect) {
			case 1:
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_ON12, 10);
				break;
			}
			break;

		case hb::shared::magic::Ice:
			pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_ON13, 10);
			break;
		}
	}

	void HandleMagicEffectOff(CGame* pGame, char* pData)
	{
		short  sMagicType, sMagicEffect;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyMagicEffect>(
			pData, sizeof(hb::net::PacketNotifyMagicEffect));
		if (!pkt) return;
		sMagicType = static_cast<short>(pkt->magic_type);
		sMagicEffect = static_cast<short>(pkt->effect);
		switch (sMagicType) {
		case hb::shared::magic::Protect:
			switch (sMagicEffect) {
			case 1: // "Protection from arrows has vanished."
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_OFF1, 10);
				break;
			case 2:	// "Protection from magic has vanished."
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_OFF2, 10);
				break;
			case 3:	// "Defense shield effect has vanished."
			case 4:	// "Defense shield effect has vanished."
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_OFF3, 10);
				break;
			case 5:	// "Absolute Magic Protection has been vanished."
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_OFF14, 10);
				break;
			}
			break;

		case hb::shared::magic::HoldObject:
			switch (sMagicEffect) {
			case 1:	// "Hold person magic effect has vanished."
				pGame->m_pPlayer->m_bParalyze = false;
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_OFF4, 10);
				break;

			case 2:	// "Paralysis magic effect has vanished."
				pGame->m_pPlayer->m_bParalyze = false;
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_OFF5, 10);
				break;
			}
			break;

		case hb::shared::magic::Invisibility:
			switch (sMagicEffect) {
			case 1:	// "Invisibility magic effect has vanished."
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_OFF6, 10);
				break;
			}
			break;

		case hb::shared::magic::Confuse:
			switch (sMagicEffect) {
			case 1:	// "Language confuse magic effect has vanished."
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_OFF7, 10);
				break;
			case 2:	// "Confusion magic has vanished."
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_OFF8, 10);
				pGame->m_pPlayer->m_bIsConfusion = false;
				break;
			case 3:	// "Illusion magic has vanished."
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_OFF9, 10);
				pGame->m_iIlusionOwnerH = 0;
				break;
			case 4:	// "At last, you gather your senses." // snoopy
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_OFF15, 10);
				pGame->m_bIllusionMVT = false;
				break;
			}
			break;

		case hb::shared::magic::Poison:
			if (pGame->m_pPlayer->m_bIsPoisoned) pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_OFF10, 10);
			pGame->m_pPlayer->m_bIsPoisoned = false;
			break;

		case hb::shared::magic::Berserk:
			switch (sMagicEffect) {
			case 1:
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_OFF11, 10);
				break;
			}
			break;

		case hb::shared::magic::Polymorph:
			switch (sMagicEffect) {
			case 1:
				pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_OFF12, 10);
				break;
			}
			break;

		case hb::shared::magic::Ice:
			pGame->AddEventList(NOTIFYMSG_MAGICEFFECT_OFF13, 10);
			break;
		}
	}

	void HandleSpellSkill(CGame* pGame, char* pData)
	{
		int i;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifySpellSkill>(
			pData, sizeof(hb::net::PacketNotifySpellSkill));
		if (!pkt) return;
		for (i = 0; i < hb::shared::limits::MaxMagicType; i++) {
			pGame->m_pPlayer->m_iMagicMastery[i] = pkt->magic_mastery[i];
		}
		for (i = 0; i < hb::shared::limits::MaxSkillType; i++) {
			pGame->m_pPlayer->m_iSkillMastery[i] = pkt->skill_mastery[i];
			if (pGame->m_pSkillCfgList[i] != 0)
				pGame->m_pSkillCfgList[i]->m_iLevel = pkt->skill_mastery[i];
		}
	}

	void HandleStateChangeSuccess(CGame* pGame, char* pData)
	{
		int i;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyStateChangeSuccess>(
			pData, sizeof(hb::net::PacketNotifyStateChangeSuccess));
		if (!pkt) return;
		for (i = 0; i < hb::shared::limits::MaxMagicType; i++) {
			pGame->m_pPlayer->m_iMagicMastery[i] = pkt->magic_mastery[i];
		}
		for (i = 0; i < hb::shared::limits::MaxSkillType; i++) {
			pGame->m_pPlayer->m_iSkillMastery[i] = pkt->skill_mastery[i];
			if (pGame->m_pSkillCfgList[i] != 0)
				pGame->m_pSkillCfgList[i]->m_iLevel = pkt->skill_mastery[i];
		}
		// Calculate majestic cost before applying (m_wLU_* are negative for reductions)
		int iTotalReduction = -(pGame->m_pPlayer->m_wLU_Str + pGame->m_pPlayer->m_wLU_Vit +
			pGame->m_pPlayer->m_wLU_Dex + pGame->m_pPlayer->m_wLU_Int +
			pGame->m_pPlayer->m_wLU_Mag + pGame->m_pPlayer->m_wLU_Char);
		int iMajesticCost = iTotalReduction / 3;

		// Apply pending stat changes (adds negative values = reduces stats)
		pGame->m_pPlayer->m_iStr += pGame->m_pPlayer->m_wLU_Str;
		pGame->m_pPlayer->m_iVit += pGame->m_pPlayer->m_wLU_Vit;
		pGame->m_pPlayer->m_iDex += pGame->m_pPlayer->m_wLU_Dex;
		pGame->m_pPlayer->m_iInt += pGame->m_pPlayer->m_wLU_Int;
		pGame->m_pPlayer->m_iMag += pGame->m_pPlayer->m_wLU_Mag;
		pGame->m_pPlayer->m_iCharisma += pGame->m_pPlayer->m_wLU_Char;
		pGame->m_pPlayer->m_iLU_Point = (pGame->m_pPlayer->m_iLevel - 1) * 3 - ((pGame->m_pPlayer->m_iStr + pGame->m_pPlayer->m_iVit + pGame->m_pPlayer->m_iDex + pGame->m_pPlayer->m_iInt + pGame->m_pPlayer->m_iMag + pGame->m_pPlayer->m_iCharisma) - 70);
		pGame->m_iGizonItemUpgradeLeft -= iMajesticCost;
		pGame->m_pPlayer->m_wLU_Str = pGame->m_pPlayer->m_wLU_Vit = pGame->m_pPlayer->m_wLU_Dex = pGame->m_pPlayer->m_wLU_Int = pGame->m_pPlayer->m_wLU_Mag = pGame->m_pPlayer->m_wLU_Char = 0;
		pGame->m_dialogBoxManager.DisableDialogBox(DialogBoxId::ChangeStatsMajestic);
		pGame->m_dialogBoxManager.EnableDialogBox(DialogBoxId::LevelUpSetting, 0, 0, 0);
		pGame->AddEventList("Your stat has been changed.", 10);
	}

	void HandleStateChangeFailed(CGame* pGame, char* pData)
	{
		pGame->m_pPlayer->m_wLU_Str = pGame->m_pPlayer->m_wLU_Vit = pGame->m_pPlayer->m_wLU_Dex = pGame->m_pPlayer->m_wLU_Int = pGame->m_pPlayer->m_wLU_Mag = pGame->m_pPlayer->m_wLU_Char = 0;
		pGame->m_pPlayer->m_iLU_Point = (pGame->m_pPlayer->m_iLevel - 1) * 3 - ((pGame->m_pPlayer->m_iStr + pGame->m_pPlayer->m_iVit + pGame->m_pPlayer->m_iDex + pGame->m_pPlayer->m_iInt + pGame->m_pPlayer->m_iMag + pGame->m_pPlayer->m_iCharisma) - 70);
		pGame->m_dialogBoxManager.DisableDialogBox(DialogBoxId::ChangeStatsMajestic);
		pGame->AddEventList("Your stat has not been changed.", 10);
	}

	void HandleSettingFailed(CGame* pGame, char* pData)
	{
		pGame->AddEventList("Your stat has not been changed.", 10);
		pGame->m_pPlayer->m_wLU_Str = pGame->m_pPlayer->m_wLU_Vit = pGame->m_pPlayer->m_wLU_Dex = pGame->m_pPlayer->m_wLU_Int = pGame->m_pPlayer->m_wLU_Mag = pGame->m_pPlayer->m_wLU_Char = 0;
		pGame->m_pPlayer->m_iLU_Point = (pGame->m_pPlayer->m_iLevel - 1) * 3 - ((pGame->m_pPlayer->m_iStr + pGame->m_pPlayer->m_iVit + pGame->m_pPlayer->m_iDex + pGame->m_pPlayer->m_iInt + pGame->m_pPlayer->m_iMag + pGame->m_pPlayer->m_iCharisma) - 70);
	}

	void HandleSpecialAbilityStatus(CGame* pGame, char* pData)
	{
		std::string G_cTxt;
		short sV1, sV2, sV3;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifySpecialAbilityStatus>(
			pData, sizeof(hb::net::PacketNotifySpecialAbilityStatus));
		if (!pkt) return;
		sV1 = pkt->status_type;
		sV2 = pkt->ability_type;
		sV3 = pkt->seconds_left;

		if (sV1 == 1) // Use SA
		{
			pGame->PlayGameSound('E', 35, 0);
			pGame->AddEventList(NOTIFY_MSG_HANDLER4, 10); 
			switch (sV2) {
			case 1: G_cTxt = std::format(NOTIFY_MSG_HANDLER5, sV3); break;
			case 2: G_cTxt = std::format(NOTIFY_MSG_HANDLER6, sV3); break;
			case 3: G_cTxt = std::format(NOTIFY_MSG_HANDLER7, sV3); break;
			case 4: G_cTxt = std::format(NOTIFY_MSG_HANDLER8, sV3); break;
			case 5: G_cTxt = std::format(NOTIFY_MSG_HANDLER9, sV3); break;
			case 50:G_cTxt = std::format(NOTIFY_MSG_HANDLER10, sV3); break;
			case 51:G_cTxt = std::format(NOTIFY_MSG_HANDLER11, sV3); break;
			case 52:G_cTxt = std::format(NOTIFY_MSG_HANDLER12, sV3); break;
			case 55: 
				if (sV3 > 90)
					G_cTxt = std::format("You cast a powerfull incantation, you can't use it again before {} minutes.", sV3 / 60);
				else
					G_cTxt = std::format("You cast a powerfull incantation, you can't use it again before {} seconds.", sV3);
				break;
			}
			pGame->AddEventList(G_cTxt.c_str(), 10);
		}
		else if (sV1 == 2) // Finished using
		{
			if (pGame->m_pPlayer->m_iSpecialAbilityType != static_cast<int>(sV2))
			{
				pGame->PlayGameSound('E', 34, 0);
				pGame->AddEventList(NOTIFY_MSG_HANDLER13, 10);
				if (sV3 >= 60)
				{
					switch (sV2) {
					case 1: G_cTxt = std::format(NOTIFY_MSG_HANDLER14, sV3 / 60); pGame->AddEventList(G_cTxt.c_str(), 10); break;
					case 2: G_cTxt = std::format(NOTIFY_MSG_HANDLER15, sV3 / 60); pGame->AddEventList(G_cTxt.c_str(), 10); break;
					case 3: G_cTxt = std::format(NOTIFY_MSG_HANDLER16, sV3 / 60); pGame->AddEventList(G_cTxt.c_str(), 10); break;
					case 4: G_cTxt = std::format(NOTIFY_MSG_HANDLER17, sV3 / 60); pGame->AddEventList(G_cTxt.c_str(), 10); break;
					case 5: G_cTxt = std::format(NOTIFY_MSG_HANDLER18, sV3 / 60); pGame->AddEventList(G_cTxt.c_str(), 10); break;
					case 50:G_cTxt = std::format(NOTIFY_MSG_HANDLER19, sV3 / 60); pGame->AddEventList(G_cTxt.c_str(), 10); break;
					case 51:G_cTxt = std::format(NOTIFY_MSG_HANDLER20, sV3 / 60); pGame->AddEventList(G_cTxt.c_str(), 10); break;
					case 52:G_cTxt = std::format(NOTIFY_MSG_HANDLER21, sV3 / 60); pGame->AddEventList(G_cTxt.c_str(), 10); break;
					}
				}
				else
				{
					switch (sV2) {
					case 1: G_cTxt = std::format(NOTIFY_MSG_HANDLER22, sV3); pGame->AddEventList(G_cTxt.c_str(), 10); break;
					case 2: G_cTxt = std::format(NOTIFY_MSG_HANDLER23, sV3); pGame->AddEventList(G_cTxt.c_str(), 10); break;
					case 3: G_cTxt = std::format(NOTIFY_MSG_HANDLER24, sV3); pGame->AddEventList(G_cTxt.c_str(), 10); break;
					case 4: G_cTxt = std::format(NOTIFY_MSG_HANDLER25, sV3); pGame->AddEventList(G_cTxt.c_str(), 10); break;
					case 5: G_cTxt = std::format(NOTIFY_MSG_HANDLER26, sV3); pGame->AddEventList(G_cTxt.c_str(), 10); break;
					case 50:G_cTxt = std::format(NOTIFY_MSG_HANDLER27, sV3); pGame->AddEventList(G_cTxt.c_str(), 10); break;
					case 51:G_cTxt = std::format(NOTIFY_MSG_HANDLER28, sV3); pGame->AddEventList(G_cTxt.c_str(), 10); break;
					case 52:G_cTxt = std::format(NOTIFY_MSG_HANDLER29, sV3); pGame->AddEventList(G_cTxt.c_str(), 10); break;
					}
				}
			}
			pGame->m_pPlayer->m_iSpecialAbilityType = static_cast<int>(sV2);
			pGame->m_dwSpecialAbilitySettingTime = pGame->m_dwCurTime;
			pGame->m_pPlayer->m_iSpecialAbilityTimeLeftSec = static_cast<int>(sV3);
		}
		else if (sV1 == 3)  // End of using time
		{
			pGame->m_pPlayer->m_bIsSpecialAbilityEnabled = false;
			pGame->m_dwSpecialAbilitySettingTime = pGame->m_dwCurTime;
			if (sV3 == 0)
			{
				pGame->m_pPlayer->m_iSpecialAbilityTimeLeftSec = 1200;
				pGame->AddEventList(NOTIFY_MSG_HANDLER30, 10);
			}
			else
			{
				pGame->m_pPlayer->m_iSpecialAbilityTimeLeftSec = static_cast<int>(sV3);
				if (sV3 > 90)
					G_cTxt = std::format("Special ability has run out! Will be available in {} minutes.", sV3 / 60);
				else G_cTxt = std::format("Special ability has run out! Will be available in {} seconds.", sV3);
				pGame->AddEventList(G_cTxt.c_str(), 10);
			}
		}
		else if (sV1 == 4) // Unequiped the SA item
		{
			pGame->AddEventList(NOTIFY_MSG_HANDLER31, 10);
			pGame->m_pPlayer->m_iSpecialAbilityType = 0;
		}
		else if (sV1 == 5) // Angel
		{
			pGame->PlayGameSound('E', 52, 0); 
		}
	}

	void HandleSpecialAbilityEnabled(CGame* pGame, char* pData)
	{
		if (pGame->m_pPlayer->m_bIsSpecialAbilityEnabled == false) {
			pGame->PlayGameSound('E', 30, 5);
			pGame->AddEventList(NOTIFY_MSG_HANDLER32, 10);
		}
		pGame->m_pPlayer->m_bIsSpecialAbilityEnabled = true;
	}

	void HandleSkillTrainFail(CGame* pGame, char* pData)
	{
		pGame->AddEventList("You failed to train skill.", 10);
		pGame->PlayGameSound('E', 24, 0);
	}
}
