// SFMLWindow.cpp: Pure SFML window implementing hb::shared::render::IWindow interface
//
// Part of SFMLEngine static library
//////////////////////////////////////////////////////////////////////

#include "SFMLWindow.h"
#include "SFMLInput.h"
#include "SFMLRenderer.h"
#include "RendererFactory.h"
#include "RenderConstants.h"
#include <SFML/Window/Event.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

namespace MouseButton = hb::shared::input::MouseButton;

SFMLWindow::SFMLWindow()
    : m_hWnd(0)
    , m_pEventHandler(nullptr)
    , m_width(0)
    , m_height(0)
    , m_fullscreen(false)
    , m_bFullscreenStretch(false)
    , m_borderless(true)
    , m_bMouseCaptureEnabled(true)
    , m_bVSync(false)
    , m_iFpsLimit(0)
    , m_windowedWidth(0)
    , m_windowedHeight(0)
    , m_active(true)
    , m_open(false)
{
}

SFMLWindow::~SFMLWindow()
{
    Destroy();
}

bool SFMLWindow::Create(const hb::shared::render::WindowParams& params)
{
    if (m_open)
        return false;

    m_width = params.width;
    m_height = params.height;
    m_windowedWidth = params.width;
    m_windowedHeight = params.height;
    m_fullscreen = params.fullscreen;
    m_borderless = params.borderless;
    m_bMouseCaptureEnabled = params.mouseCaptureEnabled;

    // Create SFML window
    sf::VideoMode videoMode({static_cast<unsigned int>(m_width), static_cast<unsigned int>(m_height)});

    sf::State state = params.fullscreen ? sf::State::Fullscreen : sf::State::Windowed;

    // Pick style: fullscreen/borderless use None, bordered uses Titlebar
    auto sfStyle = sf::Style::None;
    if (!params.fullscreen && !m_borderless)
    {
        sfStyle = sf::Style::Titlebar;
    }

    m_renderWindow.create(videoMode, params.title ? params.title : "hb::shared::render::Window", sfStyle, state);

    if (!m_renderWindow.isOpen())
        return false;

    // Get native handle for anything that needs it
#ifdef _WIN32
    m_hWnd = static_cast<HWND>(m_renderWindow.getNativeHandle());

    // For bordered mode, add minimize button (sf::Style::Titlebar doesn't include it)
    if (!params.fullscreen && !m_borderless && m_hWnd)
    {
        LONG style = GetWindowLong(m_hWnd, GWL_STYLE);
        style |= WS_MINIMIZEBOX;
        SetWindowLong(m_hWnd, GWL_STYLE, style);
        SetWindowPos(m_hWnd, nullptr, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
#endif



    // Hide the system mouse cursor (game draws its own cursor)
    m_renderWindow.setMouseCursorVisible(false);
    m_renderWindow.setMouseCursorGrabbed(m_bMouseCaptureEnabled);

    // Request 1ms timer resolution for accurate sleep-based frame limiting
#ifdef _WIN32
    timeBeginPeriod(1);
#endif

    m_open = true;
    m_active = true;

    return true;
}

void SFMLWindow::Destroy()
{
    m_pEventHandler = nullptr;

    if (m_renderWindow.isOpen())
    {
        m_renderWindow.close();
    }

    // Restore default timer resolution
#ifdef _WIN32
    timeEndPeriod(1);
#endif

    m_hWnd = 0;
    m_open = false;
    m_active = false;
}

bool SFMLWindow::IsOpen() const
{
    return m_open && m_renderWindow.isOpen();
}

void SFMLWindow::Close()
{
    // Don't call m_pEventHandler->OnClose() here — that creates infinite recursion
    // since OnClose() may call hb::shared::render::Window::Close(). OnClose() is only called from the
    // SFML event loop when the user clicks the close button.
    m_open = false;
    m_renderWindow.close();
}

hb::shared::types::NativeWindowHandle SFMLWindow::GetHandle() const
{
    return m_hWnd;
}

int SFMLWindow::GetWidth() const
{
    return m_width;
}

int SFMLWindow::GetHeight() const
{
    return m_height;
}

bool SFMLWindow::IsFullscreen() const
{
    return m_fullscreen;
}

bool SFMLWindow::IsActive() const
{
    return m_active;
}

void SFMLWindow::SetFullscreen(bool fullscreen)
{
    if (m_fullscreen == fullscreen)
        return;

    m_fullscreen = fullscreen;

    // Get display dimensions based on mode
    int windowWidth, windowHeight;
    if (fullscreen)
    {
        // Save windowed dimensions before going fullscreen
        m_windowedWidth = m_width;
        m_windowedHeight = m_height;
        windowWidth = static_cast<int>(sf::VideoMode::getDesktopMode().size.x);
        windowHeight = static_cast<int>(sf::VideoMode::getDesktopMode().size.y);
    }
    else
    {
        // Restore saved windowed dimensions
        windowWidth = m_windowedWidth > 0 ? m_windowedWidth : LOGICAL_WIDTH();
        windowHeight = m_windowedHeight > 0 ? m_windowedHeight : LOGICAL_HEIGHT();
    }

    // Recreate window with new mode
    sf::VideoMode videoMode({static_cast<unsigned int>(windowWidth), static_cast<unsigned int>(windowHeight)});
    sf::State state = fullscreen ? sf::State::Fullscreen : sf::State::Windowed;

    // Pick style based on borderless setting (fullscreen always None)
    auto sfStyle = sf::Style::None;
    if (!fullscreen && !m_borderless)
    {
        sfStyle = sf::Style::Titlebar;
    }

    m_renderWindow.create(videoMode, "Helbreath", sfStyle, state);

    // Update stored dimensions
    m_width = windowWidth;
    m_height = windowHeight;

#ifdef _WIN32
    m_hWnd = static_cast<HWND>(m_renderWindow.getNativeHandle());

    // For bordered windowed mode, add minimize button
    if (!fullscreen && !m_borderless && m_hWnd)
    {
        LONG style = GetWindowLong(m_hWnd, GWL_STYLE);
        style |= WS_MINIMIZEBOX;
        SetWindowLong(m_hWnd, GWL_STYLE, style);
        SetWindowPos(m_hWnd, nullptr, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
#endif



    // Reapply cursor settings
    m_renderWindow.setMouseCursorVisible(false);
    m_renderWindow.setMouseCursorGrabbed(m_bMouseCaptureEnabled);

    // If switching to windowed mode, center the window
    if (!fullscreen)
    {
        int screenWidth = static_cast<int>(sf::VideoMode::getDesktopMode().size.x);
        int screenHeight = static_cast<int>(sf::VideoMode::getDesktopMode().size.y);
        int newX = (screenWidth - windowWidth) / 2;
        int newY = (screenHeight - windowHeight) / 2;
#ifdef _WIN32
        SetWindowPos(m_hWnd, HWND_TOP, newX, newY, windowWidth, windowHeight, SWP_SHOWWINDOW);
#else
        m_renderWindow.setPosition(sf::Vector2i(newX, newY));
#endif
    }
}

void SFMLWindow::SetBorderless(bool borderless)
{
    if (m_borderless == borderless || m_fullscreen)
        return;

    m_borderless = borderless;

    // Recreate window with new style (same pattern as SetFullscreen)
    sf::VideoMode videoMode({static_cast<unsigned int>(m_width), static_cast<unsigned int>(m_height)});

    auto sfStyle = borderless ? sf::Style::None : sf::Style::Titlebar;

    m_renderWindow.create(videoMode, "Helbreath", sfStyle, sf::State::Windowed);

#ifdef _WIN32
    m_hWnd = static_cast<HWND>(m_renderWindow.getNativeHandle());

    // For bordered mode, add minimize button
    if (!borderless && m_hWnd)
    {
        LONG style = GetWindowLong(m_hWnd, GWL_STYLE);
        style |= WS_MINIMIZEBOX;
        SetWindowLong(m_hWnd, GWL_STYLE, style);
        SetWindowPos(m_hWnd, nullptr, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
#endif


    m_renderWindow.setMouseCursorVisible(false);
    m_renderWindow.setMouseCursorGrabbed(m_bMouseCaptureEnabled);

    // Center the window
    {
        int screenWidth = static_cast<int>(sf::VideoMode::getDesktopMode().size.x);
        int screenHeight = static_cast<int>(sf::VideoMode::getDesktopMode().size.y);
        int posX = (screenWidth - m_width) / 2;
        int posY = (screenHeight - m_height) / 2;
#ifdef _WIN32
        SetWindowPos(m_hWnd, HWND_TOP, posX, posY, m_width, m_height, SWP_SHOWWINDOW);
#else
        m_renderWindow.setPosition(sf::Vector2i(posX, posY));
#endif
    }

    // Update input system with new window handle
    if (hb::shared::input::Get())
    {
        SFMLInput* pInput = static_cast<SFMLInput*>(hb::shared::input::Get());
        pInput->Initialize(m_hWnd);
        pInput->SetRenderWindow(&m_renderWindow);
    }

    // Update renderer with new render window
    hb::shared::render::IRenderer* pRenderer = hb::shared::render::Renderer::Get();
    if (pRenderer)
    {
        static_cast<SFMLRenderer*>(pRenderer)->SetRenderWindow(&m_renderWindow);
    }
}

bool SFMLWindow::IsBorderless() const
{
    return m_borderless;
}

void SFMLWindow::SetSize(int width, int height, bool center)
{
    if (width <= 0 || height <= 0)
        return;

    m_width = width;
    m_height = height;

    // Update the SFML window size
    m_renderWindow.setSize({static_cast<unsigned int>(width), static_cast<unsigned int>(height)});

    // Update window position/size
    if (center)
    {
        int screenWidth = static_cast<int>(sf::VideoMode::getDesktopMode().size.x);
        int screenHeight = static_cast<int>(sf::VideoMode::getDesktopMode().size.y);
        int posX = (screenWidth - width) / 2;
        int posY = (screenHeight - height) / 2;
#ifdef _WIN32
        if (m_hWnd)
            SetWindowPos(m_hWnd, HWND_TOP, posX, posY, width, height, SWP_SHOWWINDOW);
#else
        m_renderWindow.setPosition(sf::Vector2i(posX, posY));
#endif
    }
    else
    {
#ifdef _WIN32
        // Just resize, keep position
        if (m_hWnd)
            SetWindowPos(m_hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_SHOWWINDOW);
#endif
    }

    // Re-apply mouse cursor grab to update grab boundaries to new window size
    m_renderWindow.setMouseCursorGrabbed(false);
    m_renderWindow.setMouseCursorGrabbed(m_bMouseCaptureEnabled);
}

void SFMLWindow::Show()
{
    m_renderWindow.setVisible(true);
    m_renderWindow.requestFocus();
#ifdef _WIN32
    // On Windows, also force foreground window to ensure focus
    if (m_hWnd)
    {
        SetForegroundWindow(m_hWnd);
        SetFocus(m_hWnd);
    }
#endif
}

void SFMLWindow::Hide()
{
    m_renderWindow.setVisible(false);
}

void SFMLWindow::SetTitle(const char* title)
{
    if (title)
    {
        m_renderWindow.setTitle(title);
    }
}

void SFMLWindow::SetFramerateLimit(int limit)
{
    m_iFpsLimit = limit;
    // When VSync is active, the monitor refresh rate controls timing —
    // don't let an FPS limit override it. The saved m_iFpsLimit will be
    // applied when VSync is turned off (see SetVSyncEnabled).
    if (m_bVSync) return;
    // Forward to renderer for engine-owned frame limiting
    hb::shared::render::IRenderer* pRenderer = hb::shared::render::Renderer::Get();
    if (pRenderer)
        static_cast<SFMLRenderer*>(pRenderer)->SetFramerateLimit(limit);
}

int SFMLWindow::GetFramerateLimit() const
{
    return m_iFpsLimit;
}

void SFMLWindow::SetVSyncEnabled(bool enabled)
{
    m_bVSync = enabled;
    hb::shared::render::IRenderer* pRenderer = hb::shared::render::Renderer::Get();
    if (!pRenderer) return;

    auto* pSFMLRenderer = static_cast<SFMLRenderer*>(pRenderer);

    if (enabled)
    {
        // Query the monitor's refresh rate and use it as the FPS target
        // This avoids SFML's blocking display() and keeps our engine-owned timing
        int refreshRate = 60; // Safe fallback
#ifdef _WIN32
        DEVMODE devMode = {};
        devMode.dmSize = sizeof(devMode);
        if (EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode))
        {
            if (devMode.dmDisplayFrequency > 0)
                refreshRate = static_cast<int>(devMode.dmDisplayFrequency);
        }
#endif
        pSFMLRenderer->SetVSyncMode(true);
        pSFMLRenderer->SetFramerateLimit(refreshRate);
    }
    else
    {
        pSFMLRenderer->SetVSyncMode(false);
        // Restore the user's FPS limit setting
        pSFMLRenderer->SetFramerateLimit(m_iFpsLimit);
    }
}

bool SFMLWindow::IsVSyncEnabled() const
{
    return m_bVSync;
}

void SFMLWindow::SetFullscreenStretch(bool stretch)
{
    m_bFullscreenStretch = stretch;
}

bool SFMLWindow::IsFullscreenStretch() const
{
    return m_bFullscreenStretch;
}

void SFMLWindow::SetMouseCursorVisible(bool visible)
{
    m_renderWindow.setMouseCursorVisible(visible);
}

void SFMLWindow::SetMouseCaptureEnabled(bool enabled)
{
    m_bMouseCaptureEnabled = enabled;
    m_renderWindow.setMouseCursorGrabbed(enabled);
}

void SFMLWindow::ShowMessageBox(const char* title, const char* message)
{
#ifdef _WIN32
    MessageBox(m_hWnd, message, title, MB_ICONEXCLAMATION | MB_OK);
#else
    fprintf(stderr, "%s: %s\n", title, message);
#endif
}

bool SFMLWindow::ProcessMessages()
{
    // Check if window was closed programmatically (via Close() method)
    if (!m_open || !m_renderWindow.isOpen())
        return false;

    while (const std::optional<sf::Event> event = m_renderWindow.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            if (m_pEventHandler)
                m_pEventHandler->OnClose();
            m_open = false;
            return false;
        }

        if (const auto* resized = event->getIf<sf::Event::Resized>())
        {
            m_width = static_cast<int>(resized->size.x);
            m_height = static_cast<int>(resized->size.y);
            if (m_pEventHandler)
                m_pEventHandler->OnResize(m_width, m_height);
        }

        if (const auto* focusGained = event->getIf<sf::Event::FocusGained>())
        {
            m_active = true;
            // Reactivate OpenGL context when focus is regained
            (void)m_renderWindow.setActive(true);
            // Re-grab mouse cursor if capture is enabled
            m_renderWindow.setMouseCursorGrabbed(m_bMouseCaptureEnabled);
            if (m_pEventHandler)
                m_pEventHandler->OnActivate(true);
        }

        if (const auto* focusLost = event->getIf<sf::Event::FocusLost>())
        {
            m_active = false;
            if (m_pEventHandler)
                m_pEventHandler->OnActivate(false);
        }

        if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            if (m_pEventHandler)
                m_pEventHandler->OnKeyDown(SfmlKeyToKeyCode(keyPressed->code));
        }

        if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
        {
            if (m_pEventHandler)
                m_pEventHandler->OnKeyUp(SfmlKeyToKeyCode(keyReleased->code));
        }

        if (const auto* textEntered = event->getIf<sf::Event::TextEntered>())
        {
            if (m_pEventHandler)
            {
                // Call OnTextInput with WM_CHAR-style parameters
                // The game's GetText() function expects Win32 message format
                m_pEventHandler->OnTextInput(m_hWnd, 0x0102 /*WM_CHAR*/,
                    static_cast<uintptr_t>(textEntered->unicode), 0);
            }
        }

        if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>())
        {
            if (m_pEventHandler)
            {
                int logicalX, logicalY;
                TransformMouseCoords(mouseMoved->position.x, mouseMoved->position.y, logicalX, logicalY);
                m_pEventHandler->OnMouseMove(logicalX, logicalY);
            }
        }

        if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
        {
            int button = MouseButton::Left;
            if (mousePressed->button == sf::Mouse::Button::Right)
                button = MouseButton::Right;
            else if (mousePressed->button == sf::Mouse::Button::Middle)
                button = MouseButton::Middle;

            if (m_pEventHandler)
            {
                int logicalX, logicalY;
                TransformMouseCoords(mousePressed->position.x, mousePressed->position.y, logicalX, logicalY);
                m_pEventHandler->OnMouseButtonDown(button, logicalX, logicalY);
            }
        }

        if (const auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>())
        {
            int button = MouseButton::Left;
            if (mouseReleased->button == sf::Mouse::Button::Right)
                button = MouseButton::Right;
            else if (mouseReleased->button == sf::Mouse::Button::Middle)
                button = MouseButton::Middle;

            if (m_pEventHandler)
            {
                int logicalX, logicalY;
                TransformMouseCoords(mouseReleased->position.x, mouseReleased->position.y, logicalX, logicalY);
                m_pEventHandler->OnMouseButtonUp(button, logicalX, logicalY);
            }
        }

        if (const auto* mouseWheel = event->getIf<sf::Event::MouseWheelScrolled>())
        {
            if (m_pEventHandler)
            {
                int logicalX, logicalY;
                TransformMouseCoords(mouseWheel->position.x, mouseWheel->position.y, logicalX, logicalY);
                int delta = static_cast<int>(mouseWheel->delta * 120);  // Convert to Windows-style delta
                m_pEventHandler->OnMouseWheel(delta, logicalX, logicalY);
            }
        }
    }

    return true;
}

void SFMLWindow::WaitForMessage()
{
    // Wait for an event and process it (don't discard it)
    // This is critical for handling FocusGained when the window is inactive
    if (const std::optional<sf::Event> event = m_renderWindow.waitEvent())
    {
        // Process the event we received (same logic as ProcessMessages)
        if (event->is<sf::Event::Closed>())
        {
            if (m_pEventHandler)
                m_pEventHandler->OnClose();
            m_open = false;
            return;
        }

        if (const auto* focusGained = event->getIf<sf::Event::FocusGained>())
        {
            m_active = true;
            (void)m_renderWindow.setActive(true);
            m_renderWindow.setMouseCursorGrabbed(m_bMouseCaptureEnabled);
            if (m_pEventHandler)
                m_pEventHandler->OnActivate(true);
        }

        if (const auto* focusLost = event->getIf<sf::Event::FocusLost>())
        {
            m_active = false;
            if (m_pEventHandler)
                m_pEventHandler->OnActivate(false);
        }

        // Other events will be handled in the next ProcessMessages() call
        // but focus events are critical to handle here for proper resume
    }
}

void SFMLWindow::SetEventHandler(hb::shared::render::IWindowEventHandler* handler)
{
    m_pEventHandler = handler;
}

hb::shared::render::IWindowEventHandler* SFMLWindow::GetEventHandler() const
{
    return m_pEventHandler;
}

// Convert SFML key codes to abstract KeyCode enum values
KeyCode SFMLWindow::SfmlKeyToKeyCode(sf::Keyboard::Key key)
{
    switch (key)
    {
    // Letters
    case sf::Keyboard::Key::A: return KeyCode::A;
    case sf::Keyboard::Key::B: return KeyCode::B;
    case sf::Keyboard::Key::C: return KeyCode::C;
    case sf::Keyboard::Key::D: return KeyCode::D;
    case sf::Keyboard::Key::E: return KeyCode::E;
    case sf::Keyboard::Key::F: return KeyCode::F;
    case sf::Keyboard::Key::G: return KeyCode::G;
    case sf::Keyboard::Key::H: return KeyCode::H;
    case sf::Keyboard::Key::I: return KeyCode::I;
    case sf::Keyboard::Key::J: return KeyCode::J;
    case sf::Keyboard::Key::K: return KeyCode::K;
    case sf::Keyboard::Key::L: return KeyCode::L;
    case sf::Keyboard::Key::M: return KeyCode::M;
    case sf::Keyboard::Key::N: return KeyCode::N;
    case sf::Keyboard::Key::O: return KeyCode::O;
    case sf::Keyboard::Key::P: return KeyCode::P;
    case sf::Keyboard::Key::Q: return KeyCode::Q;
    case sf::Keyboard::Key::R: return KeyCode::R;
    case sf::Keyboard::Key::S: return KeyCode::S;
    case sf::Keyboard::Key::T: return KeyCode::T;
    case sf::Keyboard::Key::U: return KeyCode::U;
    case sf::Keyboard::Key::V: return KeyCode::V;
    case sf::Keyboard::Key::W: return KeyCode::W;
    case sf::Keyboard::Key::X: return KeyCode::X;
    case sf::Keyboard::Key::Y: return KeyCode::Y;
    case sf::Keyboard::Key::Z: return KeyCode::Z;

    // Numbers
    case sf::Keyboard::Key::Num0: return KeyCode::Num0;
    case sf::Keyboard::Key::Num1: return KeyCode::Num1;
    case sf::Keyboard::Key::Num2: return KeyCode::Num2;
    case sf::Keyboard::Key::Num3: return KeyCode::Num3;
    case sf::Keyboard::Key::Num4: return KeyCode::Num4;
    case sf::Keyboard::Key::Num5: return KeyCode::Num5;
    case sf::Keyboard::Key::Num6: return KeyCode::Num6;
    case sf::Keyboard::Key::Num7: return KeyCode::Num7;
    case sf::Keyboard::Key::Num8: return KeyCode::Num8;
    case sf::Keyboard::Key::Num9: return KeyCode::Num9;

    // Special keys
    case sf::Keyboard::Key::Escape: return KeyCode::Escape;
    case sf::Keyboard::Key::Space: return KeyCode::Space;
    case sf::Keyboard::Key::Enter: return KeyCode::Enter;
    case sf::Keyboard::Key::Backspace: return KeyCode::Backspace;
    case sf::Keyboard::Key::Tab: return KeyCode::Tab;

    // Modifiers
    case sf::Keyboard::Key::LControl: return KeyCode::LControl;
    case sf::Keyboard::Key::LShift: return KeyCode::LShift;
    case sf::Keyboard::Key::LAlt: return KeyCode::LAlt;
    case sf::Keyboard::Key::RControl: return KeyCode::RControl;
    case sf::Keyboard::Key::RShift: return KeyCode::RShift;
    case sf::Keyboard::Key::RAlt: return KeyCode::RAlt;

    // Navigation
    case sf::Keyboard::Key::PageUp: return KeyCode::PageUp;
    case sf::Keyboard::Key::PageDown: return KeyCode::PageDown;
    case sf::Keyboard::Key::End: return KeyCode::End;
    case sf::Keyboard::Key::Home: return KeyCode::Home;
    case sf::Keyboard::Key::Insert: return KeyCode::Insert;
    case sf::Keyboard::Key::Delete: return KeyCode::Delete;

    // Arrow keys
    case sf::Keyboard::Key::Left: return KeyCode::Left;
    case sf::Keyboard::Key::Right: return KeyCode::Right;
    case sf::Keyboard::Key::Up: return KeyCode::Up;
    case sf::Keyboard::Key::Down: return KeyCode::Down;

    // Function keys
    case sf::Keyboard::Key::F1: return KeyCode::F1;
    case sf::Keyboard::Key::F2: return KeyCode::F2;
    case sf::Keyboard::Key::F3: return KeyCode::F3;
    case sf::Keyboard::Key::F4: return KeyCode::F4;
    case sf::Keyboard::Key::F5: return KeyCode::F5;
    case sf::Keyboard::Key::F6: return KeyCode::F6;
    case sf::Keyboard::Key::F7: return KeyCode::F7;
    case sf::Keyboard::Key::F8: return KeyCode::F8;
    case sf::Keyboard::Key::F9: return KeyCode::F9;
    case sf::Keyboard::Key::F10: return KeyCode::F10;
    case sf::Keyboard::Key::F11: return KeyCode::F11;
    case sf::Keyboard::Key::F12: return KeyCode::F12;

    // Numpad numbers
    case sf::Keyboard::Key::Numpad0: return KeyCode::Numpad0;
    case sf::Keyboard::Key::Numpad1: return KeyCode::Numpad1;
    case sf::Keyboard::Key::Numpad2: return KeyCode::Numpad2;
    case sf::Keyboard::Key::Numpad3: return KeyCode::Numpad3;
    case sf::Keyboard::Key::Numpad4: return KeyCode::Numpad4;
    case sf::Keyboard::Key::Numpad5: return KeyCode::Numpad5;
    case sf::Keyboard::Key::Numpad6: return KeyCode::Numpad6;
    case sf::Keyboard::Key::Numpad7: return KeyCode::Numpad7;
    case sf::Keyboard::Key::Numpad8: return KeyCode::Numpad8;
    case sf::Keyboard::Key::Numpad9: return KeyCode::Numpad9;

    // Numpad operators
    case sf::Keyboard::Key::Add: return KeyCode::NumpadAdd;
    case sf::Keyboard::Key::Subtract: return KeyCode::NumpadSubtract;
    case sf::Keyboard::Key::Multiply: return KeyCode::NumpadMultiply;
    case sf::Keyboard::Key::Divide: return KeyCode::NumpadDivide;

    // OEM keys
    case sf::Keyboard::Key::Grave: return KeyCode::Grave;

    default: return KeyCode::Unknown;
    }
}

void SFMLWindow::TransformMouseCoords(int windowX, int windowY, int& logicalX, int& logicalY) const
{
    // Get actual window size in physical pixels
    float windowWidth, windowHeight;

#ifdef _WIN32
    if (m_hWnd)
    {
        RECT clientRect;
        if (GetClientRect(m_hWnd, &clientRect))
        {
            windowWidth = static_cast<float>(clientRect.right - clientRect.left);
            windowHeight = static_cast<float>(clientRect.bottom - clientRect.top);
        }
        else
        {
            windowWidth = static_cast<float>(m_width);
            windowHeight = static_cast<float>(m_height);
        }
    }
    else
#endif
    {
        windowWidth = static_cast<float>(m_width);
        windowHeight = static_cast<float>(m_height);
    }

    float mouseX = static_cast<float>(windowX);
    float mouseY = static_cast<float>(windowY);

    if (m_fullscreen && !m_bFullscreenStretch)
    {
        // Fullscreen letterbox - account for uniform scale and offset
        float scaleX = windowWidth / static_cast<float>(RENDER_LOGICAL_WIDTH());
        float scaleY = windowHeight / static_cast<float>(RENDER_LOGICAL_HEIGHT());
        float scale = (scaleY < scaleX) ? scaleY : scaleX;

        float destWidth = static_cast<float>(RENDER_LOGICAL_WIDTH()) * scale;
        float destHeight = static_cast<float>(RENDER_LOGICAL_HEIGHT()) * scale;
        float offsetX = (windowWidth - destWidth) / 2.0f;
        float offsetY = (windowHeight - destHeight) / 2.0f;

        logicalX = static_cast<int>((mouseX - offsetX) / scale);
        logicalY = static_cast<int>((mouseY - offsetY) / scale);
    }
    else
    {
        // Windowed or fullscreen stretch - independent axis scaling
        float scaleX = windowWidth / static_cast<float>(RENDER_LOGICAL_WIDTH());
        float scaleY = windowHeight / static_cast<float>(RENDER_LOGICAL_HEIGHT());

        logicalX = static_cast<int>(mouseX / scaleX);
        logicalY = static_cast<int>(mouseY / scaleY);
    }

    // Clamp to valid range
    if (logicalX < 0) logicalX = 0;
    if (logicalX > LOGICAL_MAX_X()) logicalX = LOGICAL_MAX_X();
    if (logicalY < 0) logicalY = 0;
    if (logicalY > LOGICAL_MAX_Y()) logicalY = LOGICAL_MAX_Y();
}
