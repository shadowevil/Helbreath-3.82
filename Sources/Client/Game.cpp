#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#endif

#include "Game.h"
#include "GameConstants.h"
#include "ObjectIDRange.h"
#include "CommonTypes.h"
#include "RenderHelpers.h"
#include "GameFonts.h"
#include "TextLibExt.h"
#include "Benchmark.h"
#include "FrameTiming.h"
#include "lan_eng.h"
#include "Packet/SharedPackets.h"
#include "SharedCalculations.h"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <format>
#include <chrono>
#include <string>
#include <algorithm>
#include <cstring>
#include <charconv>
#ifdef _WIN32
#include <windows.h>
#endif

// hb::shared::render::Renderer
#include "RendererFactory.h"
#include "SpriteLoader.h"

// Manager singletons
#include "ConfigManager.h"
#include "AudioManager.h"
#include "WeatherManager.h"
#include "ChatManager.h"
#include "ItemNameFormatter.h"
#include "CombatSystem.h"
#include "InventoryManager.h"
#include "MagicCastingSystem.h"
#include "BuildItemManager.h"
#include "ShopManager.h"
#include "TeleportManager.h"
#include "TextInputManager.h"
#include "EventListManager.h"
#include "ChatCommandManager.h"
#include "HotkeyManager.h"
#include "DevConsole.h"
#include "LocalCacheManager.h"
#include "Overlay_DevConsole.h"

// DialogBox system
#include "IDialogBox.h"

// Entity render state
#include "EntityRenderState.h"

// Cursor targeting
#include "CursorTarget.h"

// Screen system
#include "IGameScreen.h"
#include "Screen_OnGame.h"
#include "Screen_MainMenu.h"
#include "Screen_Login.h"
#include "Screen_SelectCharacter.h"
#include "Screen_CreateNewCharacter.h"
#include "Screen_CreateAccount.h"
#include "Screen_Quit.h"
#include "Screen_Loading.h"
#include "Screen_Splash.h"
#include "Screen_Test.h"
#include "Screen_TestPrimitives.h"
#include "IInput.h"

// Overlay system
#include "Overlay_Connecting.h"
#include "Overlay_WaitingResponse.h"
#include "Overlay_QueryForceLogin.h"
#include "Overlay_QueryDeleteCharacter.h"
#include "Overlay_LogResMsg.h"
#include "Overlay_ChangePassword.h"
#include "Overlay_VersionNotMatch.h"
#include "Overlay_ConnectionLost.h"
#include "Overlay_Msg.h"
#include "Overlay_WaitInitData.h"


using namespace hb::shared::net;
using namespace hb::client::net;
namespace sock = hb::shared::net::socket;
using namespace hb::shared::action;

using namespace hb::shared::item;
using namespace hb::client::config;
using namespace hb::client::sprite_id;


// Drawing order arrays moved to RenderHelpers.cpp (declared extern in RenderHelpers.h)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CGame::ReadSettings()
{
	// Initialize and load settings from JSON file via ConfigManager
	ConfigManager::Get().Initialize();
	ConfigManager::Get().Load();

	// Copy values to CGame member variables
	m_sMagicShortCut = ConfigManager::Get().GetMagicShortcut();
	m_sRecentShortCut = ConfigManager::Get().GetRecentShortcut();
	for (int i = 0; i < 5; i++)
	{
		m_sShortCut[i] = ConfigManager::Get().GetShortcut(i);
	}
	m_sShortCut[5] = -1; // 6th slot unused

	// Audio settings loaded into AudioManager
	AudioManager::Get().SetMasterVolume(ConfigManager::Get().GetMasterVolume());
	AudioManager::Get().SetSoundVolume(ConfigManager::Get().GetSoundVolume());
	AudioManager::Get().SetMusicVolume(ConfigManager::Get().GetMusicVolume());
	AudioManager::Get().SetAmbientVolume(ConfigManager::Get().GetAmbientVolume());
	AudioManager::Get().SetUIVolume(ConfigManager::Get().GetUIVolume());
	AudioManager::Get().SetMasterEnabled(ConfigManager::Get().IsMasterEnabled());
	AudioManager::Get().SetSoundEnabled(ConfigManager::Get().IsSoundEnabled());
	AudioManager::Get().SetMusicEnabled(ConfigManager::Get().IsMusicEnabled());
	AudioManager::Get().SetAmbientEnabled(ConfigManager::Get().IsAmbientEnabled());
	AudioManager::Get().SetUIEnabled(ConfigManager::Get().IsUIEnabled());
}

void CGame::WriteSettings()
{
	// Copy CGame member variables to ConfigManager
	ConfigManager::Get().SetMagicShortcut(m_sMagicShortCut);
	for (int i = 0; i < 5; i++)
	{
		ConfigManager::Get().SetShortcut(i, m_sShortCut[i]);
	}

	// Audio settings from AudioManager
	ConfigManager::Get().SetMasterVolume(AudioManager::Get().GetMasterVolume());
	ConfigManager::Get().SetSoundVolume(AudioManager::Get().GetSoundVolume());
	ConfigManager::Get().SetMusicVolume(AudioManager::Get().GetMusicVolume());
	ConfigManager::Get().SetAmbientVolume(AudioManager::Get().GetAmbientVolume());
	ConfigManager::Get().SetUIVolume(AudioManager::Get().GetUIVolume());
	ConfigManager::Get().SetMasterEnabled(AudioManager::Get().IsMasterEnabled());
	ConfigManager::Get().SetSoundEnabled(AudioManager::Get().IsSoundEnabled());
	ConfigManager::Get().SetMusicEnabled(AudioManager::Get().IsMusicEnabled());
	ConfigManager::Get().SetAmbientEnabled(AudioManager::Get().IsAmbientEnabled());
	ConfigManager::Get().SetUIEnabled(AudioManager::Get().IsUIEnabled());

	// Save to JSON file
	ConfigManager::Get().Save();
}

CGame::CGame()
	: m_playerRenderer(*this)
	, m_npcRenderer(*this)
{
	m_pIOPool = std::make_unique<hb::shared::net::IOServicePool>(0);  // 0 threads = manual poll mode
	m_Renderer = nullptr;
	m_dialogBoxManager.Initialize(this);
	m_dialogBoxManager.InitializeDialogBoxes();
	srand((unsigned)time(0));
	ReadSettings();
	RegisterHotkeys();

	iMaxStats = 0;
	iMaxLevel = hb::shared::limits::PlayerMaxLevel;
	iMaxBankItems = 200; // Default soft cap, server overrides
	m_cLoading = 0;
	m_bIsFirstConn = true;
	m_iItemDropCnt = 0;
	std::fill(std::begin(m_sItemDropID), std::end(m_sItemDropID), short{0});
	m_bItemDrop = false;

	// Initialize CPlayer first since it's used below
	m_pPlayer = std::make_unique<CPlayer>();
	CombatSystem::Get().SetPlayer(*m_pPlayer);
	InventoryManager::Get().SetGame(this);
	MagicCastingSystem::Get().SetGame(this);
	BuildItemManager::Get().SetGame(this);
	ShopManager::Get().SetGame(this);
	TeleportManager::Get().SetGame(this);
	EventListManager::Get().SetGame(this);
	m_pPlayer->m_sPlayerX = 0;
	m_pPlayer->m_sPlayerY = 0;
	m_pPlayer->m_Controller.ResetCommandCount();
	m_pPlayer->m_Controller.SetCommandTime(0); //v2.15 SpeedHack
	// Camera is initialized via its default constructor (calls Reset())
	m_sVDL_X = 0;
	m_sVDL_Y = 0;
	m_wCommObjectID = 0;
	m_wLastAttackTargetID = 0;
	m_wEnterGameType = 0;
	m_pPlayer->m_Controller.SetCommand(Type::Stop);
	m_bIsObserverMode = false;

	// Initialize Managers (Networking v4) - using make_unique
	m_pEffectManager = std::make_unique<EffectManager>(this);
	m_pNetworkMessageManager = std::make_unique<NetworkMessageManager>(this);
	m_fishingManager.SetGame(this);
	m_craftingManager.SetGame(this);
	m_questManager.SetGame(this);
	m_guildManager.SetGame(this);

	// All pointer arrays (std::array<std::unique_ptr<T>, N>) default to nullptr
	// Dialog box order initialization
	for (int i = 0; i < 61; i++) m_dialogBoxManager.SetOrderAt(i, 0);

	// Previous cursor status tracking removed
	CursorTarget::ResetSelectionClickTime();

	// m_pItemForSaleList defaults to nullptr (std::array<std::unique_ptr<T>, N>)

	// Dialog boxes now self-initialize via SetDefaultRect() in their constructors
	// Dialogs without classes (GiveItem) are initialized in DialogBoxManager::InitDefaults()

	m_iTimeLeftSecAccount = 0;
	m_iTimeLeftSecIP = 0;

	// Initialize char arrays that were previously zero-initialized by HEAP_ZERO_MEMORY
	std::memset(m_cMsg, 0, sizeof(m_cMsg));
	// m_cChatMsg and m_cAmountString are std::string (auto-initialized)
}

CGame::~CGame()
{
	hb::shared::render::Renderer::Destroy();
	m_Renderer = nullptr;
}

bool CGame::bInit()
{
	m_pPlayer->m_Controller.SetCommandAvailable(true);
	m_dwTime = GameClock::GetTimeMS();

	// Initialize AudioManager (sounds loaded later during loading screen)
	AudioManager::Get().Initialize();

	// Initialize ChatCommandManager
	ChatCommandManager::Get().Initialize(this);

	// Initialize GameModeManager
	GameModeManager::Initialize(this);

	GameModeManager::set_screen<Screen_Splash>();

	m_bHideLocalCursor = false;

	// Create and initialize the renderer
	if (!hb::shared::render::Renderer::Set(hb::shared::render::RendererType::SFML))
	{
		hb::shared::render::Window::ShowError("ERROR", "Failed to create renderer!");
		return false;
	}

	m_Renderer = hb::shared::render::Renderer::Get();
	if (m_Renderer->Init(hb::shared::render::Window::GetHandle()) == false)
	{
		hb::shared::render::Window::ShowError("ERROR", "Failed to init renderer!");
		return false;
	}

	// Preload the default font for text rendering
	if (hb::shared::text::GetTextRenderer())
	{
		if (!hb::shared::text::GetTextRenderer()->LoadFontFromFile("fonts/default.ttf"))
		{
			printf("[FONT] Failed to load default.ttf font file!\n");
		}
	}

	// Initialize sprite factory and register it globally
	m_pSpriteFactory.reset(hb::shared::render::CreateSpriteFactory(m_Renderer));
	hb::shared::sprite::Sprites::SetFactory(m_pSpriteFactory.get());

	if (bCheckImportantFile() == false)
	{
		hb::shared::render::Window::ShowError("ERROR1", "File checksum error! Get Update again please!");
		return false;
	}

	if (BuildItemManager::Get().LoadRecipes() == false)
	{
		hb::shared::render::Window::ShowError("ERROR2", "File checksum error! Get Update again please!");
		return false;
	}

	m_cLogServerAddr = DEF_SERVER_IP;
	m_iLogServerPort = DEF_SERVER_PORT;
	m_iGameServerPort = DEF_GSERVER_PORT;

	// Mouse position tracking removed - use hb::shared::input::GetMouseX/Y
	m_pMapData = std::make_unique<CMapData>(this);
	m_pPlayer->m_cPlayerName.clear();
	m_pPlayer->m_cAccountName.clear();
	m_pPlayer->m_cAccountPassword.clear();

	m_pPlayer->m_sPlayerType = 2;
	m_pPlayer->m_Controller.SetPlayerTurn(0);
	// Snoopy: fixed here
	m_dialogBoxManager.SetOrderAt(60, DialogBoxId::HudPanel);
	m_dialogBoxManager.SetOrderAt(59, DialogBoxId::HudPanel); // 29�� GaugePannel

	m_cMenuDir = 4;
	m_cMenuDirCnt = 0;
	m_cMenuFrame = 0;

	_LoadGameMsgTextContents();
	m_cWorldServerName = NAME_WORLDNAME1;

	// AudioManager initialized in bInit() with HWND
	WeatherManager::Get().Initialize();
	ChatManager::Get().Initialize();
	ItemNameFormatter::Get().SetItemConfigs(m_pItemConfigList);
	LocalCacheManager::Get().Initialize();

#ifdef _DEBUG
	FrameTiming::SetProfilingEnabled(true);
#endif

	return true;
}

void CGame::Quit()
{
	WriteSettings();
	ChangeGameMode(GameMode::Null);

	// Shutdown manager singletons
	WeatherManager::Get().Shutdown();
	ChatManager::Get().Shutdown();
	AudioManager::Get().Shutdown();
	ConfigManager::Get().Shutdown();

	// Clear all unique_ptr arrays using range-based for loops
	for (auto& item : m_pItemConfigList) item.reset();

	m_pSprite.clear();
	m_pTileSpr.clear();
	m_pEffectSpr.clear();

	// Clean up sprite factory
	hb::shared::sprite::Sprites::SetFactory(nullptr);
	if (m_pSpriteFactory) {
		m_pSpriteFactory.reset();
	}

	// Sound cleanup handled by AudioManager::Shutdown()
	// Effects now managed by EffectManager (cleaned up in destructor)

	for (auto& ch : m_pCharList) ch.reset();
	for (auto& item : m_pItemList) item.reset();
	for (auto& item : m_pBankList) item.reset();
	m_floatingText.ClearAll();
	for (auto& magic : m_pMagicCfgList) magic.reset();
	for (auto& skill : m_pSkillCfgList) skill.reset();
	for (auto& msg : m_pMsgTextList) msg.reset();
	for (auto& msg : m_pMsgTextList2) msg.reset();
	for (auto& msg : m_pAgreeMsgTextList) msg.reset();
	m_pExID.reset();
	for (auto& msg : m_pGameMsgList) msg.reset();

	// Clean up single pointers (unique_ptr handles null checks automatically)
	m_pMapData.reset();
	m_pGSock.reset();
	m_pLSock.reset();
	m_pEffectManager.reset();
	m_pNetworkMessageManager.reset();
}

// UpdateScreen and DrawScreen removed - all modes now handled by Screen/Overlay system via GameModeManager

// DrawCursor: Centralized cursor drawing at end of frame
// Called at the end of DrawScreen() to ensure cursor is always on top
void CGame::DrawCursor()
{
	// Check if cursor should be hidden
	if (m_bHideLocalCursor)
		return;

	// Get mouse position
	int msX = hb::shared::input::GetMouseX();
	int msY = hb::shared::input::GetMouseY();

	// Skip if mouse position is invalid (not yet initialized)
	if (msX == 0 && msY == 0)
		return;

	// Determine cursor frame based on game mode from manager (source of truth)
	int iCursorFrame = 0;  // Default arrow cursor

	switch (GameModeManager::GetMode()) {
	case GameMode::MainGame:
		// In-game uses context-sensitive cursor from CursorTarget
		if (m_bIsObserverMode) {
			// Observer mode shows a small crosshair instead of cursor sprite
			m_Renderer->DrawPixel(msX, msY, hb::shared::render::Color::White());
			m_Renderer->DrawPixel(msX + 1, msY, hb::shared::render::Color::White());
			m_Renderer->DrawPixel(msX - 1, msY, hb::shared::render::Color::White());
			m_Renderer->DrawPixel(msX, msY + 1, hb::shared::render::Color::White());
			m_Renderer->DrawPixel(msX, msY - 1, hb::shared::render::Color::White());
			return;
		}
		iCursorFrame = CursorTarget::GetCursorFrame();
		break;

	case GameMode::Connecting:
	case GameMode::WaitingResponse:
	case GameMode::WaitingInitData:
		// Waiting/connecting states use hourglass cursor (frame 8)
		iCursorFrame = 8;
		break;

	default:
		// All other modes use default arrow cursor (frame 0)
		iCursorFrame = 0;
		break;
	}

	// Draw the cursor sprite
	if (m_pSprite[MouseCursor])
		m_pSprite[MouseCursor]->Draw(msX, msY, iCursorFrame);
}



// UpdateFrame: Logic update — runs every iteration, decoupled from frame rate
// Handles: audio, timers, network, game state transitions
void CGame::UpdateFrame()
{
	AudioManager::Get().Update();

	// Process timer and network events (must happen before any update logic)
	if (m_game_timer.check_and_reset()) {
		OnTimer();
	}
	OnGameSocketEvent();
	OnLogSocketEvent();

	// Update game mode transition state (fade in/out progress)
	GameModeManager::Update();

	// Update game screens/overlays
	FrameTiming::BeginProfile(ProfileStage::Update);
	GameModeManager::UpdateScreens();
	FrameTiming::EndProfile(ProfileStage::Update);

	// Re-activate DevConsole overlay if needed (e.g. after screen transition)
	if (DevConsole::Get().IsVisible() && !GameModeManager::has_overlay())
	{
		GameModeManager::set_overlay<Overlay_DevConsole>();
	}
}

// RenderFrame: Render only — gated by engine frame limiting
// Handles: clear backbuffer -> draw -> fade overlay -> cursor -> flip
void CGame::RenderFrame()
{
	// ============== Render Phase ==============
	FrameTiming::BeginProfile(ProfileStage::ClearBuffer);
	m_Renderer->BeginFrame();
	FrameTiming::EndProfile(ProfileStage::ClearBuffer);

	// Engine frame limiter decided to skip this frame — return early
	// Prevents wasted draw calls and keeps behavior identical across all screens
	if (!m_Renderer->WasFramePresented())
		return;

	// Mark frame as rendered so FrameTiming only accumulates profiling for real frames
	FrameTiming::SetFrameRendered(true);

	// Render screens/overlays (skipped during Switching phase)
	if (GameModeManager::GetTransitionState() != TransitionState::Switching)
	{
		GameModeManager::Render();
	}

	// Draw fade overlay if transitioning between game modes
	if (GameModeManager::IsTransitioning())
	{
		float alpha = GameModeManager::GetFadeAlpha();
		m_Renderer->DrawRectFilled(0, 0, m_Renderer->GetWidth(), m_Renderer->GetHeight(), hb::shared::render::Color::Black(static_cast<uint8_t>(alpha * 255.0f)));
	}

	// HUD metrics — drawn on all screens, on top of fade overlay
	{
		std::string G_cTxt;
		int iDisplayY = 100;

		// FPS (engine-tracked, counted at actual present)
		if (ConfigManager::Get().IsShowFpsEnabled())
		{
			G_cTxt = std::format("fps : {}", m_Renderer->GetFPS());
			hb::shared::text::DrawText(GameFont::Default, 10, iDisplayY, G_cTxt.c_str(), hb::shared::text::TextStyle::Color(GameColors::UIWhite));
			iDisplayY += 14;
		}

		// Latency
		if (ConfigManager::Get().IsShowLatencyEnabled())
		{
			if (m_iLatencyMs >= 0)
				G_cTxt = std::format("latency : {} ms", m_iLatencyMs);
			else
				G_cTxt = "latency : -- ms";
			hb::shared::text::DrawText(GameFont::Default, 10, iDisplayY, G_cTxt.c_str(), hb::shared::text::TextStyle::Color(GameColors::UIWhite));
			iDisplayY += 14;
		}

		// Profiling stage breakdown
		if (FrameTiming::IsProfilingEnabled())
		{
			iDisplayY += 4;
			hb::shared::text::DrawText(GameFont::Default, 10, iDisplayY, "--- Profile (avg ms) ---", hb::shared::text::TextStyle::Color(GameColors::UIProfileYellow));
			iDisplayY += 14;

			for (int i = 0; i < static_cast<int>(ProfileStage::COUNT); i++)
			{
				ProfileStage stage = static_cast<ProfileStage>(i);
				double avgMs = FrameTiming::GetProfileAvgTimeMS(stage);
				int wholePart = static_cast<int>(avgMs);
				int fracPart = static_cast<int>((avgMs - wholePart) * 100);
				G_cTxt = std::format("{:<12}: {:3}.{:02}", FrameTiming::GetStageName(stage), wholePart, fracPart);
				hb::shared::text::DrawText(GameFont::Default, 10, iDisplayY, G_cTxt.c_str(), hb::shared::text::TextStyle::Color(GameColors::UINearWhite));
				iDisplayY += 12;
			}
		}
	}

	// Cursor always on top - drawn LAST after everything including fade overlay
	DrawCursor();

	// Flip to show the drawn content
	FrameTiming::BeginProfile(ProfileStage::Flip);
	if (m_Renderer->EndFrameCheckLostSurface())
		RestoreSprites();
	FrameTiming::EndProfile(ProfileStage::Flip);

	// Reset scroll delta now that dialogs have consumed it this frame
	// (scroll accumulates across skip frames until a rendered frame processes it)
	hb::shared::input::ResetMouseWheelDelta();
}


// MODERNIZED: v4 Networking Architecture (Drain -> Queue -> Process)
void CGame::OnGameSocketEvent()
{
    if (m_pGSock == 0) return;

    // 1. Check for socket state changes (Connect, Close, Error)
    int iRet = m_pGSock->Poll();

    switch (iRet) {
    case sock::Event::SocketClosed:
        ChangeGameMode(GameMode::ConnectionLost);
        m_pGSock.reset();
        m_pGSock.reset();
        return;
    case sock::Event::SocketError:
        printf("[ERROR] Game socket error\n");
        ChangeGameMode(GameMode::ConnectionLost);
        m_pGSock.reset();
        m_pGSock.reset();
        return;

    case sock::Event::ConnectionEstablish:
        ConnectionEstablishHandler(static_cast<int>(ServerType::Game));
        break;
    }

    // 2. Drain all available data from TCP buffer to the Queue
    // Only drain if socket is connected (m_bIsAvailable is set on FD_CONNECT)
    if (!m_pGSock->m_bIsAvailable) {
        return; // Still connecting, don't try to read yet
    }

    // If Poll() completed a packet, queue it before DrainToQueue() overwrites the buffer
    if (iRet == sock::Event::ReadComplete) {
        size_t dwSize = 0;
        char* pData = m_pGSock->pGetRcvDataPointer(&dwSize);
        if (pData != nullptr && dwSize > 0) {
            m_pGSock->QueueCompletedPacket(pData, dwSize);
        }
    }

    int iDrained = m_pGSock->DrainToQueue();

    if (iDrained < 0) {
        printf("[ERROR] Game socket DrainToQueue failed: %d\n", iDrained);
        ChangeGameMode(GameMode::ConnectionLost);
        m_pGSock.reset();
        m_pGSock.reset();
        return;
    }

    // 3. Process the queue with a Time Budget
    //    We process as many packets as possible within the budget to keep the game responsive.
    constexpr int MAX_PACKETS_PER_FRAME = 120; // Safety limit
    constexpr uint32_t MAX_TIME_MS = 3;        // 3ms budget for network processing

    uint32_t dwStartTime = GameClock::GetTimeMS();
    int iProcessed = 0;

    hb::shared::net::NetworkPacket packet;
    while (iProcessed < MAX_PACKETS_PER_FRAME) {

        // Check budget
        if (GameClock::GetTimeMS() - dwStartTime > MAX_TIME_MS) {
            break;
        }

        // Peek next packet
        if (!m_pGSock->PeekPacket(packet)) {
            break; // Queue empty
        }

        // Update functionality timestamps (legacy requirement)
        m_dwLastNetRecvTime = GameClock::GetTimeMS();
        m_dwTime = GameClock::GetTimeMS();

        // Process (using the pointer directly from the packet vector)
        if (!packet.empty()) {
             GameRecvMsgHandler(static_cast<uint32_t>(packet.size()), 
                                const_cast<char*>(packet.ptr()));
        }

        // CRITICAL FIX: The handler might have closed/deleted the socket!
        if (m_pGSock == nullptr) return;

        // Pop logic (remove from queue)
        m_pGSock->PopPacket();
        iProcessed++;
    }
}

void CGame::RestoreSprites()
{
	for (auto& [idx, spr] : m_pSprite)
	{
		spr->Restore();
	}
}

bool CGame::bSendCommand(uint32_t message_id, uint16_t command, char direction, int value1, int value2, int value3, const char* text, int value4)
{
	int result = 0;

	if ((m_pGSock == 0) && (m_pLSock == 0)) return false;
	uint32_t current_time = GameClock::GetTimeMS();
	char key = static_cast<char>(rand() % 255) + 1;

	switch (message_id) {

	case MsgId::RequestAngel:	// to Game Server
	{
		hb::net::PacketRequestAngel req{};
		req.header.msg_id = message_id;
		req.header.msg_type = 0;
		if (text != nullptr) {
			std::size_t name_len = std::strlen(text);
			if (name_len >= sizeof(req.name)) name_len = sizeof(req.name) - 1;
			std::memcpy(req.name, text, name_len);
		}
		req.angel_id = value1;
		result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req), key);
	}
	break;

	case MsgId::RequestResurrectYes: // By snoopy
	case MsgId::RequestResurrectNo:  // By snoopy
	{
		hb::net::PacketRequestHeaderOnly req{};
		req.header.msg_id = message_id;
		req.header.msg_type = 0;
		result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req), key);
	}
	break;

	case MsgId::RequestHeldenianScroll:// By snoopy
	{
		hb::net::PacketRequestHeldenianScroll req{};
		req.header.msg_id = message_id;
		req.header.msg_type = 0;
		if (text != nullptr) {
			std::size_t name_len = std::strlen(text);
			if (name_len >= sizeof(req.name)) name_len = sizeof(req.name) - 1;
			std::memcpy(req.name, text, name_len);
		}
		req.item_id = command;
		result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req), key);
	}
	break;

	case ClientMsgId::RequestTeleportList:
	{
		hb::net::PacketRequestName20 req{};
		req.header.msg_id = message_id;
		req.header.msg_type = 0;
		std::snprintf(req.name, sizeof(req.name), "%s", "William");
		result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req), key);
	}
	break;

	case ClientMsgId::RequestHeldenianTpList: // Snoopy: Heldenian TP
	{
		hb::net::PacketRequestName20 req{};
		req.header.msg_id = message_id;
		req.header.msg_type = 0;
		std::snprintf(req.name, sizeof(req.name), "%s", "Gail");
		result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req), key);
	}
	break;

	case ClientMsgId::RequestHeldenianTp: // Snoopy: Heldenian TP
	case ClientMsgId::RequestChargedTeleport:
	{
		hb::net::PacketRequestTeleportId req{};
		req.header.msg_id = message_id;
		req.header.msg_type = 0;
		req.teleport_id = value1;
		result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req), key);
	}
	break;

	case MsgId::RequestSellItemList:
	{
		hb::net::PacketRequestSellItemList req{};
		req.header.msg_id = message_id;
		req.header.msg_type = 0;
		for (int i = 0; i < game_limits::max_sell_list; i++) {
			req.entries[i].index = static_cast<uint8_t>(m_stSellItemList[i].iIndex);
			req.entries[i].amount = m_stSellItemList[i].iAmount;
		}
		result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req), key);
	}
	break;

	case MsgId::RequestRestart:
	{
		hb::net::PacketRequestHeaderOnly req{};
		req.header.msg_id = message_id;
		req.header.msg_type = 0;
		result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req), key);
	}
	break;

	case MsgId::RequestPanning:
	{
		hb::net::PacketRequestPanning req{};
		req.header.msg_id = message_id;
		req.header.msg_type = 0;
		req.dir = static_cast<uint8_t>(direction);
		result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req), key);
	}
	break;


	case ClientMsgId::GetMinimumLoadGateway:
	case MsgId::RequestLogin:
		// to Log Server
	{
		hb::net::LoginRequest req{};
		req.header.msg_id = message_id;
		req.header.msg_type = 0;
		std::snprintf(req.account_name, sizeof(req.account_name), "%s", m_pPlayer->m_cAccountName.c_str());
		std::snprintf(req.password, sizeof(req.password), "%s", m_pPlayer->m_cAccountPassword.c_str());
		std::snprintf(req.world_name, sizeof(req.world_name), "%s", m_cWorldServerName.c_str());
		result = m_pLSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req), key);
	}

	break;

	case MsgId::RequestCreateNewCharacter:
		// to Log Server
	{
		hb::net::CreateCharacterRequest req{};
		req.header.msg_id = message_id;
		req.header.msg_type = 0;
		std::snprintf(req.character_name, sizeof(req.character_name), "%s", m_pPlayer->m_cPlayerName.c_str());
		std::snprintf(req.account_name, sizeof(req.account_name), "%s", m_pPlayer->m_cAccountName.c_str());
		std::snprintf(req.password, sizeof(req.password), "%s", m_pPlayer->m_cAccountPassword.c_str());
		std::snprintf(req.world_name, sizeof(req.world_name), "%s", m_cWorldServerName.c_str());
		req.gender = static_cast<uint8_t>(m_pPlayer->m_iGender);
		req.skin = static_cast<uint8_t>(m_pPlayer->m_iSkinCol);
		req.hairstyle = static_cast<uint8_t>(m_pPlayer->m_iHairStyle);
		req.haircolor = static_cast<uint8_t>(m_pPlayer->m_iHairCol);
		req.underware = static_cast<uint8_t>(m_pPlayer->m_iUnderCol);
		req.str = static_cast<uint8_t>(m_pPlayer->m_iStatModStr);
		req.vit = static_cast<uint8_t>(m_pPlayer->m_iStatModVit);
		req.dex = static_cast<uint8_t>(m_pPlayer->m_iStatModDex);
		req.intl = static_cast<uint8_t>(m_pPlayer->m_iStatModInt);
		req.mag = static_cast<uint8_t>(m_pPlayer->m_iStatModMag);
		req.chr = static_cast<uint8_t>(m_pPlayer->m_iStatModChr);
		result = m_pLSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req), key);
	}
	break;

	case MsgId::RequestEnterGame:
		// to Log Server
	{
		hb::net::EnterGameRequestFull req{};
		req.header.msg_id = message_id;
		req.header.msg_type = static_cast<uint16_t>(m_wEnterGameType);
		std::snprintf(req.character_name, sizeof(req.character_name), "%s", m_pPlayer->m_cPlayerName.c_str());
		std::snprintf(req.map_name, sizeof(req.map_name), "%s", m_cMapName.c_str());
		std::snprintf(req.account_name, sizeof(req.account_name), "%s", m_pPlayer->m_cAccountName.c_str());
		std::snprintf(req.password, sizeof(req.password), "%s", m_pPlayer->m_cAccountPassword.c_str());
		req.level = m_pPlayer->m_iLevel;
		std::snprintf(req.world_name, sizeof(req.world_name), "%s", m_cWorldServerName.c_str());
		result = m_pLSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req), key);
	}
	break;

	case MsgId::RequestDeleteCharacter:
		// to Log Server
	{
		hb::net::DeleteCharacterRequest req{};
		req.header.msg_id = message_id;
		req.header.msg_type = static_cast<uint16_t>(m_wEnterGameType);
		std::snprintf(req.character_name, sizeof(req.character_name), "%s", m_pCharList[m_wEnterGameType - 1]->m_cName.c_str());
		std::snprintf(req.account_name, sizeof(req.account_name), "%s", m_pPlayer->m_cAccountName.c_str());
		std::snprintf(req.password, sizeof(req.password), "%s", m_pPlayer->m_cAccountPassword.c_str());
		std::snprintf(req.world_name, sizeof(req.world_name), "%s", m_cWorldServerName.c_str());
		result = m_pLSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req), key);
	}
	break;

	case MsgId::RequestSetItemPos:
		// to Game Server
	{
		hb::net::PacketRequestSetItemPos req{};
		req.header.msg_id = message_id;
		req.header.msg_type = 0;
		req.dir = static_cast<uint8_t>(direction);
		req.x = static_cast<int16_t>(value1);
		req.y = static_cast<int16_t>(value2);
		result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req));
	}
	break;

	case MsgId::CommandCheckConnection:
	{
		hb::net::PacketCommandCheckConnection req{};
		req.header.msg_id = message_id;
		req.header.msg_type = 0;
		req.time_ms = current_time;
		result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req), key);
	}

	break;

	case MsgId::RequestInitData:
	{
		hb::net::PacketRequestInitDataEx req{};
		req.header.msg_id = message_id;
		req.header.msg_type = 0;
		std::snprintf(req.player, sizeof(req.player), "%s", m_pPlayer->m_cPlayerName.c_str());
		std::snprintf(req.account, sizeof(req.account), "%s", m_pPlayer->m_cAccountName.c_str());
		std::snprintf(req.password, sizeof(req.password), "%s", m_pPlayer->m_cAccountPassword.c_str());
		req.is_observer = static_cast<uint8_t>(m_bIsObserverMode);
		std::snprintf(req.server, sizeof(req.server), "%s", m_cGameServerName.c_str());
		req.padding = 0;
		req.itemConfigHash = LocalCacheManager::Get().GetHash(ConfigCacheType::Items);
		req.magicConfigHash = LocalCacheManager::Get().GetHash(ConfigCacheType::Magic);
		req.skillConfigHash = LocalCacheManager::Get().GetHash(ConfigCacheType::Skills);
		req.npcConfigHash = LocalCacheManager::Get().GetHash(ConfigCacheType::Npcs);
		result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req), key);
	}
	break;

	case MsgId::RequestInitPlayer:
	{
		hb::net::PacketRequestInitPlayer req{};
		req.header.msg_id = message_id;
		req.header.msg_type = 0;
		std::snprintf(req.player, sizeof(req.player), "%s", m_pPlayer->m_cPlayerName.c_str());
		std::snprintf(req.account, sizeof(req.account), "%s", m_pPlayer->m_cAccountName.c_str());
		std::snprintf(req.password, sizeof(req.password), "%s", m_pPlayer->m_cAccountPassword.c_str());
		req.is_observer = static_cast<uint8_t>(m_bIsObserverMode);
		std::snprintf(req.server, sizeof(req.server), "%s", m_cGameServerName.c_str());
		req.padding = 0;
		result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req), key);
	}
	break;
	case MsgId::LevelUpSettings:
	{
		hb::net::PacketRequestLevelUpSettings req{};
		req.header.msg_id = message_id;
		req.header.msg_type = 0;
		req.str = m_pPlayer->m_wLU_Str;
		req.vit = m_pPlayer->m_wLU_Vit;
		req.dex = m_pPlayer->m_wLU_Dex;
		req.intel = m_pPlayer->m_wLU_Int;
		req.mag = m_pPlayer->m_wLU_Mag;
		req.chr = m_pPlayer->m_wLU_Char;
		result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req), key);
	}
	break;

	case MsgId::CommandChatMsg:
		if (TeleportManager::Get().IsRequested()) return false;
		if (text == 0) return false;

		// to Game Server
		{
			hb::net::PacketCommandChatMsgHeader req{};
			req.header.msg_id = message_id;
			req.header.msg_type = 0;
			req.x = m_pPlayer->m_sPlayerX;
			req.y = m_pPlayer->m_sPlayerY;
			std::snprintf(req.name, sizeof(req.name), "%s", m_pPlayer->m_cPlayerName.c_str());
			req.chat_type = static_cast<uint8_t>(value1);
			if (bCheckLocalChatCommand(text) == true) return false;
			std::size_t text_len = std::strlen(text);
			char message[300]{};
			std::memset(message, 0, sizeof(message));
			std::memcpy(message, &req, sizeof(req));
			std::memcpy(message + sizeof(req), text, text_len + 1);
			result = m_pGSock->iSendMsg(message, static_cast<int>(sizeof(req) + text_len + 1));
		}
		break;

	case MsgId::CommandCommon:
		if (TeleportManager::Get().IsRequested()) return false;
		switch (command) {
		case CommonType::BuildItem:
		{
			hb::net::PacketCommandCommonBuild req{};
			req.base.header.msg_id = message_id;
			req.base.header.msg_type = command;
			req.base.x = m_pPlayer->m_sPlayerX;
			req.base.y = m_pPlayer->m_sPlayerY;
			req.base.dir = static_cast<uint8_t>(direction);
			if (text != 0) {
				std::size_t name_len = std::strlen(text);
				if (name_len > sizeof(req.name)) name_len = sizeof(req.name);
				std::memcpy(req.name, text, name_len);
			}
			req.item_ids[0] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV1);
			req.item_ids[1] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV2);
			req.item_ids[2] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV3);
			req.item_ids[3] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV4);
			req.item_ids[4] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV5);
			req.item_ids[5] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV6);
			result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req));
		}
		break;

		case CommonType::ReqCreatePortion:
		{
			hb::net::PacketCommandCommonItems req{};
			req.base.header.msg_id = message_id;
			req.base.header.msg_type = command;
			req.base.x = m_pPlayer->m_sPlayerX;
			req.base.y = m_pPlayer->m_sPlayerY;
			req.base.dir = static_cast<uint8_t>(direction);
			req.item_ids[0] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV1);
			req.item_ids[1] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV2);
			req.item_ids[2] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV3);
			req.item_ids[3] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV4);
			req.item_ids[4] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV5);
			req.item_ids[5] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV6);
			req.padding = 0;
			result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req));
		}
		break;

		//Crafting
		case CommonType::CraftItem:
		{
			hb::net::PacketCommandCommonBuild req{};
			req.base.header.msg_id = message_id;
			req.base.header.msg_type = command;
			req.base.x = m_pPlayer->m_sPlayerX;
			req.base.y = m_pPlayer->m_sPlayerY;
			req.base.dir = static_cast<uint8_t>(direction);
			std::memset(req.name, ' ', sizeof(req.name));
			req.item_ids[0] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV1);
			req.item_ids[1] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV2);
			req.item_ids[2] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV3);
			req.item_ids[3] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV4);
			req.item_ids[4] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV5);
			req.item_ids[5] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Manufacture).sV6);
			result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req));
		}
		break;

		// Create Slate Request - Diuuude
		case CommonType::ReqCreateSlate:
		{
			hb::net::PacketCommandCommonItems req{};
			req.base.header.msg_id = message_id;
			req.base.header.msg_type = command;
			req.base.x = m_pPlayer->m_sPlayerX;
			req.base.y = m_pPlayer->m_sPlayerY;
			req.base.dir = static_cast<uint8_t>(direction);
			req.item_ids[0] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Slates).sV1);
			req.item_ids[1] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Slates).sV2);
			req.item_ids[2] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Slates).sV3);
			req.item_ids[3] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Slates).sV4);
			req.item_ids[4] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Slates).sV5);
			req.item_ids[5] = static_cast<uint8_t>(m_dialogBoxManager.Info(DialogBoxId::Slates).sV6);
			req.padding = 0;
			result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req));
		}
		break;

		// Magic spell casting - uses time_ms field to carry target object ID for auto-aim
		case CommonType::Magic:
		{
			hb::net::PacketCommandCommonWithTime req{};
			req.base.header.msg_id = message_id;
			req.base.header.msg_type = command;
			req.base.x = m_pPlayer->m_sPlayerX;
			req.base.y = m_pPlayer->m_sPlayerY;
			req.base.dir = static_cast<uint8_t>(direction);
			req.v1 = value1;  // target X
			req.v2 = value2;  // target Y
			req.v3 = value3;  // magic type (100-199)
			req.time_ms = static_cast<uint32_t>(value4);  // target object ID (0 = tile-based, >0 = entity tracking)
			result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req));
		}
		break;

		default:
			if (text == 0)
			{
				hb::net::PacketCommandCommonWithTime req{};
				req.base.header.msg_id = message_id;
				req.base.header.msg_type = command;
				req.base.x = m_pPlayer->m_sPlayerX;
				req.base.y = m_pPlayer->m_sPlayerY;
				req.base.dir = static_cast<uint8_t>(direction);
				req.v1 = value1;
				req.v2 = value2;
				req.v3 = value3;
				req.time_ms = current_time;
				result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req));
			}
			else
			{
				hb::net::PacketCommandCommonWithString req{};
				req.base.header.msg_id = message_id;
				req.base.header.msg_type = command;
				req.base.x = m_pPlayer->m_sPlayerX;
				req.base.y = m_pPlayer->m_sPlayerY;
				req.base.dir = static_cast<uint8_t>(direction);
				req.v1 = value1;
				req.v2 = value2;
				req.v3 = value3;
				std::memset(req.text, 0, sizeof(req.text));
				std::size_t text_len = std::strlen(text);
				if (text_len > sizeof(req.text)) text_len = sizeof(req.text);
				std::memcpy(req.text, text, text_len);
				req.v4 = value4;
				result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req));
			}
			break;
		}

		break;

	case MsgId::RequestCreateNewGuild:
	case MsgId::RequestDisbandGuild:
		// to Game Server
	{
		hb::net::PacketRequestGuildAction req{};
		req.header.msg_id = message_id;
		req.header.msg_type = MsgType::Confirm;
		std::snprintf(req.player, sizeof(req.player), "%s", m_pPlayer->m_cPlayerName.c_str());
		std::snprintf(req.account, sizeof(req.account), "%s", m_pPlayer->m_cAccountName.c_str());
		std::snprintf(req.password, sizeof(req.password), "%s", m_pPlayer->m_cAccountPassword.c_str());
		std::snprintf(req.guild, sizeof(req.guild), "%s", m_pPlayer->m_cGuildName.c_str());
		CMisc::ReplaceString(req.guild, ' ', '_');
		result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req), key);
	}
	break;

	case MsgId::RequestTeleport:
	{
		hb::net::PacketRequestHeaderOnly req{};
		req.header.msg_id = message_id;
		req.header.msg_type = MsgType::Confirm;
		result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req));
	}

	TeleportManager::Get().SetRequested(true);
	break;

	case MsgId::RequestCivilRight:
	{
		hb::net::PacketRequestHeaderOnly req{};
		req.header.msg_id = message_id;
		req.header.msg_type = MsgType::Confirm;
		result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req));
	}
	break;

	case MsgId::RequestRetrieveItem:
	{
		hb::net::PacketRequestRetrieveItem req{};
		req.header.msg_id = message_id;
		req.header.msg_type = MsgType::Confirm;
		req.item_slot = static_cast<uint8_t>(value1);
		result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req));
	}
	break;

	case MsgId::RequestNoticement:
	{
		hb::net::PacketRequestNoticement req{};
		req.header.msg_id = message_id;
		req.header.msg_type = 0;
		req.value = value1;
		result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req), key);
	}
	break;

	case  MsgId::RequestFightZoneReserve:
	{
		hb::net::PacketRequestFightzoneReserve req{};
		req.header.msg_id = message_id;
		req.header.msg_type = 0;
		req.fightzone = value1;
		result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req));
	}
	break;

	case MsgId::StateChangePoint:
	{
		hb::net::PacketRequestStateChange req{};
		req.header.msg_id = message_id;
		req.header.msg_type = 0;
		req.str = static_cast<int16_t>(-m_pPlayer->m_wLU_Str);
		req.vit = static_cast<int16_t>(-m_pPlayer->m_wLU_Vit);
		req.dex = static_cast<int16_t>(-m_pPlayer->m_wLU_Dex);
		req.intel = static_cast<int16_t>(-m_pPlayer->m_wLU_Int);
		req.mag = static_cast<int16_t>(-m_pPlayer->m_wLU_Mag);
		req.chr = static_cast<int16_t>(-m_pPlayer->m_wLU_Char);
		result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req));
	}
	break;

	default:
		if (TeleportManager::Get().IsRequested()) return false;
		if ((command == Type::Attack) || (command == Type::AttackMove))
		{
			hb::net::PacketCommandMotionAttack req{};
			req.base.header.msg_id = message_id;
			req.base.header.msg_type = command;
			req.base.x = m_pPlayer->m_sPlayerX;
			req.base.y = m_pPlayer->m_sPlayerY;
			req.base.dir = static_cast<uint8_t>(direction);
			req.base.dx = static_cast<int16_t>(value1);
			req.base.dy = static_cast<int16_t>(value2);
			req.base.type = static_cast<int16_t>(value3);
			req.target_id = static_cast<uint16_t>(value4);
			req.time_ms = current_time;
			result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req));
		}
		else
		{
			hb::net::PacketCommandMotionSimple req{};
			req.base.header.msg_id = message_id;
			req.base.header.msg_type = command;
			req.base.x = m_pPlayer->m_sPlayerX;
			req.base.y = m_pPlayer->m_sPlayerY;
			req.base.dir = static_cast<uint8_t>(direction);
			req.base.dx = static_cast<int16_t>(value1);
			req.base.dy = static_cast<int16_t>(value2);
			req.base.type = static_cast<int16_t>(value3);
			req.time_ms = current_time;
			result = m_pGSock->iSendMsg(reinterpret_cast<char*>(&req), sizeof(req)); //v2.171
		}
		m_pPlayer->m_Controller.IncrementCommandCount();
		break;
	}
	switch (result) {
	case sock::Event::SocketClosed:
	case sock::Event::SocketError:
	case sock::Event::QueueFull:
		printf("[ERROR] bSendCommand failed: ret=%d msgid=0x%X cmd=0x%X\n", result, message_id, command);
		ChangeGameMode(GameMode::ConnectionLost);
	m_pGSock.reset();
	break;

	case sock::Event::CriticalError:
	{
		std::string cDbg;
		cDbg = std::format("[NETWARN] bSendCommand: CRITICAL ret={} msgid=0x{:X} cmd=0x{:X}\n", result, message_id, command);
		printf("%s", cDbg.c_str());
	}
	m_pGSock.reset();
	hb::shared::render::Window::Close();
	break;
	}
	return true;
}


bool CGame::_bDecodeItemConfigFileContents(char* pData, uint32_t dwMsgSize)
{
	LocalCacheManager::Get().AccumulatePacket(ConfigCacheType::Items, pData, dwMsgSize);

	// Parse binary item config packet
	constexpr size_t headerSize = sizeof(hb::net::PacketItemConfigHeader);
	constexpr size_t entrySize = sizeof(hb::net::PacketItemConfigEntry);

	if (dwMsgSize < headerSize) {
		DevConsole::Get().Printf("[ITEMCFG] Decode FAIL: size %u < header %u", dwMsgSize, (uint32_t)headerSize);
		return false;
	}

	const auto* pktHeader = reinterpret_cast<const hb::net::PacketItemConfigHeader*>(pData);
	uint16_t itemCount = pktHeader->itemCount;
	uint16_t totalItems = pktHeader->totalItems;

	if (dwMsgSize < headerSize + (itemCount * entrySize)) {
		DevConsole::Get().Printf("[ITEMCFG] Decode FAIL: size %u < needed %u (count=%u entry=%u)",
			dwMsgSize, (uint32_t)(headerSize + itemCount * entrySize), itemCount, (uint32_t)entrySize);
		return false;
	}

	const auto* entries = reinterpret_cast<const hb::net::PacketItemConfigEntry*>(pData + headerSize);

	for (uint16_t i = 0; i < itemCount; i++) {
		const auto& entry = entries[i];
		int itemId = entry.itemId;

		if (itemId <= 0 || itemId >= 5000) {
			continue;
		}

		// Delete existing item if present (shouldn't happen, but be safe)
		if (m_pItemConfigList[itemId] != 0) {
			m_pItemConfigList[itemId].reset();
		}

		m_pItemConfigList[itemId] = std::make_unique<CItem>();
		CItem* pItem = m_pItemConfigList[itemId].get();

		pItem->m_sIDnum = entry.itemId;
		std::snprintf(pItem->m_cName, sizeof(pItem->m_cName), "%s", entry.name);
		pItem->m_cItemType = entry.itemType;
		pItem->m_cEquipPos = entry.equipPos;
		pItem->m_sItemEffectType = entry.effectType;
		pItem->m_sItemEffectValue1 = entry.effectValue1;
		pItem->m_sItemEffectValue2 = entry.effectValue2;
		pItem->m_sItemEffectValue3 = entry.effectValue3;
		pItem->m_sItemEffectValue4 = entry.effectValue4;
		pItem->m_sItemEffectValue5 = entry.effectValue5;
		pItem->m_sItemEffectValue6 = entry.effectValue6;
		pItem->m_wMaxLifeSpan = entry.maxLifeSpan;
		pItem->m_sSpecialEffect = entry.specialEffect;
		pItem->m_sSprite = entry.sprite;
		pItem->m_sSpriteFrame = entry.spriteFrame;
		pItem->m_bIsForSale = (entry.price >= 0);
		pItem->m_wPrice = static_cast<uint32_t>(entry.price >= 0 ? entry.price : -entry.price);
		pItem->m_wWeight = entry.weight;
		pItem->m_cApprValue = entry.apprValue;
		pItem->m_cSpeed = entry.speed;
		pItem->m_sLevelLimit = entry.levelLimit;
		pItem->m_cGenderLimit = entry.genderLimit;
		pItem->m_sSpecialEffectValue1 = entry.specialEffectValue1;
		pItem->m_sSpecialEffectValue2 = entry.specialEffectValue2;
		pItem->m_sRelatedSkill = entry.relatedSkill;
		pItem->m_cCategory = entry.category;
		pItem->m_cItemColor = entry.itemColor;
	}

	// Log total count on last packet
	int totalLoaded = 0;
	for (int j = 0; j < 5000; j++) {
		if (m_pItemConfigList[j] != 0) totalLoaded++;
	}
	if (totalLoaded >= totalItems && !LocalCacheManager::Get().IsReplaying()) {
		if (LocalCacheManager::Get().FinalizeAndSave(ConfigCacheType::Items)) {
			DevConsole::Get().Printf("[ITEMCFG] Downloaded %d item configs, cached", totalLoaded);
		} else {
			DevConsole::Get().Printf("[ITEMCFG] Downloaded %d item configs", totalLoaded);
		}
	}

	return true;
}

bool CGame::_bDecodeMagicConfigFileContents(char* pData, uint32_t dwMsgSize)
{
	LocalCacheManager::Get().AccumulatePacket(ConfigCacheType::Magic, pData, dwMsgSize);

	constexpr size_t headerSize = sizeof(hb::net::PacketMagicConfigHeader);
	constexpr size_t entrySize = sizeof(hb::net::PacketMagicConfigEntry);

	if (dwMsgSize < headerSize) {
		DevConsole::Get().Printf("[MAGICCFG] Decode FAIL: size %u < header %u", dwMsgSize, (uint32_t)headerSize);
		return false;
	}

	const auto* pktHeader = reinterpret_cast<const hb::net::PacketMagicConfigHeader*>(pData);
	uint16_t magicCount = pktHeader->magicCount;
	uint16_t totalMagics = pktHeader->totalMagics;

	if (dwMsgSize < headerSize + (magicCount * entrySize)) {
		DevConsole::Get().Printf("[MAGICCFG] Decode FAIL: size %u < needed %u (count=%u entry=%u)",
			dwMsgSize, (uint32_t)(headerSize + magicCount * entrySize), magicCount, (uint32_t)entrySize);
		return false;
	}

	const auto* entries = reinterpret_cast<const hb::net::PacketMagicConfigEntry*>(pData + headerSize);

	for (uint16_t i = 0; i < magicCount; i++) {
		const auto& entry = entries[i];
		int magicId = entry.magicId;

		if (magicId < 0 || magicId >= hb::shared::limits::MaxMagicType) {
			continue;
		}

		if (m_pMagicCfgList[magicId] != 0) {
			m_pMagicCfgList[magicId].reset();
		}

		m_pMagicCfgList[magicId] = std::make_unique<CMagic>();
		CMagic* pMagic = m_pMagicCfgList[magicId].get();
		pMagic->m_cName = entry.name;
		pMagic->m_sValue1 = entry.manaCost;
		pMagic->m_sValue2 = entry.intLimit;
		pMagic->m_sValue3 = (entry.goldCost >= 0) ? entry.goldCost : -entry.goldCost;
		pMagic->m_bIsVisible = (entry.isVisible != 0);
		pMagic->m_sType = entry.magicType;
		pMagic->m_sAoERadiusX = entry.aoeRadiusX;
		pMagic->m_sAoERadiusY = entry.aoeRadiusY;
		pMagic->m_sDynamicPattern = entry.dynamicPattern;
		pMagic->m_sDynamicRadius = entry.dynamicRadius;
	}

	// Log total count on last packet
	int totalLoaded = 0;
	for (int j = 0; j < hb::shared::limits::MaxMagicType; j++) {
		if (m_pMagicCfgList[j] != 0) totalLoaded++;
	}
	if (totalLoaded >= totalMagics && !LocalCacheManager::Get().IsReplaying()) {
		if (LocalCacheManager::Get().FinalizeAndSave(ConfigCacheType::Magic)) {
			DevConsole::Get().Printf("[MAGICCFG] Downloaded %d magic configs, cached", totalLoaded);
		} else {
			DevConsole::Get().Printf("[MAGICCFG] Downloaded %d magic configs", totalLoaded);
		}
	}

	return true;
}

bool CGame::_bDecodeSkillConfigFileContents(char* pData, uint32_t dwMsgSize)
{
	LocalCacheManager::Get().AccumulatePacket(ConfigCacheType::Skills, pData, dwMsgSize);

	constexpr size_t headerSize = sizeof(hb::net::PacketSkillConfigHeader);
	constexpr size_t entrySize = sizeof(hb::net::PacketSkillConfigEntry);

	if (dwMsgSize < headerSize) {
		DevConsole::Get().Printf("[SKILLCFG] Decode FAIL: size %u < header %u", dwMsgSize, (uint32_t)headerSize);
		return false;
	}

	const auto* pktHeader = reinterpret_cast<const hb::net::PacketSkillConfigHeader*>(pData);
	uint16_t skillCount = pktHeader->skillCount;
	uint16_t totalSkills = pktHeader->totalSkills;

	if (dwMsgSize < headerSize + (skillCount * entrySize)) {
		DevConsole::Get().Printf("[SKILLCFG] Decode FAIL: size %u < needed %u (count=%u entry=%u)",
			dwMsgSize, (uint32_t)(headerSize + skillCount * entrySize), skillCount, (uint32_t)entrySize);
		return false;
	}

	const auto* entries = reinterpret_cast<const hb::net::PacketSkillConfigEntry*>(pData + headerSize);

	for (uint16_t i = 0; i < skillCount; i++) {
		const auto& entry = entries[i];
		int skillId = entry.skillId;

		if (skillId < 0 || skillId >= hb::shared::limits::MaxSkillType) {
			continue;
		}

		if (m_pSkillCfgList[skillId] != 0) {
			m_pSkillCfgList[skillId].reset();
		}

		m_pSkillCfgList[skillId] = std::make_unique<CSkill>();
		CSkill* pSkill = m_pSkillCfgList[skillId].get();
		pSkill->m_cName = entry.name;
		pSkill->m_bIsUseable = (entry.isUseable != 0);
		pSkill->m_cUseMethod = entry.useMethod;
		// Apply mastery level if already received from InitItemList
		pSkill->m_iLevel = static_cast<int>(m_pPlayer->m_iSkillMastery[skillId]);
	}

	// Log total count on last packet
	int totalLoaded = 0;
	for (int j = 0; j < hb::shared::limits::MaxSkillType; j++) {
		if (m_pSkillCfgList[j] != 0) totalLoaded++;
	}
	if (totalLoaded >= totalSkills && !LocalCacheManager::Get().IsReplaying()) {
		if (LocalCacheManager::Get().FinalizeAndSave(ConfigCacheType::Skills)) {
			DevConsole::Get().Printf("[SKILLCFG] Downloaded %d skill configs, cached", totalLoaded);
		} else {
			DevConsole::Get().Printf("[SKILLCFG] Downloaded %d skill configs", totalLoaded);
		}
	}

	return true;
}

bool CGame::_bDecodeNpcConfigFileContents(char* pData, uint32_t dwMsgSize)
{
	LocalCacheManager::Get().AccumulatePacket(ConfigCacheType::Npcs, pData, dwMsgSize);

	constexpr size_t headerSize = sizeof(hb::net::PacketNpcConfigHeader);
	constexpr size_t entrySize = sizeof(hb::net::PacketNpcConfigEntry);

	if (dwMsgSize < headerSize) {
		DevConsole::Get().Printf("[NPCCFG] Decode FAIL: size %u < header %u", dwMsgSize, (uint32_t)headerSize);
		return false;
	}

	const auto* pktHeader = reinterpret_cast<const hb::net::PacketNpcConfigHeader*>(pData);
	uint16_t npcCount = pktHeader->npcCount;
	uint16_t totalNpcs = pktHeader->totalNpcs;

	if (dwMsgSize < headerSize + (npcCount * entrySize)) {
		DevConsole::Get().Printf("[NPCCFG] Decode FAIL: size %u < needed %u (count=%u entry=%u)",
			dwMsgSize, (uint32_t)(headerSize + npcCount * entrySize), npcCount, (uint32_t)entrySize);
		return false;
	}

	const auto* entries = reinterpret_cast<const hb::net::PacketNpcConfigEntry*>(pData + headerSize);

	for (uint16_t i = 0; i < npcCount; i++) {
		const auto& entry = entries[i];
		int npcId = entry.npcId;
		int npcType = entry.npcType;

		// Store by npc_id (primary key)
		if (npcId >= 0 && npcId < hb::shared::limits::MaxNpcConfigs) {
			m_npcConfigList[npcId].npcType = static_cast<short>(npcType);
			m_npcConfigList[npcId].name = entry.name;
			m_npcConfigList[npcId].valid = true;
		}

		// Update type->name reverse map (last config per type wins)
		if (npcType >= 0 && npcType < 120) {
			m_cNpcNameByType[npcType] = entry.name;
		}
	}

	// Track raw entries received across packets
	if (pktHeader->packetIndex == 0) m_iNpcConfigsReceived = 0;
	m_iNpcConfigsReceived += npcCount;

	if (m_iNpcConfigsReceived >= totalNpcs && !LocalCacheManager::Get().IsReplaying()) {
		if (LocalCacheManager::Get().FinalizeAndSave(ConfigCacheType::Npcs)) {
			DevConsole::Get().Printf("[NPCCFG] Downloaded %d NPC configs, cached", m_iNpcConfigsReceived);
		} else {
			DevConsole::Get().Printf("[NPCCFG] Downloaded %d NPC configs", m_iNpcConfigsReceived);
		}
	}

	return true;
}

const char* CGame::GetNpcConfigName(short sType) const
{
	if (sType >= 0 && sType < 120 && !m_cNpcNameByType[sType].empty()) {
		return m_cNpcNameByType[sType].c_str();
	}
	return "Unknown";
}

const char* CGame::GetNpcConfigNameById(short npcConfigId) const
{
	if (npcConfigId >= 0 && npcConfigId < hb::shared::limits::MaxNpcConfigs && m_npcConfigList[npcConfigId].valid) {
		return m_npcConfigList[npcConfigId].name.c_str();
	}
	return "Unknown";
}

short CGame::ResolveNpcType(short npcConfigId) const
{
	if (npcConfigId >= 0 && npcConfigId < hb::shared::limits::MaxNpcConfigs && m_npcConfigList[npcConfigId].valid)
		return m_npcConfigList[npcConfigId].npcType;
	return 0;
}

void CGame::GameRecvMsgHandler(uint32_t dwMsgSize, char* pData)
{
	const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(pData, sizeof(hb::net::PacketHeader));
	if (!header) return;
	m_dwLastNetMsgId = header->msg_id;
	m_dwLastNetMsgTime = GameClock::GetTimeMS();
	m_dwLastNetMsgSize = dwMsgSize;
	switch (header->msg_id) {
	case MSGID_RESPONSE_CONFIGCACHESTATUS:
	{
		const auto* cachePkt = hb::net::PacketCast<hb::net::PacketResponseConfigCacheStatus>(
			pData, sizeof(hb::net::PacketResponseConfigCacheStatus));
		if (!cachePkt) break;

		struct ReplayCtx { CGame* game; };
		ReplayCtx ctx{ this };
		bool bNeedItems = false, bNeedMagic = false, bNeedSkills = false, bNeedNpcs = false;

		// Clear config arrays before replay so verification is accurate
		// (stale entries from previous login would cause false positives)
		for (auto& item : m_pItemConfigList) item.reset();
		for (auto& magic : m_pMagicCfgList) magic.reset();
		for (auto& skill : m_pSkillCfgList) skill.reset();

		if (cachePkt->itemCacheValid) {
			bool bReplayOk = LocalCacheManager::Get().ReplayFromCache(ConfigCacheType::Items,
				[](char* p, uint32_t s, void* c) -> bool {
					return static_cast<ReplayCtx*>(c)->game->_bDecodeItemConfigFileContents(p, s);
				}, &ctx);
			// Verify items actually loaded
			bool bHasItems = false;
			if (bReplayOk) {
				for (int i = 1; i < 5000; i++) { if (m_pItemConfigList[i]) { bHasItems = true; break; } }
			}
			if (bReplayOk && bHasItems) {
				m_eConfigRetry[0] = ConfigRetryLevel::None;
				DevConsole::Get().Printf("[ITEMCFG] Loaded from cache");
			} else {
				DevConsole::Get().Printf("[ITEMCFG] Cache replay %s, requesting from server",
					bReplayOk ? "decoded 0 items" : "failed");
				LocalCacheManager::Get().ResetAccumulator(ConfigCacheType::Items);
				m_eConfigRetry[0] = ConfigRetryLevel::ServerRequested;
				bNeedItems = true;
			}
		} else {
			DevConsole::Get().Printf("[ITEMCFG] Cache outdated, requesting from server");
			LocalCacheManager::Get().ResetAccumulator(ConfigCacheType::Items);
			m_eConfigRetry[0] = ConfigRetryLevel::ServerRequested;
			bNeedItems = true;
		}

		if (cachePkt->magicCacheValid) {
			bool bReplayOk = LocalCacheManager::Get().ReplayFromCache(ConfigCacheType::Magic,
				[](char* p, uint32_t s, void* c) -> bool {
					return static_cast<ReplayCtx*>(c)->game->_bDecodeMagicConfigFileContents(p, s);
				}, &ctx);
			bool bHasMagic = false;
			if (bReplayOk) {
				for (int i = 0; i < hb::shared::limits::MaxMagicType; i++) { if (m_pMagicCfgList[i]) { bHasMagic = true; break; } }
			}
			if (bReplayOk && bHasMagic) {
				m_eConfigRetry[1] = ConfigRetryLevel::None;
				DevConsole::Get().Printf("[MAGICCFG] Loaded from cache");
			} else {
				DevConsole::Get().Printf("[MAGICCFG] Cache replay %s, requesting from server",
					bReplayOk ? "decoded 0 entries" : "failed");
				LocalCacheManager::Get().ResetAccumulator(ConfigCacheType::Magic);
				m_eConfigRetry[1] = ConfigRetryLevel::ServerRequested;
				bNeedMagic = true;
			}
		} else {
			DevConsole::Get().Printf("[MAGICCFG] Cache outdated, requesting from server");
			LocalCacheManager::Get().ResetAccumulator(ConfigCacheType::Magic);
			m_eConfigRetry[1] = ConfigRetryLevel::ServerRequested;
			bNeedMagic = true;
		}

		if (cachePkt->skillCacheValid) {
			bool bReplayOk = LocalCacheManager::Get().ReplayFromCache(ConfigCacheType::Skills,
				[](char* p, uint32_t s, void* c) -> bool {
					return static_cast<ReplayCtx*>(c)->game->_bDecodeSkillConfigFileContents(p, s);
				}, &ctx);
			bool bHasSkills = false;
			if (bReplayOk) {
				for (int i = 0; i < hb::shared::limits::MaxSkillType; i++) { if (m_pSkillCfgList[i]) { bHasSkills = true; break; } }
			}
			if (bReplayOk && bHasSkills) {
				m_eConfigRetry[2] = ConfigRetryLevel::None;
				DevConsole::Get().Printf("[SKILLCFG] Loaded from cache");
			} else {
				DevConsole::Get().Printf("[SKILLCFG] Cache replay %s, requesting from server",
					bReplayOk ? "decoded 0 entries" : "failed");
				LocalCacheManager::Get().ResetAccumulator(ConfigCacheType::Skills);
				m_eConfigRetry[2] = ConfigRetryLevel::ServerRequested;
				bNeedSkills = true;
			}
		} else {
			DevConsole::Get().Printf("[SKILLCFG] Cache outdated, requesting from server");
			LocalCacheManager::Get().ResetAccumulator(ConfigCacheType::Skills);
			m_eConfigRetry[2] = ConfigRetryLevel::ServerRequested;
			bNeedSkills = true;
		}

		if (cachePkt->npcCacheValid) {
			bool bReplayOk = LocalCacheManager::Get().ReplayFromCache(ConfigCacheType::Npcs,
				[](char* p, uint32_t s, void* c) -> bool {
					return static_cast<ReplayCtx*>(c)->game->_bDecodeNpcConfigFileContents(p, s);
				}, &ctx);
			bool bHasNpcs = false;
			if (bReplayOk) {
				for (int i = 0; i < hb::shared::limits::MaxNpcConfigs; i++) { if (m_npcConfigList[i].valid) { bHasNpcs = true; break; } }
			}
			if (bReplayOk && bHasNpcs) {
				m_eConfigRetry[3] = ConfigRetryLevel::None;
				DevConsole::Get().Printf("[NPCCFG] Loaded from cache");
			} else {
				DevConsole::Get().Printf("[NPCCFG] Cache replay %s, requesting from server",
					bReplayOk ? "decoded 0 entries" : "failed");
				LocalCacheManager::Get().ResetAccumulator(ConfigCacheType::Npcs);
				m_eConfigRetry[3] = ConfigRetryLevel::ServerRequested;
				bNeedNpcs = true;
			}
		} else {
			DevConsole::Get().Printf("[NPCCFG] Cache outdated, requesting from server");
			LocalCacheManager::Get().ResetAccumulator(ConfigCacheType::Npcs);
			m_eConfigRetry[3] = ConfigRetryLevel::ServerRequested;
			bNeedNpcs = true;
		}

		if (bNeedItems || bNeedMagic || bNeedSkills || bNeedNpcs) {
			_RequestConfigsFromServer(bNeedItems, bNeedMagic, bNeedSkills, bNeedNpcs);
			m_dwConfigRequestTime = GameClock::GetTimeMS();
		} else {
			m_bConfigsReady = true;
			if (m_bInitDataReady) {
				DevConsole::Get().Printf("[INIT] Entering game (configs triggered)");
				GameModeManager::set_screen<Screen_OnGame>();
				m_bInitDataReady = false;
			}
		}
	}
	break;

	case MsgId::NotifyConfigReload:
	{
		const auto* reloadPkt = hb::net::PacketCast<hb::net::PacketNotifyConfigReload>(
			pData, sizeof(hb::net::PacketNotifyConfigReload));
		if (!reloadPkt) break;

		if (reloadPkt->reloadItems)
			LocalCacheManager::Get().ResetAccumulator(ConfigCacheType::Items);
		if (reloadPkt->reloadMagic)
			LocalCacheManager::Get().ResetAccumulator(ConfigCacheType::Magic);
		if (reloadPkt->reloadSkills)
			LocalCacheManager::Get().ResetAccumulator(ConfigCacheType::Skills);
		if (reloadPkt->reloadNpcs) {
			LocalCacheManager::Get().ResetAccumulator(ConfigCacheType::Npcs);
			m_npcConfigList.fill({});
			m_iNpcConfigsReceived = 0;
		}

		SetTopMsg((char*)"Administration kicked off a config reload, some lag may occur.", 5);
	}
	break;

	case MsgId::ItemConfigContents:
		_bDecodeItemConfigFileContents(pData, dwMsgSize);
		m_eConfigRetry[0] = ConfigRetryLevel::None;
		_CheckConfigsReadyAndEnterGame();
		break;
	case MsgId::MagicConfigContents:
		_bDecodeMagicConfigFileContents(pData, dwMsgSize);
		m_eConfigRetry[1] = ConfigRetryLevel::None;
		_CheckConfigsReadyAndEnterGame();
		break;
	case MsgId::SkillConfigContents:
		_bDecodeSkillConfigFileContents(pData, dwMsgSize);
		m_eConfigRetry[2] = ConfigRetryLevel::None;
		_CheckConfigsReadyAndEnterGame();
		break;
	case MsgId::NpcConfigContents:
		_bDecodeNpcConfigFileContents(pData, dwMsgSize);
		m_eConfigRetry[3] = ConfigRetryLevel::None;
		_CheckConfigsReadyAndEnterGame();
		break;
	case ClientMsgId::ResponseChargedTeleport:
		TeleportManager::Get().HandleChargedTeleport(pData);
		break;

	case ClientMsgId::ResponseTeleportList:
		TeleportManager::Get().HandleTeleportList(pData);
		break;

	case ClientMsgId::ResponseHeldenianTpList: // Snoopy Heldenian TP
		TeleportManager::Get().HandleHeldenianTeleportList(pData);
		break;

	case MsgId::ResponseNoticement:
		NoticementHandler(pData);
		break;

	case MsgId::DynamicObject:
		DynamicObjectHandler(pData);
		break;

	case MsgId::ResponseInitPlayer:
		InitPlayerResponseHandler(pData);
		break;

	case MsgId::ResponseInitData:
		DevConsole::Get().Printf("[INIT] INITDATA received");
		InitDataResponseHandler(pData);
		break;

	case MsgId::ResponseMotion:
		MotionResponseHandler(pData);
		break;

	case MsgId::EventCommon:
		CommonEventHandler(pData);
		break;

	case MsgId::EventMotion:
		MotionEventHandler(pData);
		break;

	case MsgId::EventLog:
		LogEventHandler(pData);
		break;

	case MsgId::CommandChatMsg:
		ChatMsgHandler(pData);
		break;

	case MsgId::PlayerItemListContents:
		DevConsole::Get().Printf("[INIT] PLAYERITEMLISTCONTENTS received (size=%u)", dwMsgSize);
		InitItemList(pData);
		break;

	case MsgId::Notify:
		if (m_pNetworkMessageManager) {

			m_pNetworkMessageManager->ProcessMessage(MsgId::Notify, pData, dwMsgSize);
		}
		break;

	case MsgId::ResponseCreateNewGuild:
		CreateNewGuildResponseHandler(pData);
		break;

	case MsgId::ResponseDisbandGuild:
		DisbandGuildResponseHandler(pData);
		break;

	case MsgId::PlayerCharacterContents:
		DevConsole::Get().Printf("[INIT] PLAYERCHARACTERCONTENTS received");
		InitPlayerCharacteristics(pData);
		break;

	case MsgId::ResponseCivilRight:
		CivilRightAdmissionHandler(pData);
		break;

	case MsgId::ResponseRetrieveItem:
		RetrieveItemHandler(pData);
		break;

	case MsgId::ResponsePanning:
		ResponsePanningHandler(pData);
		break;

	case MsgId::ResponseFightZoneReserve:
		ReserveFightzoneResponseHandler(pData);
		break;

	case MSGID_RESPONSE_SHOP_CONTENTS:
		ShopManager::Get().HandleResponse(pData);
		break;

	case MsgId::CommandCheckConnection:
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketCommandCheckConnection>(
			pData, sizeof(hb::net::PacketCommandCheckConnection));
		if (!pkt) return;
		uint32_t dwRecvTime = m_dwLastNetRecvTime;
		if (dwRecvTime == 0) dwRecvTime = GameClock::GetTimeMS();
		if (dwRecvTime >= pkt->time_ms)
		{
			m_iLatencyMs = static_cast<int>(dwRecvTime - pkt->time_ms);
		}
	}
	break;
	}
}

void CGame::ConnectionEstablishHandler(char cWhere)
{
	ChangeGameMode(GameMode::WaitingResponse);

	switch (cWhere) {
	case static_cast<int>(ServerType::Game):
		bSendCommand(MsgId::RequestInitPlayer, 0, 0, 0, 0, 0, 0);
		break;

	case static_cast<int>(ServerType::Log):
		switch (m_dwConnectMode) {
		case MsgId::RequestLogin:
			if (m_cWorldServerName.size() == 0) {
				printf("[ERROR] Login failed - m_cWorldServerName is empty\n");
			}
			bSendCommand(MsgId::RequestLogin, 0, 0, 0, 0, 0, 0);
			break;
		case MsgId::RequestCreateNewCharacter:
			bSendCommand(MsgId::RequestCreateNewCharacter, 0, 0, 0, 0, 0, 0);
			break;
		case MsgId::RequestEnterGame:
			bSendCommand(MsgId::RequestEnterGame, 0, 0, 0, 0, 0, 0);
			break;
		case MsgId::RequestDeleteCharacter:
			bSendCommand(MsgId::RequestDeleteCharacter, 0, 0, 0, 0, 0, 0);
			break;
		case MsgId::RequestInputKeyCode:
			bSendCommand(MsgId::RequestInputKeyCode, 0, 0, 0, 0, 0, 0);
			break;
		}

		// Send any pending packet built directly by a screen/overlay
		if (!m_pendingLoginPacket.empty()) {
			char cKey = static_cast<char>(rand() % 255) + 1;
			m_pLSock->iSendMsg(m_pendingLoginPacket.data(), static_cast<uint32_t>(m_pendingLoginPacket.size()), cKey);
			m_pendingLoginPacket.clear();
		}
		break;
	}
}

void CGame::InitPlayerResponseHandler(char* pData)
{
	const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(
		pData, sizeof(hb::net::PacketHeader));
	if (!header) return;
	switch (header->msg_type) {
	case MsgType::Confirm:
		bSendCommand(MsgId::RequestInitData, 0, 0, 0, 0, 0, 0);
		ChangeGameMode(GameMode::WaitingInitData);
		break;

	case MsgType::Reject:
		std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "3J");
		ChangeGameMode(GameMode::LogResMsg);
		break;
	}
}

void CGame::OnTimer()
{
	if (GameModeManager::GetModeValue() < 0) return;
	uint32_t dwTime = GameClock::GetTimeMS();

	if (GameModeManager::GetMode() != GameMode::Loading) {
		if ((dwTime - m_dwCheckSprTime) > 8000)
		{
			m_dwCheckSprTime = dwTime;
			ReleaseUnusedSprites();
		}
		if ((dwTime - m_dwCheckConnectionTime) > 1000)
		{
			m_dwCheckConnectionTime = dwTime;
			if ((m_pGSock != 0) && (m_pGSock->m_bIsAvailable == true))
				bSendCommand(MsgId::CommandCheckConnection, MsgType::Confirm, 0, 0, 0, 0, 0);
		}
	}

	if (GameModeManager::GetMode() == GameMode::MainGame)
	{
		if ((dwTime - m_dwCheckConnTime) > 5000)
		{
			m_dwCheckConnTime = dwTime;
			if ((m_bIsCrusadeMode) && (m_pPlayer->m_iCrusadeDuty == 0)) m_dialogBoxManager.EnableDialogBox(DialogBoxId::CrusadeJob, 1, 0, 0);
		}

		if ((dwTime - m_dwCheckChatTime) > 2000)
		{
			m_dwCheckChatTime = m_dwTime;
			m_floatingText.ReleaseExpired(m_dwCurTime);
			if (m_pPlayer->m_Controller.GetCommandCount() >= 6)
			{
				m_iNetLagCount++;
				if (m_iNetLagCount >= 7)
				{
					printf("[ERROR] NetLag threshold reached, disconnecting\n");
					ChangeGameMode(GameMode::ConnectionLost);
					m_pGSock.reset();
					return;
				}
			}
			else m_iNetLagCount = 0;
		}
	}
}

void CGame::bItemDrop_ExternalScreen(char item_id, short mouse_x, short mouse_y)
{
	std::string name;
	short owner_type, dialog_x, dialog_y;
	hb::shared::entity::PlayerStatus status;

	if (InventoryManager::Get().CheckItemOperationEnabled(item_id) == false) return;

	if ((m_sMCX != 0) && (m_sMCY != 0) && (abs(m_pPlayer->m_sPlayerX - m_sMCX) <= 8) && (abs(m_pPlayer->m_sPlayerY - m_sMCY) <= 8))
	{
		name.clear();
		m_pMapData->bGetOwner(m_sMCX, m_sMCY, name, &owner_type, &status, &m_wCommObjectID);
		if (m_pPlayer->m_cPlayerName == name)
		{
		}
		else
		{
			CItem* pCfg = GetItemConfig(m_pItemList[item_id]->m_sIDnum);
			if (pCfg && ((pCfg->GetItemType() == ItemType::Consume) || (pCfg->GetItemType() == ItemType::Arrow))
				&& (m_pItemList[item_id]->m_dwCount > 1))
			{
				m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).sX = mouse_x - 140;
				m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).sY = mouse_y - 70;
				if (m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).sY < 0) m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).sY = 0;
				m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).sV1 = m_sMCX;
				m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).sV2 = m_sMCY;
				m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).sV3 = owner_type;
				m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).sV4 = m_wCommObjectID;
				std::memset(m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).cStr, 0, sizeof(m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).cStr));
				if (owner_type < 10)
					std::snprintf(m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).cStr, sizeof(m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).cStr), "%s", name.c_str());
				else
				{
					std::snprintf(m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).cStr, sizeof(m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).cStr), "%s", GetNpcConfigName(owner_type));
				}
				m_dialogBoxManager.EnableDialogBox(DialogBoxId::ItemDropExternal, item_id, m_pItemList[item_id]->m_dwCount, 0);
			}
			else
			{
				switch (owner_type) {
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
					m_dialogBoxManager.EnableDialogBox(DialogBoxId::NpcActionQuery, 1, item_id, owner_type);
					m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sV3 = 1;
					m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sV4 = m_wCommObjectID;
					m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sV5 = m_sMCX;
					m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sV6 = m_sMCY;

					dialog_x = mouse_x - 117;
					dialog_y = mouse_y - 50;
					if (dialog_x < 0) dialog_x = 0;
					if ((dialog_x + 235) > LOGICAL_MAX_X()) dialog_x = LOGICAL_MAX_X() - 235;
					if (dialog_y < 0) dialog_y = 0;
					if ((dialog_y + 100) > LOGICAL_MAX_Y()) dialog_y = LOGICAL_MAX_Y() - 100;
					m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sX = dialog_x;
					m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sY = dialog_y;

					std::snprintf(m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).cStr, sizeof(m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).cStr), "%s", name.c_str());
					//bSendCommand(MsgId::CommandCommon, CommonType::GiveItemToChar, cItemID, 1, m_sMCX, m_sMCY, m_pItemList[cItemID]->m_cName); //v1.4
					break;

				case hb::shared::owner::Howard: // Howard
					m_dialogBoxManager.EnableDialogBox(DialogBoxId::NpcActionQuery, 3, item_id, owner_type);
					m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sV3 = 1;
					m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sV4 = m_wCommObjectID; // v1.4
					m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sV5 = m_sMCX;
					m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sV6 = m_sMCY;

					dialog_x = mouse_x - 117;
					dialog_y = mouse_y - 50;
					if (dialog_x < 0) dialog_x = 0;
					if ((dialog_x + 235) > LOGICAL_MAX_X()) dialog_x = LOGICAL_MAX_X() - 235;
					if (dialog_y < 0) dialog_y = 0;
					if ((dialog_y + 100) > LOGICAL_MAX_Y()) dialog_y = LOGICAL_MAX_Y() - 100;
					m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sX = dialog_x;
					m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sY = dialog_y;

					std::snprintf(m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).cStr, hb::shared::limits::NpcNameLen, "%s", GetNpcConfigName(owner_type));
					break;

				case hb::shared::owner::ShopKeeper: // ShopKeeper-W
				case hb::shared::owner::Tom: // Tom
					m_dialogBoxManager.EnableDialogBox(DialogBoxId::NpcActionQuery, 2, item_id, owner_type);
					m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sV3 = 1;
					m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sV4 = m_wCommObjectID; // v1.4
					m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sV5 = m_sMCX;
					m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sV6 = m_sMCY;

					dialog_x = mouse_x - 117;
					dialog_y = mouse_y - 50;
					if (dialog_x < 0) dialog_x = 0;
					if ((dialog_x + 235) > LOGICAL_MAX_X()) dialog_x = LOGICAL_MAX_X() - 235;
					if (dialog_y < 0) dialog_y = 0;
					if ((dialog_y + 100) > LOGICAL_MAX_Y()) dialog_y = LOGICAL_MAX_Y() - 100;
					m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sX = dialog_x;
					m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sY = dialog_y;

					std::snprintf(m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).cStr, hb::shared::limits::NpcNameLen, "%s", GetNpcConfigName(owner_type));
					break;

				default:
					if (pCfg) bSendCommand(MsgId::CommandCommon, CommonType::GiveItemToChar, item_id, 1, m_sMCX, m_sMCY, pCfg->m_cName);
					break;
				}
			}
			m_bIsItemDisabled[item_id] = true;
		}
	}
	else
	{
		CItem* pCfg2 = GetItemConfig(m_pItemList[item_id]->m_sIDnum);
		if (pCfg2 && ((pCfg2->GetItemType() == ItemType::Consume) || (pCfg2->GetItemType() == ItemType::Arrow))
			&& (m_pItemList[item_id]->m_dwCount > 1))
		{
			m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).sX = mouse_x - 140;
			m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).sY = mouse_y - 70;
			if (m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).sY < 0)		m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).sY = 0;
			m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).sV1 = 0;
			m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).sV2 = 0;
			m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).sV3 = 0;
			m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).sV4 = 0;
			std::memset(m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).cStr, 0, sizeof(m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).cStr));
			m_dialogBoxManager.EnableDialogBox(DialogBoxId::ItemDropExternal, item_id, m_pItemList[item_id]->m_dwCount, 0);
		}
		else
		{
			if (_ItemDropHistory(m_pItemList[item_id]->m_sIDnum))
			{
				m_dialogBoxManager.Info(DialogBoxId::ItemDropConfirm).sX = mouse_x - 140;
				m_dialogBoxManager.Info(DialogBoxId::ItemDropConfirm).sY = mouse_y - 70;
				if (m_dialogBoxManager.Info(DialogBoxId::ItemDropConfirm).sY < 0)	m_dialogBoxManager.Info(DialogBoxId::ItemDropConfirm).sY = 0;
				m_dialogBoxManager.Info(DialogBoxId::ItemDropConfirm).sV1 = 0;
				m_dialogBoxManager.Info(DialogBoxId::ItemDropConfirm).sV2 = 0;
				m_dialogBoxManager.Info(DialogBoxId::ItemDropConfirm).sV3 = 1;
				m_dialogBoxManager.Info(DialogBoxId::ItemDropConfirm).sV4 = 0;
				m_dialogBoxManager.Info(DialogBoxId::ItemDropConfirm).sV5 = item_id;
				std::memset(m_dialogBoxManager.Info(DialogBoxId::ItemDropConfirm).cStr, 0, sizeof(m_dialogBoxManager.Info(DialogBoxId::ItemDropConfirm).cStr));
				m_dialogBoxManager.EnableDialogBox(DialogBoxId::ItemDropConfirm, item_id, m_pItemList[item_id]->m_dwCount, 0);
			}
			else
			{
				if (pCfg2) bSendCommand(MsgId::CommandCommon, CommonType::ItemDrop, 0, item_id, 1, 0, pCfg2->m_cName);
			}
		}
		m_bIsItemDisabled[item_id] = true;
	}
}

void CGame::CommonEventHandler(char* pData)
{
	WORD wEventType;
	short sX, sY, sV1, sV2, sV3, sV4;
	uint32_t dwV4;

	const auto* base = hb::net::PacketCast<hb::net::PacketEventCommonBase>(pData, sizeof(hb::net::PacketEventCommonBase));
	if (!base) return;
	wEventType = base->header.msg_type;
	sX = base->x;
	sY = base->y;
	sV1 = base->v1;
	sV2 = base->v2;
	sV3 = base->v3;

	switch (wEventType) {
	case CommonType::ItemDrop:
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketEventCommonItem>(pData, sizeof(hb::net::PacketEventCommonItem));
		if (!pkt) return;
		dwV4 = pkt->v4;
	}
	if ((sV1 == 6) && (sV2 == 0)) {
		m_pEffectManager->AddEffect(EffectType::GOLD_DROP, sX, sY, 0, 0, 0);
	}
	m_pMapData->bSetItem(sX, sY, sV1, static_cast<char>(sV3), dwV4);
	break;

	case CommonType::SetItem:
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketEventCommonItem>(pData, sizeof(hb::net::PacketEventCommonItem));
		if (!pkt) return;
		dwV4 = pkt->v4;
	}
	m_pMapData->bSetItem(sX, sY, sV1, static_cast<char>(sV3), dwV4, false); // v1.4 color
	break;

	case CommonType::Magic:
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketEventCommonMagic>(pData, sizeof(hb::net::PacketEventCommonMagic));
		if (!pkt) return;
		sV4 = pkt->v4;
	}
	m_pEffectManager->AddEffect(static_cast<EffectType>(sV3), sX, sY, sV1, sV2, 0, sV4);
	break;

	case CommonType::ClearGuildName:
		ClearGuildNameList();
		break;
	}
}

void CGame::ClearGuildNameList()
{
	for (int i = 0; i < game_limits::max_guild_names; i++) {
		m_stGuildName[i].dwRefTime = 0;
		m_stGuildName[i].iGuildRank = -1;
	}
}

void CGame::InitGameSettings()
{

	m_pPlayer->m_bForceAttack = false;
	m_pPlayer->m_Controller.SetCommandTime(0);
	m_dwCheckConnectionTime = 0;


	m_Camera.SetShake(0);

	m_pPlayer->m_Controller.SetCommand(Type::Stop);
	m_pPlayer->m_Controller.ResetCommandCount();

	m_bIsGetPointingMode = false;
	m_bWaitForNewClick = false;
	m_dwMagicCastTime = 0;
	m_iPointCommandType = -1; //v2.15 0 -> -1

	for (int r = 0; r < 3; r++) m_eConfigRetry[r] = ConfigRetryLevel::None;
	m_dwConfigRequestTime = 0;
	m_bInitDataReady = false;
	m_bConfigsReady = false;

	m_pPlayer->m_bIsCombatMode = false;

	// Previous cursor status tracking removed
	CursorTarget::ResetSelectionClickTime();

	m_bSkillUsingStatus = false;
	m_bItemUsingStatus = false;
	m_bUsingSlate = false;


	m_iDownSkillIndex = -1;
	m_dialogBoxManager.Info(DialogBoxId::Skill).bFlag = false;

	m_pPlayer->m_bIsConfusion = false;

	m_iIlusionOwnerH = 0;
	m_cIlusionOwnerType = 0;

	m_iDrawFlag = 0;
	m_bIsCrusadeMode = false;
	m_pPlayer->m_iCrusadeDuty = 0;

	m_iNetLagCount = 0;
	m_iLatencyMs = -1;
	m_dwLastNetMsgId = 0;
	m_dwLastNetMsgTime = 0;
	m_dwLastNetMsgSize = 0;
	m_dwLastNetRecvTime = 0;
	m_dwLastNpcEventTime = 0;

	m_dwEnvEffectTime = GameClock::GetTimeMS();

	for (int i = 0; i < game_limits::max_guild_names; i++) {
		m_stGuildName[i].dwRefTime = 0;
		m_stGuildName[i].iGuildRank = -1;
	}
	//Snoopy: 61
	for (int i = 0; i < 61; i++)
		m_dialogBoxManager.SetEnabled(i, false);

	//Snoopy: 58 because 2 last ones alreaddy defined
	for (int i = 0; i < 58; i++)
		m_dialogBoxManager.SetOrderAt(i, 0);
	m_dialogBoxManager.SetOrderAt(60, DialogBoxId::HudPanel);
	m_dialogBoxManager.SetOrderAt(59, DialogBoxId::HudPanel);
	m_dialogBoxManager.SetEnabled(DialogBoxId::HudPanel, true);
	m_dialogBoxManager.SetEnabled(DialogBoxId::HudPanel, true);

	if (m_pEffectManager) m_pEffectManager->ClearAllEffects();

	m_floatingText.ClearAll();

	ChatManager::Get().ClearMessages();

	ChatManager::Get().ClearWhispers();


	m_pPlayer->m_iGuildRank = -1;

	for (int i = 0; i < 100; i++) {
		m_stGuildOpList[i].cOpMode = 0;
	}



	for (int i = 0; i < 41; i++) {
		m_dialogBoxManager.Info(i).bFlag = false;
		m_dialogBoxManager.Info(i).sView = 0;
		m_dialogBoxManager.Info(i).bIsScrollSelected = false;
	}

	for (int i = 0; i < hb::shared::limits::MaxItems; i++)
		if (m_pItemList[i] != 0) {
			m_pItemList[i].reset();
		}

	for (int i = 0; i < game_limits::max_sell_list; i++) {
		m_stSellItemList[i].iIndex = -1;
		m_stSellItemList[i].iAmount = 0;
	}

	for (int i = 0; i < hb::shared::limits::MaxBankItems; i++)
		if (m_pBankList[i] != 0) {
			m_pBankList[i].reset();
		}

	for (int i = 0; i < hb::shared::limits::MaxMagicType; i++)
		m_pPlayer->m_iMagicMastery[i] = 0;

	for (int i = 0; i < hb::shared::limits::MaxSkillType; i++)
		m_pPlayer->m_iSkillMastery[i] = 0;

	for (int i = 0; i < game_limits::max_text_dlg_lines; i++) {
		if (m_pMsgTextList[i] != 0)
			m_pMsgTextList[i].reset();

		if (m_pMsgTextList2[i] != 0)
			m_pMsgTextList2[i].reset();

		if (m_pAgreeMsgTextList[i] != 0)
			m_pAgreeMsgTextList[i].reset();
	}

	for (int i = 0; i < hb::shared::limits::MaxPartyMembers; i++) {
		m_stPartyMember[i].cStatus = 0;
	}

	m_pPlayer->m_iLU_Point = 0;
	m_pPlayer->m_wLU_Str = m_pPlayer->m_wLU_Vit = m_pPlayer->m_wLU_Dex = m_pPlayer->m_wLU_Int = m_pPlayer->m_wLU_Mag = m_pPlayer->m_wLU_Char = 0;
	m_cLogOutCount = -1;
	m_dwLogOutCountTime = 0;
	m_pPlayer->m_iSuperAttackLeft = 0;
	m_pPlayer->m_bSuperAttackMode = false;
	m_iFightzoneNumber = 0;
	m_stQuest.sWho = 0;
	m_stQuest.sQuestType = 0;
	m_stQuest.sContribution = 0;
	m_stQuest.sTargetType = 0;
	m_stQuest.sTargetCount = 0;
	m_stQuest.sCurrentCount = 0;
	m_stQuest.sX = 0;
	m_stQuest.sY = 0;
	m_stQuest.sRange = 0;
	m_stQuest.bIsQuestCompleted = false;
	m_bIsObserverMode = false;
	m_bIsObserverCommanded = false;
	m_pPlayer->m_bIsPoisoned = false;
	m_pPlayer->m_Controller.SetPrevMoveBlocked(false);
	m_pPlayer->m_Controller.SetPrevMove(-1, -1);
	m_pPlayer->m_sDamageMove = 0;
	m_pPlayer->m_sDamageMoveAmount = 0;
	m_bForceDisconn = false;
	m_pPlayer->m_bIsSpecialAbilityEnabled = false;
	m_pPlayer->m_iSpecialAbilityType = 0;
	m_dwSpecialAbilitySettingTime = 0;
	m_pPlayer->m_iSpecialAbilityTimeLeftSec = 0;
	CursorTarget::ClearSelection();
	m_bIsF1HelpWindowEnabled = false;
	for (int i = 0; i < hb::shared::limits::MaxCrusadeStructures; i++)
	{
		m_stCrusadeStructureInfo[i].cType = 0;
		m_stCrusadeStructureInfo[i].cSide = 0;
		m_stCrusadeStructureInfo[i].sX = 0;
		m_stCrusadeStructureInfo[i].sY = 0;
	}
	m_dwCommanderCommandRequestedTime = 0;
	m_iTopMsgLastSec = 0;
	m_dwTopMsgTime = 0;
	m_pPlayer->m_iConstructionPoint = 0;
	m_pPlayer->m_iWarContribution = 0;
	TeleportManager::Get().Reset();
	m_pPlayer->m_iConstructLocX = m_pPlayer->m_iConstructLocY = -1;

	//Snoopy: Apocalypse Gate
	m_iGatePositX = m_iGatePositY = -1;
	m_iHeldenianAresdenLeftTower = -1;
	m_iHeldenianElvineLeftTower = -1;
	m_iHeldenianAresdenFlags = -1;
	m_iHeldenianElvineFlags = -1;
	m_bIsXmas = false;
	m_iTotalPartyMember = 0;
	m_iPartyStatus = 0;
	m_iGizonItemUpgradeLeft = 0;
	m_dialogBoxManager.EnableDialogBox(DialogBoxId::GuideMap, 0, 0, 0);
}

void CGame::CreateNewGuildResponseHandler(char* pData)
{
	const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(
		pData, sizeof(hb::net::PacketHeader));
	if (!header) return;
	switch (header->msg_type) {
	case MsgType::Confirm:
		m_pPlayer->m_iGuildRank = 0;
		m_dialogBoxManager.Info(DialogBoxId::GuildMenu).cMode = 3;
		break;
	case MsgType::Reject:
		m_pPlayer->m_iGuildRank = -1;
		m_dialogBoxManager.Info(DialogBoxId::GuildMenu).cMode = 4;
		break;
	}
}

void CGame::InitPlayerCharacteristics(char* pData)
{
	// Snoopy: Angels
	m_pPlayer->m_iAngelicStr = 0;
	m_pPlayer->m_iAngelicDex = 0;
	m_pPlayer->m_iAngelicInt = 0;
	m_pPlayer->m_iAngelicMag = 0;

	const auto* pkt = hb::net::PacketCast<hb::net::PacketResponsePlayerCharacterContents>(
		pData, sizeof(hb::net::PacketResponsePlayerCharacterContents));
	if (!pkt) return;

	m_pPlayer->m_iHP = pkt->hp;
	m_pPlayer->m_iMP = pkt->mp;
	m_pPlayer->m_iSP = pkt->sp;
	m_pPlayer->m_iAC = pkt->ac;		//? m_iDefenseRatio
	m_pPlayer->m_iTHAC0 = pkt->thac0;    //? m_iHitRatio
	m_pPlayer->m_iLevel = pkt->level;
	m_pPlayer->m_iStr = pkt->str;
	m_pPlayer->m_iInt = pkt->intel;
	m_pPlayer->m_iVit = pkt->vit;
	m_pPlayer->m_iDex = pkt->dex;
	m_pPlayer->m_iMag = pkt->mag;
	m_pPlayer->m_iCharisma = pkt->chr;

	// CLEROTH - LU
	m_pPlayer->m_iLU_Point = pkt->lu_point;

	m_pPlayer->m_iExp = pkt->exp;
	m_pPlayer->m_iEnemyKillCount = pkt->enemy_kills;
	m_pPlayer->m_iPKCount = pkt->pk_count;
	m_pPlayer->m_iRewardGold = pkt->reward_gold;

	m_cLocation.assign(pkt->location, strnlen(pkt->location, sizeof(pkt->location)));
	if (m_cLocation.starts_with("aresden"))
	{
		m_pPlayer->m_bAresden = true;
		m_pPlayer->m_bCitizen = true;
		m_pPlayer->m_bHunter = false;
	}
	else if (m_cLocation.starts_with("arehunter"))
	{
		m_pPlayer->m_bAresden = true;
		m_pPlayer->m_bCitizen = true;
		m_pPlayer->m_bHunter = true;
	}
	else if (m_cLocation.starts_with("elvine"))
	{
		m_pPlayer->m_bAresden = false;
		m_pPlayer->m_bCitizen = true;
		m_pPlayer->m_bHunter = false;
	}
	else if (m_cLocation.starts_with("elvhunter"))
	{
		m_pPlayer->m_bAresden = false;
		m_pPlayer->m_bCitizen = true;
		m_pPlayer->m_bHunter = true;
	}
	else
	{
		m_pPlayer->m_bAresden = true;
		m_pPlayer->m_bCitizen = false;
		m_pPlayer->m_bHunter = true;
	}

	m_pPlayer->m_cGuildName.assign(pkt->guild_name, strnlen(pkt->guild_name, sizeof(pkt->guild_name)));

	if (m_pPlayer->m_cGuildName == "NONE")

	std::replace(m_pPlayer->m_cGuildName.begin(), m_pPlayer->m_cGuildName.end(), '_', ' ');
	m_pPlayer->m_iGuildRank = pkt->guild_rank;
	m_pPlayer->m_iSuperAttackLeft = pkt->super_attack_left;
	m_iFightzoneNumber = pkt->fightzone_number;
	iMaxStats = pkt->max_stats;
	iMaxLevel = pkt->max_level;
	iMaxBankItems = pkt->max_bank_items;
}

void CGame::DisbandGuildResponseHandler(char* pData)
{
	const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(
		pData, sizeof(hb::net::PacketHeader));
	if (!header) return;
	switch (header->msg_type) {
	case MsgType::Confirm:
		m_pPlayer->m_iGuildRank = -1;
		m_dialogBoxManager.Info(DialogBoxId::GuildMenu).cMode = 7;
		break;
	case MsgType::Reject:
		m_dialogBoxManager.Info(DialogBoxId::GuildMenu).cMode = 8;
		break;
	}
}

void CGame::_PutGuildOperationList(char* pName, char cOpMode)
{
	int i;
	for (i = 0; i < 100; i++)
		if (m_stGuildOpList[i].cOpMode == 0)
		{
			m_stGuildOpList[i].cOpMode = cOpMode;
			m_stGuildOpList[i].cName.assign(pName, strnlen(pName, 20));
			return;
		}
}

void CGame::_ShiftGuildOperationList()
{
	int i;
	m_stGuildOpList[0].cOpMode = 0;

	for (i = 1; i < 100; i++)
		if ((m_stGuildOpList[i - 1].cOpMode == 0) && (m_stGuildOpList[i].cOpMode != 0)) {
			m_stGuildOpList[i - 1].cOpMode = m_stGuildOpList[i].cOpMode;
			m_stGuildOpList[i - 1].cName = m_stGuildOpList[i].cName;

			m_stGuildOpList[i].cOpMode = 0;
		}
}

void CGame::EnableDialogBox(int iBoxID, int cType, int sV1, int sV2, char* pString)
{
	m_dialogBoxManager.EnableDialogBox(iBoxID, cType, sV1, sV2, pString);
}

void CGame::DisableDialogBox(int iBoxID)
{
	m_dialogBoxManager.DisableDialogBox(iBoxID);
}

void CGame::AddEventList(const char* pTxt, char cColor, bool bDupAllow)
{
	EventListManager::Get().AddEvent(pTxt, cColor, bDupAllow);
}

bool CGame::bCheckImportantFile()
{
	// Use the sprite factory's configured path
	std::string spritePath = hb::shared::sprite::Sprites::GetSpritePath() + "/TREES1.PAK";
	FILE* f = fopen(spritePath.c_str(), "rb");
	if (!f) return false;
	fclose(f);
	return true;
}

void CGame::RequestFullObjectData(uint16_t wObjectID)
{
	int     iRet;
	hb::net::PacketHeader header{};
	header.msg_id = MsgId::RequestFullObjectData;
	header.msg_type = wObjectID;

	iRet = m_pGSock->iSendMsg(reinterpret_cast<char*>(&header), sizeof(header));

	switch (iRet) {
	case sock::Event::SocketClosed:
	case sock::Event::SocketError:
	case sock::Event::QueueFull:
		ChangeGameMode(GameMode::ConnectionLost);
		m_pGSock.reset();
		break;

	case sock::Event::CriticalError:
		m_pGSock.reset();
		hb::shared::render::Window::Close();
		break;
	}
}


void CGame::_ReadMapData(short pivot_x, short pivot_y, const char* packet_data)
{
	char header_byte = 0, direction = 0, item_color = 0;
	std::string name;
	short total_entries = 0, map_x = 0, map_y = 0, owner_type = 0, dynamic_type = 0;
	short npc_config_id = -1;
	hb::shared::entity::PlayerStatus status;
	hb::shared::entity::PlayerAppearance appearance;
	uint16_t object_id = 0;
	uint16_t dynamic_object_id = 0;
	short item_id = 0;
	uint32_t item_attr = 0;

	const char* cursor = packet_data;
	m_sVDL_X = pivot_x; // Valid Data Loc-X
	m_sVDL_Y = pivot_y;

	const auto* mapHeader = hb::net::PacketCast<hb::net::PacketMapDataHeader>(cursor, sizeof(hb::net::PacketMapDataHeader));
	if (!mapHeader) return;
	total_entries = mapHeader->total;
	cursor += sizeof(hb::net::PacketMapDataHeader);
	for (int i = 1; i <= total_entries; i++)
	{
		const auto* entry = hb::net::PacketCast<hb::net::PacketMapDataEntryHeader>(cursor, sizeof(hb::net::PacketMapDataEntryHeader));
		if (!entry) return;
		map_x = entry->x;
		map_y = entry->y;
		header_byte = entry->flags;
		cursor += sizeof(hb::net::PacketMapDataEntryHeader);
		if (header_byte & 0x01) // object ID
		{
			const auto* objBase = hb::net::PacketCast<hb::net::PacketMapDataObjectBase>(cursor, sizeof(hb::net::PacketMapDataObjectBase));
			if (!objBase) return;
			if (hb::shared::object_id::IsPlayerID(objBase->object_id))
			{
				const auto* obj = hb::net::PacketCast<hb::net::PacketMapDataObjectPlayer>(cursor, sizeof(hb::net::PacketMapDataObjectPlayer));
				if (!obj) return;
				object_id = obj->base.object_id;
				owner_type = obj->type;
				direction = static_cast<char>(obj->dir);
				appearance = obj->appearance;
				status = obj->status;
				name.clear();
				name.assign(obj->name, strnlen(obj->name, sizeof(obj->name)));
				cursor += sizeof(hb::net::PacketMapDataObjectPlayer);
			}
			else // NPC
			{
				const auto* obj = hb::net::PacketCast<hb::net::PacketMapDataObjectNpc>(cursor, sizeof(hb::net::PacketMapDataObjectNpc));
				if (!obj) return;
				object_id = obj->base.object_id;
				npc_config_id = obj->config_id;
				owner_type = ResolveNpcType(npc_config_id);
				direction = static_cast<char>(obj->dir);
				appearance.SetFromNpcAppearance(obj->appearance);
				status.SetFromEntityStatus(obj->status);
				name.clear();
				name.assign(obj->name, strnlen(obj->name, sizeof(obj->name)));
				cursor += sizeof(hb::net::PacketMapDataObjectNpc);
			}
			{ m_pMapData->bSetOwner(object_id, pivot_x + map_x, pivot_y + map_y, owner_type, direction, appearance, status, name, Type::Stop, 0, 0, 0, 0, 0, npc_config_id); }
		}
		if (header_byte & 0x02) // object ID
		{
			const auto* objBase = hb::net::PacketCast<hb::net::PacketMapDataObjectBase>(cursor, sizeof(hb::net::PacketMapDataObjectBase));
			if (!objBase) return;
			if (hb::shared::object_id::IsPlayerID(objBase->object_id))
			{
				const auto* obj = hb::net::PacketCast<hb::net::PacketMapDataObjectPlayer>(cursor, sizeof(hb::net::PacketMapDataObjectPlayer));
				if (!obj) return;
				object_id = obj->base.object_id;
				owner_type = obj->type;
				direction = static_cast<char>(obj->dir);
				appearance = obj->appearance;
				status = obj->status;
				name.clear();
				name.assign(obj->name, strnlen(obj->name, sizeof(obj->name)));
				cursor += sizeof(hb::net::PacketMapDataObjectPlayer);
			}
			else // NPC
			{
				const auto* obj = hb::net::PacketCast<hb::net::PacketMapDataObjectNpc>(cursor, sizeof(hb::net::PacketMapDataObjectNpc));
				if (!obj) return;
				object_id = obj->base.object_id;
				npc_config_id = obj->config_id;
				owner_type = ResolveNpcType(npc_config_id);
				direction = static_cast<char>(obj->dir);
				appearance.SetFromNpcAppearance(obj->appearance);
				status.SetFromEntityStatus(obj->status);
				name.clear();
				name.assign(obj->name, strnlen(obj->name, sizeof(obj->name)));
				cursor += sizeof(hb::net::PacketMapDataObjectNpc);
			}
			{ m_pMapData->bSetDeadOwner(object_id, pivot_x + map_x, pivot_y + map_y, owner_type, direction, appearance, status, name, npc_config_id); }
		}
		if (header_byte & 0x04)
		{
			const auto* item = hb::net::PacketCast<hb::net::PacketMapDataItem>(cursor, sizeof(hb::net::PacketMapDataItem));
			if (!item) return;
			item_id = item->item_id;
			item_color = static_cast<char>(item->color);
			item_attr = item->attribute;
			cursor += sizeof(hb::net::PacketMapDataItem);
			m_pMapData->bSetItem(pivot_x + map_x, pivot_y + map_y, item_id, item_color, item_attr, false);
		}
		if (header_byte & 0x08) // Dynamic object
		{
			const auto* dyn = hb::net::PacketCast<hb::net::PacketMapDataDynamicObject>(cursor, sizeof(hb::net::PacketMapDataDynamicObject));
			if (!dyn) return;
			dynamic_object_id = dyn->object_id;
			dynamic_type = dyn->type;
			cursor += sizeof(hb::net::PacketMapDataDynamicObject);
			m_pMapData->bSetDynamicObject(pivot_x + map_x, pivot_y + map_y, dynamic_object_id, dynamic_type, false);
		}
	}
}

void CGame::LogEventHandler(char* pData)
{
	WORD wEventType, wObjectID;
	short sX, sY, sType;
	short npcConfigId = -1;
	hb::shared::entity::PlayerStatus iStatus;
	char cDir;
	std::string cName;
	hb::shared::entity::PlayerAppearance playerAppearance;

	const auto* base = hb::net::PacketCast<hb::net::PacketEventLogBase>(pData, sizeof(hb::net::PacketEventLogBase));
	if (!base) return;
	wEventType = base->header.msg_type;
	wObjectID = base->object_id;
	sX = base->x;
	sY = base->y;
	cName.clear();
	if (hb::shared::object_id::IsPlayerID(wObjectID))
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketEventLogPlayer>(pData, sizeof(hb::net::PacketEventLogPlayer));
		if (!pkt) return;
		sType = pkt->type;
		cDir = static_cast<char>(pkt->dir);
		cName.assign(pkt->name, strnlen(pkt->name, sizeof(pkt->name)));
		playerAppearance = pkt->appearance;
		iStatus = pkt->status;
	}
	else 	// NPC
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketEventLogNpc>(pData, sizeof(hb::net::PacketEventLogNpc));
		if (!pkt) return;
		npcConfigId = pkt->config_id;
		sType = ResolveNpcType(npcConfigId);
		cDir = static_cast<char>(pkt->dir);
		cName.assign(pkt->name, strnlen(pkt->name, sizeof(pkt->name)));
		playerAppearance.SetFromNpcAppearance(pkt->appearance);
		iStatus.SetFromEntityStatus(pkt->status);
	}

	switch (wEventType) {
	case MsgType::Confirm:
		{ m_pMapData->bSetOwner(wObjectID, sX, sY, sType, cDir, playerAppearance, iStatus, cName, Type::Stop, 0, 0, 0, 0, 0, npcConfigId); }
		switch (sType) {
		case hb::shared::owner::LightWarBeetle: // LWB
		case hb::shared::owner::GodsHandKnight: // GHK
		case hb::shared::owner::GodsHandKnightCK: // GHKABS
		case hb::shared::owner::TempleKnight: // TK
		case hb::shared::owner::BattleGolem: // BG
			m_pEffectManager->AddEffect(EffectType::WHITE_HALO, (sX) * 32, (sY) * 32, 0, 0, 0);
			break;
		}
		break;

	case MsgType::Reject:
		{ m_pMapData->bSetOwner(wObjectID, -1, -1, sType, cDir, playerAppearance, iStatus, cName, Type::Stop, 0, 0, 0, 0, 0, npcConfigId); }
		break;
	}

	m_floatingText.RemoveByObjectID(wObjectID);
}

// MODERNIZED: No longer a window message handler - polls socket directly
// MODERNIZED: v4 Networking Architecture (Drain -> Queue -> Process) for Login Socket
void CGame::OnLogSocketEvent()
{
    if (m_pLSock == 0) return;

    // 1. Check for socket state changes (Connect, Close, Error)
    int iRet = m_pLSock->Poll();

    switch (iRet) {
    case sock::Event::SocketClosed:
        ChangeGameMode(GameMode::ConnectionLost);
        m_pLSock.reset();
        m_pLSock.reset();
        return;
    case sock::Event::SocketError:
        printf("[ERROR] Login socket error\n");
        ChangeGameMode(GameMode::ConnectionLost);
        m_pLSock.reset();
        m_pLSock.reset();
        return;

    case sock::Event::ConnectionEstablish:
        ConnectionEstablishHandler(static_cast<int>(ServerType::Log));
        break;
    }

    // 2. Drain all available data from TCP buffer to the Queue
    // Only drain if socket is connected (m_bIsAvailable is set on FD_CONNECT)
    if (!m_pLSock->m_bIsAvailable) {
        return; // Still connecting, don't try to read yet
    }

    // If Poll() completed a packet, queue it before DrainToQueue() overwrites the buffer
    if (iRet == sock::Event::ReadComplete) {
        size_t dwSize = 0;
        char* pData = m_pLSock->pGetRcvDataPointer(&dwSize);
        if (pData != nullptr && dwSize > 0) {
            m_pLSock->QueueCompletedPacket(pData, dwSize);
        }
    }

    int iDrained = m_pLSock->DrainToQueue();

    if (iDrained < 0) {
        printf("[ERROR] Login socket DrainToQueue failed: %d\n", iDrained);
        ChangeGameMode(GameMode::ConnectionLost);
        m_pLSock.reset();
        m_pLSock.reset();
        return;
    }

    // 3. Process the queue with a Time Budget
    constexpr int MAX_PACKETS_PER_FRAME = 120; // Safety limit
    constexpr uint32_t MAX_TIME_MS = 3;        // 3ms budget for network processing

    uint32_t dwStartTime = GameClock::GetTimeMS();
    int iProcessed = 0;

    hb::shared::net::NetworkPacket packet;
    while (iProcessed < MAX_PACKETS_PER_FRAME) {

        // Check budget
        if (GameClock::GetTimeMS() - dwStartTime > MAX_TIME_MS) {
            break;
        }

        // Peek next packet
        if (!m_pLSock->PeekPacket(packet)) {
            break; // Queue empty
        }

        // Update timestamps
        m_dwLastNetRecvTime = GameClock::GetTimeMS();

        // Process (using the pointer directly from the packet vector)
        if (!packet.empty()) {
             LogRecvMsgHandler(const_cast<char*>(packet.ptr()));
        }

        // CRITICAL FIX: The handler might have closed/deleted the socket!
        if (m_pLSock == nullptr) return;

        // Pop logic (remove from queue)
        m_pLSock->PopPacket();
        iProcessed++;
    }
}

void CGame::LogResponseHandler(char* packet_data)
{
	WORD response = 0;
	char char_name[12]{};

	const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(packet_data, sizeof(hb::net::PacketHeader));
	if (!header) {
		printf("[ERROR] LogResponseHandler - invalid packet header\n");
		return;
	}
	response = header->msg_type;

	// Route to the active screen first — if it handles the response, we're done.
	IGameScreen* pScreen = GameModeManager::GetActiveScreen();
	if (pScreen && pScreen->on_net_response(response, packet_data)) {
		printf("[LogResponseHandler] Response 0x%04X handled by screen on_net_response\n", response);
		return;
	}
	printf("[LogResponseHandler] Response 0x%04X using legacy handler\n", response);

	switch (response) {
	case LogResMsg::CharacterDeleted:
	{
		const auto* list = hb::net::PacketCast<hb::net::PacketLogCharacterListHeader>(
			packet_data, sizeof(hb::net::PacketLogCharacterListHeader));
		if (!list) return;
		m_iTotalChar = list->total_chars;
		for (int i = 0; i < 4; i++)
			if (m_pCharList[i] != 0)
			{
				m_pCharList[i].reset();
			}

		const auto* entries = reinterpret_cast<const hb::net::PacketLogCharacterEntry*>(
			packet_data + sizeof(hb::net::PacketLogCharacterListHeader));
		for (int i = 0; i < m_iTotalChar; i++) {
			const auto& entry = entries[i];
			m_pCharList[i] = std::make_unique<CCharInfo>();
			m_pCharList[i]->m_cName.assign(entry.name, strnlen(entry.name, sizeof(entry.name)));
			m_pCharList[i]->m_appearance = entry.appearance;
			m_pCharList[i]->m_sSex = entry.sex;
			m_pCharList[i]->m_sSkinCol = entry.skin;
			m_pCharList[i]->m_sLevel = entry.level;
			m_pCharList[i]->m_iExp = entry.exp;
	
			m_pCharList[i]->m_cMapName.assign(entry.map_name, strnlen(entry.map_name, sizeof(entry.map_name)));
		}
		std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "3A");
		ChangeGameMode(GameMode::LogResMsg);
	}
	break;

	case LogResMsg::Confirm:
	{
		const auto* list = hb::net::PacketCast<hb::net::PacketLogCharacterListHeader>(
			packet_data, sizeof(hb::net::PacketLogCharacterListHeader));
		if (!list) {
			printf("[ERROR] LogResponseHandler - CONFIRM packet too small\n");
			return;
		}
		m_iAccntYear = 0;
		m_iAccntMonth = 0;
		m_iAccntDay = 0;
		m_iIpYear = 0;
		m_iIpMonth = 0;
		m_iIpDay = 0;
		m_iTotalChar = list->total_chars;
		for (int i = 0; i < 4; i++)
			if (m_pCharList[i] != 0)
			{
				m_pCharList[i].reset();
			}

		const auto* entries = reinterpret_cast<const hb::net::PacketLogCharacterEntry*>(
			packet_data + sizeof(hb::net::PacketLogCharacterListHeader));
		for (int i = 0; i < m_iTotalChar; i++)
		{
			const auto& entry = entries[i];
			m_pCharList[i] = std::make_unique<CCharInfo>();
			m_pCharList[i]->m_cName.assign(entry.name, strnlen(entry.name, sizeof(entry.name)));
			m_pCharList[i]->m_appearance = entry.appearance;
			m_pCharList[i]->m_sSex = entry.sex;
			m_pCharList[i]->m_sSkinCol = entry.skin;
			m_pCharList[i]->m_sLevel = entry.level;
			m_pCharList[i]->m_iExp = entry.exp;
	
			m_pCharList[i]->m_cMapName.assign(entry.map_name, strnlen(entry.map_name, sizeof(entry.map_name)));
		}
		ChangeGameMode(GameMode::SelectCharacter);
	}
	break;

	case LogResMsg::Reject:
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketLogResponseReject>(packet_data, sizeof(hb::net::PacketLogResponseReject));
		if (!pkt) return;
		m_iBlockYear = pkt->block_year;
		m_iBlockMonth = pkt->block_month;
		m_iBlockDay = pkt->block_day;

		std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "7H");
		ChangeGameMode(GameMode::LogResMsg);
	}
	break;

	case LogResMsg::NotEnoughPoint:
		std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "7I");
		ChangeGameMode(GameMode::LogResMsg);
		break;

	case LogResMsg::AccountLocked:
		std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "7K");
		ChangeGameMode(GameMode::LogResMsg);
		break;

	case LogResMsg::ServiceNotAvailable:
		std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "7L");
		ChangeGameMode(GameMode::LogResMsg);
		break;

	case LogResMsg::PasswordChangeSuccess:
		std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "6B");
		ChangeGameMode(GameMode::LogResMsg);
		break;

	case LogResMsg::PasswordChangeFail:
		std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "6C");
		ChangeGameMode(GameMode::LogResMsg);
		break;

	case LogResMsg::PasswordMismatch:
		std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "11");
		ChangeGameMode(GameMode::LogResMsg);
		break;

	case LogResMsg::NotExistingAccount:
		std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "12");
		ChangeGameMode(GameMode::LogResMsg);
		break;

	case LogResMsg::NewAccountCreated:
		std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "54");
		ChangeGameMode(GameMode::LogResMsg);
		break;

	case LogResMsg::NewAccountFailed:
		std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "05");
		ChangeGameMode(GameMode::LogResMsg);
		break;

	case LogResMsg::AlreadyExistingAccount:
		std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "06");
		ChangeGameMode(GameMode::LogResMsg);
		break;

	case LogResMsg::NotExistingCharacter:
		std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "Not existing character!");
		ChangeGameMode(GameMode::Msg);
		break;

	case LogResMsg::NewCharacterCreated:
	{
		const auto* list = hb::net::PacketCast<hb::net::PacketLogNewCharacterCreatedHeader>(
			packet_data, sizeof(hb::net::PacketLogNewCharacterCreatedHeader));
		if (!list) return;
		memcpy(char_name, list->character_name, sizeof(list->character_name));

		m_iTotalChar = list->total_chars;
		for (int i = 0; i < 4; i++)
			if (m_pCharList[i] != 0) m_pCharList[i].reset();

		const auto* entries = reinterpret_cast<const hb::net::PacketLogCharacterEntry*>(
			packet_data + sizeof(hb::net::PacketLogNewCharacterCreatedHeader));
		for (int i = 0; i < m_iTotalChar; i++) {
			const auto& entry = entries[i];
			m_pCharList[i] = std::make_unique<CCharInfo>();
			m_pCharList[i]->m_cName.assign(entry.name, strnlen(entry.name, sizeof(entry.name)));
			m_pCharList[i]->m_appearance = entry.appearance;
			m_pCharList[i]->m_sSex = entry.sex;
			m_pCharList[i]->m_sSkinCol = entry.skin;
			m_pCharList[i]->m_sLevel = entry.level;
			m_pCharList[i]->m_iExp = entry.exp;
	
			m_pCharList[i]->m_cMapName.assign(entry.map_name, strnlen(entry.map_name, sizeof(entry.map_name)));
		}
		std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "47");
		ChangeGameMode(GameMode::LogResMsg);
	}
	break;

	case LogResMsg::NewCharacterFailed:
		std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "28");
		ChangeGameMode(GameMode::LogResMsg);
		break;

	case LogResMsg::AlreadyExistingCharacter:
		std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "29");
		ChangeGameMode(GameMode::LogResMsg);
		break;

	case EnterGameRes::Playing:
		ChangeGameMode(GameMode::QueryForceLogin);
		break;

	case EnterGameRes::Confirm:
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketLogEnterGameConfirm>(
			packet_data, sizeof(hb::net::PacketLogEnterGameConfirm));
		if (!pkt) return;
		int iGameServerPort = pkt->game_server_port;
		char cGameServerAddr[16]{};
		memcpy(cGameServerAddr, pkt->game_server_addr, sizeof(pkt->game_server_addr));
		m_cGameServerName.assign(pkt->game_server_name, strnlen(pkt->game_server_name, sizeof(pkt->game_server_name)));
		(void)iGameServerPort;

		m_pGSock = std::make_unique<hb::shared::net::ASIOSocket>(m_pIOPool->GetContext(), game_limits::socket_block_limit);
		m_pGSock->bConnect(m_cLogServerAddr.c_str(), m_iGameServerPort);
		m_pGSock->bInitBufferSize(hb::shared::limits::MsgBufferSize);
	}
	break;

	case EnterGameRes::Reject:
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketLogResponseCode>(packet_data, sizeof(hb::net::PacketLogResponseCode));
		if (!pkt) return;
		std::memset(m_cMsg, 0, sizeof(m_cMsg));
		switch (pkt->code) {
		case 1:	std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "3E"); break;
		case 2:	std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "3F"); break;
		case 3:	std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "33"); break;
		case 4: std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "3D"); break;
		case 5: std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "3G"); break;
		case 6: std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "3Z"); break;
		case 7: std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "3J"); break;
		}
		ChangeGameMode(GameMode::LogResMsg);
	}
	break;

	case EnterGameRes::ForceDisconn:
		std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "3X");
		ChangeGameMode(GameMode::LogResMsg);
		break;

	case LogResMsg::NotExistingWorldServer:
		std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "1Y");
		ChangeGameMode(GameMode::LogResMsg);
		break;

	case LogResMsg::InputKeyCode:
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketLogResponseCode>(packet_data, sizeof(hb::net::PacketLogResponseCode));
		if (!pkt) return;
		std::memset(m_cMsg, 0, sizeof(m_cMsg));
		switch (pkt->code) {
		case 1:	std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "8U"); break; //MainMenu, Keycode registration success
		case 2:	std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "82"); break; //MainMenu, Not existing Account
		case 3:	std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "81"); break; //MainMenu, Password wrong
		case 4: std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "8V"); break; //MainMenu, Invalid Keycode
		case 5: std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "8W"); break; //MainMenu, Already Used Keycode
		}
		ChangeGameMode(GameMode::LogResMsg);
	}
	break;

	case LogResMsg::ForceChangePassword:
		std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "6M");
		ChangeGameMode(GameMode::LogResMsg);
		break;

	case LogResMsg::InvalidKoreanSsn:
		std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "1a");
		ChangeGameMode(GameMode::LogResMsg);
		break;

	case LogResMsg::LessThenFifteen:
		std::snprintf(m_cMsg, sizeof(m_cMsg), "%s", "1b");
		ChangeGameMode(GameMode::LogResMsg);
		break;
	}
	m_pLSock.reset();
	m_pLSock.reset();
}

void CGame::LogRecvMsgHandler(char* pData)
{
	LogResponseHandler(pData);
}

void CGame::ChangeGameMode(GameMode mode)
{
	// Determine if this mode change should be instant (no fade-out)
	// Instant transitions are used for:
	// - Error states that need immediate feedback
	// - Transitions FROM loading/waiting states that don't need fade-outs
	bool instant = false;

	// Check if TARGET mode should be instant (error states)
	switch (mode)
	{
	case GameMode::ConnectionLost:  // Error - show immediately
	case GameMode::VersionNotMatch: // Error - show immediately
	case GameMode::Msg:             // Error/info - show immediately
	case GameMode::Quit:            // Already fading out visually
		instant = true;
		break;
	default:
		break;
	}

	// Check if CURRENT mode shouldn't have fade-out (loading/waiting states)
	// Use manager's current mode as source of truth
	if (!instant)
	{
		switch (GameModeManager::GetMode())
		{
		case GameMode::Loading:         // Loading screen - no fade needed
		case GameMode::Connecting:      // Waiting screen - no fade needed
		case GameMode::WaitingInitData: // Waiting screen - no fade needed
		case GameMode::WaitingResponse: // Waiting screen - no fade needed
			instant = true;
			break;
		default:
			break;
		}
	}

	// Route to new Screen system or legacy system
	switch (mode) {
	// Full screens
	case GameMode::MainMenu:
		GameModeManager::set_screen<Screen_MainMenu>();
		break;
	case GameMode::Login:
		GameModeManager::set_screen<Screen_Login>();
		break;
	case GameMode::SelectCharacter:
		GameModeManager::set_screen<Screen_SelectCharacter>();
		break;
	case GameMode::CreateNewCharacter:
		GameModeManager::set_screen<Screen_CreateNewCharacter>();
		break;
	case GameMode::CreateNewAccount:
		GameModeManager::set_screen<Screen_CreateAccount>();
		break;
	case GameMode::Quit:
		GameModeManager::set_screen<Screen_Quit>();
		break;
	case GameMode::Loading:
		GameModeManager::set_screen<Screen_Loading>();
		break;

	// Overlays - displayed on top of current base screen
	case GameMode::Connecting:
		GameModeManager::set_overlay<Overlay_Connecting>();
		break;
	case GameMode::WaitingResponse:
		GameModeManager::set_overlay<Overlay_WaitingResponse>();
		break;
	case GameMode::QueryForceLogin:
		GameModeManager::set_overlay<Overlay_QueryForceLogin>();
		break;
	case GameMode::QueryDeleteCharacter:
		GameModeManager::set_overlay<Overlay_QueryDeleteCharacter>();
		break;
	case GameMode::LogResMsg:
		GameModeManager::set_overlay<Overlay_LogResMsg>();
		break;
	case GameMode::ChangePassword:
		GameModeManager::set_overlay<Overlay_ChangePassword>();
		break;
	case GameMode::VersionNotMatch:
		GameModeManager::set_overlay<Overlay_VersionNotMatch>();
		break;
	case GameMode::ConnectionLost:
		GameModeManager::set_overlay<Overlay_ConnectionLost>();
		break;
	case GameMode::Msg:
		GameModeManager::set_overlay<Overlay_Msg>();
		break;
	case GameMode::WaitingInitData:
		GameModeManager::set_overlay<Overlay_WaitInitData>();
		break;

	case GameMode::MainGame:
		GameModeManager::set_screen<Screen_OnGame>();
		break;

	case GameMode::Null:
		// Null mode signals application exit - just set mode, no screen needed
		GameModeManager::SetCurrentMode(GameMode::Null);
		break;

	default:
		// Unhandled modes - log warning (Introduction, Agreement, InputKeyCode are unused)
		break;
	}
}

void CGame::ReleaseUnusedSprites()
{
	for (auto& [idx, spr] : m_pSprite)
	{
		if (spr->IsLoaded() && !spr->IsInUse())
		{
			if (GameClock::GetTimeMS() - spr->GetLastAccessTime() > 60000)
				spr->Unload();
		}
	}
	for(auto& [idx, spr] : m_pTileSpr)
	{
		if (spr->IsLoaded() && !spr->IsInUse())
		{
			if (GameClock::GetTimeMS() - spr->GetLastAccessTime() > 60000)
				spr->Unload();
		}
	}
	for (auto& [idx, spr] : m_pEffectSpr)
	{
		if (spr->IsLoaded() && !spr->IsInUse())
		{
			if (GameClock::GetTimeMS() - spr->GetLastAccessTime() > 60000)
				spr->Unload();
		}
	}

	// Stale sound buffer release is now handled by AudioManager::Update()
	AudioManager::Get().Update();
}

void CGame::ChatMsgHandler(char* packet_data)
{
	int object_id = 0, location = 0;
	short map_x = 0, map_y = 0;
	std::string text2;

	char msg_type = 0, temp[100]{}, message[100]{}, text1[100]{};
	std::string name;
	uint32_t current_time = m_dwCurTime;
	bool is_done = false;

	std::string head_msg;

	std::memset(text1, 0, sizeof(text1));
	std::memset(message, 0, sizeof(message));

	const auto* pkt = hb::net::PacketCast<hb::net::PacketCommandChatMsgHeader>(
		packet_data, sizeof(hb::net::PacketCommandChatMsgHeader));
	if (!pkt) return;
	object_id = static_cast<int>(pkt->header.msg_type);
	map_x = pkt->x;
	map_y = pkt->y;
	name.clear();
	name.assign(pkt->name, strnlen(pkt->name, sizeof(pkt->name)));
	msg_type = static_cast<char>(pkt->chat_type);

	if (bCheckExID(name.c_str()) == true) return;

	std::snprintf(temp, sizeof(temp), "%s", packet_data + sizeof(hb::net::PacketCommandChatMsgHeader));

	if ((msg_type == 0) || (msg_type == 2) || (msg_type == 3))
	{
	}
	if (!ChatManager::Get().IsWhisperEnabled())
	{
		if (msg_type == 20) return;
	}
	if (!ChatManager::Get().IsShoutEnabled())
	{
		if (msg_type == 2 || msg_type == 3) return;
	}

	std::snprintf(message, sizeof(message), "%s: %s", name.c_str(), temp);
	m_Renderer->BeginTextBatch();
	is_done = false;
	short check_byte = 0;
	while (is_done == false)
	{
		int msgLen = static_cast<int>(strlen(message));
		location = m_Renderer->GetTextLength(message, 305);

		// GetTextLength returns how many chars fit; if all fit, no wrapping needed
		if (location >= msgLen)
		{
			ChatManager::Get().AddMessage(message, msg_type);
			is_done = true;
		}
		else if (location > 0)
		{
			// Count double-byte characters for proper splitting
			for (int i = 0; i < location; i++) if (message[i] < 0) check_byte++;

			if ((check_byte % 2) == 0)
			{
				std::memset(temp, 0, sizeof(temp));
				memcpy(temp, message, location);
				ChatManager::Get().AddMessage(temp, msg_type);
				std::snprintf(temp, sizeof(temp), "%s", message + location);
				std::snprintf(message, sizeof(message), " %s", temp);
			}
			else
			{
				std::memset(temp, 0, sizeof(temp));
				memcpy(temp, message, location + 1);
				ChatManager::Get().AddMessage(temp, msg_type);
				std::snprintf(temp, sizeof(temp), "%s", message + location + 1);
				std::snprintf(message, sizeof(message), " %s", temp);
			}
		}
		else
		{
			// Edge case: even a single char doesn't fit, add anyway to avoid infinite loop
			ChatManager::Get().AddMessage(message, msg_type);
			is_done = true;
		}
	}

	m_Renderer->EndTextBatch();

	m_floatingText.RemoveByObjectID(object_id);

	const char* cp = packet_data + sizeof(hb::net::PacketCommandChatMsgHeader);
	int iChatSlot = m_floatingText.AddChatText(cp, current_time, object_id, m_pMapData.get(), map_x, map_y);
	if (iChatSlot != 0 || msg_type == 20) {
			if ((msg_type != 0) && (m_dialogBoxManager.IsEnabled(DialogBoxId::ChatHistory) != true)) {
				head_msg = std::format("{}:{}", name, cp);
				if (msg_type == 10) {
					EventListManager::Get().AddEventTop(head_msg.c_str(), msg_type);
				} else {
					AddEventList(head_msg.c_str(), msg_type);
				}
			}
			return;
		}
}

void CGame::DrawBackground(short sDivX, short sModX, short sDivY, short sModY)
{
	if (sDivX < 0 || sDivY < 0) return;

	// Tile-based loop constants
	constexpr int TILE_SIZE = 32;
	const int visibleTilesX = (LOGICAL_WIDTH() / TILE_SIZE) + 2;   // +2 for partial tiles on edges
	const int visibleTilesY = (LOGICAL_HEIGHT() / TILE_SIZE) + 2;

	// Map pivot for tile array access
	const short pivotX = m_pMapData->m_sPivotX;
	const short pivotY = m_pMapData->m_sPivotY;

	// Draw tiles directly to backbuffer (no caching)
	for (int tileY = 0; tileY < visibleTilesY; tileY++)
	{
		int indexY = sDivY + pivotY + tileY;
		int iy = tileY * TILE_SIZE - sModY;

		for (int tileX = 0; tileX < visibleTilesX; tileX++)
		{
			int indexX = sDivX + pivotX + tileX;
			int ix = tileX * TILE_SIZE - sModX;

			// Bounds check for tile array (752x752)
			if (indexX >= 0 && indexX < 752 && indexY >= 0 && indexY < 752)
			{
				short sSpr = m_pMapData->m_tile[indexX][indexY].m_sTileSprite;
				short sSprFrame = m_pMapData->m_tile[indexX][indexY].m_sTileSpriteFrame;
				m_pTileSpr[sSpr]->Draw(ix - 16, iy - 16, sSprFrame, hb::shared::sprite::DrawParams::NoColorKey());
			}
		}
	}

	if (m_bIsCrusadeMode)
	{
		if (m_pPlayer->m_iConstructLocX != -1) DrawNewDialogBox(InterfaceNdCrusade, m_pPlayer->m_iConstructLocX * 32 - m_Camera.GetX(), m_pPlayer->m_iConstructLocY * 32 - m_Camera.GetY(), 41);
		if (TeleportManager::Get().GetLocX() != -1) DrawNewDialogBox(InterfaceNdCrusade, TeleportManager::Get().GetLocX() * 32 - m_Camera.GetX(), TeleportManager::Get().GetLocY() * 32 - m_Camera.GetY(), 42);
	}
}

void CGame::InitItemList(char* packet_data)
{
	int angel_value = 0;
	uint16_t total_items = 0;

	for (int i = 0; i < hb::shared::limits::MaxItems; i++)
		m_cItemOrder[i] = -1;

	for (int i = 0; i < DEF_MAXITEMEQUIPPOS; i++)
		m_sItemEquipmentStatus[i] = -1;

	for (int i = 0; i < hb::shared::limits::MaxItems; i++)
		m_bIsItemDisabled[i] = false;

	const auto* header = hb::net::PacketCast<hb::net::PacketResponseItemListHeader>(
		packet_data, sizeof(hb::net::PacketResponseItemListHeader));
	if (!header) return;
	total_items = header->item_count;

	for (int i = 0; i < hb::shared::limits::MaxItems; i++)
		if (m_pItemList[i] != 0)
		{
			m_pItemList[i].reset();
		}

	for (int i = 0; i < hb::shared::limits::MaxBankItems; i++)
		if (m_pBankList[i] != 0)
		{
			m_pBankList[i].reset();
		}

	const auto* itemEntries = reinterpret_cast<const hb::net::PacketResponseItemListEntry*>(header + 1);
	for (int i = 0; i < total_items; i++)
	{
		const auto& entry = itemEntries[i];
		m_pItemList[i] = std::make_unique<CItem>();
		m_pItemList[i]->m_sIDnum = entry.item_id;
		m_pItemList[i]->m_dwCount = entry.count;
		m_pItemList[i]->m_sX = 40;
		m_pItemList[i]->m_sY = 30;
		if (entry.is_equipped == 0) m_bIsItemEquipped[i] = false;
		else m_bIsItemEquipped[i] = true;
		CItem* pCfg = GetItemConfig(entry.item_id);
		if (m_bIsItemEquipped[i] == true && pCfg)
		{
			m_sItemEquipmentStatus[pCfg->m_cEquipPos] = i;
		}
		m_pItemList[i]->m_wCurLifeSpan = entry.cur_lifespan;
		m_pItemList[i]->m_cItemColor = entry.item_color;
		m_pItemList[i]->m_sItemSpecEffectValue2 = static_cast<short>(entry.spec_value2); // v1.41
		m_pItemList[i]->m_dwAttribute = entry.attribute;
		m_cItemOrder[i] = i;
		// Snoopy: Add Angelic Stats
		if (pCfg && (pCfg->GetItemType() == ItemType::Equip)
			&& (m_bIsItemEquipped[i] == true)
			&& (pCfg->m_cEquipPos >= 11))
		{
			angel_value = (m_pItemList[i]->m_dwAttribute & 0xF0000000) >> 28;
			if (m_pItemList[i]->m_sIDnum == hb::shared::item::ItemId::AngelicPandentSTR)
				m_pPlayer->m_iAngelicStr = 1 + angel_value;
			else if (m_pItemList[i]->m_sIDnum == hb::shared::item::ItemId::AngelicPandentDEX)
				m_pPlayer->m_iAngelicDex = 1 + angel_value;
			else if (m_pItemList[i]->m_sIDnum == hb::shared::item::ItemId::AngelicPandentINT)
				m_pPlayer->m_iAngelicInt = 1 + angel_value;
			else if (m_pItemList[i]->m_sIDnum == hb::shared::item::ItemId::AngelicPandentMAG)
				m_pPlayer->m_iAngelicMag = 1 + angel_value;
		}
	}

	const auto* bank_header = reinterpret_cast<const hb::net::PacketResponseBankItemListHeader*>(itemEntries + total_items);
	total_items = bank_header->bank_item_count;

	for (int i = 0; i < hb::shared::limits::MaxBankItems; i++)
		if (m_pBankList[i] != 0)
		{
			m_pBankList[i].reset();
		}

	const auto* bankEntries = reinterpret_cast<const hb::net::PacketResponseBankItemEntry*>(bank_header + 1);
	for (int i = 0; i < total_items; i++)
	{
		const auto& entry = bankEntries[i];
		m_pBankList[i] = std::make_unique<CItem>();
		m_pBankList[i]->m_sIDnum = entry.item_id;
		m_pBankList[i]->m_dwCount = entry.count;
		m_pBankList[i]->m_sX = 40;
		m_pBankList[i]->m_sY = 30;
		m_pBankList[i]->m_wCurLifeSpan = entry.cur_lifespan;
		m_pBankList[i]->m_cItemColor = entry.item_color;
		m_pBankList[i]->m_sItemSpecEffectValue2 = static_cast<short>(entry.spec_value2); // v1.41
		m_pBankList[i]->m_dwAttribute = entry.attribute;
	}

	const auto* mastery = reinterpret_cast<const hb::net::PacketResponseMasteryData*>(bankEntries + total_items);

	for (int i = 0; i < hb::shared::limits::MaxMagicType; i++)
		m_pPlayer->m_iMagicMastery[i] = mastery->magic_mastery[i];

	for (int i = 0; i < hb::shared::limits::MaxSkillType; i++)
	{
		m_pPlayer->m_iSkillMastery[i] = static_cast<unsigned char>(mastery->skill_mastery[i]);
		if (m_pSkillCfgList[i] != 0)
			m_pSkillCfgList[i]->m_iLevel = static_cast<int>(mastery->skill_mastery[i]);
	}

	// Diagnostic: count what was loaded
	int nItems = 0, nBank = 0, nMagic = 0, nSkills = 0;
	for (int i = 0; i < hb::shared::limits::MaxItems; i++) if (m_pItemList[i]) nItems++;
	for (int i = 0; i < hb::shared::limits::MaxBankItems; i++) if (m_pBankList[i]) nBank++;
	for (int i = 0; i < hb::shared::limits::MaxMagicType; i++) if (m_pPlayer->m_iMagicMastery[i] != 0) nMagic++;
	for (int i = 0; i < hb::shared::limits::MaxSkillType; i++) if (m_pPlayer->m_iSkillMastery[i] != 0) nSkills++;
	DevConsole::Get().Printf("[INIT] InitItemList: %d items, %d bank, %d magic, %d skills",
		nItems, nBank, nMagic, nSkills);
}

void CGame::DrawDialogBoxs(short mouse_x, short mouse_y, short mouse_z, char left_button)
{
	if (m_bIsObserverMode == true) return;
	// Note: Dialogs that handle scroll should read hb::shared::input::GetMouseWheelDelta() and clear it after processing
	//Snoopy: 41->61
	bool icon_panel_drawn = false;
	for (int i = 0; i < 61; i++)
		if (m_dialogBoxManager.OrderAt(i) != 0)
		{
			switch (m_dialogBoxManager.OrderAt(i)) {
			case DialogBoxId::CharacterInfo:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::CharacterInfo))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::Inventory:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::Inventory))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::Magic:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::Magic))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::ItemDropConfirm:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::ItemDropConfirm))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::WarningBattleArea:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::WarningBattleArea))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::GuildMenu:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::GuildMenu))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::GuildOperation:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::GuildOperation))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::GuideMap:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::GuideMap))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::ChatHistory:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::ChatHistory))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::SaleMenu:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::SaleMenu))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::LevelUpSetting:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::LevelUpSetting))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::CityHallMenu:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::CityHallMenu))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::Bank:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::Bank))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::Skill:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::Skill))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::MagicShop:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::MagicShop))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::ItemDropExternal:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::ItemDropExternal))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::Text:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::Text))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::SystemMenu:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::SystemMenu))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::NpcActionQuery:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::NpcActionQuery))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::NpcTalk:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::NpcTalk))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::Map:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::Map))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::SellOrRepair:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::SellOrRepair))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::Fishing:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::Fishing))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::Noticement:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::Noticement))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::Manufacture:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::Manufacture))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::Exchange:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::Exchange))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::Quest:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::Quest))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::HudPanel:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::HudPanel))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				icon_panel_drawn = true;
				break;
			case DialogBoxId::SellList:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::SellList))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::Party:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::Party))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::CrusadeJob:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::CrusadeJob))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::ItemUpgrade:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::ItemUpgrade))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::Help:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::Help))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::CrusadeCommander:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::CrusadeCommander))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::CrusadeConstructor:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::CrusadeConstructor))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::CrusadeSoldier:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::CrusadeSoldier))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::Slates:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::Slates))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::ConfirmExchange:	//Snoopy: Confirmation Exchange
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::ConfirmExchange))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::ChangeStatsMajestic:
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::ChangeStatsMajestic))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::Resurrect: // Snoopy: Resurection?
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::Resurrect))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::GuildHallMenu: // Gail
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::GuildHallMenu))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			case DialogBoxId::RepairAll: //50Cent - Repair All
				if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::RepairAll))
					pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
				break;
			}
		}
	if (icon_panel_drawn == false)
	{
		if (auto* pDlg = m_dialogBoxManager.GetDialogBox(DialogBoxId::HudPanel))
			pDlg->OnDraw(mouse_x, mouse_y, mouse_z, left_button);
	}
	if (m_pPlayer->m_iSuperAttackLeft > 0)
	{
		std::string G_cTxt;
		int resx = (LOGICAL_WIDTH() - 640) / 2;
		int resy = LOGICAL_HEIGHT() - 480;
		// Combat icon position (same as HudPanel's COMBAT_ICON_X/Y)
		int iconX = 368 + resx;
		int iconY = 440 + resy;
		// Combat button area for text alignment
		int btnX = 362 + resx;
		int btnY = 434 + resy;
		int btnW = 42;
		int btnH = 41;

		bool bMastered = (m_pPlayer->m_iSkillMastery[CombatSystem::Get().GetWeaponSkillType()] == 100);

		// Draw additive overlay sprite at combat icon position only when ALT is held
		if (hb::shared::input::IsAltDown() && bMastered)
			m_pSprite[InterfaceNdIconPanel]->Draw(iconX, iconY, 3, hb::shared::sprite::DrawParams::Additive(0.7f));

		// Draw super attack count text at bottom-right of combat button area
		G_cTxt = std::format("{}", m_pPlayer->m_iSuperAttackLeft);
		if (bMastered)
			hb::shared::text::DrawTextAligned(GameFont::Bitmap1, btnX, btnY, btnW, btnH, G_cTxt.c_str(), hb::shared::text::TextStyle::WithIntegratedShadow(hb::shared::render::Color(255, 255, 255)), hb::shared::text::Align::BottomRight);
		else
			hb::shared::text::DrawTextAligned(GameFont::Bitmap1, btnX, btnY, btnW, btnH, G_cTxt.c_str(), hb::shared::text::TextStyle::WithHighlight(GameColors::BmpBtnActive), hb::shared::text::Align::BottomRight);
	}
}

void CGame::_Draw_CharacterBody(short sX, short sY, short sType)
{
	uint32_t dwTime = m_dwCurTime;

	if (sType <= 3)
	{
		m_pSprite[ItemEquipPivotPoint + 0]->Draw(sX, sY, sType - 1);
		const auto& hcM = GameColors::Hair[m_entityState.m_appearance.iHairColor];
		m_pSprite[ItemEquipPivotPoint + 18]->Draw(sX, sY, m_entityState.m_appearance.iHairStyle, hb::shared::sprite::DrawParams::Tint(hcM.r, hcM.g, hcM.b));

		m_pSprite[ItemEquipPivotPoint + 19]->Draw(sX, sY, m_entityState.m_appearance.iUnderwearType);
	}
	else
	{
		m_pSprite[ItemEquipPivotPoint + 40]->Draw(sX, sY, sType - 4);
		const auto& hcF = GameColors::Hair[m_entityState.m_appearance.iHairColor];
		m_pSprite[ItemEquipPivotPoint + 18 + 40]->Draw(sX, sY, m_entityState.m_appearance.iHairStyle, hb::shared::sprite::DrawParams::Tint(hcF.r, hcF.g, hcF.b));
		m_pSprite[ItemEquipPivotPoint + 19 + 40]->Draw(sX, sY, m_entityState.m_appearance.iUnderwearType);
	}
}

int CGame::iGetTopDialogBoxIndex()
{
	int i;
	//Snoopy: 38->58
	for (i = 58; i >= 0; i--)
		if (m_dialogBoxManager.OrderAt(i) != 0)
			return m_dialogBoxManager.OrderAt(i);

	return 0;
}


void CGame::_LoadTextDlgContents(int cType)
{
	for (int i = 0; i < game_limits::max_text_dlg_lines; i++)
	{
		if (m_pMsgTextList[i] != 0)
			m_pMsgTextList[i].reset();
	}

	std::string fileName = std::format("contents/contents{}.txt", cType);

	std::ifstream file(fileName);
	if (!file) return;

	std::string line;
	int iIndex = 0;
	while (std::getline(file, line) && iIndex < game_limits::max_text_dlg_lines)
	{
		if (!line.empty())
		{
			m_pMsgTextList[iIndex] = std::make_unique<CMsg>(0, line.c_str(), 0);
			iIndex++;
		}
	}
}

int CGame::_iLoadTextDlgContents2(int iType)
{
	for (int i = 0; i < game_limits::max_text_dlg_lines; i++)
	{
		if (m_pMsgTextList2[i] != 0)
			m_pMsgTextList2[i].reset();
	}

	std::string fileName = std::format("contents/contents{}.txt", iType);

	std::ifstream file(fileName);
	if (!file) return -1;

	std::string line;
	int iIndex = 0;
	while (std::getline(file, line) && iIndex < game_limits::max_text_dlg_lines)
	{
		if (!line.empty())
		{
			m_pMsgTextList2[iIndex] = std::make_unique<CMsg>(0, line.c_str(), 0);
			iIndex++;
		}
	}
	return iIndex;
}

void CGame::_LoadGameMsgTextContents()
{
	for (int i = 0; i < game_limits::max_game_msgs; i++)
	{
		if (m_pGameMsgList[i] != 0)
			m_pGameMsgList[i].reset();
	}

	std::ifstream file("contents/GameMsgList.txt");
	if (!file) return;

	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	int iIndex = 0;
	size_t start = 0;
	size_t end = 0;
	while ((end = content.find_first_of(";\n", start)) != std::string::npos && iIndex < game_limits::max_game_msgs)
	{
		if (end > start)
		{
			std::string token = content.substr(start, end - start);
			m_pGameMsgList[iIndex] = std::make_unique<CMsg>(0, token.c_str(), 0);
			iIndex++;
		}
		start = end + 1;
	}
	if (start < content.size() && iIndex < game_limits::max_game_msgs)
	{
		std::string token = content.substr(start);
		if (!token.empty())
		{
			m_pGameMsgList[iIndex] = std::make_unique<CMsg>(0, token.c_str(), 0);
		}
	}
}

void CGame::_RequestMapStatus(const char* pMapName, int iMode)
{
	bSendCommand(MsgId::CommandCommon, CommonType::RequestMapStatus, 0, iMode, 0, 0, pMapName);
}

void CGame::AddMapStatusInfo(const char* pData, bool bIsLastData)
{
	char cTotal;
	short sIndex;
	int i;


	const auto* header = hb::net::PacketCast<hb::net::PacketNotifyMapStatusHeader>(
		pData, sizeof(hb::net::PacketNotifyMapStatusHeader));
	if (!header) return;
	m_cStatusMapName.assign(header->map_name, strnlen(header->map_name, sizeof(header->map_name)));
	sIndex = header->index;
	cTotal = header->total;

	const auto* entries = reinterpret_cast<const hb::net::PacketNotifyMapStatusEntry*>(header + 1);

	for (i = 1; i <= cTotal; i++) {
		m_stCrusadeStructureInfo[sIndex].cType = entries->type;
		m_stCrusadeStructureInfo[sIndex].sX = entries->x;
		m_stCrusadeStructureInfo[sIndex].sY = entries->y;
		m_stCrusadeStructureInfo[sIndex].cSide = entries->side;
		entries++;

		sIndex++;
	}

	if (bIsLastData == true) {
		while (sIndex < hb::shared::limits::MaxCrusadeStructures) {
			m_stCrusadeStructureInfo[sIndex].cType = 0;
			m_stCrusadeStructureInfo[sIndex].sX = 0;
			m_stCrusadeStructureInfo[sIndex].sY = 0;
			m_stCrusadeStructureInfo[sIndex].cSide = 0;
			sIndex++;
		}
	}
}

void CGame::DrawNewDialogBox(char cType, int sX, int sY, int iFrame, bool bIsNoColorKey, bool bIsTrans)
{
	if (m_pSprite[cType] == 0) return;
	if (bIsNoColorKey == false)
	{
		if (bIsTrans == true)
			m_pSprite[cType]->Draw(sX, sY, iFrame, hb::shared::sprite::DrawParams::Alpha(0.25f));
		else m_pSprite[cType]->Draw(sX, sY, iFrame);
	}
	else m_pSprite[cType]->Draw(sX, sY, iFrame, hb::shared::sprite::DrawParams::NoColorKey());
}

void CGame::SetCameraShakingEffect(short sDist, int iMul)
{
	int iDegree = 5 - sDist;
	if (iDegree <= 0) iDegree = 0;
	iDegree *= 2;

	if (iMul != 0) iDegree *= iMul;

	if (iDegree <= 2) return;

	m_Camera.SetShake(iDegree);
}

void CGame::MeteorStrikeComing(int iCode)
{
	switch (iCode) {
	case 1: //
		SetTopMsg(m_pGameMsgList[0]->m_pMsg, 5);
		break;
	case 2: //
		SetTopMsg(m_pGameMsgList[10]->m_pMsg, 10);
		break;
	case 3: //
		SetTopMsg(m_pGameMsgList[91]->m_pMsg, 5);
		break;
	case 4: //
		SetTopMsg(m_pGameMsgList[11]->m_pMsg, 10);
		break;
	}
}

void CGame::DrawObjectFOE(int ix, int iy, int iFrame)
{
	if (IsHostile(m_entityState.m_status.iRelationship)) // red crusade circle
	{
		if (iFrame <= 4) m_pEffectSpr[38]->Draw(ix, iy, iFrame, hb::shared::sprite::DrawParams::Alpha(0.5f));
	}
}

void CGame::SetTopMsg(const char* pString, unsigned char iLastSec)
{
	m_cTopMsg = pString;

	m_iTopMsgLastSec = iLastSec;
	m_dwTopMsgTime = GameClock::GetTimeMS();
}

void CGame::DrawTopMsg()
{
	if (m_cTopMsg.size() == 0) return;
	m_Renderer->DrawRectFilled(0, 0, LOGICAL_MAX_X(), 30, hb::shared::render::Color::Black(128));

	if ((((GameClock::GetTimeMS() - m_dwTopMsgTime) / 250) % 2) == 0)
		hb::shared::text::DrawTextAligned(GameFont::Default, 0, 10, LOGICAL_MAX_X(), 15, m_cTopMsg.c_str(), hb::shared::text::TextStyle::Color(GameColors::UITopMsgYellow), hb::shared::text::Align::TopCenter);

	if (GameClock::GetTimeMS() > (m_iTopMsgLastSec * 1000 + m_dwTopMsgTime)) {
	}
}

void CGame::CannotConstruct(int iCode)
{
	std::string G_cTxt;
	switch (iCode) {
	case 1: //
		SetTopMsg(m_pGameMsgList[18]->m_pMsg, 5);
		break;

	case 2: //
		G_cTxt = std::format("{} XY({}, {})", m_pGameMsgList[19]->m_pMsg, m_pPlayer->m_iConstructLocX, m_pPlayer->m_iConstructLocY);
		SetTopMsg(G_cTxt.c_str(), 5);
		break;

	case 3: //
		SetTopMsg(m_pGameMsgList[20]->m_pMsg, 5);
		break;
	case 4: //
		SetTopMsg(m_pGameMsgList[20]->m_pMsg, 5);
		break;

	}
}

std::string CGame::FormatCommaNumber(uint32_t value)
{
	auto numStr = std::format("{}", value);
#ifdef DEF_COMMA_GOLD
	std::string result;
	int len = static_cast<int>(numStr.length());
	for (int i = 0; i < len; i++)
	{
		if (i > 0 && (len - i) % 3 == 0)
			result += ',';
		result += numStr[i];
	}
	return result;
#else
	return numStr;
#endif
}


void CGame::CrusadeContributionResult(int war_contribution)
{
	char temp[120]{};
	m_dialogBoxManager.DisableDialogBox(DialogBoxId::Text);
	for (int i = 0; i < game_limits::max_text_dlg_lines; i++)
	{
		if (m_pMsgTextList[i] != 0)
			m_pMsgTextList[i].reset();
	}
	if (war_contribution > 0)
	{
		PlayGameSound('E', 23, 0, 0);
		PlayGameSound('C', 21, 0, 0);
		PlayGameSound('C', 22, 0, 0);
		m_pMsgTextList[0] = std::make_unique<CMsg>(0, m_pGameMsgList[22]->m_pMsg, 0); // Congratulations! Your nation
		m_pMsgTextList[1] = std::make_unique<CMsg>(0, m_pGameMsgList[23]->m_pMsg, 0); // was victory in the battle!
		m_pMsgTextList[2] = std::make_unique<CMsg>(0, " ", 0);
		m_pMsgTextList[3] = std::make_unique<CMsg>(0, m_pGameMsgList[24]->m_pMsg, 0); // As a victorious citizen
		m_pMsgTextList[4] = std::make_unique<CMsg>(0, m_pGameMsgList[25]->m_pMsg, 0); // You will receive
		m_pMsgTextList[5] = std::make_unique<CMsg>(0, m_pGameMsgList[26]->m_pMsg, 0); // a prize
		m_pMsgTextList[6] = std::make_unique<CMsg>(0, " ", 0);
		m_pMsgTextList[7] = std::make_unique<CMsg>(0, m_pGameMsgList[27]->m_pMsg, 0); // Experience point of the battle contribution:
		std::snprintf(temp, sizeof(temp), "+%dExp Points!", war_contribution);
		m_pMsgTextList[8] = std::make_unique<CMsg>(0, temp, 0);
		for (int i = 9; i < 18; i++)
			m_pMsgTextList[i] = std::make_unique<CMsg>(0, " ", 0);

	}
	else if (war_contribution < 0)
	{
		PlayGameSound('E', 24, 0, 0);
		PlayGameSound('C', 12, 0, 0);
		PlayGameSound('C', 13, 0, 0);
		m_pMsgTextList[0] = std::make_unique<CMsg>(0, m_pGameMsgList[28]->m_pMsg, 0); // Unfortunately! Your country
		m_pMsgTextList[1] = std::make_unique<CMsg>(0, m_pGameMsgList[29]->m_pMsg, 0); // have lost the all out war.
		m_pMsgTextList[2] = std::make_unique<CMsg>(0, " ", 0);
		m_pMsgTextList[3] = std::make_unique<CMsg>(0, m_pGameMsgList[30]->m_pMsg, 0); // As a losser citizen;
		m_pMsgTextList[4] = std::make_unique<CMsg>(0, m_pGameMsgList[31]->m_pMsg, 0); // the prize that accomplishes
		m_pMsgTextList[5] = std::make_unique<CMsg>(0, m_pGameMsgList[32]->m_pMsg, 0); // will not be given.
		m_pMsgTextList[6] = std::make_unique<CMsg>(0, " ", 0);
		m_pMsgTextList[7] = std::make_unique<CMsg>(0, m_pGameMsgList[33]->m_pMsg, 0); // I hope you to win
		m_pMsgTextList[8] = std::make_unique<CMsg>(0, m_pGameMsgList[34]->m_pMsg, 0); // in the next battle
		for (int i = 9; i < 18; i++)
			m_pMsgTextList[i] = std::make_unique<CMsg>(0, " ", 0);
	}
	else if (war_contribution == 0)
	{
		PlayGameSound('E', 25, 0, 0);
		m_pMsgTextList[0] = std::make_unique<CMsg>(0, m_pGameMsgList[50]->m_pMsg, 0); // The battle that you have participated
		m_pMsgTextList[1] = std::make_unique<CMsg>(0, m_pGameMsgList[51]->m_pMsg, 0); // is already finished;
		m_pMsgTextList[2] = std::make_unique<CMsg>(0, m_pGameMsgList[52]->m_pMsg, 0); //
		m_pMsgTextList[3] = std::make_unique<CMsg>(0, " ", 0);
		m_pMsgTextList[4] = std::make_unique<CMsg>(0, m_pGameMsgList[53]->m_pMsg, 0); // You must connect after finishing
		m_pMsgTextList[5] = std::make_unique<CMsg>(0, m_pGameMsgList[54]->m_pMsg, 0); // the previous and before starting
		m_pMsgTextList[6] = std::make_unique<CMsg>(0, m_pGameMsgList[55]->m_pMsg, 0); // the next battle so you can receive
		m_pMsgTextList[7] = std::make_unique<CMsg>(0, m_pGameMsgList[56]->m_pMsg, 0); // the prize
		for (int i = 8; i < 18; i++)
			m_pMsgTextList[i] = std::make_unique<CMsg>(0, " ", 0);
	}
	m_dialogBoxManager.EnableDialogBox(DialogBoxId::Text, 0, 0, 0);
}

void CGame::CrusadeWarResult(int winner_side)
{
	int player_side = 0;
	m_dialogBoxManager.DisableDialogBox(DialogBoxId::Text);
	for (int i = 0; i < game_limits::max_text_dlg_lines; i++)
	{
		if (m_pMsgTextList[i] != 0)
			m_pMsgTextList[i].reset();
	}
	if (m_pPlayer->m_bCitizen == false) player_side = 0;
	else if (m_pPlayer->m_bAresden == true) player_side = 1;
	else if (m_pPlayer->m_bAresden == false) player_side = 2;
	if (player_side == 0)
	{
		switch (winner_side) {
		case 0:
			PlayGameSound('E', 25, 0, 0);
			m_pMsgTextList[0] = std::make_unique<CMsg>(0, m_pGameMsgList[35]->m_pMsg, 0); // All out war finished!
			m_pMsgTextList[1] = std::make_unique<CMsg>(0, m_pGameMsgList[36]->m_pMsg, 0); // There was a draw in the
			m_pMsgTextList[2] = std::make_unique<CMsg>(0, m_pGameMsgList[37]->m_pMsg, 0); // battle
			m_pMsgTextList[3] = std::make_unique<CMsg>(0, " ", 0);
			break;
		case 1:
			PlayGameSound('E', 25, 0, 0);
			m_pMsgTextList[0] = std::make_unique<CMsg>(0, m_pGameMsgList[35]->m_pMsg, 0); // All out war finished!
			m_pMsgTextList[1] = std::make_unique<CMsg>(0, m_pGameMsgList[38]->m_pMsg, 0); // Aresden was victorious
			m_pMsgTextList[2] = std::make_unique<CMsg>(0, m_pGameMsgList[39]->m_pMsg, 0); // and put an end to the war
			m_pMsgTextList[3] = std::make_unique<CMsg>(0, " ", 0);
			break;
		case 2:
			PlayGameSound('E', 25, 0, 0);
			m_pMsgTextList[0] = std::make_unique<CMsg>(0, m_pGameMsgList[35]->m_pMsg, 0); // All out war finished!
			m_pMsgTextList[1] = std::make_unique<CMsg>(0, m_pGameMsgList[40]->m_pMsg, 0); // Elvine was victorious
			m_pMsgTextList[2] = std::make_unique<CMsg>(0, m_pGameMsgList[41]->m_pMsg, 0); // and put an end to the war
			m_pMsgTextList[3] = std::make_unique<CMsg>(0, " ", 0);
			break;
		}
		for (int i = 4; i < 18; i++)
			m_pMsgTextList[i] = std::make_unique<CMsg>(0, " ", 0);
	}
	else
	{
		if (winner_side == 0)
		{
			PlayGameSound('E', 25, 0, 0);
			m_pMsgTextList[0] = std::make_unique<CMsg>(0, m_pGameMsgList[35]->m_pMsg, 0); // All out war finished!
			m_pMsgTextList[1] = std::make_unique<CMsg>(0, m_pGameMsgList[36]->m_pMsg, 0); // There was a draw in the
			m_pMsgTextList[2] = std::make_unique<CMsg>(0, m_pGameMsgList[37]->m_pMsg, 0); // battle
			m_pMsgTextList[3] = std::make_unique<CMsg>(0, " ", 0);
			for (int i = 4; i < 18; i++)
				m_pMsgTextList[i] = std::make_unique<CMsg>(0, " ", 0);
		}
		else
		{
			if (winner_side == player_side)
			{
				PlayGameSound('E', 23, 0, 0);
				PlayGameSound('C', 21, 0, 0);
				PlayGameSound('C', 22, 0, 0);
				switch (winner_side) {
				case 1:
					m_pMsgTextList[0] = std::make_unique<CMsg>(0, m_pGameMsgList[35]->m_pMsg, 0); // All out war finished!;
					m_pMsgTextList[1] = std::make_unique<CMsg>(0, m_pGameMsgList[38]->m_pMsg, 0); // Aresden was victorious;
					m_pMsgTextList[2] = std::make_unique<CMsg>(0, m_pGameMsgList[39]->m_pMsg, 0); // and put an end to the war
					m_pMsgTextList[3] = std::make_unique<CMsg>(0, " ", 0);
					m_pMsgTextList[4] = std::make_unique<CMsg>(0, m_pGameMsgList[42]->m_pMsg, 0); // Congratulations!
					m_pMsgTextList[5] = std::make_unique<CMsg>(0, m_pGameMsgList[43]->m_pMsg, 0); // As a victorious citizen
					m_pMsgTextList[6] = std::make_unique<CMsg>(0, m_pGameMsgList[44]->m_pMsg, 0); // You will receive
					m_pMsgTextList[7] = std::make_unique<CMsg>(0, m_pGameMsgList[45]->m_pMsg, 0); // a prize
					break;
				case 2:
					m_pMsgTextList[0] = std::make_unique<CMsg>(0, m_pGameMsgList[35]->m_pMsg, 0); // All out war finished!
					m_pMsgTextList[1] = std::make_unique<CMsg>(0, m_pGameMsgList[40]->m_pMsg, 0); // Elvine was victorious
					m_pMsgTextList[2] = std::make_unique<CMsg>(0, m_pGameMsgList[41]->m_pMsg, 0); // and put an end to the war
					m_pMsgTextList[3] = std::make_unique<CMsg>(0, " ", 0);
					m_pMsgTextList[4] = std::make_unique<CMsg>(0, m_pGameMsgList[42]->m_pMsg, 0); // Congratulations!
					m_pMsgTextList[5] = std::make_unique<CMsg>(0, m_pGameMsgList[43]->m_pMsg, 0); // As a victorious citizen
					m_pMsgTextList[6] = std::make_unique<CMsg>(0, m_pGameMsgList[44]->m_pMsg, 0); // You will receive
					m_pMsgTextList[7] = std::make_unique<CMsg>(0, m_pGameMsgList[45]->m_pMsg, 0); // a prize
					break;
				}
				for (int i = 8; i < 18; i++)
					m_pMsgTextList[i] = std::make_unique<CMsg>(0, " ", 0);
			}
			else if (winner_side != player_side)
			{
				PlayGameSound('E', 24, 0, 0);
				PlayGameSound('C', 12, 0, 0);
				PlayGameSound('C', 13, 0, 0);
				switch (winner_side) {
				case 1:
					m_pMsgTextList[0] = std::make_unique<CMsg>(0, m_pGameMsgList[35]->m_pMsg, 0); // All out war finished!
					m_pMsgTextList[1] = std::make_unique<CMsg>(0, m_pGameMsgList[38]->m_pMsg, 0); // Aresden was victorious;
					m_pMsgTextList[2] = std::make_unique<CMsg>(0, m_pGameMsgList[39]->m_pMsg, 0); // and put an end to the war
					m_pMsgTextList[3] = std::make_unique<CMsg>(0, " ", 0);
					m_pMsgTextList[4] = std::make_unique<CMsg>(0, m_pGameMsgList[46]->m_pMsg, 0); // Unfortunately,
					m_pMsgTextList[5] = std::make_unique<CMsg>(0, m_pGameMsgList[47]->m_pMsg, 0); // As a losser citizen
					m_pMsgTextList[6] = std::make_unique<CMsg>(0, m_pGameMsgList[48]->m_pMsg, 0); // the prize that accomplishes
					m_pMsgTextList[7] = std::make_unique<CMsg>(0, m_pGameMsgList[49]->m_pMsg, 0); // will not be given.
					break;
				case 2:
					m_pMsgTextList[0] = std::make_unique<CMsg>(0, m_pGameMsgList[35]->m_pMsg, 0); // All out war finished!
					m_pMsgTextList[1] = std::make_unique<CMsg>(0, m_pGameMsgList[40]->m_pMsg, 0); // Elvine was victorious
					m_pMsgTextList[2] = std::make_unique<CMsg>(0, m_pGameMsgList[41]->m_pMsg, 0); // and put an end to the war
					m_pMsgTextList[3] = std::make_unique<CMsg>(0, " ", 0);
					m_pMsgTextList[4] = std::make_unique<CMsg>(0, m_pGameMsgList[46]->m_pMsg, 0); // Unfortunately,
					m_pMsgTextList[5] = std::make_unique<CMsg>(0, m_pGameMsgList[47]->m_pMsg, 0); // As a losser citizen
					m_pMsgTextList[6] = std::make_unique<CMsg>(0, m_pGameMsgList[48]->m_pMsg, 0); // the prize that accomplishes
					m_pMsgTextList[7] = std::make_unique<CMsg>(0, m_pGameMsgList[49]->m_pMsg, 0); // will not be given.
					break;
				}
				for (int i = 8; i < 18; i++)
					m_pMsgTextList[i] = std::make_unique<CMsg>(0, " ", 0);
			}
		}
	}
	m_dialogBoxManager.EnableDialogBox(DialogBoxId::Text, 0, 0, 0);
	m_dialogBoxManager.DisableDialogBox(DialogBoxId::CrusadeCommander);
	m_dialogBoxManager.DisableDialogBox(DialogBoxId::CrusadeConstructor);
	m_dialogBoxManager.DisableDialogBox(DialogBoxId::CrusadeSoldier);
}


// _Draw_UpdateScreen_OnCreateNewAccount removed - migrated to Screen_CreateAccount

void CGame::CivilRightAdmissionHandler(char* pData)
{
	uint16_t wResult;
	const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(
		pData, sizeof(hb::net::PacketHeader));
	if (!header) return;
	wResult = header->msg_type;

	switch (wResult) {
	case 0:
		m_dialogBoxManager.Info(DialogBoxId::CityHallMenu).cMode = 4;
		break;

	case 1:
		m_dialogBoxManager.Info(DialogBoxId::CityHallMenu).cMode = 3;
		const auto* pkt = hb::net::PacketCast<hb::net::PacketResponseCivilRight>(
			pData, sizeof(hb::net::PacketResponseCivilRight));
		if (!pkt) return;
		m_cLocation.assign(pkt->location, strnlen(pkt->location, 10));
		if (m_cLocation.starts_with("aresden"))
		{
			m_pPlayer->m_bAresden = true;
			m_pPlayer->m_bCitizen = true;
			m_pPlayer->m_bHunter = false;
		}
		else if (m_cLocation.starts_with("arehunter"))
		{
			m_pPlayer->m_bAresden = true;
			m_pPlayer->m_bCitizen = true;
			m_pPlayer->m_bHunter = true;
		}
		else if (m_cLocation.starts_with("elvine"))
		{
			m_pPlayer->m_bAresden = false;
			m_pPlayer->m_bCitizen = true;
			m_pPlayer->m_bHunter = false;
		}
		else if (m_cLocation.starts_with("elvhunter"))
		{
			m_pPlayer->m_bAresden = false;
			m_pPlayer->m_bCitizen = true;
			m_pPlayer->m_bHunter = true;
		}
		else
		{
			m_pPlayer->m_bAresden = true;
			m_pPlayer->m_bCitizen = false;
			m_pPlayer->m_bHunter = true;
		}
		break;
	}
}

void CGame::PlayGameSound(char cType, int iNum, int iDist, long lPan)
{
	// Forward to AudioManager
	SoundType type;
	switch (cType)
	{
	case 'C':
		type = SoundType::Character;
		break;
	case 'M':
		type = SoundType::Monster;
		break;
	case 'E':
		type = SoundType::Effect;
		break;
	default:
		return;
	}
	AudioManager::Get().PlayGameSound(type, iNum, iDist, static_cast<int>(lPan));
}

bool CGame::_bCheckItemByType(ItemType type)
{
	int i;

	for (i = 0; i < hb::shared::limits::MaxItems; i++)
		if (m_pItemList[i] != 0) {
			CItem* pCfg = GetItemConfig(m_pItemList[i]->m_sIDnum);
			if (pCfg && pCfg->GetItemType() == type) return true;
		}

	return false;
}

void CGame::DynamicObjectHandler(char* pData)
{
	short sX, sY, sV1, sV2, sV3;
	const auto* pkt = hb::net::PacketCast<hb::net::PacketResponseDynamicObject>(
		pData, sizeof(hb::net::PacketResponseDynamicObject));
	if (!pkt) return;
	sX = pkt->x;
	sY = pkt->y;
	sV1 = pkt->v1;
	sV2 = pkt->v2;
	sV3 = pkt->v3;

	switch (pkt->header.msg_type) {
	case MsgType::Confirm:// Dynamic Object
		m_pMapData->bSetDynamicObject(sX, sY, sV2, sV1, true);
		break;

	case MsgType::Reject:// Dynamic object
		m_pMapData->bSetDynamicObject(sX, sY, sV2, 0, true);
		break;
	}
}

bool CGame::_bIsItemOnHand() // Snoopy: Fixed to remove ShieldCast
{
	int i;
	uint16_t wWeaponType;
	for (i = 0; i < hb::shared::limits::MaxItems; i++)
		if ((m_pItemList[i] != 0) && (m_bIsItemEquipped[i] == true))
		{
			CItem* pCfg = GetItemConfig(m_pItemList[i]->m_sIDnum);
			if (pCfg && ((pCfg->GetEquipPos() == EquipPos::LeftHand)
				|| (pCfg->GetEquipPos() == EquipPos::TwoHand)))
				return true;
		}
	for (i = 0; i < hb::shared::limits::MaxItems; i++)
		if ((m_pItemList[i] != 0) && (m_bIsItemEquipped[i] == true))
		{
			CItem* pCfg = GetItemConfig(m_pItemList[i]->m_sIDnum);
			if (pCfg && pCfg->GetEquipPos() == EquipPos::RightHand)
			{
				wWeaponType = m_pPlayer->m_playerAppearance.iWeaponType;
				// Snoopy 34 for all wands.
				if ((wWeaponType >= 34) && (wWeaponType < 40)) return false;
				else return true;
			}
		}
	return false;
}

uint32_t CGame::iGetLevelExp(int iLevel)
{
	return hb::shared::calc::CalculateLevelExp(iLevel);
}

bool CGame::bCheckExID(const char* pName)
{
	if (m_pExID == 0) return false;
	if (m_pPlayer->m_cPlayerName == pName) return false;
	std::string cTxt;
	cTxt = m_pExID->m_pMsg;
	if (memcmp(cTxt.c_str(), pName, 10) == 0) return true;
	else return false;
}

bool CGame::_ItemDropHistory(short sItemID)
{
	bool bFlag = false;
	if (m_iItemDropCnt == 0)
	{
		m_sItemDropID[m_iItemDropCnt] = sItemID;
		m_iItemDropCnt++;
		return true;
	}
	if ((1 <= m_iItemDropCnt) && (20 >= m_iItemDropCnt))
	{
		for (int i = 0; i < m_iItemDropCnt; i++)
		{
			if (m_sItemDropID[i] == sItemID)
			{
				bFlag = true;
				break;
			}
		}
		if (bFlag)
		{
			if (m_bItemDrop)
				return false;
			else
				return true;
		}

		if (20 < m_iItemDropCnt)
		{
			for (int i = 0; i < m_iItemDropCnt; i++)
				m_sItemDropID[i - 1] = sItemID;
			m_sItemDropID[20] = sItemID;
			m_iItemDropCnt = 21;
		}
		else
		{
			m_sItemDropID[m_iItemDropCnt] = sItemID;
			m_iItemDropCnt++;
		}
	}
	return true;
}

CItem* CGame::GetItemConfig(int iItemID) const
{
	if (iItemID <= 0 || iItemID >= 5000) return nullptr;
	return m_pItemConfigList[iItemID].get();
}

bool CGame::_EnsureConfigLoaded(int type)
{
	// Fast path: check if a representative config entry exists
	bool bLoaded = false;
	switch (type) {
	case 0: // Items
		for (int i = 1; i < 5000; i++) { if (m_pItemConfigList[i]) { bLoaded = true; break; } }
		break;
	case 1: // Magic
		for (int i = 0; i < hb::shared::limits::MaxMagicType; i++) { if (m_pMagicCfgList[i]) { bLoaded = true; break; } }
		break;
	case 2: // Skills
		for (int i = 0; i < hb::shared::limits::MaxSkillType; i++) { if (m_pSkillCfgList[i]) { bLoaded = true; break; } }
		break;
	case 3: // Npcs
		for (int i = 0; i < hb::shared::limits::MaxNpcConfigs; i++) { if (m_npcConfigList[i].valid) { bLoaded = true; break; } }
		break;
	}

	if (bLoaded) {
		m_eConfigRetry[type] = ConfigRetryLevel::None;
		return true;
	}

	const char* configNames[] = { "ITEMCFG", "MAGICCFG", "SKILLCFG", "NPCCFG" };

	switch (m_eConfigRetry[type]) {
	case ConfigRetryLevel::None:
		if (_TryReplayCacheForConfig(type)) {
			DevConsole::Get().Printf("[%s] Retry: loaded from cache", configNames[type]);
			return true;
		}
		DevConsole::Get().Printf("[%s] Retry: cache failed, will request from server", configNames[type]);
		m_eConfigRetry[type] = ConfigRetryLevel::CacheTried;
		return false;

	case ConfigRetryLevel::CacheTried:
		DevConsole::Get().Printf("[%s] Retry: requesting from server", configNames[type]);
		_RequestConfigsFromServer(type == 0, type == 1, type == 2, type == 3);
		m_eConfigRetry[type] = ConfigRetryLevel::ServerRequested;
		m_dwConfigRequestTime = GameClock::GetTimeMS();
		return false;

	case ConfigRetryLevel::ServerRequested:
		if (GameClock::GetTimeMS() - m_dwConfigRequestTime > CONFIG_REQUEST_TIMEOUT_MS) {
			DevConsole::Get().Printf("[%s] Retry: server request timed out", configNames[type]);
			m_eConfigRetry[type] = ConfigRetryLevel::Failed;
			ChangeGameMode(GameMode::ConnectionLost);
		}
		return false;

	case ConfigRetryLevel::Failed:
		return false;
	}
	return false;
}

bool CGame::_TryReplayCacheForConfig(int type)
{
	struct ReplayCtx { CGame* game; };
	ReplayCtx ctx{ this };

	ConfigCacheType cacheType = static_cast<ConfigCacheType>(type);

	switch (type) {
	case 0:
		return LocalCacheManager::Get().ReplayFromCache(cacheType,
			[](char* p, uint32_t s, void* c) -> bool {
				return static_cast<ReplayCtx*>(c)->game->_bDecodeItemConfigFileContents(p, s);
			}, &ctx);
	case 1:
		return LocalCacheManager::Get().ReplayFromCache(cacheType,
			[](char* p, uint32_t s, void* c) -> bool {
				return static_cast<ReplayCtx*>(c)->game->_bDecodeMagicConfigFileContents(p, s);
			}, &ctx);
	case 2:
		return LocalCacheManager::Get().ReplayFromCache(cacheType,
			[](char* p, uint32_t s, void* c) -> bool {
				return static_cast<ReplayCtx*>(c)->game->_bDecodeSkillConfigFileContents(p, s);
			}, &ctx);
	case 3:
		return LocalCacheManager::Get().ReplayFromCache(cacheType,
			[](char* p, uint32_t s, void* c) -> bool {
				return static_cast<ReplayCtx*>(c)->game->_bDecodeNpcConfigFileContents(p, s);
			}, &ctx);
	}
	return false;
}

void CGame::_RequestConfigsFromServer(bool bItems, bool bMagic, bool bSkills, bool bNpcs)
{
	if (!m_pGSock) return;
	hb::net::PacketRequestConfigData pkt{};
	pkt.header.msg_id = MsgId::RequestConfigData;
	pkt.header.msg_type = 0;
	pkt.requestItems = bItems ? 1 : 0;
	pkt.requestMagic = bMagic ? 1 : 0;
	pkt.requestSkills = bSkills ? 1 : 0;
	pkt.requestNpcs = bNpcs ? 1 : 0;
	m_pGSock->iSendMsg(reinterpret_cast<char*>(&pkt), sizeof(pkt));
}

void CGame::_CheckConfigsReadyAndEnterGame()
{
	if (m_bConfigsReady) return;

	// Check if all four config types have at least one entry loaded
	bool bHasItems = false, bHasMagic = false, bHasSkills = false, bHasNpcs = false;
	for (int i = 1; i < 5000; i++) { if (m_pItemConfigList[i]) { bHasItems = true; break; } }
	for (int i = 0; i < hb::shared::limits::MaxMagicType; i++) { if (m_pMagicCfgList[i]) { bHasMagic = true; break; } }
	for (int i = 0; i < hb::shared::limits::MaxSkillType; i++) { if (m_pSkillCfgList[i]) { bHasSkills = true; break; } }
	for (int i = 0; i < hb::shared::limits::MaxNpcConfigs; i++) { if (m_npcConfigList[i].valid) { bHasNpcs = true; break; } }

	if (bHasItems && bHasMagic && bHasSkills && bHasNpcs) {
		DevConsole::Get().Printf("[CONFIG] All configs loaded from server, entering game");
		m_bConfigsReady = true;
		if (m_bInitDataReady) {
			GameModeManager::set_screen<Screen_OnGame>();
			m_bInitDataReady = false;
		}
	}
}

short CGame::FindItemIdByName(const char* cItemName)
{
	if (cItemName == nullptr) return 0;
	for (int i = 1; i < 5000; i++) {
		if (m_pItemConfigList[i] != nullptr &&
			memcmp(m_pItemConfigList[i]->m_cName, cItemName, hb::shared::limits::ItemNameLen - 1) == 0) {
			return static_cast<short>(i);
		}
	}
	return 0; // Not found
}

void CGame::NoticementHandler(char* pData)
{
	const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(
		pData, sizeof(hb::net::PacketHeader));
	if (!header) return;
	switch (header->msg_type) {
	case MsgType::Confirm:
	case MsgType::Reject:
		const auto* pkt = hb::net::PacketCast<hb::net::PacketResponseNoticementText>(
			pData, sizeof(hb::net::PacketResponseNoticementText));
		if (!pkt) return;
		{
			std::ofstream file("contents/contents1000.txt");
			if (!file) return;
			file << pkt->text;
		}
		m_dialogBoxManager.Info(DialogBoxId::Text).sX = 20;
		m_dialogBoxManager.Info(DialogBoxId::Text).sY = 65;
		m_dialogBoxManager.EnableDialogBox(DialogBoxId::Text, 1000, 0, 0);
		break;
	}
	AddEventList("Press F1 for news and help.", 10);
	m_dialogBoxManager.EnableDialogBox(DialogBoxId::Help, 0, 0, 0);
}

void CGame::_SetIlusionEffect(int iOwnerH)
{
	char cDir;

	m_iIlusionOwnerH = iOwnerH;

	std::string nameBuf_IE;
	m_pMapData->GetOwnerStatusByObjectID(iOwnerH, &m_cIlusionOwnerType, &cDir, &m_pPlayer->m_illusionAppearance, &m_pPlayer->m_illusionStatus, nameBuf_IE);
	m_cName_IE = nameBuf_IE;
}

void CGame::ResponsePanningHandler(char* pData)
{
	char cDir;
	short sX, sY;
	const auto* pkt = hb::net::PacketCast<hb::net::PacketResponsePanningHeader>(
		pData, sizeof(hb::net::PacketResponsePanningHeader));
	if (!pkt) return;
	sX = pkt->x;
	sY = pkt->y;
	cDir = static_cast<char>(pkt->dir);

	switch (cDir) {
	case 1: m_Camera.MoveDestination(0, -32); m_pPlayer->m_sPlayerY--; break;
	case 2: m_Camera.MoveDestination(32, -32); m_pPlayer->m_sPlayerY--; m_pPlayer->m_sPlayerX++; break;
	case 3: m_Camera.MoveDestination(32, 0); m_pPlayer->m_sPlayerX++; break;
	case 4: m_Camera.MoveDestination(32, 32); m_pPlayer->m_sPlayerY++; m_pPlayer->m_sPlayerX++; break;
	case 5: m_Camera.MoveDestination(0, 32); m_pPlayer->m_sPlayerY++; break;
	case 6: m_Camera.MoveDestination(-32, 32); m_pPlayer->m_sPlayerY++; m_pPlayer->m_sPlayerX--; break;
	case 7: m_Camera.MoveDestination(-32, 0); m_pPlayer->m_sPlayerX--; break;
	case 8: m_Camera.MoveDestination(-32, -32); m_pPlayer->m_sPlayerY--; m_pPlayer->m_sPlayerX--; break;
	}

	m_pMapData->ShiftMapData(cDir);
	const char* mapData = reinterpret_cast<const char*>(pData) + sizeof(hb::net::PacketResponsePanningHeader);
	_ReadMapData(sX, sY, mapData);

	m_bIsObserverCommanded = false;
}

/*********************************************************************************************************************
** void CGame::CreateScreenShot()										(snoopy)									**
**  description			:: Fixed Screen Shots																		**
**********************************************************************************************************************/
void CGame::CreateScreenShot()
{
	char longMapName[128] = {};
	GetOfficialMapName(m_cMapName.c_str(), longMapName);

	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);
	std::tm tm{};
#ifdef _WIN32
	localtime_s(&tm, &time);
#else
	localtime_r(&time, &tm);
#endif

	std::string timeStr = std::format("{:02d}:{:02d} - {:02d}:{:02d}:{:02d}",
		tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	hb::shared::text::DrawTextAligned(GameFont::Default, 500, 30, 150, 15, timeStr.c_str(),
		hb::shared::text::TextStyle::Color(GameColors::UIWhite),
		hb::shared::text::Align::TopCenter);

	std::filesystem::create_directory("save");

	for (int i = 0; i < 1000; i++)
	{
		std::string fileName = std::format("save/HelShot{:04d}{:02d}{:02d}_{:02d}{:02d}{:02d}_{}{:03d}.png",
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec,
			longMapName, i);

		if (!std::filesystem::exists(fileName))
		{
			m_Renderer->Screenshot(fileName.c_str());
			AddEventList(std::format(NOTIFYMSG_CREATE_SCREENSHOT1, fileName).c_str(), 10);
			return;
		}
	}
	AddEventList(NOTIFYMSG_CREATE_SCREENSHOT2, 10);
}


void CGame::OnKeyUp(KeyCode _key)
{
	if (HotkeyManager::Get().HandleKeyUp(_key)) {
		return;
	}

	// When an overlay is active, only allow certain hotkeys (like screenshot)
	// Block most hotkeys to prevent interaction with base screen
	if (GameModeManager::GetActiveOverlay() != nullptr)
	{
		// Only allow screenshot hotkey when overlay is visible
		if (_key == KeyCode::F11) {
			Hotkey_Simple_Screenshot();
		}
		return;
	}

	switch (_key) {
	case KeyCode::NumpadAdd:      // Numpad +
		Hotkey_Simple_ZoomIn();
		break;
	case KeyCode::NumpadSubtract: // Numpad -
		Hotkey_Simple_ZoomOut();
		break;

	case KeyCode::F2:
		Hotkey_Simple_UseShortcut2();
		break;

	case KeyCode::F3:
		Hotkey_Simple_UseShortcut3();
		break;

	case KeyCode::Insert:
		Hotkey_Simple_UseHealthPotion();
		break;

	case KeyCode::Delete:
		Hotkey_Simple_UseManaPotion();
		break;

	case KeyCode::End:
		Hotkey_Simple_LoadBackupChat();
		break;

	case KeyCode::F4:
		Hotkey_Simple_UseMagicShortcut();
		break;

	case KeyCode::F5:
		Hotkey_Simple_ToggleCharacterInfo();
		break;

	case KeyCode::F6:
		Hotkey_Simple_ToggleInventory();
		break;

	case KeyCode::F7:
		Hotkey_Simple_ToggleMagic();
		break;

	case KeyCode::F8:
		Hotkey_Simple_ToggleSkill();
		break;

	case KeyCode::F9:
		Hotkey_Simple_ToggleChatHistory();
		break;

	case KeyCode::F12:
		Hotkey_Simple_ToggleSystemMenu();
		break;

	case KeyCode::F1:
		Hotkey_Simple_UseShortcut1();
		break;

	case KeyCode::Up:
		Hotkey_Simple_WhisperCycleUp();
		break;

	case KeyCode::Right:
		Hotkey_Simple_ArrowRight();
		break;

	case KeyCode::Down:
		Hotkey_Simple_WhisperCycleDown();
		break;

	case KeyCode::Left:
		Hotkey_Simple_ArrowLeft();
		break;

	case KeyCode::Tab:
		Hotkey_Simple_TabToggleCombat();
		break;

	case KeyCode::Home:
		Hotkey_Simple_ToggleSafeAttack();
		break;

	case KeyCode::Escape:
		Hotkey_Simple_Escape();
		break;

	case KeyCode::PageUp:
		Hotkey_Simple_SpecialAbility();
		break;

	case KeyCode::F11:
		Hotkey_Simple_Screenshot();
		break;

	default:
		return;
	}
}

void CGame::OnKeyDown(KeyCode _key)
{

	// When an overlay is active, block all OnKeyDown actions
	// Overlays handle their own input in on_update()
	if (GameModeManager::GetActiveOverlay() != nullptr)
	{
		return;
	}

	// Filter out keys that should not trigger any action in OnKeyDown
	// These are handled in OnKeyUp or elsewhere
	switch (_key) {
	case KeyCode::Insert:
	case KeyCode::Delete:
	case KeyCode::Tab:
	case KeyCode::Escape:
	case KeyCode::End:
	case KeyCode::Home:
	case KeyCode::F1:
	case KeyCode::F2:
	case KeyCode::F3:
	case KeyCode::F4:
	case KeyCode::F5:
	case KeyCode::F6:
	case KeyCode::F7:
	case KeyCode::F8:
	case KeyCode::F9:
	case KeyCode::F10:
	case KeyCode::F11:
	case KeyCode::F12:
	case KeyCode::PageUp:
	case KeyCode::PageDown:
	case KeyCode::LWin:
	case KeyCode::RWin:
	case KeyCode::NumpadMultiply:
	case KeyCode::NumpadAdd:
	case KeyCode::NumpadSeparator:
	case KeyCode::NumpadSubtract:
	case KeyCode::NumpadDecimal:
	case KeyCode::NumpadDivide:
	case KeyCode::NumLock:
	case KeyCode::ScrollLock:
		return;
	default:
		break;
	}

	if (GameModeManager::GetMode() == GameMode::MainGame)
	{
		if (hb::shared::input::IsCtrlDown())
		{
			// Ctrl+0-9 for magic views
			switch (_key) {
			case KeyCode::Num0: m_dialogBoxManager.EnableDialogBox(DialogBoxId::Magic, 0, 0, 0); m_dialogBoxManager.Info(DialogBoxId::Magic).sView = 9; break;
			case KeyCode::Num1: m_dialogBoxManager.EnableDialogBox(DialogBoxId::Magic, 0, 0, 0); m_dialogBoxManager.Info(DialogBoxId::Magic).sView = 0; break;
			case KeyCode::Num2: m_dialogBoxManager.EnableDialogBox(DialogBoxId::Magic, 0, 0, 0); m_dialogBoxManager.Info(DialogBoxId::Magic).sView = 1; break;
			case KeyCode::Num3: m_dialogBoxManager.EnableDialogBox(DialogBoxId::Magic, 0, 0, 0); m_dialogBoxManager.Info(DialogBoxId::Magic).sView = 2; break;
			case KeyCode::Num4: m_dialogBoxManager.EnableDialogBox(DialogBoxId::Magic, 0, 0, 0); m_dialogBoxManager.Info(DialogBoxId::Magic).sView = 3; break;
			case KeyCode::Num5: m_dialogBoxManager.EnableDialogBox(DialogBoxId::Magic, 0, 0, 0); m_dialogBoxManager.Info(DialogBoxId::Magic).sView = 4; break;
			case KeyCode::Num6: m_dialogBoxManager.EnableDialogBox(DialogBoxId::Magic, 0, 0, 0); m_dialogBoxManager.Info(DialogBoxId::Magic).sView = 5; break;
			case KeyCode::Num7: m_dialogBoxManager.EnableDialogBox(DialogBoxId::Magic, 0, 0, 0); m_dialogBoxManager.Info(DialogBoxId::Magic).sView = 6; break;
			case KeyCode::Num8: m_dialogBoxManager.EnableDialogBox(DialogBoxId::Magic, 0, 0, 0); m_dialogBoxManager.Info(DialogBoxId::Magic).sView = 7; break;
			case KeyCode::Num9: m_dialogBoxManager.EnableDialogBox(DialogBoxId::Magic, 0, 0, 0); m_dialogBoxManager.Info(DialogBoxId::Magic).sView = 8; break;
			default: break;
			}
		}
		// Only Enter key activates chat input - not every key press
		else if (_key == KeyCode::Enter && (TextInputManager::Get().IsActive() == false) && (!hb::shared::input::IsAltDown()))
		{
			TextInputManager::Get().StartInput(CHAT_INPUT_X(), CHAT_INPUT_Y(), ChatMsgMaxLen, m_cChatMsg);
			TextInputManager::Get().ClearInput();
		}
	}
}

void CGame::ReserveFightzoneResponseHandler(char* pData)
{
	const auto* pkt = hb::net::PacketCast<hb::net::PacketResponseFightzoneReserve>(
		pData, sizeof(hb::net::PacketResponseFightzoneReserve));
	if (!pkt) return;
	switch (pkt->header.msg_type) {
	case MsgType::Confirm:
		AddEventList(RESERVE_FIGHTZONE_RESPONSE_HANDLER1, 10);
		m_dialogBoxManager.Info(DialogBoxId::GuildMenu).cMode = 14;
		m_iFightzoneNumber = m_iFightzoneNumberTemp;
		break;

	case MsgType::Reject:
		AddEventList(RESERVE_FIGHTZONE_RESPONSE_HANDLER2, 10);
		m_iFightzoneNumberTemp = 0;

		if (pkt->result == 0) {
			m_dialogBoxManager.Info(DialogBoxId::GuildMenu).cMode = 15;
		}
		else if (pkt->result == -1) {
			m_dialogBoxManager.Info(DialogBoxId::GuildMenu).cMode = 16;
		}
		else if (pkt->result == -2) {
			m_dialogBoxManager.Info(DialogBoxId::GuildMenu).cMode = 17;
		}
		else if (pkt->result == -3) {
			m_dialogBoxManager.Info(DialogBoxId::GuildMenu).cMode = 21;
		}
		else if (pkt->result == -4) {
			m_dialogBoxManager.Info(DialogBoxId::GuildMenu).cMode = 22;
		}
		break;
	}
}


// UpdateScreen_OnLogResMsg removed - replaced by Overlay_LogResMsg

void CGame::RetrieveItemHandler(char* pData)
{
	std::string cTxt;

	char cBankItemIndex, cItemIndex;
	int j;
	const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(
		pData, sizeof(hb::net::PacketHeader));
	if (!header) return;
	if (header->msg_type != MsgType::Reject)
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketResponseRetrieveItem>(
			pData, sizeof(hb::net::PacketResponseRetrieveItem));
		if (!pkt) return;
		cBankItemIndex = static_cast<char>(pkt->bank_index);
		cItemIndex = static_cast<char>(pkt->item_index);

		if (m_pBankList[cBankItemIndex] != 0) {
			// v1.42
			auto itemInfo = ItemNameFormatter::Get().Format(m_pBankList[cBankItemIndex].get());

			cTxt = std::format(RETIEVE_ITEM_HANDLER4, itemInfo.name.c_str());//""You took out %s."
			AddEventList(cTxt.c_str(), 10);

			CItem* pCfgBank = GetItemConfig(m_pBankList[cBankItemIndex]->m_sIDnum);
			if (pCfgBank && ((pCfgBank->GetItemType() == ItemType::Consume) ||
				(pCfgBank->GetItemType() == ItemType::Arrow)))
			{
				if (m_pItemList[cItemIndex] == 0) goto RIH_STEP2;
				m_pBankList[cBankItemIndex].reset();
				for (j = 0; j <= hb::shared::limits::MaxBankItems - 2; j++)
				{
					if ((m_pBankList[j + 1] != 0) && (m_pBankList[j] == 0))
					{
						m_pBankList[j] = std::move(m_pBankList[j + 1]);
					}
				}
			}
			else
			{
			RIH_STEP2:;
				if (m_pItemList[cItemIndex] != 0) return;
				short nX, nY;
				nX = 40;
				nY = 30;
				for (j = 0; j < hb::shared::limits::MaxItems; j++)
				{
					if ((m_pItemList[j] != 0) && (m_pItemList[j]->m_sIDnum == m_pBankList[cBankItemIndex]->m_sIDnum))
					{
						nX = m_pItemList[j]->m_sX + 1;
						nY = m_pItemList[j]->m_sY + 1;
						break;
					}
				}
				m_pItemList[cItemIndex] = std::move(m_pBankList[cBankItemIndex]);
				m_pItemList[cItemIndex]->m_sX = nX;
				m_pItemList[cItemIndex]->m_sY = nY;
				bSendCommand(MsgId::RequestSetItemPos, 0, cItemIndex, nX, nY, 0, 0);

				for (j = 0; j < hb::shared::limits::MaxItems; j++)
					if (m_cItemOrder[j] == -1)
					{
						m_cItemOrder[j] = cItemIndex;
						break;
					}
				m_bIsItemEquipped[cItemIndex] = false;
				m_bIsItemDisabled[cItemIndex] = false;
				// m_pBankList[cBankItemIndex] already moved above
				for (j = 0; j <= hb::shared::limits::MaxBankItems - 2; j++)
				{
					if ((m_pBankList[j + 1] != 0) && (m_pBankList[j] == 0))
					{
						m_pBankList[j] = std::move(m_pBankList[j + 1]);
					}
				}
			}
		}
	}
	m_dialogBoxManager.Info(DialogBoxId::Bank).cMode = 0;
}

void CGame::DrawNpcName(short screen_x, short screen_y, short owner_type, const hb::shared::entity::PlayerStatus& status, short npc_config_id)
{
	std::string text, text2;

	// Name lookup: prefer config_id (direct), fall back to type-based lookup
	auto npcName = [&]() -> const char* {
		if (npc_config_id >= 0) return GetNpcConfigNameById(npc_config_id);
		return GetNpcConfigName(owner_type);
	};

	// Crop subtypes override the base "Crop" name from config
	if (owner_type == hb::shared::owner::Crops) {
		static const char* cropNames[] = {
			"Crop", "WaterMelon", "Pumpkin", "Garlic", "Barley", "Carrot",
			"Radish", "Corn", "Chinese Bell Flower", "Melone", "Tomato",
			"Grapes", "Blue Grape", "Mushroom", "Ginseng"
		};
		int sub = m_entityState.m_appearance.iSubType;
		if (sub >= 1 && sub <= 14)
			text = cropNames[sub];
		else
			text = npcName();
	}
	// Crusade structure kit suffix
	else if ((owner_type == hb::shared::owner::ArrowGuardTower || owner_type == hb::shared::owner::CannonGuardTower ||
			  owner_type == hb::shared::owner::ManaCollector || owner_type == hb::shared::owner::Detector) &&
			 m_entityState.m_appearance.HasNpcSpecialState()) {
		text = std::format("{} Kit", npcName());
	}
	else {
		text = npcName();
	}
	if (status.bBerserk) text += DRAW_OBJECT_NAME50;//" Berserk"
	if (status.bFrozen) text += DRAW_OBJECT_NAME51;//" Frozen"
	hb::shared::text::DrawText(GameFont::Default, screen_x, screen_y, text.c_str(), hb::shared::text::TextStyle::WithShadow(GameColors::UIWhite));
	if (m_bIsObserverMode == true) hb::shared::text::DrawText(GameFont::Default, screen_x, screen_y + 14, text.c_str(), hb::shared::text::TextStyle::WithShadow(GameColors::NeutralNamePlate));
	else if (m_pPlayer->m_bIsConfusion || (m_iIlusionOwnerH != 0))
	{
		text = DRAW_OBJECT_NAME87;//"(Unknown)"
		hb::shared::text::DrawText(GameFont::Default, screen_x, screen_y + 14, text.c_str(), hb::shared::text::TextStyle::WithShadow(GameColors::UIDisabled)); // v2.171
	}
	else
	{
		if (IsHostile(status.iRelationship))
			hb::shared::text::DrawText(GameFont::Default, screen_x, screen_y + 14, DRAW_OBJECT_NAME90, hb::shared::text::TextStyle::WithShadow(GameColors::UIRed)); // "(Enemy)"
		else if (IsFriendly(status.iRelationship))
			hb::shared::text::DrawText(GameFont::Default, screen_x, screen_y + 14, DRAW_OBJECT_NAME89, hb::shared::text::TextStyle::WithShadow(GameColors::FriendlyNamePlate)); // "(Friendly)"
		else
			hb::shared::text::DrawText(GameFont::Default, screen_x, screen_y + 14, DRAW_OBJECT_NAME88, hb::shared::text::TextStyle::WithShadow(GameColors::NeutralNamePlate)); // "(Neutral)"
	}
	switch (status.iAngelPercent) {
	case 0: break;
	case 1: text2 = DRAW_OBJECT_NAME52; break;//"Clairvoyant"
	case 2: text2 = DRAW_OBJECT_NAME53; break;//"Destruction of Magic Protection"
	case 3: text2 = DRAW_OBJECT_NAME54; break;//"Anti-Physical Damage"
	case 4: text2 = DRAW_OBJECT_NAME55; break;//"Anti-Magic Damage"
	case 5: text2 = DRAW_OBJECT_NAME56; break;//"Poisonous"
	case 6: text2 = DRAW_OBJECT_NAME57; break;//"Critical Poisonous"
	case 7: text2 = DRAW_OBJECT_NAME58; break;//"Explosive"
	case 8: text2 = DRAW_OBJECT_NAME59; break;//"Critical Explosive"
	}
	hb::shared::text::DrawText(GameFont::Default, screen_x, screen_y + 28, text2.c_str(), hb::shared::text::TextStyle::WithShadow(GameColors::MonsterStatusEffect));

	// centu: no muestra la barra de hp de algunos npc
	switch (owner_type) {
	case hb::shared::owner::ShopKeeper:
	case hb::shared::owner::Gandalf:
	case hb::shared::owner::Howard:
	case hb::shared::owner::Tom:
	case hb::shared::owner::William:
	case hb::shared::owner::Kennedy:
	case hb::shared::owner::ManaStone:
	case hb::shared::owner::Bunny:
	case hb::shared::owner::Cat:
	case hb::shared::owner::McGaffin:
	case hb::shared::owner::Perry:
	case hb::shared::owner::Devlin:
	case hb::shared::owner::Crops:
	{
		switch (m_entityState.m_appearance.iSubType) {
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case hb::shared::owner::Slime:
		case hb::shared::owner::Skeleton:
		case hb::shared::owner::StoneGolem:
		case hb::shared::owner::Cyclops:
		case hb::shared::owner::OrcMage:
		default:
			break;
		}
	}
	case hb::shared::owner::Gail:
		break;
	default:
		break;
	}
}

void CGame::DrawObjectName(short screen_x, short screen_y, const char* name, const hb::shared::entity::PlayerStatus& status, uint16_t object_id)
{
	std::string guild_text;
	std::string text, text2;
	uint8_t red = 0, green = 0, blue = 0;
	int guild_index = 0, y_offset = 0;
	bool is_pk = false, is_citizen = false, is_aresden = false, is_hunter = false;
	auto relationship = status.iRelationship;
	if (IsHostile(relationship))
	{
		red = 255; green = 0; blue = 0;
	}
	else if (IsFriendly(relationship))
	{
		red = 30; green = 200; blue = 30;
	}
	else
	{
		red = 50; green = 50; blue = 255;
	}

	if (m_iIlusionOwnerH == 0)
	{
		if (m_bIsCrusadeMode == false) text = name;
		else
		{
			if (!hb::shared::object_id::IsPlayerID(m_entityState.m_wObjectID)) text = GetNpcConfigName(hb::shared::owner::Barbarian);
			else
			{
				if (relationship == EntityRelationship::Enemy) text = std::format("{}", m_entityState.m_wObjectID);
				else text = name;
			}
		}
		if (m_iPartyStatus != 0)
		{
			for (int i = 0; i < hb::shared::limits::MaxPartyMembers; i++)
			{
				if (m_stPartyMemberNameList[i].cName == name)
				{
					text += BGET_NPC_NAME23; // ", Party Member"
					break;
				}
			}
		}
	}
	else text = "?????";

	if (status.bBerserk) text += DRAW_OBJECT_NAME50;//" Berserk"
	if (status.bFrozen) text += DRAW_OBJECT_NAME51;//" Frozen"

	hb::shared::text::DrawText(GameFont::Default, screen_x, screen_y, text.c_str(), hb::shared::text::TextStyle::WithShadow(GameColors::UIWhite));

	if (object_id == m_pPlayer->m_sPlayerObjectID)
	{
		if (m_pPlayer->m_iGuildRank == 0)
		{
			guild_text = std::format(DEF_MSG_GUILDMASTER, m_pPlayer->m_cGuildName);//" Guildmaster)"
			hb::shared::text::DrawText(GameFont::Default, screen_x, screen_y + 14, guild_text.c_str(), hb::shared::text::TextStyle::WithShadow(GameColors::InfoGrayLight));
			y_offset = 14;
		}
		if (m_pPlayer->m_iGuildRank > 0)
		{
			guild_text = std::format(DEF_MSG_GUILDSMAN, m_pPlayer->m_cGuildName);//" Guildsman)"
			hb::shared::text::DrawText(GameFont::Default, screen_x, screen_y + 14, guild_text.c_str(), hb::shared::text::TextStyle::WithShadow(GameColors::InfoGrayLight));
			y_offset = 14;
		}
		if (m_pPlayer->m_iPKCount != 0)
		{
			is_pk = true;
			red = 255; green = 0; blue = 0;
		}
		else
		{
			is_pk = false;
			red = 30; green = 200; blue = 30;
		}
		is_citizen = m_pPlayer->m_bCitizen;
		is_aresden = m_pPlayer->m_bAresden;
		is_hunter = m_pPlayer->m_bHunter;
	}
	else
	{	// CLEROTH - CRASH BUG ( STATUS )
		is_pk = status.bPK;
		is_citizen = status.bCitizen;
		is_aresden = status.bAresden;
		is_hunter = status.bHunter;
		if (m_bIsCrusadeMode == false || !IsHostile(relationship))
		{
			if (FindGuildName(name, &guild_index) == true)
			{
				if (!m_stGuildName[guild_index].cGuildName.empty())
				{
					if (m_stGuildName[guild_index].cGuildName != "NONE")
					{
						if (m_stGuildName[guild_index].iGuildRank == 0)
						{
							guild_text = std::format(DEF_MSG_GUILDMASTER, m_stGuildName[guild_index].cGuildName);//
							hb::shared::text::DrawText(GameFont::Default, screen_x, screen_y + 14, guild_text.c_str(), hb::shared::text::TextStyle::WithShadow(GameColors::InfoGrayLight));
							m_stGuildName[guild_index].dwRefTime = m_dwCurTime;
							y_offset = 14;
						}
						else if (m_stGuildName[guild_index].iGuildRank > 0)
						{
							guild_text = std::format(DEF_MSG_GUILDSMAN, m_stGuildName[guild_index].cGuildName);//"
							hb::shared::text::DrawText(GameFont::Default, screen_x, screen_y + 14, guild_text.c_str(), hb::shared::text::TextStyle::WithShadow(GameColors::InfoGrayLight));
							m_stGuildName[guild_index].dwRefTime = m_dwCurTime;
							y_offset = 14;
						}
					}
					else
					{
						m_stGuildName[guild_index].dwRefTime = 0;
					}
				}
			}
			else bSendCommand(MsgId::CommandCommon, CommonType::ReqGuildName, 0, m_entityState.m_wObjectID, guild_index, 0, 0);
		}
	}

	if (is_citizen == false)	text = DRAW_OBJECT_NAME60;// "Traveller"
	else
	{
		if (is_aresden)
		{
			if (is_hunter == true) text = DEF_MSG_ARECIVIL; // "Aresden Civilian"
			else text = DEF_MSG_ARESOLDIER; // "Aresden Combatant"
		}
		else
		{
			if (is_hunter == true) text = DEF_MSG_ELVCIVIL;// "Elvine Civilian"
			else text = DEF_MSG_ELVSOLDIER;	// "Elvine Combatant"
		}
	}
	if (is_pk == true)
	{
		if (is_citizen == false) text = DEF_MSG_PK;	//"Criminal"
		else
		{
			if (is_aresden) text = DEF_MSG_AREPK;// "Aresden Criminal"
			else text = DEF_MSG_ELVPK;  // "Elvine Criminal"
		}
	}
	hb::shared::text::DrawText(GameFont::Default, screen_x, screen_y + 14 + y_offset, text.c_str(), hb::shared::text::TextStyle::WithShadow(hb::shared::render::Color(red, green, blue)));
}

bool CGame::FindGuildName(const char* pName, int* ipIndex)
{
	int i, iRet = 0;
	uint32_t dwTmpTime;
	for (i = 0; i < game_limits::max_guild_names; i++)
	{
		if (memcmp(m_stGuildName[i].cCharName.c_str(), pName, 10) == 0)
		{
			m_stGuildName[i].dwRefTime = m_dwCurTime;
			*ipIndex = i;
			return true;
		}
	}
	dwTmpTime = m_stGuildName[0].dwRefTime;
	for (i = 0; i < game_limits::max_guild_names; i++)
	{
		if (m_stGuildName[i].dwRefTime < dwTmpTime)
		{
			iRet = i;
			dwTmpTime = m_stGuildName[i].dwRefTime;
		}
	}
	m_stGuildName[iRet].cCharName.assign(pName, strnlen(pName, 10));
	m_stGuildName[iRet].dwRefTime = m_dwCurTime;
	m_stGuildName[iRet].iGuildRank = -1;
	*ipIndex = iRet;
	return false;
}

void CGame::DrawVersion()
{
	std::string G_cTxt;
	G_cTxt = std::format("Ver: {}", hb::version::GetDisplayString());
	hb::shared::text::DrawText(GameFont::Default, 12 , (LOGICAL_HEIGHT() - 12 - 14) , G_cTxt.c_str(), hb::shared::text::TextStyle::WithShadow(GameColors::UIDisabled));
}

char CGame::GetOfficialMapName(const char* pMapName, char* pName)
{	// MapIndex
	if (strcmp(pMapName, "middleland") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME28);	// Middleland
		return 4;
	}
	else if (strcmp(pMapName, "huntzone3") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME31);	// Death Valley
		return 0;
	}
	else if (strcmp(pMapName, "huntzone1") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME29);	// Rocky Highland
		return 1;
	}
	else if (strcmp(pMapName, "elvuni") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME57);	// Eldiniel Garden
		return 2;
	}
	else if (strcmp(pMapName, "elvine") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME24);	// Elvine City
		return 3;
	}
	else if (strcmp(pMapName, "elvfarm") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME2);	// Elvine Farm
		return 5;
	}
	else if (strcmp(pMapName, "arefarm") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME1);	// Aresden Farm
		return 6;
	}
	else if (strcmp(pMapName, "default") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME3);	// Beginner Zone
		return 7;
	}
	else if (strcmp(pMapName, "huntzone4") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME32);	// Silent Wood
		return 8;
	}
	else if (strcmp(pMapName, "huntzone2") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME30);	// Eternal Field
		return 9;
	}
	else if (strcmp(pMapName, "areuni") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME56);	// Aresien Garden
		return 10;
	}
	else if (strcmp(pMapName, "aresden") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME22);	// Aresden City
		return 11;
	}
	else if (strcmp(pMapName, "dglv2") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME25);	// Dungeon L2
		return 12;
	}
	else if (strcmp(pMapName, "dglv3") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME26);	// Dungeon L3
		return 13;
	}
	else if (strcmp(pMapName, "dglv4") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME53);	// Dungeon L4
		return 14;
	}
	else if (strcmp(pMapName, "elvined1") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME23);	// Elvine Dungeon
		return 15;
	}
	else if (strcmp(pMapName, "aresdend1") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME21);	// Aresden Dungeon
		return 16;
	}
	else if (strcmp(pMapName, "bisle") == 0) {
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME27);	// Bleeding Island
		return 17;
	}
	else if (strcmp(pMapName, "toh1") == 0) {
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME60);	// Tower of Hell 1
		return 18;
	}
	else if (strcmp(pMapName, "toh2") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME61);	// Tower of Hell 2
		return 19;
	}
	else if (strcmp(pMapName, "toh3") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME62);	// Tower of Hell 3
		return 20;
	}
	else if (strcmp(pMapName, "middled1x") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME58);	// Middleland Mine
		return 21;
	}
	else if (strcmp(pMapName, "middled1n") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME59);	// Middleland Dungeon
		return 22;
	}
	else if (strcmp(pMapName, "2ndmiddle") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME65);	// Promiseland
		return 23;
	}
	else if (strcmp(pMapName, "icebound") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME66);	// Ice Map
		return 24;
		// Snoopy:
	}
	else if (strcmp(pMapName, "druncncity") == 0) // Snoopy: Apocalypse maps
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME70);
		return 25;
	}
	else if (strcmp(pMapName, "inferniaA") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME71);
		return 26;
	}
	else if (strcmp(pMapName, "inferniaB") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME72);
		return 27;
	}
	else if (strcmp(pMapName, "maze") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME73);
		return 28;
	}
	else if (strcmp(pMapName, "procella") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME74);
		return 29;
	}
	else if (strcmp(pMapName, "abaddon") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME75);
		return 30;
	}
	else if (strcmp(pMapName, "BtField") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME76);
		return 35;
	}
	else if (strcmp(pMapName, "GodH") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME77);
		return 36;
	}
	else if (strcmp(pMapName, "HRampart") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME78);
		return 37;
	}
	else if (strcmp(pMapName, "cityhall_1") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME35);	// Aresden Cityhall
		return -1;
	}
	else if (strcmp(pMapName, "cityhall_2") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME36);	// Elvine Cityhall
		return -1;
	}
	else if (strcmp(pMapName, "gldhall_1") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME37);	// Aresden Guildhall
		return -1;
	}
	else if (strcmp(pMapName, "gldhall_2") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME38);	// Elvine Guildhall
		return -1;
	}
	else if (strcmp(pMapName, "bsmith_1") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME33);	// Aresden Blacksmith
		return -1;
	}
	else if (strcmp(pMapName, "bsmith_2") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME34);	// Elvine Blacksmith
		return -1;
	}
	else if (strcmp(pMapName, "gshop_1") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME39);	// Aresden Shop
		return -1;
	}
	else if (strcmp(pMapName, "gshop_2") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME40);	// Elvine Shop
		return -1;
	}
	else if (strcmp(pMapName, "wrhus_1") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME43);	// Aresden Warehouse
		return -1;
	}
	else if (strcmp(pMapName, "wrhus_2") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME44);	// Elvine Warehouse
		return -1;
	}
	else if (strcmp(pMapName, "arewrhus") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME45);	// Aresden Warehouse
		return -1;
	}
	else if (strcmp(pMapName, "elvwrhus") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME46);	// Elvine Warehouse
		return -1;
	}
	else if (strcmp(pMapName, "wzdtwr_1") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME41);	// Magic Tower
		return -1;
	}
	else if (strcmp(pMapName, "wzdtwr_2") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME42);	// Magic Tower
		return -1;
	}
	else if (strcmp(pMapName, "cath_1") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME47);	// Aresien Church
		return -1;
	}
	else if (strcmp(pMapName, "cath_2") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME48);	// Eldiniel Church
		return -1;
	}
	else if (strcmp(pMapName, "resurr1") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME54);	// Revival Zone
		return -1;
	}
	else if (strcmp(pMapName, "resurr2") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME55);	// Revival Zone
		return -1;
	}
	else if (strcmp(pMapName, "arebrk11") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME4);	// Aresden Barrack 1
		return -1;
	}
	else if (strcmp(pMapName, "arebrk12") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME5);	// Aresden Barrack 1
		return -1;
	}
	else if (strcmp(pMapName, "arebrk21") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME6);	// Aresden Barrack 2
		return -1;
	}
	else if (strcmp(pMapName, "arebrk22") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME7);	// Aresden Barrack 2
		return -1;
	}
	else if (strcmp(pMapName, "elvbrk11") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME8);	// Elvine Barrack 1
		return -1;
	}
	else if (strcmp(pMapName, "elvbrk12") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME9);	// Elvine Barrack 1
		return -1;
	}
	else if (strcmp(pMapName, "elvbrk21") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME10);	// Elvine Barrack 2
		return -1;
	}
	else if (strcmp(pMapName, "elvbrk22") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME11);	// Elvine Barrack 2
		return -1;
	}
	else if (strcmp(pMapName, "fightzone1") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME12);	// Arena 1
		return -1;
	}
	else if (strcmp(pMapName, "fightzone2") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME13);	// Arena 2
		return -1;
	}
	else if (strcmp(pMapName, "fightzone3") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME14);	// Arena 3
		return -1;
	}
	else if (strcmp(pMapName, "fightzone4") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME15);	// Arena 4
		return -1;
	}
	else if (strcmp(pMapName, "fightzone5") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME16);	// Arena 5
		return -1;
	}
	else if (strcmp(pMapName, "fightzone6") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME17);	// Arena 6
		return -1;
	}
	else if (strcmp(pMapName, "fightzone7") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME18);	// Arena 7
		return -1;
	}
	else if (strcmp(pMapName, "fightzone8") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME19);	// Arena 8
		return -1;
	}
	else if (strcmp(pMapName, "fightzone9") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME20);	// Arena 9
		return -1;
	}
	else if (strcmp(pMapName, "arejail") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME63);	// Aresden Jail
		return -1;
	}
	else if (strcmp(pMapName, "elvjail") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME64);	// Elvine Jail
		return -1;
	}
	else if (strcmp(pMapName, "CmdHall_1") == 0) // Snoopy: Commander Halls
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME79);
		return -1;
	}
	else if (strcmp(pMapName, "CmdHall_2") == 0)
	{
		std::snprintf(pName, 21, "%s", GET_OFFICIAL_MAP_NAME79);
		return -1;
	}
	else
	{
		std::snprintf(pName, 21, "%s", pMapName);
		return -1;
	}
}

bool CGame::bCheckLocalChatCommand(const char* pMsg)
{
	return ChatCommandManager::Get().ProcessCommand(pMsg);
}

void CGame::ClearSkillUsingStatus()
{
	if (m_bSkillUsingStatus == true)
	{
		AddEventList(CLEAR_SKILL_USING_STATUS1, 10);//"
		m_dialogBoxManager.DisableDialogBox(DialogBoxId::Fishing);
		m_dialogBoxManager.DisableDialogBox(DialogBoxId::Manufacture);
		if ((m_pPlayer->m_sPlayerType >= 1) && (m_pPlayer->m_sPlayerType <= 6)/* && (!m_pPlayer->m_playerAppearance.bIsWalking)*/) {
			m_pPlayer->m_Controller.SetCommand(Type::Stop);
			m_pPlayer->m_Controller.SetDestination(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY);
		}
	}
	m_bSkillUsingStatus = false;
}

void CGame::NpcTalkHandler(char* packet_data)
{
	std::string text;

	char reward_name[hb::shared::limits::ItemNameLen]{};
	char target_name[21]{};
	char temp[21]{};

	const auto* pkt = hb::net::PacketCast<hb::net::PacketNotifyNpcTalk>(
		packet_data, sizeof(hb::net::PacketNotifyNpcTalk));
	if (!pkt) return;

	short npc_type = pkt->type;
	short response = pkt->response;
	int amount = pkt->amount;
	int contribution = pkt->contribution;
	int target_type = pkt->target_type;
	int target_count = pkt->target_count;
	int target_x = pkt->x;
	int target_y = pkt->y;
	int target_range = pkt->range;
	int question_type = 0;
	int index = 0;

	std::memset(reward_name, 0, sizeof(reward_name));
	memcpy(reward_name, pkt->reward_name, hb::shared::limits::ItemNameLen - 1);
	memcpy(target_name, pkt->target_name, 20);

	m_dialogBoxManager.EnableDialogBox(DialogBoxId::NpcTalk, response, npc_type, 0);

	if ((npc_type >= 1) && (npc_type <= 100))
	{
		index = m_dialogBoxManager.Info(DialogBoxId::NpcTalk).sV1;
		m_pMsgTextList2[index] = std::make_unique<CMsg>(0, "  ", 0);
		index++;
		question_type = 0;
		switch (npc_type) {
		case 1: //Monster Hunt
			std::snprintf(temp, hb::shared::limits::NpcNameLen, "%s", GetNpcConfigName(target_type));
			text = std::format(NPC_TALK_HANDLER16, target_count, temp);
			m_pMsgTextList2[index] = std::make_unique<CMsg>(0, text.c_str(), 0);
			index++;

			if (memcmp(target_name, "NONE", 4) == 0) {
				text = NPC_TALK_HANDLER17;//"
				m_pMsgTextList2[index] = std::make_unique<CMsg>(0, text.c_str(), 0);
				index++;
			}
			else {
				std::memset(temp, 0, sizeof(temp));
				GetOfficialMapName(target_name, temp);
				text = std::format(NPC_TALK_HANDLER18, temp);//"Map : %s"
				m_pMsgTextList2[index] = std::make_unique<CMsg>(0, text.c_str(), 0);
				index++;

				if (target_x != 0) {
					text = std::format(NPC_TALK_HANDLER19, target_x, target_y, target_range);//"Position: %d,%d within %d blocks"
					m_pMsgTextList2[index] = std::make_unique<CMsg>(0, text.c_str(), 0);
					index++;
				}

				text = std::format(NPC_TALK_HANDLER20, contribution);//"
				m_pMsgTextList2[index] = std::make_unique<CMsg>(0, text.c_str(), 0);
				index++;
			}
			question_type = 1;
			break;

		case 7: //
			m_pMsgTextList2[index] = std::make_unique<CMsg>(0, NPC_TALK_HANDLER21, 0);
			index++;

			if (memcmp(target_name, "NONE", 4) == 0) {
				text = NPC_TALK_HANDLER22;
				m_pMsgTextList2[index] = std::make_unique<CMsg>(0, text.c_str(), 0);
				index++;
			}
			else {
				std::memset(temp, 0, sizeof(temp));
				GetOfficialMapName(target_name, temp);
				text = std::format(NPC_TALK_HANDLER23, temp);
				m_pMsgTextList2[index] = std::make_unique<CMsg>(0, text.c_str(), 0);
				index++;

				if (target_x != 0) {
					text = std::format(NPC_TALK_HANDLER24, target_x, target_y, target_range);
					m_pMsgTextList2[index] = std::make_unique<CMsg>(0, text.c_str(), 0);
					index++;
				}

				text = std::format(NPC_TALK_HANDLER25, contribution);
				m_pMsgTextList2[index] = std::make_unique<CMsg>(0, text.c_str(), 0);
				index++;
			}
			question_type = 1;
			break;

		case hb::shared::owner::Slime: // Crusade
			m_pMsgTextList2[index] = std::make_unique<CMsg>(0, NPC_TALK_HANDLER26, 0);
			index++;

			text = NPC_TALK_HANDLER27;//"
			m_pMsgTextList2[index] = std::make_unique<CMsg>(0, text.c_str(), 0);
			index++;

			text = NPC_TALK_HANDLER28;//"
			m_pMsgTextList2[index] = std::make_unique<CMsg>(0, text.c_str(), 0);
			index++;

			text = NPC_TALK_HANDLER29;//"
			m_pMsgTextList2[index] = std::make_unique<CMsg>(0, text.c_str(), 0);
			index++;

			text = NPC_TALK_HANDLER30;//"
			m_pMsgTextList2[index] = std::make_unique<CMsg>(0, text.c_str(), 0);
			index++;

			text = " ";
			m_pMsgTextList2[index] = std::make_unique<CMsg>(0, text.c_str(), 0);
			index++;

			if (memcmp(target_name, "NONE", 4) == 0) {
				text = NPC_TALK_HANDLER31;//"
				m_pMsgTextList2[index] = std::make_unique<CMsg>(0, text.c_str(), 0);
				index++;
			}
			else {
				std::memset(temp, 0, sizeof(temp));
				GetOfficialMapName(target_name, temp);
				text = std::format(NPC_TALK_HANDLER32, temp);//"
				m_pMsgTextList2[index] = std::make_unique<CMsg>(0, text.c_str(), 0);
				index++;
			}
			question_type = 2;
			break;
		}

		switch (question_type) {
		case 1:
			m_pMsgTextList2[index] = std::make_unique<CMsg>(0, "  ", 0);
			index++;
			m_pMsgTextList2[index] = std::make_unique<CMsg>(0, NPC_TALK_HANDLER33, 0);//"
			index++;
			m_pMsgTextList2[index] = std::make_unique<CMsg>(0, NPC_TALK_HANDLER34, 0);//"
			index++;
			m_pMsgTextList2[index] = std::make_unique<CMsg>(0, "  ", 0);
			index++;
			break;

		case 2:
			m_pMsgTextList2[index] = std::make_unique<CMsg>(0, "  ", 0);
			index++;
			m_pMsgTextList2[index] = std::make_unique<CMsg>(0, NPC_TALK_HANDLER35, 0);//"
			index++;
			m_pMsgTextList2[index] = std::make_unique<CMsg>(0, "  ", 0);
			index++;
			break;

		default: break;
		}
	}
}

void CGame::PointCommandHandler(int indexX, int indexY, char cItemID)
{
	char cTemp[31];
	if ((m_iPointCommandType >= 100) && (m_iPointCommandType < 200))
	{
		// Get target object ID for auto-aim (lag compensation)
		// If player clicked on an entity, send its ID so server can track its current position
		int targetObjectID = 0;
		const FocusedObject& focused = CursorTarget::GetFocusedObject();
		if (focused.valid && (focused.type == FocusedObjectType::Player || focused.type == FocusedObjectType::NPC))
		{
			targetObjectID = focused.objectID;
		}
		bSendCommand(MsgId::CommandCommon, CommonType::Magic, 0, indexX, indexY, m_iPointCommandType, nullptr, targetObjectID);
	}
	else if ((m_iPointCommandType >= 0) && (m_iPointCommandType < 50))
	{
		bSendCommand(MsgId::CommandCommon, CommonType::ReqUseItem, 0, m_iPointCommandType, indexX, indexY, cTemp, cItemID); // v1.4

		CItem* pCfgPt = GetItemConfig(m_pItemList[m_iPointCommandType]->m_sIDnum);
		if (pCfgPt && pCfgPt->GetItemType() == ItemType::UseSkill)
			m_bSkillUsingStatus = true;
	}
	else if (m_iPointCommandType == 200) // Normal Hand
	{
		if ((m_cMCName.size() == 0) || (m_cMCName == m_pPlayer->m_cPlayerName) || (m_cMCName[0] == '_'))
		{
			m_dialogBoxManager.Info(DialogBoxId::Party).cMode = 0;
			PlayGameSound('E', 14, 5);
			AddEventList(POINT_COMMAND_HANDLER1, 10);
		}
		else
		{
			m_dialogBoxManager.Info(DialogBoxId::Party).cMode = 3;
			PlayGameSound('E', 14, 5);
			std::snprintf(m_dialogBoxManager.Info(DialogBoxId::Party).cStr, sizeof(m_dialogBoxManager.Info(DialogBoxId::Party).cStr), "%s", m_cMCName.c_str());
			bSendCommand(MsgId::CommandCommon, CommonType::RequestJoinParty, 0, 1, 0, 0, m_cMCName.c_str());
			return;
		}
	}
}


void CGame::StartBGM()
{
	// Determine track name based on current location
	const char* trackName = "MainTm";

	if ((m_bIsXmas == true) && (WeatherManager::Get().IsSnowing()))
	{
		trackName = "Carol";
	}
	else if (m_cCurLocation.starts_with("aresden"))
	{
		trackName = "aresden";
	}
	else if (m_cCurLocation.starts_with("elvine"))
	{
		trackName = "elvine";
	}
	else if (m_cCurLocation.starts_with("dglv"))
	{
		trackName = "dungeon";
	}
	else if (m_cCurLocation.starts_with("middled1"))
	{
		trackName = "dungeon";
	}
	else if (m_cCurLocation.starts_with("middleland"))
	{
		trackName = "middleland";
	}
	else if (m_cCurLocation.starts_with("druncncity"))
	{
		trackName = "druncncity";
	}
	else if (m_cCurLocation.starts_with("inferniaA"))
	{
		trackName = "middleland";
	}
	else if (m_cCurLocation.starts_with("inferniaB"))
	{
		trackName = "middleland";
	}
	else if (m_cCurLocation.starts_with("maze"))
	{
		trackName = "dungeon";
	}
	else if (m_cCurLocation.starts_with("abaddon"))
	{
		trackName = "abaddon";
	}

	// Forward to AudioManager
	AudioManager::Get().PlayMusic(trackName);
}

void CGame::MotionResponseHandler(char* packet_data)
{
	WORD response = 0;
	short map_x = 0, map_y = 0;
	char direction = 0;
	int previous_hp = 0;
	//						          0 3        4 5						 6 7		8 9		   10	    11
	// Confirm Code(4) | MsgSize(4) | MsgID(4) | Confirm::MoveConfirm(2) | Loc-X(2) | Loc-Y(2) | Dir(1) | MapData ...
	// Confirm Code(4) | MsgSize(4) | MsgID(4) | Confirm::MoveReject(2)  | Loc-X(2) | Loc-Y(2)
	const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(
		packet_data, sizeof(hb::net::PacketHeader));
	if (!header) return;
	response = header->msg_type;

	switch (response) {
	case Confirm::MotionConfirm:
		m_pPlayer->m_Controller.DecrementCommandCount();
		break;

	case Confirm::MotionAttackConfirm:
		m_pPlayer->m_Controller.DecrementCommandCount();
		break;

	case Confirm::MotionReject:
		if (m_pPlayer->m_iHP <= 0) return;
		{
			const auto* pkt = hb::net::PacketCast<hb::net::PacketResponseMotionReject>(
				packet_data, sizeof(hb::net::PacketResponseMotionReject));
			if (!pkt) return;
			m_pPlayer->m_sPlayerX = pkt->x;
			m_pPlayer->m_sPlayerY = pkt->y;
		}

		m_pPlayer->m_Controller.SetCommand(Type::Stop);
		m_pPlayer->m_Controller.SetDestination(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY);

		m_pMapData->bSetOwner(m_pPlayer->m_sPlayerObjectID, m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pPlayer->m_sPlayerType, m_pPlayer->m_iPlayerDir,
			m_pPlayer->m_playerAppearance,
			m_pPlayer->m_playerStatus, m_pPlayer->m_cPlayerName,
			Type::Stop, 0, 0, 0);
		m_pPlayer->m_Controller.ResetCommandCount();
		m_bIsGetPointingMode = false;
		m_Camera.SnapTo((m_pPlayer->m_sPlayerX - VIEW_CENTER_TILE_X()) * 32 - 16, (m_pPlayer->m_sPlayerY - VIEW_CENTER_TILE_Y()) * 32 - 16);
		break;

	case Confirm::MoveConfirm:
	{
		const auto* pkt = hb::net::PacketCast<hb::net::PacketResponseMotionMoveConfirm>(
			packet_data, sizeof(hb::net::PacketResponseMotionMoveConfirm));
		if (!pkt) return;
		map_x = pkt->x;
		map_y = pkt->y;
		direction = static_cast<char>(pkt->dir);
		m_pPlayer->m_iSP = m_pPlayer->m_iSP - pkt->stamina_cost;
		if (m_pPlayer->m_iSP < 0) m_pPlayer->m_iSP = 0;
		previous_hp = m_pPlayer->m_iHP;
		m_pPlayer->m_iHP = pkt->hp;

		if (m_pPlayer->m_iHP != previous_hp)
		{
			std::string G_cTxt;
			if (m_pPlayer->m_iHP < previous_hp)
			{
				G_cTxt = std::format(NOTIFYMSG_HP_DOWN, previous_hp - m_pPlayer->m_iHP);
				AddEventList(G_cTxt.c_str(), 10);
				m_dwDamagedTime = GameClock::GetTimeMS();
				if ((m_cLogOutCount > 0) && (m_bForceDisconn == false))
				{
					m_cLogOutCount = -1;
					AddEventList(MOTION_RESPONSE_HANDLER2, 10);
				}
			}
			else
			{
				G_cTxt = std::format(NOTIFYMSG_HP_UP, m_pPlayer->m_iHP - previous_hp);
				AddEventList(G_cTxt.c_str(), 10);
			}
		}
		m_pMapData->ShiftMapData(direction);
		const char* mapData = reinterpret_cast<const char*>(packet_data) + sizeof(hb::net::PacketResponseMotionMoveConfirm);
		_ReadMapData(map_x, map_y, mapData);
		m_pPlayer->m_Controller.DecrementCommandCount();
	}
	break;

	case Confirm::MoveReject:
		if (m_pPlayer->m_iHP <= 0) return;
		{
			const auto* pkt = hb::net::PacketCast<hb::net::PacketResponseMotionMoveReject>(
				packet_data, sizeof(hb::net::PacketResponseMotionMoveReject));
			if (!pkt) return;
			if (m_pPlayer->m_sPlayerObjectID != pkt->object_id) return;
			m_pPlayer->m_sPlayerX = pkt->x;
			m_pPlayer->m_sPlayerY = pkt->y;
			m_pPlayer->m_sPlayerType = pkt->type;
			m_pPlayer->m_iPlayerDir = static_cast<char>(pkt->dir);
			m_pPlayer->m_playerAppearance = pkt->appearance;
			m_pPlayer->m_playerStatus = pkt->status;
			m_pPlayer->m_bIsGMMode = m_pPlayer->m_playerStatus.bGMMode;
		}
		m_pPlayer->m_Controller.SetCommand(Type::Stop);
		m_pPlayer->m_Controller.SetDestination(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY);
		m_pMapData->bSetOwner(m_pPlayer->m_sPlayerObjectID, m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pPlayer->m_sPlayerType, m_pPlayer->m_iPlayerDir,
			m_pPlayer->m_playerAppearance, // v1.4
			m_pPlayer->m_playerStatus, m_pPlayer->m_cPlayerName,
			Type::Stop, 0, 0, 0,
			0, 7);
		m_pPlayer->m_Controller.ResetCommandCount();
		m_bIsGetPointingMode = false;
		m_Camera.SnapTo((m_pPlayer->m_sPlayerX - VIEW_CENTER_TILE_X()) * 32 - 16, (m_pPlayer->m_sPlayerY - VIEW_CENTER_TILE_Y()) * 32 - 16);
		m_pPlayer->m_Controller.SetPrevMoveBlocked(true);
		switch (m_pPlayer->m_sPlayerType) {
		case 1:
		case 2:
		case 3:
			PlayGameSound('C', 12, 0);
			break;
		case 4:
		case 5:
		case 6:
			PlayGameSound('C', 13, 0);
			break;
		}
		break;
	}
}

void CGame::CommandProcessor(short mouse_x, short mouse_y, short tile_x, short tile_y, char left_button, char right_button)
{
	uint32_t current_time = GameClock::GetTimeMS();
	uint16_t action_type = 0;
	int result = 0;
	short dialog_x = 0, dialog_y = 0;
	char direction = 0;

	// Fixed by Snoopy
	if ((m_bIsObserverCommanded == false) && (m_bIsObserverMode == true))
	{
		if ((mouse_x == 0) && (mouse_y == 0) && (m_Camera.GetDestinationX() > 32 * VIEW_TILE_WIDTH()) && (m_Camera.GetDestinationY() > 32 * VIEW_TILE_HEIGHT()))
			bSendCommand(MsgId::RequestPanning, 0, 8, 0, 0, 0, 0);
		else if ((mouse_x == LOGICAL_MAX_X()) && (mouse_y == 0) && (m_Camera.GetDestinationX() < 32 * m_pMapData->m_sMapSizeX - 32 * VIEW_TILE_WIDTH()) && (m_Camera.GetDestinationY() > 32 * VIEW_TILE_HEIGHT()))
			bSendCommand(MsgId::RequestPanning, 0, 2, 0, 0, 0, 0);
		else if ((mouse_x == LOGICAL_MAX_X()) && (mouse_y == LOGICAL_MAX_Y()) && (m_Camera.GetDestinationX() < 32 * m_pMapData->m_sMapSizeX - 32 * VIEW_TILE_WIDTH()) && (m_Camera.GetDestinationY() < 32 * m_pMapData->m_sMapSizeY - 32 * VIEW_TILE_HEIGHT()))
			bSendCommand(MsgId::RequestPanning, 0, 4, 0, 0, 0, 0);
		else if ((mouse_x == 0) && (mouse_y == LOGICAL_MAX_Y()))
			bSendCommand(MsgId::RequestPanning, 0, 6, 0, 0, 0, 0);
		else if ((mouse_x == 0) && (m_Camera.GetDestinationX() > 32 * VIEW_TILE_WIDTH()))
			bSendCommand(MsgId::RequestPanning, 0, 7, 0, 0, 0, 0);
		else if ((mouse_x == LOGICAL_MAX_X()) && (m_Camera.GetDestinationX() < 32 * m_pMapData->m_sMapSizeX - 32 * VIEW_TILE_WIDTH()))
			bSendCommand(MsgId::RequestPanning, 0, 3, 0, 0, 0, 0);
		else if ((mouse_y == 0) && (m_Camera.GetDestinationY() > 32 * VIEW_TILE_HEIGHT()))
			bSendCommand(MsgId::RequestPanning, 0, 1, 0, 0, 0, 0);
		else if ((mouse_y == LOGICAL_MAX_Y()) && (m_Camera.GetDestinationY() < 32 * m_pMapData->m_sMapSizeY - 32 * VIEW_TILE_HEIGHT()))
			bSendCommand(MsgId::RequestPanning, 0, 5, 0, 0, 0, 0);
		else return;

		m_bIsObserverCommanded = true;
		m_cArrowPressed = 0;
		return;
	}

	if (m_bIsObserverMode == true) return;

	if (hb::shared::input::IsAltDown()) // [ALT]
		m_pPlayer->m_bSuperAttackMode = true;
	else m_pPlayer->m_bSuperAttackMode = false;

	switch (CursorTarget::GetCursorStatus()) {
	case CursorStatus::Null:
		if (left_button != 0)
		{
			result = m_dialogBoxManager.HandleMouseDown(mouse_x, mouse_y);
			if (result == 1)
			{
				CursorTarget::SetCursorStatus(CursorStatus::Selected);
				return;
			}
			else if (result == 0)
			{
				CursorTarget::SetCursorStatus(CursorStatus::Pressed);
				// Snoopy: Added Golden LevelUp
				if ((mouse_x > LEVELUP_TEXT_X()) && (mouse_x < (LEVELUP_TEXT_X())+75) && (mouse_y > LEVELUP_TEXT_Y()) && (mouse_y < (LEVELUP_TEXT_Y())+21))
				{
					if (m_pPlayer->m_iHP > 0)
					{
						if ((m_dialogBoxManager.IsEnabled(DialogBoxId::LevelUpSetting) != true) && (m_pPlayer->m_iLU_Point > 0))
						{
							m_dialogBoxManager.EnableDialogBox(DialogBoxId::LevelUpSetting, 0, 0, 0);
							PlayGameSound('E', 14, 5);
						}
					}
					else // Centuu : restart
					{
						if (m_cRestartCount == -1)
						{
							std::string G_cTxt;
							m_cRestartCount = 5;
							m_dwRestartCountTime = GameClock::GetTimeMS();
							G_cTxt = std::format(DLGBOX_CLICK_SYSMENU1, m_cRestartCount); // "Restarting game....%d"
							AddEventList(G_cTxt.c_str(), 10);
							PlayGameSound('E', 14, 5);

						}
					}
					return;
				}
			}
			else if (result == -1)
			{
				// Scroll/slider claimed - set status to prevent re-processing
				CursorTarget::SetCursorStatus(CursorStatus::Selected);
				return;
			}
		}
		else if (right_button != 0)
		{
			if (m_dialogBoxManager.HandleRightClick(mouse_x, mouse_y, current_time)) return;
		}
		break;
	case CursorStatus::Pressed:
		if (left_button == 0) // Normal Click
		{
			CursorTarget::SetCursorStatus(CursorStatus::Null);
		}
		break;
		case CursorStatus::Selected:
		if (left_button == 0)
		{
			CursorTarget::SetCursorStatus(CursorStatus::Null);
			//ZeroEoyPnk - Bye delay...
			if (((m_dialogBoxManager.IsEnabled(DialogBoxId::LevelUpSetting) != true) || (CursorTarget::GetSelectedID() != 12))
				&& ((m_dialogBoxManager.IsEnabled(DialogBoxId::ChangeStatsMajestic) != true) || (CursorTarget::GetSelectedID() != 42)))
			{
				if (((current_time - CursorTarget::GetSelectionClickTime()) < input_config::double_click_time_ms) 	// Double Click
					&& (abs(mouse_x - CursorTarget::GetSelectionClickX()) <= input_config::double_click_tolerance)
					&& (abs(mouse_y - CursorTarget::GetSelectionClickY()) <= input_config::double_click_tolerance))
				{
					CursorTarget::ResetSelectionClickTime(); // Reset to prevent triple-click
					m_dialogBoxManager.HandleDoubleClick(mouse_x, mouse_y);
				}
				else // Click
				{
					m_dialogBoxManager.HandleClick(mouse_x, mouse_y);
				}
			}
			else
			{
				m_dialogBoxManager.HandleClick(mouse_x, mouse_y);
			}
			CursorTarget::RecordSelectionClick(mouse_x, mouse_y, current_time);
			if (CursorTarget::GetSelectedType() == SelectedObjectType::Item)
			{
				if (!m_dialogBoxManager.HandleDraggingItemRelease(mouse_x, mouse_y))
				{
					bItemDrop_ExternalScreen(static_cast<char>(CursorTarget::GetSelectedID()), mouse_x, mouse_y);
				}
			}
			// Always clear selection after click-release to prevent stale state
			CursorTarget::ClearSelection();
			return;
		}
		else 			// v2.05 01-11-30
		{
			if ((m_pMapData->bIsTeleportLoc(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY) == true) && (m_pPlayer->m_Controller.GetCommandCount() == 0)) break;

			if ((CursorTarget::GetPrevX() != mouse_x) || (CursorTarget::GetPrevY() != mouse_y))
			{
				CursorTarget::SetCursorStatus(CursorStatus::Dragging);
				CursorTarget::SetPrevPosition(mouse_x, mouse_y);
				if ((CursorTarget::GetSelectedType() == SelectedObjectType::DialogBox) &&
					(CursorTarget::GetSelectedID() == 30))
				{
				}

				if ((CursorTarget::GetSelectedType() == SelectedObjectType::DialogBox) &&
					(CursorTarget::GetSelectedID() == 7) && (m_dialogBoxManager.Info(DialogBoxId::GuildMenu).cMode == 1))
				{
					TextInputManager::Get().EndInput();
					m_dialogBoxManager.Info(DialogBoxId::GuildMenu).cMode = 20;
				}
				// Query Drop Item Amount
				if ((CursorTarget::GetSelectedType() == SelectedObjectType::DialogBox) &&
					(CursorTarget::GetSelectedID() == 17) && (m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).cMode == 1))
				{
					TextInputManager::Get().EndInput();
					m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).cMode = 20;
				}
				return;
			}
			if ((m_pPlayer->m_Controller.GetCommand() == Type::Move) || (m_pPlayer->m_Controller.GetCommand() == Type::Run)) { ProcessMotionCommands(action_type); return; }
			return;
		}
		break;
	case CursorStatus::Dragging:
		if (left_button != 0)
		{
			if ((m_pMapData->bIsTeleportLoc(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY) == true) && (m_pPlayer->m_Controller.GetCommandCount() == 0)) break;
			if (CursorTarget::GetSelectedType() == SelectedObjectType::DialogBox)
			{
				// HudPanel is fixed and cannot be moved
				if (CursorTarget::GetSelectedID() != DialogBoxId::HudPanel)
				{
					m_dialogBoxManager.Info(CursorTarget::GetSelectedID()).sX = mouse_x - CursorTarget::GetDragDistX();
					m_dialogBoxManager.Info(CursorTarget::GetSelectedID()).sY = mouse_y - CursorTarget::GetDragDistY();
				}
			}
			CursorTarget::SetPrevPosition(mouse_x, mouse_y);

			if ((m_pPlayer->m_Controller.GetCommand() == Type::Move) || (m_pPlayer->m_Controller.GetCommand() == Type::Run)) { ProcessMotionCommands(action_type); return; }
			return;
		}
		if (left_button == 0) {
			CursorTarget::SetCursorStatus(CursorStatus::Null);
			switch (CursorTarget::GetSelectedType()) {
			case SelectedObjectType::DialogBox:
				if ((CursorTarget::GetSelectedType() == SelectedObjectType::DialogBox) &&
					(CursorTarget::GetSelectedID() == 7) && (m_dialogBoxManager.Info(DialogBoxId::GuildMenu).cMode == 20))
				{
					dialog_x = m_dialogBoxManager.Info(DialogBoxId::GuildMenu).sX;
					dialog_y = m_dialogBoxManager.Info(DialogBoxId::GuildMenu).sY;
					TextInputManager::Get().StartInput(dialog_x + 75, dialog_y + 140, 21, m_pPlayer->m_cGuildName);
					m_dialogBoxManager.Info(DialogBoxId::GuildMenu).cMode = 1;
				}

				if ((CursorTarget::GetSelectedType() == SelectedObjectType::DialogBox) &&
					(CursorTarget::GetSelectedID() == 17) && (m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).cMode == 20))
				{	// Query Drop Item Amount
					dialog_x = m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).sX;
					dialog_y = m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).sY;
					TextInputManager::Get().StartInput(dialog_x + 40, dialog_y + 57, AmountStringMaxLen, m_cAmountString);
					m_dialogBoxManager.Info(DialogBoxId::ItemDropExternal).cMode = 1;
				}

				if (CursorTarget::GetSelectedID() == 9)
				{
					{
						if (mouse_x < 400) //LifeX Fix Map
						{
							m_dialogBoxManager.Info(DialogBoxId::GuideMap).sX = 0;
						}
						else
						{
							m_dialogBoxManager.Info(DialogBoxId::GuideMap).sX = LOGICAL_MAX_X() - m_dialogBoxManager.Info(DialogBoxId::GuideMap).sSizeX;
						}

						if (mouse_y < 273)
						{
							m_dialogBoxManager.Info(DialogBoxId::GuideMap).sY = 0;
						}
						else
						{
							m_dialogBoxManager.Info(DialogBoxId::GuideMap).sY = 547 - m_dialogBoxManager.Info(DialogBoxId::GuideMap).sSizeY;
						}
					}
				}

				CursorTarget::ClearSelection();
				break;

			case SelectedObjectType::Item:
				if (!m_dialogBoxManager.HandleDraggingItemRelease(mouse_x, mouse_y))
				{
					bItemDrop_ExternalScreen(static_cast<char>(CursorTarget::GetSelectedID()), mouse_x, mouse_y);
				}
				CursorTarget::ClearSelection();
				break;

			default:
				CursorTarget::ClearSelection();
				break;
			}
			return;
		}
		break;
	}

	// Allow clicks to be responsive even if command not yet available
	if (m_pPlayer->m_Controller.IsCommandAvailable() == false)
	{
		char cmd = m_pPlayer->m_Controller.GetCommand();
		if (ConfigManager::Get().IsQuickActionsEnabled() && (cmd == Type::Move || cmd == Type::Run))
		{
			if (left_button != 0)
			{
				// Click on self while moving = pickup (interrupt movement)
				if (m_cMCName == m_pPlayer->m_cPlayerName)
				{
					if ((m_pPlayer->m_sPlayerType >= 1) && (m_pPlayer->m_sPlayerType <= 6))
					{
						m_pPlayer->m_Controller.SetCommand(Type::GetItem);
						m_pPlayer->m_Controller.SetDestination(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY);
						return;
					}
				}
				// Left click while moving: update destination immediately
				m_pPlayer->m_Controller.SetDestination(tile_x, tile_y);
			}
			else if (right_button != 0)
			{
				// Right click on self while moving = pickup (interrupt movement)
				if (m_cMCName == m_pPlayer->m_cPlayerName)
				{
					if ((m_pPlayer->m_sPlayerType >= 1) && (m_pPlayer->m_sPlayerType <= 6))
					{
						m_pPlayer->m_Controller.SetCommand(Type::GetItem);
						m_pPlayer->m_Controller.SetDestination(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY);
						return;
					}
				}
				// Right click while moving: stop after current step and face click direction
				m_pPlayer->m_Controller.SetDestination(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY);
				// Save pending direction to apply when movement stops
				char pendingDir = CMisc::cGetNextMoveDir(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, tile_x, tile_y);
				if (pendingDir != 0) m_pPlayer->m_Controller.SetPendingStopDir(pendingDir);
			}
		}
		else if (ConfigManager::Get().IsQuickActionsEnabled() && right_button != 0 && cmd == Type::Stop && !m_bIsGetPointingMode)
		{
			// Right click while stopped (and not casting): process turn immediately
			// But don't interrupt attack/magic animations — the controller command is STOP
			// after dispatch, but the tile animation may still be playing
			int dXc = m_pPlayer->m_sPlayerX - m_pMapData->m_sPivotX;
			int dYc = m_pPlayer->m_sPlayerY - m_pMapData->m_sPivotY;
			if (dXc >= 0 && dXc < MapDataSizeX && dYc >= 0 && dYc < MapDataSizeY) {
				int8_t animAction = m_pMapData->m_pData[dXc][dYc].m_animation.cAction;
				if (animAction == Type::Attack || animAction == Type::AttackMove || animAction == Type::Magic)
					return;
			}
			direction = CMisc::cGetNextMoveDir(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, tile_x, tile_y);
			if (direction != 0 && m_pPlayer->m_iPlayerDir != direction)
			{
				m_pPlayer->m_iPlayerDir = direction;
				bSendCommand(MsgId::CommandMotion, Type::Stop, direction, 0, 0, 0, 0);
				m_pMapData->bSetOwner(m_pPlayer->m_sPlayerObjectID, m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY,
					m_pPlayer->m_sPlayerType, direction, m_pPlayer->m_playerAppearance,
					m_pPlayer->m_playerStatus, m_pPlayer->m_cPlayerName, Type::Stop, 0, 0, 0, 0, 10);
				m_pPlayer->m_Controller.SetCommandTime(current_time);
			}
		}
		return;
	}
	if ((current_time - m_pPlayer->m_Controller.GetCommandTime()) < 300)
	{
		m_pGSock.reset();
		m_pGSock.reset();
		PlayGameSound('E', 14, 5);
		AudioManager::Get().StopSound(SoundType::Effect, 38);
		AudioManager::Get().StopMusic();
		ChangeGameMode(GameMode::MainMenu);
		return;
	}
	if (m_pPlayer->m_iHP <= 0) return;

	if (m_pPlayer->m_sDamageMove != 0)
	{
		m_pPlayer->m_Controller.SetCommand(Type::DamageMove);
		ProcessMotionCommands(action_type); return;
	}

	if ((m_pMapData->bIsTeleportLoc(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY) == true) && (m_pPlayer->m_Controller.GetCommandCount() == 0))
		RequestTeleportAndWaitData();

	// indexX, indexY

	if (left_button != 0)
	{
		if (ProcessLeftClick(mouse_x, mouse_y, tile_x, tile_y, current_time, action_type)) return;
	}
	else if (right_button != 0)
	{
		if (ProcessRightClick(mouse_x, mouse_y, tile_x, tile_y, current_time, action_type)) return;
	}

	ProcessMotionCommands(action_type);
}

bool CGame::ProcessLeftClick(short mouse_x, short mouse_y, short tile_x, short tile_y, uint32_t current_time, uint16_t& action_type)
{
	std::string name;
	short object_type = 0, dialog_x = 0, dialog_y = 0;
	hb::shared::entity::PlayerStatus object_status;
	char abs_x = 0, abs_y = 0;

		if (m_bIsGetPointingMode == true)
		{
			if ((m_sMCX != 0) || (m_sMCY != 0))
				PointCommandHandler(m_sMCX, m_sMCY);
			else PointCommandHandler(tile_x, tile_y);

			m_pPlayer->m_Controller.SetCommandAvailable(false);
			m_pPlayer->m_Controller.SetCommandTime(GameClock::GetTimeMS());
			m_bIsGetPointingMode = false;
			m_dwMagicCastTime = current_time;  // Track when magic was cast
			return true;
		}

		// Delay after magic cast before allowing held-click actions
		if (m_dwMagicCastTime > 0 && (current_time - m_dwMagicCastTime) < 750) return true;

		m_pMapData->bGetOwner(m_sMCX, m_sMCY - 1, name, &object_type, &object_status, &m_wCommObjectID); // v1.4
		if (m_cMCName == m_pPlayer->m_cPlayerName && (object_type <= 6 || (m_pMapData->m_pData[m_pPlayer->m_sPlayerX - m_pMapData->m_sPivotX][m_pPlayer->m_sPlayerY - m_pMapData->m_sPivotY].m_sItemID != 0 && m_pItemConfigList[m_pMapData->m_pData[m_pPlayer->m_sPlayerX - m_pMapData->m_sPivotX][m_pPlayer->m_sPlayerY - m_pMapData->m_sPivotY].m_sItemID]->m_sSprite != 0)))
		{
			if ((m_pPlayer->m_sPlayerType >= 1) && (m_pPlayer->m_sPlayerType <= 6)/* && (!m_pPlayer->m_playerAppearance.bIsWalking)*/)
			{
				m_pPlayer->m_Controller.SetCommand(Type::GetItem);
				m_pPlayer->m_Controller.SetDestination(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY);
			}
		}
		else
		{
			if (m_cMCName == m_pPlayer->m_cPlayerName) m_sMCY -= 1;
			if ((m_sMCX != 0) && (m_sMCY != 0)) // m_sMCX, m_sMCY
			{
				if (hb::shared::input::IsCtrlDown() == true)
				{
					m_pMapData->bGetOwner(m_sMCX, m_sMCY, name, &object_type, &object_status, &m_wCommObjectID);
					if (object_status.bInvisibility) return true;
					if ((object_type == 15) || (object_type == 20) || (object_type == 24)) return true;
					abs_x = abs(m_pPlayer->m_sPlayerX - m_sMCX);
					abs_y = abs(m_pPlayer->m_sPlayerY - m_sMCY);
					if ((abs_x <= 1) && (abs_y <= 1))
					{
						action_type = CombatSystem::Get().GetAttackType();
						m_pPlayer->m_Controller.SetCommand(Type::Attack);
						m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
					}
					else if ((abs_x <= 2) && (abs_y <= 2) // strike on Big mobs & gate from a range
						&& ((object_type == 66) || (object_type == 73) || (object_type == 81) || (object_type == 91)))
					{
						action_type = CombatSystem::Get().GetAttackType();
						m_pPlayer->m_Controller.SetCommand(Type::Attack);
						m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
					}
					else // Pas au corp � corp
					{
						switch (CombatSystem::Get().GetWeaponSkillType()) {
						case 6: // Bow
							m_pPlayer->m_Controller.SetCommand(Type::Attack);
							m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
							action_type = CombatSystem::Get().GetAttackType();
							break;

						case 5: // OpenHand
						case 7: // SS
							if (((abs_x == 2) && (abs_y == 2)) || ((abs_x == 0) && (abs_y == 2)) || ((abs_x == 2) && (abs_y == 0)))
							{
								if ((hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0))
								{
									if (m_pPlayer->m_iSkillMastery[CombatSystem::Get().GetWeaponSkillType()] == 100)
									{
										m_pPlayer->m_Controller.SetCommand(Type::AttackMove);
										action_type = CombatSystem::Get().GetAttackType();
									}
									else
									{
										m_pPlayer->m_Controller.SetCommand(Type::Run);
										m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
									}
									m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
								}
								else
								{
									m_pPlayer->m_Controller.SetCommand(Type::Move);
									m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
									m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
								}
							}
							else
							{
								if ((hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0)
									&& (m_pPlayer->m_sPlayerType >= 1) && (m_pPlayer->m_sPlayerType <= 6))
									m_pPlayer->m_Controller.SetCommand(Type::Run);	// Staminar
								else m_pPlayer->m_Controller.SetCommand(Type::Move);
								m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
								m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
							}
							break;

						case 8: // LS
							if ((abs_x <= 3) && (abs_y <= 3) && CombatSystem::Get().CanSuperAttack()
								&& (CombatSystem::Get().GetAttackType() != 30)) // Crit without StormBlade
							{
								action_type = CombatSystem::Get().GetAttackType();
								m_pPlayer->m_Controller.SetCommand(Type::Attack);
								m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
							}
							else if ((abs_x <= 5) && (abs_y <= 5) && CombatSystem::Get().CanSuperAttack()
								&& (CombatSystem::Get().GetAttackType() == 30))  // Crit with StormBlade (by Snoopy)
							{
								action_type = CombatSystem::Get().GetAttackType();
								m_pPlayer->m_Controller.SetCommand(Type::Attack);
								m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
							}
							else if ((abs_x <= 3) && (abs_y <= 3)
								&& (CombatSystem::Get().GetAttackType() == 5))  // Normal hit with StormBlade (by Snoopy)
							{
								action_type = CombatSystem::Get().GetAttackType();
								m_pPlayer->m_Controller.SetCommand(Type::Attack);
								m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
							}
							else // Swing
							{
								if (((abs_x == 2) && (abs_y == 2)) || ((abs_x == 0) && (abs_y == 2)) || ((abs_x == 2) && (abs_y == 0))
									&& (CombatSystem::Get().GetAttackType() != 5)) // no Dash possible with StormBlade
								{
									if ((hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0))
									{
										if (m_pPlayer->m_iSkillMastery[CombatSystem::Get().GetWeaponSkillType()] == 100)
										{
											m_pPlayer->m_Controller.SetCommand(Type::AttackMove);
											action_type = CombatSystem::Get().GetAttackType();
										}
										else
										{
											m_pPlayer->m_Controller.SetCommand(Type::Run);
											m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
										}
										m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
									}
									else
									{
										m_pPlayer->m_Controller.SetCommand(Type::Move);
										m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
										m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
									}
								}
								else
								{
									if ((hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0)
										&& (m_pPlayer->m_sPlayerType >= 1) && (m_pPlayer->m_sPlayerType <= 6))
										m_pPlayer->m_Controller.SetCommand(Type::Run);
									else m_pPlayer->m_Controller.SetCommand(Type::Move);
									m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
									m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
								}
							}
							break;

						case 9: // Fencing
							if ((abs_x <= 4) && (abs_y <= 4) && CombatSystem::Get().CanSuperAttack())
							{
								m_pPlayer->m_Controller.SetCommand(Type::Attack);
								m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
								action_type = CombatSystem::Get().GetAttackType();
							}
							else {
								if (((abs_x == 2) && (abs_y == 2)) || ((abs_x == 0) && (abs_y == 2)) || ((abs_x == 2) && (abs_y == 0))) {
									if ((hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0)) {
										if (m_pPlayer->m_iSkillMastery[CombatSystem::Get().GetWeaponSkillType()] == 100) {
											m_pPlayer->m_Controller.SetCommand(Type::AttackMove);
											action_type = CombatSystem::Get().GetAttackType();
										}
										else {
											m_pPlayer->m_Controller.SetCommand(Type::Run);
											m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
										}
										m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
									}
									else {
										m_pPlayer->m_Controller.SetCommand(Type::Move);
										m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
										m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
									}
								}
								else {
									if ((hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0) &&
										(m_pPlayer->m_sPlayerType >= 1) && (m_pPlayer->m_sPlayerType <= 6))
										m_pPlayer->m_Controller.SetCommand(Type::Run);
									else m_pPlayer->m_Controller.SetCommand(Type::Move);
									m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
									m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
								}
							}
							break;

						case hb::shared::owner::Slime: // Axe
							if ((abs_x <= 2) && (abs_y <= 2) && CombatSystem::Get().CanSuperAttack())
							{
								m_pPlayer->m_Controller.SetCommand(Type::Attack);
								m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
								action_type = CombatSystem::Get().GetAttackType();
							}
							else
							{
								if (((abs_x == 2) && (abs_y == 2)) || ((abs_x == 0) && (abs_y == 2)) || ((abs_x == 2) && (abs_y == 0)))
								{
									if ((hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0))
									{
										if (m_pPlayer->m_iSkillMastery[CombatSystem::Get().GetWeaponSkillType()] == 100)
										{
											m_pPlayer->m_Controller.SetCommand(Type::AttackMove);
											action_type = CombatSystem::Get().GetAttackType();
										}
										else
										{
											m_pPlayer->m_Controller.SetCommand(Type::Run);
											m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
										}
										m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
									}
									else
									{
										m_pPlayer->m_Controller.SetCommand(Type::Move);
										m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
										m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
									}
								}
								else
								{
									if ((hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0) &&
										(m_pPlayer->m_sPlayerType >= 1) && (m_pPlayer->m_sPlayerType <= 6))
										m_pPlayer->m_Controller.SetCommand(Type::Run);
									else m_pPlayer->m_Controller.SetCommand(Type::Move);
									m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
									m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
								}
							}
							break;
						case hb::shared::owner::OrcMage: // Hammer
							if ((abs_x <= 2) && (abs_y <= 2) && CombatSystem::Get().CanSuperAttack()) {
								m_pPlayer->m_Controller.SetCommand(Type::Attack);
								m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
								action_type = CombatSystem::Get().GetAttackType();
							}
							else {
								if (((abs_x == 2) && (abs_y == 2)) || ((abs_x == 0) && (abs_y == 2)) || ((abs_x == 2) && (abs_y == 0))) {
									if ((hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0)) {
										if (m_pPlayer->m_iSkillMastery[CombatSystem::Get().GetWeaponSkillType()] == 100) {
											m_pPlayer->m_Controller.SetCommand(Type::AttackMove);
											action_type = CombatSystem::Get().GetAttackType();
										}
										else {
											m_pPlayer->m_Controller.SetCommand(Type::Run);
											m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
										}
										m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
									}
									else {
										m_pPlayer->m_Controller.SetCommand(Type::Move);
										m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
										m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
									}
								}
								else {
									if ((hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0) &&
										(m_pPlayer->m_sPlayerType >= 1) && (m_pPlayer->m_sPlayerType <= 6))
										m_pPlayer->m_Controller.SetCommand(Type::Run);
									else m_pPlayer->m_Controller.SetCommand(Type::Move);
									m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
									m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
								}
							}
							break;
						case hb::shared::owner::Guard: // Wand
							if ((abs_x <= 2) && (abs_y <= 2) && CombatSystem::Get().CanSuperAttack()) {
								m_pPlayer->m_Controller.SetCommand(Type::Attack);
								m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
								action_type = CombatSystem::Get().GetAttackType();
							}
							else {
								if (((abs_x == 2) && (abs_y == 2)) || ((abs_x == 0) && (abs_y == 2)) || ((abs_x == 2) && (abs_y == 0))) {
									if ((hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0)) {
										if (m_pPlayer->m_iSkillMastery[CombatSystem::Get().GetWeaponSkillType()] == 100) {
											m_pPlayer->m_Controller.SetCommand(Type::AttackMove);
											action_type = CombatSystem::Get().GetAttackType();
										}
										else {
											m_pPlayer->m_Controller.SetCommand(Type::Run);
											m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
										}
										m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
									}
									else {
										m_pPlayer->m_Controller.SetCommand(Type::Move);
										m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
										m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
									}
								}
								else {
									if ((hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0) &&
										(m_pPlayer->m_sPlayerType >= 1) && (m_pPlayer->m_sPlayerType <= 6))
										m_pPlayer->m_Controller.SetCommand(Type::Run);
									else m_pPlayer->m_Controller.SetCommand(Type::Move);
									m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
									m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
								}
							}
							break;
						}
					}
				}
				else // CTRL not pressed
				{
					m_pMapData->bGetOwner(m_sMCX, m_sMCY, name, &object_type, &object_status, &m_wCommObjectID);
					if (object_type >= 10 || ((object_type >= 1) && (object_type <= 6)))
					{
						switch (object_type) { 	// CLEROTH - NPC TALK
						case hb::shared::owner::ShopKeeper: // ShopKeeper-W�
							m_dialogBoxManager.EnableDialogBox(DialogBoxId::NpcActionQuery, 5, 11, 1);
							dialog_x = mouse_x - 117;
							dialog_y = mouse_y - 50;
							if (dialog_x < 0) dialog_x = 0;
							if ((dialog_x + 235) > LOGICAL_MAX_X()) dialog_x = LOGICAL_MAX_X() - 235;
							if (dialog_y < 0) dialog_y = 0;
							if ((dialog_y + 100) > LOGICAL_MAX_Y()) dialog_y = LOGICAL_MAX_Y() - 100;
							m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sX = dialog_x;
							m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sY = dialog_y;
							m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sV3 = 15;
							break;

						case hb::shared::owner::Gandalf: // Gandlf
							m_dialogBoxManager.EnableDialogBox(DialogBoxId::NpcActionQuery, 0, 16, 0);
							dialog_x = mouse_x - 117;
							dialog_y = mouse_y - 50;
							if (dialog_x < 0) dialog_x = 0;
							if ((dialog_x + 235) > LOGICAL_MAX_X()) dialog_x = LOGICAL_MAX_X() - 235;
							if (dialog_y < 0) dialog_y = 0;
							if ((dialog_y + 100) > LOGICAL_MAX_Y()) dialog_y = LOGICAL_MAX_Y() - 100;
							m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sX = dialog_x;
							m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sY = dialog_y;
							m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sV3 = 19;
							break;

						case hb::shared::owner::Howard: // Howard
							m_dialogBoxManager.EnableDialogBox(DialogBoxId::NpcActionQuery, 0, 14, 0);
							dialog_x = mouse_x - 117;
							dialog_y = mouse_y - 50;
							if (dialog_x < 0) dialog_x = 0;
							if ((dialog_x + 235) > LOGICAL_MAX_X()) dialog_x = LOGICAL_MAX_X() - 235;
							if (dialog_y < 0) dialog_y = 0;
							if ((dialog_y + 100) > LOGICAL_MAX_Y()) dialog_y = LOGICAL_MAX_Y() - 100;
							m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sX = dialog_x;
							m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sY = dialog_y;
							m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sV3 = 20;
							m_dialogBoxManager.Info(DialogBoxId::GiveItem).sV3 = 20;
							m_dialogBoxManager.Info(DialogBoxId::GiveItem).sV4 = m_wCommObjectID;
							m_dialogBoxManager.Info(DialogBoxId::GiveItem).sV5 = m_sMCX;
							m_dialogBoxManager.Info(DialogBoxId::GiveItem).sV6 = m_sMCY;
							break;

						case hb::shared::owner::Tom: // Tom
							m_dialogBoxManager.EnableDialogBox(DialogBoxId::NpcActionQuery, 5, 11, 2);
							dialog_x = mouse_x - 117;
							dialog_y = mouse_y - 50;
							if (dialog_x < 0) dialog_x = 0;
							if ((dialog_x + 235) > LOGICAL_MAX_X()) dialog_x = LOGICAL_MAX_X() - 235;
							if (dialog_y < 0) dialog_y = 0;
							if ((dialog_y + 100) > LOGICAL_MAX_Y()) dialog_y = LOGICAL_MAX_Y() - 100;
							m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sX = dialog_x;
							m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sY = dialog_y;
							m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sV3 = 24;
							m_dialogBoxManager.Info(DialogBoxId::GiveItem).sV3 = 24;
							m_dialogBoxManager.Info(DialogBoxId::GiveItem).sV4 = m_wCommObjectID;
							m_dialogBoxManager.Info(DialogBoxId::GiveItem).sV5 = m_sMCX;
							m_dialogBoxManager.Info(DialogBoxId::GiveItem).sV6 = m_sMCY;
							break;

						case hb::shared::owner::William: // William
							m_dialogBoxManager.EnableDialogBox(DialogBoxId::NpcActionQuery, 0, 13, 0);
							dialog_x = mouse_x - 117;
							dialog_y = mouse_y - 50;
							if (dialog_x < 0) dialog_x = 0;
							if ((dialog_x + 235) > LOGICAL_MAX_X()) dialog_x = LOGICAL_MAX_X() - 235;
							if (dialog_y < 0) dialog_y = 0;
							if ((dialog_y + 100) > LOGICAL_MAX_Y()) dialog_y = LOGICAL_MAX_Y() - 100;
							m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sX = dialog_x;
							m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sY = dialog_y;
							m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sV3 = 25;
							break;

						case hb::shared::owner::Kennedy: // Kennedy
							m_dialogBoxManager.EnableDialogBox(DialogBoxId::NpcActionQuery, 0, 7, 0);
							dialog_x = mouse_x - 117;
							dialog_y = mouse_y - 50;
							if (dialog_x < 0) dialog_x = 0;
							if ((dialog_x + 235) > LOGICAL_MAX_X()) dialog_x = LOGICAL_MAX_X() - 235;
							if (dialog_y < 0) dialog_y = 0;
							if ((dialog_y + 100) > LOGICAL_MAX_Y()) dialog_y = LOGICAL_MAX_Y() - 100;
							m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sX = dialog_x;
							m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sY = dialog_y;
							m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sV3 = 26;
							break;

						case hb::shared::owner::Guard: // Guard
							if (!IsHostile(object_status.iRelationship) && (!m_pPlayer->m_bIsCombatMode))
							{
								m_dialogBoxManager.EnableDialogBox(DialogBoxId::NpcActionQuery, 4, 0, 0);
								dialog_x = mouse_x - 117;
								dialog_y = mouse_y - 50;
								if (dialog_x < 0) dialog_x = 0;
								if ((dialog_x + 235) > LOGICAL_MAX_X()) dialog_x = LOGICAL_MAX_X() - 235;
								if (dialog_y < 0) dialog_y = 0;
								if ((dialog_y + 100) > LOGICAL_MAX_Y()) dialog_y = LOGICAL_MAX_Y() - 100;
								m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sX = dialog_x;
								m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sY = dialog_y;
								m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sV3 = 21;
							}
							break;
						case hb::shared::owner::McGaffin: // McGaffin
						case hb::shared::owner::Perry: // Perry
						case hb::shared::owner::Devlin: // Devlin
							if (!m_pPlayer->m_bIsCombatMode)
							{
								m_dialogBoxManager.EnableDialogBox(DialogBoxId::NpcActionQuery, 4, 0, 0);
								dialog_x = mouse_x - 117;
								dialog_y = mouse_y - 50;
								if (dialog_x < 0) dialog_x = 0;
								if ((dialog_x + 235) > LOGICAL_MAX_X()) dialog_x = LOGICAL_MAX_X() - 235;
								if (dialog_y < 0) dialog_y = 0;
								if ((dialog_y + 100) > LOGICAL_MAX_Y()) dialog_y = LOGICAL_MAX_Y() - 100;
								m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sX = dialog_x;
								m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sY = dialog_y;
								m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sV3 = object_type;
							}
							break;

						case hb::shared::owner::Unicorn: // Unicorn
							if (!m_pPlayer->m_bIsCombatMode)
							{
								m_dialogBoxManager.EnableDialogBox(DialogBoxId::NpcActionQuery, 4, 0, 0);
								dialog_x = mouse_x - 117;
								dialog_y = mouse_y - 50;
								if (dialog_x < 0) dialog_x = 0;
								if ((dialog_x + 235) > LOGICAL_MAX_X()) dialog_x = LOGICAL_MAX_X() - 235;
								if (dialog_y < 0) dialog_y = 0;
								if ((dialog_y + 100) > LOGICAL_MAX_Y()) dialog_y = LOGICAL_MAX_Y() - 100;
								m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sX = dialog_x;
								m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sY = dialog_y;
								m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sV3 = 32;
							}
							break;

						case hb::shared::owner::Gail: // Snoopy: Gail
							m_dialogBoxManager.EnableDialogBox(DialogBoxId::NpcActionQuery, 6, 0, 0);
							dialog_x = mouse_x - 117;
							dialog_y = mouse_y - 50;
							if (dialog_x < 0) dialog_x = 0;
							if ((dialog_x + 235) > LOGICAL_MAX_X()) dialog_x = LOGICAL_MAX_X() - 235;
							if (dialog_y < 0) dialog_y = 0;
							if ((dialog_y + 100) > LOGICAL_MAX_Y()) dialog_y = LOGICAL_MAX_Y() - 100;
							m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sX = dialog_x;
							m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sY = dialog_y;
							m_dialogBoxManager.Info(DialogBoxId::NpcActionQuery).sV3 = 90;
							break;

						default: // Other mobs
							if (!IsHostile(object_status.iRelationship)) break;
							if ((object_type >= 1) && (object_type <= 6) && (m_pPlayer->m_bForceAttack == false)) break;
							abs_x = abs(m_pPlayer->m_sPlayerX - m_sMCX);
							abs_y = abs(m_pPlayer->m_sPlayerY - m_sMCY);
							if ((abs_x <= 1) && (abs_y <= 1))
							{
								action_type = CombatSystem::Get().GetAttackType();
								m_pPlayer->m_Controller.SetCommand(Type::Attack);
								m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
							}
							else if ((abs_x <= 2) && (abs_y <= 2) // strike on Big mobs & gate from a range
								&& ((object_type == 66) || (object_type == 73) || (object_type == 81) || (object_type == 91)))
							{
								action_type = CombatSystem::Get().GetAttackType();
								m_pPlayer->m_Controller.SetCommand(Type::Attack);
								m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
							}
							else // Normal hit from a range.
							{
								switch (CombatSystem::Get().GetWeaponSkillType()) {
								case 6: // Bow
									m_pPlayer->m_Controller.SetCommand(Type::Attack);
									m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
									action_type = CombatSystem::Get().GetAttackType();
									break;

								case 5: // Boxe
								case 7: // SS
									if ((hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0)
										&& (m_pPlayer->m_sPlayerType >= 1) && (m_pPlayer->m_sPlayerType <= 6))
										m_pPlayer->m_Controller.SetCommand(Type::Run);
									else m_pPlayer->m_Controller.SetCommand(Type::Move);
									m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
									m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
									break;

								case 8: // LS
									if ((abs_x <= 3) && (abs_y <= 3) && CombatSystem::Get().CanSuperAttack()
										&& (CombatSystem::Get().GetAttackType() != 30)) // Crit without StormBlade by Snoopy
									{
										if ((abs_x <= 1) && (abs_y <= 1) && (hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0))
											m_pPlayer->m_Controller.SetCommand(Type::AttackMove);
										else m_pPlayer->m_Controller.SetCommand(Type::Attack);
										m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
										action_type = CombatSystem::Get().GetAttackType();
									}
									else if ((abs_x <= 5) && (abs_y <= 5) && CombatSystem::Get().CanSuperAttack()
										&& (CombatSystem::Get().GetAttackType() == 30)) // Crit with StormBlade by Snoopy
									{
										if ((abs_x <= 1) && (abs_y <= 1) && (hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0))
											m_pPlayer->m_Controller.SetCommand(Type::AttackMove);
										else m_pPlayer->m_Controller.SetCommand(Type::Attack);
										m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
										action_type = CombatSystem::Get().GetAttackType();
									}
									else if ((abs_x <= 3) && (abs_y <= 3)
										&& (CombatSystem::Get().GetAttackType() == 5)) // Normal hit with StormBlade by Snoopy
									{
										m_pPlayer->m_Controller.SetCommand(Type::Attack);
										m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
										action_type = CombatSystem::Get().GetAttackType();
									}
									else
									{
										if ((hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0) &&
											(m_pPlayer->m_sPlayerType >= 1) && (m_pPlayer->m_sPlayerType <= 6))
											m_pPlayer->m_Controller.SetCommand(Type::Run);
										else m_pPlayer->m_Controller.SetCommand(Type::Move);
										m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
										m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
									}
									break;

								case 9: // Fencing
									if ((abs_x <= 4) && (abs_y <= 4) && CombatSystem::Get().CanSuperAttack())
									{
										if ((abs_x <= 1) && (abs_y <= 1) && (hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0))
											m_pPlayer->m_Controller.SetCommand(Type::AttackMove);
										else m_pPlayer->m_Controller.SetCommand(Type::Attack);
										m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
										action_type = CombatSystem::Get().GetAttackType();
									}
									else
									{
										if ((hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0) &&
											(m_pPlayer->m_sPlayerType >= 1) && (m_pPlayer->m_sPlayerType <= 6))
											m_pPlayer->m_Controller.SetCommand(Type::Run);
										else m_pPlayer->m_Controller.SetCommand(Type::Move);
										m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
										m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
									}
									break;

								case hb::shared::owner::Slime: //
									if ((abs_x <= 2) && (abs_y <= 2) && CombatSystem::Get().CanSuperAttack()) {
										if ((abs_x <= 1) && (abs_y <= 1) && (hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0))
											m_pPlayer->m_Controller.SetCommand(Type::AttackMove);
										else m_pPlayer->m_Controller.SetCommand(Type::Attack);
										m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
										action_type = CombatSystem::Get().GetAttackType();
									}
									else {
										if ((hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0) &&
											(m_pPlayer->m_sPlayerType >= 1) && (m_pPlayer->m_sPlayerType <= 6))
											m_pPlayer->m_Controller.SetCommand(Type::Run);
										else m_pPlayer->m_Controller.SetCommand(Type::Move);
										m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
										m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
									}
									break;
								case hb::shared::owner::OrcMage: //
									if ((abs_x <= 2) && (abs_y <= 2) && CombatSystem::Get().CanSuperAttack()) {
										if ((abs_x <= 1) && (abs_y <= 1) && (hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0))
											m_pPlayer->m_Controller.SetCommand(Type::AttackMove);
										else m_pPlayer->m_Controller.SetCommand(Type::Attack);
										m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
										action_type = CombatSystem::Get().GetAttackType();
									}
									else {
										if ((hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0) &&
											(m_pPlayer->m_sPlayerType >= 1) && (m_pPlayer->m_sPlayerType <= 6))
											m_pPlayer->m_Controller.SetCommand(Type::Run);
										else m_pPlayer->m_Controller.SetCommand(Type::Move);
										m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
										m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
									}
									break;
								case hb::shared::owner::Guard: //
									if ((abs_x <= 2) && (abs_y <= 2) && CombatSystem::Get().CanSuperAttack()) {
										if ((abs_x <= 1) && (abs_y <= 1) && (hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0))
											m_pPlayer->m_Controller.SetCommand(Type::AttackMove);
										else m_pPlayer->m_Controller.SetCommand(Type::Attack);
										m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
										action_type = CombatSystem::Get().GetAttackType();
									}
									else {
										if ((hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0) &&
											(m_pPlayer->m_sPlayerType >= 1) && (m_pPlayer->m_sPlayerType <= 6))
											m_pPlayer->m_Controller.SetCommand(Type::Run);
										else m_pPlayer->m_Controller.SetCommand(Type::Move);
										m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
										m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
									}
									break;
								}
							}
							break;
						}
					}
					else {
						if ((hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0) &&
							(m_pPlayer->m_sPlayerType >= 1) && (m_pPlayer->m_sPlayerType <= 6))
							m_pPlayer->m_Controller.SetCommand(Type::Run);
						else m_pPlayer->m_Controller.SetCommand(Type::Move);
						m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
						m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
					}
				}
			}
			else
			{
				if ((hb::shared::input::IsShiftDown() || ConfigManager::Get().IsRunningModeEnabled()) && (m_pPlayer->m_iSP > 0) &&
					(m_pPlayer->m_sPlayerType >= 1) && (m_pPlayer->m_sPlayerType <= 6))
					m_pPlayer->m_Controller.SetCommand(Type::Run);
				else m_pPlayer->m_Controller.SetCommand(Type::Move);
				m_pPlayer->m_Controller.SetDestination(tile_x, tile_y);
				m_pPlayer->m_Controller.CalculatePlayerTurn(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pMapData.get());
			}
		}
	return false;
}

bool CGame::ProcessRightClick(short mouse_x, short mouse_y, short tile_x, short tile_y, uint32_t current_time, uint16_t& action_type)
{
	std::string name;
	short object_type = 0;
	hb::shared::entity::PlayerStatus object_status;
	char direction = 0, abs_x = 0, abs_y = 0;

		// Right click on self = pickup (Quick Actions feature)
		if (ConfigManager::Get().IsQuickActionsEnabled() &&
			m_cMCName == m_pPlayer->m_cPlayerName &&
			(m_pPlayer->m_sPlayerType >= 1) && (m_pPlayer->m_sPlayerType <= 6))
		{
			m_pPlayer->m_Controller.SetCommand(Type::GetItem);
			m_pPlayer->m_Controller.SetDestination(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY);
			return false;
		}
		else
		{
			// Original right click behavior (stop, turn, attack, etc.)
			m_pPlayer->m_Controller.SetCommand(Type::Stop);
			if (m_bIsGetPointingMode == true)
			{
				m_bIsGetPointingMode = false;
				AddEventList(COMMAND_PROCESSOR1, 10);
			}
			if (m_pPlayer->m_Controller.IsCommandAvailable() == false) return true;
			if (m_pPlayer->m_Controller.GetCommandCount() >= 6) return true;

			if ((m_sMCX != 0) && (m_sMCY != 0))
			{
				abs_x = abs(m_pPlayer->m_sPlayerX - m_sMCX);
				abs_y = abs(m_pPlayer->m_sPlayerY - m_sMCY);
				if (abs_x == 0 && abs_y == 0) return true;

			if (hb::shared::input::IsCtrlDown() == true)
			{
				m_pMapData->bGetOwner(m_sMCX, m_sMCY, name, &object_type, &object_status, &m_wCommObjectID);
				if (object_status.bInvisibility) return true;
				if ((object_type == 15) || (object_type == 20) || (object_type == 24)) return true;

				if ((abs_x <= 1) && (abs_y <= 1))
				{
					action_type = CombatSystem::Get().GetAttackType();
					m_pPlayer->m_Controller.SetCommand(Type::Attack);
					m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
				}
				else if ((abs_x <= 2) && (abs_y <= 2) // strike on Big mobs & gate from a range
					&& ((object_type == 66) || (object_type == 73) || (object_type == 81) || (object_type == 91)))
				{
					action_type = CombatSystem::Get().GetAttackType();
					m_pPlayer->m_Controller.SetCommand(Type::Attack);
					m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
				}
				else
				{
					switch (CombatSystem::Get().GetWeaponSkillType()) {
					case 6: // Bow
						m_pPlayer->m_Controller.SetCommand(Type::Attack);
						m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
						action_type = CombatSystem::Get().GetAttackType();
						break;

					case 5: // Boxe
					case 7: // SS
						break;

					case 8: // LS
						if ((abs_x <= 3) && (abs_y <= 3) && CombatSystem::Get().CanSuperAttack()
							&& (CombatSystem::Get().GetAttackType() != 30)) // without StormBlade by Snoopy
						{
							action_type = CombatSystem::Get().GetAttackType();
							m_pPlayer->m_Controller.SetCommand(Type::Attack);
							m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
						}
						else if ((abs_x <= 5) && (abs_y <= 5) && CombatSystem::Get().CanSuperAttack()
							&& (CombatSystem::Get().GetAttackType() == 30)) // with stormBlade crit by Snoopy
						{
							action_type = CombatSystem::Get().GetAttackType();
							m_pPlayer->m_Controller.SetCommand(Type::Attack);
							m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
						}
						else if ((abs_x <= 3) && (abs_y <= 3)
							&& (CombatSystem::Get().GetAttackType() == 5)) // with stormBlade no crit by Snoopy
						{
							action_type = CombatSystem::Get().GetAttackType();
							m_pPlayer->m_Controller.SetCommand(Type::Attack);
							m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
						}
						break;

					case 9: // Fencing
						if ((abs_x <= 4) && (abs_y <= 4) && CombatSystem::Get().CanSuperAttack()) {
							m_pPlayer->m_Controller.SetCommand(Type::Attack);
							m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
							action_type = CombatSystem::Get().GetAttackType();
						}
						break;

					case hb::shared::owner::Slime: //
						if ((abs_x <= 2) && (abs_y <= 2) && CombatSystem::Get().CanSuperAttack()) {
							m_pPlayer->m_Controller.SetCommand(Type::Attack);
							m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
							action_type = CombatSystem::Get().GetAttackType();
						}
						break;

					case hb::shared::owner::OrcMage: //
						if ((abs_x <= 2) && (abs_y <= 2) && CombatSystem::Get().CanSuperAttack()) {
							m_pPlayer->m_Controller.SetCommand(Type::Attack);
							m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
							action_type = CombatSystem::Get().GetAttackType();
						}
						break;
					case hb::shared::owner::Guard: //
						if ((abs_x <= 2) && (abs_y <= 2) && CombatSystem::Get().CanSuperAttack()) {
							m_pPlayer->m_Controller.SetCommand(Type::Attack);
							m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
							action_type = CombatSystem::Get().GetAttackType();
						}
						break;
					}
				}
			}
			else // CTRL not pressed
			{
				abs_x = abs(m_pPlayer->m_sPlayerX - m_sMCX);
				abs_y = abs(m_pPlayer->m_sPlayerY - m_sMCY);
				m_pMapData->bGetOwner(m_sMCX, m_sMCY, name, &object_type, &object_status, &m_wCommObjectID);
				if (object_type >= 10 || ((object_type >= 1) && (object_type <= 6))) {
					switch (object_type) {
					case hb::shared::owner::ShopKeeper:
					case hb::shared::owner::Gandalf:
					case hb::shared::owner::Howard:
					case hb::shared::owner::Tom:
					case hb::shared::owner::William:
					case hb::shared::owner::Kennedy: // npcs
						break;

					default: // All "normal mobs"
						if (!IsHostile(object_status.iRelationship)) break;
						if ((object_type >= 1) && (object_type <= 6) && (m_pPlayer->m_bForceAttack == false)) break;
						if ((abs_x <= 1) && (abs_y <= 1))
						{
							action_type = CombatSystem::Get().GetAttackType();
							m_pPlayer->m_Controller.SetCommand(Type::Attack);
							m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
						}
						else if ((abs_x <= 2) && (abs_y <= 2) // strike on Big mobs & gate from a range
							&& ((object_type == 66) || (object_type == 73) || (object_type == 81) || (object_type == 91)))
						{
							action_type = CombatSystem::Get().GetAttackType();
							m_pPlayer->m_Controller.SetCommand(Type::Attack);
							m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
						}
						else //
						{
							switch (CombatSystem::Get().GetWeaponSkillType()) {
							case 6: // Bow
								m_pPlayer->m_Controller.SetCommand(Type::Attack);
								m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
								action_type = CombatSystem::Get().GetAttackType();
								break;

							case 5: // Boxe
							case 7: // SS
								break;

							case 8: // LS
								if ((abs_x <= 3) && (abs_y <= 3) && CombatSystem::Get().CanSuperAttack()
									&& (CombatSystem::Get().GetAttackType() != 30)) // crit without StormBlade by Snoopy
								{
									action_type = CombatSystem::Get().GetAttackType();
									m_pPlayer->m_Controller.SetCommand(Type::Attack);
									m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
								}
								else if ((abs_x <= 5) && (abs_y <= 5) && CombatSystem::Get().CanSuperAttack()
									&& (CombatSystem::Get().GetAttackType() == 30)) // with stormBlade crit by Snoopy
								{
									action_type = CombatSystem::Get().GetAttackType();
									m_pPlayer->m_Controller.SetCommand(Type::Attack);
									m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
								}
								else if ((abs_x <= 3) && (abs_y <= 3)
									&& (CombatSystem::Get().GetAttackType() == 5)) // with stormBlade no crit by Snoopy
								{
									action_type = CombatSystem::Get().GetAttackType();
									m_pPlayer->m_Controller.SetCommand(Type::Attack);
									m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
								}
								break;

							case 9: // fencing
								if ((abs_x <= 4) && (abs_y <= 4) && CombatSystem::Get().CanSuperAttack()) {
									m_pPlayer->m_Controller.SetCommand(Type::Attack);
									m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
									action_type = CombatSystem::Get().GetAttackType();
								}
								break;

							case hb::shared::owner::Slime: //
								if ((abs_x <= 2) && (abs_y <= 2) && CombatSystem::Get().CanSuperAttack()) {
									m_pPlayer->m_Controller.SetCommand(Type::Attack);
									m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
									action_type = CombatSystem::Get().GetAttackType();
								}
								break;
							case hb::shared::owner::OrcMage: // hammer
								if ((abs_x <= 2) && (abs_y <= 2) && CombatSystem::Get().CanSuperAttack()) {
									m_pPlayer->m_Controller.SetCommand(Type::Attack);
									m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
									action_type = CombatSystem::Get().GetAttackType();
								}
								break;
							case hb::shared::owner::Guard: // wand
								if ((abs_x <= 2) && (abs_y <= 2) && CombatSystem::Get().CanSuperAttack()) {
									m_pPlayer->m_Controller.SetCommand(Type::Attack);
									m_pPlayer->m_Controller.SetDestination(m_sMCX, m_sMCY);
									action_type = CombatSystem::Get().GetAttackType();
								}
								break;
							}
						}
						break;
					}
				}
			}
		}
		else
		{
			direction = CMisc::cGetNextMoveDir(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, tile_x, tile_y);
			if (m_pPlayer->m_iHP <= 0) return true;
			if (direction == 0) return true;
			if (m_pPlayer->m_iPlayerDir == direction) return true;
			ClearSkillUsingStatus();
			m_pPlayer->m_iPlayerDir = direction;
			bSendCommand(MsgId::CommandMotion, Type::Stop, m_pPlayer->m_iPlayerDir, 0, 0, 0, 0);

			m_pMapData->bSetOwner(m_pPlayer->m_sPlayerObjectID, m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pPlayer->m_sPlayerType, m_pPlayer->m_iPlayerDir,
				m_pPlayer->m_playerAppearance,
				m_pPlayer->m_playerStatus, m_pPlayer->m_cPlayerName,
				m_pPlayer->m_Controller.GetCommand(), 0, 0, 0, 0,
				10);
			m_pPlayer->m_Controller.SetCommandAvailable(false);
			m_pPlayer->m_Controller.SetCommandTime(GameClock::GetTimeMS());
			return true;
		}
		} // close else block for "not clicking on self"
	return false;
}

void CGame::ProcessMotionCommands(uint16_t action_type)
{
	char direction = 0;
	std::string dest_name;
	short dest_owner_type = 0;
	hb::shared::entity::PlayerStatus dest_owner_status;
	bool has_owner = false;
	std::string text;


	if (m_pPlayer->m_Controller.GetCommand() != Type::Stop)
	{
		if (m_pPlayer->m_iHP <= 0) return;
		if (m_pPlayer->m_Controller.GetCommandCount() == 5) AddEventList(COMMAND_PROCESSOR2, 10, false);
		if (m_pPlayer->m_Controller.IsCommandAvailable() == false) return;
		if (m_pPlayer->m_Controller.GetCommandCount() >= 6) return;

		if ((m_pPlayer->m_sPlayerType >= 0) && (m_pPlayer->m_sPlayerType > 6))
		{
			switch (m_pPlayer->m_Controller.GetCommand()) {
			case Type::Run:
			case Type::Magic:
			case Type::GetItem:
				m_pPlayer->m_Controller.SetCommand(Type::Stop);
				break;
			}
		}

		ClearSkillUsingStatus();

		if ((m_pPlayer->m_sDamageMove != 0) || (m_pPlayer->m_sDamageMoveAmount != 0))
		{
			if (m_pPlayer->m_sDamageMove != 0)
			{
				m_pPlayer->m_Controller.SetCommand(Type::DamageMove);
				m_pPlayer->m_Controller.SetDestination(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY);

				// mim crit fixed by kaozures tocado para ande bien by cloud :P
				if (m_bIllusionMVT == true) {
					switch (m_pPlayer->m_sDamageMove) {
					case 1: m_pPlayer->m_Controller.MoveDestination(0, 1); break;
					case 2: m_pPlayer->m_Controller.MoveDestination(-1, 1); break;
					case 3: m_pPlayer->m_Controller.MoveDestination(-1, 0); break;
					case 4: m_pPlayer->m_Controller.MoveDestination(-1, -1); break;
					case 5: m_pPlayer->m_Controller.MoveDestination(0, -1); break;
					case 6: m_pPlayer->m_Controller.MoveDestination(1, -1); break;
					case 7: m_pPlayer->m_Controller.MoveDestination(1, 0); break;
					case 8: m_pPlayer->m_Controller.MoveDestination(1, 1); break;
					}
				}
				else {
					switch (m_pPlayer->m_sDamageMove) {
					case 1: m_pPlayer->m_Controller.MoveDestination(0, -1); break;
					case 2: m_pPlayer->m_Controller.MoveDestination(1, -1); break;
					case 3: m_pPlayer->m_Controller.MoveDestination(1, 0); break;
					case 4: m_pPlayer->m_Controller.MoveDestination(1, 1); break;
					case 5: m_pPlayer->m_Controller.MoveDestination(0, 1); break;
					case 6: m_pPlayer->m_Controller.MoveDestination(-1, 1); break;
					case 7: m_pPlayer->m_Controller.MoveDestination(-1, 0); break;
					case 8: m_pPlayer->m_Controller.MoveDestination(-1, -1); break;
					}
				}
			}

			m_floatingText.AddDamageFromValue(m_pPlayer->m_sDamageMoveAmount, false, m_dwCurTime,
					m_pPlayer->m_sPlayerObjectID, m_pMapData.get());
			m_pPlayer->m_sDamageMove = 0;
			m_pPlayer->m_sDamageMoveAmount = 0;
		}

		switch (m_pPlayer->m_Controller.GetCommand()) {
		case Type::Run:
		case Type::Move:
		case Type::DamageMove: // v1.43

			if (m_pPlayer->m_bParalyze) return;
			has_owner = m_pMapData->bGetOwner(m_pPlayer->m_Controller.GetDestinationX(), m_pPlayer->m_Controller.GetDestinationY(), dest_name, &dest_owner_type, &dest_owner_status, &m_wCommObjectID); // v1.4

			if ((m_pPlayer->m_sPlayerX == m_pPlayer->m_Controller.GetDestinationX()) && (m_pPlayer->m_sPlayerY == m_pPlayer->m_Controller.GetDestinationY()))
			{
				m_pPlayer->m_Controller.SetCommand(Type::Stop);
				// Apply pending stop direction if set (from right-click while moving)
				char pendingDir = m_pPlayer->m_Controller.GetPendingStopDir();
				if (pendingDir != 0)
				{
					m_pPlayer->m_iPlayerDir = pendingDir;
					bSendCommand(MsgId::CommandMotion, Type::Stop, pendingDir, 0, 0, 0, 0);
					m_pMapData->bSetOwner(m_pPlayer->m_sPlayerObjectID, m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY,
						m_pPlayer->m_sPlayerType, pendingDir, m_pPlayer->m_playerAppearance,
						m_pPlayer->m_playerStatus, m_pPlayer->m_cPlayerName, Type::Stop, 0, 0, 0, 0, 10);
					m_pPlayer->m_Controller.ClearPendingStopDir();
				}
			}
			else if ((abs(m_pPlayer->m_sPlayerX - m_pPlayer->m_Controller.GetDestinationX()) <= 1) && (abs(m_pPlayer->m_sPlayerY - m_pPlayer->m_Controller.GetDestinationY()) <= 1) &&
				(has_owner == true) && (dest_owner_type != 0))
				m_pPlayer->m_Controller.SetCommand(Type::Stop);
			else if ((abs(m_pPlayer->m_sPlayerX - m_pPlayer->m_Controller.GetDestinationX()) <= 2) && (abs(m_pPlayer->m_sPlayerY - m_pPlayer->m_Controller.GetDestinationY()) <= 2) &&
				(m_pMapData->m_tile[m_pPlayer->m_Controller.GetDestinationX()][m_pPlayer->m_Controller.GetDestinationY()].m_bIsMoveAllowed == false))
				m_pPlayer->m_Controller.SetCommand(Type::Stop);
			else
			{
				if (m_pPlayer->m_Controller.GetCommand() == Type::Move)
				{
					if (ConfigManager::Get().IsRunningModeEnabled() || hb::shared::input::IsShiftDown()) m_pPlayer->m_Controller.SetCommand(Type::Run);
				}
				if (m_pPlayer->m_Controller.GetCommand() == Type::Run)
				{
					if ((ConfigManager::Get().IsRunningModeEnabled() == false) && (hb::shared::input::IsShiftDown() == false)) m_pPlayer->m_Controller.SetCommand(Type::Move);
					if (m_pPlayer->m_iSP < 1) m_pPlayer->m_Controller.SetCommand(Type::Move);
				}

				direction = m_pPlayer->m_Controller.GetNextMoveDir(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pPlayer->m_Controller.GetDestinationX(), m_pPlayer->m_Controller.GetDestinationY(), m_pMapData.get(), true);
				// Snoopy: Illusion Movement
				if ((m_bIllusionMVT == true) && (m_pPlayer->m_Controller.GetCommand() != Type::DamageMove))
				{
					direction = m_pPlayer->m_Controller.GetNextMoveDir(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pPlayer->m_Controller.GetDestinationX(), m_pPlayer->m_Controller.GetDestinationY(), m_pMapData.get(), true, true);
				}
				if (direction != 0)
				{
					// Cancel logout countdown on movement
					if ((m_cLogOutCount > 0) && (m_bForceDisconn == false))
					{
						m_cLogOutCount = -1;
						AddEventList(DLGBOX_CLICK_SYSMENU2, 10);
					}

					m_pPlayer->m_iPlayerDir = direction;
					bSendCommand(MsgId::CommandMotion, m_pPlayer->m_Controller.GetCommand(), direction, 0, 0, 0, 0);
					switch (direction) {
					case 1:	m_pPlayer->m_sPlayerY--; break;
					case 2:	m_pPlayer->m_sPlayerY--; m_pPlayer->m_sPlayerX++;	break;
					case 3:	m_pPlayer->m_sPlayerX++; break;
					case 4:	m_pPlayer->m_sPlayerX++; m_pPlayer->m_sPlayerY++;	break;
					case 5:	m_pPlayer->m_sPlayerY++; break;
					case 6:	m_pPlayer->m_sPlayerX--; m_pPlayer->m_sPlayerY++;	break;
					case 7:	m_pPlayer->m_sPlayerX--; break;
					case 8:	m_pPlayer->m_sPlayerX--; m_pPlayer->m_sPlayerY--;	break;
					}
					m_pMapData->bSetOwner(m_pPlayer->m_sPlayerObjectID, m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pPlayer->m_sPlayerType, m_pPlayer->m_iPlayerDir,
						m_pPlayer->m_playerAppearance, // v1.4
						m_pPlayer->m_playerStatus, m_pPlayer->m_cPlayerName,
						m_pPlayer->m_Controller.GetCommand(), 0, 0, 0);
					m_pPlayer->m_Controller.SetCommandAvailable(false);
					m_pPlayer->m_Controller.SetCommandTime(GameClock::GetTimeMS());
					m_pPlayer->m_Controller.SetPrevMove(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY);
				}
			}

			if (m_pPlayer->m_Controller.GetCommand() == Type::DamageMove)
			{
				m_bIsGetPointingMode = false;
				m_iPointCommandType = -1;
				ClearSkillUsingStatus();
				m_pPlayer->m_Controller.SetCommand(Type::Stop);
			}
			break;

		case Type::Attack:
			direction = CMisc::cGetNextMoveDir(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pPlayer->m_Controller.GetDestinationX(), m_pPlayer->m_Controller.GetDestinationY());
			// Snoopy: Illusion movement
			if (m_bIllusionMVT == true)
			{
				direction += 4;
				if (direction > 8) direction -= 8;
			}
			if (direction != 0)
			{
				// Cancel logout countdown on attack
				if ((m_cLogOutCount > 0) && (m_bForceDisconn == false))
				{
					m_cLogOutCount = -1;
					AddEventList(DLGBOX_CLICK_SYSMENU2, 10);
				}

				if ((action_type == 2) || (action_type == 25))
				{
					if (_bCheckItemByType(ItemType::Arrow) == false)
						action_type = 0;
				}
					m_pPlayer->m_iPlayerDir = direction;
				m_wLastAttackTargetID = m_wCommObjectID;
				bSendCommand(MsgId::CommandMotion, Type::Attack, direction, m_pPlayer->m_Controller.GetDestinationX(), m_pPlayer->m_Controller.GetDestinationY(), action_type, 0, m_wCommObjectID);
				m_pMapData->bSetOwner(m_pPlayer->m_sPlayerObjectID, m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pPlayer->m_sPlayerType, m_pPlayer->m_iPlayerDir,
					m_pPlayer->m_playerAppearance,
					m_pPlayer->m_playerStatus, m_pPlayer->m_cPlayerName,
					Type::Attack,
					m_pPlayer->m_Controller.GetDestinationX() - m_pPlayer->m_sPlayerX, m_pPlayer->m_Controller.GetDestinationY() - m_pPlayer->m_sPlayerY, action_type);
				m_pPlayer->m_Controller.SetCommandAvailable(false);
				m_pPlayer->m_Controller.SetCommandTime(GameClock::GetTimeMS());
				// Compute expected swing duration (must match server's bCheckClientAttackFrequency formula)
				{
					constexpr int ATTACK_FRAME_DURATIONS = 8;
					int baseFrameTime = PlayerAnim::Attack.sFrameTime; // 78
					int delay = m_pPlayer->m_playerStatus.iAttackDelay * 12;
					if (m_pPlayer->m_playerStatus.bFrozen) delay += baseFrameTime >> 2;
					if (m_pPlayer->m_playerStatus.bHaste)
						delay -= static_cast<int>(PlayerAnim::Run.sFrameTime / 2.3);
					int expectedSwingTime = ATTACK_FRAME_DURATIONS * (baseFrameTime + delay);
					m_pPlayer->m_Controller.SetAttackEndTime(GameClock::GetTimeMS() + expectedSwingTime);
				}
			}
			m_pPlayer->m_Controller.SetCommand(Type::Stop);
			break;

		case Type::AttackMove:
			if (m_pPlayer->m_bParalyze) return;
			has_owner = m_pMapData->bGetOwner(m_pPlayer->m_Controller.GetDestinationX(), m_pPlayer->m_Controller.GetDestinationY(), dest_name, &dest_owner_type, &dest_owner_status, &m_wCommObjectID);
			if ((m_pPlayer->m_sPlayerX == m_pPlayer->m_Controller.GetDestinationX()) && (m_pPlayer->m_sPlayerY == m_pPlayer->m_Controller.GetDestinationY()))
				m_pPlayer->m_Controller.SetCommand(Type::Stop);
			else if ((abs(m_pPlayer->m_sPlayerX - m_pPlayer->m_Controller.GetDestinationX()) <= 1) && (abs(m_pPlayer->m_sPlayerY - m_pPlayer->m_Controller.GetDestinationY()) <= 1) &&
				(has_owner == true) && (dest_owner_type != 0))
				m_pPlayer->m_Controller.SetCommand(Type::Stop);
			else
			{
				direction = m_pPlayer->m_Controller.GetNextMoveDir(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pPlayer->m_Controller.GetDestinationX(), m_pPlayer->m_Controller.GetDestinationY(), m_pMapData.get(), true);
				// Snoopy: Illusion mvt
				if (m_bIllusionMVT == true)
				{
					direction = m_pPlayer->m_Controller.GetNextMoveDir(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pPlayer->m_Controller.GetDestinationX(), m_pPlayer->m_Controller.GetDestinationY(), m_pMapData.get(), true, true);
				}
				if (direction != 0)
				{
					// Cancel logout countdown on attack-move
					if ((m_cLogOutCount > 0) && (m_bForceDisconn == false))
					{
						m_cLogOutCount = -1;
						AddEventList(DLGBOX_CLICK_SYSMENU2, 10);
					}

					m_pPlayer->m_iPlayerDir = direction;
					m_wLastAttackTargetID = m_wCommObjectID;
					bSendCommand(MsgId::CommandMotion, Type::AttackMove, direction, m_pPlayer->m_Controller.GetDestinationX(), m_pPlayer->m_Controller.GetDestinationY(), action_type, 0, m_wCommObjectID);
					switch (direction) {
					case 1:	m_pPlayer->m_sPlayerY--; break;
					case 2:	m_pPlayer->m_sPlayerY--; m_pPlayer->m_sPlayerX++;	break;
					case 3:	m_pPlayer->m_sPlayerX++; break;
					case 4:	m_pPlayer->m_sPlayerX++; m_pPlayer->m_sPlayerY++;	break;
					case 5:	m_pPlayer->m_sPlayerY++; break;
					case 6:	m_pPlayer->m_sPlayerX--; m_pPlayer->m_sPlayerY++;	break;
					case 7:	m_pPlayer->m_sPlayerX--; break;
					case 8:	m_pPlayer->m_sPlayerX--; m_pPlayer->m_sPlayerY--;	break;
					}

					m_pMapData->bSetOwner(m_pPlayer->m_sPlayerObjectID, m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pPlayer->m_sPlayerType, m_pPlayer->m_iPlayerDir,
						m_pPlayer->m_playerAppearance,
						m_pPlayer->m_playerStatus, m_pPlayer->m_cPlayerName,
						m_pPlayer->m_Controller.GetCommand(), m_pPlayer->m_Controller.GetDestinationX() - m_pPlayer->m_sPlayerX, m_pPlayer->m_Controller.GetDestinationY() - m_pPlayer->m_sPlayerY, action_type);
					m_pPlayer->m_Controller.SetCommandAvailable(false);
					m_pPlayer->m_Controller.SetCommandTime(GameClock::GetTimeMS());
					// Compute expected swing duration (must match server's bCheckClientAttackFrequency formula)
					{
						constexpr int ATTACK_FRAME_DURATIONS = 8;
						int baseFrameTime = PlayerAnim::Attack.sFrameTime; // 78
						int delay = m_pPlayer->m_playerStatus.iAttackDelay * 12;
						if (m_pPlayer->m_playerStatus.bFrozen) delay += baseFrameTime >> 2;
						if (m_pPlayer->m_playerStatus.bHaste)
							delay -= static_cast<int>(PlayerAnim::Run.sFrameTime / 2.3);
						int expectedSwingTime = ATTACK_FRAME_DURATIONS * (baseFrameTime + delay);
						m_pPlayer->m_Controller.SetAttackEndTime(GameClock::GetTimeMS() + expectedSwingTime);
					}
					m_pPlayer->m_Controller.SetPrevMove(m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY);
				}
			}
			m_pPlayer->m_Controller.SetCommand(Type::Stop);
			break;

		case Type::GetItem:
			// Cancel logout countdown on get item
			if ((m_cLogOutCount > 0) && (m_bForceDisconn == false))
			{
				m_cLogOutCount = -1;
				AddEventList(DLGBOX_CLICK_SYSMENU2, 10);
			}

			bSendCommand(MsgId::CommandMotion, Type::GetItem, m_pPlayer->m_iPlayerDir, 0, 0, 0, 0);
			m_pMapData->bSetOwner(m_pPlayer->m_sPlayerObjectID, m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pPlayer->m_sPlayerType, m_pPlayer->m_iPlayerDir,
				m_pPlayer->m_playerAppearance,
				m_pPlayer->m_playerStatus, m_pPlayer->m_cPlayerName,
				Type::GetItem, 0, 0, 0);
			m_pPlayer->m_Controller.SetCommandAvailable(false);
			m_pPlayer->m_Controller.SetCommand(Type::Stop);
			break;

		case Type::Magic:
			// Cancel logout countdown on magic cast
			if ((m_cLogOutCount > 0) && (m_bForceDisconn == false))
			{
				m_cLogOutCount = -1;
				AddEventList(DLGBOX_CLICK_SYSMENU2, 10);
			}

			bSendCommand(MsgId::CommandMotion, Type::Magic, m_pPlayer->m_iPlayerDir, m_iCastingMagicType, 0, 0, 0);
			m_pMapData->bSetOwner(m_pPlayer->m_sPlayerObjectID, m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pPlayer->m_sPlayerType, m_pPlayer->m_iPlayerDir,
				m_pPlayer->m_playerAppearance,
				m_pPlayer->m_playerStatus, m_pPlayer->m_cPlayerName,
				Type::Magic, m_iCastingMagicType, 0, 0);
			m_pPlayer->m_Controller.SetCommandAvailable(false);
			m_pPlayer->m_Controller.SetCommandTime(GameClock::GetTimeMS());
			// Only enter targeting mode if the cast wasn't interrupted by damage
			if (m_iPointCommandType >= 100 && m_iPointCommandType < 200)
				m_bIsGetPointingMode = true;
			m_pPlayer->m_Controller.SetCommand(Type::Stop);
			m_floatingText.RemoveByObjectID(m_pPlayer->m_sPlayerObjectID);
			{
				text = std::format("{}!", m_pMagicCfgList[m_iCastingMagicType]->m_cName);
				m_floatingText.AddNotifyText(NotifyTextType::MagicCastName, text.c_str(), GameClock::GetTimeMS(),
					m_pPlayer->m_sPlayerObjectID, m_pMapData.get());
			}
			return;

		default:
			break;
		}
	}
}

// _Draw_OnLogin removed - migrated to Screen_Login

void CGame::RequestTeleportAndWaitData()
{
	if (TeleportManager::Get().IsRequested()) return;

	bSendCommand(MsgId::RequestTeleport, 0, 0, 0, 0, 0, 0);
	ChangeGameMode(GameMode::WaitingInitData);
}

void CGame::InitDataResponseHandler(char* packet_data)
{
	short pivot_x = 0, pivot_y = 0;
	std::string text, prev_location;

	char map_filename[32]{};
	bool is_observer = false;
	uint32_t file_size = 0;

	m_pPlayer->m_bParalyze = false;
	m_pMapData->Init();

	m_sMonsterID = 0;
	m_dwMonsterEventTime = 0;

	m_dialogBoxManager.DisableDialogBox(DialogBoxId::GuildMenu);
	m_dialogBoxManager.DisableDialogBox(DialogBoxId::SaleMenu);
	m_dialogBoxManager.DisableDialogBox(DialogBoxId::CityHallMenu);
	m_dialogBoxManager.DisableDialogBox(DialogBoxId::Bank);
	m_dialogBoxManager.DisableDialogBox(DialogBoxId::MagicShop);
	m_dialogBoxManager.DisableDialogBox(DialogBoxId::Map);
	m_dialogBoxManager.DisableDialogBox(DialogBoxId::NpcActionQuery);
	m_dialogBoxManager.DisableDialogBox(DialogBoxId::NpcTalk);
	m_dialogBoxManager.DisableDialogBox(DialogBoxId::SellOrRepair);
	m_dialogBoxManager.DisableDialogBox(DialogBoxId::GuildHallMenu); // Gail's diag

	m_pPlayer->m_Controller.SetCommand(Type::Stop);
	m_pPlayer->m_Controller.ResetCommandCount();
	m_bIsGetPointingMode = false;
	m_iPointCommandType = -1;
	m_iIlusionOwnerH = 0;
	m_cIlusionOwnerType = 0;
	TeleportManager::Get().SetRequested(false);
	m_pPlayer->m_bIsConfusion = false;
	m_bSkillUsingStatus = false;

	m_bItemUsingStatus = false;

	m_cRestartCount = -1;
	m_dwRestartCountTime = 0;

	if (m_pEffectManager) m_pEffectManager->ClearAllEffects();

	WeatherManager::Get().ResetParticles();

	for (int i = 0; i < game_limits::max_guild_names; i++)
	{
		m_stGuildName[i].dwRefTime = 0;
		m_stGuildName[i].iGuildRank = -1;
	}

	m_floatingText.ClearAll();

	const auto* pkt = hb::net::PacketCast<hb::net::PacketResponseInitDataHeader>(
		packet_data, sizeof(hb::net::PacketResponseInitDataHeader));
	if (!pkt) return;
	m_pPlayer->m_sPlayerObjectID = pkt->player_object_id;
	pivot_x = pkt->pivot_x;
	pivot_y = pkt->pivot_y;
	m_pPlayer->m_sPlayerType = pkt->player_type;
	m_pPlayer->m_playerAppearance = pkt->appearance;
	m_pPlayer->m_playerStatus = pkt->status;

	//Snoopy MIM fix
	if (m_pPlayer->m_playerStatus.bIllusionMovement)
	{
		m_bIllusionMVT = true;
	}
	else
	{
		m_bIllusionMVT = false;
	}

	// GM mode detection
	m_pPlayer->m_bIsGMMode = m_pPlayer->m_playerStatus.bGMMode;
	m_cMapName.assign(pkt->map_name, strnlen(pkt->map_name, sizeof(pkt->map_name)));
	char mapMsgBuf[32]{};
	m_cMapIndex = GetOfficialMapName(m_cMapName.c_str(), mapMsgBuf);
	m_cMapMessage = mapMsgBuf;
	if (m_cMapIndex < 0)
	{
		m_dialogBoxManager.Info(DialogBoxId::GuideMap).sSizeX = -1;
		m_dialogBoxManager.Info(DialogBoxId::GuideMap).sSizeY = -1;
	}
	else
	{
		m_dialogBoxManager.Info(DialogBoxId::GuideMap).sSizeX = 128;
		m_dialogBoxManager.Info(DialogBoxId::GuideMap).sSizeY = 128;
	}

	prev_location = m_cCurLocation;
	m_cCurLocation.assign(pkt->cur_location, strnlen(pkt->cur_location, sizeof(pkt->cur_location)));

	WeatherManager::Get().SetAmbientLight(static_cast<char>(pkt->sprite_alpha));

	WeatherManager::Get().SetWeatherStatus(static_cast<char>(pkt->weather_status));
	switch (WeatherManager::Get().GetAmbientLight()) { // Xmas bulbs
		// Will be sent by server if DayTime is 3 (and a snowy weather)
	case 1:	m_bIsXmas = false; break;
	case 2: m_bIsXmas = false; break;
	case 3: // Snoopy Special night with chrismas bulbs
		if (WeatherManager::Get().GetWeatherStatus() > 3) m_bIsXmas = true;
		else m_bIsXmas = false;
		WeatherManager::Get().SetXmas(m_bIsXmas);
		WeatherManager::Get().SetAmbientLight(2);
		break;
	}
	m_pPlayer->m_iContribution = pkt->contribution;
	is_observer = pkt->observer_mode != 0;
	m_pPlayer->m_iHP = pkt->hp;
	m_cDiscount = static_cast<char>(pkt->discount);

	const char* cursor = reinterpret_cast<const char*>(packet_data) + sizeof(hb::net::PacketResponseInitDataHeader);

	{
		char ws = WeatherManager::Get().GetWeatherStatus();
		if (ws != 0)
		{
			WeatherManager::Get().SetWeather(true, ws);
			if (ws >= 4 && ws <= 6 && AudioManager::Get().IsMusicEnabled())
				StartBGM();
		}
		else WeatherManager::Get().SetWeather(false, 0);
	}

	std::memset(map_filename, 0, sizeof(map_filename));
	std::snprintf(map_filename + strlen(map_filename), sizeof(map_filename) - strlen(map_filename), "%s", "mapdata/");
	// CLEROTH - MW MAPS
	if (m_cMapName.starts_with("defaultmw"))
	{
		std::snprintf(map_filename + strlen(map_filename), sizeof(map_filename) - strlen(map_filename), "%s", "mw/defaultmw");
	}
	else
	{
		std::snprintf(map_filename + strlen(map_filename), sizeof(map_filename) - strlen(map_filename), "%s", m_cMapName.c_str());
	}

	std::snprintf(map_filename + strlen(map_filename), sizeof(map_filename) - strlen(map_filename), "%s", ".amd");
	m_pMapData->OpenMapDataFile(map_filename);

	m_pMapData->m_sPivotX = pivot_x;
	m_pMapData->m_sPivotY = pivot_y;

	m_pPlayer->m_sPlayerX = pivot_x + hb::shared::view::PlayerPivotOffsetX;
	m_pPlayer->m_sPlayerY = pivot_y + hb::shared::view::PlayerPivotOffsetY;

	m_pPlayer->m_iPlayerDir = 5;

	if (is_observer == false)
	{
		m_pMapData->bSetOwner(m_pPlayer->m_sPlayerObjectID, m_pPlayer->m_sPlayerX, m_pPlayer->m_sPlayerY, m_pPlayer->m_sPlayerType, m_pPlayer->m_iPlayerDir,
			m_pPlayer->m_playerAppearance, // v1.4
			m_pPlayer->m_playerStatus, m_pPlayer->m_cPlayerName,
			Type::Stop, 0, 0, 0);
	}

	m_Camera.SnapTo((m_pPlayer->m_sPlayerX - VIEW_CENTER_TILE_X()) * 32 - 16, (m_pPlayer->m_sPlayerY - VIEW_CENTER_TILE_Y()) * 32 - 16);
	_ReadMapData(pivot_x + hb::shared::view::MapDataBufferX, pivot_y + hb::shared::view::MapDataBufferY, cursor);
	// ------------------------------------------------------------------------+
	text = std::format(INITDATA_RESPONSE_HANDLER1, m_cMapMessage);
	AddEventList(text.c_str(), 10);

	m_dialogBoxManager.Info(DialogBoxId::WarningBattleArea).sX = 150;
	m_dialogBoxManager.Info(DialogBoxId::WarningBattleArea).sY = 130;

	if ((m_cCurLocation.starts_with("middleland"))
		|| (m_cCurLocation.starts_with("dglv2"))
		|| (m_cCurLocation.starts_with("middled1n")))
		m_dialogBoxManager.EnableDialogBox(DialogBoxId::WarningBattleArea, 0, 0, 0);

	m_bIsServerChanging = false;

	// Wait for configs before entering the game world
	m_bInitDataReady = true;
	if (m_bConfigsReady) {
		DevConsole::Get().Printf("[INIT] Entering game (initdata triggered)");
		GameModeManager::set_screen<Screen_OnGame>();
		m_bInitDataReady = false;
	}

	//v1.41
	if (m_pPlayer->m_playerAppearance.bIsWalking)
		m_pPlayer->m_bIsCombatMode = true;
	else m_pPlayer->m_bIsCombatMode = false;

	//v1.42
	if (m_bIsFirstConn == true)
	{
		m_bIsFirstConn = false;
		file_size = 0;
		{
			FILE* f = fopen("contents/contents1000.txt", "rb");
			if (f)
			{
				fseek(f, 0, SEEK_END);
				file_size = static_cast<uint32_t>(ftell(f));
				fclose(f);
			}
		}
		bSendCommand(MsgId::RequestNoticement, 0, 0, static_cast<int>(file_size), 0, 0, 0);
	}
}

void CGame::MotionEventHandler(char* packet_data)
{
	WORD event_type = 0, object_id = 0;
	short map_x = -1, map_y = -1, owner_type = 0, value1 = 0, value2 = 0, value3 = 0;
	short npc_config_id = -1;
	hb::shared::entity::PlayerStatus status;
	char direction = 0;
	std::string name;
	int location = 0;
	hb::shared::entity::PlayerAppearance appearance;
	bool prev_combat_mode = false;
	std::string text;

	const auto* header = hb::net::PacketCast<hb::net::PacketHeader>(packet_data, sizeof(hb::net::PacketHeader));
	if (!header) return;
	event_type = header->msg_type;

	const auto* baseId = hb::net::PacketCast<hb::net::PacketEventMotionBaseId>(packet_data, sizeof(hb::net::PacketEventMotionBaseId));
	if (!baseId) return;
	object_id = baseId->object_id;

	if (hb::shared::object_id::IsNpcID(object_id)) {
		m_dwLastNpcEventTime = GameClock::GetTimeMS();
	}

	if (!hb::shared::object_id::IsNearbyOffset(object_id))
	{
		if (hb::shared::object_id::IsPlayerID(object_id)) 	// Player
		{
			const auto* pkt = hb::net::PacketCast<hb::net::PacketEventMotionPlayer>(packet_data, sizeof(hb::net::PacketEventMotionPlayer));
			if (!pkt) return;
			map_x = pkt->x;
			map_y = pkt->y;
			owner_type = pkt->type;
			direction = static_cast<char>(pkt->dir);
			name.assign(pkt->name, strnlen(pkt->name, sizeof(pkt->name)));
			appearance = pkt->appearance;
			status = pkt->status;
			location = pkt->loc;
		}
		else 	// Npc or mob
		{
			const auto* pkt = hb::net::PacketCast<hb::net::PacketEventMotionNpc>(packet_data, sizeof(hb::net::PacketEventMotionNpc));
			if (!pkt) return;
			map_x = pkt->x;
			map_y = pkt->y;
			npc_config_id = pkt->config_id;
			owner_type = ResolveNpcType(npc_config_id);
			direction = static_cast<char>(pkt->dir);
			name.assign(pkt->name, strnlen(pkt->name, sizeof(pkt->name)));
			appearance.SetFromNpcAppearance(pkt->appearance);
			status.SetFromEntityStatus(pkt->status);
			location = pkt->loc;
		}
	}
	else
	{
		switch (event_type) {
		case Type::Move:
		case Type::Run:
		{
			const auto* pkt = hb::net::PacketCast<hb::net::PacketEventMotionDirOnly>(packet_data, sizeof(hb::net::PacketEventMotionDirOnly));
			if (!pkt) return;
			direction = static_cast<char>(pkt->dir);
			map_x = -1;
			map_y = -1;
		}
		break;

		case Type::Magic:
		case Type::Damage:
		case Type::DamageMove:
		{
			const auto* pkt = hb::net::PacketCast<hb::net::PacketEventMotionShort>(packet_data, sizeof(hb::net::PacketEventMotionShort));
			if (!pkt) return;
			direction = static_cast<char>(pkt->dir);
			value1 = pkt->v1; // Damage or 0
			value2 = pkt->v2;
			map_x = -1;
			map_y = -1;
		}
		break;

		case Type::Dying:
		{
			const auto* pkt = hb::net::PacketCast<hb::net::PacketEventMotionMove>(packet_data, sizeof(hb::net::PacketEventMotionMove));
			if (!pkt) return;
			direction = static_cast<char>(pkt->dir);
			value1 = pkt->v1;
			value2 = pkt->v2;
			map_x = pkt->x;
			map_y = pkt->y;
		}
		break;

		case Type::Attack:
		case Type::AttackMove:
		{
			const auto* pkt = hb::net::PacketCast<hb::net::PacketEventMotionAttack>(packet_data, sizeof(hb::net::PacketEventMotionAttack));
			if (!pkt) return;
			direction = static_cast<char>(pkt->dir);
			value1 = pkt->v1;
			value2 = pkt->v2;
			value3 = pkt->v3;
		}
		break;

		default:
		{
			const auto* pkt = hb::net::PacketCast<hb::net::PacketEventMotionDirOnly>(packet_data, sizeof(hb::net::PacketEventMotionDirOnly));
			if (!pkt) return;
			direction = static_cast<char>(pkt->dir);
		}
		break;
		}
	}

	if ((event_type == Type::NullAction) && (name == m_pPlayer->m_cPlayerName))
	{
		m_pPlayer->m_sPlayerType = owner_type;
		prev_combat_mode = m_pPlayer->m_playerAppearance.bIsWalking;
		m_pPlayer->m_playerAppearance = appearance;
		m_pPlayer->m_playerStatus = status;
		m_pPlayer->m_bIsGMMode = m_pPlayer->m_playerStatus.bGMMode;
		if (!prev_combat_mode)
		{
			if (appearance.bIsWalking)
			{
				AddEventList(MOTION_EVENT_HANDLER1, 10);
				m_pPlayer->m_bIsCombatMode = true;
			}
		}
		else
		{
			if (!appearance.bIsWalking)
			{
				AddEventList(MOTION_EVENT_HANDLER2, 10);
				m_pPlayer->m_bIsCombatMode = false;
			}
		}
		if (m_pPlayer->m_Controller.GetCommand() != Type::Run && m_pPlayer->m_Controller.GetCommand() != Type::Move) { m_pMapData->bSetOwner(object_id, map_x, map_y, owner_type, direction, appearance, status, name, (char)event_type, value1, value2, value3, location, 0, npc_config_id); }
	}
	else { m_pMapData->bSetOwner(object_id, map_x, map_y, owner_type, direction, appearance, status, name, (char)event_type, value1, value2, value3, location, 0, npc_config_id); }

	switch (event_type) {
	case Type::Magic: // Casting
		m_floatingText.RemoveByObjectID(hb::shared::object_id::ToRealID(object_id));
		{
			text = std::format("{}!", m_pMagicCfgList[value1]->m_cName);
			m_floatingText.AddNotifyText(NotifyTextType::MagicCastName, text.c_str(), m_dwCurTime,
				hb::shared::object_id::ToRealID(object_id), m_pMapData.get());
		}
		break;

	case Type::Dying:
		m_floatingText.RemoveByObjectID(hb::shared::object_id::ToRealID(object_id));
		m_floatingText.AddDamageFromValue(value1, true, m_dwCurTime,
			hb::shared::object_id::ToRealID(object_id), m_pMapData.get());
		break;

	case Type::Damage:
	case Type::DamageMove:
		if (name == m_pPlayer->m_cPlayerName)
		{
			// Cancel spell casting if in the animation phase (Type::Magic)
			if (m_pPlayer->m_Controller.GetCommand() == Type::Magic)
				m_pPlayer->m_Controller.SetCommand(Type::Stop);
			m_bIsGetPointingMode = false;
			m_iPointCommandType = -1;
			ClearSkillUsingStatus();
			// Lock the controller until the damage animation finishes.
			// Without this, quick actions allows immediate movement after being hit.
			m_pPlayer->m_Controller.SetCommand(Type::Stop);
			m_pPlayer->m_Controller.SetCommandAvailable(false);
			m_pPlayer->m_Controller.SetCommandTime(GameClock::GetTimeMS());
		}
		m_floatingText.RemoveByObjectID(hb::shared::object_id::ToRealID(object_id));
		m_floatingText.AddDamageFromValue(value1, false, m_dwCurTime,
			hb::shared::object_id::ToRealID(object_id), m_pMapData.get());
		break;

	case Type::Attack:
	case Type::AttackMove:
		if (object_id == m_pPlayer->m_sPlayerObjectID + hb::shared::object_id::NearbyOffset)
		{
			if (m_pMagicCfgList[value3] != 0)
			{
				text = m_pMagicCfgList[value3]->m_cName;
				AddEventList(text.c_str(), 10);
			}
		}
		break;
	}
}

void CGame::GrandMagicResult(const char* map_name, int ares_crusade_points, int elv_crusade_points, int ares_industry_points, int elv_industry_points, int ares_crusade_casualties, int ares_industry_casualties, int elv_crusade_casualties, int elv_industry_casualties)
{
	int text_index = 0;
	char temp[120]{};

	for (int i = 0; i < game_limits::max_text_dlg_lines; i++)
	{
		if (m_pMsgTextList[i] != 0)
			m_pMsgTextList[i].reset();
	}

	for (int i = 0; i < 92; i++)
		if (m_pGameMsgList[i] == 0) return;

	if (strcmp(map_name, "aresden") == 0)
	{
		m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[2]->m_pMsg, 0);
		m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[3]->m_pMsg, 0);
		m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, " ", 0);

		std::snprintf(temp, sizeof(temp), "%s %d", m_pGameMsgList[4]->m_pMsg, ares_crusade_points);
		m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, temp, 0);

		std::snprintf(temp, sizeof(temp), "%s %d", m_pGameMsgList[5]->m_pMsg, elv_crusade_points);
		m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, temp, 0);

		std::snprintf(temp, sizeof(temp), "%s %d", m_pGameMsgList[6]->m_pMsg, ares_industry_points);
		m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, temp, 0);

		std::snprintf(temp, sizeof(temp), "%s %d", m_pGameMsgList[58]->m_pMsg, elv_industry_points);
		m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, temp, 0);
		m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, " ", 0);

		std::snprintf(temp, sizeof(temp), "%s %d %d %d %d", NOTIFY_MSG_STRUCTURE_HP, ares_crusade_casualties, ares_industry_casualties, elv_crusade_casualties, elv_industry_casualties);
		m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, temp, 0);
		m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, " ", 0);

		if (elv_crusade_points == 0) {
			if ((m_pPlayer->m_bCitizen == true) && (m_pPlayer->m_bAresden == false))
			{
				PlayGameSound('E', 25, 0, 0);
				m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[59]->m_pMsg, 0);
				m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[60]->m_pMsg, 0);
				m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[61]->m_pMsg, 0);
				m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[62]->m_pMsg, 0);
				for (int i = text_index; i < 18; i++) m_pMsgTextList[i] = std::make_unique<CMsg>(0, " ", 0);
			}
			else if ((m_pPlayer->m_bCitizen == true) && (m_pPlayer->m_bAresden == true))
			{
				PlayGameSound('E', 25, 0, 0);
				m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[69]->m_pMsg, 0);
				m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[70]->m_pMsg, 0);
				m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[71]->m_pMsg, 0);
				m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[72]->m_pMsg, 0);
				m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[73]->m_pMsg, 0);
				m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[74]->m_pMsg, 0);
				for (int i = text_index; i < 18; i++) m_pMsgTextList[i] = std::make_unique<CMsg>(0, " ", 0);
			}
			else PlayGameSound('E', 25, 0, 0);
		}
		else
		{
			if (ares_crusade_points != 0)
			{
				if ((m_pPlayer->m_bCitizen == true) && (m_pPlayer->m_bAresden == false))
				{
					PlayGameSound('E', 23, 0, 0);
					PlayGameSound('C', 21, 0, 0);
					PlayGameSound('C', 22, 0, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[63]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[64]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[65]->m_pMsg, 0);
					for (int i = text_index; i < 18; i++) m_pMsgTextList[i] = std::make_unique<CMsg>(0, " ", 0);
				}
				else if ((m_pPlayer->m_bCitizen == true) && (m_pPlayer->m_bAresden == true))
				{
					PlayGameSound('E', 24, 0, 0);
					PlayGameSound('C', 12, 0, 0);
					PlayGameSound('C', 13, 0, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[75]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[76]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[77]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[78]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[79]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[80]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[81]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[82]->m_pMsg, 0);
					for (int i = text_index; i < 18; i++) m_pMsgTextList[i] = std::make_unique<CMsg>(0, " ", 0);
				}
				else PlayGameSound('E', 25, 0, 0);
			}
			else
			{
				if ((m_pPlayer->m_bCitizen == true) && (m_pPlayer->m_bAresden == false))
				{
					PlayGameSound('E', 23, 0, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[66]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[67]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[68]->m_pMsg, 0);
					for (int i = text_index; i < 18; i++) m_pMsgTextList[i] = std::make_unique<CMsg>(0, " ", 0);
				}
				else if ((m_pPlayer->m_bCitizen == true) && (m_pPlayer->m_bAresden == true))
				{
					PlayGameSound('E', 24, 0, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[83]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[84]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[85]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[86]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[87]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[88]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[89]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[90]->m_pMsg, 0);
					for (int i = text_index; i < 18; i++) m_pMsgTextList[i] = std::make_unique<CMsg>(0, " ", 0);
				}
				else PlayGameSound('E', 25, 0, 0);
			}
		}
	}
	else if (strcmp(map_name, "elvine") == 0)
	{
		m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[7]->m_pMsg, 0);
		m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[8]->m_pMsg, 0);
		m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, " ", 0);

		std::snprintf(temp, sizeof(temp), "%s %d", m_pGameMsgList[4]->m_pMsg, ares_crusade_points);
		m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, temp, 0);

		std::snprintf(temp, sizeof(temp), "%s %d", m_pGameMsgList[5]->m_pMsg, elv_crusade_points);
		m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, temp, 0);

		std::snprintf(temp, sizeof(temp), "%s %d", m_pGameMsgList[6]->m_pMsg, ares_industry_points);
		m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, temp, 0);

		std::snprintf(temp, sizeof(temp), "%s %d", m_pGameMsgList[58]->m_pMsg, elv_industry_points);
		m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, temp, 0);
		m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, " ", 0);

		std::snprintf(temp, sizeof(temp), "%s %d %d %d %d", NOTIFY_MSG_STRUCTURE_HP, ares_crusade_casualties, ares_industry_casualties, elv_crusade_casualties, elv_industry_casualties);
		m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, temp, 0);
		m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, " ", 0);

		if (elv_crusade_points == 0) {
			if ((m_pPlayer->m_bCitizen == true) && (m_pPlayer->m_bAresden == true))
			{
				PlayGameSound('E', 25, 0, 0);
				m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[59]->m_pMsg, 0);
				m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[60]->m_pMsg, 0);
				m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[61]->m_pMsg, 0);
				m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[62]->m_pMsg, 0);
				for (int i = text_index; i < 18; i++) m_pMsgTextList[i] = std::make_unique<CMsg>(0, " ", 0);
			}
			else if ((m_pPlayer->m_bCitizen == true) && (m_pPlayer->m_bAresden == false))
			{
				PlayGameSound('E', 25, 0, 0);
				m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[69]->m_pMsg, 0);
				m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[70]->m_pMsg, 0);
				m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[71]->m_pMsg, 0);
				m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[72]->m_pMsg, 0);
				m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[73]->m_pMsg, 0);
				m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[74]->m_pMsg, 0);
				for (int i = text_index; i < 18; i++) m_pMsgTextList[i] = std::make_unique<CMsg>(0, " ", 0);
			}
			else PlayGameSound('E', 25, 0, 0);
		}
		else
		{
			if (ares_crusade_points != 0) {
				if ((m_pPlayer->m_bCitizen == true) && (m_pPlayer->m_bAresden == true))
				{
					PlayGameSound('E', 23, 0, 0);
					PlayGameSound('C', 21, 0, 0);
					PlayGameSound('C', 22, 0, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[63]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[64]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[65]->m_pMsg, 0);
					for (int i = text_index; i < 18; i++) m_pMsgTextList[i] = std::make_unique<CMsg>(0, " ", 0);
				}
				else if ((m_pPlayer->m_bCitizen == true) && (m_pPlayer->m_bAresden == false))
				{
					PlayGameSound('E', 24, 0, 0);
					PlayGameSound('C', 12, 0, 0);
					PlayGameSound('C', 13, 0, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[75]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[76]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[77]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[78]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[79]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[80]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[81]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[82]->m_pMsg, 0);
					for (int i = text_index; i < 18; i++) m_pMsgTextList[i] = std::make_unique<CMsg>(0, " ", 0);
				}
				else PlayGameSound('E', 25, 0, 0);
			}
			else
			{
				if ((m_pPlayer->m_bCitizen == true) && (m_pPlayer->m_bAresden == true))
				{
					PlayGameSound('E', 23, 0, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[66]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[67]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[68]->m_pMsg, 0);
					for (int i = text_index; i < 18; i++) m_pMsgTextList[i] = std::make_unique<CMsg>(0, " ", 0);
				}
				else if ((m_pPlayer->m_bCitizen == true) && (m_pPlayer->m_bAresden == false))
				{
					PlayGameSound('E', 24, 0, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[83]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[84]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[85]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[86]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[87]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[88]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[89]->m_pMsg, 0);
					m_pMsgTextList[text_index++] = std::make_unique<CMsg>(0, m_pGameMsgList[90]->m_pMsg, 0);
					for (int i = text_index; i < 18; i++) m_pMsgTextList[i] = std::make_unique<CMsg>(0, " ", 0);
				}
				else PlayGameSound('E', 25, 0, 0);
			}
		}
	}

	m_dialogBoxManager.EnableDialogBox(DialogBoxId::Text, 0, 0, 0);
}

// num : 1 - F2, 2 - F3
void CGame::UseShortCut(int num)
{
	std::string G_cTxt;
	int index;
	if (num < 4) index = num;
	else index = num + 7;
	if (GameModeManager::GetMode() != GameMode::MainGame) return;
	if (hb::shared::input::IsCtrlDown() == true)
	{
		if (m_sRecentShortCut == -1)
		{
			AddEventList(MSG_SHORTCUT1, 10);
			G_cTxt = std::format(MSG_SHORTCUT2, index);// [F%d]
			AddEventList(G_cTxt.c_str(), 10);
			G_cTxt = std::format(MSG_SHORTCUT3, index);// [Control]-[F%d]
			AddEventList(G_cTxt.c_str(), 10);
		}
		else
		{
			m_sShortCut[num] = m_sRecentShortCut;
			if (m_sShortCut[num] < 100)
			{
				if (m_pItemList[m_sShortCut[num]] == 0)
				{
					m_sShortCut[num] = -1;
					m_sRecentShortCut = -1;
					return;
				}

				auto itemInfo2 = ItemNameFormatter::Get().Format(m_pItemList[m_sShortCut[num]].get());
				G_cTxt = std::format(MSG_SHORTCUT4, itemInfo2.name.c_str(), itemInfo2.effect.c_str(), itemInfo2.extra.c_str(), index);// (%s %s %s) [F%d]
				AddEventList(G_cTxt.c_str(), 10);
			}
			else if (m_sShortCut[num] >= 100)
			{
				if (m_pMagicCfgList[m_sShortCut[num] - 100] == 0)
				{
					m_sShortCut[num] = -1;
					m_sRecentShortCut = -1;
					return;
				}
				G_cTxt = std::format(MSG_SHORTCUT5, m_pMagicCfgList[m_sShortCut[num] - 100]->m_cName, index);// %s) [F%d])
				AddEventList(G_cTxt.c_str(), 10);
			}
		}
	}
	else
	{
		if (m_sShortCut[num] == -1)
		{
			AddEventList(MSG_SHORTCUT1, 10);
			G_cTxt = std::format(MSG_SHORTCUT2, index);// [F%d]
			AddEventList(G_cTxt.c_str(), 10);
			G_cTxt = std::format(MSG_SHORTCUT3, index);// [Control]-[F%d]
			AddEventList(G_cTxt.c_str(), 10);
		}
		else if (m_sShortCut[num] < 100)
		{
			InventoryManager::Get().EquipItem(static_cast<char>(m_sShortCut[num]));
		}
		else if (m_sShortCut[num] >= 100) MagicCastingSystem::Get().BeginCast(m_sShortCut[num] - 100);
	}
}

/*********************************************************************************************************************
**  void CheckActiveAura(short sX, short sY, DWORD dwTime, short sOwnerType)( initially Cleroth fixed by Snoopy )	**
**  description			: Generates special auras around players													**
**						: v351 implements this in each drawn function,beter to regroup in single function.			**
**********************************************************************************************************************/
void CGame::CheckActiveAura(short sX, short sY, uint32_t dwTime, short sOwnerType)
{	// Used at the beginning of character drawing
	// DefenseShield
	if (m_entityState.m_status.bDefenseShield)
		m_pEffectSpr[80]->Draw(sX + 75, sY + 107, m_entityState.m_iEffectFrame % 17, hb::shared::sprite::DrawParams::Alpha(0.5f));

	// Protection From Magic
	if (m_entityState.m_status.bMagicProtection)
		m_pEffectSpr[79]->Draw(sX + 101, sY + 135, m_entityState.m_iEffectFrame % 15, hb::shared::sprite::DrawParams::Alpha(0.7f));

	// Protection From Arrow
	if (m_entityState.m_status.bProtectionFromArrow)
		m_pEffectSpr[72]->Draw(sX, sY + 35, m_entityState.m_iEffectFrame % 30, hb::shared::sprite::DrawParams::Alpha(0.7f));

	// Illusion
	if (m_entityState.m_status.bIllusion)
		m_pEffectSpr[73]->Draw(sX + 125, sY + 130 - entity_visual::attacker_height[sOwnerType], m_entityState.m_iEffectFrame % 24, hb::shared::sprite::DrawParams::Alpha(0.7f));

	// Illusion movement
	if ((m_entityState.m_status.bIllusionMovement) != 0)
		m_pEffectSpr[151]->Draw(sX + 90, sY + 90 - entity_visual::attacker_height[sOwnerType], m_entityState.m_iEffectFrame % 24, hb::shared::sprite::DrawParams::Alpha(0.7f));

	// Slate red (HP)
	if (m_entityState.m_status.bSlateInvincible)
		m_pEffectSpr[149]->Draw(sX + 90, sY + 120, m_entityState.m_iEffectFrame % 15, hb::shared::sprite::DrawParams::Alpha(0.7f));

	// Slate Blue (Mana)
	if (m_entityState.m_status.bSlateMana)
		m_pEffectSpr[150]->Draw(sX + 1, sY + 26, m_entityState.m_iEffectFrame % 15, hb::shared::sprite::DrawParams::Alpha(0.7f));

	// Slate Green (XP)
	if (m_entityState.m_status.bSlateExp)
		m_pEffectSpr[148]->Draw(sX, sY + 32, m_entityState.m_iEffectFrame % 23, hb::shared::sprite::DrawParams::Alpha(0.7f));

	// Hero Flag (Heldenian)
	if (m_entityState.m_status.bHero)
		m_pEffectSpr[87]->Draw(sX + 53, sY + 54, m_entityState.m_iEffectFrame % 29, hb::shared::sprite::DrawParams::Alpha(0.7f));
}

/*********************************************************************************************************************
**  void CheckActiveAura2(short sX, short sY, DWORD dwTime,  m_entityState.m_sOwnerType) ( initially Cleroth fixed by Snoopy )	**
**  description			: Generates poison aura around players. This one should be use later...						**
**						: v351 implements this in each drawn function,beter to regroup in single function.			**
**********************************************************************************************************************/
void CGame::CheckActiveAura2(short sX, short sY, uint32_t dwTime, short sOwnerType)
{	// Poison
	if (m_entityState.m_status.bPoisoned)
		m_pEffectSpr[81]->Draw(sX + 115, sY + 120 - entity_visual::attacker_height[sOwnerType], m_entityState.m_iEffectFrame % 21, hb::shared::sprite::DrawParams::Alpha(0.7f));
}

void CGame::DrawAngel(int iSprite, short sX, short sY, char cFrame, uint32_t dwTime)
{
	switch (m_entityState.m_iDir)
	{
	case 1:
	case 2:
	case 7:
	case 8:
		sX -= 30;
		break;
	}
	if (m_entityState.m_status.bInvisibility)
	{
		if (m_entityState.m_status.bAngelSTR)
			m_pSprite[TutelaryAngelsPivotPoint + iSprite]->Draw(sX, sY, cFrame, hb::shared::sprite::DrawParams::Alpha(0.5f));  //AngelicPendant(STR)
		else if (m_entityState.m_status.bAngelDEX)
			m_pSprite[TutelaryAngelsPivotPoint + (50 * 1) + iSprite]->Draw(sX, sY, cFrame, hb::shared::sprite::DrawParams::Alpha(0.5f)); //AngelicPendant(DEX)
		else if (m_entityState.m_status.bAngelINT)
			m_pSprite[TutelaryAngelsPivotPoint + (50 * 2) + iSprite]->Draw(sX, sY - 15, cFrame, hb::shared::sprite::DrawParams::Alpha(0.5f));//AngelicPendant(INT)
		else if (m_entityState.m_status.bAngelMAG)
			m_pSprite[TutelaryAngelsPivotPoint + (50 * 3) + iSprite]->Draw(sX, sY - 15, cFrame, hb::shared::sprite::DrawParams::Alpha(0.5f));//AngelicPendant(MAG)
	}
	else
	{
		if (m_entityState.m_status.bAngelSTR)
			m_pSprite[TutelaryAngelsPivotPoint + iSprite]->Draw(sX, sY, cFrame);  //AngelicPendant(STR)
		else if (m_entityState.m_status.bAngelDEX)
			m_pSprite[TutelaryAngelsPivotPoint + (50 * 1) + iSprite]->Draw(sX, sY, cFrame); //AngelicPendant(DEX)
		else if (m_entityState.m_status.bAngelINT)
			m_pSprite[TutelaryAngelsPivotPoint + (50 * 2) + iSprite]->Draw(sX, sY - 15, cFrame);//AngelicPendant(INT)
		else if (m_entityState.m_status.bAngelMAG)
			m_pSprite[TutelaryAngelsPivotPoint + (50 * 3) + iSprite]->Draw(sX, sY - 15, cFrame);//AngelicPendant(MAG)
	}

}
/*********************************************************************************************************************
**  int CGame::bHasHeroSet( short m_sAppr3, short m_sAppr3, char OwnerType)		( Snoopy )							**
**  description			:: check weather the object (is character) is using a hero set (1:war, 2:mage)				**
**********************************************************************************************************************/
int CGame::bHasHeroSet(const hb::shared::entity::PlayerAppearance& appr, short OwnerType)
{
	char cArmor, cLeg, cBerk, cHat;
	cArmor = appr.iArmorType;
	cLeg = appr.iPantsType;
	cHat = appr.iHelmType;
	cBerk = appr.iArmArmorType;
	switch (OwnerType) {
	case 1:
	case 2:
	case 3:
		if ((cArmor == 8) && (cLeg == 5) && (cHat == 9) && (cBerk == 3)) return (1); // Warr elv M
		if ((cArmor == 9) && (cLeg == 6) && (cHat == 10) && (cBerk == 4)) return (1); // Warr ares M
		if ((cArmor == 10) && (cLeg == 5) && (cHat == 11) && (cBerk == 3)) return (2); // Mage elv M
		if ((cArmor == 11) && (cLeg == 6) && (cHat == 12) && (cBerk == 4)) return (2); // Mage ares M
		break;
	case 4:
	case 5:
	case 6: // fixed
		if ((cArmor == 9) && (cLeg == 6) && (cHat == 9) && (cBerk == 4)) return (1); //warr elv W
		if ((cArmor == 10) && (cLeg == 7) && (cHat == 10) && (cBerk == 5)) return (1); //warr ares W
		if ((cArmor == 11) && (cLeg == 6) && (cHat == 11) && (cBerk == 4)) return (2); //mage elv W
		if ((cArmor == 12) && (cLeg == 7) && (cHat == 12) && (cBerk == 5)) return (2); //mage ares W
		break;
	}
	return 0;
}
/*********************************************************************************************************************
**  void ShowHeldenianVictory( short sSide)				( Snoopy )													**
**  description			: Shows the Heldenian's End window															**
**********************************************************************************************************************/
void CGame::ShowHeldenianVictory(short side)
{
	int player_side = 0;
	m_dialogBoxManager.DisableDialogBox(DialogBoxId::Text);
	for (int i = 0; i < game_limits::max_text_dlg_lines; i++)
	{
		if (m_pMsgTextList[i] != 0)
			m_pMsgTextList[i].reset();
	}
	if (m_pPlayer->m_bCitizen == false) player_side = 0;
	else if (m_pPlayer->m_bAresden == true) player_side = 1;
	else if (m_pPlayer->m_bAresden == false) player_side = 2;
	switch (side) {
	case 0:
		PlayGameSound('E', 25, 0, 0);
		m_pMsgTextList[0] = std::make_unique<CMsg>(0, "Heldenian holy war has been closed!", 0);
		m_pMsgTextList[1] = std::make_unique<CMsg>(0, " ", 0);
		m_pMsgTextList[2] = std::make_unique<CMsg>(0, "Heldenian Holy war ended", 0);
		m_pMsgTextList[3] = std::make_unique<CMsg>(0, "in a tie.", 0);
		break;
	case 1:
		PlayGameSound('E', 25, 0, 0);
		m_pMsgTextList[0] = std::make_unique<CMsg>(0, "Heldenian holy war has been closed!", 0);
		m_pMsgTextList[1] = std::make_unique<CMsg>(0, " ", 0);
		m_pMsgTextList[2] = std::make_unique<CMsg>(0, "Heldenian Holy war ended", 0);
		m_pMsgTextList[3] = std::make_unique<CMsg>(0, "in favor of Aresden.", 0);
		break;
	case 2:
		PlayGameSound('E', 25, 0, 0);
		m_pMsgTextList[0] = std::make_unique<CMsg>(0, "Heldenian holy war has been closed!", 0);
		m_pMsgTextList[1] = std::make_unique<CMsg>(0, " ", 0);
		m_pMsgTextList[2] = std::make_unique<CMsg>(0, "Heldenian Holy war ended", 0);
		m_pMsgTextList[3] = std::make_unique<CMsg>(0, "in favor of Elvine.", 0);
		break;
	}
	m_pMsgTextList[4] = std::make_unique<CMsg>(0, " ", 0);

	if (((player_side != 1) && (player_side != 2))   // Player not a normal citizen
		|| (side == 0))								// or no winner
	{
		PlayGameSound('E', 25, 0, 0);
		m_pMsgTextList[5] = std::make_unique<CMsg>(0, " ", 0);
		m_pMsgTextList[6] = std::make_unique<CMsg>(0, " ", 0);
		m_pMsgTextList[7] = std::make_unique<CMsg>(0, " ", 0);
		m_pMsgTextList[8] = std::make_unique<CMsg>(0, " ", 0);
	}
	else
	{
		if (side == player_side)
		{
			PlayGameSound('E', 23, 0, 0);
			PlayGameSound('C', 21, 0, 0);
			PlayGameSound('C', 22, 0, 0);
			m_pMsgTextList[5] = std::make_unique<CMsg>(0, "Congratulation.", 0);
			m_pMsgTextList[6] = std::make_unique<CMsg>(0, "As cityzen of victory,", 0);
			m_pMsgTextList[7] = std::make_unique<CMsg>(0, "You will recieve a reward.", 0);
			m_pMsgTextList[8] = std::make_unique<CMsg>(0, "      ", 0);
		}
		else
		{
			PlayGameSound('E', 24, 0, 0);
			PlayGameSound('C', 12, 0, 0);
			PlayGameSound('C', 13, 0, 0);
			m_pMsgTextList[5] = std::make_unique<CMsg>(0, "To our regret", 0);
			m_pMsgTextList[6] = std::make_unique<CMsg>(0, "As cityzen of defeat,", 0);
			m_pMsgTextList[7] = std::make_unique<CMsg>(0, "You cannot recieve any reward.", 0);
			m_pMsgTextList[8] = std::make_unique<CMsg>(0, "     ", 0);
		}
	}
	for (int i = 9; i < 18; i++)
		m_pMsgTextList[i] = std::make_unique<CMsg>(0, " ", 0);
	m_dialogBoxManager.EnableDialogBox(DialogBoxId::Text, 0, 0, 0);
	m_dialogBoxManager.DisableDialogBox(DialogBoxId::CrusadeCommander);
	m_dialogBoxManager.DisableDialogBox(DialogBoxId::CrusadeConstructor);
	m_dialogBoxManager.DisableDialogBox(DialogBoxId::CrusadeSoldier);
}

/*********************************************************************************************************************
**  void 	ResponseHeldenianTeleportList(char *pData)									(  Snoopy )					**
**  description			: Gail's TP																					**
**********************************************************************************************************************/
/*********************************************************************************************************************
**  bool DKGlare(int iWeaponIndex, int iWeaponIndex, int *iWeaponGlare)	( Snoopy )									**
**  description			: test glowing condition for DK set															**
**********************************************************************************************************************/
void CGame::DKGlare(int iWeaponColor, int iWeaponIndex, int* iWeaponGlare)
{
	if (iWeaponColor != 9) return;
	if (((iWeaponIndex >= WeaponM + 64 * 14) && (iWeaponIndex < WeaponM + 64 * 14 + 56)) //msw3
		|| ((iWeaponIndex >= WeaponW + 64 * 14) && (iWeaponIndex < WeaponW + 64 * 14 + 56))) //wsw3
	{
		*iWeaponGlare = 3;
	}
	else if (((iWeaponIndex >= WeaponM + 64 * 37) && (iWeaponIndex < WeaponM + 64 * 37 + 56)) //MStaff3
		|| ((iWeaponIndex >= WeaponW + 64 * 37) && (iWeaponIndex < WeaponW + 64 * 37 + 56)))//WStaff3
	{
		*iWeaponGlare = 2;
	}
}
/*********************************************************************************************************************
**  void CGame::Abaddon_corpse(int sX, int sY);		( Snoopy )														**
**  description			: Placeholder for abaddon's death lightnings												**
**********************************************************************************************************************/
void CGame::Abaddon_corpse(int sX, int sY)
{
	int ir = (rand() % 20) - 10;
	WeatherManager::Get().DrawThunderEffect(sX + 30, 0, sX + 30, sY - 10, ir, ir, 1);
	WeatherManager::Get().DrawThunderEffect(sX + 30, 0, sX + 30, sY - 10, ir + 2, ir, 2);
	WeatherManager::Get().DrawThunderEffect(sX + 30, 0, sX + 30, sY - 10, ir - 2, ir, 2);
	ir = (rand() % 20) - 10;
	WeatherManager::Get().DrawThunderEffect(sX - 20, 0, sX - 20, sY - 35, ir, ir, 1);
	WeatherManager::Get().DrawThunderEffect(sX - 20, 0, sX - 20, sY - 35, ir + 2, ir, 2);
	WeatherManager::Get().DrawThunderEffect(sX - 20, 0, sX - 20, sY - 35, ir - 2, ir, 2);
	ir = (rand() % 20) - 10;
	WeatherManager::Get().DrawThunderEffect(sX - 10, 0, sX - 10, sY + 30, ir, ir, 1);
	WeatherManager::Get().DrawThunderEffect(sX - 10, 0, sX - 10, sY + 30, ir + 2, ir + 2, 2);
	WeatherManager::Get().DrawThunderEffect(sX - 10, 0, sX - 10, sY + 30, ir - 2, ir + 2, 2);
	ir = (rand() % 20) - 10;
	WeatherManager::Get().DrawThunderEffect(sX + 50, 0, sX + 50, sY + 35, ir, ir, 1);
	WeatherManager::Get().DrawThunderEffect(sX + 50, 0, sX + 50, sY + 35, ir + 2, ir + 2, 2);
	WeatherManager::Get().DrawThunderEffect(sX + 50, 0, sX + 50, sY + 35, ir - 2, ir + 2, 2);
	ir = (rand() % 20) - 10;
	WeatherManager::Get().DrawThunderEffect(sX + 65, 0, sX + 65, sY - 5, ir, ir, 1);
	WeatherManager::Get().DrawThunderEffect(sX + 65, 0, sX + 65, sY - 5, ir + 2, ir + 2, 2);
	WeatherManager::Get().DrawThunderEffect(sX + 65, 0, sX + 65, sY - 5, ir - 2, ir + 2, 2);
	ir = (rand() % 20) - 10;
	WeatherManager::Get().DrawThunderEffect(sX + 45, 0, sX + 45, sY - 50, ir, ir, 1);
	WeatherManager::Get().DrawThunderEffect(sX + 45, 0, sX + 45, sY - 50, ir + 2, ir + 2, 2);
	WeatherManager::Get().DrawThunderEffect(sX + 45, 0, sX + 45, sY - 50, ir - 2, ir + 2, 2);

	for (int x = sX - 50; x <= sX + 100; x += rand() % 35)
	{
		for (int y = sY - 30; y <= sY + 50; y += rand() % 45)
		{
			ir = (rand() % 20) - 10;
			WeatherManager::Get().DrawThunderEffect(x, 0, x, y, ir, ir, 2);
		}
	}
}