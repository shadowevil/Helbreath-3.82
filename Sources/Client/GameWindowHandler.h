// GameWindowHandler.h: Window event handler adapter for CGame
//
// Implements IWindowEventHandler and routes events to CGame methods
//////////////////////////////////////////////////////////////////////

#pragma once

#include "IWindow.h"

// Forward declarations
class CGame;

class GameWindowHandler : public IWindowEventHandler
{
public:
    GameWindowHandler(CGame* pGame);
    virtual ~GameWindowHandler() = default;

    // ============== IWindowEventHandler Implementation ==============

    // Window Events
    virtual bool OnClose() override;
    virtual void OnDestroy() override;
    virtual void OnActivate(bool active) override;
    virtual void OnResize(int width, int height) override;

    // Input Events
    virtual void OnKeyDown(int keyCode) override;
    virtual void OnKeyUp(int keyCode) override;
    virtual void OnChar(char character) override;
    virtual void OnMouseMove(int x, int y) override;
    virtual void OnMouseButtonDown(int button, int x, int y) override;
    virtual void OnMouseButtonUp(int button, int x, int y) override;
    virtual void OnMouseWheel(int delta, int x, int y) override;

    // Custom Messages
    virtual bool OnCustomMessage(UINT message, WPARAM wParam, LPARAM lParam) override;

    // Text Input
    virtual bool OnTextInput(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;

private:
    CGame* m_pGame;
};
