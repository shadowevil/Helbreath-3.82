#pragma once

#include "CommonTypes.h"
#include "IInput.h"  // For KeyCode
#include <cstdint>
#include <functional>

class DevConsole
{
public:
	static DevConsole& Get();

	void Initialize();
	void Shutdown();

	// State
	bool IsVisible() const { return m_bVisible; }
	void Toggle();
	void Show();
	void Hide();

	// Output
	void Print(const char* text, const hb::shared::render::Color& color = hb::shared::render::Color{192, 192, 192});
	void Printf(const char* fmt, ...);

	// Text input (called from Overlay_DevConsole / GameWindowHandler)
	bool HandleChar(unsigned int codepoint);
	bool HandleKeyDown(KeyCode keyCode);
	const char* GetInputBuffer() const { return m_szInput; }
	int GetInputLength() const { return m_iInputLen; }

	// Scroll
	void ScrollUp(int lines);
	void ScrollDown(int lines);
	int GetScrollOffset() const { return m_iScrollOffset; }

	// Ring buffer access (for rendering)
	int GetLineCount() const { return m_iLineCount; }
	int GetWriteIndex() const { return m_iWriteIndex; }

	struct ConsoleLine
	{
		char text[120];
		hb::shared::render::Color color;
	};
	const ConsoleLine* GetLines() const { return m_lines; }

	// Command system
	using CmdCallback = std::function<void(const char* args)>;
	void RegisterCommand(const char* name, CmdCallback cb);

	// Ring buffer constants (public for rendering access)
	static constexpr int MAX_LINES = 256;
	static constexpr int MAX_LINE_LEN = 120;

private:
	DevConsole();
	~DevConsole();
	DevConsole(const DevConsole&) = delete;
	DevConsole& operator=(const DevConsole&) = delete;

	void RegisterBuiltInCommands();
	void ExecuteCommand();
	void HistoryUp();
	void HistoryDown();
	void AddLine(const char* text, const hb::shared::render::Color& color);
	ConsoleLine m_lines[MAX_LINES];
	int m_iWriteIndex = 0;
	int m_iLineCount = 0;

	// Display
	bool m_bVisible = false;
	int m_iScrollOffset = 0;

	// Input
	static constexpr int CONSOLE_MAX_INPUT = 256;
	char m_szInput[CONSOLE_MAX_INPUT]{};
	int m_iInputLen = 0;

	// Command history
	static constexpr int MAX_HISTORY = 32;
	char m_szHistory[MAX_HISTORY][CONSOLE_MAX_INPUT]{};
	int m_iHistoryCount = 0;
	int m_iHistoryIndex = -1;

	// Commands
	static constexpr int MAX_COMMANDS = 64;
	struct Command
	{
		char name[32];
		CmdCallback cb;
	};
	Command m_commands[MAX_COMMANDS];
	int m_iCommandCount = 0;
};
