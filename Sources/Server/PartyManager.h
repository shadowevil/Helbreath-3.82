// PartyManager.h: interface for the PartyManager class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)

#include "Platform.h"
#include "CommonTypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "Game.h"

namespace hb::server::party { constexpr int MaxParty = 5000; constexpr int MaxPartyMember = 100; }


class PartyManager
{
public:
	void CheckMemberActivity();
	void SetServerChangeStatus(char* pName, int iPartyID);
	void GameServerDown();
	bool bGetPartyInfo(int iGSCH, char* pName, int iPartyID);
	bool bCheckPartyMember(int iGSCH, int iPartyID, char* pName);
	bool bRemoveMember(int iPartyID, char* pMemberName);
	bool bAddMember(int iPartyID, char* pMemberName);
	bool bDeleteParty(int iPartyID);
	int iCreateNewParty(char* pMasterName);
	PartyManager(class CGame* pGame);
	virtual ~PartyManager();

	int m_iMemberNumList[hb::server::party::MaxParty];

	struct {
		int  m_iPartyID, m_iIndex;
		char m_cName[hb::shared::limits::CharNameLen];
		uint32_t m_dwServerChangeTime;
	} m_stMemberNameList[hb::server::party::MaxParty];

	class CGame* m_pGame;
	uint32_t m_dwCheckMemberActTime;
};
