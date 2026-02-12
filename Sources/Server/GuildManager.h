// GuildManager.h: Handles server-side guild operations.
// Extracted from Game.cpp (Phase B4).

#pragma once

#include <cstdint>
#include <cstddef>

class CGame;

class GuildManager
{
public:
	GuildManager() = default;
	~GuildManager() = default;

	void SetGame(CGame* pGame) { m_pGame = pGame; }

	// Guild creation/disband (network handlers)
	void RequestCreateNewGuildHandler(int iClientH, char* pData, size_t dwMsgSize);
	void ResponseCreateNewGuildHandler(char* pData, int iType);
	void RequestDisbandGuildHandler(int iClientH, char* pData, size_t dwMsgSize);
	void ResponseDisbandGuildHandler(char* pData, int iType);

	// Guild membership
	void JoinGuildApproveHandler(int iClientH, const char* pName);
	void JoinGuildRejectHandler(int iClientH, const char* pName);
	void DismissGuildApproveHandler(int iClientH, const char* pName);
	void DismissGuildRejectHandler(int iClientH, const char* pName);

	// Guild messaging
	void SendGuildMsg(int iClientH, uint16_t wNotifyMsgType, short sV1, short sV2, char* pString);
	void GuildNotifyHandler(char* pData, size_t dwMsgSize);

	// Guild user commands
	void UserCommand_BanGuildsman(int iClientH, char* pData, size_t dwMsgSize);
	void UserCommand_DissmissGuild(int iClientH, char* pData, size_t dwMsgSize);

	// File-based guild management
	void RequestCreateNewGuild(int iClientH, char* pData);
	void RequestDisbandGuild(int iClientH, char* pData);

	// Guild info
	void RequestGuildNameHandler(int iClientH, int iObjectID, int iIndex);

private:
	CGame* m_pGame = nullptr;
};
