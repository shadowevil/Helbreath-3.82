#include "Platform.h"
#include "CmdShowChat.h"
#include "winmain.h"
#include <cstdio>

#ifdef _WIN32

void CmdShowChat::Execute(CGame* pGame, const char* pArgs)
{
	if (m_hProcess != nullptr)
	{
		DWORD dwExitCode = 0;
		if (GetExitCodeProcess(m_hProcess, &dwExitCode) && dwExitCode == STILL_ACTIVE)
		{
			PutLogList((char*)"(!) Chat viewer is already open.");
			return;
		}
		CloseHandle(m_hProcess);
		m_hProcess = nullptr;
	}

	char szLogPath[MAX_PATH];
	GetFullPathNameA("GameLogs/Chat.log", MAX_PATH, szLogPath, nullptr);

	char szCmdLine[1024];
	std::snprintf(szCmdLine, sizeof(szCmdLine),
		"cmd.exe /c \"title HB Chat && powershell -NoProfile -ExecutionPolicy Bypass -Command \"\"Get-Content '%s' -Wait -Tail 0\"\"\"",
		szLogPath);

	STARTUPINFOA si = {};
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi = {};

	BOOL bResult = CreateProcessA(
		nullptr,
		szCmdLine,
		nullptr,
		nullptr,
		FALSE,
		CREATE_NEW_CONSOLE,
		nullptr,
		nullptr,
		&si,
		&pi
	);

	if (bResult)
	{
		CloseHandle(pi.hThread);
		m_hProcess = pi.hProcess;
		PutLogList((char*)"(!) Chat viewer opened in new terminal.");
	}
	else
	{
		char szError[256];
		std::snprintf(szError, sizeof(szError), "(!) Failed to open chat viewer (error %lu).", GetLastError());
		PutLogList(szError);
	}
}

#else

#include <sys/wait.h>

void CmdShowChat::Execute(CGame* pGame, const char* pArgs)
{
	if (m_pid > 0)
	{
		int status = 0;
		pid_t result = waitpid(m_pid, &status, WNOHANG);
		if (result == 0)
		{
			PutLogList((char*)"(!) Chat viewer is already open.");
			return;
		}
		m_pid = -1;
	}

	pid_t pid = fork();
	if (pid < 0)
	{
		PutLogList((char*)"(!) Failed to fork chat viewer process.");
		return;
	}

	if (pid == 0)
	{
		// Child process — exec tail -f
		execlp("tail", "tail", "-f", "GameLogs/Chat.log", (char*)nullptr);
		_exit(127);
	}

	m_pid = pid;
	PutLogList((char*)"(!) Chat viewer started (tail -f GameLogs/Chat.log).");
}

#endif
