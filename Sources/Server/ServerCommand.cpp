#include "Platform.h"
#include "ServerCommand.h"
#include "CmdHelp.h"
#include "CmdShowChat.h"
#include "CmdBroadcast.h"
#include "CmdGiveItem.h"
#include "CmdReload.h"
#include "CmdSetAdmin.h"
#include "CmdSetCmdLevel.h"
#include "Game.h"
#include "winmain.h"
#include <cstring>
#include <cstdio>

ServerCommandManager& ServerCommandManager::Get()
{
	static ServerCommandManager instance;
	return instance;
}

void ServerCommandManager::Initialize(CGame* pGame)
{
	if (m_bInitialized)
		return;

	m_pGame = pGame;
	RegisterBuiltInCommands();
	m_bInitialized = true;
}

void ServerCommandManager::RegisterCommand(std::unique_ptr<ServerCommand> command)
{
	m_commands.push_back(std::move(command));
}

bool ServerCommandManager::ProcessCommand(const char* pInput)
{
	if (pInput == nullptr)
		return false;

	// Skip leading whitespace
	const char* p = pInput;
	while (*p == ' ' || *p == '\t')
		p++;

	if (*p == '\0')
		return false;

	// Extract the command name (first word)
	const char* cmdStart = p;
	while (*p != '\0' && *p != ' ' && *p != '\t')
		p++;
	size_t cmdLen = p - cmdStart;

	// Skip whitespace after command name to get args
	while (*p == ' ' || *p == '\t')
		p++;
	const char* pArgs = p;

	// Find matching command (case-insensitive)
	for (const auto& cmd : m_commands)
	{
		const char* cmdName = cmd->GetName();
		if (std::strlen(cmdName) == cmdLen && _strnicmp(cmdStart, cmdName, cmdLen) == 0)
		{
			cmd->Execute(m_pGame, pArgs);
			return true;
		}
	}

	char logBuf[256];
	std::snprintf(logBuf, sizeof(logBuf), "Unknown command: '%s'. Type 'help' for a list of commands.", cmdStart);
	PutLogList(logBuf);
	return false;
}

void ServerCommandManager::RegisterBuiltInCommands()
{
	RegisterCommand(std::make_unique<CmdHelp>(m_commands));
	RegisterCommand(std::make_unique<CmdShowChat>());
	RegisterCommand(std::make_unique<CmdBroadcast>());
	RegisterCommand(std::make_unique<CmdGiveItem>());
	RegisterCommand(std::make_unique<CmdReload>());
	RegisterCommand(std::make_unique<CmdSetAdmin>());
	RegisterCommand(std::make_unique<CmdSetCmdLevel>());
}
