// Win32Input.cpp: Win32/DirectDraw implementation of IInput
//
// Part of DDrawEngine static library
//////////////////////////////////////////////////////////////////////

#include "Win32Input.h"
#include "RenderConstants.h"
#include <cstring>

// Global input instance pointer (owned by RendererFactory)
static Win32Input* s_pInput = nullptr;

// ============== Global Input Namespace Implementation ==============
namespace Input {
    void Create()
    {
        if (!s_pInput)
        {
            s_pInput = new Win32Input();
        }
    }

    void Destroy()
    {
        delete s_pInput;
        s_pInput = nullptr;
    }

    IInput* Get()
    {
        return s_pInput;
    }
}

// ============== Win32Input Implementation ==============

Win32Input::Win32Input()
    : m_hWnd(nullptr)
    , m_active(false)
    , m_suppressed(false)
    , m_mouseX(0)
    , m_mouseY(0)
    , m_wheelDelta(0)
{
    std::memset(m_mouseDown, 0, sizeof(m_mouseDown));
    std::memset(m_mousePressed, 0, sizeof(m_mousePressed));
    std::memset(m_mouseReleased, 0, sizeof(m_mouseReleased));
    std::memset(m_keyDown, 0, sizeof(m_keyDown));
    std::memset(m_keyPressed, 0, sizeof(m_keyPressed));
    std::memset(m_keyReleased, 0, sizeof(m_keyReleased));
}

Win32Input::~Win32Input()
{
    UpdateCursorClip(false);
}

void Win32Input::Initialize(HWND hWnd)
{
    m_hWnd = hWnd;
    UpdateCursorClip(false);

    if (m_hWnd && GetForegroundWindow() == m_hWnd)
    {
        SetWindowActive(true);
    }
}

void Win32Input::BeginFrame()
{
    // Reset per-frame state
    std::memset(m_mousePressed, 0, sizeof(m_mousePressed));
    std::memset(m_mouseReleased, 0, sizeof(m_mouseReleased));
    std::memset(m_keyPressed, 0, sizeof(m_keyPressed));
    std::memset(m_keyReleased, 0, sizeof(m_keyReleased));
    m_wheelDelta = 0;
}

// ============== Keyboard ==============

bool Win32Input::IsKeyDown(int key) const
{
    if (m_suppressed) return false;
    if (key < 0 || key >= kKeyCount)
        return false;
    return m_keyDown[key];
}

bool Win32Input::IsKeyPressed(int key) const
{
    if (m_suppressed) return false;
    if (key < 0 || key >= kKeyCount)
        return false;
    return m_keyPressed[key];
}

bool Win32Input::IsKeyReleased(int key) const
{
    if (m_suppressed) return false;
    if (key < 0 || key >= kKeyCount)
        return false;
    return m_keyReleased[key];
}

void Win32Input::OnKeyDown(int key)
{
    if (key < 0 || key >= kKeyCount)
        return;

    // Only set pressed on initial key down, not on auto-repeat
    if (!m_keyDown[key])
    {
        m_keyPressed[key] = true;
    }
    m_keyDown[key] = true;
}

void Win32Input::OnKeyUp(int key)
{
    if (key < 0 || key >= kKeyCount)
        return;

    if (m_keyDown[key])
    {
        m_keyReleased[key] = true;
    }
    m_keyDown[key] = false;
}

// ============== Mouse Buttons ==============

bool Win32Input::IsMouseButtonDown(int button) const
{
    if (m_suppressed) return false;
    if (button < 0 || button > 2)
        return false;
    return m_mouseDown[button];
}

bool Win32Input::IsMouseButtonPressed(int button) const
{
    if (m_suppressed) return false;
    if (button < 0 || button > 2)
        return false;
    return m_mousePressed[button];
}

bool Win32Input::IsMouseButtonReleased(int button) const
{
    if (m_suppressed) return false;
    if (button < 0 || button > 2)
        return false;
    return m_mouseReleased[button];
}

void Win32Input::OnMouseDown(int button)
{
    if (button < 0 || button > 2)
        return;

    if (!m_mouseDown[button])
    {
        m_mousePressed[button] = true;
    }
    m_mouseDown[button] = true;

    // Capture mouse for drag operations
    if (m_hWnd)
    {
        SetCapture(m_hWnd);
    }
}

void Win32Input::OnMouseUp(int button)
{
    if (button < 0 || button > 2)
        return;

    if (m_mouseDown[button])
    {
        m_mouseReleased[button] = true;
    }
    m_mouseDown[button] = false;

    // Release capture when all buttons are up
    if (!m_mouseDown[0] && !m_mouseDown[1] && !m_mouseDown[2])
    {
        if (GetCapture() == m_hWnd)
        {
            ReleaseCapture();
        }
    }
}

// ============== Mouse Position ==============

int Win32Input::GetMouseX() const
{
    return m_mouseX;
}

int Win32Input::GetMouseY() const
{
    return m_mouseY;
}

void Win32Input::OnMouseMove(int x, int y)
{
    // Coordinates are already transformed to logical space (640x480) by the window layer
    // Just store them directly - no additional transformation needed
    m_mouseX = x;
    m_mouseY = y;
    
    // Clamp to valid range (safety check)
    if (m_mouseX < 0) m_mouseX = 0;
    if (m_mouseY < 0) m_mouseY = 0;
    if (m_mouseX > LOGICAL_MAX_X) m_mouseX = LOGICAL_MAX_X;
    if (m_mouseY > LOGICAL_MAX_Y) m_mouseY = LOGICAL_MAX_Y;
}

// ============== Mouse Wheel ==============

int Win32Input::GetMouseWheelDelta() const
{
    if (m_suppressed) return 0;
    return m_wheelDelta;
}

void Win32Input::OnMouseWheel(int delta)
{
    m_wheelDelta += delta;
}

// ============== Modifier Keys ==============

bool Win32Input::IsShiftDown() const
{
    if (m_suppressed) return false;
    return IsKeyDown(VK_SHIFT) || IsKeyDown(VK_LSHIFT) || IsKeyDown(VK_RSHIFT);
}

bool Win32Input::IsCtrlDown() const
{
    if (m_suppressed) return false;
    return IsKeyDown(VK_CONTROL) || IsKeyDown(VK_LCONTROL) || IsKeyDown(VK_RCONTROL);
}

bool Win32Input::IsAltDown() const
{
    if (m_suppressed) return false;
    return IsKeyDown(VK_MENU) || IsKeyDown(VK_LMENU) || IsKeyDown(VK_RMENU);
}

// ============== Input Suppression ==============

void Win32Input::SetSuppressed(bool suppressed)
{
    m_suppressed = suppressed;
}

bool Win32Input::IsSuppressed() const
{
    return m_suppressed;
}

// ============== Window Focus ==============

bool Win32Input::IsWindowActive() const
{
    return m_active;
}

void Win32Input::SetWindowActive(bool active)
{
    m_active = active;
    UpdateCursorClip(active);

    if (!active)
    {
        ClearAllKeys();
    }
}

void Win32Input::ClearAllKeys()
{
    std::memset(m_keyDown, 0, sizeof(m_keyDown));
    std::memset(m_keyPressed, 0, sizeof(m_keyPressed));
    std::memset(m_keyReleased, 0, sizeof(m_keyReleased));
    std::memset(m_mouseDown, 0, sizeof(m_mouseDown));
    std::memset(m_mousePressed, 0, sizeof(m_mousePressed));
    std::memset(m_mouseReleased, 0, sizeof(m_mouseReleased));
}

void Win32Input::UpdateCursorClip(bool active)
{
    // Don't clip cursor - allow mouse to move freely like SFML does
    // This lets users access window controls (minimize, maximize, close, resize)
    (void)active;  // Unused
    ClipCursor(nullptr);
}
