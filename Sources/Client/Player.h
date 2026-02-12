#pragma once
#include "PlatformCompat.h"
#include <cstdint>
#include <cstring>
#include <string>
#include <array>
#include "PlayerController.h"
#include "NetConstants.h"
#include "Appearance.h"
#include "PlayerStatusData.h"
#include "ActionID.h"

namespace hb::client::config
{
constexpr int PlayerNameLength = 12;
constexpr int GuildNameLength = 22;
constexpr int PlayerMaxMagicType = 100;
constexpr int PlayerMaxSkillType = 60;
} // namespace hb::client::config

//=============================================================================
// Player Animation Definitions
// All player types (1-6) share these timings.
// Values from original m_stFrame[1-6] initialization in MapData.cpp
//=============================================================================
struct AnimDef
{
	int16_t sMaxFrame;
	int16_t sFrameTime;  // Base ms per frame (before status modifiers)
	bool    bLoop;
};

namespace PlayerAnim {
	static constexpr AnimDef Stop       = { 14, 60,  false };
	static constexpr AnimDef Move       = {  7, 70,  false };
	static constexpr AnimDef Run        = {  7, 39,  false };
	static constexpr AnimDef Attack     = {  7, 78,  false };
	static constexpr AnimDef AttackMove = { 12, 78,  false };
	static constexpr AnimDef Magic      = { 15, 88,  false };
	static constexpr AnimDef GetItem    = {  3, 75, false };
	static constexpr AnimDef Damage     = {  7, 70,  false }; // 3+4
	static constexpr AnimDef DamageMove = {  3, 50,  false };
	static constexpr AnimDef Dying      = { 12, 80,  false };

	inline const AnimDef& FromAction(int8_t action)
	{
		switch (action) {
		case hb::shared::action::Type::Stop:       return Stop;
		case hb::shared::action::Type::Move:       return Move;
		case hb::shared::action::Type::Run:        return Run;
		case hb::shared::action::Type::Attack:     return Attack;
		case hb::shared::action::Type::AttackMove: return AttackMove;
		case hb::shared::action::Type::Magic:      return Magic;
		case hb::shared::action::Type::GetItem:    return GetItem;
		case hb::shared::action::Type::Damage:     return Damage;
		case hb::shared::action::Type::DamageMove: return DamageMove;
		case hb::shared::action::Type::Dying:      return Dying;
		default:                   return Stop;
		}
	}
}

class CPlayer
{
public:
    CPlayer();
    ~CPlayer();
    void Reset();

    // Movement Controller
    CPlayerController m_Controller;

    // IDENTITY & ACCOUNT
    std::string m_cPlayerName;
    short m_sPlayerObjectID;
    short m_sPlayerType;
    std::string m_cAccountName;
    std::string m_cAccountPassword;
    std::string m_cGuildName;
    int m_iGuildRank;

    // POSITION & MOVEMENT
    short m_sPlayerX, m_sPlayerY;
    int8_t m_iPlayerDir;
    short m_sDamageMove, m_sDamageMoveAmount;

    // RESOURCES
    int m_iHP, m_iMP, m_iSP, m_iHungerStatus;

    // BASE STATS
    int m_iStr, m_iVit, m_iDex, m_iInt, m_iMag, m_iCharisma;
    int m_iAngelicStr, m_iAngelicInt, m_iAngelicDex, m_iAngelicMag;

    // PROGRESSION
    int m_iLevel;
    uint32_t m_iExp;
    int m_iLU_Point;
    int16_t m_wLU_Str, m_wLU_Vit, m_wLU_Dex, m_wLU_Int, m_wLU_Mag, m_wLU_Char;
    int8_t m_iStatModStr, m_iStatModVit, m_iStatModDex, m_iStatModInt, m_iStatModMag, m_iStatModChr;

    // COMBAT
    int m_iAC, m_iTHAC0;
    hb::shared::entity::PlayerStatus m_playerStatus;
    int m_iPKCount, m_iEnemyKillCount, m_iRewardGold, m_iContribution;
    int m_iSuperAttackLeft, m_iSpecialAbilityType, m_iSpecialAbilityTimeLeftSec;

    // APPEARANCE
    hb::shared::entity::PlayerAppearance m_playerAppearance;

    // Illusion Effect appearance
    hb::shared::entity::PlayerStatus m_illusionStatus;
    hb::shared::entity::PlayerAppearance m_illusionAppearance;
    int8_t m_iGender, m_iSkinCol, m_iHairStyle, m_iHairCol, m_iUnderCol;

    // SKILLS & MAGIC
    std::array<int8_t, hb::client::config::PlayerMaxMagicType> m_iMagicMastery{};
    std::array<uint8_t, hb::client::config::PlayerMaxSkillType> m_iSkillMastery{};

    // STATUS FLAGS
    bool m_bIsPoisoned, m_bIsConfusion, m_bParalyze;
    bool m_bIsCombatMode, m_bIsSafeAttackMode, m_bForceAttack;
    bool m_bSuperAttackMode, m_bIsSpecialAbilityEnabled;
    bool m_bHunter, m_bAresden, m_bCitizen;

    // ADMIN / GM
    bool m_bIsGMMode = false;

    // CRUSADE/WAR
    int m_iCrusadeDuty, m_iWarContribution, m_iConstructionPoint;
    int m_iConstructLocX, m_iConstructLocY;
};
