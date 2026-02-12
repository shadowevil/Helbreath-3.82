#pragma once

#include "Platform.h"

class ServerConsole
{
public:
	ServerConsole();
	~ServerConsole();
	bool Init();
	void WriteLine(const char* text, WORD color);
	bool PollInput(char* pOutCmd, int maxLen);
	void RedrawPrompt();

private:
	void ClearInputLine();
	void DrawInputLine();

#ifdef _WIN32
	SHORT GetPromptRow();
	HANDLE m_hOut;
	HANDLE m_hIn;
	DWORD m_dwOrigMode;
	SHORT m_sWidth;
	SHORT m_sHeight;
#else
	int m_iWidth;
	int m_iHeight;
	bool m_bRawMode;
#endif
	char m_szInput[256];
	int m_iInputLen;
	int m_iCursorPos;
	bool m_bInit;
};

ServerConsole& GetServerConsole();
