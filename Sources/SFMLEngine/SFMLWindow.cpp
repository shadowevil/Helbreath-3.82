// SFMLWindow.cpp: Pure SFML window implementing IWindow interface
//
// Part of SFMLEngine static library
//////////////////////////////////////////////////////////////////////

#include "SFMLWindow.h"
#include "ConfigManager.h"
#include "RenderConstants.h"
#include <SFML/Window/Event.hpp>

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>  // For GET_X_LPARAM, GET_Y_LPARAM
#endif

SFMLWindow::SFMLWindow()
    : m_hWnd(nullptr)
    , m_pEventHandler(nullptr)
    , m_width(0)
    , m_height(0)
    , m_fullscreen(false)
    , m_resizable(false)
    , m_active(true)
    , m_open(false)
{
}

SFMLWindow::~SFMLWindow()
{
    Destroy();
}

bool SFMLWindow::Create(const WindowParams& params)
{
    if (m_open)
        return false;

    m_width = params.width;
    m_height = params.height;
    m_fullscreen = params.fullscreen;
    m_resizable = params.resizable;

    // Create SFML window
    sf::VideoMode videoMode({static_cast<unsigned int>(m_width), static_cast<unsigned int>(m_height)});

    if (params.fullscreen)
    {
        // Fullscreen mode
        m_renderWindow.create(videoMode, params.title ? params.title : "Window", 
                              sf::Style::None, sf::State::Fullscreen);
    }
    else if (m_resizable)
    {
        // Windowed with standard decorations (title bar, min/max/close, resize)
        m_renderWindow.create(videoMode, params.title ? params.title : "Window",
                              sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
    }
    else
    {
        // Borderless window (original behavior)
        m_renderWindow.create(videoMode, params.title ? params.title : "Window", sf::Style::None);
    }

    if (!m_renderWindow.isOpen())
        return false;

    // Get native handle for anything that needs it
#ifdef _WIN32
    m_hWnd = static_cast<HWND>(m_renderWindow.getNativeHandle());
#endif

    // Disable VSync by default (game has its own frame timing)
    m_renderWindow.setVerticalSyncEnabled(false);

    // Hide the system mouse cursor (game draws its own cursor)
    // Note: Do NOT use setMouseCursorGrabbed as it interferes with borderless resize
    m_renderWindow.setMouseCursorVisible(false);

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

    m_hWnd = nullptr;
    m_open = false;
    m_active = false;
}

bool SFMLWindow::IsOpen() const
{
    return m_open && m_renderWindow.isOpen();
}

void SFMLWindow::Close()
{
    bool shouldClose = true;
    if (m_pEventHandler)
        shouldClose = m_pEventHandler->OnClose();
    
    if (shouldClose)
    {
        m_open = false;
        m_renderWindow.close();
    }
}

HWND SFMLWindow::GetHandle() const
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
        // Use screen resolution for fullscreen
        windowWidth = GetSystemMetrics(SM_CXSCREEN);
        windowHeight = GetSystemMetrics(SM_CYSCREEN);
    }
    else
    {
        // Use configured window size for windowed mode
        windowWidth = ConfigManager::Get().GetWindowWidth();
        windowHeight = ConfigManager::Get().GetWindowHeight();
        if (windowWidth <= 0) windowWidth = LOGICAL_WIDTH;
        if (windowHeight <= 0) windowHeight = LOGICAL_HEIGHT;
    }

    // Recreate window with new mode
    sf::VideoMode videoMode({static_cast<unsigned int>(windowWidth), static_cast<unsigned int>(windowHeight)});
    sf::State state = fullscreen ? sf::State::Fullscreen : sf::State::Windowed;

    m_renderWindow.create(videoMode, "Helbreath", sf::Style::None, state);

    // Update stored dimensions
    m_width = windowWidth;
    m_height = windowHeight;

#ifdef _WIN32
    m_hWnd = static_cast<HWND>(m_renderWindow.getNativeHandle());
#endif

    // Disable VSync (game has its own frame timing)
    m_renderWindow.setVerticalSyncEnabled(false);

    // Hide the system mouse cursor (game draws its own cursor)
    // Also grab the cursor to ensure mouse events are tracked even when hidden
    m_renderWindow.setMouseCursorVisible(false);
    m_renderWindow.setMouseCursorGrabbed(true);

    // If switching to windowed mode, center the window
    if (!fullscreen)
    {
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        int newX = (screenWidth - windowWidth) / 2;
        int newY = (screenHeight - windowHeight) / 2;
        SetWindowPos(m_hWnd, HWND_TOP, newX, newY, windowWidth, windowHeight, SWP_SHOWWINDOW);
    }
}

void SFMLWindow::SetSize(int width, int height, bool center)
{
    if (width <= 0 || height <= 0)
        return;

    m_width = width;
    m_height = height;

    // Update the SFML window size
    m_renderWindow.setSize({static_cast<unsigned int>(width), static_cast<unsigned int>(height)});

    // Update Win32 window position/size
    if (m_hWnd)
    {
        int posX = 0, posY = 0;
        if (center)
        {
            int screenWidth = GetSystemMetrics(SM_CXSCREEN);
            int screenHeight = GetSystemMetrics(SM_CYSCREEN);
            posX = (screenWidth - width) / 2;
            posY = (screenHeight - height) / 2;
            SetWindowPos(m_hWnd, HWND_TOP, posX, posY, width, height, SWP_SHOWWINDOW);
        }
        else
        {
            // Just resize, keep position
            SetWindowPos(m_hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_SHOWWINDOW);
        }
    }

    // Re-apply mouse cursor grab to update grab boundaries to new window size
    m_renderWindow.setMouseCursorGrabbed(false);
    m_renderWindow.setMouseCursorGrabbed(true);
}

void SFMLWindow::Show()
{
    m_renderWindow.setVisible(true);
    m_renderWindow.requestFocus();
    // On Windows, also force foreground window to ensure focus
    if (m_hWnd)
    {
        SetForegroundWindow(m_hWnd);
        SetFocus(m_hWnd);
    }
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

bool SFMLWindow::ProcessMessages()
{
    // Check if window was closed programmatically (via Close() method)
    if (!m_open || !m_renderWindow.isOpen())
        return false;

    while (const std::optional<sf::Event> event = m_renderWindow.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            bool shouldClose = true;
            if (m_pEventHandler)
                shouldClose = m_pEventHandler->OnClose();
            
            if (shouldClose)
            {
                m_open = false;
                return false;
            }
            continue;  // Close was cancelled, continue processing events
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
                m_pEventHandler->OnTextInput(m_hWnd, WM_CHAR,
                    static_cast<WPARAM>(textEntered->unicode), 0);
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
            int button = MOUSE_BUTTON_LEFT;
            if (mousePressed->button == sf::Mouse::Button::Right)
                button = MOUSE_BUTTON_RIGHT;
            else if (mousePressed->button == sf::Mouse::Button::Middle)
                button = MOUSE_BUTTON_MIDDLE;

            if (m_pEventHandler)
            {
                int logicalX, logicalY;
                TransformMouseCoords(mousePressed->position.x, mousePressed->position.y, logicalX, logicalY);
                m_pEventHandler->OnMouseButtonDown(button, logicalX, logicalY);
            }
        }

        if (const auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>())
        {
            int button = MOUSE_BUTTON_LEFT;
            if (mouseReleased->button == sf::Mouse::Button::Right)
                button = MOUSE_BUTTON_RIGHT;
            else if (mouseReleased->button == sf::Mouse::Button::Middle)
                button = MOUSE_BUTTON_MIDDLE;

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
            bool shouldClose = true;
            if (m_pEventHandler)
                shouldClose = m_pEventHandler->OnClose();
            
            if (shouldClose)
            {
                m_open = false;
                return;
            }
            // Close was cancelled, continue
        }

        if (const auto* focusGained = event->getIf<sf::Event::FocusGained>())
        {
            m_active = true;
            (void)m_renderWindow.setActive(true);
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

void SFMLWindow::SetEventHandler(IWindowEventHandler* handler)
{
    m_pEventHandler = handler;
}

IWindowEventHandler* SFMLWindow::GetEventHandler() const
{
    return m_pEventHandler;
}

// Convert SFML key codes to abstract KeyCode enum values
int SFMLWindow::SfmlKeyToKeyCode(sf::Keyboard::Key key)
{
    switch (key)
    {
    // Letters
    case sf::Keyboard::Key::A: return static_cast<int>(KeyCode::A);
    case sf::Keyboard::Key::B: return static_cast<int>(KeyCode::B);
    case sf::Keyboard::Key::C: return static_cast<int>(KeyCode::C);
    case sf::Keyboard::Key::D: return static_cast<int>(KeyCode::D);
    case sf::Keyboard::Key::E: return static_cast<int>(KeyCode::E);
    case sf::Keyboard::Key::F: return static_cast<int>(KeyCode::F);
    case sf::Keyboard::Key::G: return static_cast<int>(KeyCode::G);
    case sf::Keyboard::Key::H: return static_cast<int>(KeyCode::H);
    case sf::Keyboard::Key::I: return static_cast<int>(KeyCode::I);
    case sf::Keyboard::Key::J: return static_cast<int>(KeyCode::J);
    case sf::Keyboard::Key::K: return static_cast<int>(KeyCode::K);
    case sf::Keyboard::Key::L: return static_cast<int>(KeyCode::L);
    case sf::Keyboard::Key::M: return static_cast<int>(KeyCode::M);
    case sf::Keyboard::Key::N: return static_cast<int>(KeyCode::N);
    case sf::Keyboard::Key::O: return static_cast<int>(KeyCode::O);
    case sf::Keyboard::Key::P: return static_cast<int>(KeyCode::P);
    case sf::Keyboard::Key::Q: return static_cast<int>(KeyCode::Q);
    case sf::Keyboard::Key::R: return static_cast<int>(KeyCode::R);
    case sf::Keyboard::Key::S: return static_cast<int>(KeyCode::S);
    case sf::Keyboard::Key::T: return static_cast<int>(KeyCode::T);
    case sf::Keyboard::Key::U: return static_cast<int>(KeyCode::U);
    case sf::Keyboard::Key::V: return static_cast<int>(KeyCode::V);
    case sf::Keyboard::Key::W: return static_cast<int>(KeyCode::W);
    case sf::Keyboard::Key::X: return static_cast<int>(KeyCode::X);
    case sf::Keyboard::Key::Y: return static_cast<int>(KeyCode::Y);
    case sf::Keyboard::Key::Z: return static_cast<int>(KeyCode::Z);

    // Numbers
    case sf::Keyboard::Key::Num0: return static_cast<int>(KeyCode::Num0);
    case sf::Keyboard::Key::Num1: return static_cast<int>(KeyCode::Num1);
    case sf::Keyboard::Key::Num2: return static_cast<int>(KeyCode::Num2);
    case sf::Keyboard::Key::Num3: return static_cast<int>(KeyCode::Num3);
    case sf::Keyboard::Key::Num4: return static_cast<int>(KeyCode::Num4);
    case sf::Keyboard::Key::Num5: return static_cast<int>(KeyCode::Num5);
    case sf::Keyboard::Key::Num6: return static_cast<int>(KeyCode::Num6);
    case sf::Keyboard::Key::Num7: return static_cast<int>(KeyCode::Num7);
    case sf::Keyboard::Key::Num8: return static_cast<int>(KeyCode::Num8);
    case sf::Keyboard::Key::Num9: return static_cast<int>(KeyCode::Num9);

    // Special keys
    case sf::Keyboard::Key::Escape: return static_cast<int>(KeyCode::Escape);
    case sf::Keyboard::Key::Space: return static_cast<int>(KeyCode::Space);
    case sf::Keyboard::Key::Enter: return static_cast<int>(KeyCode::Enter);
    case sf::Keyboard::Key::Backspace: return static_cast<int>(KeyCode::Backspace);
    case sf::Keyboard::Key::Tab: return static_cast<int>(KeyCode::Tab);

    // Modifiers
    case sf::Keyboard::Key::LControl: return static_cast<int>(KeyCode::LControl);
    case sf::Keyboard::Key::LShift: return static_cast<int>(KeyCode::LShift);
    case sf::Keyboard::Key::LAlt: return static_cast<int>(KeyCode::LAlt);
    case sf::Keyboard::Key::RControl: return static_cast<int>(KeyCode::RControl);
    case sf::Keyboard::Key::RShift: return static_cast<int>(KeyCode::RShift);
    case sf::Keyboard::Key::RAlt: return static_cast<int>(KeyCode::RAlt);

    // Navigation
    case sf::Keyboard::Key::PageUp: return static_cast<int>(KeyCode::PageUp);
    case sf::Keyboard::Key::PageDown: return static_cast<int>(KeyCode::PageDown);
    case sf::Keyboard::Key::End: return static_cast<int>(KeyCode::End);
    case sf::Keyboard::Key::Home: return static_cast<int>(KeyCode::Home);
    case sf::Keyboard::Key::Insert: return static_cast<int>(KeyCode::Insert);
    case sf::Keyboard::Key::Delete: return static_cast<int>(KeyCode::Delete);

    // Arrow keys
    case sf::Keyboard::Key::Left: return static_cast<int>(KeyCode::Left);
    case sf::Keyboard::Key::Right: return static_cast<int>(KeyCode::Right);
    case sf::Keyboard::Key::Up: return static_cast<int>(KeyCode::Up);
    case sf::Keyboard::Key::Down: return static_cast<int>(KeyCode::Down);

    // Function keys
    case sf::Keyboard::Key::F1: return static_cast<int>(KeyCode::F1);
    case sf::Keyboard::Key::F2: return static_cast<int>(KeyCode::F2);
    case sf::Keyboard::Key::F3: return static_cast<int>(KeyCode::F3);
    case sf::Keyboard::Key::F4: return static_cast<int>(KeyCode::F4);
    case sf::Keyboard::Key::F5: return static_cast<int>(KeyCode::F5);
    case sf::Keyboard::Key::F6: return static_cast<int>(KeyCode::F6);
    case sf::Keyboard::Key::F7: return static_cast<int>(KeyCode::F7);
    case sf::Keyboard::Key::F8: return static_cast<int>(KeyCode::F8);
    case sf::Keyboard::Key::F9: return static_cast<int>(KeyCode::F9);
    case sf::Keyboard::Key::F10: return static_cast<int>(KeyCode::F10);
    case sf::Keyboard::Key::F11: return static_cast<int>(KeyCode::F11);
    case sf::Keyboard::Key::F12: return static_cast<int>(KeyCode::F12);

    // Numpad numbers
    case sf::Keyboard::Key::Numpad0: return static_cast<int>(KeyCode::Numpad0);
    case sf::Keyboard::Key::Numpad1: return static_cast<int>(KeyCode::Numpad1);
    case sf::Keyboard::Key::Numpad2: return static_cast<int>(KeyCode::Numpad2);
    case sf::Keyboard::Key::Numpad3: return static_cast<int>(KeyCode::Numpad3);
    case sf::Keyboard::Key::Numpad4: return static_cast<int>(KeyCode::Numpad4);
    case sf::Keyboard::Key::Numpad5: return static_cast<int>(KeyCode::Numpad5);
    case sf::Keyboard::Key::Numpad6: return static_cast<int>(KeyCode::Numpad6);
    case sf::Keyboard::Key::Numpad7: return static_cast<int>(KeyCode::Numpad7);
    case sf::Keyboard::Key::Numpad8: return static_cast<int>(KeyCode::Numpad8);
    case sf::Keyboard::Key::Numpad9: return static_cast<int>(KeyCode::Numpad9);

    // Numpad operators
    case sf::Keyboard::Key::Add: return static_cast<int>(KeyCode::NumpadAdd);
    case sf::Keyboard::Key::Subtract: return static_cast<int>(KeyCode::NumpadSubtract);
    case sf::Keyboard::Key::Multiply: return static_cast<int>(KeyCode::NumpadMultiply);
    case sf::Keyboard::Key::Divide: return static_cast<int>(KeyCode::NumpadDivide);

    default: return static_cast<int>(KeyCode::Unknown);
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

    if (m_fullscreen)
    {
        // Fullscreen with letterboxing - need to account for scale and offset
        float scaleX = windowWidth / static_cast<float>(RENDER_LOGICAL_WIDTH);
        float scaleY = windowHeight / static_cast<float>(RENDER_LOGICAL_HEIGHT);
        float scale = (scaleY < scaleX) ? scaleY : scaleX;

        float destWidth = static_cast<float>(RENDER_LOGICAL_WIDTH) * scale;
        float destHeight = static_cast<float>(RENDER_LOGICAL_HEIGHT) * scale;
        float offsetX = (windowWidth - destWidth) / 2.0f;
        float offsetY = (windowHeight - destHeight) / 2.0f;

        // Transform from window coords to logical coords
        logicalX = static_cast<int>((mouseX - offsetX) / scale);
        logicalY = static_cast<int>((mouseY - offsetY) / scale);
    }
    else
    {
        // Windowed mode - simple scaling
        float scaleX = windowWidth / static_cast<float>(RENDER_LOGICAL_WIDTH);
        float scaleY = windowHeight / static_cast<float>(RENDER_LOGICAL_HEIGHT);

        logicalX = static_cast<int>(mouseX / scaleX);
        logicalY = static_cast<int>(mouseY / scaleY);
    }

    // Clamp to valid range
    if (logicalX < 0) logicalX = 0;
    if (logicalX > LOGICAL_MAX_X) logicalX = LOGICAL_MAX_X;
    if (logicalY < 0) logicalY = 0;
    if (logicalY > LOGICAL_MAX_Y) logicalY = LOGICAL_MAX_Y;
}
