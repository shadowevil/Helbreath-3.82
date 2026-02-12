#include "ServerConsole.h"
#include <cstring>
#include <cstdio>

static ServerConsole s_console;

ServerConsole& GetServerConsole()
{
	return s_console;
}

#ifdef _WIN32
// ============================================================================
// Windows implementation
// ============================================================================

ServerConsole::ServerConsole()
	: m_hOut(INVALID_HANDLE_VALUE)
	, m_hIn(INVALID_HANDLE_VALUE)
	, m_iInputLen(0)
	, m_iCursorPos(0)
	, m_sWidth(80)
	, m_sHeight(25)
	, m_bInit(false)
	, m_dwOrigMode(0)
{
	memset(m_szInput, 0, sizeof(m_szInput));
}

ServerConsole::~ServerConsole()
{
	if (m_bInit && m_hIn != INVALID_HANDLE_VALUE) {
		SetConsoleMode(m_hIn, m_dwOrigMode);
	}
}

bool ServerConsole::Init()
{
	m_hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	m_hIn = GetStdHandle(STD_INPUT_HANDLE);

	if (m_hOut == INVALID_HANDLE_VALUE || m_hIn == INVALID_HANDLE_VALUE) {
		return false;
	}

	// Save original input mode and set non-blocking character mode
	if (!GetConsoleMode(m_hIn, &m_dwOrigMode)) {
		return false;
	}

	// Disable line input and echo so we get raw key events
	DWORD mode = ENABLE_WINDOW_INPUT;
	if (!SetConsoleMode(m_hIn, mode)) {
		return false;
	}

	// Get console dimensions
	CONSOLE_SCREEN_BUFFER_INFO info = {};
	if (GetConsoleScreenBufferInfo(m_hOut, &info)) {
		m_sWidth = info.srWindow.Right - info.srWindow.Left + 1;
		m_sHeight = info.srWindow.Bottom - info.srWindow.Top + 1;
	}

	m_bInit = true;
	DrawInputLine();
	return true;
}

SHORT ServerConsole::GetPromptRow()
{
	CONSOLE_SCREEN_BUFFER_INFO info = {};
	if (GetConsoleScreenBufferInfo(m_hOut, &info)) {
		return info.dwCursorPosition.Y;
	}
	return 0;
}

void ServerConsole::ClearInputLine()
{
	if (!m_bInit) return;

	SHORT row = GetPromptRow();
	COORD pos = { 0, row };
	DWORD written = 0;
	FillConsoleOutputCharacterA(m_hOut, ' ', m_sWidth, pos, &written);
	FillConsoleOutputAttribute(m_hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, m_sWidth, pos, &written);
	SetConsoleCursorPosition(m_hOut, pos);
}

void ServerConsole::DrawInputLine()
{
	if (!m_bInit) return;

	SHORT row = GetPromptRow();
	COORD pos = { 0, row };
	DWORD written = 0;

	// Clear the line
	FillConsoleOutputCharacterA(m_hOut, ' ', m_sWidth, pos, &written);
	FillConsoleOutputAttribute(m_hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, m_sWidth, pos, &written);

	// Write green "> " prompt
	SetConsoleCursorPosition(m_hOut, pos);
	SetConsoleTextAttribute(m_hOut, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	const char* prompt = "> ";
	WriteConsoleA(m_hOut, prompt, 2, &written, nullptr);

	// Write input text in white
	if (m_iInputLen > 0) {
		SetConsoleTextAttribute(m_hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		WriteConsoleA(m_hOut, m_szInput, m_iInputLen, &written, nullptr);
	}

	// Restore default color
	SetConsoleTextAttribute(m_hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

	// Position cursor at the correct spot (prompt offset 2 + cursor position)
	COORD cursorPos = { (SHORT)(2 + m_iCursorPos), row };
	SetConsoleCursorPosition(m_hOut, cursorPos);
}

void ServerConsole::WriteLine(const char* text, WORD color)
{
	if (!m_bInit) {
		// Fallback if not initialized
		printf("%s\n", text);
		return;
	}

	DWORD written = 0;

	// Clear the prompt line
	ClearInputLine();

	// Write the log line with color at current position
	SetConsoleTextAttribute(m_hOut, color);
	WriteConsoleA(m_hOut, text, (DWORD)strlen(text), &written, nullptr);

	// Newline to scroll
	SetConsoleTextAttribute(m_hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	WriteConsoleA(m_hOut, "\n", 1, &written, nullptr);

	// Redraw prompt on the new bottom line
	DrawInputLine();
}

void ServerConsole::RedrawPrompt()
{
	if (!m_bInit) return;
	DrawInputLine();
}

bool ServerConsole::PollInput(char* pOutCmd, int maxLen)
{
	if (!m_bInit) return false;

	DWORD numEvents = 0;
	GetNumberOfConsoleInputEvents(m_hIn, &numEvents);
	if (numEvents == 0) return false;

	bool commandReady = false;

	// Process all pending events in one batch (non-blocking)
	INPUT_RECORD records[32];
	DWORD eventsRead = 0;
	while (numEvents > 0) {
		DWORD toRead = (numEvents > 32) ? 32 : numEvents;
		if (!ReadConsoleInput(m_hIn, records, toRead, &eventsRead)) {
			break;
		}
		numEvents -= eventsRead;

		for (DWORD i = 0; i < eventsRead; i++) {
			if (records[i].EventType == WINDOW_BUFFER_SIZE_EVENT) {
				m_sWidth = records[i].Event.WindowBufferSizeEvent.dwSize.X;
				m_sHeight = records[i].Event.WindowBufferSizeEvent.dwSize.Y;
				DrawInputLine();
				continue;
			}

			if (records[i].EventType != KEY_EVENT) continue;
			if (!records[i].Event.KeyEvent.bKeyDown) continue;

			KEY_EVENT_RECORD& key = records[i].Event.KeyEvent;
			WORD vk = key.wVirtualKeyCode;
			char ch = key.uChar.AsciiChar;

			if (vk == VK_RETURN) {
				// Enter pressed - copy buffer to output
				if (m_iInputLen > 0) {
					int copyLen = (m_iInputLen < maxLen - 1) ? m_iInputLen : (maxLen - 1);
					memcpy(pOutCmd, m_szInput, copyLen);
					pOutCmd[copyLen] = '\0';
					commandReady = true;
				}
				// Clear input buffer
				memset(m_szInput, 0, sizeof(m_szInput));
				m_iInputLen = 0;
				m_iCursorPos = 0;
				DrawInputLine();
				if (commandReady) return true;
				continue;
			}

			if (vk == VK_BACK) {
				if (m_iCursorPos > 0) {
					// Remove char before cursor
					memmove(&m_szInput[m_iCursorPos - 1], &m_szInput[m_iCursorPos], m_iInputLen - m_iCursorPos);
					m_iInputLen--;
					m_iCursorPos--;
					m_szInput[m_iInputLen] = '\0';
					DrawInputLine();
				}
				continue;
			}

			if (vk == VK_DELETE) {
				if (m_iCursorPos < m_iInputLen) {
					memmove(&m_szInput[m_iCursorPos], &m_szInput[m_iCursorPos + 1], m_iInputLen - m_iCursorPos - 1);
					m_iInputLen--;
					m_szInput[m_iInputLen] = '\0';
					DrawInputLine();
				}
				continue;
			}

			if (vk == VK_LEFT) {
				if (m_iCursorPos > 0) {
					m_iCursorPos--;
					DrawInputLine();
				}
				continue;
			}

			if (vk == VK_RIGHT) {
				if (m_iCursorPos < m_iInputLen) {
					m_iCursorPos++;
					DrawInputLine();
				}
				continue;
			}

			if (vk == VK_HOME) {
				m_iCursorPos = 0;
				DrawInputLine();
				continue;
			}

			if (vk == VK_END) {
				m_iCursorPos = m_iInputLen;
				DrawInputLine();
				continue;
			}

			// Printable characters
			if (ch >= 32 && ch < 127) {
				if (m_iInputLen < (int)(sizeof(m_szInput) - 1)) {
					// Insert at cursor position
					memmove(&m_szInput[m_iCursorPos + 1], &m_szInput[m_iCursorPos], m_iInputLen - m_iCursorPos);
					m_szInput[m_iCursorPos] = ch;
					m_iInputLen++;
					m_iCursorPos++;
					m_szInput[m_iInputLen] = '\0';
					DrawInputLine();
				}
				continue;
			}
		}
	}

	return false;
}

#else
// ============================================================================
// Linux/POSIX implementation using termios + ANSI escape codes
// ============================================================================

#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>

static struct termios s_origTermios;
static bool s_termiosRestored = false;

ServerConsole::ServerConsole()
	: m_iInputLen(0)
	, m_iCursorPos(0)
	, m_iWidth(80)
	, m_iHeight(25)
	, m_bInit(false)
	, m_bRawMode(false)
{
	memset(m_szInput, 0, sizeof(m_szInput));
}

ServerConsole::~ServerConsole()
{
	if (m_bRawMode && !s_termiosRestored) {
		tcsetattr(STDIN_FILENO, TCSANOW, &s_origTermios);
		s_termiosRestored = true;
	}
}

bool ServerConsole::Init()
{
	// Save original terminal settings
	if (tcgetattr(STDIN_FILENO, &s_origTermios) != 0) {
		// Not a terminal (e.g. piped input) — use simple mode
		m_bInit = true;
		return true;
	}

	// Set raw mode: disable canonical mode and echo
	struct termios raw = s_origTermios;
	raw.c_lflag &= ~(ICANON | ECHO);
	raw.c_cc[VMIN] = 0;   // Non-blocking
	raw.c_cc[VTIME] = 0;
	if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0) {
		m_bInit = true;
		return true;
	}
	m_bRawMode = true;

	// Get terminal size
	struct winsize ws;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
		m_iWidth = ws.ws_col;
		m_iHeight = ws.ws_row;
	}

	m_bInit = true;
	DrawInputLine();
	return true;
}

static const char* AnsiColorFromWinColor(WORD color)
{
	bool red   = (color & FOREGROUND_RED) != 0;
	bool green = (color & FOREGROUND_GREEN) != 0;
	bool blue  = (color & FOREGROUND_BLUE) != 0;
	bool bright = (color & FOREGROUND_INTENSITY) != 0;

	if (red && green && blue) return bright ? "\033[97m" : "\033[37m"; // white
	if (red && green)         return bright ? "\033[93m" : "\033[33m"; // yellow
	if (red && blue)          return bright ? "\033[95m" : "\033[35m"; // magenta
	if (green && blue)        return bright ? "\033[96m" : "\033[36m"; // cyan
	if (red)                  return bright ? "\033[91m" : "\033[31m"; // red
	if (green)                return bright ? "\033[92m" : "\033[32m"; // green
	if (blue)                 return bright ? "\033[94m" : "\033[34m"; // blue
	return "\033[0m";
}

void ServerConsole::ClearInputLine()
{
	if (!m_bInit) return;
	// Move to start of line and clear it
	printf("\r\033[K");
	fflush(stdout);
}

void ServerConsole::DrawInputLine()
{
	if (!m_bInit) return;
	// Clear line, draw green prompt + white input text, position cursor
	printf("\r\033[K\033[92m> \033[97m%.*s\033[0m", m_iInputLen, m_szInput);
	// Position cursor: "\r" then move right by (2 + m_iCursorPos)
	printf("\r\033[%dC", 2 + m_iCursorPos);
	fflush(stdout);
}

void ServerConsole::WriteLine(const char* text, WORD color)
{
	if (!m_bInit) {
		printf("%s\n", text);
		return;
	}

	// Clear current input line
	ClearInputLine();

	// Print log line with color
	printf("%s%s\033[0m\n", AnsiColorFromWinColor(color), text);

	// Redraw input prompt
	DrawInputLine();
}

void ServerConsole::RedrawPrompt()
{
	if (!m_bInit) return;
	DrawInputLine();
}

bool ServerConsole::PollInput(char* pOutCmd, int maxLen)
{
	if (!m_bInit) return false;

	unsigned char buf[32];
	ssize_t n = read(STDIN_FILENO, buf, sizeof(buf));
	if (n <= 0) return false;

	bool commandReady = false;

	for (ssize_t i = 0; i < n; i++) {
		unsigned char ch = buf[i];

		// Check for escape sequences (arrow keys, etc.)
		if (ch == 0x1B && (i + 2) < n && buf[i + 1] == '[') {
			unsigned char seq = buf[i + 2];
			i += 2;
			if (seq == 'D') { // Left
				if (m_iCursorPos > 0) m_iCursorPos--;
				DrawInputLine();
			} else if (seq == 'C') { // Right
				if (m_iCursorPos < m_iInputLen) m_iCursorPos++;
				DrawInputLine();
			} else if (seq == 'H') { // Home
				m_iCursorPos = 0;
				DrawInputLine();
			} else if (seq == 'F') { // End
				m_iCursorPos = m_iInputLen;
				DrawInputLine();
			} else if (seq == '3' && (i + 1) < n && buf[i + 1] == '~') { // Delete
				i++;
				if (m_iCursorPos < m_iInputLen) {
					memmove(&m_szInput[m_iCursorPos], &m_szInput[m_iCursorPos + 1], m_iInputLen - m_iCursorPos - 1);
					m_iInputLen--;
					m_szInput[m_iInputLen] = '\0';
					DrawInputLine();
				}
			}
			continue;
		}

		// Enter
		if (ch == '\n' || ch == '\r') {
			if (m_iInputLen > 0) {
				int copyLen = (m_iInputLen < maxLen - 1) ? m_iInputLen : (maxLen - 1);
				memcpy(pOutCmd, m_szInput, copyLen);
				pOutCmd[copyLen] = '\0';
				commandReady = true;
			}
			memset(m_szInput, 0, sizeof(m_szInput));
			m_iInputLen = 0;
			m_iCursorPos = 0;
			// Move to new line before drawing prompt
			printf("\n");
			DrawInputLine();
			if (commandReady) return true;
			continue;
		}

		// Backspace
		if (ch == 0x7F || ch == 0x08) {
			if (m_iCursorPos > 0) {
				memmove(&m_szInput[m_iCursorPos - 1], &m_szInput[m_iCursorPos], m_iInputLen - m_iCursorPos);
				m_iInputLen--;
				m_iCursorPos--;
				m_szInput[m_iInputLen] = '\0';
				DrawInputLine();
			}
			continue;
		}

		// Printable characters
		if (ch >= 32 && ch < 127) {
			if (m_iInputLen < (int)(sizeof(m_szInput) - 1)) {
				memmove(&m_szInput[m_iCursorPos + 1], &m_szInput[m_iCursorPos], m_iInputLen - m_iCursorPos);
				m_szInput[m_iCursorPos] = (char)ch;
				m_iInputLen++;
				m_iCursorPos++;
				m_szInput[m_iInputLen] = '\0';
				DrawInputLine();
			}
			continue;
		}
	}

	return false;
}

#endif
