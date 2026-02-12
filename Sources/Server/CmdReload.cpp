#include "Platform.h"
#include "CmdReload.h"
#include "Game.h"
#include "SkillManager.h"
#include "MagicManager.h"
#include "ItemManager.h"
#include "winmain.h"
#include <cstdio>
#include <cstring>

void CmdReload::Execute(CGame* pGame, const char* pArgs)
{
	if (pArgs == nullptr || pArgs[0] == '\0')
	{
		PutLogList((char*)"Usage: reload <items|magic|skills|npcs|all>");
		return;
	}

	bool bItems = false;
	bool bMagic = false;
	bool bSkills = false;
	bool bNpcs = false;

	if (_stricmp(pArgs, "items") == 0)
		bItems = true;
	else if (_stricmp(pArgs, "magic") == 0)
		bMagic = true;
	else if (_stricmp(pArgs, "skills") == 0)
		bSkills = true;
	else if (_stricmp(pArgs, "npcs") == 0)
		bNpcs = true;
	else if (_stricmp(pArgs, "all") == 0)
	{
		bItems = true;
		bMagic = true;
		bSkills = true;
		bNpcs = true;
	}
	else
	{
		char buf[256];
		std::snprintf(buf, sizeof(buf), "(!) Unknown reload target: '%s'. Use items, magic, skills, npcs, or all.", pArgs);
		PutLogList(buf);
		return;
	}

	// Send reload notification to clients first (shows top bar message)
	if (bItems || bMagic || bSkills || bNpcs)
		pGame->SendConfigReloadNotification(bItems, bMagic, bSkills, bNpcs);

	// Reload configs from database
	if (bItems)  pGame->m_pItemManager->ReloadItemConfigs();
	if (bMagic)  pGame->m_pMagicManager->ReloadMagicConfigs();
	if (bSkills) pGame->m_pSkillManager->ReloadSkillConfigs();
	if (bNpcs)   pGame->ReloadNpcConfigs();

	// Stream updated config data to clients
	if (bItems || bMagic || bSkills || bNpcs)
		pGame->PushConfigReloadToClients(bItems, bMagic, bSkills, bNpcs);
}
