// Win32Window.h: Win32 implementation of IWindow
//
// Part of DDrawEngine static library
// This implements window creation using the Win32 API
//////////////////////////////////////////////////////////////////////

#pragma once

#include "IWindow.h"

class Win32Window : public IWindow
{
public:
    Win32Window();
    virtual ~Win32Window();

    // ============== IWindow Implementation ==============

    // Lifecycle
    virtual bool Create(const WindowParams& params) override;
    virtual void Destroy() override;
    virtual bool IsOpen() const override;
    virtual void Close() override;

    // Properties
    virtual HWND GetHandle() const override;
    virtual int GetWidth() const override;
    virtual int GetHeight() const override;
    virtual bool IsFullscreen() const override;
    virtual bool IsActive() const override;

    // Display
    virtual void SetFullscreen(bool fullscreen) override;
    virtual void SetSize(int width, int height, bool center = true) override;
    virtual void Show() override;
    virtual void Hide() override;
    virtual void SetTitle(const char* title) override;

    // Message Processing
    virtual bool ProcessMessages() override;
    virtual void WaitForMessage() override;

    // Event Handler
    virtual void SetEventHandler(IWindowEventHandler* handler) override;
    virtual IWindowEventHandler* GetEventHandler() const override;

private:
    // Internal WndProc - static to work with Win32 API
    static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);
    
    // Transform window coordinates to logical game coordinates (640x480)
    void TransformMouseCoords(int windowX, int windowY, int& logicalX, int& logicalY) const;

    // Window state
    HWND m_hWnd;
    HINSTANCE m_hInstance;
    IWindowEventHandler* m_pEventHandler;
    int m_width;
    int m_height;
    bool m_fullscreen;
    bool m_resizable;
    bool m_active;
    bool m_open;
    char m_className[64];
};
