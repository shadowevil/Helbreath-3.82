#include "Platform.h"
#include "CmdHelp.h"
#include "winmain.h"
#include <cstdio>
#include <cstring>

void CmdHelp::Execute(CGame* pGame, const char* pArgs)
{
	// help <command> — show detailed help for a specific command
	if (pArgs != nullptr && pArgs[0] != '\0')
	{
		for (const auto& cmd : m_commands)
		{
			if (_stricmp(pArgs, cmd->GetName()) == 0)
			{
				char buf[256];
				std::snprintf(buf, sizeof(buf), "  %s - %s", cmd->GetName(), cmd->GetDescription());
				PutLogList(buf);

				// Print help text line by line (split on \n)
				const char* help = cmd->GetHelp();
				const char* p = help;
				while (*p != '\0')
				{
					const char* lineEnd = p;
					while (*lineEnd != '\0' && *lineEnd != '\n')
						lineEnd++;

					char line[256];
					size_t len = lineEnd - p;
					if (len >= sizeof(line)) len = sizeof(line) - 1;
					std::memcpy(line, p, len);
					line[len] = '\0';
					PutLogList(line);

					p = (*lineEnd == '\n') ? lineEnd + 1 : lineEnd;
				}
				return;
			}
		}

		char buf[256];
		std::snprintf(buf, sizeof(buf), "Unknown command: '%s'. Type 'help' for a list.", pArgs);
		PutLogList(buf);
		return;
	}

	// help — list all commands
	PutLogList((char*)"Available commands:");
	for (const auto& cmd : m_commands)
	{
		char buf[256];
		std::snprintf(buf, sizeof(buf), "  %-12s %s", cmd->GetName(), cmd->GetDescription());
		PutLogList(buf);
	}
	PutLogList((char*)"Type 'help <command>' for detailed usage.");
}
