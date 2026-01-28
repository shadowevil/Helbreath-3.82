// IWindow.h: Abstract interface for window management
//
// This interface allows different renderers to create windows appropriate
// for their backend (DirectDraw, OpenGL, Vulkan, etc.)
//////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <cstdint>

// Forward declaration
class IWindowEventHandler;

// Window creation parameters
struct WindowParams
{
    const char* title;
    int width;
    int height;
    bool fullscreen;
    bool centered;
    bool resizable;      // Allow window resizing (borderless with resize edges)
    HINSTANCE hInstance;
    int iconResourceId;  // 0 for default
};

// Abstract window interface
class IWindow
{
public:
    virtual ~IWindow() = default;

    // ============== Lifecycle ==============
    virtual bool Create(const WindowParams& params) = 0;
    virtual void Destroy() = 0;
    virtual bool IsOpen() const = 0;
    virtual void Close() = 0;  // Request window close (triggers close event)

    // ============== Properties ==============
    virtual HWND GetHandle() const = 0;
    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;
    virtual bool IsFullscreen() const = 0;
    virtual bool IsActive() const = 0;

    // ============== Display ==============
    virtual void SetFullscreen(bool fullscreen) = 0;
    virtual void SetSize(int width, int height, bool center = true) = 0;  // Resize window
    virtual void Show() = 0;
    virtual void Hide() = 0;
    virtual void SetTitle(const char* title) = 0;

    // ============== Message Processing ==============
    // Process pending messages, returns false if WM_QUIT received
    virtual bool ProcessMessages() = 0;

    // Wait for a message (used when inactive)
    virtual void WaitForMessage() = 0;

    // ============== Event Handler ==============
    virtual void SetEventHandler(IWindowEventHandler* handler) = 0;
    virtual IWindowEventHandler* GetEventHandler() const = 0;
};

// Window event callback interface
// Implement this to receive window events
class IWindowEventHandler
{
public:
    virtual ~IWindowEventHandler() = default;

    // ============== Window Events ==============
    // OnClose: called when user requests window close (X button, ALT+F4)
    // Return true to proceed with close, false to cancel (e.g., logout countdown)
    virtual bool OnClose() = 0;
    virtual void OnDestroy() = 0;
    virtual void OnActivate(bool active) = 0;
    virtual void OnResize(int width, int height) = 0;

    // ============== Input Events ==============
    virtual void OnKeyDown(int keyCode) = 0;
    virtual void OnKeyUp(int keyCode) = 0;
    virtual void OnChar(char character) = 0;
    virtual void OnMouseMove(int x, int y) = 0;
    virtual void OnMouseButtonDown(int button, int x, int y) = 0;
    virtual void OnMouseButtonUp(int button, int x, int y) = 0;
    virtual void OnMouseWheel(int delta, int x, int y) = 0;

    // ============== Custom Messages ==============
    // For game-specific messages (sockets, timers, etc.)
    // Return true if handled, false to pass to DefWindowProc
    virtual bool OnCustomMessage(UINT message, WPARAM wParam, LPARAM lParam) = 0;

    // ============== Text Input ==============
    // For IME and text composition
    virtual bool OnTextInput(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;
};

// Mouse button constants
#define MOUSE_BUTTON_LEFT   0
#define MOUSE_BUTTON_RIGHT  1
#define MOUSE_BUTTON_MIDDLE 2
