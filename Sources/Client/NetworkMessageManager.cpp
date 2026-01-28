#include "Game.h"
#include "NetworkMessageManager.h"
#include "Packet/SharedPackets.h"
#include "lan_eng.h"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <cmath>

namespace NetworkMessageHandlers
{
	// Stats
	void HandleHP(CGame *pGame, char *pData);
	void HandleMP(CGame *pGame, char *pData);
	void HandleSP(CGame *pGame, char *pData);
	void HandleExp(CGame *pGame, char *pData);
	void HandleLevelUp(CGame *pGame, char *pData);

	// Exchange
	void HandleExchangeItemComplete(CGame *pGame, char *pData);
	void HandleCancelExchangeItem(CGame *pGame, char *pData);

	// Bank
	void HandleItemToBank(CGame *pGame, char *pData);
	void HandleCannotItemToBank(CGame *pGame, char *pData);

	// Slates
	void HandleSlateCreateSuccess(CGame *pGame, char *pData);
	void HandleSlateCreateFail(CGame *pGame, char *pData);
	void HandleSlateInvincible(CGame *pGame, char *pData);
	void HandleSlateMana(CGame *pGame, char *pData);
	void HandleSlateExp(CGame *pGame, char *pData);
	void HandleSlateStatus(CGame *pGame, char *pData);
	void HandleSlateBerserk(CGame *pGame, char *pData);

	// Map
	void HandleMapStatusNext(CGame *pGame, char *pData);
	void HandleMapStatusLast(CGame *pGame, char *pData);
	void HandleLockedMap(CGame *pGame, char *pData);
	void HandleShowMap(CGame *pGame, char *pData);

	// Events
	void HandleSpawnEvent(CGame *pGame, char *pData);
	void HandleMonsterCount(CGame *pGame, char *pData);
	void HandleResurrectPlayer(CGame *pGame, char *pData);

	// Agriculture
	void HandleAgricultureNoArea(CGame *pGame, char *pData);
	void HandleAgricultureSkillLimit(CGame *pGame, char *pData);
	void HandleNoMoreAgriculture(CGame *pGame, char *pData);

	// Angels
	void HandleAngelFailed(CGame *pGame, char *pData);
	void HandleAngelReceived(CGame *pGame, char *pData);
	void HandleAngelicStats(CGame *pGame, char *pData);

	// Party
	void HandleParty(CGame *pGame, char *pData);
	void HandleQueryJoinParty(CGame *pGame, char *pData);
	void HandleResponseCreateNewParty(CGame *pGame, char *pData);

	// Quest
	void HandleQuestContents(CGame *pGame, char *pData);
	void HandleQuestReward(CGame *pGame, char *pData);
	void HandleQuestCompleted(CGame *pGame, char *pData);
	void HandleQuestAborted(CGame *pGame, char *pData);
	void HandleQuestCounter(CGame *pGame, char *pData);

	// Fish
	void HandleFishChance(CGame *pGame, char *pData);
	void HandleEventFishMode(CGame *pGame, char *pData);
	void HandleFishCanceled(CGame *pGame, char *pData);
	void HandleFishSuccess(CGame *pGame, char *pData);
	void HandleFishFail(CGame *pGame, char *pData);

	// Items
	void HandleItemPurchased(CGame *pGame, char *pData);
	void HandleItemObtained(CGame *pGame, char *pData);
	void HandleItemLifeSpanEnd(CGame *pGame, char *pData);
	void HandleItemReleased(CGame *pGame, char *pData);
	void HandleSetItemCount(CGame *pGame, char *pData);
	void HandleItemDepleted_EraseItem(CGame *pGame, char *pData);
	void HandleDropItemFin_EraseItem(CGame *pGame, char *pData);
	void HandleGiveItemFin_EraseItem(CGame *pGame, char *pData);
	void HandleDropItemFin_CountChanged(CGame *pGame, char *pData);
	void HandleGiveItemFin_CountChanged(CGame *pGame, char *pData);
	void HandleItemRepaired(CGame *pGame, char *pData);
	void HandleRepairItemPrice(CGame *pGame, char *pData);
	void HandleRepairAllPrices(CGame *pGame, char *pData);
	void HandleSellItemPrice(CGame *pGame, char *pData);
	void HandleCannotRepairItem(CGame *pGame, char *pData);
	void HandleCannotSellItem(CGame *pGame, char *pData);
	void HandleCannotGiveItem(CGame *pGame, char *pData);
	void HandleItemColorChange(CGame *pGame, char *pData);
	void HandleSetExchangeItem(CGame *pGame, char *pData);
	void HandleOpenExchangeWindow(CGame *pGame, char *pData);
	void HandleCurLifeSpan(CGame *pGame, char *pData);
	void HandleMaxLifeSpan(CGame *pGame, char *pData);
	void HandleNotEnoughGold(CGame *pGame, char *pData);
	void HandleCannotCarryMoreItem(CGame *pGame, char *pData);
	void HandleItemAttributeChange(CGame *pGame, char *pData);
	void HandleItemUpgradeFail(CGame *pGame, char *pData);
	void HandleGizonItemUpgradeLeft(CGame *pGame, char *pData);
	void HandleGizonItemChange(CGame *pGame, char *pData);
	void HandleItemPosList(CGame *pGame, char *pData);
	void HandleItemSold(CGame *pGame, char *pData);

	// Apocalypse
	void HandleApocGateStart(CGame *pGame, char *pData);
	void HandleApocGateEnd(CGame *pGame, char *pData);
	void HandleApocGateOpen(CGame *pGame, char *pData);
	void HandleApocGateClose(CGame *pGame, char *pData);
	void HandleApocForceRecall(CGame *pGame, char *pData);
	void HandleAbaddonKilled(CGame *pGame, char *pData);

	// Heldenian
	void HandleHeldenianTeleport(CGame *pGame, char *pData);
	void HandleHeldenianEnd(CGame *pGame, char *pData);
	void HandleHeldenianStart(CGame *pGame, char *pData);
	void HandleHeldenianVictory(CGame *pGame, char *pData);
	void HandleHeldenianCount(CGame *pGame, char *pData);
	void HandleHeldenianRecall(CGame *pGame, char *pData);

	// Admin
	void HandleAdminInfo(CGame *pGame, char *pData);
	void HandleCrashHandler(CGame *pGame, char *pData);
	void HandleIpAccountInfo(CGame *pGame, char *pData);
	void HandleRewardGold(CGame *pGame, char *pData);
	void HandleServerShutdown(CGame *pGame, char *pData);
	void HandleAdminUserLevelLow(CGame *pGame, char *pData);

	// Crafting
	void HandleCraftingSuccess(CGame *pGame, char *pData);
	void HandleCraftingFail(CGame *pGame, char *pData);
	void HandleBuildItemSuccess(CGame *pGame, char *pData);
	void HandleBuildItemFail(CGame *pGame, char *pData);
	void HandlePortionSuccess(CGame *pGame, char *pData);
	void HandlePortionFail(CGame *pGame, char *pData);
	void HandleLowPortionSkill(CGame *pGame, char *pData);
	void HandleNoMatchingPortion(CGame *pGame, char *pData);

	// Guild
	void HandleCreateNewGuildResponse(CGame *pGame, char *pData);
	void HandleDisbandGuildResponse(CGame *pGame, char *pData);
	void HandleGuildDisbanded(CGame *pGame, char *pData);
	void HandleNewGuildsMan(CGame *pGame, char *pData);
	void HandleDismissGuildsMan(CGame *pGame, char *pData);
	void HandleCannotJoinMoreGuildsMan(CGame *pGame, char *pData);
	void HandleJoinGuildApprove(CGame *pGame, char *pData);
	void HandleJoinGuildReject(CGame *pGame, char *pData);
	void HandleDismissGuildApprove(CGame *pGame, char *pData);
	void HandleDismissGuildReject(CGame *pGame, char *pData);
	void HandleQueryJoinGuildPermission(CGame *pGame, char *pData);
	void HandleQueryDismissGuildPermission(CGame *pGame, char *pData);
	void HandleReqGuildNameAnswer(CGame *pGame, char *pData);
	void HandleNoGuildMasterLevel(CGame *pGame, char *pData);
	void HandleSuccessBanGuildMan(CGame *pGame, char *pData);
	void HandleCannotBanGuildMan(CGame *pGame, char *pData);

	// Combat
	void HandleKilled(CGame *pGame, char *pData);
	void HandlePKcaptured(CGame *pGame, char *pData);
	void HandlePKpenalty(CGame *pGame, char *pData);
	void HandleEnemyKills(CGame *pGame, char *pData);
	void HandleEnemyKillReward(CGame *pGame, char *pData);
	void HandleGlobalAttackMode(CGame *pGame, char *pData);
	void HandleDamageMove(CGame *pGame, char *pData);
	void HandleObserverMode(CGame *pGame, char *pData);
	void HandleSuperAttackLeft(CGame *pGame, char *pData);
	void HandleSafeAttackMode(CGame *pGame, char *pData);
	void HandleNpcHp(CGame *pGame, char *pData);

	// Skills
	void HandleSkillTrainSuccess(CGame *pGame, char *pData);
	void HandleMagicStudySuccess(CGame *pGame, char *pData);
	void HandleMagicStudyFail(CGame *pGame, char *pData);
	void HandleDownSkillIndexSet(CGame *pGame, char *pData);
	void HandleSkill(CGame *pGame, char *pData);
	void HandleSkillUsingEnd(CGame *pGame, char *pData);
	void HandleMagicEffectOn(CGame *pGame, char *pData);
	void HandleMagicEffectOff(CGame *pGame, char *pData);
	void HandleSpellSkill(CGame *pGame, char *pData);
	void HandleStateChangeSuccess(CGame *pGame, char *pData);
	void HandleStateChangeFailed(CGame *pGame, char *pData);
	void HandleSettingFailed(CGame *pGame, char *pData);
	void HandleSpecialAbilityStatus(CGame *pGame, char *pData);
	void HandleSpecialAbilityEnabled(CGame *pGame, char *pData);
	void HandleSkillTrainFail(CGame *pGame, char *pData);

	// Player
	void HandleCharisma(CGame *pGame, char *pData);
	void HandleHunger(CGame *pGame, char *pData);
	void HandlePlayerProfile(CGame *pGame, char *pData);
	void HandlePlayerStatus(CGame *pGame, bool bOnGame, char *pData);
	void HandleWhisperMode(CGame *pGame, bool bActive, char *pData);
	void HandlePlayerShutUp(CGame *pGame, char *pData);
	void HandleRatingPlayer(CGame *pGame, char *pData);
	void HandleCannotRating(CGame *pGame, char *pData);

	// Crusade
	void HandleCrusade(CGame *pGame, char *pData);
	void HandleGrandMagicResult(CGame *pGame, char *pData);
	void HandleNoMoreCrusadeStructure(CGame *pGame, char *pData);
	void HandleEnergySphereGoalIn(CGame *pGame, char *pData);
	void HandleEnergySphereCreated(CGame *pGame, char *pData);
	void HandleMeteorStrikeComing(CGame *pGame, char *pData);
	void HandleMeteorStrikeHit(CGame *pGame, char *pData);
	void HandleCannotConstruct(CGame *pGame, char *pData);
	void HandleTCLoc(CGame *pGame, char *pData);
	void HandleConstructionPoint(CGame *pGame, char *pData);

	// System
	void HandleWhetherChange(CGame *pGame, char *pData);
	void HandleTimeChange(CGame *pGame, char *pData);
	void HandleNoticeMsg(CGame *pGame, char *pData);
	void HandleForceDisconn(CGame *pGame, char *pData);
	void HandleSettingSuccess(CGame *pGame, char *pData);
	void HandleServerChange(CGame *pGame, char *pData);
	void HandleTotalUsers(CGame *pGame, char *pData);
	void HandleChangePlayMode(CGame *pGame, char *pData);
	void HandleForceRecallTime(CGame *pGame, char *pData);
	void HandleNoRecall(CGame *pGame, char *pData);
	void HandleFightZoneReserve(CGame *pGame, char *pData);
	void HandleLoteryLost(CGame *pGame, char *pData);
	void HandleNotFlagSpot(CGame *pGame, char *pData);
	void HandleNpcTalk(CGame *pGame, char *pData);
	void HandleTravelerLimitedLevel(CGame *pGame, char *pData);
	void HandleLimitedLevel(CGame *pGame, char *pData);
	void HandleToBeRecalled(CGame *pGame, char *pData);
}

NetworkMessageManager::NetworkMessageManager(CGame *pGame)
	: m_pGame(pGame)
{
}

bool NetworkMessageManager::ProcessMessage(uint32_t dwMsgID, char *pData, uint32_t dwMsgSize)
{
	const auto *header = hb::net::PacketCast<hb::net::PacketHeader>(pData, sizeof(hb::net::PacketHeader));
	if (!header)
		return false;

	if (dwMsgID == MSGID_NOTIFY)
	{
		switch (header->msg_type)
		{
		// Stats
		case DEF_NOTIFY_HP:
			NetworkMessageHandlers::HandleHP(m_pGame, pData);
			return true;
		case DEF_NOTIFY_MP:
			NetworkMessageHandlers::HandleMP(m_pGame, pData);
			return true;
		case DEF_NOTIFY_SP:
			NetworkMessageHandlers::HandleSP(m_pGame, pData);
			return true;
		case DEF_NOTIFY_EXP:
			NetworkMessageHandlers::HandleExp(m_pGame, pData);
			return true;
		case DEF_NOTIFY_LEVELUP:
			NetworkMessageHandlers::HandleLevelUp(m_pGame, pData);
			return true;

		// Items - Purchased/Obtained
		case DEF_NOTIFY_ITEMPURCHASED:
			NetworkMessageHandlers::HandleItemPurchased(m_pGame, pData);
			return true;
		case DEF_NOTIFY_ITEMOBTAINED:
			NetworkMessageHandlers::HandleItemObtained(m_pGame, pData);
			return true;

		// Items - LifeSpan/Released
		case DEF_NOTIFY_ITEMLIFESPANEND:
			NetworkMessageHandlers::HandleItemLifeSpanEnd(m_pGame, pData);
			return true;
		case DEF_NOTIFY_ITEMRELEASED:
			NetworkMessageHandlers::HandleItemReleased(m_pGame, pData);
			return true;

		// Items - Count/Depleted
		case DEF_NOTIFY_SETITEMCOUNT:
			NetworkMessageHandlers::HandleSetItemCount(m_pGame, pData);
			return true;
		case DEF_NOTIFY_ITEMDEPLETED_ERASEITEM:
			NetworkMessageHandlers::HandleItemDepleted_EraseItem(m_pGame, pData);
			return true;

		// Items - Drop/Give
		case DEF_NOTIFY_DROPITEMFIN_ERASEITEM:
			NetworkMessageHandlers::HandleDropItemFin_EraseItem(m_pGame, pData);
			return true;
		case DEF_NOTIFY_GIVEITEMFIN_ERASEITEM:
			NetworkMessageHandlers::HandleGiveItemFin_EraseItem(m_pGame, pData);
			return true;
		case DEF_NOTIFY_DROPITEMFIN_COUNTCHANGED:
			NetworkMessageHandlers::HandleDropItemFin_CountChanged(m_pGame, pData);
			return true;
		case DEF_NOTIFY_GIVEITEMFIN_COUNTCHANGED:
			NetworkMessageHandlers::HandleGiveItemFin_CountChanged(m_pGame, pData);
			return true;

		// Items - Repair/Sell
		case DEF_NOTIFY_ITEMREPAIRED:
			NetworkMessageHandlers::HandleItemRepaired(m_pGame, pData);
			return true;
		case DEF_NOTIFY_REPAIRITEMPRICE:
			NetworkMessageHandlers::HandleRepairItemPrice(m_pGame, pData);
			return true;
		case DEF_NOTIFY_REPAIRALLPRICES:
			NetworkMessageHandlers::HandleRepairAllPrices(m_pGame, pData);
			return true;
		case DEF_NOTIFY_SELLITEMPRICE:
			NetworkMessageHandlers::HandleSellItemPrice(m_pGame, pData);
			return true;
		case DEF_NOTIFY_CANNOTREPAIRITEM:
			NetworkMessageHandlers::HandleCannotRepairItem(m_pGame, pData);
			return true;
		case DEF_NOTIFY_CANNOTSELLITEM:
			NetworkMessageHandlers::HandleCannotSellItem(m_pGame, pData);
			return true;

		// Items - Give/Cannot
		case DEF_NOTIFY_CANNOTGIVEITEM:
			NetworkMessageHandlers::HandleCannotGiveItem(m_pGame, pData);
			return true;

		// Items - Color/Exchange
		case DEF_NOTIFY_ITEMCOLORCHANGE:
			NetworkMessageHandlers::HandleItemColorChange(m_pGame, pData);
			return true;
		case DEF_NOTIFY_SETEXCHANGEITEM:
			NetworkMessageHandlers::HandleSetExchangeItem(m_pGame, pData);
			return true;
		case DEF_NOTIFY_OPENEXCHANGEWINDOW:
			NetworkMessageHandlers::HandleOpenExchangeWindow(m_pGame, pData);
			return true;
		case DEF_NOTIFY_CURLIFESPAN:
			NetworkMessageHandlers::HandleCurLifeSpan(m_pGame, pData);
			return true;
		case DEF_NOTIFY_MAXLIFESPAN:
			NetworkMessageHandlers::HandleMaxLifeSpan(m_pGame, pData);
			return true;

		// Items - Upgrade/Attribute/Errors
		case DEF_NOTIFY_NOTENOUGHGOLD:
			NetworkMessageHandlers::HandleNotEnoughGold(m_pGame, pData);
			return true;
		case DEF_NOTIFY_CANNOTCARRYMOREITEM:
			NetworkMessageHandlers::HandleCannotCarryMoreItem(m_pGame, pData);
			return true;
		case DEF_NOTIFY_ITEMATTRIBUTECHANGE:
			NetworkMessageHandlers::HandleItemAttributeChange(m_pGame, pData);
			return true;
		case 0x0BC0:
			NetworkMessageHandlers::HandleItemAttributeChange(m_pGame, pData);
			return true; // Same handler as ITEMATTRIBUTECHANGE
		case DEF_NOTIFY_ITEMUPGRADEFAIL:
			NetworkMessageHandlers::HandleItemUpgradeFail(m_pGame, pData);
			return true;
		case DEF_NOTIFY_GIZONITEMUPGRADELEFT:
			NetworkMessageHandlers::HandleGizonItemUpgradeLeft(m_pGame, pData);
			return true;
		case DEF_NOTIFY_GIZONEITEMCHANGE:
			NetworkMessageHandlers::HandleGizonItemChange(m_pGame, pData);
			return true;
		case DEF_NOTIFY_ITEMPOSLIST:
			NetworkMessageHandlers::HandleItemPosList(m_pGame, pData);
			return true;
		case DEF_NOTIFY_ITEMSOLD:
			NetworkMessageHandlers::HandleItemSold(m_pGame, pData);
			return true;

		// Bank
		case DEF_NOTIFY_ITEMTOBANK:
			NetworkMessageHandlers::HandleItemToBank(m_pGame, pData);
			return true;
		case DEF_NOTIFY_CANNOTITEMTOBANK:
			NetworkMessageHandlers::HandleCannotItemToBank(m_pGame, pData);
			return true;

		// Exchange
		case DEF_NOTIFY_EXCHANGEITEMCOMPLETE:
			NetworkMessageHandlers::HandleExchangeItemComplete(m_pGame, pData);
			return true;
		case DEF_NOTIFY_CANCELEXCHANGEITEM:
			NetworkMessageHandlers::HandleCancelExchangeItem(m_pGame, pData);
			return true;

		// Guild - Notify Messages
		case DEF_NOTIFY_GUILDDISBANDED:
			NetworkMessageHandlers::HandleGuildDisbanded(m_pGame, pData);
			return true;
		case DEF_NOTIFY_NEWGUILDSMAN:
			NetworkMessageHandlers::HandleNewGuildsMan(m_pGame, pData);
			return true;
		case DEF_NOTIFY_DISMISSGUILDSMAN:
			NetworkMessageHandlers::HandleDismissGuildsMan(m_pGame, pData);
			return true;
		case DEF_NOTIFY_CANNOTJOINMOREGUILDSMAN:
			NetworkMessageHandlers::HandleCannotJoinMoreGuildsMan(m_pGame, pData);
			return true;
		case DEF_NOTIFY_REQGUILDNAMEANSWER:
			NetworkMessageHandlers::HandleReqGuildNameAnswer(m_pGame, pData);
			return true;
		case DEF_NOTIFY_QUERY_JOINGUILDREQPERMISSION:
			NetworkMessageHandlers::HandleQueryJoinGuildPermission(m_pGame, pData);
			return true;
		case DEF_NOTIFY_QUERY_DISMISSGUILDREQPERMISSION:
			NetworkMessageHandlers::HandleQueryDismissGuildPermission(m_pGame, pData);
			return true;
		case DEF_NOTIFY_NOGUILDMASTERLEVEL:
			NetworkMessageHandlers::HandleNoGuildMasterLevel(m_pGame, pData);
			return true;
		case DEF_NOTIFY_SUCCESSBANGUILDMAN:
			NetworkMessageHandlers::HandleSuccessBanGuildMan(m_pGame, pData);
			return true;
		case DEF_NOTIFY_CANNOTBANGUILDMAN:
			NetworkMessageHandlers::HandleCannotBanGuildMan(m_pGame, pData);
			return true;

		// Guild - Common Type Messages
		case DEF_COMMONTYPE_JOINGUILDAPPROVE:
			NetworkMessageHandlers::HandleJoinGuildApprove(m_pGame, pData);
			return true;
		case DEF_COMMONTYPE_JOINGUILDREJECT:
			NetworkMessageHandlers::HandleJoinGuildReject(m_pGame, pData);
			return true;
		case DEF_COMMONTYPE_DISMISSGUILDAPPROVE:
			NetworkMessageHandlers::HandleDismissGuildApprove(m_pGame, pData);
			return true;
		case DEF_COMMONTYPE_DISMISSGUILDREJECT:
			NetworkMessageHandlers::HandleDismissGuildReject(m_pGame, pData);
			return true;

		// Combat
		case DEF_NOTIFY_KILLED:
			NetworkMessageHandlers::HandleKilled(m_pGame, pData);
			return true;
		case DEF_NOTIFY_PKCAPTURED:
			NetworkMessageHandlers::HandlePKcaptured(m_pGame, pData);
			return true;
		case DEF_NOTIFY_PKPENALTY:
			NetworkMessageHandlers::HandlePKpenalty(m_pGame, pData);
			return true;
		case DEF_NOTIFY_ENEMYKILLS:
			NetworkMessageHandlers::HandleEnemyKills(m_pGame, pData);
			return true;
		case DEF_NOTIFY_ENEMYKILLREWARD:
			NetworkMessageHandlers::HandleEnemyKillReward(m_pGame, pData);
			return true;
		case DEF_NOTIFY_GLOBALATTACKMODE:
			NetworkMessageHandlers::HandleGlobalAttackMode(m_pGame, pData);
			return true;
		case DEF_NOTIFY_DAMAGEMOVE:
			NetworkMessageHandlers::HandleDamageMove(m_pGame, pData);
			return true;
		case DEF_NOTIFY_OBSERVERMODE:
			NetworkMessageHandlers::HandleObserverMode(m_pGame, pData);
			return true;
		case DEF_NOTIFY_SUPERATTACKLEFT:
			NetworkMessageHandlers::HandleSuperAttackLeft(m_pGame, pData);
			return true;
		case DEF_NOTIFY_SAFEATTACKMODE:
			NetworkMessageHandlers::HandleSafeAttackMode(m_pGame, pData);
			return true;
		case DEF_SEND_NPCHP:
			NetworkMessageHandlers::HandleNpcHp(m_pGame, pData);
			return true;

		// Skills
		case DEF_NOTIFY_SKILLTRAINSUCCESS:
			NetworkMessageHandlers::HandleSkillTrainSuccess(m_pGame, pData);
			return true;
		case DEF_NOTIFY_MAGICSTUDYSUCCESS:
			NetworkMessageHandlers::HandleMagicStudySuccess(m_pGame, pData);
			return true;
		case DEF_NOTIFY_MAGICSTUDYFAIL:
			NetworkMessageHandlers::HandleMagicStudyFail(m_pGame, pData);
			return true;
		case DEF_NOTIFY_DOWNSKILLINDEXSET:
			NetworkMessageHandlers::HandleDownSkillIndexSet(m_pGame, pData);
			return true;
		case DEF_NOTIFY_SKILL:
			NetworkMessageHandlers::HandleSkill(m_pGame, pData);
			return true;
		case DEF_NOTIFY_SKILLUSINGEND:
			NetworkMessageHandlers::HandleSkillUsingEnd(m_pGame, pData);
			return true;
		case DEF_NOTIFY_MAGICEFFECTON:
			NetworkMessageHandlers::HandleMagicEffectOn(m_pGame, pData);
			return true;
		case DEF_NOTIFY_MAGICEFFECTOFF:
			NetworkMessageHandlers::HandleMagicEffectOff(m_pGame, pData);
			return true;
		case DEF_NOTIFY_SPELL_SKILL:
			NetworkMessageHandlers::HandleSpellSkill(m_pGame, pData);
			return true;
		case DEF_NOTIFY_STATECHANGE_SUCCESS:
			NetworkMessageHandlers::HandleStateChangeSuccess(m_pGame, pData);
			return true;
		case DEF_NOTIFY_STATECHANGE_FAILED:
			NetworkMessageHandlers::HandleStateChangeFailed(m_pGame, pData);
			return true;
		case DEF_NOTIFY_SETTING_FAILED:
			NetworkMessageHandlers::HandleSettingFailed(m_pGame, pData);
			return true;
		case DEF_NOTIFY_SPECIALABILITYSTATUS:
			NetworkMessageHandlers::HandleSpecialAbilityStatus(m_pGame, pData);
			return true;
		case DEF_NOTIFY_SPECIALABILITYENABLED:
			NetworkMessageHandlers::HandleSpecialAbilityEnabled(m_pGame, pData);
			return true;
		case DEF_NOTIFY_SKILLTRAINFAIL:
			NetworkMessageHandlers::HandleSkillTrainFail(m_pGame, pData);
			return true;

		// Player
		case DEF_NOTIFY_CHARISMA:
			NetworkMessageHandlers::HandleCharisma(m_pGame, pData);
			return true;
		case DEF_NOTIFY_HUNGER:
			NetworkMessageHandlers::HandleHunger(m_pGame, pData);
			return true;
		case DEF_NOTIFY_PLAYERPROFILE:
			NetworkMessageHandlers::HandlePlayerProfile(m_pGame, pData);
			return true;
		case DEF_NOTIFY_PLAYERONGAME:
			NetworkMessageHandlers::HandlePlayerStatus(m_pGame, true, pData);
			return true;
		case DEF_NOTIFY_PLAYERNOTONGAME:
			NetworkMessageHandlers::HandlePlayerStatus(m_pGame, false, pData);
			return true;
		case DEF_NOTIFY_WHISPERMODEON:
			NetworkMessageHandlers::HandleWhisperMode(m_pGame, true, pData);
			return true;
		case DEF_NOTIFY_WHISPERMODEOFF:
			NetworkMessageHandlers::HandleWhisperMode(m_pGame, false, pData);
			return true;
		case DEF_NOTIFY_PLAYERSHUTUP:
			NetworkMessageHandlers::HandlePlayerShutUp(m_pGame, pData);
			return true;
		case DEF_NOTIFY_RATINGPLAYER:
			NetworkMessageHandlers::HandleRatingPlayer(m_pGame, pData);
			return true;
		case DEF_NOTIFY_CANNOTRATING:
			NetworkMessageHandlers::HandleCannotRating(m_pGame, pData);
			return true;
		case DEF_NOTIFY_CHANGEPLAYMODE:
			NetworkMessageHandlers::HandleChangePlayMode(m_pGame, pData);
			return true;

		// Quest
		case DEF_NOTIFY_QUESTCONTENTS:
			NetworkMessageHandlers::HandleQuestContents(m_pGame, pData);
			return true;
		case DEF_NOTIFY_QUESTREWARD:
			NetworkMessageHandlers::HandleQuestReward(m_pGame, pData);
			return true;
		case DEF_NOTIFY_QUESTCOUNTER:
			NetworkMessageHandlers::HandleQuestCounter(m_pGame, pData);
			return true;
		case DEF_NOTIFY_QUESTCOMPLETED:
			NetworkMessageHandlers::HandleQuestCompleted(m_pGame, pData);
			return true;
		case DEF_NOTIFY_QUESTABORTED:
			NetworkMessageHandlers::HandleQuestAborted(m_pGame, pData);
			return true;

		// Party
		case DEF_NOTIFY_PARTY:
			NetworkMessageHandlers::HandleParty(m_pGame, pData);
			return true;
		case DEF_NOTIFY_QUERY_JOINPARTY:
			NetworkMessageHandlers::HandleQueryJoinParty(m_pGame, pData);
			return true;
		case DEF_NOTIFY_RESPONSE_CREATENEWPARTY:
			NetworkMessageHandlers::HandleResponseCreateNewParty(m_pGame, pData);
			return true;

		// Apocalypse
		case DEF_NOTIFY_APOCGATESTARTMSG:
			NetworkMessageHandlers::HandleApocGateStart(m_pGame, pData);
			return true;
		case DEF_NOTIFY_APOCGATEENDMSG:
			NetworkMessageHandlers::HandleApocGateEnd(m_pGame, pData);
			return true;
		case DEF_NOTIFY_APOCGATEOPEN:
			NetworkMessageHandlers::HandleApocGateOpen(m_pGame, pData);
			return true;
		case DEF_NOTIFY_APOCGATECLOSE:
			NetworkMessageHandlers::HandleApocGateClose(m_pGame, pData);
			return true;
		case DEF_NOTIFY_APOCFORCERECALLPLAYERS:
			NetworkMessageHandlers::HandleApocForceRecall(m_pGame, pData);
			return true;
		case DEF_NOTIFY_ABADDONKILLED:
			NetworkMessageHandlers::HandleAbaddonKilled(m_pGame, pData);
			return true;

		// Heldenian
		case DEF_NOTIFY_HELDENIANTELEPORT:
			NetworkMessageHandlers::HandleHeldenianTeleport(m_pGame, pData);
			return true;
		case DEF_NOTIFY_HELDENIANEND:
			NetworkMessageHandlers::HandleHeldenianEnd(m_pGame, pData);
			return true;
		case DEF_NOTIFY_HELDENIANSTART:
			NetworkMessageHandlers::HandleHeldenianStart(m_pGame, pData);
			return true;
		case DEF_NOTIFY_HELDENIANVICTORY:
			NetworkMessageHandlers::HandleHeldenianVictory(m_pGame, pData);
			return true;
		case DEF_NOTIFY_HELDENIANCOUNT:
			NetworkMessageHandlers::HandleHeldenianCount(m_pGame, pData);
			return true;
		case DEF_NOTIFY_0BE8:
			NetworkMessageHandlers::HandleHeldenianRecall(m_pGame, pData);
			return true;

		// Slates
		case DEF_NOTIFY_SLATE_CREATESUCCESS:
			NetworkMessageHandlers::HandleSlateCreateSuccess(m_pGame, pData);
			return true;
		case DEF_NOTIFY_SLATE_CREATEFAIL:
			NetworkMessageHandlers::HandleSlateCreateFail(m_pGame, pData);
			return true;
		case DEF_NOTIFY_SLATE_INVINCIBLE:
			NetworkMessageHandlers::HandleSlateInvincible(m_pGame, pData);
			return true;
		case DEF_NOTIFY_SLATE_MANA:
			NetworkMessageHandlers::HandleSlateMana(m_pGame, pData);
			return true;
		case DEF_NOTIFY_SLATE_EXP:
			NetworkMessageHandlers::HandleSlateExp(m_pGame, pData);
			return true;
		case DEF_NOTIFY_SLATE_STATUS:
			NetworkMessageHandlers::HandleSlateStatus(m_pGame, pData);
			return true;
		case DEF_NOTIFY_SLATE_BERSERK:
			NetworkMessageHandlers::HandleSlateBerserk(m_pGame, pData);
			return true;

		// Events (Generic)
		case DEF_NOTIFY_SPAWNEVENT:
			NetworkMessageHandlers::HandleSpawnEvent(m_pGame, pData);
			return true;
		case DEF_NOTIFY_MONSTERCOUNT:
			NetworkMessageHandlers::HandleMonsterCount(m_pGame, pData);
			return true;
		case DEF_NOTIFY_RESURRECTPLAYER:
			NetworkMessageHandlers::HandleResurrectPlayer(m_pGame, pData);
			return true;

		// Crusade
		case DEF_NOTIFY_CRUSADE:
			NetworkMessageHandlers::HandleCrusade(m_pGame, pData);
			return true;
		case DEF_NOTIFY_GRANDMAGICRESULT:
			NetworkMessageHandlers::HandleGrandMagicResult(m_pGame, pData);
			return true;
		case DEF_NOTIFY_NOMORECRUSADESTRUCTURE:
			NetworkMessageHandlers::HandleNoMoreCrusadeStructure(m_pGame, pData);
			return true;
		case DEF_NOTIFY_ENERGYSPHEREGOALIN:
			NetworkMessageHandlers::HandleEnergySphereGoalIn(m_pGame, pData);
			return true;
		case DEF_NOTIFY_ENERGYSPHERECREATED:
			NetworkMessageHandlers::HandleEnergySphereCreated(m_pGame, pData);
			return true;
		case DEF_NOTIFY_METEORSTRIKECOMING:
			NetworkMessageHandlers::HandleMeteorStrikeComing(m_pGame, pData);
			return true;
		case DEF_NOTIFY_METEORSTRIKEHIT:
			NetworkMessageHandlers::HandleMeteorStrikeHit(m_pGame, pData);
			return true;
		case DEF_NOTIFY_CANNOTCONSTRUCT:
			NetworkMessageHandlers::HandleCannotConstruct(m_pGame, pData);
			return true;
		case DEF_NOTIFY_TCLOC:
			NetworkMessageHandlers::HandleTCLoc(m_pGame, pData);
			return true;
		case DEF_NOTIFY_CONSTRUCTIONPOINT:
			NetworkMessageHandlers::HandleConstructionPoint(m_pGame, pData);
			return true;

		// Map
		case DEF_NOTIFY_MAPSTATUSNEXT:
			NetworkMessageHandlers::HandleMapStatusNext(m_pGame, pData);
			return true;
		case DEF_NOTIFY_MAPSTATUSLAST:
			NetworkMessageHandlers::HandleMapStatusLast(m_pGame, pData);
			return true;
		case DEF_NOTIFY_LOCKEDMAP:
			NetworkMessageHandlers::HandleLockedMap(m_pGame, pData);
			return true;
		case DEF_NOTIFY_SHOWMAP:
			NetworkMessageHandlers::HandleShowMap(m_pGame, pData);
			return true;

		// Crafting
		case DEF_NOTIFY_CRAFTING_SUCCESS:
			NetworkMessageHandlers::HandleCraftingSuccess(m_pGame, pData);
			return true;
		case DEF_NOTIFY_CRAFTING_FAIL:
			NetworkMessageHandlers::HandleCraftingFail(m_pGame, pData);
			return true;
		case DEF_NOTIFY_BUILDITEMSUCCESS:
			NetworkMessageHandlers::HandleBuildItemSuccess(m_pGame, pData);
			return true;
		case DEF_NOTIFY_BUILDITEMFAIL:
			NetworkMessageHandlers::HandleBuildItemFail(m_pGame, pData);
			return true;
		case DEF_NOTIFY_PORTIONSUCCESS:
			NetworkMessageHandlers::HandlePortionSuccess(m_pGame, pData);
			return true;
		case DEF_NOTIFY_PORTIONFAIL:
			NetworkMessageHandlers::HandlePortionFail(m_pGame, pData);
			return true;
		case DEF_NOTIFY_LOWPORTIONSKILL:
			NetworkMessageHandlers::HandleLowPortionSkill(m_pGame, pData);
			return true;
		case DEF_NOTIFY_NOMATCHINGPORTION:
			NetworkMessageHandlers::HandleNoMatchingPortion(m_pGame, pData);
			return true;

		// Fish
		case DEF_NOTIFY_FISHCHANCE:
			NetworkMessageHandlers::HandleFishChance(m_pGame, pData);
			return true;
		case DEF_NOTIFY_EVENTFISHMODE:
			NetworkMessageHandlers::HandleEventFishMode(m_pGame, pData);
			return true;
		case DEF_NOTIFY_FISHCANCELED:
			NetworkMessageHandlers::HandleFishCanceled(m_pGame, pData);
			return true;
		case DEF_NOTIFY_FISHSUCCESS:
			NetworkMessageHandlers::HandleFishSuccess(m_pGame, pData);
			return true;
		case DEF_NOTIFY_FISHFAIL:
			NetworkMessageHandlers::HandleFishFail(m_pGame, pData);
			return true;

		// Agriculture
		case DEF_NOTIFY_AGRICULTURENOAREA:
			NetworkMessageHandlers::HandleAgricultureNoArea(m_pGame, pData);
			return true;
		case DEF_NOTIFY_AGRICULTURESKILLLIMIT:
			NetworkMessageHandlers::HandleAgricultureSkillLimit(m_pGame, pData);
			return true;
		case DEF_NOTIFY_NOMOREAGRICULTURE:
			NetworkMessageHandlers::HandleNoMoreAgriculture(m_pGame, pData);
			return true;

		// Angels
		case DEF_NOTIFY_ANGEL_FAILED:
			NetworkMessageHandlers::HandleAngelFailed(m_pGame, pData);
			return true;
		case DEF_NOTIFY_ANGEL_RECEIVED:
			NetworkMessageHandlers::HandleAngelReceived(m_pGame, pData);
			return true;
		case DEF_NOTIFY_ANGELIC_STATS:
			NetworkMessageHandlers::HandleAngelicStats(m_pGame, pData);
			return true;

		// Admin
		case DEF_NOTIFY_ADMINIFO:
			NetworkMessageHandlers::HandleAdminInfo(m_pGame, pData);
			return true;
		case DEF_NOTIFY_0BEF:
			NetworkMessageHandlers::HandleCrashHandler(m_pGame, pData);
			return true;
		case DEF_NOTIFY_IPACCOUNTINFO:
			NetworkMessageHandlers::HandleIpAccountInfo(m_pGame, pData);
			return true;
		case DEF_NOTIFY_REWARDGOLD:
			NetworkMessageHandlers::HandleRewardGold(m_pGame, pData);
			return true;
		case DEF_NOTIFY_SERVERSHUTDOWN:
			NetworkMessageHandlers::HandleServerShutdown(m_pGame, pData);
			return true;
		case DEF_NOTIFY_ADMINUSERLEVELLOW:
			NetworkMessageHandlers::HandleAdminUserLevelLow(m_pGame, pData);
			return true;

		// System (Generic)
		case DEF_NOTIFY_WHETHERCHANGE:
			NetworkMessageHandlers::HandleWhetherChange(m_pGame, pData);
			return true;
		case DEF_NOTIFY_TIMECHANGE:
			NetworkMessageHandlers::HandleTimeChange(m_pGame, pData);
			return true;
		case DEF_NOTIFY_NOTICEMSG:
			NetworkMessageHandlers::HandleNoticeMsg(m_pGame, pData);
			return true;
		case DEF_NOTIFY_FORCEDISCONN:
			NetworkMessageHandlers::HandleForceDisconn(m_pGame, pData);
			return true;
		case DEF_NOTIFY_SETTING_SUCCESS:
			NetworkMessageHandlers::HandleSettingSuccess(m_pGame, pData);
			return true;
		case DEF_NOTIFY_SERVERCHANGE:
			NetworkMessageHandlers::HandleServerChange(m_pGame, pData);
			return true;
		case DEF_NOTIFY_TOTALUSERS:
			NetworkMessageHandlers::HandleTotalUsers(m_pGame, pData);
			return true;
		case DEF_NOTIFY_FORCERECALLTIME:
			NetworkMessageHandlers::HandleForceRecallTime(m_pGame, pData);
			return true;
		case DEF_NOTIFY_NORECALL:
			NetworkMessageHandlers::HandleNoRecall(m_pGame, pData);
			return true;
		case DEF_NOTIFY_FIGHTZONERESERVE:
			NetworkMessageHandlers::HandleFightZoneReserve(m_pGame, pData);
			return true;
		case DEF_NOTIFY_LOTERY_LOST:
			NetworkMessageHandlers::HandleLoteryLost(m_pGame, pData);
			return true;
		case DEF_NOTIFY_NOTFLAGSPOT:
			NetworkMessageHandlers::HandleNotFlagSpot(m_pGame, pData);
			return true;
		case DEF_NOTIFY_NPCTALK:
			NetworkMessageHandlers::HandleNpcTalk(m_pGame, pData);
			return true;
		case DEF_NOTIFY_TRAVELERLIMITEDLEVEL:
			NetworkMessageHandlers::HandleTravelerLimitedLevel(m_pGame, pData);
			return true;
		case DEF_NOTIFY_LIMITEDLEVEL:
			NetworkMessageHandlers::HandleLimitedLevel(m_pGame, pData);
			return true;
		case DEF_NOTIFY_TOBERECALLED:
			NetworkMessageHandlers::HandleToBeRecalled(m_pGame, pData);
			return true;
		}
		return false;
	}

	switch (dwMsgID)
	{
	case MSGID_RESPONSE_CREATENEWGUILD:
		NetworkMessageHandlers::HandleCreateNewGuildResponse(m_pGame, pData);
		return true;

	case MSGID_RESPONSE_DISBANDGUILD:
		NetworkMessageHandlers::HandleDisbandGuildResponse(m_pGame, pData);
		return true;

	default:
		break;
	}

	return false;
}
