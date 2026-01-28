// Win32Input.h: Win32/DirectDraw implementation of IInput
//
// Part of DDrawEngine static library
//////////////////////////////////////////////////////////////////////

#pragma once

// Prevent winsock.h conflicts
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#include <windows.h>
#include "IInput.h"

class Win32Input : public IInput
{
public:
    Win32Input();
    virtual ~Win32Input();

    void Initialize(HWND hWnd);

    // ============== IInput Implementation ==============

    // Frame management
    virtual void BeginFrame() override;

    // Keyboard
    virtual bool IsKeyDown(int key) const override;
    virtual bool IsKeyPressed(int key) const override;
    virtual bool IsKeyReleased(int key) const override;

    // Mouse buttons
    virtual bool IsMouseButtonDown(int button) const override;
    virtual bool IsMouseButtonPressed(int button) const override;
    virtual bool IsMouseButtonReleased(int button) const override;

    // Mouse position
    virtual int GetMouseX() const override;
    virtual int GetMouseY() const override;

    // Mouse wheel
    virtual int GetMouseWheelDelta() const override;

    // Modifier keys
    virtual bool IsShiftDown() const override;
    virtual bool IsCtrlDown() const override;
    virtual bool IsAltDown() const override;

    // Window focus
    virtual bool IsWindowActive() const override;
    virtual void SetWindowActive(bool active) override;

    // Input suppression
    virtual void SetSuppressed(bool suppressed) override;
    virtual bool IsSuppressed() const override;

    // Input events
    virtual void OnKeyDown(int key) override;
    virtual void OnKeyUp(int key) override;
    virtual void OnMouseMove(int x, int y) override;
    virtual void OnMouseDown(int button) override;
    virtual void OnMouseUp(int button) override;
    virtual void OnMouseWheel(int delta) override;

private:
    void UpdateCursorClip(bool active);
    void ClearAllKeys();

    HWND m_hWnd;
    bool m_active;
    bool m_suppressed;

    // Mouse state
    int m_mouseX;
    int m_mouseY;
    int m_wheelDelta;
    bool m_mouseDown[3];     // Left, Right, Middle
    bool m_mousePressed[3];
    bool m_mouseReleased[3];

    // Keyboard state
    static constexpr int kKeyCount = 256;
    bool m_keyDown[kKeyCount];
    bool m_keyPressed[kKeyCount];
    bool m_keyReleased[kKeyCount];
};
