// Win32Window.cpp: Win32 implementation of IWindow
//
// Part of DDrawEngine static library
//////////////////////////////////////////////////////////////////////

#include "Win32Window.h"
#include "RenderConstants.h"
#include <cstdio>
#include <windowsx.h>  // For GET_X_LPARAM, GET_Y_LPARAM

Win32Window::Win32Window()
    : m_hWnd(nullptr)
    , m_hInstance(nullptr)
    , m_pEventHandler(nullptr)
    , m_width(0)
    , m_height(0)
    , m_fullscreen(false)
    , m_resizable(false)
    , m_active(false)
    , m_open(false)
{
    m_className[0] = '\0';
}

Win32Window::~Win32Window()
{
    Destroy();
}

bool Win32Window::Create(const WindowParams& params)
{
    if (m_open)
        return false;  // Already created

    m_hInstance = params.hInstance;
    m_width = params.width;
    m_height = params.height;
    m_fullscreen = params.fullscreen;
    m_resizable = params.resizable;

    // Generate unique class name
    sprintf_s(m_className, sizeof(m_className), "Win32Window-%p", this);

    // Register window class
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
    wc.lpfnWndProc = StaticWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(Win32Window*);
    wc.hInstance = m_hInstance;
    wc.hIcon = params.iconResourceId ? LoadIcon(m_hInstance, MAKEINTRESOURCE(params.iconResourceId)) : LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = m_className;
    wc.hIconSm = wc.hIcon;

    if (!RegisterClassEx(&wc))
        return false;

    // Calculate window style based on resizable flag
    // Match SFML's style behavior exactly
    DWORD style;
    DWORD exStyle = 0;
    
    if (m_fullscreen)
    {
        // Fullscreen: borderless popup
        style = WS_POPUP;
    }
    else if (m_resizable)
    {
        // Windowed resizable: standard overlapped window (like SFML's Titlebar | Close | Resize)
        style = WS_OVERLAPPEDWINDOW;
    }
    else
    {
        // Borderless window (original behavior)
        style = WS_POPUP;
    }

    // For DPI-aware window size calculation
    // First create a temporary window to get the correct DPI, then adjust
    int windowWidth = m_width;
    int windowHeight = m_height;
    int posX = CW_USEDEFAULT;
    int posY = CW_USEDEFAULT;
    
    if (!m_fullscreen)
    {
        // Calculate the window size needed to have the desired CLIENT area size
        RECT windowRect = { 0, 0, m_width, m_height };
        AdjustWindowRectEx(&windowRect, style, FALSE, exStyle);
        windowWidth = windowRect.right - windowRect.left;
        windowHeight = windowRect.bottom - windowRect.top;
        
        // Calculate centered position
        if (params.centered)
        {
            int screenWidth = GetSystemMetrics(SM_CXSCREEN);
            int screenHeight = GetSystemMetrics(SM_CYSCREEN);
            posX = (screenWidth - windowWidth) / 2;
            posY = (screenHeight - windowHeight) / 2;
        }
    }
    else
    {
        // Fullscreen: use screen size
        windowWidth = GetSystemMetrics(SM_CXSCREEN);
        windowHeight = GetSystemMetrics(SM_CYSCREEN);
        posX = 0;
        posY = 0;
    }

    // Create the window
    m_hWnd = CreateWindowEx(
        exStyle,
        m_className,
        params.title ? params.title : "Window",
        style,
        posX, posY,
        windowWidth, windowHeight,
        nullptr,
        nullptr,
        m_hInstance,
        this  // Pass this pointer for WndProc
    );

    if (!m_hWnd)
    {
        UnregisterClass(m_className, m_hInstance);
        return false;
    }
    
    // Get the actual client area size (may differ due to DPI or other factors)
    RECT clientRect;
    GetClientRect(m_hWnd, &clientRect);
    m_width = clientRect.right - clientRect.left;
    m_height = clientRect.bottom - clientRect.top;

    m_open = true;
    m_active = true;

    return true;
}

void Win32Window::Destroy()
{
    if (m_hWnd)
    {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }

    if (m_className[0] != '\0' && m_hInstance)
    {
        UnregisterClass(m_className, m_hInstance);
        m_className[0] = '\0';
    }

    m_open = false;
    m_active = false;
}

bool Win32Window::IsOpen() const
{
    return m_open;
}

void Win32Window::Close()
{
    if (m_hWnd)
    {
        // Post WM_CLOSE to trigger proper shutdown sequence
        PostMessage(m_hWnd, WM_CLOSE, 0, 0);
    }
}

HWND Win32Window::GetHandle() const
{
    return m_hWnd;
}

int Win32Window::GetWidth() const
{
    return m_width;
}

int Win32Window::GetHeight() const
{
    return m_height;
}

bool Win32Window::IsFullscreen() const
{
    return m_fullscreen;
}

bool Win32Window::IsActive() const
{
    return m_active;
}

void Win32Window::SetFullscreen(bool fullscreen)
{
    // TODO: Implement fullscreen toggle
    m_fullscreen = fullscreen;
}

void Win32Window::SetSize(int width, int height, bool center)
{
    if (width <= 0 || height <= 0)
        return;

    m_width = width;
    m_height = height;

    if (m_hWnd)
    {
        if (center)
        {
            int screenWidth = GetSystemMetrics(SM_CXSCREEN);
            int screenHeight = GetSystemMetrics(SM_CYSCREEN);
            int posX = (screenWidth - width) / 2;
            int posY = (screenHeight - height) / 2;
            SetWindowPos(m_hWnd, HWND_TOP, posX, posY, width, height, SWP_SHOWWINDOW);
        }
        else
        {
            SetWindowPos(m_hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_SHOWWINDOW);
        }
    }
}

void Win32Window::Show()
{
    if (m_hWnd)
    {
        ShowWindow(m_hWnd, SW_SHOWDEFAULT);
        UpdateWindow(m_hWnd);
    }
}

void Win32Window::Hide()
{
    if (m_hWnd)
    {
        ShowWindow(m_hWnd, SW_HIDE);
    }
}

void Win32Window::SetTitle(const char* title)
{
    if (m_hWnd && title)
    {
        SetWindowText(m_hWnd, title);
    }
}

bool Win32Window::ProcessMessages()
{
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            m_open = false;
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

void Win32Window::WaitForMessage()
{
    ::WaitMessage();
}

void Win32Window::SetEventHandler(IWindowEventHandler* handler)
{
    m_pEventHandler = handler;
}

IWindowEventHandler* Win32Window::GetEventHandler() const
{
    return m_pEventHandler;
}

// Static WndProc that routes to instance method
LRESULT CALLBACK Win32Window::StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Win32Window* pWindow = nullptr;

    if (message == WM_NCCREATE)
    {
        // Get the Win32Window pointer from CreateWindowEx
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pWindow = reinterpret_cast<Win32Window*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
        pWindow->m_hWnd = hWnd;
    }
    else
    {
        pWindow = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if (pWindow)
    {
        return pWindow->HandleMessage(message, wParam, lParam);
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

// Instance method to handle messages
LRESULT Win32Window::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    // First, let event handler try to handle text input
    if (m_pEventHandler)
    {
        if (m_pEventHandler->OnTextInput(m_hWnd, message, wParam, lParam))
            return 0;
    }

    switch (message)
    {
    case WM_CLOSE:
    {
        bool shouldClose = true;
        if (m_pEventHandler)
            shouldClose = m_pEventHandler->OnClose();
        
        if (shouldClose)
        {
            // Proceed with default close behavior
            return DefWindowProc(m_hWnd, message, wParam, lParam);
        }
        // Close was cancelled (e.g., logout countdown started)
        return 0;
    }

    case WM_DESTROY:
        if (m_pEventHandler)
            m_pEventHandler->OnDestroy();
        m_open = false;
        PostQuitMessage(0);
        return 0;

    case WM_ACTIVATEAPP:
        m_active = (wParam != 0);
        if (m_pEventHandler)
            m_pEventHandler->OnActivate(m_active);
        return 0;

    case WM_SIZE:
        m_width = LOWORD(lParam);
        m_height = HIWORD(lParam);
        if (m_pEventHandler)
            m_pEventHandler->OnResize(m_width, m_height);
        return 0;

    case WM_KEYDOWN:
        if (m_pEventHandler)
            m_pEventHandler->OnKeyDown(static_cast<int>(wParam));
        return DefWindowProc(m_hWnd, message, wParam, lParam);

    case WM_KEYUP:
        if (m_pEventHandler)
            m_pEventHandler->OnKeyUp(static_cast<int>(wParam));
        return DefWindowProc(m_hWnd, message, wParam, lParam);

    case WM_CHAR:
        if (m_pEventHandler)
            m_pEventHandler->OnChar(static_cast<char>(wParam));
        return 0;

    case WM_MOUSEMOVE:
        if (m_pEventHandler)
        {
            int logicalX, logicalY;
            TransformMouseCoords(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), logicalX, logicalY);
            m_pEventHandler->OnMouseMove(logicalX, logicalY);
        }
        return 0;

    case WM_LBUTTONDOWN:
        if (m_pEventHandler)
        {
            int logicalX, logicalY;
            TransformMouseCoords(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), logicalX, logicalY);
            m_pEventHandler->OnMouseButtonDown(MOUSE_BUTTON_LEFT, logicalX, logicalY);
        }
        return 0;

    case WM_LBUTTONUP:
        if (m_pEventHandler)
        {
            int logicalX, logicalY;
            TransformMouseCoords(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), logicalX, logicalY);
            m_pEventHandler->OnMouseButtonUp(MOUSE_BUTTON_LEFT, logicalX, logicalY);
        }
        return 0;

    case WM_RBUTTONDOWN:
        if (m_pEventHandler)
        {
            int logicalX, logicalY;
            TransformMouseCoords(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), logicalX, logicalY);
            m_pEventHandler->OnMouseButtonDown(MOUSE_BUTTON_RIGHT, logicalX, logicalY);
        }
        return 0;

    case WM_RBUTTONUP:
        if (m_pEventHandler)
        {
            int logicalX, logicalY;
            TransformMouseCoords(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), logicalX, logicalY);
            m_pEventHandler->OnMouseButtonUp(MOUSE_BUTTON_RIGHT, logicalX, logicalY);
        }
        return 0;

    case WM_MBUTTONDOWN:
        if (m_pEventHandler)
        {
            int logicalX, logicalY;
            TransformMouseCoords(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), logicalX, logicalY);
            m_pEventHandler->OnMouseButtonDown(MOUSE_BUTTON_MIDDLE, logicalX, logicalY);
        }
        return 0;

    case WM_MBUTTONUP:
        if (m_pEventHandler)
        {
            int logicalX, logicalY;
            TransformMouseCoords(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), logicalX, logicalY);
            m_pEventHandler->OnMouseButtonUp(MOUSE_BUTTON_MIDDLE, logicalX, logicalY);
        }
        return 0;

    case WM_MOUSEWHEEL:
        if (m_pEventHandler)
        {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ScreenToClient(m_hWnd, &pt);
            int logicalX, logicalY;
            TransformMouseCoords(pt.x, pt.y, logicalX, logicalY);
            m_pEventHandler->OnMouseWheel(delta, logicalX, logicalY);
        }
        return 0;

    case WM_SYSCOMMAND:
        // Prevent screensaver and monitor power-off
        if ((wParam & 0xFFF0) == SC_SCREENSAVE || (wParam & 0xFFF0) == SC_MONITORPOWER)
            return 0;
        return DefWindowProc(m_hWnd, message, wParam, lParam);

    default:
        // Let event handler try custom messages
        if (m_pEventHandler)
        {
            if (m_pEventHandler->OnCustomMessage(message, wParam, lParam))
                return 0;
        }
        return DefWindowProc(m_hWnd, message, wParam, lParam);
    }
}

// Transform window coordinates to logical game coordinates (640x480)
// Must match the scaling logic in DXC_ddraw::iFlip()
void Win32Window::TransformMouseCoords(int windowX, int windowY, int& logicalX, int& logicalY) const
{
    // Get actual window client size
    RECT clientRect;
    GetClientRect(m_hWnd, &clientRect);
    int clientWidth = clientRect.right - clientRect.left;
    int clientHeight = clientRect.bottom - clientRect.top;
    
    // If client area is 0 (minimized?), use stored values
    if (clientWidth <= 0) clientWidth = m_width;
    if (clientHeight <= 0) clientHeight = m_height;
    
    // If client area matches logical size exactly, no transformation needed
    if (clientWidth == RENDER_LOGICAL_WIDTH && clientHeight == RENDER_LOGICAL_HEIGHT)
    {
        logicalX = windowX;
        logicalY = windowY;
    }
    else
    {
        // DDraw uses letterboxing (uniform scale) - must match DXC_ddraw::iFlip()
        double scaleX = static_cast<double>(clientWidth) / static_cast<double>(RENDER_LOGICAL_WIDTH);
        double scaleY = static_cast<double>(clientHeight) / static_cast<double>(RENDER_LOGICAL_HEIGHT);
        double scale = (scaleY < scaleX) ? scaleY : scaleX;
        if (scale <= 0.0) scale = 1.0;
        
        // Calculate the destination rectangle (where the game is actually drawn)
        int destWidth = static_cast<int>(RENDER_LOGICAL_WIDTH * scale);
        int destHeight = static_cast<int>(RENDER_LOGICAL_HEIGHT * scale);
        int offsetX = (clientWidth - destWidth) / 2;
        int offsetY = (clientHeight - destHeight) / 2;
        
        // Transform from window coords to logical coords
        logicalX = static_cast<int>((windowX - offsetX) / scale);
        logicalY = static_cast<int>((windowY - offsetY) / scale);
    }
    
    // Clamp to valid range
    if (logicalX < 0) logicalX = 0;
    if (logicalX > LOGICAL_MAX_X) logicalX = LOGICAL_MAX_X;
    if (logicalY < 0) logicalY = 0;
    if (logicalY > LOGICAL_MAX_Y) logicalY = LOGICAL_MAX_Y;
}
