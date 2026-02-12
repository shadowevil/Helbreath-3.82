#include "Platform.h"
#include "GameCmdGM.h"
#include "Game.h"
#include <cstring>


using namespace hb::shared::net;
using namespace hb::shared::action;

bool GameCmdGM::Execute(CGame* pGame, int iClientH, const char* pArgs)
{
	if (pGame->m_pClientList[iClientH] == nullptr)
		return true;

	if (pArgs == nullptr || pArgs[0] == '\0')
	{
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Usage: /gm on | /gm off");
		return true;
	}

	if (_stricmp(pArgs, "on") == 0)
	{
		pGame->m_pClientList[iClientH]->m_bIsGMMode = true;
		pGame->m_pClientList[iClientH]->m_status.bGMMode = true;
		pGame->SendEventToNearClient_TypeA(static_cast<short>(iClientH), hb::shared::owner_class::Player, MsgId::EventMotion, Type::NullAction, 0, 0, 0);
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "GM mode enabled.");
		return true;
	}
	else if (_stricmp(pArgs, "off") == 0)
	{
		pGame->m_pClientList[iClientH]->m_bIsGMMode = false;
		pGame->m_pClientList[iClientH]->m_status.bGMMode = false;
		pGame->SendEventToNearClient_TypeA(static_cast<short>(iClientH), hb::shared::owner_class::Player, MsgId::EventMotion, Type::NullAction, 0, 0, 0);
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "GM mode disabled.");
		return true;
	}
	else
	{
		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Usage: /gm on | /gm off");
		return true;
	}
}
