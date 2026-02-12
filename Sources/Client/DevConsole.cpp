// DevConsole.cpp: In-game developer console singleton
//
// Owns ring buffer, input/history, and command registry.
// Persists for the entire application lifetime.
//
//////////////////////////////////////////////////////////////////////

#define _WINSOCKAPI_

#include "DevConsole.h"
#include "FrameTiming.h"
#include "RendererFactory.h"

#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <format>
#include <string>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <strings.h>  // strcasecmp
#endif

// Cross-platform compat shims
#ifndef _WIN32
#define sprintf_s(buf, ...) snprintf(buf, sizeof(buf), __VA_ARGS__)
#define strncpy_s(dst, src, count) do { strncpy(dst, src, count); dst[count] = '\0'; } while(0)
#define _stricmp strcasecmp
#endif

// ============== Singleton ==============

DevConsole& DevConsole::Get()
{
	static DevConsole instance;
	return instance;
}

DevConsole::DevConsole()
{
	memset(m_lines, 0, sizeof(m_lines));
	memset(m_commands, 0, sizeof(m_commands));
}

DevConsole::~DevConsole()
{
	Shutdown();
}

// ============== Initialization ==============

void DevConsole::Initialize()
{
	RegisterBuiltInCommands();

	Print("=== Developer Console ===", hb::shared::render::Color{0, 255, 0});
	Print("Type 'help' for available commands.");
}

void DevConsole::Shutdown()
{
}

// ============== Built-In Commands ==============

void DevConsole::RegisterBuiltInCommands()
{
	RegisterCommand("help", [this](const char*) {
		Print("Available commands:", hb::shared::render::Color{0, 255, 0});
		for (int i = 0; i < m_iCommandCount; i++)
		{
			char buf[64];
			sprintf_s(buf, "  %s", m_commands[i].name);
			Print(buf, hb::shared::render::Color{0, 255, 0});
		}
	});

	RegisterCommand("clear", [this](const char*) {
		memset(m_lines, 0, sizeof(m_lines));
		m_iWriteIndex = 0;
		m_iLineCount = 0;
		m_iScrollOffset = 0;
	});

#ifdef _WIN32
	RegisterCommand("mem", [this](const char*) {
		PROCESS_MEMORY_COUNTERS_EX pmc;
		if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)))
		{
			char buf[MAX_LINE_LEN];
			sprintf_s(buf, "Working: %.2f MB | Private: %.2f MB | Peak: %.2f MB",
				pmc.WorkingSetSize / (1024.0 * 1024.0),
				pmc.PrivateUsage / (1024.0 * 1024.0),
				pmc.PeakWorkingSetSize / (1024.0 * 1024.0));
			Print(buf);
		}
	});
#endif

	RegisterCommand("fps", [this](const char*) {
		char buf[64];
		sprintf_s(buf, "FPS: %u", hb::shared::render::Renderer::Get() ? hb::shared::render::Renderer::Get()->GetFPS() : 0);
		Print(buf);
	});
}

// ============== Visibility ==============

void DevConsole::Toggle()
{
	if (m_bVisible)
		Hide();
	else
		Show();
}

void DevConsole::Show()
{
	m_bVisible = true;
	m_iScrollOffset = 0;
}

void DevConsole::Hide()
{
	m_bVisible = false;
}

// ============== Output ==============

void DevConsole::AddLine(const char* text, const hb::shared::render::Color& color)
{
	ConsoleLine& line = m_lines[m_iWriteIndex];
	strncpy_s(line.text, text, MAX_LINE_LEN - 1);
	line.text[MAX_LINE_LEN - 1] = '\0';
	line.color = color;

	m_iWriteIndex = (m_iWriteIndex + 1) % MAX_LINES;
	if (m_iLineCount < MAX_LINES)
		m_iLineCount++;
}

void DevConsole::Print(const char* text, const hb::shared::render::Color& color)
{
	AddLine(text, color);
}

void DevConsole::Printf(const char* fmt, ...)
{
	char buf[MAX_LINE_LEN];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	buf[MAX_LINE_LEN - 1] = '\0';
	Print(buf);
}

// ============== Input ==============

bool DevConsole::HandleChar(unsigned int codepoint)
{
	char c = static_cast<char>(codepoint);

	// Enter - execute command
	if (c == '\r' || c == '\n')
	{
		if (m_iInputLen > 0)
			ExecuteCommand();
		return true;
	}

	// Backspace
	if (c == '\b')
	{
		if (m_iInputLen > 0)
		{
			m_iInputLen--;
			m_szInput[m_iInputLen] = '\0';
		}
		return true;
	}

	// Escape handled in overlay on_update
	if (c == 27)
		return true;

	// Tab - ignore
	if (c == '\t')
		return true;

	// Tilde/backtick - ignore (hotkey toggle)
	if (c == '`' || c == '~')
		return true;

	// Normal character
	if (c >= 32 && m_iInputLen < CONSOLE_MAX_INPUT - 1)
	{
		m_szInput[m_iInputLen++] = c;
		m_szInput[m_iInputLen] = '\0';
	}

	return true;
}

bool DevConsole::HandleKeyDown(KeyCode keyCode)
{
	switch (keyCode)
	{
	case KeyCode::Up:
		HistoryUp();
		return true;
	case KeyCode::Down:
		HistoryDown();
		return true;
	case KeyCode::PageUp:
		ScrollUp(8);
		return true;
	case KeyCode::PageDown:
		ScrollDown(8);
		return true;
	default:
		break;
	}
	return false;
}

// ============== Scroll ==============

void DevConsole::ScrollUp(int lines)
{
	m_iScrollOffset += lines;
	int maxScroll = m_iLineCount > 16 ? m_iLineCount - 16 : 0;
	if (m_iScrollOffset > maxScroll)
		m_iScrollOffset = maxScroll;
}

void DevConsole::ScrollDown(int lines)
{
	m_iScrollOffset -= lines;
	if (m_iScrollOffset < 0)
		m_iScrollOffset = 0;
}

// ============== Command Execution ==============

void DevConsole::ExecuteCommand()
{
	// Add to history
	if (m_iHistoryCount < MAX_HISTORY)
	{
		strncpy_s(m_szHistory[m_iHistoryCount], m_szInput, CONSOLE_MAX_INPUT - 1);
		m_iHistoryCount++;
	}
	else
	{
		// Shift history up
		for (int i = 0; i < MAX_HISTORY - 1; i++)
			memcpy(m_szHistory[i], m_szHistory[i + 1], CONSOLE_MAX_INPUT);
		strncpy_s(m_szHistory[MAX_HISTORY - 1], m_szInput, CONSOLE_MAX_INPUT - 1);
	}
	m_iHistoryIndex = -1;

	// Echo command in yellow
	std::string echo;
	echo = std::format("> {}", m_szInput);
	Print(echo.c_str(), hb::shared::render::Color{255, 255, 0});

	// Parse command name and args
	char cmdName[32] = {};
	const char* args = "";
	const char* space = strchr(m_szInput, ' ');
	if (space)
	{
		int len = static_cast<int>(space - m_szInput);
		if (len > 31) len = 31;
		strncpy_s(cmdName, m_szInput, len);
		args = space + 1;
	}
	else
	{
		strncpy_s(cmdName, m_szInput, 31);
	}

	// Find and execute
	bool found = false;
	for (int i = 0; i < m_iCommandCount; i++)
	{
		if (_stricmp(m_commands[i].name, cmdName) == 0)
		{
			m_commands[i].cb(args);
			found = true;
			break;
		}
	}

	if (!found)
	{
		std::string err;
		err = std::format("Unknown command: {}", cmdName);
		Print(err.c_str(), hb::shared::render::Color{255, 68, 68});
	}

	// Clear input
	m_szInput[0] = '\0';
	m_iInputLen = 0;
	m_iScrollOffset = 0;
}

// ============== History ==============

void DevConsole::HistoryUp()
{
	if (m_iHistoryCount == 0) return;

	if (m_iHistoryIndex == -1)
		m_iHistoryIndex = m_iHistoryCount - 1;
	else if (m_iHistoryIndex > 0)
		m_iHistoryIndex--;

	strncpy_s(m_szInput, m_szHistory[m_iHistoryIndex], CONSOLE_MAX_INPUT - 1);
	m_iInputLen = static_cast<int>(strlen(m_szInput));
}

void DevConsole::HistoryDown()
{
	if (m_iHistoryIndex == -1) return;

	if (m_iHistoryIndex < m_iHistoryCount - 1)
	{
		m_iHistoryIndex++;
		strncpy_s(m_szInput, m_szHistory[m_iHistoryIndex], CONSOLE_MAX_INPUT - 1);
		m_iInputLen = static_cast<int>(strlen(m_szInput));
	}
	else
	{
		m_iHistoryIndex = -1;
		m_szInput[0] = '\0';
		m_iInputLen = 0;
	}
}

// ============== Command Registry ==============

void DevConsole::RegisterCommand(const char* name, CmdCallback cb)
{
	if (m_iCommandCount >= MAX_COMMANDS) return;

	Command& cmd = m_commands[m_iCommandCount];
	strncpy_s(cmd.name, name, 31);
	cmd.cb = std::move(cb);
	m_iCommandCount++;
}
