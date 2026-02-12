#include "Platform.h"
#include "CmdBroadcast.h"
#include "Game.h"
#include "winmain.h"
#include <cstdio>

void CmdBroadcast::Execute(CGame* pGame, const char* pArgs)
{
	if (pArgs == nullptr || pArgs[0] == '\0')
	{
		PutLogList((char*)"Usage: broadcast <message>");
		return;
	}

	pGame->BroadcastServerMessage(pArgs);

	char buf[300];
	std::snprintf(buf, sizeof(buf), "(!) Broadcast sent: %s", pArgs);
	PutLogList(buf);
}
