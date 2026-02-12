#define NOMINMAX
#include "BuildItemManager.h"
#include "Game.h"
#include <fstream>
#include <string>

BuildItemManager::BuildItemManager() = default;
BuildItemManager::~BuildItemManager() = default;

BuildItemManager& BuildItemManager::Get()
{
	static BuildItemManager instance;
	return instance;
}

void BuildItemManager::SetGame(CGame* pGame)
{
	m_game = pGame;
}

bool BuildItemManager::LoadRecipes()
{
	for (int i = 0; i < hb::shared::limits::MaxBuildItems; i++)
	{
		if (m_recipes[i] != 0)
			m_recipes[i].reset();
	}

	std::ifstream file("contents/BItemcfg.txt");
	if (!file) return false;

	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	return ParseRecipeFile(content);
}

bool BuildItemManager::UpdateAvailableRecipes()
{
	int iIndex, i, j, iMatch, iCount;
	char cTempName[hb::shared::limits::ItemNameLen];
	int  iItemCount[hb::shared::limits::MaxItems];

	for (i = 0; i < hb::shared::limits::MaxBuildItems; i++)
		if (m_display_recipes[i] != 0)
		{
			m_display_recipes[i].reset();
		}
	iIndex = 0;
	for (i = 0; i < hb::shared::limits::MaxBuildItems; i++)
		if (m_recipes[i] != 0)
		{	// Skill-Limit
			if (m_game->m_pPlayer->m_iSkillMastery[13] >= m_recipes[i]->m_iSkillLimit)
			{
				iMatch = 0;
				m_display_recipes[iIndex] = std::make_unique<CBuildItem>();
				m_display_recipes[iIndex]->m_cName = m_recipes[i]->m_cName;

				m_display_recipes[iIndex]->m_cElementName1 = m_recipes[i]->m_cElementName1;
				m_display_recipes[iIndex]->m_cElementName2 = m_recipes[i]->m_cElementName2;
				m_display_recipes[iIndex]->m_cElementName3 = m_recipes[i]->m_cElementName3;
				m_display_recipes[iIndex]->m_cElementName4 = m_recipes[i]->m_cElementName4;
				m_display_recipes[iIndex]->m_cElementName5 = m_recipes[i]->m_cElementName5;
				m_display_recipes[iIndex]->m_cElementName6 = m_recipes[i]->m_cElementName6;

				m_display_recipes[iIndex]->m_iElementCount[1] = m_recipes[i]->m_iElementCount[1];
				m_display_recipes[iIndex]->m_iElementCount[2] = m_recipes[i]->m_iElementCount[2];
				m_display_recipes[iIndex]->m_iElementCount[3] = m_recipes[i]->m_iElementCount[3];
				m_display_recipes[iIndex]->m_iElementCount[4] = m_recipes[i]->m_iElementCount[4];
				m_display_recipes[iIndex]->m_iElementCount[5] = m_recipes[i]->m_iElementCount[5];
				m_display_recipes[iIndex]->m_iElementCount[6] = m_recipes[i]->m_iElementCount[6];

				m_display_recipes[iIndex]->m_iSprH = m_recipes[i]->m_iSprH;
				m_display_recipes[iIndex]->m_iSprFrame = m_recipes[i]->m_iSprFrame;
				m_display_recipes[iIndex]->m_iMaxSkill = m_recipes[i]->m_iMaxSkill;
				m_display_recipes[iIndex]->m_iSkillLimit = m_recipes[i]->m_iSkillLimit;

				// ItemCount
				for (j = 0; j < hb::shared::limits::MaxItems; j++)
					if (m_game->m_pItemList[j] != 0)
						iItemCount[j] = m_game->m_pItemList[j]->m_dwCount;
					else iItemCount[j] = 0;

				// Element1
				std::snprintf(cTempName, sizeof(cTempName), "%s", m_recipes[i]->m_cElementName1.c_str());
				iCount = m_recipes[i]->m_iElementCount[1];
				if (iCount == 0) iMatch++;
				else
				{
					for (j = 0; j < hb::shared::limits::MaxItems; j++)
						if (m_game->m_pItemList[j] != 0) {
							CItem* pCfgJ = m_game->GetItemConfig(m_game->m_pItemList[j]->m_sIDnum);
							if (pCfgJ && (memcmp(pCfgJ->m_cName, cTempName, hb::shared::limits::ItemNameLen - 1) == 0) && (m_game->m_pItemList[j]->m_dwCount >= static_cast<DWORD>(iCount)) &&
								(iItemCount[j] > 0))
							{
								iMatch++;
								m_display_recipes[iIndex]->m_bElementFlag[1] = true;
								iItemCount[j] -= iCount;
								goto CBIS_STEP2;
							}
						}
				}

			CBIS_STEP2:;
				// Element2
				std::snprintf(cTempName, sizeof(cTempName), "%s", m_recipes[i]->m_cElementName2.c_str());
				iCount = m_recipes[i]->m_iElementCount[2];
				if (iCount == 0) iMatch++;
				else
				{
					for (j = 0; j < hb::shared::limits::MaxItems; j++)
						if (m_game->m_pItemList[j] != 0)
						{
							CItem* pCfgJ = m_game->GetItemConfig(m_game->m_pItemList[j]->m_sIDnum);
							if (pCfgJ && (memcmp(pCfgJ->m_cName, cTempName, hb::shared::limits::ItemNameLen - 1) == 0) && (m_game->m_pItemList[j]->m_dwCount >= static_cast<DWORD>(iCount)) &&
								(iItemCount[j] > 0))
							{
								iMatch++;
								m_display_recipes[iIndex]->m_bElementFlag[2] = true;
								iItemCount[j] -= iCount;
								goto CBIS_STEP3;
							}
						}
				}

			CBIS_STEP3:;
				// Element3
				std::snprintf(cTempName, sizeof(cTempName), "%s", m_recipes[i]->m_cElementName3.c_str());
				iCount = m_recipes[i]->m_iElementCount[3];
				if (iCount == 0) iMatch++;
				else
				{
					for (j = 0; j < hb::shared::limits::MaxItems; j++)
						if (m_game->m_pItemList[j] != 0)
						{
							CItem* pCfgJ = m_game->GetItemConfig(m_game->m_pItemList[j]->m_sIDnum);
							if (pCfgJ && (memcmp(pCfgJ->m_cName, cTempName, hb::shared::limits::ItemNameLen - 1) == 0) && (m_game->m_pItemList[j]->m_dwCount >= static_cast<DWORD>(iCount)) &&
								(iItemCount[j] > 0))
							{
								iMatch++;
								m_display_recipes[iIndex]->m_bElementFlag[3] = true;
								iItemCount[j] -= iCount;
								goto CBIS_STEP4;
							}
						}
				}

			CBIS_STEP4:;
				// Element4 �˻�
				std::snprintf(cTempName, sizeof(cTempName), "%s", m_recipes[i]->m_cElementName4.c_str());
				iCount = m_recipes[i]->m_iElementCount[4];
				if (iCount == 0) iMatch++;
				else
				{
					for (j = 0; j < hb::shared::limits::MaxItems; j++)
						if (m_game->m_pItemList[j] != 0)
						{
							CItem* pCfgJ = m_game->GetItemConfig(m_game->m_pItemList[j]->m_sIDnum);
							if (pCfgJ && (memcmp(pCfgJ->m_cName, cTempName, hb::shared::limits::ItemNameLen - 1) == 0) && (m_game->m_pItemList[j]->m_dwCount >= static_cast<DWORD>(iCount)) &&
								(iItemCount[j] > 0))
							{
								iMatch++;
								m_display_recipes[iIndex]->m_bElementFlag[4] = true;
								iItemCount[j] -= iCount;
								goto CBIS_STEP5;
							}
						}
				}

			CBIS_STEP5:;

				// Element5
				std::snprintf(cTempName, sizeof(cTempName), "%s", m_recipes[i]->m_cElementName5.c_str());
				iCount = m_recipes[i]->m_iElementCount[5];
				if (iCount == 0) iMatch++;
				else
				{
					for (j = 0; j < hb::shared::limits::MaxItems; j++)
						if (m_game->m_pItemList[j] != 0)
						{
							CItem* pCfgJ = m_game->GetItemConfig(m_game->m_pItemList[j]->m_sIDnum);
							if (pCfgJ && (memcmp(pCfgJ->m_cName, cTempName, hb::shared::limits::ItemNameLen - 1) == 0) && (m_game->m_pItemList[j]->m_dwCount >= static_cast<DWORD>(iCount)) &&
								(iItemCount[j] > 0))
							{
								iMatch++;
								m_display_recipes[iIndex]->m_bElementFlag[5] = true;
								iItemCount[j] -= iCount;
								goto CBIS_STEP6;
							}
						}
				}

			CBIS_STEP6:;

				// Element6
				std::snprintf(cTempName, sizeof(cTempName), "%s", m_recipes[i]->m_cElementName6.c_str());
				iCount = m_recipes[i]->m_iElementCount[6];
				if (iCount == 0) iMatch++;
				else
				{
					for (j = 0; j < hb::shared::limits::MaxItems; j++)
						if (m_game->m_pItemList[j] != 0)
						{
							CItem* pCfgJ = m_game->GetItemConfig(m_game->m_pItemList[j]->m_sIDnum);
							if (pCfgJ && (memcmp(pCfgJ->m_cName, cTempName, hb::shared::limits::ItemNameLen - 1) == 0) && (m_game->m_pItemList[j]->m_dwCount >= static_cast<DWORD>(iCount)) &&
								(iItemCount[j] > 0))
							{
								iMatch++;
								m_display_recipes[iIndex]->m_bElementFlag[6] = true;
								iItemCount[j] -= iCount;
								goto CBIS_STEP7;
							}
						}
				}

			CBIS_STEP7:;

				if (iMatch == 6) m_display_recipes[iIndex]->m_bBuildEnabled = true;
				iIndex++;
			}
		}
	return true;
}

bool BuildItemManager::ParseRecipeFile(const std::string& buffer)
{
	constexpr std::string_view seps = "= ,\t\n";
	char cReadModeA = 0;
	char cReadModeB = 0;
	int iIndex = 0;

	auto copyToCharArray = [](char* dest, size_t destSize, const std::string& src) {
		std::memset(dest, 0, destSize);
		size_t copyLen = std::min(src.length(), destSize - 1);
		std::memcpy(dest, src.c_str(), copyLen);
	};

	size_t start = 0;
	while (start < buffer.size())
	{
		size_t end = buffer.find_first_of(seps, start);
		if (end == std::string::npos) end = buffer.size();

		if (end > start)
		{
			std::string token = buffer.substr(start, end - start);

			if (cReadModeA != 0)
			{
				switch (cReadModeA) {
				case 1:
					switch (cReadModeB) {
					case 1:
						m_recipes[iIndex]->m_cName = token;
						cReadModeB = 2;
						break;
					case 2:
						m_recipes[iIndex]->m_iSkillLimit = std::stoi(token);
						cReadModeB = 3;
						break;
					case 3:
						m_recipes[iIndex]->m_cElementName1 = token;
						cReadModeB = 4;
						break;
					case 4:
						m_recipes[iIndex]->m_iElementCount[1] = std::stoi(token);
						cReadModeB = 5;
						break;
					case 5:
						m_recipes[iIndex]->m_cElementName2 = token;
						cReadModeB = 6;
						break;
					case 6:
						m_recipes[iIndex]->m_iElementCount[2] = std::stoi(token);
						cReadModeB = 7;
						break;
					case 7:
						m_recipes[iIndex]->m_cElementName3 = token;
						cReadModeB = 8;
						break;
					case 8:
						m_recipes[iIndex]->m_iElementCount[3] = std::stoi(token);
						cReadModeB = 9;
						break;
					case 9:
						m_recipes[iIndex]->m_cElementName4 = token;
						cReadModeB = 10;
						break;
					case 10:
						m_recipes[iIndex]->m_iElementCount[4] = std::stoi(token);
						cReadModeB = 11;
						break;
					case 11:
						m_recipes[iIndex]->m_cElementName5 = token;
						cReadModeB = 12;
						break;
					case 12:
						if (token == "xxx")
							m_recipes[iIndex]->m_iElementCount[5] = 0;
						else
							m_recipes[iIndex]->m_iElementCount[5] = std::stoi(token);
						cReadModeB = 13;
						break;
					case 13:
						m_recipes[iIndex]->m_cElementName6 = token;
						cReadModeB = 14;
						break;
					case 14:
						if(token == "xxx")
							m_recipes[iIndex]->m_iElementCount[6] = 0;
						else
							m_recipes[iIndex]->m_iElementCount[6] = std::stoi(token);
						cReadModeB = 15;
						break;
					case 15:
						m_recipes[iIndex]->m_iSprH = std::stoi(token);
						cReadModeB = 16;
						break;
					case 16:
						m_recipes[iIndex]->m_iSprFrame = std::stoi(token);
						cReadModeB = 17;
						break;
					case 17:
						m_recipes[iIndex]->m_iMaxSkill = std::stoi(token);
						cReadModeA = 0;
						cReadModeB = 0;
						iIndex++;
						break;
					}
					break;
				}
			}
			else
			{
				if (token.starts_with("BuildItem"))
				{
					cReadModeA = 1;
					cReadModeB = 1;
					m_recipes[iIndex] = std::make_unique<CBuildItem>();
				}
			}
		}
		start = end + 1;
	}
	return (cReadModeA == 0) && (cReadModeB == 0);
}

bool BuildItemManager::ValidateCurrentRecipe()
{
	int i, iCount2, iMatch, iIndex, iItemIndex[7];
	int iCount;
	int iItemCount[7];
	char cTempName[hb::shared::limits::ItemNameLen];
	bool bItemFlag[7];

	iIndex = m_game->m_dialogBoxManager.Info(DialogBoxId::Manufacture).cStr[0];

	if (m_recipes[iIndex] == 0) return false;

	iItemIndex[1] = m_game->m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV1;
	iItemIndex[2] = m_game->m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV2;
	iItemIndex[3] = m_game->m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV3;
	iItemIndex[4] = m_game->m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV4;
	iItemIndex[5] = m_game->m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV5;
	iItemIndex[6] = m_game->m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV6;

	for (i = 1; i <= 6; i++)
		if (iItemIndex[i] != -1)
			iItemCount[i] = m_game->m_pItemList[iItemIndex[i]]->m_dwCount;
		else iItemCount[i] = 0;
	iMatch = 0;
	for (i = 1; i <= 6; i++) bItemFlag[i] = false;

	// Element1
	std::snprintf(cTempName, sizeof(cTempName), "%s", m_display_recipes[iIndex]->m_cElementName1.c_str());
	iCount = m_display_recipes[iIndex]->m_iElementCount[1];
	if (iCount == 0) iMatch++;
	else
	{
		for (i = 1; i <= 6; i++)
		{
			if (iItemIndex[i] == -1) continue;
			CItem* pCfgBI = m_game->GetItemConfig(m_game->m_pItemList[iItemIndex[i]]->m_sIDnum);
			if (pCfgBI && (memcmp(pCfgBI->m_cName, cTempName, hb::shared::limits::ItemNameLen - 1) == 0) &&
				(m_game->m_pItemList[iItemIndex[i]]->m_dwCount >= static_cast<DWORD>(iCount)) &&
				(iItemCount[i] > 0) && (bItemFlag[i] == false))
			{
				iMatch++;
				iItemCount[i] -= iCount;
				bItemFlag[i] = true;
				goto CCBIS_STEP2;
			}
		}
	}

CCBIS_STEP2:;

	// Element2
	std::snprintf(cTempName, sizeof(cTempName), "%s", m_display_recipes[iIndex]->m_cElementName2.c_str());
	iCount = m_display_recipes[iIndex]->m_iElementCount[2];
	if (iCount == 0) iMatch++;
	else
	{
		for (i = 1; i <= 6; i++)
		{
			if (iItemIndex[i] == -1) continue;
			CItem* pCfgBI = m_game->GetItemConfig(m_game->m_pItemList[iItemIndex[i]]->m_sIDnum);
			if (pCfgBI && (memcmp(pCfgBI->m_cName, cTempName, hb::shared::limits::ItemNameLen - 1) == 0) &&
				(m_game->m_pItemList[iItemIndex[i]]->m_dwCount >= static_cast<DWORD>(iCount)) &&
				(iItemCount[i] > 0) && (bItemFlag[i] == false))
			{
				iMatch++;
				iItemCount[i] -= iCount;
				bItemFlag[i] = true;
				goto CCBIS_STEP3;
			}
		}
	}

CCBIS_STEP3:;

	// Element3
	std::snprintf(cTempName, sizeof(cTempName), "%s", m_display_recipes[iIndex]->m_cElementName3.c_str());
	iCount = m_display_recipes[iIndex]->m_iElementCount[3];
	if (iCount == 0) iMatch++;
	else
	{
		for (i = 1; i <= 6; i++)
		{
			if (iItemIndex[i] == -1) continue;
			CItem* pCfgBI = m_game->GetItemConfig(m_game->m_pItemList[iItemIndex[i]]->m_sIDnum);
			if (pCfgBI && (memcmp(pCfgBI->m_cName, cTempName, hb::shared::limits::ItemNameLen - 1) == 0) &&
				(m_game->m_pItemList[iItemIndex[i]]->m_dwCount >= static_cast<DWORD>(iCount)) &&
				(iItemCount[i] > 0) && (bItemFlag[i] == false))
			{
				iMatch++;
				iItemCount[i] -= iCount;
				bItemFlag[i] = true;
				goto CCBIS_STEP4;
			}
		}
	}

CCBIS_STEP4:;

	// Element4
	std::snprintf(cTempName, sizeof(cTempName), "%s", m_display_recipes[iIndex]->m_cElementName4.c_str());
	iCount = m_display_recipes[iIndex]->m_iElementCount[4];
	if (iCount == 0) iMatch++;
	else
	{
		for (i = 1; i <= 6; i++)
		{
			if (iItemIndex[i] == -1) continue;
			CItem* pCfgBI = m_game->GetItemConfig(m_game->m_pItemList[iItemIndex[i]]->m_sIDnum);
			if (pCfgBI && (memcmp(pCfgBI->m_cName, cTempName, hb::shared::limits::ItemNameLen - 1) == 0) &&
				(m_game->m_pItemList[iItemIndex[i]]->m_dwCount >= static_cast<DWORD>(iCount)) &&
				(iItemCount[i] > 0) && (bItemFlag[i] == false))
			{
				iMatch++;
				iItemCount[i] -= iCount;
				bItemFlag[i] = true;
				goto CCBIS_STEP5;
			}
		}
	}

CCBIS_STEP5:;

	// Element5
	std::snprintf(cTempName, sizeof(cTempName), "%s", m_display_recipes[iIndex]->m_cElementName5.c_str());
	iCount = m_display_recipes[iIndex]->m_iElementCount[5];
	if (iCount == 0) iMatch++;
	else
	{
		for (i = 1; i <= 6; i++)
		{
			if (iItemIndex[i] == -1) continue;
			CItem* pCfgBI = m_game->GetItemConfig(m_game->m_pItemList[iItemIndex[i]]->m_sIDnum);
			if (pCfgBI && (memcmp(pCfgBI->m_cName, cTempName, hb::shared::limits::ItemNameLen - 1) == 0) &&
				(m_game->m_pItemList[iItemIndex[i]]->m_dwCount >= static_cast<DWORD>(iCount)) &&
				(iItemCount[i] > 0) && (bItemFlag[i] == false))
			{
				iMatch++;
				iItemCount[i] -= iCount;
				bItemFlag[i] = true;
				goto CCBIS_STEP6;
			}
		}
	}

CCBIS_STEP6:;

	// Element6
	std::snprintf(cTempName, sizeof(cTempName), "%s", m_display_recipes[iIndex]->m_cElementName6.c_str());
	iCount = m_display_recipes[iIndex]->m_iElementCount[6];
	if (iCount == 0) iMatch++;
	else
	{
		for (i = 1; i <= 6; i++)
		{
			if (iItemIndex[i] == -1) continue;
			CItem* pCfgBI = m_game->GetItemConfig(m_game->m_pItemList[iItemIndex[i]]->m_sIDnum);
			if (pCfgBI && (memcmp(pCfgBI->m_cName, cTempName, hb::shared::limits::ItemNameLen - 1) == 0) &&
				(m_game->m_pItemList[iItemIndex[i]]->m_dwCount >= static_cast<DWORD>(iCount)) &&
				(iItemCount[i] > 0) && (bItemFlag[i] == false))
			{
				iMatch++;
				iItemCount[i] -= iCount;
				bItemFlag[i] = true;
				goto CCBIS_STEP7;
			}
		}
	}

CCBIS_STEP7:;

	iCount = 0;
	for (i = 1; i <= 6; i++)
		if (m_display_recipes[iIndex]->m_iElementCount[i] != 0) iCount++;
	iCount2 = 0;
	for (i = 1; i <= 6; i++)
		if (iItemIndex[i] != -1) iCount2++;
	if ((iMatch == 6) && (iCount == iCount2)) return true;
	return false;
}

