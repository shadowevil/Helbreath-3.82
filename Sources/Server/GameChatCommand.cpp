#include "Platform.h"
#include "GameChatCommand.h"
#include "GameCmdWhisper.h"
#include "GameCmdBlock.h"
#include "GameCmdUnblock.h"
#include "GameCmdGM.h"
#include "GameCmdRegen.h"
#include "GameCmdCreateItem.h"
#include "GameCmdGiveItem.h"
#include "GameCmdSpawn.h"
#include "GameCmdGoto.h"
#include "GameCmdCome.h"
#include "GameCmdInvis.h"
#include "Game.h"
#include "Client.h"
#include "AdminLevel.h"
#include "GameConfigSqliteStore.h"
#include "winmain.h"
#include <cstring>
#include <cstdio>

using namespace hb::shared::net;
GameChatCommandManager& GameChatCommandManager::Get()
{
	static GameChatCommandManager instance;
	return instance;
}

void GameChatCommandManager::Initialize(CGame* pGame)
{
	if (m_bInitialized)
		return;

	m_pGame = pGame;
	RegisterBuiltInCommands();
	SeedCommandPermissions();
	m_bInitialized = true;
}

void GameChatCommandManager::RegisterCommand(std::unique_ptr<GameChatCommand> command)
{
	m_commands.push_back(std::move(command));
}

bool GameChatCommandManager::ProcessCommand(int iClientH, const char* pMessage, size_t dwMsgSize)
{
	if (m_pGame == nullptr || pMessage == nullptr)
		return false;

	if (pMessage[0] != '/')
		return false;

	const char* pCommand = pMessage + 1;

	for (const auto& cmd : m_commands)
	{
		const char* cmdName = cmd->GetName();
		size_t cmdLen = std::strlen(cmdName);

		if (_strnicmp(pCommand, cmdName, cmdLen) == 0)
		{
			char nextChar = pCommand[cmdLen];
			if (nextChar == '\0' || nextChar == ' ' || nextChar == '\t')
			{
				const char* pArgs = pCommand + cmdLen;
				while (*pArgs == ' ' || *pArgs == '\t')
					pArgs++;

				// Permission check: DB is sole authority, default to Administrator if not configured
				int iRequiredLevel = m_pGame->GetCommandRequiredLevel(cmd->GetName());

				// Level 0 = no restriction (player commands like /to, /block)
				if (iRequiredLevel > 0)
				{
					int iPlayerLevel = m_pGame->m_pClientList[iClientH]->m_iAdminLevel;
					if (iPlayerLevel < iRequiredLevel)
						return false;

					// Admin commands require GM mode to be active (except /gm itself)
					if (cmd->RequiresGMMode() && !m_pGame->m_pClientList[iClientH]->m_bIsGMMode)
					{
						m_pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "You must enable GM mode first (/gm on).");
						return true;
					}
				}

				LogCommand(iClientH, pMessage);
				return cmd->Execute(m_pGame, iClientH, pArgs);
			}
		}
	}

	return false;
}

void GameChatCommandManager::LogCommand(int iClientH, const char* pCommand)
{
	if (m_pGame == nullptr || m_pGame->m_pClientList[iClientH] == nullptr)
		return;

	FILE* pFile = fopen("GameLogs/Commands.log", "a");
	if (pFile == nullptr)
		return;

	SYSTEMTIME st;
	GetLocalTime(&st);

	fprintf(pFile, "[%02d:%02d:%02d] %s: %s\n",
		st.wHour, st.wMinute, st.wSecond,
		m_pGame->m_pClientList[iClientH]->m_cCharName,
		pCommand);

	fclose(pFile);
}

void GameChatCommandManager::RegisterBuiltInCommands()
{
	RegisterCommand(std::make_unique<GameCmdWhisper>());
	RegisterCommand(std::make_unique<GameCmdBlock>());
	RegisterCommand(std::make_unique<GameCmdUnblock>());
	RegisterCommand(std::make_unique<GameCmdGM>());
	RegisterCommand(std::make_unique<GameCmdRegen>());
	RegisterCommand(std::make_unique<GameCmdCreateItem>());
	RegisterCommand(std::make_unique<GameCmdGiveItem>());
	RegisterCommand(std::make_unique<GameCmdSpawn>());
	RegisterCommand(std::make_unique<GameCmdGoto>());
	RegisterCommand(std::make_unique<GameCmdCome>());
	RegisterCommand(std::make_unique<GameCmdInvis>());
}

void GameChatCommandManager::SeedCommandPermissions()
{
	if (m_pGame == nullptr)
		return;

	bool bChanged = false;
	for (const auto& cmd : m_commands)
	{
		const char* name = cmd->GetName();
		if (m_pGame->m_commandPermissions.find(name) == m_pGame->m_commandPermissions.end())
		{
			CommandPermission perm;
			perm.iAdminLevel = cmd->GetDefaultLevel();
			m_pGame->m_commandPermissions[name] = perm;
			bChanged = true;

			char buf[128];
			std::snprintf(buf, sizeof(buf), "(!) New command '/%s' added to permissions (default: level %d)", name, cmd->GetDefaultLevel());
			PutLogList(buf);
		}
	}

	if (bChanged)
	{
		sqlite3* configDb = nullptr;
		std::string dbPath;
		if (EnsureGameConfigDatabase(&configDb, dbPath, nullptr))
		{
			SaveCommandPermissions(configDb, m_pGame);
			CloseGameConfigDatabase(configDb);
		}
	}
}
