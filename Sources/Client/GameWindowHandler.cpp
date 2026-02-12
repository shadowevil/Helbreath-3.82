// GameWindowHandler.cpp: hb::shared::render::Window event handler adapter for CGame
//
// Routes window events to CGame and hb::shared::input::
//////////////////////////////////////////////////////////////////////

#include "GameWindowHandler.h"
#include "Game.h"
#include "TextInputManager.h"
#include "IInput.h"
#include "RendererFactory.h"
#include "ISpriteFactory.h"
#include "DevConsole.h"
#include "Overlay_DevConsole.h"
#include "GameModeManager.h"

#ifndef _WIN32
// Win32 message constants - only used in OnCustomMessage/OnTextInput
// which are only called from Win32 message pump code
#define WM_CHAR           0x0102
#define WM_SETCURSOR      0x0020
#define WM_SETFOCUS       0x0007
#define WM_KILLFOCUS      0x0008
#define WM_LBUTTONDBLCLK  0x0203
typedef intptr_t LPARAM;
#define GET_X_LPARAM(lp) ((int)(short)(lp & 0xFFFF))
#define GET_Y_LPARAM(lp) ((int)(short)((lp >> 16) & 0xFFFF))
namespace MouseButton = hb::shared::input::MouseButton;
#endif

#ifdef _WIN32
#include <windowsx.h>

namespace MouseButton = hb::shared::input::MouseButton;

#endif

GameWindowHandler::GameWindowHandler(CGame* pGame)
    : m_pGame(pGame)
{
}

void GameWindowHandler::OnClose()
{
    if (!m_pGame)
    {
        // No game, just close via hb::shared::render::Window abstraction
        hb::shared::render::Window::Close();
        return;
    }

    if ((GameModeManager::GetMode() == GameMode::MainGame) && (m_pGame->m_bForceDisconn == false))
    {
        // In main game, start logout countdown instead of closing immediately
#ifdef _DEBUG
        if (m_pGame->m_cLogOutCount == -1 || m_pGame->m_cLogOutCount > 2)
            m_pGame->m_cLogOutCount = 1;
#else
        if (m_pGame->m_cLogOutCount == -1 || m_pGame->m_cLogOutCount > 11)
            m_pGame->m_cLogOutCount = 11;
#endif
    }
    else if (GameModeManager::GetMode() == GameMode::MainMenu)
    {
        // On main menu, show quit screen
        m_pGame->ChangeGameMode(GameMode::Quit);
    }
    else if (GameModeManager::GetMode() == GameMode::Null)
    {
        // Game code requested close (e.g., from quit screen), proceed with destruction
        hb::shared::render::Window::Close();
    }
    else
    {
        // Other modes (loading, etc.), proceed with closing
        hb::shared::render::Window::Close();
    }
}

void GameWindowHandler::OnDestroy()
{
    if (m_pGame)
    {
        m_pGame->Quit();
    }
    // ASIO handles Winsock cleanup internally
}

void GameWindowHandler::OnActivate(bool active)
{
    if (!m_pGame)
        return;

    if (!active)
    {
        if (hb::shared::input::Get())
            hb::shared::input::Get()->SetWindowActive(false);
    }
    else
    {
        if (hb::shared::input::Get())
            hb::shared::input::Get()->SetWindowActive(true);

        if (m_pGame->m_Renderer != nullptr)
            m_pGame->m_Renderer->ChangeDisplayMode(hb::shared::render::Window::GetHandle());

        // Only check files after game is fully initialized (sprite factory exists)
        // This prevents false failures during early window activation before bInit completes
        if (hb::shared::sprite::Sprites::GetFactory() != nullptr)
        {
            if (m_pGame->bCheckImportantFile() == false)
            {
                hb::shared::render::Window::ShowError("ERROR1", "File checksum error! Get Update again please!");
                hb::shared::render::Window::Close();
            }
        }
    }
}

void GameWindowHandler::OnResize(int width, int height)
{
    // Currently not handling resize events
}

void GameWindowHandler::OnKeyDown(KeyCode keyCode)
{
    if (!m_pGame)
        return;

    // Route all keys through Input system
    if (hb::shared::input::Get())
        hb::shared::input::Get()->OnKeyDown(keyCode);

    // Route arrow/page keys to DevConsole when visible
    if (DevConsole::Get().IsVisible())
    {
        DevConsole::Get().HandleKeyDown(keyCode);
        return;
    }

    // Also notify game for special key handling (hotkeys, etc.)
    // Skip modifier keys as they're handled purely through hb::shared::input::
    if (keyCode != KeyCode::Shift && keyCode != KeyCode::Control && keyCode != KeyCode::Alt &&
        keyCode != KeyCode::LShift && keyCode != KeyCode::RShift &&
        keyCode != KeyCode::LControl && keyCode != KeyCode::RControl &&
        keyCode != KeyCode::LAlt && keyCode != KeyCode::RAlt &&
        keyCode != KeyCode::Enter && keyCode != KeyCode::Escape)
    {
        m_pGame->OnKeyDown(keyCode);
    }
}

void GameWindowHandler::OnKeyUp(KeyCode keyCode)
{
    if (!m_pGame)
        return;

    // Route all keys through Input system
    if (hb::shared::input::Get())
        hb::shared::input::Get()->OnKeyUp(keyCode);

    // Alt+Tilde: Toggle developer console (GM mode only)
    if (keyCode == KeyCode::Grave && hb::shared::input::IsAltDown())
    {
        DevConsole& console = DevConsole::Get();
        if (console.IsVisible())
        {
            console.Hide();
            GameModeManager::clear_overlay();
        }
        else
        {
            if (m_pGame != nullptr && m_pGame->m_pPlayer != nullptr && m_pGame->m_pPlayer->m_bIsGMMode)
            {
                console.Show();
                GameModeManager::set_overlay<Overlay_DevConsole>();
            }
        }
        return;
    }

    // Block all other keys when DevConsole is visible
    if (DevConsole::Get().IsVisible())
        return;

    // Also notify game for special key handling
    // Skip modifier keys as they're handled purely through hb::shared::input::
    if (keyCode != KeyCode::Shift && keyCode != KeyCode::Control && keyCode != KeyCode::Alt &&
        keyCode != KeyCode::LShift && keyCode != KeyCode::RShift &&
        keyCode != KeyCode::LControl && keyCode != KeyCode::RControl &&
        keyCode != KeyCode::LAlt && keyCode != KeyCode::RAlt &&
        keyCode != KeyCode::Enter)
    {
        m_pGame->OnKeyUp(keyCode);
    }
}

void GameWindowHandler::OnChar(char character)
{
    // Character input is handled through OnTextInput
}

void GameWindowHandler::OnMouseMove(int x, int y)
{
    if (hb::shared::input::Get())
        hb::shared::input::Get()->OnMouseMove(x, y);
}

void GameWindowHandler::OnMouseButtonDown(int button, int x, int y)
{
    if (hb::shared::input::Get())
    {
        hb::shared::input::Get()->OnMouseMove(x, y);
        hb::shared::input::Get()->OnMouseDown(button);
    }
}

void GameWindowHandler::OnMouseButtonUp(int button, int x, int y)
{
    if (hb::shared::input::Get())
    {
        hb::shared::input::Get()->OnMouseMove(x, y);
        hb::shared::input::Get()->OnMouseUp(button);
    }
}

void GameWindowHandler::OnMouseWheel(int delta, int x, int y)
{
    if (hb::shared::input::Get())
    {
        hb::shared::input::Get()->OnMouseMove(x, y);
        hb::shared::input::Get()->OnMouseWheel(delta);
    }
}

bool GameWindowHandler::OnCustomMessage(uint32_t message, uintptr_t wParam, intptr_t lParam)
{
    if (!m_pGame)
        return false;

    switch (message)
    {
    case WM_SETCURSOR:
        if (hb::shared::render::Window::Get())
            hb::shared::render::Window::Get()->SetMouseCursorVisible(false);
        return true;

    case WM_SETFOCUS:
        if (hb::shared::input::Get())
            hb::shared::input::Get()->SetWindowActive(true);
        return true;

    case WM_KILLFOCUS:
        if (hb::shared::input::Get())
            hb::shared::input::Get()->SetWindowActive(false);
        return true;

    case WM_LBUTTONDBLCLK:
        // Handle double-click as button down for manual detection
        if (hb::shared::input::Get())
        {
            hb::shared::input::Get()->OnMouseMove(GET_X_LPARAM(static_cast<LPARAM>(lParam)), GET_Y_LPARAM(static_cast<LPARAM>(lParam)));
            hb::shared::input::Get()->OnMouseDown(MouseButton::Left);
        }
        return true;
    }

    return false;
}

bool GameWindowHandler::OnTextInput(hb::shared::types::NativeWindowHandle hWnd, uint32_t message, uintptr_t wParam, intptr_t lParam)
{
    // Route WM_CHAR text input to DevConsole when visible
    if (message == WM_CHAR && DevConsole::Get().IsVisible())
    {
        return DevConsole::Get().HandleChar(static_cast<unsigned int>(wParam));
    }

    if (m_pGame)
    {
        return TextInputManager::Get().HandleChar(hWnd, message, wParam, lParam) != 0;
    }
    return false;
}
