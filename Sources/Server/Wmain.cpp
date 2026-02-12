// --------------------------------------------------------------
//                      New Game Server
//
//                      1998.11 by Soph
//
// --------------------------------------------------------------


// --------------------------------------------------------------

#include "Platform.h"
#include "CommonTypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cstring>
#include "winmain.h"
#include "Game.h"
#include "UserMessages.h"
#include "LoginServer.h"
#include "ServerConsole.h"
#include "ServerCommand.h"
#include "ChatLog.h"
#include "ItemLog.h"
#include "GameChatCommand.h"
#include "IOServicePool.h"
#include "ConcurrentMsgQueue.h"

#ifndef _WIN32
#include <signal.h>
#endif

using namespace hb::server::config;

void PutHackLogFileList(char* cStr);
void PutPvPLogFileList(char* cStr);

// --------------------------------------------------------------

#define WM_USER_TIMERSIGNAL		WM_USER + 500

char			szAppClass[32];
HWND			G_hWnd = 0;
char            G_cTxt[512];
char			G_cData50000[50000];
MMRESULT        G_mmTimer = 0;


class hb::shared::net::ASIOSocket* G_pListenSock = 0;
class hb::shared::net::ASIOSocket* G_pLogSock = 0;
class CGame* G_pGame = 0;
class hb::shared::net::ASIOSocket* G_pLoginSock = 0;
class LoginServer* g_login;

// Shared I/O service pool: 4 threads for async networking
hb::shared::net::IOServicePool* G_pIOPool = nullptr;

// Thread-safe queues for async accept
hb::shared::net::ConcurrentQueue<asio::ip::tcp::socket> G_gameAcceptQueue;
hb::shared::net::ConcurrentQueue<asio::ip::tcp::socket> G_loginAcceptQueue;

// Thread-safe error queue from I/O threads
hb::shared::net::ConcurrentQueue<hb::shared::net::SocketErrorEvent> G_errorQueue;

int             G_iQuitProgramCount = 0;
bool			G_bIsThread = true;

FILE* pLogFile;

// --------------------------------------------------------------

#ifndef _WIN32
static volatile sig_atomic_t g_shutdownRequested = 0;

static void SignalHandler(int sig)
{
	(void)sig;
	g_shutdownRequested = 1;
}
#endif

unsigned __stdcall ThreadProc(void* ch)
{
	class CTile* pTile;
	while (G_bIsThread)
	{
		Sleep(1000);

		for(int a = 0; a < MaxMaps; a++)
		{
			if (G_pGame->m_pMapList[a] != 0)
			{
				for(int iy = 0; iy < G_pGame->m_pMapList[a]->m_sSizeY; iy++)
				{
					for(int ix = 0; ix < G_pGame->m_pMapList[a]->m_sSizeX; ix++)
					{
						pTile = (class CTile*)(G_pGame->m_pMapList[a]->m_pTile + ix + iy * G_pGame->m_pMapList[a]->m_sSizeY);
						if ((pTile != 0) && (pTile->m_sOwner != 0) && (pTile->m_cOwnerClass == 0))
						{
							pTile->m_sOwner = 0;
						}
					}
				}
			}
		}
	}

	_endthread();
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_USER_STARTGAMESIGNAL:
		G_pGame->OnStartGameSignal();
		break;

	case WM_USER_TIMERSIGNAL:
		G_pGame->OnTimer(0);
		break;

	case WM_DESTROY:
		OnDestroy();
		break;

	case WM_CLOSE:
		if (G_pGame->bOnClose())
			return (DefWindowProc(hWnd, message, wParam, lParam));
		break;

	default:
		if ((message >= WM_USER_BOT_ACCEPT + 1) && (message <= WM_USER_BOT_ACCEPT + MaxClientLoginSock))
			G_pGame->OnSubLogSocketEvent(message, wParam, lParam);

		return (DefWindowProc(hWnd, message, wParam, lParam));
	}

	return 0;
}

int main()
{
#ifndef _WIN32
	signal(SIGINT, SignalHandler);
	signal(SIGTERM, SignalHandler);
#endif

	HINSTANCE hInstance = GetModuleHandle(0);
	int nCmdShow = SW_SHOW;
	sprintf(szAppClass, "GameServer%p", static_cast<void*>(hInstance));
	if (!InitApplication(hInstance))		return (false);
	if (!InitInstance(hInstance, nCmdShow)) return (false);

	Initialize();
	EventLoop();
	return 0;
}

#ifdef _WIN32
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return main();
}
#endif



bool InitApplication(HINSTANCE hInstance)
{
	WNDCLASS  wc;

	wc.style = (CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS);
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(int);
	wc.hInstance = hInstance;
	wc.hIcon = 0;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = 0;
	wc.lpszClassName = szAppClass;

	return (RegisterClass(&wc));
}


bool InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	char cTitle[100];
	SYSTEMTIME SysTime;

	GetLocalTime(&SysTime);
	std::snprintf(cTitle, sizeof(cTitle), "Helbreath GameServer V%s.%s %d (Executed at: %d %d %d)", hb::server::version::Upper, hb::server::version::Lower, hb::server::version::BuildDate, SysTime.wMonth, SysTime.wDay, SysTime.wHour);

	// Create message-only window (hidden) for socket events
	G_hWnd = CreateWindowEx(
		0,
		szAppClass,
		cTitle,
		WS_OVERLAPPED,        // Not visible
		0, 0, 0, 0,           // No size/position
		HWND_MESSAGE,         // Message-only window (no UI)
		0,
		hInstance,
		0);

	if (!G_hWnd) return (false);

	// Allocate console for output
	AllocConsole();
	SetConsoleTitle(cTitle);

#ifdef _WIN32
	// Redirect stdout/stderr to console (Windows only — Linux stdout already works)
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
#endif

	// Print startup banner
	printf("\n");
	printf("=======================================================================\n");
	printf("         HELBREATH GAME SERVER - CONSOLE MODE                         \n");
	printf("=======================================================================\n");
	printf("Version: %s.%s\n", hb::server::version::Upper, hb::server::version::Lower);
	printf("Build: %d\n", hb::server::version::BuildDate);
	printf("Started: %d/%d/%d %02d:%02d\n", SysTime.wMonth, SysTime.wDay, SysTime.wYear, SysTime.wHour, SysTime.wMinute);
	printf("=======================================================================\n\n");
	printf("Initializing server...\n\n");

	return (true);
}

// Drain async accept queues (called from game thread in EventLoop)
void DrainAcceptQueues()
{
	if (G_pGame == nullptr) return;

	// Drain game client accepts
	asio::ip::tcp::socket peer(G_pIOPool->GetContext());
	while (G_gameAcceptQueue.Pop(peer)) {
		G_pGame->bAcceptFromAsync(std::move(peer));
	}

	// Drain login client accepts
	asio::ip::tcp::socket loginPeer(G_pIOPool->GetContext());
	while (G_loginAcceptQueue.Pop(loginPeer)) {
		G_pGame->bAcceptLoginFromAsync(std::move(loginPeer));
	}
}

// Drain error queue from I/O threads (called from game thread in EventLoop)
void DrainErrorQueue()
{
	if (G_pGame == nullptr) return;

	hb::shared::net::SocketErrorEvent evt;
	while (G_errorQueue.Pop(evt)) {
		if (evt.iSocketIndex > 0 && evt.iSocketIndex < MaxClients) {
			if (G_pGame->m_pClientList[evt.iSocketIndex] != nullptr) {
				std::snprintf(G_cTxt, sizeof(G_cTxt),
					"<%d> Client Disconnected! Async error=%d (%s) CharName=%s",
					evt.iSocketIndex, evt.iErrorCode,
					G_pGame->m_pClientList[evt.iSocketIndex]->m_cIPaddress,
					G_pGame->m_pClientList[evt.iSocketIndex]->m_cCharName);
				PutLogList(G_cTxt);
				G_pGame->DeleteClient(evt.iSocketIndex, true, true);
			}
		}
		else if (evt.iSocketIndex < 0) {
			int loginH = -(evt.iSocketIndex + 1);
			if (loginH >= 0 && loginH < MaxClientLoginSock) {
				G_pGame->DeleteLoginClient(loginH);
			}
		}
	}
}

// Poll login client sockets (still uses polling for login clients)
void PollLoginClients()
{
	if (G_pGame == nullptr) return;

	for(int i = 0; i < MaxClientLoginSock; i++) {
		if (G_pGame->_lclients[i] != nullptr && G_pGame->_lclients[i]->_sock != nullptr) {
			G_pGame->OnLoginClientSocketEvent(i);
		}
	}
}

// Legacy PollAllSockets - called from EntityManager during NPC processing
void PollAllSockets()
{
	DrainAcceptQueues();
	DrainErrorQueue();
	PollLoginClients();
}

WPARAM EventLoop()
{
	static unsigned short _usCnt = 0;
	static uint32_t dwLastDebug = 0;
	static uint32_t dwLastGameProcess = 0;
	static uint32_t dwLastTimerTick = 0;
	MSG msg;

	while (true) {
#ifndef _WIN32
		if (g_shutdownRequested) {
			OnDestroy();
			return 0;
		}
#endif
		if (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE)) {
			if (!GetMessage(&msg, 0, 0, 0)) {
				return msg.wParam;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			if (G_pGame != nullptr) {
				uint32_t dwNow = GameClock::GetTimeMS();

				if (dwNow - dwLastGameProcess >= (300/ hb::server::config::GameTickMultiplier)) {
					G_pGame->GameProcess();
					dwLastGameProcess = dwNow;
				}

#ifndef _WIN32
				// On Linux, the multimedia timer doesn't exist.
				// Call OnTimer at the same interval to handle game startup and periodic events.
				if (dwNow - dwLastTimerTick >= (300 / hb::server::config::GameTickMultiplier)) {
					G_pGame->OnTimer(0);
					dwLastTimerTick = dwNow;
				}
#endif

				DrainAcceptQueues();
				DrainErrorQueue();
				PollLoginClients();

				char szCmd[256];
				if (GetServerConsole().PollInput(szCmd, sizeof(szCmd))) {
					ServerCommandManager::Get().ProcessCommand(szCmd);
				}

				if (dwNow - dwLastDebug > 60000) {
					int activeClients = 0;
					int activeLoginClients = 0;
					for(int i = 1; i < MaxClients; i++) {
						if (G_pGame->m_pClientList[i] != nullptr) activeClients++;
					}
					for(int i = 0; i < MaxClientLoginSock; i++) {
						if (G_pGame->_lclients[i] != nullptr) activeLoginClients++;
					}
					dwLastDebug = dwNow;
				}
			}
			Sleep(1);
		}
	}
}



void Initialize()
{

	// Create shared I/O service pool before anything that creates sockets
	G_pIOPool = new hb::shared::net::IOServicePool(4);

	G_pGame = new class CGame(G_hWnd);
	if (G_pGame->bInit() == false) {
		PutLogList("(!!!) STOPPED!");
		return;
	}

	ServerCommandManager::Get().Initialize(G_pGame);
	ChatLog::Get().Initialize();
	ItemLog::Get().Initialize();
	GameChatCommandManager::Get().Initialize(G_pGame);
	GetServerConsole().Init();

	G_mmTimer = _StartTimer((300 / hb::server::config::GameTickMultiplier));

	G_pListenSock = new class hb::shared::net::ASIOSocket(G_pIOPool->GetContext(), ServerSocketBlockLimit);
	G_pListenSock->bListen(G_pGame->m_cGameListenIP, G_pGame->m_iGameListenPort);

	G_pLoginSock = new class hb::shared::net::ASIOSocket(G_pIOPool->GetContext(), ServerSocketBlockLimit);
	G_pLoginSock->bListen(G_pGame->m_cLoginListenIP, G_pGame->m_iLoginListenPort);

	// Start async accept on both listen sockets
	G_pListenSock->StartAsyncAccept([](asio::ip::tcp::socket peer) {
		G_gameAcceptQueue.Push(std::move(peer));
	});
	G_pLoginSock->StartAsyncAccept([](asio::ip::tcp::socket peer) {
		G_loginAcceptQueue.Push(std::move(peer));
	});

	// Start the I/O thread pool
	G_pIOPool->Start();

	pLogFile = 0;
}

void OnDestroy()
{
	// Stop I/O pool first (cancels async ops, joins threads)
	if (G_pIOPool != nullptr) {
		G_pIOPool->Stop();
	}

	if (G_pListenSock != 0) delete G_pListenSock;
	if (G_pLogSock != 0) delete G_pLogSock;
	if (G_pLoginSock) delete G_pLoginSock;

	if (G_pGame != 0) {
		G_pGame->Quit();
		delete G_pGame;
	}

	if (g_login)
	{
		delete g_login;
		g_login = 0;
	}

	if (G_mmTimer != 0) _StopTimer(G_mmTimer);

	if (pLogFile != 0) fclose(pLogFile);

	if (G_pIOPool != nullptr) {
		delete G_pIOPool;
		G_pIOPool = nullptr;
	}

	PostQuitMessage(0);
}

void OnAcceptLogin()
{
	G_pGame->bAcceptLogin(G_pLoginSock);
}

namespace
{
	const char* GetLevelName(int level)
	{
		switch (level) {
		case LOG_LEVEL_NOTICE: return "NOTICE";
		case LOG_LEVEL_WARNING: return "WARNING";
		case LOG_LEVEL_ERROR: return "ERROR";
		default: return "INFO";
		}
	}

	WORD GetLevelColor(int level)
	{
		switch (level) {
		case LOG_LEVEL_NOTICE: return FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
		case LOG_LEVEL_WARNING: return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
		case LOG_LEVEL_ERROR: return FOREGROUND_RED | FOREGROUND_INTENSITY;
		default: return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
		}
	}

	const char* TrimPrefix(const char* msg, int* outLevel)
	{
		if (msg == nullptr) {
			return msg;
		}

		const char* p = msg;
		while (*p == ' ' || *p == '\t') {
			++p;
		}

		if (std::strncmp(p, "(!!!)", 5) == 0) {
			*outLevel = LOG_LEVEL_ERROR;
			p += 5;
		}
		else if (std::strncmp(p, "(XXX)", 5) == 0) {
			*outLevel = LOG_LEVEL_ERROR;
			p += 5;
		}
		else if (std::strncmp(p, "(X)", 3) == 0) {
			*outLevel = LOG_LEVEL_ERROR;
			p += 3;
		}
		else if (std::strncmp(p, "(!)", 3) == 0) {
			*outLevel = LOG_LEVEL_NOTICE;
			p += 3;
		}
		else if (std::strncmp(p, "(*)", 3) == 0) {
			*outLevel = LOG_LEVEL_INFO;
			p += 3;
		}
		else if (std::strncmp(p, "[ERROR]", 7) == 0) {
			*outLevel = LOG_LEVEL_ERROR;
			p += 7;
		}
		else if (std::strncmp(p, "[WARNING]", 9) == 0) {
			*outLevel = LOG_LEVEL_WARNING;
			p += 9;
		}
		else if (std::strncmp(p, "[NOTICE]", 8) == 0) {
			*outLevel = LOG_LEVEL_NOTICE;
			p += 8;
		}
		else if (std::strncmp(p, "[INFO]", 6) == 0) {
			*outLevel = LOG_LEVEL_INFO;
			p += 6;
		}

		while (*p == ' ' || *p == '\t') {
			++p;
		}
		if (*outLevel == LOG_LEVEL_INFO) {
			if (std::strstr(p, "CRITICAL ERROR") != nullptr) {
				*outLevel = LOG_LEVEL_ERROR;
			}
			else if (std::strstr(p, "WARNING") != nullptr) {
				*outLevel = LOG_LEVEL_WARNING;
			}
		}
		return p;
	}

	void WriteServerLogLine(const char* line)
	{
		if (line == nullptr) {
			return;
		}
		CreateDirectoryA("GameLogs", nullptr);
		FILE* file = fopen("GameLogs/server.log", "at");
		if (file == nullptr) {
			return;
		}
		fwrite(line, 1, std::strlen(line), file);
		fwrite("\n", 1, 1, file);
		fclose(file);
	}

	void LogWrite(int level, const char* message, bool writeConsole)
	{
		if (message == nullptr) {
			return;
		}

		const char* trimmed = message;
		while (*trimmed == ' ' || *trimmed == '\t') {
			++trimmed;
		}
		if (*trimmed == '\0') {
			return;
		}
		bool onlyDots = true;
		for (const char* p = trimmed; *p != '\0'; ++p) {
			if (*p != '.' && *p != ' ') {
				onlyDots = false;
				break;
			}
		}
		if (onlyDots) {
			return;
		}

		SYSTEMTIME st;
		GetLocalTime(&st);

		char line[1024] = {};
		std::snprintf(line, sizeof(line), "[%02d:%02d:%02d] [%s] %s",
			st.wHour, st.wMinute, st.wSecond, GetLevelName(level), trimmed);

		if (writeConsole) {
			GetServerConsole().WriteLine(line, GetLevelColor(level));
		}

		WriteServerLogLine(line);
	}
}

void PutLogListLevel(int level, const char* cMsg)
{
	if (cMsg == nullptr) {
		return;
	}
	LogWrite(level, cMsg, true);
}

void PutLogList(char* cMsg)
{
	int level = LOG_LEVEL_INFO;
	const char* msg = TrimPrefix(cMsg, &level);
	LogWrite(level, msg, true);
}

void PutXSocketLogList(char* cMsg)
{
	PutXSocketLogFileList(cMsg);
}

void  OnKeyUp(WPARAM wParam, LPARAM lParam)
{
}


void OnAccept()
{
	G_pGame->bAccept(G_pListenSock);
}

void CALLBACK _TimerFunc(UINT wID, UINT wUser, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	PostMessage(G_hWnd, WM_USER_TIMERSIGNAL, wID, 0);
}



MMRESULT _StartTimer(uint32_t dwTime)
{
	TIMECAPS caps;
	MMRESULT timerid;

	timeGetDevCaps(&caps, sizeof(caps));
	timeBeginPeriod(caps.wPeriodMin);
	timerid = timeSetEvent(dwTime, 0, _TimerFunc, 0, (UINT)TIME_PERIODIC);

	return timerid;
}



void _StopTimer(MMRESULT timerid)
{
	TIMECAPS caps;

	if (timerid != 0) {
		timeKillEvent(timerid);
		timerid = 0;
		timeGetDevCaps(&caps, sizeof(caps));
		timeEndPeriod(caps.wPeriodMin);
	}
}

/*********************************************************************************************************************
**  void PutLogFileList(char * cStr)																				**
**  description			:: writes data into "Events.log"															**
**  last updated		:: November 22, 2004; 5:40 PM; Hypnotoad													**
**	return value		:: void																						**
**********************************************************************************************************************/
void PutLogFileList(char* cStr)
{
	FILE* pFile;
	char cBuffer[512];
	SYSTEMTIME SysTime;

	pFile = fopen("GameLogs/Events.log", "at");
	if (pFile == 0) return;
	std::memset(cBuffer, 0, sizeof(cBuffer));
	GetLocalTime(&SysTime);
	std::snprintf(cBuffer, sizeof(cBuffer), "(%4d:%2d:%2d:%2d:%2d) - ", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute);
	strcat(cBuffer, cStr);
	strcat(cBuffer, "\n");
	fwrite(cBuffer, 1, strlen(cBuffer), pFile);
	fclose(pFile);
}

void PutHackLogFileList(char* cStr)
{
	FILE* pFile;
	char cBuffer[512];
	SYSTEMTIME SysTime;

	pFile = fopen("GameLogs/HackEvents.log", "at");
	if (pFile == 0) return;

	std::memset(cBuffer, 0, sizeof(cBuffer));

	GetLocalTime(&SysTime);
	std::snprintf(cBuffer, sizeof(cBuffer), "(%4d:%2d:%2d:%2d:%2d) - ", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute);
	strcat(cBuffer, cStr);
	strcat(cBuffer, "\n");

	fwrite(cBuffer, 1, strlen(cBuffer), pFile);
	fclose(pFile);
}

void PutPvPLogFileList(char* cStr)
{
	FILE* pFile;
	char cBuffer[512];
	SYSTEMTIME SysTime;

	pFile = fopen("GameLogs/PvPEvents.log", "at");
	if (pFile == 0) return;

	std::memset(cBuffer, 0, sizeof(cBuffer));

	GetLocalTime(&SysTime);
	std::snprintf(cBuffer, sizeof(cBuffer), "(%4d:%2d:%2d:%2d:%2d) - ", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute);
	strcat(cBuffer, cStr);
	strcat(cBuffer, "\n");

	fwrite(cBuffer, 1, strlen(cBuffer), pFile);
	fclose(pFile);
}

void PutXSocketLogFileList(char* cStr)
{
	FILE* pFile;
	char cBuffer[512];
	SYSTEMTIME SysTime;

	pFile = fopen("GameLogs/XSocket.log", "at");
	if (pFile == 0) return;

	std::memset(cBuffer, 0, sizeof(cBuffer));

	GetLocalTime(&SysTime);
	std::snprintf(cBuffer, sizeof(cBuffer), "(%4d:%2d:%2d:%2d:%2d) - ", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute);
	strcat(cBuffer, cStr);
	strcat(cBuffer, "\n");

	fwrite(cBuffer, 1, strlen(cBuffer), pFile);
	fclose(pFile);
}

void PutLogEventFileList(char* cStr)
{
	FILE* pFile;
	char cBuffer[512];
	SYSTEMTIME SysTime;

	pFile = fopen("GameLogs/LogEvents.log", "at");
	if (pFile == 0) return;

	std::memset(cBuffer, 0, sizeof(cBuffer));

	GetLocalTime(&SysTime);
	std::snprintf(cBuffer, sizeof(cBuffer), "(%4d:%2d:%2d:%2d:%2d) - ", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute);
	strcat(cBuffer, cStr);
	strcat(cBuffer, "\n");

	fwrite(cBuffer, 1, strlen(cBuffer), pFile);
	fclose(pFile);
}
