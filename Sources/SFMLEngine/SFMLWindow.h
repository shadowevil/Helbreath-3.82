// SFMLWindow.h: Pure SFML window implementing IWindow interface
//
// Part of SFMLEngine static library
//////////////////////////////////////////////////////////////////////

#pragma once

#include "IWindow.h"
#include "IInput.h"  // For KeyCode enum
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>

class SFMLWindow : public IWindow
{
public:
    SFMLWindow();
    virtual ~SFMLWindow();

    // ============== IWindow Implementation ==============

    // Lifecycle
    bool Create(const WindowParams& params) override;
    void Destroy() override;
    bool IsOpen() const override;
    void Close() override;

    // Properties
    HWND GetHandle() const override;
    int GetWidth() const override;
    int GetHeight() const override;
    bool IsFullscreen() const override;
    bool IsActive() const override;

    // Display
    void SetFullscreen(bool fullscreen) override;
    void SetSize(int width, int height, bool center = true) override;
    void Show() override;
    void Hide() override;
    void SetTitle(const char* title) override;

    // Message Processing
    bool ProcessMessages() override;
    void WaitForMessage() override;

    // Event Handler
    void SetEventHandler(IWindowEventHandler* handler) override;
    IWindowEventHandler* GetEventHandler() const override;

    // ============== SFML-Specific Access ==============

    sf::RenderWindow* GetRenderWindow() { return &m_renderWindow; }
    const sf::RenderWindow* GetRenderWindow() const { return &m_renderWindow; }

private:
    // Convert SFML key to abstract KeyCode
    static int SfmlKeyToKeyCode(sf::Keyboard::Key key);

    // Transform window coordinates to logical game coordinates (640x480)
    void TransformMouseCoords(int windowX, int windowY, int& logicalX, int& logicalY) const;

    sf::RenderWindow m_renderWindow;
    HWND m_hWnd;  // Native handle for compatibility
    IWindowEventHandler* m_pEventHandler;
    int m_width;
    int m_height;
    bool m_fullscreen;
    bool m_resizable;
    bool m_active;
    bool m_open;
};
