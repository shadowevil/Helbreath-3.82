#pragma once
// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)

#include "Platform.h"
#include "CommonTypes.h"
#include <iostream>
#include <vector>
using namespace std;

#include "Game.h"
#include "AccountSqliteStore.h"

enum class LogIn
{
	Ok,
	NoAcc,
	NoPass,
};

class LoginServer
{
public:
	LoginServer();
	~LoginServer();

	void RequestLogin(int h, char* pData);
	void GetCharList(string acc, char*& cp2, const std::vector<AccountDbCharacterSummary>& chars);
	LogIn AccountLogIn(string name, string pass, std::vector<AccountDbCharacterSummary>& chars);
	void ResponseCharacter(int h, char* pData);
	void DeleteCharacter(int h, char* pData);
	void ChangePassword(int h, char* pData);
	void RequestEnterGame(int h, char* pData);
	void CreateNewAccount(int h, char* pData);
	void SendLoginMsg(uint32_t msgid, uint16_t msgtype, char* data, size_t sz, int h);
	void LocalSavePlayerData(int h);
	void Activated();
};

extern LoginServer* g_login;
