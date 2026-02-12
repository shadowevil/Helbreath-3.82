#pragma once

#include <cstdint>
#include <cstddef>

class CGame;

class SkillManager
{
public:
	SkillManager() = default;
	~SkillManager() = default;

	void SetGame(CGame* pGame) { m_pGame = pGame; }

	// Skill config
	bool bSendClientSkillConfigs(int iClientH);
	void ReloadSkillConfigs();

	// Skill use
	void UseSkillHandler(int iClientH, int iV1, int iV2, int iV3);
	void ClearSkillUsingStatus(int iClientH);
	void _TamingHandler(int iClientH, int iSkillNum, char cMapIndex, int dX, int dY);

	// Skill mastery / SSN
	void CalculateSSN_SkillIndex(int iClientH, short sSkillIndex, int iValue);
	bool bCheckTotalSkillMasteryPoints(int iClientH, int iSkill);
	int _iCalcSkillSSNpoint(int iLevel);
	void TrainSkillResponse(bool bSuccess, int iClientH, int iSkillNum, int iSkillLevel);

	// Skill management
	void SetDownSkillIndexHandler(int iClientH, int iSkillIndex);
	void SkillCheck(int sTargetH);
	void SetSkillAll(int iClientH, char* pData, size_t dwMsgSize);

private:
	CGame* m_pGame = nullptr;
};
