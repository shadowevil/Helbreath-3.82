// -------------------------------------------------------------- 
//                      New Game Server  						  
//
//                      1998.11 by Soph
//
// --------------------------------------------------------------







// --------------------------------------------------------------


#ifdef _WIN32
// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)
#define _WINSOCKAPI_

#include <windows.h>
#include "CommonTypes.h"
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <winbase.h>
#include <mmsystem.h>
#include <time.h>
#include <cstring>
#include <conio.h>
#include "winmain.h"
#include "Game.h"
#include "UserMessages.h"
// #include "resource.h" - REMOVED: No resources needed for console mode
#include "LoginServer.h"

void PutAdminLogFileList(char* cStr);
void PutHackLogFileList(char* cStr);
void PutPvPLogFileList(char* cStr);

// --------------------------------------------------------------

#define WM_USER_TIMERSIGNAL		WM_USER + 500

char			szAppClass[32];
HWND			G_hWnd = 0;
// G_cMsgList - REMOVED: No longer needed for console output
// G_cMsgUpdated - REMOVED: No longer needed for console output
char            G_cTxt[512];
char			G_cData50000[50000];
MMRESULT        G_mmTimer = 0;


class XSocket* G_pListenSock = 0;
class XSocket* G_pLogSock = 0;
class CGame* G_pGame = 0;
class XSocket* G_pLoginSock = 0;
class LoginServer* g_login;

int             G_iQuitProgramCount = 0;
bool			G_bIsThread = true;

FILE* pLogFile;

//char			G_cCrashTxt[50000];
// --------------------------------------------------------------

unsigned __stdcall ThreadProc(void* ch)
{
	class CTile* pTile;
	while (G_bIsThread)
	{
		Sleep(1000);

		for (int a = 0; a < DEF_MAXMAPS; a++)
		{
			if (G_pGame->m_pMapList[a] != 0)
			{
				for (int iy = 0; iy < G_pGame->m_pMapList[a]->m_sSizeY; iy++)
				{
					for (int ix = 0; ix < G_pGame->m_pMapList[a]->m_sSizeX; ix++)
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
	// WM_CREATE - REMOVED: No initialization needed
	// WM_KEYDOWN - REMOVED: No keyboard input for console mode
	// WM_KEYUP - REMOVED: No keyboard input for console mode
	// WM_PAINT - REMOVED: No GDI rendering for console mode

	case WM_USER_STARTGAMESIGNAL:
		G_pGame->OnStartGameSignal();
		break;

	case WM_USER_TIMERSIGNAL:
		// MODERNIZED: Game logic moved to EventLoop to prevent blocking socket polling
		// OnTimer still called for other timer-based events (if any)
		G_pGame->OnTimer(0);
		break;

	// MODERNIZED: Removed WM_USER_ACCEPT and WM_USER_ACCEPT_LOGIN handlers
	// Listen sockets are now polled directly in OnTimer() instead of using window messages

	case WM_DESTROY:
		OnDestroy();
		break;

	case WM_CLOSE:
		// Simple shutdown without MessageBox
		if (G_pGame->bOnClose())
			return (DefWindowProc(hWnd, message, wParam, lParam));
		break;

	case WM_ONGATESOCKETEVENT:
		G_pGame->OnGateSocketEvent(message, wParam, lParam);
		break;

	case WM_ONLOGSOCKETEVENT:
		G_pGame->OnMainLogSocketEvent(message, wParam, lParam);
		break;

	default:
		// Handle sub log socket events
		if ((message >= WM_USER_BOT_ACCEPT + 1) && (message <= WM_USER_BOT_ACCEPT + DEF_MAXCLIENTLOGINSOCK))
			G_pGame->OnSubLogSocketEvent(message, wParam, lParam);

		// MODERNIZED: Removed WM_ONCLIENTSOCKETEVENT handler
		// Client sockets are now polled directly in GameProcess() instead of using window messages

		return (DefWindowProc(hWnd, message, wParam, lParam));
	}

	return 0;
}

/*void GetOSName(){
	OSVERSIONINFOEX osvi;
	bool bOsVersionInfoEx;

	std::memset(&osvi, 0, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
	{
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) )
			return;
	}

	//OS Info
	strcat(G_cCrashTxt, "System Information\r\n");
	strcat(G_cCrashTxt, "Operating System : ");

	switch (osvi.dwPlatformId)
	{
		// Test for the Windows NT product family.
	case VER_PLATFORM_WIN32_NT:

		// Test for the specific product family.
		if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
			strcat(G_cCrashTxt,"Microsoft Windows Server 2003 family, ");

		if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
			strcat(G_cCrashTxt,"Microsoft Windows XP ");

		if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
			strcat(G_cCrashTxt,"Microsoft Windows 2000 ");

		if ( osvi.dwMajorVersion <= 4 )
			strcat(G_cCrashTxt,"Microsoft Windows NT ");

		// Test for specific product on Windows NT 4.0 SP6 and later.
		if( bOsVersionInfoEx )
		{
			// Test for the workstation type.
			if ( osvi.wProductType == VER_NT_WORKSTATION )
			{
				if( osvi.dwMajorVersion == 4 )
					strcat(G_cCrashTxt, "Workstation 4.0 " );
				else if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
					strcat(G_cCrashTxt, "Home Edition " );
				else
					strcat(G_cCrashTxt, "Professional " );
			}

			// Test for the server type.
			else if ( osvi.wProductType == VER_NT_SERVER ||
				osvi.wProductType == VER_NT_DOMAIN_CONTROLLER )
			{
				if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
				{
					if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
						strcat(G_cCrashTxt, "Datacenter Edition " );
					else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
						strcat(G_cCrashTxt, "Enterprise Edition " );
					else if ( osvi.wSuiteMask == VER_SUITE_BLADE )
						strcat(G_cCrashTxt, "Web Edition " );
					else
						strcat(G_cCrashTxt, "Standard Edition " );
				}

				else if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
				{
					if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
						strcat(G_cCrashTxt, "Datacenter Server " );
					else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
						strcat(G_cCrashTxt, "Advanced Server " );
					else
						strcat(G_cCrashTxt, "Server " );
				}

				else  // Windows NT 4.0
				{
					if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
						strcat(G_cCrashTxt,"Server 4.0, Enterprise Edition " );
					else
						strcat(G_cCrashTxt, "Server 4.0 " );
				}
			}
		}
		else  // Test for specific product on Windows NT 4.0 SP5 and earlier
		{
			#define BUFSIZE 80
			HKEY hKey;
			char szProductType[BUFSIZE];
			uint32_t dwBufLen=BUFSIZE;
			LONG lRet;

			lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
				"SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
				0, KEY_QUERY_VALUE, &hKey );
			if( lRet != ERROR_SUCCESS )
				return;

			lRet = RegQueryValueEx( hKey, "ProductType", 0, 0,
				(LPBYTE) szProductType, &dwBufLen);
			if( (lRet != ERROR_SUCCESS) || (dwBufLen > BUFSIZE) )
				return;

			RegCloseKey( hKey );

			if ( lstrcmpi( "WINNT", szProductType) == 0 )
				printf( "Workstation " );
			if ( lstrcmpi( "LANMANNT", szProductType) == 0 )
				printf( "Server " );
			if ( lstrcmpi( "SERVERNT", szProductType) == 0 )
				printf( "Advanced Server " );

			std::snprintf(&G_cCrashTxt[strlen(G_cCrashTxt)],
				sizeof(G_cCrashTxt) - strlen(G_cCrashTxt), "%d.%d ", osvi.dwMajorVersion, osvi.dwMinorVersion);
		}

		// Display service pack (if any) and build number.

		if( osvi.dwMajorVersion == 4 &&
			lstrcmpi( osvi.szCSDVersion, "Service Pack 6" ) == 0 )
		{
			HKEY hKey;
			LONG lRet;

			// Test for SP6 versus SP6a.
			lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
				"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q246009",
				0, KEY_QUERY_VALUE, &hKey );
			if( lRet == ERROR_SUCCESS )
				std::snprintf(&G_cCrashTxt[strlen(G_cCrashTxt)],
					sizeof(G_cCrashTxt) - strlen(G_cCrashTxt),
					"Service Pack 6a (Build %d)\r\n", osvi.dwBuildNumber & 0xFFFF);
			else // Windows NT 4.0 prior to SP6a
			{
				std::snprintf(&G_cCrashTxt[strlen(G_cCrashTxt)],
					sizeof(G_cCrashTxt) - strlen(G_cCrashTxt), "%s (Build %d)\r\n",
					osvi.szCSDVersion,
					osvi.dwBuildNumber & 0xFFFF);
			}

			RegCloseKey( hKey );
		}
		else // Windows NT 3.51 and earlier or Windows 2000 and later
		{
			std::snprintf(&G_cCrashTxt[strlen(G_cCrashTxt)],
				sizeof(G_cCrashTxt) - strlen(G_cCrashTxt), "%s (Build %d)\r\n",
				osvi.szCSDVersion,
				osvi.dwBuildNumber & 0xFFFF);
		}


		break;

		// Test for the Windows 95 product family.
	case VER_PLATFORM_WIN32_WINDOWS:

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
		{
			strcat(G_cCrashTxt,"Microsoft Windows 95 ");
			if ( osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B' )
				strcat(G_cCrashTxt,"OSR2 " );
		}

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
		{
			strcat(G_cCrashTxt,"Microsoft Windows 98 ");
			if ( osvi.szCSDVersion[1] == 'A' )
				strcat(G_cCrashTxt,"SE " );
		}

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
		{
			strcat(G_cCrashTxt,"Microsoft Windows Millennium Edition\r\n");
		}
		break;

	case VER_PLATFORM_WIN32s:

		strcat(G_cCrashTxt,"Microsoft Win32s\r\n");
		break;
	}
}*/

/*bool CALLBACK lpCrashDialogFunc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam){
HANDLE outHand;
char cCrashFileName[MAX_PATH];
char cLF[]={0x0d,0x0a};
char cDash ='-';
SYSTEMTIME sysTime;
uint32_t written;

	switch(uMsg) {
	case WM_CLOSE:
		EndDialog(hDlg, true);
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_CLOSE:
			EndDialog(hDlg, true);
			break;
		}
		break;

	case WM_INITDIALOG:
		//Show Crash Data
		SetWindowText(GetDlgItem(hDlg, IDC_EDIT1), G_cCrashTxt);
		GetLocalTime(&sysTime);
		std::snprintf(cCrashFileName, sizeof(cCrashFileName),"CrashData - %d-%d-%d.txt", sysTime.wDay, sysTime.wMonth, sysTime.wYear);
		SetWindowText(GetDlgItem(hDlg, IDC_EDITPATH), cCrashFileName);
		//Open File For Writing
		outHand = CreateFile(cCrashFileName,GENERIC_READ+GENERIC_WRITE,FILE_SHARE_READ+FILE_SHARE_WRITE,0,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
		SetFilePointer(outHand, 0, 0, FILE_END);
		WriteFile(outHand, G_cCrashTxt, strlen(G_cCrashTxt), &written, 0);
		for (int i = 0; i < 80; i++)
			WriteFile(outHand, &cDash, 1, &written, 0);
		WriteFile(outHand, cLF, 2, &written, 0);
		WriteFile(outHand, cLF, 2, &written, 0);
		CloseHandle(outHand);
		break;
//	default:
//		return 0;
	}
	return 0;
}*/

/*LONG lpTopLevelExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo){

	//Shutdown everything
	G_bIsThread = false;
	_StopTimer(G_mmTimer);

	try{
		delete G_pGame;
		G_pGame = 0;
	}
	catch (...) {
	}

	std::memset(G_cCrashTxt, 0, sizeof(G_cCrashTxt));

	//Format a nice output

	//Reason for crash
	strcpy(G_cCrashTxt, "HGServer Exception Information\r\n");
	strcat(G_cCrashTxt, "Code : ");
	std::snprintf(&G_cCrashTxt[strlen(G_cCrashTxt)],
		sizeof(G_cCrashTxt) - strlen(G_cCrashTxt), "0x%.8X\r\n", ExceptionInfo->ExceptionRecord->ExceptionCode);
	strcat(G_cCrashTxt, "Flags : ");
	std::snprintf(&G_cCrashTxt[strlen(G_cCrashTxt)],
		sizeof(G_cCrashTxt) - strlen(G_cCrashTxt), "0x%.8X\r\n", ExceptionInfo->ExceptionRecord->ExceptionFlags);
	strcat(G_cCrashTxt, "Address : ");
	std::snprintf(&G_cCrashTxt[strlen(G_cCrashTxt)],
		sizeof(G_cCrashTxt) - strlen(G_cCrashTxt), "0x%.8X\r\n", ExceptionInfo->ExceptionRecord->ExceptionAddress);
	strcat(G_cCrashTxt, "Parameters : ");
	std::snprintf(&G_cCrashTxt[strlen(G_cCrashTxt)],
		sizeof(G_cCrashTxt) - strlen(G_cCrashTxt), "0x%.8X\r\n\r\n", ExceptionInfo->ExceptionRecord->NumberParameters);

	//Retrieve OS version
	GetOSName();
	strcat(G_cCrashTxt, "\r\n");

	//Crash Details
	strcat(G_cCrashTxt, "Context :\r\n");
	std::snprintf(&G_cCrashTxt[strlen(G_cCrashTxt)],
		sizeof(G_cCrashTxt) - strlen(G_cCrashTxt),
		"EDI: 0x%.8X\t\tESI: 0x%.8X\t\tEAX: 0x%.8X\r\n",ExceptionInfo->ContextRecord->Edi,
																						ExceptionInfo->ContextRecord->Esi,
																						ExceptionInfo->ContextRecord->Eax);
	std::snprintf(&G_cCrashTxt[strlen(G_cCrashTxt)],
		sizeof(G_cCrashTxt) - strlen(G_cCrashTxt),
		"EBX: 0x%.8X\t\tECX: 0x%.8X\t\tEDX: 0x%.8X\r\n",ExceptionInfo->ContextRecord->Ebx,
																						ExceptionInfo->ContextRecord->Ecx,
																						ExceptionInfo->ContextRecord->Edx);
	std::snprintf(&G_cCrashTxt[strlen(G_cCrashTxt)],
		sizeof(G_cCrashTxt) - strlen(G_cCrashTxt),
		"EIP: 0x%.8X\t\tEBP: 0x%.8X\t\tSegCs: 0x%.8X\r\n",ExceptionInfo->ContextRecord->Eip,
																						ExceptionInfo->ContextRecord->Ebp,
																						ExceptionInfo->ContextRecord->SegCs);
	std::snprintf(&G_cCrashTxt[strlen(G_cCrashTxt)],
		sizeof(G_cCrashTxt) - strlen(G_cCrashTxt),
		"EFlags: 0x%.8X\tESP: 0x%.8X\t\tSegSs: 0x%.8X\r\n",ExceptionInfo->ContextRecord->EFlags,
																						ExceptionInfo->ContextRecord->Esp,
																						ExceptionInfo->ContextRecord->SegSs);
	// Show Dialog
	DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_DIALOG2), 0, (DLGPROC)lpCrashDialogFunc);
	SendMessage(0, WM_CLOSE, 0, 0);
	return EXCEPTION_EXECUTE_HANDLER;
}*/

int main()
{
	HINSTANCE hInstance = GetModuleHandle(0);
	int nCmdShow = SW_SHOW;
	// Install SEH
	// SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)lpTopLevelExceptionFilter);
	sprintf(szAppClass, "GameServer%p", static_cast<void*>(hInstance));
	if (!InitApplication(hInstance))		return (false);
	if (!InitInstance(hInstance, nCmdShow)) return (false);

	Initialize();
	EventLoop();
	return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return main();
}



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
	std::snprintf(cTitle, sizeof(cTitle), "Helbreath GameServer V%s.%s %d (Executed at: %d %d %d)", DEF_UPPERVERSION, DEF_LOWERVERSION, DEF_BUILDDATE, SysTime.wMonth, SysTime.wDay, SysTime.wHour);

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

	// Redirect stdout/stderr to console
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	// Print startup banner
	printf("\n");
	printf("=======================================================================\n");
	printf("         HELBREATH GAME SERVER - CONSOLE MODE                         \n");
	printf("=======================================================================\n");
	printf("Version: %s.%s\n", DEF_UPPERVERSION, DEF_LOWERVERSION);
	printf("Build: %d\n", DEF_BUILDDATE);
	printf("Started: %d/%d/%d %02d:%02d\n", SysTime.wMonth, SysTime.wDay, SysTime.wYear, SysTime.wHour, SysTime.wMinute);
	printf("=======================================================================\n\n");
	printf("Initializing server...\n\n");

	return (true);
}

//------------------------------------------------------------------------------
// Console Command Processing
//------------------------------------------------------------------------------
static char g_cConsoleBuffer[256] = {};
static int g_iConsoleBufferPos = 0;

void ProcessConsoleCommand(const char* cmd)
{
	if (cmd == nullptr || *cmd == '\0') return;
	
	// Trim leading whitespace
	while (*cmd == ' ' || *cmd == '\t') cmd++;
	if (*cmd == '\0') return;
	
	if (strcmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0) {
		printf("\n=== Server Console Commands ===\n");
		printf("  help, ?         - Show this help message\n");
		printf("  reloaditems     - Hot reload item configurations from database\n");
		printf("  status          - Show server status\n");
		printf("  quit, exit      - Shutdown the server\n");
		printf("================================\n\n");
	}
	else if (strcmp(cmd, "reloaditems") == 0) {
		printf("\n[CONSOLE] Executing item config reload...\n");
		if (G_pGame != nullptr && G_pGame->ReloadItemConfigs()) {
			printf("[CONSOLE] Item configs reloaded successfully!\n\n");
		} else {
			printf("[CONSOLE] Item config reload FAILED! Check server logs.\n\n");
		}
	}
	else if (strcmp(cmd, "status") == 0) {
		if (G_pGame != nullptr) {
			int activeClients = 0;
			for (int i = 1; i < DEF_MAXCLIENTS; i++) {
				if (G_pGame->m_pClientList[i] != nullptr) activeClients++;
			}
			printf("\n=== Server Status ===\n");
			printf("  Active clients: %d / %d\n", activeClients, DEF_MAXCLIENTS - 1);
			printf("  Game started: %s\n", G_pGame->m_bIsGameStarted ? "Yes" : "No");
			printf("=====================\n\n");
		}
	}
	else if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0) {
		printf("\n[CONSOLE] Shutting down server...\n");
		PostMessage(G_hWnd, WM_CLOSE, 0, 0);
	}
	else {
		printf("[CONSOLE] Unknown command: '%s'. Type 'help' for available commands.\n", cmd);
	}
}

void CheckConsoleInput()
{
	// Check if there's keyboard input available (non-blocking)
	while (_kbhit()) {
		int ch = _getch();
		
		if (ch == '\r' || ch == '\n') {
			// Enter pressed - process command
			g_cConsoleBuffer[g_iConsoleBufferPos] = '\0';
			printf("\n");
			ProcessConsoleCommand(g_cConsoleBuffer);
			g_iConsoleBufferPos = 0;
			g_cConsoleBuffer[0] = '\0';
			printf("> ");
		}
		else if (ch == '\b' || ch == 127) {
			// Backspace
			if (g_iConsoleBufferPos > 0) {
				g_iConsoleBufferPos--;
				printf("\b \b");
			}
		}
		else if (ch >= 32 && ch < 127 && g_iConsoleBufferPos < (int)sizeof(g_cConsoleBuffer) - 1) {
			// Regular character
			g_cConsoleBuffer[g_iConsoleBufferPos++] = (char)ch;
			printf("%c", ch);
		}
	}
}

// MODERNIZED: Poll all sockets for network events
// This function can be called from anywhere to keep sockets responsive during long operations
void PollAllSockets()
{
	if (G_pGame == nullptr) return;

	// Poll listen sockets for new connections
	if (G_pListenSock != nullptr) {
		int iRet = G_pListenSock->Poll();
		if (iRet == DEF_XSOCKEVENT_CONNECTIONESTABLISH) {
			G_pGame->bAccept(G_pListenSock);
		}
		else if (iRet < 0 && iRet != DEF_XSOCKEVENT_QUENEFULL) {
			// Log errors only (negative values except queue full)
			std::snprintf(G_cTxt, sizeof(G_cTxt), "[ERROR] ListenSock Poll() error: %d", iRet);
			PutLogList(G_cTxt);
		}
	}
	if (G_pLoginSock != nullptr) {
		int iRet = G_pLoginSock->Poll();
		if (iRet == DEF_XSOCKEVENT_CONNECTIONESTABLISH) {
			G_pGame->bAcceptLogin(G_pLoginSock);
		}
		else if (iRet < 0 && iRet != DEF_XSOCKEVENT_QUENEFULL) {
			// Log errors only (negative values except queue full)
			std::snprintf(G_cTxt, sizeof(G_cTxt), "[ERROR] LoginSock Poll() error: %d", iRet);
			PutLogList(G_cTxt);
		}
	}

	// Poll all game client sockets
	for (int i = 1; i < DEF_MAXCLIENTS; i++) {
		if (G_pGame->m_pClientList[i] != nullptr && G_pGame->m_pClientList[i]->m_pXSock != nullptr) {
			G_pGame->OnClientSocketEvent(i);
		}
	}

	// Poll all login client sockets
	for (int i = 0; i < DEF_MAXCLIENTLOGINSOCK; i++) {
		if (G_pGame->_lclients[i] != nullptr && G_pGame->_lclients[i]->_sock != nullptr) {
			G_pGame->OnLoginClientSocketEvent(i);
		}
	}
}

int EventLoop()
{
	static unsigned short _usCnt = 0;
	static uint32_t dwLastDebug = 0;
	static uint32_t dwLastGameProcess = 0;
	MSG msg;

	while (true) {
		if (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE)) {
			if (!GetMessage(&msg, 0, 0, 0)) {
				return msg.wParam;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			// MODERNIZED: Poll sockets and run game logic continuously without blocking
			if (G_pGame != nullptr) {
				uint32_t dwNow = GameClock::GetTimeMS();

				// Run game logic every 300ms (same as old timer)
				// GameProcess calls NpcProcess which now polls sockets during entity processing
				if (dwNow - dwLastGameProcess >= 300) {
					G_pGame->GameProcess();
					dwLastGameProcess = dwNow;
				}

				// Poll all sockets for network events
				PollAllSockets();

				// Debug: Show polling stats every 60 seconds (optional, can be removed)
				if (dwNow - dwLastDebug > 60000) {
					int activeClients = 0;
					int activeLoginClients = 0;
					for (int i = 1; i < DEF_MAXCLIENTS; i++) {
						if (G_pGame->m_pClientList[i] != nullptr) activeClients++;
					}
					for (int i = 0; i < DEF_MAXCLIENTLOGINSOCK; i++) {
						if (G_pGame->_lclients[i] != nullptr) activeLoginClients++;
					}
					dwLastDebug = dwNow;
				}
			}
			
			// Check for console input (non-blocking)
			CheckConsoleInput();
			
			// Small sleep to prevent 100% CPU usage
			Sleep(1);
		}
	}
}



void Initialize()
{

	if (_InitWinsock() == false) {
#ifdef _WIN32
		MessageBox(G_hWnd, "Socket 1.1 not found! Cannot execute program.", "ERROR", MB_ICONEXCLAMATION | MB_OK);
#else
		fprintf(stderr, "Socket 1.1 not found! Cannot execute program.\n");
#endif
		PostQuitMessage(0);
		return;
	}

	G_pGame = new class CGame(G_hWnd);
	if (G_pGame->bInit() == false) {
		PutLogList("(!!!) STOPPED!");
		return;
	}

	// ���� ����� Ÿ�̸� 
	G_mmTimer = _StartTimer(300);

	// MODERNIZED: Removed G_hWnd parameter, using WSAEventSelect
	G_pListenSock = new class XSocket(DEF_SERVERSOCKETBLOCKLIMIT);
	G_pListenSock->bListen(G_pGame->m_cGameListenIP, G_pGame->m_iGameListenPort);

	G_pLoginSock = new class XSocket(DEF_SERVERSOCKETBLOCKLIMIT);
	G_pLoginSock->bListen(G_pGame->m_cLoginListenIP, G_pGame->m_iLoginListenPort);

	pLogFile = 0;
	//pLogFile = fopen("test.log","wt+");
	
	// Show console command prompt
	printf("\n=======================================================================\n");
	printf("Server ready. Type 'help' for available console commands.\n");
	printf("=======================================================================\n");
	printf("> ");
}

void OnDestroy()
{
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
	_TermWinsock();

	if (pLogFile != 0) fclose(pLogFile);

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
		FILE* file = fopen("GameLogs\\server.log", "at");
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
			HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			CONSOLE_SCREEN_BUFFER_INFO info = {};
			WORD original = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
			if (hOut != INVALID_HANDLE_VALUE && GetConsoleScreenBufferInfo(hOut, &info)) {
				original = info.wAttributes;
				SetConsoleTextAttribute(hOut, GetLevelColor(level));
			}
			printf("%s\n", line);
			if (hOut != INVALID_HANDLE_VALUE && GetConsoleScreenBufferInfo(hOut, &info)) {
				SetConsoleTextAttribute(hOut, original);
			}
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
	// char cTemp[120*50];

		//G_cMsgUpdated = true;
		//std::memset(cTemp, 0, sizeof(cTemp));
		//memcpy((cTemp + 120), G_cMsgList, 120*49);
		//memcpy(cTemp, cMsg, strlen(cMsg));
		//memcpy(G_cMsgList, cTemp, 120*50);
	PutXSocketLogFileList(cMsg);

}

// UpdateScreen() - REMOVED: No longer needed for console output

// OnPaint() - REMOVED: No longer needed for console output



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

	// Original:
	// pFile = fopen("Events.log", "at");
	pFile = fopen("GameLogs\\Events.log", "at");
	if (pFile == 0) return;
	std::memset(cBuffer, 0, sizeof(cBuffer));
	GetLocalTime(&SysTime);
	std::snprintf(cBuffer, sizeof(cBuffer), "(%4d:%2d:%2d:%2d:%2d) - ", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute);
	strcat(cBuffer, cStr);
	strcat(cBuffer, "\n");
	fwrite(cBuffer, 1, strlen(cBuffer), pFile);
	fclose(pFile);
}

void PutAdminLogFileList(char* cStr)
{
	FILE* pFile;
	char cBuffer[512];
	SYSTEMTIME SysTime;

	pFile = fopen("GameLogs\\AdminEvents.log", "at");
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

	pFile = fopen("GameLogs\\HackEvents.log", "at");
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

	pFile = fopen("GameLogs\\PvPEvents.log", "at");
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

	pFile = fopen("GameLogs\\XSocket.log", "at");
	if (pFile == 0) return;

	std::memset(cBuffer, 0, sizeof(cBuffer));

	GetLocalTime(&SysTime);
	std::snprintf(cBuffer, sizeof(cBuffer), "(%4d:%2d:%2d:%2d:%2d) - ", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute);
	strcat(cBuffer, cStr);
	strcat(cBuffer, "\n");

	fwrite(cBuffer, 1, strlen(cBuffer), pFile);
	fclose(pFile);
}

void PutItemLogFileList(char* cStr)
{
	FILE* pFile;
	char cBuffer[512];
	SYSTEMTIME SysTime;

	pFile = fopen("GameLogs\\ItemEvents.log", "at");
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

	pFile = fopen("GameLogs\\LogEvents.log", "at");
	if (pFile == 0) return;

	std::memset(cBuffer, 0, sizeof(cBuffer));

	GetLocalTime(&SysTime);
	std::snprintf(cBuffer, sizeof(cBuffer), "(%4d:%2d:%2d:%2d:%2d) - ", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute);
	strcat(cBuffer, cStr);
	strcat(cBuffer, "\n");

	fwrite(cBuffer, 1, strlen(cBuffer), pFile);
	fclose(pFile);
}
#else
int main()
{
	return 0;
}
#endif
