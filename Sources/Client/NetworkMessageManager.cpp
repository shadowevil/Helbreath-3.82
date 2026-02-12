#include "Game.h"
#include "NetworkMessageManager.h"
#include "Packet/SharedPackets.h"
#include "lan_eng.h"
#include "PlatformCompat.h"
#include <cstdio>
#include <cstring>
#include <cmath>

using namespace hb::shared::net;
namespace NetworkMessageHandlers {
	// Stats
	void HandleHP(CGame* pGame, char* pData);
	void HandleMP(CGame* pGame, char* pData);
	void HandleSP(CGame* pGame, char* pData);
	void HandleExp(CGame* pGame, char* pData);
	void HandleLevelUp(CGame* pGame, char* pData);

	// Exchange
	void HandleExchangeItemComplete(CGame* pGame, char* pData);
	void HandleCancelExchangeItem(CGame* pGame, char* pData);

	// Bank
	void HandleItemToBank(CGame* pGame, char* pData);
	void HandleCannotItemToBank(CGame* pGame, char* pData);

	// Slates
	void HandleSlateCreateSuccess(CGame* pGame, char* pData);
	void HandleSlateCreateFail(CGame* pGame, char* pData);
	void HandleSlateInvincible(CGame* pGame, char* pData);
	void HandleSlateMana(CGame* pGame, char* pData);
	void HandleSlateExp(CGame* pGame, char* pData);
	void HandleSlateStatus(CGame* pGame, char* pData);
	void HandleSlateBerserk(CGame* pGame, char* pData);

	// Map
	void HandleMapStatusNext(CGame* pGame, char* pData);
	void HandleMapStatusLast(CGame* pGame, char* pData);
	void HandleLockedMap(CGame* pGame, char* pData);
	void HandleShowMap(CGame* pGame, char* pData);

	// Events
	void HandleSpawnEvent(CGame* pGame, char* pData);
	void HandleMonsterCount(CGame* pGame, char* pData);
	void HandleResurrectPlayer(CGame* pGame, char* pData);

	// Agriculture
	void HandleAgricultureNoArea(CGame* pGame, char* pData);
	void HandleAgricultureSkillLimit(CGame* pGame, char* pData);
	void HandleNoMoreAgriculture(CGame* pGame, char* pData);

	// Angels
	void HandleAngelFailed(CGame* pGame, char* pData);
	void HandleAngelReceived(CGame* pGame, char* pData);
	void HandleAngelicStats(CGame* pGame, char* pData);

	// Party
	void HandleParty(CGame* pGame, char* pData);
	void HandleQueryJoinParty(CGame* pGame, char* pData);
	void HandleResponseCreateNewParty(CGame* pGame, char* pData);

	// Quest
	void HandleQuestContents(CGame* pGame, char* pData);
	void HandleQuestReward(CGame* pGame, char* pData);
	void HandleQuestCompleted(CGame* pGame, char* pData);
	void HandleQuestAborted(CGame* pGame, char* pData);
	void HandleQuestCounter(CGame* pGame, char* pData);

	// Fish
	void HandleFishChance(CGame* pGame, char* pData);
	void HandleEventFishMode(CGame* pGame, char* pData);
	void HandleFishCanceled(CGame* pGame, char* pData);
	void HandleFishSuccess(CGame* pGame, char* pData);
	void HandleFishFail(CGame* pGame, char* pData);

	// Items
	void HandleItemPurchased(CGame* pGame, char* pData);
	void HandleItemObtained(CGame* pGame, char* pData);
	void HandleItemObtainedBulk(CGame* pGame, char* pData);
	void HandleItemLifeSpanEnd(CGame* pGame, char* pData);
	void HandleItemReleased(CGame* pGame, char* pData);
	void HandleSetItemCount(CGame* pGame, char* pData);
	void HandleItemDepleted_EraseItem(CGame* pGame, char* pData);
	void HandleDropItemFin_EraseItem(CGame* pGame, char* pData);
	void HandleGiveItemFin_EraseItem(CGame* pGame, char* pData);
	void HandleDropItemFin_CountChanged(CGame* pGame, char* pData);
	void HandleGiveItemFin_CountChanged(CGame* pGame, char* pData);
	void HandleItemRepaired(CGame* pGame, char* pData);
	void HandleRepairItemPrice(CGame* pGame, char* pData);
	void HandleRepairAllPrices(CGame* pGame, char* pData);
	void HandleSellItemPrice(CGame* pGame, char* pData);
	void HandleCannotRepairItem(CGame* pGame, char* pData);
	void HandleCannotSellItem(CGame* pGame, char* pData);
	void HandleCannotGiveItem(CGame* pGame, char* pData);
	void HandleItemColorChange(CGame* pGame, char* pData);
	void HandleSetExchangeItem(CGame* pGame, char* pData);
	void HandleOpenExchangeWindow(CGame* pGame, char* pData);
	void HandleCurLifeSpan(CGame* pGame, char* pData);
	void HandleNotEnoughGold(CGame* pGame, char* pData);
	void HandleCannotCarryMoreItem(CGame* pGame, char* pData);
	void HandleItemAttributeChange(CGame* pGame, char* pData);
	void HandleItemUpgradeFail(CGame* pGame, char* pData);
	void HandleGizonItemUpgradeLeft(CGame* pGame, char* pData);
	void HandleGizonItemChange(CGame* pGame, char* pData);
	void HandleItemPosList(CGame* pGame, char* pData);
	void HandleItemSold(CGame* pGame, char* pData);

	// Apocalypse
	void HandleApocGateStart(CGame* pGame, char* pData);
	void HandleApocGateEnd(CGame* pGame, char* pData);
	void HandleApocGateOpen(CGame* pGame, char* pData);
	void HandleApocGateClose(CGame* pGame, char* pData);
	void HandleApocForceRecall(CGame* pGame, char* pData);
	void HandleAbaddonKilled(CGame* pGame, char* pData);

	// Heldenian
	void HandleHeldenianTeleport(CGame* pGame, char* pData);
	void HandleHeldenianEnd(CGame* pGame, char* pData);
	void HandleHeldenianStart(CGame* pGame, char* pData);
	void HandleHeldenianVictory(CGame* pGame, char* pData);
	void HandleHeldenianCount(CGame* pGame, char* pData);
	void HandleHeldenianRecall(CGame* pGame, char* pData);


	void HandleCrashHandler(CGame* pGame, char* pData);
	void HandleIpAccountInfo(CGame* pGame, char* pData);
	void HandleRewardGold(CGame* pGame, char* pData);
	void HandleServerShutdown(CGame* pGame, char* pData);
	// Crafting
	void HandleCraftingSuccess(CGame* pGame, char* pData);
	void HandleCraftingFail(CGame* pGame, char* pData);
	void HandleBuildItemSuccess(CGame* pGame, char* pData);
	void HandleBuildItemFail(CGame* pGame, char* pData);
	void HandlePortionSuccess(CGame* pGame, char* pData);
	void HandlePortionFail(CGame* pGame, char* pData);
	void HandleLowPortionSkill(CGame* pGame, char* pData);
	void HandleNoMatchingPortion(CGame* pGame, char* pData);

	// Guild
	void HandleCreateNewGuildResponse(CGame* pGame, char* pData);
	void HandleDisbandGuildResponse(CGame* pGame, char* pData);
	void HandleGuildDisbanded(CGame* pGame, char* pData);
	void HandleNewGuildsMan(CGame* pGame, char* pData);
	void HandleDismissGuildsMan(CGame* pGame, char* pData);
	void HandleCannotJoinMoreGuildsMan(CGame* pGame, char* pData);
	void HandleJoinGuildApprove(CGame* pGame, char* pData);
	void HandleJoinGuildReject(CGame* pGame, char* pData);
	void HandleDismissGuildApprove(CGame* pGame, char* pData);
	void HandleDismissGuildReject(CGame* pGame, char* pData);
	void HandleQueryJoinGuildPermission(CGame* pGame, char* pData);
	void HandleQueryDismissGuildPermission(CGame* pGame, char* pData);
	void HandleReqGuildNameAnswer(CGame* pGame, char* pData);
	void HandleNoGuildMasterLevel(CGame* pGame, char* pData);
	void HandleSuccessBanGuildMan(CGame* pGame, char* pData);
	void HandleCannotBanGuildMan(CGame* pGame, char* pData);

	// Combat
	void HandleSpellInterrupted(CGame* pGame, char* pData);
	void HandleKilled(CGame* pGame, char* pData);
	void HandlePKcaptured(CGame* pGame, char* pData);
	void HandlePKpenalty(CGame* pGame, char* pData);
	void HandleEnemyKills(CGame* pGame, char* pData);
	void HandleEnemyKillReward(CGame* pGame, char* pData);
	void HandleGlobalAttackMode(CGame* pGame, char* pData);
	void HandleDamageMove(CGame* pGame, char* pData);
	void HandleObserverMode(CGame* pGame, char* pData);
	void HandleSuperAttackLeft(CGame* pGame, char* pData);
	void HandleSafeAttackMode(CGame* pGame, char* pData);
	// Skills
	void HandleSkillTrainSuccess(CGame* pGame, char* pData);
	void HandleMagicStudySuccess(CGame* pGame, char* pData);
	void HandleMagicStudyFail(CGame* pGame, char* pData);
	void HandleDownSkillIndexSet(CGame* pGame, char* pData);
	void HandleSkill(CGame* pGame, char* pData);
	void HandleSkillUsingEnd(CGame* pGame, char* pData);
	void HandleMagicEffectOn(CGame* pGame, char* pData);
	void HandleMagicEffectOff(CGame* pGame, char* pData);
	void HandleSpellSkill(CGame* pGame, char* pData);
	void HandleStateChangeSuccess(CGame* pGame, char* pData);
	void HandleStateChangeFailed(CGame* pGame, char* pData);
	void HandleSettingFailed(CGame* pGame, char* pData);
	void HandleSpecialAbilityStatus(CGame* pGame, char* pData);
	void HandleSpecialAbilityEnabled(CGame* pGame, char* pData);
	void HandleSkillTrainFail(CGame* pGame, char* pData);

	// Player
	void HandleCharisma(CGame* pGame, char* pData);
	void HandleHunger(CGame* pGame, char* pData);
	void HandlePlayerProfile(CGame* pGame, char* pData);
	void HandlePlayerStatus(CGame* pGame, bool bOnGame, char* pData);
	void HandleWhisperMode(CGame* pGame, bool bActive, char* pData);
	void HandlePlayerShutUp(CGame* pGame, char* pData);
	void HandleRatingPlayer(CGame* pGame, char* pData);
	void HandleCannotRating(CGame* pGame, char* pData);

	// Crusade
	void HandleCrusade(CGame* pGame, char* pData);
	void HandleGrandMagicResult(CGame* pGame, char* pData);
	void HandleNoMoreCrusadeStructure(CGame* pGame, char* pData);
	void HandleEnergySphereGoalIn(CGame* pGame, char* pData);
	void HandleEnergySphereCreated(CGame* pGame, char* pData);
	void HandleMeteorStrikeComing(CGame* pGame, char* pData);
	void HandleMeteorStrikeHit(CGame* pGame, char* pData);
	void HandleCannotConstruct(CGame* pGame, char* pData);
	void HandleTCLoc(CGame* pGame, char* pData);
	void HandleConstructionPoint(CGame* pGame, char* pData);

	// System
	void HandleWeatherChange(CGame* pGame, char* pData);
	void HandleTimeChange(CGame* pGame, char* pData);
	void HandleNoticeMsg(CGame* pGame, char* pData);
	void HandleStatusText(CGame* pGame, char* pData);
	void HandleForceDisconn(CGame* pGame, char* pData);
	void HandleSettingSuccess(CGame* pGame, char* pData);
	void HandleServerChange(CGame* pGame, char* pData);
	void HandleTotalUsers(CGame* pGame, char* pData);
	void HandleChangePlayMode(CGame* pGame, char* pData);
	void HandleForceRecallTime(CGame* pGame, char* pData);
	void HandleNoRecall(CGame* pGame, char* pData);
	void HandleFightZoneReserve(CGame* pGame, char* pData);
	void HandleLoteryLost(CGame* pGame, char* pData);
	void HandleNotFlagSpot(CGame* pGame, char* pData);
	void HandleNpcTalk(CGame* pGame, char* pData);
	void HandleTravelerLimitedLevel(CGame* pGame, char* pData);
	void HandleLimitedLevel(CGame* pGame, char* pData);
	void HandleToBeRecalled(CGame* pGame, char* pData);
}

NetworkMessageManager::NetworkMessageManager(CGame* pGame)
	: m_pGame(pGame)
{
}

bool NetworkMessageManager::ProcessMessage(uint32_t dwMsgID, char* pData, uint32_t dwMsgSize)
{
	const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(pData, sizeof(hb::net::PacketHeader));
	if (!header) return false;

	if (dwMsgID == MsgId::Notify)
	{
		switch (header->msg_type)
		{
		// Stats
		case Notify::Hp: NetworkMessageHandlers::HandleHP(m_pGame, pData); return true;
		case Notify::Mp: NetworkMessageHandlers::HandleMP(m_pGame, pData); return true;
		case Notify::Sp: NetworkMessageHandlers::HandleSP(m_pGame, pData); return true;
		case Notify::Exp: NetworkMessageHandlers::HandleExp(m_pGame, pData); return true;
		case Notify::LevelUp: NetworkMessageHandlers::HandleLevelUp(m_pGame, pData); return true;

		// Items - Purchased/Obtained
		case Notify::ItemPurchased: NetworkMessageHandlers::HandleItemPurchased(m_pGame, pData); return true;
		case Notify::ItemObtained: NetworkMessageHandlers::HandleItemObtained(m_pGame, pData); return true;
		case Notify::ItemObtainedBulk: NetworkMessageHandlers::HandleItemObtainedBulk(m_pGame, pData); return true;

		// Items - LifeSpan/Released
		case Notify::ItemLifeSpanEnd: NetworkMessageHandlers::HandleItemLifeSpanEnd(m_pGame, pData); return true;
		case Notify::ItemReleased: NetworkMessageHandlers::HandleItemReleased(m_pGame, pData); return true;

		// Items - Count/Depleted
		case Notify::SetItemCount: NetworkMessageHandlers::HandleSetItemCount(m_pGame, pData); return true;
		case Notify::ItemDepletedEraseItem: NetworkMessageHandlers::HandleItemDepleted_EraseItem(m_pGame, pData); return true;

		// Items - Drop/Give
		case Notify::DropItemFinEraseItem: NetworkMessageHandlers::HandleDropItemFin_EraseItem(m_pGame, pData); return true;
		case Notify::GiveItemFinEraseItem: NetworkMessageHandlers::HandleGiveItemFin_EraseItem(m_pGame, pData); return true;
		case Notify::DropItemFinCountChanged: NetworkMessageHandlers::HandleDropItemFin_CountChanged(m_pGame, pData); return true;
		case Notify::GiveItemFinCountChanged: NetworkMessageHandlers::HandleGiveItemFin_CountChanged(m_pGame, pData); return true;

		// Items - Repair/Sell
		case Notify::ItemRepaired: NetworkMessageHandlers::HandleItemRepaired(m_pGame, pData); return true;
		case Notify::RepairItemPrice: NetworkMessageHandlers::HandleRepairItemPrice(m_pGame, pData); return true;
		case Notify::RepairAllPrices: NetworkMessageHandlers::HandleRepairAllPrices(m_pGame, pData); return true;
		case Notify::SellItemPrice: NetworkMessageHandlers::HandleSellItemPrice(m_pGame, pData); return true;
		case Notify::CannotRepairItem: NetworkMessageHandlers::HandleCannotRepairItem(m_pGame, pData); return true;
		case Notify::CannotSellItem: NetworkMessageHandlers::HandleCannotSellItem(m_pGame, pData); return true;

		// Items - Give/Cannot
		case Notify::CannotGiveItem: NetworkMessageHandlers::HandleCannotGiveItem(m_pGame, pData); return true;

		// Items - hb::shared::render::Color/Exchange
		case Notify::ItemColorChange: NetworkMessageHandlers::HandleItemColorChange(m_pGame, pData); return true;
		case Notify::SetExchangeItem: NetworkMessageHandlers::HandleSetExchangeItem(m_pGame, pData); return true;
		case Notify::OpenExchangeWindow: NetworkMessageHandlers::HandleOpenExchangeWindow(m_pGame, pData); return true;
		case Notify::CurLifeSpan: NetworkMessageHandlers::HandleCurLifeSpan(m_pGame, pData); return true;

		// Items - Upgrade/Attribute/Errors
		case Notify::NotEnoughGold: NetworkMessageHandlers::HandleNotEnoughGold(m_pGame, pData); return true;
		case Notify::CannotCarryMoreItem: NetworkMessageHandlers::HandleCannotCarryMoreItem(m_pGame, pData); return true;
		case Notify::ItemAttributeChange: NetworkMessageHandlers::HandleItemAttributeChange(m_pGame, pData); return true;
		case 0x0BC0: NetworkMessageHandlers::HandleItemAttributeChange(m_pGame, pData); return true; // Same handler as ITEMATTRIBUTECHANGE
		case Notify::ItemUpgradeFail: NetworkMessageHandlers::HandleItemUpgradeFail(m_pGame, pData); return true;
		case Notify::GizonItemUpgradeLeft: NetworkMessageHandlers::HandleGizonItemUpgradeLeft(m_pGame, pData); return true;
		case Notify::GizoneItemChange: NetworkMessageHandlers::HandleGizonItemChange(m_pGame, pData); return true;
		case Notify::ItemPosList: NetworkMessageHandlers::HandleItemPosList(m_pGame, pData); return true;
		case Notify::ItemSold: NetworkMessageHandlers::HandleItemSold(m_pGame, pData); return true;

		// Bank
		case Notify::ItemToBank: NetworkMessageHandlers::HandleItemToBank(m_pGame, pData); return true;
		case Notify::CannotItemToBank: NetworkMessageHandlers::HandleCannotItemToBank(m_pGame, pData); return true;

		// Exchange
		case Notify::ExchangeItemComplete: NetworkMessageHandlers::HandleExchangeItemComplete(m_pGame, pData); return true;
		case Notify::CancelExchangeItem: NetworkMessageHandlers::HandleCancelExchangeItem(m_pGame, pData); return true;

		// Guild - Notify Messages
		case Notify::GuildDisbanded: NetworkMessageHandlers::HandleGuildDisbanded(m_pGame, pData); return true;
		case Notify::NewGuildsman: NetworkMessageHandlers::HandleNewGuildsMan(m_pGame, pData); return true;
		case Notify::DismissGuildsman: NetworkMessageHandlers::HandleDismissGuildsMan(m_pGame, pData); return true;
		case Notify::CannotJoinMoreGuildsman: NetworkMessageHandlers::HandleCannotJoinMoreGuildsMan(m_pGame, pData); return true;
		case Notify::ReqGuildNameAnswer: NetworkMessageHandlers::HandleReqGuildNameAnswer(m_pGame, pData); return true;
		case Notify::QueryJoinGuildReqPermission: NetworkMessageHandlers::HandleQueryJoinGuildPermission(m_pGame, pData); return true;
		case Notify::QueryDismissGuildReqPermission: NetworkMessageHandlers::HandleQueryDismissGuildPermission(m_pGame, pData); return true;
		case Notify::NoGuildMasterLevel: NetworkMessageHandlers::HandleNoGuildMasterLevel(m_pGame, pData); return true;
		case Notify::SuccessBanGuildman: NetworkMessageHandlers::HandleSuccessBanGuildMan(m_pGame, pData); return true;
		case Notify::CannotBanGuildman: NetworkMessageHandlers::HandleCannotBanGuildMan(m_pGame, pData); return true;

		// Guild - Common Type Messages
		case CommonType::JoinGuildApprove: NetworkMessageHandlers::HandleJoinGuildApprove(m_pGame, pData); return true;
		case CommonType::JoinGuildReject: NetworkMessageHandlers::HandleJoinGuildReject(m_pGame, pData); return true;
		case CommonType::DismissGuildApprove: NetworkMessageHandlers::HandleDismissGuildApprove(m_pGame, pData); return true;
		case CommonType::DismissGuildReject: NetworkMessageHandlers::HandleDismissGuildReject(m_pGame, pData); return true;

		// Combat
		case Notify::Killed: NetworkMessageHandlers::HandleKilled(m_pGame, pData); return true;
		case Notify::PkCaptured: NetworkMessageHandlers::HandlePKcaptured(m_pGame, pData); return true;
		case Notify::PkPenalty: NetworkMessageHandlers::HandlePKpenalty(m_pGame, pData); return true;
		case Notify::EnemyKills: NetworkMessageHandlers::HandleEnemyKills(m_pGame, pData); return true;
		case Notify::EnemyKillReward: NetworkMessageHandlers::HandleEnemyKillReward(m_pGame, pData); return true;
		case Notify::GlobalAttackMode: NetworkMessageHandlers::HandleGlobalAttackMode(m_pGame, pData); return true;
		case Notify::DamageMove: NetworkMessageHandlers::HandleDamageMove(m_pGame, pData); return true;
		case Notify::ObserverMode: NetworkMessageHandlers::HandleObserverMode(m_pGame, pData); return true;
		case Notify::SuperAttackLeft: NetworkMessageHandlers::HandleSuperAttackLeft(m_pGame, pData); return true;
		case Notify::SafeAttackMode: NetworkMessageHandlers::HandleSafeAttackMode(m_pGame, pData); return true;
		// Skills
		case Notify::SkillTrainSuccess: NetworkMessageHandlers::HandleSkillTrainSuccess(m_pGame, pData); return true;
		case Notify::MagicStudySuccess: NetworkMessageHandlers::HandleMagicStudySuccess(m_pGame, pData); return true;
		case Notify::MagicStudyFail: NetworkMessageHandlers::HandleMagicStudyFail(m_pGame, pData); return true;
		case Notify::DownSkillIndexSet: NetworkMessageHandlers::HandleDownSkillIndexSet(m_pGame, pData); return true;
		case Notify::Skill: NetworkMessageHandlers::HandleSkill(m_pGame, pData); return true;
		case Notify::SkillUsingEnd: NetworkMessageHandlers::HandleSkillUsingEnd(m_pGame, pData); return true;
		case Notify::MagicEffectOn: NetworkMessageHandlers::HandleMagicEffectOn(m_pGame, pData); return true;
		case Notify::MagicEffectOff: NetworkMessageHandlers::HandleMagicEffectOff(m_pGame, pData); return true;
		case Notify::SpellSkill: NetworkMessageHandlers::HandleSpellSkill(m_pGame, pData); return true;
		case Notify::SpellInterrupted: NetworkMessageHandlers::HandleSpellInterrupted(m_pGame, pData); return true;
		case Notify::StateChangeSuccess: NetworkMessageHandlers::HandleStateChangeSuccess(m_pGame, pData); return true;
		case Notify::StateChangeFailed: NetworkMessageHandlers::HandleStateChangeFailed(m_pGame, pData); return true;
		case Notify::SettingFailed: NetworkMessageHandlers::HandleSettingFailed(m_pGame, pData); return true;
		case Notify::SpecialAbilityStatus: NetworkMessageHandlers::HandleSpecialAbilityStatus(m_pGame, pData); return true;
		case Notify::SpecialAbilityEnabled: NetworkMessageHandlers::HandleSpecialAbilityEnabled(m_pGame, pData); return true;
		case Notify::SkillTrainFail: NetworkMessageHandlers::HandleSkillTrainFail(m_pGame, pData); return true;

		// Player
		case Notify::Charisma: NetworkMessageHandlers::HandleCharisma(m_pGame, pData); return true;
		case Notify::Hunger: NetworkMessageHandlers::HandleHunger(m_pGame, pData); return true;
		case Notify::PlayerProfile: NetworkMessageHandlers::HandlePlayerProfile(m_pGame, pData); return true;
		case Notify::PlayerOnGame: NetworkMessageHandlers::HandlePlayerStatus(m_pGame, true, pData); return true;
		case Notify::PlayerNotOnGame: NetworkMessageHandlers::HandlePlayerStatus(m_pGame, false, pData); return true;
		case Notify::WhisperModeOn: NetworkMessageHandlers::HandleWhisperMode(m_pGame, true, pData); return true;
		case Notify::WhisperModeOff: NetworkMessageHandlers::HandleWhisperMode(m_pGame, false, pData); return true;
		case Notify::PlayerShutUp: NetworkMessageHandlers::HandlePlayerShutUp(m_pGame, pData); return true;
		case Notify::RatingPlayer: NetworkMessageHandlers::HandleRatingPlayer(m_pGame, pData); return true;
		case Notify::CannotRating: NetworkMessageHandlers::HandleCannotRating(m_pGame, pData); return true;
		case Notify::ChangePlayMode: NetworkMessageHandlers::HandleChangePlayMode(m_pGame, pData); return true;

		// Quest
		case Notify::QuestContents: NetworkMessageHandlers::HandleQuestContents(m_pGame, pData); return true;
		case Notify::QuestReward: NetworkMessageHandlers::HandleQuestReward(m_pGame, pData); return true;
		case Notify::QuestCounter: NetworkMessageHandlers::HandleQuestCounter(m_pGame, pData); return true;
		case Notify::QuestCompleted: NetworkMessageHandlers::HandleQuestCompleted(m_pGame, pData); return true;
		case Notify::QuestAborted: NetworkMessageHandlers::HandleQuestAborted(m_pGame, pData); return true;

		// Party
		case Notify::Party: NetworkMessageHandlers::HandleParty(m_pGame, pData); return true;
		case Notify::QueryJoinParty: NetworkMessageHandlers::HandleQueryJoinParty(m_pGame, pData); return true;
		case Notify::ResponseCreateNewParty: NetworkMessageHandlers::HandleResponseCreateNewParty(m_pGame, pData); return true;

		// Apocalypse
		case Notify::ApocGateStartMsg: NetworkMessageHandlers::HandleApocGateStart(m_pGame, pData); return true;
		case Notify::ApocGateEndMsg: NetworkMessageHandlers::HandleApocGateEnd(m_pGame, pData); return true;
		case Notify::ApocGateOpen: NetworkMessageHandlers::HandleApocGateOpen(m_pGame, pData); return true;
		case Notify::ApocGateClose: NetworkMessageHandlers::HandleApocGateClose(m_pGame, pData); return true;
		case Notify::ApocForceRecallPlayers: NetworkMessageHandlers::HandleApocForceRecall(m_pGame, pData); return true;
		case Notify::AbaddonKilled: NetworkMessageHandlers::HandleAbaddonKilled(m_pGame, pData); return true;

		// Heldenian
		case Notify::HeldenianTeleport: NetworkMessageHandlers::HandleHeldenianTeleport(m_pGame, pData); return true;
		case Notify::HeldenianEnd: NetworkMessageHandlers::HandleHeldenianEnd(m_pGame, pData); return true;
		case Notify::HeldenianStart: NetworkMessageHandlers::HandleHeldenianStart(m_pGame, pData); return true;
		case Notify::HeldenianVictory: NetworkMessageHandlers::HandleHeldenianVictory(m_pGame, pData); return true;
		case Notify::HeldenianCount: NetworkMessageHandlers::HandleHeldenianCount(m_pGame, pData); return true;
		case Notify::Unknown0BE8: NetworkMessageHandlers::HandleHeldenianRecall(m_pGame, pData); return true;

		// Slates
		case Notify::SlateCreateSuccess: NetworkMessageHandlers::HandleSlateCreateSuccess(m_pGame, pData); return true;
		case Notify::SlateCreateFail: NetworkMessageHandlers::HandleSlateCreateFail(m_pGame, pData); return true;
		case Notify::SlateInvincible: NetworkMessageHandlers::HandleSlateInvincible(m_pGame, pData); return true;
		case Notify::SlateMana: NetworkMessageHandlers::HandleSlateMana(m_pGame, pData); return true;
		case Notify::SlateExp: NetworkMessageHandlers::HandleSlateExp(m_pGame, pData); return true;
		case Notify::SlateStatus: NetworkMessageHandlers::HandleSlateStatus(m_pGame, pData); return true;
		case Notify::SlateBerserk: NetworkMessageHandlers::HandleSlateBerserk(m_pGame, pData); return true;

		// Events (Generic)
		case Notify::SpawnEvent: NetworkMessageHandlers::HandleSpawnEvent(m_pGame, pData); return true;
		case Notify::MonsterCount: NetworkMessageHandlers::HandleMonsterCount(m_pGame, pData); return true;
		case Notify::ResurrectPlayer: NetworkMessageHandlers::HandleResurrectPlayer(m_pGame, pData); return true;

		// Crusade
		case Notify::Crusade: NetworkMessageHandlers::HandleCrusade(m_pGame, pData); return true;
		case Notify::GrandMagicResult: NetworkMessageHandlers::HandleGrandMagicResult(m_pGame, pData); return true;
		case Notify::NoMoreCrusadeStructure: NetworkMessageHandlers::HandleNoMoreCrusadeStructure(m_pGame, pData); return true;
		case Notify::EnergySphereGoalIn: NetworkMessageHandlers::HandleEnergySphereGoalIn(m_pGame, pData); return true;
		case Notify::EnergySphereCreated: NetworkMessageHandlers::HandleEnergySphereCreated(m_pGame, pData); return true;
		case Notify::MeteorStrikeComing: NetworkMessageHandlers::HandleMeteorStrikeComing(m_pGame, pData); return true;
		case Notify::MeteorStrikeHit: NetworkMessageHandlers::HandleMeteorStrikeHit(m_pGame, pData); return true;
		case Notify::CannotConstruct: NetworkMessageHandlers::HandleCannotConstruct(m_pGame, pData); return true;
		case Notify::TcLoc: NetworkMessageHandlers::HandleTCLoc(m_pGame, pData); return true;
		case Notify::ConstructionPoint: NetworkMessageHandlers::HandleConstructionPoint(m_pGame, pData); return true;

		// Map
		case Notify::MapStatusNext: NetworkMessageHandlers::HandleMapStatusNext(m_pGame, pData); return true;
		case Notify::MapStatusLast: NetworkMessageHandlers::HandleMapStatusLast(m_pGame, pData); return true;
		case Notify::LockedMap: NetworkMessageHandlers::HandleLockedMap(m_pGame, pData); return true;
		case Notify::ShowMap: NetworkMessageHandlers::HandleShowMap(m_pGame, pData); return true;

		// Crafting
		case Notify::CraftingSuccess: NetworkMessageHandlers::HandleCraftingSuccess(m_pGame, pData); return true;
		case Notify::CraftingFail: NetworkMessageHandlers::HandleCraftingFail(m_pGame, pData); return true;
		case Notify::BuildItemSuccess: NetworkMessageHandlers::HandleBuildItemSuccess(m_pGame, pData); return true;
		case Notify::BuildItemFail: NetworkMessageHandlers::HandleBuildItemFail(m_pGame, pData); return true;
		case Notify::PortionSuccess: NetworkMessageHandlers::HandlePortionSuccess(m_pGame, pData); return true;
		case Notify::PortionFail: NetworkMessageHandlers::HandlePortionFail(m_pGame, pData); return true;
		case Notify::LowPortionSkill: NetworkMessageHandlers::HandleLowPortionSkill(m_pGame, pData); return true;
		case Notify::NoMatchingPortion: NetworkMessageHandlers::HandleNoMatchingPortion(m_pGame, pData); return true;

		// Fish
		case Notify::FishChance: NetworkMessageHandlers::HandleFishChance(m_pGame, pData); return true;
		case Notify::EventFishMode: NetworkMessageHandlers::HandleEventFishMode(m_pGame, pData); return true;
		case Notify::FishCanceled: NetworkMessageHandlers::HandleFishCanceled(m_pGame, pData); return true;
		case Notify::FishSuccess: NetworkMessageHandlers::HandleFishSuccess(m_pGame, pData); return true;
		case Notify::FishFail: NetworkMessageHandlers::HandleFishFail(m_pGame, pData); return true;

		// Agriculture
		case Notify::AgricultureNoArea: NetworkMessageHandlers::HandleAgricultureNoArea(m_pGame, pData); return true;
		case Notify::AgricultureSkillLimit: NetworkMessageHandlers::HandleAgricultureSkillLimit(m_pGame, pData); return true;
		case Notify::NoMoreAgriculture: NetworkMessageHandlers::HandleNoMoreAgriculture(m_pGame, pData); return true;
		
		// Angels
		case Notify::AngelFailed: NetworkMessageHandlers::HandleAngelFailed(m_pGame, pData); return true;
		case Notify::AngelReceived: NetworkMessageHandlers::HandleAngelReceived(m_pGame, pData); return true;
		case Notify::AngelicStats: NetworkMessageHandlers::HandleAngelicStats(m_pGame, pData); return true;

		case Notify::Unknown0BEF: NetworkMessageHandlers::HandleCrashHandler(m_pGame, pData); return true;
		case Notify::IpAccountInfo: NetworkMessageHandlers::HandleIpAccountInfo(m_pGame, pData); return true;
		case Notify::RewardGold: NetworkMessageHandlers::HandleRewardGold(m_pGame, pData); return true;
		case Notify::ServerShutdown: NetworkMessageHandlers::HandleServerShutdown(m_pGame, pData); return true;

		// System (Generic)
		case Notify::WhetherChange: NetworkMessageHandlers::HandleWeatherChange(m_pGame, pData); return true;
		case Notify::TimeChange: NetworkMessageHandlers::HandleTimeChange(m_pGame, pData); return true;
		case Notify::NoticeMsg: NetworkMessageHandlers::HandleNoticeMsg(m_pGame, pData); return true;
		case Notify::StatusText: NetworkMessageHandlers::HandleStatusText(m_pGame, pData); return true;
		case Notify::ForceDisconn: NetworkMessageHandlers::HandleForceDisconn(m_pGame, pData); return true;
		case Notify::SettingSuccess: NetworkMessageHandlers::HandleSettingSuccess(m_pGame, pData); return true;
		case Notify::ServerChange: NetworkMessageHandlers::HandleServerChange(m_pGame, pData); return true;
		case Notify::TotalUsers: NetworkMessageHandlers::HandleTotalUsers(m_pGame, pData); return true;
		case Notify::ForceRecallTime: NetworkMessageHandlers::HandleForceRecallTime(m_pGame, pData); return true;
		case Notify::NoRecall: NetworkMessageHandlers::HandleNoRecall(m_pGame, pData); return true;
		case Notify::FightZoneReserve: NetworkMessageHandlers::HandleFightZoneReserve(m_pGame, pData); return true;
		case Notify::LoteryLost: NetworkMessageHandlers::HandleLoteryLost(m_pGame, pData); return true;
		case Notify::NotFlagSpot: NetworkMessageHandlers::HandleNotFlagSpot(m_pGame, pData); return true;
		case Notify::NpcTalk: NetworkMessageHandlers::HandleNpcTalk(m_pGame, pData); return true;
		case Notify::TravelerLimitedLevel: NetworkMessageHandlers::HandleTravelerLimitedLevel(m_pGame, pData); return true;
		case Notify::LimitedLevel: NetworkMessageHandlers::HandleLimitedLevel(m_pGame, pData); return true;
		case Notify::ToBeRecalled: NetworkMessageHandlers::HandleToBeRecalled(m_pGame, pData); return true;

		}
		return false;
	}

	switch (dwMsgID)
	{
	case MsgId::ResponseCreateNewGuild:
		NetworkMessageHandlers::HandleCreateNewGuildResponse(m_pGame, pData);
		return true;

	case MsgId::ResponseDisbandGuild:
		NetworkMessageHandlers::HandleDisbandGuildResponse(m_pGame, pData);
		return true;

	default:
		break;
	}

	return false;
}
