// GameWindowHandler.cpp: Window event handler adapter for CGame
//
// Routes window events to CGame and Input::
//////////////////////////////////////////////////////////////////////

// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)
#define _WINSOCKAPI_

#include "GameWindowHandler.h"
#include "Game.h"
#include "IInput.h"
#include "RendererFactory.h"
#include "ISpriteFactory.h"

// Custom message IDs (these should match what's used in the game)
#define WM_USER_CALCSOCKETEVENT (WM_USER + 600)

GameWindowHandler::GameWindowHandler(CGame* pGame)
    : m_pGame(pGame)
{
}

bool GameWindowHandler::OnClose()
{
    if (!m_pGame)
    {
        // No game, just close via Window abstraction
        DestroyWindow(Window::GetHandle());
        return true;  // Proceed with close
    }

    if ((m_pGame->m_cGameMode == DEF_GAMEMODE_ONMAINGAME) && (m_pGame->m_bForceDisconn == false))
    {
        // In main game, start logout countdown instead of closing immediately
#ifdef _DEBUG
        if (m_pGame->m_cLogOutCount == -1 || m_pGame->m_cLogOutCount > 2)
            m_pGame->m_cLogOutCount = 1;
#else
        if (m_pGame->m_cLogOutCount == -1 || m_pGame->m_cLogOutCount > 11)
            m_pGame->m_cLogOutCount = 11;
#endif
        return false;  // Cancel close - let logout countdown handle it
    }
    else if (m_pGame->m_cGameMode == DEF_GAMEMODE_ONMAINMENU)
    {
        // On main menu, show quit screen
        m_pGame->ChangeGameMode(DEF_GAMEMODE_ONQUIT);
        return false;  // Cancel close - let quit screen handle it
    }
    else if (m_pGame->m_cGameMode == DEF_GAMEMODE_NULL)
    {
        // Game code requested close (e.g., from quit screen), proceed with destruction
        DestroyWindow(m_pGame->m_hWnd);
        return true;  // Proceed with close
    }
    else
    {
        // Other modes (loading, etc.), proceed with closing
        DestroyWindow(m_pGame->m_hWnd);
        return true;  // Proceed with close
    }
}

void GameWindowHandler::OnDestroy()
{
    if (m_pGame)
    {
        m_pGame->m_bIsProgramActive = false;
        m_pGame->Quit();
    }
    WSACleanup();
}

void GameWindowHandler::OnActivate(bool active)
{
    if (!m_pGame)
        return;

    if (!active)
    {
        m_pGame->m_bIsProgramActive = false;
        if (Input::Get())
            Input::Get()->SetWindowActive(false);
    }
    else
    {
        m_pGame->m_bIsProgramActive = true;
        if (Input::Get())
            Input::Get()->SetWindowActive(true);

        m_pGame->m_bIsRedrawPDBGS = true;
        if (m_pGame->m_Renderer != nullptr)
            m_pGame->m_Renderer->ChangeDisplayMode(Window::GetHandle());

        // Only check files after game is fully initialized (sprite factory exists)
        // This prevents false failures during early window activation before bInit completes
        if (SpriteLib::Sprites::GetFactory() != nullptr)
        {
            if (m_pGame->bCheckImportantFile() == false)
            {
                MessageBox(m_pGame->m_hWnd, "File checksum error! Get Update again please!", "ERROR1", MB_ICONEXCLAMATION | MB_OK);
                PostQuitMessage(0);
            }
        }
    }
}

void GameWindowHandler::OnResize(int width, int height)
{
    // Currently not handling resize events
}

void GameWindowHandler::OnKeyDown(int keyCode)
{
    if (!m_pGame)
        return;

    // Route all keys through Input system
    if (Input::Get())
        Input::Get()->OnKeyDown(keyCode);

    // Also notify game for special key handling (hotkeys, etc.)
    // Skip modifier keys as they're handled purely through Input::
    if (keyCode != VK_SHIFT && keyCode != VK_CONTROL && keyCode != VK_MENU &&
        keyCode != VK_LSHIFT && keyCode != VK_RSHIFT &&
        keyCode != VK_LCONTROL && keyCode != VK_RCONTROL &&
        keyCode != VK_LMENU && keyCode != VK_RMENU &&
        keyCode != VK_RETURN && keyCode != VK_ESCAPE)
    {
        m_pGame->OnKeyDown(keyCode);
    }
}

void GameWindowHandler::OnKeyUp(int keyCode)
{
    if (!m_pGame)
        return;

    // Route all keys through Input system
    if (Input::Get())
        Input::Get()->OnKeyUp(keyCode);

    // Also notify game for special key handling
    // Skip modifier keys as they're handled purely through Input::
    if (keyCode != VK_SHIFT && keyCode != VK_CONTROL && keyCode != VK_MENU &&
        keyCode != VK_LSHIFT && keyCode != VK_RSHIFT &&
        keyCode != VK_LCONTROL && keyCode != VK_RCONTROL &&
        keyCode != VK_LMENU && keyCode != VK_RMENU &&
        keyCode != VK_RETURN && keyCode != VK_ESCAPE)
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
    if (Input::Get())
        Input::Get()->OnMouseMove(x, y);
}

void GameWindowHandler::OnMouseButtonDown(int button, int x, int y)
{
    if (Input::Get())
    {
        Input::Get()->OnMouseMove(x, y);
        Input::Get()->OnMouseDown(button);
    }
}

void GameWindowHandler::OnMouseButtonUp(int button, int x, int y)
{
    if (Input::Get())
    {
        Input::Get()->OnMouseMove(x, y);
        Input::Get()->OnMouseUp(button);
    }
}

void GameWindowHandler::OnMouseWheel(int delta, int x, int y)
{
    if (Input::Get())
    {
        Input::Get()->OnMouseMove(x, y);
        Input::Get()->OnMouseWheel(delta);
    }
}

bool GameWindowHandler::OnCustomMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (!m_pGame)
        return false;

    switch (message)
    {
    case WM_USER_CALCSOCKETEVENT:
        m_pGame->_CalcSocketClosed();
        return true;

    case WM_SETCURSOR:
        SetCursor(nullptr);
        return true;

    case WM_SETFOCUS:
        if (Input::Get())
            Input::Get()->SetWindowActive(true);
        return true;

    case WM_KILLFOCUS:
        if (Input::Get())
            Input::Get()->SetWindowActive(false);
        return true;

    case WM_LBUTTONDBLCLK:
        // Handle double-click as button down for manual detection
        if (Input::Get())
        {
            Input::Get()->OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            Input::Get()->OnMouseDown(MOUSE_BUTTON_LEFT);
        }
        return true;
    }

    return false;
}

bool GameWindowHandler::OnTextInput(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (m_pGame)
    {
        return m_pGame->GetText(hWnd, message, wParam, lParam) != 0;
    }
    return false;
}
