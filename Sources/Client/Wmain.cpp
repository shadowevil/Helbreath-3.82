// --------------------------------------------------------------
//                      Helbreath Client
//
//                      1998.10 by Soph
//
// --------------------------------------------------------------

// MODERNIZED: Prevent old winsock.h from loading (must be before windows.h)
#define _WINSOCKAPI_

#include <windows.h>
#include "CommonTypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <mmsystem.h>
#include <process.h>
#include <atomic>
#include <thread>
#include <chrono>

#include "Game.h"
#include "GameModeManager.h"
#include "GlobalDef.h"
#include "resource.h"
#include "FrameTiming.h"
#include "ConfigManager.h"
#include "IInput.h"
#include "RendererFactory.h"
#include "GameWindowHandler.h"
#include "Benchmark.h"

// --------------------------------------------------------------
// GPU Selection - Force discrete GPU on hybrid systems
// These exports tell NVIDIA Optimus and AMD PowerXpress to prefer
// the high-performance GPU over integrated graphics
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

// --------------------------------------------------------------
// Global state
HWND G_hWnd = 0;
HWND G_hEditWnd = 0;
class CGame* G_pGame = nullptr;

// Timer thread state (replaces Windows multimedia timer)
std::atomic<bool> G_bTimerSignal{false};
std::atomic<bool> G_bTimerRunning{false};
std::thread G_timerThread;
class XSocket* G_pCalcSocket = 0;
bool G_bIsCalcSocketConnected = true;
uint32_t G_dwCalcSocketTime = 0, G_dwCalcSocketSendTime = 0;


// Window event handler
static GameWindowHandler* g_pWindowHandler = nullptr;

// Function declarations
void EventLoop();
void Initialize(char* pCmdLine);
void StartTimerThread();
void StopTimerThread();

// Timer thread function - signals main thread every 1000ms
void TimerThreadFunc()
{
    while (G_bTimerRunning.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        if (G_bTimerRunning.load()) {
            G_bTimerSignal.store(true);
        }
    }
}

// --------------------------------------------------------------
int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    srand((unsigned)time(0));

    // Allocate debug console
    DebugConsole::Allocate();

    // Ensure consistent pixel coordinates under RDP and high-DPI setups
    if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
        SetProcessDPIAware();
    }

    // Load settings early so window size is available
    ConfigManager::Get().Initialize();
    ConfigManager::Get().Load();

    // Create game instance
    G_pGame = new class CGame;

    // Create window using engine abstraction
    WindowParams params = {};
    params.title = "Helbreath";
    params.width = ConfigManager::Get().GetWindowWidth();
    params.height = ConfigManager::Get().GetWindowHeight();
    params.fullscreen = false;
    params.centered = true;
    params.resizable = true;  // Enable borderless resize
    params.hInstance = hInstance;
    params.iconResourceId = IDI_ICON1;

    if (!Window::Create(params))
    {
        MessageBox(nullptr, "Failed to create window!", "ERROR", MB_ICONEXCLAMATION | MB_OK);
        delete G_pGame;
        return 1;
    }

    // Set up event handler
    g_pWindowHandler = new GameWindowHandler(G_pGame);
    Window::Get()->SetEventHandler(g_pWindowHandler);
    Window::Get()->Show();

    // Store window handle globally for legacy code
    G_hWnd = Window::GetHandle();

    // Initialize game systems
    Initialize((char*)lpCmdLine);

    // Main loop
    EventLoop();

    // Cleanup
    StopTimerThread();

    // Clear event handler from window BEFORE deleting it
    // This prevents dangling pointer access during window destruction
    Window::Get()->SetEventHandler(nullptr);

    delete g_pWindowHandler;
    g_pWindowHandler = nullptr;

    delete G_pGame;
    G_pGame = nullptr;

    Window::Destroy();
    Renderer::Destroy();

    // Deallocate debug console
    DebugConsole::Deallocate();

    return 0;
}

void EventLoop()
{
    IWindow* pWindow = Window::Get();
    if (!pWindow)
        return;

    // Main event loop - BeginFrame must be called BEFORE ProcessMessages
    // so that events processed in this frame are visible to game code
    while (true)
    {
        // Reset per-frame input state from PREVIOUS frame
        Input::BeginFrame();

        // Process window messages - this sets pressed/released states for THIS frame
        if (!pWindow->ProcessMessages())
            break;

        if (G_pGame->m_bIsProgramActive || GameModeManager::GetMode() == GameMode::Loading)
        {
            FrameTiming::BeginFrame();
            G_pGame->RenderFrame();
            FrameTiming::EndFrame();
        }
        else
        {
            pWindow->WaitForMessage();
        }
    }
}

void Initialize(char* pCmdLine)
{
    int iErrCode;
    uint16_t wVersionRequested;
    WSADATA wsaData;

    // Initialize timing systems
    FrameTiming::Initialize();

    // Initialize Winsock
    wVersionRequested = MAKEWORD(2, 2);
    iErrCode = WSAStartup(wVersionRequested, &wsaData);
    if (iErrCode)
    {
        MessageBox(G_hWnd, "Winsock-V2.2 not found! Cannot execute program.", "ERROR", MB_ICONEXCLAMATION | MB_OK);
        PostQuitMessage(0);
        return;
    }

    // Initialize game
    if (G_pGame->bInit() == false)
    {
        PostQuitMessage(0);
        return;
    }

    // Start timer thread
    StartTimerThread();
}

void StartTimerThread()
{
    G_bTimerRunning.store(true);
    G_timerThread = std::thread(TimerThreadFunc);
}

void StopTimerThread()
{
    G_bTimerRunning.store(false);
    if (G_timerThread.joinable()) {
        G_timerThread.join();
    }
}
