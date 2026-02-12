#include "Platform.h"
#include "GameCmdInvis.h"
#include "Game.h"
#include <cstring>


using namespace hb::shared::net;
using namespace hb::shared::action;

bool GameCmdInvis::Execute(CGame* pGame, int iClientH, const char* pArgs)
{
	if (pGame->m_pClientList[iClientH] == nullptr)
		return true;

	if (pGame->m_pClientList[iClientH]->m_bIsAdminInvisible)
	{
		// Toggle OFF
		pGame->m_pClientList[iClientH]->m_bIsAdminInvisible = false;

		// Broadcast full appearance to all nearby (re-appear)
		pGame->SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventMotion, Type::NullAction, 0, 0, 0);

		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Admin invisibility disabled.");
	}
	else
	{
		// Toggle ON — despawn from non-qualifying viewers BEFORE setting the flag,
		// otherwise the filtering in SendEventToNearClient_TypeA will skip the despawn packet
		pGame->SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventLog, MsgType::Reject, 0, 0, 0);

		pGame->m_pClientList[iClientH]->m_bIsAdminInvisible = true;

		// Now re-broadcast as a NULLACTION so higher-level admins see the invis+GM flagged version
		pGame->SendEventToNearClient_TypeA(iClientH, hb::shared::owner_class::Player, MsgId::EventMotion, Type::NullAction, 0, 0, 0);

		pGame->SendNotifyMsg(0, iClientH, Notify::NoticeMsg, 0, 0, 0, "Admin invisibility enabled.");
	}

	return true;
}
